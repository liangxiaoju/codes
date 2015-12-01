#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/pkt_sched.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
#include <netlink/object-api.h>
#include <netlink-types.h>
#include "klog.h"

#define UNUSE(x) ((void)x)
#define min(a,b) (((a)>(b))?(b):(a))

struct family_data {
	const char *group;
	int id;
};

static int error_handler(struct sockaddr_nl *nla, struct nlmsgerr *err,
		void *arg)
{
	int *ret = arg;
	*ret = err->error;
	UNUSE(nla);

	return NL_STOP;
}

static int finish_handler(struct nl_msg *msg, void *arg)
{
	int *ret = arg;
	*ret = 0;
	UNUSE(msg);

	return NL_SKIP;
}

static int ack_handler(struct nl_msg *msg, void *arg)
{
	int *ret = arg;
	*ret = 0;
	UNUSE(msg);

	return NL_STOP;
}

static int no_seq_check(struct nl_msg *msg, void *arg)
{
	UNUSE(msg);
	UNUSE(arg);

	return NL_OK;
}

static int send_and_recv(klog_t *klog,
		struct nl_sock *nl_sock, struct nl_msg *msg,
		int (*valid_handler)(struct nl_msg *, void *),
		void *valid_data)
{
	struct nl_cb *cb;
	int err = -ENOMEM;

	cb = nl_cb_clone(klog->nl_cb);
	if (!cb)
		goto out;

	err = nl_send_auto_complete(nl_sock, msg);
	if (err < 0)
		goto out;

	err = 1;

	nl_cb_err(cb, NL_CB_CUSTOM, error_handler, &err);
	nl_cb_set(cb, NL_CB_FINISH, NL_CB_CUSTOM, finish_handler, &err);
	nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, ack_handler, &err);

	if (valid_handler)
		nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM,
				valid_handler, valid_data);

	while (err > 0)
		nl_recvmsgs(nl_sock, cb);
out:
	nl_cb_put(cb);
	nlmsg_free(msg);
	return err;
}


int send_and_recv_msgs(klog_t *klog, struct nl_msg *msg,
		int (*valid_handler)(struct nl_msg *, void *),
		void *valid_data)
{
	return send_and_recv(klog, klog->nl, msg, valid_handler,
			valid_data);
}

