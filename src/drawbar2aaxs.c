/*
 * Copyright (C) 2018-2021 by Erik Hofman.
 * Copyright (C) 2018-2021 by Adalin B.V.
 * All rights reserved.
 *
 * This file is part of AeonWave-MIDI
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  version 3 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <strings.h>
#include <string.h>

#include <base/types.h>

#include "driver.h"

struct params {
    char percussion;
    char harmonic;
    char overdrive;
    char commons;
    char leslie;
    char chorus;
    char reverb;
    char detuned;

    char program;
    char bank;
    char *name;

    char *drawbar;
    float db[9];
};

static float _db2lin(float v) {
    return _MINMAX(powf(10.0f,v/20.0f),0.0f,10.0f);
}

static const char* format_float6(float f)
{
    static char buf[32];

    if (f >= 100.0f) {
        snprintf(buf, 20, "%.1f", f);
    }
    else
    {
        snprintf(buf, 20, "%.6g", f);
        if (!strchr(buf, '.')) {
            strcat(buf, ".0");
        }
    }
    return buf;
}

void print_layer(FILE* output, struct params *param, int layer)
{
    float pitch[9] = { 0.5f, 0.75f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 8.0f };
    float total;
    int i, num;

    fprintf(output, "  <layer n=\"%i\">\n", layer);
    fprintf(output, "   <waveform src=\"white-noise\" ratio=\"0.1\"/>\n");

    num = 0;
    total = 0.0f;
    for (i=0; i<9; ++i) {
        if (param->db[i] > 0.f) ++num;
        total += _db2lin(-3.0f*(8.0f-param->db[i]));
    }
    total *= 0.5f;

    if (num)
    {
        for (i=0; i<9; ++i)
        {
            float v = _db2lin(-3.0f*(8.0f-param->db[i]))/total;
            if (param->db[i] > 0.f)
            {
                if (layer == 1 && (i == 3 || i == 4)) {
                    v *= 2.0f;
                }

                if (!i) {
                    fprintf(output, "   <waveform src=\"sine\" ratio=\"%s\"", format_float6(v));
                } else {
                    fprintf(output, "   <waveform src=\"sine\" processing=\"add\" ratio=\"%s\"", format_float6(v));
                }
                if (pitch[i] != 1.0f) {
                    fprintf(output, " pitch=\"%s\"", format_float6(pitch[i]));
                }
                fprintf(output, "/>\n");
            }
        }
    }
    fprintf(output, "  </layer>\n");
}

void print_aaxs(const char* outfile, struct params param)
{
    FILE *output;
    struct tm* tm_info;
    time_t timer;
    char year[5];
    int i;

    time(&timer);
    tm_info = localtime(&timer);
    strftime(year, 5, "%Y", tm_info);

    if (outfile)
    {
        output = fopen(outfile, "w");
        testForError(output, "Output file could not be created.");
    }
    else {
        output = stdout;
    }

    fprintf(output, "<?xml version=\"1.0\"?>\n\n");

    fprintf(output, "<!--\n");
    fprintf(output, " * Copyright (C) 2017-%s by Erik Hofman.\n", year);
    fprintf(output, " * Copyright (C) 2017-%s by Adalin B.V.\n", year);
    fprintf(output, " * All rights reserved.\n");
    fprintf(output, " *\n");

    if (!param.commons)
    {
        fprintf(output, " * This is UNPUBLISHED PROPRIETARY SOURCE CODE; the contents of this file may\n");
        fprintf(output, " * not be disclosed to third parties, copied or duplicated in any form, in\n");
        fprintf(output, " * whole or in part, without the prior written permission of the author.\n");
    }
    else
    {
        fprintf(output, " * This file is part of AeonWave and covered by the\n");
        fprintf(output, " * Creative Commons Attribution-ShareAlike 4.0 International Public License\n");
        fprintf(output, " * https://creativecommons.org/licenses/by-sa/4.0/legalcode\n");
    }
    fprintf(output, " *\n");

    fprintf(output, " * Drawbar Settings: ");
    for (i=0; i<2; ++i) fprintf(output, "%1.0f", param.db[i]);
    fprintf(output, " ");
    for (i=2; i<6; ++i) fprintf(output, "%1.0f", param.db[i]);
    fprintf(output, " ");
    for (i=6; i<9; ++i) fprintf(output, "%1.0f", param.db[i]);
    fprintf(output, "\n");
    fprintf(output, " * Percussive      : %s, harmonic: %i\n", param.percussion ? ((param.percussion == 2) ? "fast" : "slow") : "no", param.harmonic);
    fprintf(output, " * Overdrive       : ");
    if (param.overdrive == 3) fprintf(output, "strong\n");
    else if (param.overdrive == 2) fprintf(output, "medium\n");
    else if (param.overdrive == 1) fprintf(output, "mild\n");
    else fprintf(output, "no\n");
    fprintf(output, " * Leslie          : %s\n", param.leslie ? ((param.leslie > 1) ? "fast" : "slow") : "no");
    fprintf(output, " * Chorus          : %s\n", param.chorus ? "yes" : "no");
    fprintf(output, " * Reverb          : %s\n", param.reverb ? "yes" : "no");
    fprintf(output, "-->\n\n");

    fprintf(output, "<aeonwave>\n\n");

    fprintf(output, " <info name=\"%s\" bank=\"%i\" program=\"%i\">\n",
                      param.name, param.bank, param.program);
    if (param.commons) {
        fprintf(output, "  <license type=\"Attribution-ShareAlike 4.0 International\"/>\n");
    } else {
        fprintf(output, "  <license type=\"Proprietary/Commercial\"/>\n");
    }
    fprintf(output, "  <copyright from=\"2017\" until=\"%s\" by=\"Erik Hofman\"/>\n", year);
    fprintf(output, "  <copyright from=\"2017\" until=\"%s\" by=\"Adalin B.V.\"/>\n", year);
    fprintf(output, "  <note polyphony=\"10\" min=\"36\" max=\"96\" step=\"12\"/>\n");
    fprintf(output, " </info>\n\n");

    if (param.reverb) {
        fprintf(output, " <sound gain=\"2.0\" frequency=\"55\" voices=\"3\" spread=\"0.1\">\n");
    } else {
        fprintf(output, " <sound gain=\"1.0\" frequency=\"55\">\n");
    }

    print_layer(output, &param, 0);
    if (param.percussion) {
        print_layer(output, &param, 1);
    }

    fprintf(output, " </sound>\n\n");

    fprintf(output, " <emitter looping=\"true\">\n");
    if (param.percussion)
    {
        fprintf(output, "  <filter type=\"dynamic-layer\" src=\"inverse-timed\">\n");
        fprintf(output, "    <param n=\"0\">0.2</param>\n");
        fprintf(output, "    <param n=\"1\">0.3</param>\n");
        fprintf(output, "    <param n=\"2\">1.0</param>\n");
        fprintf(output, "    <param n=\"3\">0.0</param>\n");
        fprintf(output, "  </filter>\n");
        fprintf(output, "  <filter type=\"frequency\" src=\"timed\">\n");
        fprintf(output, "   <slot n=\"0\">\n");
        fprintf(output, "    <param n=\"0\" pitch=\"%g\">%4.1f</param>\n", 1.2f*param.harmonic, 55.0f*param.harmonic/1.2f);
        fprintf(output, "    <param n=\"1\">1.0</param>\n");
        fprintf(output, "    <param n=\"2\">0.0</param>\n");
        fprintf(output, "    <param n=\"3\">1.0</param>\n");
        fprintf(output, "   </slot>\n");
        fprintf(output, "   <slot n=\"1\">\n");
        fprintf(output, "    <param n=\"0\" pitch=\"8.0\">1760.0</param>\n");
        fprintf(output, "    <param n=\"1\">0.0</param>\n");
        fprintf(output, "    <param n=\"2\">0.0</param>\n");
        fprintf(output, "    <param n=\"3\">%g</param>\n", (param.percussion == 2) ? 0.1f : 0.5f );
        fprintf(output, "   </slot>\n");
        fprintf(output, "  </filter>\n");
    }
    fprintf(output, "  <filter type=\"timed-gain\" src=\"envelope\"");
    if (param.reverb) fprintf(output, " release-factor=\"7.0\"");
    fprintf(output, ">\n");
    fprintf(output, "   <slot n=\"0\">\n");
    fprintf(output, "    <param n=\"0\">%g</param>\n", param.percussion ? 1.5f : 0.25f);
    fprintf(output, "    <param n=\"1\">%g</param>\n", (param.percussion == 1) ? 0.16f : 0.08f);
    fprintf(output, "    <param n=\"2\">1.2</param>\n");
    fprintf(output, "    <param n=\"3\">inf</param>\n");
    fprintf(output, "   </slot>\n");
    fprintf(output, "   <slot n=\"1\">\n");
    fprintf(output, "    <param n=\"0\">1.2</param>\n");
    fprintf(output, "    <param n=\"1\">%g</param>\n", param.reverb ? 0.7 : 0.2);
    fprintf(output, "    <param n=\"2\">0.0</param>\n");
    fprintf(output, "    <param n=\"3\">0.0</param>\n");
    fprintf(output, "   </slot>\n");
    fprintf(output, "  </filter>\n");
    if (param.detuned)
    {
        fprintf(output, "  <effect type=\"timed-pitch\">\n");
        fprintf(output, "   <slot n=\"0\">\n");
        fprintf(output, "    <param n=\"0\">0.987</param>\n");
        fprintf(output, "    <param n=\"1\">0.0</param>\n");
        fprintf(output, "    <param n=\"2\">0.0</param>\n");
        fprintf(output, "    <param n=\"3\">0.0</param>\n");
        fprintf(output, "   </slot>\n");
        fprintf(output, "  </effect>\n");
    }
    fprintf(output, " </emitter>\n\n");

    fprintf(output, " <audioframe>\n");
    fprintf(output, "  <filter type=\"equalizer\" optional=\"true\">\n");
    fprintf(output, "   <slot n=\"0\">\n");
    fprintf(output, "    <param n=\"0\">65.0</param>\n");
    fprintf(output, "    <param n=\"1\">0.5</param>\n");
    fprintf(output, "    <param n=\"2\">1.0</param>\n");
    fprintf(output, "    <param n=\"3\">1.0</param>\n");
    fprintf(output, "   </slot>\n");
    fprintf(output, "   <slot n=\"1\">\n");
    fprintf(output, "    <param n=\"0\">3700.0</param>\n");
    fprintf(output, "    <param n=\"1\">1.0</param>\n");
    fprintf(output, "    <param n=\"2\">0.0</param>\n");
    fprintf(output, "    <param n=\"3\">1.0</param>\n");
    fprintf(output, "   </slot>\n");
    fprintf(output, "  </filter>\n");
    if (param.overdrive)
    {
        fprintf(output, "  <effect type=\"distortion\" optional=\"true\">\n");
        fprintf(output, "   <slot n=\"0\">\n");
        fprintf(output, "    <param n=\"0\">%g</param>\n", 0.1f*param.overdrive);
        fprintf(output, "    <param n=\"1\">0.0</param>\n");
        fprintf(output, "    <param n=\"2\">0.15</param>\n");
        fprintf(output, "    <param n=\"3\">1.0</param>\n");
        fprintf(output, "   </slot>\n");
        fprintf(output, "  </effect>\n");
    }
    if (param.chorus)
    {
        if (param.leslie) {
            fprintf(output, "  <effect type=\"chorus\" src=\"sine\" optional=\"true\">\n");
        } else {
            fprintf(output, "  <effect type=\"chorus\" optional=\"true\">\n");
        }
        fprintf(output, "   <slot n=\"0\">\n");
        fprintf(output, "    <param n=\"0\">0.383</param>\n");
        (fprintf(output, "    <param n=\"1\">%g</param>\n", param.leslie ? ((param.leslie == 1) ? 1.54f : 5.54f) : 0.0f));
        fprintf(output, "    <param n=\"2\">0.03</param>\n");
        fprintf(output, "    <param n=\"3\">0.9</param>\n");
        fprintf(output, "   </slot>\n");
        fprintf(output, "  </effect>\n");
    }
    else if (param.leslie)
    {
        fprintf(output, "  <effect type=\"phasing\" src=\"sine\" optional=\"true\">\n");
        fprintf(output, "   <slot n=\"0\">\n");
        fprintf(output, "    <param n=\"0\">0.383</param>\n");
        fprintf(output, "    <param n=\"1\">%g</param>\n", (param.leslie == 1) ? 1.54f : 5.54f);
        fprintf(output, "    <param n=\"2\">0.15</param>\n");
        fprintf(output, "    <param n=\"3\">0.5</param>\n");
        fprintf(output, "   </slot>\n");
        fprintf(output, "  </effect>\n");
    }
    fprintf(output, " </audioframe>\n\n");

    fprintf(output, "</aeonwave>\n");

    if (outfile) {
        fclose(output);
    }
}

void help()
{
    printf("drawbar2aaxs version %i.%i.%i\n\n", AAX_MIDI_MAJOR_VERSION,
                                                AAX_MIDI_MINOR_VERSION,
                                                AAX_MIDI_MICRO_VERSION);
    printf("Usage: drawbar2aaxs [options]\n");
    printf("Creates an AAXS configuration file based on the drawbar organ\n");
    printf("drawbar settings.\n");

    printf("\nOptions:\n");
    printf(" -o, --output <file>\t\twrite the new .aaxs configuration to this file.\n");
    printf(" -d, --drawbar <XXXXXXXXX>\tUse these drawbar settings.\n");
    printf("     --omit-cc-by\t\tDo not add the CC-BY license reference.\n");
//  printf("     --chorus\t\t\tAdd the chorus effect.\n");
    printf("     --leslie <slow|fast>\tAdd the Leslie speaker in slow or fast mode.\n");
    printf("     --overdrive <mild|strong>\tAdd a mild or strong tube overdrive effect.\n");
    printf("     --percussion <slow|fast>,h\tAdd the percussion effect with a hamronic.\n");
    printf("     --reverb\t\t\tAdd the reverb effect.\n");
    printf(" -n, --name <s>\t\t\tProvide the display name.\n");
    printf(" -p, --program <n>\t\t\tProvide the MIDI program number.\n");
    printf(" -b, --bank <n>\t\t\tProvide the MIDI bank number.\n");
    printf("     --detuned\t\t\tMake the organ detuned.\n");
    printf(" -h, --help\t\t\tprint this message and exit\n");

    printf("\nWhen no output file is specified then stdout will be used.\n");
    printf("Note: Either Leslie speaker or reverb can be used but not both.\n");
//  printf("      Reverb also turns on chorus automatically.\n");

    printf("\n");
    exit(-1);
}

int main(int argc, char **argv)
{
    char *s, *outfile;
    struct params param;

    memset(&param, 0, sizeof(struct params));
    param.harmonic = 3;
    param.commons = 1;

    if (argc == 1 || getCommandLineOption(argc, argv, "-h") ||
                    getCommandLineOption(argc, argv, "--help"))
    {
        help();
    }

    if (getCommandLineOption(argc, argv, "--omit-cc-by")) {
        param.commons = 0;
    }

    s = getCommandLineOption(argc, argv, "--percussion");
    if (s)
    {
        char *p = strchr(s, ',');
        int len;
        if (!p)  {
            len = strlen(s);
        } else {
            len = p-s;
            param.harmonic = _MINMAX(atoi(p+1), 2, 3);
        }
        if (!strncasecmp(s, "fast", len)) param.percussion = 2;
        else param.percussion = 1;
    }

    if (getCommandLineOption(argc, argv, "--chorus")) {
        param.chorus = 1;
    }

    if (getCommandLineOption(argc, argv, "--reverb"))
    {
        param.reverb = 1;
        param.chorus = 1;
    }

    s = getCommandLineOption(argc, argv, "--leslie");
    if (s)
    {
        if (!strcasecmp(s, "fast")) param.leslie = 2;
        else param.leslie = 1;
    }

    s = getCommandLineOption(argc, argv, "--overdrive");
    if (s)
    {
        if (!strcasecmp(s, "strong")) param.overdrive = 3;
        else if (!strcasecmp(s, "medium")) param.overdrive = 2;
        else param.overdrive = 1;
    }

    outfile = getOutputFile(argc, argv, NULL);
    param.drawbar = getCommandLineOption(argc, argv, "-d");
    if (!param.drawbar) {
        param.drawbar = getCommandLineOption(argc, argv, "--drawbar");
    }

    s = getCommandLineOption(argc, argv, "-n");
    if (!s) s = getCommandLineOption(argc, argv, "--name");
    if (s) param.name = s;
    else param.name = "Drawbar";

    s = getCommandLineOption(argc, argv, "-p");
    if (!s) s = getCommandLineOption(argc, argv, "--program");
    if (s) param.program = atoi(s);

    s = getCommandLineOption(argc, argv, "-b");
    if (!s) s = getCommandLineOption(argc, argv, "--bank");
    if (s) param.bank = atoi(s);

    s = getCommandLineOption(argc, argv, "--detuned");
    if (s) param.detuned = 1;

    if (param.drawbar)
    {
        int max, i;

        max = strlen(param.drawbar);
        if (max > 9) i = 9;
        for(i=0; i<max; ++i) {
            param.db[i] = (float)(param.drawbar[i] - '0');
        }

        print_aaxs(outfile, param);
    }
    else {
        help();
    }

    return 0;
}

