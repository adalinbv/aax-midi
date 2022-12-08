

#include <map>
#include <vector>
#include <iterator>
#include <algorithm>
#include <iostream>
#include <regex>

#include <stdio.h>
#include <string.h>
#include <strings.h>
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

    printf("Usage: %s -i <file>\n", pname);
    printf("\nWhere <file> is either the gmmidi.xml or gmdrums.xml file.\n");

    printf("\nOptions:\n");
    printf("  --combine=<file2>\t\tUse file names from this file.\n");
    printf("  --mode=<ascii|html|xml>\tSet the output mode.\n");

    printf("\n");
    exit(-1);
}

using name_t = struct {
    std::string name;
    std::string file;
    bool stereo;
    int wide;
    float spread;
    bool patch;
};
//using name_t = std::pair<std::string,std::string>;
using entry_t = std::map<unsigned,name_t>;
using bank_t = std::map<unsigned,std::pair<std::string,entry_t>>;

const char *sections[] = {
 "Piano", "Chromatic Percussion", "Organ", "Guitar", "Bass",
 "Strings", "Ensemble", "Brass", "Reed", "Pipe",
 "Synth Lead", "Synth Pad", "Synth Effects", "Ethnic/World", "Percussive",
 "Sound Effects"
};

enum mode_e {
   ASCII = 0,
   HTML,
   XML
};

const char *xml_header =
"<!--\n"
" * Copyright (C) 2017-@YEAR@ by Erik Hofman.\n"
" * Copyright (C) 2017-@YEAR@ by Adalin B.V.\n"
" * All rights reserved.\n"
" *\n"
" * Reference: https://en.wikipedia.org/wiki/General_MIDI_Level_2\n"
" *\n"
" * GS MIDI:\n"
" * - Bank Select LSB is used to select a family (1 = SC-55, 2 = SC-88, etc)\n"
" *\n"
" * ** instrument only **\n"
" * - wide=\"true\"\n"
" *   Spread all notes across the stereo space based on the key number.\n"
" *   This equals to: spread=\"1.0\" wide=\"127\"\n"
" *\n"
" * - wide=\"[+-]<num>\" (e.g. wide=\"-4\")\n"
" *   Split the stereo space up in <num> sections and assign a key to one of the\n"
" *   sections based on key number. If the number is negative the left-right\n"
" *   assignment will be reversed.\n"
" *\n"
" * - spread=\"[0.0 .. 1.0]\"\n"
" *   Limit the stereo spread around the center. Setting spread to \"0.5\" will\n"
" *   limit the stereo spread between -0.5 left and +0.5 right.\n"
" *\n"
" * - stereo=\"true\"\n"
" *   Sets wide to true if wide was not already set and add effects to make\n"
" *   3d sounds sound spatial.\n"
"-->\n";


