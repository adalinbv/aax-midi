
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <map>
#include <vector>
#include <iterator>
#include <algorithm>
#include <iostream>
#include <filesystem>
#include <regex>

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <stdlib.h>

#include <xml.h>

#include "driver.h"
#include "common.h"
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
    std::string key_on;
    std::string key_off;
    float spread;
    bool patch;
    bool stereo;
    int wide;
};
                      // inst_pos, name
using entry_t = std::map<unsigned,name_t>;
                      // bank_pos(msb,lsb)   bank_name, entry
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
" * Copyright (C) 2017-%s by Erik Hofman.\n"
" * Copyright (C) 2017-%s by Adalin B.V.\n"
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
" *\n"
" * - key-on=\"<instrument>\"\n"
" *   Add a pitched, non-looping sound effect when the key is pressed.\n"
" *   If a non-pitched sound is required then add a pitch-fraction of 0.0\n"
" *\n"
" * - key-off=\"<instrument>\"\n"
" *   Add a pitched, non-looping sound effect when the key is released.\n"
" *   If a non-pitched sound is required then add a pitch-fraction of 0.0\n"
"-->\n";

char inst_set[65] = "";

unsigned int
get_elem(const char *dir, std::string &file)
{
    unsigned int rv = 0;
    xmlId *xid;

    std::filesystem::path path(dir);
    path.make_preferred();

    std::filesystem::path fname = path;
    fname.replace_filename(file + ".aaxs");

    xid = xmlOpen(fname.string().c_str());
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
    else
    {
        fname = path;
        fname.replace_filename(file + ".xml");
        xid = xmlOpen(fname.string().c_str());
        if (xid)
        {
            xmlId *xlid = xmlNodeGet(xid, "/instrument/layer");
            if (xlid)
            {
                rv = xmlNodeGetNum(xlid, "patch");
                if (!rv) rv = 1;
                xmlFree(xlid);
            }
            xmlFree(xid);
        }
    }

    return rv;
}

static char inst_offs = 0;
int
fill_bank(bank_t& bank, xmlId *xid, const char *tag, char clear)
{
    int rv = 0;

    if (clear) bank.clear();

    xmlId *xmid;
    xmid = xmlNodeGet(xid, "/aeonwave/midi");
    if (xmid)
    {
        unsigned int bnum = xmlNodeGetNum(xmid, "bank");
        xmlId *xbid = xmlMarkId(xmid);
        unsigned int b, i, bank_pos;
        char file[1024];
        char name[1024];

        xmlAttributeCopyString(xmid, "name", inst_set, 64);
        for (b=0; b<bnum; ++b)
        {
            if (xmlNodeGetPos(xmid, xbid, "bank", b) != 0)
            {
                unsigned int slen, inum = xmlNodeGetNum(xbid, tag);
                xmlId *xiid = xmlMarkId(xbid);

                std::string bank_name;
                slen = xmlAttributeCopyString(xiid, "name", name, 64);
                if (slen) bank_name = name;

                bank_pos = xmlAttributeGetInt(xbid, "n") << 16;
                bank_pos += xmlAttributeGetInt(xbid, "l");

                inst_offs = xmlAttributeGetInt(xbid, "offset");
                if (!inst_offs) inst_offs = 1;
                else inst_offs--;

                entry_t entry;
                for (i=0; i<inum; ++i)
                {
                    if (xmlNodeGetPos(xbid, xiid, tag, i) != 0)
                    {
                        unsigned int pos = xmlAttributeGetInt(xiid, "n");

                        slen = xmlAttributeCopyString(xiid, "name", name, 64);
                        if (slen)
                        {
                            std::string key_on, key_off;
                            bool patch, stereo;
                            float spread;
                            int wide;

                            patch = false;
                            if (!xmlAttributeCopyString(xiid, "file", file, 64))
                            {
                                if (xmlAttributeCopyString(xiid, "patch", file, 64)) {
                                    patch = true;
                                }
                            }

                            if (xmlAttributeExists(xiid, "key-on")) {
                                key_on = xmlAttributeGetString(xiid, "key-on");
                            }
                            if (xmlAttributeExists(xiid, "key-off")) {
                                key_off = xmlAttributeGetString(xiid, "key-off");
                            }


                            spread = xmlAttributeGetDouble(xiid, "spread");

                            stereo = xmlAttributeGetBool(xiid, "stereo");
                            if (stereo && wide == -1) wide = 0;

                            if (xmlAttributeCompareString(xiid, "wide","true")){
                                wide = xmlAttributeGetInt(xiid, "wide");
                            } else {
                                wide = -1;
                            }
                            if (!wide) wide = xmlAttributeGetBool(xiid, "wide");

                            entry[pos] = {
                                name,
                                file,
                                key_on,
                                key_off,
                                spread,
                                patch,
                                stereo,
                                wide
                            };
                            rv++;
                        }
                    }
                }
                xmlFree(xiid);

                bank[bank_pos] = std::make_pair<std::string,entry_t>(std::move(bank_name),std::move(entry));
            }
        }
        xmlFree(xbid);
        xmlFree(xmid);
    }
    return rv;
}

