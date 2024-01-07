/*
 * Copyright (C) 2018-2024 by Erik Hofman.
 * Copyright (C) 2018-2024 by Adalin B.V.
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

#include <sys/stat.h>
#include <climits>

#include <map>
#include <chrono>

#include <midi/shared.hpp>

#include "base/types.h"

namespace aax
{

class MIDIEnsemble;


enum {
    MIDI_POLYPHONIC = 3,
    MIDI_MONOPHONIC
};

struct info_t
{
    info_t() = default;
    ~info_t() = default;

    std::string name;
    std::string file;
    std::string key_on;
    std::string key_off;

    float gain = 1.0f;
    float pitch = 1.0f;

    float spread = 1.0f;
    int wide = 0;

    int min_key = 0;
    int max_key = 128;

    bool stereo = false;
};

class MIDIDriver : public AeonWave
{
private:
    using configuration_map_t = std::map<int, info_t>;
    using patch_map_t = std::map<uint16_t, configuration_map_t>;
    using channel_map_t = std::map<uint16_t, std::shared_ptr<MIDIEnsemble>>;

public:
    MIDIDriver(const char* n, const char *tnames = nullptr,
         enum aaxRenderMode m=AAX_MODE_WRITE_STEREO);
    MIDIDriver(const char* n, enum aaxRenderMode m=AAX_MODE_WRITE_STEREO) :
        MIDIDriver(n, nullptr, m) {}

    virtual ~MIDIDriver() {
        AeonWave::remove(delay);
        AeonWave::remove(reverb);
    }

    bool process(uint8_t channel, uint8_t message, uint8_t key, uint8_t velocity, bool omni);

    MIDIEnsemble& new_channel(uint8_t channel, uint16_t bank, uint8_t program);

    MIDIEnsemble& channel(uint16_t channel_no);

    channel_map_t& get_channels() {
        return channels;
    }

    void set_drum_file(std::string p) { drum = p; }
    void set_instrument_file(std::string p) { instr = p; }
    void set_file_path(std::string p) {
        set(AAX_SHARED_DATA_DIR, p.c_str()); path = p;
    }

    const std::string& get_patch_set() { return patch_set; }
    const std::string& get_patch_version() { return patch_version; }
    std::vector<std::string>& get_selections() { return selection; }

    void set_track_active(uint16_t t) {
        active_track.push_back(t);
    }
    uint16_t no_active_tracks() { return active_track.size(); }
    bool is_track_active(uint16_t t) {
        return active_track.empty() ? true : is_avail(active_track, t);
    }

    void read_instruments(std::string gmidi=std::string(), std::string gmdrums=std::string());

    void grep(std::string& filename, const char *grep);
    void load(std::string& name) { loaded.push_back(name); }
    bool is_loaded(std::string& name) {
        return (std::find(loaded.begin(), loaded.end(), name) != loaded.end());
    }

    void start();
    void stop();
    void rewind();

    void finish(uint8_t n);
    bool finished(uint8_t n);

    void set_volume(float);
    float get_volume();

    void set_balance(float);

    bool is_drums(uint8_t);

    void set_capabilities(enum aaxCapabilities m) {
        instrument_mode = m; set(AAX_CAPABILITIES, m); set_path();
    }

    std::string& get_effects() { return effects; }

    int get_refresh_rate() { return refresh_rate; }
    int get_polyphony() { return polyphony; }

    void set_tuning_coarse(float semi_tones) { tuning_coarse = semi_tones; }
    float get_tuning_coarse() { return tuning_coarse; }

    void set_tuning_fine(float cents) { tuning_fine = cents; }
    float get_tuning_fine() { return tuning_fine; }

    void set_mode(uint8_t m) { if (m > mode) mode = m; }
    uint8_t get_mode() { return mode; }

    void set_grep(bool g) { grep_mode = g; }
    bool get_grep() { return grep_mode; }

    const info_t get_drum(uint16_t bank, uint16_t& program, uint8_t key, bool all=false);
    const info_t get_instrument(uint16_t bank, uint8_t program, bool all=false);
    configuration_map_t& get_configurations() { return configuration_map; }

    void set_initialize(bool i) { initialize = i; };
    bool get_initialize() { return initialize; }

    void set_mono(bool m) { mono = m; }
    bool get_mono() { return mono; }

    void set_verbose(char v) { verbose = v; }
    char get_verbose() { return csv ? 0 : verbose; }

    void set_csv(char v) { csv = v; }
    char get_csv(signed char t = -1) {
        return (t == -1) ? csv : (csv && is_track_active(t));
    }

    void set_lyrics(bool v) { lyrics = v; }
    bool get_lyrics() { return lyrics; }
    void set_display(bool v) { display = v; }
    bool get_display() { return display; }

    void set_format(uint16_t fmt) { format = fmt; }
    uint16_t get_format() { return format; }

    void set_tempo(uint32_t t) { tempo = t; uSPP = t/PPQN; }

    void set_uspp(uint32_t uspp) { uSPP = uspp; }
    int32_t get_uspp() { return uSPP; }

    void set_ppqn(uint16_t ppqn) { PPQN = ppqn; }
    uint16_t get_ppqn() { return PPQN; }

    /* chorus */
    bool set_chorus(const char *t, uint16_t type = -1, uint8_t vendor = 0);
    void set_chorus_delay(float delay);
    void set_chorus_depth(float depth);
    void set_chorus_feedback(float feedback);
    void set_chorus_rate(float rate);
    void set_chorus_level(uint16_t part_no, float lvl);
    void send_chorus_to_reverb(float val);
    void set_chorus_cutoff_frequency(float fc);
    void set_chorus_level(float value);

    void set_chorus_type(uint16_t value) { chorus_type = value; }
    uint32_t get_chorus_type() { return chorus_type; }

    void set_gm2_chorus_type(uint16_t value);

    /* delay */
    bool set_delay(const char *t, uint16_t type = -1, uint8_t vendor = 0);
    void set_delay_delay(float delay);
    void set_delay_depth(float depth);
    void set_delay_feedback(float feedback);
    void set_delay_rate(float rate);
    void set_delay_level(uint16_t part_no, float lvl);
    void send_delay_to_reverb(float val);
    void set_delay_cutoff_frequency(float fc);
    void set_delay_level(float value);

    void set_delay_type(uint16_t value) { delay_type = value; }
    uint16_t get_delay_type() { return delay_type; }

    /* reverb */
    bool set_reverb(const char *t, uint16_t type = -1, uint8_t vendor = 0);
    void set_reverb_cutoff_frequency(float value);
    void set_reverb_decay_depth(float value);
    void set_reverb_time_rt60(float value);
    void set_reverb_delay_depth(float value);
    void set_reverb_decay_level(float value) { reverb_decay_level = value; }
    void set_reverb_level(uint16_t part_no, float value);
    void set_reverb_level(float value);

    void set_reverb_type(uint16_t type) { reverb_type = type; }
    uint16_t get_reverb_type() { return reverb_type; }

    void set_gm2_reverb_type(uint16_t value);

    bool exists(const std::string& path) {
        struct stat buffer;
        return (stat(path.c_str(), &buffer) == 0);
    }

    std::string get_channel_type(uint16_t);
    std::string get_channel_name(uint16_t);

    MIDIDriver &midi = *this;
    int capabilities = midi.get(AAX_CAPABILITIES);
    int cores = (capabilities & AAX_CPU_CORES)+1;
    int midi_mode = (capabilities & AAX_RENDER_MASK);
    int simd64 = (capabilities & AAX_SIMD256);
    int simd = (capabilities & AAX_SIMD);

    void reset_timer() {
        start_time = std::chrono::system_clock::now();
        timer_started = true;
    }
    bool elapsed_time(double dt);

