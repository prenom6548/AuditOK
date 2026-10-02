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

#include "audiorecorder.h"

#include <chrono>

#include "logger.h"
#include "util.h"

static bool stringContains(const std::string &str, const std::string &sub) {
    if (str.length() < sub.length()) {
        return false;
    };
    return str.find(sub) != std::string::npos;
}

bool AudioRecorder::isMicrophoneAvailable() {
    pa_mainloop *mainLoop = pa_mainloop_new();
    pa_mainloop_api *mainLoopApi = pa_mainloop_get_api(mainLoop);

    pa_context *context = pa_context_new(mainLoopApi, "DefaultSourceChecker");
    UserData userData{mainLoop};
    pa_context_set_state_callback(context, contextStateCallback, &userData);
    pa_context_connect(context, nullptr, PA_CONTEXT_NOFLAGS, nullptr);

    if (pa_mainloop_run(mainLoop, nullptr) < 0) {
        LOGE("pa_mainloop_run() failed.");
        pa_context_unref(context);
        pa_mainloop_free(mainLoop);
        return false;
    }

    pa_context_unref(context);
    pa_mainloop_free(mainLoop);

    if (userData.deviceName_.empty()) {
        return false;
    }

    if (stringContains(userData.deviceName_, ".monitor") && stringContains(userData.deviceName_, "output")) {
        LOGD("{}", userData.deviceName_);
        return false;
    }

    LOGD("{}", userData.deviceName_);
    return true;
}

std::string AudioRecorder::getMicrophoneName() {
    pa_mainloop *mainLoop = pa_mainloop_new();
    pa_mainloop_api *mainLoopApi = pa_mainloop_get_api(mainLoop);

    pa_context *context = pa_context_new(mainLoopApi, "DefaultSourceChecker");
    UserData userData{mainLoop};
    pa_context_set_state_callback(context, contextStateCallback, &userData);
    pa_context_connect(context, nullptr, PA_CONTEXT_NOFLAGS, nullptr);

    if (pa_mainloop_run(mainLoop, nullptr) < 0) {
        LOGE("pa_mainloop_run() failed.");
        pa_context_unref(context);
        pa_mainloop_free(mainLoop);
        return "";
    }

    pa_context_unref(context);
    pa_mainloop_free(mainLoop);

    if (userData.deviceName_.empty()) {
        return "";
    }

    if (stringContains(userData.deviceName_, ".monitor") && stringContains(userData.deviceName_, "output")) {
        LOGD("{}", userData.deviceName_);
        return "";
    }

    // LOGD("{}", userData.deviceName_);
    return userData.deviceName_;
}

void AudioRecorder::getDefaultSourceCallback(pa_context *context, const pa_source_info *info, int eol, void *userdata) {
    UserData *pUserData = (UserData *)(userdata);
    pa_mainloop *mainLoop = pUserData->mainLoop;
    if (eol < 0) {
        if (pa_context_errno(context) == PA_ERR_NOENTITY) {
            LOGE("No default source found.");
        } else {
            LOGE("Failed to get source information.");
        }
        pa_mainloop_quit(mainLoop, -1);
        return;
    }

    if (!eol) {
        // 正常退出
        pUserData->deviceName_ = info->name;
        pa_mainloop_quit(mainLoop, 0);
    }
    pa_mainloop_quit(mainLoop, -1);
}

void AudioRecorder::contextStateCallback(pa_context *context, void *userdata) {
    UserData *pUserData = (UserData *)(userdata);
    pa_mainloop *mainLoop = pUserData->mainLoop;
    pa_context_state_t status = pa_context_get_state(context);
    switch (status) {
    case PA_CONTEXT_READY: {
        pa_operation_unref(pa_context_get_source_info_by_name(context, nullptr, getDefaultSourceCallback, userdata));
        break;
    }
    case PA_CONTEXT_FAILED:
        LOGE("Connection failed.");
        pa_mainloop_quit(mainLoop, -1);
        break;
    case PA_CONTEXT_TERMINATED:
        LOGE("Connection terminated.");
        pa_mainloop_quit(mainLoop, -1);
        break;
    default:
        break;
    }
}

AudioRecorder::AudioRecorder() {}

AudioRecorder::~AudioRecorder() {
    stop();
    if (handle_ != nullptr) {
        pa_simple_free(handle_);
    }
}

bool AudioRecorder::start() {
    std::lock_guard locker(mutex_);
    if (isRunning_) {
        return false;
    }
    int error{};
    handle_ = pa_simple_new(nullptr,          // 使用默认服务器
                            "AudioRecorder",  // 客户端名称
                            PA_STREAM_RECORD, // 录制流
                            nullptr,          // 使用默认设备
                            "Record",         // 流描述
                            &samplingParams_, // 样本格式
                            nullptr,          // 使用默认通道映射
                            nullptr,          // 使用默认缓冲区属性
                            &error            // 错误码
    );
    if (handle_ == nullptr) {
        return false;
    }
    isRunning_ = true;
    thread_ = std::thread([this]() { loop(); });
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1s);
    return true;
}

void AudioRecorder::stop() {
    {
        std::lock_guard locker(mutex_);
        if (!isRunning_) {
            return;
        }
        isRunning_ = false;
    }
    if (thread_.joinable()) {
        thread_.join();
    }
}

std::vector<uint8_t> AudioRecorder::getSingleFrameAudioData() {
    std::lock_guard lock(mutex_);
    if (audioDataQueue_.empty()) {
        return {};
    }
    auto data = std::move(audioDataQueue_.front());
    audioDataQueue_.pop();
    return data;
}

void AudioRecorder::setSamplingParams(int sampleRate, int channels) {
    samplingParams_.rate = sampleRate;
    samplingParams_.channels = channels;
    samplingParams_.format = PA_SAMPLE_S16LE; // 暂不考虑其他采样位数
}

void AudioRecorder::setSamplingTime(std::chrono::seconds samplingTime) { samplingTime_ = samplingTime; }

void AudioRecorder::loop() {
    std::vector<uint8_t> buffer(bufferSize_);
    while (isRunning_) {
        int error{};
        if (pa_simple_read(handle_, buffer.data(), buffer.size(), &error) < 0) {
            break;
        }
        {
            std::lock_guard lock(mutex_);
            audioDataQueue_.push(buffer); // 将数据推入队列
        }
    }
}
