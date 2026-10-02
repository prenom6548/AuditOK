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

#ifndef SRC_AUDIO_CONFIG_H
#define SRC_AUDIO_CONFIG_H

#include <cstdint>
#include <filesystem>

#include "audiodatastream.h"
#include "util.h"

struct InputAudioInfo {
    enum class InputSource { DEFAULT_MIC, STREAM, PCM_FILE, RAW_DATA };
    enum class Format { PCM, MP3 };  // 指的是文件格式
    enum class Encode {
        SAMPLE_S16LE,  // Signed 16 Bit PCM, little endian
    };  // 音频存储格式，也有的库叫format
    InputSource inputSource{InputSource::DEFAULT_MIC};
    Format format{Format::PCM};
    Encode encode{Encode::SAMPLE_S16LE};
    int rate{16000};
    int channel{1};
    std::vector<uint8_t> audioData;  // 原始音频数据
    std::filesystem::path inputFilePath{};
    AudioDataStream* audioDataStream{nullptr};
};

struct OutputAudioInfo {
    enum class OutputTarget { PCM_DATA, DEFAULT_SPEAKER, PCM_OUTPUT_FILE };
    OutputTarget outputTarget{OutputTarget::DEFAULT_SPEAKER};
    int speed{50};   // 语速，可选值：[0-100]，默认为50
    int volume{50};  // 音量，可选值：[0-100]，默认为50
    int pitch{50};   // 音高，可选值：[0-100]，默认为50
    std::filesystem::path outputFilePath{};
};

struct _AudioConfig {
    std::string toJsonString() const;
    InputAudioInfo inputAudioInfo{};
    OutputAudioInfo outputAudioInfo{};
};

#endif
