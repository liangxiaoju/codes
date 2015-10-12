#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

#include "camera.h"

static void usage(void)
{
    fprintf(stderr, "Usage: ./camera -d /dev/video0 -o /dev/fb0\n");
}

static int enum_frame_intervals(int fd, __u32 pixfmt, __u32 width, __u32 height)
{
    int err;
    struct v4l2_frmivalenum fival;

    memset(&fival, 0, sizeof(fival));
    fival.index = 0;
    fival.pixel_format = pixfmt;
    fival.width = width;
    fival.height = height;

    printf("\tfps:");
    while ((err = ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &fival)) == 0) {
        if (fival.type == V4L2_FRMIVAL_TYPE_DISCRETE) {
            printf(" [%u/%u]", fival.discrete.numerator, fival.discrete.denominator);
        } else if (fival.type == V4L2_FRMIVAL_TYPE_CONTINUOUS) {
            printf(" [%u/%u]..[%u/%u]",
                    fival.stepwise.min.numerator, fival.stepwise.min.denominator,
                    fival.stepwise.max.numerator, fival.stepwise.max.denominator);
            break;
        } else if (fival.type == V4L2_FRMIVAL_TYPE_STEPWISE) {
            printf(" [%u/%u]..[%u/%u] stepsize[%u/%u],",
                    fival.stepwise.min.numerator, fival.stepwise.min.denominator,
                    fival.stepwise.max.numerator, fival.stepwise.max.denominator,
                    fival.stepwise.step.numerator, fival.stepwise.step.denominator);
            break;
        }
        fival.index++;
    }

    printf("\n");
    if (err != 0 && errno != EINVAL) {
        perror("error enumerating frame intervals");
        return -1;
    }

    return 0;
}

static int enum_frame_sizes(int fd, __u32 pixfmt)
{
    int err;
    struct v4l2_frmsizeenum fsize;

    memset(&fsize, 0, sizeof(fsize));
    fsize.index = 0;
    fsize.pixel_format = pixfmt;

    while ((err = ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &fsize)) == 0) {
        if (fsize.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
            printf("discrete: width=%u height=%u\n",
                    fsize.discrete.width, fsize.discrete.height);
            err = enum_frame_intervals(fd, pixfmt,
                    fsize.discrete.width, fsize.discrete.height);
            if (err)
                printf("Unable to enumerate frame sizes.\n");
        } else if (fsize.type == V4L2_FRMSIZE_TYPE_CONTINUOUS) {
            printf("continuous: min[%u %u]..max[%u %u]",
                    fsize.stepwise.min_width, fsize.stepwise.min_height,
                    fsize.stepwise.max_width, fsize.stepwise.max_height);
            break;
        } else if (fsize.type == V4L2_FRMSIZE_TYPE_STEPWISE) {
            printf("setpwise: min[%u %u]..max[%u %u] stepsize[%u %u]",
                    fsize.stepwise.min_width, fsize.stepwise.min_height,
                    fsize.stepwise.max_width, fsize.stepwise.max_height,
                    fsize.stepwise.step_width, fsize.stepwise.step_height);
            break;
        }
        fsize.index++;
    }

    if (err != 0 && errno != EINVAL) {
        perror("error enumerating frame sizes");
        return -1;
    }

    return 0;
}

static int enum_frame_formats(struct video *vd)
{
    int err;
    struct v4l2_fmtdesc fmt;

    memset(&fmt, 0, sizeof(fmt));
    fmt.index = 0;
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    while ((err = ioctl(vd->fd, VIDIOC_ENUM_FMT, &fmt)) == 0) {
        printf("pixelformat='%c%c%c%c' description='%s'\n",
                fmt.pixelformat & 0xff, (fmt.pixelformat >> 8) & 0xff,
                (fmt.pixelformat >> 16) & 0xff, (fmt.pixelformat >> 24) & 0xff,
                fmt.description);
        err = enum_frame_sizes(vd->fd, fmt.pixelformat);
        if (err)
            printf("Unable to enumerate frame sizes.\n");

        fmt.index++;
    }

    if (errno != EINVAL) {
        perror("error enum frame formats");
        return -1;
    }

    return 0;
}

