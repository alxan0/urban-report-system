#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "report.h"

int main(int argc, char *argv[])
{
    char path[512];
    int fd;
    Report r;
    char names[64][64];
    int scores[64];
    int count;
    int i;
    int found;

    if (argc < 2)
        return 1;

    count = 0;
    snprintf(path, sizeof(path), "%s/reports.dat", argv[1]);

    fd = open(path, O_RDONLY);
    if (fd < 0)
    {
        perror("open");
        return 1;
    }

    while (read(fd, &r, sizeof(Report)) == sizeof(Report))
    {
        found = 0;
        for (i = 0; i < count; i++)
        {
            if (strcmp(names[i], r.inspector) == 0)
            {
                scores[i] += r.severity;
                found = 1;
                break;
            }
        }
        if (!found && count < 64)
        {
            strncpy(names[count], r.inspector, 63);
            names[count][63] = '\0';
            scores[count] = r.severity;
            count++;
        }
    }

    close(fd);

    printf("District: %s\n", argv[1]);
    for (i = 0; i < count; i++)
        printf("  %s: %d\n", names[i], scores[i]);

    return 0;
}
