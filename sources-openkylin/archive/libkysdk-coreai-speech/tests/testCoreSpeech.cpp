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

#include <gio/gio.h>
#include <gio/giotypes.h>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>
#include <vector>

#include "_speechmodelconfig.h"
#include "coreai/speech/audiodatastream.h"
#include "logger.h"
#include "recognizer.h"
#include "synthesizer.h"

using namespace std::chrono_literals;

void callback(SpeechRecognitionResult *result, void *user_data) {
    LOGI("reason      :", speech_recognition_result_get_reason(result));
    // LOGI("reason      :", speech_recognition_result_get_reason_str(result));
    LOGI("text        :", speech_recognition_result_get_text(result));
    LOGI("err code    :", speech_recognition_result_get_error_code(result));
    LOGI("err message :", speech_recognition_result_get_error_message(result));

    if (user_data != nullptr) {
        LOGD(*reinterpret_cast<int *>(user_data));
    } else {
        LOGD("user data is nullptr");
    }
}

void test01_ContinuousRecognitionFromDefaultMic() {
    SpeechRecognitionSession *session = speech_recognizer_create_session();
    // "讯飞-语音大模型" or "百度-语音大模型"
    SpeechModelConfig *modelconfig = speech_model_config_create();
    speech_model_config_set_name(modelconfig, "讯飞-语音大模型");
    speech_model_config_set_deploy_type(modelconfig, ModelDeployType::PublicCloud);
    speech_recognizer_set_model_config(session, modelconfig);

    speech_recognizer_init_session(session);
    speech_recognizer_result_set_callback(session, callback, nullptr);

    auto *config = audio_config_create_continuous_audio_input_from_default_microphone();
    speech_recognizer_set_audio_config(session, config);

    speech_recognizer_start_continuous_recognition_async(session);

    GMainLoop *pMainLoop = g_main_loop_new(nullptr, false);

    std::thread ctrlThread([&config, &session, pMainLoop, &modelconfig] {
        while (std::getchar() != '\n') {
        }
        speech_recognizer_stop_continuous_recognition_async(session);
        audio_config_destroy(&config);
        speech_recognizer_destroy_session(&session);
        speech_model_config_destroy(&modelconfig);
        g_main_loop_quit(pMainLoop);
    });
    ctrlThread.detach();
    g_main_loop_run(pMainLoop);
    g_main_loop_unref(pMainLoop);
}

std::vector<uint8_t> readAudioData(const std::string &filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        LOGE("Failed to open file:", filePath);
        return {};
    }

    file.seekg(0, std::ios::end);
    std::streampos fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<uint8_t> audioData(fileSize);
    file.read(reinterpret_cast<char *>(audioData.data()), fileSize);
    return audioData;
}

void test02_ContinuousRecognitionFromRawData() {
    namespace fs = std::filesystem;
    fs::path dir = fs::path(__FILE__).parent_path();
    fs::path path = dir / "十秒音频.pcm";
    if (not fs::exists(path)) {
        LOGE("error");
        return;
    }
    std::vector<uint8_t> audioData = readAudioData(path.string());

    SpeechRecognitionSession *session = speech_recognizer_create_session();

    // "讯飞-语音大模型" or "百度-语音大模型"
    SpeechModelConfig *modelconfig = speech_model_config_create();
    speech_model_config_set_name(modelconfig, "讯飞-语音大模型");
    speech_model_config_set_deploy_type(modelconfig, ModelDeployType::PublicCloud);
    speech_recognizer_set_model_config(session, modelconfig);

    speech_recognizer_init_session(session);
    int a = 100;
    speech_recognizer_result_set_callback(session, callback, &a);

    auto *config = audio_config_create_once_audio_input_from_pcm_data(audioData.data(), audioData.size());

    speech_recognizer_set_audio_config(session, config);

    speech_recognizer_start_continuous_recognition_async(session);

    GMainLoop *pMainLoop = g_main_loop_new(nullptr, false);

    std::thread ctrlThread([&config, &session, pMainLoop, &modelconfig] {
        while (std::getchar() != '\n') {
        }
        speech_recognizer_stop_continuous_recognition_async(session);
        audio_config_destroy(&config);
        speech_recognizer_destroy_session(&session);
        speech_model_config_destroy(&modelconfig);
        g_main_loop_quit(pMainLoop);
    });
    ctrlThread.detach();
    g_main_loop_run(pMainLoop);
    g_main_loop_unref(pMainLoop);
}

