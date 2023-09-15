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

using namespace aax;

std::string MIDIStream::GM_initialize(uint8_t mode)
{
    std::string expl;

    midi.set_mode(mode);
    switch(mode)
    {
    case GMMIDI_GM_RESET:
        expl = "GM RESET";
        midi.set_reverb_type(4);
        midi.set_chorus_type(2);
        for (auto& it : midi.get_channels())
        {
            midi.process(it.first, MIDI_NOTE_OFF, 0, 0, true);
            midi.set_reverb_level(it.first, 0.0f);
            midi.set_chorus_level(it.first, 0.0f);
            it.second->set_expression(midi.ln(127.0f/127.0f));
            it.second->set_gain(midi.ln(100.0f/127.0f));
            it.second->set_pan(0.0f);
            if (it.first != MIDI_DRUMS_CHANNEL) {
                it.second->set_drums(false);
            } else it.second->set_drums(true);
        }
        midi.set_mode(MIDI_GENERAL_MIDI1);
        break;
    case 0x02:
        // midi.set_mode(MIDI_MODE0);
        break;
    case GMMIDI_GM2_RESET:
        expl = "GM2 RESET";
        midi.set_reverb("GM2/concerthall-large");
        INFO("Switching to Large Concert Hall reveberation");

        midi.set_chorus("GM2/chorus3");
        INFO("Switching to Chorus3");
        for (auto& it : midi.get_channels())
        {
            midi.process(it.first, MIDI_NOTE_OFF, 0, 0, true);

            midi.set_reverb_level(it.first, 0.0f);
            midi.set_chorus_level(it.first, 0.0f);
            it.second->set_expression(midi.ln(127.0f/127.0f));
            it.second->set_gain(midi.ln(100.0f/127.0f));
            it.second->set_pan(0.0f);
            if (it.first != MIDI_DRUMS_CHANNEL) {
                it.second->set_drums(false);
            } else it.second->set_drums(true);
        }
        midi.set_mode(MIDI_GENERAL_MIDI2);
        break;
    default:
        expl = "Unkown SYSTEM";
        break;
    }
    return expl;
}

bool MIDIStream::GM_process_sysex_non_realtime(uint64_t size, std::string& expl)
{
    bool rv = true;
    uint64_t offs = offset();
    uint8_t type, devno;
    uint8_t value;

#if 0
 printf(" System Exclusive:");
 push_value(); push_value(); push_value();
 while ((value = pull_byte()) != MIDI_SYSTEM_EXCLUSIVE_END) printf(" %x", value);
 printf("\n");
 value_stream::rewind( offset() - offs);
#endif

    // GM1 reset: F0 7E 7F 09 01 F7
    // GM2 reset: F0 7E 7F 09 03 F7
    value = pull_byte();
    CSV(channel_no, ", %d", value);
    if (value == GMMIDI_BROADCAST)
    {
        value = pull_byte();
        CSV(channel_no, ", %d", value);
        switch(value)
        {
        case GENERAL_MIDI_SYSTEM:
            value = pull_byte();
            CSV(channel_no, ", %d", value);
            expl = GM_initialize(value);
            break;
        case MIDI_EOF:
        case MIDI_WAIT:
        case MIDI_CANCEL:
        case MIDI_NAK:
        case MIDI_ACK:
            break;
        default:
            expl = "Unkown BROADCAST";
            break;
        }
    }
    else expl = "Unkown SYSEX NR";

    return rv;
}

