
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <time.h>

#include <xml.h>
#include <aax/aax.h>

#include "driver.h"
#include "wavfile.h"

#if defined(WIN32)
# define TEMP_DIR		getenv("TEMP")
#else   /* !WIN32 */
# define TEMP_DIR		"/tmp"
#endif
#define LEVEL_16DB		0.15848931670f

static float freq = 22.0f;

enum type_t
{
   WAVEFORM = 0,
   FILTER,
   EFFECT,
   EMITTER,
   FRAME
};

const char* format_float(float f)
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

struct info_t
{
    uint8_t program;
    uint8_t bank;
    char* name;

    struct note_t
    {
        uint8_t polyphony;
        uint8_t min, max, step;
    } note;

     struct position_t {
        double x, y, z;
    } position;
};

void fill_info(struct info_t *info, void *xid)
{
    void *xtid;

    info->program = xmlAttributeGetInt(xid, "program");
    info->bank = xmlAttributeGetInt(xid, "bank");
    info->name = xmlAttributeGetString(xid, "name");

    xtid = xmlNodeGet(xid, "note");
    if (xtid)
    {
        info->note.polyphony = xmlAttributeGetInt(xtid, "polyphony");
        info->note.min = xmlAttributeGetInt(xtid, "min");
        info->note.max = xmlAttributeGetInt(xtid, "max");
        info->note.step = xmlAttributeGetInt(xtid, "step");
        xmlFree(xtid);
    }

    xtid = xmlNodeGet(xid, "position");
    if (xtid)
    {
        info->position.x = xmlAttributeGetDouble(xtid, "x");
        info->position.y = xmlAttributeGetDouble(xtid, "y");
        info->position.z = xmlAttributeGetDouble(xtid, "z");
        xmlFree(xtid);
    }
}

void print_info(struct info_t *info, FILE *output)
{
    fprintf(output, " <info");
    if (info->name) fprintf(output, " name=\"%s\"", info->name);
    if (info->note.polyphony)
    {
        fprintf(output, " bank=\"%i\"", info->bank);
        fprintf(output, " program=\"%i\"", info->program);
    }
    fprintf(output, ">\n");

    if (info->note.polyphony)
    {
        fprintf(output, "  <note polyphony=\"%i\"", info->note.polyphony);
        if (info->note.min) fprintf(output, " min=\"%i\"", info->note.min);
        if (info->note.max) fprintf(output, " max=\"%i\"", info->note.max);
        if (info->note.step) fprintf(output, " step=\"%i\"", info->note.step);
        fprintf(output, "/>\n");
    }

    if (info->position.x || info->position.y || info->position.z)
    {
        fprintf(output, "  <position x=\"%s\"", format_float(info->position.x));
        fprintf(output, " y=\"%s\"", format_float(info->position.y));
        fprintf(output, " z=\"%s\"/>\n", format_float(info->position.z));
    };

    fprintf(output, " </info>\n\n");
}

void free_info(struct info_t *info)
{
    aaxFree(info->name);
}

struct dsp_t
{
    enum type_t dtype;
    char *type;
    char *src;
    int stereo;
    int repeat;
    int optional;

    uint8_t no_slots;
    struct slot_t
    {
        struct param_t
        {
           float value;
           float pitch;
           float sustain;
        } param[4];
    } slot[4];
};

void fill_dsp(struct dsp_t *dsp, void *xid, enum type_t t)
{
    unsigned int s, snum;
    void *xsid;

    dsp->dtype = t;
    dsp->type = xmlAttributeGetString(xid, "type");
    dsp->src = xmlAttributeGetString(xid, "src");
    dsp->stereo = xmlAttributeGetInt(xid, "stereo");
    dsp->repeat = xmlAttributeGetInt(xid, "repeat");
    dsp->optional = xmlAttributeGetBool(xid, "optional");

    xsid = xmlMarkId(xid);
    dsp->no_slots = snum = xmlNodeGetNum(xid, "slot");
    for (s=0; s<snum; s++)
    {
        if (xmlNodeGetPos(xid, xsid, "slot", s) != 0)
        {
            unsigned int p, pnum = xmlNodeGetNum(xsid, "param");
            void *xpid = xmlMarkId(xsid);
            int sn = s;

            if (xmlAttributeExists(xsid, "n")) {
                sn = xmlAttributeGetInt(xsid, "n");
            }

            for (p=0; p<pnum; p++)
            {
                if (xmlNodeGetPos(xsid, xpid, "param", p) != 0)
                {
                    int pn = p;

                    if (xmlAttributeExists(xpid, "n")) {
                        pn = xmlAttributeGetInt(xpid, "n");
                    }

                    dsp->slot[sn].param[pn].value = xmlGetDouble(xpid);
                    dsp->slot[sn].param[pn].pitch = xmlAttributeGetDouble(xpid, "pitch");
                    dsp->slot[sn].param[pn].sustain = xmlAttributeGetDouble(xpid, "auto-sustain");
                }
            }
            xmlFree(xpid);
        }
    }
    xmlFree(xsid);
}

