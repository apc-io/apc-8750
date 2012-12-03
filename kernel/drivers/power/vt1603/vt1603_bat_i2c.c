/*++
	Copyright (c) 2008  WonderMedia Technologies, Inc.

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
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/i2c-id.h>
#include <mach/hardware.h>
#include <linux/platform_device.h>
#include "vt1603_bat.h"

//#define DEBUG
#ifdef  DEBUG
static int dbg_mask = 0;
module_param(dbg_mask, int, S_IRUGO | S_IWUSR);
#define bat_dbg(fmt, args...) \
        do {\
            if (dbg_mask) \
                printk(KERN_ERR "[%s]_%d: " fmt, __func__ , __LINE__, ## args);\
        } while(0)
#define bat_trace() \
        do {\
            if (dbg_mask) \
                printk(KERN_ERR "trace in %s %d\n", __func__, __LINE__);\
        } while(0)
#else
#define bat_dbg(fmt, args...)
#define bat_trace()
#endif

/* 
 * vt1603_i2c_fix_addr - fix vt1603 i2c address, this function MUST
 *    be invoked before seting/getting vt1603's register via i2c bus
 * @i2c:  client for vt1603 (as i2c slave)
 */
static inline void vt1603_i2c_fix_addr(struct i2c_client *i2c)
{
    i2c->addr = VT1603_I2C_FIX_ADDR;
}

/*
 * vt1603_i2c_fake_addr - fake vt1603 i2c address, this function MUST
 *    be invoked after setting/getting vt1603's register via i2c bus
 * @i2c:  client for vt1603 (as i2c slave)
 */
static inline void vt1603_i2c_fake_addr(struct i2c_client *i2c)
{
    i2c->addr = VT1603_I2C_FAKE_ADDR;
}

/*
 * vt1603_i2c_write - write a u8 data to vt1603 via i2c bus
 * @i2c:  client for vt1603 (as i2c slave)
 * @addr: vt1603 register address
 * @data: data write to register
 */
static int vt1603_i2c_write(struct i2c_client *i2c, u8 reg, u8 *data)
{
    int ret = 0;
    int retry = 10;
    u8 cmd[2] = {0};

    cmd[0] = reg;
    cmd[1] = data[0];
    vt1603_i2c_fix_addr(i2c);
    while (--retry) {
        ret = i2c_master_send(i2c, cmd, 2);
        if (ret == 2)
            break;
        msleep(5);
    }
    vt1603_i2c_fake_addr(i2c);
    return ret;
}

/*
 * vt1603_i2c_read - read a u8 data to vt1603 via i2c bus
 * @i2c:  client for vt1603 (as i2c slave)
 * @addr: vt1603 register address
 * @data: data read from vt1603
 */
static int vt1603_i2c_read(struct i2c_client *i2c, u8 reg, u8 *data)
{
    int i = 10;
    int ret = 0;
    struct i2c_msg msg[2] = { {0} };

    /* fix i2c address first */
    vt1603_i2c_fix_addr(i2c);
    /* build msg to write    */
    msg[0].flags = VT1603_I2C_RWCMD;
    msg[0].addr  = i2c->addr;
    msg[0].len   = 1;
    msg[0].buf   = &reg;
    /* build msg to read     */
    msg[1].flags = VT1603_I2C_RCMD;
    msg[1].addr  = i2c->addr;
    msg[1].len   = 1;
    msg[1].buf   = data;
    /* start data transfer   */
    while (i--) {
        ret = i2c_transfer(i2c->adapter, msg, 2);
        if (ret == 2)
            break;
        msleep(5);
    }
    /* fake i2c address last */
    vt1603_i2c_fake_addr(i2c);
    /* ret == 2, means transfer success */
    if (ret == 2)
        ret = 0;
    return ret;
}

/*
 * vt1603_set_reg8 - set register value of vt1603
 * @bat_drv: vt1603 battery driver data
 * @reg: vt1603 register address
 * @val: value register will be set
 */
static int vt1603_set_reg8(struct vt1603_bat_drvdata *bat_drv, u8 reg, u8 val)
{
    return vt1603_i2c_write(bat_drv->i2c, reg, &val);
}

