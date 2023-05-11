#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <time.h>

typedef struct {
    const char* name;
    int param[16];
} GSMIDI_effect_t;

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
#define GSMIDI_MAX_CHORUS_TYPES         8
static GSMIDI_effect_t GSMIDI_chorus_types[GSMIDI_MAX_CHORUS_TYPES] = {
// param:                     0   1    2    3    4    5    6   7
 { "chorus1",              { 64,  0,   0, 112,   3,   5,   0,  0 } },
 { "chorus2",              { 64,  0,   5,  80,   9,  19,   0,  0 } },
 { "chorus3",              { 64,  0,   8,  80,   3,  19,   0,  0 } },
 { "chorus4",              { 64,  0,  16,  64,   9,  16,   0,  0 } },
 { "chorus-feedback",      { 64,  0,  64, 127,   2,  24,   0,  0 } },
 { "flanger",              { 64,  0, 112, 127,   1,   5,   0,  0 } },
 { "chorus-short-delay",   { 64,  0,   0, 127,   0, 127,   0,  0 } },
 { "chorus-delay-feedback",{ 64,  0,  80, 127,   0, 127,   0,  0 } },
};

/* DELAY
 *  p	description		range
 *  --	-----------		---------
 *  0	Delay Level		0 ~ 127
 *  1	Delay Pre-LFP		0 ~ 7
 *  2	Delay Time Center	1 ~ 115 (0.1ms ~ 1sec) (*)
 *  3	Delay Time Ratio Left	1 ~ 120	(4% ~ 500%) (#)
 *  4	Delay Time Ratio Right	1 ~ 120	(4% ~ 500%) (#)
 *  5	Delay Level Center	0 ~ 127
 *  6	Delay Level Left	0 ~ 127
 *  7	Delay Level Right	0 ~ 127
 *  8	Delay Feedback		-64 ~ 16 ~ 63 (%)
 *  9	Delay To Reverb		0 ~ 127
 *
 * *) The relation between the DELAY TIME CENTER value and the actual delay
 *    time is as follows:
 *    DELAY TIME	Time Range[ms]    |	DELAY TIME	Time Range[ms]
 *       1- 20		 0.1 ~  2.0	  |	  70- 90	 50.0 ~  100.0
 *      20- 35		 2.0 ~  5.0	  |	  80- 90	100.0 ~  200.0
 *      35- 45		 5.0 ~ 10.0	  |	  90-105	200.0 ~  500.0
 *      45- 55		10.0 ~ 20.0	  |	 105-115	500.0 ~ 1000.0
 *      55- 70		20.0 ~ 50.0	  |
 *
 * #) DELAY TIME RATIO LEFT and DELAY TIME RATIO RIGHT specify the ratio in
 *    relation to DELAY TIME CENTER. The resolution is (100/24)%
 *
 * %) With negative values, the delay will be fed back with inverted phase.
 */
#define GSMIDI_MAX_DELAY_TYPES		10
static GSMIDI_effect_t GSMIDI_delay_types[GSMIDI_MAX_DELAY_TYPES] = {
// param:		 0   1     2    3    4    5    6    7    8   9
 { "delay1",          { 64,  0,  340,   4,   4, 127,   0,   0,  16,  0 } },
 { "delay2",          { 64,  0,  550,   4,   4, 127,   0,   0,  16,  0 } },
 { "delay3",          { 64,  0, 1000,   4,   4, 127,   0,   0,   8,  0 } },
 { "delay4",          { 64,  0,  130,   4,   4, 127,   0,   0,   8,  0 } },
 { "pan-delay1",      { 64,  0,  500,  50, 100,   0, 125,  60,  10,  0 } },
 { "pan-delay2",      { 64,  0,  700,  50, 100,   0, 125,  60,   7,  0 } },
 { "pan-delay3",      { 64,  0, 1000,  50, 100,   0, 120,  64,   9,  0 } },
 { "pan-delay4",      { 64,  0,  260,  50, 100,   0, 120,  64,   8,  0 } },
 { "delay-to-reverb", { 64,  0,  700,  50, 100,   0, 114,  60,  -3, 36 } },
 { "pan-repeat",      { 64,  0,  750,  88, 133,  97, 127,  67, -24,  0 } }
};

