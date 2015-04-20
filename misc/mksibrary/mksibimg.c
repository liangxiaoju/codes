/*
 * make sibrary.img
 */

#include <stdio.h>
#include <sys/sendfile.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

struct SibUpdate
{
	char flag[32];
	int version;
	int size;
};

struct Package
{
	char module[4];
	int version;
	int size;
	/* int offset; */
};

typedef struct item {
	char opt[64];
	char value[64];
	struct item *next;
} item_t;

typedef struct group {
	char tag[64];
	item_t *item;
	struct group *next;
} group_t;

group_t *cfg = NULL;

void trim(char *buf)
{
	int i=0, j=0;

	if (!buf)
		return;

	while (j<strlen(buf)) {
		if (buf[j] != ' ' && buf[j] != '\t' && buf[j] != '\n') {
			buf[i] = buf[j];
			i++;
		}
		j++;
	}
	buf[i] = '\0';
}

int parseconf(int fd)
{
	char *line=NULL;
	int len;
	FILE *fp;
	group_t *group=NULL, *pre_group=NULL;
	item_t *item, *pre_item=NULL;

	fp = fdopen(fd, "r");
	if (!fp) {
		perror("fdopen");
		return -1;
	}

	while (getline(&line, &len, fp) > 0) {
		trim(line);
		if (line[0]=='#' || line[0]=='\0') 
			continue;

		if (line[0] == '[') { /* group */
			if (line[strlen(line)-1] != ']') {
				fprintf(stderr, "Parse config file [group] error\n");
				return -1;
			}
			line[strlen(line)-1]='\0';
			group = malloc(sizeof(*group));
			if (!group) {
				perror("malloc group error\n");
				return -1;
			}
			if (cfg)
				pre_group->next = group;
			else
				cfg = group;

			memset(group, 0, sizeof(*group));
			strncpy(group->tag, line+1, sizeof(group->tag));

			pre_group = group;

		} else { /* item */
			int pos = 0;
			if (!group) {
				fprintf(stderr, "Parse config [item] error\n");
				return -1;
			}
			item = malloc(sizeof(*item));
			if (!item) {
				perror("malloc item error");
				return -1;
			}
			if (group->item)
				pre_item->next = item;
			else
				group->item = item;

			strncpy(item->opt, strtok(line+pos, "="), sizeof(item->opt));
			pos += strlen(line+pos) + 1;
			strncpy(item->value, strtok(line+pos, "="), sizeof(item->value));
			if (!item->value || !item->opt) {
				fprintf(stderr, "Parse config [item] error\n");
				return -1;
			}

			pre_item = item;
			item->next = NULL;
		}
	}

	free(line);

	return 0;
}

int cat(int out_fd, int in_fd)
{
	struct stat buf;

	if (fstat(in_fd, &buf) < 0) {
		perror("fstat");
		return -1;
	}

	if (sendfile(out_fd, in_fd, NULL, buf.st_size) < 0) {
		perror("sendfile");
		return -1;
	}
	
	return 0;
}

int main(int argc, const char *argv[])
{
	int opt, fd;
	int file;
	int version;
	int ret = 0;
	group_t *g;
	item_t  *it;
	struct stat st;
	struct SibUpdate update;
	struct Package package;
	char *cfile="image.cfg";
	char *ofile="sibrary.img";

	while ((opt=getopt(argc, argv, "c:o:")) > 0 ) {
		switch (opt) {
			case 'c':
				cfile = optarg;
				break;
			case 'o':
				ofile = optarg;
				break;
			default:
				fprintf(stderr, "Usage: %s [-c config] [-o image]", argv[0]);
				return -1;
		}
	}

	printf("%s -c %s -o %s\n", argv[0], cfile, ofile);

	fd = open(cfile, O_RDONLY);
	if (fd < 0) {
		perror("open error");
		return -1;
	}

	if (parseconf(fd) < 0) 
		return -1;

	g = cfg;
	while (g) {
		item_t *it = g->item;
		printf("\n[%s]\n", g->tag);
		while (it) {
			printf("%s=%s\n", it->opt, it->value);
			it = it->next;
		}
		g = g->next;
	}
	
	close(fd);
	fd = open(ofile, O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	if (fd < 0) {
		perror("open error");
		return -1;
	}
	g = cfg;
	while (g) {
		it = g->item;
		if (it) {
			while (it) {
				if (!strncmp(it->opt, "file", sizeof(it->opt))) {
					file = open(it->value, O_RDONLY);
					if (file < 0) {
						perror("open");
						return -1;
					}
					if (fstat(file, &st) < 0) {
						perror("fstat");
						return -1;
					}
				} else if (!strncmp(it->opt, "version", sizeof(it->opt)))
					version = atoi(it->value);
				 else
						printf("%s=%s ignored\n", it->opt, it->value);
				 it = it->next;
			}
		} else  {
			if (strncmp(g->tag, "update", sizeof(g->tag)) == 0) {
				fprintf(stderr, "update file is prerequisite\n");
				ret = -1;
				break;
			}

			g = g->next;
			continue;
		}

		if (strncmp(g->tag, "update", sizeof(g->tag)) == 0) {
			strncpy(update.flag, "boeyesibraryupdate", sizeof(update.flag));
			update.version=version;
			update.size=st.st_size;
			if (write(fd, &update, sizeof(update)) != sizeof(update)) {
				perror("write update");
				ret = -1;
			}
		} else {
			strncpy(package.module, g->tag, sizeof(package.module));
			package.version=version;
			package.size=st.st_size;
			if (write(fd, &package, sizeof(package)) != sizeof(package)) {
				perror("write package");
				ret = -1;
			}
		}

		cat(fd, file);
		
		close(file);
		g = g->next;
	}

	return ret;
}