static void handle_frame(struct video *vd, void *src, int size, int width, int height, int fmt)
{
    post_fb(&vd->fb, src, width, height, size, fmt);
}

static int grab_frame(struct video *vd)
{
    struct v4l2_buffer buf;

    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    if (ioctl(vd->fd, VIDIOC_DQBUF, &buf) < 0) {
        perror("ioctl VIDIOC_DQBUF");
        return -1;
    }
/*
    printf("sequence:%d timeval:[%u.%u] \n",
            buf.sequence, buf.timestamp.tv_sec, buf.timestamp.tv_usec);
*/
    handle_frame(vd, vd->buffers[buf.index].start, buf.bytesused,
            vd->fmt.fmt.pix.width, vd->fmt.fmt.pix.height, vd->fmt.fmt.pix.pixelformat);

    if (ioctl(vd->fd, VIDIOC_QBUF, &buf) < 0) {
        perror("ioctl VIDIOC_QBUF");
        return -1;
    }

    return 0;
}

static int mainloop(struct video *vd)
{
    int counter = 0;
    float fps;
    int currtime, lasttime;
    int interval;
    struct timeval tv;

    gettimeofday(&tv, NULL);
    lasttime = tv.tv_sec*1000 + tv.tv_usec/1000;

    for (;;) {

        if (counter++ % 30 == 0) {
            gettimeofday(&tv, NULL);
            currtime = tv.tv_sec*1000 + tv.tv_usec/1000;

            interval = currtime - lasttime;
            if (interval > 0) {
                fps = 30 * (1000.0 / interval);
            }

            lasttime = currtime;

            printf("# fps: %f\n", fps);
        }

        fd_set fds;
        struct timeval tv;
        int r;

        FD_ZERO(&fds);
        FD_SET(vd->fd, &fds);

        tv.tv_sec = 2;
        tv.tv_usec = 0;

        r = select(vd->fd+1, &fds, NULL, NULL, &tv);
        if (r == -1) {
            perror("select");
            return -1;
        } else if (r == 0) {
            perror("select timeout");
            return -1;
        }

        grab_frame(vd);
    }
}

static int start_capture(struct video *vd)
{
    int i;
    enum v4l2_buf_type type;

    for (i = 0; i < vd->num_buffers; i++) {
        struct v4l2_buffer buf;

        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (ioctl(vd->fd, VIDIOC_QBUF, &buf) < 0) {
            perror("ioctl VIDIOC_QBUF");
            return -1;
        }
    }

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(vd->fd, VIDIOC_STREAMON, &type) < 0) {
        perror("ioctl VIDIOC_STREAMON");
        return -1;
    }

    return 0;
}

static int stop_capture(struct video *vd)
{
    enum v4l2_buf_type type;

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(vd->fd, VIDIOC_STREAMOFF, &type) < 0) {
        perror("ioctl VIDIOC_STREAMOFF");
        return -1;
    }

    return 0;
}

static int open_device(struct video *vd)
{
    int fd;
    struct stat st;

    if (stat(vd->dev, &st)) {
        perror("stat");
        return -1;
    }

    fd = open(vd->dev, O_RDWR/* | O_NONBLOCK*/);
    if (fd < 0) {
        perror("open");
        return -1;
    }

    vd->fd = fd;

    return 0;
}

static int init_mmap(struct video *vd)
{
    struct v4l2_requestbuffers req;
    int i;

    memset(&req, 0, sizeof(req));
    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (ioctl(vd->fd, VIDIOC_REQBUFS, &req) < 0) {
        perror("ioctl VIDIOC_REQBUFS");
        return -1;
    }

    vd->num_buffers = req.count;
    vd->buffers = calloc(req.count, sizeof(*vd->buffers));
    if (!vd->buffers) {
        perror("calloc");
        return -1;
    }

    for (i = 0; i < req.count; i++) {
        struct v4l2_buffer buf;

        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (ioctl(vd->fd, VIDIOC_QUERYBUF, &buf) < 0) {
            perror("ioctl VIDIOC_QUERYBUF");
            return -1;
        }

        vd->buffers[i].length = buf.length;
        vd->buffers[i].start = mmap(NULL, buf.length, PROT_READ|PROT_WRITE, MAP_SHARED, vd->fd, buf.m.offset);
        if (vd->buffers[i].start == MAP_FAILED) {
            perror("mmap");
            return -1;
        }
    }

    return 0;
}