unsigned int
get_elem(const char *dir, std::string &file)
{
    unsigned int rv = 0;
    size_t fpos;
    xmlId *xid;

    std::string path(dir);
    fpos = path.find_last_of("/\\");
    if (fpos != std::string::npos) {
        path = path.substr(0, fpos+1);
    }
    path.append(file);
    path.append(".xml");

    xid = xmlOpen(path.c_str());
    if (xid)
    {
        xmlId *xsid = xmlNodeGet(xid, "/aeonwave/sound");
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
fill_bank(bank_t& bank, xmlId *xid, const char *tag)
{
    int rv = 0;

    bank.clear();

    xmlId *xmid;
    xmid = xmlNodeGet(xid, "/aeonwave/midi");
    if (xmid)
    {
        unsigned int bnum = xmlNodeGetNum(xmid, "bank");
        xmlId *xbid = xmlMarkId(xmid);
        unsigned int b, i, nl;
        char file[1024];
        char name[1024];

        for (b=0; b<bnum; ++b)
        {
            if (xmlNodeGetPos(xmid, xbid, "bank", b) != 0)
            {
                unsigned int slen, inum = xmlNodeGetNum(xbid, tag);
                xmlId *xiid = xmlMarkId(xbid);

                std::string bank_name;
                slen = xmlAttributeCopyString(xiid, "name", name, 64);
                if (slen) bank_name = name;

                nl = xmlAttributeGetInt(xbid, "n") << 8;
                nl += xmlAttributeGetInt(xbid, "l");

                entry_t e;
                for (i=0; i<inum; ++i)
                {
                    bool patch = false;
                    if (xmlNodeGetPos(xbid, xiid, tag, i) != 0)
                    {
                        unsigned int pos = xmlAttributeGetInt(xiid, "n");

                        if (!xmlAttributeCopyString(xiid, "file", file, 64))
                        {
                            xmlAttributeCopyString(xiid, "patch", file, 64);
                            patch = true;
                        }

                        slen = xmlAttributeCopyString(xiid, "name", name, 64);
                        if (slen)
                        {
                            float spread;
                            bool stereo;
                            int wide;

                            wide = xmlAttributeGetInt(xiid, "wide");
                            if (!wide) wide = xmlAttributeGetBool(xiid, "wide");

                            stereo = xmlAttributeGetBool(xiid, "stereo");
                            if (stereo && wide == -1) wide = 0;

                            spread = xmlAttributeGetDouble(xiid, "spread");
                            e[pos] = {name,file,stereo,wide,spread,patch};
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
print_xml(bank_t &bank, bank_t &bank2, const char *dir, bool it)
{
    const char *inst = it ? "instrument" : "drum";
    printf("<?xml version=\"1.0\"?>\n");
    printf("%s\n", xml_header);
    printf("\n<aeonwave>\n");
    printf("\n <midi name=\"Advanced Grooves\" version=\"@ULTRASYNTH_VERSION@\">\n");

    const char *t = "";
    for (auto &b : bank)
    {
        entry_t &e = b.second.second;
        unsigned int nl = b.first;
        const char *type = "GM";
        int msb = nl >> 8;
        int lsb = nl & 0xff;
        int i = 0;
//
        if (msb == 0x79) type = "GM2";
        else if (msb == 127 && !lsb) type = "MT32";
        else if ((msb == 64 && !lsb) || (lsb && msb == 0)) type = "XG";
        else if (msb) type = "GS";

        if (strcmp(t, type))
        {
           printf("  <!-- %s -->\n", type);
           t = type;
        }

        bool found = false;
        for (auto it : e)
        {
            if (!found)
            {
                if (lsb) printf("  <bank n=\"%i\" l=\"%i\">\n", msb, lsb);
                else printf("  <bank n=\"%i\">\n", msb);
                found = true;
            }

            std::string name = canonical_name(it.second.name);
            std::string& filename = it.second.file;
            bool stereo = it.second.stereo;
            float spread = it.second.spread;
            int wide = it.second.wide;
            bool patch = it.second.patch;

            bool found2 = false;
            for (auto &b2 : bank2)
            {
                entry_t &e2 = b2.second.second;
                unsigned int nl2 = b2.first;
                if (nl2 == nl)
                {
                   for (auto it2 : e2)
                   {
                      if (it2.first == it.first)
                      {
                          filename = it2.second.file;
                          stereo = it.second.stereo;
                          spread = it2.second.spread;
                          wide = it2.second.wide;
                          patch = it2.second.patch;
                          found2 = true;
                          break;
                      }
                   }
                }
                if (found2) break;
            }

            printf("   <%s%s n=\"%i\" name=\"%s\"",
                        found2 ? "" : "unsupported-", inst, it.first,
                        name.c_str());
            if (patch) printf(" patch=\"%s\"", filename.c_str());
            else printf(" file=\"%s\"", filename.c_str());
            if (stereo) printf(" stereo=\"true\"");
            if (wide) {
               if (wide == -1) printf(" wide=\"true\"");
               else printf(" wide=\"%i\"", wide);
            }
            if (spread) printf(" spread=\"%2.1f\"", spread);
            printf("/>\n");
        }

        if (found) printf("  </bank>\n\n");
    }

    printf("\n </midi>\n");
    printf("\n</aeonwave>\n");
}

void
print_instruments(bank_t &bank, bank_t &bank2, const char *dir, enum mode_e mode)
{
    bool found = false;

    for (int i=0; i<128; ++i)
    {
        if ((i % 8) == 0)
        {
            switch (mode)
            {
            case ASCII:
                printf("\n=== %s\n", sections[i/8]);
                printf(" PC  msb lsb elem  instrument name\n");
                printf("---  --- --- ----  ------------------------------\n");
                break;
            case HTML:
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
                break;
            default:
                break;
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

                elem = get_elem(dir, it->second.file);
                std::string name = canonical_name(it->second.name);
                switch(mode)
                {
                case ASCII:
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
                    break;
                case HTML:
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
                    break;
                default:
                    break;
                }
            }
        }
    }

    if (found)
    {
        switch(mode)
        {
        case HTML:
            printf("  </table>\n");
            printf(" </body>\n");
            printf("</html>\n");
            break;
        default:
            break;
        }
    }
}

void
print_drums(bank_t &bank, bank_t &bank2, const char *dir, enum mode_e mode)
{
    bool found = false;

    for (auto &b : bank)
    {
        unsigned int nl = b.first;
        entry_t &e = b.second.second;

        switch(mode)
        {
        case ASCII:
            printf("\n=== %s\n", b.second.first.c_str());
            printf(" PC  key elem  drum name\n");
            printf("---  --- ----  ------------------------------\n");
            break;
        case HTML:
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
            break;
        default:
            break;
        }

        bool first = true;
        for (auto& it : e)
        {
            unsigned int elem;

            elem = get_elem(dir, it.second.file);
            switch(mode)
            {
            case ASCII:
                if (first)
                {
                    printf("%3i  %3i  %3i  %s\n",
                            nl >> 8, it.first, elem,
                            it.second.name.c_str());
                    first = false;
                }
                else {
                    printf("%3s  %3i  %3i  %s\n",
                            "", it.first, elem,
                            it.second.name.c_str());
                }
                break;
            case HTML:
                printf("   <tr>\n");
                if (first) {
                    printf("    <td class=\"arch\" rowspan=\"%lu\">"
                                    "%u</td>\n", e.size(), nl >> 8);
                    first = false;
                }

                printf("    <td class=\"arch\">%u</td>\n", it.first);
                printf("    <td class=\"arch\">%u</td>\n", elem);
                printf("    <td class=\"name\">%s</td>\n", it.second.name.c_str());
                printf("   </tr>\n");
                break;
            default:
                break;
            }
        }
    }
}

int main(int argc, char **argv)
{
    const char *filename;
    enum mode_e mode = ASCII;
    const char *env;
    bank_t bank, bank2;
    xmlId *xid2;
    xmlId *xid;

    if (argc < 2) help(argv[0]);

    env = getCommandLineOption(argc, argv, "--mode");
    if (env)
    {
        if (!strcasecmp(env, "HTML")) mode = HTML;
        else if (!strcasecmp(env, "XML")) mode = XML;
    }

    env = getCommandLineOption(argc, argv, "--combine");
    if (env) xid2 = xmlOpen(env);
    else xid2 = nullptr;

    filename = getInputFile(argc, argv, NULL);
    xid = xmlOpen(filename);
    if (xid)
    {
        char file[1024];
        char *ptr;
        int num;

        snprintf(file, 255, "%s", filename);
        ptr = strrchr(file, '/');
        if (!ptr)
        {
            snprintf(file, 255, "./%s", filename);
            ptr = strrchr(file, '/');
        }
        ptr++;

        switch(mode)
        {
        case ASCII:
        case HTML:
            num = fill_bank(bank, xid, "instrument");
            if (num) {
                print_instruments(bank, bank2, filename, mode);
            }
            else
            {
                num = fill_bank(bank, xid, "drum");
                if (num) {
                    print_drums(bank, bank2, filename, mode);
                }
            }

            if (!bank.size()) {
                printf("no insruments or drums found in %s\n", filename);
            }
            break;
        case XML:
            num = fill_bank(bank, xid, "instrument");
            if (num)
            {
               if (xid2) num = fill_bank(bank2, xid2, "instrument");
               print_xml(bank, bank2, filename, true);
            }
            else
            {
                num = fill_bank(bank, xid, "drum");
                if (num)
                {
                    if (xid2) num = fill_bank(bank2, xid2, "drum");
                    print_xml(bank, bank2, filename, false);
                }
            }

            if (!bank.size()) {
                printf("no insruments or drums found in %s\n", filename);
            }
            break;
        default:
            break;
        }

        xmlClose(xid);
        if (xid2) xmlClose(xid2);
    }
    else {
        printf("Wanring: could not open %s\n", filename);
    }
}
