/*
 * export kernel informations to android system properties
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <cutils/properties.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void dummy(void *unused)
{
    fprintf(stdout, "dummy\n");
}

#define SYS_IMT_WIFI_TYPE_PATH "/sys/class/imt/wifi_type"
#define SYS_RK_WIFI_TYPE_PATH "/sys/class/rkwifi/chip"
int get_wifi_type(const char *path, char *buf)
{
    char line[PROPERTY_VALUE_MAX];
    FILE *fp = NULL;

    if (access(path, R_OK))
        return -1;

    fp = fopen(path, "r");
    if (!fp)
        return -1;

    fgets(line, sizeof(line), fp);
    fclose(fp);

    if (strncasecmp(line, "unknown", 7) == 0)
        return -1;

    if (line[strlen(line)-1] == '\n')
        line[strlen(line)-1] = '\0';

    snprintf(buf, PROPERTY_VALUE_MAX, "%s", line);

    return 0;
}

#define WIFI_TYPE_PROP "sys.wifi.type"
void export_wifi_type(void *unused)
{
    char buf[PROPERTY_VALUE_MAX];
    int count = 100; /* wait for 10s */

    property_set(WIFI_TYPE_PROP, "unknown");

    while (count-- > 0) {
        if ((get_wifi_type(SYS_IMT_WIFI_TYPE_PATH, buf) == 0) ||
            (get_wifi_type(SYS_RK_WIFI_TYPE_PATH, buf) == 0))
            break;
        usleep(100000);
    }

    if (count > 0)
        property_set(WIFI_TYPE_PROP, buf);
}

int run_async(void (*call)(void *), void *d)
{
    pid_t pid;

    pid = fork();
    if (pid < 0) {
        fprintf(stderr, "kprop: Failed to fork\n");
        return -errno;
    }

    if (pid == 0) {
        call(d);
        exit(0);
    }

    return pid;
}

void wait_for_all(void)
{
    int rv;

    do {
        rv = wait(NULL);
        if ((rv == -1) && (errno == ECHILD))
            break;

        fprintf(stdout, "kprop: pid %d exit.\n", rv);

    } while (1);
}

int main(int argc, char *argv[])
{
    run_async(dummy, NULL);
    run_async(export_wifi_type, NULL);

    wait_for_all();

    return 0;
}
