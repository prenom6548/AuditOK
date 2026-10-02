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

#ifndef AUDIO_CONFIG_INFO_H
#define AUDIO_CONFIG_INFO_H

#include <jsoncpp/json/json.h>

#include <cstdint>
#include <filesystem>
#include <vector>

struct InputAudioInfo {
    enum class InputSource { DEFAULT_MIC, STREAM, PCM_FILE, RAW_DATA };
    enum class Format { PCM, MP3 };  // 指的是文件格式
    enum class Encode {
        SAMPLE_S16LE,  // Signed 16 Bit PCM, little endian
    };                 // 音频存储格式，也有的库叫format
    InputSource inputSource{InputSource::DEFAULT_MIC};
    Format format{Format::PCM};
    Encode encode{Encode::SAMPLE_S16LE};
    int rate{16000};
    int channel{1};
    std::vector<uint8_t> audioData;  // 原始音频数据
    std::filesystem::path inputFilePath{};
    std::string requestId{};

    std::string getFormaString() const {
        switch (format) {
            case Format::PCM:
                return "pcm";
            case Format::MP3:
                return "mp3";
            default:
                return "unknown";
        }
    }
};

struct OutputAudioInfo {
    enum class OutputTarget { PCM_DATA, DEFAULT_SPEAKER, PCM_OUTPUT_FILE };
    OutputTarget outputTarget{OutputTarget::DEFAULT_SPEAKER};
    int speed{50};
    int volume{50};
    int pitch{50};
    std::filesystem::path outputFilePath{};
};

class AudioConfigInfo {
public:
    AudioConfigInfo() = default;
    explicit AudioConfigInfo(const std::string& jsonString);
    [[nodiscard]] InputAudioInfo inputAudioInfo() const { return inputAudioInfo_; }
    [[nodiscard]] OutputAudioInfo outputAudioInfo() const { return outputAudioInfo_; }

private:
    void parseInputInfo(const Json::Value& value);
    void parseOutputInfo(const Json::Value& value);

private:
    InputAudioInfo inputAudioInfo_{};
    OutputAudioInfo outputAudioInfo_{};
};

#endif  // AUDIO_CONFIG_INFO_H