/*
 * vt1603_get_reg8 - get register value of vt1603
 * @bat_drv: vt1603 battery driver data
 * @reg: vt1603 register address
 */
static u8 vt1603_get_reg8(struct vt1603_bat_drvdata *bat_drv, u8 reg)
{
    u8  val = 0;
    int ret = 0;

    ret = vt1603_i2c_read(bat_drv->i2c, reg, &val);
    if (ret)
        bat_dbg("vt1603_bat: read register failed with error:%d\n", ret);
    return val;
}

/*
 * vt1603_setbits - write bit1 to related register's bit
 * @bat_drv: vt1603 battery driver data
 * @reg: vt1603 register address
 * @mask: bit setting mask
 */
static void vt1603_setbits(struct vt1603_bat_drvdata *bat_drv, u8 reg, u8 mask)
{
    u8 tmp = vt1603_get_reg8(bat_drv, reg) | mask;
    vt1603_set_reg8(bat_drv, reg, tmp);
}

/*
 * vt1603_clrbits - write bit0 to related register's bit
 * @bat_drv: vt1603 battery driver data
 * @reg: vt1603 register address
 * @mask:bit setting mask
 */
static void vt1603_clrbits(struct vt1603_bat_drvdata *bat_drv, u8 reg, u8 mask)
{
    u8 tmp = vt1603_get_reg8(bat_drv, reg) & (~mask);
    vt1603_set_reg8(bat_drv, reg, tmp);
}

/*
 * vt1603_reg_dump - dubug function, for dump vt1603 related registers
 * @bat_drv: vt1603 battery driver data
 */
static void vt1603_reg_dump(struct vt1603_bat_drvdata *bat_drv, u8 addr, int len)
{
    u8 i;
    for (i = addr; i < addr + len; i += 2)
        bat_dbg("reg[%d]:0x%02X,  reg[%d]:0x%02X\n", 
                i, vt1603_get_reg8(bat_drv, i), 
                i + 1, vt1603_get_reg8(bat_drv, i + 1));
}

/*
 * vt1603_bat - global data for battery information maintenance
 */
static struct vt1603_bat_drvdata *vt1603_bat = NULL;

#ifdef CONFIG_VT1603_BATTERY_ENABLE
/*
 * vt1603_get_bat_info - get battery status, API for wmt_battery.c
 */
unsigned int vt1603_get_bat_info(void)
{
    if (vt1603_bat)
        return vt1603_bat->bat_val;
    else
        return -ENODEV;
}
EXPORT_SYMBOL_GPL(vt1603_get_bat_info);

/*
 * vt1603_get_bat_convert_data - get battery converted data
 * @bat_drv: vt1603 battery driver data
 */
static u16 vt1603_get_bat_convert_data(struct vt1603_bat_drvdata *bat_drv)
{
    u8 data_l, data_h;

    data_l = vt1603_get_reg8(bat_drv, VT1603_DATL_REG);
    data_h = vt1603_get_reg8(bat_drv, VT1603_DATH_REG);
    return ADC_DATA(data_l, data_h);
}

/*
 * vt1603_work_mode_switch - switch VT1603 to battery mode
 * @bat_drv: vt1603 battery driver data
 */
static void vt1603_switch_to_bat_mode(struct vt1603_bat_drvdata *bat_drv)
{
    bat_dbg("Enter\n");    
    vt1603_set_reg8(bat_drv, VT1603_CR_REG, 0x00);
    vt1603_set_reg8(bat_drv, VT1603_AMCR_REG, BIT0);
    bat_dbg("Exit\n");
}

/*
 * vt1603_bat_tmr_isr - vt1603 battery detect timer isr
 * @bat_drvdata_addr: address of vt1603 battery driver data
 */
static void vt1603_bat_tmr_isr(unsigned long bat_drvdata_addr)
{
    struct vt1603_bat_drvdata *bat_drv;

    bat_dbg("Enter\n");
    bat_drv = (struct vt1603_bat_drvdata *)bat_drvdata_addr;
    schedule_work(&bat_drv->work);
    bat_dbg("Exit\n");
    return ;
}

/*
 * vt1603_get_pen_state - get touch panel pen state from vt1603 
 *         interrup status register
 * @bat_drv: vt1603 battery driver data
 */