void print_dsp(struct dsp_t *dsp, FILE *output)
{
    unsigned int s, p;

    if (dsp->dtype == FILTER) {
        fprintf(output, "  <filter type=\"%s\"", dsp->type);
    } else {
        fprintf(output, "  <effect type=\"%s\"", dsp->type);
    }
    if (dsp->src) fprintf(output, " src=\"%s\"", dsp->src);
    if (dsp->repeat) fprintf(output, " repeat=\"%i\"", dsp->repeat);
    if (dsp->stereo) fprintf(output, " stereo=\"true\"");
    if (dsp->optional) fprintf(output, " optional=\"true\"");
    fprintf(output, ">\n");

    for(s=0; s<dsp->no_slots; ++s)
    {
        fprintf(output, "   <slot n=\"%i\">\n", s);
        for(p=0; p<4; ++p)
        {
            float sustain = dsp->slot[s].param[p].sustain;
            float pitch = dsp->slot[s].param[p].pitch;

            fprintf(output, "    <param n=\"%i\"", p);
            if (pitch)
            {
                fprintf(output, " pitch=\"%s\"", format_float(pitch));
                dsp->slot[s].param[p].value = freq*pitch;
            }
            if (sustain) {
                fprintf(output, " auto-sustain=\"%s\"", format_float(sustain));
            }

            fprintf(output, ">%s</param>\n", format_float(dsp->slot[s].param[p].value));
        }
        fprintf(output, "   </slot>\n");
    }

    if (dsp->dtype == FILTER) {
        fprintf(output, "  </filter>\n");
    } else {
        fprintf(output, "  </effect>\n");
    }
}

void free_dsp(struct dsp_t *dsp)
{
    aaxFree(dsp->type);
    aaxFree(dsp->src);
}

struct waveform_t
{
    char *src;
    char *processing;
    float ratio;
    float pitch;
    int voices;
    float spread;
};

void fill_waveform(struct waveform_t *wave, void *xid)
{
    wave->src = xmlAttributeGetString(xid, "src");
    wave->processing = xmlAttributeGetString(xid, "processing");
    wave->ratio = xmlAttributeGetDouble(xid, "ratio");
    wave->pitch = xmlAttributeGetDouble(xid, "pitch");
    wave->voices = xmlAttributeGetInt(xid, "voices");
    wave->spread = xmlAttributeGetDouble(xid, "spread");
}

void print_waveform(struct waveform_t *wave, FILE *output)
{
    fprintf(output, "  <waveform src=\"%s\"", wave->src);
    if (wave->processing) fprintf(output, " processing=\"%s\"", wave->processing);
    if (wave->ratio) fprintf(output, " ratio=\"%s\"", format_float(wave->ratio));
    if (wave->pitch && wave->pitch != 1.0f) fprintf(output, " pitch=\"%s\"", format_float(wave->pitch));
    if (wave->voices)
    {
        fprintf(output, " voices=\"%i\"", wave->voices);
        if (wave->spread) fprintf(output, " spread=\"%2.1f\"", wave->spread);
    }
    fprintf(output, "/>\n");
}

void free_waveform(struct waveform_t *wave)
{
    aaxFree(wave->src);
    aaxFree(wave->processing);
}

struct sound_t
{
    float gain;
    int frequency;
    float duration;
    int voices;
    float spread;

    uint8_t no_entries;
    struct entry_t
    {
        enum type_t type;
        union
        {
            struct waveform_t waveform;
            struct dsp_t dsp;
        } slot;
    } entry[32];
};

