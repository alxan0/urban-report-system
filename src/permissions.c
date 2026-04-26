#include "permissions.h"
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>

void mode_to_string(mode_t mode, char out[10])
{
    strcpy(out, "---------");
    if (mode & S_IRUSR) out[0] = 'r';
    if (mode & S_IWUSR) out[1] = 'w';
    if (mode & S_IXUSR) out[2] = 'x';
    if (mode & S_IRGRP) out[3] = 'r';
    if (mode & S_IWGRP) out[4] = 'w';
    if (mode & S_IXGRP) out[5] = 'x';
    if (mode & S_IROTH) out[6] = 'r';
    if (mode & S_IWOTH) out[7] = 'w';
    if (mode & S_IXOTH) out[8] = 'x';
    out[9] = '\0';
}

int check_permission(const char *path, const char *role, int need_read, int need_write)
{
    struct stat st;
    if (stat(path, &st) < 0)
        return 0;

    if (strcmp(role, "manager") == 0)
    {
        if (need_read && !(st.st_mode & S_IRUSR))
            return 0;
        if (need_write && !(st.st_mode & S_IWUSR))
            return 0;
    }
    else if (strcmp(role, "inspector") == 0)
    {
        if (need_read && !(st.st_mode & S_IRGRP))
            return 0;
        if (need_write && !(st.st_mode & S_IWGRP))
            return 0;
    }
    else
    {
        return 0;
    }
    return 1;
}
