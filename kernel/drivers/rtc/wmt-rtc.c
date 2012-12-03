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

#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/rtc.h>

#include <linux/bitops.h>
#include <linux/pm.h>
#include <mach/hardware.h>
#include <asm/irq.h>

#ifndef DPRINTK
#define DPRINTK                 printk
#endif

#define DRIVER_VERSION          "0.54"

#define RTC_DEF_FREQ            (32768)
#define RTC_DEF_DIVIDER         (32768 - 1)
#define IRQ_RTC_ALARM IRQ_RTC1
#define IRQ_RTC_UPDATE IRQ_RTC2
/*
 * Following are the bits from a classic RTC we want to mimic
 */
#define RTC_IRQF                0x80      /* any of the following 3 is active */
#define RTC_PF                  0x40
#define RTC_AF                  0x20
#define RTC_UF                  0x10

struct rtc_regs_t {
	unsigned int volatile RTTS;             /* RTC Time Set Register         */
	unsigned int volatile RTDS;             /* RTC Date Set Register         */
	unsigned int volatile RTAS;             /* RTC Alarm Set Register        */
	unsigned int volatile RTCC;             /* RTC Control Register          */
	unsigned int volatile RTCT;             /* RTC Current Time Register     */
	unsigned int volatile RTCD;             /* RTC Current Date Register     */
	unsigned int volatile RTWS;             /* RTC Write Status Register     */
	unsigned int volatile RTTM;             /* RTC Test Mode Register        */
	unsigned int volatile RTTC;             /* RTC Time Calibration Register */
	unsigned int volatile RTIS;             /* RTC Interrupt Status Register */
};

/*
 * For RTTS and RTCT registers
 */
struct bcd_time_t {
	unsigned int sec:7;            /* second       */
	unsigned int min:7;            /* minute       */
	unsigned int hour:6;           /* hour         */
	unsigned int wday:3;           /* day of week  */
	unsigned int rev:8;            /* reversed     */
	unsigned int flag:1;           /* invalid flag */
};

/*
 * For RTDS and RTCD registers
 */
struct bcd_date_t {
	unsigned int mday:6;   /* day of month     */
	unsigned int mon:5;    /* month            */
	unsigned int year:8;   /* year             */
	unsigned int cen:1;    /* century after 2k */
	unsigned int rev:11;   /* reversed         */
	unsigned int flag:1;   /* invalid flag     */
};

/*
 * For RTAS register
 */
struct bcd_alarm_t {
	unsigned int sec:7;    /* second      */
	unsigned int min:7;    /* minute      */
	unsigned int hour:6;   /* hour        */
	unsigned int mday:6;   /* day of week */
	unsigned int cmp:4;    /* compare     */
	unsigned int rev:2;    /* reversed    */
};

struct wmt_alarm_t {
	int type;
	int sec;                        /* seconds after the minute - [0,59] */
	int min;                        /* minutes after the hour - [0,59]   */
	int hour;                       /* hours since midnight - [0,23]     */
	int mday;                       /* day of the month - [1,31]         */
};

/*
 * WMT RTC operation structure.
 */
struct wmt_rtc_t {
	struct rtc_regs_t *const regs;    /* register set */
	unsigned int      rtct;
	unsigned int      rtcd;
	unsigned int      rtas;
};

static struct wmt_rtc_t rtc = {
	/* register set */
	(struct rtc_regs_t *)(io_p2v(__RTC_BASE)),
	/* shadow registers */
	0, 0, 0,
};

/*
 * This struct keep the alarm setting.
 */
static struct rtc_time rtc_alarm = {
	.tm_year        = 100,          /* 2000 */
	.tm_mon         = 0,            /* Jan  */
	.tm_mday        = 1,            /* 1st  */
	.tm_hour        = 0,
	.tm_min         = 0,
	.tm_sec         = 0,
};

extern spinlock_t rtc_lock;       /* in $ARCH\kernel\time.c */
extern int wmt_rtc_on;

/* wmt_set_time()
 *
 * Write new setting to RTC Time Set Register.
 *
 * Note: Check RTC Write Stauts Register first before writing
 *       new value to RTC Time Set Register.
 */
static int wmt_set_time(unsigned int value)
{
	int ret = 0;

	/*
	 * Check for free writing
	 */
	if ((rtc.regs->RTWS & RTWS_TIMESET) == 0) {
		rtc.regs->RTTS = (value & RTTS_TIME);
	} else {
		printk(KERN_ERR "rtc_err : RTTS register write busy!\n");
		ret = -EBUSY;
	}
	return ret;
}

