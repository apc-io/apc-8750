#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <linux/gpio.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/freezer.h>
#include <linux/platform_device.h>
#include <linux/proc_fs.h>
#include <mach/hardware.h>
#include <linux/earlysuspend.h>
#include "mma7660.h"

//#define DEBUG 1
//#define DEBUG_SENSOR
//#define GSENSOR_INT_MODE
//#define WM3445_A0

#ifdef DEBUG_SENSOR
	#define gsensor_dbg(format, arg...) printk(KERN_ALERT format, ## arg)
#else
	#define gsensor_dbg(format, arg...)
#endif

#define X_CONVERT(x) (x*l_sensorconfig.xyz_axis[ABS_X][1])
#define Y_CONVERT(y) (y*l_sensorconfig.xyz_axis[ABS_Y][1])
#define Z_CONVERT(z) (z*l_sensorconfig.xyz_axis[ABS_Z][1])
static int *x_count;
static int *y_count;
static int *z_count;
static int x_total = 0, y_total = 0, z_total = 0;
static int xyz_index = 0;

int xy_degree_table[64] = {
0,269,538,808,1081,1355,1633,1916,
2202,2495,2795,3104,3423,3754,4101,4468,
4859,5283,5754,6295,6964,7986,8000,8000,
8000,8000,8000,8000,9000,9000,9000,9000,
9000,9000,9000,9000,9000,8000,8000,8000,
8000,8000,8000,-7986,-6964,-6295,-5754,-5283,
-4859,-4468,-4101,-3754,-3423,-3104,-2795,-2495,
-2202,-1916,-1633,-1355,-1081,-808,-538,-269
};

int z_degree_table[64] = {
9000,8731,8462,8192,7919,7645,7367,7084,
6798,6505,6205,5896,5577,5246,4899,4532,
4141,3717,3246,2705,2036,1014,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,-1014,-2036,-2705,-3246,-3717,
-4141,-4532,-4899,-5246,-5577,-5896,-6205,-6505,
-6798,-7084,-7367,-7645,-7919,-8192,-8462,-8731
};

// g-value *1000
/*
int xyz_g_table[64] = {
0,47,94,141,188,234,281,328,
375,422,469,516,563,609,656,703,
750,797,844,891,938,984,1031,1078,
1125,1172,1219,1266,1313,1359,1406,1453,
-1500,-1453,-1406,-1359,-1313,-1266,-1219,-1172,
-1125,-1078,-1031,-984,-938,-891,-844,-797,
-750,-703,-656,-609,-563,-516,-469,-422,
-375,-328,-281,-234,-188,-141,-94,-47
};
*/

// g-value*1024
int xyz_g_table[64] = {
0,48,96,144,193,240,288,336,
384,432,480,528,577,624,672,720,
768,816,864,912,961,1008,1056,1104,
1152,1200,1248,1296,1345,1392,1440,1488,
-1536,-1488,-1440,-1392,-1345,-1296,-1248,-1200,
-1152,-1104,-1056,-1008,-961,-912,-864,-816,
-768,-720,-672,-624,-577,-528,-480,-432,
-384,-336,-288,-240,-193,-144,-96,-48
};

static struct platform_device *this_pdev;

struct mma7660_data {
	struct input_dev *input_dev;
#ifdef GSENSOR_INT_MODE
	struct work_struct work;
#else
	struct delayed_work delayed_work;
#endif
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend gsensor_earlysuspend;
#endif
};

struct mma7660_config
{
	int op;
	int samp;
	int xyz_axis[3][3]; // (axis,direction)
	struct proc_dir_entry* sensor_proc;
	int sensorlevel;
	unsigned int sensitive;
	int shake_enable; // 1--enable shake, 0--disable shake
	int manual_rotation; // 0--landance, 90--vertical
	int suspend;
	int work_finish;
	int avg_count;
#ifdef GSENSOR_INT_MODE
	int name;
	int bmp;
	int ctraddr;
	int ocaddr;
	int idaddr;
	int peaddr;
	int pcaddr;
	int itbmp;
	int itaddr;
	int isbmp;
	int isaddr;
	int irq;
#else
	int interval;
#endif
};

static struct mma7660_config l_sensorconfig = {
	.op = 0,
	.samp = 32,
	.xyz_axis = {
		{ABS_X, -1},
		{ABS_Y, 1},
		{ABS_Z, -1},
		},
	.sensor_proc = NULL,
	.sensorlevel = SENSOR_GRAVITYGAME_MODE,
	.sensitive = 0x9,
	.shake_enable = 1, // default enable shake
	.suspend = 0,
	.work_finish = 1,
	.avg_count = 4,
#ifdef GSENSOR_INT_MODE
	.name = 3,
#ifdef WM3445_A0
	.bmp = 0x100, /* GPIO 8 */
	.ctraddr = GPIO_BASE_ADDR + 0x40,
	.ocaddr = GPIO_BASE_ADDR + 0x80,
	.idaddr = GPIO_BASE_ADDR + 0x00,
	.peaddr = GPIO_BASE_ADDR + 0x480,
	.pcaddr = GPIO_BASE_ADDR + 0x4c0,
	.itbmp = 0x30000, /* Rising Edge */
	.itaddr = GPIO_BASE_ADDR + 0x300,
	.isbmp = 0x100,
	.isaddr = GPIO_BASE_ADDR + 0x304,
	.irq = IRQ_GPIO8,
#else
	.bmp = 0x8, /* GPIO 3 */
	.ctraddr = GPIO_BASE_ADDR + 0x40,
	.ocaddr = GPIO_BASE_ADDR + 0x80,
	.idaddr = GPIO_BASE_ADDR + 0x00,
	.peaddr = GPIO_BASE_ADDR + 0x480,
	.pcaddr = GPIO_BASE_ADDR + 0x4c0,
	.itbmp = 0x83000000, /* Rising Edge */
	.itaddr = GPIO_BASE_ADDR + 0x300,
	.isbmp = 0x8,
	.isaddr = GPIO_BASE_ADDR + 0x320,
	.irq = 6,
#endif
#else
	.interval = 250,
#endif
};

#ifdef GSENSOR_INT_MODE
#ifdef WM3445_A0
#define SET_GPIO_GSENSOR_INT() {\
	REG32_VAL(l_sensorconfig.ctraddr) &= ~l_sensorconfig.bmp; \
	REG32_VAL(l_sensorconfig.ocaddr) &= ~l_sensorconfig.bmp; \
	REG32_VAL(l_sensorconfig.pcaddr) &= ~l_sensorconfig.bmp; \
	REG32_VAL(l_sensorconfig.itaddr) |= l_sensorconfig.itbmp; \
	REG32_VAL(GPIO_BASE_ADDR + 0x308) &= ~l_sensorconfig.bmp; \
	REG32_VAL(l_sensorconfig.isaddr) |= l_sensorconfig.isbmp; \
}
#else
#define SET_GPIO_GSENSOR_INT() {\
	REG32_VAL(l_sensorconfig.ctraddr) |= l_sensorconfig.bmp; \
	REG32_VAL(l_sensorconfig.ocaddr) &= ~l_sensorconfig.bmp; \
	REG32_VAL(l_sensorconfig.pcaddr) &= ~l_sensorconfig.bmp; \
	REG32_VAL(l_sensorconfig.itaddr) |= l_sensorconfig.itbmp; \
	REG32_VAL(l_sensorconfig.isaddr) |= l_sensorconfig.isbmp; \
}
#endif
#endif

