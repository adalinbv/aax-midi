/*
 * Copyright (C) 2018-2021 by Erik Hofman.
 * Copyright (C) 2018-2021 by Adalin B.V.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *        this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY ADALIN B.V. ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
 * NO EVENT SHALL ADALIN B.V. OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUTOF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of Adalin B.V.
 */

#include <aax/midi.h>

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

#define XGMIDI_MAX_REVERB_TYPES		12
static XGMIDI_effect_t XGMIDI_reverb_types[XGMIDI_MAX_REVERB_TYPES] = {
 { "HALL1",    18, 10,  8, 13, 49,  0,  0,  0,  0, 40,  0, 4, 50, 8, 64, 0 },
 { "HALL2",    25, 10, 28,  6, 46,  0,  0,  0,  0, 40, 13, 3, 74, 7, 64, 0 },
 { "ROOM1",     5, 10, 16,  4, 49,  0,  0,  0,  0, 40,  5, 3, 64, 8, 64, 0 },
 { "ROOM2",    12, 10,  5,  4, 38,  0,  0,  0,  0, 40,  0, 4, 50, 8, 64, 0 },
 { "ROOM3",     9, 10, 47,  5, 36,  0,  0,  0,  0, 40,  0, 4, 60, 8, 64, 0 },
 { "STAGE1",   19, 10, 16,  7, 54,  0,  0,  0,  0, 40,  0, 3, 64, 6, 64, 0 },
 { "STAGE2",   11, 10, 16,  7, 51,  0,  0,  0,  0, 40,  2, 2, 64, 6, 64, 0 },
 { "PLATE",    25, 10,  6,  8, 49,  0,  0,  0,  0, 40,  2, 3, 64, 5, 64, 0 },
 { "WHITEROOM", 9,  5, 11,  0, 46, 30, 50, 70,  7, 40, 34, 4, 64, 7, 64, 0 },
 { "TUNNEL",   48,  6, 19,  0, 44, 33, 52, 70, 16, 40, 20, 4, 64, 7, 64, 0 },
 { "CANYON",   59,  6, 63,  0, 45, 34, 62, 91, 13, 40, 25, 4, 64, 4, 64, 0 },
 { "BASEMENT",  3,  6,  3,  0, 34, 26, 29, 59, 15, 40, 32, 4, 64, 8, 64, 0 }
};

#define XGMIDI_MAX_CHORUS_TYPES		13
static XGMIDI_effect_t XGMIDI_chorus_types[XGMIDI_MAX_CHORUS_TYPES] = {
 { "CHORUS1",   6,  54,  77, 106, 0, 28, 64, 46, 64,  64, 46, 64, 10, 0, 0, 0 },
 { "CHORUS2",   8,  63,  64,  30, 0, 28, 62, 42, 58,  64, 46, 64, 10, 0, 0, 0 },
 { "CHORUS3",   4,  44,  64, 110, 0, 28, 64, 46, 66,  64, 46, 64, 10, 0, 0, 0 },
 { "CHORUS4",   9,  32,  69, 104, 0, 28, 64, 46, 64,  64, 46, 64, 10, 0, 1, 0 },
 { "CELESTE1", 12,  32,  64,   0, 0, 28, 64, 46, 64, 127, 40, 68, 10, 0, 0, 0 },
 { "CELESTE2", 28,  18,  90,   2, 0, 28, 62, 42, 60,  84, 40, 68, 10, 0, 0, 0 },
 { "CELESTE3",  4,  63,  44,   2, 0, 28, 64, 46, 68, 127, 40, 68, 10, 0, 0, 0 },
 { "CELESTE4",  8,  29,  64,   0, 0, 28, 64, 51, 66, 127, 40, 68, 10, 0, 1, 0 },
 { "FLANGER1", 14,  14, 104,   2, 0, 28, 64, 46, 64,  96, 40, 64, 10, 4, 0, 0 },
 { "FLANGER2", 32,  17,  26,   2, 0, 28, 64, 46, 60,  96, 40, 64, 10, 4, 0, 0 },
 { "FLANGER3",  4, 109, 109,   2, 0, 28, 64, 46, 64, 127, 40, 64, 10, 4, 0, 0 },
 { "PHASER1",   8, 111,  74, 104, 0, 28, 64, 46, 64,  64,  6,  1, 64, 0, 0, 0 },
 { "PHASER2",   8, 111,  74, 104, 0, 28, 64, 46, 64,  64,  5,  1,  4, 0, 0, 0 }
};

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
MIDIStream::display_XG_data(uint32_t size, uint8_t padding, std::string &text)
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

