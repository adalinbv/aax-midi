/*
 * Copyright (C) 2008-2015 by Erik Hofman.
 * Copyright (C) 2009-2015 by Adalin B.V.
 * All rights reserved.
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>	// getenv
#include <time.h>	// nanosleep
#include <errno.h>	// EINTR


#include <aax/aax.h>

#define	SAMPLE_FREQ		48000

#if 1
#define GAIN1			0.333f
#define GAIN2			1.0f
#define GAIN3			1.0f
#define GAIN4			0.5333f
#define GAIN5			0.0667f
#define GAIN6			0.6667f
#define GAIN7			0.0f
#define GAIN8			0.6667f
#else
#define GAIN1                   1.0f
#define GAIN2                   1.0f
#define GAIN3                   1.0f
#define GAIN4                   1.0f
#define GAIN5                   1.0f
#define GAIN6                   1.0f
#define GAIN7                   1.0f
#define GAIN8                   1.0f
#endif

void
testForError(void *p, char *s)
{
    if (p == NULL)
    {
        int err = aaxGetErrorNo();
        printf("\nError: %s\n", s);
        if (err) {
            printf("%s\n\n", aaxGetErrorString(err));
        }
        exit(-1);
    }
}

void
testForState(int res, const char *func)
{
    if (res != AAX_TRUE)
    {
        int err = aaxGetErrorNo();
        printf("%s:\t\t%i\n", func, res);
        printf("(%i) %s\n\n", err, aaxGetErrorString(err));
        exit(-1);
    }
}

int msecSleep(unsigned int dt_ms)
{
   static struct timespec s;
   if (dt_ms > 0)
   {
      s.tv_sec = (dt_ms/1000);
      s.tv_nsec = (dt_ms % 1000)*1000000L;
      while(nanosleep(&s,&s)==-1 && errno == EINTR)
         continue;
   }
   else
   {
      s.tv_sec = 0;
      s.tv_nsec = 500000L;
      return nanosleep(&s, 0);
   }
   return 0;
}

int main(int argc, char **argv)
{
    char *tmp, devname[128], filename[64];
    aaxConfig config;
    int res = 0;

    tmp = getenv("TEMP");
    if (!tmp) tmp = getenv("TMP");
    if (!tmp) tmp = "/tmp";

    snprintf(filename, 64, "%s/whitenoise.wav", tmp);
    snprintf(devname, 128, "AeonWave on Audio Files: %s", filename);

    config = aaxDriverOpenByName(devname, AAX_MODE_WRITE_STEREO);
    testForError(config, "No default audio device available.");

    if (config)
    {
        int state, no_samples;
        aaxEmitter emitter;
        aaxBuffer buffer;
        aaxFilter filter;
        float dt;

        no_samples = (unsigned int)(4*SAMPLE_FREQ);
        buffer = aaxBufferCreate(config, no_samples, 1, AAX_PCM16S);
        testForError(buffer, "Unable to generate buffer\n");

        res = aaxBufferSetSetup(buffer, AAX_FREQUENCY, SAMPLE_FREQ);
        testForState(res, "aaxBufferSetFrequency");

        res = aaxBufferProcessWaveform(buffer, 0.0f, AAX_WHITE_NOISE, 1.0f,
                                       AAX_OVERWRITE);
        testForState(res, "aaxBufferProcessWaveform");

        /** mixer */
        res = aaxMixerSetState(config, AAX_INITIALIZED);
        testForState(res, "aaxMixerInit");

        res = aaxMixerSetState(config, AAX_PLAYING);
        testForState(res, "aaxMixerStart");

        /** emitter */
        emitter = aaxEmitterCreate();
        testForError(emitter, "Unable to create a new emitter\n");

        res = aaxEmitterAddBuffer(emitter, buffer);
        testForState(res, "aaxEmitterAddBuffer");

        res = aaxEmitterSetMode(emitter, AAX_LOOPING, AAX_TRUE);
        testForState(res, "aaxEmitterSetMode");

        /* equalizer */
        filter = aaxFilterCreate(config, AAX_GRAPHIC_EQUALIZER);
        testForError(filter, "aaxFilterCreate");

        filter = aaxFilterSetSlot(filter, 0, AAX_LINEAR, GAIN1, GAIN2, GAIN3, GAIN4);
                                             
        testForError(filter, "aaxFilterSetSlot 0");

        filter = aaxFilterSetSlot(filter, 1, AAX_LINEAR, GAIN5, GAIN6, GAIN7, GAIN8);
        testForError(filter, "aaxFilterSetSlot 1");

        filter = aaxFilterSetState(filter, AAX_TRUE);
        testForError(filter, "aaxFilterSetState");

        res = aaxMixerSetFilter(config, filter);
        testForState(res, "aaxMixerSetFilter");

        res = aaxFilterDestroy(filter);

        res = aaxMixerRegisterEmitter(config, emitter);
        testForState(res, "aaxMixerRegisterEmitter");

        printf("writing white noise to: %s\n", filename);
        res = aaxEmitterSetState(emitter, AAX_PLAYING);
        testForState(res, "aaxEmitterStart");

        dt = 0.0f;
        do
        {
            dt += 0.05f;
            msecSleep(50);
            state = aaxEmitterGetState(emitter);
        }
        while (dt < 1.0f); // state == AAX_PLAYING);

        res = aaxEmitterSetState(emitter, AAX_PROCESSED);
        testForState(res, "aaxEmitterStop");

        res = aaxEmitterRemoveBuffer(emitter);
        testForState(res, "aaxEmitterRemoveBuffer");

        res = aaxBufferDestroy(buffer);
        testForState(res, "aaxBufferDestroy");

        res = aaxMixerDeregisterEmitter(config, emitter);
        res = aaxMixerSetState(config, AAX_STOPPED);
        res = aaxEmitterDestroy(emitter);
    }

    res = aaxDriverClose(config);
    res = aaxDriverDestroy(config);

    return res;
}