bool MIDIStream::GM_process_sysex_realtime(uint64_t size, std::string& expl)
{
    bool rv = true;
    uint64_t offs = offset();
    uint8_t type, devno;
    int32_t value;

#if 0
 printf(" System Exclusive:");
 push_value(); push_value(); push_value();
 while ((value = pull_byte()) != MIDI_SYSTEM_EXCLUSIVE_END) printf(" %x", value);
 printf("\n");
 value_stream::rewind( offset() - offs);
#endif

    value = pull_byte();
    CSV(channel_no, ", %d", value);
    switch(value)
    {
    case MIDI_BROADCAST:
    {
        value = pull_byte();
        CSV(channel_no, ", %d", value);
        switch(value)
        {
        case MIDI_DEVICE_CONTROL:
        {
            value = pull_byte();
            CSV(channel_no, ", %d", value);
            switch(value)
            {
            case MIDI_MASTER_VOLUME:
            {
                expl = "DEVICE_VOLUME";
                float v;
                value = pull_byte();
                CSV(channel_no, ", %d", value);
                v = float(value);
                value = pull_byte();
                CSV(channel_no, ", %d", value);
                v += float(value << 7);
                v /= (127.0f*127.0f); // 14-bit volume value
                midi.set_volume(v);
                break;
            }
            case MIDI_MASTER_BALANCE:
                expl = "BALANCE";
                value = pull_byte();
                CSV(channel_no, ", %d", value);
                midi.set_balance(float(value-64)/64.0f);
                break;
            case MIDI_MASTER_FINE_TUNING:
            {
                expl = "FINE_TUNING";
                uint16_t tuning;
                float pitch;

                value = pull_byte();
                CSV(channel_no, ", %d", value);
                tuning = value;

                value = pull_byte();
                CSV(channel_no, ", %d", value);
                tuning |= value << 7;

                pitch = float(tuning-8192);
                if (pitch < 0) pitch /= 8192.0f;
                else pitch /= 8191.0f;
                midi.set_tuning(pitch);
                break;
            }
            case MIDI_MASTER_COARSE_TUNING:
            {
                expl = "COARSE_TUNING";
                float pitch;

                value = pull_byte();     // lsb, always zero
                CSV(channel_no, ", %d", value);

                value = pull_byte();     // msb
                CSV(channel_no, ", %d", value);

                pitch = float(value-64);
                if (pitch < 0) pitch /= 64.0f;
                else pitch /= 63.0f;
                midi.set_tuning(pitch);
                break;
            }
            case MIDI_GLOBAL_PARAMETER_CONTROL:
            {
                uint8_t path_len, id_width, val_width;
                uint8_t param, value;
                uint16_t slot_path;

                path_len = pull_byte();
                CSV(channel_no, ", %d", path_len);

                id_width = pull_byte();
                CSV(channel_no, ", %d", id_width);

                val_width = pull_byte();
                CSV(channel_no, ", %d", val_width);

                slot_path = pull_byte();
                CSV(channel_no, ", %d", slot_path);

                value = pull_byte();
                slot_path |= value << 7;
                CSV(channel_no, ", %d", value);

                param =  pull_byte();
                CSV(channel_no, ", %d", param);

                value = pull_byte();
                CSV(channel_no, ", %d", value);

                switch(slot_path)
                {
                case MIDI_CHORUS_PARAMETER:
                    switch(param)
                    {
                    case 0:     // CHORUS_TYPE
                        expl = "CHORUS_TYPE";
                        midi.set_chorus_type(value);
                        break;
                    case 1:     // CHORUS_MOD_RATE
                        expl = "CHORUS_MOD_RATE";
                        // the modulation frequency in Hz
                        midi.set_chorus_rate(0.122f*value);
                        break;
                    case 2:     // CHORUS_MOD_DEPTH
                        expl = "CHORUS_MOD_DEPTH";
                        // the peak-to-peak swing of the modulation in ms
                        midi.set_chorus_depth(((value+1.0f)/3.2f)*1e-3f);
                        break;
                    case 3:     // CHORUS_FEEDBACK
                        expl = "CHORUS_FEEDBACK";
                        // the amount of feedback from Chorus output in %
                        midi.set_chorus_feedback(0.763f*value*1e-2f);
                        break;
                    case 4:     // CHORUS_SEND_TO_REVERB
                        expl = "CHORUS_SEND_TO_REVERB";
                        // the send level from Chorus to Reverb in %
                        midi.send_chorus_to_reverb(0.787f*value*1e-2f);
                        break;
                    default:
                        expl = "Unkown CHORUS_PARAMETER";
                        LOG(99, "LOG: Unsupported realtime sysex chorus parameter: %x\n",
                                 param);
                        break;
                    }
                    break;
                case MIDI_REVERB_PARAMETER:
                    switch(param)
                    {
                    case 0:     // Reverb Type
                        expl = "REVERB_TYPE";
                        midi.set_reverb_type(value);
                        break;
                    case 1:     //Reverb Time
                        expl = "REVERB_TIME";
                        midi.set_reverb_time_rt60(expf((value-40)*0.025f));
                        break;
                    default:
                        LOG(99, "LOG: Unsupported realtime sysex reverb parameter: %x\n",
                                param);
                        break;
                    }
                    break;
                default:
                    expl = "Unkown GLOBAL_PARAMETER";
                    break;
                }
                break;
            }
            default:
               expl = "Unkown DEVICE_CONTROL";
                LOG(99, "LOG: Unsupported realtime sysex parameter: %x\n", value);
                break;
            }
            break;
        } // MIDI_DEVICE_CONTROL
        case MIDI_SCALE_ADJUST:
            expl = "SCALE_ADJUST";
            LOG(99, "LOG: Unsupported realtime sysex scale adjust\n");
            break;
        case MIDI_CONTROLLER_DESTINATION:
        case MIDI_KEY_BASED_INSTRUMENT:
        default:
            expl = "Unkown BROADCAST";
            value <= 8;
            value += pull_byte();
            CSV(channel_no, ", %d", value & 0xff);
            LOG(99, "LOG: Unsupported realtime sysex sub id: %x %x (%d %d)\n",
                     value >> 8, value & 0xf, value >> 8, value & 0xf);
            break;
        }
        break;
    } // MIDI_BROADCAST
    default:
        expl = "Unkown SYSEX";
        LOG(99, "LOG: Unknown realtime sysex device id: %x\n", value);
        break;
    }

    return rv;
}

