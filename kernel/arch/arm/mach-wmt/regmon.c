/*++
linux/arch/arm/mach-wmt/regmon.c

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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/ioport.h>

#include <asm/uaccess.h>
#include <mach/hardware.h>

#define NO_PARENT       NULL

#define MOD_NAME       "regmon"
#define SOC_NAME        "wmt"
#define REG_NAME        "registers"

#define RBUF_SIZE       128             /* 128 bytes should be enough */

struct soc_regs_s {
	u32 addr;
       char *name;
       char *desc;
       /*u16 low_ino;*/
       unsigned int low_ino;
       struct proc_dir_entry *entry;
};

static struct soc_regs_s wmt_regs[] =
{
	/* addr, name, desc */

	/*
	 * GPIO
	 */
	{ GPIO_CTRL_I2C_ADDR, "GPIO_0000", "GPIO Enable Control Register for I2C" },
	{ GPIO_CTRL_SF_ADDR, "GPIO_0004", "GPIO Enable Control Register for SF" },
	{ GPIO_CTRL_KPAD_ADDR, "GPIO_0008", "GPIO Enable Control Register for KPAD" },
	{ GPIO_CTRL_I2S_ADDR, "GPIO_000C", "GPIO Enable Control Register for I2S" },
	{ GPIO_CTRL_LCD_ADDR, "GPIO_0010", "GPIO Enable Control for LCD" },
	{ GPIO_CTRL_PWM_ADDR, "GPIO_0014", "GPIO Eanble Control for PWM" },
	{ GPIO_CTRL_CF_ADDR, "GPIO_0018", "GPIO Eanble Control for CF" },
	{ GPIO_CTRL_SPI_ADDR, "GPIO_001C", "GPIO Eanble Control for SPI" },
	{ GPIO_CTRL_CDET_ADDR, "GPIO_0020", "GPIO Eanble Control for CDET" },
	{ GPIO_CTRL_GPIO_ADDR, "GPIO_0024", "GPIO Eanble Control for GPIO" },
	{ GPIO_CTRL_NF_ADDR, "GPIO_0028", "GPIO Eanble Control for NF" },
	{ GPIO_CTRL_UART_ADDR, "GPIO_002C", "GPIO Eanble Control for UART" },

	{ GPIO_OC_I2C_ADDR, "GPIO_0030", "GPIO Output Control Register for I2C" },
	{ GPIO_OC_SF_ADDR, "GPIO_0034", "GPIO Output Control Register for SF" },
	{ GPIO_OC_KPAD_ADDR, "GPIO_0038", "GPIO Output Control Register for KPAD" },
	{ GPIO_OC_I2S_ADDR, "GPIO_003C", "GPIO Output Control Register for I2S" },
	{ GPIO_OC_LCD_ADDR, "GPIO_0040", "GPIO Output Control Register for LCD" },
	{ GPIO_OC_PWM_ADDR, "GPIO_0044", "GPIO Output Control Register for PWM" },
	{ GPIO_OC_CF_ADDR, "GPIO_0048", "GPIO Output Control Register for CF" },
	{ GPIO_OC_SPI_ADDR, "GPIO_004C", "GPIO Output Control Register for SPI" },
	{ GPIO_OC_CDET_ADDR, "GPIO_0050", "GPIO Output Control Register for CDET" },
	{ GPIO_OC_GPIO_ADDR, "GPIO_0054", "GPIO Output Control Register for GPIO" },
	{ GPIO_OC_NF_ADDR, "GPIO_0058", "GPIO Output Control Register for NF" },
	{ GPIO_OC_UART_ADDR, "GPIO_005C", "GPIO Output Control Register for UART " },

	{ GPIO_OD_I2C_ADDR, "GPIO_0060", "GPIO Output Data Register for I2C" },
	{ GPIO_OD_SF_ADDR, "GPIO_0064", "GPIO Output Data Register for SF" },
	{ GPIO_OD_KPAD_ADDR, "GPIO_0068", "GPIO Output Data Register for KPAD" },
	{ GPIO_OD_I2S_ADDR, "GPIO_006C", "GPIO Output Data Register for I2S" },
	{ GPIO_OD_LCD_ADDR, "GPIO_0070", "GPIO Output Data Register for LCD" },
	{ GPIO_OD_PWM_ADDR, "GPIO_0074", "GPIO Output Data Register for PWM" },
	{ GPIO_OD_CF_ADDR, "GPIO_0078", "GPIO Output Data Register for CF" },
	{ GPIO_OD_SPI_ADDR, "GPIO_007C", "GPIO Output Data Register for SPI" },
	{ GPIO_OD_CDET_ADDR, "GPIO_0080", "GPIO Output Data Register for CDET" },
	{ GPIO_OD_GPIO_ADDR, "GPIO_0084", "GPIO Output Data Register for GPIO" },
	{ GPIO_OD_NF_ADDR, "GPIO_0088", "GPIO Output Data Register for NF" },
	{ GPIO_OD_UART_ADDR, "GPIO_008C", "GPIO Output Data Register for UART" },

