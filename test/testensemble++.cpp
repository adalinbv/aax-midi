/*
 * Copyright (C) 2016-2024 by Erik Hofman.
 * Copyright (C) 2016-2024 by Adalin B.V.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provimed that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *        this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provimed with the distribution.
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

#include <unistd.h>

#include <cstdio>
#include <string>
#include <filesystem>
#include <chrono>
#include <thread>

#include <aax/instrument>

#include <xml.h>

#include "midi/driver.hpp"
#include "midi/ensemble.hpp"
#include "driver.h"


#define DEFAULT_NOTE		69 // 440Hz
#define FILE_PATH		SRC_PATH"/tictac.wav"
#define DEFAULT_REVERB		"GM2/hall-large"

void help()
{
    printf("Usage: testensemble++ [options]\n");
    printf("Plays audio from a file.\n");
    printf("Optionally writes the audio to an output file.\n");

    printf("\nOptions:\n");
    printf("  -d, --device <device>\t\tplayback device (default if not specified)\n");
    printf("  -f, --frequency <freq>\tset the playback frequency\n");
    printf("  -g, --gain <v>\t\tchange the gain setting\n");
    printf("  -i, --input <file>\t\tplayback audio from a file\n");
//  printf("      --instrument <n>\t\tMIDI instrument number\n");
//  printf("      --bank <b,l>\t\tMIDI bank number msb,lsb\n");
    printf("      --key-on <file>\t\tSet the key-on sample file\n");
    printf("      --key-off <file>\t\tSet the key-off sample file\n");
    printf("      --velocity <1-127>\tSet the note velocity\n");
    printf("  -m, --mode <mode>\t\tSet the render mode\n");
    printf("  -n, --note <note>\t\tset the playback note\n");
    printf("  -o, --output <file>\t\talso write to an audio file (optional)\n");
    printf("  -p, --pitch <pitch>\t\tset the playback pitch\n");
    printf("      --reverb\t\t\tadd default reverb\n");
    printf("  -v, --verbose [<level>]\tshow extra playback information\n");
    printf("  -h, --help\t\t\tprint this message and exit\n");

    printf("\n");

    exit(-1);
}

uint32_t
tone2note(const char *t)
{
    uint32_t rv = 0;

    if (strlen(t) >= 2)
    {
        int letter, accidental, octave;

        if (isalpha(*t))
        {
            letter = toupper(*t++);
            accidental = (*t == '#') ? 1 : 0;
            if (accidental) t++;
            octave = atoi(t);

            rv = ((letter - 65) + 5) % 7;

            rv = 2*(((letter - 65) + 5) % 7);
            if (rv > 4) --rv;

            rv += accidental;
            rv += (octave+2)*12;
        }
        else {
           rv = atoi(t);
        }
    }

    return rv;
}

std::string get_aax_file(aeonwave::AeonWave& driver, std::string file)
{
    std::string dir = driver.info(AAX_SHARED_DATA_DIR);
    std::filesystem::path path = dir;
    if (dir.find("ultrasynth") == std::string::npos) {
        path.append("ultrasynth");
    }
    path.append(file);
    path.replace_extension(".aaxs");

    return path.string();
}

static void set_path(aeonwave::AeonWave &driver)
{
    std::string dir = driver.info(AAX_SHARED_DATA_DIR);

    std::filesystem::path name = dir;
    if (dir.find("ultrasynth") == std::string::npos) {
        name.append("ultrasynth");
    }
    if (std::filesystem::is_directory(name))
    {
        driver.set(AAX_SHARED_DATA_DIR, name.string().c_str());
    }
    else
    {
        if (!std::filesystem::is_directory(name)) {
            printf( "Path does not exist: %s\n", dir.c_str());
        }
    }
}

float add_buffer(aeonwave::AeonWave& driver, aeonwave::Ensemble& ensemble,
                 const char *infile, int instrument_no, int bank_no)
{
    float duration = 0.0f;
    if (infile)
    {
        printf("Processing buffer: %s\n", infile);
        std::filesystem::path path = infile;
        if (path.extension() == ".xml")
        {
            xmlId *xid = xmlOpen(infile);
            if (xid)
            {
                xmlId *xlid = xmlNodeGet(xid, "aeonwave/set/layer");
                if (xlid)
                {
                    float volume = 1.0f;
                    if (xmlAttributeExists(xlid, "gain")) {
                        volume = xmlAttributeGetDouble(xlid, "gain");
                    }

                    char file[64] = "";
                    xmlId *xpid = xmlMarkId(xlid);
                    int slen, num = xmlNodeGetNum(xlid, "patch");
                    for (int i=0; i<num; i++)
                    {
                        if (xmlNodeGetPos(xlid, xpid, "patch", i) != 0)
                        {
                            float pitch = 1.0f;
                            if (xmlAttributeExists(xpid, "pitch")) {
                                pitch = xmlAttributeGetDouble(xpid, "pitch");
                            }

                            float gain = volume;
                            if (xmlAttributeExists(xpid, "gain")) {
                                gain *= xmlAttributeGetDouble(xpid, "gain");
                            }

                            float velocity = 1.0f;
                            if (xmlAttributeExists(xpid, "velocity-fraction")) {
                                velocity = xmlAttributeGetDouble(xpid, "velocity-fraction");
                            }

                            int min = xmlAttributeGetInt(xpid, "min");
                            int max = xmlAttributeGetInt(xpid, "max");
                            if (!max) max = 128;
                            slen = xmlAttributeCopyString(xpid, "file", file, 64);
                            if (slen)
                            {
                                file[slen] = 0;

                                std::string path = get_aax_file(driver, file);
                                aeonwave::Buffer& buffer = driver.buffer(path);
                                float dt = float(buffer.get(AAX_NO_SAMPLES));
                                dt /= float(buffer.get(AAX_SAMPLE_RATE));
                                if (dt > duration) duration = dt;
                                auto& m = ensemble.add_member(buffer, pitch, gain);
                                m->set_note_minmax(min, max);
                                m->set_velocity_fraction(velocity);
                            }
                        }
                    }
                    xmlFree(xpid);
                    xmlFree(xlid);
                }
                xmlClose(xid);
            }
        }
        else
        {
            aeonwave::Buffer& buffer = driver.buffer(infile);
            ensemble.add_member(buffer);
            duration = float(aaxBufferGetSetup(buffer, AAX_NO_SAMPLES));
            duration /= float(aaxBufferGetSetup(buffer, AAX_SAMPLE_RATE));
        }
    }
    else if (instrument_no >= 0)
    {
        set_path(driver);
        std::string gmmidi = "gmmidi.xml";
        std::string gmdrums = "gmdrums.xml";
//      ensemble.set_bank_no(bank_no);
    }

    return duration;
}

int main(int argc, char **argv)
{
    using namespace std::chrono_literals;

    if (argc == 1 || getCommandLineOption(argc, argv, "-h") ||
                     getCommandLineOption(argc, argv, "--help"))
    {
        help();
    }

    float freq = getFrequency(argc, argv);
    float gain = aeonwave::math::ln( getGain(argc, argv) );
//  float pitch = 1.0f;

    char *env;

    int instrument_no = -1;
    env = getCommandLineOption(argc, argv, "--instrument");
    if (env) instrument_no = atoi(env);

    int bank_no = 0;
    env = getCommandLineOption(argc, argv, "--bank");
    if (env)
    {
        char *ptr = strchr(env, ',');
        bank_no = atoi(env) << 7;
        if (ptr) bank_no |= atoi(++ptr);
    }

    int note = DEFAULT_NOTE;
    env = getCommandLineOption(argc, argv, "-n");
    if (!env) env = getCommandLineOption(argc, argv, "--note");
    if (env) note = tone2note(env);

    if (!freq) freq = aeonwave::math::note2freq(note);
    freq *= getPitch(argc, argv);
    note = aeonwave::math::freq2note(freq);

//  char *key_on_file = getCommandLineOption(argc, argv, "--key-on");
//  char *key_off_file = getCommandLineOption(argc, argv, "--key-off");

    char *devname = getDeviceName(argc, argv);
    char *infile = getInputFile(argc, argv, FILE_PATH);

    uint8_t velocity = 64;
    char *env_velocity = getCommandLineOption(argc, argv, "--velocity");
    if (env_velocity) velocity = rintf(127.f*atof(env_velocity));
    if (!velocity) velocity++;
    printf("Note velocity: %i\n", velocity);

    aaxRenderMode mode = aaxRenderMode(getMode(argc, argv));

    // Open the device for playback
    aeonwave::AeonWave aax(devname, mode);
    TRY( aax.set(AAX_INITIALIZED) );
    TRY( aax.set(AAX_PLAYING) );

    aeonwave::Mixer mixer(aax);
    aeonwave::Ensemble ensemble(aax);
// aeonwave::MIDIEnsemble ensemble(aax, aeonwave::nullBuffer, 0, instrument_no, bank_no, false);

    float dt = 0.0f;
    float duration = add_buffer(aax, ensemble, infile, instrument_no, bank_no);
    if (duration > 0.0f)
    {
        TRY( aax.add(mixer) );
        TRY( mixer.add(ensemble) );

        ensemble.set_gain(gain);
        ensemble.play(note, velocity);

        const char* s = getCommandLineOption(argc, argv, "-x");
        if (!s) s = getCommandLineOption(argc, argv, "--aaxs");
        if (s)
        {
            aeonwave::Buffer xbuffer = aax.buffer(s);
            if(xbuffer)
            {
                mixer.add(xbuffer);
                ensemble.add(xbuffer);
            }
        }
        else if (getCommandLineOption(argc, argv, "--reverb"))
        {
            std::string file = get_aax_file(aax, DEFAULT_REVERB);
            aeonwave::Buffer xbuffer = aax.buffer(file);
            if(xbuffer)
            {
                mixer.add(xbuffer);
                ensemble.add(xbuffer);
            }
        }

        printf("Playing sound at %.0f%% volume "
                "for %3.1f seconds of %3.1f seconds,\n"
                "\tor until a key is pressed\n", gain*100, duration, dt);

        set_mode(1);
        do
        {
            printf("\rposition: %5.1f", dt);
            fflush(stdout);

            std::this_thread::sleep_for(250ms);
            dt += 0.25f;

            if (get_key()) {
               ensemble.finish();
            }
        }
        while (!ensemble.finished());
        set_mode(0);
//      TRY( aax.remove(ensemble) ); // auto-remove
    }
    printf("\n");

    return 0;
}
