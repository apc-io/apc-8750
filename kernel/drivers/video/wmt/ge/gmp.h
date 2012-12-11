/* Generic Memory Provider */

#ifndef GMP_H
#define GMP_H
#ifdef __KERNEL__
#include <linux/types.h>
#include <linux/fb.h>
#else
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#endif

#define GMP_IOCTL_MAGIC        'g'
#define GMP_GET_PHYS           _IOW(GMP_IOCTL_MAGIC,  1, unsigned int)
/* #define GMP_MAP             _IOW(GMP_IOCTL_MAGIC,  2, unsigned int) */
#define GMP_GET_SIZE           _IOW(GMP_IOCTL_MAGIC,  3, unsigned int)
/* #define GMP_UNMAP           _IOW(GMP_IOCTL_MAGIC,  4, unsigned int) */
/* #define GMP_ALLOCATE        _IOW(GMP_IOCTL_MAGIC,  5, unsigned int) */
/* #define GMP_CONNECT         _IOW(GMP_IOCTL_MAGIC,  6, unsigned int) */
/* #define GMP_GET_TOTAL_SIZE  _IOW(GMP_IOCTL_MAGIC,  7, unsigned int) */
#define GMP_CACHE_FLUSH        _IOW(GMP_IOCTL_MAGIC,  8, unsigned int)
#define GMP_CACHE_FLUSH_ALL    _IOW(GMP_IOCTL_MAGIC, 10, unsigned int)
#define GMP_LOCK               _IOW(GMP_IOCTL_MAGIC, 15, unsigned int)

#define GMP_SHIFT         (16) /* 64 KB */
#define GMP_MASK          ((1 << GMP_SHIFT) - 1)
#define GMP_NR(descr)     (1 << descr->order)
#define GMP_SIZE(descr)   (GMP_NR(descr) << GMP_SHIFT)

#define GMP_LOG_EMERG     0   /* system is unusable                   */
#define GMP_LOG_ALERT     1   /* action must be taken immediately     */
#define GMP_LOG_CRIT      2   /* critical conditions                  */
#define GMP_LOG_ERR       3   /* error conditions                     */
#define GMP_LOG_WARNING   4   /* warning conditions                   */
#define GMP_LOG_NOTICE    5   /* normal but significant condition     */
#define GMP_LOG_INFO      6   /* informational                        */
#define GMP_LOG_DEBUG     7   /* debug-level messages                 */

#define GMP_TAG_DEFAULT   (0x30303030) /* 0000 in ascii */

struct gmdesc {
	unsigned long pa;
	int order;
	int used;
	unsigned int tag;
	pid_t owner;
	int tail;
};

struct gmp {
	struct gmdesc *gmdesc;
	unsigned long pa;
	int len;
	unsigned long hnd;
};

struct gmp_region {
	unsigned long pa;
	unsigned long len;
};

struct gmp *create_gmp(unsigned long pa, size_t size);
void release_gmp(struct gmp *gmp);

int gmp_lock(struct gmp *gmp);
int gmp_unlock(struct gmp *gmp);

unsigned long gmp_alloc(struct gmp *gmp, size_t size, unsigned int tag);
void gmp_free(struct gmp *gmp, unsigned long pa);

unsigned int gmp_itag(unsigned int value);
unsigned int gmp_ctag(char a, char b, char c, char d);

void *gmp_ioremap(struct gmp *gmp, unsigned long physaddr, size_t size);
void gmp_iounmap(struct gmp *gmp, void *va, size_t size);
void gmp_cache_flush(struct gmp *gmp, unsigned long physaddr, size_t size);

/* internal function */

void gmp_reset(struct gmp *gmp);
void gmp_invalidate(struct gmp *gmp, pid_t pid);

void gmp_set_log_level(int level);
void gmp_get_status(struct gmp *gmp, int *used, int *avail);
void gmp_print_status(struct gmp *gmp);

struct gmdesc *prev_gmdesc(struct gmp *gmp, struct gmdesc *gmdesc);
struct gmdesc *next_gmdesc(struct gmp *gmp, struct gmdesc *gmdesc);

#ifdef __KERNEL__
int register_gmp_device(unsigned long physaddr, unsigned long size);
int unregister_gmp_device(void);
#endif /* __KERNEL__ */

#endif /* GMP_H */
