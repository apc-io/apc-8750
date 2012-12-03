/*++
	drivers/power/wmt_battery.c

	Copyright (c) 2008 WonderMedia Technologies, Inc.

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
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
//#include <linux/wm97xx.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
//#include <linux/wm97xx_batt.h>
#include <linux/io.h>
#include <mach/hardware.h>
#include <linux/wmt_battery.h>
#include <mach/wmt_spi.h>

static int batt_charge_max = 0xB78;         
static int batt_charge_min = 0x972;        
static int batt_discharge_max = 0xB29;  
static int batt_discharge_v9 = 0xB11;   
static int batt_discharge_v8 = 0xAFE;   
static int batt_discharge_v7 = 0xACA;   
static int batt_discharge_v6 = 0xAAB;   
static int batt_discharge_v5 = 0xA94;  
static int batt_discharge_v4 = 0xA80;   
static int batt_discharge_v3 = 0xA64;   
static int batt_discharge_v2 = 0xA40;   
static int batt_discharge_v1 = 0xA1B;   
static int batt_discharge_min = 0x972;  
static int batt_update_time = 5000;
static int wmt_operation = 0;
static int ADC_USED = 0;
static int batt_operation = 0;
static int ac_dcin = 1;
static int ac_dcin_old = 1;
/*static int spi_mode = 0;*/
static int adcdevicename = 0;
static int bustype = 0;
static int busportnum = 0;

/*  Bus Define */
#define spibus 0
#define i2cbus 1

/*  ADC device define */
#define vt1609 0

/*touchscreen AD7843*/
#define ad7843_start_bit BIT7
#define ad7843_a2_bit BIT6
#define ad7843_a1_bit BIT5
#define ad7843_a0_bit BIT4
#define ad7843_mode_bit BIT3
#define ad7843_ser_bit BIT2
#define ad7843_pd1_bit BIT1
#define ad7843_pd0_bit BIT0
/*cmd*/
//#define CMD_BAT_12BIT (ad7843_start_bit|ad7843_a1_bit|ad7843_ser_bit|ad7843_pd0_bit);
//#define CMD_BAT_12BIT (ad7843_start_bit|ad7843_a1_bit|ad7843_ser_bit);
#define CMD_BAT_12BIT (ad7843_start_bit|ad7843_a2_bit|ad7843_a1_bit|ad7843_ser_bit);
#define AD7843_ID 0x800
/* SPI */
struct spi_user_rec_s *spi_user_rec_b;
#define SPI_USER_ID 0
#define SPI_USER_NAME "ad7843"
#define ADC_USER_NAME "cs7146"
#define SPI_FREQ 3    /*50k*/

/* Debug macros */
#if 0
#define DPRINTK(fmt, args...) printk(KERN_ALERT "[%s]: " fmt, __func__ , ## args)
#else
#define DPRINTK(fmt, args...)
#endif

static struct timer_list polling_timer;

static DEFINE_MUTEX(bat_lock);
static struct work_struct bat_work;
static struct work_struct ac_work;
struct mutex work_lock;
struct mutex ac_work_lock;
static int bat_low = 0;
static int bat_dcin = 1;
static int bat_status = POWER_SUPPLY_STATUS_UNKNOWN;
static int bat_health = POWER_SUPPLY_HEALTH_GOOD;
static int bat_online = true;
static int bat_capacity = 30;
static int bat_low_old = 0;
static int bat_dcin_old = 1;
static int bat_status_old = POWER_SUPPLY_STATUS_UNKNOWN;
static int bat_health_old = POWER_SUPPLY_HEALTH_GOOD;
static int bat_online_old = true;
static int bat_capacity_old = 30;
static struct wmt_battery_info *pdata;
static unsigned int bat_temp_capacity = 0;
static int first_update;
static int charing_first_update = 0;
static int discharing_first_update = 0;

int polling_interval= 5000;
extern int wmt_getsyspara(char *varname, char *varval, int *varlen);
extern unsigned short wmt_read_batstatus_if(void);

struct wmt_batgpio_set{ 
     int  name;        
     int  active;   
     int  bitmap;   
     int  ctradd;   
     int  icaddr;   
     int  idaddr;   
     int  ipcaddr;  
     int  ipdaddr;  
};                      
/*
struct wmt_bitmap{ 
     int  ctradd;   
     int  icaddr;   
     int  idaddr;   
     int  ipcaddr;  
     int  ipdaddr;  
};                      
*/
/*
static struct wmt_batgpio_set src_dcin;    
static struct wmt_batgpio_set src_batstate;
static struct wmt_batgpio_set src_batlow;  
*/
static struct wmt_batgpio_set dcin;    
static struct wmt_batgpio_set batstate;
static struct wmt_batgpio_set batlow;  
/*
static struct wmt_bitmap dcin_bitmap;    
static struct wmt_bitmap batstate_bitmap;
static struct wmt_bitmap batlow_bitmap;  
*/
#if 0
static void	spi_ini(void)
{
    char buf[800];                                                                                                  
    char varname[] = "wmt.io.ts";                                                                                
    unsigned char tmp_buf[80];
    int varlen = 800;                                                                                               
    bool tsused =0;
	bool is_cs7146 =0;
	int i=0;
	int j=0;

	/*printk("Bat: wmt_spi_init  \n");*/
	if (wmt_getsyspara(varname, buf, &varlen) == 0){                                                                                                                               
        for (i = 0; i < 80; ++i) {
            if (buf[i] == ':')
                break;
            tsused = (buf[i] - '0' == 0)?0:1;
        }
		
        ++i;
		
        for (; i < 80; ++i) {
                if (buf[i] == ':')
                    break;
                tmp_buf[j] = buf[i];
                ++j;
        }
		
        if (tmp_buf[0] == 'c' && tmp_buf[1] == 's'
                && tmp_buf[2] == '7' && tmp_buf[3] == '1'
                && tmp_buf[4] == '4' && tmp_buf[5] == '6'){
            is_cs7146 = 1;
        }
		
        //sscanf(buf,"%x:%x", & tsused, & tsname);                                                                       

		if(tsused && is_cs7146){
			spi_mode = 0;
		}else{
			spi_mode = 1;
		}
    }else{
			spi_mode = 1;
	}

	/*printk("Bat: spi_mode =%d,tsused=%d, is_cs7146 = 0x%x  \n",spi_mode,tsused,is_cs7146);*/
}
#endif

