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

// c
#include <cstdio>

// cpp
#include <filesystem>
#include <fstream>
#include <thread>
#include <vector>

// glib
#include <gio/gio.h>

// kysdk-ai coreai speech

#include "coreai/speech/audioconfig.h"
#include "coreai/speech/audiodatastream.h"
#include "coreai/speech/error.h"
#include "coreai/speech/recognizer.h"
#include "coreai/speech/result.h"

// test
#include "AudioRecorder.h"

// util
#include "logger.h"

// 开始：测试通过麦克风语音识别
// TODO no copyable
class OnceAsrTool {
public:
    OnceAsrTool() {
        stdinChannel_ = g_io_channel_unix_new(STDIN_FILENO);
        g_io_channel_set_encoding(stdinChannel_, nullptr, nullptr);
        g_io_channel_set_buffered(stdinChannel_, TRUE);
        g_io_channel_set_flags(stdinChannel_, G_IO_FLAG_NONBLOCK, nullptr);
        g_io_add_watch(stdinChannel_, GIOCondition(G_IO_IN | G_IO_HUP | G_IO_ERR), onStdinEnter, this);

        mainLoop_ = g_main_loop_new(nullptr, false);

        session_ = speech_recognizer_create_session();
        int ret = speech_recognizer_init_session(session_);
        if (ret < 0) {
            LOGE("创建session失败");
            return;
        }
        speech_recognizer_result_set_callback(session_, onAsr, nullptr);
    }

    ~OnceAsrTool() {
        speech_recognizer_destroy_session(&session_);
        audio_config_destroy(&audioConfig_);
        speech_model_config_destroy(&modelConfig_);

        g_main_loop_unref(mainLoop_);
        g_io_channel_unref(stdinChannel_);
    }

    void run() { g_main_loop_run(mainLoop_); }

private:
    static void onAsr(SpeechRecognitionResult *result, void *userData) {
        LOGI("开    始：==============================");
        LOGI("消息类型：", speech_recognition_result_get_reason(result));
        LOGI("识别信息：", speech_recognition_result_get_text(result));
        LOGI("错 误 码：", speech_recognition_result_get_error_code(result));
        LOGI("错误信息：", speech_recognition_result_get_error_message(result));
        LOGI("用户数据：", userData);
        LOGI("结    束：==============================");
    }

    static gboolean onStdinEnter(GIOChannel *channel, GIOCondition condition, gpointer userData) {
        OnceAsrTool *tool = (OnceAsrTool *)userData;
        gchar *line{nullptr};
        if (condition & G_IO_IN) {
            GIOStatus status = g_io_channel_read_line(channel, &line, nullptr, nullptr, nullptr);
            if (status == G_IO_STATUS_NORMAL) {
                std::string input(line);
                if (!input.empty() && input[input.size() - 1] == '\n') {
                    input.erase(input.size() - 1);
                }
                tool->control(input);
                g_free(line);
            }
        }
        return TRUE;
    }

    void control(const std::string &input) {
        if (input == "quit") {
            g_main_loop_quit(mainLoop_);
        } else if (input == "start") {
            LOGI("start");
            thread_ = std::make_shared<std::thread>([this] {
                while (!stopped_) {
                    recorder_.recording(audioData_);
                }
            });

        } else if (input == "stop") {
            LOGI("stop");
            stopped_ = true;
            thread_->join();
            uint8_t *data = (uint8_t *)(audioData_.data());
            size_t size = audioData_.size();
            audioConfig_ = audio_config_create_once_audio_input_from_pcm_data(data, size);
            speech_recognizer_set_audio_config(session_, audioConfig_);
            speech_recognizer_recognize_once_async(session_);
        } else {
            LOGI();
        }
    }

private:
    SpeechModelConfig *modelConfig_{nullptr};
    AudioConfig *audioConfig_{nullptr};
    SpeechRecognitionSession *session_{nullptr};
    GMainLoop *mainLoop_{nullptr};
    GIOChannel *stdinChannel_{nullptr};
    std::shared_ptr<std::thread> thread_{nullptr};
    std::vector<char> audioData_{};
    AudioRecorder recorder_{};
    std::atomic_bool stopped_{false};
};

int main() {
    try {
        OnceAsrTool tool{};
        tool.run();
    } catch (std::exception &exception) {
        LOGE("测试异常：", exception.what());
        return -1;
    }
    return 0;
}
