#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <time.h>

#include "EAX/efx-presets.h"

typedef struct {
    const char* name;
    EFXEAXREVERBPROPERTIES param;
} EAX_effect_t;

#define EAX_MAX_REVERB_TYPES	113

static EAX_effect_t EAX_reverb_types[EAX_MAX_REVERB_TYPES] = {
 { "generic", EFX_REVERB_PRESET_GENERIC },
 { "padded-cell", EFX_REVERB_PRESET_PADDEDCELL },
 { "room", EFX_REVERB_PRESET_ROOM },
 { "bathroom", EFX_REVERB_PRESET_BATHROOM },
 { "living-room", EFX_REVERB_PRESET_LIVINGROOM },
 { "room-stone", EFX_REVERB_PRESET_STONEROOM },
 { "auditorium", EFX_REVERB_PRESET_AUDITORIUM },
 { "hall-concert", EFX_REVERB_PRESET_CONCERTHALL },
 { "cave", EFX_REVERB_PRESET_CAVE },
 { "arena", EFX_REVERB_PRESET_ARENA },
 { "hangar", EFX_REVERB_PRESET_HANGAR },
 { "hallway-carpeted", EFX_REVERB_PRESET_CARPETEDHALLWAY },
 { "hallway", EFX_REVERB_PRESET_HALLWAY },
 { "corridor-stone", EFX_REVERB_PRESET_STONECORRIDOR },
 { "alley", EFX_REVERB_PRESET_ALLEY },
 { "forest", EFX_REVERB_PRESET_FOREST },
 { "city", EFX_REVERB_PRESET_CITY },
 { "mountains", EFX_REVERB_PRESET_MOUNTAINS },
 { "quarry", EFX_REVERB_PRESET_QUARRY },
 { "plain", EFX_REVERB_PRESET_PLAIN },
 { "parking-lot", EFX_REVERB_PRESET_PARKINGLOT },
 { "pipe-sewer", EFX_REVERB_PRESET_SEWERPIPE },
 { "underwater", EFX_REVERB_PRESET_UNDERWATER },
 { "drugged", EFX_REVERB_PRESET_DRUGGED },
 { "dizzy", EFX_REVERB_PRESET_DIZZY },
 { "psychotic", EFX_REVERB_PRESET_PSYCHOTIC },
 { "room-small", EFX_REVERB_PRESET_CASTLE_SMALLROOM },
 { "passage-short", EFX_REVERB_PRESET_CASTLE_SHORTPASSAGE },
 { "room-medium", EFX_REVERB_PRESET_CASTLE_MEDIUMROOM },
 { "room-large", EFX_REVERB_PRESET_CASTLE_LARGEROOM },
 { "passage-long", EFX_REVERB_PRESET_CASTLE_LONGPASSAGE },
 { "castle-hall", EFX_REVERB_PRESET_CASTLE_HALL },
 { "castle-cupboard", EFX_REVERB_PRESET_CASTLE_CUPBOARD },
 { "castle-courtyard", EFX_REVERB_PRESET_CASTLE_COURTYARD },
 { "castle-alcove", EFX_REVERB_PRESET_CASTLE_ALCOVE },
 { "factory-room-small", EFX_REVERB_PRESET_FACTORY_SMALLROOM },
 { "factory-passage-short", EFX_REVERB_PRESET_FACTORY_SHORTPASSAGE },
 { "factory-room-medium", EFX_REVERB_PRESET_FACTORY_MEDIUMROOM },
 { "factory-room-large", EFX_REVERB_PRESET_FACTORY_LARGEROOM },
 { "factory-passage-long", EFX_REVERB_PRESET_FACTORY_LONGPASSAGE },
 { "factory-hall", EFX_REVERB_PRESET_FACTORY_HALL },
 { "factory-cupboard", EFX_REVERB_PRESET_FACTORY_CUPBOARD },
 { "factory-courtyard", EFX_REVERB_PRESET_FACTORY_COURTYARD },
 { "factory-alcove", EFX_REVERB_PRESET_FACTORY_ALCOVE },
 { "ice-palace-room-small", EFX_REVERB_PRESET_ICEPALACE_SMALLROOM },
 { "ice-palace-passage-short", EFX_REVERB_PRESET_ICEPALACE_SHORTPASSAGE },
 { "ice-palace-room-medium", EFX_REVERB_PRESET_ICEPALACE_MEDIUMROOM },
 { "ice-palace-room-large", EFX_REVERB_PRESET_ICEPALACE_LARGEROOM },
 { "ice-palace-passage-long", EFX_REVERB_PRESET_ICEPALACE_LONGPASSAGE },
 { "ice-palace-hall", EFX_REVERB_PRESET_ICEPALACE_HALL },
 { "ice-palace-cupboard", EFX_REVERB_PRESET_ICEPALACE_CUPBOARD },
 { "ice-palace-courtyard", EFX_REVERB_PRESET_ICEPALACE_COURTYARD },
 { "ice-palace-alcove", EFX_REVERB_PRESET_ICEPALACE_ALCOVE },
 { "space-station-room-small", EFX_REVERB_PRESET_SPACESTATION_SMALLROOM },
 { "space-station-passage-short", EFX_REVERB_PRESET_SPACESTATION_SHORTPASSAGE },
 { "space-station-room-medium", EFX_REVERB_PRESET_SPACESTATION_MEDIUMROOM },
 { "space-station-room-large", EFX_REVERB_PRESET_SPACESTATION_LARGEROOM },
 { "space-station-passage-long", EFX_REVERB_PRESET_SPACESTATION_LONGPASSAGE },
 { "space-station-hall", EFX_REVERB_PRESET_SPACESTATION_HALL },
 { "space-station-cupboard", EFX_REVERB_PRESET_SPACESTATION_CUPBOARD },
 { "space-station-alcove", EFX_REVERB_PRESET_SPACESTATION_ALCOVE },
 { "wooden-room-small", EFX_REVERB_PRESET_WOODEN_SMALLROOM },
 { "wooden-passage-short", EFX_REVERB_PRESET_WOODEN_SHORTPASSAGE },
 { "wooden-room-medium", EFX_REVERB_PRESET_WOODEN_MEDIUMROOM },
 { "wooden-room-large", EFX_REVERB_PRESET_WOODEN_LARGEROOM },
 { "wooden-passage-long", EFX_REVERB_PRESET_WOODEN_LONGPASSAGE },
 { "wooden-hall", EFX_REVERB_PRESET_WOODEN_HALL },
 { "wooden-cupboard", EFX_REVERB_PRESET_WOODEN_CUPBOARD },
 { "wooden-courtyard", EFX_REVERB_PRESET_WOODEN_COURTYARD },
 { "wooden-alcove", EFX_REVERB_PRESET_WOODEN_ALCOVE },
 { "stadium-empty", EFX_REVERB_PRESET_SPORT_EMPTYSTADIUM },
 { "squash-court", EFX_REVERB_PRESET_SPORT_SQUASHCOURT },
 { "swimmingpool", EFX_REVERB_PRESET_SPORT_SMALLSWIMMINGPOOL },
 { "swimmingpool-large", EFX_REVERB_PRESET_SPORT_LARGESWIMMINGPOOL },
 { "gymnasium", EFX_REVERB_PRESET_SPORT_GYMNASIUM },
 { "stadium-full", EFX_REVERB_PRESET_SPORT_FULLSTADIUM },
 { "stadium-tannoy", EFX_REVERB_PRESET_SPORT_STADIUMTANNOY },
 { "workshop", EFX_REVERB_PRESET_PREFAB_WORKSHOP },
 { "room-school", EFX_REVERB_PRESET_PREFAB_SCHOOLROOM },
 { "room-practice", EFX_REVERB_PRESET_PREFAB_PRACTISEROOM },
 { "outhouse", EFX_REVERB_PRESET_PREFAB_OUTHOUSE },
 { "caravan", EFX_REVERB_PRESET_PREFAB_CARAVAN },
 { "dome-tomb", EFX_REVERB_PRESET_DOME_TOMB },
 { "pipe-small", EFX_REVERB_PRESET_PIPE_SMALL },
 { "dome-st.pauls", EFX_REVERB_PRESET_DOME_SAINTPAULS },
 { "pipe-long-thin", EFX_REVERB_PRESET_PIPE_LONGTHIN },
 { "pipe-large", EFX_REVERB_PRESET_PIPE_LARGE },
 { "pipe-resonant", EFX_REVERB_PRESET_PIPE_RESONANT },
 { "backyard", EFX_REVERB_PRESET_OUTDOORS_BACKYARD },
 { "plains-rolling", EFX_REVERB_PRESET_OUTDOORS_ROLLINGPLAINS },
 { "canyon-deep", EFX_REVERB_PRESET_OUTDOORS_DEEPCANYON },
 { "creek", EFX_REVERB_PRESET_OUTDOORS_CREEK },
 { "valley", EFX_REVERB_PRESET_OUTDOORS_VALLEY },
 { "mood-heaven", EFX_REVERB_PRESET_MOOD_HEAVEN },
 { "mood-hell", EFX_REVERB_PRESET_MOOD_HELL },
 { "mood-memory", EFX_REVERB_PRESET_MOOD_MEMORY },
 { "commentator", EFX_REVERB_PRESET_DRIVING_COMMENTATOR },
 { "pit-rage", EFX_REVERB_PRESET_DRIVING_PITGARAGE },
 { "incar-racer", EFX_REVERB_PRESET_DRIVING_INCAR_RACER },
 { "incar-sports", EFX_REVERB_PRESET_DRIVING_INCAR_SPORTS },
 { "incar-luxury", EFX_REVERB_PRESET_DRIVING_INCAR_LUXURY },
 { "grandstand-full", EFX_REVERB_PRESET_DRIVING_FULLGRANDSTAND },
 { "grandstand-empty", EFX_REVERB_PRESET_DRIVING_EMPTYGRANDSTAND },
 { "tunnel", EFX_REVERB_PRESET_DRIVING_TUNNEL },
 { "streets", EFX_REVERB_PRESET_CITY_STREETS },
 { "subway", EFX_REVERB_PRESET_CITY_SUBWAY },
 { "museum", EFX_REVERB_PRESET_CITY_MUSEUM },
 { "library", EFX_REVERB_PRESET_CITY_LIBRARY },
 { "underpass", EFX_REVERB_PRESET_CITY_UNDERPASS },
 { "city-abandoned", EFX_REVERB_PRESET_CITY_ABANDONED },
 { "room-dusty", EFX_REVERB_PRESET_DUSTYROOM },
 { "chapel", EFX_REVERB_PRESET_CHAPEL },
 { "room-small-water", EFX_REVERB_PRESET_SMALLWATERROOM },
};

