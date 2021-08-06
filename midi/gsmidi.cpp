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

#include <midi/instrument.hpp>
#include <midi/stream.hpp>
#include <midi/driver.hpp>


using namespace aax;



bool MIDIStream::process_GS_sysex(uint64_t size)
{
    bool rv = false;
    uint64_t offs = offset();
    uint8_t type, devno;
    uint8_t byte;

#if 0
 printf(" System Exclusive:");
 push_byte(); push_byte(); push_byte();
 while ((byte = pull_byte()) != MIDI_SYSTEM_EXCLUSIVE_END) printf(" %x", byte);
 printf("\n");
 byte_stream::rewind( offset() - offs);
#endif

    type = pull_byte();
    CSV(", %d", type);
    devno = type & 0xF;
    switch (type & 0xF0)
    {
    case GSMIDI_SYSTEM:
    {
        byte = pull_byte();
        CSV(", %d", byte);
        switch (byte)
        {
        case GSMIDI_MODEL_GS:
        {
            byte = pull_byte();
            CSV(", %d", byte);
            switch (byte)
            {
            case GSMIDI_DATA_SET1:
            {
                uint8_t addr_high = pull_byte();
                uint8_t addr_mid = pull_byte();
                uint8_t addr_low = pull_byte();
                uint16_t addr = addr_mid << 8 | addr_low;
                uint8_t value = pull_byte();
                CSV(", %d", value);
                switch (addr_high)
                {
                case GSMIDI_PARAMETER_CHANGE:
                {
                    switch(addr)
                    {
                    case GSMIDI_GS_RESET:
                        if (value != 0x00) break;

                        byte = pull_byte();
                        CSV(", %d", byte);
                        if (byte == 0x41) // checksum
                        {
                            midi.set_mode(MIDI_GENERAL_STANDARD);
                            rv = true;
                        }
                        break;
                    case GSMIDI_VOICE_RESERVE_PART1:
                    case GSMIDI_VOICE_RESERVE_PART2:
                    case GSMIDI_VOICE_RESERVE_PART3:
                    case GSMIDI_VOICE_RESERVE_PART4:
                    case GSMIDI_VOICE_RESERVE_PART5:
                    case GSMIDI_VOICE_RESERVE_PART6:
                    case GSMIDI_VOICE_RESERVE_PART7:
                    case GSMIDI_VOICE_RESERVE_PART8:
                    case GSMIDI_VOICE_RESERVE_PART9:
                    case GSMIDI_VOICE_RESERVE_PART10:
                    case GSMIDI_VOICE_RESERVE_PART11:
                    case GSMIDI_VOICE_RESERVE_PART12:
                    case GSMIDI_VOICE_RESERVE_PART13:
                    case GSMIDI_VOICE_RESERVE_PART14:
                    case GSMIDI_VOICE_RESERVE_PART15:
                    case GSMIDI_VOICE_RESERVE_PART16:
                        LOG(99, "LOG: Unsupported GS sysex voice reserve\n");
                        break;
                    case GSMIDI_DRUM_PART1:
                    case GSMIDI_DRUM_PART2:
                    case GSMIDI_DRUM_PART3:
                    case GSMIDI_DRUM_PART4:
                    case GSMIDI_DRUM_PART5:
                    case GSMIDI_DRUM_PART6:
                    case GSMIDI_DRUM_PART7:
                    case GSMIDI_DRUM_PART8:
                    case GSMIDI_DRUM_PART9:
                    case GSMIDI_DRUM_PART11:
                    case GSMIDI_DRUM_PART12:
                    case GSMIDI_DRUM_PART13:
                    case GSMIDI_DRUM_PART14:
                    case GSMIDI_DRUM_PART15:
                    case GSMIDI_DRUM_PART16:
                    {
                        uint8_t part_no = addr_mid & 0xf;
                        if (value == 0x02)
                        {
                            byte = pull_byte();
                            CSV(",%d", byte); 
                            if (byte == 0x10) {
                                midi.channel(part_no).set_drums(true);
                                rv = true;
                            }
                        }
                        break;
                    }
                    case GSMIDI_REVERB_MACRO:
                    case GSMIDI_REVERB_CHARACTER:
                        switch (value)
                        {
                        case GSMIDI_REVERB_ROOM1:
                            midi.set_reverb("reverb/room-small");
                            INFO("Switching to Small Room reveberation");
                            break;
                        case GSMIDI_REVERB_ROOM2:
                            midi.set_reverb("reverb/room-medium");
                            INFO("Switching to Medium Room reveberation");
                            break;
                        case GSMIDI_REVERB_ROOM3:
                            midi.set_reverb("reverb/room-large");
                            INFO("Switching to Large Room reveberation");
                            break;
                        case GSMIDI_REVERB_HALL1:
                            midi.set_reverb("reverb/concerthall");
                            INFO("Switching to Concert Hall Reveberation");
                            break;
                        case GSMIDI_REVERB_HALL2:
                            midi.set_reverb("reverb/concerthall-large");
                            INFO("Switching to Large Concert Hall reveberation");
                            break;
                        case GSMIDI_REVERB_PLATE:
                            midi.set_reverb("reverb/plate");
                            INFO("Switching to Plate reveberation");
                            break;
                        case GSMIDI_REVERB_DELAY:
                        case GSMIDI_REVERB_PAN_DELAY:
                        default:
                            LOG(99, "LOG: Unsupported GS sysex reverb type:"
                                    " 0x%x (%d)\n", type, type);
                            break;
                        }
                        break;
                    case GSMIDI_REVERB_PRE_LPF:
                    {
                        float val = value/7.0f;
                        float fc = 22000.0f - _log2lin(val*_lin2log(22000.0f));
                        midi.set_reverb_cutoff_frequency(fc);
                        break;
                    }
                    case GSMIDI_REVERB_LEVEL:
                    {
                        float val = (float)value/127.0f;
                        midi.set_reverb_level(track_no, val);
                        break;
                    }
                    case GSMIDI_REVERB_TIME:
                    {
                        float reverb_time = 0.7f*value/127.0f;
                        midi.set_reverb_time_rt60(reverb_time);
                        break;
                    }
                    case GSMIDI_REVERB_DELAY_FEEDBACK:
                        LOG(99, "LOG: Unsupported GS sysex Reverb Delay Feedback\n");
                        break;
                    case GSMIDI_CHORUS_MACRO:
                        switch (value)
                        {
                        case GSMIDI_CHORUS1:
                            midi.set_chorus("chorus/chorus1");
                            INFO("Switching to GS type 1 chorus");
                            break;
                        case GSMIDI_CHORUS2:
                            midi.set_chorus("chorus/chorus2");
                            INFO("Switching to GS type 2 chorus");
                            break;
                        case GSMIDI_CHORUS3:
                            midi.set_chorus("chorus/chorus3");
                            INFO("Switching to GS type 3 chorus");
                            break;
                        case GSMIDI_CHORUS4:
                            midi.set_chorus("chorus/chorus4");
                            INFO("Switching to GS type 4 chorus");
                            break;
                        case GSMIDI_FEEDBACK_CHORUS:
                            midi.set_chorus("chorus/chorus_freedback");
                            INFO("Switching to GS feedback chorus");
                            break;
                        case GSMIDI_FLANGER:
                            midi.set_chorus("chorus/flanger");
                            INFO("Switching to GS flanging");
                            break;
                        case GSMIDI_DELAY:
                            midi.set_chorus("chorus/delay");
                            INFO("Switching to GS short delay");
                            break;
                        case GSMIDI_DELAY_FEEDBACK:
                            midi.set_chorus("chorus/delay_feedback");
                            INFO("Switching to GS short delay with feedback");
                            break;
                        default:
                            LOG(99, "LOG: Unsupported GS sysex chorus type:"
                                    " 0x%x (%d)\n", type, type);
                            break;
                        }
                        break;
                    case GSMIDI_CHORUS_PRE_LPF:
                        LOG(99, "LOG: Unsupported GS sysex chorus pre-LPF\n");
                        break;
                    case GSMIDI_CHORUS_LEVEL:
                    {
                        float val = (float)value/127.0f;
                        midi.set_chorus_level(track_no, val);
                        break;
                    }
                    case GSMIDI_CHORUS_FEEDBACK:
                        midi.set_chorus_feedback(0.763f*value*1e-2f);
                        break;
                    case GSMIDI_CHORUS_DELAY:
                        LOG(99, "LOG: Unsupported GS sysex chorus delay\n");
                        break;
                    case GSMIDI_CHORUS_RATE:
                    {
                        float val = (float)value/127.0f;
                        midi.set_chorus_rate(val);
                        break;
                    }
                    case GSMIDI_CHORUS_DEPTH:
                    {
                        float val = (float)value/127.0f;
                        midi.set_chorus_depth(val);
                        break;
                    }
                    case GSMIDI_CHORUS_SEND_LEVEL_TO_REVERB:
                        midi.send_chorus_to_reverb(value/127.0f);
                        break;
                    default:
                    {
                        uint8_t part_no = addr_mid & 0xF;
                        switch (addr_mid)
                        {
                        case GSMIDI_EQUALIZER:
                            process_GS_sysex_equalizer(part_no, addr_low, value);
                            break;
                        case GSMIDI_INSERTION_EFFECT:
                            process_GS_sysex_insertion(part_no, addr_low, value);
                            break;
                        default:
                            switch (addr_mid & 0xF0)
                            {
                            case GSMIDI_PART_SET:
                                process_GS_sysex_part(part_no, addr_low, value);
                                break;
                            case GSMIDI_MODULATION_SET:
                                process_GS_sysex_modulation(part_no, addr_low, value);
                            case GSMIDI_PART_SWITCH:
                               LOG(99, "LOG: Unsupported GS sysex part switch\n");
                            default:
                                LOG(99, "LOG: Unsupported GS sysex address:"
                                        " 0x%x 0x%x (%d %d)\n",
                                        addr_mid, addr_low, addr_mid, addr_low);
                                break;
                            }
                            break;
                        }
                        break;
                    } // default
                    }
                    break;
                } // GSMIDI_PARAMETER_CHANGE
                case GSMIDI_DISPLAY_DATA:
                {
                    std::string text;
                    for (int i=offset()-offs; i<size; ++i) {
                        toUTF8(text, pull_byte());
                    }
                    MESSAGE("Display: %s\n", text.c_str());
                    break;
                }
                case GSMIDI_SYSTEM_PARAMETER_CHANGE:
                    switch (addr)
                    {
                    case GSMIDI_SYSTEM_MODE_SET:
                        LOG(99, "LOG: Unsupported GS sysex system mode set: MODE-%i\n", value+1);
                        break;
                    default:
                        LOG(99, "LOG: Unsupported GS sysex system parameter change\n");
                        break;
                    }
                    break;
                case GSMIDI_SYSTEM_INFORMATION:
                    break;
                default:
                    LOG(99, "LOG: Unsupported GS sysex effect type: 0x%x 0x%x 0x%x (%d %d %d)\n",
                            addr_high, addr_mid, addr_low,
                            addr_high, addr_mid, addr_low);
                   break;
                }
                break;
            } // GSMIDI_DATA_SET1
            case GSMIDI_DATA_REQUEST1:
                LOG(99, "LOG: Unsupported GS sysex data request\n");
                break;
            default:
                LOG(99, "LOG: Unsupported GS sysex parameter category: 0x%x (%d)\n",
                     byte, byte);
                break;
            }
            break;
        } // GSMIDI_MODEL_GS
        default:
            LOG(99, "LOG: Unsupported GS sysex model ID: 0x%x (%d)\n",
                     byte, byte);
            break;
        }
        break;
    } // GSMIDI_SYSTEM
    default:
        LOG(99, "LOG: Unsupported GS sysex category type: 0x%x (%d)\n", byte, byte);
        break;
    }

    return rv;
};