/* REVERB
 *  p   description             range
 *  --  -----------             ---------
 *  0   Reverb Level            0 ~ 64 ~ 127
 *  1   Reverb Character        0 ~ 4 ~ 7(*)
 *  2   Reverb Pre-LFP          0 ~ 7
 *  3   Reverb Time             0 ~ 64 ~ 127
 *  4   Reverb Delay feedback   0 ~ 127
 *  5   Reverb Pre Delay Time   0 ~ 127 (ms)
 *
 * *) 0-5 are reverb effects and 6 and 7 are delay effects
 *    Corresponds to the REVERB MACRO of the same number.
 */
#define GSMIDI_MAX_REVERB_TYPES		8
static GSMIDI_effect_t GSMIDI_reverb_types[GSMIDI_MAX_REVERB_TYPES] = {
// param:                 0   1   2    3    4    5
 { "room1",            { 64,  0,  3,  80,   0,   0 } },
 { "room2",            { 64,  1,  4,  56,   0,   0 } },
 { "room3",            { 64,  2,  0,  64,   0,   0 } },
 { "hall1",            { 64,  3,  4,  72,   0,   0 } },
 { "hall2",            { 64,  4,  0,  64,   0,   0 } },
 { "plate",            { 64,  5,  0,  88,   0,   0 } },
 { "reverb-delay",     { 64,  6,  0,  32,  40,   0 } },
 { "reverb-pan-delay", { 64,  7,  0,  64,  32,   0 } },
};

