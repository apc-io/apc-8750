#include <linux/module.h>
#include <linux/types.h>
#include <linux/io.h>

/*
 * Copy data from IO memory space to "real" memory space.
 * This needs to be optimized.
 */
void _memcpy_fromio(void *to, const volatile void __iomem *from, size_t count)
{
	unsigned char *t = to;
	while (count) {
		count--;
		*t = readb(from);
		t++;
		from++;
	}
}

/*
 * Copy data from "real" memory space to IO memory space.
 * This needs to be optimized.
 */
void _memcpy_toio(volatile void __iomem *to, const void *from, size_t count)
{
	/*const unsigned char *f = from;
	while (count) {
		count--;
		writeb(*f, to);
		f++;
		to++;
	}*/
	/*Dannierchen update for 4 byte data memory cpy*/
	void *vdest = (void __force *) to;
	
	//__asm__ __volatile__ ("sync" : : : "memory");
	while(count && ((((unsigned long)vdest)&3) || (((unsigned long)from)&3))) {
		*((volatile u8 *)vdest) = *((u8 *)from);
		from++;
		vdest++;
		count--;
		/*printk("start sf_write(vdest:0x%x, from:0x%x )\n", vdest, from);*/
	}
	while(count >= 4) {
		*((volatile u32 *)vdest) = *((volatile u32 *)from);
		from += 4;
		vdest += 4;
		count-=4;
	}
	while(count) {
		*((volatile u8 *)vdest) = *((u8 *)from);
		from++;
		vdest++;
		count--;
		/*printk("end sf_write(vdest:0x%x, from:0x%x )\n", vdest, from);*/
	}
	//__asm__ __volatile__ ("sync" : : : "memory");
}

/*
 * "memset" on IO memory space.
 * This needs to be optimized.
 */
void _memset_io(volatile void __iomem *dst, int c, size_t count)
{
	while (count) {
		count--;
		writeb(c, dst);
		dst++;
	}
}

EXPORT_SYMBOL(_memcpy_fromio);
EXPORT_SYMBOL(_memcpy_toio);
EXPORT_SYMBOL(_memset_io);
