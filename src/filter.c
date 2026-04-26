#include "filter.h"
#include "report.h"
#include "permissions.h"
#include "district.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>

static int compare_with_operator(long left, long right, const char *op) {
    if (strcmp(op, "==") == 0) return left == right;
    if (strcmp(op, "!=") == 0) return left != right;
    if (strcmp(op, ">") == 0) return left > right;
    if (strcmp(op, ">=") == 0) return left >= right;
    if (strcmp(op, "<") == 0) return left < right;
    if (strcmp(op, "<=") == 0) return left <= right;
    return 0;
}

static int is_supported_field(const char *field) {
    return strcmp(field, "severity") == 0 ||
           strcmp(field, "category") == 0 ||
           strcmp(field, "inspector") == 0 ||
           strcmp(field, "timestamp") == 0;
}

static int is_supported_operator(const char *op) {
    return strcmp(op, "==") == 0 ||
           strcmp(op, "!=") == 0 ||
           strcmp(op, ">") == 0 ||
           strcmp(op, ">=") == 0 ||
           strcmp(op, "<") == 0 ||
           strcmp(op, "<=") == 0;
}

static int parse_long_strict(const char *text, long *out) {
    char *end = NULL;
    long parsed;

    if (!text || !out || text[0] == '\0')
    {
        return 0;
    }
    errno = 0;
    parsed = strtol(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0')
    {
        return 0;
    }
    *out = parsed;
    return 1;
}

int parse_condition(const char *input, char *field, char *op, char *value) {
    if (!input || !field || !op || !value) return 0;

    const char *colon1 = strchr(input, ':');
    if (!colon1) return 0;

    if ((size_t)(colon1 - input) >= 32) return 0;
    strncpy(field, input, colon1 - input);
    field[colon1 - input] = '\0';

    const char *colon2 = strchr(colon1 + 1, ':');
    if (!colon2) return 0;

    if ((size_t)(colon2 - (colon1 + 1)) >= 8) return 0;
    strncpy(op, colon1 + 1, colon2 - (colon1 + 1));
    op[colon2 - (colon1 + 1)] = '\0';

    if (strlen(colon2 + 1) >= 128) return 0;
    strcpy(value, colon2 + 1);

    if (value[0] == '\0') return 0;
    if (!is_supported_field(field)) return 0;
    if (!is_supported_operator(op)) return 0;

    if (strcmp(field, "severity") == 0 || strcmp(field, "timestamp") == 0) {
        long parsed;
        if (!parse_long_strict(value, &parsed)) return 0;
    }

    return 1;
}

int match_condition(Report *r, const char *field, const char *op, const char *value) {
    if (!r || !field || !op || !value) return 0;

    if (strcmp(field, "severity") == 0) {
        long parsed;
        if (!parse_long_strict(value, &parsed)) return 0;
        if (parsed < INT_MIN || parsed > INT_MAX) return 0;
        return compare_with_operator((long)r->severity, parsed, op);
    } else if (strcmp(field, "category") == 0) {
        int cmp = strcmp(r->category, value);
        return compare_with_operator(cmp, 0, op);
    } else if (strcmp(field, "inspector") == 0) {
        int cmp = strcmp(r->inspector, value);
        return compare_with_operator(cmp, 0, op);
    } else if (strcmp(field, "timestamp") == 0) {
        long parsed;
        if (!parse_long_strict(value, &parsed)) return 0;
        return compare_with_operator((long)r->timestamp, parsed, op);
    }
    return 0;
}

void cmd_filter(const char *district, int argc, char *argv[], const char *role, const char *user) {
    if (argc < 1) {
        fprintf(stderr, "Error: filter requires at least one condition (e.g. severity:>=:2).\n");
        exit(1);
    }

    char path[512];
    snprintf(path, sizeof(path), "%s/reports.dat", district);

    if (!check_permission(path, role, 1, 0)) {
        fprintf(stderr, "Access denied.\n");
        exit(1);
    }

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return;
    }

    for (int i = 0; i < argc; i++) {
        char field[32], op[8], value[128];
        if (!parse_condition(argv[i], field, op, value)) {
            fprintf(stderr, "Invalid condition: %s\n", argv[i]);
            close(fd);
            return;
        }
    }

    Report r;
    while (read(fd, &r, sizeof(Report)) == sizeof(Report)) {
        int all_match = 1;
        for (int i = 0; i < argc; i++) {
            char field[32], op[8], value[128];
            if (parse_condition(argv[i], field, op, value)) {
                if (!match_condition(&r, field, op, value)) {
                    all_match = 0;
                    break;
                }
            }
        }
        if (all_match) {
            printf("ID: %d | Cat: %s | Sev: %d | Insp: %s\n", r.id, r.category, r.severity, r.inspector);
        }
    }

    close(fd);
    district_log(district, role, user, "filter");
}
