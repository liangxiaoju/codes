#include <netlink/cli/utils.h>
#include "imtsrv.h"

#define ARRAY_SIZE(X) (sizeof(X) / sizeof((X)[0]))

struct wifi_info {
    char version[16];
    char type[64];
};

struct imtsrv_genl {
    struct nl_sock *sock;
    struct genl_ops *ops;
};

static struct nla_policy imtsrv_policy[] = {
    [IMTSRV_ATTR_WIFI_VERSION] = { .type = NLA_STRING },
    [IMTSRV_ATTR_WIFI_TYPE] = { .type = NLA_STRING },
};

static struct wifi_info wifi_info;

static int parse_cmd_wifi_info(struct nl_cache_ops *unused, struct genl_cmd *cmd,
        struct genl_info *info, void *arg)
{
    struct wifi_info *wifi = &wifi_info;
    char *value = NULL;

    if (info->attrs[IMTSRV_ATTR_WIFI_VERSION]) {
        value = nla_data(info->attrs[IMTSRV_ATTR_WIFI_VERSION]);
        strncpy(wifi->version, value, sizeof(wifi->version));
    }

    if (info->attrs[IMTSRV_ATTR_WIFI_TYPE]) {
        value = nla_data(info->attrs[IMTSRV_ATTR_WIFI_TYPE]);
        strncpy(wifi->type, value, sizeof(wifi->type));
    }

    return 0;
}

static int parse_cb(struct nl_msg *msg, void *arg)
{
	return genl_handle_msg(msg, arg);
}

static struct genl_cmd imtsrv_cmds[] = {
    {
        .c_id = IMTSRV_CMD_GET_WIFI_VERSION,
        .c_name = "imtsrv_get_wifi_version()",
        .c_maxattr = IMTSRV_ATTR_MAX,
        .c_attr_policy = imtsrv_policy,
        .c_msg_parser = &parse_cmd_wifi_info,
    },
    {
        .c_id = IMTSRV_CMD_GET_WIFI_TYPE,
        .c_name = "imtsrv_get_wifi_type()",
        .c_maxattr = IMTSRV_ATTR_MAX,
        .c_attr_policy = imtsrv_policy,
        .c_msg_parser = &parse_cmd_wifi_info,
    },
};

static struct genl_ops ops = {
    .o_name = IMTSRV_GENL_NAME,
    .o_cmds = imtsrv_cmds,
    .o_ncmds = ARRAY_SIZE(imtsrv_cmds),
};

static int imtsrv_set_str(struct imtsrv_genl *imtsrv, int cmd, int attr, const char *value)
{
    struct nl_msg *msg;
    void *hdr;
    int err;
    struct nl_sock *sock = imtsrv->sock;

    msg = nlmsg_alloc();
    if (msg == NULL)
		nl_cli_fatal(NLE_NOMEM, "Unable to allocate netlink message");

    hdr = genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ, imtsrv->ops->o_id,
            0, 0, cmd, 1);
	if (hdr == NULL)
		nl_cli_fatal(ENOMEM, "Unable to write genl header");

	if ((err = nla_put_string(msg, attr, value)) < 0)
		nl_cli_fatal(err, "Unable to add attribute: %s", nl_geterror(err));

	if ((err = nl_send_auto_complete(sock, msg)) < 0)
		nl_cli_fatal(err, "Unable to send message: %s", nl_geterror(err));

	nlmsg_free(msg);

	if ((err = nl_wait_for_ack(sock)) < 0)
		nl_cli_fatal(err, "Unable to wait ack: %s", nl_geterror(err));

    return 0;
}

static int imtsrv_set_wifi_version(struct imtsrv_genl *imtsrv, const char *value)
{
    return imtsrv_set_str(imtsrv, IMTSRV_CMD_SET_WIFI_VERSION, IMTSRV_ATTR_WIFI_VERSION, value);
}