static void gpio_ini(void)
{
    char buf[800];                                                                                                  
    char varname[] = "wmt.gpi.bat";                                                                                
    int varlen = 800;                                                                                               
	if (wmt_getsyspara(varname, buf, &varlen) == 0){                                                                                                                               
        sscanf(buf,"[%x:%x:%x:%x:%x:%x:%x:%x][%x:%x:%x:%x:%x:%x:%x:%x][%x:%x:%x:%x:%x:%x:%x:%x]",              
            &dcin.name,                                                                               
            &dcin.active,                                                                             
            &dcin.bitmap,                                                                             
            &dcin.ctradd,                                                                             
            &dcin.icaddr,                                                                             
            &dcin.idaddr,                                                                             
            &dcin.ipcaddr,                                                                            
            &dcin.ipdaddr,                                                                            
            &batstate.name,                                                                           
            &batstate.active,                                                                         
            &batstate.bitmap,                                                                         
            &batstate.ctradd,                                                                         
            &batstate.icaddr,                                                                         
            &batstate.idaddr,                                                                         
            &batstate.ipcaddr,                                                                        
            &batstate.ipdaddr,
            &batlow.name,                                                                             
            &batlow.active,                                                                           
            &batlow.bitmap,                                                                           
            &batlow.ctradd,                                                                           
            &batlow.icaddr,                                                                           
            &batlow.idaddr,                                                                           
            &batlow.ipcaddr,                                                                          
            &batlow.ipdaddr);                                                                       

	dcin.ctradd += WMT_MMAP_OFFSET;
	dcin.icaddr += WMT_MMAP_OFFSET;
	dcin.idaddr += WMT_MMAP_OFFSET;
	dcin.ipcaddr += WMT_MMAP_OFFSET;
	dcin.ipdaddr += WMT_MMAP_OFFSET;

	batstate.ctradd += WMT_MMAP_OFFSET;
	batstate.icaddr += WMT_MMAP_OFFSET;
	batstate.idaddr += WMT_MMAP_OFFSET;
	batstate.ipcaddr += WMT_MMAP_OFFSET;
	batstate.ipdaddr += WMT_MMAP_OFFSET;

	batlow.ctradd += WMT_MMAP_OFFSET;
	batlow.icaddr += WMT_MMAP_OFFSET;
	batlow.idaddr += WMT_MMAP_OFFSET;
	batlow.ipcaddr += WMT_MMAP_OFFSET;
	batlow.ipdaddr += WMT_MMAP_OFFSET;
/*	
    printk("Bat: dcin.active = 0x%x  \n",dcin.active);
	printk("Bat: dcin.active data= 0x%x  \n",(dcin.active & 0x01));
	printk("Bat: dcin.active pull= 0x%x  \n",((dcin.active & 0x10)>>4));
	printk("Bat: batstate.active = 0x%x  \n",batstate.active);
	printk("Bat: batstate.active data= 0x%x  \n",(batstate.active & 0x01));
	printk("Bat: batstate.active pull= 0x%x  \n",((batstate.active & 0x10)>>4));
	printk("Bat: batlow.active = 0x%x  \n",batlow.active);
	printk("Bat: batlow.active data= 0x%x  \n",(batlow.active & 0x01));
	printk("Bat: batlow.active pull= 0x%x  \n",((batlow.active & 0x10)>>4));

		if((src_dcin.bitmap > 13) || (src_batstate.bitmap > 13) || (src_batlow.bitmap > 13)){
            src_dcin.name = 0xFF;
            src_batstate.name = 0xFF;
            src_batlow.name = 0xFF;
			return;
		}
*/
	    if((dcin.name != 0) || (batstate.name != 1) || (batlow.name != 2) ){
			batt_operation = 0;
			return;
		}
/*
		dcin.bitmap = BIT0 << src_dcin.bitmap;
        batstate.bitmap = BIT0 << src_batstate.bitmap;
        batlow.bitmap = BIT0 << src_batlow.bitmap ;
        dcin.ctradd = src_dcin.ctradd & (~0x03);                                                                            
        dcin.icaddr = src_dcin.icaddr & (~0x03);                                                                           
        dcin.idaddr = src_dcin.idaddr & (~0x03);                                                                            
        dcin.ipcaddr = src_dcin.ipcaddr & (~0x03);                                                                          
        dcin.ipdaddr =  src_dcin.ipdaddr & (~0x03);                                                                           
        batstate.ctradd = src_batstate.ctradd & (~0x03);                                                                        
        batstate.icaddr = src_batstate.icaddr & (~0x03);                                                                         
        batstate.idaddr = src_batstate.idaddr & (~0x03);                                                                         
        batstate.ipcaddr = src_batstate.ipcaddr & (~0x03);                                                                        
        batstate.ipdaddr = src_batstate.ipdaddr & (~0x03);
        batlow.ctradd =src_batlow.ctradd & (~0x03);                                                                           
        batlow.icaddr = src_batlow.icaddr & (~0x03);                                                                           
        batlow.idaddr = src_batlow.idaddr & (~0x03);                                                                           
        batlow.ipcaddr =src_batlow.ipcaddr & (~0x03);                                                                          
        batlow.ipdaddr = src_batlow.ipdaddr & (~0x03);                                                                       

		dcin_bitmap.ctradd = dcin.bitmap << (8* (src_dcin.ctradd -dcin.ctradd));
		dcin_bitmap.icaddr= dcin.bitmap << (8* (src_dcin.icaddr - dcin.icaddr));
		dcin_bitmap.idaddr= dcin.bitmap << (8* (src_dcin.idaddr -dcin.idaddr));
		dcin_bitmap.ipcaddr= dcin.bitmap << (8* (src_dcin.ipcaddr -dcin.ipcaddr));
		dcin_bitmap.ipdaddr= dcin.bitmap << (8* (src_dcin.ipdaddr -dcin.ipdaddr));
		batstate_bitmap.ctradd = batstate.bitmap << (8* (src_batstate.ctradd - batstate.ctradd));
		batstate_bitmap.icaddr= batstate.bitmap << (8* (src_batstate.icaddr - batstate.icaddr));
		batstate_bitmap.idaddr= batstate.bitmap << (8* (src_batstate.idaddr -batstate.idaddr));
		batstate_bitmap.ipcaddr= batstate.bitmap << (8* (src_batstate.ipcaddr -batstate.ipcaddr));
		batstate_bitmap.ipdaddr= batstate.bitmap << (8* (src_batstate.ipdaddr - batstate.ipdaddr));
		batlow_bitmap.ctradd = batlow.bitmap << (8* (src_batlow.ctradd - batlow.ctradd));
		batlow_bitmap.icaddr= batlow.bitmap << (8* (src_batlow.icaddr - batlow.icaddr));
		batlow_bitmap.idaddr= batlow.bitmap << (8* (src_batlow.idaddr - batlow.idaddr));
		batlow_bitmap.ipcaddr= batlow.bitmap << (8* (src_batlow.ipcaddr - batlow.ipcaddr));
		batlow_bitmap.ipdaddr= batlow.bitmap << (8* (src_batlow.ipdaddr - batlow.ipdaddr));

*/
		REG32_VAL(dcin.ctradd) = REG32_VAL(dcin.ctradd) | dcin.bitmap;
        REG32_VAL(dcin.icaddr) = REG32_VAL(dcin.icaddr) & (~dcin.bitmap);
	    REG32_VAL(dcin.ipcaddr) = REG32_VAL(dcin.ipcaddr) | dcin.bitmap;
		if(dcin.active & 0x10){
	        REG32_VAL(dcin.ipdaddr) = REG32_VAL(dcin.ipdaddr) | dcin.bitmap;
		}else{
	        REG32_VAL(dcin.ipdaddr) = REG32_VAL(dcin.ipdaddr) & (~dcin.bitmap);
		}
        REG32_VAL(batstate.ctradd) = REG32_VAL(batstate.ctradd) | batstate.bitmap;
        REG32_VAL(batstate.icaddr) = REG32_VAL(batstate.icaddr) & (~batstate.bitmap);
	    REG32_VAL(batstate.ipcaddr) = REG32_VAL(batstate.ipcaddr) | batstate.bitmap;
		if(batstate.active & 0x10){
	        REG32_VAL(batstate.ipdaddr) = REG32_VAL(batstate.ipdaddr) | batstate.bitmap;
		}else{
	        REG32_VAL(batstate.ipdaddr) = REG32_VAL(batstate.ipdaddr) & (~batstate.bitmap);
		}
        REG32_VAL(batlow.ctradd) = REG32_VAL(batlow.ctradd) | batlow.bitmap;
        REG32_VAL(batlow.icaddr) = REG32_VAL(batlow.icaddr) & (~batlow.bitmap);
	    REG32_VAL(batlow.ipcaddr) = REG32_VAL(batlow.ipcaddr) | batlow.bitmap;
		if(batlow.active & 0x10){
	        REG32_VAL(batlow.ipdaddr) = REG32_VAL(batlow.ipdaddr) | batlow.bitmap;
		}else{
	        REG32_VAL(batlow.ipdaddr) = REG32_VAL(batlow.ipdaddr) & (~batlow.bitmap);
		}
#if 0
    /*  DCIN_DT initial --- WAKE_UP1 */
        /* enable WAKE_UP1*/
        REG8_VAL(GPIO_CTRL_GP2_WAKEUP_SUS_BYTE_ADDR) = REG8_VAL(GPIO_CTRL_GP2_WAKEUP_SUS_BYTE_ADDR) | 0x02;
        /* set output or input WAKE_UP1*/
        REG8_VAL(GPIO_OC_GP2_WAKEUP_SUS_BYTE_ADDR) = REG8_VAL(GPIO_OC_GP2_WAKEUP_SUS_BYTE_ADDR) & 0xFD;
        /* set enable pull high or low and set pull high WAKE_UP1*/
	    REG8_VAL(GPIO_PULL_EN_GP2_WAKEUP_SUS_BYTE_ADDR) = REG8_VAL(GPIO_PULL_EN_GP2_WAKEUP_SUS_BYTE_ADDR) | 0x02;
         /* set pull low WAKE_UP1*/
	    REG8_VAL(GPIO_PULL_CTRL_GP2_WAKEUP_SUS_BYTE_ADDR) = REG8_VAL(GPIO_PULL_CTRL_GP2_WAKEUP_SUS_BYTE_ADDR) & 0xFD;
   
    /*  Charger_STAT initial --- GPIO_05 */
	    /* enable GPIO_05*/
	    REG8_VAL(GPIO_CTRL_GP0_BYTE_ADDR) = REG8_VAL(GPIO_CTRL_GP0_BYTE_ADDR) | 0x02;
        /* set output or input GPIO_05*/
	    REG8_VAL(GPIO_OC_GP0_BYTE_ADDR) = REG8_VAL(GPIO_OC_GP0_BYTE_ADDR) & 0xFD;
        /* set enable pull high or low GPIO_05*/
	    REG8_VAL(GPIO_PULL_EN_GP0_BYTE_ADDR) = REG8_VAL(GPIO_PULL_EN_GP0_BYTE_ADDR) | 0x02;
        /* set pull low GPIO_05*/
	    REG8_VAL(GPIO_PULL_CTRL_GP0_BYTE_ADDR) = REG8_VAL(GPIO_PULL_CTRL_GP0_BYTE_ADDR) & 0xFD;

    /*  BAT_LOW initial --- WAKE_UP0 */
	    /* enable WAKE_UP0 and set GPIO mode */
	    REG8_VAL(GPIO_CTRL_GP2_WAKEUP_SUS_BYTE_ADDR) = REG8_VAL(GPIO_CTRL_GP2_WAKEUP_SUS_BYTE_ADDR) | 0x01;
        /* set output or input WAKE_UP0*/
	    REG8_VAL(GPIO_OC_GP2_WAKEUP_SUS_BYTE_ADDR) = REG8_VAL(GPIO_OC_GP2_WAKEUP_SUS_BYTE_ADDR) & 0xFE;
        /* set enable pull high or low WAKE_UP0*/
	    REG8_VAL(GPIO_PULL_EN_GP2_WAKEUP_SUS_BYTE_ADDR) = REG8_VAL(GPIO_PULL_EN_GP2_WAKEUP_SUS_BYTE_ADDR) | 0x01;
        /* set pull high WAKE_UP0*/
	    REG8_VAL(GPIO_PULL_CTRL_GP2_WAKEUP_SUS_BYTE_ADDR) = REG8_VAL(GPIO_PULL_CTRL_GP2_WAKEUP_SUS_BYTE_ADDR) | 0x01;

    /*  BAT_DT initial --- not use */
	/* enable WAKE_UP1*/
        /* set output or input WAKE_UP1*/
#endif		
    }else{
			batt_operation = 0;
	}
}