void test03_ContinuousRecognitionFromStream() {
    namespace fs = std::filesystem;
    fs::path dir = fs::path(__FILE__).parent_path();
    fs::path path = dir / "十秒音频.pcm";
    if (not fs::exists(path)) {
        LOGE("error");
        return;
    }
    std::vector<uint8_t> audioData = readAudioData(path.string());

    SpeechRecognitionSession *session = speech_recognizer_create_session();

    // "讯飞-语音大模型" or "百度-语音大模型"
    SpeechModelConfig *modelconfig = speech_model_config_create();
    speech_model_config_set_name(modelconfig, "讯飞-语音大模型");
    speech_model_config_set_deploy_type(modelconfig, ModelDeployType::PublicCloud);
    speech_recognizer_set_model_config(session, modelconfig);

    speech_recognizer_init_session(session);
    speech_recognizer_result_set_callback(session, callback, nullptr);

    AudioDataStream *audio_data_stream = audio_data_stream_create();

    AudioConfig *config = audio_config_create_continuous_audio_input_from_audio_data_stream(audio_data_stream);

    LOGD(audioData.size());

    speech_recognizer_set_audio_config(session, config);

    speech_recognizer_start_continuous_recognition_async(session);

    for (int i = 0; i < audioData.size(); i += 1280) {
        std::this_thread::sleep_for(40ms);
        audio_data_stream_write(audio_data_stream, audioData.data() + i, 1280);
    }

    GMainLoop *pMainLoop = g_main_loop_new(nullptr, false);

    std::thread ctrlThread([&session, &config, &audio_data_stream, pMainLoop, &modelconfig] {
        while (std::getchar() != '\n') {
        }
        speech_recognizer_stop_continuous_recognition_async(session);

        audio_config_destroy(&config);
        audio_data_stream_destroy(&audio_data_stream);
        speech_recognizer_destroy_session(&session);
        speech_model_config_destroy(&modelconfig);
        g_main_loop_quit(pMainLoop);
    });
    ctrlThread.detach();
    g_main_loop_run(pMainLoop);
    g_main_loop_unref(pMainLoop);
}

void test04_ContinuousRecognitionFromFile() {
    namespace fs = std::filesystem;
    fs::path dir = fs::path(__FILE__).parent_path();
    fs::path path = dir / "北京科技馆.pcm";
    if (not fs::exists(path)) {
        LOGE("error");
        return;
    }

    SpeechRecognitionSession *session = speech_recognizer_create_session();

    // "讯飞-语音大模型" or "百度-语音大模型"
    SpeechModelConfig *modelconfig = speech_model_config_create();
    speech_model_config_set_name(modelconfig, "讯飞-语音大模型");
    speech_model_config_set_deploy_type(modelconfig, ModelDeployType::PublicCloud);
    speech_recognizer_set_model_config(session, modelconfig);

    speech_recognizer_init_session(session);
    speech_recognizer_result_set_callback(session, callback, nullptr);

    auto *config = audio_config_create_once_audio_input_from_pcm_file(path.c_str());

    speech_recognizer_set_audio_config(session, config);

    speech_recognizer_start_continuous_recognition_async(session);

    GMainLoop *pMainLoop = g_main_loop_new(nullptr, false);

    std::thread ctrlThread([&session, &config, pMainLoop, &modelconfig] {
        while (std::getchar() != '\n') {
        }
        speech_recognizer_stop_continuous_recognition_async(session);
        audio_config_destroy(&config);
        speech_recognizer_destroy_session(&session);
        speech_model_config_destroy(&modelconfig);
        g_main_loop_quit(pMainLoop);
    });
    ctrlThread.detach();
    g_main_loop_run(pMainLoop);
    g_main_loop_unref(pMainLoop);
}

void test05_RecognitionOnce() {
    namespace fs = std::filesystem;
    fs::path dir = fs::path(__FILE__).parent_path();
    fs::path path = dir / "北京科技馆.pcm";
    if (not fs::exists(path)) {
        LOGE("error");
        return;
    }
    std::vector<uint8_t> audioData = readAudioData(path.string());

    SpeechRecognitionSession *session = speech_recognizer_create_session();

    // "讯飞-语音大模型" or "百度-语音大模型"
    SpeechModelConfig *modelconfig = speech_model_config_create();
    speech_model_config_set_name(modelconfig, "讯飞-语音大模型");
    speech_model_config_set_deploy_type(modelconfig, ModelDeployType::PublicCloud);
    speech_recognizer_set_model_config(session, modelconfig);

    speech_recognizer_init_session(session);
    speech_recognizer_result_set_callback(session, callback, nullptr);

    auto *config = audio_config_create_once_audio_input_from_pcm_data(audioData.data(), audioData.size());

    LOGD(audioData.size());

    speech_recognizer_set_audio_config(session, config);

    speech_recognizer_recognize_once_async(session);

    GMainLoop *pMainLoop = g_main_loop_new(nullptr, false);
    std::thread ctrlThread([&session, &config, pMainLoop, &modelconfig] {
        while (std::getchar() != '\n') {
        }
        speech_recognizer_stop_continuous_recognition_async(session);
        audio_config_destroy(&config);
        speech_recognizer_destroy_session(&session);
        speech_model_config_destroy(&modelconfig);
        g_main_loop_quit(pMainLoop);
    });
    ctrlThread.detach();
    g_main_loop_run(pMainLoop);
    g_main_loop_unref(pMainLoop);
}