static char in_file = 0;
void
print_xml(bank_t &bank, bank_t &bank2, const char *dir, bool it)
{
    const char *inst = it ? "instrument" : "drum";
    std::string name = "Advanced Grooves";
    std::string date = "@YEAR@";

    if (inst_set[0]) {
        name = inst_set;
    }

    if (!in_file)
    {
        std::time_t t = std::time(nullptr);
        std::tm *const pTInfo = std::localtime(&t);
        date = std::to_string(1900 + pTInfo->tm_year);
    }

    printf("<?xml version=\"1.0\"?>\n");
    printf(xml_header, date.c_str(), date.c_str());
    printf("\n\n<aeonwave>\n");
    if (!in_file) {
        printf("\n <midi name=\"%s\">\n", name.c_str());
    } else {
        printf("\n <midi name=\"%s\" version=\"@ULTRASYNTH_VERSION@\">\n", name.c_str());
    }

    const char *t = "";
    for (auto &b : bank)
    {
        entry_t &entry = b.second.second;
        unsigned int bank_pos = b.first;
        const char *type = "GM";
        int bank_msb = bank_pos >> 16;
        int bank_lsb = bank_pos & 0xffff;
//
        if (bank_msb == 0x79) type = "GM2";
        else if (bank_msb == 127 && bank_lsb == 0) type = "MT32";
        else if ((bank_msb == 64 && bank_lsb == 0) ||
                 (bank_lsb != 0 && bank_msb == 0))
        {
            type = "XG";
        }
        else if (bank_msb) type = "GS";

        if (strcmp(t, type))
        {
           printf("  <!-- %s -->\n", type);
           t = type;
        }

        bool found = false;
        for (auto it_entry : entry)
        {
            name_t &it_name = it_entry.second;
            if (!found)
            {
                if (bank_lsb != 0 ) {
                    printf("  <bank n=\"%i\" l=\"%i\" offset=\"%i\">\n", bank_msb, bank_lsb, inst_offs);
                } else {
                    printf("  <bank n=\"%i\" offset=\"%i\">\n", bank_msb, inst_offs);
                }
                found = true;
            }

            std::string name = canonical_name(it_name.name);
            std::string& filename = it_name.file;
            std::string &key_on = it_name.key_on;
            std::string &key_off = it_name.key_off;
            bool stereo = it_name.stereo;
            float spread = it_name.spread;
            int wide = it_name.wide;
            bool patch = it_name.patch;
            bool found2 = false;

            for (auto &it_bank2 : bank2)
            {
                entry_t &entry2 = it_bank2.second.second;
                unsigned int bank_pos2 = it_bank2.first;
                if (bank_pos2 == bank_pos)
                {
                   for (auto it_entry2 : entry2)
                   {
                      if (it_entry2.first == it_entry.first)
                      {
                          filename = it_entry2.second.file;
                          stereo = it_name.stereo;
                          spread = it_entry2.second.spread;
                          wide = it_entry2.second.wide;
                          patch = it_entry2.second.patch;
                          found2 = true;
                          break;
                      }
                   }
                }
                if (found2) break;
            }

            if (!found2) {
                found2 = check_file(it_entry.second.file.c_str(), patch ? ".xml" : ".aaxs");
            }

            printf("   <%s%s n=\"%i\" name=\"%s\"",
                        found2 ? "" : "unsupported-", inst, it_entry.first+inst_offs,
                        name.c_str());
            if (patch) printf(" patch=\"%s\"", filename.c_str());
            else printf(" file=\"%s\"", filename.c_str());
            if (!key_on.empty())  printf(" key-on=\"%s\"", key_on.c_str());
            if (!key_off.empty()) printf(" key-off=\"%s\"", key_off.c_str());
            if (stereo) printf(" stereo=\"true\"");
            if (wide) {
               if (wide == -1) printf(" wide=\"true\"");
               else printf(" wide=\"%i\"", wide);
            }
	    if (spread) printf(" spread=\"%3.2g\"", spread);
            printf("/>\n");
        }

        if (found) printf("  </bank>\n\n");
    }

    printf("\n </midi>\n");
    printf("\n</aeonwave>\n");
}

