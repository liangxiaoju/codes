#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/poll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/time.h>
#include <linux/errno.h>

typedef struct
{
  /* V4l2 structures */
  struct v4l2_capability vc;
  struct v4l2_tuner vt;
  struct v4l2_control vctrl;
  struct v4l2_frequency vf;
  struct v4l2_hw_freq_seek vseek;

  int fd;

  unsigned int freq;
  unsigned int vol;

}radio_desc;

int radio_mute(radio_desc* radio)
{
	int ret;
	if(radio->fd < 0) return -ENODEV;
	(radio->vctrl).id = V4L2_CID_AUDIO_MUTE;
	(radio->vctrl).value = 1;
	ret = ioctl(radio->fd, VIDIOC_S_CTRL, &(radio->vctrl));
	return ret;
}
int radio_unmute(radio_desc* radio)
{
	int ret;
	if(radio->fd < 0) return -ENODEV;
	(radio->vctrl).id = V4L2_CID_AUDIO_MUTE;
	(radio->vctrl).value = 0;
	ret = ioctl(radio->fd, VIDIOC_S_CTRL, &(radio->vctrl));
	return ret;
}
int radio_setfreq(radio_desc *radio, unsigned int freq)
{	
	int ret;
	
	if(radio->fd < 0) return -ENODEV;
	(radio->vf).tuner = 0;
	(radio->vf).frequency = freq;
 
	ret = ioctl(radio->fd, VIDIOC_S_FREQUENCY, &(radio->vf));
	
	return ret;
}
int radio_autoseek(radio_desc *radio, unsigned int freq, int upward, int around)
{
	int ret;

	ret = radio_setfreq(radio, freq);
	if(ret < 0) {
		printf("Failed to set freq.\n");
		return ret;
	}
	radio->vseek.seek_upward = upward;
	radio->vseek.wrap_around = around;
	ret = ioctl(radio->fd, VIDIOC_S_HW_FREQ_SEEK, &(radio->vseek));
	if(ret < 0)
		printf("Failed to auto seek channel.\n");
	return ret;
}
int radio_seekall(radio_desc *radio)
{
	return 0;
}
int radio_getfreq(radio_desc *radio)
{
	int ret;
	
	if(radio->fd < 0) return -ENODEV;
	ret = ioctl(radio->fd, VIDIOC_G_FREQUENCY, &(radio->vf));
	if(ret < 0) return ret;
	
	return (radio->vf).frequency;
}
int radio_setvolume(radio_desc *radio, unsigned int vol)
{
	int ret;

	if(radio->fd < 0) return -ENODEV;
	radio->vctrl.id = V4L2_CID_AUDIO_VOLUME;
	radio->vctrl.value = vol;
	ret = ioctl(radio->fd, VIDIOC_S_CTRL, &(radio->vctrl));
	return ret;
}
int radio_getvolume(radio_desc *radio)
{
	int ret;

	if(radio->fd < 0) return -ENODEV;
	radio->vctrl.id = V4L2_CID_AUDIO_VOLUME;
	ret = ioctl(radio->fd, VIDIOC_G_CTRL, &(radio->vctrl));
	if(ret < 0) return ret;
	return (radio->vctrl).value;
}
int radio_getrssi(radio_desc *radio)
{
	int ret;

	if(radio->fd < 0) return -ENODEV;
	ret = ioctl(radio->fd, VIDIOC_G_TUNER, &(radio->vt));
	if(ret < 0) return ret;
	return radio->vt.signal;
}
int radio_open(radio_desc *radio)
{
	int ret;

	radio->fd = -1;
	ret = open("/dev/radio0", O_RDONLY);
	if(ret < 0) goto err;
	radio->fd = ret;

	ret = ioctl(radio->fd, VIDIOC_QUERYCAP, &(radio->vc));
	if(ret < 0) goto err;

	ret = ioctl(radio->fd, VIDIOC_S_TUNER, &(radio->vt));
	if(ret < 0) goto err;

	if(!((radio->vc).capabilities & V4L2_CAP_TUNER)) goto err;

	ret = radio_setvolume(radio, 8);
	if(ret < 0) goto err;

	return 0;
err:
	if(radio->fd >= 0) {
		close(radio->fd);
		radio->fd = -1;
	}
	return -1;
}
void radio_close(radio_desc *radio)
{
	radio_mute(radio);
	if(radio->fd >= 0) {
		close(radio->fd);
		radio->fd = -1;
	}
}