private:
    void set_path();

    std::string preset_file(aaxConfig c, std::string& name) {
        std::string rv = midi.info(AAX_SHARED_DATA_DIR);
        rv.append("/"); rv.append(name);
        return rv;
    }

    std::string aaxs_file(aaxConfig c, std::string& name) {
        std::string rv = midi.info(AAX_SHARED_DATA_DIR);
        rv.append("/"); rv.append(name); rv.append(".aaxs");
        return rv;
    }

    std::string patch_set = "default";
    std::string patch_version = "1.0.0";

    std::string effects;
    std::string track_name;
    channel_map_t channels;
    channel_map_t chorus_channels;
    channel_map_t delay_channels;
    channel_map_t reverb_channels;

    // banks name and submixer filter and effects file
    configuration_map_t configuration_map;

    patch_map_t drum_map;
    patch_map_t instrument_map;

    std::vector<uint16_t> missing_drum_bank;
    std::vector<uint16_t> missing_instrument_bank;

    bool is_avail(std::vector<uint16_t>& vec, uint16_t item) {
        return (std::find(vec.begin(), vec.end(), item) != vec.end());
    }

    std::vector<std::string> loaded;

    std::vector<std::string> selection;
    std::vector<uint16_t> active_track;

    std::string instr = "gmmidi.xml";
    std::string drum = "gmdrums.xml";
    std::string path;

    float volume = 1.0f;

    float tuning_coarse = 0.0f;
    float tuning_fine = 0.0f;

    int refresh_rate = 0;
    int polyphony = UINT_MAX;

    int16_t drum_set_no = -1;
    uint16_t PPQN = 24;
    uint32_t tempo = 500000;
    uint32_t uSPP = tempo/PPQN;
    uint16_t format = 0;

    enum aaxCapabilities instrument_mode = AAX_RENDER_NORMAL;
    uint8_t mode = MIDI_MODE0;
    bool initialize = false;
    char verbose = 0;
    bool lyrics = false;
    bool display = false;
    bool grep_mode = false;
    bool mono = false;
    bool csv = false;

    uint32_t chorus_type = (GM2<<16)|GM2_CHORUS3;
    Param chorus_rate = 0.4f;
    Param chorus_level = 0.5f;
    Param chorus_feedback = 0.06f;
    Param chorus_depth = Param(6300.0f, AAX_MICROSECONDS);
    Status chorus_state = AAX_FALSE;
    aax::Mixer chorus = aax::Mixer(*this);
    aax::Buffer* chorus_buffer = &AeonWave::buffer("GM2/chorus3");
    float chorus_to_reverb = 0.0f;

    uint32_t delay_type = (GS<<16)|GSMIDI_DELAY1;
    Param delay_rate = 0.0f;
    Param delay_level = 0.5f;
    Param delay_feedback = 0.25f;
    Param delay_depth = Param(340000.0f, AAX_MICROSECONDS);
    Status delay_state = AAX_FALSE;
    aax::Mixer delay = aax::Mixer(*this);
    aax::Buffer* delay_buffer = &AeonWave::buffer("GM2/delay0");
    float delay_to_reverb = 0.0f;

    uint32_t reverb_type = (GM2<<16)|GM2_REVERB_CONCERTHALL_LARGE;
    float reverb_time = 0.0f;
    Param reverb_decay_level = 0.66f;
    Param reverb_decay_depth = 0.3f;
    Param reverb_cutoff_frequency = 790.0f;
    Status reverb_state = AAX_FALSE;
    aax::Mixer reverb = aax::Mixer(*this);
    aax::Buffer* reverb_buffer = &AeonWave::buffer("GM2/hall2");

    static const std::vector<std::string> midi_channel_convention;

    bool timer_started = false;
    std::chrono::time_point<std::chrono::system_clock> start_time;
};

} // namespace aax

