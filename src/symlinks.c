#define _POSIX_C_SOURCE 200809L

#include "symlinks.h"
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

void update_symlink(const char *district)
{
    char link_name[256];
    char target[256];
    char existing_target[256];
    struct stat st;

    snprintf(link_name, sizeof(link_name), "active_reports-%s", district);
    snprintf(target, sizeof(target), "%s/reports.dat", district);

    if (lstat(link_name, &st) == 0)
    {
        if (!S_ISLNK(st.st_mode))
        {
            fprintf(stderr, "Warning: %s exists and is not a symlink.\n", link_name);
            return;
        }

        ssize_t len = readlink(link_name, existing_target, sizeof(existing_target) - 1);
        if (len >= 0)
        {
            existing_target[len] = '\0';
            if (strcmp(existing_target, target) != 0)
            {
                if (unlink(link_name) < 0)
                {
                    perror("unlink");
                    return;
                }
                if (symlink(target, link_name) < 0)
                {
                    perror("symlink");
                    return;
                }
            }
        }

        struct stat st_target;
        if (stat(link_name, &st_target) < 0)
        {
            fprintf(stderr, "Warning: %s is a dangling symlink.\n", link_name);
        }
        return;
    }
    else if (errno != ENOENT)
    {
        perror("lstat");
        return;
    }

    if (symlink(target, link_name) < 0)
    {
        perror("symlink");
    }
}
