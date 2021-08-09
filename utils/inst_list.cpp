

#include <map>

#include <stdio.h>
#include <string.h>
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

using entry_t = std::map<unsigned,std::string>;

int main(int argc, char **argv)
{
    std::map<unsigned, entry_t> bank;
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
        char *name;
        void *xmid;

        snprintf(dir, 255, "%s", filename);
        name = strrchr(dir, '/');
        if (!name)
        {
            snprintf(dir, 255, "./%s", filename);
            name = strrchr(dir, '/');
        }
        name++;

        xmid = xmlNodeGet(xid, "/aeonwave/midi");
        if (xmid)
        {
            unsigned int bnum = xmlNodeGetNum(xmid, "bank");
            void *xbid = xmlMarkId(xmid);
            unsigned int b, i, n;

            for (b=0; b<bnum; ++b)
            {
                if (xmlNodeGetPos(xmid, xbid, "bank", b) != 0)
                {
                    unsigned int slen, inum = xmlNodeGetNum(xbid, "instrument");
                    void *xiid = xmlMarkId(xbid);

                    n = xmlAttributeGetInt(xbid, "n") << 8;
                    n += xmlAttributeGetInt(xbid, "l");

                    entry_t e;
                    for (i=0; i<inum; ++i)
                    {
                        if (xmlNodeGetPos(xbid, xiid, "instrument", i) != 0)
                        {
                            unsigned int pos = xmlAttributeGetInt(xiid, "n");
                            slen = xmlAttributeCopyString(xiid, "name", name, 64);
                            if (slen) e[pos] = name;
                        }
                    }
                    xmlFree(xiid);

                    bank[n] = std::move(e);
                }
            }
            xmlFree(xbid);
            xmlFree(xmid);

            if (bank.size())
            {
                for (i=0; i<128; ++i)
                {
                    for (auto &b : bank)
                    {
                        unsigned int nl = b.first;
                        entry_t &e = b.second;
                        auto it = e.find(i);
                        if (it != e.end()) {
                            printf("%3i  %3i %3i %s\n", i, nl >> 8, nl & 0xf, it->second.c_str());
                        }
                    }
                }
            }
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
