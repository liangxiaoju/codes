#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>

#define FIRMWARE_DIR	"/system/etc/firmware/goodix"
#define PROC_PATH	"/proc/goodix-update"

#define goodix_dbg(fmt, arg...)	printf("Goodix: " fmt, ## arg)

typedef enum {
	STEP_SET_PATH = 1,
	STEP_CHECK_FILE = 2,
	STEP_WRITE_SYN = 3,
	STEP_WAIT_SYN = 4,
	STEP_WRITE_LENGTH = 5,
	STEP_WAIT_READY = 6,
	STEP_WRITE_DATA = 7,
	STEP_READ_STATUS = 8,
	FUN_CLR_VAL = 9,
	FUN_CMD = 10,
	FUN_WRITE_CONFIG = 11,
} ts_cmd_t;

typedef enum {
	ARG_DISABLE_TP = 0,
	ARG_ENABLE_TP = 1,
	ARG_READ_VER = 2,
	ARG_READ_RAW = 3,
	ARG_READ_DIF = 4,
	ARG_READ_CFG = 5,
	ARG_SYS_REBOOT = 101,
} ts_arg_t;

typedef struct {
	int fd;
	char src_version[20];
	char dst_version[20];
	char *firmware_path;
	char *proc_path;
} ts_info_t;

static int ts_send_cmd(ts_info_t *ts_info, ts_cmd_t cmd, ts_arg_t arg)
{
	int ret;
	char buf[2];

	buf[0] = (char)cmd;
	buf[1] = (char)arg;

	ret = write(ts_info->fd, buf, 2);

	return ret;
}

static int ts_get_src_version(ts_info_t *ts_info)
{
	int ret;

	ts_send_cmd(ts_info, FUN_CMD, ARG_READ_VER);
	ret = read(ts_info->fd, ts_info->src_version, 20);
	if (ret <= 0)
		goodix_dbg("Failed to get src version.\n");

	return ret;
}

static int ts_cmp_version(ts_info_t *ts_info)
{
	int ret;

	ts_get_src_version(ts_info);

	/* we only take care of GT801 */
	if (strncmp(ts_info->src_version, "GT80", 4))
		return 0;

	ret = strcmp(ts_info->src_version, ts_info->dst_version);

	if (ret != 0)
		goodix_dbg("%s(%ld) --> %s(%ld)\n",
			ts_info->src_version, strlen(ts_info->src_version),
			ts_info->dst_version, strlen(ts_info->dst_version));

	return ret;
}

static int ts_update_firmware(ts_info_t *ts_info)
{
	int ret = 0, length;
	char *buf;
	ts_cmd_t cmd;

	ts_send_cmd(ts_info, FUN_CLR_VAL, 0);
	ts_send_cmd(ts_info, FUN_CMD, ARG_DISABLE_TP);

	/* set path */
	length = strlen(ts_info->firmware_path) + 1;
	buf = malloc(length + 1);
	if (!buf)
		return -1;

	buf[0] = STEP_SET_PATH;
	strncpy(buf + 1, ts_info->firmware_path, length);
	write(ts_info->fd, buf, length + 1);
	free(buf);

	/* update */
	for (cmd = STEP_CHECK_FILE; cmd < FUN_CLR_VAL; cmd++) {
		ret = ts_send_cmd(ts_info, cmd, 0);
		if (ret < 0)
			break;
	}

	ts_send_cmd(ts_info, FUN_CMD, ARG_ENABLE_TP);

	return ret;
}

static int look_for_the_correct_bin(ts_info_t *ts_info)
{
	int ret = 0;
	DIR *dirp;
	struct dirent *dirent;
	int length;
	char *str;

	dirp = opendir(FIRMWARE_DIR);
	if (dirp == NULL)
		return -1;

	for (;;) {
		dirent = readdir(dirp);
		if (dirent == NULL) {
			ret = -1;
			break;
		}
		if ((str = strstr(dirent->d_name, ".bin"))) {

			length = strlen(dirent->d_name);
			length += sizeof(FIRMWARE_DIR) + 1;
			ts_info->firmware_path = malloc(length);
			snprintf(ts_info->firmware_path, length,
				"%s/%s", FIRMWARE_DIR, dirent->d_name);

			length = str - dirent->d_name + 1;
			length = length < 20 ? length : 20;
			snprintf(ts_info->dst_version, length,
				"%s", dirent->d_name);

			goodix_dbg("Found %s(%d).\n",
					ts_info->firmware_path,
					strlen(ts_info->firmware_path));

			break;
		}
	}

	closedir(dirp);

	return ret;
}

static int ts_open(ts_info_t *ts_info)
{
	int ret = 0;

	memset(ts_info, 0, sizeof(ts_info_t));

	ts_info->proc_path = malloc(sizeof(PROC_PATH));
	strcpy(ts_info->proc_path, PROC_PATH);

	ret = look_for_the_correct_bin(ts_info);
	if (ret < 0)
		return ret;

	ret = open(PROC_PATH, O_RDWR);
	if (ret <= 0) {
		goodix_dbg("Failed to open %s.\n", PROC_PATH);
	}

	ts_info->fd = ret;

	return ret;
}

static void ts_close(ts_info_t *ts_info)
{
	if (ts_info->fd > 0)
		close(ts_info->fd);

	free(ts_info->proc_path);
	free(ts_info->firmware_path);
}

int main(int argc, char *argv)
{
	int ret;
	ts_info_t ts_info;

	ret = ts_open(&ts_info);
	if (ret < 0)
		goto out;

	ret = ts_cmp_version(&ts_info);
	if (ret != 0) {
		goodix_dbg("Update ... \n");
		ts_update_firmware(&ts_info);
	}

out:
	ts_close(&ts_info);

	return 0;
}
