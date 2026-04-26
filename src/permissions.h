#ifndef PERMISSIONS_H
#define PERMISSIONS_H

#include <sys/types.h>

void mode_to_string(mode_t mode, char out[10]);
int check_permission(const char *path, const char *role, int need_read, int need_write);

#endif
