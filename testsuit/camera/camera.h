#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "fb.h"

struct video_buffer {
    void *start;
    size_t length;
    struct v4l2_buffer buf;
};

struct video {
    int fd;
    char *dev;

    struct v4l2_capability cap;
    struct v4l2_format fmt;

    void *tmpbuf;

    int num_buffers;
    struct video_buffer *buffers;

    struct fb_dev fb;
};

#endif
