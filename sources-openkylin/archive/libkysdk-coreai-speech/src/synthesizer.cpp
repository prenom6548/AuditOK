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

#include "synthesizer.h"

#include "_speechsynthesizersession.h"

namespace {
using SpeechSynthesizerSessionPtr = kyai::core::speech::_SpeechSynthesizerSession *;
}

SpeechSynthesizerSession *speech_synthesizer_create_session() {
    auto *session = new kyai::core::speech::_SpeechSynthesizerSession();
    return (SpeechSynthesizerSession *)session;
}

void speech_synthesizer_destroy_session(SpeechSynthesizerSession **session) {
    if (session == nullptr) {
        return;
    }
    if (*session == nullptr) {
        return;
    }
    ((kyai::core::speech::_SpeechSynthesizerSession *)*session)->destroySynthesizer();

    delete (kyai::core::speech::_SpeechSynthesizerSession *)*session;
    *session = nullptr;
}

int speech_synthesizer_init_session(SpeechSynthesizerSession *session) {
    if (session == nullptr) {
        return AISDK_INVALID_SESSION;
    }
    return ((kyai::core::speech::_SpeechSynthesizerSession *)session)->init();
}

void speech_synthesizer_set_audio_config(SpeechSynthesizerSession *session, AudioConfig *audio_config) {
    if (session == nullptr) {
        return;
    }
    if (audio_config == nullptr) {
        return;
    }
    ((kyai::core::speech::_SpeechSynthesizerSession *)session)->setSynthesizerAudioConfig((_AudioConfig *)audio_config);
}

void speech_synthesizer_result_set_callback(SpeechSynthesizerSession *session, SpeechSynthesisResultCallback callback,
                                            void *user_data) {
    if (session == nullptr) {
        return;
    }
    ((kyai::core::speech::_SpeechSynthesizerSession *)session)->setSynthesizerResultCallback(callback, user_data);}

void speech_synthesizer_set_model_config(SpeechSynthesizerSession *session, SpeechModelConfig *config) {
    if (session == nullptr or config == nullptr) {
        return;
    }
    ((kyai::core::speech::_SpeechSynthesizerSession *)session)->setSynthesizerModelConfig(*config);
}

void speech_synthesizer_synthesize_text_async(SpeechSynthesizerSession *session, const char *text,
                                              uint32_t text_length) {
    if (session == nullptr) {
        return;
    }
    ((kyai::core::speech::_SpeechSynthesizerSession *)session)->synthesizeTextOnce(text, text_length);
}

int speech_synthesizer_synthesize_text_async_with_request_id(SpeechSynthesizerSession *session, const char *request_id,
                                                             const char *text, uint32_t text_length) {
    if (session == nullptr) {
        return AI_COMMON_INVALID_SESSION;
    }
    return ((kyai::core::speech::_SpeechSynthesizerSession *)session)
        ->synthesizeTextOnceWithRequestId(request_id, text, text_length);
}

int speech_synthesizer_stop_speaking(SpeechSynthesizerSession *session) {
    if (session == nullptr) {
        return SPEECH_PARAM_INVALID;
    }
    return ((kyai::core::speech::_SpeechSynthesizerSession *)session)->stopSpeaking();
}

void speech_synthesizer_enable_internal_event_loop(SpeechSynthesizerSession *session, bool enable) {
    if (session == nullptr) {
        return;
    }
    SpeechSynthesizerSessionPtr(session)->enableInternalEventLoop(enable);
}