void fill_sound(struct sound_t *sound, void *xid, float gain)
{
    unsigned int p, e, emax;
    void *xeid;

    if (gain == 0.0f) {
        sound->gain = xmlAttributeGetDouble(xid, "gain");
    } else {
        sound->gain = gain; // xmlAttributeGetDouble(xid, "gain");
    }
    sound->frequency = xmlAttributeGetInt(xid, "frequency");
    sound->duration = xmlAttributeGetDouble(xid, "duration");
    sound->voices = xmlAttributeGetInt(xid, "voices");
    sound->spread = xmlAttributeGetDouble(xid, "spread");

    p = 0;
    xeid = xmlMarkId(xid);
    emax = xmlNodeGetNum(xid, "*");
    for (e=0; e<emax; e++)
    {
        if (xmlNodeGetPos(xid, xeid, "*", e) != 0)
        {
            char *name = xmlNodeGetName(xeid);
            if (!strcasecmp(name, "waveform"))
            {
                sound->entry[p].type = WAVEFORM;
                fill_waveform(&sound->entry[p++].slot.waveform, xeid);
            }
            else if (!strcasecmp(name, "filter"))
            {
                sound->entry[p].type = FILTER;
                fill_dsp(&sound->entry[p++].slot.dsp, xeid, FILTER);
            }
            else if (!strcasecmp(name, "effect"))
            {
                sound->entry[p].type = EFFECT;
                fill_dsp(&sound->entry[p++].slot.dsp, xeid, EFFECT);
            }

            xmlFree(name);
        }
    }
    sound->no_entries = p;
    xmlFree(xeid);
}

void print_sound(struct sound_t *sound, FILE *output)
{
    unsigned int e;

    fprintf(output, " <sound");
    if (sound->gain) fprintf(output, " gain=\"%3.2f\"", sound->gain);
    if (sound->frequency)
    {
        freq = sound->frequency;
        fprintf(output, " frequency=\"%i\"", sound->frequency);
    }
    if (sound->duration) fprintf(output, " duration=\"%s\"", format_float(sound->duration));
    if (sound->voices)
    {
        fprintf(output, " voices=\"%i\"", sound->voices);
        if (sound->spread) fprintf(output, " spread=\"%2.1f\"", sound->spread);
    }
    fprintf(output, ">\n");

    for (e=0; e<sound->no_entries; ++e)
    {
        if (sound->entry[e].type == WAVEFORM) {
            print_waveform(&sound->entry[e].slot.waveform, output);
        } else {
            print_dsp(&sound->entry[e].slot.dsp, output);
        }
    }
    fprintf(output, " </sound>\n\n");
}

void free_sound(struct sound_t *sound)
{
}

struct object_t		// emitter and audioframe
{
    char *mode;
    int looping;

    uint8_t no_dsps;
    struct dsp_t dsp[16];
};

void fill_object(struct object_t *obj, void *xid)
{
    unsigned int p, d, dnum;
    void *xdid;

    obj->mode = xmlAttributeGetString(xid, "mode");
    obj->looping = xmlAttributeGetBool(xid, "looping");

    p = 0;
    xdid = xmlMarkId(xid);
    dnum = xmlNodeGetNum(xdid, "filter");
    for (d=0; d<dnum; d++)
    {
        if (xmlNodeGetPos(xid, xdid, "filter", d) != 0) {
            fill_dsp(&obj->dsp[p++], xdid, FILTER);
        }
    }
    xmlFree(xdid);

    xdid = xmlMarkId(xid);
    dnum = xmlNodeGetNum(xdid, "effect");
    for (d=0; d<dnum; d++)
    {
        if (xmlNodeGetPos(xid, xdid, "effect", d) != 0) {
            fill_dsp(&obj->dsp[p++], xdid, EFFECT);
        }
    }
    xmlFree(xdid);
    obj->no_dsps = p;
}

void print_object(struct object_t *obj, enum type_t type, FILE *output)
{
    unsigned int d;

    if (type == FRAME)
    {
        if (!obj->no_dsps) return;
        fprintf(output, " <audioframe");
    }
    else {
        fprintf(output, " <emitter");
    }

    if (obj->mode) fprintf(output, " mode=\"%s\"", obj->mode);
    if (obj->looping) fprintf(output, " looping=\"true\"");

    if (obj->no_dsps)
    {
        fprintf(output, ">\n");

        for (d=0; d<obj->no_dsps; ++d) {
            print_dsp(&obj->dsp[d], output);
        }

        if (type == EMITTER) {
            fprintf(output, " </emitter>\n\n");
        } else {
            fprintf(output, " </audioframe>\n");
        }
    }
    else {
        fprintf(output, "/>\n\n");
    }
}

