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

#include <midi/instrument.hpp>
#include <midi/stream.hpp>
#include <midi/driver.hpp>

/*
 * The SC-8850 has two MIDI IN connectors. Each MIDI IN is able to receive data
 * for 16 parts, meaning that if the MIDI IN connectors are used to make
 * connections, a maximum of 32 parts can be played.
 *
 * Normally, MIDI IN 1 i used to play parts A01 through A16,
 * and MIDI IN 2 is used to play parts B01 through B16
 */

using namespace aax;

void MIDIStream::GS_initialize()
{
    float val = 40.0f/127.0f;
    for (auto& it : midi.get_channels())
    {
        midi.process(it.first, MIDI_NOTE_OFF, 0, 0, true);
        midi.set_reverb_level(it.first, val);
        midi.set_chorus_level(it.first, 0.0f);
        it.second->set_expression(_ln(127.0f/127.0f));
        it.second->set_gain(_ln(100.0f/127.0f));
        it.second->set_pan(0.0f);
        if (it.first != MIDI_DRUMS_CHANNEL) {
            it.second->set_drums(false);
        } else it.second->set_drums(true);
    }
    midi.set_mode(MIDI_GENERAL_STANDARD);
}

uint8_t MIDIStream::GS_checksum(uint64_t sum)
{
    return 128 - (sum % 128);
}

uint8_t MIDIStream::GS_Address2Part(uint8_t addr)
{
    uint8_t part_no = (addr & 0xf)-1;
    if (part_no == 255) part_no = 9;
    return part_no;
}

