/*++
linux/drivers/input/keyboard/wmt_kpad.c

Some descriptions of such software. Copyright (c) 2008  WonderMedia Technologies, Inc.

This program is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software Foundation,
either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with
this program.  If not, see <http://www.gnu.org/licenses/>.

WonderMedia Technologies, Inc.
10F, 529, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C.
--*/

#include <linux/module.h>
#include <linux/init.h> 
#include <linux/interrupt.h>
#include <linux/types.h>
#include <linux/input.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/cpufreq.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/errno.h>
#include <asm/mach-types.h>
#include <mach/hardware.h>
#include <mach/kpad.h>
#include <linux/suspend.h>



/* Debug macros */
#if 0
#define DPRINTK(fmt, args...) printk(KERN_ALERT "[%s]: " fmt, __func__ , ## args)
#else
#define DPRINTK(fmt, args...)
#endif

#define USE_HOME
#define wmt_kpad_timeout (HZ/100)*10
#ifdef USE_HOME
#define WMT_KPAD_FUNCTION_NUM  5
#else 
#define WMT_KPAD_FUNCTION_NUM  4
#endif
#define DIRECT_EN (BIT0 | BIT1 | BIT2 | BIT3)
static unsigned int wmt_kpad_codes[WMT_KPAD_FUNCTION_NUM] = {  
	[0]	= KEY_VOLUMEUP,
	[1]	= KEY_VOLUMEDOWN,
	[2] = KEY_BACK,
    [3] = KEY_MENU,
#ifdef USE_HOME
	[4] = KEY_HOME,
#endif
};    
#define WMT_ROW0_KEY_NUM 2
#define WMT_ROW1_KEY_NUM 3
#define WMT_ROW2_KEY_NUM 0
#define WMT_ROW3_KEY_NUM 1
#define WMT_ROW0_1_KEY_NUM 3

#ifdef USE_HOME
#define WMT_GPIO10_KEY_NUM 4
#endif

#if 0
#define WMT_ROW4_KEY_NUM 2
#define WMT_ROW5_KEY_NUM 3
#define WMT_ROW6_KEY_NUM 0
#define WMT_ROW7_KEY_NUM 1
#define WMT_ROW4_5_KEY_NUM 3
#endif

static struct timer_list   wmt_kpad_timer_row0;
static struct timer_list   wmt_kpad_timer_row1;
static struct timer_list   wmt_kpad_timer_row2;
static struct timer_list   wmt_kpad_timer_row3;

#ifdef USE_HOME
static struct timer_list   wmt_kpad_timer_gpio10;
#endif

#if 0
static struct timer_list   wmt_kpad_timer_row4;
static struct timer_list   wmt_kpad_timer_row5;
static struct timer_list   wmt_kpad_timer_row6;
static struct timer_list   wmt_kpad_timer_row7;
#endif

static struct input_dev *kpad_dev;

static struct wmt_kpad_s kpad = {
	.ref	= 0,
	.res	= NULL,
	.regs   = NULL,
	.irq	= 0,
	.ints   = { 0, 0, 0, 0, 0 },
};

int enable_keypad = 1;
int back_menu_timeout = 0; /*0.1s unit*/
unsigned int back_menu_timeout_counter = 0;
bool back_menu_pressed = false;

#define KEYPAD_ROW0_GPIO_pin BIT0
#define KEYPAD_ROW1_GPIO_pin BIT1
#define KEYPAD_ROW2_GPIO_pin BIT2
#define KEYPAD_ROW3_GPIO_pin BIT3

#if 0
#define KEYPAD_ROW4_GPIO_pin BIT1
#define KEYPAD_ROW5_GPIO_pin BIT2
#define KEYPAD_ROW6_GPIO_pin BIT3
#define KEYPAD_ROW7_GPIO_pin BIT0
#endif

#define KEYPAD_GPIO_MASK (KEYPAD_ROW0_GPIO_pin | \
                          KEYPAD_ROW1_GPIO_pin | \
                          KEYPAD_ROW2_GPIO_pin | \
                          KEYPAD_ROW3_GPIO_pin)
                          
//#define KEYPAD_PIN_SHARE (BIT5 | BIT6)

#define KEYPAD_PIN_SHARING_SEL_VAL GPIO_PIN_SHARING_SEL_4BYTE_VAL
#define KEYPAD_GPIO_CTRL_VAL GPIO_CTRL_GP26_KPAD_BYTE_VAL
#define KEYPDA_GPIO_OC_VAL GPIO_OC_GP26_KPAD_BYTE_VAL
#define KEYPAD_GPIO_ID_VAL GPIO_ID_GP26_KPAD_BYTE_VAL
#define KEYPAD_GPIO_PULL_CTRL_VAL GPIO_PULL_CTRL_GP26_KPAD_BYTE_VAL
#define KEYPAD_GPIO_PULL_EN_VAL GPIO_PULL_EN_GP26_KPAD_BYTE_VAL
#ifdef CONFIG_CPU_FREQ
/*
 * Well, the debounce time is not very critical while zac2_clk
 * rescaling, but we still do it.
 */

/* kpad_clock_notifier()
 *
 * When changing the processor core clock frequency, it is necessary
 * to adjust the KPMIR register.
 *
 * Returns: 0 on success, -1 on error
 */
static int kpad_clock_notifier(struct notifier_block *nb, unsigned long event,
	void *data)
{

	switch (event) {
	case CPUFREQ_PRECHANGE:
		/*
		 * Disable input.
		 */
		kpad.regs->kpmcr &= ~KPMCR_EN;
		break;

	case CPUFREQ_POSTCHANGE:
		/*
		 * Adjust debounce time then enable input.
		 */
		kpad.regs->kpmir = KPMIR_DI((125 * wm8510_ahb_khz()) / \
			(262144)) | KPMIR_SI(0xFF);
		kpad.regs->kpmcr |= KPMCR_EN;
		break;
	}

	return 0;
}

