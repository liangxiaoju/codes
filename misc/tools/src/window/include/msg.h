#ifndef __MSG_H__
#define __MSG_H__

#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif

#define container_of(ptr, type, member)						\
({		      												\
	const typeof(((type *)0)->member)*__mptr = (ptr);    	\
	(type *)((char *)__mptr - offsetof(type, member));		\
 })

struct list {
	struct list *prev;
	struct list *next;
};

static inline void init_list(struct list *node)
{
	node->prev = node;
	node->next = node;
}

static inline void _list_add(struct list *new, struct list *prev, struct list *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

static inline void list_add(struct list *new, struct list *head)
{
	_list_add(new, head->prev, head);
}

static inline void _list_del(struct list *prev, struct list *next)
{
	next->prev = prev;
	prev->next = next;
}

static inline void list_del(struct list *node)
{
	_list_del(node->prev, node->next);
	node->prev = node;
	node->next = node;
}

static inline int list_empty(struct list *head)
{
	return head->next == head;
}

struct message {
	struct list list;

	const char *from;
	const char *to;

	union {
		char *msg;
		struct {
			void *unused;
		};
	};
};

#define to_msg(ptr) container_of(ptr, struct message, list)

static inline void msg_init(struct message *node)
{
	init_list(&node->list);
}

static inline void msg_add(struct message *new, struct message *head)
{
	list_add(&new->list, &head->list);
}

static inline void msg_del(struct message *node)
{
	list_del(&node->list);
}

static inline int msg_empty(struct message *head)
{
	return list_empty(&head->list);
}

static inline int msg_send(module_t *module, struct message *message)
{
	msg_add(message, module->msg_list);
	return 0;
}

static inline int msg_recv(module_t *module, struct message **message)
{
	struct list *head = &(module->msg_list->list);
	struct list *ptr;
	struct message *msg;

	for (ptr = head->next; ptr != head; ptr = ptr->next) {
		msg = to_msg(ptr);
		if (!strcmp(module->name, msg->to)) {
			msg_del(msg);
			*message = msg;
			return 1;
		}
	}
	return 0;
}

static inline struct message *mkmsg(const char *from, const char *to, char *msg, void *ptr)
{
	struct message *message;

	message = malloc(sizeof(struct message));
	if (!message)
		return NULL;

	init_list(&message->list);

	message->from = strdup(from);
	message->to = strdup(to);
	message->msg = strdup(msg);

	return message;
}

static inline void freemsg(struct message *message)
{
	free((void *)(message->from));
	free((void *)(message->to));
	free((void *)(message->msg));
	free(message);
}

#endif