static float GSMIDI_reverb_character[8] = {
 0.007f, 0.012f, 0.027f, 0.035f, 0.035f, 0.0095f, 0.0f, 0.0f
};
#define PHASING_MIN      50e-6f
#define PHASING_MAX      10e-3f
#define FLANGING_MIN     10e-3f
#define FLANGING_MAX     60e-3f
#define DELAY_MIN        60e-3f
#define DELAY_MAX       250e-3f
#define LEVEL_60DB		0.001f
#define MAX_REVERB_EFFECTS_TIME	0.700f

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
   for (int i=0; i<GSMIDI_MAX_CHORUS_TYPES; ++i)
   {
      char fname[256];

      GSMIDI_effect_t *type = &GSMIDI_chorus_types[i];

      snprintf(fname, 255, "%s.aaxs", type->name);
      printf("Generating: %s\n", fname);

      FILE *stream = fopen(fname, "w+");
      if (stream)
      {
         int cl = type->param[0];	// Level 0-64-127: default 64
         int cfc = type->param[1];	// Pre LPF cutoff behavior: 0-7, def. 0
         int fb = type->param[2];	// Feedback Level
//       int ct = type->param[3];	// Pre Delay Time: 0-100ms
         int cr = type->param[4];	// Rate: 0-127: 0.05 - 10.0
         int cd = type->param[5];	// Depth: 0-127
//       int crev = type->param[6];	// Chorus to Reverb: 0-127
//       int ddly = type->param[7];	// Chorus to Delay: 1-27

         float gain = 0.98f*cl/127.0f;
         float rate = 0.05f + 9.95f*cr/127.0f;

         float lfo_depth, lfo_offset;
         if (rate > 0.0f)
         {
            lfo_depth = 20.0f*cd/127.0f;
            lfo_offset = 10.0f;
         }
         else
         {
            lfo_depth = 0.0f;
            lfo_offset = 10.0f+20.0f*cd/127.0f;
         }
         float feedback = 0.98f*fb/127.0f;

         float val = (7-cfc)/7.0f;
         float fc = _log2lin(val*_lin2log(22000.0f));
         if (fc >= 20000.0f) fc = 0.0f;

         fprintf(stream, "<?xml version=\"1.0\"?>\n\n");

         print_header(stream);

         fprintf(stream, "<aeonwave>\n\n");

         print_info(stream);

         fprintf(stream, " <audioframe mode=\"append\">\n");
         fprintf(stream, "  <effect type=\"");
         fprintf(stream, "chorus");
         fprintf(stream, "\"");
         if (rate > 0.0f) fprintf(stream, " src=\"sine\"");
         fprintf(stream, ">\n");
         fprintf(stream, "   <slot n=\"0\">\n");
         fprintf(stream, "    <param n=\"0\">%.2f</param>\n", gain);
         fprintf(stream, "    <param n=\"1\">%.1f</param>\n", rate);
         fprintf(stream, "    <param n=\"2\" type=\"msec\">%.3f</param>\n", lfo_depth);
         fprintf(stream, "    <param n=\"3\" type=\"msec\">%.3f</param>\n", lfo_offset);
         fprintf(stream, "   </slot>\n");
         if (feedback > 0.0f || (fc > 0.0 && fc < 20000.0f)) {
             fprintf(stream, "   <slot n=\"1\">\n");
             fprintf(stream, "    <param n=\"0\">%.1f</param>\n", fc);
             fprintf(stream, "    <param n=\"1\">%.1f</param>\n", 0.0f);
             fprintf(stream, "    <param n=\"2\">%.3f</param>\n", feedback);
             fprintf(stream, "    <param n=\"3\">%.1f</param>\n", 1.0f);
             fprintf(stream, "   </slot>\n");
         }
         fprintf(stream, "  </effect>\n");
         fprintf(stream, " </audioframe>\n\n");

         fprintf(stream, " <mixer mode=\"append\">\n");
         fprintf(stream, "  <effect type=\"");
         fprintf(stream, "chorus");
         fprintf(stream, "\"");
         if (rate > 0.0f) fprintf(stream, " src=\"sine\"");
         fprintf(stream, ">\n");
         fprintf(stream, "   <slot n=\"0\">\n");
         fprintf(stream, "    <param n=\"0\">%.2f</param>\n", gain);
         fprintf(stream, "    <param n=\"1\">%.1f</param>\n", rate);
         fprintf(stream, "    <param n=\"2\" type=\"msec\">%.3f</param>\n", lfo_depth);
         fprintf(stream, "    <param n=\"3\" type=\"msec\">%.3f</param>\n", lfo_offset);
         fprintf(stream, "   </slot>\n");
         if (feedback > 0.0f || (fc > 0.0 && fc < 20000.0f)) {
             fprintf(stream, "   <slot n=\"1\">\n");
             fprintf(stream, "    <param n=\"0\">%.1f</param>\n", fc);
             fprintf(stream, "    <param n=\"1\">%.1f</param>\n", 0.0f);
             fprintf(stream, "    <param n=\"2\">%.3f</param>\n", feedback);
             fprintf(stream, "    <param n=\"3\">%.1f</param>\n", 1.0f);
             fprintf(stream, "   </slot>\n");
         }
         fprintf(stream, "  </effect>\n");
         fprintf(stream, " </mixer>\n\n");
         fprintf(stream, "</aeonwave>\n");

         fclose(stream);
      }
      else printf(" Failed to open for writing: %s\n", strerror(errno));
   }

   return 0;
}

/*
 * *) The relation between the DELAY TIME CENTER value and the actual delay
 *    time is as follows:
 *    DELAY TIME        Time Range[ms]    |     DELAYTIME       Time Range[ms]
 *       1- 20           0.1 ~  2.0       |       70- 80         50.0 ~  100.0
 *      20- 35           2.0 ~  5.0       |       80- 90        100.0 ~  200.0
 *      35- 45           5.0 ~ 10.0       |       90-105        200.0 ~  500.0
 *      45- 55          10.0 ~ 20.0       |      105-115        500.0 ~ 1000.0
 *      55- 70          20.0 ~ 50.0       |
 */