static inline int vt1603_get_pen_state(struct vt1603_bat_drvdata *bat_drv)
{
    u8 state = vt1603_get_reg8(bat_drv, VT1603_INTS_REG);
    return (((state & BIT4) == 0) ? TS_PENDOWN_STATE : TS_PENUP_STATE);
}

static inline void vt1603_bat_pen_manual(struct vt1603_bat_drvdata *bat_drv)
{
    vt1603_setbits(bat_drv, VT1603_INTCR_REG, BIT7);
}

static void vt1603_bat_power_up(struct vt1603_bat_drvdata *bat_drv)
{
    if (vt1603_get_reg8(bat_drv, VT1603_PWC_REG) != 0x08)
        vt1603_set_reg8(bat_drv, VT1603_PWC_REG, 0x08);

    return ;
}

/*
 * vt1603_bat_work - vt1603 battery workqueue routine, switch 
 *  vt1603 working mode to battery detecting
 * @work: battery work struct
 */
static void vt1603_bat_work(struct work_struct *work)
{
    u8 tmp = 0;
    u16 val = 0;
    int tout = 0;
    unsigned long now = 0;
    struct vt1603_bat_drvdata *bat_drv;

    bat_dbg("Enter\n");
    bat_drv = container_of(work, struct vt1603_bat_drvdata, work);
    if (unlikely(vt1603_get_pen_state(bat_drv) == TS_PENDOWN_STATE)) {
        bat_dbg("vt1603 pendown when battery detect\n");
        tout = 1000;
        goto out;
    }
    /* enable sar-adc power and clock        */
    vt1603_bat_power_up(bat_drv);
    /* enable pen down/up to avoid miss irq  */
    vt1603_bat_pen_manual(bat_drv);
    /* switch vt1603 to battery detect mode  */
    vt1603_switch_to_bat_mode(bat_drv);
    /* do conversion use battery manual mode */
    vt1603_setbits(bat_drv, VT1603_INTS_REG, BIT0);
    if (vt1603_get_reg8(bat_drv, VT1603_AMCR_REG) != 0x01) {
        tout = 1000;
        bat_dbg("vt1603 battery channel changed already?\n");
        goto out;
    }
    vt1603_set_reg8(bat_drv, VT1603_CR_REG, BIT4);
    now = jiffies;
    while (time_before(jiffies, now + msecs_to_jiffies(POLL_TOUT))) {
        tmp = vt1603_get_reg8(bat_drv, VT1603_INTS_REG);
        if (tmp & BIT0) {
            val = vt1603_get_bat_convert_data(bat_drv);
            bat_dbg("vt1603 battery value is %d\n", val);
            if (vt1603_get_reg8(bat_drv, VT1603_AMCR_REG) != 0x01) {
                tout = 1000;
                bat_dbg("vt1603 battery channel changed already?\n");
                goto out;
            }
            if (val < 2048) {
                tout = 1000;
                printk(KERN_ERR "vt1603 battery conversion failed?!\n");
                goto out;
            }
            bat_drv->bat_val = val;
            bat_drv->detect_time++;
            bat_drv->time_stamp = jiffies;
            tout = bat_drv->interval;
            goto out;
        }
    }
    bat_dbg("vt1603 battery detect failed, conversion timeout!\n");
    tout = 1000;
    goto out;

out:
    vt1603_clrbits(bat_drv, VT1603_INTCR_REG, BIT7);
    vt1603_setbits(bat_drv, VT1603_INTS_REG, BIT0 | BIT3);
    vt1603_set_reg8(bat_drv, VT1603_CR_REG, BIT1);
    mod_timer(&bat_drv->bat_tmr, jiffies + msecs_to_jiffies(tout));
    bat_dbg("Exit\n\n\n");
    return ;
}

/*
 * vt1603_bat_info_init - vt1603 battery initialization
 * @bat_drv: vt1603 battery driver data
 */
