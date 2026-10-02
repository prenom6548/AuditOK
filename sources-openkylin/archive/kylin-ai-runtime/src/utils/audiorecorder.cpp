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

#include "logger.h"
#include "util.h"

void AudioRecorder::getMicrophoneInfo(std::string &deviceName, bool &mute) {
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
        return;
    }

    pa_context_unref(context);
    pa_mainloop_free(mainLoop);

    if (userData.deviceName.empty()) {
        return;
    }

    if (util::stringContains(userData.deviceName, ".monitor") && util::stringContains(userData.deviceName, "output")) {
        LOGD("Device name: {}", userData.deviceName);
        return;
    }
    // 屏蔽名为"auto_null.monitor"的特殊音频输入设备，它无法正常输入音频，v11-2603系统
    if (userData.deviceName == "auto_null.monitor") {
        return;
    }

    deviceName = userData.deviceName;
    mute = userData.mute;
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
        pUserData->deviceName = info->name;
        pUserData->mute = info->mute;
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
            pa_operation_unref(
                pa_context_get_source_info_by_name(context, nullptr, getDefaultSourceCallback, userdata));
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

AudioRecorder::AudioRecorder() { setSamplingParams(16000, 1, 40); }

AudioRecorder::AudioRecorder(uint32_t sampleRate, uint32_t channels, uint32_t ms) {
    setSamplingParams(sampleRate, channels, ms);
}

AudioRecorder::~AudioRecorder() { stop(); }

bool AudioRecorder::start() {
    std::lock_guard locker(runningMutex_);
    if (isRunning_) {
        return false;
    }
    int error{};
    handle_ = pa_simple_new(nullptr,           // 使用默认服务器
                            "AudioRecorder",   // 客户端名称
                            PA_STREAM_RECORD,  // 录制流
                            nullptr,           // 使用默认设备
                            "Record",          // 流描述
                            &samplingParams_,  // 样本格式
                            nullptr,           // 使用默认通道映射
                            &attr_,
                            &error  // 错误码
    );
    if (handle_ == nullptr) {
        return false;
    }
    isRunning_ = true;
    thread_ = std::thread([this]() { loop(); });
    return true;
}

void AudioRecorder::stop() {
    std::lock_guard locker(runningMutex_);
    if (!isRunning_) {
        return;
    }
    isRunning_ = false;

    if (thread_.joinable()) {
        thread_.join();
    }

    // 在线程结束后，才能释放handle_，否则会线程不安全
    if (handle_ != nullptr) {
        pa_simple_free(handle_);
    }
}

std::vector<std::vector<uint8_t> > AudioRecorder::getAudioDatas() {
    std::lock_guard lock(queueMutex_);
    std::vector<std::vector<uint8_t> > datas;
    while (!audioDataQueue_.empty()) {
        datas.push_back(audioDataQueue_.front());
        audioDataQueue_.pop();
    }
    return datas;
}

void AudioRecorder::setSamplingParams(uint32_t sampleRate, uint32_t channels, uint32_t ms) {
    std::lock_guard locker(runningMutex_);
    if (isRunning_) {
        LOGW("AudioRecorder is running, parameter settings will not succeed!");
        return;
    }
    samplingParams_.rate = sampleRate;
    samplingParams_.channels = channels;
    samplingParams_.format = PA_SAMPLE_S16LE;  // 暂不考虑其他采样位数
    attr_.fragsize = (sampleRate * channels * sizeof(int16_t) * ms) / 1000;
}

void AudioRecorder::loop() {
    size_t bufferSize{};
    {
        std::lock_guard lock(runningMutex_);
        bufferSize = attr_.fragsize;
    }

    std::vector<uint8_t> buffer(bufferSize, 0);

    LOGD("Buffer bytes: {}", bufferSize);
    while (isRunning_) {
        buffer.assign(bufferSize, 0);
        int error{};
        int readBytes = pa_simple_read(handle_, buffer.data(), bufferSize, &error);
        if (readBytes < 0) {
            LOGE("AudioRecorder abnormal exit!");
            break;
        }
        {
            std::lock_guard lock(queueMutex_);
            audioDataQueue_.push(buffer);  // 将数据推入队列
        }
    }
}
