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
#include <random>

#include <aax/byte_stream.hpp>

#include <midi/shared.hpp>

#include "base/types.h"

namespace aeonwave
{

#define _MINMAX(a,b,c)  (((a)>(c)) ? (c) : (((a)<(b)) ? (b) : (a)))

class MIDIDriver;

struct param_t
{
   uint8_t coarse;
   uint8_t fine;
};

class MIDIEnsemble;

class MIDIStream : public byte_stream
{
public:
    MIDIStream() = default;

    MIDIStream(MIDIDriver& ptr, byte_stream& stream, size_t len,  uint16_t track);
    MIDIStream(const MIDIStream&) = default;

    virtual ~MIDIStream() = default;

    void rewind();
    bool process(uint64_t, uint32_t&, uint32_t&);

    inline uint8_t get_track_no() { return track_no; }
    inline uint16_t get_channel_no() { return channel_no; }
    inline const std::string& get_channel_name() { return name; }

    MIDIDriver& midi;
private:
    float cents2pitch(float p, uint8_t channel);
    float cents2modulation(float p, uint8_t channel);

    // https://stackoverflow.com/questions/4059775/convert-iso-8859-1-strings-to-utf-8-in-c-c
    inline void toUTF8(std::string& text, uint8_t c) {
       if (c < 128) {
          if (c == '\r') text += '\n';
          else text += c;
       }
       else if (c >= 160) { // skip control characters
           text += 0xc2+(c > 0xbf); text += (c & 0x3f)+0x80;
        }
    }

    uint32_t pull_message();
    bool registered_param(uint8_t, uint8_t, uint8_t, const char*);
    bool registered_param_3d(uint8_t, uint8_t, uint8_t);

    std::mt19937 m_mt;

    std::string name;

    uint8_t mode = 0;
    uint8_t track_no = 0;
    uint16_t channel_no = 0;
    uint8_t program_no = 0;
    uint16_t bank_no = 0;

    uint8_t previous = 0;
    uint32_t wait_parts = 1;
    uint64_t timestamp_parts = 0;
    bool polyphony = true;
    bool omni = true;

    bool pitch_bend_enabled = true;
    bool channel_pressure_enabled = true;
    bool program_change_enabled = true;
    bool control_change_enabled = true;
    bool poly_pressure_enabled = true;
    bool note_message_enabled = true;
    bool rpn_enabled = true;
    bool nrpn_enabled = true;
    bool modulation_enabled = true;
    bool volume_enabled = true;
    bool pan_enabled = true;
    bool expression_enabled = true;
    bool hold1_enabled = true;
    bool sustain_enabled = true;
    bool soft_enabled = true;
    bool bank_select_enabled = true;
    bool bank_select_lsb_enabled = true;
    bool equalizer_enabled = true;

    bool rpn = true;
    bool registered = false;
    uint8_t prev_controller = 0;

    uint8_t key_range_low = 0;
    uint8_t key_range_high = 127;

    uint32_t smpte_parts = 0;
    float smpte_framerate[4] = {
        24.0f, 25.0f, 29.97f, 30.0f
    };

    uint16_t msb_type = 0;
    uint16_t lsb_type = 0;
    std::map<uint16_t,struct param_t> param = {
        {0, { 2, 0 }}, {1, { 0x40, 0 }}, {2, { 0x20, 0 }}, {3, { 0, 0 }},
        {4, { 0, 0 }}, {5, { 1, 0 }}, {6, { 0, 0 }}
    };
    std::map<uint16_t,struct param_t> param_3d;

    const std::string type_name[10] = {
        "Sequencey", "Text", "Copyright", "Title", "Instrument", "Lyrics",
        "Marker", "Cue", "Program", "Device"
    };
    const std::string csv_name[10] = {
        "Sequence_number", "Text_t", "Copyright_t", "Title_t",
        "Instrument_name_t", "Lyrics_t", "Marker_t", "Cue_point_t",
        "Program_name_t", "Device_name_t"
    };

    bool process_control(uint8_t);
    bool process_meta();
    bool process_sysex();

    std::string GM_initialize(uint8_t mode);
    bool GM_process_sysex_realtime(uint64_t, std::string&);
    bool GM_process_sysex_non_realtime(uint64_t, std::string&);

    uint8_t GS_mode = 0;
    void GS_initialize();
    uint8_t GS_checksum(uint64_t);
    uint8_t GS_Address2Part(uint8_t);
    bool GS_process_sysex(uint64_t, std::string& expl);
    bool GS_sysex_equalizer(uint8_t part_no, uint8_t addr, uint8_t value);
    bool GS_sysex_insertion(uint8_t part_no, uint8_t addr, uint16_t type, std::string& expl);
    bool GS_sysex_modulation(uint8_t part_no, uint8_t addr, uint8_t value, std::string& expl);
    bool GS_sysex_part(uint8_t part_no, uint8_t addr, uint8_t value, std::string& expl);

    void XG_initialize();
    bool XG_process_sysex(uint64_t, std::string&);
    uint8_t XG_part_no[32] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
       17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
    };
};

} // namespace aeonwave