static void vt1603_bat_info_init(struct vt1603_bat_drvdata *bat_drv)
{
    struct vt1603_bat_platform_data *bat_pdata = NULL;
    bat_pdata = bat_drv->i2c->dev.platform_data;
    bat_drv->interval     = bat_pdata->interval;
    bat_drv->bat_val      = 3500;
    bat_drv->detect_time  = 0;
    bat_drv->time_stamp   = jiffies;
    INIT_WORK(&bat_drv->work, vt1603_bat_work);
    setup_timer(&bat_drv->bat_tmr, vt1603_bat_tmr_isr, (u32)bat_drv);
}
#endif

static int __devinit 
vt1603_bat_i2c_probe(struct i2c_client *i2c, const struct i2c_device_id *id)
{
    int ret = 0;
    struct vt1603_bat_drvdata *bat_drv = NULL;
    struct vt1603_bat_platform_data *bat_pdata;

    bat_dbg("Enter\n");
    bat_pdata = i2c->dev.platform_data;
    if (NULL == bat_pdata) {
        bat_dbg("vt1603_bat: platform data NULL\n");
        ret = -ENODATA;
        goto out;
    }
    bat_drv = kzalloc(sizeof(*bat_drv), GFP_KERNEL);
    if (!bat_drv) {
        bat_dbg("vt1603_bat: alloc driver data failed\n");
        ret = -ENOMEM;
        goto out;
    }
    vt1603_bat   = bat_drv;
    bat_drv->i2c = i2c;
    dev_set_drvdata(&i2c->dev, bat_drv);
    vt1603_reg_dump(bat_drv, 0xc0, 12);
#ifdef CONFIG_VT1603_BATTERY_ENABLE
    vt1603_bat_info_init(bat_drv);
    vt1603_bat_work(&bat_drv->work);
#endif
    printk(KERN_INFO "VT1603 SAR-ADC Battery Driver OK!\n");
    goto out;

out:
    bat_dbg("Exit\n");
    return ret;
}

#ifdef CONFIG_VT1603_BATTERY_ALARM
static void vt1603_gpio1_low_active_setup(struct vt1603_bat_drvdata *bat_drv)
{
    /* mask other module interrupts */
    vt1603_set_reg8(bat_drv, VT1603_IMASK_REG27, 0xFF);
    vt1603_set_reg8(bat_drv, VT1603_IMASK_REG28, 0xFF);
    vt1603_set_reg8(bat_drv, VT1603_IMASK_REG29, 0xFF);
    /* gpio1 low active             */
    vt1603_set_reg8(bat_drv, VT1603_IPOL_REG33, 0xFF);
    /* vt1603 gpio1 as IRQ output   */
    vt1603_set_reg8(bat_drv, VT1603_ISEL_REG36, 0x04);
    /* clear vt1603 irq             */
    vt1603_setbits(bat_drv, VT1603_INTS_REG, BIT0 | BIT1 | BIT2 | BIT3);
}

static void wm8650_wakeup_init(int wk_src)
{
    bat_dbg("Enter\n");
    bat_dbg("vt1603 battery alarm use wakeup src:0x%08x\n", wk_src);
    /* disable WAKE_UP first  */
    REG8_VAL(GPIO_CTRL_GP2_WAKEUP_SUS_BYTE_ADDR) &= ~wk_src;
    /* disable WAKE_UP output */
    REG8_VAL(GPIO_OC_GP2_WAKEUP_SUS_BYTE_ADDR) &= ~wk_src;
    /* enable pull high/low   */
    REG8_VAL(GPIO_PULL_EN_GP2_WAKEUP_SUS_BYTE_ADDR) |= wk_src;
    /* set pull high WAKE_UP  */
    REG8_VAL(GPIO_PULL_CTRL_GP2_WAKEUP_SUS_BYTE_ADDR) |= wk_src;
    /* enable WAKE_UP last    */
    REG8_VAL(GPIO_CTRL_GP2_WAKEUP_SUS_BYTE_ADDR) |= wk_src;

    bat_dbg("Exit\n");
    return ;
}