#define LEVEL_60DB		0.001f
#define MAX_DELAY_DEPTH		0.07f
#define MAX_REVERB_EFFECTS_TIME	1.0f

#define _MAX(a,b)       (((a)>(b)) ? (a) : (b))
#define _MIN(a,b)       (((a)<(b)) ? (a) : (b))
#define _MINMAX(a,b,c)  (((a)>(c)) ? (c) : (((a)<(b)) ? (b) : (a)))

float _lin2log(float v) { return log10f(v); }
float _log2lin(float v) { return powf(10.0f,v); }
float _lin2db(float v) { return 20.0f*log10f(v); }
float _db2lin(float v) { return _MINMAX(powf(10.0f,v/20.0f),0.0f,10.0f); }

static float
get_decay_depth(float time, float level) {
    if (level < LEVEL_60DB) level = LEVEL_60DB;
    return 5.0f*time*powf(LEVEL_60DB, level);
}

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

int write_reverb()
{
   for (int i=0; i<EAX_MAX_REVERB_TYPES; ++i)
   {
      char fname[256];

      EAX_effect_t *type = &EAX_reverb_types[i];

      snprintf(fname, 255, "%s.aaxs", type->name);
      printf("Generating: %s\n", fname);

      FILE *stream = fopen(fname, "w+");
      if (stream)
      {
         float gain = type->param.flGain;

         float delay_depth = MAX_DELAY_DEPTH*type->param.flDiffusion;

         float decay_time = type->param.flDecayTime;
         float decay_level = gain*_MAX(type->param.flLateReverbGain, 1.0f);
         float decay_depth = get_decay_depth(decay_time, decay_level);

         // 1.0 specifies that the reflected sound will decay by 6 dB
         // every time the distance doubles. Max. 10.0
         float val = type->param.flDensity*type->param.flDecayHFRatio;
         float fc = 500.0f+_log2lin(val*_lin2log(22000.0f-500.0f));
         if (fc >= 20000.0f) fc = 0.0f;

         float mod_time = type->param.flModulationTime;
         float mod_depth = type->param.flModulationDepth;

         fprintf(stream, "<?xml version=\"1.0\"?>\n\n");

#if 0
 fprintf(stream, "<!--\n");
 fprintf(stream, " Density: %f\n", type->param.flDensity);
 fprintf(stream, " Diffusion: %f\n", type->param.flDiffusion);

 fprintf(stream, "\n Gain: %f\n", gain);
 fprintf(stream, "Decay HF Ratio: %f\n", type->param.flDecayHFRatio);
 fprintf(stream, " Reflections Gain: %f\n", type->param.flReflectionsGain);
 fprintf(stream, " Late Reverb Gain: %f\n", type->param.flLateReverbGain);
 fprintf(stream, "\n Decay Time: %f\n", decay_time);

 fprintf(stream, "\n Modulation Time: %f\n", mod_time);
 fprintf(stream, " Modulation Depth: %f\n", mod_depth);
 fprintf(stream, "-->\n\n");
#endif
         print_header(stream);

         fprintf(stream, "<aeonwave>\n\n");

         print_info(stream);

         fprintf(stream, " <audioframe mode\"append\"");
         fprintf(stream, ">\n");
         fprintf(stream, "  <effect type=\"reverb\"");
         if (type->param.flModulationDepth > 0.0f) {
             fprintf(stream, "src=\"sine\"");
         }
         fprintf(stream, ">\n");
         fprintf(stream, "   <slot n=\"0\">\n");
         fprintf(stream, "    <param n=\"0\">%.1f</param>\n", fc);
         fprintf(stream, "    <param n=\"1\">%.3f</param>\n", delay_depth);
         fprintf(stream, "    <param n=\"2\">%.3f</param>\n", decay_level);
         fprintf(stream, "    <param n=\"3\">%.3f</param>\n", decay_depth);
         fprintf(stream, "   </slot>\n");
         if (type->param.flModulationDepth > 0.0f)
         {
             fprintf(stream, "   <slot n=\"2\">\n");
             fprintf(stream, "    <param n=\"0\">0.0</param>\n");
             fprintf(stream, "    <param n=\"1\">%.3f</param>\n", mod_time);
             fprintf(stream, "    <param n=\"2\">%.3f</param>\n", mod_depth);
             fprintf(stream, "    <param n=\"3\">0.0</param>\n");
             fprintf(stream, "   </slot>\n");
         }
         fprintf(stream, "  </effect>\n");
         fprintf(stream, " </audioframe>\n\n");

         fprintf(stream, " <mixer mode\"append\">\n");
         fprintf(stream, "  <effect type=\"reverb\"");
         if (type->param.flModulationDepth > 0.0f) {
             fprintf(stream, "src=\"sine\"");
         }
         fprintf(stream, ">\n");
         fprintf(stream, "   <slot n=\"0\">\n");
         fprintf(stream, "    <param n=\"0\">%.1f</param>\n", fc);
         fprintf(stream, "    <param n=\"1\">%.3f</param>\n", delay_depth);
         fprintf(stream, "    <param n=\"2\">%.3f</param>\n", decay_level);
         fprintf(stream, "    <param n=\"3\">%.3f</param>\n", decay_depth);
         fprintf(stream, "   </slot>\n");
         if (type->param.flModulationDepth > 0.0f)
         {
             fprintf(stream, "   <slot n=\"2\">\n");
             fprintf(stream, "    <param n=\"0\">0.0</param>\n");
             fprintf(stream, "    <param n=\"1\">%.3f</param>\n", mod_time);
             fprintf(stream, "    <param n=\"2\">%.3f</param>\n", mod_depth);
             fprintf(stream, "    <param n=\"3\">0.0</param>\n");
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

int main()
{
   write_reverb();
   return 0;
}
