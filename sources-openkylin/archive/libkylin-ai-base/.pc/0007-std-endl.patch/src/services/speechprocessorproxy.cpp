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
    GVariant *int_error = g_variant_iter_next_value(&iter);

    char *text = (char*)g_variant_get_data(char_text);
    int *length = (int *)g_variant_get_data(int_length);
    int *error = (int *)g_variant_get_data(int_error);

    RecognitionResult recognitionResult {
        text, (unsigned int)(*length), *error };
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

void onRecognitionAudioResult(GDBusConnection *connection,
                              const gchar *sender_name,
                              const gchar *object_path,
                              const gchar *interface_name,
                              const gchar *signal_name,
                              GVariant *parameters,
                              gpointer user_data) {
    auto *proxy = static_cast<SpeechProcessorProxy *>(user_data);
    if (proxy->inputAudioDataCallback_ == nullptr) {
        std::cout << "RecognitionAudioResult callback is nullptr";
        return;
    }
    proxy->inputAudioDataCallback_(recognitionResult(parameters));
}

void onStopRecognitionAudioResult(GDBusConnection *connection,
                                  const gchar *sender_name,
                                  const gchar *object_path,
                                  const gchar *interface_name,
                                  const gchar *signal_name,
                                  GVariant *parameters,
                                  gpointer user_data) {
    auto *proxy = static_cast<SpeechProcessorProxy *>(user_data);
    if (proxy->stopRecognizingAudioCallback_ == nullptr) {
        std::cout << "StopRecognitionAudioResult callback is nullptr";
        return;
    }
    proxy->stopRecognizingAudioCallback_(recognitionResult(parameters));
}

void onRecognitionAudioOnceResult(GDBusConnection *connection,
                                  const gchar *sender_name,
                                  const gchar *object_path,
                                  const gchar *interface_name,
                                  const gchar *signal_name,
                                  GVariant *parameters,
                                  gpointer user_data) {
    auto *proxy = static_cast<SpeechProcessorProxy *>(user_data);
    if (proxy->recognizingCompleteAudioCallback_ == nullptr) {
        std::cout << "RecognitionAudioOnceResult callback is nullptr";
        return;
    }
    proxy->recognizingCompleteAudioCallback_(recognitionResult(parameters));
}

void onSynthesisAudioResult(GDBusConnection *connection,
                            const gchar *sender_name,
                            const gchar *object_path,
                            const gchar *interface_name,
                            const gchar *signal_name,
                            GVariant *parameters,
                            gpointer user_data) {
    auto *proxy = static_cast<SpeechProcessorProxy *>(user_data);
    if (proxy->inputContinuousTextCallback_ == nullptr) {
        std::cout << "SynthesisAudioResult callback is nullptr";
        return;
    }
    proxy->inputContinuousTextCallback_(synthesisResult(parameters));
}

void onSynthesisAudioOnceResult(GDBusConnection *connection,
                                const gchar *sender_name,
                                const gchar *object_path,
                                const gchar *interface_name,
                                const gchar *signal_name,
                                GVariant *parameters,
                                gpointer user_data) {
    auto *proxy = static_cast<SpeechProcessorProxy *>(user_data);
    if (proxy->recognizingCompleteTextCallback_ == nullptr) {
        std::cout << "SynthesisAudioOnceResult callback is nullptr";
        return;
    }
    proxy->recognizingCompleteTextCallback_(synthesisResult(parameters));
}


SpeechProcessorProxy::SpeechProcessorProxy() { init(); }

SpeechProcessorProxy::~SpeechProcessorProxy() { destroy(); }

void SpeechProcessorProxy::initEngine(
        const std::string &engineName, const std::string &appId,
        const std::string &apiKey, const std::string &secretKey,
        gpointer userData) {
    aisdk_speech_processor_call_init(
        delegate_, engineName.c_str(), appId.c_str(),
        apiKey.c_str(), secretKey.c_str(), nullptr,
        reinterpret_cast<GAsyncReadyCallback>(
            SpeechProcessorProxy::onInitEngineCallback),
        this);
}

void SpeechProcessorProxy::startRecognizingAudio(const char *audioProperties) {
    GError *error = nullptr;
    aisdk_speech_processor_call_start_recognizing_audio_sync(
        delegate_, audioProperties, nullptr, &error);
}

