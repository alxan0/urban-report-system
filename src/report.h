#ifndef REPORT_H
#define REPORT_H

#include <time.h>

typedef struct
{
    int id;
    char inspector[64];
    double latitude;
    double longitude;
    char category[32];
    int severity;
    time_t timestamp;
    char description[128];
} Report;

void cmd_add(const char *district, const char *role, const char *user);
void cmd_list(const char *district, const char *role, const char *user);
void cmd_view(const char *district, int id, const char *role, const char *user);
void cmd_remove(const char *district, int id, const char *role, const char *user);

#endif
