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

#ifndef __AAX_MIDIINSTRUMENT
#define __AAX_MIDIINSTRUMENT

#include <map>

#include <aax/instrument.hpp>

namespace aax
{

class MIDIDriver;

class MIDIInstrument : public Instrument
{
private:
    MIDIInstrument(const MIDIInstrument&) = delete;

    MIDIInstrument& operator=(const MIDIInstrument&) = delete;

public:
    MIDIInstrument(MIDIDriver& ptr, Buffer &buffer, uint8_t channel,
                uint16_t bank, uint8_t program, bool is_drums);

    MIDIInstrument(MIDIInstrument&&) = default;

    virtual ~MIDIInstrument() = default;

    MIDIInstrument& operator=(MIDIInstrument&&) = default;

    void play(uint8_t key_no, uint8_t velocity, float pitch);

    inline void set_drums(bool d = true) { drum_channel = d; }
    inline bool is_drums() { return drum_channel; }

    inline uint16_t get_channel_no() { return channel_no; }
    inline void set_channel_no(uint16_t channel) { channel_no = channel; }

    inline uint8_t get_program_no() { return program_no; }
    inline void set_program_no(uint8_t program) { program_no = program; }

    inline uint16_t get_bank_no() { return bank_no; }
    inline void set_bank_no(uint16_t bank) { bank_no = bank; }

    inline void set_tuning(float pitch) { tuning = powf(2.0f, pitch/12.0f); }
    inline float get_tuning() { return tuning; }

    inline void set_semi_tones(float s) { semi_tones = s; }
    inline float get_semi_tones() { return semi_tones; }

    inline void set_modulation_depth(float d) { modulation_range = d; }
    inline float get_modulation_depth() { return modulation_range; }

    inline bool get_pressure_volume_bend() { return pressure_volume_bend; }
    inline bool get_pressure_pitch_bend() { return pressure_pitch_bend; }
    inline float get_aftertouch_sensitivity() { return pressure_sensitivity; }

    inline void set_track_name(std::string& tname) { track_name = tname; }

    inline Buffer& get_buffer(uint8_t key) {
        auto it = name_map.find(key);
        if (it == name_map.end()) return nullBuffer;
        return it->second;
    }

private:
    std::pair<uint8_t,std::string> get_patch(std::string& name, uint8_t& key);

    std::map<uint8_t,Buffer&> name_map;
    std::string track_name;

    MIDIDriver &midi;

    Buffer nullBuffer;

    float tuning = 1.0f;
    float modulation_range = 2.0f;
    float pressure_sensitivity = 1.0f;
    float semi_tones = 2.0f;

    uint16_t bank_no = 0;
    uint16_t channel_no = 0;
    uint8_t program_no = 0;

    bool drum_channel = false;
    bool pressure_volume_bend = true;
    bool pressure_pitch_bend = false;
};

} // namespace aax


#endif // __AAX_MIDIINSTRUMENT
