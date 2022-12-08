/*
xmlIdd * Copyright (C) 2018-2022 by Erik Hofman.
 * Copyright (C) 2018-2022 by Adalin B.V.
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

#include <cassert>

#include <thread>

#include <midi/shared.hpp>
#include <midi/file.hpp>
#include <midi/driver.hpp>
#include <midi/instrument.hpp>

using namespace aax;

MIDIInstrument::MIDIInstrument(MIDIDriver& ptr, Buffer &buffer,
                 uint8_t channel, uint16_t bank, uint8_t program, bool is_drums)
   : Instrument(ptr, channel == MIDI_DRUMS_CHANNEL), midi(ptr),
     channel_no(channel), bank_no(bank),
     program_no(program),
     drum_channel(channel == MIDI_DRUMS_CHANNEL ? true : is_drums)
{
    set_gain(100.0f/127.0f);
    if (drum_channel && buffer) {
       Mixer::add(buffer);
    }
    Mixer::set(AAX_PLAYING);
}

void
MIDIInstrument::set_stereo(bool s)
{
    stereo = s;
    if (stereo)
    {
        std::string name = "stereo";
        Buffer &buffer = midi.buffer(name, 0);
        int res = MIDIInstrument::add(buffer);
    }
}

void
MIDIInstrument::play(uint8_t key_no, uint8_t velocity, float pitch)
{
    assert (velocity);

    bool all = midi.no_active_tracks() > 0;
    auto it = name_map.begin();
    if (midi.channel(channel_no).is_drums())
    {
        it = name_map.find(key_no);
        if (it == name_map.end())
        {
            auto inst = midi.get_drum(bank_no, program_no, key_no, all);
            std::string& filename = inst.first.file;
            if (!filename.empty() && filename != "")
            {
                if (!midi.buffer_avail(filename))
                {
                    std::string& display = (midi.get_verbose() >= 99) ?
                                           inst.first.file : inst.first.name;

                    DISPLAY(2, "Loading drum:  %3i bank: %3i/%3i, program: %3i: # %s\n",
                             key_no, bank_no >> 7, bank_no & 0x7F,
                             program_no, display.c_str());
                    midi.load(filename);
                }

                if (midi.get_grep())
                {
                   auto ret = name_map.insert({key_no,nullBuffer});
                   it = ret.first;
                }
                else
                {
                    Buffer& buffer = midi.buffer(filename);
                    if (buffer)
                    {
                        auto ret = name_map.insert({key_no,buffer});
                        it = ret.first;
                    }
                    else {
                        throw(std::invalid_argument("Instrument file "+filename+" could not load"));
                    }
                }
            }
        }
    }
    else
    {
        uint8_t key = key_no;
        it = name_map.upper_bound(key);
        if (it == name_map.end())
        {
            auto inst = midi.get_instrument(bank_no, program_no, all);
            auto patch = midi.get_patch(inst.first.file, key);
            std::string& patch_name = patch.second;
            uint8_t level = patch.first;
            if (!patch_name.empty())
            {
                if (!midi.buffer_avail(patch_name) &&
                    !midi.is_loaded(patch_name))
                {
                    std::string& display = (midi.get_verbose() >= 99) ?
                                           inst.first.file : inst.first.name;

                    DISPLAY(2, "Loading instrument bank: %3i/%3i, program: %3i: %s\n",
                             bank_no >> 7, bank_no & 0x7F, program_no,
                             display.c_str());
                    midi.load(patch_name);
                }

                if (midi.get_grep())
                {
                   auto ret = name_map.insert({key,nullBuffer});
                   it = ret.first;
                }
                else
                {
                    Buffer& buffer = midi.buffer(patch_name, level);
                    if (buffer)
                    {
                        auto ret = name_map.insert({key,buffer});
                        it = ret.first;

                        // mode == 0: volume bend only
                        // mode == 1: pitch bend only
                        // mode == 2: volume and pitch bend
                        int pressure_mode = buffer.get(AAX_PRESSURE_MODE);
                        if (pressure_mode == 0 || pressure_mode == 2) {
                           pressure_volume_bend = true;
                        }
                        if (pressure_mode > 0) {
                           pressure_pitch_bend = true;
                        }

                        // AAX_AFTERTOUCH_SENSITIVITY == AAX_VELOCITY_FACTOR
                        pressure_sensitivity = 0.01f*buffer.get(AAX_VELOCITY_FACTOR);
                    }
                    else {
                        throw(std::invalid_argument("Instrument file "+patch_name+" could not load"));
                    }
                    midi.channel(channel_no).set_wide(inst.second.wide);
                    midi.channel(channel_no).set_spread(inst.second.spread);
                    midi.channel(channel_no).set_stereo(inst.second.stereo);
                }
            }
        }
    }

    if (!midi.get_initialize() && it != name_map.end())
    {
        if (midi.channel(channel_no).is_drums())
        {
            switch(program_no)
            {
            case 0:	// Standard Set
            case 16:	// Power set
            case 32:	// Jazz set
            case 40:	// Brush set
                switch(key_no)
                {
                case 29: // EXC7
                    Instrument::stop(30);
                    break;
                case 30: // EXC7
                    Instrument::stop(29);
                    break;
                case 42: // EXC1
                    Instrument::stop(44);
                    Instrument::stop(46);
                    break;
                case 44: // EXC1
                    Instrument::stop(42);
                    Instrument::stop(46);
                    break;
                case 46: // EXC1
                    Instrument::stop(42);
                    Instrument::stop(44);
                    break;
                case 71: // EXC2
                    Instrument::stop(72);
                    break;
                case 72: // EXC2
                    Instrument::stop(71);
                    break;
                case 73: // EXC3
                    Instrument::stop(74);
                    break;
                case 74: // EXC3
                    Instrument::stop(73);
                    break;
                case 78: // EXC4
                    Instrument::stop(79);
                    break;
                case 79: // EXC4
                    Instrument::stop(78);
                    break;
                case 80: // EXC5
                    Instrument::stop(81);
                    break;
                case 81: // EXC5
                    Instrument::stop(80);
                    break;
                case 86: // EXC6
                    Instrument::stop(87);
                    break;
                case 87: // EXC6
                    Instrument::stop(86);
                    break;
                default:
                    break;
                }
                break;
            case 26:	// Analog Set
                switch(key_no)
                {
                case 42: // EXC1
                    Instrument::stop(44);
                    Instrument::stop(46);
                    break;
                case 44: // EXC1
                    Instrument::stop(42);
                    Instrument::stop(46);
                    break;
                case 46: // EXC1
                    Instrument::stop(42);
                    Instrument::stop(44);
                    break;
                default:
                    break;
                }
                break;
            case 48:	// Orchestra Set
                switch(key_no)
                {
                case 27: // EXC1
                    Instrument::stop(28);
                    Instrument::stop(29);
                    break;
                case 28: // EXC1
                    Instrument::stop(27);
                    Instrument::stop(29);
                    break;
                case 29: // EXC1
                    Instrument::stop(27);
                    Instrument::stop(28);
                    break;
                case 42: // EXC1
                    Instrument::stop(44);
                    Instrument::stop(46);
                    break;
                case 44: // EXC1
                    Instrument::stop(42);
                    Instrument::stop(46);
                    break;
                case 46: // EXC1
                    Instrument::stop(42);
                    Instrument::stop(44);
                    break;
                default:
                    break;
                }
                break;
            case 57:	// SFX Set
                switch(key_no)
                {
                case 41: // EXC7
                    Instrument::stop(42);
                    break;
                case 42: // EXC7
                    Instrument::stop(41);
                    break;
                default:
                    break;
                }
                break;
            default:
                break;
            }
        }

        Instrument::play(key_no, velocity/127.0f, it->second, pitch);

        bool all = midi.no_active_tracks() > 0;
        auto inst = midi.get_instrument(bank_no, program_no, all);
        std::string& patch_name = inst.first.key_on;
        if (!patch_name.empty())
        {
            bool wide = inst.second.wide;
            if (!key_on)
            {
                key_on = Emitter(wide ? AAX_ABSOLUTE : AAX_RELATIVE);

                std::string name = inst.first.name;
                MESSAGE(3, "Loading %s key-off file: %s\n",
                        name.c_str(),  patch_name.c_str());
                key_on.add( midi.buffer(patch_name) );

                buffer_frequency = midi.buffer(patch_name).get(AAX_UPDATE_RATE);
                buffer_fraction = 1e-6f*midi.buffer(patch_name).get(AAX_REFRESH_RATE);
                key_on.tie(key_on_pitch_param, AAX_PITCH_EFFECT, AAX_PITCH);

                pan.wide = inst.second.wide;

                Mixer::add(key_on);
            }

            // note2pitch
            float key_frequency = note2freq(key_no);
            float key_freq = (key_frequency - buffer_frequency)*buffer_fraction;
            key_freq += buffer_frequency;

            float pitch = key_freq/buffer_frequency;
            key_on_pitch_param = pitch;

            // panning
            if (wide)
            {
                key_freq = (key_frequency - buffer_frequency); //*buffer_fraction;
                key_freq += buffer_frequency;

                float p = (lin2log(key_freq) - 1.3f)/2.8f; // 0.0f .. 1.0f
                p = floorf(-2.0f*(p-0.5f)*PAN_LEVELS)/PAN_LEVELS;
                if (p != pan_prev)
                {
                    pan.set(p, true);
                    key_on.matrix(pan.mtx);
                    pan_prev = p;
                }
            }

            key_on.set(AAX_PROCESSED);
            key_on.set(AAX_INITIALIZED);
            key_on.set(AAX_MIDI_ATTACK_VELOCITY_FACTOR, velocity);
            key_on.set(AAX_PLAYING);
        }
    } else {
//      throw(std::invalid_argument("Instrument file "+name+" not found"));
    }
}

void
MIDIInstrument::stop(uint32_t key_no, float velocity)
{
    Instrument::stop(key_no, velocity);
//  if (midi.get_initialize()) return;

    bool all = midi.no_active_tracks() > 0;
    auto inst = midi.get_instrument(bank_no, program_no, all);
    std::string& patch_name = inst.first.key_off;
    if (!patch_name.empty())
    {
        bool wide = inst.second.wide;
        if (!key_off)
        {
            key_off = Emitter(wide ? AAX_ABSOLUTE : AAX_RELATIVE);

            std::string name = inst.first.name;
            MESSAGE(3, "Loading %s key-off file: %s\n",
                    name.c_str(),  patch_name.c_str());
            key_off.add( midi.buffer(patch_name) );

            buffer_frequency = midi.buffer(patch_name).get(AAX_UPDATE_RATE);
            buffer_fraction = 1e-6f*midi.buffer(patch_name).get(AAX_REFRESH_RATE);
            key_off.tie(key_off_pitch_param, AAX_PITCH_EFFECT, AAX_PITCH);

            pan.wide = inst.second.wide;

            Mixer::add(key_off);
        }

        // note2pitch
        float key_frequency = note2freq(key_no);
        float key_freq = (key_frequency - buffer_frequency)*buffer_fraction;
        key_freq += buffer_frequency;

        float pitch = key_freq/buffer_frequency;
        key_off_pitch_param = pitch;

        // panning
        if (wide)
        {
            key_freq = (key_frequency - buffer_frequency); //*buffer_fraction;
            key_freq += buffer_frequency;

            float p = (lin2log(key_freq) - 1.3f)/2.8f; // 0.0f .. 1.0f
            p = floorf(-2.0f*(p-0.5f)*PAN_LEVELS)/PAN_LEVELS;
            if (p != pan_prev)
            {
                pan.set(p, true);
                key_off.matrix(pan.mtx);
                pan_prev = p;
            }
        }

        key_off.set(AAX_PROCESSED);
        key_off.set(AAX_INITIALIZED);
        key_off.set(AAX_MIDI_ATTACK_VELOCITY_FACTOR, velocity);
        key_off.set(AAX_PLAYING);
    }
}
