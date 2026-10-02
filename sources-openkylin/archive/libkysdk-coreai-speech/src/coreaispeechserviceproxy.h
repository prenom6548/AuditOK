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

#ifndef _CORE_SPEECH_SERVICE_PROXY_H_
#define _CORE_SPEECH_SERVICE_PROXY_H_

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <thread>

#include "_speechmodelconfig.h"
#include "corespeechserviceglue.h"
#include "error.h"
#include "result.h"

namespace kyai {
namespace core {
namespace speech {

using ReconnectedCallback = std::function<void()>;

class CoreAiSpeechServiceProxy {
public:
    ~CoreAiSpeechServiceProxy();

    bool connectToServer();

    bool reconnectServer();

    void setRecognitionResultCallback(SpeechRecognitionResultCallback callback, void *userData);

    void setSynthesizerCallback(SpeechSynthesisResultCallback callback, void *userData);

    void setReconnectedCallback(const ReconnectedCallback &callback);

    int initRecognizer(const char *config, int &sessionId);

    int initSynthesizer(const char *config, int &sessionId);

    void destroyRecognizer(int sessionId);

    void destroySynthesizer(int sessionId);

    void setRecognizerAudioConfig(int sessionId, const std::string &audioConfig);
    void startContinuousRecognition(int sessionId);

    void stopContinuousRecognition(int sessionId);

    void continuousRecognitionWriteAudioData(int sessionId, const uint8_t *data, uint32_t length);

    void recognizeOnce(int sessionId);

    int recognizeOnceWithRequestId(int sessionId, const char *requestId);

    void setSynthesizerAudioConfig(int sessionId, const std::string &audioConfig);
    void synthesizeTextOnce(int sessionId, const char *text, uint32_t textLength);
    int synthesizeTextOnceWithRequestId(int sessionId, const char *requestId, const char *text, uint32_t textLength);
    void synthesizeTextContinuous(int sessionId, const char *text, uint32_t textLength);
    int stopSpeaking(int sessionId);

    void enableInternalEventLoop(bool enable);

private:
    void startInternalEventLoop();

    void stopInternalEventLoop();

    void connectSpeechRecognitionResultSignals(int sessionId);

    void connectSpeechSynthesisResultSignals(int sessionId);

    static void onRecognitionResultCallback(GDBusConnection *connection, const gchar *senderName,
                                            const gchar *objectPath, const gchar *interfaceName,
                                            const gchar *signalName, GVariant *parameters, gpointer userData);

    static void onSynthesisResult(GDBusConnection *connection, const gchar *senderName, const gchar *objectPath,
                                  const gchar *interfaceName, const gchar *signalName, GVariant *parameters,
                                  gpointer userData);

    static void onOnceRecognitionFinishedCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData);

    void handleOnceRecognitionError(int errorCode);

    void handleOnceRecognitionServerClosed();

    void handleOnceRecognitionResultTimeout();

    void handleOnceRecognitionGeneralError();

    static void onOnceSynthesisFinishedCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData);

    void handleOnceSynthesisErrorOccurred(int errorCode);

    void handleOnceSynthesisServerClosed();

    void handleOnceSynthesisResultTimeout();

    void handleOnceSynthesisGeneralError();

    void refreshRuntimeProtocolVersion();
    bool ensureProtocolSupportedForApi(const char *apiName, int *errorCode = nullptr);

private:
    SpeechRecognitionResultCallback recognitionCallback_{nullptr};
    void *recognitionUserData_{nullptr};

    SpeechSynthesisResultCallback synthesizerCallback_{nullptr};
    void *synthesizerUserData_{nullptr};

    guint recognitionSubscriptionId_{0};
    guint synthesizerSubscriptionId_{0};

    AiRuntimeCoreSpeechService *proxy_{nullptr};

    // 以下变量是为了重连
    ReconnectedCallback reconnectedCallback_{nullptr};
    int reconnectAttempts_{0};
    static constexpr int maxReconnectAttempts = 10;
    bool recognitionResultSignalConnected_{false};
    bool synthesisResultSignalConnected_{false};

    bool continuousRecognitionStarted_{false};

    std::atomic<bool> internalLoopRequested_{false};
    GMainContext *eventLoopContext_{nullptr};
    std::thread eventLoopThread_;
};

}  // namespace speech
}  // namespace core
}  // namespace kyai
#endif