static void writeBinaryDataToFile(const std::string &filename, const std::vector<char> &data) {
    if (data.size() == 0) {
        std::cerr << "data is empty." << std::endl;
        return;
    }
    std::ofstream outputFile(filename, std::ios::out | std::ios::binary | std::ios::app);
    if (!outputFile.is_open()) {
        std::cerr << "Failed to open file for writing." << std::endl;
        return;
    }
    // 写入数据到文件
    outputFile.write(data.data(), data.size());

    // 检查是否写入成功
    if (!outputFile.good()) {
        std::cerr << "Error occurred while writing to file." << std::endl;
    } else {
        std::cout << "Binary data has been written to file: " << filename << std::endl;
    }

    // 关闭文件
    outputFile.close();
}

void onSynthesisResult(SpeechSynthesisResult *result, void *userData) {
    const char *userdata = static_cast<const char *>(userData);

    uint32_t audioDataLength;
    const uint8_t *audioData = speech_synthesis_result_get_data(result, &audioDataLength);
    SpeechResultReason reason = speech_synthesis_result_get_reason(result);
    std::fprintf(stdout, "test Synthesis result reason=%i audioDataLength=%i userData=%s \n", (int)reason,
                 (int)audioDataLength, userdata);

    std::vector<char> data{audioData, audioData + audioDataLength};
    writeBinaryDataToFile("./test.mp3", data);
}

void test06_Synthesizer() {
    // test synthesizer
    auto *config = audio_config_create_audio_output_from_default_speaker();

    SpeechSynthesizerSession *session = speech_synthesizer_create_session();

    // "讯飞-语音大模型" or "百度-语音大模型"
    SpeechModelConfig *modelconfig = speech_model_config_create();
    speech_model_config_set_name(modelconfig, "讯飞-语音大模型");
    speech_model_config_set_deploy_type(modelconfig, ModelDeployType::PublicCloud);
    speech_synthesizer_set_model_config(session, modelconfig);

    std::string synthesizingResult = "synthesizing result";
    speech_synthesizer_result_set_callback(session, onSynthesisResult, (void *)synthesizingResult.c_str());

    speech_synthesizer_init_session(session);
    audio_config_set_input_audio_rate(config, 16000);
    audio_config_set_output_audio_speed(config, 90);
    audio_config_set_output_audio_volume(config, 90);
    audio_config_set_output_audio_pitch(config, 60);

    speech_synthesizer_set_audio_config(session, config);

    std::string str =
        "你好，百度AI大模型"
        "你好，百度AI大模型"
        "你好，百度AI大模型"
        "你好，百度AI大模型";
    speech_synthesizer_synthesize_text_async(session, str.data(), str.size());

    speech_synthesizer_stop_speaking(session);

    auto *pMainLoop = g_main_loop_new(nullptr, false);

    std::thread ctrlThread([&session, &config, pMainLoop, &modelconfig] {
        while (std::getchar() != '\n') {
        }
        audio_config_destroy(&config);
        speech_synthesizer_destroy_session(&session);
        speech_model_config_destroy(&modelconfig);
        g_main_loop_quit(pMainLoop);
    });
    ctrlThread.detach();

    g_main_loop_run(pMainLoop);
    g_main_loop_unref(pMainLoop);
}

int main() {
    LOGI("test01_ContinuousRecognitionFromDefaultMic");
    test01_ContinuousRecognitionFromDefaultMic();

    LOGI("test02_ContinuousRecognitionFromRawData");
    test02_ContinuousRecognitionFromRawData();

    LOGI("test03_ContinuousRecognitionFromStream");
    test03_ContinuousRecognitionFromStream();

    LOGI("test04_ContinuousRecognitionFromFile");
    test04_ContinuousRecognitionFromFile();

    LOGI("test05_RecognitionOnce");
    test05_RecognitionOnce();

    LOGI("test06_Synthesizer");
    test06_Synthesizer();

    return 0;
}
