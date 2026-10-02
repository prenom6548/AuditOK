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

#include <pulse/pulseaudio.h>
#include <pulse/simple.h>

#include <atomic>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

// thread-safe
class AudioRecorder {
public:
    static void getMicrophoneInfo(std::string &deviceName, bool &mute);

private:
    struct UserData {
        pa_mainloop *mainLoop{nullptr};
        std::string deviceName;
        bool mute{true};
    };

    // 回调函数，用于获取默认麦克风设备
    static void getDefaultSourceCallback(pa_context *context, const pa_source_info *info, int eol, void *userdata);

    // 主循环回调函数
    static void contextStateCallback(pa_context *context, void *userdata);

public:
    AudioRecorder();

    AudioRecorder(uint32_t sampleRate, uint32_t channels, uint32_t ms);

    ~AudioRecorder();

    bool start();

    void stop();

    std::vector<std::vector<uint8_t>> getAudioDatas();

    void setSamplingParams(uint32_t sampleRate, uint32_t channels, uint32_t ms);

private:
    void loop();

private:
    pa_simple *handle_{nullptr};
    pa_sample_spec samplingParams_{PA_SAMPLE_S16LE, 16000, 1};
    pa_buffer_attr attr_{
        static_cast<decltype(pa_buffer_attr::maxlength)>(-1),  // 默认值
        static_cast<decltype(pa_buffer_attr::tlength)>(-1),    // 默认值
        static_cast<decltype(pa_buffer_attr::prebuf)>(-1),     // 默认值
        static_cast<decltype(pa_buffer_attr::minreq)>(-1),     // 默认值
        static_cast<decltype(pa_buffer_attr::fragsize)>(-1),   // 默认值
    };

    std::atomic_bool isRunning_{false};
    std::thread thread_;
    std::queue<std::vector<uint8_t>> audioDataQueue_;
    std::mutex runningMutex_;
    std::mutex queueMutex_;
};

#endif
