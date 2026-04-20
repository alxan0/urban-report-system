#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "district.h"

static void print_usage(const char *prog) {
    fprintf(stderr,
        "Usage: %s --role <inspector|manager> --user <name> --<command> [args...]\n"
        "\n"
        "Commands:\n"
        "  --add <district>                        Add a new report\n"
        "  --list <district>                       List all reports\n"
        "  --view <district> <report_id>           View a specific report\n"
        "  --remove_report <district> <report_id>  Remove a report (manager only)\n"
        "  --update_threshold <district> <value>   Update severity threshold (manager only)\n"
        "  --filter <district> <condition...>      Filter reports by condition\n",
        prog);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    const char *role = NULL;
    const char *user = NULL;
    const char *command = NULL;
    const char *district = NULL;
    int arg_start = -1;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--role") == 0 && i + 1 < argc) {
            role = argv[++i];
        } else if (strcmp(argv[i], "--user") == 0 && i + 1 < argc) {
            user = argv[++i];
        } else if (argv[i][0] == '-' && argv[i][1] == '-' && command == NULL) {
            command = argv[i] + 2;
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                district = argv[++i];
                arg_start = i + 1;
            }
        }
    }

    if (!role || !user || !command) {
        fprintf(stderr, "Error: --role, --user, and a command are required.\n\n");
        print_usage(argv[0]);
        return 1;
    }

    if (strcmp(role, "inspector") != 0 && strcmp(role, "manager") != 0) {
        fprintf(stderr, "Error: role must be 'inspector' or 'manager'.\n");
        return 1;
    }

    if (!district) {
        fprintf(stderr, "Error: command '--%s' requires a district argument.\n", command);
        return 1;
    }

    if (strcmp(command, "add") == 0) {
        district_init(district);
        district_log(district, role, user, "add");
        printf("District '%s' ready.\n", district);
        return 0;
    }

    fprintf(stderr, "Command '%s' not yet implemented.\n", command);
    (void)arg_start;
    return 1;
}