bool
MIDIStream::process_GS_sysex_equalizer(uint8_t part_no, uint8_t addr, uint8_t value)
{
    bool rv = true;
    switch(addr)
    {
    case GSMIDI_PART_EQUALIZER_SWITCH:
    default:
        LOG(99, "LOG: Unsupported GS sysex equalizer set: %x (%d)\n",
                     addr, addr);
        rv = false;
        break;
    }
    return rv;
}

bool
MIDIStream::process_GS_sysex_insertion(uint8_t part_no, uint8_t addr, uint8_t value)
{
    LOG(99, "LOG: Unsupported GS sysex insertion type: %x (%d)\n",
                 addr, addr);
    return false;
}

bool
MIDIStream::process_GS_sysex_modulation(uint8_t part_no, uint8_t addr, uint8_t value)
{
    bool rv = true;
    switch(addr)
    {
    case GSMIDI_MODULATION_DEPTH:
        LOG(99, "LOG: Unsupported GS sysex modulation depth\n");
        break;
    case GSMIDI_BEND_RANGE:
        LOG(99, "LOG: Unsupported GS sysex bend range\n");
        break;
    default:
        LOG(99, "LOG: Unsupported GS sysex modulation type: %x (%d)\n",
                 addr, addr);
        rv = false;
        break;
    }
    return rv;
}

