/**************************************************************
Copyright (c) 2008 WonderMedia Technologies, Inc.

Module Name:
    $Workfile: wmt-lcd-backlight.c $
Abstract:
    This program is the WMT LCD backlight driver for Android 1.6 system.
    Andriod1.6 API adjusts the LCD backlight by writing follwing file:
        /sys/class/leds/lcd-backlight/brightness
    Use WMT PWM to control the LCD backlight
Revision History:
    Jan.08.2010 First Created by HowayHuo
	
**************************************************************/
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/leds.h>
#include <linux/io.h>

#include "../char/wmt-pwm.h"

static int pwm_no;  // 0:PWM0, 1:PWM1
static unsigned int pwm_period;
struct wmt_pwm_reg_t {
	unsigned int scalar;
	unsigned int period;
	unsigned int duty;
	unsigned int config;
};

static struct wmt_pwm_reg_t g_pwm_setting;

static int __init pwm_no_arg
(
	char *str			/*!<; // argument string */
)
{
	sscanf(str,"%d",&pwm_no);
	printk("set pwm no = %d\n",pwm_no);

  	return 1;
} 

__setup("pwmno=", pwm_no_arg); //从UBOOT参数bootargs中读取pwmno


/*
 * For simplicity, we use "brightness" as if it were a linear function
 * of PWM duty cycle.  However, a logarithmic function of duty cycle is
 * probably a better match for perceived brightness: two is half as bright
 * as four, four is half as bright as eight, etc
 */
static void lcd_brightness(struct led_classdev *cdev, enum led_brightness b)
{
    unsigned int duty,  pwm_enable;

	/*printk(KERN_ALERT "[%s]pwm_no = %d pwm_period = %d b= %d\n", __func__, pwm_no,pwm_period, b);*/

	pwm_enable = (REG32_VAL(PWM_CTRL_REG_ADDR + (0x10 * pwm_no)) & 0xFFF);
    duty = (b *  pwm_period) / 255;
	
    if(duty) {
		if(--duty > 0xFFF)
	    	duty = 0xFFF;
    }

    pwm_set_duty(pwm_no, duty);
	pwm_enable = pwm_get_enable(pwm_no);
	
    if(duty) {
		if(!pwm_enable) {
			/*printk(KERN_ALERT "[%s]lcd turn on -->\n",__func__);*/
            pwm_set_enable(pwm_no,1);
        }
    } else {
    	if(pwm_enable) {
        	/*printk(KERN_ALERT "[%s]lcd turn off  <--\n",__func__);*/
			pwm_set_enable(pwm_no,0);
    	}
    }
    return;
}

/*
 * NOTE:  we reuse the platform_data structure of GPIO leds,
 * but repurpose its "gpio" number as a PWM channel number.
 */
static int lcd_backlight_probe(struct platform_device *pdev)
{
	const struct gpio_led_platform_data	*pdata;
	struct led_classdev				*leds;
	int					i;
	int					status;
	unsigned int brightness;

	pdata = pdev->dev.platform_data;
	if (!pdata || pdata->num_leds < 1)
		return -ENODEV;

	leds = kcalloc(pdata->num_leds, sizeof(*leds), GFP_KERNEL);
	if (!leds)
		return -ENOMEM;

	REG32_VAL(PMCEL_ADDR) |= BIT10; //Enable PWM Clock
	pwm_no = 0;
	if((pwm_no < 0) || (pwm_no > 1)) //WM8505 only has 2 PWM
	    pwm_no = 0; //default set to 0 when pwm_no is invalid

	pwm_period  = pwm_get_period(pwm_no) + 1;

	/*calculate the default brightness*/
	brightness = (pwm_get_duty(pwm_no) * 255) / pwm_period;
	
	for (i = 0; i < pdata->num_leds; i++) {
		struct led_classdev		*led = leds + i;
		const struct gpio_led	*dat = pdata->leds + i;

		led->name = dat->name;
		led->brightness = brightness;
		led->brightness_set = lcd_brightness;
		led->default_trigger = dat->default_trigger;

		/* Hand it over to the LED framework */
		status = led_classdev_register(&pdev->dev, led);
		if (status < 0) {
			goto err;
		}
	}

	platform_set_drvdata(pdev, leds);

	return 0;

err:
	if (i > 0) {
		for (i = i - 1; i >= 0; i--) {
			led_classdev_unregister(leds + i);
		}
	}
	kfree(leds);

	return status;
}