/*
 * Notify callback while issusing zac2_clk rescale.
 */
static struct notifier_block kpad_clock_nblock = {
	.notifier_call  = kpad_clock_notifier,
	.priority = 1
};
#endif

#ifdef USE_HOME
static void wmt_gpio_home_key_hw_init(void)
{	
	GPIO_PIN_SHARING_SEL_4BYTE_VAL &= ~BIT4; // as gpio
	GPIO10_INT_REQ_TYPE_VAL |= BIT1; //falling edge irq type
	GPIO10_INT_REQ_TYPE_VAL &= ~(BIT0 | BIT2);
	GPIO_CTRL_GP1_BYTE_VAL &= ~BIT2; //enable gpio
	GPIO_OC_GP1_BYTE_VAL &= ~BIT2;  //as input
	GPIO_PULL_CTRL_GP1_BYTE_VAL |= BIT2;
	GPIO_PULL_EN_GP1_BYTE_VAL |= BIT2; //disable pull
	GPIO1_INT_REQ_STS_VAL |= BIT2; /*Clear interrupt*/
	GPIO10_INT_REQ_TYPE_VAL |= BIT7; //enable irq10
}

static void wmt_control_home_irq(int on)
{
	if (on == 0)
		GPIO10_INT_REQ_TYPE_VAL &= ~BIT7; //disable GPIO10 irq
	else
		GPIO10_INT_REQ_TYPE_VAL |= BIT7; //enable irq10
}
#endif

static void wmt_kpad_hw_init(void)
{
    unsigned int status;
    DPRINTK("Start\n");

	/*Set ROW4~7 share pin to KPAD mode*/
    KEYPAD_PIN_SHARING_SEL_VAL |= BIT5;
	KEYPAD_PIN_SHARING_SEL_VAL &= ~BIT6;
      
	/*Enable ROW0~3*/
    KEYPAD_GPIO_CTRL_VAL &= ~KEYPAD_GPIO_MASK;

	/*Enable ROW0~3 Pull-up/down & pull down*/
    //KEYPAD_GPIO_PULL_CTRL_VAL &= ~KEYPAD_GPIO_MASK;
    KEYPAD_GPIO_PULL_CTRL_VAL |= KEYPAD_GPIO_MASK;
    KEYPAD_GPIO_PULL_EN_VAL |= KEYPAD_GPIO_MASK;

    /*
	 * Turn on keypad clocks.
	 */
    auto_pll_divisor(DEV_KEYPAD, CLK_ENABLE, 0, 0);

	/* Clean keypad matrix input control registers. */
	kpad.regs->kpmcr = 0;

	/*
	 * Simply disable keypad direct input function first.
	 * Also clear all direct input enable bits.
	 */
	kpad.regs->kpdcr = 0;
    status = kpad.regs->kpdsr;

    /*
	 * Simply clean any exist keypad matrix status.
	 */
	status = kpad.regs->kpstr;
	kpad.regs->kpstr |= status;
	if (kpad.regs->kpstr != 0)
		printk(KERN_ERR "[kpad] clear status failed!\n");

   // printk("[%s] 5\n",__func__);    
	/*
	 * Set keypad debounce time to be about 125 ms.
	 */
	kpad.regs->kpmir = KPMIR_DI(0x0FFF) | KPMIR_SI(0x01);
    kpad.regs->kpdir = KPDIR_DI(0x0FFF);

    /*
	 * Enable keypad direct input with interrupt enabled and
	 * automatic scan on activity.
	 */
	/*Active High*/
    //kpad.regs->kpicr &= ~KPICR_IRIMASK;
    /*Active Low*/
    kpad.regs->kpicr |= KPICR_IRIMASK;
    
    /*Ignore Multiple Key press disable*/
	kpad.regs->kpdcr |= KPDCR_EN | KPDCR_IEN | KPDCR_ASA | KPDCR_DEN(DIRECT_EN) ;//|KPDCR_IMK;
    
    DPRINTK("End\n");
}

