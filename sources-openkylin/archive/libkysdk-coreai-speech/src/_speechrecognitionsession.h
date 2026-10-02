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

#ifndef _SPEECHRECOGNIZER_
#define _SPEECHRECOGNIZER_

#include "_audioconfig.h"
#include "_speechmodelconfig.h"
#include "coreaispeechserviceproxy.h"
#include "result.h"

namespace kyai {
namespace core {
namespace speech {

class _SpeechRecognitionSession {
public:
    _SpeechRecognitionSession();

    ~_SpeechRecognitionSession();

    int init();

    void destroyRecognizer();

    void setRecognizerAudioConfig(_AudioConfig *config);

    [[nodiscard]] _AudioConfig *getRecognizerAudioConfig() const;

    void setRecognitionResultCallback(SpeechRecognitionResultCallback callback, void *userData);

    void startContinuousRecognition();

    void stopContinuousRecognition();

    void continuousRecognitionWriteAudioData(const uint8_t *data, uint32_t length);

    void recognizeOnce();

    int recognizeOnceWithRequestId(const char *requestId);

    void setRecognitionModelConfig(const SpeechModelConfig &config);

    void enableInternalEventLoop(bool enable);

private:
    bool initServer();

    int initSession();

private:
    _AudioConfig *audioConfig_{nullptr};

    CoreAiSpeechServiceProxy serviceProxy_;

    int sessionId_{-1};

    SpeechModelConfig modelConfig_{};
};

}  // namespace speech
}  // namespace core
}  // namespace kyai

#endif
