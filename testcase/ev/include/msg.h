#ifndef __MSG_H__
#define __MSG_H__

#include <linux/input.h>

enum {
    NOTICE_STARTUP,
    NOTICE_STOP,
    NOTICE_PAUSE,
};

enum {
    MSG_TYPE_RESERVE = 0,
    MSG_TYPE_INPUT_EVENT,
    MSG_TYPE_SYSTEM,
};

#define MSG_DATA_SIZE 64

struct system_event {
    int notice;
};

typedef struct {
    long mtype;

    union {
        char mtext[MSG_DATA_SIZE];
        struct input_event input_event;
        struct system_event system_event;
    };

} msg_t;

#define MSG_SIZE sizeof(msg_t)

#endif