void free_object(struct object_t *obj)
{
    xmlFree(obj->mode);
}

struct aax_t
{
    struct info_t info;
    struct sound_t sound;
    struct object_t emitter;
    struct object_t audioframe;
};

void fill_aax(struct aax_t *aax, const char *filename, float gain)
{
    void *xid;

    memset(aax, 0, sizeof(struct aax_t));
    xid = xmlOpen(filename);
    if (xid)
    {
        void *xaid = xmlNodeGet(xid, "/aeonwave");
        if (xaid)
        {
            void *xtid = xmlNodeGet(xaid, "instrument");
            if (!xtid) xtid = xmlNodeGet(xaid, "info");
            if (xtid)
            {
                fill_info(&aax->info, xtid);
                xmlFree(xtid);
            }

            xtid = xmlNodeGet(xaid, "sound");
            if (xtid)
            {
                fill_sound(&aax->sound, xtid, gain);
                xmlFree(xtid);
            }

            xtid = xmlNodeGet(xaid, "emitter");
            if (xtid)
            {
                fill_object(&aax->emitter, xtid);
                xmlFree(xtid);
            }

            xtid = xmlNodeGet(xaid, "audioframe");
            if (xtid)
            {
                fill_object(&aax->audioframe, xtid);
                xmlFree(xtid);
            }

            xmlFree(xaid);
        }
        else {
            printf("%s does not seem to be AAXS compatible.\n", filename);
        }
        xmlClose(xid);
    }
    else {
        printf("%s not found.\n", filename);
    }
}

void print_aax(struct aax_t *aax, const char *outfile)
{
    FILE *output;
    struct tm* tm_info;
    time_t timer;
    char year[5];

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
    fprintf(output, "-->\n\n");

    fprintf(output, "<aeonwave>\n\n");
    print_info(&aax->info, output);
    print_sound(&aax->sound, output);
    print_object(&aax->emitter, EMITTER, output);
    print_object(&aax->audioframe, FRAME, output);
    fprintf(output, "</aeonwave>\n");

    if (outfile) {
        fclose(output);
    }
}

void free_aax(struct aax_t *aax)
{
    free_info(&aax->info);
    free_sound(&aax->sound);
    free_object(&aax->emitter);
    free_object(&aax->audioframe);
}

void help()
{
    printf("aaxsstandardize version %i.%i.%i\n\n", AAX_UTILS_MAJOR_VERSION,
                                                   AAX_UTILS_MINOR_VERSION,
                                                  AAX_UTILS_MICRO_VERSION);
    printf("Usage: aaxsstandardize [options]\n");
    printf("Reads a user generated .aaxs configuration file and outputs a\n");
    printf("standardized version of the file.\n");

    printf("\nOptions:\n");
    printf("  -i, --input <file>\t\tthe .aaxs configuration file to standardize.\n");
    printf("  -o, --output <file>\t\twrite the new .aaxs configuration to this file.\n");
    printf("  -h, --help\t\t\tprint this message and exit\n");

    printf("\nWhen no output file is specified then stdout will be used.\n");

    printf("\n");
    exit(-1);
}