#if 0
static float
delay_time_to_offset_ms(int dt)
{
   float rv;

   if (dt < 20) {
      rv = 1e-3f*(0.1f + (2.0f-0.1f)*(dt-1)/(20.0f-1.0f));
   } else if (dt < 35) {
      rv = 1e-3f*(2.0f + (5.0f-2.0f)*(dt-20)/(35.0f-20.0f));
   } else if (dt < 45) {
      rv = 1e-3f*(5.0f + (10.0f-5.0f)*(dt-35)/(45.0f-35.0f));
   } else if (dt < 55) {
      rv = 1e-3f*(10.0f + (20.0f-10.0f)*(dt-45)/(55.0f-45.0f));
   } else if (dt < 70) {
      rv = 1e-3f*(20.0f + (50.0f-20.0f)*(dt-55)/(70.0f-55.0f));
   } else if (dt < 80) {
      rv = 1e-3f*(50.0f + (100.0f-50.0f)*(dt-70)/(80.0f-70.0f));
   } else if (dt < 90) {
      rv = 1e-3f*(100.0f + (200.0f-100.0f)*(dt-80)/(90.0f-80.0f));
   } else if (dt < 105) {
      rv = 1e-3f*(200.0f + (500.0f-200.0f)*(dt-90)/(105.0f-90.0f));
   } else {
      rv = 1e-3f*(500.0f + (1000.0f-500.0f)*(dt-105)/(115.0f-105.0f));
   }

   return rv;
}
#endif

