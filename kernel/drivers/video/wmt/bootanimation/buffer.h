#ifndef ANIMATION_BUFFER_H_INCLUDED
#define ANIMATION_BUFFER_H_INCLUDED

#include "LzmaDec.h"
#include <linux/semaphore.h>

typedef struct 
{
    unsigned char *buffer;
    unsigned char *w_pos;
    unsigned char *r_pos;
    int frame_count;
    int frame_size;
    struct semaphore sem_writable;
    struct semaphore sem_readable;
    int     eof;
}animation_buffer;
    
                              
int animation_buffer_init(animation_buffer * buf, int size, int count, ISzAlloc *alloc);

int animation_buffer_stop(animation_buffer * buf);
int animation_buffer_release(animation_buffer * buf, ISzAlloc *alloc);


unsigned char * animation_buffer_get_writable(animation_buffer * buf, unsigned int * pSize);
void            animation_buffer_write_finish(animation_buffer * buf, unsigned char * addr);

unsigned char * animation_buffer_get_readable(animation_buffer * buf);
void            animation_buffer_read_finish(animation_buffer * buf, unsigned char * addr);


#endif /* ANIMATION_BUFFER_H_INCLUDED */

