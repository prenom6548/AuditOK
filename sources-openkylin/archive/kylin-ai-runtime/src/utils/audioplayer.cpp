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

#include "audioplayer.h"

AudioPlayer::AudioPlayer() {
    if (not open()) {
        LOGE("AudioPlayer init error");
    }
}

AudioPlayer::~AudioPlayer() { close(); }

void AudioPlayer::playing(const void* data, size_t size) {
    std::lock_guard<std::mutex> locker(mutex_);
    if (!playing_) {
        audioDataCache_.clear();
        return;
    }
    if (simple_ == nullptr or data == nullptr or size == 0) {
        return;
    }

    // 1280字节是40毫秒的音频数据（16k采样 单通道 16bit存储）
    // 16000*2*1*0.04=1280
    const uint8_t* ptr{reinterpret_cast<const uint8_t*>(data)};
    audioDataCache_.insert(audioDataCache_.end(), ptr, ptr + size);
    while (audioDataCache_.size() > 1280) {
        if (!playing_) {
            audioDataCache_.clear();
            return;
        }
        int error;
        int ret = pa_simple_write(simple_, audioDataCache_.data(), 1280, &error);
        if (ret < 0) {
            LOGE("playing error {}", pa_strerror(error));
        }
        // 这里可以优化一下
        audioDataCache_.erase(audioDataCache_.begin(), audioDataCache_.begin() + 1280);
    }
}

void AudioPlayer::playing(const std::vector<uint8_t>& audioData) { playing(audioData.data(), audioData.size()); }

void AudioPlayer::setPlaying(bool playing) { playing_ = playing; }

bool AudioPlayer::open() {
    // 创建 PulseAudio 简单接口
    pa_sample_spec sampleSpec;
    sampleSpec.format = PA_SAMPLE_S16LE;  // 16-bit 小端格式
    sampleSpec.rate = 16000;              // 16k 采样率
    sampleSpec.channels = 1;              // 单通道

    int error;
    simple_ = pa_simple_new(nullptr,             // 默认服务器
                            "Audio Player",      // 应用名称
                            PA_STREAM_PLAYBACK,  // 播放流
                            nullptr,             // 默认设备
                            "Audio",             // 流描述
                            &sampleSpec,         // 音频参数
                            nullptr,             // 默认属性
                            nullptr,             // 错误码
                            &error               // 存储错误码
    );
    return simple_ != nullptr;
}

void AudioPlayer::close() {
    std::lock_guard<std::mutex> locker(mutex_);
    if (simple_ != nullptr) {
        pa_simple_free(simple_);
        simple_ = nullptr;
    }
}
