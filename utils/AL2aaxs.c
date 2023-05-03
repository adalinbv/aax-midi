#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#include "AL/efx.h"
#include "AL/efx-presets.h"

typedef struct {
    const char* name;
    EFXEAXREVERBPROPERTIES params;
} AL_effect_t;

static AL_effect_t AL_reverb_types[] = {
 { "generic", EFX_REVERB_PRESET_GENERIC },
 { "padded-cell", EFX_REVERB_PRESET_PADDEDCELL },
 { "room", EFX_REVERB_PRESET_ROOM },
 { "bathroom", EFX_REVERB_PRESET_BATHROOM },
 { "living-room", EFX_REVERB_PRESET_LIVINGROOM },
 { "room-stone", EFX_REVERB_PRESET_STONEROOM },
 { "auditorium", EFX_REVERB_PRESET_AUDITORIUM },
 { "concert-hall", EFX_REVERB_PRESET_CONCERTHALL },
 { "cave", EFX_REVERB_PRESET_CAVE },
 { "arena", EFX_REVERB_PRESET_ARENA },
 { "hangar", EFX_REVERB_PRESET_HANGAR },
 { "carpeted-hallway", EFX_REVERB_PRESET_CARPETEDHALLWAY },
 { "hallway", EFX_REVERB_PRESET_HALLWAY },
 { "stone-corridor", EFX_REVERB_PRESET_STONECORRIDOR },
 { "alley", EFX_REVERB_PRESET_ALLEY },
 { "forest", EFX_REVERB_PRESET_FOREST },
 { "city", EFX_REVERB_PRESET_CITY },
 { "mountains", EFX_REVERB_PRESET_MOUNTAINS },
 { "quarry", EFX_REVERB_PRESET_QUARRY },
 { "plain", EFX_REVERB_PRESET_PLAIN },
 { "parking-lot", EFX_REVERB_PRESET_PARKINGLOT },
 { "sewer-pipe", EFX_REVERB_PRESET_SEWERPIPE },
 { "under-water", EFX_REVERB_PRESET_UNDERWATER },
 { "drugged", EFX_REVERB_PRESET_DRUGGED },
 { "dizzy", EFX_REVERB_PRESET_DIZZY },
 { "psychotic", EFX_REVERB_PRESET_PSYCHOTIC },
 { "room-small", EFX_REVERB_PRESET_CASTLE_SMALLROOM },
 { "short-passage", EFX_REVERB_PRESET_CASTLE_SHORTPASSAGE },
 { "room-medium", EFX_REVERB_PRESET_CASTLE_MEDIUMROOM },
 { "room-large", EFX_REVERB_PRESET_CASTLE_LARGEROOM },
 { "low-passage", EFX_REVERB_PRESET_CASTLE_LONGPASSAGE },
 { "castle-hall", EFX_REVERB_PRESET_CASTLE_HALL },
 { "castle-cupboard", EFX_REVERB_PRESET_CASTLE_CUPBOARD },
 { "castle-courtyard", EFX_REVERB_PRESET_CASTLE_COURTYARD },
 { "castle-alcove", EFX_REVERB_PRESET_CASTLE_ALCOVE },
 { "factory-room-small", EFX_REVERB_PRESET_FACTORY_SMALLROOM },
 { "factory-short-passage", EFX_REVERB_PRESET_FACTORY_SHORTPASSAGE },
 { "factory-room-medium", EFX_REVERB_PRESET_FACTORY_MEDIUMROOM },
 { "factory-room-large", EFX_REVERB_PRESET_FACTORY_LARGEROOM },
 { "factory-low-passage", EFX_REVERB_PRESET_FACTORY_LONGPASSAGE },
 { "factory-hall", EFX_REVERB_PRESET_FACTORY_HALL },
 { "factory-cupboard", EFX_REVERB_PRESET_FACTORY_CUPBOARD },
 { "factory-courtyard", EFX_REVERB_PRESET_FACTORY_COURTYARD },
 { "factory-alcove", EFX_REVERB_PRESET_FACTORY_ALCOVE },
 { "icepalace-room-small", EFX_REVERB_PRESET_ICEPALACE_SMALLROOM },
 { "icepalace-short-passage", EFX_REVERB_PRESET_ICEPALACE_SHORTPASSAGE },
 { "icepalace-room-medium", EFX_REVERB_PRESET_ICEPALACE_MEDIUMROOM },
 { "icepalace-room-large", EFX_REVERB_PRESET_ICEPALACE_LARGEROOM },
 { "icepalace-low-passage", EFX_REVERB_PRESET_ICEPALACE_LONGPASSAGE },
 { "icepalace-hall", EFX_REVERB_PRESET_ICEPALACE_HALL },
 { "icepalace-cupboard", EFX_REVERB_PRESET_ICEPALACE_CUPBOARD },
 { "icepalace-courtyard", EFX_REVERB_PRESET_ICEPALACE_COURTYARD },
 { "icepalace-alcove", EFX_REVERB_PRESET_ICEPALACE_ALCOVE },
 { "spacestation-room-small", EFX_REVERB_PRESET_SPACESTATION_SMALLROOM },
 { "spacestation-short-passage", EFX_REVERB_PRESET_SPACESTATION_SHORTPASSAGE },
 { "spacestation-room-medium", EFX_REVERB_PRESET_SPACESTATION_MEDIUMROOM },
 { "spacestation-room-large", EFX_REVERB_PRESET_SPACESTATION_LARGEROOM },
 { "spacestation-low-passage", EFX_REVERB_PRESET_SPACESTATION_LONGPASSAGE },
 { "spacestation-hall", EFX_REVERB_PRESET_SPACESTATION_HALL },
 { "spacestation-cupboard", EFX_REVERB_PRESET_SPACESTATION_CUPBOARD },
 { "spacestation-", EFX_REVERB_PRESET_SPACESTATION_ALCOVE },
 { "wooden-room-small", EFX_REVERB_PRESET_WOODEN_SMALLROOM },
 { "wooden-short-passage", EFX_REVERB_PRESET_WOODEN_SHORTPASSAGE },
 { "wooden-room-medium", EFX_REVERB_PRESET_WOODEN_MEDIUMROOM },
 { "wooden-room-large", EFX_REVERB_PRESET_WOODEN_LARGEROOM },
 { "wooden-low-passage", EFX_REVERB_PRESET_WOODEN_LONGPASSAGE },
 { "wooden-hall", EFX_REVERB_PRESET_WOODEN_HALL },
 { "wooden-cupboard", EFX_REVERB_PRESET_WOODEN_CUPBOARD },
 { "wooden-", EFX_REVERB_PRESET_WOODEN_COURTYARD },
 { "wooden-", EFX_REVERB_PRESET_WOODEN_ALCOVE },
 { "sport-stadium-empty", EFX_REVERB_PRESET_SPORT_EMPTYSTADIUM },
 { "sport-squash-court", EFX_REVERB_PRESET_SPORT_SQUASHCOURT },
 { "sport-swimmingpool", EFX_REVERB_PRESET_SPORT_SMALLSWIMMINGPOOL },
 { "sport-swimmingpool-large", EFX_REVERB_PRESET_SPORT_LARGESWIMMINGPOOL },
 { "sport-gymnasium", EFX_REVERB_PRESET_SPORT_GYMNASIUM },
 { "sport-stadium-full", EFX_REVERB_PRESET_SPORT_FULLSTADIUM },
 { "sport-stadium-tannoy", EFX_REVERB_PRESET_SPORT_STADIUMTANNOY },
 { "prefab-workshop", EFX_REVERB_PRESET_PREFAB_WORKSHOP },
 { "prefab-school-room", EFX_REVERB_PRESET_PREFAB_SCHOOLROOM },
 { "prefab-practice-room", EFX_REVERB_PRESET_PREFAB_PRACTISEROOM },
 { "prefab-outhouse", EFX_REVERB_PRESET_PREFAB_OUTHOUSE },
 { "prefab-caravan", EFX_REVERB_PRESET_PREFAB_CARAVAN },
 { "dome-tomb", EFX_REVERB_PRESET_DOME_TOMB },
 { "pipe-small", EFX_REVERB_PRESET_PIPE_SMALL },
 { "dome-st.pauls", EFX_REVERB_PRESET_DOME_SAINTPAULS },
 { "pipe-long-thin", EFX_REVERB_PRESET_PIPE_LONGTHIN },
 { "pipe-large", EFX_REVERB_PRESET_PIPE_LARGE },
 { "pipe-resonant", EFX_REVERB_PRESET_PIPE_RESONANT },
 { "backyard", EFX_REVERB_PRESET_OUTDOORS_BACKYARD },
 { "rolling-plains", EFX_REVERB_PRESET_OUTDOORS_ROLLINGPLAINS },
 { "canyon-deep", EFX_REVERB_PRESET_OUTDOORS_DEEPCANYON },
 { "creek-", EFX_REVERB_PRESET_OUTDOORS_CREEK },
 { "valley-", EFX_REVERB_PRESET_OUTDOORS_VALLEY },
 { "mood-heaven", EFX_REVERB_PRESET_MOOD_HEAVEN },
 { "mood-hell", EFX_REVERB_PRESET_MOOD_HELL },
 { "mood-memory", EFX_REVERB_PRESET_MOOD_MEMORY },
 { "driving-commentator", EFX_REVERB_PRESET_DRIVING_COMMENTATOR },
 { "driving-pitrage", EFX_REVERB_PRESET_DRIVING_PITGARAGE },
 { "driving-incar-racer", EFX_REVERB_PRESET_DRIVING_INCAR_RACER },
 { "driving-incar-sports", EFX_REVERB_PRESET_DRIVING_INCAR_SPORTS },
 { "driving-incar-luxury", EFX_REVERB_PRESET_DRIVING_INCAR_LUXURY },
 { "driving-grandstand-full", EFX_REVERB_PRESET_DRIVING_FULLGRANDSTAND },
 { "driving-grandstand-empty", EFX_REVERB_PRESET_DRIVING_EMPTYGRANDSTAND },
 { "driving-tunnel", EFX_REVERB_PRESET_DRIVING_TUNNEL },
 { "streets", EFX_REVERB_PRESET_CITY_STREETS },
 { "subway", EFX_REVERB_PRESET_CITY_SUBWAY },
 { "museum", EFX_REVERB_PRESET_CITY_MUSEUM },
 { "library", EFX_REVERB_PRESET_CITY_LIBRARY },
 { "underpass", EFX_REVERB_PRESET_CITY_UNDERPASS },
 { "city-abandoned", EFX_REVERB_PRESET_CITY_ABANDONED },
 { "room-dusty", EFX_REVERB_PRESET_DUSTYROOM },
 { "chapel", EFX_REVERB_PRESET_CHAPEL },
 { "room-smallwater", EFX_REVERB_PRESET_SMALLWATERROOM },
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

#if 0
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
//       int ct = type->param[3];	// Delay Time: 0.1ms-1.0s, def. 340ms.
         int cr = type->param[4];	// Rate: 0-127
         int cd = type->param[5];	// Depth: 0-127
//       int crev = type->param[6];	// Chorus to Reverb: 0-127
//       int ddly = type->param[7];	// Chorus to Delay: 1-27

         float gain = cl/128.0f;
         float rate = 10.0f*cr/127.0f;

         float lfo_depth, lfo_offset;
         if (rate > 0.0f)
         {
            lfo_depth = cd/127.0f;
            lfo_offset = 0.0f;
         }
         else
         {
            lfo_depth = 0.0f;
            lfo_offset = cd/127.0f;
         }

         float val = (7-cfc)/7.0f;
         float fc = _log2lin(val*_lin2log(22000.0f));
         if (fc >= 20000.0f) fc = 0.0f;
         float feedback = fb/127.0f;

         fprintf(stream, "<?xml version=\"1.0\"?>\n\n");
         fprintf(stream, "<aeonwave>\n\n");
         fprintf(stream, " <audioframe>\n");
         fprintf(stream, "  <effect type=\"chorus\"");
         if (rate > 0.0f) fprintf(stream, " src=\"sine|1st-order\"");
         fprintf(stream, " src=\"1st-order\">\n");
         fprintf(stream, "   <slot n=\"0\">\n");
         fprintf(stream, "    <param n=\"0\">%.2f</param>\n", gain);
         fprintf(stream, "    <param n=\"1\">%.1f</param>\n", rate);
         fprintf(stream, "    <param n=\"2\">%.3f</param>\n", lfo_depth);
         fprintf(stream, "    <param n=\"3\">%.3f</param>\n", lfo_offset);
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

         fprintf(stream, " <mixer>\n");
         fprintf(stream, "  <effect type=\"chorus\"");
         if (rate > 0.0f) fprintf(stream, " src=\"sine|2nd-order\"");
         fprintf(stream, " src=\"2nd-order\">\n");
         fprintf(stream, "   <slot n=\"0\">\n");
         fprintf(stream, "    <param n=\"0\">%.2f</param>\n", gain);
         fprintf(stream, "    <param n=\"1\">%.1f</param>\n", rate);
         fprintf(stream, "    <param n=\"2\">%.3f</param>\n", lfo_depth);
         fprintf(stream, "    <param n=\"3\">%.3f</param>\n", lfo_offset);
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
         int dl = type->param[0];	// Delay Level 0-64-127: default 64
         int dfc = type->param[1];	// Pre LPF cutoff behavior: 0-7, def. 0
         int dtc = type->param[2];	// Delay Offset: 0.1ms-1.0s, def. 340ms.
//       int dtrl = type->param[3];	// Delay time ratio left (%)
//       int dtrr = type->param[4];	// Delay time ratio right (%)
//       int dlc = type->param[5];	// Delay Level Center (ms)
//       int dll = type->param[6];	// Delay Level Left
//       int dlr = type->param[7];	// Delay Level Right
         int fb = type->param[8];	// Feedback Level
//       int drev = type->param[9];	// Delay to reverb

         float gain = (fb < 0 ? -1.0f : 1.0f)*dl/127.0f;
         float feedback = abs(fb)/64.0f;
         float lfo_offset = dtc*1e3f;

         float val = (7-dfc)/7.0f;
         float fc = _log2lin(val*_lin2log(22000.0f));
         if (fc >= 20000.0f) fc = 0.0f;

         fprintf(stream, "<?xml version=\"1.0\"?>\n\n");
         fprintf(stream, "<aeonwave>\n\n");
         fprintf(stream, " <audioframe>\n");
         fprintf(stream, "  <effect type=\"delay\" src=\"1st-order\">\n");
         fprintf(stream, "   <slot n=\"0\">\n");
         fprintf(stream, "    <param n=\"0\">%.2f</param>\n", gain);
         fprintf(stream, "    <param n=\"1\">0.0</param>\n");
         fprintf(stream, "    <param n=\"2\">0.0</param>\n");
         fprintf(stream, "    <param n=\"3\" type=\"usec\">%.1f</param>\n", lfo_offset);
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

         fprintf(stream, " <mixer>\n");
         fprintf(stream, "  <effect type=\"delay\" src=\"2nd-order\">\n");
         fprintf(stream, "   <slot n=\"0\">\n");
         fprintf(stream, "    <param n=\"0\">%.2f</param>\n", gain);
         fprintf(stream, "    <param n=\"1\">0.0</param>\n");
         fprintf(stream, "    <param n=\"2\">0.0</param>\n");
         fprintf(stream, "    <param n=\"3\" type=\"usec\">%.1f</param>\n", lfo_offset);
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

         float delay_depth = GSMIDI_reverb_character[rc];
         float decay_level = rl/127.0f;
         float decay_time = 1.66f*rt/127.0f;
         float decay_depth = get_decay_depth(decay_time, decay_level);

         fprintf(stream, "<?xml version=\"1.0\"?>\n\n");
         fprintf(stream, "<aeonwave>\n\n");
         fprintf(stream, " <audioframe>\n");
         fprintf(stream, "  <effect type=\"reverb\" src=\"1st-order\">\n");
         fprintf(stream, "   <slot n=\"0\">\n");
         fprintf(stream, "    <param n=\"0\">%.1f</param>\n", fc);
         fprintf(stream, "    <param n=\"1\">%.3f</param>\n", delay_depth);
         fprintf(stream, "    <param n=\"2\">%.3f</param>\n", decay_level);
         fprintf(stream, "    <param n=\"3\">%.3f</param>\n", decay_depth);
         fprintf(stream, "   </slot>\n");
         fprintf(stream, "  </effect>\n");
         fprintf(stream, " </audioframe>\n\n");

         fprintf(stream, " <mixer>\n");
         fprintf(stream, "  <effect type=\"reveb\" src=\"2nd-order\">\n");
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
#endif

int main()
{
#if 0
   write_chorus();
   write_delay();
   write_reverb();
#endif
   return 0;
}
