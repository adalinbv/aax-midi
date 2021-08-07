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

namespace aax
{

static float XGMIDI_LFO_table[128] = {	// Hz
 0.f, 0.08f, 0.08f, 0.16f, 0.16f, 0.25f, 0.25f, 0.33f, 0.33f, 0.42f, 0.42f,
 0.5f, 0.5f, 0.58f, 0.58f, 0.67f, 0.67f, 0.75f, 0.75f, 0.84, 0.84f, 0.92f,
 0.92f, 1.f, 1.f, 1.09f, 1.09f, 1.17f, 1.17f, 1.26f, 1.26f, 1.34f, 1.34f,
 1.43f, 1.43f, 1.51f, 1.51f, 1.59f, 1.59f, 1.68f, 1.68f, 1.76f, 1.76f, 1.85f,
 1.85f, 1.93f, 1.93f, 2.01f, 2.01f, 2.1f, 2.1f, 2.18f, 2.18f, 2.27f, 2.27f,
 2.35f, 2.35f, 2.43f, 2.43f, 2.52f, 2.52f, 2.6f, 2.6f, 2.69f, 2.69f, 2.77f,
 2.86f, 2.94f, 3.02f, 3.11f, 3.19f, 3.28f, 3.36f, 3.44f, 3.53f, 3.61f, 3.7f,
 3.86f, 4.03f, 4.2f, 4.37f, 4.54f, 4.71f, 4.87f, 5.04f, 5.21f, 5.38f, 5.55f,
 5.72f, 6.05f, 6.39f, 6.72f, 7.06f, 7.4f, 7.73f, 8.07f, 8.1f, 8.74f, 9.08f,
 9.42f, 8.75f, 10.f, 11.4f, 12.1f, 12.7f, 13.4f, 14.1f, 14.8f, 15.4f, 16.1f,
 16.8f, 17.4f, 18.1f, 19.5f, 20.8f, 22.2f, 23.5f, 24.8f, 26.2f, 27.5f, 28.9f,
 30.2f, 31.6f, 32.9f, 34.3f, 37.f, 39.7f
};

static float XGMIDI_delay_offset_table[128] = {
 0.f, 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f, 1.f, 1.1f, 1.2f,
 1.3f, 1.4f, 1.5f, 1.6f, 1.7f, 1.8f, 1.9f, 2.f, 2.1f, 2.2f, 2.3f, 2.4f, 2.5f,
 2.6f, 2.7f, 2.8f, 2.9f, 3.f, 3.1f, 3.2f, 3.3f, 3.4f, 3.5f, 3.6f, 3.7f, 3.8f,
 3.9f, 4.f, 4.1f, 4.2f, 4.3f, 4.4f, 4.5f, 4.6f, 4.7f, 4.8f, 4.9f, 5.f, 5.1f,
 5.2f, 5.3f, 5.4f, 5.5f, 5.6f, 5.7f, 5.8f, 5.9f, 6.f, 6.1f, 6.2f, 6.3f, 6.4f,
 6.5f, 6.6f, 6.7f, 6.8f, 6.9f, 7.f, 7.1f, 7.2f, 7.3f, 7.4f, 7.5f, 7.6f, 7.7f,
 7.8f, 7.9f, 8.f, 8.1f, 8.2f, 8.3f, 8.4f, 8.5f, 8.6f, 8.7f, 8.8f, 8.9f, 9.f,
 9.1f, 9.2f, 9.3f, 9.4f, 9.5f, 9.6f, 9.7f, 9.8f, 9.9f, 10.f, 11.1f, 12.2f,
 13.3f, 14.4f, 15.5f, 17.1f, 18.6f, 20.2f, 21.8f, 23.3f, 24.9f, 26.5f, 28.f,
 29.6f, 31.2f, 32.8f, 34.3f, 35.9f, 37.f, 39.f, 40.6f, 42.2f, 43.7f, 45.3f,
 46.9f, 48.4f, 50.f
};

static float XGMIDI_delay_time_table[128] = {	// ms
 0.1f, 1.7f, 3.2f, 4.8f, 6.4f, 8.0f, 9.5f, 11.1f, 12.7f, 14.3f, 15.8f, 17.4f,
 19.f, 2.6f, 22.1f, 23.7f, 25.3f, 26.9f, 28.4f, 30.f, 31.6f, 33.2f, 34.7f,
 36.3f, 37.9f, 39.5f, 41.f, 42.6f, 44.2f, 45.7f, 47.3f, 48.9f, 50.5f, 52.f,
 53.6f, 55.2f, 56.8f, 58.3f, 59.9f, 61.5f, 63.1f, 64.4f, 66.2f, 67.8f, 69.4f,
 70.9f, 72.5f, 74.1f, 75.7f, 77.2f, 78.8f, 80.4f, 81.9f, 83.5f, 85.1f, 86.7f,
 88.2f, 89.8f, 91.4f, 93.f, 94.5f, 96.1f, 97.7f, 99.3f, 100.8f, 102.4f, 104.f,
 105.6f, 107.1f, 108.7f, 110.3f, 111.9f, 113.4f, 115.f, 116.6f, 118.2f, 119.7f,
 121.3f, 122.9f, 124.4f, 126.f, 127.6f, 129.2f, 130.7f, 132.3f, 133.9f, 135.5f,
 137.f, 138.6f, 1402.f, 141.8f, 143.3f, 144.9f, 146.5f, 148.1f, 149.6f, 151.2f,
 152.8f, 154.4f, 155.9f, 157.5f, 159.1f, 160.6f, 162.2f, 163.8f, 165.4f,
 166.9f, 168.5f, 170.1f, 171.7f, 173.2f, 174.8f, 176.4f, 178.f, 179.5f, 181.1f,
 182.7f, 184.3f, 185.8f, 187.4f, 189.f, 190.6f, 192.1f, 193.7f, 195.3f, 169.6f,
 198.4f, 200.f
};

static float XGMIDI_EQ_frequency_table[61] = {
 20.f, 22.f, 25.f, 28.f, 32.f, 36.f, 40.f, 45.f, 50.f, 56.f, 63.f, 70.f, 80.f,
 90.f, 100.f, 110.f, 125.f, 140.f, 160.f, 180.f, 200.f, 225.f, 250.f, 280.f,
 315.f, 355.f, 400.f, 450.f, 500.f, 560.f, 630.f, 700.f, 800.f, 900.f, 1000.f,
 1100.f, 1200.f, 1400.f, 1600.f, 1800.f, 2000.f, 2200.f, 2500.f, 2800.f,
 3200.f, 3600.f, 4000.f, 4500.f, 5000.f, 5600.f, 6300.f, 7000.0f, 8000.f,
 9000.f, 10000.f, 11000.0f, 12000.f, 14000.f, 16000.f, 18000.f, 20000.f
};

static float XGMIDI_room_size_table[45] = {
 0.1f, 0.3f, 0.4f, 0.6f, .7f, 0.9f, 1.f, 1.2f, 1.4f, 1.5f, 1.7f, 1.8f, 2.f,
 2.1f, 2.3f, 2.5f, 2.6f, 2.8f, 2.9f, 3.1f, 4.2f, 3.4f, 3.5f, 3.7f, 3.9f, 4.f,
 4.2f, 4.3f, 4.5f, 4.6f, 4.8f, 5.f, 5.1f, 5.3f, 5.4f, 5.6f, 5.7f, 5.9f, 6.1f,
 6.2f, 6.4f, 6.5f, 6.7f, 6.8f, 7.f
};

static float XGMIDI_compressor_release_time_table[16] = {
 10.f, 15.f, 25.f, 35.f, 45.f, 55.f, 65.f, 75.f, 85.f, 100.f, 115.f, 140.f,
 170.f, 230.f, 340.f, 680.f
};

static float XGMIDI_compressor_ratio_table[8] = {
 1.f, 1.5f, 2.f, 3.f, 5.f, 7.f, 10.f, 20.0f
};

static float XGMIDI_reverb_time[70] = {
 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f, 1.f, 1.1f, 1.2f, 1.3f, 1.4f, 1.5f,
 1.6f, 1.7f, 1.8f, 1.9f, 2.f, 2.1f, 2.2f, 2.3f, 2.4f, 2.5f, 2.6f, 2.7f, 2.8f,
 2.9f, 3.f, 3.1f, 3.2f, 3.3f, 3.4f, 3.5f, 3.6f, 3.7f, 3.8f, 3.9f, 4.f, 4.1f,
 4.2f, 4.3f, 4.4f, 4.5f, 4.6f, 4.7f, 4.8f, 4.9f, 5.f, 5.5f, 6.f, 6.5f, 7.f,
 7.5f, 8.f, 8.5f, 9.f, 9.5f, 10.f, 11.f, 12.f, 13.f, 14.f, 15.f, 16.f, 17.f,
 18.f, 19.f, 20.f, 25.f, 30.f
};

static float XGMIDI_reverb_dimensions[105] = {
 0.5f, 0.8f, 1.f, 1.3f, 1.5f, 1.8f, 2.f, 2.3f, 2.6f, 2.8f, 3.1f, 3.3f, 3.6f,
 3.9f, 4.1f, 4.4f, 4.6f, 4.9f, 5.2f, 5.4f, 5.7f, 5.9f, 6.2f, 6.5f, 6.7f, 7.f,
 7.2f, 7.5f, 7.8f, 8.f, 8.3f, 8.6f, 8.8f, 9.1f, 9.4f, 9.6f, 9.9f, 10.2f, 10.4f,
 10.7f, 11.f, 11.2f, 11.5f, 11.8f, 12.1f, 12.3f, 12.6f, 12.9f, 13.1f, 3.4f,
 14.f, 14.2f, 14.5f, 14.8f, 15.1f, 15.4f, 15.6f, 15.9f, 16.2f, 16.5f, 16.8f,
 17.1f, 17.3f, 17.6f, 17.9f, 18.2f, 18.5f, 18.8f, 19.1f, 19.4f, 19.7f, 2.0f,
 20.5f, 20.8f, 21.1f, 22.f, 22.4f, 22.7f, 23.f, 23.3f, 23.6f, 23.9f, 24.2f,
 24.5f, 24.9f, 25.2f, 25.5f, 25.8f, 27.1f, 27.5f, 27.8f, 28.1f, 28.5f, 28.8f,
 29.2f, 29.5f, 29.9f, 30.2f
};

typedef struct {
    const char* name;
    int param[16];
} XGMIDI_effect_t;

#define XGMIDI_MAX_DISTORTION_TYPES	2
static XGMIDI_effect_t XGMIDI_distortion_types[XGMIDI_MAX_DISTORTION_TYPES] = {
 { "DISTORTION", 40, 20, 72, 53, 48, 0, 43, 74, 10, 127, 120, 0, 0, 0, 0, 0 },
 { "OVERDRIVE",  29, 24, 68, 45, 55, 0, 41, 72, 10, 127, 104, 0, 0, 0, 0, 0 }
};

#define XGMIDI_MAX_EQ_TYPES		2
static XGMIDI_effect_t XGMIDI_EQ_types[XGMIDI_MAX_EQ_TYPES] = {
 { "3-BAND EQ", 70, 34, 60, 10, 70, 28, 46, 0, 0, 127,  0,  0, 0,  0,  0,  0 },
 { "2-BAND EQ", 28, 70, 46, 70,  0,  0,  0, 0, 0, 127, 34, 64, 10, 0,  0,  0 }
};

}