/* Addresses to scan -- protected by sense_data_mutex */
static struct mutex sense_data_mutex;

static DECLARE_WAIT_QUEUE_HEAD(data_ready_wq);
static DECLARE_WAIT_QUEUE_HEAD(open_wq);

static char cspec_num;
static atomic_t cspec_frq;

static atomic_t open_count;
static atomic_t open_flag;
static atomic_t reserve_open_flag;

static atomic_t m_flag;
static atomic_t a_flag;
static atomic_t t_flag;
static atomic_t mv_flag;

static short mmad_delay = 0;

static atomic_t suspend_flag = ATOMIC_INIT(0);

static int revision = -1;

extern int wmt_getsyspara(char *varname, unsigned char *varval, int *varlen);
extern unsigned int wmt_read_oscr(void); 

#ifdef CONFIG_HAS_EARLYSUSPEND
static void gsensor_early_suspend(struct early_suspend *h);
static void gsensor_late_resume(struct early_suspend *h);
#endif

/* mma HW info */
static ssize_t gsensor_vendor_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;

	sprintf(buf, "AK8976A_%#x\n", revision);
	ret = strlen(buf) + 1;

	return ret;
}

static DEVICE_ATTR(vendor, 0444, gsensor_vendor_show, NULL);

static struct kobject *android_gsensor_kobj;

static int gsensor_sysfs_init(void)
{
	int ret ;

	android_gsensor_kobj = kobject_create_and_add("android_gsensor", NULL);
	if (android_gsensor_kobj == NULL) {
		printk(KERN_ERR
		       "mma7660 gsensor_sysfs_init:"\
		       "subsystem_register failed\n");
		ret = -ENOMEM;
		goto err;
	}

	ret = sysfs_create_file(android_gsensor_kobj, &dev_attr_vendor.attr);
	if (ret) {
		printk(KERN_ERR
		       "mma7660 gsensor_sysfs_init:"\
		       "sysfs_create_group failed\n");
		goto err4;
	}

	return 0 ;
err4:
	kobject_del(android_gsensor_kobj);
err:
	return ret ;
}

extern int i2c_wmt_read_msg(
	unsigned int slave_addr, 	/*!<; //[IN] Salve address */
	char *buf, 					/*!<; //[OUT] Pointer to data */
	unsigned int length,		/*!<; //Data length */
	int restart, 				/*!<; //Need to restart after a complete read */
	int last 					/*!<; //Last read */
);
extern int i2c_wmt_write_msg(
	unsigned int slave_addr, 	/*!<; //[IN] Salve address */
	char *buf, 					/*!<; //[OUT] Pointer to data */
	unsigned int length,		/*!<; //Data length */
	int restart, 				/*!<; //Need to restart after a complete write */
	int last 					/*!<; //Last read */
);

extern int wmt_i2c_xfer_continue_if_2(
	struct i2c_msg *msg,
	unsigned int num
);

extern int wmt_i2c_xfer_continue_if_4(struct i2c_msg *msg, unsigned int num, int bus_id);

int sensor_i2c_write(unsigned int addr,unsigned int index,char *pdata,int len)
{
    struct i2c_msg msg[1];
	unsigned char buf[len+1];

    buf[0] = index;
	memcpy(&buf[1],pdata,len);
    msg[0].addr = addr;
    msg[0].flags = 0 ;
    msg[0].flags &= ~(I2C_M_RD);
    msg[0].len = len+1;
    msg[0].buf = buf;
    wmt_i2c_xfer_continue_if_4(msg, 1, 0);
#ifdef DEBUG
	int i;
	printk("sensor_i2c_write(addr 0x%x,index 0x%x,len %d\n",addr,index,len);
	for(i=0;i<len;i+=8){
		printk("%d : 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",i,
			pdata[i],pdata[i+1],pdata[i+2],pdata[i+3],pdata[i+4],pdata[i+5],pdata[i+6],pdata[i+7]);
	}
#endif
    return 0;
} /* End of sensor_i2c_write */

int sensor_i2c_read(unsigned int addr,unsigned int index,char *pdata,int len)
{
	struct i2c_msg msg[2];
	unsigned char buf[len+1];

	memset(buf,0x55,len+1);
	buf[0] = index;
	buf[1] = 0x0;

	msg[0].addr = addr;
	msg[0].flags = 0 ;
	msg[0].flags &= ~(I2C_M_RD);
	msg[0].len = 1;
	msg[0].buf = buf;

	msg[1].addr = addr;
	msg[1].flags = 0 ;
	msg[1].flags |= (I2C_M_RD);
	msg[1].len = len;
	msg[1].buf = buf;

    wmt_i2c_xfer_continue_if_4(msg, 2, 0);

	memcpy(pdata,buf,len);
#ifdef DEBUG
	int i;
	printk("sensor_i2c_read(addr 0x%x,index 0x%x,len %d\n",addr,index,len);
	for(i=0;i<len;i+=8){
		printk("%d : 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",i,
			pdata[i],pdata[i+1],pdata[i+2],pdata[i+3],pdata[i+4],pdata[i+5],pdata[i+6],pdata[i+7]);
	}
#endif
    return 0;
} /* End of sensor_i2c_read */

