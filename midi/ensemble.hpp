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

#include <aax/instrument.hpp>

#include "base/types.h"

namespace aax
{

class MIDIDriver;

class MIDIEnsemble : public Ensemble
{
private:
    MIDIEnsemble(const MIDIEnsemble&) = delete;

    MIDIEnsemble& operator=(const MIDIEnsemble&) = delete;

public:
    MIDIEnsemble(MIDIDriver& ptr, Buffer& buffer,
                   uint8_t channel, uint16_t bank, uint8_t program,
                   bool is_drums);

    MIDIEnsemble(MIDIEnsemble&&) = default;

    virtual ~MIDIEnsemble() = default;

    MIDIEnsemble& operator=(MIDIEnsemble&&) = default;

    void play(uint8_t key_no, uint8_t velocity, float pitch);
    void stop(uint32_t key_no, float velocity = 0);

    uint16_t get_channel_no() { return channel_no; }
    void set_channel_no(uint16_t channel) { channel_no = channel; }

    uint16_t get_program_no() { return program_no; }
    void set_program_no(uint16_t program) { program_no = program; }

    uint16_t get_bank_no() { return bank_no; }
    void set_bank_no(uint16_t bank) { bank_no = bank; }

    void set_tuning(float pitch) { tuning = powf(2.0f, pitch/12.0f); }
    float get_tuning() { return tuning; }

    void set_semi_tones(float s) { semi_tones = s; }
    float get_semi_tones() { return semi_tones; }

    void set_modulation_depth(float d) { modulation_range = d; }
    float get_modulation_depth() { return modulation_range; }

    bool get_pressure_volume_bend() { return pressure_volume_bend; }
    bool get_pressure_pitch_bend() { return pressure_pitch_bend; }
    float get_aftertouch_sensitivity() { return pressure_sensitivity; }

    void set_track_name(std::string& tname) { track_name = tname; }

    void set_stereo(bool s);
    bool get_stereo() { return stereo; }

    Buffer& get_buffer(uint8_t key) {
        auto it = name_map.find(key);
        if (it == name_map.end()) return aax::nullBuffer;
        return it->second;
    }

private:
    std::map<uint8_t,Buffer&> name_map;
    std::string track_name;

    MIDIDriver &midi;

    Emitter key_on;
    Emitter key_off;
    Param key_on_pitch_param = 1.0f;
    Param key_off_pitch_param = 1.0f;

    float tuning = 1.0f;

    float pan_prev = -1000.0f;

    float buffer_frequency = 22050.f;
    float buffer_fraction = 1.0f;

    float modulation_range = 2.0f;
    float pressure_sensitivity = 1.0f;
    float semi_tones = 2.0f;

    uint16_t bank_no = 0;
    uint16_t channel_no = 0;
    uint16_t program_no = 0;

    bool stereo = false;
    bool pressure_volume_bend = true;
    bool pressure_pitch_bend = false;
};

} // namespace aax
