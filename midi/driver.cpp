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

    chorus.tie(chorus_level, AAX_VOLUME_FILTER, AAX_GAIN);
    chorus.tie(chorus_depth, AAX_CHORUS_EFFECT, AAX_LFO_OFFSET);
    chorus.tie(chorus_rate, AAX_CHORUS_EFFECT, AAX_LFO_FREQUENCY);
    chorus.tie(chorus_feedback, AAX_CHORUS_EFFECT, AAX_FEEDBACK_GAIN);

    // TODO: delay
    delay.tie(delay_state, AAX_DELAY_EFFECT);

    reverb.tie(reverb_decay_level, AAX_REVERB_EFFECT, AAX_DECAY_LEVEL);
    reverb.tie(reverb_decay_depth, AAX_REVERB_EFFECT, AAX_DECAY_DEPTH);
    reverb.tie(reverb_cutoff_frequency, AAX_REVERB_EFFECT, AAX_CUTOFF_FREQUENCY);
    reverb.tie(reverb_state, AAX_REVERB_EFFECT);
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
    chorus_state = AAX_SINE_WAVE;
    set_chorus_type(2);
    chorus.set(AAX_INITIALIZED);
    chorus.set(AAX_PLAYING);
    AeonWave::add(chorus);

    delay_state = AAX_REVERB_2ND_ORDER;
    delay.set(AAX_INITIALIZED);
    delay.set(AAX_PLAYING);
    AeonWave::add(delay);

    reverb_state = AAX_REVERB_2ND_ORDER;
    set_reverb_type(4);
    reverb.set(AAX_INITIALIZED);
    reverb.set(AAX_PLAYING);
    AeonWave::add(reverb);

    midi.set_gain(1.0f);
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

void
MIDIDriver::set_gain(float g)
{
    aax::dsp dsp = AeonWave::get(AAX_VOLUME_FILTER);
    dsp.set(AAX_GAIN, g);
//  dsp.set(AAX_AGC_RESPONSE_RATE, 1.5f);
    AeonWave::set(dsp);
}

