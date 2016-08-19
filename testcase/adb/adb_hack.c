#include <linux/init.h>
#include <linux/module.h>
#include <linux/kmod.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/input.h>
#include <linux/printk.h>
#include <linux/string.h>
#include <linux/workqueue.h>
#include <linux/net.h>
#include <linux/un.h>
#include <linux/delay.h>

#define ANDROID_SOCKET_DIR "/dev/socket"

#define PROP_NAME_MAX   32
#define PROP_VALUE_MAX  92

#define PROP_MSG_SETPROP 1

struct prop_msg
{
    unsigned cmd;
    char name[PROP_NAME_MAX];
    char value[PROP_VALUE_MAX];
};

static struct socket *sock_init(const char *sock_path)
{
	struct socket *sock = NULL;
	struct sockaddr_un *sunaddr;
	int err;

	err = sock_create_kern(AF_UNIX, SOCK_STREAM, 0, &sock);
	if (err < 0) {
		pr_err("Failed to create sock err=%d.\n", err);
		return NULL;
	}

	/* add one more byte here, find the reason in af_unix.c:unix_mkname */
	sunaddr = kzalloc(sizeof(*sunaddr)+1, GFP_KERNEL);
	if (!sunaddr)
		return NULL;

	sunaddr->sun_family = AF_UNIX;
	snprintf(sunaddr->sun_path, UNIX_PATH_MAX, "%s", sock_path);

	err = kernel_connect(sock, (struct sockaddr *)sunaddr,
			sizeof(*sunaddr), O_NONBLOCK);

	kfree(sunaddr);

	if (err < 0) {
		pr_err("Failed to connect sock err=%d.\n", err);
		if (sock)
			sock_release(sock);
		return NULL;
	}

	return sock;
}

static void sock_deinit(struct socket *sock)
{
	if (sock)
		sock_release(sock);
}

static int sock_recv(struct socket *sock, char *buf, int len)
{
	struct msghdr msg;
	struct kvec vec;
	int bytes;

	memset(&msg, 0, sizeof(msg));
	vec.iov_base = buf;
	vec.iov_len = len;

	bytes = kernel_recvmsg(sock, &msg, &vec, 1, len, MSG_DONTWAIT);
	if (bytes < 0) {
		pr_err("Failed to recvmsg %d.\n", bytes);
		return -1;
	}

	return bytes;
}

static int sock_send(struct socket *sock, char *buf, int len)
{
	struct msghdr msg;
	struct kvec vec;
	int bytes;

	memset(&msg, 0, sizeof(msg));
	vec.iov_base = buf;
	vec.iov_len = len;

	bytes = kernel_sendmsg(sock, &msg, &vec, 1, len);
	if (bytes < 0) {
		pr_err("Failed to sendmsg %d.\n", bytes);
		return -1;
	}

	return bytes;
}

#if 0
static int restart_adbd(void)
{
	char **argv;
	char *envp[] = {
		"HOME=/",
		"PATH=/sbin:/bin:/system/xbin:/system/bin",
		NULL
	};
	const char *action_cmd = "/system/bin/setprop sys.usb.config mtp,adb";
	int ret;

	pr_info("WARN: running '%s'\n", action_cmd);

	argv = argv_split(GFP_KERNEL, action_cmd, NULL);
	if (argv) {
		ret = call_usermodehelper(argv[0], argv, envp, UMH_WAIT_EXEC);
		argv_free(argv);
	}

	if (ret < 0) {
		pr_err("Failed to run %s\n", action_cmd);
	}

	return ret;
}
#else
static int restart_adbd(void)
{
    struct prop_msg msg;
	struct socket *sock;
	int bytes;

	sock = sock_init(ANDROID_SOCKET_DIR"/property_service");
	if (!sock) {
		pr_err("Failed to connect to socket:property_service.\n");
		return -1;
	}

    memset(&msg, 0, sizeof msg);
    msg.cmd = PROP_MSG_SETPROP;
	snprintf(msg.name, sizeof(msg.name), "sys.usb.config");
	snprintf(msg.value, sizeof(msg.value), "mtp,adb");

	bytes = sock_send(sock, (char *)&msg, sizeof(msg));
	if (bytes == sizeof(msg)) {
		msleep(250);
		pr_info("Success to setprop %s %s\n", msg.name, msg.value);
	} else {
		pr_err("Failed to send msg to socket:property_service.\n");
	}

	sock_deinit(sock);
	return 0;
}
#endif

