#include <linux/init.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/printk.h>
#include <linux/rbtree.h>
#include <linux/rwsem.h>
#include <net/genetlink.h>

#include "imtsrv.h"

struct d_node {
    struct rb_node node;
    char *key;
    char *value;
};

struct imtsrv {
    struct rb_root tree;
    struct rw_semaphore sem;
};

static struct imtsrv *g_imtsrv;

static int imtsrv_pre_doit(struct genl_ops *, struct sk_buff *, struct genl_info *);
static void imtsrv_post_doit(struct genl_ops *, struct sk_buff *, struct genl_info *);

static struct genl_family imtsrv_fam = {
    .id = GENL_ID_GENERATE,	/* don't bother with a hardcoded ID */
    .name = IMTSRV_GENL_NAME,	/* have users key off the name instead */
    .hdrsize = 0,		/* no private header */
    .version = 1,		/* no particular meaning now */
    .maxattr = IMTSRV_ATTR_MAX,
    .netnsok = true,
    .pre_doit = imtsrv_pre_doit,
    .post_doit = imtsrv_post_doit,
};

static const struct nla_policy imtsrv_policy[] = {
    [IMTSRV_ATTR_WIFI_VERSION] = { .type = NLA_STRING },
    [IMTSRV_ATTR_WIFI_TYPE] = { .type = NLA_STRING },
};

static struct d_node *search_node(struct rb_root *root, const char *key)
{
    struct rb_node *node = root->rb_node;

    while (node) {
        struct d_node *d = rb_entry(node, struct d_node, node);
        int result = strcmp(key, d->key);

        if (result < 0)
            node = node->rb_left;
        else if (result > 0)
            node = node->rb_right;
        else
            return d;
    }
    return NULL;
}

static int insert_node(struct rb_root *root, struct d_node *d)
{
    struct rb_node **new = &(root->rb_node), *parent = NULL;

    /* Figure out where to put new node */
    while (*new) {
        struct d_node *this = rb_entry(*new, struct d_node, node);
        int result = strcmp(d->key, this->key);

        parent = *new;
        if (result < 0)
            new = &((*new)->rb_left);
        else if (result > 0)
            new = &((*new)->rb_right);
        else
            return -EEXIST;
    }

    /* Add new node and rebalance tree */
    rb_link_node(&d->node, parent, new);
    rb_insert_color(&d->node, root);

    return 0;
}

/*
static void erase_node(struct rb_root *root, struct d_node *d)
{
    rb_erase(&d->node, root);
    kfree(d->key);
    kfree(d->value);
    kfree(d);
}
*/

static int imtsrv_get(struct imtsrv *imtsrv, const char *key, char *value, int len)
{
    struct d_node *d;
    int ret = 0;

    down_read(&imtsrv->sem);
    d = search_node(&imtsrv->tree, key);
    if (!d)
        goto out;

    ret = snprintf(value, len, "%s", d->value);

out:
    up_read(&imtsrv->sem);
    return ret;
}

static int imtsrv_set(struct imtsrv *imtsrv, const char *key, char *value)
{
    struct d_node *d;
    int ret = 0;

    down_write(&imtsrv->sem);
    d = search_node(&imtsrv->tree, key);
    up_write(&imtsrv->sem);
    if (!d) {
        d = kzalloc(sizeof(struct d_node), GFP_KERNEL);
        if (!d) {
            ret = -ENOMEM;
            goto out;
        }
        d->key = kasprintf(GFP_KERNEL, "%s", key);
        d->value = kasprintf(GFP_KERNEL, "%s", value);
        down_write(&imtsrv->sem);
        ret = insert_node(&imtsrv->tree, d);
        up_write(&imtsrv->sem);
    } else {
        down_write(&imtsrv->sem);
        kfree(d->value);
        d->value = kasprintf(GFP_KERNEL, "%s", value);
        up_write(&imtsrv->sem);
    }

out:
    return ret;
}

static int imtsrv_fill_info(struct sk_buff *msg, struct genl_info *info,
        const char *key, int attr, int cmd)
{
    struct imtsrv *imtsrv = info->user_ptr[0];
    char value[256] = {0};
    char *hdr;

    hdr = genlmsg_put_reply(msg, info, &imtsrv_fam, 0, cmd);
    if (hdr == NULL)
        return -1;

    imtsrv_get(imtsrv, key, value, sizeof(value));

    if (nla_put_string(msg, attr, value))
        goto nla_put_failure;

    return genlmsg_end(msg, hdr);

nla_put_failure:
    genlmsg_cancel(msg, hdr);
    return -EMSGSIZE;
}