bool
MIDIDriver::is_drums(uint8_t n)
{
    auto it = channels.find(n);
    if (it == channels.end()) return false;
    return it->second->is_drums();
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
MIDIDriver::set_chorus_type(uint8_t type)
{
    switch(type)
    {
    case 0:
        midi.set_chorus("chorus/chorus1");
        INFO("Switching to type 1 chorus");
        break;
    case 1:
        midi.set_chorus("chorus/chorus2");
        INFO("Switching to type 2 chorus");
        break;
    case 2:
        midi.set_chorus("chorus/chorus3");
        INFO("Switching to type 3 chorus");
        break;
    case 3:
        midi.set_chorus("chorus/chorus4");
        INFO("Switching to type 4 chorus");
        break;
    case 4:
        midi.set_chorus("chorus/chorus_freedback");
        INFO("Switching to feedback chorus");
        break;
    case 5:
        midi.set_chorus("chorus/flanger");
        INFO("Switching to flanging");
        break;
    default:
        LOG(99, "LOG: Unsupported GS chorus type: 0x%x (%d)\n",
                                type, type);
        break;
    }
}

void
MIDIDriver::set_chorus(const char *t)
{
    Buffer& buf = AeonWave::buffer(t);
    for(int i=0; i<chorus_channels.size(); ++i) {
        midi.channel(i).add(buf);
    }
}

void
MIDIDriver::send_chorus_to_reverb(float val)
{
    if (val > 0.0f) {
        MESSAGE(3, "Send %.0f%% chorus to reverb\n", val*100);
    }
}

void
MIDIDriver::set_chorus_level(uint16_t part_no, float val)
{
    val = _ln(val);
    auto it = std::find(chorus_channels.begin(),chorus_channels.end(), part_no);
    if (val > 0 && it == chorus_channels.end()) {
        chorus_channels.push_back(part_no);
    } else if (it != chorus_channels.end()) {
        chorus_channels.erase(it);
    }

    auto& part = midi.channel(part_no);
    if (val > 0.0f && part.get_chorus_level() != val) {
        MESSAGE(3, "Set part %i chorus to %.0f%%: %s\n", part_no, val*100.0f,
                    get_channel_name(part_no).c_str());
    }
    part.set_chorus_level(val);
}

void
MIDIDriver::set_chorus_delay(float ms) {
#if 0
    chorus_depth = ms*1e-3f;
    if (ms > 0.0f) {
        MESSAGE(4, "Set chorus delay to %.0f%%\n", chorus_delay*100.0f);
    }
    for(int i=0; i<chorus_channels.size(); ++i) {
        midi.channel(i).set_chorus_depth(chorus_depth);
    }
#endif
}

void
MIDIDriver::set_chorus_depth(float ms) {
    chorus_depth = ms*1e-3f;
    if (ms > 0.0f) {
        MESSAGE(4, "Set chorus depth to %.0f%%\n", chorus_depth*100.0f);
    }
    for(int i=0; i<chorus_channels.size(); ++i) {
        midi.channel(i).set_chorus_depth(chorus_depth);
    }
}

void
MIDIDriver::set_chorus_rate(float rate) {
    if (rate > 0.0f) {
        MESSAGE(4, "Set chorus rate to %.2fHz\n", rate);
    }
    for(int i=0; i<chorus_channels.size(); ++i) {
        midi.channel(i).set_chorus_rate(rate);
    }
}

void
MIDIDriver::set_chorus_feedback(float feedback) {
    if (feedback > 0.0f) {
        MESSAGE(4, "Set chorus feedback to %.0f%%\n", feedback);
    }
    for(int i=0; i<chorus_channels.size(); ++i) {
        midi.channel(i).set_chorus_feedback(feedback);
    }
}

void
MIDIDriver::set_chorus_cutoff_frequency(float fc)
{
#if AAX_PATCH_LEVEL >= 230425
    if (fc < 22000.0f) {
        MESSAGE(4, "Set chorus cutoff frequency to %.2fHz\n", fc);
    }
    for(int i=0; i<chorus_channels.size(); ++i) {
        midi.channel(i).set_chorus_cutoff(fc);
    }
#endif
}


void
MIDIDriver::set_delay(const char *t)
{
// TODO:
#if 0
    Buffer& buf = AeonWave::buffer(t);
    delay.add(buf);
    for(auto& it : channels) {
        it.second->set_delay(buf);
    }
#endif
}

void
MIDIDriver::set_delay_level(uint16_t part_no, float val)
{
// TODO:
#if 0
    val = _ln(val);
    auto& part = midi.channel(part_no);
    if (val > 0.0f && part.get_delay_level() != val)
    {
        auto it = delay_channels.find(part_no);
        if (it == delay_channels.end())
        {
            it = channels.find(part_no);
            if (it != channels.end() && it->second)
            {
                AeonWave::remove(*it->second);
                delay.add(*it->second);
                delay_channels[it->first] = it->second;
                MESSAGE(3, "Set part %i delay to %.0f%%: %s\n",
                        part_no, val*100, get_channel_name(part_no).c_str());
            }
        }
        part.set_delay_level(val);
    }
    else
    {
        auto it = delay_channels.find(part_no);
        if (it != delay_channels.end() && it->second)
        {
            delay.remove(*it->second);
            AeonWave::add(*it->second);
            MESSAGE(3, "Remove part %i from delay\n", part_no);
        }
    }
#endif
}

void
MIDIDriver::set_reverb(const char *t)
{
    Buffer& buf = AeonWave::buffer(t);
    reverb.add(buf);
    for(auto& it : channels) {
        it.second->set_reverb(buf);
    }
}

void
MIDIDriver::set_reverb_type(uint8_t type)
{
    reverb_type = type;
    switch (type)
    {
    case 0:
        midi.set_reverb("reverb/room-small");
        INFO("Switching to Small Room reveberation");
        break;
    case 1:
        midi.set_reverb("reverb/room-medium");
        INFO("Switching to Medium Room reveberation");
        break;
    case 2:
        midi.set_reverb("reverb/room-large");
        INFO("Switching to Large Room reveberation");
        break;
    case 3:
        midi.set_reverb("reverb/concerthall");
        INFO("Switching to Concert Hall Reveberation");
        break;
    case 4:
        midi.set_reverb("reverb/concerthall-large");
        INFO("Switching to Large Concert Hall reveberation");
        break;
    case 8:
        midi.set_reverb("reverb/plate");
        INFO("Switching to Plate reveberation");
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
    val = _ln(val);
    auto& part = midi.channel(part_no);
    if (val > 0.0f && part.get_reverb_level() != val)
    {
        auto it = reverb_channels.find(part_no);
        if (it == reverb_channels.end())
        {
            it = channels.find(part_no);
            if (it != channels.end() && it->second)
            {
                AeonWave::remove(*it->second);
                reverb.add(*it->second);
                reverb_channels[it->first] = it->second;
                MESSAGE(3, "Set part %i reverb to %.0f%%: %s\n",
                        part_no, val*100, get_channel_name(part_no).c_str());
            }
        }
        part.set_reverb_level(val);
    }
    else
    {
        auto it = reverb_channels.find(part_no);
        if (it != reverb_channels.end() && it->second)
        {
            reverb.remove(*it->second);
            AeonWave::add(*it->second);
            MESSAGE(3, "Remove part %i from reverb\n", part_no);
        }
    }
}

void
MIDIDriver::set_reverb_cutoff_frequency(float value) {
    reverb_cutoff_frequency = value;
}
void
MIDIDriver::set_reverb_time_rt60(float value) {
    reverb_time = value;
    reverb_decay_level = powf(LEVEL_60DB, 0.2f*reverb_decay_depth/value);
}
void
MIDIDriver::set_reverb_decay_depth(float value) {
    reverb_decay_depth = 0.1f*value;
    set_reverb_time_rt60(reverb_time);
}
void
MIDIDriver::set_reverb_delay_depth(float value) {
    for(auto& it : reverb_channels) {
        it.second->set_reverb_delay_depth(value);
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
                xmid = xmlNodeGet(xaid, "midi");
            }

            if (xmid)
            {
                if (xmlAttributeExists(xmid, "name"))
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

                unsigned int bnum = xmlNodeGetNum(xmid, "bank");
                xmlId *xbid = xmlMarkId(xmid);
                for (unsigned int b=0; b<bnum; b++)
                {
                    if (xmlNodeGetPos(xmid, xbid, "bank", b) != 0)
                    {
                        unsigned int slen, inum = xmlNodeGetNum(xbid, type);
                        xmlId *xiid = xmlMarkId(xbid);
                        uint16_t bank_no;

                        bank_no = xmlAttributeGetInt(xbid, "n") << 7;
                        bank_no += xmlAttributeGetInt(xbid, "l");

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
                            frames.insert({bank_no,{name,file,""}});
                        }

                        auto bank = imap[bank_no];
                        for (unsigned int i=0; i<inum; i++)
                        {
                            if (xmlNodeGetPos(xbid, xiid, type, i) != 0)
                            {
                                uint16_t n = xmlAttributeGetInt(xiid, "n");
                                float spread = 1.0f;
                                bool stereo = false;
                                int wide = 0;

                                stereo = xmlAttributeGetBool(xiid, "stereo");

                                if (simd64) {
                                    wide = xmlAttributeGetInt(xiid, "wide");
                                }
                                if (!wide && xmlAttributeGetBool(xiid, "wide"))
                                {
                                    wide = -1;
                                }
                                if (!wide && stereo) wide = -1;

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
                                    bank.insert({n,{{name,file,key_on,key_off},{wide,spread,stereo}}});

                                    _patch_map_t p;
                                    p.insert({0,{i,file}});

                                    patches.insert({file,p});
//                                  if (id == 0) printf("{%x, {%i, {%s, %i}}}\n", bank_no, n, file, wide);
                                }
                                else
                                {
                                    slen = xmlAttributeCopyString(xiid, "patch",
                                                                  file, 64);
                                    if (slen)
                                    {
                                        file[slen] = 0;
                                        bank.insert({n,{{name,file,""},{wide,spread,stereo}}});

                                        add_patch(file);
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
            type = "drum";
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
MIDIDriver::add_patch(const char *file)
{
    const char *path = midi.info(AAX_SHARED_DATA_DIR);

    std::string xmlfile(path);
    xmlfile.append("/");
    xmlfile.append(file);
    xmlfile.append(".xml");

    xmlId *xid = xmlOpen(xmlfile.c_str());
    if (xid)
    {
        xmlId *xlid = xmlNodeGet(xid, "instrument/layer");
        if (xlid)
        {
            unsigned int pnum = xmlNodeGetNum(xlid, "patch");
            xmlId *xpid = xmlMarkId(xlid);
            _patch_map_t p;
            for (unsigned int i=0; i<pnum; i++)
            {
                if (xmlNodeGetPos(xlid, xpid, "patch", i) != 0)
                {
                    unsigned int slen;
                    char file[64] = "";

                    slen = xmlAttributeCopyString(xpid, "file", file, 64);
                    if (slen)
                    {
                        uint8_t max = xmlAttributeGetInt(xpid, "max");
                        file[slen] = 0;

                        p.insert({max,{i,file}});
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
                            bank.insert({key_no,{{"",""},{}}});
                        }
                    }
                    return iti->second;
                } else {
                    return empty_map;
                }
            }
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
                DISPLAY(4, "Drum %i not found in bank %i, trying bank: %i\n",
                        key_no, prev_program_no, program_no);
            } else {
                DISPLAY(4, "Drum %i not found.\n", key_no);
            }
        } else if (!is_avail(missing_drum_bank, prev_program_no)) {
            DISPLAY(4, "Drum bank %i not found, trying %i\n",
                        prev_program_no, program_no);
            missing_drum_bank.push_back(prev_program_no);
            if (prev_program_no == req_program_no) {
               req_program_no = program_no;
            }
        }
    }
    while (true);

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
            auto iti = bank.find(program_no);
            if (iti != bank.end())
            {
                if (all || selection.empty() ||
                    std::find(selection.begin(), selection.end(),
                              iti->second.first.file) != selection.end() ||
                    std::find(selection.begin(), selection.end(),
                              iti->second.first.name) != selection.end())
                {
                    return iti->second;
                } else {
                    return empty_map;
                }
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

    auto itb = instruments.find(bank_no);
    auto& bank = itb->second;
    auto iti = bank.insert({program_no, std::move(empty_map)});
    return iti.first->second;
}

std::pair<uint8_t,std::string>
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
    }

    key = 255;
    return {0,name};
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

    int level = 0;
    std::string file = "";
    if (drums && !frames.empty())
    {
        auto it = frames.find(program_no);
        if (it != frames.end()) {
            level = it->first;
            file = it->second.file;
        }
    }

    Buffer& buffer = midi.buffer(file, level);
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