/* wmt_set_date()
 *
 * Write new setting to RTC Date Set Register.
 *
 * Note: Check RTC Write Stauts Register first before writing
 *       new value to RTC Date Set Register.
 */
static int wmt_set_date(unsigned int value)
{
	int ret = 0;

	/*
	 * Check for free writing
	 */
	if ((rtc.regs->RTWS & RTWS_DATESET) == 0)
		rtc.regs->RTDS = (value & RTDS_DATE);
	else
		ret = -EBUSY;

	return ret;
}

/* wmt_set_alarm()
 *
 * Parsing tm structure and then set to RTAS register.
 *
 * Note: Check RTC Write Stauts Register first before writing
 *       new value to RTC Alarm Set Register.
 *       Current policy only can set the alarm to day of month.
 */
static int wmt_set_alarm(struct rtc_time *alarm)
{
	int ret = 0;
	unsigned int value;

	/*
	 * Following one is a good example if you wanna
	 * see the result in RTAS register.
	 * You can clearly see BCD in time and date structure.
	 */

	/*
	 * Check structure "alarm".
	 */
	value = 0;

	if ((alarm->tm_sec >= 0) && (alarm->tm_sec < 60)) {   /* [0,59] valid */
		value |= RTAS_SEC(alarm->tm_sec);
		value |= RTAS_CMPSEC;
	}

	if ((alarm->tm_min >= 0) && (alarm->tm_min < 60)) {   /* [0,59] valid */
		value |= RTAS_MIN(alarm->tm_min);
		value |= RTAS_CMPMIN;
	}

	if ((alarm->tm_hour >= 0) && (alarm->tm_hour < 24)) { /* [0,23] valid */
		value |= RTAS_HOUR(alarm->tm_hour);
		value |= RTAS_CMPHOUR;
	}

	if ((alarm->tm_mday > 0) && (alarm->tm_mday <= 31)) { /* [1,31] valid */
		value |= RTAS_DAY(alarm->tm_mday);
		value |= RTAS_CMPDAY;
	}

	/*
	 * Update new alarm time.
	 */
	if (value) {
		if ((rtc.regs->RTWS & RTWS_ALARMSET) == 0)    /* write free */
			rtc.regs->RTAS = value;
		else
			ret = -EBUSY;
	}
	return ret;
}

static inline int rtc_periodic_alarm(struct rtc_time *tm)
{
	return  (tm->tm_year == -1) ||
		((unsigned)tm->tm_mon >= 12) ||
		((unsigned)(tm->tm_mday - 1) >= 31) ||
		((unsigned)tm->tm_hour > 23) ||
		((unsigned)tm->tm_min > 59) ||
		((unsigned)tm->tm_sec > 59);
}


static irqreturn_t
wmt_rtc_interrupt(int irq, void *dev_id)
{
	struct platform_device *pdev = to_platform_device(dev_id);
	struct rtc_device *wmt_rtc = platform_get_drvdata(pdev);
	unsigned int rtis;
	unsigned long events = 0;

	spin_lock(&rtc_lock);

	/*
	 * Save status in a shadow register
	 * Clear interrupt source.
	 */
	rtis = RTIS_VAL;
	RTIS_VAL = rtis;

	/*
	 * Clear alarm interrupt if it has occurred
	 */
	if (rtis & RTIS_ALARM)
		rtc.regs->RTAS &= ~RTAS_CMPMASK;

	/*
	 * Update irq data & counter
	 */
	if (rtis & RTIS_ALARM)
		events |= (RTC_AF|RTC_IRQF);
	if (rtis & RTIS_UPDATE)
		events |= (RTC_UF|RTC_IRQF);

	rtc_update_irq(wmt_rtc, 1, events);         /* in rtctime.c */

	if ((rtis & RTIS_ALARM) && rtc_periodic_alarm(&rtc_alarm))
		wmt_set_alarm(&rtc_alarm);   /* in rtctime.c */

	spin_unlock(&rtc_lock);

	return IRQ_HANDLED;
}

static int wmt_rtc_open(struct device *dev)
{
	int ret;

	ret = request_irq(IRQ_RTC_UPDATE,
		wmt_rtc_interrupt,
		IRQF_DISABLED,
		"rtc_ticks",
		NULL);
	if (ret) {
		printk(KERN_ERR "rtc: IRQ%d already in use.\n", IRQ_RTC_UPDATE);
		goto fail_update;
	}

	ret = request_irq(IRQ_RTC_ALARM,
		wmt_rtc_interrupt,
		IRQF_DISABLED,
		"rtc_alarm",
		NULL);
	if (ret) {
		printk(KERN_ERR "rtc: IRQ%d already in use.\n", IRQ_RTC_ALARM);
		goto fail_alarm;
	}

	return 0;

fail_alarm:
	free_irq(IRQ_RTC_UPDATE, NULL);
fail_update:
	return ret;
}

