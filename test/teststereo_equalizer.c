/*
 * Copyright (C) 2008-2018 by Erik Hofman.
 * Copyright (C) 2009-2018 by Adalin B.V.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *        this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY ADALIN B.V. ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
 * NO EVENT SHALL ADALIN B.V. OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUTOF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of Adalin B.V.
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include <aax/aax.h>

#include "base/types.h"
#include "driver.h"
#include "wavfile.h"

#define FILE_PATH		SRC_PATH"/stereo.wav"

int main(int argc, char **argv)
{
    char *devname, *infile;
    aaxConfig config = NULL;
    aaxBuffer buffer = 0;
    int res;

    devname = getDeviceName(argc, argv);
    infile = getInputFile(argc, argv, FILE_PATH);
    config = aaxDriverOpenByName(devname, AAX_MODE_WRITE_STEREO);
    testForError(config, "No default audio device available.");

    if (config)
    {
        buffer = bufferFromFile(config, infile);
        if (buffer)
        {
            char *fparam = getCommandLineOption(argc, argv, "-f");
            aaxEmitter emitter;
            aaxFilter filter;
            aaxEffect effect;
            aaxFrame frame;
            float dt = 0.0f;
            int q, state;
            float pitch;

            printf("\nPlayback stereo with equalizer enabled.\n");
            /** emitter */
            emitter = aaxEmitterCreate();
            testForError(emitter, "Unable to create a new emitter");

            /* pitch */
            pitch = getPitch(argc, argv);
            effect = aaxEffectCreate(config, AAX_PITCH_EFFECT);
            testForError(effect, "Unable to create the pitch effect");

            res = aaxEffectSetParam(effect, AAX_PITCH, AAX_LINEAR, pitch);
            testForState(res, "aaxEffectSetParam");

            res = aaxEmitterSetEffect(emitter, effect);
            testForState(res, "aaxEmitterSetPitch");
            aaxEffectDestroy(effect);

            res = aaxEmitterAddBuffer(emitter, buffer);
            testForState(res, "aaxEmitterAddBuffer");

            /** mixer */
            res = aaxMixerSetState(config, AAX_INITIALIZED);
            testForState(res, "aaxMixerInit");

            if (!fparam)
            {
                res = aaxMixerRegisterEmitter(config, emitter);
                testForState(res, "aaxMixerRegisterEmitter");
            }
            else
            {
                frame = aaxAudioFrameCreate(config);
                testForError(frame, "Unable to create a new frame");

                res = aaxMixerRegisterAudioFrame(config, frame);
                testForState(res, "aaxMixerRegisterAudioFrame");

                res = aaxAudioFrameRegisterEmitter(frame, emitter);
                testForState(res, "aaxAudioFrameRegisterEmitter");

                res = aaxAudioFrameSetState(frame, AAX_PLAYING);
                testForState(res, "aaxAudioFrameSetState");
            }

            res = aaxMixerSetState(config, AAX_PLAYING);
            testForState(res, "aaxMixerStart");

            /* equalizer */
            printf("%2.1f | %5.0f Hz | %2.1f | %5.0f Hz | %2.1f | %5.0f Hz | %2.1f\n", 1.0f, 500.0f, 0.1f, 2000.0f, 0.5f, 8000.0f, 0.1f);

            filter = aaxFilterCreate(config, AAX_EQUALIZER);
            testForError(filter, "aaxFilterCreate");

            res = aaxFilterSetSlot(filter, 0, AAX_LINEAR,
                                              500.0f, 1.0f, 0.1f, 2.0f);
            testForState(res, "aaxFilterSetSlot/0");

            res = aaxFilterSetSlot(filter, 1, AAX_LINEAR,
                                              2000.0f, 0.1f, 0.5f, 2.0f);
            testForState(res, "aaxFilterSetSlot/1");

            res = aaxFilterSetSlot(filter, 2, AAX_LINEAR,
                                              8000.0f, 0.5f, 0.1f, 2.0f);
            testForState(res, "aaxFilterSetSlot/1");

            res = aaxFilterSetState(filter, AAX_TRUE);
            testForState(res, "aaxFilterSetState");

            if (!fparam)
            {
               res = aaxMixerSetFilter(config, filter);
               testForState(res, "aaxMixerSetFilter");
            }
            else
            {
               res = aaxAudioFrameSetFilter(frame, filter);
               testForState(res, "aaxAudioFrameSetFilter");
            }

            res = aaxFilterDestroy(filter);
            testForState(res, "aaxFilterDestroy");

            /** schedule the emitter for playback */
            res = aaxEmitterSetState(emitter, AAX_PLAYING);
            testForState(res, "aaxEmitterStart");

            q = 0;
            do
            {
                msecSleep(50);
                dt += 0.05f;

                if (++q > 10)
                {
                    unsigned long offs, offs_bytes;
                    float off_s;
                    q = 0;

                    off_s = aaxEmitterGetOffsetSec(emitter);
                    offs = aaxEmitterGetOffset(emitter, AAX_SAMPLES);
                    offs_bytes = aaxEmitterGetOffset(emitter, AAX_BYTES);
                    printf("playing time: %5.2f, buffer position: %5.2f "
                           "(%li samples/ %li bytes)\n", dt, off_s,
                           offs, offs_bytes);
                }
                state = aaxEmitterGetState(emitter);
            }
            while (state == AAX_PLAYING);

            filter = aaxMixerGetFilter(config, AAX_EQUALIZER);
            aaxFilterSetState(filter, AAX_FALSE);
            aaxMixerSetFilter(config, filter);
            aaxFilterDestroy(filter);
            aaxBufferDestroy(buffer);

            if (!fparam)
            {
                res = aaxMixerDeregisterEmitter(config, emitter);
                testForState(res, "aaxMixerDeregisterEmitter");
            }
            else
            {
                res = aaxAudioFrameDeregisterEmitter(frame, emitter);
                testForState(res, "aaxMixerDeregisterEmitter");

                res = aaxAudioFrameSetState(frame, AAX_PROCESSED);
                testForState(res, "aaxAudioFrameSetState");

                res = aaxMixerDeregisterAudioFrame(config, frame);
                testForState(res, "aaxMixerRegisterAudioFrame");

                 res = aaxAudioFrameDestroy(frame);
            }

            res = aaxMixerSetState(config, AAX_STOPPED);
            res = aaxEmitterDestroy(emitter);
        }
    }

    res = aaxDriverClose(config);
    res = aaxDriverDestroy(config);

    return 0;
}