	{ GPIO_ID_I2C_ADDR, "GPIO_0090", "GPIO Input Data Register for I2C" },
	{ GPIO_ID_SF_ADDR, "GPIO_0094", "GPIO Input Data Register for SF" },
	{ GPIO_ID_KPAD_ADDR, "GPIO_0098", "GPIO Input Data Register for KPAD" },
	{ GPIO_ID_I2S_ADDR, "GPIO_009C", "GPIO Input Data Register for I2S" },
	{ GPIO_ID_LCD_ADDR, "GPIO_00A0", "GPIO Input Data Register for LCD" },
	{ GPIO_ID_PWM_ADDR, "GPIO_00A4", "GPIO Input Data Register for PWM" },
	{ GPIO_ID_CF_ADDR, "GPIO_00A8", "GPIO Input Data Register for CF" },
	{ GPIO_ID_SPI_ADDR, "GPIO_00AC", "GPIO Input Data Register for SPI" },
	{ GPIO_ID_CDET_ADDR, "GPIO_00B0", "GPIO Input Data Register for CDET" },
	{ GPIO_ID_GPIO_ADDR, "GPIO_00B4", "GPIO Input Data Register for GPIO" },
	{ GPIO_ID_NF_ADDR, "GPIO_00B8", "GPIO Input Data Register for NF" },
	{ GPIO_ID_UART_ADDR, "GPIO_00BC", "GPIO Input Data Register for UART" },

	{ GPIO_STRAP_STATUS_ADDR, "GPIO_0100", "GPIO Strapping Status Register" },
	{ GPIO_AHB_CTRL_ADDR, "GPIO_0108", "GPIO AHB Control Register" },
	{ GPIO_BINDING_STATUS_ADDR, "GPIO_0110", "Binding Option Status Register" },

	{ GPIO_PIN_SELECT_ADDR, "GPIO_0200", "GPIO Pin Selection Register" },
	{ GPIO_PLL_CFG_ADDR, "GPIO_0204", "PLL configure register" },
	
	{ GPIO_INT_REQ_TYPE_R0_ADDR, "GPIO_0300", "GPIO Interrupt Request Type Register0" },
	{ GPIO_INT_REQ_TYPE_R1_ADDR, "GPIO_0304", "GPIO Interrupt Request Type Register1" },
	{ GPIO_INT_REQ_TYPE_R2_ADDR, "GPIO_0308", "GPIO Interrupt Request Type Register2" },
	{ GPIO_INT_REQ_TYPE_R3_ADDR, "GPIO_030C", "GPIO Interrupt Request Type Register3" },
	{ GPIO_INT_REQ_TYPE_R4_ADDR, "GPIO_0310", "GPIO Interrupt Request Type Register4" },
	{ GPIO_INT_REQ_TYPE_R5_ADDR, "GPIO_0314", "GPIO Interrupt Request Type Register5" },
	{ GPIO_INT_REQ_STS_ADDR, "GPIO_0320", "GPIO Interrupt Request Status Register" },

	{ GPIO_XD_IO_DRV_ADDR, "GPIO_0400", "XD CLOCK I/O Drive Strength Register" },
	{ GPIO_SD_IO_DRV_ADDR, "GPIO_0404", "Secure Digital I/O Drive Strength and Slew Rate Register" },
	{ GPIO_LCD_IO_DRV_ADDR, "GPIO_0408", "LCD CLOCK I/O Drive Strength Register" },
	{ GPIO_SF_IO_DRV_ADDR, "GPIO_040C", "SF CLOCK I/O Drive Strength Register" },
	{ GPIO_CF_IO_DRV_ADDR, "GPIO_0410", "CF CLOCK I/O Drive Strength Register" },
	{ GPIO_NAND_IO_DRV_ADDR, "GPIO_0414", "NAND CLOCK I/O Drive Strength Register" },
	{ GPIO_SPI_IO_DRV_ADDR, "GPIO_0418", "SPI CLOCK I/O Drive Strength Register" },

