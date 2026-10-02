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

#include "speech.h"
#include "speechsession.h"
#include "settings/globalsettings.h"

SpeechResult speech_create_session(SpeechSession *session) {
    speech::SpeechSession *speechSession = new speech::SpeechSession();
    *session = speechSession;
    return SPEECH_SUCCESS;
}

void speech_destory_session(SpeechSession session) {
    speech::SpeechSession *speechSession = static_cast<speech::SpeechSession *>(session);
    if (speechSession) {
        delete speechSession;
    }
}

SpeechResult speech_init_session(SpeechSession session) {
    speech::SpeechSession *speechSession = static_cast<speech::SpeechSession *>(session);
    if (!speechSession) {
        return SPEECH_SESSION_ERROR;
    }
    const auto& speechSettings = aisdk::GlobalSettings::getInstance().getChildSettings(CAPABILITY_SPEECH);

    const std::string model = speechSettings.getModel(PUBLIC_CLOUD);
    auto config = speechSettings.getModelConfig(PUBLIC_CLOUD, model);
    return speechSession->init(model, config.get("apiKey", "").asString(),
                               config.get("secretKey", "").asString(),
                               config.get("appId", "").asString());
}

void speech_set_recognizing_callback(
    SpeechSession session, SpeechRecognitionResultCallback callback, void *user_data) {
    speech::SpeechSession *speechSession = static_cast<speech::SpeechSession *>(session);
    if (!speechSession) {
        return;
    }

    speechSession->setRecognizingCallback(callback, user_data);
}

void speech_set_recognized_callback(
    SpeechSession session, SpeechRecognitionResultCallback callback, void* user_data) {
    speech::SpeechSession *speechSession = static_cast<speech::SpeechSession *>(session);
    if (!speechSession) {
        return;
    }

    speechSession->setRecognizedCallback(callback, user_data);
}

void speech_set_synthesizing_callback(
    SpeechSession session, SpeechSynthesisResultCallback callback, void* user_data) {
    speech::SpeechSession *speechSession = static_cast<speech::SpeechSession *>(session);
    if (!speechSession) {
        return;
    }

    speechSession->setSynthesizingCallback(callback, user_data);
}

void speech_set_synthesized_callback(
    SpeechSession session, SpeechFinishedCallback callback, void* user_data) {
    speech::SpeechSession *speechSession = static_cast<speech::SpeechSession *>(session);
    if (!speechSession) {
        return;
    }

    speechSession->setSynthesizedCallback(callback, user_data);
}

SpeechResult speech_start_continuous_recognition(SpeechSession session,
                                                 const char* params) {
    if (!params) {
        fprintf(stderr, "speech start continuous recognition error: params is nullptr!\n");
        return SpeechResult::SPEECH_PARAM_ERROR;
    }
    speech::SpeechSession *speechSession = static_cast<speech::SpeechSession *>(session);
    if (!speechSession) {
        return SpeechResult::SPEECH_SESSION_ERROR;
    }

    speechSession->startContinuousRecognition(params);
    return SpeechResult::SPEECH_SUCCESS;
}


SpeechResult speech_continuous_recognition_write_audio_data_async(
    SpeechSession session, const void* audio_data, unsigned int audio_data_length) {
    if (audio_data == nullptr || audio_data_length == 0) {
        fprintf(stderr, "speech continuous recognition error: audio data is nullptr!\n");
        return SpeechResult::SPEECH_PARAM_ERROR;
    }
    speech::SpeechSession *speechSession = static_cast<speech::SpeechSession *>(session);
    if (!speechSession) {
        return SpeechResult::SPEECH_SESSION_ERROR;
    }

    speechSession->writeContinuousAudioData(audio_data, audio_data_length);
    return SpeechResult::SPEECH_SUCCESS;
}

SpeechResult speech_stop_continuous_recognition(SpeechSession session) {
    speech::SpeechSession *speechSession = static_cast<speech::SpeechSession *>(session);
    if (!speechSession) {
        return SpeechResult::SPEECH_SESSION_ERROR;
    }
    speechSession->stopContinuousRecognition();
    return SpeechResult::SPEECH_SUCCESS;
}

