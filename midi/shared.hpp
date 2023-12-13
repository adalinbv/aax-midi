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

#include <aax/midi.h>

#define DISPLAY(l,...) \
  if(midi.get_initialize() && l <= midi.get_verbose()) printf(__VA_ARGS__)
#define MESSAGE(l,...) \
  if(!midi.get_initialize() && midi.get_verbose() >= l) printf(__VA_ARGS__)
#define INFO(s) \
  if(!midi.get_initialize() && midi.get_verbose() >= 1 && !midi.get_lyrics()) \
    printf("%-79s\n", (s))
#define LOG(l,...) \
  if(midi.get_initialize() && l == midi.get_verbose()) printf(__VA_ARGS__)
#define ERROR(...) \
  if(!midi.get_csv()) { std::cerr << __VA_ARGS__ << std::endl; }
#define FLUSH() \
  if (!midi.get_initialize() && midi.get_verbose() > 0) fflush(stdout)

# define CSV(t,...) \
  if(midi.get_initialize() && midi.get_csv(t)) printf(__VA_ARGS__)
# define CSV_TEXT(t,c,s) \
  if(midi.get_initialize() && midi.get_csv(t)) do { \
    printf("%s, \"",c); \
    for (int i=0; i<strlen(s); ++i) { \
      if (s[i] == '\"') printf("\"\""); \
      else if ((s[i]<' ') || ((s[i]>'~') && (s[i]<=160))) \
        printf("\\%03o", s[i]); \
      else printf("%c", s[i]); } \
    printf("\"\n"); \
  } while(0);


namespace aax
{
#define MIDI_DRUMS_CHANNEL              0x9
#define MIDI_DRUMS_CHANNEL_MT32		0x3f80
#define MIDI_DRUMS_CHANNEL_XG		0x3f00

#define MIDI_FILE_FORMAT_MAX            0x3

#ifndef LEVEL_60DB
# define LEVEL_60DB			0.0001f
#endif
#define MAX_REVERB_EFFECTS_TIME		0.210f

enum {
    MIDI_MODE0 = 0,
    MIDI_GENERAL_MIDI1,
    MIDI_GENERAL_MIDI2,
    MIDI_GENERAL_STANDARD,
    MIDI_EXTENDED_GENERAL_MIDI,

    MIDI_MODE_MAX,

    GM = MIDI_GENERAL_MIDI1,
    GM2 = MIDI_GENERAL_MIDI2,
    GS = MIDI_GENERAL_STANDARD,
    XG = MIDI_EXTENDED_GENERAL_MIDI
};

enum {
   /* GM, GM2 */
   GM_STANDARD_DRUM_SET = 1,
   GM2_ROOM_DRUM_SET = 9,
   GM2_POWER_DRUM_SET = 17,
   GM2_ELECTRONIC_DRUM_SET = 25,
   GM2_ANALOG_DRUM_SET = 26,
   GM2_JAZZ_DRUM_SET = 33,
   GM2_BRUSH_DRUM_SET = 41,
   GM2_ORCHESTRA_DRUM_SET = 49,

   /* GS */
   GS_STANDARD2_DRUM_SET = 2,
   GS_RND_DRUM_SET = 3,
   GS_HIP_HOP_DRUM_SET = 10,
   GS_JUNGLE_DRUM_SET = 11,
   GS_TECHNO_DRUM_SET = 12,
   GS_DANCE_DRUM_SET = 27,
   GS_CR_78_DRUM_SET = 28,
   GS_TR_606_DRUM_SET = 29,
   GS_TR_707_DRUM_SET = 30,
   GS_TR_909_DRUM_SET = 31,
   GS_ETHNIC_DRUM_SET = 50,
   GS_KICK_SNARE_DRUM_SET = 51,
   GS_ASIA_DRUM_SET = 53,
   GS_CYMBAL_CLAPS_DRUM_SET = 54,
   GS_RHYTHM_FX_DRUM_SET = 58,
   GS_RHYTHM_FX2_DRUM_SET = 59,

