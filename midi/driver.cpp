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

#include <cstring>

#include <regex>
#include <iostream>
#include <cstring>

#include <aax/strings.hpp>
#include <xml.h>

#include <midi/shared.hpp>
#include <midi/file.hpp>
#include <midi/driver.hpp>
#include <midi/instrument.hpp>

using namespace aax;

MIDIDriver::MIDIDriver(const char* n, const char *selections, enum aaxRenderMode m)
        : AeonWave(n, m)
{
    if (*this) {
        set_path();
    }
    else
    {
        if (n) {
            throw(std::runtime_error("Unable to open device '"+std::string(n)+"'"));
        } else {
            throw(std::runtime_error("Unable to open the default device"));
        }
        return;
    }

    if (selections)
    {
        std::string s(selections);
        std::regex regex{R"(,+)"}; // split on a comma
        std::sregex_token_iterator it{s.begin(), s.end(), regex, -1};
        selection = std::vector<std::string>{it, {}};

        for(auto s : selection) {
            uint16_t t = atoi(s.c_str());
            if (t) active_track.push_back(t);
        }
    }

    chorus.tie(chorus_level, AAX_CHORUS_EFFECT, AAX_DELAY_GAIN);
    chorus.tie(chorus_depth, AAX_CHORUS_EFFECT, AAX_LFO_OFFSET);
    chorus.tie(chorus_rate, AAX_CHORUS_EFFECT, AAX_LFO_FREQUENCY);
    chorus.tie(chorus_feedback, AAX_CHORUS_EFFECT, AAX_FEEDBACK_GAIN);
    chorus.tie(chorus_state, AAX_CHORUS_EFFECT);

    delay.tie(delay_level, AAX_DELAY_EFFECT, AAX_DELAY_GAIN);
    delay.tie(delay_depth, AAX_DELAY_EFFECT, AAX_LFO_OFFSET);
    delay.tie(delay_rate, AAX_DELAY_EFFECT, AAX_LFO_FREQUENCY);
    delay.tie(delay_feedback, AAX_DELAY_EFFECT, AAX_FEEDBACK_GAIN);
    delay.tie(delay_state, AAX_DELAY_EFFECT);

    reverb.tie(reverb_decay_level, AAX_REVERB_EFFECT, AAX_DECAY_LEVEL);
    reverb.tie(reverb_decay_depth, AAX_REVERB_EFFECT, AAX_DECAY_DEPTH);
    reverb.tie(reverb_cutoff_frequency, AAX_REVERB_EFFECT, AAX_CUTOFF_FREQUENCY);
    reverb.tie(reverb_state, AAX_REVERB_EFFECT);

    volume = get_volume();
}

void
MIDIDriver::set_path()
{
    path = AeonWave::info(AAX_SHARED_DATA_DIR);

    std::string name = path;
    if (instrument_mode == AAX_RENDER_NORMAL) {
        name.append("/ultrasynth/");
    }
    if (midi.exists(name))
    {
        path = name;
        AeonWave::set(AAX_SHARED_DATA_DIR, path.c_str());
    }
}

void
MIDIDriver::start()
{
    set_gm2_reverb_type(GM2_REVERB_CONCERTHALL_LARGE);
    set_reverb_level(0.0f);

    set_chorus_type(GM2_CHORUS3);
    set_chorus_level(0.0f);

    chorus_state = AAX_SINE;
    chorus.set(AAX_INITIALIZED);
    chorus.set(AAX_PLAYING);
    AeonWave::add(chorus);

    delay_state = AAX_EFFECT_2ND_ORDER;
    delay.set(AAX_INITIALIZED);
    delay.set(AAX_PLAYING);
    AeonWave::add(delay);

    reverb_state = AAX_EFFECT_2ND_ORDER;
    reverb.set(AAX_INITIALIZED);
    reverb.set(AAX_PLAYING);
    AeonWave::add(reverb);

    midi.set_volume(100.0f/127.0f);
    midi.set(AAX_PLAYING);
}

void
MIDIDriver::stop()
{
    chorus.set(AAX_STOPPED);
    delay.set(AAX_STOPPED);
    reverb.set(AAX_STOPPED);
    midi.set(AAX_STOPPED);
}

void
MIDIDriver::rewind()
{
    channels.clear();
    uSPP = tempo/PPQN;

    chorus_channels.clear();

    for (const auto& it : delay_channels)
    {
        delay.remove(*it.second);
        AeonWave::add(*it.second);
    }
    delay_channels.clear();

    for (const auto& it : reverb_channels)
    {
        reverb.remove(*it.second);
        AeonWave::add(*it.second);
    }
    reverb_channels.clear();
}

void MIDIDriver::finish(uint8_t n)
{
    auto it = channels.find(n);
    if (it == channels.end()) return;

    if (it->second->finished() == false) {
        it->second->finish();
    }
}

bool
MIDIDriver::finished(uint8_t n)
{
    auto it = channels.find(n);
    if (it == channels.end()) return true;
    return it->second->finished();
}