bool MIDIStream::GS_process_sysex(uint64_t size, std::string& expl)
{
    bool rv = false;
    uint64_t offs = offset();
    uint8_t type, devno;
    uint8_t byte;

#if 0
 printf(" System Exclusive:");
 push_byte(); push_byte(); push_byte();
 while ((byte = pull_byte()) != MIDI_SYSTEM_EXCLUSIVE_END) printf(" 0x%02x", byte);
 printf("\n");
 byte_stream::rewind( offset() - offs);
#endif

    type = pull_byte();
    CSV(channel_no, ", %d", type);
    devno = type & 0xF;
    switch (type & 0xF0)
    {
    case 0x70:
    case GSMIDI_SYSTEM:
    {
        byte = pull_byte();
        CSV(channel_no, ", %d", byte);
        switch (byte)
        {
        case GSMIDI_MODEL_GS:
        {
            byte = pull_byte();
            CSV(channel_no, ", %d", byte);
            switch (byte)
            {
            case GSMIDI_DATA_SET1:
            {
                uint8_t addr_high = pull_byte();
                uint8_t addr_mid = pull_byte();
                uint8_t addr_low = pull_byte();
                uint16_t addr = addr_mid << 8 | addr_low;
                int32_t value = pull_byte();
                uint64_t sum = addr_high + addr_mid + addr_low + value;
                CSV(channel_no, ", %d, %d, %d", addr_high, addr_mid, addr_low);
                CSV(channel_no, ", %d", value);
                switch (addr_high)
                {
                case GSMIDI_PARAMETER_CHANGE:
                {
                    switch(addr)
                    {
                    case GSMIDI_GS_RESET:
                        if (value != 0x00) break;

                        byte = pull_byte();
                        CSV(channel_no, ", %d", byte);

                        if (GS_checksum(sum) == byte)
                        {
                            expl = "RESET";
                            GS_initialize();
                            rv = true;
                        }
                        else expl = "RESET: Invalid checksum";
                        break;
                    case GSMIDI_MASTER_TUNE:
                    {   // 1st bit3-0: bit7-4, 2nd bit3-0: bit3-0
                        expl = "MASTER_TUNE";
                        int8_t tune = (value << 4);
                        float level;
                        byte = pull_byte();
                        CSV(channel_no, ", %d", byte);
                        tune |= byte & 0xf;
                        level = cents2pitch(0.1f*tune, channel_no);
                        for(auto& it : midi.get_channels()) {
                            it.second->set_detune(level);
                        }
                        break;
                    }
                    case GSMIDI_MASTER_VOLUME:
                        expl = "MASTER_VOLUME";
                        midi.set_gain(_ln(float(value)/127.0f));
                        break;
                    case GSMIDI_MASTER_KEY_SHIFT:
                        expl = "MASTER_KEY_SHIFT";
                        LOG(99, "LOG: Unsupported GS sysex Key-Shift\n");
                        break;
                    case GSMIDI_MASTER_PAN:
                        expl = "PAN";
                        if (mode != MIDI_MONOPHONIC) {
                            for(auto& it : midi.get_channels()) {
                                it.second->set_pan(float(value-64)/64.f);
                            }
                        }
                        break;
                    case GSMIDI_REVERB_MACRO:
                    case GSMIDI_REVERB_CHARACTER:
                        switch (value)
                        {
                        case GSMIDI_REVERB_ROOM1:
                            expl = "REVERB_ROOM1";
                            midi.set_reverb("reverb/room-small");
                            INFO("Switching to Small Room reveberation");
                            break;
                        case GSMIDI_REVERB_ROOM2:
                            expl = "REVERB_ROOM2";
                            midi.set_reverb("reverb/room-medium");
                            INFO("Switching to Medium Room reveberation");
                            break;
                        case GSMIDI_REVERB_ROOM3:
                            expl = "REVERB_ROOM3";
                            midi.set_reverb("reverb/room-large");
                            INFO("Switching to Large Room reveberation");
                            break;
                        case GSMIDI_REVERB_HALL1:
                            expl = "REVERB_HALL1";
                            midi.set_reverb("reverb/concerthall");
                            INFO("Switching to Concert Hall Reveberation");
                            break;
                        case GSMIDI_REVERB_HALL2:
                            expl = "REVERB_HALL2";
                            midi.set_reverb("reverb/concerthall-large");
                            INFO("Switching to Large Concert Hall reveberation");
                            break;
                        case GSMIDI_REVERB_PLATE:
                            expl = "REVERB_PLATE";
                            midi.set_reverb("reverb/plate");
                            INFO("Switching to Plate reveberation");
                            break;
                        case GSMIDI_REVERB_DELAY:
                        case GSMIDI_REVERB_PAN_DELAY:
                        default:
                            expl = "REVERB_MACRO";
                            LOG(99, "LOG: Unsupported GS sysex Reverb type:"
                                    " 0x%02x (%d)\n", type, type);
                            break;
                        }
                        break;
                    case GSMIDI_REVERB_PRE_LPF:
                    {
                        expl = "REVERB_PRE_LPF";
                        float val = value/7.0f;
                        float fc = 22000.0f - _log2lin(val*_lin2log(22000.0f));
                        midi.set_reverb_cutoff_frequency(fc);
                        break;
                    }
                    case GSMIDI_REVERB_LEVEL:
                    {
                        expl = "REVERB_LEVEL";
                        float val = float(value)/127.0f;
                        midi.set_reverb_level(track_no, val);
                        break;
                    }
                    case GSMIDI_REVERB_TIME:
                    {
                        expl = "REVERB_TIME";
                        float reverb_time = 0.7f*value/127.0f;
                        midi.set_reverb_time_rt60(reverb_time);
                        break;
                    }
                    case GSMIDI_REVERB_DELAY_FEEDBACK:
                        expl = "REVERB_DELAY_FEEDBACK";
                        LOG(99, "LOG: Unsupported GS sysex Reverb Delay Feedback\n");
                        break;
                    case GSMIDI_CHORUS_MACRO:
                        switch (value)
                        {
                        case GSMIDI_CHORUS1:
                            expl = "CHORUS1";
                            midi.set_chorus("chorus/chorus1");
                            INFO("Switching to GS type 1 chorus");
                            break;
                        case GSMIDI_CHORUS2:
                            expl = "CHORUS2";
                            midi.set_chorus("chorus/chorus2");
                            INFO("Switching to GS type 2 chorus");
                            break;
                        case GSMIDI_CHORUS3:
                            expl = "CHORUS3";
                            midi.set_chorus("chorus/chorus3");
                            INFO("Switching to GS type 3 chorus");
                            break;
                        case GSMIDI_CHORUS4:
                            expl = "CHORUS4";
                            midi.set_chorus("chorus/chorus4");
                            INFO("Switching to GS type 4 chorus");
                            break;
                        case GSMIDI_FEEDBACK_CHORUS:
                            expl = "FEEDBACK_CHORUS";
                            midi.set_chorus("chorus/chorus_freedback");
                            INFO("Switching to GS feedback chorus");
                            break;
                        case GSMIDI_FLANGER:
                            expl = "FLANGER";
                            midi.set_chorus("chorus/flanger");
                            INFO("Switching to GS flanging");
                            break;
                        case GSMIDI_DELAY1:
                            expl = "DELAY1";
                            midi.set_chorus("chorus/delay");
                            INFO("Switching to GS short delay");
                            break;
                        case GSMIDI_DELAY1_FEEDBACK:
                            expl = "DELAY1_FEEDBACK";
                            midi.set_chorus("chorus/delay_feedback");
                            INFO("Switching to GS short delay with feedback");
                            break;
                        default:
                            expl = "Unkown CHORUS_MACRO " + std::to_string(value);
                            LOG(99, "LOG: Unsupported GS sysex Chorus type:"
                                    " 0x%02x (%d)\n", type, type);
                            break;
                        }
                        break;
                    case GSMIDI_CHORUS_PRE_LPF:
                        expl = "CHORUS_PRE_LPF";
                        LOG(99, "LOG: Unsupported GS sysex Chorus pre-LPF\n");
                        break;
                    case GSMIDI_CHORUS_LEVEL:
                    {
                        expl = "CHORUS_LEVEL";
                        float val = float(value)/127.0f;
                        midi.set_chorus_level(track_no, val);
                        break;
                    }
                    case GSMIDI_CHORUS_FEEDBACK:
                        expl = "CHORUS_FEEDBACK";
                        midi.set_chorus_feedback(0.763f*value*1e-2f);
                        break;
                    case GSMIDI_CHORUS_DELAY:
                        expl = "CHORUS_DELAY";
                        LOG(99, "LOG: Unsupported GS sysex Chorus delay\n");
                        break;
                    case GSMIDI_CHORUS_RATE:
                    {
                        expl = "CHORUS_RATE";
                        float val = float(value)/127.0f;
                        midi.set_chorus_rate(val);
                        break;
                    }
                    case GSMIDI_CHORUS_DEPTH:
                    {
                        expl = "CHORUS_DEPTH";
                        float val = float(value)/127.0f;
                        midi.set_chorus_depth(val);
                        break;
                    }
                    case GSMIDI_CHORUS_SEND_LEVEL_TO_REVERB:
                        expl = "CHORUS_SEND_LEVEL_TO_REVERB";
                        midi.send_chorus_to_reverb(value/127.0f);
                        break;
                    case GSMIDI_DELAY_MACRO:
                        expl = "DELAY_MACRO";
                        LOG(99, "LOG: Unsupported GS sysex Delay Macro\n");
                        break;
                    case GSMIDI_DELAY_PRE_LPF:
                        expl = "DELAY_PRE_LPF";
                        LOG(99, "LOG: Unsupported GS sysex Delay Pre-LPF\n");
                        break;
                    case GSMIDI_DELAY_TIME_RATIO_LEFT:
                        expl = "DELAY_TIME_RATIO_LEFT";
                        LOG(99, "LOG: Unsupported GS sysex Delay Time Ratio left\n");
                        break;
                    case GSMIDI_DELAY_TIME_CENTER:
                        expl = "DELAY_TIME_CENTER";
                        LOG(99, "LOG: Unsupported GS sysex Delay Time Center\n");
                        break;
                    case GSMIDI_DELAY_TIME_RATIO_RIGHT:
                        expl = "DELAY_TIME_RATIO_RIGHT";
                        LOG(99, "LOG: Unsupported GS sysex Delay Time Ratio Right\n");
                        break;
                    case GSMIDI_DELAY_FEEDBACK:
                        expl = "DELAY_FEEDBACK";
                        LOG(99, "LOG: Unsupported GS sysex Delay Feedback\n");
                        break;
                    case GSMIDI_DELAY_LEVEL:
                        expl = "DELAY_LEVEL";
                        LOG(99, "LOG: Unsupported GS sysex Delay Level\n");
                        break;
                    case GSMIDI_DELAY_SEND_LEVEL_TO_REVERB:
                        expl = "DELAY_SEND_LEVEL_TO_REVERB";
                        LOG(99, "LOG: Unsupported GS sysex Delay Send Level To Reverb\n");
                        break;
                    case GSMIDI_DELAY_LEVEL_LEFT:
                        expl = "DELAY_LEVEL_LEFT";
                        LOG(99, "LOG: Unsupported GS sysex Delay Level Left\n");
                        break;
                    case GSMIDI_DELAY_LEVEL_CENTER:
                        expl = "DELAY_LEVEL_CENTER";
                        LOG(99, "LOG: Unsupported GS sysex Delay Level Center\n");
                        break;
                    case GSMIDI_DELAY_LEVEL_RIGHT:
                        expl = "DELAY_LEVEL_RIGHT";
                        LOG(99, "LOG: Unsupported GS sysex Delay Level Right\n");
                        break;
                    case GSMIDI_RYTHM_PART1:
                    case GSMIDI_RYTHM_PART2:
                    case GSMIDI_RYTHM_PART3:
                    case GSMIDI_RYTHM_PART4:
                    case GSMIDI_RYTHM_PART5:
                    case GSMIDI_RYTHM_PART6:
                    case GSMIDI_RYTHM_PART7:
                    case GSMIDI_RYTHM_PART8:
                    case GSMIDI_RYTHM_PART9:
                    case GSMIDI_RYTHM_PART10:
                    case GSMIDI_RYTHM_PART11:
                    case GSMIDI_RYTHM_PART12:
                    case GSMIDI_RYTHM_PART13:
                    case GSMIDI_RYTHM_PART14:
                    case GSMIDI_RYTHM_PART15:
                    case GSMIDI_RYTHM_PART16:
                    {
                        expl = "DRUM_PART";
                        uint8_t part_no = GS_Address2Part(addr_mid);
                        byte = pull_byte();
                        CSV(part_no, ", %d", byte);
                        if (GS_checksum(sum) == byte)
                        {
                            bool drums = value ? true : false;
                            midi.channel(part_no).set_drums(drums);

                            std::string name = midi.get_channel_type(part_no);
                            MESSAGE(3, "Set part %i to %s\n", part_no, name.c_str());
                            rv = true;
                        }
                        break;
                    }
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
                        expl = "VOICE_RESERVE_PART";
                        LOG(99, "LOG: Unsupported GS sysex Voice Reserve\n");
                        break;
                    case GSMIDI_TX_CHANNEL:
                        expl = "TX_CHANNEL";
                        LOG(99, "LOG: Unsupported GS sysex Tx Channel");
                        break;
                    case GSMIDI_RCV_CHANNEL:
                        expl = "RCV_CHANNEL";
                        LOG(99, "LOG: Unsupported GS sysex Rcv Channel\n");
                        break;
                    case GSMIDI_BREATH_CONTROL_NUMBER:
                        expl = "BREATH_CONTROL_NUMBER";
                        LOG(99, "LOG: Unsupported GS sysex Breath Control Number\n");
                        break;
                    case GSMIDI_BREATH_CONTROL_CURVE:
                        expl = "BREATH_CONTROL_CURVE";
                        LOG(99, "LOG: Unsupported GS sysex Breath Control Curve\n");
                        break;
                    case GSMIDI_BREATH_SET_LOCK:
                        expl = "BREATH_SET_LOCK";
                        LOG(99, "LOG: Unsupported GS sysex Breath Set Lock\n");
                        break;
                    case GSMIDI_BREATH_MODE:
                        expl = "BREATH_MODE";
                        LOG(99, "LOG: Unsupported GS sysex Breath Mode\n");
                        break;
                    case GSMIDI_VELOCITY_DEPTH:
                        expl = "VELOCITY_DEPTH";
                        LOG(99, "LOG: Unsupported GS sysex Velocity Depth\n");
                        break;
                    case GSMIDI_VELOCITY_OFFSET:
                        expl = "VELOCITY_OFFSET";
                        LOG(99, "LOG: Unsupported GS sysex Velocity Offset\n");
                        break;
                    default:
                    {
                        uint8_t part_no = addr_mid & 0xF;
                        switch (addr_mid)
                        {
                        case GSMIDI_EQUALIZER:
                            expl = "EQUALIZER";
                            if (!equalizer_enabled) break;
                            if (GS_mode != 1) {
                                GS_sysex_equalizer(part_no, addr_low, value);
                            }
                            break;
                        case GSMIDI_EFX_TYPE_EFFECT:
                        {
                            uint8_t lsb = pull_byte();
                            value = value << 8 | lsb;
                            CSV(channel_no, ", %d", lsb);
                            GS_sysex_insertion(part_no, addr_low, value, expl);
                            break;
                        }
                        default:
                            switch (addr_mid & 0xF0)
                            {
                            case GSMIDI_PART_SET:
                            case GSMIDI_PART_SWITCH:
                                GS_sysex_part(part_no, addr_low, value, expl);
                                expl = "PART_SET " + expl;
                                break;
                            case GSMIDI_MODULATION_SET:
                                GS_sysex_modulation(part_no, addr_low, value, expl);
                                expl = "MODULATION_SET " + expl;
                               break;
                            default:
                                expl = "Unkown PARAMETER_CHANGE " + std::to_string(addr_mid & 0xF);
                                LOG(99, "LOG: GS Data Set 1: Unsupported address:"
                                        " 0x%02x 0x%02x (%d %d)\n",
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
                    expl = "DISPLAY_DATA";
                    std::string text;
                    for (int i=offset()-offs; i<size; ++i) {
                        toUTF8(text, pull_byte());
                    }
                    MESSAGE(1, "Display: %s\n", text.c_str());
                    break;
                }
                case GSMIDI_SYSTEM_PARAMETER_CHANGE:
                    switch (addr)
                    {
                    case GSMIDI_SYSTEM_MODE_SET:
                        expl = "SYSTEM_MODE_SET";
                        GS_mode = value;
                        GS_initialize();
                        break;
                    default:
                        expl = "Unkown SYSTEM_PARAMETER_CHANGE " + std::to_string(addr);
                        LOG(99, "LOG: Unsupported GS sysex system parameter change\n");
                        break;
                    }
                    break;
                case GSMIDI_DISPLAY_BITMAP:
                    expl = "DISPLAY_BITMAP";
                    LOG(99, "LOG: Unsupported GS sysex Display Bitmap\n");
                    break;
                case GSMIDI_USER_INSTRUMENT:
                    expl = "USER_INSTRUMENT";
                    LOG(99, "LOG: Unsupported GS sysex User Instrument\n");
                    break;
                case GSMIDI_USER_DRUM_SET:
                    expl = "USER_DRUM_SET";
                    LOG(99, "LOG: Unsupported GS sysex User Drum Set\n");
                    break;
                case GSMIDI_USER_EFFECT:
                    expl = "USER_EFFECT";
                    LOG(99, "LOG: Unsupported GS sysex User Effect\n");
                    break;
                case GSMIDI_USER_PATCH:
                    expl = "USER_PATCH";
                    LOG(99, "LOG: Unsupported GS sysex User Patch\n");
                    break;
                case GSMIDI_USER_PART_PATCH:
                    expl = "USER_PART_PATCH";
                    LOG(99, "LOG: Unsupported GS sysex User Patch Part\n");
                    break;
                case GSMIDI_SYSTEM_INFORMATION:
                    expl = "SYSTEM_INFORMATION";
                    break;
                default:
                    expl = "Unkown DATA_SET1 " + std::to_string(addr_high);
                    LOG(99, "LOG: Unsupported GS sysex effect type: 0x%02x 0x%02x 0x%02x (%d %d %d)\n",
                            addr_high, addr_mid, addr_low,
                            addr_high, addr_mid, addr_low);
                   break;
                }
                break;
            } // GSMIDI_DATA_SET1
            case GSMIDI_DATA_REQUEST1:
                expl = "DATA_REQUEST1";
                LOG(99, "LOG: Unsupported GS sysex data Request\n");
                break;
            default:
                expl = "Unkown MODEL_GS " + std::to_string(byte);
                LOG(99, "LOG: Unsupported GS sysex parameter category: 0x%02x (%d)\n",
                     byte, byte);
                break;
            }
            break;
        } // GSMIDI_MODEL_GS
        default:
            expl = "SYSTEM";
            LOG(99, "LOG: Unsupported GS sysex model ID: 0x%02x (%d)\n",
                     byte, byte);
            break;
        }
        break;
    } // GSMIDI_SYSTEM
    default:
        expl = "Unkown SYSEX " + std::to_string(type & 0xF);
        LOG(99, "LOG: Unsupported GS sysex category type: 0x%02x (%d)\n", byte, byte);
        break;
    }

    return rv;
};

bool
MIDIStream::GS_sysex_equalizer(uint8_t part_no, uint8_t addr, uint8_t value)
{
    bool rv = true;
    switch(addr)
    {
    case GSMIDI_PART_EQUALIZER_SWITCH:
    default:
        LOG(99, "LOG: Unsupported GS sysex equalizer set: 0x%02x (%d)\n",
                     addr, addr);
        rv = false;
        break;
    }
    return rv;
}

// SC-8850_OM page 78, 89, 91 (type), 216 (effect list)
bool
MIDIStream::GS_sysex_insertion(uint8_t part_no, uint8_t addr, uint16_t type, std::string& expl)
{
    bool rv = true;
    switch(addr)
    {
    case GSMIDI_EFX_TYPE:
        switch(type)
        {
        case GSMIDI_EFX_TYPE_THRU:
            expl = "Unsupported THRU";
            break;
        case GSMIDI_EFX_TYPE_STEREO_EQ:
            expl = "Unsupported STEREO_EQ";
            break;
        case GSMIDI_EFX_TYPE_SPECTRUM:
            expl = "Unsupported SPECTRUM";
            break;
        case GSMIDI_EFX_TYPE_ENHANCER:
            expl = "Unsupported ENHANCER";
            break;
        case GSMIDI_EFX_TYPE_HUMANIZER:
            expl = "Unsupported HUMANIZER";
            break;
        case GSMIDI_EFX_TYPE_OVERDRIVE:
            expl = "Unsupported OVERDRIVE";
            break;
        case GSMIDI_EFX_TYPE_DISTORTION:
            expl = "Unsupported DISTORTION";
            break;
        case GSMIDI_EFX_TYPE_PHASER:
            expl = "Unsupported PHASER";
            break;
        case GSMIDI_EFX_TYPE_AUTO_WAH:
            expl = "Unsupported AUTO_WAH";
            break;
        case GSMIDI_EFX_TYPE_ROTARY:
            expl = "Unsupported ROTARY";
            break;
        case GSMIDI_EFX_TYPE_STEREO_FLANGER:
            expl = "Unsupported STEREO_FLANGER";
            break;
        case GSMIDI_EFX_TYPE_STEP_FLANGER:
            expl = "Unsupported STEP_FLANGER";
            break;
        case GSMIDI_EFX_TYPE_TREMOLO:
            expl = "Unsupported TREMOLO";
            break;
        case GSMIDI_EFX_TYPE_AUTO_PAN:
            expl = "Unsupported AUTO_PAN";
            break;
        case GSMIDI_EFX_TYPE_COMPRESSOR:
            expl = "Unsupported COMPRESSOR";
            break;
        case GSMIDI_EFX_TYPE_LIMITER:
            expl = "Unsupported LIMITER";
            break;
        case GSMIDI_EFX_TYPE_HEXA_CHORUS:
            expl = "Unsupported HEXA_CHORUS";
            break;
        case GSMIDI_EFX_TYPE_TREMOLO_CHORUS:
            expl = "Unsupported TREMOLO_CHORUS";
            break;
        case GSMIDI_EFX_TYPE_STEREO_CHORUS:
            expl = "Unsupported STEREO_CHORUS";
            break;
        case GSMIDI_EFX_TYPE_SPACE_D:
            expl = "Unsupported SPACE_D";
            break;
        case GSMIDI_EFX_TYPE_3D_CHORUS:
            expl = "Unsupported 3D_CHORUS";
            break;
        case GSMIDI_EFX_TYPE_STEREO_DELAY:
            expl = "Unsupported STEREO_DELAY";
            break;
        case GSMIDI_EFX_TYPE_MOD_DELAY:
            expl = "Unsupported MOD_DELAY";
            break;
        case GSMIDI_EFX_TYPE_3TAP_DELAY:
            expl = "Unsupported 3TAP_DELAY";
            break;
        case GSMIDI_EFX_TYPE_4TAP_DELAY:
            expl = "Unsupported 4TAP_DELAY";
            break;
        case GSMIDI_EFX_TYPE_TIME_CONTROL_DELAY:
            expl = "Unsupported TIME_CONTROL_DELAY";
            break;
        case GSMIDI_EFX_TYPE_REVERB:
            expl = "Unsupported REVERB";
            break;
        case GSMIDI_EFX_TYPE_GATE_REVERB:
            expl = "Unsupported GATE_REVERB";
            break;
        case GSMIDI_EFX_TYPE_3D_DELAY:
            expl = "Unsupported 3D_DELAY";
            break;
        case GSMIDI_EFX_TYPE_2PITCH_SHIFTER:
            expl = "Unsupported 2PITCH_SHIFTER";
            break;
        case GSMIDI_EFX_TYPE_FEEDBACK_PITCH_SHIFTER:
            expl = "Unsupported FEEDBACK_PITCH_SHIFTER";
            break;
        case GSMIDI_EFX_TYPE_3D_AUTO:
            expl = "Unsupported 3D_AUTO";
            break;
        case GSMIDI_EFX_TYPE_3D_MANUAL:
            expl = "Unsupported 3D_MANUAL";
            break;
        case GSMIDI_EFX_TYPE_LOFI1:
            expl = "Unsupported LOFI1";
            break;
        case GSMIDI_EFX_TYPE_LOFI2:
            expl = "Unsupported LOFI2";
            break;
        case GSMIDI_EFX_TYPE_OVERDRIVE_TO_CHORUS:
            expl = "Unsupported OVERDRIVE_TO_CHORUS";
            break;
        case GSMIDI_EFX_TYPE_OVERDRIVE_TO_FLANGER:
            expl = "Unsupported OVERDRIVE_TO_FLANGER";
            break;
        case GSMIDI_EFX_TYPE_OVERDRIVE_TO_DELAY:
            expl = "Unsupported OVERDRIVE_TO_DELAY";
            break;
        case GSMIDI_EFX_TYPE_DISTORTION_TO_CHORUS:
            expl = "Unsupported DISTORTION_TO_CHORUS";
            break;
        case GSMIDI_EFX_TYPE_DISTORTION_TO_FLANGER:
            expl = "Unsupported DISTORTION_TO_FLANGER";
            break;
        case GSMIDI_EFX_TYPE_DISTORTION_TO_DELAY:
            expl = "Unsupported DISTORTION_TO_DELAY";
            break;
        case GSMIDI_EFX_TYPE_ENHANCER_TO_CHORUS:
            expl = "Unsupported ENHANCER_TO_CHORUS";
            break;
        case GSMIDI_EFX_TYPE_ENHANCER_TO_FLANGER:
            expl = "Unsupported ENHANCER_TO_FLANGER";
            break;
        case GSMIDI_EFX_TYPE_ENHANCER_TO_DELAY:
            expl = "Unsupported ENHANCER_TO_DELAY";
            break;
        case GSMIDI_EFX_TYPE_CHORUS_TO_DELAY:
            expl = "Unsupported CHORUS_TO_DELAY";
            break;
        case GSMIDI_EFX_TYPE_FLANGER_TO_DELAY:
            expl = "Unsupported FLANGER_TO_DELAY";
            break;
        case GSMIDI_EFX_TYPE_CHORUS_TO_FLANGER:
            expl = "Unsupported CHORUS_TO_FLANGER";
            break;
        case GSMIDI_EFX_TYPE_ROTARY_MULTI:
            expl = "Unsupported ROTARY_MULTI";
            break;
        case GSMIDI_EFX_TYPE_GUITAR_MULTI1:
            expl = "Unsupported GUITAR_MULTI1";
            break;
        case GSMIDI_EFX_TYPE_GUITAR_MILTI2:
            expl = "Unsupported GUITAR_MILTI2";
            break;
        case GSMIDI_EFX_TYPE_GUITAR_MULTI3:
            expl = "Unsupported GUITAR_MULTI3";
            break;
        case GSMIDI_EFX_TYPE_CLEAN_GUITAR_MULTI1:
            expl = "Unsupported CLEAN_GUITAR_MULTI1";
            break;
        case GSMIDI_EFX_TYPE_CLEAN_GUITAR_MULTI2:
            expl = "Unsupported CLEAN_GUITAR_MULTI2";
            break;
        case GSMIDI_EFX_TYPE_BASS_MULTI:
            expl = "Unsupported BASS_MULTI";
            break;
        case GSMIDI_EFX_TYPE_RHODES_MULTI:
            expl = "Unsupported RHODES_MULTI";
            break;
        case GSMIDI_EFX_TYPE_KEYBOARD_MULTI:
            expl = "Unsupported KEYBOARD_MULTI";
            break;
        case GSMIDI_EFX_TYPE_CHORUS_DELAY:
            expl = "Unsupported CHORUS_DELAY";
            break;
        case GSMIDI_EFX_TYPE_FLANGER_DELAY:
            expl = "Unsupported FLANGER_DELAY";
            break;
        case GSMIDI_EFX_TYPE_CHORUS_FLANGER:
            expl = "Unsupported CHORUS_FLANGER";
            break;
        case GSMIDI_EFX_TYPE_OVERDRIVE_DISTORTION12:
            expl = "Unsupported OVERDRIVE_DISTORTION12";
            break;
        case GSMIDI_EFX_TYPE_OVERDRIVE_DISTORTION_ROTARY:
            expl = "Unsupported OVERDRIVE_DISTORTION_ROTARY";
            break;
        case GSMIDI_EFX_TYPE_OVERDRIVE_DISTORTION_PHASER:
            expl = "Unsupported OVERDRIVE_DISTORTION_PHASER";
            break;
        case GSMIDI_EFX_TYPE_OVERDRIVE_DISTORTION_AUTO_WAH:
            expl = "Unsupported OVERDRIVE_DISTORTION_AUTO_WAH";
            break;
        case GSMIDI_EFX_TYPE_PHASER_ROTARY:
            expl = "Unsupported PHASER_ROTARY";
            break;
        case GSMIDI_EFX_TYPE_PHASER_AUTO_WAH:
            expl = "Unsupported PHASER_AUTO_WAH";
            break;
        default:
            expl = "Unknwon INSERTION_EFFECT";
            break;
        }
        LOG(99, "LOG %s\n", expl.c_str());
        break;
    case GSMIDI_EFX_PARAMETER1:
    case GSMIDI_EFX_PARAMETER2:
    case GSMIDI_EFX_PARAMETER3:
    case GSMIDI_EFX_PARAMETER4:
    case GSMIDI_EFX_PARAMETER5:
    case GSMIDI_EFX_PARAMETER6:
    case GSMIDI_EFX_PARAMETER7:
    case GSMIDI_EFX_PARAMETER8:
    case GSMIDI_EFX_PARAMETER9:
    case GSMIDI_EFX_PARAMETER10:
    case GSMIDI_EFX_PARAMETER11:
    case GSMIDI_EFX_PARAMETER12:
    case GSMIDI_EFX_PARAMETER13:
    case GSMIDI_EFX_PARAMETER14:
    case GSMIDI_EFX_PARAMETER15:
    case GSMIDI_EFX_PARAMETER16:
    case GSMIDI_EFX_PARAMETER17:
    case GSMIDI_EFX_PARAMETER18:
    case GSMIDI_EFX_PARAMETER19:
    case GSMIDI_EFX_PARAMETER20:
        expl = "EFX_PARAMETER" + std::to_string((addr - GSMIDI_EFX_PARAMETER1 + 1));
        break;
    case GSMIDI_EFX_SEND_LEVEL_TO_REVERB:
        expl = "GSMIDI_EFX_SEND_LEVEL_TO_REVERB";
        break;
    case GSMIDI_EFX_SEND_LEVEL_TO_CHORUS:
        expl = "GSMIDI_EFX_SEND_LEVEL_TO_CHORUS";
        break;
    case GSMIDI_EFX_SEND_LEVEL_TO_DELAY:
        expl = "GSMIDI_EFX_SEND_LEVEL_TO_DELAY";
        break;
    case GSMIDI_EFX_CONTROL_SOURCE1:
        expl = "GSMIDI_EFX_CONTROL_SOURCE1";
        break;
    case GSMIDI_EFX_CONTROL_DEPTH1:
        expl = "GSMIDI_EFX_CONTROL_DEPTH1";
        break;
    case GSMIDI_EFX_CONTROL_SOURCE2:
        expl = "GSMIDI_EFX_CONTROL_SOURCE2";
        break;
    case GSMIDI_EFX_CONTROL_DEPTH2:
        expl = "GSMIDI_EFX_CONTROL_DEPTH2";
        break;
    case GSMIDI_EFX_SEND_EQ_TYPE:
        expl = "GSMIDI_EFX_SEND_EQ_TYPE";
        break;
    default:
        LOG(99, "LOG: Unsupported GS sysex insertion type: 0x%02x (%d)\n",
                 addr, addr);
        break;
    }
    return rv;
}

bool
MIDIStream::GS_sysex_modulation(uint8_t part_no, uint8_t addr, uint8_t value, std::string& expl)
{
    bool rv = true;
    switch(addr)
    {
    case GSMIDI_MODULATION_DEPTH:
        expl = "Unsupported MODULATION_DEPTH";
        LOG(99, "LOG: Unsupported GS sysex modulation depth\n");
        break;
    case GSMIDI_BEND_RANGE:
        expl = "Unsupported BEND_RANGE";
        LOG(99, "LOG: Unsupported GS sysex bend range\n");
        break;
    default:
        expl = "Unkown";
        LOG(99, "LOG: Unsupported GS sysex modulation type: 0x%02x (%d)\n",
                 addr, addr);
        rv = false;
        break;
    }
    return rv;
}

bool
MIDIStream::GS_sysex_part(uint8_t part_no, uint8_t addr, uint8_t value, std::string &expl)
{
    auto& channel = midi.channel(part_no);
    bool rv = true;
    switch(addr)
    {
    case GSMIDI_PART_TONE_NUMBER: // CC#00 VALUE 0 – 127
    {
        expl = "TONE_NUMBER";
#if 0
        uint8_t program_no = pull_byte(); // P.C. VALUE 1 – 128
        process_control(value);
        try {
            midi.new_channel(channel_no, bank_no, program_no);
            if (midi.is_drums(channel_no))
            {
                auto frames = midi.get_frames();
                auto it = frames.find(program_no);
                if (it != frames.end()) {
                    name = it->second.name;
                }
            }
            else
            {
                auto inst = midi.get_instrument(bank_no, program_no);
                name = inst.first.name;
            }
        } catch(const std::invalid_argument& e) {
            ERROR("Error: " << e.what());
        }
#endif
        break;
    }
    case GSMIDI_PART_RX_CHANNEL: // 1 – 16, 0 = OFF
        expl = "Unsupported RC CHANNEL";
        LOG(99, "LOG: Unsupported GS sysex part channel\n");
        break;
    case GSMIDI_PART_PITCH_BEND_SWITCH:
        expl = "PITCH_BEND_SWITCH";
#if AAX_PATCH_LEVEL > 210112
        channel.set_pitch_slide_state(value);
#endif
        break;
    case GSMIDI_PART_CHANNEL_PRESSURE_SWITCH:
        expl = "Unsupported CHANNEL_PRESSURE_SWITCH";
        break;
    case GSMIDI_PART_PROGRAM_CHANGE_SWITCH:
        expl = "PROGRAM_CHANGE";
        program_no = value;
        try {
            midi.new_channel(part_no, bank_no, program_no);
        } catch(const std::invalid_argument& e) {
            ERROR("Error: " << e.what());
        }
        break;
    case GSMIDI_PART_CONTROL_CHANGE_SWITCH:
        expl = "Unsupported CONTROL_CHANGE_SWITCH";
        break;
    case GSMIDI_PART_POLY_PRESSURE_SWITCH:
        expl = "Unsupported POLY_PRESSURE_SWITCH";
        break;
    case GSMIDI_PART_NOTE_MESSAGE_SWITCH:
        expl = "Unsupported NOTE_MESSAGE_SWITCH";
        break;
    case GSMIDI_PART_RPN_SWITCH:
        expl = "RPN_SWITCH";
        rpn_enabled = value;
        break;
    case GSMIDI_PART_NRPN_SWITCH:
        expl = "Unsupported NRPN_SWITCH";
        break;
    case GSMIDI_PART_MODULATION_SWITCH:
        expl = "Unsupported MODULATION_SWITCH";
        break;
    case GSMIDI_PART_VOLUME_SWITCH:
        expl = "Unsupported VOLUME_SWITCH";
        break;
    case GSMIDI_PART_PAN_SWITCH:
        expl = "Unsupported PAN_SWITCH";
        break;
    case GSMIDI_PART_EXPRESSION_SWITCH:
        expl = "Unsupported EXPRESSION_SWITCH";
        break;
    case GSMIDI_PART_HOLD1_SWITCH:
        expl = "HOLD1_SWITCH";
        channel.set_hold(value);
        break;
    case GSMIDI_PART_PORTAMENTO_SWITCH:
        expl = "PORTAMENTO_SWITCH";
#if AAX_PATCH_LEVEL > 210112
        channel.set_pitch_slide_state(value >= 0x40);
#endif
        break;
    case GSMIDI_PART_SOSTENUTO_SWITCH:
        expl = "SOSTENUTO_SWITCH";
        channel.set_sustain(value);
        break;
    case GSMIDI_PART_SOFT_SWITCH:
        expl = "Unsupported SOFT_SWITCH";
        break;
    case GSMIDI_PART_POLY_MODE:
        expl = "POLY_MODE";
        midi.process(part_no, MIDI_NOTE_OFF, 0, 0, true);
        if (value == 0) {
            mode = MIDI_MONOPHONIC;
            channel.set_monophonic(true);
        } else {
            channel.set_monophonic(false);
            mode = MIDI_POLYPHONIC;
        }
        break;
    case GSMIDI_PART_ASSIGN_MODE:
        expl = "Unsupported ASSIGN_MODE";
        break;
    case GSMIDI_PART_RYTHM_MODE:
        expl = "Unsupported RYTHM_MODE";
        break;
    case GSMIDI_PART_PITCH_KEY_SHIFT:
        expl = "Unsupported PITCH_KEY_SHIFT";
        break;
    case GSMIDI_PART_PITCH_OFFSET_FINE:
        expl = "Unsupported PITCH_OFFSET_FINE";
        break;
    case GSMIDI_PART_VOLUME:
        expl = "VOLUME";
        channel.set_gain(_ln(float(value)/127.0f));
        break;
    case GSMIDI_PART_VELOCITY_SENSE_DEPTH:
        expl = "Unsupported VELOCITY_SENSE_DEPTH";
        break;
    case GSMIDI_PART_VELOCITY_SENSE_OFFSET:
        expl = "Unsupported VELOCITY_SENSE_OFFSET";
        break;
    case GSMIDI_PART_PAN:
        expl = "PAN";
        if (mode != MIDI_MONOPHONIC) {
            channel.set_pan((float(value)-64.f)/64.f);
        }
        break;

    case GSMIDI_PART_KEYBOARD_RANGE_LOW:
        expl = "Unsupported KEYBOARD_RANGE_LOW";
        break;
    case GSMIDI_PART_KEYBOARD_RANGE_HIGH:
        expl = "Unsupported KEYBOARD_RANGE_HIGH";
        break;
    case GSMIDI_PART_CC1_CONTROL_NUMBER:
        expl = "Unsupported CC1_CONTROL_NUMBER";
        break;
    case GSMIDI_PART_CC2_CONTROL_NUMBER:
        expl = "Unsupported CC2_CONTROL_NUMBER";
        break;
    case GSMIDI_PART_CHORUS_SEND_LEVEL:
        expl = "Unsupported CHORUS_SEND_LEVEL";
        break;
    case GSMIDI_PART_REVERB_SEND_LEVEL:
    {
        expl = "REVERB_SEND_LEVEL";
        float val = float(value)/127.0f;
        midi.set_reverb_level(part_no, val);
        break;
    }
    case GSMIDI_PART_BANK_SELECT_SWITCH:
        expl = "BANK_SELECT_SWITCH";
        bank_select_enabled = value;
        break;
    case GSMIDI_PART_BANK_SELECT_LSB_SWITCH:
        expl = "BANK_SELECT_LSB_SWITCH";
        bank_select_lsb_enabled = value;
        break;
    case GSMIDI_PART_PITCH_FINE_TUNE:
        expl = "Unsupported PITCH_FINE_TUNE";
        break;
    case GSMIDI_PART_DELAY_SEND_LEVEL:
        expl = "Unsupported DELAY_SEND_LEVEL";
        break;
    case GSMIDI_PART_VIBRATO_RATE:
    {
        expl = "VIBRATO_RATE";
        float val = 0.5f + float(value)/64.0f;
        channel.set_vibrato_rate(val);
        break;
    }
    case GSMIDI_PART_VIBRATO_DEPTH:
    {
        expl = "VIBRATO_DEPTH";
        float val = float(value)/64.0f;
        channel.set_vibrato_depth(val);
        break;
    }
    case GSMIDI_PART_CUTOFF_FREQUENCY:
        expl = "Unsupported CUTOFF_FREQUENCY";
        break;
    case GSMIDI_PART_RESONANCE:
        expl = "Unsupported RESONANCE";
        break;
    case GSMIDI_PART_ATTACK_TIME:
        expl = "ATTACK_TIME";
        channel.set_attack_time(value);
        break;
    case GSMIDI_PART_DECAY_TIME:
        expl = "DECAY_TIME";
        channel.set_decay_time(value);
        break;
    case GSMIDI_PART_RELEASE_TIME:
        expl = "RELEASE_TIME";
        channel.set_release_time(value);
        break;
    case GSMIDI_PART_VIBRATO_DELAY:
    {
        expl = "VIBRATO_DELAY";
        float val = float(value)/64.0f;
        channel.set_vibrato_delay(val);
        break;
    }
    default:
        expl = "Unknown " + std::to_string(addr);
        LOG(99, "LOG: Unsupported GS sysex part set: 0x%02x (%d)\n", addr, addr);
        rv = false;
        break;
    }
    expl += ": " + std::to_string(value);
    return rv;
}
