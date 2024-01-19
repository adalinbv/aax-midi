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

#pragma once

#include <map>

#include <aax/byte_stream.hpp>

#include <midi/shared.hpp>
#include <midi/driver.hpp>

namespace aeonwave
{

class MIDIStream;

class MIDIFile : public MIDIDriver
{
public:
    MIDIFile(const char *filename) : MIDIFile(nullptr, filename) {}

    MIDIFile(const char *devname, const char *filename, const char *track=nullptr, enum aaxRenderMode mode=AAX_MODE_WRITE_STEREO, const char *config=nullptr);

    explicit MIDIFile(std::string& devname, std::string& filename)
       :  MIDIFile(devname.c_str(), filename.c_str()) {}

    virtual ~MIDIFile() = default;

    inline operator bool() {
        return midi_data.capacity();
    }

    void initialize(const char *grep = nullptr);
    inline void start() { midi.start(); }
    inline void stop() { midi.stop(); }
    void rewind();

    inline void set_volume(float g = 1.0f) { midi.set_volume(g); }
    inline float get_volume() { return midi.get_volume(); }

    inline float get_duration_sec() { return duration_sec; }
    inline float get_pos_sec() { return pos_sec; }

    bool process(uint64_t, uint32_t&);

private:
    std::string file;
    std::string gmmidi;
    std::string gmdrums;
    std::vector<uint8_t> midi_data;
    std::vector<std::shared_ptr<MIDIStream>> streams;

    uint16_t no_tracks = 0;
    float duration_sec = 0.0f;
    float pos_sec = 0.0f;

    const std::string format_name[MIDI_FILE_FORMAT_MAX+1] = {
        "MIDI File 0", "MIDI File 1", "MIDI File 2",
        "Unknown MIDI File format"
    };
    const std::string mode_name[MIDI_MODE_MAX] = {
        "MIDI", "General MIDI 1.0", "General MIDI 2.0", "GS MIDI", "XG MIDI"
    };
};

} // namespace aeonwave