bool
MIDIStream::process_GS_sysex_part(uint8_t part_no, uint8_t addr, uint8_t value)
{
    auto& channel = midi.channel(part_no);
    bool rv = true;
    switch(addr)
    {
    case GSMIDI_PART_PAN:
        if (mode != MIDI_MONOPHONIC) {
            channel.set_pan(((float)value-64.f)/64.f);
        }
        break;
    case GSMIDI_PART_VIBRATO_RATE:
    {
        float val = 0.5f + (float)value/64.0f;
        channel.set_vibrato_rate(val);
        break;
    }
    case GSMIDI_PART_VIBRATO_DEPTH:
    {
        float val = (float)value/64.0f;
        channel.set_vibrato_depth(val);
        break;
    }
    case GSMIDI_PART_VIBRATO_DELAY:
    {
        float val = (float)value/64.0f;
        channel.set_vibrato_delay(val);
        break;
    }
    case GSMIDI_PART_ATTACK_TIME:
        channel.set_attack_time(value);
        break;
    case GSMIDI_PART_DECAY_TIME:
        channel.set_decay_time(value);
        break;
    case GSMIDI_PART_RELEASE_TIME:
        channel.set_release_time(value);
        break;
    case GSMIDI_PART_REVERB_SEND_LEVEL:
    {
        float val = (float)value/127.0f;
        midi.set_reverb_level(part_no, val);
        break;
    }
    case GSMIDI_PART_VOLUME:
        channel.set_gain((float)value/127.0f);
        break;
    case MIDI_PROGRAM_CHANGE:
        try {
            midi.new_channel(part_no, bank_no, program_no);
        } catch(const std::invalid_argument& e) {
            ERROR("Error: " << e.what());
        }
        break;
    case GSMIDI_PART_POLY_MONO:
        midi.process(part_no, MIDI_NOTE_OFF, 0, 0, true);
        if (value == 0) {
            mode = MIDI_MONOPHONIC;
            channel.set_monophonic(true);
        } else {
            channel.set_monophonic(false);
            mode = MIDI_POLYPHONIC;
        }
        break;
    case GSMIDI_PART_NRPN_SWITCH:
    default:
        LOG(99, "LOG: Unsupported GS sysex part set: %x (%d)\n", addr, addr);
        rv = false;
        break;
    }
    return rv;
}