SpeechResult speech_recognize_once_async(
    SpeechSession session, const char* params,
    const void* audio_data, unsigned int audio_data_length) {
    if (audio_data == nullptr || audio_data_length == 0) {
        fprintf(stderr, "speech recognize once error: audio data is nullptr!\n");
        return SpeechResult::SPEECH_PARAM_ERROR;
    }
    speech::SpeechSession *speechSession = static_cast<speech::SpeechSession *>(session);
    if (!speechSession) {
        return SpeechResult::SPEECH_SESSION_ERROR;
    }
    speechSession->speechRecognizeOnce(params, audio_data, audio_data_length);
    return SpeechResult::SPEECH_SUCCESS;
}

SpeechResult speech_start_continuous_synthesis(SpeechSession session, const char* params) {
    if (params == nullptr) {
        fprintf(stderr, "speech start continuous synthesis error: params is nullptr!\n");
        return SpeechResult::SPEECH_PARAM_ERROR;
    }
    speech::SpeechSession *speechSession = static_cast<speech::SpeechSession *>(session);
    if (!speechSession) {
        return SpeechResult::SPEECH_SESSION_ERROR;
    }
    speechSession->startContinuousSynthesis(params);
    return SpeechResult::SPEECH_SUCCESS;
}

SpeechResult speech_continuous_synthesize_write_text_async(SpeechSession session, const char* text) {
    if (text == nullptr) {
        fprintf(stderr, "speech continuous synthesize error: text is nullptr!\n");
        return SpeechResult::SPEECH_PARAM_ERROR;
    }
    speech::SpeechSession *speechSession = static_cast<speech::SpeechSession *>(session);
    if (!speechSession) {
        return SpeechResult::SPEECH_SESSION_ERROR;
    }
    speechSession->writeContinuousText(text);
    return SpeechResult::SPEECH_SUCCESS;
}

SpeechResult speech_stop_continuous_synthesis(SpeechSession session) {
    speech::SpeechSession *speechSession = static_cast<speech::SpeechSession *>(session);
    if (!speechSession) {
        return SpeechResult::SPEECH_SESSION_ERROR;
    }
    speechSession->stopContinuousSynthesis();
    return SpeechResult::SPEECH_SUCCESS;
}

SpeechResult speech_synthesize_once_async(
    SpeechSession session,const char* params, const char* text) {
    if (text == nullptr) {
        fprintf(stderr, "speech continuous synthesize error: text is nullptr!\n");
        return SpeechResult::SPEECH_PARAM_ERROR;
    }
    speech::SpeechSession *speechSession = static_cast<speech::SpeechSession *>(session);
    if (!speechSession) {
        return SpeechResult::SPEECH_SESSION_ERROR;
    }
    speechSession->speechSynthesizeOnce(params, text);
    return SpeechResult::SPEECH_SUCCESS;
}

void speech_set_keyword_recognized_callback(SpeechSession session,
                                            SpeechRecognitionResultCallback callback, void* user_data) {
    speech::SpeechSession *speechSession = static_cast<speech::SpeechSession *>(session);
    if (!speechSession)
    {
        return;
    }

    speechSession->setKeywordRecognizedCallback(callback, user_data);
}

SpeechResult speech_start_keyword_recognition(SpeechSession session) {
    speech::SpeechSession *speechSession = static_cast<speech::SpeechSession *>(session);
    if (!speechSession)
    {
        return SpeechResult::SPEECH_SESSION_ERROR;
    }

    speechSession->startKeywordRecognizer();
    return SpeechResult::SPEECH_SUCCESS;
}

SpeechResult speech_stop_keyword_recognition(SpeechSession session) {
    speech::SpeechSession *speechSession = static_cast<speech::SpeechSession *>(session);
    if (!speechSession)
    {
        return SpeechResult::SPEECH_SESSION_ERROR;
    }

    speechSession->stopKeywordRecognizer();
    return SpeechResult::SPEECH_SUCCESS;
}

const char* speech_get_last_error_message() {
    return "";
}