   /* XG */
   XG_STANDARD2_DRUM_SET = 2,
   XG_DRY_DRUM_SET = 3,
   XG_BRIGHT_DRUM_SET = 4,
   XG_DARK_ROOM_DRUM_SET = 10,
   XG_ROCK2_DRUM_SET = 18,
   XG_ANALOG2_DRUM_SET = 27,
   XG_HIP_HOP_DRUM_SET = 29,
   XG_JAZZ2_DRUM_SET = 34
};

/**** GMMIDI ****/
#define GMMIDI_BROADCAST					0x7f

#define GMMIDI_GM_RESET						0x01
#define GMMIDI_GM2_RESET					0x03

#define GM2_CHORUS1						0x00
#define GM2_CHORUS2						0x01
#define GM2_CHORUS3						0x02
#define GM2_CHORUS4						0x03
#define GM2_CHORUS_FEEDBACK					0x04
#define GM2_FLANGER						0x05

#define GM2_REVERB_ROOM_SMALL					0x00
#define GM2_REVERB_ROOM_MEDIUM					0x01
#define GM2_REVERB_ROOM_LARGE					0x02
#define GM2_REVERB_CONCERTHALL					0x03
#define GM2_REVERB_CONCERTHALL_LARGE				0x04
#define GM2_REVERB_PLATE					0x08

/**** GSMIDI ****/
#define GSMIDI_SYSTEM						0x10

#define GSMIDI_DATA_REQUEST1					0x11
#define GSMIDI_DATA_SET1					0x12

/* parameter type high address */
#define GSMIDI_SYSTEM_PARAMETER_CHANGE				0x00
#define GSMIDI_SYSTEM_INFORMATION				0x01
#define GSMIDI_DISPLAY_DATA					0x06
#define GSMIDI_DISPLAY_BITMAP					0x07
#define GSMIDI_USER_INSTRUMENT					0x20
#define GSMIDI_USER_DRUM_SET					0x21
#define GSMIDI_USER_EFFECT					0x22
#define GSMIDI_USER_PATCH					0x23
#define GSMIDI_USER_PART_PATCH					0x2a
#define GSMIDI_PARAMETER_CHANGE					0x40
#define GSMIDI_DRUM_SETUP_PARAMETER_CHANGE			0x41
#define GSMIDI_MODEL_GS						0x42

/* GS parameter map */
/* system parameter mid & low address */
#define GSMIDI_GS_RESET						0x007f
#define GSMIDI_SYSTEM_MODE_SET					0x007f

#define GSMIDI_MASTER_TUNE					0x0000
#define GSMIDI_MASTER_VOLUME					0x0004
#define GSMIDI_MASTER_KEY_SHIFT					0x0005
#define GSMIDI_MASTER_PAN					0x0006
#define GSMIDI_TX_CHANNEL					0x0009
#define GSMIDI_RCV_CHANNEL					0x000a
#define GSMIDI_BREATH_CONTROL_NUMBER				0x000b
#define GSMIDI_BREATH_CONTROL_CURVE				0x000c
#define GSMIDI_BREATH_SET_LOCK					0x000e
#define GSMIDI_BREATH_MODE					0x0010
#define GSMIDI_VELOCITY_DEPTH					0x0011
#define GSMIDI_VELOCITY_OFFSET					0x0012

#define GSMIDI_VOICE_RESERVE_PART1				0x0110
#define GSMIDI_VOICE_RESERVE_PART2				0x0111
#define GSMIDI_VOICE_RESERVE_PART3				0x0112
#define GSMIDI_VOICE_RESERVE_PART4				0x0113
#define GSMIDI_VOICE_RESERVE_PART5				0x0114
#define GSMIDI_VOICE_RESERVE_PART6				0x0115
#define GSMIDI_VOICE_RESERVE_PART7				0x0116
#define GSMIDI_VOICE_RESERVE_PART8				0x0117
#define GSMIDI_VOICE_RESERVE_PART9				0x0118
#define GSMIDI_VOICE_RESERVE_PART10				0x0119
#define GSMIDI_VOICE_RESERVE_PART11				0x011a
#define GSMIDI_VOICE_RESERVE_PART12				0x011b
#define GSMIDI_VOICE_RESERVE_PART13				0x011c
#define GSMIDI_VOICE_RESERVE_PART14				0x011d
#define GSMIDI_VOICE_RESERVE_PART15				0x011e
#define GSMIDI_VOICE_RESERVE_PART16				0x011f

#define GSMIDI_MULTI_PART					0x1100

/* patch parameters */
#define GSMIDI_REVERB_MACRO					0x0130
#define GSMIDI_REVERB_CHARACTER					0x0131
#define GSMIDI_REVERB_PRE_LPF					0x0132
#define GSMIDI_REVERB_LEVEL					0x0133
#define GSMIDI_REVERB_TIME					0x0134
#define GSMIDI_REVERB_DELAY_FEEDBACK				0x0135

#define GSMIDI_CHORUS_MACRO					0x0138
#define GSMIDI_CHORUS_PRE_LPF					0x0139
#define GSMIDI_CHORUS_LEVEL					0x013a
#define GSMIDI_CHORUS_FEEDBACK					0x013b
#define GSMIDI_CHORUS_DELAY					0x013c
#define GSMIDI_CHORUS_RATE					0x013d
#define GSMIDI_CHORUS_DEPTH					0x013e
#define GSMIDI_CHORUS_SEND_LEVEL_TO_REVERB			0x013f

#define GSMIDI_DELAY_MACRO					0x0150
#define GSMIDI_DELAY_PRE_LPF					0x0051
#define GSMIDI_DELAY_TIME_RATIO_LEFT				0x0153
#define GSMIDI_DELAY_TIME_CENTER				0x0152
#define GSMIDI_DELAY_TIME_RATIO_RIGHT				0x0154
#define GSMIDI_DELAY_FEEDBACK					0x0159
#define GSMIDI_DELAY_LEVEL					0x0158
#define GSMIDI_DELAY_SEND_LEVEL_TO_REVERB			0x015a
#define GSMIDI_DELAY_LEVEL_LEFT					0x0156
#define GSMIDI_DELAY_LEVEL_CENTER				0x0155
#define GSMIDI_DELAY_LEVEL_RIGHT				0x0157

#define GSMIDI_RYTHM_PART10					0x1015
#define GSMIDI_RYTHM_PART1					0x1115
#define GSMIDI_RYTHM_PART2					0x1215
#define GSMIDI_RYTHM_PART3					0x1315
#define GSMIDI_RYTHM_PART4					0x1415
#define GSMIDI_RYTHM_PART5					0x1515
#define GSMIDI_RYTHM_PART6					0x1615
#define GSMIDI_RYTHM_PART7					0x1715
#define GSMIDI_RYTHM_PART8					0x1815
#define GSMIDI_RYTHM_PART9					0x1915
#define GSMIDI_RYTHM_PART11					0x1a15
#define GSMIDI_RYTHM_PART12					0x1b15
#define GSMIDI_RYTHM_PART13					0x1c15
#define GSMIDI_RYTHM_PART14					0x1d15
#define GSMIDI_RYTHM_PART15					0x1e15
#define GSMIDI_RYTHM_PART16					0x1f15

/* parameter mid address */
#define GSMIDI_
#define GSMIDI_EQUALIZER					0x02
#define GSMIDI_EFX_TYPE_EFFECT					0x03
#define GSMIDI_PART_SET						0x10
#define GSMIDI_MODULATION_SET					0x20
#define GSMIDI_PART_SWITCH					0x40

/* equalizer parameters */
#define GSMIDI_EQUALIZER_FREQUENCY_LOW				0x00
#define GSMIDI_EQUALIZER_GAIN_LOW				0x01
#define GSMIDI_EQUALIZER_FREQUENCY_HIGH				0x02
#define GSMIDI_EQUALIZER_GAIN_HIGH				0x03

/* part parameters */
#define GSMIDI_PART_TONE_NUMBER					0x00
#define GSMIDI_PART_RX_CHANNEL					0x02
#define GSMIDI_PART_PITCH_BEND_SWITCH				0x03
#define GSMIDI_PART_CHANNEL_PRESSURE_SWITCH			0x04
#define GSMIDI_PART_PROGRAM_CHANGE_SWITCH			0x05
#define GSMIDI_PART_CONTROL_CHANGE_SWITCH			0x06
#define GSMIDI_PART_POLY_PRESSURE_SWITCH			0x07
#define GSMIDI_PART_NOTE_MESSAGE_SWITCH				0x08
#define GSMIDI_PART_RPN_SWITCH					0x09
#define GSMIDI_PART_NRPN_SWITCH 				0x0a
#define GSMIDI_PART_MODULATION_SWITCH				0x0b
#define GSMIDI_PART_VOLUME_SWITCH				0x0c
#define GSMIDI_PART_PAN_SWITCH					0x0d
#define GSMIDI_PART_EXPRESSION_SWITCH				0x0e
#define GSMIDI_PART_HOLD1_SWITCH				0x0f
#define GSMIDI_PART_PORTAMENTO_SWITCH				0x10
#define GSMIDI_PART_SOSTENUTO_SWITCH				0x11
#define GSMIDI_PART_SOFT_SWITCH					0x12
#define GSMIDI_PART_POLY_MODE 					0x13
#define GSMIDI_PART_ASSIGN_MODE					0x14
#define GSMIDI_PART_RYTHM_MODE					0x15
#define GSMIDI_PART_PITCH_KEY_SHIFT				0x16
#define GSMIDI_PART_PITCH_OFFSET_FINE				0x17
#define GSMIDI_PART_VOLUME					0x19
#define GSMIDI_PART_VELOCITY_SENSE_DEPTH			0x1a
#define GSMIDI_PART_VELOCITY_SENSE_OFFSET			0x1b
#define GSMIDI_PART_PAN						0x1c
#define GSMIDI_PART_KEYBOARD_RANGE_LOW				0x1d
#define GSMIDI_PART_KEYBOARD_RANGE_HIGH				0x1e
#define GSMIDI_PART_CC1_CONTROL_NUMBER				0x1f
#define GSMIDI_PART_CC2_CONTROL_NUMBER				0x20
#define GSMIDI_PART_CHORUS_SEND_LEVEL				0x21
#define GSMIDI_PART_REVERB_SEND_LEVEL				0x22
#define GSMIDI_PART_BANK_SELECT_SWITCH				0x23
#define GSMIDI_PART_BANK_SELECT_LSB_SWITCH			0x24
#define GSMIDI_PART_PITCH_FINE_TUNE				0x2a
#define GSMIDI_PART_DELAY_SEND_LEVEL				0x2c
#define GSMIDI_PART_VIBRATO_RATE				0x30
#define GSMIDI_PART_VIBRATO_DEPTH				0x31
#define GSMIDI_PART_CUTOFF_FREQUENCY				0x32
#define GSMIDI_PART_RESONANCE					0x33
#define GSMIDI_PART_ATTACK_TIME					0x34
#define GSMIDI_PART_DECAY_TIME					0x35
#define GSMIDI_PART_RELEASE_TIME				0x36
#define GSMIDI_PART_VIBRATO_DELAY				0x37

/* modulation parameters */
#define GSMIDI_MODULATION_DEPTH					0x04
#define GSMIDI_BEND_RANGE					0x10

/* equalizer switch parameters */
#define GSMIDI_PART_EQUALIZER_SWITCH				0x20

/* reverb parameters */
#define GSMIDI_REVERB_ROOM1					0x00
#define GSMIDI_REVERB_ROOM2					0x01
#define GSMIDI_REVERB_ROOM3					0x02
#define GSMIDI_REVERB_HALL1					0x03
#define GSMIDI_REVERB_HALL2					0x04
#define GSMIDI_REVERB_PLATE					0x05
#define GSMIDI_REVERB_DELAY					0x06
#define GSMIDI_REVERB_PAN_DELAY					0x07

/* delay parameters */
#define GSMIDI_DELAY1						0x00
#define GSMIDI_DELAY2						0x01
#define GSMIDI_DELAY3						0x02
#define GSMIDI_DELAY4						0x03
#define GSMIDI_PAN_DELAY1					0x04
#define GSMIDI_PAN_DELAY2					0x05
#define GSMIDI_PAN_DELAY3					0x06
#define GSMIDI_PAN_DELAY4					0x07
#define GSMIDI_DELAY_TO_REVERB					0x08
#define GSMIDI_PAN_REPEAT					0x09

/* chorus parameters */
#define GSMIDI_CHORUS1						0x00
#define GSMIDI_CHORUS2						0x01
#define GSMIDI_CHORUS3						0x02
#define GSMIDI_CHORUS4						0x03
#define GSMIDI_FEEDBACK_CHORUS					0x04
#define GSMIDI_FLANGER						0x05
#define GSMIDI_CHORUS_DELAY1					0x06
#define GSMIDI_CHORUS_DELAY1_FEEDBACK				0x07

/* insertion effects */
#define GSMIDI_EFX_TYPE						0x00
#define GSMIDI_EFX_PARAMETER1					0x03
#define GSMIDI_EFX_PARAMETER2					0x04
#define GSMIDI_EFX_PARAMETER3					0x05
#define GSMIDI_EFX_PARAMETER4					0x06
#define GSMIDI_EFX_PARAMETER5					0x07
#define GSMIDI_EFX_PARAMETER6					0x08
#define GSMIDI_EFX_PARAMETER7					0x09
#define GSMIDI_EFX_PARAMETER8					0x0a
#define GSMIDI_EFX_PARAMETER9					0x0b
#define GSMIDI_EFX_PARAMETER10					0x0c
#define GSMIDI_EFX_PARAMETER11					0x0d
#define GSMIDI_EFX_PARAMETER12					0x0e
#define GSMIDI_EFX_PARAMETER13					0x0f
#define GSMIDI_EFX_PARAMETER14					0x10
#define GSMIDI_EFX_PARAMETER15					0x11
#define GSMIDI_EFX_PARAMETER16					0x12
#define GSMIDI_EFX_PARAMETER17					0x13
#define GSMIDI_EFX_PARAMETER18					0x14
#define GSMIDI_EFX_PARAMETER19					0x15
#define GSMIDI_EFX_PARAMETER20					0x16
#define GSMIDI_EFX_SEND_LEVEL_TO_REVERB				0x17
#define GSMIDI_EFX_SEND_LEVEL_TO_CHORUS				0x18
#define GSMIDI_EFX_SEND_LEVEL_TO_DELAY				0x19
#define GSMIDI_EFX_CONTROL_SOURCE1				0x1b
#define GSMIDI_EFX_CONTROL_DEPTH1				0x1c
#define GSMIDI_EFX_CONTROL_SOURCE2				0x1d
#define GSMIDI_EFX_CONTROL_DEPTH2				0x1e
#define GSMIDI_EFX_SEND_EQ_TYPE					0x1f

/*insertion effect types */
#define GSMIDI_EFX_TYPE_THRU					0x0000
#define GSMIDI_EFX_TYPE_STEREO_EQ				0x0100
#define GSMIDI_EFX_TYPE_SPECTRUM				0x0101
#define GSMIDI_EFX_TYPE_ENHANCER				0x0102
#define GSMIDI_EFX_TYPE_HUMANIZER				0x0103
#define GSMIDI_EFX_TYPE_OVERDRIVE				0x0110
#define GSMIDI_EFX_TYPE_DISTORTION				0x0111
#define GSMIDI_EFX_TYPE_PHASER					0x0120
#define GSMIDI_EFX_TYPE_AUTO_WAH				0x0121
#define GSMIDI_EFX_TYPE_ROTARY					0x0122
#define GSMIDI_EFX_TYPE_STEREO_FLANGER				0x0123
#define GSMIDI_EFX_TYPE_STEP_FLANGER				0x0124
#define GSMIDI_EFX_TYPE_TREMOLO					0x0125
#define GSMIDI_EFX_TYPE_AUTO_PAN				0x0126
#define GSMIDI_EFX_TYPE_COMPRESSOR				0x0130
#define GSMIDI_EFX_TYPE_LIMITER					0x0131
#define GSMIDI_EFX_TYPE_HEXA_CHORUS				0x0140
#define GSMIDI_EFX_TYPE_TREMOLO_CHORUS				0x0141
#define GSMIDI_EFX_TYPE_STEREO_CHORUS				0x0142
#define GSMIDI_EFX_TYPE_SPACE_D					0x0143
#define GSMIDI_EFX_TYPE_3D_CHORUS				0x0144
#define GSMIDI_EFX_TYPE_STEREO_DELAY				0x0150
#define GSMIDI_EFX_TYPE_MOD_DELAY				0x0151
#define GSMIDI_EFX_TYPE_3TAP_DELAY				0x0152
#define GSMIDI_EFX_TYPE_4TAP_DELAY				0x0153
#define GSMIDI_EFX_TYPE_TIME_CONTROL_DELAY			0x0154
#define GSMIDI_EFX_TYPE_REVERB					0x0155
#define GSMIDI_EFX_TYPE_GATE_REVERB				0x0156
#define GSMIDI_EFX_TYPE_3D_DELAY				0x0157
#define GSMIDI_EFX_TYPE_2PITCH_SHIFTER				0x0160
#define GSMIDI_EFX_TYPE_FEEDBACK_PITCH_SHIFTER			0x0161
#define GSMIDI_EFX_TYPE_3D_AUTO					0x0170
#define GSMIDI_EFX_TYPE_3D_MANUAL				0x0171
#define GSMIDI_EFX_TYPE_LOFI1					0x0172
#define GSMIDI_EFX_TYPE_LOFI2					0x0173
#define GSMIDI_EFX_TYPE_OVERDRIVE_TO_CHORUS			0x0200
#define GSMIDI_EFX_TYPE_OVERDRIVE_TO_FLANGER			0x0210
#define GSMIDI_EFX_TYPE_OVERDRIVE_TO_DELAY			0x0202
#define GSMIDI_EFX_TYPE_DISTORTION_TO_CHORUS			0x0203
#define GSMIDI_EFX_TYPE_DISTORTION_TO_FLANGER			0x0204
#define GSMIDI_EFX_TYPE_DISTORTION_TO_DELAY			0x0205
#define GSMIDI_EFX_TYPE_ENHANCER_TO_CHORUS			0x0206
#define GSMIDI_EFX_TYPE_ENHANCER_TO_FLANGER			0x0207
#define GSMIDI_EFX_TYPE_ENHANCER_TO_DELAY			0x0208
#define GSMIDI_EFX_TYPE_CHORUS_TO_DELAY				0x0209
#define GSMIDI_EFX_TYPE_FLANGER_TO_DELAY			0x020a
#define GSMIDI_EFX_TYPE_CHORUS_TO_FLANGER			0x020b
#define GSMIDI_EFX_TYPE_ROTARY_MULTI				0x0300
#define GSMIDI_EFX_TYPE_GUITAR_MULTI1				0x0400
#define GSMIDI_EFX_TYPE_GUITAR_MILTI2				0x0401
#define GSMIDI_EFX_TYPE_GUITAR_MULTI3				0x0402
#define GSMIDI_EFX_TYPE_CLEAN_GUITAR_MULTI1			0x0403
#define GSMIDI_EFX_TYPE_CLEAN_GUITAR_MULTI2			0x0404
#define GSMIDI_EFX_TYPE_BASS_MULTI				0x0405
#define GSMIDI_EFX_TYPE_RHODES_MULTI				0x0406
#define GSMIDI_EFX_TYPE_KEYBOARD_MULTI				0x0500
#define GSMIDI_EFX_TYPE_CHORUS_DELAY				0x1100
#define GSMIDI_EFX_TYPE_FLANGER_DELAY				0x1101
#define GSMIDI_EFX_TYPE_CHORUS_FLANGER				0x1102
#define GSMIDI_EFX_TYPE_OVERDRIVE_DISTORTION12			0x1103
#define GSMIDI_EFX_TYPE_OVERDRIVE_DISTORTION_ROTARY		0x1104
#define GSMIDI_EFX_TYPE_OVERDRIVE_DISTORTION_PHASER		0x1105
#define GSMIDI_EFX_TYPE_OVERDRIVE_DISTORTION_AUTO_WAH		0x1106
#define GSMIDI_EFX_TYPE_PHASER_ROTARY				0x1107
#define GSMIDI_EFX_TYPE_PHASER_AUTO_WAH				0x1108

/**** XGMIDI *****/
/* categroy */
#define XGMIDI_BULK_DUMP					0x00
#define XGMIDI_PARAMETER_CHANGE					0x10
#define XGMIDI_DUMP_REQUEST					0x20
#define XGMIDI_PARAMETER_REQUEST				0x30

/* parameter category */
#define XGMIDI_MASTER_TUNING					0x27	// 39
#define XGMIDI_BLOCK1						0x29	// 41
#define XGMIDI_BLOCK2						0x3f	// 53
#define XGMIDI_MODEL_MU100_SET					0x49
#define XGMIDI_MODEL_VL70					0x47
#define XGMIDI_MODEL_MU100_MODIFY				0x59
#define XGMIDI_MODEL_CLAVINOVA					0x73
#define XGMIDI_MODEL_XG						0x4c	// 76

/* parameter effect type */
#define XGMIDI_SYSTEM						0x00
#define XGMIDI_EFFECT1						0x02
#define XGMIDI_EFFECT2						0x03
#define XGMIDI_DISPLAY_DATA					0x06
#define XGMIDI_MULTI_PART					0x08
#define XGMIDI_A_D_PART						0x10
#define XGMIDI_A_D_SETUP					0x11
#define XGMIDI_DRUM_SETUP					0x30
#define XGMIDI_MULTI_EQ						0x40

/* XG Effect Map */
/* system */
#define XGMIDI_MASTER_TUNE					0x0000
#define XGMIDI_MASTER_VOLUME					0x0004
#define XGMIDI_ATTENUATOR					0x0005
#define XGMIDI_TRANSPOSE					0x0006
#define XGMIDI_DRUM_SETUP_RESET					0x007d
#define XGMIDI_SYSTEM_ON					0x007e

/* effect1 address (0x02) */
#define XGMIDI_REVERB_TYPE					0x0100
#define XGMIDI_REVERB_PARAMETER1				0x0102
#define XGMIDI_REVERB_PARAMETER2				0x0103
#define XGMIDI_REVERB_PARAMETER3				0x0104
#define XGMIDI_REVERB_PARAMETER4				0x0105
#define XGMIDI_REVERB_PARAMETER5				0x0106
#define XGMIDI_REVERB_PARAMETER6				0x0107
#define XGMIDI_REVERB_PARAMETER7				0x0108
#define XGMIDI_REVERB_PARAMETER8				0x0109
#define XGMIDI_REVERB_PARAMETER9				0x010a
#define XGMIDI_REVERB_PARAMETER10				0x010b
#define XGMIDI_REVERB_RETURN					0x010c
#define XGMIDI_REVERB_PAN					0x010d

#define XGMIDI_REVERB_PARAMETER11				0x0110
#define XGMIDI_REVERB_PARAMETER12				0x0111
#define XGMIDI_REVERB_PARAMETER13				0x0112
#define XGMIDI_REVERB_PARAMETER14				0x0113
#define XGMIDI_REVERB_PARAMETER15				0x0114
#define XGMIDI_REVERB_PARAMETER16				0x0115

#define XGMIDI_CHORUS_TYPE					0x0120
#define XGMIDI_CHORUS_PARAMETER1				0x0122
#define XGMIDI_CHORUS_PARAMETER2				0x0123
#define XGMIDI_CHORUS_PARAMETER3				0x0124
#define XGMIDI_CHORUS_PARAMETER4				0x0125
#define XGMIDI_CHORUS_PARAMETER5				0x0126
#define XGMIDI_CHORUS_PARAMETER6				0x0127
#define XGMIDI_CHORUS_PARAMETER7				0x0128
#define XGMIDI_CHORUS_PARAMETER8				0x0129
#define XGMIDI_CHORUS_PARAMETER9				0x012a
#define XGMIDI_CHORUS_PARAMETER10				0x012b
#define XGMIDI_CHORUS_RETURN					0x012c
#define XGMIDI_CHORUS_PAN					0x012d
#define XGMIDI_CHORUS_SEND_TO_REVERB				0x012e

#define XGMIDI_CHORUS_PARAMETER11				0x0130
#define XGMIDI_CHORUS_PARAMETER12				0x0131
#define XGMIDI_CHORUS_PARAMETER13				0x0132
#define XGMIDI_CHORUS_PARAMETER14				0x0133
#define XGMIDI_CHORUS_PARAMETER15				0x0134
#define XGMIDI_CHORUS_PARAMETER16				0x0135

#define XGMIDI_VARIATION_TYPE					0x0140
#define XGMIDI_VARIATION_PARAMETER1				0x0142
#define XGMIDI_VARIATION_PARAMETER2				0x0144
#define XGMIDI_VARIATION_PARAMETER3				0x0146
#define XGMIDI_VARIATION_PARAMETER4				0x0148
#define XGMIDI_VARIATION_PARAMETER5				0x014a
#define XGMIDI_VARIATION_PARAMETER6				0x014c
#define XGMIDI_VARIATION_PARAMETER7				0x014e
#define XGMIDI_VARIATION_PARAMETER8				0x0150
#define XGMIDI_VARIATION_PARAMETER9				0x0152
#define XGMIDI_VARIATION_PARAMETER10				0x0154
#define XGMIDI_VARIATION_RETURN					0x0156
#define XGMIDI_VARIATION_PAN					0x0157
#define XGMIDI_VARIATION_SEND_TO_REVERB				0x0158
#define XGMIDI_VARIATION_SEND_TO_CHORUS				0x0159
#define XGMIDI_VARIATION_CONNECTION				0x015a
#define XGMIDI_VARIATION_PART_NUMBER				0x015b
#define XGMIDI_MW_VARIATION_CONTROL_DEPTH			0x015c
#define XGMIDI_BEND_VARIATION_CONTROL_DEPTH			0x015d
#define XGMIDI_CAT_VARIATION_CONTROL_DEPTH			0x015e
#define XGMIDI_AC1_VARIATION_CONTROL_DEPTH			0x015f
#define XGMIDI_AC2_VARIATION_CONTROL_DEPTH			0x0160

#define XGMIDI_VARIATION_PARAMETER11				0x0170
#define XGMIDI_VARIATION_PARAMETER12				0x0171
#define XGMIDI_VARIATION_PARAMETER13				0x0172
#define XGMIDI_VARIATION_PARAMETER14				0x0173
#define XGMIDI_VARIATION_PARAMETER15				0x0174
#define XGMIDI_VARIATION_PARAMETER16				0x0175

#define XGMIDI_MULTI_EQ_TYPE					0x4000
#define XGMIDI_MULTI_EQ_GAIN1					0x4001
#define XGMIDI_MULTI_EQ_FREQUENCY1				0x4002
#define XGMIDI_MULTI_EQ_Q1					0x4003
#define XGMIDI_MULTI_EQ_SHAPE1					0x4004
#define XGMIDI_MULTI_EQ_GAIN2					0x4005
#define XGMIDI_MULTI_EQ_FREQUENCY2				0x4006
#define XGMIDI_MULTI_EQ_Q2					0x4007
#define XGMIDI_MULTI_EQ_GAIN3					0x4009
#define XGMIDI_MULTI_EQ_FREQUENCY3				0x400a
#define XGMIDI_MULTI_EQ_Q3					0x400b
#define XGMIDI_MULTI_EQ_GAIN4					0x400d
#define XGMIDI_MULTI_EQ_FREQUENCY4				0x400e
#define XGMIDI_MULTI_EQ_Q4					0x400f
#define XGMIDI_MULTI_EQ_GAIN5					0x4011
#define XGMIDI_MULTI_EQ_FREQUENCY5				0x4012
#define XGMIDI_MULTI_EQ_Q5					0x4013
#define XGMIDI_MULTI_EQ_SHAPE5					0x4014

/* reverb type */
#define XGMIDI_REVERB_HALL1					0x0100
#define XGMIDI_REVERB_HALL2 					0x0101
#define XGMIDI_REVERB_ROOM1					0x0200
#define XGMIDI_REVERB_ROOM2					0x0201
#define XGMIDI_REVERB_ROOM3					0x0202
#define XGMIDI_REVERB_STAGE1					0x0300
#define XGMIDI_REVERB_STAGE2					0x0301
#define XGMIDI_REVERB_PLATE					0x0400
#define XGMIDI_REVERB_WHITE_ROOM				0x1000
#define XGMIDI_REVERB_TUNNEL					0x1100
#define XGMIDI_REVERB_CANYON					0x1200
#define XGMIDI_REVERB_BASEMENT					0x1300

/* chorus type */
#define XGMIDI_CHORUS1						0x4100
#define XGMIDI_CHORUS2						0x4101
#define XGMIDI_CHORUS3						0x4102
#define XGMIDI_CHORUS4						0x4108
#define XGMIDI_CELESTE1						0x4200
#define XGMIDI_CELESTE2						0x4201
#define XGMIDI_CELESTE3						0x4202
#define XGMIDI_CELESTE4						0x4208
#define XGMIDI_FLANGING1					0x4300
#define XGMIDI_FLANGING2					0x4301
#define XGMIDI_FLANGING3					0x4308
#define XGMIDI_SYMPHONIC					0x4400
#define XGMIDI_PHASER1						0x4800
#define XGMIDI_PHASER2						0x4808

/* variation type */
#define XGMIDI_NO_EFFECT					0x0000
#define XGMIDI_DELAY_LCR					0x0500
#define XGMIDI_DELAY_LR						0x0600
#define XGMIDI_ECHO						0x0700
#define XGMIDI_X_DELAY						0x0800
#define XGMIDI_ER1						0x0900
#define XGMIDI_ER2						0x0901
#define XGMIDI_GATED_REVERB					0x0a00
#define XGMIDI_REVERSE_GATE					0x0b00
#define XGMIDI_KARAOKE1						0x1400
#define XGMIDI_KARAOKE2						0x1401
#define XGMIDI_KARAOKE3						0x1402
#define XGMIDI_THRU						0x4000
#define XGMIDI_ENSEMBLE_DETUNE					0x5700
#define XGMIDI_AMBIENCE						0x5800
#define XGMIDI_ROTARY_SPEAKER					0x4500
#define XGMIDI_ROTARY_SPEAKER_2_WAY				0x5600
#define XGMIDI_TREMOLO						0x4600
#define XGMIDI_AUTO_PAN						0x4700
#define XGMIDI_DISTORTION					0x4900
#define XGMIDI_OVER_DRIVE					0x4a00
#define XGMIDI_AMP_SIMULATOR					0x4b00
#define XGMIDI_3_BAND_EQ					0x4c00
#define XGMIDI_2_BAND_EQ					0x4d00
#define XGMIDI_AUTO_WAH						0x4e00
#define XGMIDI_AUTO_WAH_DISTORTION				0x4e01
#define XGMIDI_AUTO_WAH_OVER_DRIVE				0x4e02
#define XGMIDI_PITCH_CHANGE1					0x5000
#define XGMIDI_PITCH_CHANGE2					0x5001
#define XGMIDI_AURAL_ENHANCER					0x5100
#define XGMIDI_TOUCH_WAH1					0x5200
#define XGMIDI_TOUCH_WAH2					0x5201
#define XGMIDI_TOUCH_WAH_DISTORTION				0x5202
#define XGMIDI_TOUCH_WAH_OVER_DRIVE				0x5203
#define XGMIDI_COMPRESSOR					0x5300
#define XGMIDI_NOISE_GATE					0x5400
#define XGMIDI_VOICE_CANCEL					0x5500
#define XGMIDI_TALKING_SIMULATOR				0x5d00
#define XGMIDI_LO_FI						0x5e00
#define XGMIDI_DISTORTION_DELAY					0x5f00
#define XGMIDI_OVER_DRIVE_DELAY					0x5f01
#define XGMIDI_COMPRESSOR_DISTORTION_DELAY			0x6000
#define XGMIDI_COMPRESSOR_OVER_DRIVE_DELAY			0x6001
#define XGMIDI_WAH_DISTORTION_DELAY				0x6100
#define XGMIDI_WAH_OVER_DRIVE_DELAY				0x6101

/* A/D part, addr_mid = channel no. */

/* Multi part */
#define XGMIDI_BANK_SELECT_MSB					0x01
#define XGMIDI_BANK_SELECT_LSB					0x02
#define XGMIDI_PROGRAM_NUMBER					0x03
#define XGMIDI_RECV_CHANNEL					0x04
#define XGMIDI_MONO_POLY_MODE					0x05
#define XGMIDI_KEY_ON_ASSIGN					0x06
#define XGMIDI_PART_MODE					0x07
#define XGMIDI_NOTE_SHIFT					0x08
#define XGMIDI_DETUNE						0x09
#define XGMIDI_VOLUME						0x0b
#define XGMIDI_VELOCITY_SENSE_DEPTH				0x0c
#define XGMIDI_VELOCITY_SENSE_OFFSET				0x0d
#define XGMIDI_PAN						0x0e
#define XGMIDI_NOTE_LIMIT_LOW					0x0f
#define XGMIDI_NOTE_LIMIT_HIGH					0x10
#define XGMIDI_DRY_LEVEL					0x11
#define XGMIDI_CHORUS_SEND					0x12
#define XGMIDI_REVERB_SEND					0x13
#define XGMIDI_VARIATION_SEND					0x14
#define XGMIDI_VIBRATO_RATE					0x15
#define XGMIDI_VIBRATO_DEPTH					0x16
#define XGMIDI_VIBRATO_DELAY					0x17
#define XGMIDI_FILTER_CUTOFF_FREQUENCY				0x18
#define XGMIDI_FILTER_RESONANCE					0x19
#define XGMIDI_EG_ATTACK_TIME					0x1a
#define XGMIDI_EG_DECAY_TIME					0x1b
#define XGMIDI_EG_RELEASE_TIME					0x1c
#define XGMIDI_MW_PITCH_CONTROL					0x1d
#define XGMIDI_MW_FILTER_CONTROL				0x1e
#define XGMIDI_MW_AMPLITUDE_CONTROL				0x1f
#define XGMIDI_MW_LFO_PMOD_DEPTH				0x20
#define XGMIDI_MW_LFO_FMOD_DEPTH				0x21
#define XGMIDI_MW_LFO_AMOD_DEPTH				0x22
#define XGMIDI_BEND_PITCH_CONTROL				0x23
#define XGMIDI_BEND_FILTER_CONTROL				0x24
#define XGMIDI_BEND_AMPLITUDE_CONTROL				0x25
#define XGMIDI_BEND_LFO_PMOD_DEPTH				0x26
#define XGMIDI_BEND_LFO_FMOD_DEPTH				0x27
#define XGMIDI_BEND_LFO_AMOD_DEPTH				0x28

} // namespace aax

