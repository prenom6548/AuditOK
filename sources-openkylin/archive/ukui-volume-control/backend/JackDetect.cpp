/*
 * Copyright (C) 2020-2024 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "JackDetect.h"

#include <sys/inotify.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <poll.h>
#include <algorithm>
#include <cstring>
#include <iostream>

#include "AudioContext.h"


namespace UkuiAudioFramwork
{
std::vector<std::string> JackDetect::listSoundCards()
{
    std::vector<std::string> cards;
    DIR* d = opendir("/dev/snd");
    if (!d)
        return cards;
    dirent* ent;
    while ((ent = readdir(d))) {
        if (strncmp(ent->d_name, "controlC", 8) == 0) {
            cards.emplace_back(ent->d_name + 8);
        }
    }
    closedir(d);

    return cards;
}

bool JackDetect::jackIsPlugged(snd_ctl_elem_id_t* id, const char* card)
{
    snd_ctl_t* handle;
    std::string name = std::string("hw:") + card;
    if (snd_ctl_open(&handle, name.c_str(), 0) < 0)
        return false;

    snd_ctl_elem_value_t* val;
    snd_ctl_elem_value_alloca(&val);
    snd_ctl_elem_value_set_id(val, id);

    bool plugged = false;
    if (snd_ctl_elem_read(handle, val) == 0) {
        plugged = snd_ctl_elem_value_get_boolean(val, 0);
    }

    snd_ctl_close(handle);
    return plugged;
}

void JackDetect::enumJacks(const std::string& card, JackDetect::EventCallback cb)
{
    snd_ctl_t* handle;
    std::string name = "hw:" + card;

    if (snd_ctl_open(&handle, name.c_str(), 0) < 0)
        return;

    snd_ctl_elem_list_t* list;
    snd_ctl_elem_list_alloca(&list);
    if (snd_ctl_elem_list(handle, list) < 0) {
        snd_ctl_close(handle);
        return;
    }

    unsigned int count = snd_ctl_elem_list_get_count(list);
    snd_ctl_elem_list_alloc_space(list, count);
    if (snd_ctl_elem_list(handle, list) < 0) {
        snd_ctl_close(handle);
        return;
    }

    AlsaCard c;
    c.name = card;

    for (unsigned int idx = 0; idx < count; ++idx) {
        snd_ctl_elem_id_t* id;
        snd_ctl_elem_id_alloca(&id);
        snd_ctl_elem_list_get_id(list, idx, id);

        const char* ename = snd_ctl_elem_id_get_name(id);
        if (!ename || !strstr(ename, "Jack"))
            continue;

        bool plugged = jackIsPlugged(id, card.c_str());
        
        AlsaJack jack;
        jack.name = ename;
        jack.numid = snd_ctl_elem_id_get_index(id);
        jack.plugin = plugged;
        jack.type = getJackTypeByName(ename);
        c.jacks.emplace_back(jack);

        if (cb)
            cb(card, ename, jack.type, plugged);
    }
    
    m_cards.emplace_back(c);
    snd_ctl_close(handle);
}

AlsaJackType JackDetect::getJackTypeByName(const std::string& name) const
{
    for (const auto& [k, v] : m_jackMap) {
        if (k.compare(name) == 0)
            return v;
    }

    return AlsaJackType::ALSA_JACK_UNKNOEW;
}

JackDetect::JackDetect(EventCallback cb) : m_callback(std::move(cb)) {}

JackDetect::~JackDetect()
{
    stop();
}

void JackDetect::start()
{
    if (m_bRunning)
        return;
    m_bRunning = true;
    m_inotifyFd = inotify_init1(IN_NONBLOCK);
    scanCards();
    m_worker = std::thread(&JackDetect::loop, this);
}

void JackDetect::stop()
{
    if (!m_bRunning)
        return;
    m_bRunning = false;

    if (m_worker.joinable())
        m_worker.join();

    for (int wd : m_watchDescriptors)
        ::inotify_rm_watch(m_inotifyFd, wd);
    m_watchDescriptors.clear();

    if (m_inotifyFd >= 0)
        ::close(m_inotifyFd);
}

void JackDetect::scanCards()
{
    for (const auto& card : listSoundCards()) {
        std::string path = "/dev/snd/controlC" + card;
        int wd = inotify_add_watch(m_inotifyFd, path.c_str(), IN_ATTRIB);
        if (wd >= 0)
            m_watchDescriptors.push_back(wd);
        enumJacks(card, m_callback);
    }
}

void JackDetect::loop()
{
    std::vector<std::pair<snd_ctl_t*, std::string>> ctlList;

    for (const auto& card : listSoundCards()) {
        snd_ctl_t* handle;
        std::string hw = "hw:" + card;
        if (snd_ctl_open(&handle, hw.c_str(), 0) < 0)
            continue;

        if (snd_ctl_subscribe_events(handle, 1) == 0){
            ctlList.emplace_back(handle, card);
        }
        else {
            snd_ctl_close(handle);
        }
    }

    while (m_bRunning) {
        if (ctlList.empty())
            break;

        for (auto& [handle, card] : ctlList) {
            pollfd pfd{.fd = snd_ctl_poll_descriptors_count(handle),
                       .events = POLLIN,
                       .revents = 0};

            int count = snd_ctl_poll_descriptors(handle, &pfd, 1);
            if (count <= 0 || poll(&pfd, 1, 200) <= 0)
                continue;

            snd_ctl_event_t* ev;
            snd_ctl_event_alloca(&ev);
            if (snd_ctl_read(handle, ev) < 0)
                continue;

            if (snd_ctl_event_get_type(ev) != SND_CTL_EVENT_ELEM)
                continue;

            snd_ctl_elem_id_t* id;
            snd_ctl_elem_id_alloca(&id);
            snd_ctl_event_elem_get_id(ev, id);

            const char* ename = snd_ctl_elem_id_get_name(id);
            if (!ename || !strstr(ename, "Jack"))
                continue;

            bool plugged = jackIsPlugged(id, card.c_str());
            auto it = std::find_if(m_cards.begin(), m_cards.end(), [card, plugged](const AlsaCard& c) {
                return c.name == card;
            });
            if (it != m_cards.end()) {
                auto jacks = (*it).jacks;
                auto jack = std::find_if(jacks.begin(), jacks.end(), [ename, plugged](const AlsaJack& jack) {
                    return ename == jack.name;
                });
                if (jack != jacks.end()) {
                    (*jack).plugin = plugged;
                }
                else {
                    AUDIO_WARN("Jacks not found {} jack.", ename);
                }

            }
            else {
                AUDIO_WARN("Cards not found {} card.", card);
            }

            if (m_callback)
                m_callback(card, ename, getJackTypeByName(ename), plugged);
        }
    }

    for (auto& [handle, _] : ctlList) {
        snd_ctl_subscribe_events(handle, 0);
        snd_ctl_close(handle);
    }
}

}
