/*
 * Copyright (C) 2022 by Erik Hofman.
 * Copyright (C) 2022 by Adalin B.V.
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

#include <aax/midi.h>
#include <midi/file.hpp>

using namespace aax;

MIDI::MIDI(const char *devname, const char *filename,
          const char *selection, enum aaxRenderMode mode,
          const char *config)
   : midi(new MIDIFile(devname, filename, selection, mode, config))
{
}

MIDI::~MIDI()
{
}

void
MIDI::start()
{
   midi->start();
}

void MIDI::stop()
{
   midi->stop();
}

void MIDI::rewind()
{
   midi->rewind();
}

void
MIDI::initialize(const char *grep)
{
    midi->initialize(grep);
}

bool
MIDI::process(uint64_t time_parts, uint32_t& next)
{
    return midi->process(time_parts, next);
}

bool
MIDI::wait(float t)
{
    return midi->wait(t);
}

bool
MIDI::set(enum aaxSetupType t, const char* s)
{
    return midi->set(t, s);
}

bool
MIDI::set(enum aaxSetupType t, unsigned int s)
{
    return midi->set(t, s);
}

bool
MIDI::set(enum aaxState s)
{
    return midi->set(s);
}

unsigned int
MIDI::get(enum aaxSetupType t)
{
    return midi->get(t);
}

float
MIDI::get_pos_sec()
{
    return midi->get_pos_sec();
}

int32_t
MIDI::get_uspp()
{
    return midi->get_uspp();
}

bool
MIDI::add(Sensor& s)
{
    return midi->add(s);
}

bool
MIDI::sensor(enum aaxState s)
{
    return midi->sensor(s);
}

Buffer
MIDI::get_buffer()
{
    return midi->get_buffer();
}

void
MIDI::set_mono(bool m)
{
    midi->set_mono(m);
}

void
MIDI::set_verbose(char v)
{
    midi->set_verbose(v);
}

void
MIDI::set_csv(char v)
{
    midi->set_csv(v);
}
