
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include <xml.h>

void help(const char *path)
{
    const char *pname = strrchr(path, '/');
    if (!pname) pname = path;
    else pname++;

    printf("Usage: %s <file>\n", pname);
    printf("\nWhere <file> is either the gmmidi.xml or gmdrums.xml file.\n");
    printf("\n");
    exit(-1);
}

int main(int argc, char **argv)
{
    const char *filename;
    void *xid;

    if (argc != 2) {
        help(argv[0]);
    }

    filename = argv[1];
    xid = xmlOpen(filename);
    if (xid)
    {
        char dir[1024];
        char *file;
        void *xmid;

        snprintf(dir, 255, "%s", filename);
        file = strrchr(dir, '/');
        if (!file)
        {
            snprintf(dir, 255, "./%s", filename);
            file = strrchr(dir, '/');
        }
        file++;

        xmid = xmlNodeGet(xid, "/aeonwave/midi");
        if (xmid)
        {
            unsigned int bnum = xmlNodeGetNum(xmid, "bank");
            void *xbid = xmlMarkId(xmid);
            unsigned int b;

            for (b=0; b<bnum; b++)
            {
                if (xmlNodeGetPos(xmid, xbid, "bank", b) != 0)
                {
                    unsigned int slen, inum = xmlNodeGetNum(xbid, "instrument");
                    void *xiid = xmlMarkId(xbid);
                    unsigned int i;

                    for (i=0; i<inum; i++)
                    {
                        if (xmlNodeGetPos(xbid, xiid, "instrument", i) != 0)
                        {
                            slen = xmlAttributeCopyString(xiid, "file", file, 64);
                            if (slen)
                            {
                                char *aaxs = file+slen;
                                snprintf(aaxs, 6, "%s", ".aaxs");

                                if (access(dir, R_OK) == -1 ) {
                                   printf("Warning: instrument file %s does not exist\n\t%s\n", file, filename);
                                }
                                continue;
                            }

                            slen = xmlAttributeCopyString(xiid, "patch", file, 64);
                            if (slen)
                            {
                                char *aaxs = file+slen;
                                snprintf(aaxs, 6, "%s", ".xml");

                                if (access(dir, R_OK) == -1 ) {
                                   printf("Warning: patch file %s does not exist\n\t%s\n", file, filename);
                                }
                            }

                            slen = xmlAttributeCopyString(xiid, "key-on", file, 64);
                            if (slen)
                            {
                                char *aaxs = file+slen;
                                snprintf(aaxs, 6, "%s", ".aaxs");

                                if (access(dir, R_OK) == -1 ) {
                                   printf("Warning: key-on event file %s does not exist\n\t%s\n", file, filename);
                                }
                            }

                            slen = xmlAttributeCopyString(xiid, "key-off", file, 64);
                            if (slen)
                            {
                                char *aaxs = file+slen;
                                snprintf(aaxs, 6, "%s", ".aaxs");

                                if (access(dir, R_OK) == -1 ) {
                                   printf("Warning: key-off event file %s does not exist\n\t%s\n", file, filename);
                                }
                            }
                        }
                    }
                    xmlFree(xiid);

                    inum = xmlNodeGetNum(xbid, "drum");
                    xiid = xmlMarkId(xbid);

                    for (i=0; i<inum; i++)
                    {
                        if (xmlNodeGetPos(xbid, xiid, "drum", i) != 0)
                        {
                            slen = xmlAttributeCopyString(xiid, "file", file, 64);
                            if (slen)
                            {
                                char *aaxs = file+slen;
                                snprintf(aaxs, 6, "%s", ".aaxs");

                                if (access(dir, R_OK) == -1 ) {
                                   printf("Warning: drum file %s does not exist\n\t%s\n", file, filename);
                                }
                            }
                        }
                    }
                    xmlFree(xiid);
                }
            }
            xmlFree(xbid);
            xmlFree(xmid);
        }
        else {
            printf("/aeonwave/midi not found in %s\n", filename);
        }

        xmlClose(xid);
    }
    else {
        printf("Wanring: could not open %s\n", filename);
    }
}