static void mma_work_func(struct work_struct *work)
{
	struct mma7660_data *data;
	unsigned char rxData[3];
	int i = 0, timeout = 0;
	int xyz[3];

	if (l_sensorconfig.suspend)
		return;

	l_sensorconfig.work_finish = 0;

	data = dev_get_drvdata(&this_pdev->dev);
	sensor_i2c_read(MMA7660_ADDR,3,rxData,1);
	if (rxData[0] & 0x40) {
		sensor_i2c_read(MMA7660_ADDR,3,rxData,1);
		if (rxData[0] & 0x40) {
			printk(KERN_DEBUG "G-Sensor Alert!\n");
			goto exit;
		}
	}
	if (rxData[0] & 0x80) {
		if (1 == l_sensorconfig.shake_enable) {
			printk(KERN_NOTICE "shake!!!\n");
			input_report_key(data->input_dev, KEY_NEXTSONG, 1);
			input_report_key(data->input_dev, KEY_NEXTSONG, 0);
			input_sync(data->input_dev);
		}
	} else {
		sensor_i2c_read(MMA7660_ADDR,0,rxData,3);
		while (i < 3) {	
			if (rxData[i] & 0xC0) {
				if (timeout) {
					printk(KERN_DEBUG "G-Sensor Alert: x=%x, y=%x, z=%x\n",
						rxData[0],
						rxData[1],
						rxData[2]);
					goto exit;
				}
				timeout++;
				sensor_i2c_read(MMA7660_ADDR,0,rxData,3);
				i = -1;;
			}
			i++;
		}
		if (xyz_index >= l_sensorconfig.avg_count)
			xyz_index = 0;
		x_total -= x_count[xyz_index];
		y_total -= y_count[xyz_index];
		z_total -= z_count[xyz_index];
		x_count[xyz_index] = xyz_g_table[rxData[0]];
		y_count[xyz_index] = xyz_g_table[rxData[1]];
		z_count[xyz_index] = xyz_g_table[rxData[2]];
		x_total += x_count[xyz_index];
		y_total += y_count[xyz_index];
		z_total += z_count[xyz_index];
		xyz[ABS_X] = x_total/l_sensorconfig.avg_count;
		xyz[ABS_Y] = y_total/l_sensorconfig.avg_count;
		xyz[ABS_Z] = z_total/l_sensorconfig.avg_count;
		//printk(KERN_DEBUG "   [%d] x:%d y:%d z:%d\n", xyz_index, x_count[xyz_index], y_count[xyz_index], z_count[xyz_index]);
		//printk(KERN_DEBUG " total x:%d y:%d z:%d\n", x_total, y_total, z_total);
		//printk(KERN_DEBUG "   avg x:%d y:%d z:%d\n", x_total/l_sensorconfig.avg_count, y_total/l_sensorconfig.avg_count, z_total/l_sensorconfig.avg_count);
		//printk(KERN_DEBUG "report x:%d y:%d z:%d\n", xyz[ABS_X], xyz[ABS_Y], xyz[ABS_Z]);
		xyz_index++;
		/*
		gsensor_dbg(KERN_DEBUG "Angle: x=%d, y=%d, z=%d\n",
			xy_degree_table[rxData[0]],
			xy_degree_table[rxData[1]],
			z_degree_table[rxData[2]]);
		gsensor_dbg(KERN_DEBUG "G value: x=%d, y=%d, z=%d\n",
			xyz_g_table[rxData[0]],
			xyz_g_table[rxData[1]],
			xyz_g_table[rxData[2]]);
		*/
		printk(KERN_DEBUG "x:%d y:%d z:%d\n",
			X_CONVERT(xyz[l_sensorconfig.xyz_axis[ABS_X][0]]),
			Y_CONVERT(xyz[l_sensorconfig.xyz_axis[ABS_Y][0]]),
			Z_CONVERT(xyz[l_sensorconfig.xyz_axis[ABS_Z][0]]));
		/*
		input_report_abs(data->input_dev, l_sensorconfig.xyz_axis[0][0], xyz_g_table[(int)rxData[ABS_X]]*l_sensorconfig.xyz_axis[0][1]);
		input_report_abs(data->input_dev, l_sensorconfig.xyz_axis[1][0], xyz_g_table[(int)rxData[ABS_Y]]*l_sensorconfig.xyz_axis[1][1]);
		input_report_abs(data->input_dev, l_sensorconfig.xyz_axis[2][0], xyz_g_table[(int)rxData[ABS_Z]]*l_sensorconfig.xyz_axis[2][1]);
		*/
		input_report_abs(data->input_dev, ABS_X, X_CONVERT(xyz[l_sensorconfig.xyz_axis[ABS_X][0]]));
		input_report_abs(data->input_dev, ABS_Y, Y_CONVERT(xyz[l_sensorconfig.xyz_axis[ABS_Y][0]]));
		input_report_abs(data->input_dev, ABS_Z, Z_CONVERT(xyz[l_sensorconfig.xyz_axis[ABS_Z][0]]));

		input_sync(data->input_dev);
	}
exit:
#ifndef GSENSOR_INT_MODE
	schedule_delayed_work(&data->delayed_work, msecs_to_jiffies(l_sensorconfig.interval));
#endif
	l_sensorconfig.work_finish = 1;
	return;
}

#ifdef GSENSOR_INT_MODE
static void gsensor_int_ctrl(int status)
{
	if (l_sensorconfig.op == 1) {
#ifdef WM3445_A0
		if (status == DISABLE)
			REG32_VAL(GPIO_BASE_ADDR + 0x308) |= l_sensorconfig.bmp;
		else
			REG32_VAL(GPIO_BASE_ADDR + 0x308) &= ~l_sensorconfig.bmp;
#else
		if (status == DISABLE)
			REG32_VAL(l_sensorconfig.itaddr) &= ~(l_sensorconfig.itbmp & 0x80808080);
		else
			REG32_VAL(l_sensorconfig.itaddr) |= (l_sensorconfig.itbmp & 0x80808080);
#endif
	}
}

static int get_gpioset(void)
{
	char varbuf[96];
	int n;
	int varlen;

	memset(varbuf, 0, sizeof(varbuf));
	varlen = sizeof(varbuf);
	if (wmt_getsyspara("wmt.gpt.gsensor", varbuf, &varlen)) {
		printk(KERN_DEBUG "wmt.gpt.gsensor not defined! -> Use default config\n");
		return 0;
	} else {
		n = sscanf(varbuf, "%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x",
				&l_sensorconfig.name,
				&l_sensorconfig.bmp,
				&(l_sensorconfig.ctraddr),
				&(l_sensorconfig.ocaddr),
				&(l_sensorconfig.idaddr),
				&(l_sensorconfig.peaddr),
				&(l_sensorconfig.pcaddr),
				&(l_sensorconfig.itbmp),
				&(l_sensorconfig.itaddr),
				&(l_sensorconfig.isbmp),
				&(l_sensorconfig.isaddr),
				&(l_sensorconfig.irq));
		if (n != 12) {
			printk(KERN_ERR "wmt.gpt.gsensor format is incorrect!\n");
			return 0;
		}

		l_sensorconfig.ctraddr += WMT_MMAP_OFFSET;
		l_sensorconfig.ocaddr += WMT_MMAP_OFFSET;
		l_sensorconfig.idaddr += WMT_MMAP_OFFSET;
		l_sensorconfig.peaddr += WMT_MMAP_OFFSET;
		l_sensorconfig.pcaddr += WMT_MMAP_OFFSET;
		l_sensorconfig.itaddr += WMT_MMAP_OFFSET;
		l_sensorconfig.isaddr += WMT_MMAP_OFFSET;

		printk(KERN_DEBUG "wmt.gpt.gsensor = %x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x\n",
			l_sensorconfig.name,
			l_sensorconfig.bmp,
			l_sensorconfig.ctraddr,
			l_sensorconfig.ocaddr,
			l_sensorconfig.idaddr,
			l_sensorconfig.peaddr,
			l_sensorconfig.pcaddr,
			l_sensorconfig.itbmp,
			l_sensorconfig.itaddr,
			l_sensorconfig.isbmp,
			l_sensorconfig.isaddr,
			l_sensorconfig.irq);
	}
	return 1;
}

