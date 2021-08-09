

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

using name_t = std::pair<std::string,std::string>;
using entry_t = std::map<unsigned,name_t>;

const char *sections[] = {
 "Piano", "Chromatic Pericussion", "Organ", "Guitar", "Bass",
 "Strings", "Ensemble", "Brass", "Reed", "Pipe",
 "Synth Lead", "Synth Pad", "Synth Effects", "Ethnic", "Percussive",
 "Sound Effects"
};

unsigned int
get_elem(const char *dir, std::string &file)
{
    unsigned int rv = 0;
    size_t fpos;
    void *xid;

    std::string path(dir);
    fpos = path.find_last_of("/\\");
    if (fpos != std::string::npos) {
        path = path.substr(0, fpos+1);
    }
    path.append(file);
    path.append(".aaxs");

    xid = xmlOpen(path.c_str());
    if (xid)
    {
        void *xsid = xmlNodeGet(xid, "/aeonwave/sound");
        if (xsid)
        {
            rv = xmlNodeGetNum(xsid, "layer");
            if (!rv) rv = 1;
            xmlFree(xsid);
        }
        xmlFree(xid);
    }

    return rv;
}

int main(int argc, char **argv)
{
    std::map<unsigned,entry_t> bank;
    const char *filename;
    void *xid;

    if (argc != 2) {
        help(argv[0]);
    }

    filename = argv[1];
    xid = xmlOpen(filename);
    if (xid)
    {
        char file[1024];
        char name[1024];
        char *ptr;
        void *xmid;

        snprintf(file, 255, "%s", filename);
        ptr = strrchr(file, '/');
        if (!ptr)
        {
            snprintf(file, 255, "./%s", filename);
            ptr = strrchr(file, '/');
        }
        ptr++;

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

                            xmlAttributeCopyString(xiid, "file", file, 64);
                            slen = xmlAttributeCopyString(xiid, "name", name, 64);
                            if (slen) {
                                e[pos] = std::make_pair<std::string,std::string>(name,file);
                            }
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
                    if ((i % 8) == 0)
                    {
                        printf("\n=== %s\n", sections[i/8]);
                        printf(" PC  msb lsb elem  instrument name\n");
                        printf("---  --- --- ----  ------------------------------\n");
                    }

                    for (auto &b : bank)
                    {
                        unsigned int nl = b.first;
                        entry_t &e = b.second;
                        auto it = e.find(i);
                        if (it != e.end())
                        {
                            unsigned int elem;

                            elem = get_elem(filename, it->second.second);
                            printf("%3i  %3i %3i  %3i  %s\n",
                                    i+1, nl >> 8, nl & 0xf, elem,
                                    it->second.first.c_str());
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
