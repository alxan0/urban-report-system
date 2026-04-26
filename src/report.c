#define _POSIX_C_SOURCE 200809L

#include "report.h"
#include "district.h"
#include "permissions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

void cmd_add(const char *district, const char *role, const char *user)
{
    char path[512];
    snprintf(path, sizeof(path), "%s/reports.dat", district);

    chmod(path, 0664);

    Report r;
    memset(&r, 0, sizeof(Report));

    printf("Category: ");
    scanf("%31s", r.category);
    printf("Severity (1-3): ");
    scanf("%d", &r.severity);
    printf("Latitude: ");
    scanf("%lf", &r.latitude);
    printf("Longitude: ");
    scanf("%lf", &r.longitude);
    printf("Description: ");
    getchar();
    fgets(r.description, sizeof(r.description), stdin);
    r.description[strcspn(r.description, "\n")] = 0;

    strncpy(r.inspector, user, sizeof(r.inspector) - 1);
    r.timestamp = time(NULL);

    int fd = open(path, O_RDWR);
    if (fd < 0)
    {
        perror("open");
        return;
    }

    r.id = 0;
    Report existing;
    while (read(fd, &existing, sizeof(Report)) == sizeof(Report))
    {
        if (existing.id >= r.id)
        {
            r.id = existing.id + 1;
        }
    }

    if (lseek(fd, 0, SEEK_END) < 0)
    {
        perror("lseek");
        close(fd);
        return;
    }

    if (write(fd, &r, sizeof(Report)) != sizeof(Report))
    {
        perror("write");
        close(fd);
        return;
    }
    close(fd);

    district_log(district, role, user, "add");
}

void cmd_list(const char *district, const char *role, const char *user)
{
    char path[512];
    snprintf(path, sizeof(path), "%s/reports.dat", district);

    if (!check_permission(path, role, 1, 0))
    {
        fprintf(stderr, "Access denied.\n");
        exit(1);
    }

    int fd = open(path, O_RDONLY);
    if (fd < 0)
    {
        perror("open");
        return;
    }

    Report r;
    while (read(fd, &r, sizeof(Report)) == sizeof(Report))
    {
        printf("ID: %d | Cat: %s | Sev: %d | Insp: %s\n", r.id, r.category, r.severity, r.inspector);
    }

    struct stat st;
    fstat(fd, &st);
    char modestr[10];
    char timestr[64];

    mode_to_string(st.st_mode, modestr);
    if (strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", localtime(&st.st_mtime)) == 0)
    {
        strcpy(timestr, "unknown");
    }
    printf("\nFile: %s | Size: %ld | Perms: %s | Modified: %s\n",
           path, (long)st.st_size, modestr, timestr);

    close(fd);
    district_log(district, role, user, "list");
}

void cmd_view(const char *district, int id, const char *role, const char *user)
{
    char path[512];
    snprintf(path, sizeof(path), "%s/reports.dat", district);

    if (!check_permission(path, role, 1, 0))
    {
        fprintf(stderr, "Access denied.\n");
        exit(1);
    }

    int fd = open(path, O_RDONLY);
    if (fd < 0)
    {
        perror("open");
        return;
    }

    Report r;
    int found = 0;
    while (read(fd, &r, sizeof(Report)) == sizeof(Report))
    {
        if (r.id == id)
        {
            printf("ID: %d\nInspector: %s\nCategory: %s\nSeverity: %d\nLat/Long: %f, %f\nTime: %ld\nDesc: %s\n",
                   r.id, r.inspector, r.category, r.severity, r.latitude, r.longitude, (long)r.timestamp, r.description);
            found = 1;
            break;
        }
    }

    if (!found)
        printf("Report %d not found.\n", id);

    close(fd);
    district_log(district, role, user, "view");
}

void cmd_remove(const char *district, int id, const char *role, const char *user)
{
    if (strcmp(role, "manager") != 0)
    {
        fprintf(stderr, "Error: manager role required.\n");
        exit(1);
    }

    char path[512];
    snprintf(path, sizeof(path), "%s/reports.dat", district);

    if (!check_permission(path, role, 1, 1))
    {
        fprintf(stderr, "Access denied.\n");
        exit(1);
    }

    int fd = open(path, O_RDWR);
    if (fd < 0)
    {
        perror("open");
        return;
    }

    Report r;
    int found = 0;
    while (read(fd, &r, sizeof(Report)) == sizeof(Report))
    {
        if (r.id == id)
        {
            found = 1;
            break;
        }
    }

    if (!found)
    {
        printf("Report %d not found.\n", id);
        close(fd);
        district_log(district, role, user, "remove");
        return;
    }

    Report next;
    ssize_t nread;
    while ((nread = read(fd, &next, sizeof(Report))) == sizeof(Report))
    {
        if (lseek(fd, -2 * (off_t)sizeof(Report), SEEK_CUR) < 0)
        {
            perror("lseek");
            close(fd);
            return;
        }
        if (write(fd, &next, sizeof(Report)) != sizeof(Report))
        {
            perror("write");
            close(fd);
            return;
        }
        if (lseek(fd, sizeof(Report), SEEK_CUR) < 0)
        {
            perror("lseek");
            close(fd);
            return;
        }
    }

    if (nread < 0)
    {
        perror("read");
        close(fd);
        return;
    }

    if (lseek(fd, -(off_t)sizeof(Report), SEEK_END) < 0)
    {
        perror("lseek");
        close(fd);
        return;
    }
    if (ftruncate(fd, lseek(fd, 0, SEEK_CUR)) < 0)
    {
        perror("ftruncate");
        close(fd);
        return;
    }

    printf("Report %d removed.\n", id);

    close(fd);
    district_log(district, role, user, "remove");
}
