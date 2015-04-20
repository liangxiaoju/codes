#include <stdio.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <sys/reboot.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/sendfile.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/netlink.h>
#include <linux/fb.h>
#include <time.h>

#define DEBUG

#define EVENT_KEY (KEY_MAX + 1)
#define EVENT_UEVENT (KEY_MAX + 2)
#define EVENT_TIMEOUT (KEY_MAX + 3)

/* long press powerkey to boot system (unit: ms) */
#define POWER_LONG_PRESS_TIME 5000

/* poweroff pending time (unit: ms) */
#define POWEROFF_PENDING_TIME 5000

/* timeout to enter mem state (unit: ms) */
#define ENTER_MEM_STATE_TIME 10000

/* loop time (unit: ms) */
#define LOOP_TIME 5000

#define UEVENT_MSG_LEN 1024

#define BITS_PER_LONG (sizeof(unsigned long) * 8)
#define BITS_TO_LONGS(x) (((x) + BITS_PER_LONG - 1) / BITS_PER_LONG)

#define test_bit(bit, array) \
        ((array)[(bit)/BITS_PER_LONG] & (1 << ((bit) % BITS_PER_LONG)))

#define ARRAY_SIZE(x) (int)(sizeof(x)/sizeof(x[0]))

#define PSY_PATH	"/sys/class/power_supply"

#ifdef DEBUG
#define LOG(fmt, arg...) printf("[LPM]" fmt, ##arg)
#else
#define LOG(fmt, arg...)
#endif

typedef struct power_info {
	int usb_online;
	int ac_online;
	int batt_present;
	int batt_volt;
	int batt_cap;
	int batt_charged;
} power_info_t;

struct uevent {
    const char *res;
};

struct key_state {
    int down;
    int64_t timestamp;
    int pending;
};

enum power_state {
    POWER_STATE_ON = 0,
    POWER_STATE_MEM,
};

typedef struct batt {

    struct pollfd ev_fds[32];
    int ev_count;
    int uevent_fd;

    struct key_state keys[KEY_MAX + 1];

    int vthreshold;
    int cthreshold;
    power_info_t info;

    enum power_state power_state;

    int64_t poweroff_timestamp;
    int64_t state_on_timestamp;

} batt_t;


int64_t get_time_ms(void)
{
    struct timespec tm;
    clock_gettime(CLOCK_MONOTONIC, &tm);
    return tm.tv_sec * 1000LL + (tm.tv_nsec / 1000000LL);
}

int writesysfs(const char *path, const char *str)
{
    char buf[128];
    int fd, ret;

    fd = open(path, O_RDWR);
    if (fd < 0) {
        LOG("Could not open '%s'\n", path);
        return -1;
    }

    snprintf(buf, sizeof(buf), "%s\n", str);

    write(fd, buf, strlen(buf)+1);

    close(fd);

    return 0;
}

int readsysfs(const char *path, char *buf, size_t size)
{
    int fd;
    size_t count;

    fd = open(path, O_RDONLY);
    if (fd < 0) {
        LOG("Could not open '%s'\n", path);
        return -1;
    }

    count = read(fd, buf, size);

    close(fd);

    if (count > 0) {

        count = (count < size) ? count : size - 1;

        while (count > 0 && buf[count-1] == '\n')
            count--;

        buf[count] = '\0';

    } else {
        buf[0] = '\0';
    }

    return count;
}

int read_boolean(const char *path)
{
    char buf[64];
    int ret;

    ret = readsysfs(path, buf, sizeof(buf));
    if (ret < 0)
        return -1;

    return (strncmp(buf, "1", 1) == 0);
}

int read_int(const char *path)
{
    char buf[64];
    int ret;

    ret = readsysfs(path, buf, sizeof(buf));
    if (ret < 0)
        return -1;

    return (int)strtol(buf, NULL, 0);
}