static irqreturn_t mma7660_interrupt(int irq, void *dev_id)
{
	struct mma7660_data *data = dev_id;

	if (REG32_VAL(l_sensorconfig.isaddr) & l_sensorconfig.isbmp) {
		REG32_VAL(l_sensorconfig.isaddr) = l_sensorconfig.isbmp;
		schedule_work(&data->work);
		return IRQ_HANDLED;
	} else {
		return IRQ_NONE;
	}
}
#endif

void mma7660_chip_init(struct mma7660_data *data)
{
	char txData[1];
	char i = 0;
	char amsr = 0;
	int samp = 0;

	// the default mode is for gravity
	txData[0]=0;
	sensor_i2c_write(MMA7660_ADDR,7,txData,1);
	switch (l_sensorconfig.sensorlevel)
	{
		case SENSOR_UI_MODE:
			txData[0] = 60;
			sensor_i2c_write(MMA7660_ADDR,5,txData,1);
			txData[0] = (1 == l_sensorconfig.shake_enable) ? 0xE3:0x03;
			sensor_i2c_write(MMA7660_ADDR,6,txData,1);
			txData[0] = 0xFC;
			sensor_i2c_write(MMA7660_ADDR,8,txData,1);
			break;
		case SENSOR_GRAVITYGAME_MODE:
			txData[0] = 0;
			sensor_i2c_write(MMA7660_ADDR,5,txData,1);
#ifdef GSENSOR_INT_MODE
			txData[0] = (1 == l_sensorconfig.shake_enable) ? 0xF0:0x10;
#else
			txData[0] = 0;
#endif
			sensor_i2c_write(MMA7660_ADDR,6,txData,1);
			samp = l_sensorconfig.samp;
			if(samp >= 120) {
				amsr = 0;
			} else {
				while(samp) {
					samp >>= 1;
					i++;
				}
			}
			amsr = 8 - i;
			txData[0] = 0xF8 | amsr;
			gsensor_dbg(KERN_DEBUG "G-Sensor: SR = 0x%X\n", txData[0]);
			sensor_i2c_write(MMA7660_ADDR,8,txData,1);
			break;
	};
	txData[0] = 0xF9;
	sensor_i2c_write(MMA7660_ADDR,7,txData,1);

#ifdef GSENSOR_INT_MODE
	// set gpio as external interrupt
	disable_irq(l_sensorconfig.irq);
#ifdef WM3445_A0
	if (l_sensorconfig.itbmp & 0x3)
		REG32_VAL(l_sensorconfig.itaddr) &= ~0x3;
	else if (l_sensorconfig.itbmp & 0xc)
		REG32_VAL(l_sensorconfig.itaddr) &= ~0xc;
	else if (l_sensorconfig.itbmp & 0x30)
		REG32_VAL(l_sensorconfig.itaddr) &= ~0x30;
	else if (l_sensorconfig.itbmp & 0xc0)
		REG32_VAL(l_sensorconfig.itaddr) &= ~0xc0;
	else if (l_sensorconfig.itbmp & 0x300)
		REG32_VAL(l_sensorconfig.itaddr) &= ~0x300;
	else if (l_sensorconfig.itbmp & 0xc00)
		REG32_VAL(l_sensorconfig.itaddr) &= ~0xc00;
	else if (l_sensorconfig.itbmp & 0x3000)
		REG32_VAL(l_sensorconfig.itaddr) &= ~0x3000;
	else if (l_sensorconfig.itbmp & 0xc000)
		REG32_VAL(l_sensorconfig.itaddr) &= ~0xc000;
	else if (l_sensorconfig.itbmp & 0x30000)
		REG32_VAL(l_sensorconfig.itaddr) &= ~0x30000;
	else if (l_sensorconfig.itbmp & 0xc0000)
		REG32_VAL(l_sensorconfig.itaddr) &= ~0xc0000;
	else if (l_sensorconfig.itbmp & 0x300000)
		REG32_VAL(l_sensorconfig.itaddr) &= ~0x300000;
	else if (l_sensorconfig.itbmp & 0xc00000)
		REG32_VAL(l_sensorconfig.itaddr) &= ~0xc00000;
#else /* WM3445_A0 */
	if (l_sensorconfig.itbmp & 0xff)
		REG32_VAL(l_sensorconfig.itaddr) &= ~0xff;
	else if (l_sensorconfig.itbmp & 0xff00)
		REG32_VAL(l_sensorconfig.itaddr) &= ~0xff00;
	else if (l_sensorconfig.itbmp & 0xff0000)
		REG32_VAL(l_sensorconfig.itaddr) &= ~0xff0000;
	else if (l_sensorconfig.itbmp & 0xff000000)
		REG32_VAL(l_sensorconfig.itaddr) &= ~0xff000000;
#endif /* WM3445_A0 */
	SET_GPIO_GSENSOR_INT();
	enable_irq(l_sensorconfig.irq);
#else /* GSENSOR_INT_MODE */
	schedule_delayed_work(&data->delayed_work, msecs_to_jiffies(2000));
#endif /*GSENSOR_INT_MODE */
	return;
}

static int mma_aot_open(struct inode *inode, struct file *file)
{
	int ret = -1;
	if (atomic_cmpxchg(&open_count, 0, 1) == 0) {
		if (atomic_cmpxchg(&open_flag, 0, 1) == 0) {
			atomic_set(&reserve_open_flag, 1);
			wake_up(&open_wq);
			ret = 0;
		}
	}
	return ret;
}

static int mma_aot_release(struct inode *inode, struct file *file)
{
	atomic_set(&reserve_open_flag, 0);
	atomic_set(&open_flag, 0);
	atomic_set(&open_count, 0);
	wake_up(&open_wq);
	return 0;
}

static int
mma_aot_ioctl(struct inode *inode, struct file *file,
	      unsigned int cmd, unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	short flag;

	switch (cmd) {
	case ECS_IOCTL_APP_SET_MFLAG:
	case ECS_IOCTL_APP_SET_AFLAG:
	case ECS_IOCTL_APP_SET_TFLAG:
	case ECS_IOCTL_APP_SET_MVFLAG:
		if (copy_from_user(&flag, argp, sizeof(flag)))
			return -EFAULT;
		if (flag < 0 || flag > 1)
			return -EINVAL;
		break;
	case ECS_IOCTL_APP_SET_DELAY:
		if (copy_from_user(&flag, argp, sizeof(flag)))
			return -EFAULT;
		break;
	default:
		break;
	}

	switch (cmd) {
	case ECS_IOCTL_APP_SET_MFLAG:
		atomic_set(&m_flag, flag);
		break;
	case ECS_IOCTL_APP_GET_MFLAG:
		flag = atomic_read(&m_flag);
		break;
	case ECS_IOCTL_APP_SET_AFLAG:
		atomic_set(&a_flag, flag);
		break;
	case ECS_IOCTL_APP_GET_AFLAG:
		flag = atomic_read(&a_flag);
		break;
	case ECS_IOCTL_APP_SET_TFLAG:
		atomic_set(&t_flag, flag);
		break;
	case ECS_IOCTL_APP_GET_TFLAG:
		flag = atomic_read(&t_flag);
		break;
	case ECS_IOCTL_APP_SET_MVFLAG:
		atomic_set(&mv_flag, flag);
		break;
	case ECS_IOCTL_APP_GET_MVFLAG:
		flag = atomic_read(&mv_flag);
		break;
	case ECS_IOCTL_APP_SET_DELAY:
		mmad_delay = flag;
		break;
	case ECS_IOCTL_APP_GET_DELAY:
		flag = mmad_delay;
		break;
	default:
		return -ENOTTY;
	}

	switch (cmd) {
	case ECS_IOCTL_APP_GET_MFLAG:
	case ECS_IOCTL_APP_GET_AFLAG:
	case ECS_IOCTL_APP_GET_TFLAG:
	case ECS_IOCTL_APP_GET_MVFLAG:
	case ECS_IOCTL_APP_GET_DELAY:
		if (copy_to_user(argp, &flag, sizeof(flag)))
			return -EFAULT;
		break;
	default:
		break;
	}

	return 0;
}