static void wmt_kpad_timeout_row0(unsigned long fcontext)
{
    DPRINTK("Start\n");
    if (back_menu_timeout == 0) {
        //if (KEYPAD_GPIO_ID_VAL & KEYPAD_ROW0_GPIO_pin) { /*Active High*/
        if ((KEYPAD_GPIO_ID_VAL & KEYPAD_ROW0_GPIO_pin) == 0) {   /*Active Low*/
            mod_timer(&wmt_kpad_timer_row0, jiffies + wmt_kpad_timeout);
    		DPRINTK("WMT_ROW0_KEY_NUM keep press\n");
        } else {
            input_report_key(kpad_dev, wmt_kpad_codes[WMT_ROW0_KEY_NUM], 0); /*row4 key is release*/
        	input_sync(kpad_dev);
            KEYPAD_GPIO_CTRL_VAL &= ~KEYPAD_ROW0_GPIO_pin;
            kpad.regs->kpdcr |= KPDCR_DEN(BIT0);
            DPRINTK("WMT_ROW0_KEY_NUM release key = %d \n",
                wmt_kpad_codes[WMT_ROW0_KEY_NUM]);
        }
    } else {
        back_menu_timeout_counter++;
        //if (KEYPAD_GPIO_ID_VAL & KEYPAD_ROW0_GPIO_pin) { /*Active High*/
        if ((KEYPAD_GPIO_ID_VAL & KEYPAD_ROW0_GPIO_pin) == 0) {   /*Active Low*/
            if (back_menu_timeout_counter == back_menu_timeout) {
				input_report_key(kpad_dev, wmt_kpad_codes[WMT_ROW0_1_KEY_NUM], 1); /*row0_1 key is pressed*/
		        input_sync(kpad_dev);
				input_report_key(kpad_dev, wmt_kpad_codes[WMT_ROW0_1_KEY_NUM], 0); /*row0_1 key is release*/
	        	input_sync(kpad_dev);
				DPRINTK("WMT_ROW0_1_KEY_NUM report key = %d\n",wmt_kpad_codes[WMT_ROW0_1_KEY_NUM]);
			}
            mod_timer(&wmt_kpad_timer_row0, jiffies + wmt_kpad_timeout);
    		DPRINTK("WMT_ROW0_KEY_NUM keep press\n");
        } else {
            if (back_menu_timeout_counter < back_menu_timeout) {
                DPRINTK("back_menu_timeout_counter = %d back_menu_timeout = %d \n",back_menu_timeout_counter,back_menu_timeout);
				input_report_key(kpad_dev, wmt_kpad_codes[WMT_ROW0_KEY_NUM], 1); /*row0 key is pressed*/
	        	input_sync(kpad_dev);
				DPRINTK("WMT_ROW0_KEY_NUM press 2 key = %d\n",wmt_kpad_codes[WMT_ROW0_KEY_NUM]);
				input_report_key(kpad_dev, wmt_kpad_codes[WMT_ROW0_KEY_NUM], 0); /*row0 key is release*/
	        	input_sync(kpad_dev);
				back_menu_pressed = false;
				DPRINTK("WMT_ROW0_KEY_NUM release 2 key = %d\n",wmt_kpad_codes[WMT_ROW0_KEY_NUM]);
            } else {
                DPRINTK("back_menu_timeout_counter = %d back_menu_timeout = %d \n",back_menu_timeout_counter,back_menu_timeout);
				back_menu_pressed = false;
				DPRINTK("WMT_ROW0_KEY_NUM release\n");
            }

            KEYPAD_GPIO_CTRL_VAL &= ~KEYPAD_ROW0_GPIO_pin;
            kpad.regs->kpdcr |= KPDCR_DEN(BIT0);
        }
    }
    DPRINTK("End\n");

}

static void wmt_kpad_timeout_row1(unsigned long fcontext)
{
    DPRINTK("Start\n");
    
    //if (KEYPAD_GPIO_ID_VAL & KEYPAD_ROW1_GPIO_pin) { /*Active High*/
    if ((KEYPAD_GPIO_ID_VAL & KEYPAD_ROW1_GPIO_pin) == 0) {   /*Active Low*/
        mod_timer(&wmt_kpad_timer_row1, jiffies + wmt_kpad_timeout);
		DPRINTK("WMT_ROW1_KEY_NUM keep press\n");
    } else {
        input_report_key(kpad_dev, wmt_kpad_codes[WMT_ROW1_KEY_NUM], 0); /*row1 key is release*/
    	input_sync(kpad_dev);
        KEYPAD_GPIO_CTRL_VAL &= ~KEYPAD_ROW1_GPIO_pin;
        kpad.regs->kpdcr |= KPDCR_DEN(BIT1);
        DPRINTK("WMT_ROW1_KEY_NUM release key = %d \n",
            wmt_kpad_codes[WMT_ROW1_KEY_NUM]);
    }
    DPRINTK("End\n");
}

static void wmt_kpad_timeout_row2(unsigned long fcontext)
{
    DPRINTK("Start\n");
    
    //if (KEYPAD_GPIO_ID_VAL & KEYPAD_ROW2_GPIO_pin) { /*Active High*/
    if ((KEYPAD_GPIO_ID_VAL & KEYPAD_ROW2_GPIO_pin) == 0) {   /*Active Low*/
        mod_timer(&wmt_kpad_timer_row2, jiffies + wmt_kpad_timeout);
		DPRINTK("WMT_ROW2_KEY_NUM keep press\n");
    } else {
        input_report_key(kpad_dev, wmt_kpad_codes[WMT_ROW2_KEY_NUM], 0); /*row2 key is release*/
    	input_sync(kpad_dev);
        KEYPAD_GPIO_CTRL_VAL &= ~KEYPAD_ROW2_GPIO_pin;
        kpad.regs->kpdcr |= KPDCR_DEN(BIT2);
        DPRINTK("WMT_ROW2_KEY_NUM release key = %d \n",
            wmt_kpad_codes[WMT_ROW2_KEY_NUM]);
    }
    DPRINTK("End\n");
}

static void wmt_kpad_timeout_row3(unsigned long fcontext)
{
    DPRINTK("Start\n");
    //printk("[%s] KEYPAD_GPIO_ID_VAL = %x\n",__func__,KEYPAD_GPIO_ID_VAL);
    //if (KEYPAD_GPIO_ID_VAL & KEYPAD_ROW3_GPIO_pin) { /*Active High*/
    if ((KEYPAD_GPIO_ID_VAL & KEYPAD_ROW3_GPIO_pin) == 0) {   /*Active Low*/
        mod_timer(&wmt_kpad_timer_row3, jiffies + wmt_kpad_timeout);
		DPRINTK("WMT_ROW3_KEY_NUM keep press\n");
    } else {
        input_report_key(kpad_dev, wmt_kpad_codes[WMT_ROW3_KEY_NUM], 0); /*row3 key is release*/
    	input_sync(kpad_dev);
        KEYPAD_GPIO_CTRL_VAL &= ~KEYPAD_ROW3_GPIO_pin;
        kpad.regs->kpdcr |= KPDCR_DEN(BIT3);
        DPRINTK("WMT_ROW3_KEY_NUM release key = %d \n",
            wmt_kpad_codes[WMT_ROW3_KEY_NUM]);
    }
    DPRINTK("End\n");
}