static int wmt_batt_init(void)
{
	char buf[128];
	char varname[] = "wmt.io.bat";
	int varlen = 128;

	printk("Bat: wmt_batt_init  \n");

	if (wmt_getsyspara(varname, buf, &varlen) == 0){
		sscanf(buf,"%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x",
			    &wmt_operation,
			    &batt_update_time,
			    &batt_charge_max,
			    &batt_charge_min,
			    &batt_discharge_max,
			    &batt_discharge_v9,
			    &batt_discharge_v8,
			    &batt_discharge_v7,
   			    &batt_discharge_v6,
			    &batt_discharge_v5,
			    &batt_discharge_v4,
			    &batt_discharge_v3,
			    &batt_discharge_v2,
			    &batt_discharge_v1,
			    &batt_discharge_min);
		switch( wmt_operation& 0x000F){
			case 0: /* for RDK */
				batt_operation = 0;
				break;
			case 1: /* for droid book */
				batt_operation = 1;
				ADC_USED= 0;
				break;
			case 2: /* for MID */
				batt_operation = 1;
				ADC_USED= 1;
				break;
			default:
				batt_operation = 1;
				ADC_USED= 0;
				break;
		}

		switch( wmt_operation& 0x00F0){
			case 0: 
				adcdevicename = vt1609;
				break;
			default:
				adcdevicename = vt1609;
				break;
		}

		switch( wmt_operation& 0x0F00){
			case 0: /* for RDK */
				bustype = spibus;
				break;
			case 1: /* for droid book */
				bustype = i2cbus;
				break;
			default:
				bustype = spibus;
				break;
		}

		busportnum = wmt_operation& 0xF000;
		polling_interval = batt_update_time;
	}else{
		batt_charge_max = 0xB78;         
		batt_charge_min = 0x972;       
		batt_discharge_max = 0xB29; 
		batt_discharge_v9 = 0xB11;   
		batt_discharge_v8 = 0xAFE;   
		batt_discharge_v7 = 0xACA;   
		batt_discharge_v6 = 0xAAB;   
		batt_discharge_v5 = 0xA94;  
		batt_discharge_v4 = 0xA80;  
		batt_discharge_v3 = 0xA64;  
		batt_discharge_v2 = 0xA40;  
		batt_discharge_v1 = 0xA1B;  
		batt_discharge_min = 0x972;   
		polling_interval = 5000;
		ADC_USED = 0;
		batt_operation = 1;
	}
/*
	    printk(("Bat: wmt_operation = 0x%x \n"),wmt_operation);
	    printk(("Bat: batt_update_time = 0x%x \n"),batt_update_time);
	    printk(("Bat: batt_charge_max = 0x%x \n"),batt_charge_max);
	    printk(("Bat: batt_charge_min = 0x%x \n"),batt_charge_min);
	    printk(("Bat: batt_discharge_max = 0x%x \n"),batt_discharge_max);
	    printk(("Bat: batt_discharge_v9 = 0x%x \n"),batt_discharge_v9);
	    printk(("Bat: batt_discharge_v8 = 0x%x \n"),batt_discharge_v8);
	    printk(("Bat: batt_discharge_v7 = 0x%x \n"),batt_discharge_v7);
	    printk(("Bat: batt_discharge_v6 = 0x%x \n"),batt_discharge_v6);
	    printk(("Bat: batt_discharge_v5 = 0x%x \n"),batt_discharge_v5);
	    printk(("Bat: batt_discharge_v4 = 0x%x \n"),batt_discharge_v4);
	    printk(("Bat: batt_discharge_v3 = 0x%x \n"),batt_discharge_v3);
	    printk(("Bat: batt_discharge_v2 = 0x%x \n"),batt_discharge_v2);
	    printk(("Bat: batt_discharge_v1 = 0x%x \n"),batt_discharge_v1);
	    printk(("Bat: batt_discharge_min = 0x%x \n"),batt_discharge_min);
*/
	return 0;
}