bool
MIDIDriver::is_drums(uint8_t n)
{
    auto it = channels.find(n);
    if (it == channels.end()) return false;
    return it->second->is_drums();
}

void
MIDIDriver::set_volume(float g)
{
    g = volume * ln(g);

    aax::dsp dsp = get(AAX_VOLUME_FILTER);
    dsp.set(AAX_GAIN, g);
    set(dsp);
}

float
MIDIDriver::get_volume()
{
    aax::dsp dsp = get(AAX_VOLUME_FILTER);
    return dsp.get(AAX_GAIN);
}

void
MIDIDriver::set_balance(float b)
{
    Matrix64 m;
    m.rotate(1.57*b, 0.0, 1.0, 0.0);
    m.inverse();
    AeonWave::matrix(m);
}

void
MIDIDriver::set_gm2_chorus_type(uint16_t type)
{
    switch(type)
    {
    case GM2_CHORUS1:
        midi.set_chorus("GM2/chorus1", type, GM2);
        break;
    case GM2_CHORUS2:
        midi.set_chorus("GM2/chorus2", type, GM2);
        break;
    case GM2_CHORUS3:
        midi.set_chorus("GM2/chorus3", type, GM2);
        break;
    case GM2_CHORUS4:
        midi.set_chorus("GM2/chorus4", type, GM2);
        break;
    case GM2_CHORUS_FEEDBACK:
        midi.set_chorus("GM2/chorus_freedback", type, GM2);
        break;
    case GM2_FLANGER:
        midi.set_chorus("GM2/flanger", type, GM2);
        break;
    default:
        LOG(99, "LOG: Unsupported GS chorus type: 0x%x (%d)\n",
                                type, type);
        break;
    }
}

bool
MIDIDriver::set_chorus(const char *t, uint16_t type, uint8_t vendor)
{
    if (type != -1)
    {
        uint32_t vendor_type = uint32_t(vendor) << 16 | type;
        if (vendor_type != chorus_type) {
            MESSAGE(1, "Switching to %s\n", t);
        }
        chorus_type = vendor_type;
    }

    chorus_buffer = &AeonWave::buffer(t);
    for (auto& it : channels) {
        if (it.second->get_chorus_level() > 0.0f) {
            it.second->set_chorus(*chorus_buffer);
        }
    }
    return true;
}

void
MIDIDriver::send_chorus_to_reverb(float val)
{
    if (val > 0.0f) {
        MESSAGE(3, "Send %.0f%% chorus to reverb\n", val*100);
    }
    chorus_to_reverb = val;
}

void
MIDIDriver::set_chorus_level(float val)
{
    for (auto& it : chorus_channels) {
        set_chorus_level(it.first, val);
    }
}

void
MIDIDriver::set_chorus_level(uint16_t part_no, float val)
{
    auto& part = midi.channel(part_no);
#if 0
    if (val > 0.0f)
    {
        if (part.get_chorus_level() == 0.0f)
        {
            auto it = chorus_channels.find(part_no);
            if (it == chorus_channels.end())
            {
                it = channels.find(part_no);
                if (it != channels.end() && it->second)
                {
                    if (AeonWave::remove(*it->second))
                    {
                        it->second->add(*chorus_buffer);
                        chorus.add(*it->second);
                        chorus_channels[it->first] = it->second;
                    }
                }
            }
        }
    }
    else
    {
        auto it = chorus_channels.find(part_no);
        if (it != chorus_channels.end() && it->second)
        {
            chorus.remove(*it->second);
            it->second->add(nullBuffer);
            AeonWave::add(*it->second);
            MESSAGE(3, "Remove part %i from chorus\n", part_no);
        }
    }
#endif
    MESSAGE(3, "Set part %i chorus to %.0f%%: %s\n",
                part_no, val*100, get_channel_name(part_no).c_str());

    aax::Buffer& disabled = AeonWave::buffer("GM2/chorus0");
    part.set_chorus(val > 0.0f ? *chorus_buffer : disabled);
    part.set_chorus_level(val);
}

void
MIDIDriver::set_chorus_delay(float ms) {
#if 0
    chorus_depth = ms*1e-3f;
    if (ms > 0.0f) {
        MESSAGE(4, "Set chorus delay to %.0f%%\n", chorus_delay*100.0f);
    }
    for (auto& it : chorus_channels) {
        set_chorus_depth(it.first, val);
    }
#endif
}

void
MIDIDriver::set_chorus_depth(float val) {
    if (val > 0.0f) {
        MESSAGE(4, "Set chorus depth to %.0f%%\n", chorus_depth*100.0f);
    }

    for (auto& it : chorus_channels) {
        it.second->set_chorus_depth(val);
    }
}

void
MIDIDriver::set_chorus_rate(float val) {
    if (val > 0.0f) {
        MESSAGE(4, "Set chorus rate to %.2fHz\n", val);
    }
    for (auto& it : chorus_channels) {
        it.second->set_chorus_rate(val);
    }
}

void
MIDIDriver::set_chorus_feedback(float val) {
    if (val > 0.0f) {
        MESSAGE(4, "Set chorus feedback to %.0f%%\n", val);
    }
    for (auto& it : chorus_channels) {
        it.second->set_chorus_feedback(val);
    }
}