int write_delay()
{
   for (int i=0; i<GSMIDI_MAX_DELAY_TYPES; ++i)
   {
      char fname[256];

      GSMIDI_effect_t *type = &GSMIDI_delay_types[i];

      snprintf(fname, 255, "%s.aaxs", type->name);
      printf("Generating: %s\n", fname);

      FILE *stream = fopen(fname, "w+");
      if (stream)
      {
#if 0
 *  0   Delay Level             0 ~ 127
 *  1   Delay Pre-LFP           0 ~ 7
 *  2   Delay Time Center       1 ~ 115 (0.1ms ~ 1sec) (*)
 *  3   Delay Time Ratio Left   1 ~ 120 (4% ~ 500%) (#)
 *  4   Delay Time Ratio Right  1 ~ 120 (4% ~ 500%) (#)
 *  5   Delay Level Center      0 ~ 127
 *  6   Delay Level Left        0 ~ 127
 *  7   Delay Level Right       0 ~ 127
 *  8   Delay Feedback          -64 ~ 16 ~ 63 (%)
 *  9   Delay To Reverb         0 ~ 127
 *
 * #) DELAY TIME RATIO LEFT and DELAY TIME RATIO RIGHT specify the ratio in
 *    relation to DELAY TIME CENTER. The resolution is (100/24)%
#endif
         int dl = type->param[0];	// Delay Level 0-64-127: default 64
         int dfc = type->param[1];	// Pre LPF cutoff behavior: 0-7, def. 0
         int dtc = type->param[2];	// Delay Offset: 0.1ms-1.0s, def. 340ms.
         int dtrl = type->param[3];	// Delay time ratio left (%)
         int dtrr = type->param[4];	// Delay time ratio right (%)
//       int dlc = type->param[5];	// Delay Level Center (ms)
//       int dll = type->param[6];	// Delay Level Left
//       int dlr = type->param[7];	// Delay Level Right
         int fb = type->param[8];	// Feedback Level
//       int drev = type->param[9];	// Delay to reverb
         char pan = (dtrl != dtrr);

         float gain = (fb < 0 ? -1.0f : 1.0f)*dl/127.0f;
         float feedback = abs(fb)/64.0f;
         float lfo_offset = dtc;

         float lfo_offset_l = lfo_offset * (dtrl/100.0f);
         float lfo_offset_r = lfo_offset * (dtrr/100.0f);

         float val = (7-dfc)/7.0f;
         float fc = _log2lin(val*_lin2log(22000.0f));
         if (fc >= 20000.0f) fc = 0.0f;

         fprintf(stream, "<?xml version=\"1.0\"?>\n\n");

         print_header(stream);

         fprintf(stream, "<aeonwave>\n\n");

         print_info(stream);

         fprintf(stream, " <audioframe mode=\"append\">\n");
         fprintf(stream, "  <effect type=\"delay\"");
         if (pan) fprintf(stream, " stereo=\"true\"");
         fprintf(stream, ">\n");
         fprintf(stream, "   <slot n=\"0\">\n");
         fprintf(stream, "    <param n=\"0\">%.2f</param>\n", gain);
         fprintf(stream, "    <param n=\"1\">0.0</param>\n");
         if (pan)
         {
            fprintf(stream, "    <param n=\"2\" type=\"msec\">%.1f</param>\n", lfo_offset_r);
            fprintf(stream, "    <param n=\"3\" type=\"msec\">%.1f</param>\n", lfo_offset_l);
         }
         else
         {
            fprintf(stream, "    <param n=\"2\">0.0</param>\n");
            fprintf(stream, "    <param n=\"3\" type=\"msec\">%.1f</param>\n", lfo_offset);
         }
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

         fprintf(stream, " <mixer mode=\"append\">\n");
         fprintf(stream, "  <effect type=\"delay\"");
         if (pan) fprintf(stream, " stereo=\"true\"");
         fprintf(stream, ">\n");
         fprintf(stream, "   <slot n=\"0\">\n");
         fprintf(stream, "    <param n=\"0\">%.2f</param>\n", gain);
         fprintf(stream, "    <param n=\"1\">0.0</param>\n");
         if (pan)
         {
            fprintf(stream, "    <param n=\"2\" type=\"msec\">%.1f</param>\n", lfo_offset_r);
            fprintf(stream, "    <param n=\"3\" type=\"msec\">%.1f</param>\n", lfo_offset_l);
         }
         else
         {
            fprintf(stream, "    <param n=\"2\">0.0</param>\n");
            fprintf(stream, "    <param n=\"3\" type=\"msec\">%.1f</param>\n", lfo_offset);
         }
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
         fprintf(stream, " </mixer>\n\n");
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
   for (int i=0; i<GSMIDI_MAX_REVERB_TYPES; ++i)
   {
      char fname[256];

      GSMIDI_effect_t *type = &GSMIDI_reverb_types[i];

      snprintf(fname, 255, "%s.aaxs", type->name);
      printf("Generating: %s\n", fname);

      FILE *stream = fopen(fname, "w+");
      if (stream)
      {
         int rl = type->param[0];	// Reverb Level 0-64-127: default 64
         int rc = type->param[1];	// Reverb Character: 0-5, 6, 7
         int rfc = type->param[2];	// Pre LPF cutoff behavior: 0-7, def. 0
         int rt = type->param[3];	// Reverb Time: 0-64-127
//       int rb = type->param[4];	// Reverb Delay Feedback: 0-127
//       int dpd = type->param[5];	// Reverb pre Delay Time: 0-127 ms

         float val = (7-rfc)/7.0f;
         float fc = 500.0f+_log2lin(val*_lin2log(22000.0f-500.0f));
         if (fc >= 20000.0f) fc = 0.0f;

//       float delay_depth = dpd*1e-3f;
         float delay_depth = GSMIDI_reverb_character[rc];
         float decay_level = rl/127.0f;
         float decay_time = 1.66f*rt/127.0f;
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

         fprintf(stream, " <mixer mode=\"append\">\n");
         fprintf(stream, "  <effect type=\"reveb\">\n");
         fprintf(stream, "   <slot n=\"0\">\n");
         fprintf(stream, "    <param n=\"0\">%.1f</param>\n", fc);
         fprintf(stream, "    <param n=\"1\">%.3f</param>\n", delay_depth);
         fprintf(stream, "    <param n=\"2\">%.3f</param>\n", decay_level);
         fprintf(stream, "    <param n=\"3\">%.3f</param>\n", decay_depth);
         fprintf(stream, "   </slot>\n");
         fprintf(stream, "  </effect>\n");
         fprintf(stream, " </mixer>\n\n");
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
   write_delay();
   write_reverb();

   return 0;
}
