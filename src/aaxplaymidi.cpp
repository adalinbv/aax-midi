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

#include <chrono>
#include <thread>

#include <stdio.h>
#include <assert.h>
#ifdef HAVE_RMALLOC_H
# include <rmalloc.h>
#else
# include <string.h>
# if HAVE_STRINGS_H
#  include <strings.h>
# endif
#endif
#include <locale.h>

#include <aax/aeonwave.hpp>
#include <aax/instrument.hpp>

#include <midi/driver.hpp>
#include <midi/file.hpp>

#include "driver.h"

void
help()
{
    aaxConfig cfgi, cfgo;

    printf("aaxplaymidi version %i.%i.%i\n\n", AAX_MIDI_MAJOR_VERSION,
                                               AAX_MIDI_MINOR_VERSION,
                                               AAX_MIDI_MICRO_VERSION);
    printf("Usage: aaxplaymidi [options]\n");
    printf("Plays a MIDI file to an audio output device.\n");

    printf("\nOptions:\n");
    printf("  -i, --input <file>\t\tplay back audio from a file\n");
    printf("  -d, --device <device>\t\tplayback device (default if not specified)\n");
    printf("  -s, --select <name|track>\tonly play the track with this name or number\n");
    printf("  -t, --time <offs>\t\ttime offset in seconds or (hh:)mm:ss\n");
    printf("  -l, --load <instr>\t\tmidi instrument configuration overlay file\n");
    printf("  -m, --mono\t\t\tplay back in mono mode\n");
//  printf("  -b, --batched\t\t\tprocess the file in batched (high-speed) mode.\n");
    printf("  -v, --verbose <0-4>\t\tshow extra playback information\n");
    printf("  -h, --help\t\t\tprint this message and exit\n");

    printf("\nVerbosity levels:\n");
    printf("1: General playback information and lyrics.\n");
    printf("2: Loading instrument messages.\n");
    printf("3: MIDI Text messages.\n");
    printf("4: Instrument not found, trying a different bank messages.\n");

    printf("\nUse aaxplay for playing other audio file formats.\n");

    printf("\n");

    exit(-1);
}

static void sleep_for(float dt)
{
    if (dt > 1e-6f)
    {
        int64_t sleep_us = dt/1e-6f;
        std::this_thread::sleep_for(std::chrono::microseconds(sleep_us));
    }
    else // sub-microsecond
    {
        std::chrono::high_resolution_clock::time_point start;
        std::chrono::nanoseconds min_duration;

        min_duration = std::chrono::nanoseconds::zero();
        start = std::chrono::high_resolution_clock::now();
        while (std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - start).count() < dt) {
            std::this_thread::sleep_for(min_duration);
        }
    }
}

