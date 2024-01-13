
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <xml.h>

#include "common.h"

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

        xmid = xmlNodeGet(xid, "/aeonwave/set");
        if (xmid)
        {
            unsigned int bnum = xmlNodeGetNum(xmid, "layer");
            void *xbid = xmlMarkId(xmid);
            unsigned int b;

            for (b=0; b<bnum; b++)
            {
                if (xmlNodeGetPos(xmid, xbid, "layer", b) != 0)
                {
                    unsigned int slen, inum = xmlNodeGetNum(xbid, "instrument");
                    void *xiid = xmlMarkId(xbid);
                    unsigned int i;

                    slen = xmlAttributeCopyString(xbid, "file", file, 64);
                    if (slen)
                    {
                        if (!check_file(file, ".aaxs")) {
                           printf("Warning: instrument file %s does not exist\n\t%s\n", file, filename);
                        }
                    }

                    for (i=0; i<inum; i++)
                    {
                        if (xmlNodeGetPos(xbid, xiid, "instrument", i) != 0)
                        {
                            slen = xmlAttributeCopyString(xiid, "file", file, 64);
                            if (slen)
                            {
                                if (!check_file(file, ".aaxs")) {
                                   printf("Warning: instrument file %s does not exist\n\t%s\n", file, filename);
                                }
                                continue;
                            }

                            slen = xmlAttributeCopyString(xiid, "include", file, 64);
                            if (slen)
                            {
                                if (!check_file(file, ".xml")) {
                                   printf("Warning: patch file %s does not exist\n\t%s\n", file, filename);
                                }
                            }

                            slen = xmlAttributeCopyString(xiid, "key-on", file, 64);
                            if (slen)
                            {
                                if (!check_file(file, ".aaxs")) {
                                   printf("Warning: key-on event file %s does not exist\n\t%s\n", file, filename);
                                }
                            }

                            slen = xmlAttributeCopyString(xiid, "key-off", file, 64);
                            if (slen)
                            {
                                if (!check_file(file, ".aaxs")) {
                                   printf("Warning: key-off event file %s does not exist\n\t%s\n", file, filename);
                                }
                            }
                        }
                    }
                    xmlFree(xiid);

                    inum = xmlNodeGetNum(xbid, "patch");
                    xiid = xmlMarkId(xbid);

                    for (i=0; i<inum; i++)
                    {
                        if (xmlNodeGetPos(xbid, xiid, "drum", i) != 0)
                        {
                            slen = xmlAttributeCopyString(xiid, "file", file, 64);
                            if (slen)
                            {
                                if (!check_file(file, ".aaxs")) {
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