void
MIDIDriver::set_chorus_cutoff_frequency(float val)
{
    if (val < 22000.0f) {
        MESSAGE(4, "Set chorus cutoff frequency to %.2fHz\n", val);
    }
    for (auto& it : chorus_channels) {
        it.second->set_chorus_cutoff(val);
    }
}


bool
MIDIDriver::set_delay(const char *t, uint16_t type, uint8_t vendor)
{
    if (type != -1)
    {
        uint32_t vendor_type = uint32_t(vendor) << 16 | type;
        if (vendor_type != delay_type) {
            MESSAGE(1, "Switching to %s\n", t);
        }
        delay_type = vendor_type;
    }

    delay_buffer = &AeonWave::buffer(t);
    delay.add(*delay_buffer);
    for(auto& it : channels) {
        if (it.second->get_delay_level() > 0.0f) {
            it.second->set_delay(*delay_buffer);
        }
    }
    return true;
}

void
MIDIDriver::send_delay_to_reverb(float val)
{
    if (val > 0.0f) {
        MESSAGE(3, "Send %.0f%% delay to reverb\n", val*100);
    }
}

void
MIDIDriver::set_delay_level(float val)
{
    for (auto& it: delay_channels) {
        set_delay_level(it.first, val);
    }
}

void
MIDIDriver::set_delay_level(uint16_t part_no, float val)
{
    auto& part = midi.channel(part_no);
#if 0
    if (val > 0.0f)
    {
        if (part.get_delay_level() == 0.0f)
        {
            auto it = delay_channels.find(part_no);
            if (it == delay_channels.end())
            {
                it = channels.find(part_no);
                if (it != channels.end() && it->second)
                {
                    if (AeonWave::remove(*it->second))
                    {
                        it->second->add(*delay_buffer);
                        delay.add(*it->second);
                        delay_channels[it->first] = it->second;
                    }
                }
            }
        }
    }
    else
    {
        auto it = delay_channels.find(part_no);
        if (it != delay_channels.end() && it->second)
        {
            delay.remove(*it->second);
            it->second->add(nullBuffer);
            AeonWave::add(*it->second);
            MESSAGE(3, "Remove part %i from delay\n", part_no);
        }
    }
# endif
    MESSAGE(3, "Set part %i delay to %.0f%%: %s\n",
                part_no, val*100, get_channel_name(part_no).c_str());
    aax::Buffer& disabled = AeonWave::buffer("GM2/delay0");
    part.set_delay(val > 0.0f ? *delay_buffer : disabled);
    part.set_delay_level(val);
}

void
MIDIDriver::set_delay_depth(float ms) {
    delay_depth = ms*1e-3f;
    if (ms > 0.0f) {
        MESSAGE(4, "Set delays depth to %.0f%%\n", delay_depth*100.0f);
    }
    for(auto& it : delay_channels) {
        it.second->set_delay_depth(delay_depth);
    }
}

void
MIDIDriver::set_delay_rate(float rate) {
    if (rate > 0.0f) {
        MESSAGE(4, "Set delay rate to %.2fHz\n", rate);
    }
    for(auto& it : delay_channels) {
        it.second->set_delay_rate(rate);
    }
}

void
MIDIDriver::set_delay_feedback(float feedback) {
    if (feedback > 0.0f) {
        MESSAGE(4, "Set delay feedback to %.0f%%\n", feedback);
    }
    for(auto& it : delay_channels) {
        it.second->set_delay_feedback(feedback);
    }
}

void
MIDIDriver::set_delay_cutoff_frequency(float fc)
{
    if (fc < 22000.0f) {
        MESSAGE(4, "Set delay cutoff frequency to %.2fHz\n", fc);
    }
    for(auto& it : delay_channels) {
        it.second->set_chorus_cutoff(fc);
    }
}

bool
MIDIDriver::set_reverb(const char *t, uint16_t type, uint8_t vendor)
{
    if (type != -1)
    {
        uint32_t vendor_type = uint32_t(vendor) << 16 | type;
        if (vendor_type != reverb_type) {
            MESSAGE(1, "Switching to %s reveberation\n", t);
        }
        reverb_type = vendor_type;
    }

    reverb_buffer = &AeonWave::buffer(t);
    reverb.add(*reverb_buffer);
    for(auto& it : channels) {
        if (it.second->get_reverb_level() > 0.0f) {
            it.second->set_reverb(*reverb_buffer);
        }
    }
    return true;
}

void
MIDIDriver::set_gm2_reverb_type(uint16_t type)
{
    reverb_type = (GM2<<16)|type;
    switch (type)
    {
    case GM2_REVERB_ROOM_SMALL:
        midi.set_reverb("GM2/room-small", type, GM2);
        break;
    case GM2_REVERB_ROOM_MEDIUM:
        midi.set_reverb("GM2/room-medium", type, GM2);
        break;
    case GM2_REVERB_ROOM_LARGE:
        midi.set_reverb("GM2/room-large", type, GM2);
        break;
    case GM2_REVERB_CONCERTHALL:
        midi.set_reverb("GM2/concerthall", type, GM2);
        break;
    case GM2_REVERB_CONCERTHALL_LARGE:
        midi.set_reverb("GM2/concerthall-large", type, GM2);
        break;
    case GM2_REVERB_PLATE:
        midi.set_reverb("GM2/plate", type, GM2);
        break;
    default:
        LOG(99, "LOG: Unsupported reverb type: 0x%x (%d)\n",
                                type, type);
        break;
    }
}

