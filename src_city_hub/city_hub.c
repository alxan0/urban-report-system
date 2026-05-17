#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void run_hub_mon(void)
{
    int pipefd[2];
    pid_t pid;
    char buf[512];
    int i;
    char c;
    ssize_t n;

    if (pipe(pipefd) < 0)
    {
        perror("pipe");
        exit(1);
    }

    pid = fork();
    if (pid < 0)
    {
        perror("fork");
        exit(1);
    }

    if (pid == 0)
    {
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        execl("./monitor_reports", "monitor_reports", NULL);
        perror("execl");
        exit(1);
    }

    close(pipefd[1]);

    i = 0;
    while ((n = read(pipefd[0], &c, 1)) == 1)
    {
        if (c == '\n')
        {
            buf[i] = '\0';
            i = 0;

            if (strncmp(buf, "err:", 4) == 0)
            {
                printf("[monitor] %s\n", buf + 4);
                fflush(stdout);
                break;
            }
            else if (strncmp(buf, "quit:", 5) == 0)
            {
                printf("[monitor] %s\n", buf + 5);
                fflush(stdout);
                break;
            }
            else if (strncmp(buf, "msg:", 4) == 0)
            {
                printf("[monitor] %s\n", buf + 4);
                fflush(stdout);
            }
        }
        else if (i < sizeof(buf) - 1)
        {
            buf[i++] = c;
        }
    }

    printf("[monitor] Monitor has ended.\n");
    fflush(stdout);
    close(pipefd[0]);
    exit(0);
}

int main(void)
{
    char line[256];
    pid_t pid;

    printf("city_hub> ");
    fflush(stdout);

    while (fgets(line, sizeof(line), stdin) != NULL)
    {
        line[strcspn(line, "\n")] = '\0';

        if (strcmp(line, "start_monitor") == 0)
        {
            pid = fork();
            if (pid < 0)
            {
                perror("fork");
            }
            else if (pid == 0)
            {
                run_hub_mon();
            }
            else
            {
                printf("Monitor starting...\n");
            }
        }
        else if (strcmp(line, "quit") == 0 || strcmp(line, "exit") == 0)
        {
            break;
        }
        else if (line[0] != '\0')
        {
            printf("Unknown command: %s\n", line);
        }

        printf("city_hub> ");
        fflush(stdout);
    }

    return 0;
}
