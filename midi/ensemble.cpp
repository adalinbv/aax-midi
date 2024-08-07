/*
 * SPDX-FileCopyrightText: Copyright © 2018-2024 by Erik Hofman.
 * SPDX-FileCopyrightText: Copyright © 2018-2024 by Adalin B.V.
 *
 * Package Name: AeonWave MIDI library.
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
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
     bank_no(bank), channel_no(channel), program_no(program)
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
        MIDIEnsemble::add(buffer);
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
            auto& inst = midi.get_drum(bank_no, program, note_no, all);
            if (inst.size() && !inst[0].file.empty())
            {
                const std::string& filename = inst[0].file;
                if (!midi.buffer_avail(filename))
                {
                    uint16_t bank_no = midi.channel(channel_no).get_bank_no();
                    const std::string& display = (midi.get_verbose() >= 99) ?
                                                  inst[0].file : inst[0].name;

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
        auto& inst = midi.get_instrument(bank_no, program_no);
        if (inst.size() && !inst[0].file.empty())
        {
            const std::string& patch_file = inst[0].file;
            const std::string& patch_name = inst[0].name;
            if (!midi.is_loaded(patch_name))
            {
                uint16_t bank_no = midi.channel(channel_no).get_bank_no();
                const std::string& display = (midi.get_verbose() >= 99) ?
                                              inst[0].file : inst[0].name;

                if (inst[0].ensemble) {
                    DISPLAY(2, "Loading ensemble   bank: %3i/%3i, program: %3i: %s\n",
                         bank_no >> 7, bank_no & 0x7F, program_no+1,
                         display.c_str());
                } else {
                    DISPLAY(2, "Loading instrument bank: %3i/%3i, program: %3i: %s\n",
                         bank_no >> 7, bank_no & 0x7F, program_no+1,
                         display.c_str());
                }
                midi.load(patch_name);
            }

            if (midi.get_grep())
            {
               auto ret = name_map.insert({note_no,aax::nullBuffer});
               it = ret.first;
            }
            else
            {
                Buffer& buffer = midi.buffer(patch_file);
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
                    throw(std::invalid_argument("Instrument file "+patch_file+" could not load"));
                }
                midi.channel(channel_no).set_wide(inst[0].wide);
                midi.channel(channel_no).set_spread(inst[0].spread);
                midi.channel(channel_no).set_stereo(inst[0].stereo);
            }
        }
    }

    if (!midi.get_initialize() && it != name_map.end())
    {
        if (midi.channel(channel_no).is_drums())
        {
            switch(program_no)
            {
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
                case 41: // EXC7: Scratch Push
                    Instrument::stop(42);
                    break;
                case 42: // EXC7: Scratch Pull
                    Instrument::stop(41);
                    break;
                default:
                    break;
                }
                break;
            case 0: // Standard Set
            case 16: // Power set
            case 32: // Jazz set
            case 40: // Brush set
            default:
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
            }

            Instrument::play(note_no, velocity, it->second);
            return;
        }

        if (Ensemble::no_members() == 0)
        {
            bool all = midi.no_active_tracks() > 0;
            auto& ens = midi.get_instrument(bank_no, program_no, all);
            for (size_t n=0; n<ens.size(); ++n)
            {
                auto& i = ens[n];
                Buffer& buffer = midi.buffer(i.file);
                if (buffer)
                {
                    auto& m = Ensemble::add_member(buffer, i.pitch, i.gain, i.count);
                    m->set_note_minmax(i.min_note, i.max_note);
                    m->set_velocity_fraction(i.velocity_fraction);
//                  Ensemble::set_pan(i.pan);
                } else {
                    ERROR("Unable to open: " << i.file);
                }
            }
        }
        Ensemble::play(note_no, velocity, 1.0f);

        bool all = midi.no_active_tracks() > 0;
        auto& inst = midi.get_instrument(bank_no, program_no, all);
        if (inst.size() && !inst[0].key_on.empty())
        {
            const std::string& patch_name = inst[0].key_on;
            bool wide = inst[0].wide;
            if (!note_on)
            {
                note_on = Emitter(wide ? AAX_ABSOLUTE : AAX_RELATIVE);

                std::string name = inst[0].name;
                MESSAGE(3, "Loading %s: note-on file: %s\n",
                        name.c_str(),  patch_name.c_str());
                note_on.add( midi.buffer(patch_name) );
                note_on.tie(note_on_pitch_param, AAX_PITCH_EFFECT, AAX_PITCH);

                pan.wide = inst[0].wide;

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
MIDIEnsemble::stop(int note_no, uint8_t velocity)
{
    Ensemble::stop(note_no, velocity);
    if (is_drums()) return;

    bool all = midi.no_active_tracks() > 0;
    auto& inst = midi.get_instrument(bank_no, program_no, all);
    if (inst.size() && !inst[0].key_off.empty())
    {
        const std::string& patch_name = inst[0].key_off;
        bool wide = inst[0].wide;
        if (!note_off)
        {
            note_off = Emitter(wide ? AAX_ABSOLUTE : AAX_RELATIVE);

            std::string name = inst[0].name;
            MESSAGE(3, "Loading %s: note-off file: %s\n",
                    name.c_str(),  patch_name.c_str());
            note_off.add( midi.buffer(patch_name) );
            note_off.tie(note_off_pitch_param, AAX_PITCH_EFFECT, AAX_PITCH);

            pan.wide = inst[0].wide;

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