void play(char *devname, enum aaxRenderMode mode, char *infile, char *outfile,
          const char *track, const char *config, float time_offs,
          const char *grep, bool mono, char verbose, bool batched, bool fm,
          bool csv)
{
    if (grep) devname = (char*)"None"; // fastest for searching

    if (!infile) {
        std::cerr << "Error while processing the MIDI file: No input file was declared." << std::endl;
        return;
    }

    try {
        aax::MIDIFile midi(devname, infile, track, mode, config);
        aax::Sensor file;
        uint64_t time_parts = 0;
        uint32_t wait_parts;
        char obuf[256];

        if (outfile)
        {
            snprintf(obuf, 256, "AeonWave on Audio Files: %s", outfile);

            file = aax::Sensor(obuf, AAX_MODE_WRITE_STEREO);
            midi.add(file);
            file.set(AAX_INITIALIZED);
            file.set(AAX_PLAYING);
        }

        midi.set_csv(csv);
        midi.set_mono(mono);
        midi.set_verbose(verbose);
        if (fm)
        {
            char *env = getenv("AAX_SHARED_DATA_DIR");
            midi.set(AAX_CAPABILITIES, AAX_RENDER_SYNTHESIZER);
            if (env) midi.set(AAX_SHARED_DATA_DIR, env);
        }
        midi.initialize(grep);
        if (!grep)
        {
            midi.start();

            if (batched) {
                midi.sensor(AAX_CAPTURING);
            }

            wait_parts = 1000;
            set_mode(1);

            int key, paused = AAX_FALSE;
            auto now = std::chrono::high_resolution_clock::now();
            do
            {
                if (!paused)
                {
                    if (!midi.process(time_parts, wait_parts)) break;

                    if (wait_parts > 0 && midi.get_pos_sec() >= time_offs)
                    {
                        double sleep_us, wait_us;

                        auto next = std::chrono::high_resolution_clock::now();
                        std::chrono::duration<double, std::micro> dt_us = next - now;

                        wait_us = wait_parts*midi.get_uspp();
                        sleep_us = wait_us - dt_us.count();

                        if (wait_us > 1e6)
                        {
                            if (wait_us > 15e6) break;
//                          sleep_us = 1.0;
                        }

                        if (sleep_us > 0)
                        {
                            if (batched)
                            {
                                midi.sensor(AAX_UPDATE);
                                midi.wait(sleep_us*1e-6f);
                                midi.get_buffer();
                            }
                            else {
                                sleep_for(sleep_us*1e-6f);
                            }
                        }

                        now = std::chrono::high_resolution_clock::now();
                    }
                    time_parts += wait_parts;
                }
                else {
                    sleep_for(0.1f);
                }

                key = get_key();
                if (key)
                {
                    if (key == ' ')
                    {
                        if (paused)
                        {
                            midi.set(AAX_PLAYING);
                            printf("\nRestart playback.\n");
                            paused = AAX_FALSE;
                        }
                        else
                        {
                            midi.set(AAX_SUSPENDED);
                            printf("\nPause playback.\n");
                            paused = AAX_TRUE;
                        }
                    }
                    else {
                        break;
                    }
                }
            }
            while(true);
            set_mode(0);
            midi.stop();
        }
    } catch (const std::exception& e) {
        if (!csv) {
            std::cerr << "Error while processing the MIDI file: "
                      << e.what() << std::endl;
        }
    }
}

int verbose = 0;
int main(int argc, char **argv)
{
    // setting the locale messes up playback
//  std::setlocale(LC_ALL, "");
//  std::locale::global(std::locale(""));
//  std::cout.imbue(std::locale());

    if (argc == 1 || getCommandLineOption(argc, argv, "-h") ||
                     getCommandLineOption(argc, argv, "--help"))
    {
        help();
    }

    enum aaxRenderMode render_mode = aaxRenderMode(getMode(argc, argv));
    char *devname = getDeviceName(argc, argv);
    char *infile = getInputFileExt(argc, argv, ".mid", NULL);
    bool batched = false;
    char mono = false;
    bool csv = false;
    char verbose = 0;
    bool fm = false;
    char *arg;

    try
    {
        float time_offs = getTime(argc, argv);
        char *outfile = getOutputFile(argc, argv, NULL);
        const char *track, *grep, *config;

        track = getCommandLineOption(argc, argv, "-s");
        if (!track) {
            track = getCommandLineOption(argc, argv, "--select");
        }

        config = getCommandLineOption(argc, argv, "-l");
        if (!config) {
           config = getCommandLineOption(argc, argv, "--load");
        }

        grep = getCommandLineOption(argc, argv, "-g");
        if (!grep) {
            grep = getCommandLineOption(argc, argv, "--grep");
        }

        arg = getCommandLineOption(argc, argv, "-v");
        if (!arg) arg = getCommandLineOption(argc, argv, "--verbose");
        if (arg) verbose = atoi(arg);
        if (arg && !verbose) verbose = 1;

        if (getCommandLineOption(argc, argv, "-b") ||
            getCommandLineOption(argc, argv, "--batched"))
        {
            batched = true;
        }
        if (getCommandLineOption(argc, argv, "-m") ||
            getCommandLineOption(argc, argv, "--mono"))
        {
            mono = true;
        }

        if (getCommandLineOption(argc, argv, "--fm")) {
            fm = true;
        }

        if (getCommandLineOption(argc, argv, "--csv")) {
            csv = true;
        }

        std::thread midiThread(play, devname, render_mode, infile, outfile, track, config, time_offs, grep, mono, verbose, batched, fm, csv);
        midiThread.join();

    } catch (const std::exception& e) {
        if (!csv) {
            std::cerr << "Error while processing the MIDI file: "
                      << e.what() << std::endl;
        }
    }

    return 0;
}