int get_power_info(power_info_t *info)
{
    char path[128];
    char buf[64], batt_status[32];

//    snprintf(path, sizeof(path), "%s/%s/%s", PSY_PATH, "usb", "online");
//    info->usb_online = read_boolean(path);

    snprintf(path, sizeof(path), "%s/%s/%s", PSY_PATH, "ac", "online");
    info->ac_online = read_boolean(path);

    snprintf(path, sizeof(path), "%s/%s/%s", PSY_PATH, "battery", "present");
    info->batt_present = read_boolean(path);

    snprintf(path, sizeof(path), "%s/%s/%s", PSY_PATH, "battery", "voltage_now");
    info->batt_volt = read_int(path);

    snprintf(path, sizeof(path), "%s/%s/%s", PSY_PATH, "battery", "capacity");
    info->batt_cap = read_int(path);

    snprintf(path, sizeof(path), "%s/%s/%s", PSY_PATH, "battery", "status");
    readsysfs(path, buf, sizeof(buf));
    snprintf(batt_status, sizeof(batt_status), "%s", buf);
    info->batt_charged = (strncmp(buf, "Charging", 8) == 0);

    LOG("----------info-----------\n"
            "POWER INFO:\n"
//            "USB: %s\n"
            "AC: %s\n"
            "BATTERY:\n"
            "\tvolt: %d\n"
            "\tcapacity: %d\n"
            "\tstatus: %s\n"
            "-------------------------\n",
//            info->usb_online ? "online" : "offline",
            info->ac_online ? "online" : "offline",
            info->batt_volt,
            info->batt_cap,
            batt_status
       );

    return 0;
}

void poweroff(void)
{
    LOG("poweroff !!!\n");
    reboot(RB_POWER_OFF);
    while (1);
}

int check_battery(batt_t *batt)
{
    power_info_t *info = &batt->info;
    int64_t now = get_time_ms();
    int ret = 0;

    LOG("checking battery ...\n");

    get_power_info(info);
    if ((info->batt_volt < batt->vthreshold) || (info->batt_cap < batt->cthreshold)) {
        if (info->batt_charged == 1) {
            batt->poweroff_timestamp = 0;
        } else {
            if (batt->poweroff_timestamp == 0) {
                batt->poweroff_timestamp = now;
                LOG("Poweroff pending ...\n");
            }
            if ((now - batt->poweroff_timestamp) >= POWEROFF_PENDING_TIME)
                poweroff();
        }
    } else {
        LOG("check OK !\n");
        ret = 1;
    }

    return ret;
}

int write_logo(int fd_fb, const char *logo)
{
    struct stat st;
    int fd_logo, ret;

    fd_logo = open(logo, O_RDONLY);
    if (fd_logo < 0) {
        LOG("failed to open logo.\n");
        return -1;
    }

    if (fstat(fd_logo, &st) < 0) {
        LOG("failed to fstat logo.\n");
        close(fd_logo);
        return -1;
    }

    if (sendfile(fd_fb, fd_logo, NULL, st.st_size) < 0) {
        LOG("failed to sendfile.\n");
        close(fd_logo);
        return -1;
    }

    close(fd_logo);

    return 0;
}

int display_logo(batt_t *batt)
{
    struct fb_var_screeninfo var;
    int fd, ret;

    fd = open("/dev/graphics/fb0", O_RDWR);
    if (fd < 0) {
        fd = open("/dev/fb0", O_RDWR);
        if (fd < 0) {
            LOG("failed to open fb.\n");
            return -1;
        }
    }

    ioctl(fd, FBIOGET_VSCREENINFO, &var);

    LOG("xres: %d, yres: %d, bpp: %d\n", var.xres, var.yres, var.bits_per_pixel);

    ioctl(fd, FBIOBLANK, FB_BLANK_POWERDOWN);
    ioctl(fd, FBIOBLANK, FB_BLANK_UNBLANK);

    if ((var.xres == 800) && (var.yres == 600)) {
        ret = write_logo(fd, "/logo-low-l");
    } else {
        ret = write_logo(fd, "/logo-low-h");
    }

    LOG("display logo (%s).\n", ret ? "failed" : "success");

    close(fd);

    return ret;
}

#define BL_DIR "/sys/class/backlight"
int set_backlight(batt_t *batt, int value)
{
    char buf[16], path[256];
    struct stat st;
    DIR *dir;
    struct dirent *de;

    dir = opendir(BL_DIR);
    if (dir == NULL)
        return -1;
    while ((de = readdir(dir))) {
        snprintf(path, sizeof(path), "%s/%s/brightness", BL_DIR, de->d_name);
        if (stat(path, &st) == 0) {
            break;
        }
    }

    snprintf(buf, sizeof(buf), "%d", value);

    LOG("set backlight: %d\n", value);

    return writesysfs(path, buf);
}