void SpeechProcessorProxy::inputAudioData(const char *audioData,
                                          int audioDataLength) {
    std::string data = base64Encode(std::string(audioData, audioDataLength));

    aisdk_speech_processor_call_input_audio_data(
        delegate_, data.data(), data.size(), nullptr,
        reinterpret_cast<GAsyncReadyCallback>(
            SpeechProcessorProxy::onInputAudioDataCallback),
        this);
}

void SpeechProcessorProxy::stopRecognizingAudio() {
    aisdk_speech_processor_call_stop_recognizing_audio(
        delegate_,
        nullptr,
        reinterpret_cast<GAsyncReadyCallback>(
            SpeechProcessorProxy::onStopRecognizingAudioCallback),
        this);
}

void SpeechProcessorProxy::recognizingCompleteAudio(const char *audioProperties,
                                                    const char *audioData,
                                                    int audioDataLength) {
    std::string data = base64Encode(std::string(audioData, audioDataLength));

    aisdk_speech_processor_call_recognizing_complete_audio(
        delegate_, audioProperties, data.data(), data.size(), nullptr,
        reinterpret_cast<GAsyncReadyCallback>(
            SpeechProcessorProxy::onRecognizingCompleteAudioCallback),
        this);
}

bool SpeechProcessorProxy::startSynthesizingAudio(const char *voiceProperties) {
    GError *error = nullptr;
    bool success = aisdk_speech_processor_call_start_synthesizing_audio_sync(
        delegate_, voiceProperties, nullptr, &error);

    return success;
}

void SpeechProcessorProxy::inputContinuousText(const char *text) {
    aisdk_speech_processor_call_input_continuous_text(
        delegate_, text, nullptr,
        reinterpret_cast<GAsyncReadyCallback>(
            SpeechProcessorProxy::onInputContinuousTextCallback),
        this);
}

void SpeechProcessorProxy::stopSynthesizingAudio() {
    aisdk_speech_processor_call_stop_synthesizing_audio(
        delegate_, nullptr,
        reinterpret_cast<GAsyncReadyCallback>(
            SpeechProcessorProxy::onStopSynthesizingAudioCallback),
        this);
}

void SpeechProcessorProxy::recognizingCompleteText(const char *voiceProperties,
                                                   const char *text) {
    aisdk_speech_processor_call_recognizing_complete_text(
        delegate_, voiceProperties, text, nullptr,
        reinterpret_cast<GAsyncReadyCallback>(
            SpeechProcessorProxy::onRecognizingCompleteTextCallback),
        this);
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
        interfaceName_, "RecognitionAudioResult", objectPath_, NULL,
        G_DBUS_SIGNAL_FLAGS_NONE, onRecognitionAudioResult, this, nullptr);

    g_dbus_connection_signal_subscribe(connection, nullptr,
        interfaceName_, "StopRecognitionAudioResult", objectPath_, NULL,
        G_DBUS_SIGNAL_FLAGS_NONE, onStopRecognitionAudioResult, this, nullptr);

    g_dbus_connection_signal_subscribe(connection, nullptr,
        interfaceName_, "RecognitionAudioOnceResult", objectPath_, NULL,
        G_DBUS_SIGNAL_FLAGS_NONE, onRecognitionAudioOnceResult, this, nullptr);

    g_dbus_connection_signal_subscribe(connection, nullptr,
        interfaceName_, "SynthesisAudioResult", objectPath_, NULL,
        G_DBUS_SIGNAL_FLAGS_NONE, onSynthesisAudioResult, this, nullptr);

    g_dbus_connection_signal_subscribe(connection, nullptr,
        interfaceName_, "SynthesisAudioOnceResult", objectPath_, NULL,
        G_DBUS_SIGNAL_FLAGS_NONE, onSynthesisAudioOnceResult, this, nullptr);
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

void SpeechProcessorProxy::printErrorMessage(GError *error, std::string callMethodName)
{
    g_printerr("Error get speech processor proxy %s result: %s\n",
                callMethodName.c_str(), error->message);
    g_error_free(error);
}

void SpeechProcessorProxy::onInitEngineCallback(AisdkSpeechProcessor *delegate,
                                                GAsyncResult *result, gpointer userData) {
    GError *error = nullptr;
    aisdk_speech_processor_call_init_finish(delegate, result, &error);
}