static void wmt_rtc_release(struct device *dev)
{
	spin_lock_irq(&rtc_lock);

	/*
	 * Release RTC resource.
	 */
	rtc.regs->RTCC &= ~RTCC_INTENA;
	rtc.regs->RTAS = 0;
	rtc.rtas = 0;

	spin_unlock_irq(&rtc_lock);

	/*
	 * Release IRQ resource.
	 */
	free_irq(IRQ_RTC_UPDATE, NULL);
	free_irq(IRQ_RTC_ALARM, NULL);
}

static int wmt_rtc_ioctl(struct device *dev, unsigned int cmd, unsigned long arg)
{
	switch (cmd) {
	case RTC_AIE_OFF:
	/*
	 * Disable Alarm Interrupt
	 */
		spin_lock_irq(&rtc_lock);
	/*
	 * Save RTAS to shadow register.
	 */
		rtc.rtas = rtc.regs->RTAS;
		rtc.regs->RTAS &= ~RTAS_CMPMASK;
		spin_unlock_irq(&rtc_lock);
		return 0;
	case RTC_AIE_ON:
	/*
	 * Enable Alarm Interrupt
	 */
		spin_lock_irq(&rtc_lock);
	/*
	 * Since the alarm time setting was not changed,
	 * we only recover compare bits from shadow register.
	 */
		rtc.regs->RTAS |= (rtc.rtas & RTAS_CMPMASK);
		spin_unlock_irq(&rtc_lock);
		return 0;
	case RTC_UIE_OFF:
	/*
	* Disable Update Interrupt
	*/
		spin_lock_irq(&rtc_lock);
		rtc.regs->RTCC &= ~RTCC_INTENA;
		spin_unlock_irq(&rtc_lock);
		return 0;
	case RTC_UIE_ON:
	/*
	 * Enable Update Interrupt
	 */
		spin_lock_irq(&rtc_lock);
		rtc.regs->RTCC |= RTCC_INTENA;
		spin_unlock_irq(&rtc_lock);
		return 0;

	/*
	 * Current RTC driver does not support following ioctls
	 */

	case RTC_PIE_OFF:       /* Disable Periodic Interrupt */
	case RTC_PIE_ON:        /* Enable Periodic Interrupt  */
	case RTC_IRQP_READ:     /* Read IRQ rate              */
	case RTC_IRQP_SET:      /* Set IRQ rate               */
		return -EINVAL;
	}
	return -EINVAL;
}

static int wmt_rtc_read_time(struct device *dev, struct rtc_time *tm)
{
	if (tm == NULL) {      /* for debugging */
		printk(KERN_WARNING "RTC: %s *tm is invalid!\n", __func__);
		return -EINVAL;
	}

	rtc.rtct = rtc.regs->RTCT;
	rtc.rtcd = rtc.regs->RTCD;

	if ((rtc.rtct & RTCT_INVALID) || (rtc.rtcd & RTCD_INVALID)) {
		printk(KERN_WARNING "RTC: RTCD/RTCT register invalid flag on!\n");
		printk(KERN_WARNING "RTC: RTCD=0x%.8x RTCT=0x%.8x\n",
		       rtc.regs->RTCD,
		       rtc.regs->RTCT);
		return -EINVAL;
	}

	/*
	 * BCD2BIN translation
	 */
	tm->tm_sec  = RTCT_SEC(rtc.rtct);

	tm->tm_min  = RTCT_MIN(rtc.rtct);

	tm->tm_hour = RTCT_HOUR(rtc.rtct);

	tm->tm_wday = RTCT_DAY(rtc.rtct);

	tm->tm_mday = RTCD_MDAY(rtc.rtcd);

	tm->tm_mon  = RTCD_MON(rtc.rtcd) - 1;       /* 0 means January */

	tm->tm_year = RTCD_YEAR(rtc.rtcd) + ((RTCD_CENT(rtc.rtcd)) * 100 + 100);

	return 0;
}

/* wmt_rtc_set_time()
 *
 * Setup the RTC date and time.
 *
 * In: tm, a rtc_time structure.
 */