static int lcd_backlight_remove(struct platform_device *pdev)
{
	const struct gpio_led_platform_data	*pdata;
	struct led_classdev				*leds;
	unsigned				i;

	pdata = pdev->dev.platform_data;
	leds = platform_get_drvdata(pdev);

	for (i = 0; i < pdata->num_leds; i++) {
		struct led_classdev		*led = leds + i;

		led_classdev_unregister(led);
	}

	kfree(leds);
	platform_set_drvdata(pdev, NULL);
	return 0;
}

static int lcd_backlight_suspend
(
	struct platform_device *pdev,     /*!<; // a pointer point to struct device */
	pm_message_t state		/*!<; // suspend state */
)
{
	unsigned int addr;

	addr = PWM_CTRL_REG_ADDR + (0x10 * pwm_no);
	g_pwm_setting.config = (REG32_VAL(addr) & 0xFFF);
	addr = PWM_PERIOD_REG_ADDR + (0x10 * pwm_no);
	g_pwm_setting.period = (REG32_VAL(addr) & 0xFFF);

	addr = PWM_SCALAR_REG_ADDR + (0x10 * pwm_no);
	g_pwm_setting.scalar = (REG32_VAL(addr) & 0xFFF);
	
	//g_pwm_setting.duty = 0; // for android , AP will set duty to 0

	return 0;
}

static int lcd_backlight_resume
(
	struct platform_device *pdev 	/*!<; // a pointer point to struct device */
)
{

	if( (REG32_VAL(PMCEL_ADDR) & BIT10) == 0 )	// check pwm power on
		printk("PWM power is off!!!!!!!!!\n");

	pwm_set_scalar(pwm_no, g_pwm_setting.scalar);
	pwm_set_period(pwm_no, g_pwm_setting.period);

	pwm_set_duty(pwm_no, 1);
	pwm_set_control(pwm_no, g_pwm_setting.config);
	//pwm_set_gpio(pwm_no, 1);

	return 0;
}


static struct gpio_led lcd_pwm[] = {
	{
		.name			= "lcd-backlight",
	}, 
};


static struct gpio_led_platform_data lcd_backlight_data = {
	.leds		= lcd_pwm,
	.num_leds	= ARRAY_SIZE(lcd_pwm),
};

static struct platform_device lcd_backlight_device = {
	.name           = "lcd-backlight",
	.id             = 0,
	.dev            = 	{	.platform_data = &lcd_backlight_data,			
	},
};

static struct platform_driver lcd_backlight_driver = {
	.driver = {
		.name =		"lcd-backlight",
		.owner =	THIS_MODULE,
	},
	/* REVISIT add suspend() and resume() methods */
	.probe	=	lcd_backlight_probe,
	.remove =	__exit_p(lcd_backlight_remove),
	.suspend        = lcd_backlight_suspend,
	.resume         = lcd_backlight_resume
};

static int __init lcd_backlight_init(void)
{
	int ret;

	ret = platform_device_register(&lcd_backlight_device);
	if(ret) {
		printk("[lcd_backlight_init]Error: Can not register LCD backlight device\n");
		return ret;
	}
	//ret = platform_driver_probe(&lcd_backlight_driver, lcd_backlight_probe);
	ret = platform_driver_register(&lcd_backlight_driver);
	if(ret) {
		printk("[lcd_backlight_init]Error: Can not register LCD backlight driver\n");
		platform_device_unregister(&lcd_backlight_device);
		return ret;
	}
	return 0; 
}
module_init(lcd_backlight_init);

static void __exit lcd_backlight_exit(void)
{
	platform_driver_unregister(&lcd_backlight_driver);
	platform_device_unregister(&lcd_backlight_device);
}
module_exit(lcd_backlight_exit);

MODULE_DESCRIPTION("Driver for LCD with PWM-controlled brightness");
MODULE_LICENSE("GPL");