void
MIDIDriver::set_reverb_level(float val)
{
    for (auto& it: channels) {
        set_reverb_level(it.first, val);
    }
}

void
MIDIDriver::set_reverb_level(uint16_t part_no, float val)
{
    auto& part = midi.channel(part_no);
    if (val > 0.0f)
    {
        if (1 || part.get_reverb_level() == 0.0f)
        {
            auto it = reverb_channels.find(part_no);
            if (it == reverb_channels.end())
            {
                it = channels.find(part_no);
                if (it != channels.end() && it->second)
                {
                    AeonWave::remove(*it->second);
                    it->second->set_reverb(*reverb_buffer);
                    reverb.add(*it->second);
                    reverb_channels[it->first] = it->second;
                    MESSAGE(3, "Set part %i reverb to %.0f%%: %s\n",
                            part_no, val*100, get_channel_name(part_no).c_str());
                }
            }
        }
    }
    else
    {
        auto it = reverb_channels.find(part_no);
        if (it != reverb_channels.end() && it->second)
        {
            reverb.remove(*it->second);
            aax::Buffer& disabled = AeonWave::buffer("GM2/room0");
            it->second->set_reverb(disabled);
            AeonWave::add(*it->second);
            MESSAGE(3, "Remove part %i from reverb\n", part_no);
        }
    }
    part.set_reverb_level(val);
}

void
MIDIDriver::set_reverb_cutoff_frequency(float val) {
    reverb_cutoff_frequency = val;
}
void
MIDIDriver::set_reverb_time_rt60(float val) {
    reverb_time = val;
    reverb_decay_level = powf(LEVEL_60DB, 0.2f*reverb_decay_depth/val);
}
void
MIDIDriver::set_reverb_decay_depth(float val) {
    reverb_decay_depth = 0.1f*val;
    set_reverb_time_rt60(reverb_time);
}
void
MIDIDriver::set_reverb_delay_depth(float val) {
    for(auto& it : reverb_channels) {
        it.second->set_reverb_delay_depth(val);
    }
}

/*
 * Create map of instrument banks and program numbers with their associated
 * file names from the XML files for a quick access during playback.
 */