static int vt1609_init(void){
    bool status= 0;

	return status;
}
static int adc_init(void){
    bool status = 0;
	switch(adcdevicename){
        case vt1609:
            status = vt1609_init();			
			break;
		default:
			break;
	}
	return status;
}

static unsigned long wmt_read_bat(struct power_supply *bat_ps)
{
	/*printk("Bat: read sbat...\n");*/
	return 4000;
	/*
	return wmt_read_aux_adc(bat_ps->dev->parent->driver_data,
					pdata->batt_aux) * pdata->batt_mult /
					pdata->batt_div;
	*/				
}

static unsigned long wmt_read_temp(struct power_supply *bat_ps)
{
	/*printk("Bat: read temp...\n");*/
	return 50;
}

//static unsigned long wmt_read_batlow_DT(struct power_supply *bat_ps)
static bool wmt_read_batlow_DT(struct power_supply *bat_ps)
{

	bool status;
	/*printk("Bat: read battlow...\n");*/
	/* WAKE_UP0 */
	//status = (REG8_VAL(GPIO_ID_GP2_WAKEUP_SUS_BYTE_ADDR) & 0x01);
	status = (REG32_VAL(batlow.idaddr) & batlow.bitmap);
	if(!(batlow.active & 0x01)){
        status = !status;
	}
	return !status;
}
//static unsigned long wmt_read_dcin_DT(struct power_supply *bat_ps)
static bool wmt_read_dcin_DT(struct power_supply *bat_ps)
{

	bool status;

	/*printk("Bat: read battlow...\n");*/
	/* WAKE_UP1 */
	//status = (REG8_VAL(GPIO_ID_GP2_WAKEUP_SUS_BYTE_ADDR) & 0x02)>>1;
	status = (REG32_VAL(dcin.idaddr) & dcin.bitmap);
	if(!(dcin.active & 0x01)){
        status = !status;
	}
	return status;
}

