#ifndef __KLOG_H__
#define __KLOG_H__

enum klog_genl_attrs {
	NL_KLOG_ATTR_UNSPEC,
	NL_KLOG_ATTR_FIRST,

	__NL_KLOG_ATTR_LAST,
	NL_KLOG_ATTR_MAX = __NL_KLOG_ATTR_LAST - 1,
};

struct klog_msg {
	int id;
	char label[16];
	unsigned long long ts_nsec;
	int level;
	char data[512];
};

typedef int (*klog_msg_handler_t)(struct klog_msg *, void *arg);

typedef struct {
	int fd;
	int id;
	char *group;
	char *file;
	int size;
	klog_msg_handler_t handler;
} kinfo_t;

typedef struct {
	kinfo_t **kinfo;
	int num;

	struct nl_sock *nl;
	struct nl_sock *nl_event;
	struct nl_cache *nl_cache;
	struct genl_family *family;
	struct nl_cb *nl_cb;

	klog_msg_handler_t custom_cb;
	void *custom_arg;
} klog_t;

extern klog_t *klog_alloc(kinfo_t **kinfo, int n);
extern void klog_free(klog_t *klog);
extern int klog_init(klog_t *klog);
extern int default_custom_msg_handler(struct klog_msg *msg, void *arg);
extern int klog_set_handler(klog_t *klog, klog_msg_handler_t cb, void *arg);
extern int klog_event_handle(klog_t *klog);
extern void klog_loop(klog_t *klog);

#endif