static int wmt_rtc_set_time(struct device *dev, struct rtc_time *tm)
{
	int ret = 0;
	unsigned int value;

	/*
	 * Following two are good examples if you wanna
	 * see the result in RTTS and RTDS registers.
	 * You can clearly see BCD in time and date structure.
	 */

	if ((tm->tm_sec < 0) || (tm->tm_sec > 59)) {          /* [0,59] valid */
		printk(KERN_ERR "rtc_err : invalid sec\n");
		ret = -EINVAL;
		goto out;
	}

	if ((tm->tm_min < 0) || (tm->tm_min > 59)) {          /* [0,59] valid */
		printk(KERN_ERR "rtc_err : invalid min\n");
		ret = -EINVAL;
		goto out;
	}

	if ((tm->tm_hour < 0) || (tm->tm_hour > 23)) {        /* [0,23] valid */
		printk(KERN_ERR "rtc_err : invalid hour\n");
		ret = -EINVAL;
		goto out;
	}

	if ((tm->tm_wday < 0) || (tm->tm_wday > 6)) {         /* [0,6] valid */
		printk(KERN_ERR "rtc_err : invalid wday\n");
		ret = -EINVAL;
		goto out;
	}

	if ((tm->tm_mday < 0) || (tm->tm_mday > 31)) {        /* [1,31] valid */
		printk(KERN_ERR "rtc_err : invalid mday\n");
		ret = -EINVAL;
		goto out;
	}

	if ((tm->tm_mon < 0) || (tm->tm_mon > 11)) {          /* [0,11] valid */
		printk(KERN_ERR "rtc_err : invalid mon\n");
		ret = -EINVAL;
		goto out;
	}

	if (tm->tm_year < 100) {                              /* >= 100 valid */
		printk(KERN_ERR "rtc_err : invalid year\n");
		ret = -EINVAL;
		goto out;
	}

	value = RTTS_SEC(tm->tm_sec) | RTTS_MIN(tm->tm_min) | \
		RTTS_HOUR(tm->tm_hour) | RTTS_DAY(tm->tm_wday);

	ret = wmt_set_time(value);

	if (ret)
		goto out;

	value = RTDS_MDAY(tm->tm_mday) | RTDS_MON(tm->tm_mon + 1) | \
		RTDS_YEAR(tm->tm_year%100) | RTDS_CENT((tm->tm_year/100) - 1);

	ret = wmt_set_date(value);
out:
	return ret;
}

static int wmt_rtc_read_alarm(struct device *dev, struct rtc_wkalrm *alrm)
{
	memcpy(&alrm->time, &rtc_alarm, sizeof(struct rtc_time));
	alrm->pending = ((rtc.regs->RTIS & RTIS_ALARM) ? 1 : 0);
	return 0;
}

/*
 * Handle wakeup alarm API information defined in include/linux/rtc.h
 */
static int wmt_rtc_set_alarm(struct device *dev, struct rtc_wkalrm *alrm)
{
	int ret;

	spin_lock_irq(&rtc_lock);

	ret = wmt_set_alarm(&alrm->time);

	if (ret == 0) {
		memcpy(&rtc_alarm, &alrm->time, sizeof(struct rtc_time));

		if (alrm->enabled)
			enable_irq_wake(IRQ_RTC_ALARM);
		else
			disable_irq_wake(IRQ_RTC_ALARM);
	}
	spin_unlock_irq(&rtc_lock);

	return ret;
}

static int wmt_rtc_proc(struct device *dev, struct seq_file *seq)
{

//	char *p = seq->buf;
//	int ret;

	int is_testmode = (RTTM_VAL & RTTM_ENABLE);
/*		
	p += sprintf(p, "test_mode\t: %s\n",
		(is_testmode) ? "enabled" : "disabled");

	p += sprintf(p, "%s\t: 0x%08x\n",
		((is_testmode) ? "divider" : "calibration"),
		RTTC_VAL);

	p += sprintf(p, "alarm_IRQ\t: %s\n",
		(RTIS_VAL & RTIS_ALARM) ? "yes" : "no");

	p += sprintf(p, "ticks_IRQ\t: %s\n",
		(RTIS_VAL & RTIS_UPDATE) ? "yes" : "no");

	ret = p - seq->buf;

	return ret;
*/

    seq_printf(seq, "test_mode\t: %s\n",
    		(is_testmode) ? "enabled" : "disabled");
    
    seq_printf(seq, "%s\t: 0x%08x\n",
    		((is_testmode) ? "divider" : "calibration"),
    		RTTC_VAL);
	
    seq_printf(seq, "alarm_IRQ\t: %s\n",
    		(RTIS_VAL & RTIS_ALARM) ? "yes" : "no");
	
    seq_printf(seq, "ticks_IRQ\t: %s\n",
    		(RTIS_VAL & RTIS_UPDATE) ? "yes" : "no");
    
    return 0;

}

/*
 * Wrap "rtc_ops" to "file_operations" in common/rtctime.c
 */
