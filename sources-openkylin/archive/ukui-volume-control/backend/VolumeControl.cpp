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

#include "VolumeControl.h"
#include <iostream>
#include <future>
#include <functional>

#include "AudioContext.h"
#include "AudioMethod.h"
#include "CustomAudioVolumeJson.h"
#include "JackDetect.h"
#include <algorithm>


namespace UkuiAudioFramwork
{
VolumeControl& VolumeControl::getInstance()
{
    static VolumeControl instance;
    return instance;
}

void VolumeControl::init()
{
    // 1. get backend

    // 2. created backend factory
    switch (m_type) {
    case BackendType::PIPEWIRE: {
        m_pBackendFactory = std::make_shared<PipewireBackendFactory>();
        m_pManagerFactory = std::make_shared<PipewireManagerFactory>();
        break;
    }
    case BackendType::PULSEAUDIO: {
        m_pBackendFactory = std::make_shared<PulseaudioBackendFactory>();
        m_pManagerFactory = std::make_shared<PulseaudioManagerFactory>();
        break;
    }
    case BackendType::ALSA:
    case BackendType::JACK:
    default:
        AUDIO_ERROR("Can not support {} backend, exit.", getBackendName(m_type));
        return;
    }

    // 3. created backend
    m_pBackend = m_pBackendFactory->createBackend();
    m_pAudioManager = m_pManagerFactory->createManager();
    m_pAudioManager->init();
    m_pAudioManager->connect();

    AudioMethod::getInstance().setManager(m_pAudioManager);
    AudioMethod::getInstance().init();

    std::thread([this]() {
        auto moduleList = m_pAudioManager->getModuleList();
        AUDIO_DEBUG("The size of the loaded module is {}.", moduleList.size());
        for (const auto& [k, v] : m_settingsMap) {

            bool status = m_statusFunctions[v[0]]();

            if (!status)
                continue;

            auto it = std::find_if(moduleList.begin(), moduleList.end(), [v](const std::string& name) {
                return name == v[1];
            });
            if (it == moduleList.end()) {
                if (v[1] == "module-switch-on-connect" && AudioMethod::getInstance().getStartupStatus())
                    std::this_thread::sleep_for(std::chrono::milliseconds(6500));
                AudioMethod::getInstance().loadModule(v[1], v[2]);
            }
            else {
                AUDIO_DEBUG("Module {} is loaded.", v[1]);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }).detach();

    JackDetect::EventCallback jackCallback = [](const std::string& card,
                        const std::string& jack,
                        AlsaJackType type,
                        bool plugged) {
        AUDIO_DEBUG("Card: {} jack: {} plugged: {}.", card, jack, plugged);
        if (plugged)
            return;

        if (type == AlsaJackType::ALSA_JACK_FRONTMIC ||
            type == AlsaJackType::ALSA_JACK_REARMIC ||
            type == AlsaJackType::ALSA_JACK_LINEIN ||
            type == AlsaJackType::ALSA_JACK_HEADPHONEMIC ||
            type == AlsaJackType::ALSA_JACK_INPUTMIC) {
            auto list = AudioMethod::getInstance().getAvailablePortList(NodeType::INPUT);
            if (list.size() <= 1) {
                AUDIO_DEBUG("{} jack unplugin, unload module-echo-cancel.", jack);
                AudioMethod::getInstance().unloadModule("module-echo-cancel");
            }
        }
        else if (type == AlsaJackType::ALSA_JACK_HEADPHONE ||
                 type == AlsaJackType::ALSA_JACK_HEADPHONE2 ||
                 type == AlsaJackType::ALSA_JACK_LINEOUT) {
            auto list = AudioMethod::getInstance().getAvailablePortList(NodeType::OUTPUT);
            if (list.size() <= 1) {
                AUDIO_DEBUG("{} jack unplugin, unload module-remap-sink.", jack);
                AudioMethod::getInstance().unloadModule("module-remap-sink");
            }
        }
    };
    JackDetect monitor(jackCallback);
    monitor.start();

    AudioMethod::getInstance().run();
}

}