void
print_instruments(bank_t &bank, bank_t &bank2, const char *dir, enum mode_e mode, const char *app)
{
    bool found = false;

    for (int i=1; i<129; ++i)
    {
        if ((i % 8) == 1)
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
                    printf("<!-- created using: %s -->\n", app);
                    printf("<html>\n");
                    printf(" <head>\n");
                    printf("  <link rel=\"stylesheet\" type=\"text/css\" "
                                   "href=\"/adalin.css\" title=\"style\"/>\n");
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
                printf("    <td class=\"head\" width=\"20px\">PC</td>\n");
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
        for (auto &it_bank : bank)
        {
            entry_t &entry = it_bank.second.second;
            auto it = entry.find(i);
            if (it != entry.end()) num++;
        }

        bool first = true;
        for (auto &it_bank : bank)
        {
            unsigned int bank_pos = it_bank.first;
            entry_t &entry = it_bank.second.second;
            auto it = entry.find(i);
            if (it != entry.end())
            {
                const char *type = "GM";
                unsigned int elem;
                int bank_msb = bank_pos >> 16;
                int bank_lsb = bank_pos & 0xffff;

                if (bank_msb == 0x79) type = "GM2";
                else if (bank_msb == 127 && bank_lsb == 0) {
                    type = "MT32";
                }
                else if ((bank_msb == 64 && bank_lsb == 0) ||
                         (bank_lsb != 0 && bank_msb == 0))
                {
                    type = "XG";
                } else if (bank_msb != 0) type = "GS";

                elem = get_elem(dir, it->second.file);
                std::string name = canonical_name(it->second.name);
                switch(mode)
                {
                case ASCII:
                    if (first)
                    {
                        printf("%3i  %3i %3i  %3i  %s\n", i+inst_offs,
                                bank_msb, bank_lsb, elem, name.c_str());
                        first = false;
                    }
                    else
                    {
                        printf("%3s  %3i %3i  %3i  %s\n", "",
                                bank_msb, bank_lsb, elem, name.c_str());
                    }
                    break;
                case HTML:
                    printf("   <tr class=\"section\">\n");
                    if (first) {
                        printf("    <td class=\"%s\" rowspan=\"%u\">%u</td>\n",
                                     ((i % 2) != 0) ? "head_src" : "head",
                                     num, i+inst_offs);
                        first = false;
                    }

                    printf("    <td class=\"%s\">%u</td>\n", type, bank_msb);
                    printf("    <td class=\"%s\">%u</td>\n", type, bank_lsb);
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
        entry_t &entry = b.second.second;
        int msb = b.first & 0xffff;
        int lsb = b.first >> 16;

        switch(mode)
        {
        case ASCII:
            printf("\n=== %s\n", b.second.first.c_str());
            printf("BANK   PC  elem  drum name\n");
            printf("------ --- ----  ------------------------------\n");
            break;
        case HTML:
            if (!found)
            {
                printf("<!DOCTYPE html>\n\n");
                printf("<html>\n");
                printf(" <head>\n");
                printf("  <link rel=\"stylesheet\" type=\"text/css\" "
                               "href=\"/adalin.css\" title=\"style\"/>\n");
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
            printf("    <td class=\"head\">bank</td>\n");
            printf("    <td class=\"head\" width=\"20px\">PC</td>\n");
            printf("    <td class=\"head\">elem</td>\n");
            printf("    <td class=\"head\">drum name</td>\n");
            printf("   </tr>\n");
            break;
        default:
            break;
        }

        bool first = true;
        for (auto& it_entry : entry)
        {
            name_t &it_name = it_entry.second;
            unsigned int elem;

            elem = get_elem(dir, it_name.file);
            switch(mode)
            {
            case ASCII:
                if (first)
                {
                    printf("%3i %1i  %3i  %3i  %s\n",
                            msb, lsb, it_entry.first, elem,
                            it_name.name.c_str());
                    first = false;
                }
                else {
                    printf("%5s  %3i  %3i  %s\n",
                            "", it_entry.first, elem,
                            it_name.name.c_str());
                }
                break;
            case HTML:
                printf("   <tr>\n");
                if (first) {
                    printf("    <td class=\"arch\" rowspan=\"%" PRIu64 "\" style=\"vertical-align:top\">"
                                    "%3i %1i</td>\n", entry.size(), msb, lsb);
                    first = false;
                }

                printf("    <td class=\"arch\">%u</td>\n", it_entry.first);
                printf("    <td class=\"arch\">%u</td>\n", elem);
                printf("    <td class=\"name\">%s</td>\n", it_name.name.c_str());
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

    env = getCommandLineOption(argc, argv, "--to-midi");
    if (env) inst_offs = 1;

    env = getCommandLineOption(argc, argv, "--combine");
    if (env) xid2 = xmlOpen(env);
    else xid2 = nullptr;

    filename = getInputFile(argc, argv, NULL);
    if (filename)
    {
        size_t slen = strlen(filename);
        if (slen > 3 && !strcmp(filename+slen-3, ".in")) {
            in_file = 1;
        }
    }

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
            num = fill_bank(bank, xid, "instrument", 1);
            if (num) {
                print_instruments(bank, bank2, filename, mode, argv[0]);
            }
            else
            {
                num = fill_bank(bank, xid, "drum", 1);
                if (num) {
                    print_drums(bank, bank2, filename, mode);
                }
            }

            if (!bank.size()) {
                printf("no insruments or drums found in %s\n", filename);
            }
            break;
        case XML:
            num = fill_bank(bank, xid, "*", 1);
            if (num)
            {
               if (xid2) num = fill_bank(bank2, xid2, "instrument", 1);
               print_xml(bank, bank2, filename, true);
            }
            else
            {
                num = fill_bank(bank, xid, "drum", 1);
                if (num)
                {
                    if (xid2) num = fill_bank(bank2, xid2, "drum", 1);
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
    else
    {
        printf("Warning: could not open %s\n", filename);
        if (xmlErrorGetNo(xid, 0) != XML_NO_ERROR)
        {
             printf("%s\n", xmlErrorGetString(xid, 0));
             printf("  at line: %i, column: %i\n",
                       xmlErrorGetLineNo(xid, 0), xmlErrorGetColumnNo(xid, 1));
        }
    }
}
