/*
 * Copyright (C) 2018 by Erik Hofman.
 * Copyright (C) 2018 by Adalin B.V.
 *
 * This file is part of AeonWave
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
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef AEONWAVE_INSTRUMENT_HPP
#define AEONWAVE_INSTRUMENT_HPP 1

#include <map>

#include <aax/aeonwave.hpp>

namespace aax
{

class Note : public Emitter
{
private:
    Note(const Note&) = delete;

    Note& operator=(const Note&) = delete;

public:
    Note(float p) : Emitter(AAX_RELATIVE) {
        Emitter::matrix(mtx);
        pitch_param = pitch = p;
        tie(pitch_param, AAX_PITCH_EFFECT, AAX_PITCH);
        tie(gain_param, AAX_VOLUME_FILTER, AAX_GAIN);
    }

    friend void swap(Note& n1, Note& n2) noexcept {
        std::swap(static_cast<Emitter&>(n1), static_cast<Emitter&>(n2));
        n1.pitch_param = std::move(n2.pitch_param);
        n1.gain_param = std::move(n2.gain_param);
        n1.pitch = std::move(n2.pitch);
        n2.playing = std::move(n2.playing);
    }

    Note& operator=(Note&&) = default;

    operator Emitter&() {
        return *this;
    }

    bool play(float g) {
        hold = false;
        gain_param = gain = g;
        Emitter::set(AAX_INITIALIZED);
        if (!playing) playing = Emitter::set(AAX_PLAYING);
        return playing;
    }

    bool stop(float g = 1.0f) {
        playing = false;
        if (fabsf(g - gain) > 0.1f) gain_param = gain = g;
        return hold ? true : Emitter::set(AAX_STOPPED);
    }

    // notes hold until hold becomes false, even after a stop message
    void set_hold(bool h) {
        if (hold && !h) Emitter::set(AAX_STOPPED);
        hold = h;
    }

    // only notes started before this command should hold until stop arrives
    void set_sustain(bool s) {
        hold = s;
    }

    bool buffer(Buffer& buffer) {
        Emitter::remove_buffer();
        return Emitter::add(buffer);
    }

    inline void set_gain(float expr) { gain_param = expr*gain; }
    inline void set_pitch(float bend) { pitch_param = bend*pitch; }

private:
    Matrix64 mtx;
    Param pitch_param = 1.0f;
    Param gain_param = 1.0f;
    float pitch = 1.0f;
    float gain = 1.0f;
    bool playing = false;
    bool hold = true;
};


class Instrument : public Mixer
{
private:
    Instrument(const Instrument& i) = delete;

    Instrument& operator=(const Instrument&) = delete;

public:
    Instrument(AeonWave& ptr, bool drums = false)
        : Mixer(ptr), aax(&ptr), is_drums(drums)
    {
        Mixer::tie(modulate_freq, AAX_RINGMODULATOR_EFFECT, AAX_LFO_OFFSET);
        Mixer::tie(modulate_depth, AAX_RINGMODULATOR_EFFECT, AAX_GAIN);
        Mixer::tie(modulate_state, AAX_RINGMODULATOR_EFFECT);
        Mixer::matrix(mtx);
        Mixer::set(AAX_POSITION, AAX_RELATIVE);
        Mixer::set(AAX_PLAYING);
        if (is_drums) {
            Mixer::set(AAX_MONO_EMITTERS, 10);
        }
    }

    friend void swap(Instrument& i1, Instrument& i2) noexcept {
        i1.key = std::move(i2.key);
        i1.aax = std::move(i2.aax);
        i1.playing = std::move(i2.playing);
    }

    Instrument& operator=(Instrument&&) = default;

    operator Mixer&() {
        return *this;
    }

    void play(uint8_t key_no, uint8_t velocity, Buffer& buffer) {
        auto it = key.find(key_no);
        if (it == key.end()) {
            float pitch = 1.0f;
            float frequency = buffer.get(AAX_UPDATE_RATE);
            if (!is_drums) pitch = note2freq(key_no)/(float)frequency;
            auto ret = key.insert({key_no, new Note(pitch)});
            it = ret.first;
            if (!playing && !is_drums) {
                Mixer::add(buffer);
                playing = true;
            }
            it->second->buffer(buffer);
        }
        Mixer::add(*it->second);
        float g = 3.321928f*log10f(1.0f+(1+velocity)/128.0f);
        it->second->play(volume*g*soft);
    }

    void stop(uint8_t key_no, uint8_t velocity) {
        auto it = key.find(key_no);
        if (it != key.end()) {
            float g = 0.25f + 0.75f*2*velocity/128.0f;
            it->second->stop(volume*g*soft);
        }
    }

    inline void set_pitch(float pitch) {
        for (auto& it : key) {
            it.second->set_pitch(pitch);
        }
    }

    void set_pitch(uint8_t key_no, float pitch) {
        auto it = key.find(key_no);
        if (it != key.end()) {
            it->second->set_pitch(pitch);
        }
    }

    inline void set_gain(float v) { volume = v; }

    inline void set_pressure(float p) { pressure = p; }
    void set_pressure(uint8_t key_no, float p) {
        auto it = key.find(key_no);
        if (it != key.end()) {
            it->second->set_gain(p*pressure*soft*volume);
        }
    }

    inline void set_expression(float e) {
        for (auto& it : key) {
            it.second->set_gain(e*pressure*soft*volume);
        }
    }

    inline void set_pan(float p) {
        Matrix64 m;
        m.rotate(1.57*p, 0.0, 1.0, 0.0);
        m.multiply(mtx);
        Mixer::matrix(m);
    }

    inline void set_soft(bool s) { soft = s ? 0.5f : 1.0f; }

    void set_hold(bool h) {
        for (auto& it : key) {
            it.second->set_hold(h);
        }
    }

    void set_sustain(bool s) {
        for (auto& it : key) {
            it.second->set_sustain(s);
        }
    }

    inline void set_modulation_depth(float d) { modulation_range = d; }

    void set_modulation(float m) {
        bool enabled = (m > 0.05f);
        mdepth = -m*modulation_range;
        if ((enabled && !modulate_state) || (!enabled && modulate_state)) {
            modulate_state = enabled;
        }
        if (modulate_state) {
            modulate_depth = mdepth;
        }
    }

private:
    inline float note2freq(uint8_t d) {
        return 440.0f*powf(2.0f, ((float)d-69.0f)/12.0f);
    }

    std::map<uint8_t,Note*> key;
    AeonWave* aax;

    Vector dir = Vector(0.0f, 0.0f, -1.0f);
    Vector64 pos = Vector64(0.0, 1.0, -2.75);
    Matrix64 mtx = Matrix64(pos, dir);

    Param modulate_freq = 1.5f;
    Param modulate_depth = 0.0f;
    Status modulate_state = AAX_FALSE;

    float mfreq = 1.5f;
    float mdepth = 0.0f;

    bool is_drums;
    float soft = 1.0f;
    float volume = 1.0f;
    float pressure = 1.0f;
    float modulation_range = 1.0f;
    bool playing = false;
};

} // namespace aax

#endif

