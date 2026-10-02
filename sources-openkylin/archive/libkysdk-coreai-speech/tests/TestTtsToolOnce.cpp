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
#include "coreai/speech/result.h"
#include "coreai/speech/synthesizer.h"

// test
#include "AudioPlayer.h"

// util
#include "logger.h"

// TODO no copyable
class OnceTtsTool {
public:
    OnceTtsTool() {
        stdinChannel_ = g_io_channel_unix_new(STDIN_FILENO);
        g_io_channel_set_encoding(stdinChannel_, nullptr, nullptr);
        g_io_channel_set_buffered(stdinChannel_, TRUE);
        g_io_channel_set_flags(stdinChannel_, G_IO_FLAG_NONBLOCK, nullptr);
        g_io_add_watch(stdinChannel_, GIOCondition(G_IO_IN | G_IO_HUP | G_IO_ERR), onStdinEnter, this);

        mainLoop_ = g_main_loop_new(nullptr, false);

        audioConfig_ = audio_config_create_audio_output_from_pcm_data();
        audio_config_set_output_audio_pitch(audioConfig_, 50);
        audio_config_set_output_audio_volume(audioConfig_, 50);
        audio_config_set_output_audio_speed(audioConfig_, 50);

        session_ = speech_synthesizer_create_session();
        int ret = speech_synthesizer_init_session(session_);
        if (ret < 0) {
            LOGE("创建session失败");
            return;
        }
        speech_synthesizer_set_audio_config(session_, audioConfig_);
        speech_synthesizer_result_set_callback(session_, onTts, this);
    }

    ~OnceTtsTool() {
        speech_synthesizer_destroy_session(&session_);
        audio_config_destroy(&audioConfig_);

        g_main_loop_unref(mainLoop_);
        g_io_channel_unref(stdinChannel_);
    }

    void run() { g_main_loop_run(mainLoop_); }

private:
    static void onTts(SpeechSynthesisResult *result, void *userData) {
        OnceTtsTool *tool = (OnceTtsTool *)userData;
        LOGI("开    始：==============================");
        LOGI("消息类型：", speech_synthesis_result_get_reason(result));
        LOGI("音频格式：", speech_synthesis_result_get_audio_format(result));
        LOGI("音频速率：", speech_synthesis_result_get_audio_rate(result));
        LOGI("音频通道：", speech_synthesis_result_get_audio_channel(result));
        LOGI("错 误 码：", speech_synthesis_result_get_error_code(result));
        LOGI("错误信息：", speech_synthesis_result_get_error_message(result));
        LOGI("结    束：==============================");

        const uint8_t *data{nullptr};
        uint32_t size{};
        data = speech_synthesis_result_get_data(result, &size);
        tool->player_.playing(data, size);
    }

    static gboolean onStdinEnter(GIOChannel *channel, GIOCondition condition, gpointer userData) {
        OnceTtsTool *tool = (OnceTtsTool *)userData;
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
        } else {
            std::string text = input;
            speech_synthesizer_synthesize_text_async(session_, text.data(), text.size());
        }
    }

private:
    AudioConfig *audioConfig_{nullptr};
    SpeechSynthesizerSession *session_{nullptr};
    GMainLoop *mainLoop_{nullptr};
    GIOChannel *stdinChannel_{nullptr};
    AudioPlayer player_{};
};

int main() {
    try {
        OnceTtsTool tool{};
        tool.run();
    } catch (std::exception &exception) {
        LOGE("测试异常：", exception.what());
        return -1;
    }
    return 0;
}