static int mma_pffd_open(struct inode *inode, struct file *file)
{
	int ret = -1;
	if (atomic_cmpxchg(&open_count, 0, 1) == 0) {
		if (atomic_cmpxchg(&open_flag, 0, 2) == 0) {
			atomic_set(&reserve_open_flag, 2);
			wake_up(&open_wq);
			ret = 0;
		}
	}
	return ret;
}

static int mma_pffd_release(struct inode *inode, struct file *file)
{
	atomic_set(&reserve_open_flag, 0);
	atomic_set(&open_flag, 0);
	atomic_set(&open_count, 0);
	wake_up(&open_wq);
	return 0;
}

static int
mma_pffd_ioctl(struct inode *inode, struct file *file,
	       unsigned int cmd, unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	short flag;
	int ret = 0;

	switch (cmd) {
	case ECS_IOCTL_APP_SET_DELAY:
		if (copy_from_user(&flag, argp, sizeof(flag)))
			return -EFAULT;
		break;
	default:
		break;
	}

	switch (cmd) {
	case ECS_IOCTL_APP_RESET_PEDOMETER:
		//ret = AKECS_Set_PERST();
		if (ret < 0)
			return ret;
		break;
	case ECS_IOCTL_APP_SET_DELAY:
		mmad_delay = flag;
		break;
	case ECS_IOCTL_APP_GET_DELAY:
		flag = mmad_delay;
		break;
	default:
		return -ENOTTY;
	}

	switch (cmd) {
	case ECS_IOCTL_APP_GET_DELAY:
		if (copy_to_user(argp, &flag, sizeof(flag)))
			return -EFAULT;
		break;
	default:
		break;
	}

	return 0;
}

static int mmad_open(struct inode *inode, struct file *file)
{
	return nonseekable_open(inode, file);
}

static int mmad_release(struct inode *inode, struct file *file)
{
	//AKECS_CloseDone();
	return 0;
}

static int
mmad_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
	   unsigned long arg)
{

	void __user *argp = (void __user *)arg;

	char msg[RBUFF_SIZE + 1], rwbuf[5], numfrq[2];
	int ret = -1, status;
	short mode, value[12], step_count, delay;
	char *pbuffer = 0;

	switch (cmd) {
	case ECS_IOCTL_READ:
	case ECS_IOCTL_WRITE:
		if (copy_from_user(&rwbuf, argp, sizeof(rwbuf)))
			return -EFAULT;
		break;
	case ECS_IOCTL_SET_MODE:
		if (copy_from_user(&mode, argp, sizeof(mode)))
			return -EFAULT;
		break;
	case ECS_IOCTL_SET_YPR:
		if (copy_from_user(&value, argp, sizeof(value)))
			return -EFAULT;
		break;
	case ECS_IOCTL_SET_STEP_CNT:
		if (copy_from_user(&step_count, argp, sizeof(step_count)))
			return -EFAULT;
		break;
	default:
		break;
	}

	switch (cmd) {
	case ECS_IOCTL_INIT:
		//ret = AKECS_Init();
		if (ret < 0)
			return ret;
		break;
	case ECS_IOCTL_RESET:
		//AKECS_Reset();
		break;
	case ECS_IOCTL_READ:
		if (rwbuf[0] < 1)
			return -EINVAL;
		//ret = AKI2C_RxData(&rwbuf[1], rwbuf[0]);
		if (ret < 0)
			return ret;
		break;
	case ECS_IOCTL_WRITE:
		if (rwbuf[0] < 2)
			return -EINVAL;
		//ret = AKI2C_TxData(&rwbuf[1], rwbuf[0]);
		if (ret < 0)
			return ret;
		break;
	case ECS_IOCTL_SET_MODE:
		//ret = AKECS_SetMode((char)mode);
		if (ret < 0)
			return ret;
		break;
	case ECS_IOCTL_GETDATA:
		//ret = AKECS_TransRBuff(msg, RBUFF_SIZE);
		if (ret < 0)
			return ret;
		break;
	case ECS_IOCTL_GET_NUMFRQ:
		numfrq[0] = cspec_num;
		numfrq[1] = atomic_read(&cspec_frq);
		break;
	case ECS_IOCTL_SET_PERST:
		//ret = AKECS_Set_PERST();
		if (ret < 0)
			return ret;
		break;
	case ECS_IOCTL_SET_G0RST:
		//ret = AKECS_Set_G0RST();
		if (ret < 0)
			return ret;
		break;
	case ECS_IOCTL_SET_YPR:
		//AKECS_Report_Value(value);
		break;
	case ECS_IOCTL_GET_OPEN_STATUS:
		//status = AKECS_GetOpenStatus();
		break;
	case ECS_IOCTL_GET_CLOSE_STATUS:
		//status = AKECS_GetCloseStatus();
		break;
	case ECS_IOCTL_SET_STEP_CNT:
		//AKECS_Report_StepCount(step_count);
		break;
	case ECS_IOCTL_GET_CALI_DATA:
		//pbuffer = get_mma_cal_ram();
		break;
	case ECS_IOCTL_GET_DELAY:
		delay = mmad_delay;
		break;
	default:
		return -ENOTTY;
	}

	switch (cmd) {
	case ECS_IOCTL_READ:
		if (copy_to_user(argp, &rwbuf, sizeof(rwbuf)))
			return -EFAULT;
		break;
	case ECS_IOCTL_GETDATA:
		if (copy_to_user(argp, &msg, sizeof(msg)))
			return -EFAULT;
		break;
	case ECS_IOCTL_GET_NUMFRQ:
		if (copy_to_user(argp, &numfrq, sizeof(numfrq)))
			return -EFAULT;
		break;
	case ECS_IOCTL_GET_OPEN_STATUS:
	case ECS_IOCTL_GET_CLOSE_STATUS:
		if (copy_to_user(argp, &status, sizeof(status)))
			return -EFAULT;
		break;
	case ECS_IOCTL_GET_CALI_DATA:
		if (copy_to_user(argp, pbuffer, MAX_CALI_SIZE))
			return -EFAULT;
		break;
	case ECS_IOCTL_GET_DELAY:
		if (copy_to_user(argp, &delay, sizeof(delay)))
			return -EFAULT;
		break;
	default:
		break;
	}

	return 0;
}

