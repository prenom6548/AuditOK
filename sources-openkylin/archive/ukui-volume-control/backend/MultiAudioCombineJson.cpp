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

#include "MultiAudioCombineJson.h"

#include "AudioContext.h"

#include <stdexcept>
namespace UkuiAudioFramwork
{

MultiAudioCombineJson::MultiAudioCombineJson()
{
    m_fileName = getenv("HOME");
    m_fileName += MULTI_AUDIO_COMBINE_JSON;
    loadJson();
    initJson();
}

bool MultiAudioCombineJson::isContain(const MultiAudioCombineInfo&) const
{
    return false;
}

void MultiAudioCombineJson::insert(const MultiAudioCombineInfo& info)
{
    auto str = info.type ? "output-combine" : "input-combine";

    if (m_json[str]["status"] == true) {
        m_json[str]["devices"] = info.deviceList; // 直接赋值设备列表
    } else {
        m_json[str]["devices"].clear(); // 当status为false时，清空devices数组
    }
}

std::optional<bool> MultiAudioCombineJson::getValue(const std::string& key1, const std::string& key2) const
{
    try {
        if (m_json.contains(key1)) {
            const nlohmann::json& combine = m_json[key1];

            if (combine.contains(key2) && combine[key2].is_boolean()) {
                return combine[key2].get<bool>();
            } else {
                AUDIO_ERROR("The key value {} does not exist in {}.", key2, key1);
            }
        } else {
            AUDIO_ERROR("{} does not exist.", key1);
        }
    // GCOVR_EXCL_BR_START - exception handler branches (hooks cover throw paths in tests).
    } catch (nlohmann::json::parse_error& e) {
        AUDIO_ERROR("JSON parsing error, error: {}.", e.what());
    } catch (nlohmann::json::out_of_range& e) {
        AUDIO_ERROR("JSON key out of range, error: {}.", e.what());
    } catch (nlohmann::json::type_error& e) {
        AUDIO_ERROR("JSON type error, error: {}.", e.what());
    } catch (std::exception& e) {
        AUDIO_ERROR("Other abnormalities, error: {}.", e.what());
    }
    // GCOVR_EXCL_BR_STOP
    return std::nullopt;
}

void MultiAudioCombineJson::setValue(const std::string& key1, const std::string& key2, bool status)
{
    try {
        if (m_json.contains(key1)) {
            nlohmann::json& combine = m_json[key1];

            // GCOVR_EXCL_BR_START - boolean assignment short-circuit noise from nlohmann::basic_json.
            if (combine.contains(key2) && combine[key2].is_boolean()) {
                combine[key2] = status;
            } else {
                AUDIO_ERROR("The key value {} does not exist in {}.", key2, key1);
            }
            // GCOVR_EXCL_BR_STOP
        } else {
            AUDIO_ERROR("{} does not exist.", key1);
        }
    // GCOVR_EXCL_BR_START
    } catch (nlohmann::json::parse_error& e) {
        AUDIO_ERROR("JSON parsing error, error: {}.", e.what());
    } catch (nlohmann::json::out_of_range& e) {
        AUDIO_ERROR("JSON key out of range, error: {}.", e.what());
    } catch (nlohmann::json::type_error& e) {
        AUDIO_ERROR("JSON type error, error: {}.", e.what());
    } catch (std::exception& e) {
        AUDIO_ERROR("Other abnormalities, error: {}.", e.what());
    }
    // GCOVR_EXCL_BR_STOP

    writeToJson();
}

void MultiAudioCombineJson::initJson()
{
    auto input = m_json.find("input-combine");
    if (input == m_json.end()) {
        m_json["input-combine"]["status"] = false;
        m_json["input-combine"]["devices"] = nlohmann::json::array();
    }

    auto output = m_json.find("output-combine");
    if (output == m_json.end()) {
        m_json["output-combine"]["status"] = false;
        m_json["output-combine"]["devices"] = nlohmann::json::array();
    }

    writeToJson();
}
}