#ifdef USE_HOME
static void wmt_kpad_timeout_gpio10(unsigned long fcontext)
{
    DPRINTK("Start\n");
    if ((GPIO_ID_GP1_BYTE_VAL & BIT2) == 0) {   /*Active Low*/
        mod_timer(&wmt_kpad_timer_gpio10, jiffies + wmt_kpad_timeout);
		DPRINTK("[WMT_GPIO10_KEY_NUM keep press\n");
    } else {
        input_report_key(kpad_dev, wmt_kpad_codes[WMT_GPIO10_KEY_NUM], 0); /*row3 key is release*/
    	input_sync(kpad_dev);
        DPRINTK("WMT_GPIO10_KEY_NUM release key = %d \n",
            wmt_kpad_codes[WMT_GPIO10_KEY_NUM]);
        //GPIO10_INT_REQ_TYPE_VAL |= BIT7; //enable irq10
        wmt_control_home_irq(1);
    }
    DPRINTK("End\n");
}
#endif

#if 0
static void wmt_kpad_timeout_row4(unsigned long fcontext)
{
    DPRINTK("Start\n");
    if (back_menu_timeout == 0) {
        //if (KEYPAD_GPIO_ID_VAL & KEYPAD_ROW4_GPIO_pin) { /*Active High*/
        if ((KEYPAD_GPIO_ID_VAL & KEYPAD_ROW4_GPIO_pin) == 0) {   /*Active Low*/
            mod_timer(&wmt_kpad_timer_row4, jiffies + wmt_kpad_timeout);
    		DPRINTK("WMT_ROW4_KEY_NUM keep press\n");
        } else {
            input_report_key(kpad_dev, wmt_kpad_codes[WMT_ROW4_KEY_NUM], 0); /*row4 key is release*/
        	input_sync(kpad_dev);
            KEYPAD_GPIO_CTRL_VAL &= ~KEYPAD_ROW4_GPIO_pin;
            kpad.regs->kpdcr |= KPDCR_DEN(BIT4);
            DPRINTK("WMT_ROW4_KEY_NUM release key = %d \n",
                wmt_kpad_codes[WMT_ROW4_KEY_NUM]);
        }
    } else {
        back_menu_timeout_counter++;
        //if (KEYPAD_GPIO_ID_VAL & KEYPAD_ROW4_GPIO_pin) { /*Active High*/
        if ((KEYPAD_GPIO_ID_VAL & KEYPAD_ROW4_GPIO_pin) == 0) {   /*Active Low*/
            if (back_menu_timeout_counter == back_menu_timeout) {
				input_report_key(kpad_dev, wmt_kpad_codes[WMT_ROW4_5_KEY_NUM], 1); /*row4_5 key is pressed*/
		        input_sync(kpad_dev);
				input_report_key(kpad_dev, wmt_kpad_codes[WMT_ROW4_5_KEY_NUM], 0); /*row4_5 key is release*/
	        	input_sync(kpad_dev);
				DPRINTK("WMT_ROW4_5_KEY_NUM report key = %d\n",wmt_kpad_codes[WMT_ROW4_5_KEY_NUM]);
			}
            mod_timer(&wmt_kpad_timer_row4, jiffies + wmt_kpad_timeout);
    		DPRINTK("WMT_ROW4_KEY_NUM keep press\n");
        } else {
            if (back_menu_timeout_counter < back_menu_timeout) {
                DPRINTK("back_menu_timeout_counter = %d back_menu_timeout = %d \n",back_menu_timeout_counter,back_menu_timeout);
				input_report_key(kpad_dev, wmt_kpad_codes[WMT_ROW4_KEY_NUM], 1); /*row4 key is pressed*/
	        	input_sync(kpad_dev);
				DPRINTK("WMT_ROW4_KEY_NUM press 2 key = %d\n",wmt_kpad_codes[WMT_ROW4_KEY_NUM]);
				input_report_key(kpad_dev, wmt_kpad_codes[WMT_ROW4_KEY_NUM], 0); /*row4 key is release*/
	        	input_sync(kpad_dev);
				back_menu_pressed = false;
				DPRINTK("WMT_ROW4_KEY_NUM release 2 key = %d\n",wmt_kpad_codes[WMT_ROW4_KEY_NUM]);
            } else {
                DPRINTK("back_menu_timeout_counter = %d back_menu_timeout = %d \n",back_menu_timeout_counter,back_menu_timeout);
				back_menu_pressed = false;
				DPRINTK("WMT_ROW4_KEY_NUM release\n");
            }

            KEYPAD_GPIO_CTRL_VAL &= ~KEYPAD_ROW4_GPIO_pin;
            kpad.regs->kpdcr |= KPDCR_DEN(BIT4);
        }
    }
    DPRINTK("End\n");

}

static void wmt_kpad_timeout_row5(unsigned long fcontext)
{
    DPRINTK("Start\n");
    
    //if (KEYPAD_GPIO_ID_VAL & KEYPAD_ROW5_GPIO_pin) { /*Active High*/
    if ((KEYPAD_GPIO_ID_VAL & KEYPAD_ROW5_GPIO_pin) == 0) {   /*Active Low*/
        mod_timer(&wmt_kpad_timer_row5, jiffies + wmt_kpad_timeout);
		DPRINTK("WMT_ROW5_KEY_NUM keep press\n");
    } else {
        input_report_key(kpad_dev, wmt_kpad_codes[WMT_ROW5_KEY_NUM], 0); /*row5 key is release*/
    	input_sync(kpad_dev);
        KEYPAD_GPIO_CTRL_VAL &= ~KEYPAD_ROW5_GPIO_pin;
        kpad.regs->kpdcr |= KPDCR_DEN(BIT5);
        DPRINTK("WMT_ROW5_KEY_NUM release key = %d \n",
            wmt_kpad_codes[WMT_ROW5_KEY_NUM]);
    }
    DPRINTK("End\n");
}

