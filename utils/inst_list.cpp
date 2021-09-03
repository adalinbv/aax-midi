

#include <map>
#include <vector>
#include <iterator>
#include <algorithm>
#include <iostream>
#include <regex>

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <xml.h>

#include "driver.h"
#include "canonical_names.hpp"

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
using bank_t = std::map<unsigned,std::pair<std::string,entry_t>>;

const char *sections[] = {
 "Piano", "Chromatic Percussion", "Organ", "Guitar", "Bass",
 "Strings", "Ensemble", "Brass", "Reed", "Pipe",
 "Synth Lead", "Synth Pad", "Synth Effects", "Ethnic/World", "Percussive",
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

int
fill_bank(bank_t& bank, void *xid, const char *tag)
{
    int rv = 0;

    bank.clear();

    void *xmid;
    xmid = xmlNodeGet(xid, "/aeonwave/midi");
    if (xmid)
    {
        unsigned int bnum = xmlNodeGetNum(xmid, "bank");
        void *xbid = xmlMarkId(xmid);
        unsigned int b, i, nl;
        char file[1024];
        char name[1024];

        for (b=0; b<bnum; ++b)
        {
            if (xmlNodeGetPos(xmid, xbid, "bank", b) != 0)
            {
                unsigned int slen, inum = xmlNodeGetNum(xbid, tag);
                void *xiid = xmlMarkId(xbid);

                std::string bank_name;
                slen = xmlAttributeCopyString(xiid, "name", name, 64);
                if (slen) bank_name = name;

                nl = xmlAttributeGetInt(xbid, "n") << 8;
                nl += xmlAttributeGetInt(xbid, "l");

                entry_t e;
                for (i=0; i<inum; ++i)
                {
                    if (xmlNodeGetPos(xbid, xiid, tag, i) != 0)
                    {
                        unsigned int pos = xmlAttributeGetInt(xiid, "n");

                        xmlAttributeCopyString(xiid, "file", file, 64);
                        slen = xmlAttributeCopyString(xiid, "name", name, 64);
                        if (slen)
                        {
                            e[pos] = std::make_pair<std::string,std::string>(name,file);
                            rv++;
                        }
                    }
                }
                xmlFree(xiid);

                bank[nl] = std::make_pair<std::string,entry_t>(std::move(bank_name),std::move(e));
            }
        }
        xmlFree(xbid);
        xmlFree(xmid);
    }
    return rv;
}

void
print_instruments(bank_t &bank, const char *dir, bool html)
{
    bool found = false;

    for (int i=0; i<128; ++i)
    {
        if ((i % 8) == 0)
        {
            if (!html)
            {
                printf("\n=== %s\n", sections[i/8]);
                printf(" PC  msb lsb elem  instrument name\n");
                printf("---  --- --- ----  ------------------------------\n");
            }
            else
            {
                if (!found)
                {
                    printf("<!DOCTYPE html>\n\n");
                    printf("<html>\n");
                    printf(" <head>\n");
                    printf("  <link rel=\"stylesheet\" type=\"text/css\" "
                                   "href=\"adalin.css\" title=\"style\"/>\n");
                    printf(" </head>\n");
                    printf(" <body>\n");
                    printf("  <table class=\"downloads\">\n");
                    found = true;
                }

                printf("   <tr>\n");
                printf("    <td class=\"head\" colspan=\"5\">%s</td>\n",
                               sections[i/8]);
                printf("   </tr>\n");

                printf("   <tr>\n");
                printf("    <td class=\"head\">PC</td>\n");
                printf("    <td class=\"head\">msb</td>\n");
                printf("    <td class=\"head\">lsb</td>\n");
                printf("    <td class=\"head\">elem</td>\n");
                printf("    <td class=\"head\" width=\"100%%\">instrument name</td>\n");
                printf("    <td class=\"head\">type</td>\n");
                printf("   </tr>\n");
            }
        }

        int num = 0;
        for (auto &b : bank)
        {
            entry_t &e = b.second.second;
            auto it = e.find(i);
            if (it != e.end()) num++;
        }

        bool first = true;
        for (auto &b : bank)
        {
            unsigned int nl = b.first;
            entry_t &e = b.second.second;
            auto it = e.find(i);
            if (it != e.end())
            {
                const char *type = "GM";
                unsigned int elem;
                int msb = nl >> 8;
                int lsb = nl & 0xff;

                if (msb == 0x79) type = "GM2";
                else if (msb == 127 && !lsb) type = "MT32";
                else if ((msb == 64 && !lsb) || (lsb && msb == 0)) type = "XG";
                else if (msb) type = "GS";

                elem = get_elem(dir, it->second.second);
                std::string name = canonical_name(it->second.first);
                if (!html)
                {
                    if (first)
                    {
                        printf("%3i  %3i %3i  %3i  %s\n", i+1, msb, lsb, elem,
                                name.c_str());
                        first = false;
                    }
                    else
                    {
                        printf("%3s  %3i %3i  %3i  %s\n", "", msb, lsb, elem,
                                name.c_str());
                    }
                }
                else
                {
                    printf("   <tr class=\"section\">\n");
                    if (first) {
                        printf("    <td class=\"%s\" rowspan=\"%u\">%u</td>\n",
                                     ((i % 2) != 0) ? "head_src" : "head",
                                     num, i+1);
                        first = false;
                    }

                    printf("    <td class=\"%s\">%u</td>\n", type, msb);
                    printf("    <td class=\"%s\">%u</td>\n", type, lsb);
                    printf("    <td class=\"%s\">%u</td>\n", type, elem);
                    printf("    <td class=\"name\">%s</td>\n", name.c_str());
                    printf("    <td class=\"%s\">%s</td>\n", type, type);
                    printf("   </tr>\n");
                }
            }
        }
    }

    if (found && html) {
        printf("  </table>\n");
        printf(" </body>\n");
        printf("</html>\n");
    }
}

void
print_drums(bank_t &bank, const char *dir, bool html)
{
    bool found = false;

    for (auto &b : bank)
    {
        unsigned int nl = b.first;
        entry_t &e = b.second.second;

        if (!html)
        {
            printf("\n=== %s\n", b.second.first.c_str());
            printf(" PC  key elem  drum name\n");
            printf("---  --- ----  ------------------------------\n");
        }
        else
        {
            if (!found)
            {
                printf("<!DOCTYPE html>\n\n");
                printf("<html>\n");
                printf(" <head>\n");
                printf("  <link rel=\"stylesheet\" type=\"text/css\" "
                               "href=\"adalin.css\" title=\"style\"/>\n");
                printf(" </head>\n");
                printf(" <body>\n");
                printf("  <table class=\"downloads\">\n");
                found = true;
            }

            printf("   <tr>\n");
            printf("    <td class=\"head\" colspan=\"5\">%s</td>\n",
                           b.second.first.c_str());
            printf("   </tr>\n");

            printf("   <tr>\n");
            printf("    <td class=\"head\">PC</td>\n");
            printf("    <td class=\"head\">key</td>\n");
            printf("    <td class=\"head\">elem</td>\n");
            printf("    <td class=\"head\">drum name</td>\n");
            printf("   </tr>\n");
        }

        bool first = true;
        for (auto& it : e)
        {
            unsigned int elem;

            elem = get_elem(dir, it.second.second);
            if (!html)
            {
                if (first)
                {
                    printf("%3i  %3i  %3i  %s\n",
                            nl >> 8, it.first, elem,
                            it.second.first.c_str());
                    first = false;
                }
                else {
                    printf("%3s  %3i  %3i  %s\n",
                            "", it.first, elem,
                            it.second.first.c_str());
                }
            }
            else
            {
                printf("   <tr>\n");
                if (first) {
                    printf("    <td class=\"arch\" rowspan=\"%lu\">"
                                    "%u</td>\n", e.size(), nl >> 8);
                    first = false;
                }

                printf("    <td class=\"arch\">%u</td>\n", it.first);
                printf("    <td class=\"arch\">%u</td>\n", elem);
                printf("    <td class=\"name\">%s</td>\n", it.second.first.c_str());
                printf("   </tr>\n");
            }
        }
    }
}

int main(int argc, char **argv)
{
    const char *filename;
    bool html = false;
    bank_t bank;
    void *xid;

    if (argc != 2 && argc != 3) {
        help(argv[0]);
    }

    if (argc == 3)
    {
        const char *s = getCommandLineOption(argc, argv, "--html");
        if (s) html = true;
    }

    filename = argv[1];
    xid = xmlOpen(filename);
    if (xid)
    {
        char file[1024];
        char *ptr;

        snprintf(file, 255, "%s", filename);
        ptr = strrchr(file, '/');
        if (!ptr)
        {
            snprintf(file, 255, "./%s", filename);
            ptr = strrchr(file, '/');
        }
        ptr++;

        int num = fill_bank(bank, xid, "instrument");
        if (num) print_instruments(bank, filename, html);
        else
        {
            num = fill_bank(bank, xid, "drum");
            if (num) print_drums(bank, filename, html);
        }

        if (!bank.size()) {
            printf("no insruments or drums found in %s\n", filename);
        }

        xmlClose(xid);
    }
    else {
        printf("Wanring: could not open %s\n", filename);
    }
}
