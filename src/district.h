#ifndef DISTRICT_H
#define DISTRICT_H

int district_add(const char *name);
int district_init(const char *name);
void district_log(const char *name, const char *role, const char *user, const char *action);

#endif