static int __genl_ctrl_alloc_cache(struct nl_sock *sock, struct nl_cache **result)
{
	int rc = -1;
	int klog_genl_id = -1;
	char sendbuf[sizeof(struct nlmsghdr)+sizeof(struct genlmsghdr)];
	struct nlmsghdr nlmhdr;
	struct genlmsghdr gmhhdr;
	struct iovec sendmsg_iov;
	struct msghdr msg;
	int num_char;
	const int RECV_BUF_SIZE = getpagesize();
	char *recvbuf;
	struct iovec recvmsg_iov;
	int klog_flag = 0, nlm_f_multi = 0, nlmsg_done = 0;
	struct nlmsghdr *nlh;

	/* REQUEST GENERIC NETLINK FAMILY ID */
	/* Message buffer */
	nlmhdr.nlmsg_len = sizeof(sendbuf);
	nlmhdr.nlmsg_type = NETLINK_GENERIC;
	nlmhdr.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK | NLM_F_DUMP;
	nlmhdr.nlmsg_seq = sock->s_seq_next;
	nlmhdr.nlmsg_pid = sock->s_local.nl_pid;

	/* Generic netlink header */
	memset(&gmhhdr, 0, sizeof(gmhhdr));
	gmhhdr.cmd = CTRL_CMD_GETFAMILY;
	gmhhdr.version = CTRL_ATTR_FAMILY_ID;

	/* Combine netlink and generic netlink headers */
	memcpy(&sendbuf[0], &nlmhdr, sizeof(nlmhdr));
	memcpy(&sendbuf[0]+sizeof(nlmhdr), &gmhhdr, sizeof(gmhhdr));

	/* Create IO vector with Netlink message */
	sendmsg_iov.iov_base = &sendbuf;
	sendmsg_iov.iov_len = sizeof(sendbuf);

	/* Socket message */
	msg.msg_name = (void *) &sock->s_peer;
	msg.msg_namelen = sizeof(sock->s_peer);
	msg.msg_iov = &sendmsg_iov;
	msg.msg_iovlen = 1; /* Only sending one iov */
	msg.msg_control = NULL;
	msg.msg_controllen = 0;
	msg.msg_flags = 0;

	/* Send message and verify sent */
	num_char = sendmsg(sock->s_fd, &msg, 0);
	if (num_char == -1)
		return -errno;

	/* RECEIVE GENL CMD RESPONSE */

	/* Create receive iov buffer */
	recvbuf = (char *) malloc(RECV_BUF_SIZE);

	/* Attach to iov */
	recvmsg_iov.iov_base = recvbuf;
	recvmsg_iov.iov_len = RECV_BUF_SIZE;

	msg.msg_iov = &recvmsg_iov;
	msg.msg_iovlen = 1;

	/***************************************************************/
	/* Receive message. If multipart message, keep receiving until */
	/* message type is NLMSG_DONE				       */
	/***************************************************************/

	do {

		int recvmsg_len, nlmsg_rem;

		/* Receive message */
		memset(recvbuf, 0, RECV_BUF_SIZE);
		recvmsg_len = recvmsg(sock->s_fd, &msg, 0);

		/* Make sure receive successful */
		if (recvmsg_len < 0) {
			rc = -errno;
			goto error_recvbuf;
		}

		/* Parse nlmsghdr */
		nlmsg_for_each_msg(nlh, (struct nlmsghdr *) recvbuf, \
				recvmsg_len, nlmsg_rem) {
			struct nlattr *nla;
			int nla_rem;

			/* Check type */
			switch (nlh->nlmsg_type) {
				case NLMSG_DONE:
					goto return_genl_id;
					break;
				case NLMSG_ERROR:

					/* Should check nlmsgerr struct received */
					fprintf(stderr, "Receive message error\n");
					goto error_recvbuf;
				case NLMSG_OVERRUN:
					fprintf(stderr, "Receive data partly lost\n");
					goto error_recvbuf;
				case NLMSG_MIN_TYPE:
				case NLMSG_NOOP:
					break;
				default:
					break;
			}



			/* Check flags */
			if (nlh->nlmsg_flags & NLM_F_MULTI)
				nlm_f_multi = 1;
			else
				nlm_f_multi = 0;

			if (nlh->nlmsg_type & NLMSG_DONE)
				nlmsg_done = 1;
			else
				nlmsg_done = 0;

			/* Iteratve over attributes */
			nla_for_each_attr(nla,
					nlmsg_attrdata(nlh, GENL_HDRLEN),
					nlmsg_attrlen(nlh, GENL_HDRLEN),
					nla_rem){

				/* If this family is klog */
				if (nla->nla_type == CTRL_ATTR_FAMILY_NAME &&
						!strcmp((char *)nla_data(nla),
							"klog"))
					klog_flag = 1;

				/* Save the family id */
				else if (klog_flag &&
						nla->nla_type == CTRL_ATTR_FAMILY_ID) {
					klog_genl_id =
						*((int *)nla_data(nla));
					klog_flag = 0;
				}

			}

		}

	} while (nlm_f_multi && !nlmsg_done);

return_genl_id:
	/* Return family id as cache pointer */
	*result = (struct nl_cache *) klog_genl_id;
	rc = 0;
error_recvbuf:
	free(recvbuf);
error:
	return rc;
}

struct genl_family *__genl_ctrl_search_by_name(struct nl_cache *cache, \
		const char *name)
{
	struct genl_family *gf = (struct genl_family *) \
							 malloc(sizeof(struct genl_family));
	if (!gf)
		goto fail;
	memset(gf, 0, sizeof(*gf));

	/* Add ref */
	gf->ce_refcnt++;

	/* Overriding cache pointer as family id for now */
	gf->gf_id = (uint16_t) ((uint32_t) cache);
	strncpy(gf->gf_name, name, GENL_NAMSIZ);

	return gf;
fail:
	return NULL;

}

