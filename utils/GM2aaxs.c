#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <time.h>

typedef struct {
    const char* name;
    int param[16];
} GM2MIDI_effect_t;

#define CHORUS_MIN	0.0

/* CHORUS
 *  p   description             range
 *  --  -----------             ---------
 *  0   Chorus Level            0 ~ 64 ~ 127
 *  1   Chorus Pre-LFP          0 ~ 7(*)
 *  2   Chorus Feedback         0 ~  8 ~ 127
 *  3   Chorus Delay            0 ~ 90 ~ 127(#)
 *  4   Chorus Rate             0 ~  3 ~ 127
 *  5   Chorus Depth            0 ~ 19 ~ 127
 *  6   Chorus to Reverb        0 ~ 127
 *  7   Chorus to Delay         0 ~ 127
 *
 * *) Pre-LPF higher values will cut more of the high frequencies.
 * #) Initial delay before chorus starts.
 */
#define GM2MIDI_MAX_CHORUS_TYPES         6
static GM2MIDI_effect_t GM2MIDI_chorus_types[GM2MIDI_MAX_CHORUS_TYPES] = {
// param:                     0   1    2    3    4    5    6   7
 { "chorus1",              { 64,  0,   0, 112,   3,   5,   0,  0 } },
 { "chorus2",              { 64,  0,   5,  80,   9,  19,   0,  0 } },
 { "chorus3",              { 64,  0,   8,  80,   3,  19,   0,  0 } },
 { "chorus4",              { 64,  0,  16,  64,   9,  16,   0,  0 } },
 { "chorus-feedback",      { 64,  0,  65, 127,   2,  24,   0,  0 } },
 { "flanger",              { 64,  0, 112, 127,   1,   5,   0,  0 } },
};

/* REVERB
 *  p   description             range
 *  --  -----------             ---------
 *  0   Reverb Level            0 ~ 64 ~ 127
 *  1   Reverb Character        0 ~ 4
 *  2   Reverb Pre-LFP          0 ~ 7
 *  3   Reverb Time             0 ~ 64 ~ 127
 *  4   Reverb Delay feedback   0 ~ 127
 *  5   Reverb Pre Delay Time   0 ~ 127 (ms)
 */
#define GM2MIDI_MAX_REVERB_TYPES		6
static GM2MIDI_effect_t GM2MIDI_reverb_types[GM2MIDI_MAX_REVERB_TYPES] = {
// param:                 0   1   2    3    4    5
 { "room-small",       { 64,  0,  3,  44,   0,   0 } },
 { "room-medium",      { 64,  1,  4,  50,   0,   0 } },
 { "room-large",       { 64,  2,  0,  56,   0,   0 } },
 { "hall-medium",      { 64,  3,  4,  64,   0,   0 } },
 { "hall-large",       { 64,  4,  0,  64,   0,   0 } },
 { "plate",            { 64,  5,  0,  50,   0,   0 } },
};

static float GM2MIDI_reverb_character[8] = {
 0.007f, 0.012f, 0.027f, 0.035f, 0.035f, 0.0095f, 0.0f, 0.0f
};
#define LEVEL_60DB		0.001f

#define _MAX(a,b)       (((a)>(b)) ? (a) : (b))
#define _MIN(a,b)       (((a)<(b)) ? (a) : (b))
#define _MINMAX(a,b,c)  (((a)>(c)) ? (c) : (((a)<(b)) ? (b) : (a)))

float _lin2log(float v) { return log10f(v); }
float _log2lin(float v) { return powf(10.0f,v); }
float _lin2db(float v) { return 20.0f*log10f(v); }
float _db2lin(float v) { return _MINMAX(powf(10.0f,v/20.0f),0.0f,10.0f); }

void
print_header(FILE *stream)
{
    time_t seconds=time(NULL);
    struct tm* current_time=localtime(&seconds);
    int year = current_time->tm_year + 1900;

    fprintf(stream, "\n<!--\n");
    fprintf(stream, " * Copyright (C) 2017-%d by Erik Hofman.\n", year);
    fprintf(stream, " * Copyright (C) 2017-%d by Adalin B.V.\n", year);
    fprintf(stream, " * All rights reserved.\n");
    fprintf(stream, " *\n");
    fprintf(stream, " * This file is part of AeonWave and covered by the\n");
    fprintf(stream, " * Creative Commons Attribution-ShareAlike 4.0 International Public License\n");
    fprintf(stream, " * https://creativecommons.org/licenses/by-sa/4.0/legalcode\n");
    fprintf(stream, "-->\n\n");

}

void
print_info(FILE *stream)
{
    time_t seconds=time(NULL);
    struct tm* current_time=localtime(&seconds);
    int year = current_time->tm_year + 1900;

    fprintf(stream, "\n <info>\n");
    fprintf(stream, "  <license type=\"Attribution-ShareAlike 4.0 International\"/>\n");
    fprintf(stream, "  <copyright from=\"2017\" until=\"%d\" by=\"Adalin B.V.\"/>\n", year);
    fprintf(stream, "  <copyright from=\"2017\" until=\"%d\" by=\"Erik Hofman\"/>\n", year);
    fprintf(stream, "  <contact author=\"Erik Hofman\" website=\"aeonwave.xyz\"/>\n");
    fprintf(stream, " </info>\n\n");
}

