#ifndef _BOOTANIMATION_H_
#define _BOOTANIMATION_H_


struct animation_fb_info {
    unsigned char * addr;    // frame buffer start address
    unsigned int width;      // width
    unsigned int height;     // height
    unsigned int color_fmt;  // color format,  0 -- rgb565, 1 -- rgb888
};

int animation_start(struct animation_fb_info *info);
int animation_stop(void);

#endif
