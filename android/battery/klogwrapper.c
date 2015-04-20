#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>

#define KERN_EMERG      "<0>"   /* system is unusable                   */
#define KERN_ALERT      "<1>"   /* action must be taken immediately     */
#define KERN_CRIT       "<2>"   /* critical conditions                  */
#define KERN_ERR        "<3>"   /* error conditions                     */
#define KERN_WARNING    "<4>"   /* warning conditions                   */
#define KERN_NOTICE     "<5>"   /* normal but significant condition     */
#define KERN_INFO       "<6>"   /* informational                        */
#define KERN_DEBUG      "<7>"   /* debug-level messages                 */

#define ERROR(fmt, arg...) klog_write(KERN_ERR fmt, ##arg)
#define INFO(fmt, arg...) klog_write(KERN_INFO fmt, ##arg)
#define NOTICE(fmt, arg...) klog_write(KERN_NOTICE fmt, ##arg)

static int klog_fd = -1;

void klog_init(void)
{
    const char *name = "/dev/__kmsg__";
    mknod(name, S_IFCHR | 0666, (1 << 8) | 11);
    klog_fd = open(name, O_WRONLY);
    if (klog_fd < 0) {
        fprintf(stderr, "failed to open %s\n", name);
        return;
    }
    fcntl(klog_fd, F_SETFD, FD_CLOEXEC);
    unlink(name);
}

#define LOG_BUF_MAX 512

void klog_write(const char *fmt, ...)
{
    char buf[LOG_BUF_MAX];
    va_list ap;

    if (klog_fd < 0) {
        fprintf(stderr, "no klog fd.\n");
        return;
    }

    va_start(ap, fmt);
    vsnprintf(buf, LOG_BUF_MAX, fmt, ap);
    buf[LOG_BUF_MAX - 1] = 0;
    va_end(ap);
    write(klog_fd, buf, strlen(buf));
}

void child(int argc, char* argv[])
{
    char *argv_child[argc + 1];

    memcpy(argv_child, argv, argc * sizeof(char *));
    argv_child[argc] = NULL;

    if (execv(argv_child[0], argv_child)) {
        fprintf(stderr, "executing %s failed: %s\n",
                argv_child[0], strerror(errno));
        exit(-1);
    }
}

int klogwrap(int argc, char *argv[])
{
    pid_t pid;
    int pipefd[2];

    pipe(pipefd);

    pid = fork();
    if (pid < 0) {
	ERROR("Failed to fork\n");
        return -errno;
    } else if (pid == 0) {

        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);

        child(argc, argv);
    } else {
        char pbuf[1024];

        close(pipefd[1]);
        FILE *from_child = fdopen(pipefd[0], "r");

        while (fgets(pbuf, sizeof(pbuf), from_child) != NULL) {
            NOTICE("%s", pbuf);
        }
        fclose(from_child);

        int status;
        waitpid(pid, &status, 0);
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            ERROR("Error exec %s !\n(Status %d)\n", argv[0],
                    WEXITSTATUS(status));
            return -1;
        }
    }

    return 0;
}

int main(int argc, char *argv[])
{
    klog_init();
    klogwrap(argc-1, &argv[1]);

    return 0;
}