static void wmt_kpad_timeout_row6(unsigned long fcontext)
{
    DPRINTK("Start\n");
    
    //if (KEYPAD_GPIO_ID_VAL & KEYPAD_ROW6_GPIO_pin) { /*Active High*/
    if ((KEYPAD_GPIO_ID_VAL & KEYPAD_ROW6_GPIO_pin) == 0) {   /*Active Low*/
        mod_timer(&wmt_kpad_timer_row6, jiffies + wmt_kpad_timeout);
		DPRINTK("WMT_ROW6_KEY_NUM keep press\n");
    } else {
        input_report_key(kpad_dev, wmt_kpad_codes[WMT_ROW6_KEY_NUM], 0); /*row6 key is release*/
    	input_sync(kpad_dev);
        KEYPAD_GPIO_CTRL_VAL &= ~KEYPAD_ROW6_GPIO_pin;
        kpad.regs->kpdcr |= KPDCR_DEN(BIT6);
        DPRINTK("WMT_ROW6_KEY_NUM release key = %d \n",
            wmt_kpad_codes[WMT_ROW6_KEY_NUM]);
    }
    DPRINTK("End\n");
}

static void wmt_kpad_timeout_row7(unsigned long fcontext)
{
    DPRINTK("Start\n");
    //printk("[%s] KEYPAD_GPIO_ID_VAL = %x\n",__func__,KEYPAD_GPIO_ID_VAL);
    //if (KEYPAD_GPIO_ID_VAL & KEYPAD_ROW7_GPIO_pin) { /*Active High*/
    if ((KEYPAD_GPIO_ID_VAL & KEYPAD_ROW7_GPIO_pin) == 0) {   /*Active Low*/
        mod_timer(&wmt_kpad_timer_row7, jiffies + wmt_kpad_timeout);
		DPRINTK("WMT_ROW7_KEY_NUM keep press\n");
    } else {
        input_report_key(kpad_dev, wmt_kpad_codes[WMT_ROW7_KEY_NUM], 0); /*row7 key is release*/
    	input_sync(kpad_dev);
        KEYPAD_GPIO_CTRL_VAL &= ~KEYPAD_ROW7_GPIO_pin;
        kpad.regs->kpdcr |= KPDCR_DEN(BIT7);
        DPRINTK("WMT_ROW7_KEY_NUM release key = %d \n",
            wmt_kpad_codes[WMT_ROW7_KEY_NUM]);
    }
    DPRINTK("End\n");
}
#endif

#ifdef USE_HOME
static irqreturn_t wmt_home_key_interrupt(int irq, void *dev_id)
{
	unsigned int status;

	DPRINTK("[%s]s\n",__func__);

	wmt_control_home_irq(0); //disable GPIO10 irq 
	
	if (!(GPIO1_INT_REQ_STS_VAL & BIT2)) {
		wmt_control_home_irq(1); //Enable GPIO10 irq 
		return IRQ_NONE;
	}
	  
	status = GPIO1_INT_REQ_STS_VAL;
	GPIO1_INT_REQ_STS_VAL = BIT2; /*Clear interrupt*/

	if (GPIO1_INT_REQ_STS_VAL & BIT2)
		DPRINTK("[GPIO10] Clear interrupt failed.\n");

	if (!(status & BIT2)) {
		printk("[%s] Dummy interrupt for GPIO10, status = 0x%x\n", __func__,status);
		wmt_control_home_irq(1);
		goto OUT;
	} else {
		input_report_key(kpad_dev, wmt_kpad_codes[WMT_GPIO10_KEY_NUM], 1); /*gpio10 key is pressed*/
        input_sync(kpad_dev);
    	mod_timer(&wmt_kpad_timer_gpio10, jiffies + wmt_kpad_timeout);
    	DPRINTK("[%s]WMT_GPIO10_KEY_NUM press = %d\n", __func__,wmt_kpad_codes[WMT_GPIO10_KEY_NUM]);
	}
OUT:
	DPRINTK("[%s]e\n",__func__);
	return IRQ_HANDLED;
}
#endif