static unsigned long wmt_read_status(struct power_supply *bat_ps)
{
	int status;
	int chargering;

	/*printk("Bat: read status...\n");*/
	/* GPIO5 AC*/
	//chargering = (REG8_VAL(GPIO_ID_GP0_BYTE_ADDR) & 0x20)>>5;
	chargering = (REG32_VAL(batstate.idaddr)& batstate.bitmap);
	if(!(batstate.active & 0x01)){
        chargering = !chargering;
	}
	
	if(chargering > 0){
		status = POWER_SUPPLY_STATUS_CHARGING;
		/*printk("Bat: Charging...\n");*/
	}else{
		status = POWER_SUPPLY_STATUS_DISCHARGING;
	    /*printk("Bat: Discharging...\n");*/
	}
	
	//if(bat_capacity >95)
	//	status = POWER_SUPPLY_STATUS_FULL;
	return status;
}

static unsigned long wmt_read_health(struct power_supply *bat_ps)
{
	return POWER_SUPPLY_HEALTH_GOOD;
}

static unsigned long wmt_read_online(struct power_supply *bat_ps)
{

	return true;
}

#ifdef CONFIG_VT1603_BATTERY_ENABLE
extern unsigned int vt1603_get_bat_info(void);
#endif
static unsigned short hx_batt_read(void)
{
	unsigned char wbuf[3];
	unsigned char rbuf[3];
	unsigned short spi_buf = 0; 

	//spi_buf = vt1603_get_bat_info();
    if(ADC_USED){	
        switch(bustype){
            case spibus:
				if(adcdevicename == vt1609){
                    //spi_buf = vt1603_adc_data();
#ifdef CONFIG_VT1603_BATTERY_ENABLE
				    spi_buf = vt1603_get_bat_info();
#endif
				}else{
					wbuf[0] = CMD_BAT_12BIT;
					wbuf[1] = 0;
					wbuf[2] = 0;
					rbuf[0] = 0;
					rbuf[1] = 0;
					rbuf[2] = 0;
					//spi_write_and_read_data(spi_user_rec_b, wbuf, rbuf, 3);
					spi_buf = (rbuf[1] << 5) | (rbuf[2] >> 3);
				}
              break;
			case i2cbus:
              break;
            default:
				//spi_buf = wmt_read_batstatus_if();
              break;
		}
	}
#if 0	
	 //if(ADC_USED && spi_mode){	
	 if(ADC_USED && (bustype== spibus)){	
		wbuf[0] = CMD_BAT_12BIT;
		wbuf[1] = 0;
		wbuf[2] = 0;
		rbuf[0] = 0;
		rbuf[1] = 0;
		rbuf[2] = 0;
		spi_write_and_read_data(spi_user_rec_b, wbuf, rbuf, 3);
		spi_buf = (rbuf[1] << 5) | (rbuf[2] >> 3);
	}else{
		spi_buf = wmt_read_batstatus_if();
	}
#endif

    //printk("spi_buf = 0x%x \n ",spi_buf);
	return spi_buf;
}

static unsigned long wmt_read_capacity(struct power_supply *bat_ps)
{
	unsigned int capacity=0;
	unsigned short ADC_val = 0;
	
	/*printk("Bat: read capacity...\n");*/
	ADC_val = hx_batt_read();
	/*printk("ADC value = 0x%x  \n",ADC_val);*/
	if(bat_status== POWER_SUPPLY_STATUS_DISCHARGING){
        if(ADC_val > batt_discharge_max){
			capacity = 100;
		}
        else if((ADC_val <= batt_discharge_max)&&(ADC_val > batt_discharge_v9)){
			capacity = ((ADC_val - batt_discharge_v9) * 10 / (batt_discharge_max - batt_discharge_v9))+90;
        }
        else if((ADC_val <= batt_discharge_v9)&&(ADC_val > batt_discharge_v8)){
			capacity = ((ADC_val - batt_discharge_v8) * 10 / (batt_discharge_v9 - batt_discharge_v8))+80;
        }
        else if((ADC_val <= batt_discharge_v8)&&(ADC_val > batt_discharge_v7)){
			capacity = ((ADC_val - batt_discharge_v7) * 10 / (batt_discharge_v8 - batt_discharge_v7))+70;
		}
		else if((ADC_val <= batt_discharge_v7)&&(ADC_val > batt_discharge_v6)){
			capacity = ((ADC_val - batt_discharge_v6) * 10 / (batt_discharge_v7 - batt_discharge_v6))+60;
        }
        else if((ADC_val <= batt_discharge_v6)&&(ADC_val > batt_discharge_v5)){
			capacity = ((ADC_val - batt_discharge_v5) * 10 / (batt_discharge_v6 - batt_discharge_v5))+50;
        }
        else if((ADC_val <= batt_discharge_v5)&&(ADC_val > batt_discharge_v4)){
			capacity = ((ADC_val - batt_discharge_v4) * 10 / (batt_discharge_v5 - batt_discharge_v4))+40;
        }
        else if((ADC_val <= batt_discharge_v4)&&(ADC_val > batt_discharge_v3)){
			capacity = ((ADC_val - batt_discharge_v3) * 10 / (batt_discharge_v4 - batt_discharge_v3))+30;
        }
        else if((ADC_val <= batt_discharge_v3)&&(ADC_val > batt_discharge_v2)){
			capacity = ((ADC_val - batt_discharge_v2) * 10 / (batt_discharge_v3 - batt_discharge_v2))+20;
        }
        else if((ADC_val <= batt_discharge_v2)&&(ADC_val > batt_discharge_v1)){
			capacity = ((ADC_val - batt_discharge_v1) * 10 / (batt_discharge_v2 - batt_discharge_v1))+10;
        }
        else if((ADC_val <= batt_discharge_v1)&&(ADC_val > batt_discharge_min)){
			capacity = ((ADC_val - batt_discharge_min) * 10 / (batt_discharge_v1 - batt_discharge_min));
        }
        else {
			capacity = 0;
        }
		/* 
		capacity = (ADC_val - batt_discharge_min) * 100 / (batt_discharge_max - batt_discharge_min);
         */
		/*printk("DISCHARGING Capacity = %d \n",capacity);*/
	}else{
		capacity = (ADC_val - batt_charge_min) * 100 / (batt_charge_max - batt_charge_min);
		/*printk("CHARGING Capacity = %d \n",capacity);*/
	}
	return capacity;
}