static const struct rtc_class_ops wmt_rtc_ops = {
	.open           = wmt_rtc_open,
	.release        = wmt_rtc_release,
	.ioctl          = wmt_rtc_ioctl,
	.read_time      = wmt_rtc_read_time,
	.set_time       = wmt_rtc_set_time,
	.read_alarm     = wmt_rtc_read_alarm,
	.set_alarm      = wmt_rtc_set_alarm,
	.proc           = wmt_rtc_proc,
};

static int wmt_rtc_probe(struct platform_device *pdev)
{
    struct rtc_device *wmt_rtc;

    printk("wmt_rtc on\n");
    if ((rtc.regs->RTTM & RTTM_ENABLE) && (rtc.regs->RTTC == 0)) {
        printk(KERN_WARNING "rtc_warn: initializing default clock divider value\n");
        rtc.regs->RTTC = RTC_DEF_DIVIDER;
    }

    if ((rtc.regs->RTCC & RTCC_ENA) == 0) {
        /*
         * Reset RTC alarm settings
         */
        rtc.regs->RTAS = 0;

        /*
         * Reset RTC test mode.
         */
        rtc.regs->RTTM = 0;

        /*
         * Disable all RTC control functions.
         * Set to 24-hr format and update type to each second.
         * Disable sec/min update interrupt.
         * Let RTC free run without interrupts.
         */
        rtc.regs->RTCC = (RTCC_ENA | RTCC_INTTYPE);

        if (rtc.regs->RTCD == 0) {
            while (rtc.regs->RTWS & RTWS_DATESET)
                ;
            rtc.regs->RTDS = rtc.regs->RTDS ;
        }
    }
	
    device_init_wakeup(&pdev->dev, 1);

    wmt_rtc = rtc_device_register(pdev->name, &pdev->dev, &wmt_rtc_ops, THIS_MODULE);

#ifndef CONFIG_SKIP_DRIVER_MSG
    printk(KERN_INFO "WMT Real Time Clock driver v" DRIVER_VERSION " initialized: %s\n",
           wmt_rtc ? "ok" : "failed");
#endif

    if (IS_ERR(wmt_rtc))
        return PTR_ERR(wmt_rtc);

    platform_set_drvdata(pdev, wmt_rtc);
	
    return 0;
}

static int wmt_rtc_remove(struct platform_device *pdev)
{
	struct rtc_device *wmt_rtc = platform_get_drvdata(pdev);

 	if (wmt_rtc)
 	{
 	    rtc.regs->RTCC &= ~RTCC_INTENA;
	    rtc_device_unregister(wmt_rtc);
 	}

	return 0;
}

#ifdef CONFIG_PM
static int wmt_rtc_suspend(struct platform_device *pdev, pm_message_t state)
{
/*
	if (device_may_wakeup(&pdev->dev))
		enable_irq_wake(IRQ_RTC_ALARM);
*/
	return 0;
}

static int wmt_rtc_resume(struct platform_device *pdev)
{
/*	
	if (device_may_wakeup(&pdev->dev))
		disable_irq_wake(IRQ_RTC_ALARM);
*/
	return 0;
}
#else
#define wmt_rtc_suspend	NULL
#define wmt_rtc_resume	NULL
#endif

/***************************************************************************
	platform device struct define
****************************************************************************/
static struct platform_device wmt_rtc_device = {
    .name           = "wmt-rtc",
    .id             = 0,    
};

static struct platform_driver wmt_rtc_driver = {
	.probe		= wmt_rtc_probe,
	.remove		= wmt_rtc_remove,
	.suspend	= wmt_rtc_suspend,
	.resume		= wmt_rtc_resume,
	.driver		= {
		.name		= "wmt-rtc",
	},
};

static int __init wmt_rtc_init(void)
{
    int ret = 0;

    if (wmt_rtc_on) {
        ret = platform_device_register(&wmt_rtc_device);
        if(ret)
            return ret;
  
        ret = platform_driver_register(&wmt_rtc_driver);
        if(ret)
        {
            printk("[wmt_rtc_init]Error: Can not register WMT RTC driver\n");
            platform_device_unregister(&wmt_rtc_device);
            return ret;
        }
    }
    return ret;
}

static void __exit wmt_rtc_exit(void)
{
    if (wmt_rtc_on) {
        platform_driver_unregister(&wmt_rtc_driver);
        platform_device_unregister(&wmt_rtc_device);
    }
}

module_init(wmt_rtc_init);
module_exit(wmt_rtc_exit);

MODULE_AUTHOR("WonderMedia Technologies, Inc.");
MODULE_DESCRIPTION("WMT [real time clock] driver");
MODULE_LICENSE("GPL");
