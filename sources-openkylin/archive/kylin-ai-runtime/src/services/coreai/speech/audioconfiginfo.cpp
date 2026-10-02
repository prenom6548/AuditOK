/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "audioconfiginfo.h"

#include "utils/logger.h"
#include "utils/util.h"

AudioConfigInfo::AudioConfigInfo(const std::string& jsonString) {
    Json::Value root = util::stringToJson(jsonString);
    if (root.isMember("input_source")) {
        inputAudioInfo_.inputSource = InputAudioInfo::InputSource(root["input_source"].asInt());
    }
    if (root.isMember("output_target")) {
        outputAudioInfo_.outputTarget = OutputAudioInfo::OutputTarget(root["output_target"].asInt());
    }
    parseInputInfo(root);
    parseOutputInfo(root);
}

void AudioConfigInfo::parseInputInfo(const Json::Value& value) {
    if (inputAudioInfo_.inputSource == InputAudioInfo::InputSource::RAW_DATA) {
        if (value.isMember("raw_data")) {
            std::string data = value["raw_data"].asString();
            inputAudioInfo_.audioData = util::base64Decode(data);
        }
    } else if (inputAudioInfo_.inputSource == InputAudioInfo::InputSource::PCM_FILE) {
        if (value.isMember("pcm_file_path")) {
            inputAudioInfo_.inputFilePath = value["pcm_file_path"].asString();
        }
    }
    if (value.isMember("sample_rate")) {
        inputAudioInfo_.rate = value["sample_rate"].asInt();
    }
}

void AudioConfigInfo::parseOutputInfo(const Json::Value& value) {
    if (outputAudioInfo_.outputTarget == OutputAudioInfo::OutputTarget::PCM_OUTPUT_FILE) {
        if (value.isMember("output_pcm_file_path")) {
            outputAudioInfo_.outputFilePath = value["output_pcm_file_path"].asString();
        }
    }
    if (value.isMember("speed")) {
        outputAudioInfo_.speed = value["speed"].asInt();
    }
    if (value.isMember("volume")) {
        outputAudioInfo_.volume = value["volume"].asInt();
    }
    if (value.isMember("pitch")) {
        outputAudioInfo_.pitch = value["pitch"].asInt();
    }
}