static int wmt_bat_get_property(struct power_supply *bat_ps,
			    enum power_supply_property psp,
			    union power_supply_propval *val)
{

	/*printk("Bat: get property...\n");*/
	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
	    /*printk("Bat:POWER_SUPPLY_PROP_STATUS...\n");*/
		val->intval = bat_status;
		break;
	case POWER_SUPPLY_PROP_HEALTH:
	    /*printk("Bat:POWER_SUPPLY_PROP_HEALTH...\n");*/
		val->intval = bat_health;
		break;
	case POWER_SUPPLY_PROP_ONLINE:
	    /*printk("Bat:POWER_SUPPLY_PROP_ONLINE...\n");*/
		val->intval = bat_online;
		break;		
	case POWER_SUPPLY_PROP_TECHNOLOGY:
	    /*printk("Bat:POWER_SUPPLY_PROP_TECHNOLOGY...\n");*/
		val->intval = pdata->batt_tech;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
	    /*printk("Bat:POWER_SUPPLY_PROP_VOLTAGE_NOW...\n");*/
		if (pdata->batt_aux >= 0)
			val->intval = wmt_read_bat(bat_ps);
		else
			return -EINVAL;
		break;
	case POWER_SUPPLY_PROP_TEMP:
	    /*printk("Bat:POWER_SUPPLY_PROP_TEMP...\n");*/
		if (pdata->temp_aux >= 0)
			val->intval = wmt_read_temp(bat_ps);
		else
			return -EINVAL;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MAX:
	    /*printk("Bat:POWER_SUPPLY_PROP_VOLTAGE_MAX...\n");*/
		if (pdata->max_voltage >= 0)
			val->intval = pdata->max_voltage;
		else
			return -EINVAL;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MIN:
	    /*printk("Bat:POWER_SUPPLY_PROP_VOLTAGE_MIN...\n");*/
		if (pdata->min_voltage >= 0)
			val->intval = pdata->min_voltage;
		else
			return -EINVAL;
		break;
	case POWER_SUPPLY_PROP_PRESENT:
	    /*printk("Bat:POWER_SUPPLY_PROP_PRESENT...\n");*/
		val->intval = 1;
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
	    /*printk("Bat:POWER_SUPPLY_PROP_CAPACITY...\n");*/
		val->intval = bat_capacity;
		break;		
	case POWER_SUPPLY_PROP_VOLTAGE_AVG:
		    /*printk("Bat:POWER_SUPPLY_PROP_VOLTAGE_AVG...\n");*/
	   val->intval = 3;
	   break;
	case POWER_SUPPLY_PROP_CURRENT_AVG:
		    /*printk("Bat:POWER_SUPPLY_PROP_CURRENT_AVG...\n");*/
	   val->intval = 3;
	   break;
		
	default:
			/*printk("%s %d\n",__FUNCTION__,__LINE__);*/

		return -EINVAL;
	}
			/*printk("%s %d psp %d\n",__FUNCTION__,val->intval,psp);*/
	return 0;
}

static void wmt_bat_external_power_changed(struct power_supply *bat_ps)
{
	/*printk("wmt_bat_external_power_changed======================\n");*/

	schedule_work(&bat_work);
}

static void wmt_bat_update(struct power_supply *bat_ps)
{
	unsigned int current_percent = 0;
	static unsigned int capacity_first_update;
	mutex_lock(&work_lock);
    /* printk("wmt_bat_update======================\n");*/
	if(batt_operation){
		bat_health = wmt_read_health(bat_ps);
		bat_online = wmt_read_online(bat_ps);
		bat_low = wmt_read_batlow_DT(bat_ps);
	    bat_dcin= wmt_read_dcin_DT(bat_ps);
		bat_status = wmt_read_status(bat_ps);
		/*printk("\Bat: bat_low = %d \n",bat_low);*/
		/*printk("\Bat: bat_dcin = %d \n",bat_dcin);*/

		if((bat_low) && (!bat_dcin)){
	         /* If battery low signal occur. */ 
			 /*Setting battery capacity is empty. */ 
			 /* System will force shut down. */
			printk("Battery Low. \n");
			bat_capacity = 0;
		}else{		
	        if (!ADC_USED){
			/*printk("Bat: Droid book. \n");*/
				/*bat_status = wmt_read_status(bat_ps);*/
				if(bat_dcin){  /* Charging */
				    /*printk("\Bat: bat_status = %d\n",bat_status);*/
					if(bat_status== POWER_SUPPLY_STATUS_DISCHARGING){
						bat_capacity = 100;
					}else{
						bat_capacity = 50;
					}         
				}else{ /* Discharging */
					bat_capacity = 100;
				}
			}else{
			     /*printk("Bat: MID. \n");*/
				/*bat_status = wmt_read_status(bat_ps);*/
				if(bat_dcin){   /* Charging */
				    /*printk("\Bat: bat_status = %d\n",bat_status);*/
					if(bat_status== POWER_SUPPLY_STATUS_DISCHARGING){
						bat_capacity = 100;
						bat_status = POWER_SUPPLY_STATUS_FULL;
					}else{
			            current_percent = wmt_read_capacity(bat_ps);
                       /*  For AC plug from out to in.  */
						if (!charing_first_update) {
							if(bat_temp_capacity > current_percent)
								bat_temp_capacity = current_percent;
							else{
								if(!capacity_first_update){
									mod_timer(&polling_timer,
										  jiffies + msecs_to_jiffies(polling_interval));
								    ++capacity_first_update;
								}else{
									++charing_first_update;
								    capacity_first_update = 0;
								}
							}
						    discharing_first_update = 0;
						}


						if (bat_temp_capacity < current_percent){
		                    bat_temp_capacity = current_percent;
						}
				
						
	                    bat_temp_capacity = current_percent;
		                if((bat_temp_capacity < 0) || (bat_temp_capacity > 100)){
			                if(bat_temp_capacity < 0){
								bat_capacity = 0;
							}else{
								bat_capacity = 100;
							}
		                }else{
							bat_capacity = bat_temp_capacity;
						}
					} 
				}else{  /* Discharging */
			        current_percent = wmt_read_capacity(bat_ps);

					if (!discharing_first_update) {
						if(bat_temp_capacity <= current_percent)
							bat_temp_capacity = current_percent;
						else{
							if(!capacity_first_update){
								mod_timer(&polling_timer,
									  jiffies + msecs_to_jiffies(polling_interval));
							    ++capacity_first_update;
							}else{
								++discharing_first_update;
							    capacity_first_update = 0;
							}
						}
						charing_first_update = 0;
					}

					if (bat_temp_capacity > current_percent){
		                bat_temp_capacity = current_percent;
					}
				

			        if((bat_temp_capacity < 0) || (bat_temp_capacity > 100)){
			            if(bat_temp_capacity < 0){
							bat_capacity = 0;
						}else{
							bat_capacity = 100;
						}
			        }else{
						bat_capacity = bat_temp_capacity;
					}
				}

			}

	    }
	}else{
		/*printk("Bat: RDK \n");*/
		bat_health = wmt_read_health(bat_ps);
		bat_online = wmt_read_online(bat_ps);
		bat_status= POWER_SUPPLY_STATUS_FULL;
		bat_capacity = 100;
	}
	/*
	printk("Bat: current_percent = %d \n",current_percent);
    printk("Bat: bat_temp_capacity = %d \n",bat_temp_capacity);
    printk("Bat: capacity %d \n",bat_capacity);
    */
    if((bat_low) ||
       (bat_dcin != bat_dcin_old) ||
       (bat_health != bat_health_old) ||
       (bat_online != bat_online_old) ||
       (bat_status != bat_status_old) ||
       (bat_capacity != bat_capacity_old) ||
       (bat_capacity <= 0)){
		power_supply_changed(bat_ps);
    }
	/*
    printk("Bat: ****************************************************************************  \n");
	printk("Bat: bat_low = %d, bat_low_old = %d  \n",bat_low,bat_low_old);
	printk("Bat: bat_dcin = %d, bat_dcin_old = %d  \n",bat_dcin,bat_dcin_old);
	//printk("Bat: bat_health = %d, bat_health_old = %d  \n",bat_health,bat_health_old);
	//printk("Bat: bat_online = %d, bat_online_old = %d  \n",bat_online,bat_online_old);
	printk("Bat: bat_status = %d, bat_status_old = %d  \n",bat_status,bat_status_old);
	printk("Bat: bat_capacity = %d, bat_capacity_old = %d  \n",bat_capacity,bat_capacity_old);
	printk("Bat: first_update = %x  \n",first_update);
	printk("Bat: ****************************************************************************  \n");
   */
	bat_low_old=bat_low;
	bat_dcin_old = bat_dcin;
    bat_health_old =bat_health;
    bat_online_old= bat_online;
    bat_status_old=bat_status;
    bat_capacity_old=bat_capacity;
	mutex_unlock(&work_lock);
	/*printk("Bat: update ^^^\n");*/
}


