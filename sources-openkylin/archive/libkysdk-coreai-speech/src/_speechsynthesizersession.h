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

#ifndef _SPEECHSYNTHESIZER_
#define _SPEECHSYNTHESIZER_

#include "_audioconfig.h"
#include "_speechmodelconfig.h"
#include "coreaispeechserviceproxy.h"
#include "result.h"

namespace kyai {
namespace core {
namespace speech {

class _SpeechSynthesizerSession {
public:
    _SpeechSynthesizerSession();

    ~_SpeechSynthesizerSession();

    int init();

    void destroySynthesizer();

    void setSynthesizerAudioConfig(_AudioConfig *config);

    void setSynthesizerResultCallback(SpeechSynthesisResultCallback callback, void *userData);

    void synthesizeTextOnce(const char *text, uint32_t textLength);

    int synthesizeTextOnceWithRequestId(const char *requestId, const char *text, uint32_t textLength);

    void synthesizeTextContinuous(const char *text, uint32_t textLength);

    int stopSpeaking();

    void setSynthesizerModelConfig(const SpeechModelConfig &config);

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
