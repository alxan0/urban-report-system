#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "district.h"
#include "report.h"
#include "filter.h"
#include "permissions.h"
#include "symlinks.h"

static void print_usage(const char *prog)
{
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

void cmd_update_threshold(const char *district, int value, const char *role, const char *user)
{
    if (strcmp(role, "manager") != 0)
    {
        fprintf(stderr, "Access denied.\n");
        exit(1);
    }
    char path[512];
    struct stat st;

    snprintf(path, sizeof(path), "%s/district.cfg", district);

    if (stat(path, &st) < 0)
    {
        perror("stat");
        exit(1);
    }

    if ((st.st_mode & 0777) != 0640)
    {
        fprintf(stderr, "Refusing to update: district.cfg permissions must be 640.\n");
        exit(1);
    }

    if (!check_permission(path, role, 1, 1))
    {
        fprintf(stderr, "Permission denied on config.\n");
        exit(1);
    }

    FILE *f = fopen(path, "w");
    if (!f)
    {
        perror("fopen");
        exit(1);
    }

    fprintf(f, "threshold=%d\n", value);
    fclose(f);
    printf("Threshold updated to %d.\n", value);

    district_log(district, role, user, "update_threshold");
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        print_usage(argv[0]);
        return 1;
    }

    const char *role = NULL;
    const char *user = NULL;
    const char *command = NULL;
    const char *district = NULL;
    int arg_start = -1;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--role") == 0 && i + 1 < argc)
        {
            role = argv[++i];
        }
        else if (strcmp(argv[i], "--user") == 0 && i + 1 < argc)
        {
            user = argv[++i];
        }
        else if (argv[i][0] == '-' && argv[i][1] == '-' && command == NULL)
        {
            command = argv[i] + 2;
            if (i + 1 < argc && argv[i + 1][0] != '-')
            {
                district = argv[++i];
                arg_start = i + 1;
            }
        }
    }

    if (!role || !user || !command)
    {
        fprintf(stderr, "Error: --role, --user, and a command are required.\n\n");
        print_usage(argv[0]);
        return 1;
    }

    if (strcmp(role, "inspector") != 0 && strcmp(role, "manager") != 0)
    {
        fprintf(stderr, "Error: role must be 'inspector' or 'manager'.\n");
        return 1;
    }

    if (!district)
    {
        fprintf(stderr, "Error: command '--%s' requires a district argument.\n", command);
        return 1;
    }

    if (strcmp(command, "add") == 0)
    {
        if (district_init(district) < 0)
        {
            fprintf(stderr, "Error: failed to initialize district '%s'.\n", district);
            return 1;
        }
    }
    else
    {
        struct stat dst;
        if (stat(district, &dst) < 0 || !S_ISDIR(dst.st_mode))
        {
            fprintf(stderr, "Error: district '%s' does not exist.\n", district);
            return 1;
        }
    }

    update_symlink(district);

    if (strcmp(command, "add") == 0)
    {
        cmd_add(district, role, user);
    }
    else if (strcmp(command, "list") == 0)
    {
        cmd_list(district, role, user);
    }
    else if (strcmp(command, "view") == 0)
    {
        if (arg_start != -1 && arg_start < argc)
        {
            cmd_view(district, atoi(argv[arg_start]), role, user);
        }
    }
    else if (strcmp(command, "remove_report") == 0)
    {
        if (arg_start != -1 && arg_start < argc)
        {
            cmd_remove(district, atoi(argv[arg_start]), role, user);
        }
    }
    else if (strcmp(command, "update_threshold") == 0)
    {
        if (arg_start != -1 && arg_start < argc)
        {
            cmd_update_threshold(district, atoi(argv[arg_start]), role, user);
        }
    }
    else if (strcmp(command, "filter") == 0)
    {
        cmd_filter(district, argc - arg_start, &argv[arg_start], role, user);
    }
    else
    {
        fprintf(stderr, "Unknown command: %s\n", command);
        return 1;
    }

    return 0;
}
