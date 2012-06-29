#include <mach/hardware.h>
#include <linux/i2c.h>

struct i2c_slave_dev {
	unsigned short addr;
	char * dev_name;
	void (*callback)(void *data);
	struct i2c_msg *msg;
};

struct wmt_i2cbusfifo {
	struct i2c_msg *msg;
	int msg_num;
	struct list_head busfifohead;
	int non_block;/*1:non-block, 0: block*/
	int xfer_length;
	int xfer_msgnum;
	int restart;
	void (*callback)(void *data);
	void *data;
};


struct i2c_algo_wmt_data {
	int  (*write_msg)(unsigned int slave_addr, char *buf, unsigned int length , int restart, int last) ;
	int  (*read_msg)(unsigned int slave_addr, char *buf, unsigned int length , int restart, int last) ;
	int  (*send_request)(struct i2c_msg *msg, int msg_num, int non_block, void (*callbck)(void *data), void *data);
#ifdef CONFIG_SND_SOC_VT1603
	int (*vt1603_write_for_read)(unsigned int slave_addr, char *buf, unsigned int length , int restart, int last);
#endif
	int  (*wait_bus_not_busy) (void);
	void (*reset) (void);
	void (*set_mode)(enum i2c_mode_e) ;
	int  udelay;
	int  timeout;
};

extern int wmt_i2c_transfer(struct i2c_msg* msgs, int msg_num, int bus_id, void (*callback)(void *data), void *data);

