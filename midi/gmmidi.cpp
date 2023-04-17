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

bool MIDIStream::GM_process_sysex_non_realtime(uint64_t size)
{
    bool rv = true;
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

    // GM1 reset: F0 7E 7F 09 01 F7
    // GM2 reset: F0 7E 7F 09 03 F7
    byte = pull_byte();
    CSV(channel_no, ", %d", byte);
    if (byte == GMMIDI_BROADCAST)
    {
        byte = pull_byte();
        CSV(channel_no, ", %d", byte);
        switch(byte)
        {
        case GENERAL_MIDI_SYSTEM:
            byte = pull_byte();
            CSV(channel_no, ", %d", byte);
            midi.set_mode(byte);
            switch(byte)
            {
            case GMMIDI_GM_RESET:
                midi.process(channel_no, MIDI_NOTE_OFF, 0, 0, true);
                midi.set_mode(MIDI_GENERAL_MIDI1);
                break;
            case 0x02:
                // midi.set_mode(MIDI_MODE0);
                break;
            case GMMIDI_GM2_RESET:
                midi.process(channel_no, MIDI_NOTE_OFF, 0, 0, true);
                midi.set_mode(MIDI_GENERAL_MIDI2);
                break;
            default:
                break;
            }
            break;
        case MIDI_EOF:
        case MIDI_WAIT:
        case MIDI_CANCEL:
        case MIDI_NAK:
        case MIDI_ACK:
            break;
        default:
            break;
        }
    }

    return rv;
}

bool MIDIStream::GM_process_sysex_realtime(uint64_t size)
{
    bool rv = true;
    uint64_t offs = offset();
    uint8_t type, devno;
    uint16_t byte;

#if 0
 printf(" System Exclusive:");
 push_byte(); push_byte(); push_byte();
 while ((byte = pull_byte()) != MIDI_SYSTEM_EXCLUSIVE_END) printf(" %x", byte);
 printf("\n");
 byte_stream::rewind( offset() - offs);
#endif

    byte = pull_byte();
    CSV(channel_no, ", %d", byte);
    switch(byte)
    {
    case MIDI_BROADCAST:
    {
        byte = pull_byte();
        CSV(channel_no, ", %d", byte);
        switch(byte)
        {
        case MIDI_DEVICE_CONTROL:
        {
            byte = pull_byte();
            CSV(channel_no, ", %d", byte);
            switch(byte)
            {
            case MIDI_DEVICE_VOLUME:
            {
                float v;
                byte = pull_byte();
                CSV(channel_no, ", %d", byte);
                v = (float)byte;
                byte = pull_byte();
                CSV(channel_no, ", %d", byte);
                v += (float)((uint16_t)(byte << 7));
                v /= (127.0f*127.0f);
                midi.set_gain(v);
                break;
            }
            case MIDI_DEVICE_BALANCE:
                byte = pull_byte();
                CSV(channel_no, ", %d", byte);
                midi.set_balance(((float)byte-64.0f)/64.0f);
                break;
            case MIDI_DEVICE_FINE_TUNING:
            {
                uint16_t tuning;
                float pitch;

                byte = pull_byte();
                CSV(channel_no, ", %d", byte);
                tuning = byte;

                byte = pull_byte();
                CSV(channel_no, ", %d", byte);
                tuning |= byte << 7;

                pitch = (float)tuning-8192.0f;
                if (pitch < 0) pitch /= 8192.0f;
                else pitch /= 8191.0f;
                midi.set_tuning(pitch);
                break;
            }
            case MIDI_DEVICE_COARSE_TUNING:
            {
                float pitch;

                byte = pull_byte();     // lsb, always zero
                CSV(channel_no, ", %d", byte);

                byte = pull_byte();     // msb
                CSV(channel_no, ", %d", byte);

                pitch = (float)byte-64.0f;
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

                byte = pull_byte();
                slot_path |= byte << 7;
                CSV(channel_no, ", %d", byte);

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
                        midi.set_chorus_type(value);
                        break;
                    case 1:     // CHORUS_MOD_RATE
                        // the modulation frequency in Hz
                        midi.set_chorus_rate(0.122f*value);
                        break;
                    case 2:     // CHORUS_MOD_DEPTH
                        // the peak-to-peak swing of the modulation in ms
                        midi.set_chorus_depth(((value+1.0f)/3.2f)*1e-3f);
                        break;
                    case 3:     // CHORUS_FEEDBACK
                        // the amount of feedback from Chorus output in %
                        midi.set_chorus_feedback(0.763f*value*1e-2f);
                        break;
                    case 4:     // CHORUS_SEND_TO_REVERB
                        // the send level from Chorus to Reverb in %
                        midi.send_chorus_to_reverb(0.787f*value*1e-2f);
                        break;
                    default:
                        LOG(99, "LOG: Unsupported realtime sysex chorus parameter: %x\n",
                                 param);
                        break;
                    }
                    break;
                case MIDI_REVERB_PARAMETER:
                    switch(param)
                    {
                    case 0:     // Reverb Type
                        midi.set_reverb_type(value);
                        break;
                    case 1:     //Reverb Time
                        midi.set_reverb_time_rt60(expf((value-40)*0.025f));
                        break;
                    default:
                        LOG(99, "LOG: Unsupported realtime sysex reverb parameter: %x\n",
                                param);
                        break;
                    }
                    break;
                default:
                    break;
                }
                break;
            }
            default:
                LOG(99, "LOG: Unsupported realtime sysex parameter: %x\n", byte);
                break;
            }
            break;
        } // MIDI_DEVICE_CONTROL
        case MIDI_SCALE_ADJUST:
            LOG(99, "LOG: Unsupported realtime sysex scale adjust\n");
            break;
        case MIDI_CONTROLLER_DESTINATION:
        case MIDI_KEY_BASED_INSTRUMENT:
        default:
            byte <= 8;
            byte += pull_byte();
            CSV(channel_no, ", %d", byte & 0xff);
            LOG(99, "LOG: Unsupported realtime sysex sub id: %x %x (%d %d)\n",
                     byte >> 8, byte & 0xf, byte >> 8, byte & 0xf);
            break;
        }
        break;
    } // MIDI_BROADCAST
    default:
        LOG(99, "LOG: Unknown realtime sysex device id: %x\n", byte);
        break;
    }

    return rv;
}

