

#include <map>
#include <algorithm>

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <xml.h>

#include "driver.h"

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
        unsigned int b, i, n;
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

                n = xmlAttributeGetInt(xbid, "n") << 8;
                n += xmlAttributeGetInt(xbid, "l");

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

                bank[n] = std::make_pair<std::string,entry_t>(std::move(bank_name),std::move(e));
            }
        }
        xmlFree(xbid);
        xmlFree(xmid);
    }
    return rv;
}

void
remove_double_spaces(std::string& str)
{
    std::string::iterator new_end =
        std::unique(str.begin(), str.end(),
        [](char lhs, char rhs){ return (lhs == rhs) && (lhs == ' '); }
        );
    str.erase(new_end, str.end());
}

void
str_prepend(std::string& name, std::string section, const char *replacement = nullptr)
{
    std::size_t pos;

    pos = name.find(section);
    if (pos == std::string::npos)
    {
        section[0] = std::toupper(section[0]);
        pos = name.find(section);
    }

    if (pos != std::string::npos)
    {
        name.replace(pos, section.size(), "");
        if (!replacement)
        {
            section[0] = std::toupper(section[0]);
            name = section + " " + name;
        }
        else name = std::string(replacement) + " " + name;
    }
    remove_double_spaces(name);
}

void
str_append(std::string& name, std::string& suffix, std::string section, const char *replacement = nullptr)
{
    std::size_t pos;

    pos = name.find(section);
    if (pos == std::string::npos)
    {
        section[0] = std::toupper(section[0]);
        pos = name.find(section);
    }

    if (pos != std::string::npos)
    {
        if (!suffix.empty()) suffix.append(", ");
        if (!replacement)
        {
            section[0] = std::tolower(section[0]);
            suffix.append(section);
        }
        else suffix.append(replacement);
        name.replace(pos, section.size(), "");
    }
    remove_double_spaces(name);
}

void
str_remove(std::string& name, std::string section)
{

    std::size_t pos = name.find(section);
    if (pos != std::string::npos) name.replace(pos, section.size(), "");
    remove_double_spaces(name);
}

std::string
canonical_name(std::string name)
{
    std::string suffix;

    str_prepend(name, "square");
    str_prepend(name, "sine Wave");
    str_prepend(name, "sine");
    str_prepend(name, "sawtooth");
    str_prepend(name, "pulse");
    str_prepend(name, "saw ", "Saw");
    str_prepend(name, "double");
    str_prepend(name, "finger");
    str_prepend(name, "pick");
    str_prepend(name, "bass");
    str_prepend(name, "mallet");
    str_prepend(name, "electric");
    str_prepend(name, "acoustic");
    str_prepend(name, "synth");
    str_prepend(name, "analog");
    str_prepend(name, "FM");
    str_prepend(name, "DX");
    str_prepend(name, "LM");
    str_prepend(name, "big");
    str_prepend(name, "thick");
    str_prepend(name, "fat");
    str_prepend(name, "digi ", "digital");
    str_prepend(name, "digital");
    str_prepend(name, "heavy");
    str_prepend(name, "wire");
    str_prepend(name, "rhythm");
    str_prepend(name, "funk");
    str_prepend(name, "jazzy", "jazz");
    str_prepend(name, "jazz");
    str_prepend(name, "rock ", "rock");
    str_prepend(name, "baroque", "chamber");
    str_prepend(name, "chamber");
    str_prepend(name, "smooth");
    str_prepend(name, "soft");
    str_prepend(name, "reverse");
    str_prepend(name, "clean");
    str_prepend(name, "over Driven");
    str_prepend(name, "overdriven", "Over Driven");
    str_prepend(name, "overdrive", "Over Driven");
    str_prepend(name, "distorted");
    str_prepend(name, "distrtion");
    str_prepend(name, "attack");
    str_prepend(name, "rubber");
    str_prepend(name, "modulated");
    str_prepend(name, "modular");
    str_prepend(name, "pedal");
    str_prepend(name, "french");
    str_prepend(name, "italian");
    str_prepend(name, "tango");
    str_prepend(name, "diapasom");
    str_prepend(name, "praise");
    str_prepend(name, "halo");
    str_prepend(name, "sweep");
    str_prepend(name, "metallic");
    str_prepend(name, "bowed");
    str_prepend(name, "poly");
    str_prepend(name, "choir");
    str_prepend(name, "itopia");
    str_prepend(name, "new Age");
    str_prepend(name, "voice");
    str_prepend(name, "charang");
    str_prepend(name, "calliope");
    str_prepend(name, "shwimmer");
    str_prepend(name, "chiff");

    str_append(name, suffix, "wide");
    str_append(name, suffix, "stereo", "wide");
    str_append(name, suffix, "dark");
    str_append(name, suffix, "bright ", "bright");
    str_append(name, suffix, "mellow");
    str_append(name, suffix, "warm");
    str_append(name, suffix, "slow");
    str_append(name, suffix, "fast");
    str_append(name, suffix, "octave Mix", "octave");
    str_append(name, suffix, "octave mix", "octave");
    str_append(name, suffix, "coupled", "octave");
    str_append(name, suffix, "octave");
    str_append(name, suffix, "phase", "phased");
    str_append(name, suffix, "phased");
    str_append(name, suffix, "chorused", "chorus");
    str_append(name, suffix, "chorus");
    str_append(name, suffix, "flanger", "flanged");
    str_append(name, suffix, "flanged");
    str_append(name, suffix, "detuned");
    str_append(name, suffix, "hard", "velocity");
    str_append(name, suffix, "wild", "velocity");
    str_append(name, suffix, "power", "velocity");
    str_append(name, suffix, "expressive", "velocity");
    str_append(name, suffix, "slow attack", "slow");
    str_append(name, suffix, "attack", "velocity");
    str_append(name, suffix, "velocity mix", "velocity");
    str_append(name, suffix, "velocity");
    str_append(name, suffix, "cross-fade");
    str_append(name, suffix, "50's", "early");
    str_append(name, suffix, "60's", "vintage");
    str_append(name, suffix, "70's", "classic");
    str_append(name, suffix, "key-off");
    str_append(name, suffix, "steel");
    str_append(name, suffix, "nylon");
    str_append(name, suffix, "muted");
    str_append(name, suffix, "modern");

    str_remove(name, "( )");
    str_remove(name, "(+ )");
    str_remove(name, ", ");
    str_remove(name, "( + Lead)");
    str_remove(name, "( lead)");
    str_remove(name, "( Pad)");
    str_remove(name, "( pad)");
    str_remove(name, "()");

    if (!suffix.empty()) {
        name += " (" + suffix + ")";
    }

    return name;
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
                char msb = nl >> 8;
                char lsb = nl & 0xf;

                if (msb == 0x79) type = "GM2";
                else if (msb == 127) type = "MT32";
                else if (nl && msb == 0) type = "XG";
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