void
MIDIDriver::read_instruments(std::string gmmidi, std::string gmdrums)
{
    const char *filename, *type = "instrument";
    auto imap = instruments;

    std::string iname;
    if (!gmmidi.empty())
    {
        iname = gmmidi;

        struct stat buffer;
        if (stat(iname.c_str(), &buffer) != 0)
        {
           iname = path;
           iname.append("/");
           iname.append(gmmidi);
        }
    } else {
        iname = path;
        iname.append("/");
        iname.append(instr);
    }

    filename = iname.c_str();
    for(unsigned int id=0; id<2; ++id)
    {
        xmlId *xid = xmlOpen(filename);
        if (xid)
        {
            xmlId *xaid = xmlNodeGet(xid, "aeonwave");
            xmlId *xmid = nullptr;
            char key_off[64] = "";
            char key_on[64] = "";
            char name[64] = "";
            char file[64] = "";

            if (xaid)
            {
                if (xmlAttributeExists(xaid, "rate"))
                {
                    unsigned int rate = xmlAttributeGetInt(xaid, "rate");
                    if (rate >= 25 && rate <= 200) {
                       refresh_rate = rate;
                    }
                }
                if (xmlAttributeExists(xaid, "polyphony"))
                {
                    polyphony =  xmlAttributeGetInt(xaid, "polyphony");
                    if (polyphony < 32) polyphony = 32;
                }
                xmid = xmlNodeGet(xaid, "set"); // was: midi
            }

            if (xmid)
            {
                if (patch_set == "default" && xmlAttributeExists(xmid, "name"))
                {
                    char *set = xmlAttributeGetString(xmid, "name");
                    if (set && strlen(set) != 0) {
                        patch_set = set;
                    }
                    xmlFree(set);
                }

                if (xmlAttributeExists(xmid, "mode"))
                {
                   if (!xmlAttributeCompareString(xmid, "mode", "synthesizer"))
                   {
                       instrument_mode = AAX_RENDER_SYNTHESIZER;
                   }
                   else if (!xmlAttributeCompareString(xmid, "mode", "arcade"))
                   {
                       instrument_mode = AAX_RENDER_ARCADE;
                   }
                }

                if (xmlAttributeExists(xmid, "version"))
                {
                    char *set = xmlAttributeGetString(xmid, "version");
                    if (set && strlen(set) != 0) {
                        patch_version = set;
                    }
                    xmlFree(set);
                }

                if (xmlAttributeExists(xmid, "file")) {
                    effects = xmlAttributeGetString(xmid, "file");
                }

                unsigned int bnum = xmlNodeGetNum(xmid, "layer"); // was: bank
                xmlId *xbid = xmlMarkId(xmid);
                for (unsigned int b=0; b<bnum; b++)
                {
                    if (xmlNodeGetPos(xmid, xbid, "layer", b) != 0)
                    {
                        xmlId *xiid = xmlMarkId(xbid);
                        unsigned int slen, inum;
                        uint16_t bank_no;
                        char offset;

                        bank_no = xmlAttributeGetInt(xbid, "n") << 7;
                        bank_no += xmlAttributeGetInt(xbid, "l");

                        offset = xmlAttributeGetInt(xbid, "offset");

                        if (bank_no == 0 && xmlAttributeExists(xbid, "default-drums"))
                        {
                            drum_set_no = xmlAttributeGetInt(xbid, "default-drums");
                        }

                        // bank name
                        xmlAttributeCopyString(xiid, "name", name, 64);

                        // bank audio-frame filter and effects file
                        slen = xmlAttributeCopyString(xbid, "file", file, 64);
                        if (slen)
                        {
                            file[slen] = 0;
                            frames.insert({bank_no,{name,file}});
                        }

                        // type is 'instrument' or ´patch' for drums/ensembles
                        inum = xmlNodeGetNum(xbid, type);
                        auto bank = imap[bank_no];
                        for (int i=0; i<inum; i++)
                        {
                            if (xmlNodeGetPos(xbid, xiid, type, i) != 0)
                            {
                                int n, min, max, wide;
                                float spread;
                                bool stereo;

                                min = 0;
                                max = 128;
                                if (xmlAttributeExists(xiid, "min")) {
                                    min = xmlAttributeGetInt(xiid, "min");
                                }
                                if (xmlAttributeExists(xiid, "max")) {
                                    max = xmlAttributeGetInt(xiid, "max");
                                }
                                n = xmlAttributeGetInt(xiid, "n");
                                if (type == "instrument")
                                {
                                    if (!offset) n++;
                                    else n -= (offset-1);
                                }
                                else if (!min && !max) {
                                    min = max = n;
                                }

                                stereo = xmlAttributeGetBool(xiid, "stereo");

                                wide = xmlAttributeGetInt(xiid, "wide");
                                if (!wide && (stereo || xmlAttributeGetBool(xiid, "wide"))) {
                                    wide = -1;
                                }

                                spread = 1.0f;
                                if (xmlAttributeExists(xiid, "spread")) {
                                   spread = xmlAttributeGetDouble(xiid, "spread");
                                }

                                // instrument name
                                xmlAttributeCopyString(xiid, "name",
                                                              name, 64);

                                // key-on file-name
                                key_on[0] = '\0';
                                xmlAttributeCopyString(xiid, "key-on",
                                                              key_on, 64);

                                // key-off file-name
                                key_off[0] = '\0';
                                xmlAttributeCopyString(xiid, "key-off",
                                                              key_off, 64);

                                // instrument file-name
                                slen = xmlAttributeCopyString(xiid, "file",
                                                              file, 64);
                                if (slen)
                                {
                                    file[slen] = 0;
                                    bank.insert({n,{{name,file,key_on,key_off},
                                                   {wide,spread,stereo,min,max}}
                                                });

                                    patch_map_t p;
                                    p.insert({0,{i,{name,file}}});

                                    patches.insert({file,p});
//                                  if (id == 0) printf("{%x, {%i, {%s, %i}}}\n", bank_no, n, file, wide);
                                }
                                else
                                {
                                    slen = xmlAttributeCopyString(xiid, "include",
                                                                  file, 64);
                                    if (slen)
                                    {
                                        file[slen] = 0;
					add_patch(file, name, 64);
                                        bank.insert({n,{{name,file,""},{wide,spread,stereo}}});
                                    }
                                }
                            }
                        }
                        imap[bank_no] = bank;
                        xmlFree(xiid);
                    }
                }
                xmlFree(xbid);
                xmlFree(xmid);
                xmlFree(xaid);
            }
            else {
                ERROR("aeonwave/midi not found in: " << filename);
            }
            xmlClose(xid);
        }
        else {
            ERROR("Unable to open: " << filename);
        }

        if (id == 0)
        {
            instruments = std::move(imap);

            // next up: drums
            if (!gmdrums.empty())
            {
                iname = gmdrums;

                struct stat buffer;
                if (stat(iname.c_str(), &buffer) != 0)
                {
                   iname = path;
                   iname.append("/");
                   iname.append(gmmidi);
                }
            } else {
                iname = path;
                iname.append("/");
                iname.append(drum);
            }
            filename = iname.c_str();
            type = "patch"; // was: "drum";
            imap = drums;
        }
        else {
            drums = std::move(imap);
        }
    }

    if (!midi.get_initialize() && drum_set_no != -1)
    {
        std::ostringstream s;

        auto it = frames.find(drum_set_no<<7);
        if (it != frames.end()) {
            s << "Switching to drum " << it->second.name;
        } else {
            s << "Switching to drum set number:  " << drum_set_no+1;
        }
        INFO(s.str().c_str());
    }
}

