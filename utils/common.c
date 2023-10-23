
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>

int
check_file(const char *filename, const char *extension)
{
    char dir[1025];
    char *file;
    char *aaxs;
    int slen;

    snprintf(dir, 255, "%s", filename);
    file = strrchr(dir, '/');
    if (!file)
    {
        snprintf(dir, 255, "./%s", filename);
        file = strrchr(dir, '/');
    }
    file++;

    slen = strlen(file);
    aaxs = file+slen;
    snprintf(aaxs, 6, "%s", extension);

    if (access(dir, R_OK) == -1 ) {
        return 0;
    }
    return -1;
}