static int init_device(struct video *vd)
{
    struct v4l2_format *fmt = &vd->fmt;
    struct v4l2_streamparm streamparm;

    memset(&vd->cap, 0, sizeof(struct v4l2_capability));
    if (ioctl(vd->fd, VIDIOC_QUERYCAP, &vd->cap) < 0) {
        perror("ioctl VIDIOC_QUERYCAP");
        return -1;
    }

    memset(fmt, 0, sizeof(struct v4l2_format));
    fmt->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(vd->fd, VIDIOC_G_FMT, fmt) < 0) {
        perror("ioctl VIDIOC_G_FMT");
        return -1;
    }

    if (1) {
        fmt->fmt.pix.width = 640;
        fmt->fmt.pix.height = 480;
        fmt->fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
        //fmt->fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
        fmt->fmt.pix.field = V4L2_FIELD_INTERLACED;

        if (ioctl(vd->fd, VIDIOC_S_FMT, fmt) < 0) {
            perror("ioctl VIDIOC_S_FMT");
            return -1;
        }
    }

    memset(&streamparm, 0, sizeof(streamparm));
    streamparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(vd->fd, VIDIOC_G_PARM, &streamparm) < 0) {
        perror("ioctl VIDIOC_G_PARM");
        return -1;
    }

    if (1) {
        streamparm.parm.capture.timeperframe.numerator = 1;
        streamparm.parm.capture.timeperframe.denominator = 30;

        if (ioctl(vd->fd, VIDIOC_S_PARM, &streamparm) < 0) {
            perror("ioctl VIDIOC_S_PARM");
            return -1;
        }
    }

    printf("# select: width=%d height=%d pixelformat=%c%c%c%c fps=%d/%d\n",
            fmt->fmt.pix.width, fmt->fmt.pix.height,
            fmt->fmt.pix.pixelformat & 0xff, (fmt->fmt.pix.pixelformat >> 8) & 0xff,
            (fmt->fmt.pix.pixelformat >> 16) & 0xff, (fmt->fmt.pix.pixelformat >> 24) & 0xff,
            streamparm.parm.capture.timeperframe.numerator, streamparm.parm.capture.timeperframe.denominator);

    init_mmap(vd);

    return 0;
}

static void close_device(struct video *vd)
{
    close(vd->fd);
}

static void dump_device(struct video *vd)
{
    struct v4l2_capability *cap = &vd->cap;

    printf("# device: '%s'\n", vd->dev);

    printf("capabilities: 0x%x\n", cap->capabilities);

    if (cap->capabilities & V4L2_CAP_VIDEO_CAPTURE)
        printf("V4L2_CAP_VIDEO_CAPTURE\n");

    if (cap->capabilities & V4L2_CAP_READWRITE)
        printf("V4L2_CAP_READWRITE\n");

    if (cap->capabilities & V4L2_CAP_STREAMING)
        printf("V4L2_CAP_STREAMING\n");

    if (cap->capabilities & V4L2_CAP_DEVICE_CAPS)
        printf("V4L2_CAP_DEVICE_CAPS\n");

    enum_frame_formats(vd);
}

int main(int argc, char *argv[])
{
    int opt;
    char *device = "/dev/video0";
    char *output = "/dev/fb0";
    struct video vd;
    int dump = 0;

    memset(&vd, 0, sizeof(vd));

    while ((opt = getopt(argc, argv, "d:o:lh")) != -1) {
        switch (opt) {
            case 'd':
                device = strdup(optarg);
                break;
            case 'o':
                output = strdup(optarg);
                break;
            case 'l':
                dump = 1;
                break;
            case 'h':
            default:
                usage();
                return 0;
        }
    }

    vd.dev = device;
    open_device(&vd);
    init_device(&vd);

    if (dump) {
        dump_device(&vd);
        return 0;
    }

    vd.fb.dev = output;
    init_fb(&vd.fb);
    initLut();

    start_capture(&vd);
    mainloop(&vd);
    stop_capture(&vd);

    close_device(&vd);

    return 0;
}