static int family_handler(struct nl_msg *msg, void *arg)
{
	struct family_data *res = arg;
	struct nlattr *tb[CTRL_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct nlattr *mcgrp;
	int i;

	nla_parse(tb, CTRL_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
			genlmsg_attrlen(gnlh, 0), NULL);
	if (!tb[CTRL_ATTR_MCAST_GROUPS])
		return NL_SKIP;

	nla_for_each_nested(mcgrp, tb[CTRL_ATTR_MCAST_GROUPS], i) {
		struct nlattr *tb2[CTRL_ATTR_MCAST_GRP_MAX + 1];
		nla_parse(tb2, CTRL_ATTR_MCAST_GRP_MAX, nla_data(mcgrp),
				nla_len(mcgrp), NULL);
		if (!tb2[CTRL_ATTR_MCAST_GRP_NAME] ||
				!tb2[CTRL_ATTR_MCAST_GRP_ID] ||
				strncmp(nla_data(tb2[CTRL_ATTR_MCAST_GRP_NAME]),
					res->group,
					nla_len(tb2[CTRL_ATTR_MCAST_GRP_NAME])) != 0)
			continue;
		res->id = nla_get_u32(tb2[CTRL_ATTR_MCAST_GRP_ID]);
		break;
	};

	return NL_SKIP;
}

static int nl_get_multicast_id(klog_t *klog, const char *family, const char *group)
{
	struct nl_msg *msg;
	int ret = -1;
	struct family_data res = { group, -ENOENT };

	msg = nlmsg_alloc();
	if (!msg)
		return -ENOMEM;
	genlmsg_put(msg, 0, 0, genl_ctrl_resolve(klog->nl, "nlctrl"),
			0, 0, CTRL_CMD_GETFAMILY, 0);
	NLA_PUT_STRING(msg, CTRL_ATTR_FAMILY_NAME, family);

	ret = send_and_recv_msgs(klog, msg, family_handler, &res);
	msg = NULL;
	if (ret == 0)
		ret = res.id;

nla_put_failure:
	nlmsg_free(msg);
	return ret;
}

static int genl_init(klog_t *klog)
{
	int err, fd;

	klog->nl_cb = nl_cb_alloc(NL_CB_DEFAULT);
	if (!klog->nl_cb) {
		fprintf(stderr, "Failed to allocate nl callback.\n");
		return -ENOMEM;
	}

	/* nl */
	klog->nl = nl_socket_alloc();
	if (!klog->nl) {
		fprintf(stderr, "Failed to allocate netlink socket.\n");
		err = -ENOMEM;
		goto out;
	}

	if (genl_connect(klog->nl)) {
		fprintf(stderr, "Failed to connect to nl.\n");
		err = -ENOLINK;
		goto out;
	}

	/* nl_event */
	klog->nl_event = nl_socket_alloc();
	if (!klog->nl_event) {
		fprintf(stderr, "Failed to allocate netlink socket.\n");
		err = -ENOMEM;
		goto out;
	}

	if (genl_connect(klog->nl_event)) {
		fprintf(stderr, "Failed to connect to nl.\n");
		err = -ENOLINK;
		goto out;
	}

	fd = nl_socket_get_fd(klog->nl_event);
	fcntl(fd, F_SETFD, FD_CLOEXEC);
	fcntl(fd, F_SETFL, O_NONBLOCK);

	/* find nl family id */
	if (__genl_ctrl_alloc_cache(klog->nl, &klog->nl_cache)) {
		fprintf(stderr, "Failed to allocate genl cache.\n");
		err = -ENOMEM;
		goto out;
	}

	klog->family = __genl_ctrl_search_by_name(klog->nl_cache, "klog");
	if (!klog->family) {
		fprintf(stderr, "klog family not found.\n");
		err = -ENOENT;
		goto out;
	}

	return 0;

out:
	if (klog->nl_cb)
		nl_cb_put(klog->nl_cb);
	if (klog->nl_cache)
		nl_cache_free(klog->nl_cache);
	if (klog->nl)
		nl_socket_free(klog->nl);
	if (klog->nl_event)
		nl_socket_free(klog->nl_event);
	return err;
}

int kinfo_init(klog_t *klog)
{
	int err = 0, i;

	for (i = 0; i < klog->num; i++) {
		kinfo_t *kinfo = klog->kinfo[i];

		if (!kinfo->group)
			continue;

		err = nl_get_multicast_id(klog, "klog", kinfo->group);
		if (err >= 0)
			err = nl_socket_add_membership(klog->nl_event, err);
		else
			fprintf(stderr, "Failed to add multicast.\n");

		fprintf(stdout, "add multicast [%s].\n", kinfo->group);
		fprintf(stdout, "open log file [%s].\n", kinfo->file);
	}

	return err;
}

int recv_msgs(klog_t *klog, struct nl_sock *nl_sock,
		int (*valid_handler)(struct nl_msg *, void *),
		void *valid_data)
{
	struct nl_cb *cb;
	int err = 0;

	cb = nl_cb_clone(klog->nl_cb);
	if (!cb) {
		err = -ENOMEM;
		goto out;
	}

	nl_cb_set(cb, NL_CB_SEQ_CHECK, NL_CB_CUSTOM, no_seq_check, NULL);

	if (valid_handler)
		nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM,
				valid_handler, valid_data);

	nl_recvmsgs(nl_sock, cb);

out:
	nl_cb_put(cb);
	return err;
}