// patches are xml configuration files which define two or more
// instrument definitions spread across midi note ranges.
void
MIDIDriver::add_patch(const char *file, char *name, size_t nlen)
{
    const char *path = midi.info(AAX_SHARED_DATA_DIR);

    std::string xmlfile(path);
    xmlfile.append("/");
    xmlfile.append(file);
    xmlfile.append(".xml");

    xmlId *xid = xmlOpen(xmlfile.c_str());
    if (xid)
    {
        xmlId *xlid = xmlNodeGet(xid, "aeonwave/set/layer");
        if (xlid)
        {
            unsigned int pnum = xmlNodeGetNum(xlid, "patch");
            xmlId *xpid = xmlMarkId(xlid);
            patch_map_t p;
            for (uint8_t i=0; i<pnum; i++)
            {
                if (xmlNodeGetPos(xlid, xpid, "patch", i) != 0)
                {
                    unsigned int slen;
                    char file[64] = "";

		    if (xmlAttributeExists(xpid, "name")) {
     		        xmlAttributeCopyString(xpid, "name", name, nlen);
		    }
                    slen = xmlAttributeCopyString(xpid, "file", file, 64);
                    if (slen)
                    {
                        uint8_t max = xmlAttributeGetInt(xpid, "max");
                        file[slen] = 0;

                        p.insert({max,{i,{name,file}}});
                    }
                }
            }
            patches.insert({file,p});

            xmlFree(xpid);
            xmlFree(xlid);
        }
        xmlFree(xid);
    }
}

/*
 * For drum mapping the program_no is stored in the upper 8 bits, and the
 * bank_no (msb) in the lower eight bits of the bank number of the map
 * and the key_no in the program number of the map.
 */
const inst_t
MIDIDriver::get_drum(uint16_t bank_no, uint16_t& program_no, uint8_t key_no, bool all)
{
    if (program_no == 0 && drum_set_no != -1) {
        program_no = drum_set_no;
    }

    inst_t empty_map;
    uint16_t prev_program_no = program_no;
    uint16_t req_program_no = program_no;
    do
    {
        auto itb = drums.find(program_no << 7 | bank_no);
        bool bank_found = (itb != drums.end());
        if (bank_found)
        {
            auto bank = itb->second;
            auto iti = bank.find(key_no);
            if (iti != bank.end())
            {
                if (all || selection.empty() ||
                    std::find(selection.begin(), selection.end(),
                              iti->second.first.file) != selection.end() ||
                    std::find(selection.begin(), selection.end(),
                              iti->second.first.name) != selection.end())
                {
                    if (req_program_no != program_no)
                    {
                        auto itrb = drums.find(req_program_no << 7);
                        if (itrb != drums.end()) {
                            auto& bank = itrb->second;
//                          bank.insert({key_no,{{"",""},{}}});
                            bank.insert({key_no,iti->second});
                        }
                    }
                    return iti->second;
                } else {
//                  return empty_map;
                }
            }
        }

        // The drum was not found, try something different.
        // If the MSB was set then clear it and try again.
        if (bank_no >> 7)
        {
            DISPLAY(4, "Drum %i not found in bank %i/%i, trying bank: 0/%i\n",
                       key_no, bank_no >> 7, bank_no & 0x7F, bank_no &= 0x7F);
            bank_no &= 0x7F;
            continue;
        }

        // If the LSB was set then clear it and try again.
        if (bank_no & 0x7F)
        {
            DISPLAY(4, "Drum %i not found in bank %i/%i, trying bank: 0/0\n",
                       key_no, bank_no >> 7, bank_no & 0x7F);
            bank_no = 0;
            continue;
        }

        if (!prev_program_no && !program_no) {
            break;
        }
        prev_program_no = program_no;

        switch (midi.get_mode())
        {
        default: // General MIDI or unspecified
            if ((program_no % 10) == 0) {
                program_no = 0;
            } else if ((program_no & 0xF8) == program_no) {
                program_no -= (program_no % 10);
            } else {
                program_no &= 0xF8;
            }
            break;
        }

        if (bank_found) {
            if (prev_program_no != program_no) {
                DISPLAY(4, "Drum %i not found in program %i, trying: %i\n",
                        key_no, prev_program_no, program_no);
            } else {
                DISPLAY(4, "Drum %i not found.\n", key_no);
            }
        } else if (!is_avail(missing_drum_bank, prev_program_no)) {
            DISPLAY(4, "Drum program %i not found, trying %i\n",
                        prev_program_no, program_no);
            missing_drum_bank.push_back(prev_program_no);
            if (prev_program_no == req_program_no) {
               req_program_no = program_no;
            }
        }
    }
    while (true);

    // default drums
    auto itb = drums.find(program_no);
    auto& bank = itb->second;
    auto iti = bank.insert({key_no, std::move(empty_map)});
    return iti.first->second;
}