	{ GPIO_CTRL_ENET_ADDR, "GPIO_0500", "GPIO Enable Control Register for ENET" },
	{ GPIO_OC_ENET_ADDR, "GPIO_0504", "GPIO Output Control Register for ENET" },
	{ GPIO_OD_ENET_ADDR, "GPIO_0508", "GPIO Output Data Register for ENET" },
	{ GPIO_ID_ENET_ADDR, "GPIO_050C", "GPIO Input Data Register for ENET" },
	{ GPIO_SD1_PWR_SW_ADDR, "GPIO_0510", "Enable SD1 power switch" },

	{ GPIO_PAD_I2C_ADDR, "GPIO_0600", "PAD pull up/down Control Register for I2C" },
	{ GPIO_PAD_SF_ADDR, "GPIO_0604", "PAD pull up/down Control Register for SF" },
	{ GPIO_PAD_KPAD_ADDR, "GPIO_0608", "PAD pull up/down Control Register for KPAD" },
	{ GPIO_PAD_I2S_ADDR, "GPIO_060C", "PAD pull up/down Control Register for I2S" },
	{ GPIO_PAD_LCD_ADDR, "GPIO_0610", "PAD pull up/down Control for LCD" },
	{ GPIO_PAD_LCD_EN_ADDR, "GPIO_0614", "PAD pull up/down Enable Control for LCD" },
	{ GPIO_PAD_PWM_ADDR, "GPIO_0618", "PAD pull up/down Control for PWM" },
	{ GPIO_PAD_CF_ADDR, "GPIO_061C", "PAD pull up/down Control for CF" },
	{ GPIO_PAD_CF_EN_ADDR, "GPIO_0620", "PAD pull up/down Enable Control for CF" },

	{ GPIO_PAD_SPI_ADDR, "GPIO_0624", "PAD pull up/down Control for SPI" },
	{ GPIO_PAD_NF_ADDR, "GPIO_0628", "PAD pull up/down Control for NF" },
	{ GPIO_PAD_NF_EN_ADDR, "GPIO_062C", "PAD pull up/down enable Control for NF" },
	{ GPIO_PAD_UART_ADDR, "GPIO_0630", "PAD pull up/down Control for UART" },
	{ GPIO_PAD_ENET_ADDR, "GPIO_0634", "PAD pull up/down Control for ENET" },
	{ GPIO_PAD_ENET_EN_ADDR, "GPIO_0638", "PAD pull up/down enable  Control for ENET" },
	{ GPIO_PAD_JTAG_ADDR, "GPIO_063C", "PAD pull up/down Control for JTAG" },
	{ GPIO_PAD_GPIO_ADDR, "GPIO_0640", "PAD pull up/down Control for GPIO" },
	{ GPIO_PAD_SUSPEND_ADDR, "GPIO_0644", "PAD pull up/down Control for SUSPEND" },
	{ GPIO_PAD_SUSPEND_EN_ADDR, "GPIO_0648", "PAD pull up/down Enable Control for SUSPEND" },
	

	/* Add more registers depend on you */
	/* ..... */
};

#define NR_wmt_REGS    (sizeof(wmt_regs)/sizeof(struct soc_regs_s))

static int proc_read_reg(struct file *file, char *buf,
		size_t nbytes, loff_t *ppos)
{
	char output[RBUF_SIZE];
	int i, count = 0;
	int i_ino = (file->f_dentry->d_inode)->i_ino;

	struct soc_regs_s *p = NULL;

	if (*ppos > 0)        /* Assume reading completed in previous read */
		return 0;

	/*
	* Get the register entry.
	*/
	for (i = 0; i < NR_wmt_REGS; i++) {
		if (wmt_regs[i].low_ino == i_ino) {
			p = &wmt_regs[i];
			break;
		}
	}

	if (p == NULL)
		return -EINVAL;

	count += sprintf(output, "0x%.8x\n", REG32_VAL(p->addr));

#ifdef CONFIG_WMT_REGMON_RF_B
	count += sprintf((output + count), "%s\n", p->desc);
#endif

	*ppos += count;

	if (count > nbytes)   /* Assume output can be read at one time */
		return -EINVAL;

	if (copy_to_user(buf, output, count))
		return -EFAULT;

	return count;
}