static int
vt1603_bat_alarm_setup(struct vt1603_bat_drvdata *bat_drv, u8 threshold, int wk_src)
{
    bat_dbg("Enter\n");

    /* register setting  */
    vt1603_set_reg8(bat_drv, VT1603_PWC_REG,  0x08);
    vt1603_set_reg8(bat_drv, VT1603_CR_REG,   0x00);
    vt1603_set_reg8(bat_drv, VT1603_AMCR_REG, 0x01);
    vt1603_set_reg8(bat_drv, VT1603_BTHD_REG, threshold & 0xFF);
    vt1603_clrbits(bat_drv,  VT1603_BAEN_REG, BIT2);
    vt1603_setbits(bat_drv,  VT1603_BAEN_REG, BIT1 | BIT0);
    vt1603_set_reg8(bat_drv, VT1603_BCLK_REG, 0x28);
    /* vt1603 gpio1 setup */
    vt1603_gpio1_low_active_setup(bat_drv);
    /* wakeup setup       */
    wm8650_wakeup_init(wk_src);

    bat_dbg("Exit\n");
    return 0;
}

static void wm8650_wakeup_exit(u8 wk_src)
{
    bat_dbg("Enter\n");
    bat_dbg("vt1603 battery alarm use wakeup src:0x%08x\n", wk_src);
    /* disable WAKE_UP */
    REG8_VAL(GPIO_CTRL_GP2_WAKEUP_SUS_BYTE_ADDR) &= ~wk_src;
    bat_dbg("Exit\n");
    return ;
}

static int
vt1603_bat_alarm_release(struct vt1603_bat_drvdata *bat_drv, int wk_src)
{
    bat_dbg("Enter\n");

    wm8650_wakeup_exit(wk_src);
    vt1603_clrbits(bat_drv, VT1603_BAEN_REG, BIT1 | BIT0);

    bat_dbg("Exit\n");
    return 0;
}
#endif

static int __devexit vt1603_bat_i2c_remove(struct i2c_client *i2c)
{
    struct vt1603_bat_drvdata *bat_drv = dev_get_drvdata(&i2c->dev);

    bat_dbg("Enter\n");
    vt1603_bat = NULL;
#ifdef CONFIG_VT1603_BATTERY_ENABLE
    del_timer_sync(&bat_drv->bat_tmr);
    bat_drv->bat_val = 0;
    bat_drv->detect_time = 0;
#endif
    dev_set_drvdata(&i2c->dev, NULL);
    kfree(bat_drv);
    bat_drv = NULL;

    bat_dbg("Exit\n");
    return 0;
}

#ifdef CONFIG_PM
static int vt1603_bat_i2c_suspend(struct i2c_client *i2c, pm_message_t message)
{
    struct vt1603_bat_drvdata *bat_drv = NULL;
    struct vt1603_bat_platform_data *bat_pdata = NULL;

    bat_dbg("Enter\n");
    bat_drv = dev_get_drvdata(&i2c->dev);
    bat_pdata = i2c->dev.platform_data;
#ifdef CONFIG_VT1603_BATTERY_ENABLE
    del_timer_sync(&bat_drv->bat_tmr);
#endif
#ifdef CONFIG_VT1603_BATTERY_ALARM
    vt1603_bat_alarm_setup(bat_drv, bat_pdata->alarm_threshold, 
                                bat_pdata->wakeup_src);
#endif
    bat_dbg("Exit\n");
    return 0;
}

static int vt1603_bat_i2c_resume(struct i2c_client *i2c)
{
    struct vt1603_bat_drvdata *bat_drv = NULL;
    struct vt1603_bat_platform_data *bat_pdata = NULL;

    bat_dbg("Enter\n");
    bat_drv = dev_get_drvdata(&i2c->dev);
    bat_pdata = i2c->dev.platform_data;
#ifdef CONFIG_VT1603_BATTERY_ALARM
    vt1603_bat_alarm_release(bat_drv, bat_pdata->wakeup_src);
#endif
#ifdef CONFIG_VT1603_BATTERY_ENABLE
    mod_timer(&bat_drv->bat_tmr, jiffies + msecs_to_jiffies(3000));
#endif
    bat_dbg("Exit\n");
    return 0;
}

#else
#define vt1603_bat_i2c_suspend NULL
#define vt1603_bat_i2c_resume  NULL
#endif

static struct vt1603_bat_platform_data vt1603_bat_pdata = {
    .i2c_bus_id      = VT1603_I2C_BUS_0,
    .interval        = DFLT_POLLING_BAT_INTERVAL,
    .alarm_threshold = 0xc0,
    .wakeup_src      = BA_WAKEUP_SRC_0,
};

