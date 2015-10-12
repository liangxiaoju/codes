#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "unpack.h"

typedef struct {
    char name[32];
    char file[64];
    unsigned int offset;
    unsigned int flash_offset;
    unsigned int userspace;
    unsigned int size;
} RKIMAGE_ITEM;

typedef struct {
    unsigned int tag;
    unsigned int size;
    char machine_model[64];
    char manufacturer[60];
    unsigned int version;
    int item_count;
    RKIMAGE_ITEM item[16];
} RKIMAGE_HDR;

typedef struct {
    RKIMAGE_HDR hdr;
    unsigned int fwoffset;
} PRIV_INFO;

void usage(void)
{
    fprintf(stderr, "./unpack \
            \n\t<-i input> # update.img \
            \n\t[-o output] # output filename \
            \n\t[-v] # dump info \
            \n\t[-c] # check crc of update.img \
            \n\t[-a] # unpack all partitions [bootloader, kernel, boot, system, ...] \
            \n\t[-p partition] # unpack which partition \
            \n");
}

void dump_hdr(RKIMAGE_HDR *hdr)
{
    int i;
    RKIMAGE_ITEM *item;

    fprintf(stderr, "tag: %d\n", hdr->tag);
    fprintf(stderr, "size: %d\n", hdr->size);
    fprintf(stderr, "machine_model: %s\n", hdr->machine_model);
    fprintf(stderr, "manufacturer: %s\n", hdr->manufacturer);
    fprintf(stderr, "version: %d\n", hdr->version);
    fprintf(stderr, "item_count: %d\n", hdr->item_count);
    for (i = 0; i < hdr->item_count; i++) {

        item = &hdr->item[i];

        fprintf(stderr, "#%d:\n", i);
        fprintf(stderr, "\tname=%s\n", item->name);
        fprintf(stderr, "\tfile=%s\n", item->file);
        fprintf(stderr, "\toffset=%d\n", item->offset);
        fprintf(stderr, "\tflash_offset=%d\n", item->flash_offset);
        fprintf(stderr, "\tuserspace=%d\n", item->userspace);
        fprintf(stderr, "\tsize=%d\n", item->size);
    }
}

RKIMAGE_ITEM *find_item(RKIMAGE_HDR *hdr, const char *name)
{
    int i;

    for (i = 0; i < hdr->item_count; i++) {
        if (!strcmp(hdr->item[i].name, name))
            return &hdr->item[i];
    }

    return NULL;
}

int get_updateimg_info(int fd, PRIV_INFO *info)
{
    int i, ret, offset = 0;
    char buf[512];
    RKIMAGE_HDR *hdr = &info->hdr;

    memset(buf, 0, sizeof(buf));

    ret = read(fd, buf, sizeof(buf));
    if (ret != sizeof(buf)) {
        perror("read");
        return -1;
    }

    if (*((unsigned int *)buf) == 0x57464b52) {
        offset = *(unsigned int *)(buf+0x21);
        info->fwoffset = offset;
    }

    lseek(fd, offset, SEEK_SET);
    ret = read(fd, hdr, sizeof(*hdr));
    if (ret != sizeof(*hdr)) {
        perror("read");
        return -1;
    }

    if (!strcasestr(hdr->machine_model, "rk")) {
        fprintf(stderr, "Error: illegal file.\n");
        return -1;
    }

    for (i = 0; i < hdr->item_count; i++) {
        hdr->item[i].offset += offset;
    }

    return 0;
}

int check_updateimg_crc(int fd, PRIV_INFO *info)
{
    char buf[16*1024];
    unsigned int crc_calc = 0, crc_read = 0;
    int read_count = 0;
    RKIMAGE_HDR *hdr = &info->hdr;
    unsigned int remain = hdr->size;

    lseek(fd, info->fwoffset, SEEK_SET);

    /* calc crc */
    while (remain > 0) {
        read_count = remain > sizeof(buf) ? sizeof(buf) : remain;
        if (read(fd, buf, read_count) != read_count) {
            perror("read");
            return -1;
        }
        remain -= read_count;
        crc_calc = CRC_32_NEW((unsigned char *)buf, read_count, crc_calc);
    }

    /* read crc */
    if (read(fd, &crc_read, 4) != 4) {
        perror("read");
        return -1;
    }

    if (crc_calc != crc_read) {
        fprintf(stderr, "crc not match.\n");
        return -1;
    }

    return 0;
}

int unpack(int ifd, int ofd, RKIMAGE_ITEM *item)
{
    unsigned int bytes;
    size_t size = item->size;
    off_t offset = item->offset;

    bytes = sendfile(ofd, ifd, &offset, size);
    if (bytes != item->size) {
        perror("sendfile");
        return -1;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    int opt;
    int unpackall = 0, dump = 0, check = 0;
    int ifd, ofd;
    char *input = NULL, *output = NULL;
    char *partition = NULL;
    PRIV_INFO info;
    RKIMAGE_HDR *hdr = &info.hdr;

    while ((opt = getopt(argc, argv, "i:o:p:avch")) != -1) {
        switch (opt) {
            case 'i':
                input = strdup(optarg);
                break;
            case 'o':
                output = strdup(optarg);
                break;
            case 'p':
                partition = strdup(optarg);
                break;
            case 'a':
                unpackall = 1;
                break;
            case 'v':
                dump = 1;
                break;
            case 'c':
                check = 1;
                break;
            case 'h':
            default:
                usage();
                return 0;
        }
    }

    memset(&info, 0, sizeof(info));

    ifd = open(input, O_RDONLY);
    if (ifd <= 0) {
        perror("open");
        return -1;
    }

    ofd = open(output, O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    if (ofd <= 0)
        ofd = STDOUT_FILENO;

    if (get_updateimg_info(ifd, &info)) {
        perror("get_updateimg_info");
        return -1;
    }

    if (check) {
        if (check_updateimg_crc(ifd, &info)) {
            perror("check_updateimg_crc");
            return -1;
        }
    }

    if (dump)
        dump_hdr(hdr);

    if (unpackall) {
        int i;
        char name[512];
        for (i = 0; i < hdr->item_count; i++) {
            sprintf(name, "%s.img", hdr->item[i].name);
            ofd = open(name, O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
            if (ofd <= 0) {
                perror("open");
                return -1;
            }
            unpack(ifd, ofd, &hdr->item[i]);
        }
    } else if (partition) {
        RKIMAGE_ITEM *item = find_item(hdr, partition);
        if (item)
            unpack(ifd, ofd, item);
    }

    close(ifd);

    if (ofd != STDOUT_FILENO)
        close(ofd);

    return 0;
}