#define SHOW_AXIS_INFO(axis, val)
/*{ \
	switch (axis) \
	{ \
		case ABS_X: \
			printk(KERN_ALERT "X-axis:%d\n", val); \
			break; \
		case ABS_Y: \
			printk(KERN_ALERT "Y-axis:%d\n", val); \
			break; \
		case ABS_Z: \
			printk(KERN_ALERT "Z-axis:%d\n", val); \
			break; \
	}; \
}
*/

static int sensor_writeproc( struct file   *file,
                           const char    *buffer,
                           unsigned long count,
                           void          *data )
{
	char regval[1];
	char oldval;
	unsigned int sensitive_level = 0;
	struct mma7660_data * mma7660data = dev_get_drvdata(&this_pdev->dev);

	mutex_lock(&sense_data_mutex);
#ifdef GSENSOR_INT_MODE
	disable_irq(l_sensorconfig.irq);
#endif
	// get sensor level and set sensor level
	if (sscanf(buffer, "sensitive=%d\n", &sensitive_level)) {
		if ((sensitive_level >= 0) && (sensitive_level <= 7)) {
			l_sensorconfig.sensitive = 7 - sensitive_level;
			sensor_i2c_read(MMA7660_ADDR,7,regval,1);
			regval[0] &= 0xFE;
			oldval = regval[0];
			sensor_i2c_write(MMA7660_ADDR,7,regval,1); // standard mode
			sensor_i2c_read(MMA7660_ADDR,8,regval,1);
			regval[0] = (regval[0] & 0xF8) | l_sensorconfig.sensitive;
			sensor_i2c_write(MMA7660_ADDR,8,regval,1);
			oldval |= BIT0; // active mode
			sensor_i2c_write(MMA7660_ADDR,7,&oldval,1);
		}
	} else if (sscanf(buffer, "level=%d\n", &l_sensorconfig.sensorlevel)) {
		// write sensor reg
		sensor_i2c_read(MMA7660_ADDR,7,regval,1);
		oldval = regval[0];
		regval[0] &= 0xFE;
		sensor_i2c_write(MMA7660_ADDR,7,regval,1); // standard mode
		switch (l_sensorconfig.sensorlevel)
		{
			case SENSOR_UI_MODE: // UI mode
				regval[0] = (1 == l_sensorconfig.shake_enable) ? 0xe3:0x03;
				//sensor_i2c_read(MMA7660_ADDR,6,regval,1);
				//regval[0] &= 0xEF;
				sensor_i2c_write(MMA7660_ADDR,6, regval,1);
				regval[0]=UI_SAMPLE_RATE;
				sensor_i2c_write(MMA7660_ADDR,8, regval,1);
				break;
			case SENSOR_GRAVITYGAME_MODE: // grative game mode, no shake and other interrupt
				regval[0]=0x10; // report every measurement
				sensor_i2c_write(MMA7660_ADDR,6, regval,1);
				regval[0]=0xFB;
				sensor_i2c_write(MMA7660_ADDR,8, regval,1);
				break;
		};
		oldval |= BIT0; // active mode
		sensor_i2c_write(MMA7660_ADDR,7,&oldval,1);
	} else if (sscanf(buffer, "shakenable=%d\n", &l_sensorconfig.shake_enable)) {
		sensor_i2c_read(MMA7660_ADDR,7,regval,1);
		oldval = regval[0];
		regval[0] &= 0xFE;
		sensor_i2c_write(MMA7660_ADDR,7,regval,1); // standard mode
		sensor_i2c_read(MMA7660_ADDR,6,regval,1);
		switch (l_sensorconfig.shake_enable)
		{
			case 0: // disable shake
				regval[0] &= 0x1F;
				sensor_i2c_write(MMA7660_ADDR,6, regval,1);
				gsensor_dbg("Shake disable!!\n");
				break;
			case 1: // enable shake
				regval[0] |= 0xE0;
				sensor_i2c_write(MMA7660_ADDR,6, regval,1);
				gsensor_dbg("Shake enable!!\n");
				break;
		};
		sensor_i2c_write(MMA7660_ADDR,7,&oldval,1);
	} else if (sscanf(buffer, "rotation=%d\n", &l_sensorconfig.manual_rotation)) {
		switch (l_sensorconfig.manual_rotation)
		{
			case 90:
				// portrait
				input_report_abs(mma7660data->input_dev, ABS_X, cos30_1000);
				input_report_abs(mma7660data->input_dev, ABS_Y, 0);
				input_report_abs(mma7660data->input_dev, ABS_Z, sin30_1000);
				input_sync(mma7660data->input_dev);
				break;
			case 0:
				// landscape
				input_report_abs(mma7660data->input_dev, ABS_X, 0);
				input_report_abs(mma7660data->input_dev, ABS_Y, cos30_1000);
				input_report_abs(mma7660data->input_dev, ABS_Z, sin30_1000);
				input_sync(mma7660data->input_dev);
				break;
		};
	}
#ifdef GSENSOR_INT_MODE
	enable_irq(l_sensorconfig.irq);
#endif
	mutex_unlock(&sense_data_mutex);
	return count;
}

static int sensor_readproc(char *page, char **start, off_t off,
			  int count, int *eof, void *data)
{
	int len = 0;

	len = sprintf(page, "level=%d\nshakenable=%d\nrotation=%d\n",
				l_sensorconfig.sensorlevel,
				l_sensorconfig.shake_enable,
				l_sensorconfig.manual_rotation);
	return len;
}



static int mma7660_init_client(struct platform_device *pdev)
{
	struct mma7660_data *data;
#ifdef GSENSOR_INT_MODE
	int ret;
#endif

	data = dev_get_drvdata(&pdev->dev);
	mutex_init(&sense_data_mutex);

#ifdef WM3445_A0  // Patch GPIO interrupt mask for WM3445 A0
#ifdef GSENSOR_INT_MODE
	gsensor_int_ctrl(DISABLE);
#else
	REG32_VAL(GPIO_BASE_ADDR + 0x308) |= 0x100;
	REG32_VAL(GPIO_BASE_ADDR + 0x40) |= 0x100;
	REG32_VAL(GPIO_BASE_ADDR + 0x80) &= ~0x100;
	REG32_VAL(GPIO_BASE_ADDR + 0x480) |= 0x100;
	REG32_VAL(GPIO_BASE_ADDR + 0x4c0) &= ~0x100;
#endif
#endif

#ifdef GSENSOR_INT_MODE
	ret = request_irq(l_sensorconfig.irq, mma7660_interrupt, IRQF_SHARED,//IRQF_DISABLED|IRQF_SHARED,
			  "mma7660", data);
	if (ret < 0) {
		printk(KERN_ERR "mma7660_init_client: request irq failed\n");
		goto err;
	}
#endif
	l_sensorconfig.sensor_proc = create_proc_entry(GSENSOR_PROC_NAME, 0666, NULL/*&proc_root*/);
	if (l_sensorconfig.sensor_proc != NULL) {
		l_sensorconfig.sensor_proc->write_proc = sensor_writeproc;
		l_sensorconfig.sensor_proc->read_proc = sensor_readproc;
	}

	init_waitqueue_head(&data_ready_wq);
	init_waitqueue_head(&open_wq);

	/* As default, report all information */
	atomic_set(&m_flag, 1);
	atomic_set(&a_flag, 1);
	atomic_set(&t_flag, 1);
	atomic_set(&mv_flag, 1);

	return 0;

//err_free_irq:
//	free_irq(l_sensorconfig.irq, 0);
//err_alloc_data_failed:
#ifdef GSENSOR_INT_MODE
err:
	return ret;
#endif
}