static enum power_supply_property wmt_bat_props[] = {
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_TECHNOLOGY,
#if 0	
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_VOLTAGE_MAX,
	POWER_SUPPLY_PROP_VOLTAGE_MIN,
#endif
    POWER_SUPPLY_PROP_ONLINE,
    POWER_SUPPLY_PROP_CAPACITY,
	
};

static struct power_supply bat_ps = {
	.name					= "wmt-battery",
	.type					= POWER_SUPPLY_TYPE_BATTERY,
	.get_property			= wmt_bat_get_property,
	.external_power_changed = wmt_bat_external_power_changed,
	.properties 			= wmt_bat_props,
	.num_properties 		= ARRAY_SIZE(wmt_bat_props),
	.use_for_apm			= 1,
};

static void wmt_battery_work(struct work_struct *work)
{
    /*printk("wmt_bat_work\n");*/

	wmt_bat_update(&bat_ps);
}


static void polling_timer_func(unsigned long unused)
{
	/*printk("Bat: polling...\n");*/
	schedule_work(&bat_work);
	schedule_work(&ac_work);
	mod_timer(&polling_timer,
		  jiffies + msecs_to_jiffies(polling_interval));
		 
	/*printk("Bat: polling ^^^\n");*/
}

#ifdef CONFIG_PM
static int wmt_battery_suspend(struct platform_device *dev, pm_message_t state)
{
	/*flush_scheduled_work();*/
	del_timer(&polling_timer);
	
    if(ADC_USED){	
        switch(bustype){
            case spibus:
				//unregister_user(spi_user_rec_b, SPI_USER_ID);
              break;
			case i2cbus:
              break;
            default:
              break;
		}
	}

	return 0;
}

static int wmt_battery_resume(struct platform_device *dev)
{
	wmt_batt_init();
	
	if(batt_operation){
		gpio_ini();
	    if(ADC_USED){	
            switch(bustype){
                case spibus:
					/* Register a SPI channel.*/
					//spi_user_rec_b = register_user(SPI_USER_NAME, SPI_USER_ID);
					/*Configure SPI Mode*/
					//spi_set_arbiter(spi_user_rec_b, SPI_ARBITER_MASTER);  /*SPI as Master*/
					//spi_set_op_mode(spi_user_rec_b, SPI_POLLING_MODE);
					//spi_set_port_mode(spi_user_rec_b, SPI_SSN_PORT_PTP); /*Point to Point*/
					//spi_set_clk_mode(spi_user_rec_b, SPI_MODE_3); /*(polarity, phase) => (1, 1)*/
					//spi_set_freq(spi_user_rec_b, SPI_FREQ);
                  break;
				case i2cbus:
                  break;
                default:
                  break;
			}
		}
	}
	/*schedule_work(&bat_work);*/
	/*schedule_work(&ac_work);*/
	setup_timer(&polling_timer, polling_timer_func, 0);
	mod_timer(&polling_timer,
		  jiffies + msecs_to_jiffies(polling_interval));
    first_update = 0;
    charing_first_update = 0;
    discharing_first_update = 0;

	return 0;
}
#else
#define wmt_battery_suspend NULL
#define wmt_battery_resume NULL
#endif

/*****************************************************************/
/*                              AC                               */
/*****************************************************************/
static int wmt_ac_get_property(struct power_supply *ac_ps,
			    enum power_supply_property psp,
			    union power_supply_propval *val)
{
	switch (psp) {
		case POWER_SUPPLY_PROP_ONLINE:
					val->intval = ac_dcin;
			break;		

		default:
			return -EINVAL;
	}
	return 0;
}