using namespace aax;

void
MIDIStream::XG_display_data(uint32_t size, uint8_t padding, std::string &text)
{
    if (size > 6)
    {
        midi.set_lyrics(true);

        text.insert(0, padding, ' ');

        size_t len = text.size();
        if (len > 32) {
            text = text.substr(0, 32);
        }

        len = text.size();
        if (len > 16)
        {
            std::string line1 = text.substr(0, 16);
            std::string line2 = text.substr(16);
            MESSAGE("Display: %-16s - %-16s\r",
                     line1.c_str(), line2.c_str());
        } else {
            MESSAGE("Display: %-16s%-19s\r", text.c_str(), "");
        }
        FLUSH();
    }
    else
    {
         MESSAGE("Display: %-16s   %-16s\r", "", "");
        midi.set_lyrics(false);
    }
}

bool MIDIStream::XG_process_sysex(uint64_t size)
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
    case XGMIDI_BULK_DUMP:
        break;
    case XGMIDI_PARAMETER_CHANGE:
        byte = pull_byte();
        CSV(", %d", byte);
        switch(byte)
        {
        case XGMIDI_MODEL_XG:
        {
            uint8_t addr_high = pull_byte();
            uint8_t addr_mid = pull_byte();
            uint8_t addr_low = pull_byte();
            uint8_t value = pull_byte();
            uint16_t addr = addr_mid << 8 | addr_low;
            uint16_t part_no = XG_part_no[addr_mid];
            auto& channel = midi.channel(part_no);
            CSV(", %d, %d, %d, %d", addr_high, addr_mid, addr_low, value);
            switch (addr_high)
            {
            case XGMIDI_SYSTEM:
                if (addr == XGMIDI_SYSTEM_ON)
                {
                    if (value == 0x00) {
                        midi.set_mode(MIDI_EXTENDED_GENERAL_MIDI);
                    }
                    rv = true;
                }
                break;
            case XGMIDI_EFFECT1:
                switch (addr)
                {
                case XGMIDI_REVERB_TYPE:
                {
                    uint16_t type = value << 8;
                    byte = pull_byte();
                    CSV(", %d", byte);
                    type |= byte;
                    switch (type)
                    {
                    case XGMIDI_REVERB_HALL1:
                        midi.set_reverb("XG/hall1");
                        INFO("Switching to XG Concert Hall Reveberation");
                        rv = true;
                        break;
                    case XGMIDI_REVERB_HALL2:
                        midi.set_reverb("XG/hall2");
                        INFO("Switching to XG Large Concert Hall reveberation");
                        rv = true;
                        break;
                    case XGMIDI_REVERB_ROOM1:
                        midi.set_reverb("XG/room1");
                        INFO("Switching to XG Medium Room reveberation");
                        rv = true;
                        break;
                    case XGMIDI_REVERB_ROOM2:
                        midi.set_reverb("XG/room2");
                        INFO("Switching to XG Small Room reveberation");
                        rv = true;
                        break;
                    case XGMIDI_REVERB_ROOM3:
                        midi.set_reverb("XG/room3");
                        INFO("Switching to XG Large Room reveberation");
                        rv = true;
                        break;
                    case XGMIDI_REVERB_STAGE1:
                        midi.set_reverb("XG/stage1");
                        INFO("Switching to XG Small Stage reveberation");
                        rv = true;
                       break;
                    case XGMIDI_REVERB_STAGE2:
                        midi.set_reverb("XG/stage2");
                        INFO("Switching to XG Large Stage reveberation");
                        rv = true;
                       break;
                    case XGMIDI_REVERB_PLATE:
                        midi.set_reverb("XG/plate");
                        INFO("Switching to XG Plate reveberation");
                        rv = true;
                        break;
                    case XGMIDI_REVERB_WHITE_ROOM:
                        midi.set_reverb("XG/whiteroom");
                        INFO("Switching to XG White Room reveberation");
                        rv = true;
                       break;
                    case XGMIDI_REVERB_TUNNEL:
                        midi.set_reverb("XG/tunnel");
                        INFO("Switching to XG Tunnel reveberation");
                        rv = true;
                       break;
                    case XGMIDI_REVERB_CANYON:
                        midi.set_reverb("XG/canyon");
                        INFO("Switching to XG Canyon reveberation");
                        rv = true;
                       break;
                    case XGMIDI_REVERB_BASEMENT:
                        midi.set_reverb("XG/basement");
                        INFO("Switching to XG Basement reveberation");
                        rv = true;
                       break;
                    default:
                        LOG(99, "LOG: Unsupported XG sysex reverb type: 0x%02x (%d)\n",
                                type, type);
                        break;
                    }
                    break;
                }
                case XGMIDI_REVERB_PARAMETER1:	// Reverb Time
                {
                    float reverb_time = XGMIDI_reverb_time[value];
                    midi.set_reverb_time_rt60(reverb_time);
                    break;
                }
                case XGMIDI_REVERB_PARAMETER2:	// Diffusion
                {
                    float decay_depth = 0.1f*MAX_REVERB_EFFECTS_TIME*value;
                    midi.set_reverb_decay_depth(decay_depth);
                    break;
                }
                case XGMIDI_REVERB_PARAMETER3:	// Initial Delay
                {
                    float delay_depth = XGMIDI_delay_time_table[value]*1e-3f;
                    midi.set_reverb_delay_depth(delay_depth);
                    break;
                }
                case XGMIDI_REVERB_PARAMETER5:	// LPF Cutoff
                {
                    float cutoff_freq = XGMIDI_EQ_frequency_table[value];
                    midi.set_reverb_cutoff_frequency(cutoff_freq);
                    break;
                }
                case XGMIDI_REVERB_PARAMETER14:	// High Damp
                case XGMIDI_REVERB_PARAMETER6:	// Room Width
                case XGMIDI_REVERB_PARAMETER7:	// Room Height
                case XGMIDI_REVERB_PARAMETER8:	// Room Depth
                case XGMIDI_REVERB_PARAMETER9:	// Wall Vary
                case XGMIDI_REVERB_PARAMETER4:	// HPF Cutoff
                case XGMIDI_REVERB_PARAMETER10:	// Dry/Wet
                case XGMIDI_REVERB_PARAMETER11:	// Rev Delay
                case XGMIDI_REVERB_PARAMETER12:	// Density
                case XGMIDI_REVERB_PARAMETER13:	// Rev/Er Balance
                case XGMIDI_REVERB_PARAMETER15:	// Feedback Level
                case XGMIDI_REVERB_PARAMETER16:
                {
                    uint8_t param = addr - XGMIDI_REVERB_TYPE;
                    if (param > XGMIDI_REVERB_PARAMETER10) {
                        param -= (XGMIDI_REVERB_PARAMETER1 - XGMIDI_REVERB_PARAMETER10);
                    }
                    LOG(99, "LOG: Unsupported XG sysex Reverb Parameter %d\n",
                            param);
                    break;
                }
                case XGMIDI_REVERB_RETURN:
                    LOG(99, "LOG: Unsupported XG sysex Reverb Return\n");
                    break;
                case XGMIDI_REVERB_PAN:
                    LOG(99, "LOG: Unsupported XG sysex Reverb Pan\n");
                    break;
                case XGMIDI_CHORUS_TYPE:
                {
                    uint16_t type = value << 8;
                    byte = pull_byte();
                    CSV(", %d", byte);
                    type |= byte;
                    switch (type)
                    {
                    case XGMIDI_CHORUS1:
                        midi.set_chorus("XG/chorus1");
                        INFO("Switching to XG type 1 chorus");
                        rv = true;
                        break;
                    case XGMIDI_CHORUS2:
                        midi.set_chorus("XG/chorus2");
                        INFO("Switching to XG type 2 chorus");
                        rv = true;
                        break;
                    case XGMIDI_CHORUS3:
                        midi.set_chorus("XG/chorus3");
                        INFO("Switching to XG type 3chorus");
                        rv = true;
                        break;
                    case XGMIDI_CHORUS4:
                        midi.set_chorus("XG/chorus4");
                        INFO("Switching to XG type 4 chorus");
                        rv = true;
                        break;
                    case XGMIDI_CELESTE1:
                        midi.set_chorus("XG/celeste1");
                        INFO("Switching to XG type 1 celeste");
                        rv = true;
                        break;
                    case XGMIDI_CELESTE2:
                        midi.set_chorus("XG/celeste2");
                        INFO("Switching to XG type 2 celeste");
                        rv = true;
                        break;
                    case XGMIDI_CELESTE3:
                        midi.set_chorus("XG/celeste3");
                        INFO("Switching to XG type 3 celeste");
                        rv = true;
                        break;
                    case XGMIDI_CELESTE4:
                        midi.set_chorus("XG/celeste4");
                        INFO("Switching to XG type 4 celeste");
                        rv = true;
                        break;
                    case XGMIDI_FLANGING1:
                        midi.set_chorus("XG/flanger1");
                        INFO("Switching to XG type 1 flanging");
                        rv = true;
                        break;
                    case XGMIDI_FLANGING2:
                        midi.set_chorus("XG/flanger2");
                        INFO("Switching to XG type 2 flanging");
                        rv = true;
                        break;
                    case XGMIDI_FLANGING3:
                        midi.set_chorus("XG/flanger3");
                        INFO("Switching to XG type 3 flanging");
                        rv = true;
                        break;
                    case XGMIDI_SYMPHONIC:
                        midi.set_chorus("XG/symphony");
                        INFO("Switching to XG symphony");
                        rv = true;
                        break;
                    case XGMIDI_PHASER1:
                        midi.set_chorus("XG/phaser1");
                        INFO("Switching to XG type 1 phasing");
                        rv = true;
                        break;
                    case XGMIDI_PHASER2:
                        midi.set_chorus("XG/phaser2");
                        INFO("Switching to XG type 2 phasing");
                        rv = true;
                        break;
                    default:
                        LOG(99, "LOG: Unsupported XG sysex Chorus type: 0x%02x (%d)\n",
                                type, type);
                        break;
                    }
                    break;
                }
                case XGMIDI_CHORUS_PARAMETER1:	// LO Frequency
                case XGMIDI_CHORUS_PARAMETER2:	// LFO (PM) Depth
                case XGMIDI_CHORUS_PARAMETER3:	// Feedback Level
                case XGMIDI_CHORUS_PARAMETER4:	// Delay Offset
                case XGMIDI_CHORUS_PARAMETER5:
                case XGMIDI_CHORUS_PARAMETER6:	// EQ Low Frequency
                case XGMIDI_CHORUS_PARAMETER7:	// EQ Low Gain
                case XGMIDI_CHORUS_PARAMETER8:	// EQ High Frequency
                case XGMIDI_CHORUS_PARAMETER9:	// EQ High Gain
                case XGMIDI_CHORUS_PARAMETER10:	// Dry/Wet
                case XGMIDI_CHORUS_PARAMETER11:	// EQ Mid Frequency
                case XGMIDI_CHORUS_PARAMETER12:	// EQ Mid Gain
                case XGMIDI_CHORUS_PARAMETER13:	// EQ Mid Width
                case XGMIDI_CHORUS_PARAMETER14:	// EQ High Gain
                case XGMIDI_CHORUS_PARAMETER15:	// LFO AM Depth
                case XGMIDI_CHORUS_PARAMETER16:	// Mono/Stereo
                {
                    uint8_t param = addr - XGMIDI_CHORUS_TYPE;
                    if (param > XGMIDI_CHORUS_PARAMETER10) {
                        param -= (XGMIDI_CHORUS_PARAMETER11 - XGMIDI_CHORUS_PARAMETER10);
                    }
                    LOG(99, "LOG: Unsupported XG sysex Chorus Parameter %d\n",
                            param);
                    break;
                }
                case XGMIDI_CHORUS_RETURN:
                    // -inf dB...0dB...+6dB(0...64...127)
                    LOG(99, "LOG: Unsupported XG sysex Chorus Return\n");
                    break;
                case XGMIDI_CHORUS_PAN:
                    // L63...C...R63(1...64...127)
                    LOG(99, "LOG: Unsupported XG sysex Chorus sysex Pan\n");
                    break;
                case XGMIDI_CHORUS_SEND_TO_REVERB:
                    // -inf dB...0dB...+6dB(0...64...127)
                    midi.send_chorus_to_reverb(value/64.0f);
                    break;
                case XGMIDI_VARIATION_TYPE:
                {
                    uint16_t type = value << 8;
                    byte = pull_byte();
                    CSV(", %d", byte);
                    type |= byte;
                    switch (type)
                    {
                    case XGMIDI_NO_EFFECT:
                        break;
                    case XGMIDI_REVERB_HALL1:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Hall 1 reverb\n");
                        break;
                    case XGMIDI_REVERB_HALL2:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Hall 2 reverb\n");
                        break;
                    case XGMIDI_REVERB_ROOM1:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Room 1 reverb\n");
                        break;
                    case XGMIDI_REVERB_ROOM2:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Room 2 reverb\n");
                        break;
                    case XGMIDI_REVERB_ROOM3:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Room 3 reverb\n");
                        break;
                    case XGMIDI_REVERB_STAGE1:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Stage 1 reverb\n");
                        break;
                    case XGMIDI_REVERB_STAGE2:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Stage 2 reverb\n");
                        break;
                    case XGMIDI_REVERB_PLATE:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Plate reverb\n");
                        break;
                    case XGMIDI_REVERB_WHITE_ROOM:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Whiteroom reverb\n");
                        break;
                    case XGMIDI_REVERB_TUNNEL:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Tunnel reverb\n");
                        break;
                    case XGMIDI_REVERB_CANYON:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Canyon reverb\n");
                        break;
                    case XGMIDI_REVERB_BASEMENT:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Basement reverb\n");
                        break;
                    case XGMIDI_DELAY_LCR:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Delay L/C/R\n");
                        break;
                    case XGMIDI_DELAY_LR:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Delay L/R\n");
                        break;
                    case XGMIDI_ECHO:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Echo\n");
                        break;
                    case XGMIDI_X_DELAY:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Crossed feedback Delay\n");
                        break;
                    case XGMIDI_ER1:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Early Reflections 1\n");
                        break;
                    case XGMIDI_ER2:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Early Reflections 2\n");
                        break;
                    case XGMIDI_GATED_REVERB:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Gated Reverb\n");
                        break;
                    case XGMIDI_REVERSE_GATE:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Reversed Gate Reverb\n");
                        break;
                    case XGMIDI_KARAOKE1:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Karaoke type 1\n");
                        break;
                    case XGMIDI_KARAOKE2:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Karaoke type 2\n");
                        break;
                    case XGMIDI_KARAOKE3:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Karaoke type 3\n");
                        break;
                    case XGMIDI_CHORUS1:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Chorus type 1\n");
                        break;
                    case XGMIDI_CHORUS2:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Chorus type 2\n");
                        break;
                    case XGMIDI_CHORUS3:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Chorus type 3\n");
                        break;
                    case XGMIDI_CHORUS4:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Chorus type 4\n");
                        break;
                    case XGMIDI_CELESTE1:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Celeste type 1\n");
                        break;
                    case XGMIDI_CELESTE2:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Celeste type 2\n");
                        break;
                    case XGMIDI_CELESTE3:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Celeste type 3\n");
                        break;
                    case XGMIDI_CELESTE4:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Celeste type 4\n");
                        break;
                    case XGMIDI_FLANGING1:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Flamger type 1\n");
                        break;
                    case XGMIDI_FLANGING2:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Flanger type 2\n");
                        break;
                    case XGMIDI_FLANGING3:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Flanger type 3\n");
                        break;
                    case XGMIDI_SYMPHONIC:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Symphonic\n");
                        break;
                    case XGMIDI_THRU:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Thru\n");
                        break;
                    case XGMIDI_ROTARY_SPEAKER:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Rotary Speaker\n");
                        break;
                    case XGMIDI_ROTARY_SPEAKER_2_WAY:
                        LOG(99, "LOG: Unsupported XG sysex variation type: 2-Way Rotary Speaker\n");
                        break;
                    case XGMIDI_TREMOLO:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Tremolo\n");
                        break;
                    case XGMIDI_AUTO_PAN:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Auto Pan\n");
                        break;
                    case XGMIDI_PHASER1:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Phaser type 1\n");
                        break;
                    case XGMIDI_PHASER2:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Phaser type 2\n");
                        break;
                    case XGMIDI_DISTORTION:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Distortion\n");
                        break;
                    case XGMIDI_OVER_DRIVE:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Over Drive\n");
                        break;
                    case XGMIDI_AMP_SIMULATOR:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Amp Simulator\n");
                        break;
                    case XGMIDI_3_BAND_EQ:
                        LOG(99, "LOG: Unsupported XG sysex variation type: 3 band EQ\n");
                        break;
                    case XGMIDI_2_BAND_EQ:
                        LOG(99, "LOG: Unsupported XG sysex variation type: 2 band EQ\n");
                        break;
                    case XGMIDI_AUTO_WAH:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Auto Wah\n");
                        break;
                    case XGMIDI_AUTO_WAH_DISTORTION:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Auto Wah and Distortion\n");
                        break;
                    case XGMIDI_AUTO_WAH_OVER_DRIVE:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Auto Wah and Over Drive\n");
                        break;
                    case XGMIDI_PITCH_CHANGE1:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Pitch Change 1\n");
                        break;
                    case XGMIDI_PITCH_CHANGE2:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Pitch Change 2\n");
                        break;
                    case XGMIDI_AURAL_ENHANCER:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Aural Enhancer\n");
                        break;
                    case XGMIDI_TOUCH_WAH1:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Touch Wah 1\n");
                        break;
                    case XGMIDI_TOUCH_WAH2:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Touch Wah 2\n");
                        break;
                    case XGMIDI_TOUCH_WAH_DISTORTION:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Touch Wah and Distortion\n");
                        break;
                    case XGMIDI_TOUCH_WAH_OVER_DRIVE:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Touch Wah and over Drive\n");
                        break;
                    case XGMIDI_COMPRESSOR:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Compressor\n");
                        break;
                    case XGMIDI_NOISE_GATE:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Noise Gate\n");
                        break;
                    case XGMIDI_VOICE_CANCEL:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Voice Cancel\n");
                        break;
                    case XGMIDI_TALKING_SIMULATOR:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Talk Box Simulator\n");
                        break;
                    case XGMIDI_LO_FI:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Lo-Fi\n");
                        break;
                    case XGMIDI_DISTORTION_DELAY:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Distortion and Delay\n");
                        break;
                    case XGMIDI_OVER_DRIVE_DELAY:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Over Drive and Delay\n");                        
                        break;
                    case XGMIDI_COMPRESSOR_DISTORTION_DELAY:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Compressor and Distortion and Delay\n");                        
                        break;
                    case XGMIDI_COMPRESSOR_OVER_DRIVE_DELAY:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Compressor and Over Drive and Delay\n");
                        break;
                    case XGMIDI_WAH_DISTORTION_DELAY:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Wah and Distortion and Delay\n");                        
                        break;
                    case XGMIDI_WAH_OVER_DRIVE_DELAY:
                        LOG(99, "LOG: Unsupported XG sysex variation type: Wah and Over Drive and Delay\n");
                    default:
                        LOG(99, "LOG: Unsupported XG sysex variation type: 0x%02x (%d)\n",
                                type, type);
                        break;
                    }
                    break;
                }
                case XGMIDI_VARIATION_PARAMETER1:
                case XGMIDI_VARIATION_PARAMETER2:
                case XGMIDI_VARIATION_PARAMETER3:
                case XGMIDI_VARIATION_PARAMETER4:
                case XGMIDI_VARIATION_PARAMETER5:
                case XGMIDI_VARIATION_PARAMETER6:
                case XGMIDI_VARIATION_PARAMETER7:
                case XGMIDI_VARIATION_PARAMETER8:
                case XGMIDI_VARIATION_PARAMETER9:
                case XGMIDI_VARIATION_PARAMETER10:
                {
                    uint8_t param = (addr - XGMIDI_VARIATION_TYPE )/2;
                    LOG(99, "LOG: Unsupported XG sysex Variation Parameter %d\n",
                            param);
                    break;
                }
                case XGMIDI_VARIATION_RETURN:
                    LOG(99, "LOG: Unsupported XG sysex Variation Return\n");
                    break;
                case XGMIDI_VARIATION_PAN:
                    LOG(99, "LOG: Unsupported XG sysex Variation Pan\n");
                    break;
                case XGMIDI_VARIATION_SEND_TO_REVERB:
                    LOG(99, "LOG: Unsupported XG sysex Variation Send to Reverb\n");
                    break;
                case XGMIDI_VARIATION_SEND_TO_CHORUS:
                    LOG(99, "LOG: Unsupported XG sysex Variation Send to Chorus\n");
                    break;
                case XGMIDI_VARIATION_CONNECTION:
                    LOG(99, "LOG: Unsupported XG sysex Variation Connection\n");
                    break;
                case XGMIDI_VARIATION_PART_NUMBER:
                    LOG(99, "LOG: Unsupported XG sysex Variation Part Number\n");
                    break;
                case XGMIDI_MW_VARIATION_CONTROL_DEPTH:
                    LOG(99, "LOG: Unsupported XG sysex Variation Mod Wheel Contgrol Depth\n");
                    break;
                case XGMIDI_BEND_VARIATION_CONTROL_DEPTH:
                    LOG(99, "LOG: Unsupported XG sysex Variation Bend Control Depth\n");
                    break;
                case XGMIDI_CAT_VARIATION_CONTROL_DEPTH:
                    LOG(99, "LOG: Unsupported XG sysex Variation Cat Control Depth\n");
                    break;
                case XGMIDI_AC1_VARIATION_CONTROL_DEPTH:
                    LOG(99, "LOG: Unsupported XG sysex Variation AC1 Control Depth\n");
                    break;
                case XGMIDI_AC2_VARIATION_CONTROL_DEPTH:
                    LOG(99, "LOG: Unsupported XG sysex Variation AC2 Control Depth\n");
                    break;
                default:
                    LOG(99, "LOG: Unsupported XG sysex Effect1 address: 0x%02x 0x%02x (%d %d)\n",
                            addr_mid, addr_low, addr_mid, addr_low);
                    break;
                }
                break;
            case XGMIDI_EFFECT2:
                LOG(99, "LOG: Unsupported XG sysex type: Effect2\n");
                break;
            case XGMIDI_MULTI_EQ:
                LOG(99, "LOG: Unsupported XG sysex type: Multi EQ\n");
                break;
            case XGMIDI_MULTI_PART:
            {	// http://www.studio4all.de/htmle/main92.html#xgprgxgpart02a01
                switch (addr_low)
                {
                case XGMIDI_BANK_SELECT_MSB:// 0-127
                    bank_no = (uint16_t)value << 7;
                    break;
                case XGMIDI_BANK_SELECT_LSB: // 0-127
                    bank_no += value;
                    break;
                case XGMIDI_PROGRAM_NUMBER: // 1-128
                    try {
                        midi.new_channel(channel_no, bank_no, program_no);
                    } catch(const std::invalid_argument& e) {
                        ERROR("Error: " << e.what());
                    }
                    break;
                case XGMIDI_RECV_CHANNEL:
                    XG_part_no[addr_mid] = value;
                    break;
                case XGMIDI_MONO_POLY_MODE: // 0: mono, 1: poly
                    midi.process(part_no, MIDI_NOTE_OFF, 0, 0, true);
                    if (value == 0) {
                        mode = MIDI_MONOPHONIC;
                        channel.set_monophonic(true);
                    } else {
                        channel.set_monophonic(false);
                        mode = MIDI_POLYPHONIC;
                    }
                    break;
                case XGMIDI_KEY_ON_ASSIGN: // 0: multi, 1: inst (for drum)
                    LOG(99, "LOG: Unsupported XG sysex Same Note Number Key On Assign\n");
                    break;
                case XGMIDI_PART_MODE: // 0: normal, 1: drum, 2-5: drums1-4
                    // http://www.studio4all.de/htmle/main93.html
                    if (value) channel.set_drums(true);
                    else channel.set_drums(false);
                    break;
                case XGMIDI_NOTE_SHIFT: // -24 - +24 semitones
                    LOG(99, "LOG: Unsupported XG sysex Note Shift\n");
                    break;
                case XGMIDI_DETUNE: // -12.8 - 12.7 cent
                {   // 1st bit3-0: bit7-4, 2nd bit3-0: bit3-0
                    int8_t tune = (value << 4);
                    float level;
                    byte = pull_byte();
                    CSV(", %d", byte);
                    tune |= byte & 0xf;
                    level = cents2pitch(0.1f*tune, part_no);
                    channel.set_detune(level);
                    break;
                }
                case XGMIDI_VOLUME: // 0-127
                    channel.set_gain((float)value/127.0f);
                    break;
                case XGMIDI_VELOCITY_SENSE_DEPTH: // 0-127
                    LOG(99, "LOG: Unsupported XG sysex Velocity Sense Depth\n");
                    break;
                case XGMIDI_VELOCITY_SENSE_OFFSET: // 0-127
                    LOG(99, "LOG: Unsupported XG sysex Velocity Sense Offset\n");
                    break;
                case XGMIDI_PAN: // 0: random, L63 - C - R63 (1 - 64 - 127)
                    if (mode != MIDI_MONOPHONIC) {
                        channel.set_pan(((float)value-64.f)/64.f);
                    }
                    break;
                case XGMIDI_NOTE_LIMIT_LOW: // C2 - G8
                    LOG(99, "LOG: Unsupported XG sysex Note Limit Low\n");
                    break;
                case XGMIDI_NOTE_LIMIT_HIGH: // C2 - G8
                    LOG(99, "LOG: Unsupported XG sysex Note Limit High\n");
                    break;
                case XGMIDI_DRY_LEVEL: // 0-127
                    LOG(99, "LOG: Unsupported XG sysex Dry Level\n");
                    break;
                case XGMIDI_CHORUS_SEND: // 0-127
                {
                    float val = (float)value/127.0f;
                    midi.set_chorus_level(part_no, val);
                    break;
                }
                case XGMIDI_REVERB_SEND: // 0-127
                {
                    float val = (float)value/127.0f;
                    midi.set_reverb_level(part_no, val);
                    break;
                }
                case XGMIDI_VARIATION_SEND: // 0-127
                    LOG(99, "LOG: Unsupported XG sysex Variation Send\n");
                    break;
                case XGMIDI_VIBRATO_RATE: // -64 - +63
                {
                    float val = 0.5f + (float)value/64.0f;
                    channel.set_vibrato_rate(val);
                    break;
                }
                case XGMIDI_VIBRATO_DEPTH: // -64 - +63
                {
                    float val = (float)value/64.0f;
                    channel.set_vibrato_depth(val);
                    break;
                }
                case XGMIDI_VIBRATO_DELAY: // -64 - +63
                {
                    float val = (float)value/64.0f;
                    channel.set_vibrato_delay(val);
                    break;
                }
                case XGMIDI_FILTER_CUTOFF_FREQUENCY: // -64 - +63
                {
                    float val = (float)value/64.0f;
                    if (val < 1.0f) val = 0.5f + 0.5f*val;
                    channel.set_filter_cutoff(val);
                    break;
                }
                case XGMIDI_FILTER_RESONANCE: // -64 - +63
                {
                    float val = -1.0f+(float)value/16.0f; // relative: 0.0 - 8.0
                    channel.set_filter_resonance(val);
                    break;
                }
                case XGMIDI_EG_ATTACK_TIME: // -64 - +63
                    channel.set_attack_time(value);
                    break;
                case XGMIDI_EG_DECAY_TIME: // -64 - +63
                    channel.set_decay_time(value);
                    break;
                case XGMIDI_EG_RELEASE_TIME: // -64 - +63
                    channel.set_release_time(value);
                    break;
                case XGMIDI_MW_PITCH_CONTROL: // -24 - +24 semitones
                {
                    float val = 0.375f*(value-64);
                    channel.set_semi_tones(val);
                    break;
                }
                case XGMIDI_MW_FILTER_CONTROL: // -9600 - +9450 cents
                    LOG(99, "LOG: Unsupported XG sysex Mod Wheel Filter Control\n");
                    break;
                case XGMIDI_MW_AMPLITUDE_CONTROL: // -100 - +100%
                    LOG(99, "LOG: Unsupported XG sysex Mod Wheel Amplitude Control\n");
                    break;
                case XGMIDI_MW_LFO_PMOD_DEPTH: // 0 - 127
                    LOG(99, "LOG: Unsupported XG sysex Mod Wheel LFO Phase Modulation Depth\n");
                    break;
                case XGMIDI_MW_LFO_FMOD_DEPTH: // 0 - 127
                    LOG(99, "LOG: Unsupported XG sysex Mod Wheel LFO Frequency Modulation Depth\n");
                    break;
                case XGMIDI_MW_LFO_AMOD_DEPTH: // 0 - 127
                    LOG(99, "LOG: Unsupported XG sysex Mod Wheel LFO Amplitude Modulation Depth\n");
                    break;
                case XGMIDI_BEND_PITCH_CONTROL: // // -24 - +24 semitones
                    LOG(99, "LOG: Unsupported XG sysex Bend Pitch Control\n");
                    break;
                case XGMIDI_BEND_FILTER_CONTROL: // -9600 - +9450 cents
                    LOG(99, "LOG: Unsupported XG sysex Bend Filter Control\n");
                    break;
                case XGMIDI_BEND_AMPLITUDE_CONTROL: // -100 - +100%
                    LOG(99, "LOG: Unsupported XG sysex Bend Amplitude Control\n");
                    break;
                case XGMIDI_BEND_LFO_PMOD_DEPTH: // 0 - 127
                    LOG(99, "LOG: Unsupported XG sysex Bend LFO Phase Modulation Depth\n");
                    break;
                case XGMIDI_BEND_LFO_FMOD_DEPTH: // 0 - 127
                    LOG(99, "LOG: Unsupported XG sysex Bend LFO Frequency Modulation  Depth\n");
                    break;
                case XGMIDI_BEND_LFO_AMOD_DEPTH: // 0 - 127
                    LOG(99, "LOG: Unsupported XG sysex bend LFO Amplitude Modulation Depth\n");
                    break;
                default:
                    LOG(99, "LOG: Unsupported XG sysex multi part type: 0x%02x (%d)\n",
                            addr_mid, addr_mid);
                    break;
                }
                break;
            }
            case XGMIDI_A_D_PART:
                LOG(99, "LOG: Unsupported XG sysex type: A/D Part\n");
                break;
            case XGMIDI_A_D_SETUP:
                LOG(99, "LOG: Unsupported XG sysex type: A/D Setup\n");
                break;
            case XGMIDI_DRUM_SETUP:
                LOG(99, "LOG: Unsupported XG sysex type: Drum Setup\n");
                break;
            case XGMIDI_DISPLAY_DATA:
            {
                std::string text;
                for (int i=offset()-offs; i<size; ++i) {
                    toUTF8(text, pull_byte());
                }
                XG_display_data(size, addr_low, text);
                break;
            }
            default:
                LOG(99, "LOG: Unsupported XG sysex effect type: 0x%02x (%d)\n",
                        addr_high, addr_high);
                break;
            }
            break;
        }
        case XGMIDI_MASTER_TUNING:
        {
            uint8_t addr_high = pull_byte();
            uint8_t addr_mid = pull_byte();
            uint8_t addr_low = pull_byte();
            uint32_t addr = addr_high << 16 | addr_mid << 8 | addr_low;
            CSV(", %d, %d, %d", addr_high, addr_mid, addr_low);
            if (addr == 0x300000)
            {
                uint8_t mm = pull_byte();
                uint8_t ll = pull_byte();
                uint8_t cc = pull_byte();
                uint16_t tuning = mm << 7 | ll;
                float pitch = (float)tuning-8192.0f;
                CSV(", %d, %d, %d", mm, ll, cc);
                if (pitch < 0) pitch /= 8192.0f;
                else pitch /= 8191.0f;
                midi.set_tuning(pitch);
                rv = true;
            }
            break;
        }
        case XGMIDI_MODEL_MU100_SET:
        case XGMIDI_MODEL_MU100_MODIFY:
            LOG(99, "LOG: Unsupported XG sysex model ID: MU100\n");
            break;
        case XGMIDI_MODEL_VL70:
            LOG(99, "LOG: Unsupported XG sysex model ID: VL70\n");
            break;
        default:
            LOG(99, "LOG: Unsupported XG sysex parameter category: 0x%02x (%d)\n",
                    byte, byte);
            break;
        }
        break;
    default:
        LOG(99, "LOG: Unsupported XG sysex category type: 0x%02x (%d)\n", type, type);
        break;
    }

#if 0
    size -= (offset() - offs);
    if (size)
    {
        if (midi.get_csv())
        {
            while (size--) CSV(", %d", pull_byte());
            CSV("\n");
        }
        else forward(size);
    }
#endif

    return rv;
}

