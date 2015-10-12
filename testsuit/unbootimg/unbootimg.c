#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "bootimg.h"

void dump_hdr(boot_img_hdr *hdr)
{
    int i;
    unsigned char magic[BOOT_MAGIC_SIZE+1];

    memcpy(magic, hdr->magic, BOOT_MAGIC_SIZE);
    magic[BOOT_MAGIC_SIZE] = '\0';

    printf("===== HEADER =====\n");
    printf("magic:\t\t\t%s\n", magic);
    printf("kernel size:\t\t%u\n", hdr->kernel_size);
    printf("kernel addr:\t\t0x%x\n", hdr->kernel_addr);
    printf("ramdisk size:\t\t%u\n", hdr->ramdisk_size);
    printf("ramdisk addr:\t\t0x%x\n", hdr->ramdisk_addr);
    printf("second size:\t\t%u\n", hdr->second_size);
    printf("second addr:\t\t0x%x\n", hdr->second_addr);
    printf("tags addr:\t\t0x%x\n", hdr->tags_addr);
    printf("page size:\t\t%u\n", hdr->page_size);
    printf("dt size:\t\t%u\n", hdr->dt_size);
    printf("product name:\t\t%s\n", hdr->name);
    printf("cmdline: %s\n", hdr->cmdline);
    printf("id:");
    for (i = 0; i < sizeof(hdr->id)/sizeof(hdr->id[0]); i++) {
        printf(" 0x%x", hdr->id[i]);
    }
    printf("\n");
    printf("==================\n");
}

unsigned padding_offset(unsigned offset, unsigned pagesize)
{
    unsigned pagemask = pagesize - 1;
    unsigned count;

    if ((offset & pagemask) == 0)
        count = 0;
    else
        count = pagesize - (offset & pagemask);

    return offset + count;
}

int extract(int in_fd, boot_img_hdr *hdr, const char *path, const char *label)
{
    int out_fd;
    unsigned offset = 0, size = 0;

    if (strcmp(label, "kernel") == 0) {

        offset = padding_offset(sizeof(boot_img_hdr), hdr->page_size);

        size = hdr->kernel_size;

    } else if (strcmp(label, "ramdisk") == 0) {

        offset = padding_offset(sizeof(boot_img_hdr), hdr->page_size) +
            padding_offset(hdr->kernel_size, hdr->page_size);

        size = hdr->ramdisk_size;

    } else if (strcmp(label, "second") == 0) {

        offset = padding_offset(sizeof(boot_img_hdr), hdr->page_size) +
            padding_offset(hdr->kernel_size, hdr->page_size) +
            padding_offset(hdr->ramdisk_size, hdr->page_size);

        size = hdr->second_size;

    } else if (strcmp(label, "dt") == 0) {

        offset = padding_offset(sizeof(boot_img_hdr), hdr->page_size) +
            padding_offset(hdr->kernel_size, hdr->page_size) +
            padding_offset(hdr->ramdisk_size, hdr->page_size) +
			padding_offset(hdr->second_size, hdr->page_size);

        size = hdr->dt_size;

	}

    if (size == 0)
        return 0;

    if (lseek(in_fd, offset, SEEK_SET) < 0) {
        perror("lseek");
        return -1;
    }

    out_fd = open(path, O_CREAT|O_TRUNC|O_RDWR, 0644);
    if (out_fd <= 0) {
        perror("open");
        return -1;
    }

    if (sendfile(out_fd, in_fd, NULL, size) < 0) {
        perror("sendfile");
        return -1;
    }

    close(out_fd);

    return 0;
}

void usage(void)
{
    printf("usage: extract [kernel ramdisk second] from bootimg\n");
    printf("\t-i <input bootimg filename>\n");
    printf("\t-k <output kernel filename>\n");
    printf("\t-r <output ramdisk filename>\n");
    printf("\t-s <output second filename>\n");
    printf("\t-t <output dt filename>\n");
    printf("\t-d dump header\n");
    printf("\t-h help\n");
}

int main(int argc, char *argv[])
{
    boot_img_hdr hdr;
    char *bootimg, *kernel = NULL, *ramdisk = NULL, *second = NULL, *dt = NULL;
    int bootfd;
    int pagesize;
    int opt;
    int dump = 0;

    if (argc < 2) {
        usage();
        return 0;
    }

    memset(&hdr, 0, sizeof(hdr));


    while ((opt = getopt(argc, argv, "i:k:r:s:t:dh")) != -1) {
        switch (opt) {
            case 'i':
                bootimg = strdup(optarg);
                break;
            case 'k':
                kernel = strdup(optarg);
                break;
            case 'r':
                ramdisk = strdup(optarg);
                break;
            case 's':
                second = strdup(optarg);
                break;
            case 't':
                dt = strdup(optarg);
                break;
            case 'd':
                dump = 1;
                break;
            case 'h':
            default:
                usage();
                return 0;
        }
    }

    bootfd = open(bootimg, O_RDONLY);
    if (bootfd <= 0) {
        perror("open");
        return -1;
    }

    if (read(bootfd, &hdr, sizeof(hdr)) != sizeof(hdr)) {
        perror("read");
        return -1;
    }

    if (dump) {
        dump_hdr(&hdr);
        return 0;
    }

    if (kernel)
       if (extract(bootfd, &hdr, kernel, "kernel") < 0)
           goto out;

    if (ramdisk)
       if (extract(bootfd, &hdr, ramdisk, "ramdisk") < 0)
           goto out;

    if (second)
       if (extract(bootfd, &hdr, second, "second") < 0)
           goto out;

    if (dt)
       if (extract(bootfd, &hdr, dt, "dt") < 0)
           goto out;

    if (!(kernel || ramdisk || second || dt)) {
       extract(bootfd, &hdr, "kernel.img", "kernel");
       extract(bootfd, &hdr, "ramdisk.img", "ramdisk");
       extract(bootfd, &hdr, "second.img", "second");
       extract(bootfd, &hdr, "dt.img", "dt");
    }

out:
    close(bootfd);

    return 0;
}
