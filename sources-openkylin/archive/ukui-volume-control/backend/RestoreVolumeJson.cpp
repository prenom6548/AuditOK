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

#include "RestoreVolumeJson.h"
#include "AudioContext.h"

namespace UkuiAudioFramwork
{
template<typename T>
void updateField(nlohmann::json& jsonObj, const std::string& key, const T& value) {
    if (jsonObj[key] != value) {
        jsonObj[key] = value;
    }
}

template<typename T>
T getMemberValue(const RestoreVolumeInfo& info, T RestoreVolumeInfo::* memberPtr) {
    return info.*memberPtr;
}

bool RestoreVolumeJson::getStatus(const std::string& cardName, const std::string& portName, const std::string& key) const
{
    auto& ports = m_json[cardName]["ports"];
    auto portIt = ports.find(portName);

    if (portIt != ports.end()) {
        return (*portIt)[key];
    }

    AUDIO_ERROR("Can not get the mute status of sound card: {} port: {}, the device does not exist", cardName, portName);
    return true;
}

void RestoreVolumeJson::insert(const RestoreVolumeInfo& info) {
    auto& ports = m_json[info.cardName]["ports"];
    auto portIt = ports.find(info.portName);

    if (portIt == ports.end()) {
        // GCOVR_EXCL_BR_START - nlohmann aggregate initializer branch explosion (WB-BCONF insert-new-port).
        ports[info.portName] = {
            {"name", info.portName},
            {"volume", info.volume},
            {"priority", info.priority},
            {"balance", info.balance},
            {"enable", info.enable},
            {"mute", info.muted}
        };
        // GCOVR_EXCL_BR_STOP
    } else {
        nlohmann::json& portJson = *portIt;
        for (const auto& entry : m_filedMap) {
            const char* const fieldKey = entry.first;
            const auto& memberVar = entry.second;
            std::visit(
                [&](auto&& ptr) {
                    using T = std::decay_t<decltype(info.*ptr)>;
                    if constexpr (std::is_same_v<T, bool>) {
                        updateField(portJson, fieldKey, info.*ptr);
                    } else if constexpr (std::is_same_v<T, uint32_t> || std::is_same_v<T, float> ||
                                          std::is_same_v<T, double>) {
                        updateField(portJson, fieldKey, info.*ptr);
                    }
                },
                memberVar);
        }
    }

    writeToJson();
}

RestoreVolumeJson::RestoreVolumeJson()
{
    m_fileName = getenv("HOME");
    m_fileName += RESTORE_VOLUME_JSON;
    loadJson();
    initJson();
}

bool RestoreVolumeJson::isContain(const RestoreVolumeInfo& info) const
{
    const auto& card = m_json.find(info.cardName);
    if (card != m_json.end()) {
        const auto& port = (*card)["ports"].find(info.portName);
        return port != (*card)["ports"].end();
    }
    return false;
}

void RestoreVolumeJson::initJson()
{
    auto autoPause = m_json.find("auto-pause");
    if (autoPause == m_json.end()) {
        m_json["auto-pause"] = false;
    }

    writeToJson();
}

//template<typename T>
//std::optional<T> RestoreVolumeJson::getValue(const std::string& cardName, const std::string& portName, const std::string& key) const {
//    auto cardIt = m_json.find(cardName);
//    if (cardIt != m_json.end()) {
//        auto& cardJson = cardIt.second;
//        auto portsIt = cardJson.find("ports");
//        if (portsIt != cardJson.end()) {
//            auto& portsJson = portsIt->second;
//            auto portIt = portsJson.find(portName);
//            if (portIt != portsJson.end()) {
//                auto& portJson = portIt->second;
//                if (portJson.contains(key)) {
//                    return portJson[key].get<T>();
//                }
//            }
//        }
//    }
//    return std::nullopt;
//}

std::optional<bool> RestoreVolumeJson::getValue(const std::string& key) const
{
    auto it = m_json.find(key);
    if (it != m_json.end()) {
        return it->get<bool>();
    }
    return std::nullopt;
}

void RestoreVolumeJson::setValue(const std::string& key, bool status)
{
    auto autoPause = m_json.find(key);
    if (autoPause != m_json.end()) {
        m_json[key] = status;
    }

    writeToJson();
}
}
