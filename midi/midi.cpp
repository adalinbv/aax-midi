/*
 * Copyright (C) 2022-2023 by Erik Hofman.
 * Copyright (C) 2022-2023 by Adalin B.V.
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
   : file(new MIDIFile(devname, filename, selection, mode, config))
{
}

MIDI::~MIDI()
{
}

void
MIDI::start()
{
   file->start();
}

void MIDI::stop()
{
   file->stop();
}

void MIDI::rewind()
{
   file->rewind();
}

void
MIDI::initialize(const char *grep)
{
    file->initialize(grep);
}

bool
MIDI::process(uint64_t time_parts, uint32_t& next)
{
    return file->process(time_parts, next);
}

bool
MIDI::wait(float t)
{
    return file->wait(t);
}

void
MIDI::set_volume(float g) {
    file->set_volume(100.0f*g/127.0f);
}

bool
MIDI::set(enum aaxSetupType t, const char* s)
{
    return file->set(t, s);
}

#if 0
bool
MIDI::set(enum aaxSetupType t, float s)
{
    return file->set(t, int(s));
}
#endif

bool
MIDI::set(enum aaxSetupType t, int64_t s)
{
    return file->set(t, s);
}

bool
MIDI::set(enum aaxState s)
{
    return file->set(s);
}

int64_t
MIDI::get(enum aaxSetupType t)
{
    return file->get(t);
}

float
MIDI::getf(enum aaxSetupType t)
{
    return file->getf(t);
}

float
MIDI::get_pos_sec()
{
    return file->get_pos_sec();
}

int32_t
MIDI::get_uspp()
{
    return file->get_uspp();
}

bool
MIDI::add(Sensor& s)
{
    return file->add(s);
}

bool
MIDI::sensor(enum aaxState s)
{
    return file->sensor(s);
}

Buffer
MIDI::get_buffer()
{
    return file->get_buffer();
}

void
MIDI::set_mono(bool m)
{
    file->set_mono(m);
}

void
MIDI::set_verbose(char v)
{
    file->set_verbose(v);
}

void
MIDI::set_csv(char v)
{
    file->set_csv(v);
}

// -----------------------------------------------------------------------
// C API
// -----------------------------------------------------------------------

// track and config may be NULL
aaxMIDI*
aaxMIDICreate(const char *devname, const char *filename, const char *track, enum aaxRenderMode mode, const char *config)
{
    return reinterpret_cast<aaxMIDI*>(new MIDI(devname, filename, track, mode, config));
}

void
aaxMIDIDesrtroy(aaxMIDI *handle)
{
    delete reinterpret_cast<MIDI*>(handle);
}

void
aaxMIDIStart(aaxMIDI *handle)
{
    reinterpret_cast<MIDI*>(handle)->start();
}

void
aaxMIDIStop(aaxMIDI *handle)
{
    reinterpret_cast<MIDI*>(handle)->stop();
}

void
aaxMIDIRewind(aaxMIDI *handle)
{
    reinterpret_cast<MIDI*>(handle)->rewind();
}

void
aaxMIDIInitialize(aaxMIDI *handle, const char *grep)
{
    reinterpret_cast<MIDI*>(handle)->initialize(grep);
}

int
aaxMIDIProcess(aaxMIDI *handle, uint64_t time_parts, uint32_t* next)
{
    uint32_t ntime = *next;
    int rv = reinterpret_cast<MIDI*>(handle)->process(time_parts, ntime);
    *next = ntime;
    return rv;
}

int
aaxMIDIWait(aaxMIDI *handle, float t)
{
    return reinterpret_cast<MIDI*>(handle)->wait(t);
}

int
aaxMIDISetSetupString(aaxMIDI *handle, enum aaxSetupType t, const char* s)
{
    return reinterpret_cast<MIDI*>(handle)->set(t, s);
}

int
aaxMIDISetSetup(aaxMIDI *handle, enum aaxSetupType t, int64_t s)
{
    return reinterpret_cast<MIDI*>(handle)->set(t, s);
}

int
aaxMIDISetState(aaxMIDI *handle, enum aaxState s)
{
    return reinterpret_cast<MIDI*>(handle)->set(s);
}

int64_t
aaxMIDIGetSetup(aaxMIDI *handle, enum aaxSetupType t)
{
    return reinterpret_cast<MIDI*>(handle)->get(t);
}

float
aaxMIDIGetPosSec(aaxMIDI *handle)
{
    return reinterpret_cast<MIDI*>(handle)->get_pos_sec();
}

int32_t
get_uspp(aaxMIDI *handle)
{
    return reinterpret_cast<MIDI*>(handle)->get_uspp();
}

int
aaxMIDIAdd(aaxMIDI *handle, aaxConfig s)
{
    aax::Sensor ssr = aax::Sensor(s);
    return reinterpret_cast<MIDI*>(handle)->add(ssr);
}

int
aaxMIDISensor(aaxMIDI *handle, enum aaxState s)
{
    return reinterpret_cast<MIDI*>(handle)->sensor(s);
}

aaxBuffer
aaxMIDIrGetBuffer(aaxMIDI *handle)
{
    return reinterpret_cast<MIDI*>(handle)->get_buffer();
}

void
aaxMIDISetMono(aaxMIDI *handle, int m)
{
    reinterpret_cast<MIDI*>(handle)->set_mono(m);
}

void
aaxMIDISetVerbose(aaxMIDI *handle, char v)
{
    reinterpret_cast<MIDI*>(handle)->set_verbose(v);
}

void
aaxMIDISetCSV(aaxMIDI *handle, char v)
{
}