static int imtsrv_set_wifi_type(struct imtsrv_genl *imtsrv, const char *value)
{
    return imtsrv_set_str(imtsrv, IMTSRV_CMD_SET_WIFI_TYPE, IMTSRV_ATTR_WIFI_TYPE, value);
}

static int imtsrv_get_str(struct imtsrv_genl *imtsrv, int cmd)
{
    struct nl_sock *sock = imtsrv->sock;
    struct nl_msg *msg;
    void *hdr;
    int err;

    msg = nlmsg_alloc();
    if (msg == NULL)
		nl_cli_fatal(NLE_NOMEM, "Unable to allocate netlink message");

    hdr = genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ, imtsrv->ops->o_id,
            0, 0, cmd, 1);
	if (hdr == NULL)
		nl_cli_fatal(ENOMEM, "Unable to write genl header");

	if ((err = nl_send_auto_complete(sock, msg)) < 0)
		nl_cli_fatal(err, "Unable to send message: %s", nl_geterror(err));

	nlmsg_free(msg);

	if ((err = nl_socket_modify_cb(sock, NL_CB_VALID, NL_CB_CUSTOM,
			parse_cb, imtsrv)) < 0)
		nl_cli_fatal(err, "Unable to modify valid message callback");

	if ((err = nl_recvmsgs_default(sock)) < 0)
		nl_cli_fatal(err, "Unable to receive message: %s", nl_geterror(err));

	if ((err = nl_wait_for_ack(sock)) < 0)
		nl_cli_fatal(err, "Unable to wait ack: %s", nl_geterror(err));

    return 0;
}

static int imtsrv_get_wifi_version(struct imtsrv_genl *imtsrv, char *value, int len)
{
    imtsrv_get_str(imtsrv, IMTSRV_CMD_GET_WIFI_VERSION);
    strncpy(value, wifi_info.version, len);
    return 0;
}

static int imtsrv_get_wifi_type(struct imtsrv_genl *imtsrv, char *value, int len)
{
    imtsrv_get_str(imtsrv, IMTSRV_CMD_GET_WIFI_TYPE);
    strncpy(value, wifi_info.type, len);
    return 0;
}

int imtsrv_genl_init(struct imtsrv_genl *imtsrv)
{
    struct nl_sock *sock;
    int err;

    imtsrv->ops = &ops;
    imtsrv->sock = sock = nl_cli_alloc_socket();
    nl_socket_enable_auto_ack(sock);
    nl_cli_connect(sock, NETLINK_GENERIC);

    if ((err = genl_register_family(&ops)) < 0)
        nl_cli_fatal(err, "Unable to register Generic Netlink family");

    if ((err = genl_ops_resolve(sock, &ops)) < 0)
        nl_cli_fatal(err, "Unable to resolve family name");

    if (genl_ctrl_resolve(sock, "nlctrl") != GENL_ID_CTRL)
        nl_cli_fatal(NLE_INVAL, "Resolving of \"nlctrl\" failed");

    return 0;
}

void imtsrv_genl_exit(struct imtsrv_genl *imtsrv)
{
    struct nl_sock *sock = imtsrv->sock;

	nl_close(sock);
	nl_socket_free(sock);
}

int main(int argc, char *argv[])
{
    struct imtsrv_genl *imtsrv;
    char value[128];
    void *hdr;
    int err;

    imtsrv = malloc(sizeof(*imtsrv));
    if (imtsrv == NULL)
        return -1;

    imtsrv_genl_init(imtsrv);

    imtsrv_set_wifi_version(imtsrv, "1.0");
    imtsrv_get_wifi_version(imtsrv, value, sizeof(value));
    printf("WIFI VERSION: %s\n", value);

    imtsrv_set_wifi_type(imtsrv, "wifi-1");
    imtsrv_get_wifi_type(imtsrv, value, sizeof(value));
    printf("WIFI TYPE: %s\n", value);

    imtsrv_genl_exit(imtsrv);

	return 0;
}

