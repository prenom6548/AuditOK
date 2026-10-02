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

#include "recognizer.h"

#include "_audiodatastream.h"
#include "_speechrecognitionsession.h"

namespace {
using SpeechRecognitionSessionPtr = kyai::core::speech::_SpeechRecognitionSession *;
}

SpeechRecognitionSession *speech_recognizer_create_session() {
    auto *session = new kyai::core::speech::_SpeechRecognitionSession();
    return (SpeechRecognitionSession *)session;
}

void speech_recognizer_destroy_session(SpeechRecognitionSession **session) {
    if (session == nullptr) {
        return;
    }
    if (*session == nullptr) {
        return;
    }
    ((kyai::core::speech::_SpeechRecognitionSession *)*session)->destroyRecognizer();

    delete (kyai::core::speech::_SpeechRecognitionSession *)*session;
    *session = nullptr;
}

int speech_recognizer_init_session(SpeechRecognitionSession *session) {
    if (session == nullptr) {
        return AISDK_INVALID_SESSION;
    }
    return ((kyai::core::speech::_SpeechRecognitionSession *)session)->init();
}

void speech_recognizer_result_set_callback(SpeechRecognitionSession *session, SpeechRecognitionResultCallback callback,
                                           void *user_data) {
    if (session == nullptr) {
        return;
    }
    ((kyai::core::speech::_SpeechRecognitionSession *)session)->setRecognitionResultCallback(callback, user_data);
}
void speech_recognizer_set_model_config(SpeechRecognitionSession *session, SpeechModelConfig *config) {
    if (session == nullptr or config == nullptr) {
        return;
    }
    ((kyai::core::speech::_SpeechRecognitionSession *)session)->setRecognitionModelConfig(*config);
}
static int speech_recognizer_continuous_recognition_write_audio_data(const uint8_t *audio_data, uint32_t audio_length,
                                                                     void *user_data) {
    if (user_data == nullptr) {
        return AiSdkCommonErrorCode::AISDK_RUNTIME_ERROR;
    }
    // 此函数为private
    ((kyai::core::speech::_SpeechRecognitionSession *)user_data)
        ->continuousRecognitionWriteAudioData(audio_data, audio_length);
    return AiSdkCommonErrorCode::AISDK_NO_ERROR;
}

void speech_recognizer_set_audio_config(SpeechRecognitionSession *session, AudioConfig *audio_config) {
    if (session == nullptr) {
        return;
    }
    if (audio_config == nullptr) {
        return;
    }
    ((kyai::core::speech::_SpeechRecognitionSession *)session)->setRecognizerAudioConfig(audio_config);
    if (audio_config->inputAudioInfo.inputSource != InputAudioInfo::InputSource::STREAM) {
        return;
    }
    if (audio_config->inputAudioInfo.audioDataStream == nullptr) {
        return;
    }
    audio_config->inputAudioInfo.audioDataStream->p_user_data = session;
    audio_config->inputAudioInfo.audioDataStream->audio_data_to_stream_write_func =
        speech_recognizer_continuous_recognition_write_audio_data;
}

void speech_recognizer_start_continuous_recognition_async(SpeechRecognitionSession *session) {
    if (session == nullptr) {
        return;
    }
    ((kyai::core::speech::_SpeechRecognitionSession *)session)->startContinuousRecognition();
}

// 停止连续语音识别
void speech_recognizer_stop_continuous_recognition_async(SpeechRecognitionSession *session) {
    if (session == nullptr) {
        return;
    }
    ((kyai::core::speech::_SpeechRecognitionSession *)session)->stopContinuousRecognition();
}

// 非流式语音识别
// 支持文件和内存数据
void speech_recognizer_recognize_once_async(SpeechRecognitionSession *session) {
    if (session == nullptr) {
        return;
    }
    AudioConfig *audioConfig = ((kyai::core::speech::_SpeechRecognitionSession *)session)->getRecognizerAudioConfig();
    if (audioConfig == nullptr) {
        return;
    }
    if (audioConfig->inputAudioInfo.inputSource == InputAudioInfo::InputSource::DEFAULT_MIC) {
        return;
    }
    if (audioConfig->inputAudioInfo.inputSource == InputAudioInfo::InputSource::STREAM) {
        return;
    }
    ((kyai::core::speech::_SpeechRecognitionSession *)session)->recognizeOnce();
}

void speech_recognizer_enable_internal_event_loop(SpeechRecognitionSession *session, bool enable) {
    if (session == nullptr) {
        return;
    }
    SpeechRecognitionSessionPtr(session)->enableInternalEventLoop(enable);
}

int speech_recognizer_recognize_once_async_with_request_id(SpeechRecognitionSession *session, const char *request_id) {
    if (session == nullptr) {
        return AI_COMMON_INVALID_SESSION;
    }
    AudioConfig *audioConfig = ((kyai::core::speech::_SpeechRecognitionSession *)session)->getRecognizerAudioConfig();
    if (audioConfig == nullptr) {
        return AI_COMMON_SPEECH_PARAM_INVALID;
    }
    if (audioConfig->inputAudioInfo.inputSource == InputAudioInfo::InputSource::DEFAULT_MIC) {
        return AI_COMMON_SPEECH_PARAM_INVALID;
    }
    if (audioConfig->inputAudioInfo.inputSource == InputAudioInfo::InputSource::STREAM) {
        return AI_COMMON_SPEECH_PARAM_INVALID;
    }
    return ((kyai::core::speech::_SpeechRecognitionSession *)session)->recognizeOnceWithRequestId(request_id);
}