int write_chorus()
{
   for (int i=0; i<GM2MIDI_MAX_CHORUS_TYPES; ++i)
   {
      char fname[256];

      GM2MIDI_effect_t *type = &GM2MIDI_chorus_types[i];

      snprintf(fname, 255, "%s.aaxs", type->name);
      printf("Generating: %s\n", fname);

      FILE *stream = fopen(fname, "w+");
      if (stream)
      {
         int cl = type->param[0];	// Level 0-64-127: default 64
         int cfc = type->param[1];	// Pre LPF cutoff behavior: 0-7, def. 0
         int fb = type->param[2];	// Feedback Level
         int dt = type->param[3];	// Delay
         int cr = type->param[4];	// Rate: 0-127
         int cd = type->param[5];	// Depth: 0-127

         float gain = cl/128.0f;
         float rate = cr*0.122f;

         float lfo_depth, lfo_offset;
         if (rate > 0.0f)
         {
            lfo_depth = (cd+1)/3.2f; // ms
            lfo_offset = 20e-3f*dt;
         }
         else
         {
            lfo_depth = 0.0f;
            lfo_offset = CHORUS_MIN + (cd+1)/3.2f;
         }

         float val = (7-cfc)/7.0f;
         float fc = _log2lin(val*_lin2log(22000.0f));
         if (fc >= 20000.0f) fc = 0.0f;
         float feedback = fb*0.763f/100.0f;

         fprintf(stream, "<?xml version=\"1.0\"?>\n\n");

         print_header(stream);

         fprintf(stream, "<aeonwave>\n\n");

         print_info(stream);

         fprintf(stream, " <audioframe mode=\"append\">\n");
         fprintf(stream, "  <effect type=\"");
         if (lfo_depth < 80.0f) fprintf(stream, "chorus");
         else fprintf(stream, "delay");
         fprintf(stream, "\"");
         if (rate > 0.0f) fprintf(stream, " src=\"sine\"");
         fprintf(stream, ">\n");
         fprintf(stream, "   <slot n=\"0\">\n");
         fprintf(stream, "    <param n=\"0\">%.2f</param>\n", gain);
         fprintf(stream, "    <param n=\"1\">%.1f</param>\n", rate);
         fprintf(stream, "    <param n=\"2\" type=\"msec\">%.3f</param>\n", lfo_depth);
         fprintf(stream, "    <param n=\"3\" type=\"msec\">%.3f</param>\n", lfo_offset);
         fprintf(stream, "   </slot>\n");
         if (feedback > 0.0f || fc < 20000.0f) {
             fprintf(stream, "   <slot n=\"1\">\n");
             fprintf(stream, "    <param n=\"0\">%.1f</param>\n", fc);
             fprintf(stream, "    <param n=\"1\">%.1f</param>\n", 0.0f);
             fprintf(stream, "    <param n=\"2\">%.3f</param>\n", feedback);
             fprintf(stream, "    <param n=\"3\">%.1f</param>\n", 1.0f);
             fprintf(stream, "   </slot>\n");
         }
         fprintf(stream, "  </effect>\n");
         fprintf(stream, " </audioframe>\n\n");
         fprintf(stream, "</aeonwave>\n");

         fclose(stream);
      }
      else printf(" Failed to open for writing: %s\n", strerror(errno));
   }

   return 0;
}

static float
get_decay_depth(float time, float level) {
    if (level < 0.001f) level = 0.001f;
    return 5.0f*time*powf(LEVEL_60DB, level);
}

int write_reverb()
{
   for (int i=0; i<GM2MIDI_MAX_REVERB_TYPES; ++i)
   {
      char fname[256];

      GM2MIDI_effect_t *type = &GM2MIDI_reverb_types[i];

      snprintf(fname, 255, "%s.aaxs", type->name);
      printf("Generating: %s\n", fname);

      FILE *stream = fopen(fname, "w+");
      if (stream)
      {
         int rl = type->param[0];	// Reverb Level 0-64-127: default 64
         int rc = type->param[1];	// Reverb Character: 0-5, 6, 7
         int rfc = type->param[2];	// Pre LPF cutoff behavior: 0-7, def. 0
         int rt = type->param[3];	// Reverb Time: 0-64-127

         float val = (7-rfc)/7.0f;
         float fc = 500.0f+_log2lin(val*_lin2log(22000.0f-500.0f));
         if (fc >= 20000.0f) fc = 0.0f;

         float delay_depth = GM2MIDI_reverb_character[rc];
         float decay_level = rl/127.0f;
         float decay_time = expf((rt-40)*0.025f);
         float decay_depth = get_decay_depth(decay_time, decay_level);

         fprintf(stream, "<?xml version=\"1.0\"?>\n\n");

         print_header(stream);

         fprintf(stream, "<aeonwave>\n\n");

         print_info(stream);

         fprintf(stream, " <audioframe mode=\"append\">\n");
         fprintf(stream, "  <effect type=\"reverb\">\n");
         fprintf(stream, "   <slot n=\"0\">\n");
         fprintf(stream, "    <param n=\"0\">%.1f</param>\n", fc);
         fprintf(stream, "    <param n=\"1\">%.3f</param>\n", delay_depth);
         fprintf(stream, "    <param n=\"2\">%.3f</param>\n", decay_level);
         fprintf(stream, "    <param n=\"3\">%.3f</param>\n", decay_depth);
         fprintf(stream, "   </slot>\n");
         fprintf(stream, "  </effect>\n");
         fprintf(stream, " </audioframe>\n\n");
         fprintf(stream, "</aeonwave>\n");

         fclose(stream);
      }
      else printf(" Failed to open for writing: %s\n", strerror(errno));
   }

   return 0;
}

int main()
{
   write_chorus();
   write_reverb();

   return 0;
}