bool MIDIStream::process_XG_sysex(uint64_t size)
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
            uint16_t addr = addr_mid << 8 | addr_low;
            CSV(", %d, %d, %d", addr_high, addr_mid, addr_low);
            switch (addr_high)
            {
            case XGMIDI_SYSTEM:
                if (addr == XGMIDI_SYSTEM_ON)
                {
                    byte = pull_byte();
                    CSV(", %d", byte);
                    if (byte == 0x00) {
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
                    uint16_t type = pull_byte() << 8 | pull_byte();
                    switch (type)
                    {
                    case XGMIDI_REVERB_HALL1:
                        midi.set_reverb("reverb/concerthall");
                        INFO("Switching to Concert Hall Reveberation");
                        rv = true;
                        break;
                    case XGMIDI_REVERB_HALL2:
                        midi.set_reverb("reverb/concerthall-large");
                        INFO("Switching to Large Concert Hall reveberation");
                        rv = true;
                        break;
                    case XGMIDI_REVERB_ROOM1:
                        midi.set_reverb("reverb/room-small");
                        INFO("Switching to Small Room reveberation");
                        rv = true;
                        break;
                    case XGMIDI_REVERB_ROOM2:
                        midi.set_reverb("reverb/room-large");
                        INFO("Switching to Medium Room reveberation");
                        rv = true;
                        break;
                    case XGMIDI_REVERB_ROOM3:
                        midi.set_reverb("reverb/room-medium");
                        INFO("Switching to Large Room reveberation");
                        rv = true;
                        break;
                    case XGMIDI_REVERB_STAGE1:
                       midi.set_reverb("reverb/stage-large");
                       INFO("Switching to Stage reveberation");
                        rv = true;
                       break;
                    case XGMIDI_REVERB_STAGE2:
                       midi.set_reverb("reverb/stage-small");
                       INFO("Switching to Large Stage reveberation");
                        rv = true;
                       break;
                    case XGMIDI_REVERB_PLATE:
                        midi.set_reverb("reverb/plate");
                        INFO("Switching to Plate reveberation");
                        rv = true;
                        break;
                    case XGMIDI_REVERB_WHITE_ROOM:
                       midi.set_reverb("reverb/bathroom");
                       INFO("Switching to White Room reveberation");
                        rv = true;
                       break;
                    case XGMIDI_REVERB_TUNNEL:
                       midi.set_reverb("reverb/tunnel");
                       INFO("Switching to Tunnel reveberation");
                        rv = true;
                       break;
                    case XGMIDI_REVERB_CANYON:
                       midi.set_reverb("reverb/canyon");
                       INFO("Switching to Canyon reveberation");
                        rv = true;
                       break;
                    case XGMIDI_REVERB_BASEMENT:
                       midi.set_reverb("reverb/basement");
                       INFO("Switching to Basement reveberation");
                        rv = true;
                       break;
                    default:
                        LOG(99, "LOG: Unsupported XG reverb type: 0x%x (%d)\n",
                                type, type);
                        break;
                    }
                    break;
                }
                case XGMIDI_REVERB_PARAMETER1:
                case XGMIDI_REVERB_PARAMETER2:
                case XGMIDI_REVERB_PARAMETER3:
                case XGMIDI_REVERB_PARAMETER4:
                case XGMIDI_REVERB_PARAMETER5:
                case XGMIDI_REVERB_PARAMETER6:
                case XGMIDI_REVERB_PARAMETER7:
                case XGMIDI_REVERB_PARAMETER8:
                case XGMIDI_REVERB_PARAMETER9:
                case XGMIDI_REVERB_PARAMETER10:
                case XGMIDI_REVERB_PARAMETER11:
                case XGMIDI_REVERB_PARAMETER12:
                case XGMIDI_REVERB_PARAMETER13:
                case XGMIDI_REVERB_PARAMETER14:
                case XGMIDI_REVERB_PARAMETER15:
                case XGMIDI_REVERB_PARAMETER16:
                {
                    uint8_t param = addr - XGMIDI_REVERB_TYPE;
                    if (param > XGMIDI_REVERB_PARAMETER10) {
                        param -= (XGMIDI_REVERB_PARAMETER1 - XGMIDI_REVERB_PARAMETER10);
                    }
                    LOG(99, "LOG: Unsupported XG Reverb Parameter %d\n",
                            param);
                    break;
                }
                case XGMIDI_REVERB_RETURN:
                    LOG(99, "LOG: Unsupported XG Reverb Return\n");
                    break;
                case XGMIDI_REVERB_PAN:
                    LOG(99, "LOG: Unsupported XG Reverb Pan\n");
                    break;
                case XGMIDI_CHORUS_TYPE:
                {
                    uint16_t type = pull_byte() << 8 | pull_byte();
                    switch (type)
                    {
                    case XGMIDI_CHORUS1:
                        midi.set_chorus("chorus/chorus1");
                        INFO("Switching to type 1 chorus");
                        rv = true;
                        break;
                    case XGMIDI_CHORUS2:
                        midi.set_chorus("chorus/chorus2");
                        INFO("Switching to type 2 chorus");
                        rv = true;
                        break;
                    case XGMIDI_CHORUS3:
                        midi.set_chorus("chorus/chorus3");
                        INFO("Switching to type 3chorus");
                        rv = true;
                        break;
                    case XGMIDI_CHORUS4:
                        midi.set_chorus("chorus/chorus4");
                        INFO("Switching to type 4 chorus");
                        rv = true;
                        break;
                    case XGMIDI_CELESTE1:
                        midi.set_chorus("chorus/chorus1");
                        INFO("Switching to type 1 celeste");
                        rv = true;
                        break;
                    case XGMIDI_CELESTE2:
                        midi.set_chorus("chorus/chorus2");
                        INFO("Switching to type 2 celeste");
                        rv = true;
                        break;
                    case XGMIDI_CELESTE3:
                        midi.set_chorus("chorus/chorus3");
                        INFO("Switching to type 3 celeste");
                        rv = true;
                        break;
                    case XGMIDI_CELESTE4:
                        midi.set_chorus("chorus/chorus4");
                        INFO("Switching to type 4 celeste");
                        rv = true;
                        break;
                    case XGMIDI_FLANGING1:
                        midi.set_chorus("chorus/flanger");
                        INFO("Switching to type 1 flanging");
                        rv = true;
                        break;
                    case XGMIDI_FLANGING2:
                        midi.set_chorus("chorus/flanger2");
                        INFO("Switching to type 2 flanging");
                        rv = true;
                        break;
                    case XGMIDI_FLANGING3:
                        midi.set_chorus("chorus/flanger3");
                        INFO("Switching to type 3 flanging");
                        rv = true;
                        break;
                    case XGMIDI_SYMPHONIC:
                        midi.set_chorus("chorus/symphony");
                        INFO("Switching to symphony");
                        rv = true;
                        break;
                    case XGMIDI_PHASING:
                        midi.set_chorus("chorus/phaser");
                        INFO("Switching to phasing");
                        rv = true;
                        break;
                    default:
                        LOG(99, "LOG: Unsupported XG chorus type: 0x%x (%d)\n",
                                type, type);
                        break;
                    }
                    break;
                }
                case XGMIDI_CHORUS_PARAMETER1:
                case XGMIDI_CHORUS_PARAMETER2:
                case XGMIDI_CHORUS_PARAMETER3:
                case XGMIDI_CHORUS_PARAMETER4:
                case XGMIDI_CHORUS_PARAMETER5:
                case XGMIDI_CHORUS_PARAMETER6:
                case XGMIDI_CHORUS_PARAMETER7:
                case XGMIDI_CHORUS_PARAMETER8:
                case XGMIDI_CHORUS_PARAMETER9:
                case XGMIDI_CHORUS_PARAMETER10:
                case XGMIDI_CHORUS_PARAMETER11:
                case XGMIDI_CHORUS_PARAMETER12:
                case XGMIDI_CHORUS_PARAMETER13:
                case XGMIDI_CHORUS_PARAMETER14:
                case XGMIDI_CHORUS_PARAMETER15:
                case XGMIDI_CHORUS_PARAMETER16:
                {
                    uint8_t param = addr - XGMIDI_CHORUS_TYPE;
                    if (param > XGMIDI_CHORUS_PARAMETER10) {
                        param -= (XGMIDI_CHORUS_PARAMETER11 - XGMIDI_CHORUS_PARAMETER10);
                    }
                    LOG(99, "LOG: Unsupported XG Chorus Parameter %d\n",
                            param);
                    break;
                }
                case XGMIDI_CHORUS_RETURN:
                    LOG(99, "LOG: Unsupported XG Chorus Return\n");
                    break;
                case XGMIDI_CHORUS_PAN:
                    LOG(99, "LOG: Unsupported XG Chorus Pan\n");
                    break;
                case XGMIDI_CHORUS_SEND_TO_REVERB:
                    LOG(99, "LOG: Unsupported XG Chorus Send to Reverb\n");
                    break;
                case XGMIDI_VARIATION_TYPE:
                {
                    uint16_t type = pull_byte() << 8 | pull_byte();
                    switch (type)
                    {
                    case XGMIDI_DELAY_LCR:
                        LOG(99, "LOG: Unsupported XG variation type: Delay L/C/R\n");
                        break;
                    case XGMIDI_DELAY_LR:
                        LOG(99, "LOG: Unsupported XG variation type: Delay L/R\n");
                        break;
                    case XGMIDI_ECHO:
                        LOG(99, "LOG: Unsupported XG variation type: Echo\n");
                        break;
                    case XGMIDI_X_DELAY:
                        LOG(99, "LOG: Unsupported XG variation type: Cross Delay\n");
                        break;
                    case XGMIDI_ER1:
                        LOG(99, "LOG: Unsupported XG variation type: ER1\n");
                        break;
                    case XGMIDI_ER2:
                        LOG(99, "LOG: Unsupported XG variation type: ER2\n");
                        break;
                    case XGMIDI_GATED_REVERB:
                        LOG(99, "LOG: Unsupported XG variation type: gated Reverb\n");
                        break;
                    case XGMIDI_REVERSE_GATE:
                        LOG(99, "LOG: Unsupported XG variation type: Reverse Gate\n");
                        break;
                    case XGMIDI_THRU:
                        LOG(99, "LOG: Unsupported XG variation type: Thru\n");
                        break;
                    case XGMIDI_ROTARY_SPEAKER:
                        LOG(99, "LOG: Unsupported XG variation type: Rotary Speaker\n");
                        break;
                    case XGMIDI_TREMOLO:
                        LOG(99, "LOG: Unsupported XG variation type: Tremolo\n");
                        break;
                    case XGMIDI_AUTO_PAN:
                        LOG(99, "LOG: Unsupported XG variation type: Auto Pan\n");
                        break;
                    case XGMIDI_DISTORTION:
                        LOG(99, "LOG: Unsupported XG variation type: Distortion\n");
                        break;
                    case XGMIDI_OVER_DRIVE:
                        LOG(99, "LOG: Unsupported XG variation type: Over Drive\n");
                        break;
                    case XGMIDI_AMP_SIMULATOR:
                        LOG(99, "LOG: Unsupported XG variation type: Amp Simulator\n");
                        break;
                    case XGMIDI_3_BAND_EQ:
                        LOG(99, "LOG: Unsupported XG variation type: 3 band EQ\n");
                        break;
                    case XGMIDI_2_BAND_EQ:
                        LOG(99, "LOG: Unsupported XG variation type: 2 band EQ\n");
                        break;
                    case XGMIDI_AUTO_WAH:
                        LOG(99, "LOG: Unsupported XG variation type: Auto Wah\n");
                        break;
                    case XGMIDI_PITCH_CHANGE:
                        LOG(99, "LOG: Unsupported XG variation type: Pitch Change\n");
                        break;
                    case XGMIDI_AURAL_ENHANCER:
                        LOG(99, "LOG: Unsupported XG variation type: Aural Enhancer\n");
                        break;
                    case XGMIDI_TOUCH_WAH:
                        LOG(99, "LOG: Unsupported XG variation type: Touch Wah\n");
                        break;
                    case XGMIDI_TOUCH_WAH_DIST:
                        LOG(99, "LOG: Unsupported XG variation type: Touch Wah and Distortion\n");
                        break;
                    case XGMIDI_COMPRESSOR:
                        LOG(99, "LOG: Unsupported XG variation type: Compressor\n");
                        break;
                    case XGMIDI_NOISE_GATE:
                        LOG(99, "LOG: Unsupported XG variation type: Noise Gate\n");
                        break;
                    case XGMIDI_VOICE_CANCEL:
                        LOG(99, "LOG: Unsupported XG variation type: Voice Cancel\n");
                        break;
                    default:
                        LOG(99, "LOG: Unsupported XG variation type: 0x%x (%d)\n",
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
                    LOG(99, "LOG: Unsupported XG Variation Parameter %d\n",
                            param);
                    break;
                }
                case XGMIDI_VARIATION_RETURN:
                    LOG(99, "LOG: Unsupported XG Variation Return\n");
                    break;
                case XGMIDI_VARIATION_PAN:
                    LOG(99, "LOG: Unsupported XG Variation Pan\n");
                    break;
                case XGMIDI_VARIATION_SEND_TO_REVERB:
                    LOG(99, "LOG: Unsupported XG Variation Send to Reverb\n");
                    break;
                case XGMIDI_VARIATION_SEND_TO_CHORUS:
                    LOG(99, "LOG: Unsupported XG Variation Send to Chorus\n");
                    break;
                case XGMIDI_VARIATION_CONNECTION:
                    LOG(99, "LOG: Unsupported XG Variation Connection\n");
                    break;
                case XGMIDI_VARIATION_PART_NUMBER:
                    LOG(99, "LOG: Unsupported XG Variation Part Number\n");
                    break;
                case XGMIDI_MW_VARIATION_CONTROL_DEPTH:
                    LOG(99, "LOG: Unsupported XG Variation MW Contgrol Depth\n");
                    break;
                case XGMIDI_BEND_VARIATION_CONTROL_DEPTH:
                    LOG(99, "LOG: Unsupported XG Variation Bend Control Depth\n");
                    break;
                case XGMIDI_CAT_VARIATION_CONTROL_DEPTH:
                    LOG(99, "LOG: Unsupported XG Variation Cat Control Depth\n");
                    break;
                case XGMIDI_AC1_VARIATION_CONTROL_DEPTH:
                    LOG(99, "LOG: Unsupported XG Variation AC1 Control Depth\n");
                    break;
                case XGMIDI_AC2_VARIATION_CONTROL_DEPTH:
                    LOG(99, "LOG: Unsupported XG Variation AC2 Control Depth\n");
                    break;
                default:
                    LOG(99, "LOG: Unsupported XG Effect1 address: 0x%x 0x%x (%d %d)\n",
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
                switch (addr_mid)
                {
                case XGMIDI_BANK_SELECT_MSB:// 0-127
                    bank_no = (uint16_t)addr_low << 7;
                    break;
                case XGMIDI_BANK_SELECT_LSB: // 0-127
                    bank_no += addr_low;
                    break;
                case XGMIDI_PROGRAM_NUMBER: // 1-128
                    try {
                        midi.new_channel(channel_no, bank_no, program_no);
                    } catch(const std::invalid_argument& e) {
                        ERROR("Error: " << e.what());
                    }
                    break;
                case XGMIDI_REV_CHANNEL:
                    LOG(99, "LOG: Unsupported XG Rev. Channel\n");
                    break;
                case XGMIDI_MONO_POLY_MODE: // 0: mono, 1: poly
                {
                    auto& channel = midi.channel(track_no);
                    midi.process(track_no, MIDI_NOTE_OFF, 0, 0, true);
                    if (addr_low == 0) {
                        mode = MIDI_MONOPHONIC;
                        channel.set_monophonic(true);
                    } else {
                        channel.set_monophonic(false);
                        mode = MIDI_POLYPHONIC;
                    }
                    break;
                }
                case XGMIDI_KEY_ON_ASSIGN: // 0: multi, 1: inst (for drum)
                    LOG(99, "LOG: Unsupported XG Same Note Number Key On Assign\n");
                    break;
                case XGMIDI_PART_MODE: // 0: normal, 1: drum, 2-5: drums1-4
                    LOG(99, "LOG: Unsupported XG Part Mode\n");
                    break;
                case XGMIDI_NOTE_SHIFT: // -24 - +24 semitones
                    LOG(99, "LOG: Unsupported XG Note Shift\n");
                    break;
                case XGMIDI_DETUNE: // -12.8 - 12.7Hz
                {   // 1st bit3-0: bit7-4, 2nd bit3-0: bit3-0
                    int8_t tune = (addr_low << 4) | (pull_byte() & 0xf);
                    LOG(99, "LOG: Unsupported XG Detune: %.1fHz\n", 0.1f* tune);
                    break;
                }
                case XGMIDI_VOLUME: // 0-127
                    LOG(99, "LOG: Unsupported XG Volume\n");
                    break;
                case XGMIDI_VELOCITY_SENSE_DEPTH: // 0-127
                    LOG(99, "LOG: Unsupported XG Velocity Sense Depth\n");
                    break;
                case XGMIDI_VELOCITY_SENSE_OFFSET: // 0-127
                    LOG(99, "LOG: Unsupported XG Velocity Sense Offset\n");
                    break;
                case XGMIDI_PAN: // 0: random, L63 - C - R63 (1 - 64 - 127)
                    LOG(99, "LOG: Unsupported XG Pan\n");
                    break;
                case XGMIDI_NOTE_LIMIT_LOW: // C2 - G8
                    LOG(99, "LOG: Unsupported XG Note Limit Low\n");
                    break;
                case XGMIDI_NOTE_LIMIT_HIGH: // C2 - G8
                    LOG(99, "LOG: Unsupported XG Note Limit High\n");
                    break;
                case XGMIDI_DRY_LEVEL: // 0-127
                     LOG(99, "LOG: Unsupported XG Dry Level\n");
                     break;
                case XGMIDI_CHORUS_SEND: // 0-127
                     LOG(99, "LOG: Unsupported XG Chorus Send\n");
                     break;
                case XGMIDI_REVERB_SEND: // 0-127
                     LOG(99, "LOG: Unsupported XG Reverb Send\n");
                     break;
                case XGMIDI_VARIATION_SEND: // 0-127
                     LOG(99, "LOG: Unsupported XG Variation Send\n");
                     break;
                case XGMIDI_VIBRATO_RATE: // -64 - +63
                    LOG(99, "LOG: Unsupported XG Vibrato Rate\n");
                    break;
                case XGMIDI_VIBRATO_DEPTH: // -64 - +63
                    LOG(99, "LOG: Unsupported XG Vibrato Depth\n");
                    break;
                case XGMIDI_VIBRATO_DELAY: // -64 - +63
                    LOG(99, "LOG: Unsupported XG Vibrato Delay\n");
                    break;
                case XGMIDI_FILTER_CUTOFF_FREQUENCY: // -64 - +63
                    LOG(99, "LOG: Unsupported XG Filter Cutoff Frequency\n");
                    break;
                case XGMIDI_FILTER_RESONANCE: // -64 - +63
                    LOG(99, "LOG: Unsupported XG Filter Resonance\n");
                    break;
                case XGMIDI_EG_ATTACK_TIME: // -64 - +63
                    LOG(99, "LOG: Unsupported XG EG Attack Time\n");
                    break;
                case XGMIDI_EG_DECAY_TIME: // -64 - +63
                    LOG(99, "LOG: Unsupported XG EG Decay Time\n");
                    break;
                case XGMIDI_EG_RELEASE_TIME: // -64 - +63
                    LOG(99, "LOG: Unsupported XG EG Release Time\n");
                    break;
                case XGMIDI_MW_PITCH_CONTROL: // -24 - +24 semitones
                    LOG(99, "LOG: Unsupported XG MW Pitch Control\n");
                    break;
                case XGMIDI_MW_FILTER_CONTROL: // -9600 - +9450 cents
                    LOG(99, "LOG: Unsupported XG MW Filter Control\n");
                    break;
                case XGMIDI_MW_AMPLITUDE_CONTROL: // -100 - +100%
                    LOG(99, "LOG: Unsupported XG MW Amplitude Control\n");
                    break;
                case XGMIDI_MW_LFO_PMOD_DEPTH: // 0 - 127
                    LOG(99, "LOG: Unsupported XG MW LFO PMOD Depth\n");
                    break;
                case XGMIDI_MW_LFO_FMOD_DEPTH: // 0 - 127
                    LOG(99, "LOG: Unsupported XG MW LFO FMOD Depth\n");
                    break;
                case XGMIDI_MW_LFO_AMOD_DEPTH: // 0 - 127
                    LOG(99, "LOG: Unsupported XG MW LFO AMOD depth\n");
                    break;
                case XGMIDI_BEND_PITCH_CONTROL: // // -24 - +24 semitones
                    LOG(99, "LOG: Unsupported XG Bend Pitch Control\n");
                    break;
                case XGMIDI_BEND_FILTER_CONTROL: // -9600 - +9450 cents
                    LOG(99, "LOG: Unsupported XG Bend Filter Control\n");
                    break;
                case XGMIDI_BEND_AMPLITUDE_CONTROL: // -100 - +100%
                    LOG(99, "LOG: Unsupported XG Bend Amplitude Control\n");
                    break;
                case XGMIDI_BEND_LFO_PMOD_DEPTH: // 0 - 127
                    LOG(99, "LOG: Unsupported XG Bend LFO PMOD Depth\n");
                    break;
                case XGMIDI_BEND_LFO_FMOD_DEPTH: // 0 - 127
                    LOG(99, "LOG: Unsupported XG Bend LFO FMOD Depth\n");
                    break;
                case XGMIDI_BEND_LFO_AMOD_DEPTH: // 0 - 127
                    LOG(99, "LOG: Unsupported XG bend LFO AMOD Depth\n");
                    break;
                default:
                    LOG(99, "LOG: Unsupported XG multi part type: 0x%x (%d)\n",
                            addr_mid, addr_mid);
                    break;
                }
                break;
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
                display_XG_data(size, addr_low, text);
                break;
            }
            default:
                LOG(99, "LOG: Unsupported XG sysex effect type: 0x%x (%d)\n",
                        type, type);
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
            if (addr == 0x300000)
            {
                uint8_t mm = pull_byte();
                uint8_t ll = pull_byte();
                uint8_t cc = pull_byte();
                uint16_t tuning = mm << 7 | ll;
                float pitch = (float)tuning-8192.0f;
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
            LOG(99, "LOG: Unsupported XG sysex parameter category: 0x%x (%d)\n",
                    byte, byte);
            break;
        }
        break;
    default:
        LOG(99, "LOG: Unsupported XG category type: 0x%x (%d)\n", type, type);
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
