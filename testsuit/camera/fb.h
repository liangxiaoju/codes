#ifndef __FB_H__
#define __FB_H__

#include <linux/fb.h>

struct fb_dev {
    int fd;
    char *dev;

    struct fb_var_screeninfo var;
    void *fbmem;
    int fblen;

    void *tmpbuf;
};

#endif
