#ifndef DISTRICT_H
#define DISTRICT_H

int district_init(const char *name);
void district_log(const char *name, const char *role, const char *user, const char *action);
void district_remove(const char *name, const char *role, const char *user);

#endif