static void action(struct work_struct *work)
{
	struct socket *sock = NULL;
	int bytes = -1;
	char *buf;
	int len = 4096;
	int i;
	int retries = 10;

	pr_info("ADB HACK: START.\n");

	buf = kmalloc(len, GFP_KERNEL);
	if (!buf) {
		pr_err("Failed to kmalloc.\n");
		return;
	}

	while ((retries--) > 0) {
		restart_adbd();
		msleep(1000);

		if (!sock) {
			sock = sock_init(ANDROID_SOCKET_DIR"/adbd");
			if (!sock) {
				msleep(1000);
				continue;
			}
		}

		for (i = 0; i < 10; i++) {
			bytes = sock_recv(sock, buf, len);
			if (bytes > 0) {
				print_hex_dump(KERN_CONT, "auth_sock: ", DUMP_PREFIX_OFFSET,
						16, 1,
						buf, bytes, true);

				if ((buf[0] == 'P') && (buf[1] == 'K')) {
					bytes = snprintf(buf, len, "OK");
					sock_send(sock, buf, bytes);
					pr_info("ADB HACK: DONE.\n");
					goto out;
				}
			}
			msleep(500);
		}

		sock_deinit(sock);
		sock = NULL;
	}

out:
	if (buf)
		kfree(buf);
}

static DECLARE_DELAYED_WORK(action_work, action);

static void button_jack_event(int pressed)
{
	if (pressed) {
		queue_delayed_work(system_long_wq, &action_work, 10*HZ);
	} else {
		cancel_delayed_work(&action_work);;
	}
}

static void jack_event(struct input_handle *handle,
			unsigned int type, unsigned int code, int value)
{
	pr_debug("jack event: type=%d code=%d value=%d\n", type, code, value);

	if (code == KEY_MEDIA) {
		button_jack_event(value);
	}
}

static int jack_connect(struct input_handler *handler, struct input_dev *dev,
			 const struct input_device_id *id)
{
	int error;
	struct input_handle *handle;

	handle = kzalloc(sizeof(struct input_handle), GFP_KERNEL);
	if (!handle) {
		return -ENOMEM;
	}

	handle->dev = input_get_device(dev);
	handle->name = "Jack";
	handle->handler = handler;
	handle->private = NULL;

	error = input_register_handle(handle);
	if (error) {
		kfree(handle);
		return error;
	}

	error = input_open_device(handle);
	if (error) {
		input_unregister_handle(handle);
		kfree(handle);
		return error;
	}

	return 0;
}

static void jack_disconnect(struct input_handle *handle)
{
	input_flush_device(handle, NULL);
	input_close_device(handle);
	input_unregister_handle(handle);
	kfree(handle);
}

bool jack_match(struct input_handler *handler, struct input_dev *dev)
{
	if (strstr(dev->name, "Button Jack") && strstr(dev->phys, "ALSA")) {
		pr_debug("Found: %s\n", dev->name);
		return true;
	}
	return false;
}

static const struct input_device_id jack_ids[] = {
	{ .driver_info = 1 },
	{ },
};
MODULE_DEVICE_TABLE(input, jack_ids);

static struct input_handler jack_handler = {
	.event		= jack_event,
	.connect	= jack_connect,
	.disconnect	= jack_disconnect,
	.name		= "jack",
	.id_table	= jack_ids,
	.match		= jack_match,
};

static int __init hack_init(void)
{
	return input_register_handler(&jack_handler);
}

static void __exit hack_exit(void)
{
	input_unregister_handler(&jack_handler);
}

module_init(hack_init);
module_exit(hack_exit);

MODULE_LICENSE("GPL");