int input_init(batt_t *batt)
{
    char name[64];
    DIR *dir;
    struct dirent *de;
    int fd;

    dir = opendir("/dev/input");
    if (dir == NULL)
        return -1;

    while ((de = readdir(dir))) {
        unsigned long ev_bits[BITS_TO_LONGS(EV_MAX)];

        if (strncmp(de->d_name, "event", 5))
            continue;
        fd = openat(dirfd(dir), de->d_name, O_RDONLY);
        if (fd < 0)
            continue;
        if (ioctl(fd, EVIOCGBIT(0, sizeof(ev_bits)), ev_bits) < 0) {
            close(fd);
            continue;
        }
        if (!test_bit(EV_KEY, ev_bits)/* && !test_bit(EV_REL, ev_bits)*/) {
            close(fd);
            continue;
        }

        ioctl(fd, EVIOCGNAME(64), name);

        LOG("add input: /dev/input/%s [%s]\n", de->d_name, name);
 
        fcntl(fd, F_SETFL, O_NONBLOCK);

        if (batt->ev_count > (int)(ARRAY_SIZE(batt->ev_fds) - 1))
            break;

        batt->ev_fds[batt->ev_count].fd = fd;
        batt->ev_count++;
    }

    return 0;
}

int uevent_init(batt_t *batt)
{
    struct sockaddr_nl addr;
    int buf_sz = 64 * 1024;
    int on = 1;
    int s;

    memset(&addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_pid = getpid();
    addr.nl_groups = 0xffffffff;

    s = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
    if (s < 0)
        return -1;

    setsockopt(s, SOL_SOCKET, SO_RCVBUFFORCE, &buf_sz, sizeof(buf_sz));
    setsockopt(s, SOL_SOCKET, SO_PASSCRED, &on, sizeof(on));

    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(s);
        return -1;
    }

    fcntl(s, F_SETFL, O_NONBLOCK);

    if (batt->ev_count > (ARRAY_SIZE(batt->ev_fds) - 1)) {
        close(s);
        return -1;
    }

    batt->uevent_fd = s;
    batt->ev_fds[batt->ev_count].fd = s;
    batt->ev_count++;

    LOG("add kernel uevent\n");

    return s;
}

int event_init(batt_t *batt)
{
    input_init(batt);
    uevent_init(batt);

    return 0;
}

int push_input(batt_t *batt, int fd)
{
    int64_t now = get_time_ms();
    struct input_event ev;
    int r;

    r = read(fd, &ev, sizeof(ev));
    if (r == sizeof(ev)) {
        if ((ev.code > 0) && (ev.code < KEY_MAX)) {
            if (ev.value == batt->keys[ev.code].down)
                return -1;
            batt->keys[ev.code].down = ev.value;
            batt->keys[ev.code].pending = true;
            if (ev.value)
                batt->keys[ev.code].timestamp = now;
            LOG("push_input: code=%d down=%d\n", ev.code, ev.value);
            return 0;
        }
    }

    return -1;
}

int pop_input(batt_t *batt, int *code, int *down)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(batt->keys); i++) {
        if (!batt->keys[i].pending)
            continue;
        batt->keys[i].pending = false;
        *code = i;
        *down = batt->keys[i].down;
        LOG("pop_input: code=%d down=%d\n", *code, *down);
        return 0;
    }

    return -1;
}

int get_event(batt_t *batt, int timeout)
{
    struct pollfd *ev_fd;
    int nr, i;

    for (i = 0; i < batt->ev_count; i++) {
        batt->ev_fds[i].events = POLLIN;
        batt->ev_fds[i].revents = 0;
    }

    nr = poll(batt->ev_fds, batt->ev_count, timeout);
    if (nr < 0)
        return -1;

    /* timeout */
    if (nr == 0)
        return EVENT_TIMEOUT;

    for (i = 0; i < batt->ev_count; i++) {
        ev_fd = &batt->ev_fds[i];
        if (ev_fd->revents == POLLIN) {
            if (ev_fd->fd == batt->uevent_fd) {
                LOG("get_event: UEVENT\n");
                return EVENT_UEVENT;
            } else {
                LOG("get_event: KEY\n");
                push_input(batt, ev_fd->fd);
            }
        }
    }

    return EVENT_KEY;
}

ssize_t uevent_kernel_multicast_recv(int socket, void *buffer, size_t length)
{
    int n;

    n = recv(socket, buffer, length, 0);
    LOG("socket (%d): %s\n", n, (char *)buffer);

    return n;
}

int parse_uevent(const char *msg, struct uevent *uevent)
{
    uevent->res = msg;
    return 0;
}

