/*
 * Copyright (C) 2018-2024 by Erik Hofman.
 * Copyright (C) 2018-2024 by Adalin B.V.
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

#include <xml.h>

#include <midi/shared.hpp>
#include <midi/file.hpp>
#include <midi/driver.hpp>
#include <midi/ensemble.hpp>

using namespace aax;

MIDIEnsemble::MIDIEnsemble(MIDIDriver& ptr, Buffer &buffer,
                    uint8_t channel, uint16_t bank, uint8_t program, bool drums)
   : Ensemble(ptr, buffer, channel == MIDI_DRUMS_CHANNEL), midi(ptr),
     channel_no(channel), bank_no(bank),
     program_no(program)
{
    set_gain(aax::math::ln(100.0f/127.0f));
    set_expression(aax::math::ln(127.0f/127.0f));
    set_pan(0.0f/64.f);
    set_drums(channel == MIDI_DRUMS_CHANNEL ? true : drums);
    if (is_drums() && buffer) {
       Mixer::add(buffer);
    }
    Mixer::set(AAX_PLAYING);
}

void
MIDIEnsemble::set_stereo(bool s)
{
    stereo = s;
    if (stereo)
    {
        std::string name = "stereo";
        Buffer &buffer = midi.buffer(name);
        int res = MIDIEnsemble::add(buffer);
    }
}

void
MIDIEnsemble::play(int note_no, uint8_t velocity)
{
    assert (velocity);

    bool all = midi.no_active_tracks() > 0;
    auto it = name_map.begin();
    if (midi.channel(channel_no).is_drums())
    {
        it = name_map.find(note_no);
        if (it == name_map.end())
        {
            uint16_t program = program_no;
            auto inst = midi.get_drum(bank_no, program, note_no, all);
            std::string& filename = inst.file;
            if (!filename.empty() && filename != "")
            {
                if (!midi.buffer_avail(filename))
                {
                    uint16_t bank_no = midi.channel(channel_no).get_bank_no();
                    std::string& display = (midi.get_verbose() >= 99) ?
                                           inst.file : inst.name;

                    DISPLAY(2, "Loading drum:  %3i bank: %3i/%3i, program: %3i: # %s\n",
                             note_no, bank_no >> 7, bank_no & 0x7F,
                             program, display.c_str());
                    midi.load(filename);
                }

                if (midi.get_grep())
                {
                   auto ret = name_map.insert({note_no,aax::nullBuffer});
                   it = ret.first;
                }
                else
                {
                    Buffer& buffer = midi.buffer(filename);
                    if (buffer)
                    {
                        auto ret = name_map.insert({note_no,buffer});
                        it = ret.first;
                    }
                    else {
                        throw(std::invalid_argument("Instrument file "+filename+" could not load"));
                    }
                }
            }
        }
    }
    else // !drums
    {
        auto inst = midi.get_instrument(bank_no, program_no, all);
        std::string& patch_name = inst.file;
        if (!patch_name.empty())
        {
            if (!midi.buffer_avail(patch_name) && !midi.is_loaded(patch_name))
            {
                uint16_t bank_no = midi.channel(channel_no).get_bank_no();
                std::string& display = (midi.get_verbose() >= 99) ?
                                       inst.file : inst.name;

                DISPLAY(2, "Loading instrument bank: %3i/%3i, program: %3i: %s\n",
                         bank_no >> 7, bank_no & 0x7F, program_no+1,
                         display.c_str());
                midi.load(patch_name);
            }

            if (midi.get_grep())
            {
               auto ret = name_map.insert({note_no,aax::nullBuffer});
               it = ret.first;
            }
            else
            {
                Buffer& buffer = midi.buffer(patch_name);
                if (buffer)
                {
                    auto ret = name_map.insert({note_no,buffer});
                    it = ret.first;

                    // mode == 0: volume bend only
                    // mode == 1: pitch bend only
                    // mode == 2: volume and pitch bend
                    int pressure_mode = buffer.get(AAX_MIDI_PRESSURE_FACTOR);
                    if (pressure_mode == 0 || pressure_mode == 2) {
                       p.pressure_volume_bend = true;
                    }
                    if (pressure_mode > 0) {
                       p.pressure_pitch_bend = true;
                    }

                    p.pressure_sensitivity = 0.01f*buffer.get(AAX_MIDI_RELEASE_VELOCITY_FACTOR);
                }
                else {
                    throw(std::invalid_argument("Instrument file "+patch_name+" could not load"));
                }
                midi.channel(channel_no).set_wide(inst.wide);
                midi.channel(channel_no).set_spread(inst.spread);
                midi.channel(channel_no).set_stereo(inst.stereo);
            }
        }
    }

    if (!midi.get_initialize() && it != name_map.end())
    {
        if (midi.channel(channel_no).is_drums())
        {
            switch(program_no)
            {
            case 0: // Standard Set
            case 16: // Power set
            case 32: // Jazz set
            case 40: // Brush set
                switch(note_no)
                {
                case 29: // EXC7: Scratch Push
                    Instrument::stop(30);
                    break;
                case 30: // EXC7: Scratch Pull
                    Instrument::stop(29);
                    break;
                case 42: // EXC1: Closed Hi-Hat"
                    Instrument::stop(44);
                    Instrument::stop(46);
                    break;
                case 44: // EXC1: Pedal Hi-Hat
                    Instrument::stop(42);
                    Instrument::stop(46);
                    break;
                case 46: // EXC1: Open Hi-Hat
                    Instrument::stop(42);
                    Instrument::stop(44);
                    break;
                case 71: // EXC2: Short Whistle
                    Instrument::stop(72);
                    break;
                case 72: // EXC2: Long Whistle
                    Instrument::stop(71);
                    break;
                case 73: // EXC3: Short Guir
                    Instrument::stop(74);
                    break;
                case 74: // EXC3: Long Guiro
                    Instrument::stop(73);
                    break;
                case 78: // EXC4: Mute Cuica
                    Instrument::stop(79);
                    break;
                case 79: // EXC4: Open Cuica
                    Instrument::stop(78);
                    break;
                case 80: // EXC5: Mute Triangle
                    Instrument::stop(81);
                    break;
                case 81: // EXC5: Open Triangle
                    Instrument::stop(80);
                    break;
                case 86: // EXC6: Mute Surdo
                    Instrument::stop(87);
                    break;
                case 87: // EXC6: Open Surdo
                    Instrument::stop(86);
                    break;
                default:
                    break;
                }
                break;
            case 26: // Analog Set
                switch(note_no)
                {
                case 42: // EXC1: Closed Hi-Hat
                    Instrument::stop(44);
                    Instrument::stop(46);
                    break;
                case 44: // EXC1: Pedal Hi-Hat
                    Instrument::stop(42);
                    Instrument::stop(46);
                    break;
                case 46: // EXC1: Open Hi-Hat
                    Instrument::stop(42);
                    Instrument::stop(44);
                    break;
                default:
                    break;
                }
                break;
            case 48: // Orchestra Set
                switch(note_no)
                {
                case 27: // EXC1: Closed Hi-Hat
                    Instrument::stop(28);
                    Instrument::stop(29);
                    break;
                case 28: // EXC1: Pedal Hi-Ha
                    Instrument::stop(27);
                    Instrument::stop(29);
                    break;
                case 29: // EXC1: Open Hi-Hat
                    Instrument::stop(27);
                    Instrument::stop(28);
                    break;
                default:
                    break;
                }
                break;
            case 57: // SFX Set
                switch(note_no)
                {
                case 41: // EXC7: Scratch Pus
                    Instrument::stop(42);
                    break;
                case 42: // EXC7: Scratch Pul
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

        if (is_drums())
        {
            Instrument::play(note_no, velocity/127.0f, it->second);
            return;
        }

        if (Ensemble::no_members() == 0) {
            register_members();
        }
        Ensemble::play(note_no, velocity/127.0f);

        bool all = midi.no_active_tracks() > 0;
        auto inst = midi.get_instrument(bank_no, program_no, all);
        std::string& patch_name = inst.key_on;
        if (!patch_name.empty())
        {
            bool wide = inst.wide;
            if (!note_on)
            {
                note_on = Emitter(wide ? AAX_ABSOLUTE : AAX_RELATIVE);

                std::string name = inst.name;
                MESSAGE(3, "Loading %s: note-on file: %s\n",
                        name.c_str(),  patch_name.c_str());
                note_on.add( midi.buffer(patch_name) );
                note_on.tie(note_on_pitch_param, AAX_PITCH_EFFECT, AAX_PITCH);

                pan.wide = inst.wide;

                Mixer::add(note_on);
            }

            // note2pitch
            float note_frequency =  aax::math::note2freq(note_no);
            note_on_pitch_param = buffer.get_pitch(note_no);

            // panning
            if (wide)
            {
                float p = (math::lin2log(note_frequency) - 1.3f)/2.8f; // 0.0f .. 1.0f
                p = floorf(-2.0f*(p-0.5f)*note::pan_levels)/note::pan_levels;
                if (p != pan_prev)
                {
                    pan.set(p, true);
                    note_on.matrix(pan.mtx);
                    pan_prev = p;
                }
            }

            note_on.set(AAX_PROCESSED);
            note_on.set(AAX_INITIALIZED);
            note_on.set(AAX_MIDI_ATTACK_VELOCITY_FACTOR, 127.0f*velocity);
            note_on.set(AAX_PLAYING);
        }
    } else {
//      throw(std::invalid_argument("Instrument file "+name+" not found"));
    }
}

void
MIDIEnsemble::stop(int note_no, float velocity)
{
    Ensemble::stop(note_no, velocity);
    if (is_drums()) return;

    bool all = midi.no_active_tracks() > 0;
    auto inst = midi.get_instrument(bank_no, program_no, all);
    std::string& patch_name = inst.key_off;
    if (!patch_name.empty())
    {
        bool wide = inst.wide;
        if (!note_off)
        {
            note_off = Emitter(wide ? AAX_ABSOLUTE : AAX_RELATIVE);

            std::string name = inst.name;
            MESSAGE(3, "Loading %s: note-off file: %s\n",
                    name.c_str(),  patch_name.c_str());
            note_off.add( midi.buffer(patch_name) );
            note_off.tie(note_off_pitch_param, AAX_PITCH_EFFECT, AAX_PITCH);

            pan.wide = inst.wide;

            Mixer::add(note_off);
        }

        // note2pitch
        float note_frequency =  aax::math::note2freq(note_no);
        note_off_pitch_param = buffer.get_pitch(note_no);

        // panning
        if (wide)
        {   // 0.0f .. 1.0f
            float p = (math::lin2log(note_frequency) - 1.3f)/2.8f;
            p = floorf(-2.0f*(p-0.5f)*note::pan_levels)/note::pan_levels;
            if (p != pan_prev)
            {
                pan.set(p, true);
                note_off.matrix(pan.mtx);
                pan_prev = p;
            }
        }

        note_off.set(AAX_PROCESSED);
        note_off.set(AAX_INITIALIZED);
        note_off.set(AAX_MIDI_ATTACK_VELOCITY_FACTOR, 64.0f*velocity);
        note_off.set(AAX_PLAYING);
    }
}

void // // TODO: add ensembles
MIDIEnsemble::register_members()
{
    bool all = midi.no_active_tracks() > 0;
    auto inst = midi.get_instrument(bank_no, program_no, all);
    if (inst.ensemble)
    {
        std::string path = midi.info(AAX_SHARED_DATA_DIR);
        path += inst.file.c_str();
        path += ".xml";
        xmlId *xid = xmlOpen(path.c_str());
        if (xid)
        {
            xmlId *xlid = xmlNodeGet(xid, "aeonwave/set/layer");
            if (xlid)
            {
                char file[64] = "";
                xmlId *xpid = xmlMarkId(xlid);
                int slen, num = xmlNodeGetNum(xlid, "patch");
                for (int i=0; i<num; i++)
                {
                    if (xmlNodeGetPos(xlid, xpid, "patch", i) != 0)
                    {
                        float gain = 1.0f, pitch = 1.0f;
                        if (xmlAttributeExists(xpid, "gain")) {
                            gain = xmlAttributeGetDouble(xpid, "gain");
                        }
                        if (xmlAttributeExists(xpid, "pitch")) {
                            gain = xmlAttributeGetDouble(xpid, "pitch");
                        }
                        int min = xmlAttributeGetInt(xpid, "min");
                        int max = xmlAttributeGetInt(xpid, "max");
                        slen = xmlAttributeCopyString(xpid, "file", file, 64);
                        if (slen)
                        {
                            file[slen] = 0;
                            Buffer& buffer = midi.buffer(file);
                            Ensemble::add_member(buffer, pitch, gain, min, max);
                        }
                    }
                }
                xmlFree(xpid);
            }
            xmlFree(xid);
        }
    }
    else
    {
        Buffer& buffer = midi.buffer(inst.file);
        Ensemble::add_member(buffer);
    }
}