static int imtsrv_get_wifi_version(struct sk_buff *skb, struct genl_info *info)
{
    struct sk_buff *msg;
    int err;

    msg = genlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL);
    if (msg == NULL)
        return -1;

    err = imtsrv_fill_info(msg, info, "WIFI VERSION",
            IMTSRV_ATTR_WIFI_VERSION, IMTSRV_CMD_GET_WIFI_VERSION);
    if (err < 0) {
        nlmsg_free(msg);
        return -1;
    }

    return genlmsg_reply(msg, info);
}

static int imtsrv_set_wifi_version(struct sk_buff *skb, struct genl_info *info)
{
    struct imtsrv *imtsrv = info->user_ptr[0];
    char *value = NULL;

    if (info->attrs[IMTSRV_ATTR_WIFI_VERSION])
        value = nla_data(info->attrs[IMTSRV_ATTR_WIFI_VERSION]);

    imtsrv_set(imtsrv, "WIFI VERSION", value);

    return 0;
}

static int imtsrv_get_wifi_type(struct sk_buff *skb, struct genl_info *info)
{
    struct sk_buff *msg;
    int err;

    msg = genlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL);
    if (msg == NULL)
        return -1;

    err = imtsrv_fill_info(msg, info, "WIFI TYPE",
            IMTSRV_ATTR_WIFI_TYPE, IMTSRV_CMD_GET_WIFI_TYPE);
    if (err < 0) {
        nlmsg_free(msg);
        return -1;
    }

    return genlmsg_reply(msg, info);
}

static int imtsrv_set_wifi_type(struct sk_buff *skb, struct genl_info *info)
{
    struct imtsrv *imtsrv = info->user_ptr[0];
    char *value = NULL;

    if (info->attrs[IMTSRV_ATTR_WIFI_TYPE])
        value = nla_data(info->attrs[IMTSRV_ATTR_WIFI_TYPE]);

    imtsrv_set(imtsrv, "WIFI TYPE", value);

    return 0;
}

static int imtsrv_pre_doit(struct genl_ops *ops,
        struct sk_buff *skb, struct genl_info *info)
{
    info->user_ptr[0] = g_imtsrv;
    return 0;
}

static void imtsrv_post_doit(struct genl_ops *ops,
        struct sk_buff *skb, struct genl_info *info)
{
}

static struct genl_ops imtsrv_ops[] = {
    {
        .cmd = IMTSRV_CMD_GET_WIFI_VERSION,
        .doit = imtsrv_get_wifi_version,
        .policy = imtsrv_policy,
    },
    {
        .cmd = IMTSRV_CMD_SET_WIFI_VERSION,
        .doit = imtsrv_set_wifi_version,
        .policy = imtsrv_policy,
    },
    {
        .cmd = IMTSRV_CMD_GET_WIFI_TYPE,
        .doit = imtsrv_get_wifi_type,
        .policy = imtsrv_policy,
    },
    {
        .cmd = IMTSRV_CMD_SET_WIFI_TYPE,
        .doit = imtsrv_set_wifi_type,
        .policy = imtsrv_policy,
    },
};

int imtsrv_genl_init(void)
{
    struct imtsrv *imtsrv;
    int err;

    imtsrv = kzalloc(sizeof(struct imtsrv), GFP_KERNEL);
    if (!imtsrv)
        return -ENOMEM;

    init_rwsem(&imtsrv->sem);

    g_imtsrv = imtsrv;

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 13, 0)
    err = genl_register_family_with_ops(&imtsrv_fam,
            imtsrv_ops, ARRAY_SIZE(imtsrv_ops));
#else
    err = genl_register_family_with_ops(&imtsrv_fam, imtsrv_ops);
#endif
    if (err)
        return err;

    return 0;
}
subsys_initcall(imtsrv_genl_init);

void imtsrv_genl_exit(void)
{
    genl_unregister_family(&imtsrv_fam);
}
module_exit(imtsrv_genl_exit);

MODULE_LICENSE("GPL");
