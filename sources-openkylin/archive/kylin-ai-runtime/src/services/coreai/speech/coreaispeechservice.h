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

#ifndef SERVICES_CORE_AI_SPEECH_SERVICE_H
#define SERVICES_CORE_AI_SPEECH_SERVICE_H

#include <gio/gio.h>
#include <kylin-ai/aiengine.h>

#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>

#include "coreaispeechmodelconfig.h"
#include "corespeechserviceglue.h"
#include "engineselector/engineselector.h"
#include "speechrecognizer.h"
#include "speechsynthesizer.h"

class CoreAiSpeechService : public std::enable_shared_from_this<CoreAiSpeechService> {
public:
    CoreAiSpeechService(GDBusConnection& connection, EngineSelector& engineSelector);

    ~CoreAiSpeechService();

    void stop();

private:
    void exportSkeleton();
    void connectSignal();
    void unexportSkeleton();
    std::string getProtocolVersion();
    void setRuntimeProtocolVersion(const std::string& version);
    int generateRandomSessionId();
    void releaseRandomSessionId(int sessionId);
    bool isRecognizerSessionIdValid(int sessionId);
    bool isSynthesizerSessionIdValid(int sessionId);
    CoreSpeechModelConfig parseModelConfig(const std::string& jsonString);
    DeployType parseDeployType(const std::string& typeStr);

    void emitSynthesisResultSignal(const std::string& interfaceName, const SpeechSynthesisResult& result);

    bool initSpeechRecognizer(int& sessionId, int& errorCode, const CoreSpeechModelConfig& config);
    bool initSpeechSynthesizer(int& sessionId, int& errorCode, const CoreSpeechModelConfig& config);

    static bool onHandleInitRecognizer(AiRuntimeCoreSpeechService* service, GDBusMethodInvocation* invocation,
                                       const gchar* modelconfig, gpointer userData);

    static bool onHandleInitSynthesizer(AiRuntimeCoreSpeechService* service, GDBusMethodInvocation* invocation,
                                        const gchar* modelconfig, gpointer userData);

    static bool onHandleDestroyRecognizer(AiRuntimeCoreSpeechService* service, GDBusMethodInvocation* invocation,
                                          int sessionId, gpointer userData);

    static bool onHandleDestroySynthesizer(AiRuntimeCoreSpeechService* service, GDBusMethodInvocation* invocation,
                                           int sessionId, gpointer userData);

    static bool onHandleSetRecognizerAudioConfig(AiRuntimeCoreSpeechService* service, GDBusMethodInvocation* invocation,
                                                 int sessionId, gchar* audioConfig, gpointer userData);

    static bool onHandleStartContinuousRecognition(AiRuntimeCoreSpeechService* service,
                                                   GDBusMethodInvocation* invocation, gint sessionId,
                                                   gpointer userData);

    static bool onHandleStopContinuousRecognition(AiRuntimeCoreSpeechService* service,
                                                  GDBusMethodInvocation* invocation, gint sessionId, gpointer userData);

    static bool onHandleSetSynthesizerAudioConfig(AiRuntimeCoreSpeechService* service,
                                                  GDBusMethodInvocation* invocation, int sessionId, gchar* audioConfig,
                                                  gpointer userData);

    static bool onHandleStopSpeaking(AiRuntimeCoreSpeechService* service, GDBusMethodInvocation* invocation,
                                     gint sessionId, gpointer userData);

    static bool onHandleSynthesizeTextOnce(AiRuntimeCoreSpeechService* service, GDBusMethodInvocation* invocation,
                                           gint sessionId, gchar* text, gint textLength, gpointer userData);

    static bool onHandleSynthesizeTextOnceWithRequestId(AiRuntimeCoreSpeechService* service,
                                                        GDBusMethodInvocation* invocation, gint sessionId,
                                                        const gchar* requestId, gchar* text, gint textLength,
                                                        gpointer userData);

    static bool onHandleContinuousRecognitionWriteAudioData(AiRuntimeCoreSpeechService* service,
                                                            GDBusMethodInvocation* invocation, int sessionId,
                                                            gchar* audioData, gpointer userData);

    static bool onHandleRecognizeOnce(AiRuntimeCoreSpeechService* service, GDBusMethodInvocation* invocation,
                                      gint sessionId, gpointer userData);

    static bool onHandleRecognizeOnceWithRequestId(AiRuntimeCoreSpeechService* service,
                                                   GDBusMethodInvocation* invocation, gint sessionId,
                                                   const gchar* requestId, gpointer userData);

private:
    std::shared_ptr<CoreAiSpeechService> getSharedObject() { return shared_from_this(); }

    bool tryToChangeRecognizerModel(int sessionId, int& errorCode);

    bool tryToChangeSynthesizerModel(int sessionId, int& errorCode);

    std::string interfaceName(int sessionId);

    void sendRecognizerResult(int sessionId, const SpeechRecognitionResult& result);

    void setRecognizerCallback(int sessionId);

    void setSynthesizerCallback(int sessionId);

    using CompleteFunc = std::function<void()>;
    bool synthesizeTextOnceImpl(gint sessionId, const gchar* requestId, gchar* text,
                                const CompleteFunc& completeFunc);
    bool recognizeOnceImpl(gint sessionId, const gchar* requestId, const CompleteFunc& completeFunc);

private:
    bool isExported_ = false;
    AiRuntimeCoreSpeechService* delegate_ = nullptr;

    GDBusConnection& connection_;

    EngineSelector& engineSelector_;

    std::set<int> sessionIdSet_;
    std::map<int, std::shared_ptr<SpeechRecognizer>> speechRecognizers_;
    std::map<int, std::shared_ptr<SpeechSynthesizer>> speechSynthesizers_;
    std::map<int, std::atomic_bool> modelChangedMap_;
    std::vector<gulong> handlerIds_;
};

#endif  // SERVICES_CORE_AI_SPEECH_SERVICE_H