void SpeechProcessorProxy::onInputAudioDataCallback(AisdkSpeechProcessor *delegate,
                                                    GAsyncResult *result, gpointer userData) {
    auto *proxy = static_cast<SpeechProcessorProxy *>(userData);
    if (proxy->inputAudioDataCallback_ == nullptr) {
        return;
    }

    gchar *outTextData = nullptr;
    gint outTextDataLength;
    gint outErrorCode;
    GError *error = nullptr;
    bool success = aisdk_speech_processor_call_input_audio_data_finish(
        delegate, &outTextData, &outTextDataLength, &outErrorCode, result, &error);
    if (!success) {
        printErrorMessage(error, "input_audio_data");
        return;
    }
    g_free(outTextData);
}

void SpeechProcessorProxy::onStopRecognizingAudioCallback(AisdkSpeechProcessor *delegate,
                                                          GAsyncResult *result, gpointer userData) {
    auto *proxy = static_cast<SpeechProcessorProxy *>(userData);
    if (proxy->stopRecognizingAudioCallback_ == nullptr) {
        return;
    }

    GError *error = nullptr;
    bool success = aisdk_speech_processor_call_stop_recognizing_audio_finish(
        delegate, result, &error);

    if (!success) {
        printErrorMessage(error, "stop_recognizing_audio");
        return;
    }
}

void SpeechProcessorProxy::onRecognizingCompleteAudioCallback(AisdkSpeechProcessor *delegate,
                                                              GAsyncResult *result, gpointer userData) {
    auto *proxy = static_cast<SpeechProcessorProxy *>(userData);
    if (proxy->recognizingCompleteAudioCallback_ == nullptr) {
        return;
    }

    gchar *outTextData = nullptr;
    gint outTextDataLength;
    gint outErrorCode;
    GError *error = nullptr;
    bool success = aisdk_speech_processor_call_recognizing_complete_audio_finish(
        delegate, &outTextData, &outTextDataLength, &outErrorCode, result, &error);

    if (!success) {
        printErrorMessage(error, "recognizing_complete_audio");
        return;
    }
    g_free(outTextData);
}

void SpeechProcessorProxy::onInputContinuousTextCallback(AisdkSpeechProcessor *delegate,
                                                         GAsyncResult *result, gpointer userData) {
    auto *proxy = static_cast<SpeechProcessorProxy *>(userData);
    if (proxy->inputAudioDataCallback_ == nullptr) {
        return;
    }

    gchar *outAudioFormat = nullptr;
    gint outAudioRate;
    gint outAudioChannel;
    gchar *outAudioData = nullptr;
    gint outAudioDataLength;
    gint outErrorMsg;
    GError *error = nullptr;
    bool success = aisdk_speech_processor_call_input_continuous_text_finish(
                delegate, &outAudioFormat, &outAudioRate, &outAudioChannel,
                &outAudioData, &outAudioDataLength, &outErrorMsg,
                result, &error);

    if (!success) {
        printErrorMessage(error, "input_continuous_text");
        return;
    }
    g_free(outAudioFormat);
    g_free(outAudioData);
}

void SpeechProcessorProxy::onStopSynthesizingAudioCallback(AisdkSpeechProcessor *delegate,
                                                           GAsyncResult *result, gpointer userData) {
    auto *proxy = static_cast<SpeechProcessorProxy *>(userData);
    if (proxy->stopSynthesizingAudioCallback_ == nullptr) {
        return;
    }

    GError *error = nullptr;
    bool success = aisdk_speech_processor_call_stop_synthesizing_audio_finish(
                delegate, result, &error);

    if (!success) {
        printErrorMessage(error, "input_continuous_text");
        return;
    }
    proxy->stopSynthesizingAudioCallback_();
}

void SpeechProcessorProxy::onRecognizingCompleteTextCallback(AisdkSpeechProcessor *delegate,
                                                             GAsyncResult *result, gpointer userData) {
    auto *proxy = static_cast<SpeechProcessorProxy *>(userData);
    if (proxy->recognizingCompleteTextCallback_ == nullptr) {
        return;
    }

    gchar *outAudioFormat = nullptr;
    gint outAudioRate;
    gint outAudioChannel;
    gchar *outAudioData = nullptr;
    gint outAudioDataLength;
    gint outErrorMsg;
    GError *error = nullptr;
    bool success = aisdk_speech_processor_call_recognizing_complete_text_finish(
                delegate, &outAudioFormat, &outAudioRate, &outAudioChannel,
                &outAudioData, &outAudioDataLength, &outErrorMsg,
                result, &error);

    if (!success) {
        printErrorMessage(error, "input_continuous_text");
        return;
    }
    g_free(outAudioFormat);
    g_free(outAudioData);
}
