#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>

/**
 *@brief  设置串口通信速率
 *@param  fd     类型 int  打开串口的文件句柄
 *@param  speed  类型 int  串口速度
 *@return  void
 */
static int speed_arr[] = {B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300,
    B38400, B19200, B9600, B4800, B2400, B1200, B300, };
static int name_arr[] = {115200, 38400,  19200,  9600,  4800,  2400,  1200,  300, 38400,  
    19200,  9600, 4800, 2400, 1200,  300, };
static void set_speed(int fd, int speed)
{
    int   i;
    int   status;
    struct termios options;
    tcgetattr(fd, &options);
    options.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++) {
        if  (speed == name_arr[i]) {
            tcflush(fd, TCIOFLUSH);
            cfsetispeed(&options, speed_arr[i]);
            cfsetospeed(&options, speed_arr[i]);
            status = tcsetattr(fd, TCSANOW, &options);
            if  (status != 0) {
                perror("tcsetattr fd");
                return;
            }
            tcflush(fd,TCIOFLUSH);
        }
    }
}

/**
 *@brief   设置串口数据位，停止位和效验位
 *@param  fd     类型  int  打开的串口文件句柄
 *@param  databits 类型  int 数据位   取值 为 5,6,7 或者8
 *@param  stopbits 类型  int 停止位   取值为 1 或者2
 *@param  parity  类型  int  效验类型 取值为N,E,O,S
 */
static int set_parity(int fd,int databits,int stopbits,int parity)
{ 
    struct termios options;
    if(tcgetattr( fd,&options) != 0) {
        perror("SetupSerial 1");
        return -1;
    }
    options.c_cflag &= ~CSIZE;
    switch (databits) /*设置数据位数*/
    {
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
    switch (parity) 
    {   
        case 'n':
        case 'N':    
            options.c_cflag &= ~PARENB;   /* Clear parity enable */
            options.c_iflag &= ~INPCK;     /* Enable parity checking */ 
            break;  
        case 'o':   
        case 'O':     
            options.c_cflag |= (PARODD | PARENB); /* 设置为奇效验*/  
            options.c_iflag |= INPCK;             /* Disnable parity checking */ 
            break;  
        case 'e':  
        case 'E':   
            options.c_cflag |= PARENB;     /* Enable parity */    
            options.c_cflag &= ~PARODD;   /* 转换为偶效验*/     
            options.c_iflag |= INPCK;       /* Disnable parity checking */
            break;
        case 'S':
        case 's':  /*as no parity*/
            options.c_cflag &= ~PARENB;
            options.c_cflag &= ~CSTOPB;
            break;
        default:   
            fprintf(stderr,"Unsupported parity\n");    
            return -1;  
    }  
    /* 设置停止位*/  
    switch (stopbits)
    {   
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
    options.c_cc[VTIME] = 150; /* 设置超时15 seconds*/   
    options.c_cc[VMIN] = 0; /* Update the options and do it NOW */
    if (tcsetattr(fd,TCSANOW,&options) != 0)   
    { 
        perror("SetupSerial 3");   
        return -1;
    } 
    return 0;
}

int main(int argc, char **argv)
{

    int fd;
    int i;

    if (argc != 3) {
        perror("param error\n");
        exit(-1);
    }

    fd = open(argv[1], O_RDWR | O_NOCTTY);
    if (fd < 0)
        return -1;

    set_speed(fd, atoi(argv[2]));
    set_parity(fd, 8, 1, 'N');

    if (fork()) {
        unsigned char buf[1024];
        while (1) {
            memset (buf, 0, sizeof(buf));
            if (read(fd, buf, sizeof(buf))) {
                fputs(buf, stdout);
            }
        }
    } else {
        unsigned char buf[1024];
        while (1) {
            memset (buf, 0, sizeof(buf));
            scanf("%s",buf);
            buf[strlen(buf)] = '\r';
            write(fd, buf, strlen(buf)+1);
        }
    }

    close(fd);
    return 0;
}