static struct i2c_device_id vt1603_bat_i2c_id[] = {
	{ DEV_NAME, 0 },
	{ },
};

static struct i2c_driver vt1603_bat_i2c_driver = {
    .driver    = {
        .name  = DEV_NAME,
        .owner = THIS_MODULE,
        },
    .probe     = vt1603_bat_i2c_probe,
    .remove    = vt1603_bat_i2c_remove,
    .suspend   = vt1603_bat_i2c_suspend,
    .resume    = vt1603_bat_i2c_resume,
    .id_table  = vt1603_bat_i2c_id,
};

static struct i2c_board_info vt1603_bat_i2c_board_info = {
	.type          = DEV_NAME,
	.flags         = 0x00,
    .addr          = VT1603_I2C_FAKE_ADDR,
	.archdata      = NULL,
	.irq           = -1,
    .platform_data = &vt1603_bat_pdata,
};

static int __init vt1603_bat_uboot_env_check(void)
{
	int len = 128;
    char buf[128] = { 0 };
    int ret = 0;
    int bat_op = 0;
    int bat_update_interval = 0;

    /* TODO uboot env should be like
     * 2:1000:b78:972:b29:b11:afe:aca:aab:a94:a80:a64:a40:a1b:972 */
	ret = wmt_getsyspara("wmt.io.bat", buf, &len);
    if (ret) {
        bat_dbg("can not find parameter: wmt.io.bat\n");
        ret = -ENODATA;
        goto out;
    }
    bat_dbg("%s", buf);
    sscanf(buf,"%x:%x", &bat_op, &bat_update_interval);
    bat_dbg("bat_op:%d, bat_update_interval:%d\n", bat_op, bat_update_interval);
	if (bat_op != 2) {
		bat_dbg("VT1603 Battery disabled now\n");
		ret = -ENODEV;
        goto out;
    }
    /* battery update interval 1sec ~ 30sec */
    if ((bat_update_interval > 1000) && (bat_update_interval < 30000))
        vt1603_bat_pdata.interval = bat_update_interval;

out:
    return ret;
}

static int __init vt1603_bat_i2c_init(void)
{
    int ret = 0;
    struct i2c_adapter *adapter = NULL;
    struct i2c_client *client   = NULL;
    struct i2c_board_info *vt1603_i2c_bi = NULL;

    bat_dbg("Enter\n");
    ret = vt1603_bat_uboot_env_check();
    if (ret) {
        bat_dbg("vt1603_bat uboot env check failed\n");
        goto out;                
    }

    vt1603_i2c_bi = &vt1603_bat_i2c_board_info;
    adapter = i2c_get_adapter(vt1603_bat_pdata.i2c_bus_id);
    if (NULL == adapter) {
        bat_dbg("can not get i2c adapter, client address error\n");
        ret = -ENODEV;
        goto out;
    }

    client = i2c_new_device(adapter, vt1603_i2c_bi);
    if (client == NULL) {
        bat_dbg("allocate i2c client failed\n");
        ret = -ENOMEM;
        goto out;
    }
    i2c_put_adapter(adapter);

    ret =  i2c_add_driver(&vt1603_bat_i2c_driver);
    if (ret) {
        bat_dbg("register vt1603 battery i2c driver failed\n");
        goto release_client;
    }
    bat_dbg("Exit\n");
    goto out;

release_client:
    i2c_unregister_device(client);
    client = NULL;
out:
    return ret;
}
late_initcall(vt1603_bat_i2c_init);

static void __exit vt1603_bat_i2c_exit(void)
{
    bat_dbg("Enter\n");
    i2c_unregister_device(vt1603_bat->i2c);
    i2c_del_driver(&vt1603_bat_i2c_driver);
    bat_dbg("Exit\n");
}
module_exit(vt1603_bat_i2c_exit);

MODULE_AUTHOR("WonderMedia Technologies, Inc");
MODULE_DESCRIPTION("VT1603A SAR-ADC Battery Driver");
MODULE_LICENSE("Dual BSD/GPL");
