#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>

static int speed_arr[] = {
    B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300
};
static int name_arr[] = {
    115200, 38400,  19200,  9600,  4800,  2400,  1200,  300
};

static void set_speed(int fd, int speed)
{
    int i;
    int status;
    struct termios options;

    tcgetattr(fd, &options);
    options.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);

    for (i = 0; i < sizeof(speed_arr)/sizeof(speed_arr[0]); i++) {
        if (speed == name_arr[i]) {
            tcflush(fd, TCIOFLUSH);
            cfsetispeed(&options, speed_arr[i]);
            cfsetospeed(&options, speed_arr[i]);
            status = tcsetattr(fd, TCSANOW, &options);
            if (status != 0) {
                perror("tcsetattr fd");
                return;
            }
            tcflush(fd,TCIOFLUSH);
        }
    }
}

static int set_parity(int fd, int databits, int stopbits, int parity)
{
    struct termios options;

    if(tcgetattr( fd,&options) != 0) {
        perror("tcgetattr");
        return -1;
    }

    options.c_cflag &= ~CSIZE;

    /* setup data bits */
    switch (databits) {
    case 5:
        options.c_cflag |= CS5;
        break;
    case 6:
        options.c_cflag |= CS6;
        break;
    case 7:
        options.c_cflag |= CS7;
        break;
    case 8:
        options.c_cflag |= CS8;
        break;
    default:
        fprintf(stderr,"Unsupported data size\n");
        return -1;
    }

    /* setup parity */
    switch (parity) {
    case 'n':
    case 'N':
        options.c_cflag &= ~PARENB;     /* Clear parity enable */
        options.c_iflag &= ~INPCK;      /* Enable parity checking */
        break;
    case 'o':
    case 'O':
        options.c_cflag |= (PARODD | PARENB);   /* Odd parity */
        options.c_iflag |= INPCK;               /* Disnable parity checking */
        break;
    case 'e':
    case 'E':
        options.c_cflag |= PARENB;      /* Enable parity */
        options.c_cflag &= ~PARODD;     /* Even parity */
        options.c_iflag |= INPCK;       /* Disnable parity checking */
        break;
    case 'S':
    case 's':
        options.c_cflag &= ~PARENB;     /* no parity */
        options.c_cflag &= ~CSTOPB;
        break;
    default:
        fprintf(stderr,"Unsupported parity\n");
        return -1;
    }

    /* setup stop bit */
    switch (stopbits) {
    case 1:
        options.c_cflag &= ~CSTOPB;
        break;
    case 2:
        options.c_cflag |= CSTOPB;
        break;
    default:
        fprintf(stderr,"Unsupported stop bits\n");
        return -1;
    }

    /* Set input parity option */
    if (parity != 'n')
        options.c_iflag |= INPCK;

    tcflush(fd,TCIFLUSH);
    options.c_cc[VTIME] = 150;      /* set timeout 15 seconds*/
    options.c_cc[VMIN] = 0;         /* Update the options and do it NOW */
    if (tcsetattr(fd,TCSANOW,&options) != 0) {
        perror("tcsetattr");
        return -1;
    }

    return 0;
}

#define GPS_DEV	"/dev/gps_sirf"
#define UART_DEV "/dev/s3c2410_serial3"
int main(int argc, char **argv)
{

    int tmp = 0;
    int fd, fd_gps;
    char buf[1024];
    char uart_name[] = UART_DEV;
    int i, ret;

    if (argc >= 2) {
        strncpy(uart_name, argv[1], sizeof(uart_name));
    }

    fd_gps = open(GPS_DEV, O_RDWR);
    if(fd_gps < 0) {
        printf("Failed to open %s\n", GPS_DEV);
        return -1;
    }

    /* power on gps */
    ioctl(fd_gps, 1);
    /* reset gps */
    ioctl(fd_gps, 2);
    sleep(2);

    for (i = 1; ; i++) {
        fd = open(uart_name, O_RDWR | O_NOCTTY);
        printf("Try %d times: %s to open %s\n", i, fd < 0 ? "failed" : "success", uart_name);
        if (fd >= 0)
            break;
        if (fd<0 && i>=3)
            return -1;
        sleep(1);
    }

    set_speed(fd, 4800);
    set_parity(fd, 8, 1, 'N');

    if (fork()) {
        while (1) {
            memset(buf, 0, sizeof(buf));
            if (ret = read(fd, buf, sizeof(buf))) {
                for (i = 0; i < ret; i++) {
                    if (buf[i] == '\n')
                        tmp ++;
                    else
                        tmp = 0;
                    if (tmp >= 2)
                        continue;

                    putchar(buf[i]);
                }
            }
        }
    } else {
        while (1) {
            memset(buf, 0, sizeof(buf));
            scanf("%s",buf);
            write(fd, buf, strlen(buf));
        }
    }

    close(fd);
    close(fd_gps);

    return 0;
}

