#ifndef __MODULE_H__
#define __MODULE_H__

#include <cdk.h>

#define MODULES_DIR "./modules"
#define MODULE_INFO_SYM "module_info_sym"

typedef int (*handler_t)(WINDOW *win, void *ptr);

struct msgfunc;

typedef struct {

	int version;
	/* the name of the tag that this module belong to */
	const char *id;
	/* the item name display in the selection */
	const char *name;

	handler_t handler;

	struct message *msg_list;

	struct msgfunc *msgfunc;

} module_t;

struct msgfunc {
	struct message *(*mkmsg)(const char *from, const char *to, char *msg, void *ptr);
	void (*freemsg)(struct message *msg);
	int (*msg_send)(module_t *module, struct message *msg);
	int (*msg_recv)(module_t *module, struct message **msg);
};

#endif