static irqreturn_t
kpad_interrupt(int irq, void *dev_id)
{
	unsigned int status;
    unsigned int rvalue;
    DPRINTK("Start\n");
	/* Disable interrupt */
	disable_irq_nosync(kpad.irq);
	kpad.regs->kpdcr &= ~(KPDCR_EN | KPDCR_IEN) ;
 
	/*
	 * Get keypad interrupt status and clean interrput source.
	 */
	status = kpad.regs->kpstr;
	kpad.regs->kpstr |= status;

	if (kpad.regs->kpstr != 0)
		printk("[kpad] status clear failed! \n");

	if (status & KPSTR_DIA) {
        rvalue = kpad.regs->kpdsr;

        if (rvalue & Dir_Vaild_Scan) {
            if ((rvalue & Dir_Input) == BIT0) {
                /*disable row0 and set as gpi pin*/
                kpad.regs->kpdcr &= ~ KPDCR_DEN(BIT0);
                KEYPAD_GPIO_CTRL_VAL |= KEYPAD_ROW0_GPIO_pin;
                KEYPDA_GPIO_OC_VAL &= ~KEYPAD_ROW0_GPIO_pin;
                if (back_menu_timeout == 0) {
        			input_report_key(kpad_dev, wmt_kpad_codes[WMT_ROW0_KEY_NUM], 1); /*row0 key is pressed*/
                	input_sync(kpad_dev);
        			mod_timer(&wmt_kpad_timer_row0, jiffies + wmt_kpad_timeout);
        			DPRINTK("WMT_ROW0_KEY_NUM press\n");
                } else {
                    if (back_menu_pressed == false) {
    					back_menu_timeout_counter = 0;
    					back_menu_pressed = true;
				    }
                    mod_timer(&wmt_kpad_timer_row0, jiffies + wmt_kpad_timeout);
                    DPRINTK("WMT_ROW0_KEY_NUM press 2\n");
                }
		    }

            if ((rvalue & Dir_Input) == BIT1) {
                /*disable row1 and set as gpi pin*/
                kpad.regs->kpdcr &= ~ KPDCR_DEN(BIT1);
                KEYPAD_GPIO_CTRL_VAL |= KEYPAD_ROW1_GPIO_pin;
                KEYPDA_GPIO_OC_VAL &= ~KEYPAD_ROW1_GPIO_pin;
                
    			input_report_key(kpad_dev, wmt_kpad_codes[WMT_ROW1_KEY_NUM], 1); /*row1 key is pressed*/
            	input_sync(kpad_dev);
    			mod_timer(&wmt_kpad_timer_row1, jiffies + wmt_kpad_timeout);
    			DPRINTK("WMT_ROW1_KEY_NUM press\n");
		    }

            if ((rvalue & Dir_Input) == BIT2) {
                /*disable row2 and set as gpi pin*/
                kpad.regs->kpdcr &= ~ KPDCR_DEN(BIT2);
                KEYPAD_GPIO_CTRL_VAL |= KEYPAD_ROW2_GPIO_pin;
                KEYPDA_GPIO_OC_VAL &= ~KEYPAD_ROW2_GPIO_pin;
                
    			input_report_key(kpad_dev, wmt_kpad_codes[WMT_ROW2_KEY_NUM], 1); /*row6 key is pressed*/
            	input_sync(kpad_dev);
    			mod_timer(&wmt_kpad_timer_row2, jiffies + wmt_kpad_timeout);
    			DPRINTK("WMT_ROW2_KEY_NUM press\n");
		    }

            if ((rvalue & Dir_Input) == BIT3) {
                /*disable row3 and set as gpi pin*/
                kpad.regs->kpdcr &= ~ KPDCR_DEN(BIT3);
                KEYPAD_GPIO_CTRL_VAL |= KEYPAD_ROW3_GPIO_pin;
                KEYPDA_GPIO_OC_VAL &= ~KEYPAD_ROW3_GPIO_pin;
                
    			input_report_key(kpad_dev, wmt_kpad_codes[WMT_ROW3_KEY_NUM], 1); /*row3 key is pressed*/
            	input_sync(kpad_dev);
    			mod_timer(&wmt_kpad_timer_row3, jiffies + wmt_kpad_timeout);
    			DPRINTK("WMT_ROW3_KEY_NUM press\n");
		    }
#if 0            
            if ((rvalue & Dir_Input) == BIT4) {
                /*disable row4 and set as gpi pin*/
                kpad.regs->kpdcr &= ~ KPDCR_DEN(BIT4);
                KEYPAD_GPIO_CTRL_VAL |= KEYPAD_ROW4_GPIO_pin;
                KEYPDA_GPIO_OC_VAL &= ~KEYPAD_ROW4_GPIO_pin;
                if (back_menu_timeout == 0) {
        			input_report_key(kpad_dev, wmt_kpad_codes[WMT_ROW4_KEY_NUM], 1); /*row4 key is pressed*/
                	input_sync(kpad_dev);
        			mod_timer(&wmt_kpad_timer_row4, jiffies + wmt_kpad_timeout);
        			DPRINTK("WMT_ROW4_KEY_NUM press\n");
                } else {
                    if (back_menu_pressed == false) {
    					back_menu_timeout_counter = 0;
    					back_menu_pressed = true;
				    }
                    mod_timer(&wmt_kpad_timer_row4, jiffies + wmt_kpad_timeout);
                    DPRINTK("WMT_ROW4_KEY_NUM press 2\n");
                }
		    }

            if ((rvalue & Dir_Input) == BIT5) {
                /*disable row5 and set as gpi pin*/
                kpad.regs->kpdcr &= ~ KPDCR_DEN(BIT5);
                KEYPAD_GPIO_CTRL_VAL |= KEYPAD_ROW5_GPIO_pin;
                KEYPDA_GPIO_OC_VAL &= ~KEYPAD_ROW5_GPIO_pin;
                
    			input_report_key(kpad_dev, wmt_kpad_codes[WMT_ROW5_KEY_NUM], 1); /*row5 key is pressed*/
            	input_sync(kpad_dev);
    			mod_timer(&wmt_kpad_timer_row5, jiffies + wmt_kpad_timeout);
    			DPRINTK("WMT_ROW5_KEY_NUM press\n");
		    }

            if ((rvalue & Dir_Input) == BIT6) {
                /*disable row6 and set as gpi pin*/
                kpad.regs->kpdcr &= ~ KPDCR_DEN(BIT6);
                KEYPAD_GPIO_CTRL_VAL |= KEYPAD_ROW6_GPIO_pin;
                KEYPDA_GPIO_OC_VAL &= ~KEYPAD_ROW6_GPIO_pin;
                
    			input_report_key(kpad_dev, wmt_kpad_codes[WMT_ROW6_KEY_NUM], 1); /*row6 key is pressed*/
            	input_sync(kpad_dev);
    			mod_timer(&wmt_kpad_timer_row6, jiffies + wmt_kpad_timeout);
    			DPRINTK("WMT_ROW6_KEY_NUM press\n");
		    }

            if ((rvalue & Dir_Input) == BIT7) {
                /*disable row7 and set as gpi pin*/
                kpad.regs->kpdcr &= ~ KPDCR_DEN(BIT7);
                KEYPAD_GPIO_CTRL_VAL |= KEYPAD_ROW7_GPIO_pin;
                KEYPDA_GPIO_OC_VAL &= ~KEYPAD_ROW7_GPIO_pin;
                
    			input_report_key(kpad_dev, wmt_kpad_codes[WMT_ROW7_KEY_NUM], 1); /*row7 key is pressed*/
            	input_sync(kpad_dev);
    			mod_timer(&wmt_kpad_timer_row7, jiffies + wmt_kpad_timeout);
    			DPRINTK("WMT_ROW7_KEY_NUM press\n");
		    }
#endif
        } 
	} else
		printk(KERN_WARNING "KPSTR=0x%.8x\n", status);

	/* Enable interrupt */
    kpad.regs->kpdcr |= (KPDCR_EN | KPDCR_IEN) ;
	enable_irq(kpad.irq);
    DPRINTK("End\n");
	return IRQ_HANDLED;
}

