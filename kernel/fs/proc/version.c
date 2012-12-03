#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/utsname.h>
#include <asm/mach/version.h>

static int version_proc_show(struct seq_file *m, void *v)
{
#if 1 /* Willy 2009/07/01 add to show kernel version */
    seq_printf(m, "%s\n",linux_banner);
    seq_printf(m, "Kernel %s (%s - %s)\n",CONFIG_KERNEL_VERSION,__DATE__,__TIME__);
#else
	seq_printf(m, linux_proc_banner);
		utsname()->sysname,
		utsname()->release,
		utsname()->version);
#endif
	return 0;
}

static int version_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, version_proc_show, NULL);
}

static const struct file_operations version_proc_fops = {
	.open		= version_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int __init proc_version_init(void)
{
	proc_create("version", 0, NULL, &version_proc_fops);
	return 0;
}
module_init(proc_version_init);
