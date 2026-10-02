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

#ifndef SERVICES_SPEECHPROCESSORPROXY_H
#define SERVICES_SPEECHPROCESSORPROXY_H

#include <functional>
#include <string>

#include "services/speechprocessorglue.h"
#include <kylin-ai/ai-base/speech.h>

class SpeechProcessorProxy {
public:
    using SpeechRecognitionResultCallback = std::function<void(RecognitionResult result)>;
    using SpeechSynthesisResultCallback = std::function<void(SynthesisResult result)>;
    using SpeechFinishedCallback = std::function<void()>;

    SpeechProcessorProxy();
    ~SpeechProcessorProxy();

    void setInputAudioDataCallback(
            SpeechProcessorProxy::SpeechRecognitionResultCallback callback) {
        inputAudioDataCallback_ = callback;
    }
    void setStopRecognizingAudioCallback(
            SpeechProcessorProxy::SpeechRecognitionResultCallback callback) {
        stopRecognizingAudioCallback_ = callback;
    }
    void setRecognizingCompleteAudioCallback(
            SpeechProcessorProxy::SpeechRecognitionResultCallback callback) {
        recognizingCompleteAudioCallback_ = callback;
    }

    void setInputContinuousTextCallback(
            SpeechProcessorProxy::SpeechSynthesisResultCallback callback) {
        inputContinuousTextCallback_ = callback;
    }
    void setStopSynthesizingAudioCallback(
            SpeechProcessorProxy::SpeechFinishedCallback callback) {
        stopSynthesizingAudioCallback_ = callback;
    }
    void setRecognizingCompleteTextCallback(
            SpeechProcessorProxy::SpeechSynthesisResultCallback callback) {
        recognizingCompleteTextCallback_ = callback;
    }

    void initEngine(const std::string &engineName, const std::string &appId,
                    const std::string &apiKey, const std::string &secretKey,
                    gpointer userData);
    void startRecognizingAudio(const char *audioProperties);
    void inputAudioData(const char *audioData,
                        int audioDataLength);
    void stopRecognizingAudio();
    void recognizingCompleteAudio(const char *audioProperties,
                                  const char *audioData,
                                  int audioDataLength);

    bool startSynthesizingAudio(const char *voiceProperties);
    void inputContinuousText(const char *text);
    void stopSynthesizingAudio();

    void recognizingCompleteText(const char *voiceProperties,
                                 const char *text);

private:
    void init();
    void destroy();
    void connectSignals();
    std::string base64Encode(const std::string &input);

    friend RecognitionResult recognitionResult(GVariant *parameters);
    friend SynthesisResult synthesisResult(GVariant *parameters);
    friend void onRecognitionAudioResult(GDBusConnection *connection,
                                         const gchar *sender_name,
                                         const gchar *object_path,
                                         const gchar *interface_name,
                                         const gchar *signal_name,
                                         GVariant *parameters,
                                         gpointer user_data);
    friend void onStopRecognitionAudioResult(GDBusConnection *connection,
                                             const gchar *sender_name,
                                             const gchar *object_path,
                                             const gchar *interface_name,
                                             const gchar *signal_name,
                                             GVariant *parameters,
                                             gpointer user_data);
    friend void onRecognitionAudioOnceResult(GDBusConnection *connection,
                                             const gchar *sender_name,
                                             const gchar *object_path,
                                             const gchar *interface_name,
                                             const gchar *signal_name,
                                             GVariant *parameters,
                                             gpointer user_data);
    friend void onSynthesisAudioResult(GDBusConnection *connection,
                                       const gchar *sender_name,
                                       const gchar *object_path,
                                       const gchar *interface_name,
                                       const gchar *signal_name,
                                       GVariant *parameters,
                                       gpointer user_data);
    friend void onStopSynthesizingAudio(GDBusConnection *connection,
                                        const gchar *sender_name,
                                        const gchar *object_path,
                                        const gchar *interface_name,
                                        const gchar *signal_name,
                                        GVariant *parameters,
                                        gpointer user_data);
    friend void onSynthesisAudioOnceResult(GDBusConnection *connection,
                                           const gchar *sender_name,
                                           const gchar *object_path,
                                           const gchar *interface_name,
                                           const gchar *signal_name,
                                           GVariant *parameters,
                                           gpointer user_data);

private:
    AisdkSpeechProcessor *delegate_ = nullptr;

    SpeechRecognitionResultCallback inputAudioDataCallback_ = nullptr;
    SpeechRecognitionResultCallback stopRecognizingAudioCallback_ = nullptr;
    SpeechRecognitionResultCallback recognizingCompleteAudioCallback_ = nullptr;

    SpeechSynthesisResultCallback inputContinuousTextCallback_ = nullptr;
    SpeechFinishedCallback stopSynthesizingAudioCallback_ = nullptr;
    SpeechSynthesisResultCallback recognizingCompleteTextCallback_ = nullptr;

    const char *objectPath_ = "/org/openkylin/aisdk/speechprocessor";
    const char *interfaceName_ = "org.openkylin.aisdk.speechprocessor";
};
#endif
