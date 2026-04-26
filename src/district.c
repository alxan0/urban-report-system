#include "district.h"
#include "permissions.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

static int create_file(const char *path, mode_t mode, const char *initial)
{
    int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, mode);
    if (fd < 0)
    {
        return 1;
    }
    if (initial)
    {
        write(fd, initial, strlen(initial));
    }
    close(fd);
    chmod(path, mode);
    return 1;
}

int district_init(const char *name)
{
    char path[512];

    if (mkdir(name, 0750) < 0)
    {
        struct stat st;
        if (!(stat(name, &st) == 0 && S_ISDIR(st.st_mode)))
        {
            return -1;
        }
    }
    chmod(name, 0750);

    snprintf(path, sizeof(path), "%s/reports.dat", name);
    if (!create_file(path, 0664, NULL))
    {
        return -1;
    }

    snprintf(path, sizeof(path), "%s/district.cfg", name);
    if (!create_file(path, 0640, "threshold=1\n"))
    {
        return -1;
    }

    snprintf(path, sizeof(path), "%s/logged_district", name);
    if (!create_file(path, 0644, NULL))
    {
        return -1;
    }

    return 1;
}

void district_log(const char *name, const char *role, const char *user, const char *action)
{
    char path[512];
    snprintf(path, sizeof(path), "%s/logged_district", name);

    if (!check_permission(path, role, 0, 1))
    {
        return;
    }

    int fd = open(path, O_WRONLY | O_APPEND, 0644);
    if (fd < 0)
    {
        return;
    }

    char buf[256];
    time_t now = time(NULL);
    int len = snprintf(buf, sizeof(buf), "%ld\t%s\t%s\t%s\n", (long)now, user, role, action);
    write(fd, buf, len);
    close(fd);
}
