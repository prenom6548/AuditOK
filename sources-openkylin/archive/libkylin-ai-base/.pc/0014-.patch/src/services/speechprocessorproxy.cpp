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
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "services/speechprocessorproxy.h"

#include <iostream>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <iomanip>
#include "services/serverproxy.h"

RecognitionResult recognitionResult(GVariant *parameters) {
    GVariantIter iter;
    g_variant_iter_init(&iter, parameters);

    GVariant *char_text = g_variant_iter_next_value(&iter);
    GVariant *int_length = g_variant_iter_next_value(&iter);
    GVariant *int_speaker = g_variant_iter_next_value(&iter);
    GVariant *int_error = g_variant_iter_next_value(&iter);

    char *text = (char*)g_variant_get_data(char_text);
    int *length = (int *)g_variant_get_data(int_length);
    int *speaker = (int *)g_variant_get_data(int_speaker);
    int *error = (int *)g_variant_get_data(int_error);

    RecognitionResult recognitionResult {
        text, (unsigned int)(*length), *speaker, *error };
    return recognitionResult;
}

SynthesisResult synthesisResult(GVariant *parameters) {
    GVariantIter iter;
    g_variant_iter_init(&iter, parameters);

    GVariant *array_data = g_variant_iter_next_value(&iter); // 获取字节数组
    GVariant *int_audioFormat = g_variant_iter_next_value(&iter); // 获取整数
    GVariant *int_audioRate = g_variant_iter_next_value(&iter);
    GVariant *int_audioChannel = g_variant_iter_next_value(&iter);
    GVariant *int_error = g_variant_iter_next_value(&iter);

    char* audioFormat = (char*)g_variant_get_data(int_audioFormat);
    int* audioRate = (int*)g_variant_get_data(int_audioRate);
    int* audioChannel = (int*)g_variant_get_data(int_audioChannel);
    int* error = (int*)g_variant_get_data(int_error);

    gchar *arr = (gchar*)g_variant_get_data(array_data);
    std::vector<char> vec { arr, arr + g_variant_get_size(array_data) - 1 };

    SynthesisResult synthesisResult {
        audioFormat, *audioRate, *audioChannel, (unsigned char*)vec.data(),
        (unsigned int)vec.size(), *error };
    return synthesisResult;
}

void onRecognizingAudioResult(GDBusConnection *connection,
                              const gchar *sender_name,
                              const gchar *object_path,
                              const gchar *interface_name,
                              const gchar *signal_name,
                              GVariant *parameters,
                              gpointer user_data) {
    auto *proxy = static_cast<SpeechProcessorProxy *>(user_data);
    if (proxy->recognizingAudioCallback_ == nullptr) {
        std::cout << "RecognitionAudioResult callback is nullptr" << std::endl;
        return;
    }
    proxy->recognizingAudioCallback_(recognitionResult(parameters));
}

void onStopRecognizingAudioResult(GDBusConnection *connection,
                                  const gchar *sender_name,
                                  const gchar *object_path,
                                  const gchar *interface_name,
                                  const gchar *signal_name,
                                  GVariant *parameters,
                                  gpointer user_data) {
    auto *proxy = static_cast<SpeechProcessorProxy *>(user_data);
    if (proxy->stopRecognizingAudioCallback_ == nullptr) {
        std::cout << "StopRecognitionAudioResult callback is nullptr" << std::endl;
        return;
    }
    proxy->stopRecognizingAudioCallback_(recognitionResult(parameters));
}


void onSynthesizingAudioResult(GDBusConnection *connection,
                               const gchar *sender_name,
                               const gchar *object_path,
                               const gchar *interface_name,
                               const gchar *signal_name,
                               GVariant *parameters,
                               gpointer user_data) {
    auto *proxy = static_cast<SpeechProcessorProxy *>(user_data);
    if (proxy->synthesizingAudioCallback_ == nullptr) {
        std::cout << "SynthesisAudioResult callback is nullptr" << std::endl;
        return;
    }
    proxy->synthesizingAudioCallback_(synthesisResult(parameters));
}

void onStopSynthesizingAudioResult(GDBusConnection *connection,
                                   const gchar *sender_name,
                                   const gchar *object_path,
                                   const gchar *interface_name,
                                   const gchar *signal_name,
                                   GVariant *parameters,
                                   gpointer user_data) {
    auto *proxy = static_cast<SpeechProcessorProxy *>(user_data);
    if (proxy->stopSynthesizingAudioCallback_ == nullptr) {
        std::cout << "stopSynthesizingAudio callback is nullptr" << std::endl;
        return;
    }
    proxy->stopSynthesizingAudioCallback_();
}

SpeechProcessorProxy::SpeechProcessorProxy() { init(); }

SpeechProcessorProxy::~SpeechProcessorProxy() { destroy(); }