static int kpad_open(struct input_dev *dev)
{
	int ret = 0;
	unsigned int i;
	DPRINTK("Start\n");
    
	if (kpad.ref++) {
		/* Return success, but not initialize again. */
		DPRINTK("End 1\n");
		return 0;
	}

	ret = request_irq(kpad.irq, kpad_interrupt, IRQF_DISABLED, "keypad", dev);

	if (ret) {
		printk(KERN_ERR "%s: Can't allocate irq %d\n", __func__, IRQ_KPAD);
		kpad.ref--;
		goto kpad_open_out;
	}
	
	/*Home key*/
#ifdef USE_HOME
    ret = request_irq(IRQ_GPIO, wmt_home_key_interrupt, IRQF_SHARED, "home-key", dev);
	if (ret) {
		printk(KERN_ERR "%s: Can't allocate irq %d ret = %d\n", __func__, IRQ_GPIO,ret);
        free_irq(kpad.irq, dev);
        goto kpad_open_out;
    }
#endif

    /*init timer*/
    init_timer(&wmt_kpad_timer_row0);
    wmt_kpad_timer_row0.function = wmt_kpad_timeout_row0;
    wmt_kpad_timer_row0.data = (unsigned long)dev;

	init_timer(&wmt_kpad_timer_row1);
    wmt_kpad_timer_row1.function = wmt_kpad_timeout_row1;
    wmt_kpad_timer_row1.data = (unsigned long)dev;

	init_timer(&wmt_kpad_timer_row2);
    wmt_kpad_timer_row2.function = wmt_kpad_timeout_row2;
    wmt_kpad_timer_row2.data = (unsigned long)dev;

	init_timer(&wmt_kpad_timer_row3);
    wmt_kpad_timer_row3.function = wmt_kpad_timeout_row3;
    wmt_kpad_timer_row3.data = (unsigned long)dev;

#ifdef USE_HOME
    init_timer(&wmt_kpad_timer_gpio10);
    wmt_kpad_timer_gpio10.function = wmt_kpad_timeout_gpio10;
    wmt_kpad_timer_gpio10.data = (unsigned long)dev;
#endif

#if 0    
	init_timer(&wmt_kpad_timer_row4);
    wmt_kpad_timer_row4.function = wmt_kpad_timeout_row4;
    wmt_kpad_timer_row4.data = (unsigned long)dev;

	init_timer(&wmt_kpad_timer_row5);
    wmt_kpad_timer_row5.function = wmt_kpad_timeout_row5;
    wmt_kpad_timer_row5.data = (unsigned long)dev;

	init_timer(&wmt_kpad_timer_row6);
    wmt_kpad_timer_row6.function = wmt_kpad_timeout_row6;
    wmt_kpad_timer_row6.data = (unsigned long)dev;

	init_timer(&wmt_kpad_timer_row7);
    wmt_kpad_timer_row7.function = wmt_kpad_timeout_row7;
    wmt_kpad_timer_row7.data = (unsigned long)dev;
#endif    
    
	/* Register an input event device. */
	dev->name = "keypad",
	dev->phys = "keypad",

	/*
	 *  Let kpad to implement key repeat.
	 */

	set_bit(EV_KEY, dev->evbit);

	for (i = 0; i < WMT_KPAD_FUNCTION_NUM; i++)
		set_bit(wmt_kpad_codes[i], dev->keybit);


	dev->keycode = wmt_kpad_codes;
	dev->keycodesize = sizeof(unsigned int);
	dev->keycodemax = WMT_KPAD_FUNCTION_NUM;

	/*
	 * For better view of /proc/bus/input/devices
	 */
	dev->id.bustype = 0;
	dev->id.vendor  = 0;
	dev->id.product = 0;
	dev->id.version = 0;

	input_register_device(dev);
	
#ifdef USE_HOME	
	wmt_gpio_home_key_hw_init();
#endif

	wmt_kpad_hw_init();
	DPRINTK("End2\n");
kpad_open_out:
	DPRINTK("End3\n");
	return ret;
}

static void kpad_close(struct input_dev *dev)
{
	DPRINTK("Start\n");
	if (--kpad.ref) {
		DPRINTK("End1\n");
		return;
	}

	/*
	 * Free interrupt resource
	 */
	kpad.regs->kpmcr = 0;
	free_irq(kpad.irq, dev);
#ifdef USE_HOME
	free_irq(IRQ_GPIO, NULL);
#endif
    /*Disable clock*/
    auto_pll_divisor(DEV_KEYPAD, CLK_DISABLE, 0, 0);


	/*
	 * Unregister input device driver
	 */
	input_unregister_device(dev);
	DPRINTK("End2\n");
}