#ifdef CONFIG_WMT_REGMON_WF
/*
 * Write interface
 */
static ssize_t proc_write_reg(struct file *file, const char *buffer,
		size_t count, loff_t *ppos)
{
	char *endp;
	int i;
	unsigned long new;
	int i_ino = (file->f_dentry->d_inode)->i_ino;
	ssize_t result;
	struct soc_regs_s *p = NULL;

	/*
	* Get the register entry.
	*/
	for (i = 0; i < NR_wmt_REGS; i++) {
		if (wmt_regs[i].low_ino == i_ino) {
			p = &wmt_regs[i];
			break;
		}
	}

	if (p == NULL)
		return -EINVAL;

	/*
	* Convert input to new value;
	*/
	new = simple_strtoul(buffer, &endp, 0);

	/*
	* Now assign new value to register.
	*/
	REG32_VAL(p->addr) = new;
	result = count + endp - buffer;
	return result;
}

/*
 * Provide both read and write interfaces
 */
static struct file_operations reg_fops = {
	.read   = proc_read_reg,
	.write  = proc_write_reg,
};
#else
/*
 * Provide only read interface
 */
static struct file_operations reg_fops = {
	.read   = proc_read_reg,
};
#endif

static struct proc_dir_entry *proc_soc;
static struct proc_dir_entry *proc_reg;

static int __init monitor_init(void)
{
	int i, err = 0;
	struct soc_regs_s *p = NULL;

	/*
	* Make /proc/wmt directory.
	*/
	proc_soc = proc_mkdir("wmt", &proc_root);

	if (proc_soc == NULL) {
		err = -ENOMEM;
		printk(KERN_ERR MOD_NAME ": can't create /proc/" SOC_NAME "\n");
		goto monitor_err;
	}

	proc_soc->owner = THIS_MODULE;

	/*
	* Make /proc/wmt/registers directory.
	*/
	proc_reg = proc_mkdir(REG_NAME, proc_soc);

	if (proc_reg == NULL) {
		err = -ENOMEM;
		printk(KERN_ERR MOD_NAME ": can't create /proc/" SOC_NAME \
			"/" REG_NAME "\n");
		goto monitor_err;
	}

	/*
	* Make /proc/wmt/registers/XXXX entries.
	*/
	for (i = 0, p = &wmt_regs[0]; i < NR_wmt_REGS; i++, p++) {
		p->entry = NULL;
		p->entry = create_proc_entry(p->name,
			S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH,
			proc_reg);

		if (p->entry == NULL) {
			err = -ENOMEM;
			printk(KERN_ERR MOD_NAME \
			": can't create /proc/" SOC_NAME \
			"/" REG_NAME "/%s\n", p->name);
			goto monitor_err;
		}

		p->low_ino = p->entry->low_ino;
		p->entry->proc_fops = &reg_fops;
	}

	return err;

monitor_err:
	/*
	* Free all allocated entries if we got error.
	*/
	for (i = 0, p = &wmt_regs[0]; i < NR_wmt_REGS; i++, p++) {
		if (p->entry != NULL) {
			remove_proc_entry(p->name, proc_reg);
			p->entry = NULL;
		}
	}

	remove_proc_entry(REG_NAME, proc_soc);
	remove_proc_entry(SOC_NAME, &proc_root);

	proc_reg = NULL;
	proc_soc = NULL;

	return err;
}

static void __exit monitor_exit(void)
{
	int i;
	struct soc_regs_s *p = NULL;

	/*
	* Free all allocated entries if we got error.
	*/
	for (i = 0, p = &wmt_regs[0]; i < NR_wmt_REGS; i++, p++) {
		if (p->entry != NULL) {
			remove_proc_entry(p->name, proc_reg);
			p->entry = NULL;
		}
	}

	remove_proc_entry(REG_NAME, proc_soc);
	remove_proc_entry(SOC_NAME, &proc_root);

	proc_reg = NULL;
	proc_soc = NULL;
}

module_init(monitor_init);
module_exit(monitor_exit);

MODULE_AUTHOR("VIA RISC & DSP SW Team");
MODULE_DESCRIPTION("wmt registers monitor driver");
MODULE_LICENSE("GPL");
