#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <math.h>

typedef struct {
    const char* name;
    int param[16];
} GSMIDI_effect_t;

/* REVERB
 *  p	description		range
 *  --	-----------		---------
 *  0	Delay Level		0 ~ 127
 *  1	Delay Pre-LFP		0 ~ 7
 *  2	Delay Time Center	1 ~ 115 (0.1ms ~ 1sec)*
 *  3	Delay Time Ratio Left	1 ~ 120	(4 ~ 500%)#
 *  4	Delay Time Ratio Right	1 ~ 120	(4 ~ 500%)#
 *  5	Delay Level Center	0 ~ 127
 *  6	Delay Level Left	0 ~ 127
 *  7	Delay Level Right	0 ~ 127
 *  8	Delay Feedback		0 ~ 64 ~ 127 (-64 ~ 0 ~ +63)
 *  9	Delay To Reverb		0 ~ 127
 *
 * *) The releation between the DELAY TIME CENTER value and the actual delay
 *    time is as follows:
 *    DELAY TIME	Time Range[ms]		DELAY TIME	Time Range[ms]
 *    01-14		 0.1 ~  2.0		46-50		 50.0 ~  100.0
 *    14-23		 2.0 ~  5.0		50-5a		100.0 ~  200.0
 *    23-2d		 5.0 ~ 10.0		5a-69		200.0 ~  500.0
 *    2d -37		10.0 ~ 20.0		69-73		500.0 ~ 1000.0
 *    37-46		20.0 ~ 50.0
 *
 * #) DELAY TIME RATIO LEFT and DELAY TIME RATIO RIGHT specify the ratio in
 *    releation to DELAY TIME CENTER. The resolution is (100/24)%
 */
#define GSMIDI_MAX_DELAY_TYPES		10
static GSMIDI_effect_t GSMIDI_delay_types[GSMIDI_MAX_DELAY_TYPES] = {
// param:		 0   1     2    3    4    5    6    7    8    9
 { "delay1",          { 64,  0,  340,   4,   4, 127,   0,   0,  16 } },
 { "delay2",          { 64,  0,  550,   4,   4, 127,   0,   0,  16 } },
 { "delay3",          { 64,  0, 1000,   4,   4, 127,   0,   0,   8 } },
 { "delay4",          { 64,  0,  130,   4,   4, 127,   0,   0,   8 } },
 { "pan-delay1",      { 64,  0,  500,  50, 100,   0, 125,  60,  10 } },
 { "pan-delay2",      { 64,  0,  700,  50, 100,   0, 125,  60,   7 } },
 { "pan-delay3",      { 64,  0, 1000,  50, 100,   0, 120,  64,   9 } },
 { "pan-delay4",      { 64,  0,  260,  50, 100,   0, 120,  64,   8 } },
 { "delay-to-reverb", { 64,  0,  700,  50, 100,   0, 114,  60,  -3 } },
 { "pan-repeat",      { 64,  0,  750,  88, 133,  97, 127,  67, -24 } }
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
         int dl = type->param[0];	// Delay Level
         int dc = type->param[5];	// Delay Level Center
         int fb = type->param[8];	// Feedback Level
         int dt = type->param[2];	// Delay Offset (ms)

         float gain = (2.0f*dl/128.0f); // * (dc/127.0f);
         float feedback = _MAX(fb - 64, 0)/127.0f;
         float lfo_offset = dt;

         fprintf(stream, "<?xml version=\"1.0\"?>\n\n");
         fprintf(stream, "<aeonwave>\n\n");
         fprintf(stream, " <audioframe>\n");
         fprintf(stream, "  <effect type=\"delay\" src=\"true\">\n");
         fprintf(stream, "   <slot n=\"0\">\n");
         fprintf(stream, "    <param n=\"0\">%5.3f</param>\n", gain);
         fprintf(stream, "    <param n=\"1\">0.0</param>\n");
         fprintf(stream, "    <param n=\"2\">0.0</param>\n");
         fprintf(stream, "    <param n=\"3\" type=\"usec\">%5.1f</param>\n", lfo_offset*1e3f);
         fprintf(stream, "   </slot>\n");
         if (feedback > 0.0f) {
             fprintf(stream, "   <slot n=\"1\">\n");
             fprintf(stream, "    <param n=\"0\">%2.1f</param>\n", 0.0f);
             fprintf(stream, "    <param n=\"1\">%2.1f</param>\n", 0.0f);
             fprintf(stream, "    <param n=\"2\">%5.3f</param>\n", feedback);
             fprintf(stream, "    <param n=\"3\">%2.1f</param>\n", 1.0f);
             fprintf(stream, "   </slot>\n");
         }
         fprintf(stream, "  </effect>\n");
         fprintf(stream, " </audioframe>\n\n");

         fprintf(stream, " <mixer>\n");
         fprintf(stream, "  <effect type=\"delay\" src=\"true\">\n");
         fprintf(stream, "   <slot n=\"0\">\n");
         fprintf(stream, "    <param n=\"0\">%5.3f</param>\n", gain);
         fprintf(stream, "    <param n=\"1\">0.0</param>\n");
         fprintf(stream, "    <param n=\"2\">0.0</param>\n");
         fprintf(stream, "    <param n=\"3\" type=\"usec\">%5.1f</param>\n", lfo_offset*1e3f);
         fprintf(stream, "   </slot>\n");
         if (feedback > 0.0f) {
             fprintf(stream, "   <slot n=\"1\">\n");
             fprintf(stream, "    <param n=\"0\">%2.1f</param>\n", 0.0f);
             fprintf(stream, "    <param n=\"1\">%2.1f</param>\n", 0.0f);
             fprintf(stream, "    <param n=\"2\">%5.3f</param>\n", feedback);
             fprintf(stream, "    <param n=\"3\">%2.1f</param>\n", 1.0f);
             fprintf(stream, "   </slot>\n");
         }
         fprintf(stream, "  </effect>\n");
         fprintf(stream, " </mixer>\n\n");
         fprintf(stream, "</aeonwave>\n");
      }
      else printf(" Failed to open for writing: %s\n", strerror(errno));
   }

   return 0;
}

int main()
{
   write_delay();

   return 0;
}
