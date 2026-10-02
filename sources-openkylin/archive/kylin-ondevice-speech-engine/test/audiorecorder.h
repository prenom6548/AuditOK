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

#ifndef AUDIO_RECORDER_H_
#define AUDIO_RECORDER_H_

#include <pulse/error.h>
#include <pulse/pulseaudio.h>
#include <pulse/simple.h>

#include <chrono>
#include <vector>

#include "logger.h"

class AudioRecorder {
public:
    static bool isMicrophoneAvailable();

    static std::string getMicrophoneName();

private:
    struct UserData {
        pa_mainloop *mainLoop{nullptr};
        std::string deviceName_;
    };

    // 回调函数，用于获取默认麦克风设备
    static void getDefaultSourceCallback(pa_context *context, const pa_source_info *info, int eol, void *userdata);

    // 主循环回调函数
    static void contextStateCallback(pa_context *context, void *userdata);

public:
    AudioRecorder();
    ~AudioRecorder();

    void recording(std::vector<uint8_t> &audioData) const;

    void setSamplingParams(int sampleRate, int channels);

    void setSamplingTime(std::chrono::milliseconds samplingTime);

private:
    bool open();

    void close();

private:
    pa_sample_spec samplingParams_{PA_SAMPLE_S16LE, 16000, 1};
    pa_simple *handle_{nullptr};

    std::chrono::milliseconds samplingTime_{40};
};

#endif