int main(int argc, char **argv)
{
    char tmpfile[128], aaxsfile[128];
    char *infile, *outfile;

    if (argc == 1 || getCommandLineOption(argc, argv, "-h") ||
                     getCommandLineOption(argc, argv, "--help"))
    {
        help();
    }

    snprintf(aaxsfile, 120, "%s/aaxsstandardize.aaxs", TEMP_DIR);
    snprintf(tmpfile, 120, "AeonWave on Audio Files: %s/aaxsstandardize.wav", TEMP_DIR);

    infile = getInputFile(argc, argv, NULL);
    outfile = getOutputFile(argc, argv, NULL);
    if (infile)
    {
        float gain, rms1, rms2, dt, step;
        struct aax_t aax;
        aaxBuffer buffer;
        aaxConfig config;
        aaxEmitter emitter;
        aaxFilter filter;
        aaxFrame frame;
        int res;

        /* mixer */
        config = aaxDriverOpenByName(tmpfile, AAX_MODE_WRITE_STEREO);
        testForError(config, "unable to open the temporary file.");

        res = aaxMixerSetSetup(config, AAX_FORMAT, AAX_FLOAT);
        testForState(res, "aaxMixerSetSetup, format");

        res = aaxMixerSetSetup(config, AAX_TRACKS, 1);
        testForState(res, "aaxMixerSetSetup, no_tracks");

        res = aaxMixerSetSetup(config, AAX_REFRESH_RATE, 90);
        testForState(res, "aaxMixerSetSetup, refresh rate");

        res = aaxMixerSetState(config, AAX_INITIALIZED);
        testForState(res, "aaxMixerInit");

        fill_aax(&aax, infile, 1.0f);
        print_aax(&aax, aaxsfile);
        free_aax(&aax);

        /* buffer */
        buffer = aaxBufferReadFromStream(config, aaxsfile);
        testForError(buffer, "Unable to create a buffer from an aaxs file.");

        rms1 = (float)aaxBufferGetSetup(buffer, AAX_AVERAGE_VALUE);
        rms1 = 83886.08f/rms1;

        /* emitter */
        emitter = aaxEmitterCreate();
        testForError(emitter, "Unable to create a new emitter");

        res = aaxEmitterAddBuffer(emitter, buffer);
        testForState(res, "aaxEmitterAddBuffer");

        res = aaxEmitterSetState(emitter, AAX_PLAYING);
        testForState(res, "aaxEmitterStart");

#if 0
        /* gain */
        filter = aaxFilterCreate(config, AAX_VOLUME_FILTER);
        testForError(filter, "Unable to create the volume filter");

        res = aaxFilterSetParam(filter, AAX_GAIN, AAX_LINEAR, 1.0f/gain);
        testForState(res, "aaxFilterSetParam");

        res = aaxEmitterSetFilter(emitter, filter);
        testForState(res, "aaxEmitterSetGain");
        aaxFilterDestroy(filter);
#endif

        /* frame */
        frame = aaxAudioFrameCreate(config);
        testForError(frame, "Unable to create a new audio frame");

        res = aaxMixerRegisterAudioFrame(config, frame);
        testForState(res, "aaxMixerRegisterAudioFrame");

        res = aaxAudioFrameRegisterEmitter(frame, emitter);
        testForState(res, "aaxAudioFrameRegisterEmitter");

        res = aaxAudioFrameSetState(frame, AAX_PLAYING);
        testForState(res, "aaxAudioFrameStart");

        res = aaxAudioFrameAddBuffer(frame, buffer);

        /* playback */
        res = aaxMixerSetState(config, AAX_PLAYING);
        testForState(res, "aaxMixerStart");

        dt = 0.0f;
        step = 1.0f/aaxMixerGetSetup(config, AAX_REFRESH_RATE);
        do
        {
            aaxMixerSetState(config, AAX_UPDATE);
            dt += step;
        }
        while (dt < 0.25f && aaxEmitterGetState(emitter) == AAX_PLAYING);

        res = aaxEmitterSetState(emitter, AAX_SUSPENDED);
        testForState(res, "aaxEmitterStop");

        res = aaxAudioFrameSetState(frame, AAX_STOPPED);
        res = aaxAudioFrameDeregisterEmitter(frame, emitter);
        res = aaxMixerDeregisterAudioFrame(config, frame);
        res = aaxMixerSetState(config, AAX_STOPPED);
        res = aaxAudioFrameDestroy(frame);
        res = aaxEmitterDestroy(emitter);
        res = aaxBufferDestroy(buffer);

        res = aaxDriverClose(config);
        res = aaxDriverDestroy(config);


        config = aaxDriverOpenByName("None", AAX_MODE_WRITE_STEREO);
        testForError(config, "No default audio device available.");

        snprintf(tmpfile, 120, "%s/aaxsstandardize.wav", TEMP_DIR);
        buffer = aaxBufferReadFromStream(config, tmpfile);
        testForError(buffer, "Unable to read the buffer.");

//      peak = (float)aaxBufferGetSetup(buffer, AAX_PEAK_VALUE);
        rms2 = (float)aaxBufferGetSetup(buffer, AAX_AVERAGE_VALUE);
        rms2 = 83886.08f/rms2;
        aaxBufferDestroy(buffer);

        aaxDriverClose(config);
        aaxDriverDestroy(config);

        printf("%s: %5.4f, %5.4f", infile, rms1, rms2);
        rms1 = (0.1f*rms1 + 0.9f*rms2)/LEVEL_16DB;
        printf(", new gain: %f\n", rms1);
        fill_aax(&aax, infile, rms1);
        print_aax(&aax, outfile);
        free_aax(&aax);
    }
    else {
        help();
    }

    return 0;
}