static struct file_operations mmad_fops = {
	.owner = THIS_MODULE,
	.open = mmad_open,
	.release = mmad_release,
	.ioctl = mmad_ioctl,
};

static struct file_operations mma_aot_fops = {
	.owner = THIS_MODULE,
	.open = mma_aot_open,
	.release = mma_aot_release,
	.ioctl = mma_aot_ioctl,
};

static struct file_operations mma_pffd_fops = {
	.owner = THIS_MODULE,
	.open = mma_pffd_open,
	.release = mma_pffd_release,
	.ioctl = mma_pffd_ioctl,
};

static struct miscdevice mma_aot_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "mma7660_aot",
	.fops = &mma_aot_fops,
};

static struct miscdevice mma_pffd_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "mma7660_pffd",
	.fops = &mma_pffd_fops,
};

static struct miscdevice mmad_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "mma7660_daemon",
	.fops = &mmad_fops,
};

static int mma7660_probe(
	struct platform_device *pdev)
{
	struct mma7660_data *mma;
	int err;
//	char rxData[8];
//	char rxData1[8];
//	char txData[2] = {0xf,0};
//	char txData1[2] = {0xff,0};

	l_sensorconfig.sensorlevel = SENSOR_GRAVITYGAME_MODE;//SENSOR_UI_MODE;

	/*
	sensor_i2c_write(MMA7660_ADDR,5,txData,1);
	sensor_i2c_write(MMA7660_ADDR,6,txData1,1);

	sensor_i2c_read(MMA7660_ADDR,5,rxData,1);
	sensor_i2c_read(MMA7660_ADDR,6,rxData1,1);
	*/

#if 1
	mma = kzalloc(sizeof(struct mma7660_data), GFP_KERNEL);
	if (!mma) {
		err = -ENOMEM;
		goto exit_alloc_data_failed;
	}
	x_count = kzalloc(l_sensorconfig.avg_count*sizeof(int), GFP_KERNEL);
	y_count = kzalloc(l_sensorconfig.avg_count*sizeof(int), GFP_KERNEL);
	z_count = kzalloc(l_sensorconfig.avg_count*sizeof(int), GFP_KERNEL);

#ifdef GSENSOR_INT_MODE
	INIT_WORK(&mma->work, mma_work_func);
#else
	INIT_DELAYED_WORK(&mma->delayed_work, mma_work_func);
#endif

	dev_set_drvdata(&pdev->dev,mma);
	mma7660_init_client(pdev);
	this_pdev = pdev;

	mma->input_dev = input_allocate_device();

	if (!mma->input_dev) {
		err = -ENOMEM;
		printk(KERN_ERR
		       "mma7660_probe: Failed to allocate input device\n");
		goto exit_input_dev_alloc_failed;
	}
	set_bit(EV_KEY, mma->input_dev->evbit);
	set_bit(KEY_NEXTSONG, mma->input_dev->keybit);

	set_bit(EV_ABS, mma->input_dev->evbit);
	/* yaw */
	input_set_abs_params(mma->input_dev, ABS_RX, 0, 360*100, 0, 0);
	/* pitch */
	input_set_abs_params(mma->input_dev, ABS_RY, -180*100, 180*100, 0, 0);
	/* roll */
	input_set_abs_params(mma->input_dev, ABS_RZ, -90*100, 90*100, 0, 0);
	/* x-axis acceleration */
	input_set_abs_params(mma->input_dev, ABS_X, -1872, 1872, 0, 0);
	/* y-axis acceleration */
	input_set_abs_params(mma->input_dev, ABS_Y, -1872, 1872, 0, 0);
	/* z-axis acceleration */
	input_set_abs_params(mma->input_dev, ABS_Z, -1872, 1872, 0, 0);
	/* temparature */
	input_set_abs_params(mma->input_dev, ABS_THROTTLE, -30, 85, 0, 0);
	/* status of magnetic sensor */
	input_set_abs_params(mma->input_dev, ABS_RUDDER, -32768, 3, 0, 0);
	/* status of acceleration sensor */
	input_set_abs_params(mma->input_dev, ABS_WHEEL, -32768, 3, 0, 0);
	/* step count */
	input_set_abs_params(mma->input_dev, ABS_GAS, 0, 65535, 0, 0);
	/* x-axis of raw magnetic vector */
	input_set_abs_params(mma->input_dev, ABS_HAT0X, -2048, 2032, 0, 0);
	/* y-axis of raw magnetic vector */
	input_set_abs_params(mma->input_dev, ABS_HAT0Y, -2048, 2032, 0, 0);
	/* z-axis of raw magnetic vector */
	input_set_abs_params(mma->input_dev, ABS_BRAKE, -2048, 2032, 0, 0);

	mma->input_dev->name = "g-sensor";

	err = input_register_device(mma->input_dev);

	if (err) {
		printk(KERN_ERR
		       "mma7660_probe: Unable to register input device: %s\n",
		       mma->input_dev->name);
		goto exit_input_register_device_failed;
	}

	err = misc_register(&mmad_device);
	if (err) {
		printk(KERN_ERR
		       "mma7660_probe: mmad_device register failed\n");
		goto exit_misc_device_register_failed;
	}

	err = misc_register(&mma_aot_device);
	if (err) {
		printk(KERN_ERR
		       "mma7660_probe: mma_aot_device register failed\n");
		goto exit_misc_device_register_failed;
	}

	err = misc_register(&mma_pffd_device);
	if (err) {
		printk(KERN_ERR
		       "mma7660_probe: mma_pffd_device register failed\n");
		goto exit_misc_device_register_failed;
	}

	//err = device_create_file(&pdev->dev, &dev_attr_ms1);
	//err = device_create_file(&pdev->dev, &dev_attr_ms2);
	//err = device_create_file(&pdev->dev, &dev_attr_ms3);

#ifdef CONFIG_HAS_EARLYSUSPEND
        mma->gsensor_earlysuspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
        mma->gsensor_earlysuspend.suspend = gsensor_early_suspend;
        mma->gsensor_earlysuspend.resume = gsensor_late_resume;
        register_early_suspend(&mma->gsensor_earlysuspend);
#endif

