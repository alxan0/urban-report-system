#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#define PID_FILE ".monitor_pid"

void handle_sigusr1(int sig)
{
    printf("New report received.\n");
}

void handle_sigint(int sig)
{
    unlink(PID_FILE);
    printf("Monitor shutting down.\n");
    exit(0);
}

int main(void)
{
    int fd;
    char buf[32];
    int len;
    struct sigaction sa;

    fd = open(PID_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0)
    {
        perror("open");
        return 1;
    }
    len = snprintf(buf, sizeof(buf), "%d\n", getpid());
    write(fd, buf, len);
    close(fd);

    sa.sa_handler = handle_sigusr1;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGUSR1, &sa, NULL) < 0)
    {
        perror("sigaction");
        return 1;
    }

    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) < 0)
    {
        perror("sigaction");
        return 1;
    }

    printf("Monitor started (PID %d).\n", getpid());

    while (1)
        pause();

    return 0;
}