const inst_t
MIDIDriver::get_instrument(uint16_t bank_no, uint8_t program_no, bool all)
{
    inst_t empty_map;
    uint16_t prev_bank_no = bank_no;
    uint16_t req_bank_no = bank_no;

    do
    {
        auto itb = instruments.find(bank_no);
        bool bank_found = (itb != instruments.end());
        if (bank_found)
        {
            auto bank = itb->second;
            auto iti = bank.find(program_no+1);
            if (iti != bank.end())
            {
                if (all || selection.empty() ||
                    std::find(selection.begin(), selection.end(),
                              iti->second.first.file) != selection.end() ||
                    std::find(selection.begin(), selection.end(),
                              iti->second.first.name) != selection.end())
                {
                    return iti->second;
                }

                auto& inst = iti->second;
                std::string& display = (midi.get_verbose() >= 99) ?
                                           inst.first.file : inst.first.name;
                DISPLAY(4, "No instrument mapped to bank %i, program %i\n",
                            bank_no, program_no);
                DISPLAY(4,  "%s will not be heard\n", display.c_str());
                return empty_map;
            }
        }

        if (!prev_bank_no && !bank_no) {
            break;
        }
        prev_bank_no = bank_no;

        switch (midi.get_mode())
        {
        case MIDI_EXTENDED_GENERAL_MIDI:
            if (bank_no & 0x7F) {          // switch to MSB only
                bank_no &= ~0x7F;
            } else if (bank_no > 0) {      // fall back to bank-0, (GM-MIDI 1)
                bank_no = 0;
            }
        case MIDI_GENERAL_STANDARD:
        {
            bool sc88pro = ((bank_no & 0x7) == 3);
            if (bank_no & 0x7F) {          // Remove Model-ID
                 bank_no &= ~0x7F;
                 if (sc88pro) continue;
            } else if (bank_no > 0) {      // fall back to bank-0, (GM-MIDI 1)
                bank_no = 0;
            }
            break;
        }
        default: // General MIDI or unspecified
            if (bank_no & 0x7F) {          // LSB (XG-MIDI)
                bank_no &= ~0x7F;
            } else if (bank_no & 0x3F80) { // MSB (GS-MIDI / GM-MIDIr 2)
                bank_no &= ~0x3F80;
            } else if (bank_no > 0) {      // fall back to bank-0, (GM-MIDI 1)
                bank_no = 0;
            }
            break;
        }

        if (bank_found) {
            if (prev_bank_no != bank_no) {
                DISPLAY(4, "Instrument %i not found in bank %i/%i, trying: %i/%i\n",
                         program_no, prev_bank_no >> 7, prev_bank_no & 0x7F,
                         bank_no >> 7, bank_no & 0x7F);
            } else {
                DISPLAY(4, "Instrument %i not found.\n", program_no);
            }
        } else if (!is_avail(missing_instrument_bank, prev_bank_no)) {
            DISPLAY(4, "Instrument bank %i/%i not found, trying %i/%i\n",
                        prev_bank_no >> 7, prev_bank_no & 0x7F,
                        bank_no >> 7, bank_no & 0x7F);
            missing_instrument_bank.push_back(prev_bank_no);
        }
    }
    while (true);

    DISPLAY(4, "No instrument mapped to bank %i, program %i\n",
            bank_no, program_no);

    auto itb = instruments.find(bank_no);
    auto& bank = itb->second;
    auto iti = bank.insert({program_no, std::move(empty_map)});
    return iti.first->second;
}

const MIDIDriver::patch_entry_t
MIDIDriver::get_patch(std::string& name, uint8_t& key)
{
    auto patches = get_patches();
    auto it = patches.find(name);
    if (it != patches.end())
    {
        auto patch = it->second.upper_bound(key);
        if (patch != it->second.end()) {
            return patch->second;
        }

        // above the largest upper key, use the last key.
        auto last = it->second.rbegin();
        if (last != it->second.rend()) {
            return last->second;
        }
    }

    key = 255;
    return {0,{name,name}};
}

void
MIDIDriver::grep(std::string& filename, const char *grep)
{
    if (midi.get_csv()) return;

    std::string s(grep);
    std::regex regex{R"(,+)"}; // split on a comma
    std::sregex_token_iterator it{s.begin(), s.end(), regex, -1};
    auto selection = std::vector<std::string>{it, {}};

    bool found = false;
    for (auto it : loaded)
    {
        for (auto greps : selection)
        {
            if (it.find(greps) != std::string::npos)
            {
                if (!found) {
                    printf("%s found:\n", filename.c_str());
                    found =  true;
                }
                printf("    %s\n", it.c_str());
            }
        }
    }
}