int process_uevent(batt_t *batt, struct uevent *uevent)
{
    int ret = 0;

    if (strstr(uevent->res, "power_supply"))
        ret = check_battery(batt);

    return ret;
}

int handle_uevent(batt_t *batt)
{
    char msg[UEVENT_MSG_LEN + 2];
    int n, ret = 0;

    while (true) {
        struct uevent uevent;

        n = uevent_kernel_multicast_recv(batt->uevent_fd, msg, UEVENT_MSG_LEN);
        if (n <= 0)
            break;
        if (n >= UEVENT_MSG_LEN)
            continue;

        msg[n] = '\0';
        msg[n+1] = '\0';

        parse_uevent(msg, &uevent);
        ret = process_uevent(batt, &uevent);
    }

    return ret;
}

int enter_power_state(batt_t *batt, enum power_state state)
{
    int64_t now = get_time_ms();
    char str[32];
    int fd, len;

    batt->power_state = state;

    if (state == POWER_STATE_ON) {
        batt->state_on_timestamp = now;
    }

    snprintf(str, sizeof(str), "%s\n", state == POWER_STATE_ON ? "on" : "mem");
    len = strlen(str) + 1;

    fd = open("/sys/power/state", O_RDWR);
    if (fd <= 0)
        return -1;

    LOG("enter_state: %s\n", str);

    write(fd, str, len);

    close(fd);

    return 0;
}

enum power_state get_power_state(batt_t *batt)
{
    return batt->power_state;
}

int handle_key_event(batt_t *batt)
{
    int64_t now = get_time_ms();
    int ret, code, down;

    ret = pop_input(batt, &code, &down);
    if (ret >= 0) {
        LOG("handle_key_event: code=%d down=%d timestamp=%lld\n",
                code, down, batt->keys[code].timestamp);
        switch (code) {
            case KEY_POWER:
                if ((now - batt->keys[code].timestamp) >= POWER_LONG_PRESS_TIME) {
                    LOG("power key long time pressed !\n");
                    LOG("--> enter system ...\n");
                    return 1;
                }
                if (!down) {
                    if (get_power_state(batt) == POWER_STATE_ON) {
                        enter_power_state(batt, POWER_STATE_MEM);
                    } else {
                        enter_power_state(batt, POWER_STATE_ON);
                        system("cat /sys/power/wait_for_fb_wake");
                        set_backlight(batt, 5);
                        display_logo(batt);
                    }
                }
                break;
            default:
                break;
        }
    }

    return 0;
}

int handle_timeout(batt_t *batt)
{
    int64_t now = get_time_ms();

    /* timeout to enter mem state */
    if ((now - batt->state_on_timestamp) >= ENTER_MEM_STATE_TIME) {
        if (get_power_state(batt) != POWER_STATE_MEM) {
            LOG("Timeout to enter mem state.\n");
            enter_power_state(batt, POWER_STATE_MEM);
        }
    }

    return check_battery(batt);
}

void event_loop(batt_t *batt)
{
    int event, ret = 0;

    while (true) {
        event = get_event(batt, LOOP_TIME);
        if (event == -1)
            continue;
        else if (event == EVENT_UEVENT)
            ret = handle_uevent(batt);
        else if (event == EVENT_KEY)
            ret = handle_key_event(batt);
        else if (event == EVENT_TIMEOUT)
            ret = handle_timeout(batt);

        /* ok, boot to system */
        if (ret > 0) {
            enter_power_state(batt, POWER_STATE_ON);
            display_logo(batt);
            set_backlight(batt, 5);
            break;
        }

        /* shutdown */
        if (ret < 0)
            poweroff();
    }
}

int main(int argc, char *argv[])
{
    batt_t batt;
    int ret = 0;
    int vth = 3550000;
    int cth = 3;

    setlinebuf(stdout);
    setlinebuf(stderr);

    memset(&batt, 0, sizeof(batt_t));

    if (argc >= 2)
        vth = atoi(argv[1]);
    if (argc >= 3)
        cth = atoi(argv[2]);

    batt.vthreshold = vth;
    batt.cthreshold = cth;

    LOG("limit: vth=%duV cth=%d\n", vth, cth);

    ret = check_battery(&batt);
    if (ret > 0) {
        LOG("boot to system !\n");
        return 0;
    }

    display_logo(&batt);

    LOG("~~~~~~~~~~~~~~~~~~low power mode~~~~~~~~~~~~~~~~~~\n");

    event_init(&batt);
    event_loop(&batt);

    return ret;
}