int klog_recv_msg_handler(struct nl_msg *m, void *arg)
{
	struct nlattr *tb[NL_KLOG_ATTR_MAX+1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(m));
	struct klog_msg msg;
	klog_t *klog = arg;
	int len, i;

	nla_parse(tb, NL_KLOG_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
			genlmsg_attrlen(gnlh, 0), NULL);

	for (i = 0; i <= NL_KLOG_ATTR_MAX; i++) {
		if (tb[i]) {
			memset(&msg, 0, sizeof(msg));
			len = min(nla_len(tb[i]), (int)sizeof(msg));
			memcpy(&msg, nla_data(tb[i]), len);
			if (klog->custom_cb(&msg, klog->custom_arg) < 0)
				break;
		}
	}

	return NL_SKIP;
}

/* API */

klog_t *klog_alloc(kinfo_t **kinfo, int n)
{
	klog_t *klog;

	klog = malloc(sizeof(*klog));
	if (!klog)
		return NULL;

	klog->kinfo = kinfo;
	klog->num = n;
	return klog;
}

void klog_free(klog_t *klog)
{
	free(klog);
}

int klog_init(klog_t *klog)
{
	fprintf(stdout, "klog init.\n");

	genl_init(klog);

	kinfo_init(klog);

	return 0;
}

int default_custom_msg_handler(struct klog_msg *msg, void *arg)
{
	klog_t *klog = arg;
	int i;

	for (i = 0; i < klog->num; i++) {
		kinfo_t *kinfo = klog->kinfo[i];
		if (strcmp(msg->label, kinfo->group) == 0)
			kinfo->handler(msg, kinfo);
	}

	return 0;
}

int klog_set_handler(klog_t *klog, klog_msg_handler_t cb, void *arg)
{
	klog->custom_cb = cb;
	klog->custom_arg = arg;
	return 0;
}

int klog_event_handle(klog_t *klog)
{
	return recv_msgs(klog, klog->nl_event, klog_recv_msg_handler, klog);
}

void klog_loop(klog_t *klog)
{
	struct pollfd ufd;
	int nr;

	ufd.events = POLLIN;
	ufd.fd = nl_socket_get_fd(klog->nl_event);

	do {
		ufd.revents = 0;
		nr = poll(&ufd, 1, -1);
		if (nr <= 0)
			continue;
		if (ufd.revents & POLLIN) {
			klog_event_handle(klog);
		}
	} while (1);
}

