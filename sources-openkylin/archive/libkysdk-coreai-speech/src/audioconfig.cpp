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

#include "audioconfig.h"

#include <jsoncpp/json/json.h>

#include <cstring>

#include "_audioconfig.h"
#include "logger.h"

std::string _AudioConfig::toJsonString() const {
    Json::Value root;

    // 设置输入来源类型
    switch (inputAudioInfo.inputSource) {
        case InputAudioInfo::InputSource::DEFAULT_MIC:
            root["input_source"] = int(InputAudioInfo::InputSource::DEFAULT_MIC);
            break;
        case InputAudioInfo::InputSource::STREAM:
            root["input_source"] = int(InputAudioInfo::InputSource::STREAM);
            break;
        case InputAudioInfo::InputSource::PCM_FILE:
            root["input_source"] = int(InputAudioInfo::InputSource::PCM_FILE);
            root["pcm_file_path"] = inputAudioInfo.inputFilePath.string();
            break;
        case InputAudioInfo::InputSource::RAW_DATA:
            root["input_source"] = int(InputAudioInfo::InputSource::RAW_DATA);
            root["raw_data"] = util::base64Encode(inputAudioInfo.audioData.data(), inputAudioInfo.audioData.size());
            break;
        default:
            LOGE("未知输入源");
            break;
    }

    // 设置输出目标类型
    switch (outputAudioInfo.outputTarget) {
        case OutputAudioInfo::OutputTarget::DEFAULT_SPEAKER:
            root["output_target"] = int(OutputAudioInfo::OutputTarget::DEFAULT_SPEAKER);
            break;
        case OutputAudioInfo::OutputTarget::PCM_OUTPUT_FILE:
            root["output_target"] = int(OutputAudioInfo::OutputTarget::PCM_OUTPUT_FILE);
            root["output_pcm_file_path"] = outputAudioInfo.outputFilePath.string();
            break;
        case OutputAudioInfo::OutputTarget::PCM_DATA:
            root["output_target"] = int(OutputAudioInfo::OutputTarget::PCM_DATA);
            break;
        default:
            LOGE("未知输出源");
            break;
    }

    // 设置采样率
    root["sample_rate"] = inputAudioInfo.rate;

    // 设置输出音频格式
    root["speed"] = outputAudioInfo.speed;
    root["volume"] = outputAudioInfo.volume;
    root["pitch"] = outputAudioInfo.pitch;

    // 序列化 JSON 对象
    Json::FastWriter writer;
    return writer.write(root);
}

AudioConfig *audio_config_create_continuous_audio_input_from_default_microphone() {
    AudioConfig *config = new AudioConfig{};
    config->inputAudioInfo.inputSource = InputAudioInfo::InputSource::DEFAULT_MIC;
    return config;
}

AudioConfig *audio_config_create_continuous_audio_input_from_audio_data_stream(AudioDataStream *audio_data_stream) {
    AudioConfig *config = new AudioConfig{};
    config->inputAudioInfo.inputSource = InputAudioInfo::InputSource::STREAM;
    config->inputAudioInfo.audioDataStream = audio_data_stream;
    return config;
}

AudioConfig *audio_config_create_once_audio_input_from_pcm_file(const char *pcm_file) {
    AudioConfig *config = new AudioConfig{};
    config->inputAudioInfo.inputSource = InputAudioInfo::InputSource::PCM_FILE;
    config->inputAudioInfo.inputFilePath = pcm_file;
    return config;
}

AudioConfig *audio_config_create_once_audio_input_from_pcm_data(const uint8_t *audio_data, uint32_t data_length) {
    AudioConfig *config = new AudioConfig{};
    config->inputAudioInfo.inputSource = InputAudioInfo::InputSource::RAW_DATA;
    config->inputAudioInfo.audioData.assign(audio_data, audio_data + data_length);
    return config;
}

AudioConfig *audio_config_create_audio_output_from_pcm_data() {
    AudioConfig *config = new AudioConfig{};
    config->outputAudioInfo.outputTarget = OutputAudioInfo::OutputTarget::PCM_DATA;
    return config;
}

// 使用系统默认扬声器作为音频输出
AudioConfig *audio_config_create_audio_output_from_default_speaker() {
    AudioConfig *config = new AudioConfig{};
    config->outputAudioInfo.outputTarget = OutputAudioInfo::OutputTarget::DEFAULT_SPEAKER;
    return config;
}

// 使用pcm文件作为音频输出
AudioConfig *audio_config_create_audio_output_from_pcm_file_name(const char *pcm_file) {
    AudioConfig *config = new AudioConfig{};
    config->outputAudioInfo.outputTarget = OutputAudioInfo::OutputTarget::PCM_OUTPUT_FILE;
    config->outputAudioInfo.outputFilePath = pcm_file;
    return config;
}

void audio_config_destroy(AudioConfig **config) {
    if (config == nullptr) {
        return;
    }
    delete *config;
    *config = nullptr;
}

void audio_config_set_input_audio_rate(AudioConfig *config, int rate) {
    if (config == nullptr) {
        return;
    }
    config->inputAudioInfo.rate = rate;
}

void audio_config_set_output_audio_speed(AudioConfig *config, int speed) {
    if (config == nullptr) {
        return;
    }
    config->outputAudioInfo.speed = speed;
}

void audio_config_set_output_audio_volume(AudioConfig *config, int volume) {
    if (config == nullptr) {
        return;
    }
    config->outputAudioInfo.volume = volume;
}

void audio_config_set_output_audio_pitch(AudioConfig *config, int pitch) {
    if (config == nullptr) {
        return;
    }
    config->outputAudioInfo.pitch = pitch;
}