void SpeechProcessorProxy::initEngine(
        const std::string &engineName, const std::string &appId,
        const std::string &apiKey, const std::string &secretKey,
        gpointer userData) {
    aisdk_speech_processor_call_init(
        delegate_, engineName.c_str(), appId.c_str(),
        apiKey.c_str(), secretKey.c_str(), nullptr, nullptr, this);
}

void SpeechProcessorProxy::startRecognizingAudio(const char *audioProperties) {
    GError *error = nullptr;
    aisdk_speech_processor_call_start_recognizing_audio_sync(
        delegate_, audioProperties, nullptr, &error);
}

void SpeechProcessorProxy::inputAudioData(const char *audioData,
                                          int audioDataLength) {
    std::string data = base64Encode(std::string(audioData, audioDataLength));
    //data.data()是编码后的数据
    //audioDataLength是原始数据的数据长度
    aisdk_speech_processor_call_input_audio_data(
        delegate_, data.data(), audioDataLength, nullptr, nullptr, this);
}

void SpeechProcessorProxy::stopRecognizingAudio() {
    aisdk_speech_processor_call_stop_recognizing_audio(
        delegate_, nullptr, nullptr, this);
}

void SpeechProcessorProxy::recognizingCompleteAudio(const char *audioProperties,
                                                    const char *audioData,
                                                    int audioDataLength) {
    std::string data = base64Encode(std::string(audioData, audioDataLength));
    //data.data()是编码后的数据
    //audioDataLength是原始数据的数据长度
    aisdk_speech_processor_call_recognizing_complete_audio(
        delegate_, audioProperties, data.data(), audioDataLength,
        nullptr, nullptr, this);
}

bool SpeechProcessorProxy::startSynthesizingAudio(const char *voiceProperties) {
    GError *error = nullptr;
    bool success = aisdk_speech_processor_call_start_synthesizing_audio_sync(
        delegate_, voiceProperties, nullptr, &error);

    return success;
}

void SpeechProcessorProxy::inputContinuousText(const char *text) {
    aisdk_speech_processor_call_input_continuous_text(
        delegate_, text, nullptr, nullptr, this);
}

void SpeechProcessorProxy::stopSynthesizingAudio() {
    aisdk_speech_processor_call_stop_synthesizing_audio(
        delegate_, nullptr, nullptr, this);
}

void SpeechProcessorProxy::recognizingCompleteText(const char *voiceProperties,
                                                   const char *text) {
    aisdk_speech_processor_call_recognizing_complete_text(
        delegate_, voiceProperties, text, nullptr, nullptr, this);
}


void SpeechProcessorProxy::init() {
    if (!ServerProxy::getInstance().available()) {
        g_printerr("Error creating speech processor proxy: Server proxy "
                   "connection is unavailable.\n");
        return;
    }
    connectSignals();
}

void SpeechProcessorProxy::destroy() {
    if (delegate_ != nullptr) {
        g_object_unref(delegate_);
    }
}

void SpeechProcessorProxy::connectSignals() {
    GError *error = nullptr;
    auto *connection = ServerProxy::getInstance().getConnection();
    delegate_ = aisdk_speech_processor_proxy_new_sync(
        connection, G_DBUS_PROXY_FLAGS_NONE, nullptr, objectPath_,
        nullptr, &error);
    if (delegate_ == nullptr) {
        g_printerr("Error creating speech processor proxy %s: %s\n",
                   objectPath_, error->message);
        g_error_free(error);
    }

    g_dbus_connection_signal_subscribe(connection, nullptr,
        interfaceName_, "RecognizingAudioResult", objectPath_, NULL,
        G_DBUS_SIGNAL_FLAGS_NONE, onRecognizingAudioResult, this, nullptr);

    g_dbus_connection_signal_subscribe(connection, nullptr,
        interfaceName_, "StopRecognizingAudioResult", objectPath_, NULL,
        G_DBUS_SIGNAL_FLAGS_NONE, onStopRecognizingAudioResult, this, nullptr);

    g_dbus_connection_signal_subscribe(connection, nullptr,
        interfaceName_, "SynthesizingAudioResult", objectPath_, NULL,
        G_DBUS_SIGNAL_FLAGS_NONE, onSynthesizingAudioResult, this, nullptr);

    g_dbus_connection_signal_subscribe(connection, nullptr,
        interfaceName_, "StopSynthesizingAudioResult", objectPath_, NULL,
        G_DBUS_SIGNAL_FLAGS_NONE, onStopSynthesizingAudioResult, this, nullptr);

}

std::string SpeechProcessorProxy::base64Encode(const std::string &input) {
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;
    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);
    BIO_write(bio, input.c_str(), input.length());
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);
    std::string result(bufferPtr->data, bufferPtr->length);

    BIO_free_all(bio);

    return result;
}
