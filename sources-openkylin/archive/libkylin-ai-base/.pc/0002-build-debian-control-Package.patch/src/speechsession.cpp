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

#include "speechsession.h"

namespace speech {

SpeechSession::SpeechSession() {}

SpeechResult SpeechSession::init(
        const std::string &engineName, const std::string &apiKey,
        const std::string &secretKey, const std::string &appId) {
    speechProcessorProxy_.initEngine(
                engineName, appId, apiKey, secretKey, nullptr);
    return SPEECH_SUCCESS;
}

void SpeechSession::setRecognizingCallback(
    SpeechRecognitionResultCallback callback, std::any userData) {
    callbacks_[SpeechSession::WorkStage::Recognizing] = { callback, userData };
    
    speechProcessorProxy_.setInputAudioDataCallback(
                [callback, userData](RecognitionResult result) {
        callback(result, std::any_cast<void *>(userData));
    });
    speechProcessorProxy_.setRecognizingCompleteAudioCallback(
                [callback, userData](RecognitionResult result) {
        callback(result, std::any_cast<void *>(userData));
    });
}

void SpeechSession::setRecognizedCallback(
    SpeechFinishedCallback callback, std::any userData) {
    callbacks_[SpeechSession::WorkStage::Recognized] = { callback, userData };
    speechProcessorProxy_.setStopRecognizingAudioCallback([callback, userData]() {
       callback(std::any_cast<void *>(userData));
    });
}

void SpeechSession::setSynthesizingCallback(
    SpeechSynthesisResultCallback callback, std::any userData) {
    callbacks_[SpeechSession::WorkStage::Synthesizing] = { callback, userData };

    speechProcessorProxy_.setInputContinuousTextCallback(
                [callback, userData](SynthesisResult result) {
        callback(result, std::any_cast<void *>(userData));
    });
    speechProcessorProxy_.setRecognizingCompleteTextCallback(
                [callback, userData](SynthesisResult result) {
        callback(result, std::any_cast<void *>(userData));
    });
}

void SpeechSession::setSynthesizedCallback(
    SpeechFinishedCallback callback, std::any userData) {
    callbacks_[SpeechSession::WorkStage::Synthesized] = { callback, userData };
    speechProcessorProxy_.setStopSynthesizingAudioCallback(
                [callback, userData]() {
        callback(std::any_cast<void *>(userData));
    });
}

SpeechResult SpeechSession::startContinuousRecognition(const char* params) {
    speechProcessorProxy_.startRecognizingAudio(params);
    return SpeechResult::SPEECH_SUCCESS;
}                                                   

SpeechResult SpeechSession::writeContinuousAudioData(const void* audio_data,
                                                     unsigned int audio_data_length) {
    speechProcessorProxy_.inputAudioData((char*)audio_data, audio_data_length);
    return SpeechResult::SPEECH_SUCCESS;
}

SpeechResult SpeechSession::stopContinuousRecognition() {
    speechProcessorProxy_.stopRecognizingAudio();
    return SpeechResult::SPEECH_SUCCESS;
}

SpeechResult SpeechSession::speechRecognizeOnce(const char* params,
                                                const void* audio_data,
                                                unsigned int audio_data_length) {
    speechProcessorProxy_.recognizingCompleteAudio(
        params, (char*)audio_data, audio_data_length);
    return SpeechResult::SPEECH_SUCCESS;
}

SpeechResult SpeechSession::startContinuousSynthesis(const char* params) {
    speechProcessorProxy_.startSynthesizingAudio(params);
    return SpeechResult::SPEECH_SUCCESS;
}

SpeechResult SpeechSession::writeContinuousText(const char* text) {
    speechProcessorProxy_.inputContinuousText(text);
    return SpeechResult::SPEECH_SUCCESS;
}

SpeechResult SpeechSession::stopContinuousSynthesis() {
    speechProcessorProxy_.stopSynthesizingAudio();
    return SpeechResult::SPEECH_SUCCESS;
}

SpeechResult SpeechSession::speechSynthesizeOnce(const char* params,
                                                 const char* text) {
    speechProcessorProxy_.recognizingCompleteText(params, text);
    return SpeechResult::SPEECH_SUCCESS;
}

}


