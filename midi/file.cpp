/*
 * Copyright (C) 2018-2023 by Erik Hofman.
 * Copyright (C) 2018-2023 by Adalin B.V.
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

#include <cstring>
#include <cassert>

#include <fstream>

#include <aax/strings>

#include <midi/stream.hpp>
#include <midi/file.hpp>

using namespace aax;

MIDIFile::MIDIFile(const char *devname, const char *filename,
                   const char *selection, enum aaxRenderMode mode,
                   const char *config)
    : MIDIDriver(devname, selection, mode), file(filename)
{
    if (!midi.exists(filename) || midi.is_directory(filename)) {
        throw(std::invalid_argument("File not found: "+std::string(filename)));
        return;
    }

    std::ifstream file(filename, std::ios::in|std::ios::binary|std::ios::ate);
    ssize_t size = file.tellg();
    file.seekg(0, std::ios::beg);

    if (config)
    {
        static const char *prefix = "gmmidi-";
        static const char *ext = ".xml";
        gmmidi = config;

        if (gmmidi.compare(0, strlen(prefix), prefix)) {
            gmmidi.insert(0, prefix);
        }
        if (gmmidi.compare(gmmidi.length()-strlen(ext), strlen(ext), ext)) {
            gmmidi.append(ext);
        }
    }

    if (size > 0)
    {
        midi_data.reserve(size);
        if (midi_data.capacity() == size)
        {
            std::streamsize fileSize = size;
            if (file.read((char*)midi_data.data(), fileSize))
            {
                buffer_map<uint8_t> map(midi_data.data(), size);
                byte_stream stream(map);

                try
                {
                    uint32_t size, header = stream.pull_long();
                    uint16_t format, track_no = 0;
                    uint16_t PPQN = 0;

                    if (header == 0x4d546864) // "MThd"
                    {
                        size = stream.pull_long();
                        if (size != 6)
                        {
                            throw(std::runtime_error("Premature end of file."));
                            return;
                        }

                        format = stream.pull_word();
                        if (format > 3)
                        {
                            throw(std::runtime_error("MIDI file format not supported"));
                            return;
                        }

                        no_tracks = stream.pull_word();
                        if (format == 0 && no_tracks != 1)
                        {
                            throw(std::runtime_error("MIDI format 0 requested with more than one track"));
                            return;
                        }

                        midi.set_format(format);

                        PPQN = stream.pull_word();
                        if (PPQN & 0x8000) // SMPTE
                        {
                            uint8_t fps = (PPQN >> 8) & 0xff;
                            uint8_t resolution = PPQN & 0xff;
                            if (fps == 232) fps = 24;
                            else if (fps == 231) fps = 25;
                            else if (fps == 227) fps = 29;
                            else if (fps == 226) fps = 30;
                            else fps = 0;
                            PPQN = fps*resolution;
                        }
                        midi.set_ppqn(PPQN);
                    }

                    while (stream.remaining() > sizeof(header))
                    {
                        header = stream.pull_long();
                        if (header == 0x4d54726b) // "MTrk"
                        {
                            uint32_t length = stream.pull_long();
                            if (length >= sizeof(uint32_t) &&
                                length <= stream.remaining())
                            {
                                streams.push_back(std::shared_ptr<MIDIStream>(
                                                   new MIDIStream(*this, stream,
                                                         length, track_no++)));
                                stream.forward(length);
                            }
                        }
                        else {
                            break;
                        }
                    }
                    no_tracks = track_no;

                    midi.set_initialize(true);
                    CSV(track_no, "0, 0, Header, 0, %d, %d\n", no_tracks, PPQN);
                    for (track_no=0; track_no<no_tracks; ++track_no) {
                        CSV(track_no, "%d, 0, Start_track\n", track_no+1);
                    }
                    midi.set_initialize(false);
                } catch (const std::overflow_error& e) {
                    throw(std::invalid_argument("Error while processing the MIDI file: "+std::string(e.what())));
                }
            }
            else {
                throw(std::invalid_argument("Error: Unable to open: "+std::string(filename)));
            }
        }
        else if (!midi_data.size()) {
            throw(std::invalid_argument("Error: Out of memory."));
        }
    }
    else {
        throw(std::invalid_argument("Error: Unable to open: "+std::string(filename)));
    }
}

void
MIDIFile::initialize(const char *grep)
{
    char *env = getenv("AAX_MIDI_MODE");
    double eps;
    clock_t t;

    if (env)
    {
        if (!strcasecmp(env, "synthesizer")) {
            midi.set_capabilities(AAX_RENDER_SYNTHESIZER);
        } else if (!strcasecmp(env, "arcade")) {
            midi.set_capabilities(AAX_RENDER_ARCADE);
        }
    }

    // Read the overlay instruments
    if (!gmmidi.empty() || gmdrums.empty()) {
       midi.read_instruments(gmmidi, gmdrums);
    }

    // Read the default instruments
    midi.set_initialize(true);
    midi.read_instruments();

    midi.set_grep(grep);
    duration_sec = 0.0f;

    uint64_t time_parts = 0;
    uint32_t wait_parts = 1000000;
    t = clock();
    try
    {
        while (process(time_parts, wait_parts))
        {
            time_parts += wait_parts;
            duration_sec += wait_parts*midi.get_uspp()*1e-6f;
        }
    }
    catch (const std::runtime_error &e) {
       throw(e);
    }
    eps = (double)(clock() - t)/ CLOCKS_PER_SEC;

    midi.set_initialize(false);

    if (!grep)
    {
        rewind();
        pos_sec = 0;

        midi.set(AAX_INITIALIZED);
        if (midi.get_effects().length())
        {
           Buffer &buffer = midi.buffer(midi.get_effects());
           Sensor::add(buffer);
        }

        if (midi.get_verbose())
        {

            MESSAGE(1, "Frequency : %li Hz\n", midi.get(AAX_FREQUENCY));
            MESSAGE(1, "Upd. rate : %li Hz\n", midi.get(AAX_REFRESH_RATE));
            MESSAGE(1, "Init time : %.1f ms\n", eps*1000.0f);

            unsigned int polyphony = midi.get(AAX_MONO_EMITTERS);
            if (polyphony == UINT_MAX) {
                MESSAGE(1, "Polyphony : unlimited\n");
            } else {
                MESSAGE(1, "Polyphony : %lu\n", midi.get(AAX_MONO_EMITTERS));
            }

            enum aaxRenderMode render_mode = aaxRenderMode(midi.render_mode());
            std::string r;
            if (cores < 4 || !simd) {
                r += " mono playback";
                midi.set_mono(true);
            } else {
                r += to_string(render_mode);
            }
            if (midi_mode) {
                r += ", ";
                r += to_string(aaxCapabilities(midi_mode));
            }
            MESSAGE(1, "Rendering : %s\n", r.c_str());
            MESSAGE(1, "Patch set : %s", midi.get_patch_set().c_str());
            MESSAGE(1, " instrument set version %s\n", midi.get_patch_version().c_str());
            MESSAGE(1, "Directory : %s\n", midi.info(AAX_SHARED_DATA_DIR));

            int hour, minutes, seconds;
            unsigned int format = midi.get_format();
            if (format >= MIDI_FILE_FORMAT_MAX) format = MIDI_FILE_FORMAT_MAX;
            MESSAGE(1, "Format    : %s\n", format_name[format].c_str());

            unsigned int mode = midi.get_mode();
            assert(mode < MIDI_MODE_MAX);
            MESSAGE(1, "MIDI Mode : %s\n", mode_name[mode].c_str());

            seconds = duration_sec;
            hour = seconds/(60*60);
            seconds -= hour*60*60;
            minutes = seconds/60;
            seconds -= minutes*60;
            if (hour) {
                MESSAGE(1, "Duration  : %02i:%02i:%02i hours\n", hour, minutes, seconds);
            } else {
                MESSAGE(1, "Duration  : %02i:%02i minutes\n", minutes, seconds);
            }
        }
    }
    else {
        midi.grep(file, grep);
    }
}

void
MIDIFile::rewind()
{
    midi.rewind();
    midi.set_lyrics(false);
    for (auto& it : streams) {
        it->rewind();
    }
}

bool
MIDIFile::process(uint64_t time_parts, uint32_t& next)
{
    uint32_t elapsed_parts = next;
    uint32_t wait_parts;
    bool rv = false;

    if (streams.size() == 0)
    {
        throw(std::runtime_error("No streams to process"));
        return rv;
    }

    next = UINT_MAX;
    for (size_t t=0; t<no_tracks; ++t)
    {
        wait_parts = next;

        try
        {
            std::shared_ptr<MIDIStream> s = streams[t];
            bool res = !s->eof();
            if (s->eof())
            {
                if (!midi.get_format() || s->get_track_no()) {
                   res = !midi.finished(s->get_channel_no());
                }
            }
            else {
                res = s->process(time_parts, elapsed_parts, wait_parts);
            }

            if (t || !midi.get_format()) rv |= res;

        } catch (const std::runtime_error &e) {
            throw(e);
            break;
        }

        if (next > wait_parts) {
            next = wait_parts;
        }
    }

    if (next == UINT_MAX) {
        next = 100;
    }

    if (midi.get_verbose() && (!midi.get_lyrics() || midi.elapsed_time(5.0)))
    {
        std::string text = midi.get_display_data();
        int len = text.size();
        char display[41] = "";
        if (len < 16) {
            snprintf(display, 40, "%-32s", text.c_str());
        }
        else
        {
            snprintf(display, 40, "%32s", "");
            sprintf(display, "%s %s", text.substr(0, 16).c_str(),
                                      text.substr(16, 16).c_str());
        }

        int hour, minutes, seconds;

        pos_sec += elapsed_parts*midi.get_uspp()*1e-6f;

        seconds = pos_sec;
        hour = seconds/(60*60);
        seconds -= hour*60*60;
        minutes = seconds/60;
        seconds -= minutes*60;
        if (hour) {
            MESSAGE(1, "pos: %02i:%02i:%02i hours %s\r", hour, minutes, seconds, display);
        } else {
            MESSAGE(1, "pos: %02i:%02i minutes %s\r", minutes, seconds, display);
        }
        if (!rv) MESSAGE(1, "\n\n");
        fflush(stdout);
    }

    if (!rv) CSV(0, "0, 0, End_of_file\n");

    return rv;
}