int main(int argc, char **argv)
{
	int ret, i, test_num;
	radio_desc radio={0};

	if(argc == 3) {
		ret = radio_open(&radio);
		if(ret < 0) {
			printf("Failed to open radio.\n");
			exit(-1);
		}

		printf("autoseek.\n");
		radio_autoseek(&radio, atoi(argv[1]), atoi(argv[2]), 1);

		while(1);
		radio_close(&radio);
		return 0;
	} else {
		printf("##### radio test #####\n");
		printf("[1] Test: open/close radio.\n");
		printf("[2] Test: set/get freq.\n");
		printf("[3] Test: set/get volume.\n");
		printf("[4] Test: autoseek.\n");
		printf("[5] Test: seekall.\n");
		printf("[6] Test: mute/unmute.\n");
	}
	printf("Select: ");
	scanf("%d",&test_num);

	system("echo 7 > /proc/sys/kernel/printk");

	switch(test_num) {
		case 1:
			printf("open/close radio test ...\n");
			for(i=0;i<20;i++) {
				ret = radio_open(&radio);
				if(ret < 0) {
					printf("Failed to open radio.\n");
					exit(-1);
				}
				radio_close(&radio);
			}
			for(i=0;i<5;i++) {
				int fd1,fd2;
				fd1 = open("/dev/radio0", O_RDONLY);
				if(fd1 < 0) {
					printf("Failed to open radio.\n");
					exit(-1);
				}
				fd2 = open("/dev/radio0", O_RDONLY);
				if(fd2 < 0) {
					printf("Radio already open.\n");
				}
				ret = close(fd1);
				if(ret < 0) {
					printf("Failed to close radio.\n");
					exit(-1);
				}
			}
			break;
		case 2:
			printf("set/get frequency test ...\n");
			printf("This may take a few minutes.\n");
			ret = radio_open(&radio);
			if(ret < 0) {
				printf("Failed to open radio.\n");
				exit(-1);
			}
			for(i=85000000;i<120000000;i+=10000) {
				ret = radio_setfreq(&radio, i);
				if(ret < 0) {
					printf("frequency %dHz over range.\n",i);
				} else {
					ret = radio_getfreq(&radio);
					if(ret != i) {
						printf("Failed: radio_getfreq.\n");
						exit(-1);
					}
				}
			}
			radio_close(&radio);
			break;
		case 3:
			printf("set/get volume test ...\n");
			ret = radio_open(&radio);
			if(ret < 0) {
				printf("Failed to open radio.\n");
				exit(-1);
			}
			ret = radio_autoseek(&radio, 87000000, 1, 1);
			if(ret < 0) {
				printf("Failed: radio_autoseek\n");
				exit(-1);
			}
			for(i=-1;i<20;i++) {
				ret = radio_setvolume(&radio, i);
				if(ret < 0) {
					printf("volume over range: %d.\n",i);
				} else {
					printf("setvolume: %d\n",i);
					ret = radio_getvolume(&radio);
					if(ret != i) {
						printf("Failed: radio_getvolume.\n");
						exit(-1);
					}
					sleep(2);
				}
			}
			radio_close(&radio);
			break;
		case 4:
			printf("autoseek test ...\n");
			printf("This may take a few minutes.\n");
			ret = radio_open(&radio);
			if(ret < 0) {
				printf("Failed to open radio.\n");
				exit(-1);
			}
			for(i=87000000;i<=108000000;i+=50000) {
				ret = radio_autoseek(&radio, i, 1, 1);
				if(ret < 0) {
					printf("Failed: radio_autoseek\n");
					exit(-1);
				}
			}
			ret = radio_autoseek(&radio, 87000000, 1, 1);
			if(ret < 0) {
				printf("Failed to auto seek: upward->1, wrap_around->1");
				exit(-1);
			}
			ret = radio_autoseek(&radio, 87000000, 1, 0);
			if(ret < 0) {
				printf("Failed to auto seek: upward->1, wrap_around->0");
				exit(-1);
			}
			ret = radio_autoseek(&radio, 87000000, 0, 1);
			if(ret < 0) {
				printf("Failed to auto seek: upward->0, wrap_around->1");
				exit(-1);
			}
			ret = radio_autoseek(&radio, 87000000, 0, 0);
			if(ret < 0) {
				printf("Failed to auto seek: upward->0, wrap_around->0");
				exit(-1);
			}
			radio_close(&radio);
			break;
		case 5:
			printf("seekall test ...\n");
			ret = radio_open(&radio);
			if(ret < 0) {
				printf("Failed to open radio.\n");
				exit(-1);
			}
			radio_close(&radio);
			break;
		case 6:
			printf("mute/unmute test ...\n");
			ret = radio_open(&radio);
			if(ret < 0) {
				printf("Failed to open radio.\n");
				exit(-1);
			}
			ret = radio_autoseek(&radio, 87000000, 1, 1);
			if(ret < 0) {
				printf("Failed: radio_autoseek\n");
				exit(-1);
			}
			sleep(3);
			for(i=0;i<5;i++) {
				printf("mute.\n");
				ret = radio_mute(&radio);
				if(ret < 0) {
					printf("Failed: mute.\n");
					exit(-1);
				}
				sleep(3);
				printf("unmute.\n");
				ret = radio_unmute(&radio);
				if(ret < 0) {
					printf("Failed: unmute.\n");
					exit(-1);
				}
				sleep(3);
			}
			radio_close(&radio);
			break;
		default:
			return -EINVAL;

	}
	printf("Everything is OK !\n");

	system("echo 7 > /proc/sys/kernel/printk");

	return 0;
}
