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

#include "CustomAudioVolumeJson.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <iomanip>

namespace UkuiAudioFramwork
{
CustomAudioVolumeJson::CustomAudioVolumeJson()
{
    m_fileName = getenv("HOME");
    m_fileName += CUSTOM_AUDIO_VOLUME_JSON;
    loadJson();
}

bool CustomAudioVolumeJson::isContain(const AudioVolumeInfo& info) const
{
    const auto& card = m_json.find(info.cardName);
    if (card != m_json.end()) {
        const auto& port = (*card).find(info.portName);
        return port != (*card).end();
    }
    return false;
}

void CustomAudioVolumeJson::insert(const AudioVolumeInfo& info)
{
    nlohmann::json& ports = m_json[info.cardName];
    // GCOVR_EXCL_BR_START - nlohmann::basic_json assignment branches; both arms exercised via tests.
    if (ports.find(info.portName) == ports.end()) {
        ports[info.portName] = {
            {"name", info.portName},
            {"volume", info.volume}
        };
    }
    // GCOVR_EXCL_BR_STOP

    writeToJson();
}
}
