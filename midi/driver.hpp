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

#ifndef __AAX_MIDI_DRIVER
#define __AAX_MIDI_DRIVER

#include <sys/stat.h>
#include <climits>

#include <map>

#include <midi/shared.hpp>

#include "base/types.h"

namespace aax
{

enum {
    MIDI_POLYPHONIC = 3,
    MIDI_MONOPHONIC
};

struct wide_t
{
    wide_t() = default;
    wide_t(int w, float s) : wide(w), spread(s) {};
    wide_t(int w, float s, bool st) : wide(w), spread(s), stereo(st) {};

    ~wide_t() = default;

    int wide = 0;
    float spread = 1.0f;
    bool stereo = false;
};

struct patch_t
{
   std::string name;
   std::string file;
   std::string key_on;
   std::string key_off;
};

using inst_t = std::pair<struct patch_t, struct wide_t>;

class MIDIInstrument;


class MIDIDriver : public AeonWave
{
private:
    using _patch_map_t = std::map<uint8_t,std::pair<uint8_t,std::string>>;
    using _channel_map_t = std::map<uint16_t,std::shared_ptr<MIDIInstrument>>;

public:
    MIDIDriver(const char* n, const char *tnames = nullptr,
         enum aaxRenderMode m=AAX_MODE_WRITE_STEREO);
    MIDIDriver(const char* n, enum aaxRenderMode m=AAX_MODE_WRITE_STEREO) :
        MIDIDriver(n, nullptr, m) {}

    virtual ~MIDIDriver() {
        AeonWave::remove(delay);
        AeonWave::remove(reverb);
        for (auto it : buffers) {
            aaxBufferDestroy(*it.second.second); it.second.first = 0;
        }
    }

    bool process(uint8_t channel, uint8_t message, uint8_t key, uint8_t velocity, bool omni, float pitch=1.0f);

    MIDIInstrument& new_channel(uint8_t channel, uint16_t bank, uint8_t program);

    MIDIInstrument& channel(uint16_t channel_no);

    inline _channel_map_t& get_channels() {
        return channels;
    }

    inline void set_drum_file(std::string p) { drum = p; }
    inline void set_instrument_file(std::string p) { instr = p; }
    inline void set_file_path(std::string p) {
        set(AAX_SHARED_DATA_DIR, p.c_str()); path = p;
    }

    inline const std::string& get_patch_set() { return patch_set; }
    inline const std::string& get_patch_version() { return patch_version; }
    inline std::vector<std::string>& get_selections() { return selection; }

    inline void set_track_active(uint16_t t) {
        active_track.push_back(t);
    }
    inline uint16_t no_active_tracks() { return active_track.size(); }
    inline bool is_track_active(uint16_t t) {
        return active_track.empty() ? true : is_avail(active_track, t);
    }

    void read_instruments(std::string gmidi=std::string(), std::string gmdrums=std::string());

    void grep(std::string& filename, const char *grep);
    inline void load(std::string& name) { loaded.push_back(name); }
    inline bool is_loaded(std::string& name) {
        return (std::find(loaded.begin(), loaded.end(), name) != loaded.end());
    }

    void start();
    void stop();
    void rewind();

    void finish(uint8_t n);
    bool finished(uint8_t n);

    void set_gain(float);
    void set_balance(float);

    bool is_drums(uint8_t);

    inline void set_capabilities(enum aaxCapabilities m) {
        instrument_mode = m; set(AAX_CAPABILITIES, m); set_path();
    }

    inline std::string& get_effects() { return effects; }

    inline unsigned int get_refresh_rate() { return refresh_rate; }
    inline unsigned int get_polyphony() { return polyphony; }

    inline void set_tuning(float pitch) { tuning = powf(2.0f, pitch/12.0f); }
    inline float get_tuning() { return tuning; }

    inline void set_mode(uint8_t m) { if (m > mode) mode = m; }
    inline uint8_t get_mode() { return mode; }

    inline void set_grep(bool g) { grep_mode = g; }
    inline bool get_grep() { return grep_mode; }

    const inst_t get_drum(uint16_t bank, uint16_t& program, uint8_t key, bool all=false);
    const inst_t get_instrument(uint16_t bank, uint8_t program, bool all=false);
    std::map<uint16_t,patch_t>& get_frames() { return frames; }
    std::map<std::string,_patch_map_t>& get_patches() { return patches; }

    std::pair<uint8_t,std::string> get_patch(std::string& name, uint8_t& key);
    std::pair<uint8_t,std::string> get_patch(uint16_t bank_no, uint8_t program_no, uint8_t& key) {
        auto inst = get_instrument(bank_no, program_no, no_active_tracks() > 0);
        return get_patch(inst.first.file, key);
    }

    inline void set_initialize(bool i) { initialize = i; };
    inline bool get_initialize() { return initialize; }

    inline void set_mono(bool m) { mono = m; }
    inline bool get_mono() { return mono; }

    inline void set_verbose(char v) { verbose = v; }
    inline char get_verbose() { return csv ? 0 : verbose; }

    inline void set_csv(char v) { csv = v; }
    inline char get_csv(signed char t = -1) {
        return (t == -1) ? csv : (csv && is_track_active(t));
    }

    inline void set_lyrics(bool v) { lyrics = v; }
    inline bool get_lyrics() { return lyrics; }

    inline void set_format(uint16_t fmt) { format = fmt; }
    inline uint16_t get_format() { return format; }

    inline void set_tempo(uint32_t t) { tempo = t; uSPP = t/PPQN; }