MIDIInstrument&
MIDIDriver::new_channel(uint8_t track_no, uint16_t bank_no, uint8_t program_no)
{
    bool drums = is_drums(track_no);
    auto it = channels.find(track_no);
    if (!drums && it != channels.end())
    {
        if (it->second) AeonWave::remove(*it->second);
        channels.erase(it);
    }

    std::string file = "";
    if (drums && !frames.empty())
    {
        auto it = frames.find(program_no);
        if (it != frames.end()) {
            file = it->second.file;
        }
    }

    Buffer& buffer = midi.buffer(file);
    if (buffer) {
        buffer.set(AAX_CAPABILITIES, int(instrument_mode));
    }

    it = channels.find(track_no);
    if (it == channels.end())
    {
        try {
            auto ret = channels.insert(
                { track_no, std::shared_ptr<MIDIInstrument>(
                                    new MIDIInstrument(*this, buffer,
                                          track_no, bank_no, program_no, drums))
                } );
            it = ret.first;
            AeonWave::add(*it->second);
        } catch(const std::invalid_argument& e) {
            throw(e);
        }
    }

    MIDIInstrument& rv = *it->second;
    rv.set_program_no(program_no);
    rv.set_bank_no(bank_no);

    char *env = getenv("AAX_KEY_FINISH");
    if (env && atoi(env)) {
        rv.set_key_finish(true);
    }

    return rv;
}

MIDIInstrument&
MIDIDriver::channel(uint16_t track_no)
{
    auto it = channels.find(track_no);
    if (it != channels.end()) {
        return *it->second;
    }
    return new_channel(track_no, 0, 0);
}

/**
 * Note Off messages are ignored on Rhythm Channels, with the exception of the
 * ORCHESTRA SET (specifically, Note number 88) and the SFX SET
 * (Note numbers 47-84).
 *
 * Some percussion timbres require a mutually exclusive Note On/Off assignment.
 * For example, when a Note On message for Note number 42 (Closed Hi Hat) is
 * received while Note number 46 (Open Hi Hat) is sounding, Note number 46 is
 * promptly muted and Note number 42 sounds.
 *
 * <Standard Set> (1)
 * Scratch Push(29)  | Scratch Pull(30)
 * Closed HH(42)     | Pedal HH(44)     | Open HH(46)
 * Short Whistle(71) | Long Whistle(72)
 * Short Guiro(73)   | Long Guiro(74)
 * Mute Cuica(78)    | Open Cuica(79)
 * Mute Triangle(80) | Open Triangle(81)
 * Mute Surdo(86)    | Open Surdo(87)
 *
 * <Analog Set> (26)
 * Analog CHH 1(42) | Analog C HH 2(44) | Analog OHH (46)
 *
 * <Orchestra Set> (49)
 * Closed HH 2(27) | Pedal HH (28) | Open HH 2 (29)
 *
 * <SFX Set> (57)
 * Scratch Push(41) | Scratch Pull (42)
 */
bool
MIDIDriver::process(uint8_t track_no, uint8_t message, uint8_t key, uint8_t velocity, bool omni, float pitch)
{
    // Omni mode: Device responds to MIDI data regardless of channel
    if (message == MIDI_NOTE_ON && velocity) {
        if (is_track_active(track_no)) {
            try {
                channel(track_no).play(key, velocity, pitch);
                if (channel(track_no).get_stereo()) {
                    set_reverb_level(track_no, 1.0f);
                }
            } catch (const std::runtime_error &e) {
                throw(e);
            }
        }
    }
    else
    {
        if (message == MIDI_NOTE_ON) {
            velocity = 64;
        }
        channel(track_no).stop(key, velocity);
    }
    return true;
}

std::string
MIDIDriver::get_channel_type(uint16_t part_no)
{
    if (is_drums(part_no))  return "Drums";
    return "Instrument";
}

std::string
MIDIDriver::get_channel_name(uint16_t part_no)
{
    std::string rv;
    if (is_drums(part_no))
    {
        rv = "Drums";
        uint16_t bank_no = channel(part_no).get_bank_no();
        auto itb = frames.find(bank_no);
        if (itb != frames.end())
        {
           auto bank = itb->second;
           rv = bank.name;
        }
    }
    else
    {
        uint16_t bank_no = channel(part_no).get_bank_no();
        uint8_t program_no = channel(part_no).get_program_no();
        auto inst = midi.get_instrument(bank_no, program_no);
        rv = inst.first.name;
    }
    return rv;
}

bool
MIDIDriver::elapsed_time(double dt)
{
    bool rv = display;

    if (display && timer_started)
    {
        std::chrono::time_point<std::chrono::system_clock> end_time;
        end_time = std::chrono::system_clock::now();

        std::chrono::duration<double> elapsed_seconds = end_time - start_time;
        if (elapsed_seconds.count() > dt) {
            timer_started = false;
        } else {
            rv = false;
       }
    }
    return rv;
}

const std::vector<std::string>
MIDIDriver::midi_channel_convention = {
    "Piano Solo (Left & Right Hand)",
    "Bass Instrument",
    "Primary Accompaniment",
    "Primary Melodic Instrument with Lyrics",
    "Secondary Accompaniment",
    "Secondary Melodic Instrument",
    "Alternative 1",
    "Alternative 2",
    "Alternative 3",
    "Drums & Percussion",
    "--Reserved--",
    "--Reserved--",
    "--Reserved--",
    "--Reserved--",
    "--Reserved--",
    "--Reserved--"
};