static void wmt_ac_external_power_changed(struct power_supply *ac_ps)
{
	//printk("wmt_ac_external_power_changed======================\n");
	schedule_work(&ac_work);
}

static void wmt_ac_update(struct power_supply *ac_ps)
{

	mutex_lock(&ac_work_lock);
	if(batt_operation){
		ac_dcin= wmt_read_dcin_DT(ac_ps);

		if((bat_low) || (ac_dcin != ac_dcin_old)){
			power_supply_changed(ac_ps);

		}
		ac_dcin_old = ac_dcin;
		/*printk("Bat: ac_dcin = %d, ac_dcin_old = %d \n",ac_dcin,ac_dcin_old);*/
	    if (!first_update) {
			power_supply_changed(ac_ps);
		    ++first_update;
	    }

	}else{
         ac_dcin = 1;
	}
	mutex_unlock(&ac_work_lock);
}


static enum power_supply_property wmt_ac_props[] = {
    POWER_SUPPLY_PROP_ONLINE,
};

static struct power_supply ac_ps = {
	.name					= "wmt-ac",
	.type					= POWER_SUPPLY_TYPE_MAINS,
	.get_property			= wmt_ac_get_property,
	.external_power_changed = wmt_ac_external_power_changed,
	.properties 			= wmt_ac_props,
	.num_properties 		= ARRAY_SIZE(wmt_ac_props),
	.use_for_apm			= 1,
};


static void wmt_ac_work(struct work_struct *work)
{
	wmt_ac_update(&ac_ps);
}

/*****************************************************************/
/*                         AC     End                            */
/*****************************************************************/

static int __devinit wmt_battery_probe(struct platform_device *dev)
{
	int ret = 0;

	/*printk("Bat: Probe...\n");*/
	pdata = dev->dev.platform_data;
	if (dev->id != -1)
		return -EINVAL;

	mutex_init(&work_lock);
	mutex_init(&ac_work_lock);

	if (!pdata) {
		dev_err(&dev->dev, "Please use wmt_bat_set_pdata\n");
		return -EINVAL;
	}

	INIT_WORK(&bat_work, wmt_battery_work);
	INIT_WORK(&ac_work, wmt_ac_work);
	if (!pdata->batt_name) {
		dev_info(&dev->dev, "Please consider setting proper battery "
				"name in platform definition file, falling "
				"back to name \"wmt-batt\"\n");
		bat_ps.name = "wmt-batt";
	} else
		bat_ps.name = pdata->batt_name;


	ret = power_supply_register(&dev->dev, &bat_ps);
	if (!ret)
		schedule_work(&bat_work);
	else
		goto err;

     /*************     AC     ************/
	
	printk("Bat: Probe...0\n");
	ret = power_supply_register(&dev->dev, &ac_ps);
	if (!ret)
		schedule_work(&ac_work);
	else
		goto err;

	setup_timer(&polling_timer, polling_timer_func, 0);
	mod_timer(&polling_timer,
			  jiffies + msecs_to_jiffies(polling_interval));

	ret = wmt_batt_init();
	if (ret)
		goto err;
	
	if(batt_operation){
		gpio_ini();
	    if(ADC_USED){	
            switch(bustype){
                case spibus:
					//spi_ini();
					/* Register a SPI channel.*/
					//spi_user_rec_b = register_user(SPI_USER_NAME, SPI_USER_ID);
					/*Configure SPI Mode*/
					//spi_set_arbiter(spi_user_rec_b, SPI_ARBITER_MASTER);  /*SPI as Master*/
					//spi_set_op_mode(spi_user_rec_b, SPI_POLLING_MODE);
					//spi_set_port_mode(spi_user_rec_b, SPI_SSN_PORT_PTP); /*Point to Point*/
					//spi_set_clk_mode(spi_user_rec_b, SPI_MODE_3); /*(polarity, phase) => (1, 1)*/
					//spi_set_freq(spi_user_rec_b, SPI_FREQ);
                  break;
				case i2cbus:
                  break;
                default:
                  break;
			}

			ret = adc_init();
			if (!ret)
				goto err;
			}
	}

	return 0;
err:
	return ret;
}

static int __devexit wmt_battery_remove(struct platform_device *dev)
{

	flush_scheduled_work();
	del_timer_sync(&polling_timer);
	
	/*printk("Bat: remove...\n");*/
	power_supply_unregister(&bat_ps);
	power_supply_unregister(&ac_ps);

	kfree(wmt_bat_props);
	kfree(wmt_ac_props);
	return 0;
}

static struct wmt_battery_info wmt_battery_pdata = {
        .charge_gpio    = 5,
        .max_voltage    = 4000,
        .min_voltage    = 3000,
        .batt_mult      = 1000,
        .batt_div       = 414,
        .temp_mult      = 1,
        .temp_div       = 1,
        .batt_tech      = POWER_SUPPLY_TECHNOLOGY_LION,
        .batt_name      = "wmt-battery",
};


static struct platform_device wmt_battery_device = {
	.name           = "wmt-battery",
	.id             = -1,
	.dev            = {
		.platform_data = &wmt_battery_pdata,
	
	},
};


static struct platform_driver wmt_battery_driver = {
	.driver	= {
		.name	= "wmt-battery",
		.owner	= THIS_MODULE,
	},
	.probe		= wmt_battery_probe, 
	.remove		= __devexit_p(wmt_battery_remove),
	.suspend	= wmt_battery_suspend,
	.resume		= wmt_battery_resume,
};

static int __init wmt_battery_init(void)
{
	int ret;
	ret = platform_device_register(&wmt_battery_device);
	if (ret != 0) {
		/*DPRINTK("End1 ret = %x\n",ret);*/
		return -ENODEV;
	}
	ret = platform_driver_register(&wmt_battery_driver);
	
	return ret;
}
static void __exit wmt_battery_exit(void)
{
	platform_driver_unregister(&wmt_battery_driver);
	platform_device_unregister(&wmt_battery_device);
}


//module_init(wmt_battery_init);
late_initcall(wmt_battery_init);
module_exit(wmt_battery_exit);

MODULE_AUTHOR("WonderMedia Technologies, Inc.");
MODULE_DESCRIPTION("WONDERMEDIA battery driver");
MODULE_LICENSE("GPL");