    inline void set_uspp(uint32_t uspp) { uSPP = uspp; }
    inline int32_t get_uspp() { return uSPP; }

    inline void set_ppqn(uint16_t ppqn) { PPQN = ppqn; }
    inline uint16_t get_ppqn() { return PPQN; }

    /* chorus */
    void set_chorus(const char *t);
    void set_chorus_type(uint8_t value);
    void set_chorus_delay(float delay);
    void set_chorus_depth(float depth);
    void set_chorus_feedback(float feedback);
    void set_chorus_rate(float rate);
    void set_chorus_level(uint16_t part_no, float lvl);
    void send_chorus_to_reverb(float val);
    void set_chorus_cutoff_frequency(float fc);
    void set_chorus_level(float value);

    /* delay */
    void set_delay(const char *t);
    void set_delay_type(uint8_t value);
    void set_delay_delay(float delay);
    void set_delay_depth(float depth);
    void set_delay_feedback(float feedback);
    void set_delay_rate(float rate);
    void set_delay_level(uint16_t part_no, float lvl);
    void send_delay_to_reverb(float val);
    void set_delay_cutoff_frequency(float fc);
    void set_delay_level(float value);

    /* reverb */
    void set_reverb(const char *t);
    void set_reverb_type(uint8_t value);
    void set_reverb_cutoff_frequency(float value);
    void set_reverb_decay_depth(float value);
    void set_reverb_time_rt60(float value);
    void set_reverb_delay_depth(float value);
    void set_reverb_decay_level(float value) { reverb_decay_level = value; }
    void set_reverb_level(uint16_t part_no, float value);
    void set_reverb_level(float value);

    // ** buffer management ******
    Buffer& buffer(std::string& name, int level=0) {
        if (level) { name = name + "?patch=" + std::to_string(level); }
        auto it = buffers.find(name);
        if (it == buffers.end()) {
            std::shared_ptr<Buffer> b(new Buffer(ptr,name.c_str(),false,true));
            if (b) {
                auto ret = buffers.insert({name,{0,b}});
                it = ret.first;
            } else {
                return nullBuffer;
            }
        }
        it->second.first++;
        return *it->second.second;
    }
    void destroy(Buffer& b) {
        for(auto it=buffers.begin(); it!=buffers.end(); ++it)
        {
            if ((*it->second.second == b) && it->second.first && !(--it->second.first)) {
                aaxBufferDestroy(*it->second.second);
                buffers.erase(it); break;
            }
        }
    }
    bool buffer_avail(std::string &name) {
        auto it = buffers.find(name);
        if (it == buffers.end()) return false;
        return true;
    }

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
private:
    float _ln(float v) { return powf(v, GMATH_1_E1); }

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

    void add_patch(const char *patch);

    std::string patch_set = "default";
    std::string patch_version = "1.0.0";

    std::string effects;
    std::string track_name;
    _channel_map_t channels;
    _channel_map_t chorus_channels;
    _channel_map_t delay_channels;
    _channel_map_t reverb_channels;

    // banks name and audio-frame filter and effects file
    std::map<uint16_t,patch_t> frames;

    std::map<uint16_t,std::map<uint16_t,inst_t>> drums;
    std::map<uint16_t,std::map<uint16_t,inst_t>> instruments;
    std::map<std::string,_patch_map_t> patches;

    std::vector<uint16_t> missing_drum_bank;
    std::vector<uint16_t> missing_instrument_bank;

    inline bool is_avail(std::vector<uint16_t>& vec, uint16_t item) {
        return (std::find(vec.begin(), vec.end(), item) != vec.end());
    }

    std::unordered_map<std::string,std::pair<size_t,std::shared_ptr<Buffer>>> buffers;

    Buffer nullBuffer;

    std::vector<std::string> loaded;

    std::vector<std::string> selection;
    std::vector<uint16_t> active_track;

    std::string instr = "gmmidi.xml";
    std::string drum = "gmdrums.xml";
    std::string path;

    float tuning = 1.0f;

    unsigned int refresh_rate = 0;
    unsigned int polyphony = UINT_MAX;

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
    bool grep_mode = false;
    bool mono = false;
    bool csv = false;

    uint8_t chorus_type = 2;
    Param chorus_rate = 0.4f;
    Param chorus_level = _ln(0.5f);
    Param chorus_feedback = 0.06f;
    Param chorus_depth = Param(6300.0f, AAX_MICROSECONDS);
    Status chorus_state = AAX_FALSE;
    aax::Mixer chorus = aax::Mixer(*this);
    aax::Buffer* chorus_buffer = &AeonWave::buffer("GM2/chorus0");
    float chorus_to_reverb = 0.0f;

    Param delay_rate = 0.0f;
    Param delay_level = _ln(0.5f);
    Param delay_feedback = 0.25f;
    Param delay_depth = Param(340000.0f, AAX_MICROSECONDS);
    Status delay_state = AAX_FALSE;
    aax::Mixer delay = aax::Mixer(*this);
    aax::Buffer* delay_buffer = &AeonWave::buffer("GM2/delay0");
    float delay_to_reverb = 0.0f;

    uint8_t reverb_type = 4;
    float reverb_time = 0.0f;
    Param reverb_decay_level = _ln(0.66f);
    Param reverb_decay_depth = 0.3f;
    Param reverb_cutoff_frequency = 790.0f;
    Status reverb_state = AAX_FALSE;
    aax::Mixer reverb = aax::Mixer(*this);
    aax::Buffer* reverb_buffer = &AeonWave::buffer("GM2/room0");

    static const std::vector<std::string> midi_channel_convention;
};

} // namespace aax


#endif