	gsensor_sysfs_init();
	mma7660_chip_init(mma);
#endif
	return 0;

exit_misc_device_register_failed:
exit_input_register_device_failed:
	input_free_device(mma->input_dev);
exit_input_dev_alloc_failed:
	kfree(mma);
exit_alloc_data_failed:
//exit_check_functionality_failed:
	return err;
}

static int mma7660_remove(struct platform_device *pdev)
{
	struct mma7660_data *mma = dev_get_drvdata(&pdev->dev);
#ifdef GSENSOR_INT_MODE
	free_irq(l_sensorconfig.irq, mma);
#endif
	input_unregister_device(mma->input_dev);
	kfree(mma);
	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void gsensor_early_suspend(struct early_suspend *h)
{
#ifndef GSENSOR_INT_MODE
	int ret = 0;
	unsigned int now_time = 0;
	unsigned int delay_time = 0;
	struct mma7660_data *data;
#endif

	l_sensorconfig.suspend = 1;

#ifdef GSENSOR_INT_MODE
	gsensor_int_ctrl(DISABLE);
#else
	data = container_of(h, struct mma7660_data, gsensor_earlysuspend);

	ret = cancel_delayed_work(&data->delayed_work);
	if (ret == 0)
		printk(KERN_DEBUG "G-Sensor work is running\n");
	flush_scheduled_work();
	now_time = wmt_read_oscr();
	while (!l_sensorconfig.work_finish) {
		delay_time = wmt_read_oscr() - now_time;
		if (delay_time > 60000) {//20ms
			printk(KERN_WARNING "G-Sensor Timeout!\n");
			break;
		}
	}
#endif
}

static void gsensor_late_resume(struct early_suspend *h)
{
	struct mma7660_data *data;

	l_sensorconfig.suspend = 0;
	data = container_of(h, struct mma7660_data, gsensor_earlysuspend);
	mma7660_chip_init(data);
}
#endif

static int mma7660_suspend(struct platform_device *pdev, pm_message_t state)
{
	char txData[1];
	atomic_set(&suspend_flag, 1);
	if (atomic_read(&open_flag) == 2) {
	     txData[0] = 0;
	     sensor_i2c_write(MMA7660_ADDR,7,txData,1);
	}
	atomic_set(&reserve_open_flag, atomic_read(&open_flag));
	atomic_set(&open_flag, 0);
	wake_up(&open_wq);
	return 0;
}

static int mma7660_resume(struct platform_device *pdev)
{
	/*
	char txData[1];
	if (atomic_read(&open_flag) == 2) {
	    txData[0] = 0xf9;
	    sensor_i2c_write(MMA7660_ADDR,7,txData,1);
	}
	*/
	atomic_set(&suspend_flag, 0);
	atomic_set(&open_flag, atomic_read(&reserve_open_flag));
	wake_up(&open_wq);
	return 0;
}

static struct platform_device mma7660_device = {
    .name           = "mma7660",
    .id             = 0,
};

static struct platform_driver mma7660_driver = {
	.probe = mma7660_probe,
	.remove = mma7660_remove,
	.suspend	= mma7660_suspend,
	.resume		= mma7660_resume,
	.driver = {
		.name = "mma7660",
		},
};

/*
 * Get the configure of sensor from u-boot.
 * Return: 1--success, 0--error.
 */
static int get_axisset(void)
{
	char varbuf[64];
	int n;
	int varlen;

	memset(varbuf, 0, sizeof(varbuf));
	varlen = sizeof(varbuf);
	if (wmt_getsyspara("wmt.io.gsensor", varbuf, &varlen)) {
		printk(KERN_DEBUG "KXTE9: wmt.io.gsensor not defined!\n");
		return 0;
	} else {
		n = sscanf(varbuf, "%d:%d:%d:%d:%d:%d:%d:%d:%d",
				&l_sensorconfig.op,
				&l_sensorconfig.samp,
				&(l_sensorconfig.xyz_axis[0][0]),
				&(l_sensorconfig.xyz_axis[0][1]),
				&(l_sensorconfig.xyz_axis[1][0]),
				&(l_sensorconfig.xyz_axis[1][1]),
				&(l_sensorconfig.xyz_axis[2][0]),
				&(l_sensorconfig.xyz_axis[2][1]),
				&l_sensorconfig.avg_count);
		if (n != 9) {
			gsensor_dbg("MMA7660FC: wmt.io.gsensor format is incorrect!\n");
			return 0;
		}
	}
	if (l_sensorconfig.op != 2)
		return 0;
	printk(KERN_INFO "MMA7660FC G-Sensor driver init\n");			
	gsensor_dbg("AVG Count: %d\n", l_sensorconfig.avg_count);
	if (l_sensorconfig.samp > 120) {
		printk(KERN_ERR "Sample Rate can't be greater than 120 !\n");
		l_sensorconfig.samp = 120;
	}
	if (l_sensorconfig.avg_count == 0)
		l_sensorconfig.avg_count = 1;
	gsensor_dbg("wmt.io.gsensor = %d:%d:%d:%d:%d:%d:%d:%d:%d\n",
		l_sensorconfig.op,
		l_sensorconfig.samp,
		l_sensorconfig.xyz_axis[0][0],
		l_sensorconfig.xyz_axis[0][1],
		l_sensorconfig.xyz_axis[1][0],
		l_sensorconfig.xyz_axis[1][1],
		l_sensorconfig.xyz_axis[2][0],
		l_sensorconfig.xyz_axis[2][1],
		l_sensorconfig.avg_count);
#ifdef GSENSOR_INT_MODE
	printk(KERN_INFO "G-Sensor Sample Rate: %d sample/sec\n", l_sensorconfig.samp);
#else
	if ((1000000/l_sensorconfig.samp)%1000)
		l_sensorconfig.interval = 1000/l_sensorconfig.samp + 1;
	else
		l_sensorconfig.interval = 1000/l_sensorconfig.samp;
	printk(KERN_INFO "G-Sensor Time Interval: %d ms\n", l_sensorconfig.interval);
#endif
	return 1;
}

static int __init mma7660_init(void)
{
    int ret = 0;

	ret = get_axisset(); // get gsensor config from u-boot

	if (!ret)
		return -EINVAL;

#ifdef GSENSOR_INT_MODE
	get_gpioset(); // get gsensor's gpio config from u-boot
#endif

    ret = platform_device_register(&mma7660_device);
    if (ret)
        return ret;

	return platform_driver_register(&mma7660_driver);
}

static void __exit mma7660_exit(void)
{
	if (l_sensorconfig.sensor_proc != NULL) {
		remove_proc_entry(GSENSOR_PROC_NAME, NULL);
		l_sensorconfig.sensor_proc = NULL;
	}
    platform_driver_unregister(&mma7660_driver);
    platform_device_unregister(&mma7660_device);
}

module_init(mma7660_init);
module_exit(mma7660_exit);
MODULE_LICENSE("GPL");