static int wmt_kpad_probe(struct platform_device *pdev)
{
	unsigned long base;
	int ret = 0;
	DPRINTK("Start\n");
	kpad_dev = input_allocate_device();
	if (kpad_dev == NULL) {
		DPRINTK("End 1\n");
		return -1;
    }
	/*
	 * Simply check resources parameters.
	 */
	if (pdev->num_resources < 2 || pdev->num_resources > 3) {
		ret = -ENODEV;
		goto kpad_probe_out;
	}

	base = pdev->resource[0].start;

	kpad.irq = pdev->resource[1].start;
#if 0    
	kpad.res = request_mem_region(base, KPAD_IO_SIZE, "keypad");
    
	if (!kpad.res || !kpad.irq) {
		ret = -ENODEV;
		goto kpad_probe_out;
	}
#endif    
	//kpad.regs = ioremap(base, KPAD_IO_SIZE);
    kpad.regs = (struct kpad_regs_s *)KPAD_BASE_ADDR;
    
	if (!kpad.regs) {
		ret = -ENOMEM;
		goto kpad_probe_out;
	}

	kpad_dev->open = kpad_open,
	kpad_dev->close = kpad_close,

	kpad_open(kpad_dev);
	DPRINTK("End2\n");
kpad_probe_out:

#ifndef CONFIG_SKIP_DRIVER_MSG
	printk(KERN_INFO "WMT keypad driver initialized: %s\n",
		  (ret == 0) ? "ok" : "failed");
#endif
	DPRINTK("End3\n");
	return ret;
}

static int wmt_kpad_remove(struct platform_device *pdev)
{
	DPRINTK("Start\n");
	kpad_close(kpad_dev);

	/*
	 * Free allocated resource
	 */
	//kfree(kpad.res);
	//kpad.res = NULL;

	//if (kpad.regs) {
	//	iounmap(kpad.regs);
	//	kpad.regs = NULL;
	//}

	kpad.ref = 0;
	kpad.irq = 0;
	
	memset(&kpad.ints, 0, KPAD_INTS_SIZE);
    DPRINTK("End\n");
	return 0;
}

static int wmt_kpad_suspend(struct platform_device *pdev, pm_message_t state)
{
	DPRINTK("Start\n");
    
    switch (state.event) {
	case PM_EVENT_SUSPEND:             
		/*Disable clock*/
        auto_pll_divisor(DEV_KEYPAD, CLK_DISABLE, 0, 0);
#ifdef USE_HOME
		wmt_control_home_irq(0);		
        //disable_irq_nosync(IRQ_GPIO);
#endif
		break;
	case PM_EVENT_FREEZE:
	case PM_EVENT_PRETHAW:
        
	default:
		break;
	}

	DPRINTK("End2\n");
	return 0;
}

static int wmt_kpad_resume(struct platform_device *pdev)
{  
	DPRINTK("Start\n");
	wmt_kpad_hw_init();
#ifdef USE_HOME
	wmt_gpio_home_key_hw_init();
#endif
	DPRINTK("End\n");
	return 0;
}

static struct platform_driver wmt_kpad_driver = {
	.driver.name = "wmt-kpad",
	.probe = &wmt_kpad_probe,
	.remove = &wmt_kpad_remove,
	.suspend = &wmt_kpad_suspend,
	.resume	= &wmt_kpad_resume
};

static struct resource wmt_kpad_resources[] = {
	[0] = {
		.start  = KPAD_BASE_ADDR,
		.end    = (KPAD_BASE_ADDR + 0xFFFF),
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.start  = IRQ_KPAD,
		.end    = IRQ_KPAD,
		.flags  = IORESOURCE_IRQ,
	},
};

static struct platform_device wmt_kpad_device = {
	.name			= "wmt-kpad",
	.id				= 0,
	.num_resources  = ARRAY_SIZE(wmt_kpad_resources),
	.resource		= wmt_kpad_resources,
};

extern int wmt_getsyspara(char *varname, unsigned char *varval, int *varlen);

static int __init kpad_init(void)
{
	int ret;
	int retval;
	unsigned char buf[80];
	int varlen = 80;
	char *varname = "wmt.io.keypad";
	int timeout_value;
	
	DPRINTK(KERN_ALERT "Start\n");
	/*read back&menu button integration enable >1 enable & the value is the timeout value*/
	/*read keypad enable*/
	retval = wmt_getsyspara(varname, buf, &varlen);
	if (retval == 0) {
		sscanf(buf,"%d:%d", &enable_keypad, &timeout_value);
		printk(KERN_ALERT "wmt.io.keypad = %d:%d\n",enable_keypad,timeout_value);
		if (enable_keypad <= 0)
			return -ENODEV;
		if (timeout_value >= 0) {
			back_menu_timeout = timeout_value;
		}
		printk(KERN_ALERT "back_menu_timeout = %d \n",back_menu_timeout);
	} else {
        printk("##Warning: \"wmt.io.keypad\" not find\n");
        printk(KERN_ALERT "Default wmt.io.keypad = %d:%d\n",enable_keypad,back_menu_timeout);
    }
	
#ifdef CONFIG_CPU_FREQ
	ret = cpufreq_register_notifier(&kpad_clock_nblock, \
		CPUFREQ_TRANSITION_NOTIFIER);

	if (ret) {
		printk(KERN_ERR "Unable to register CPU frequency " \
			"change notifier (%d)\n", ret);
	}
#endif
	ret = platform_device_register(&wmt_kpad_device);
	if (ret != 0) {
		DPRINTK("End1 ret = %x\n",ret);
		return -ENODEV;
	}

	ret = platform_driver_register(&wmt_kpad_driver);
	DPRINTK("End2 ret = %x\n",ret);
	return ret;
}

static void __exit kpad_exit(void)
{
	DPRINTK("Start\n");
	platform_driver_unregister(&wmt_kpad_driver);
	platform_device_unregister(&wmt_kpad_device);
	DPRINTK("End\n");
}

module_init(kpad_init);
module_exit(kpad_exit);

MODULE_AUTHOR("WonderMedia Technologies, Inc.");
MODULE_DESCRIPTION("WMT [generic keypad] driver");
MODULE_LICENSE("GPL");
