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

#include "xunfeispeechengine.h"
#include "xunfeispeechengine_p.h"

XunfeiSpeechEngine::XunfeiSpeechEngine() : d(new XunfeiSpeechEnginePrivate()) {}

XunfeiSpeechEngine::~XunfeiSpeechEngine() { delete d; }

std::string XunfeiSpeechEngine::engineName() const { return d->engineName(); }

std::string XunfeiSpeechEngine::currentModel() const { return d->currentModel(); }

std::string XunfeiSpeechEngine::currentModelName() const { return d->currentModelName(); }

bool XunfeiSpeechEngine::isCloud() const { return d->isCloud(); }

bool XunfeiSpeechEngine::isBuiltInEngine() const { return d->isBuiltInEngine(); }

bool XunfeiSpeechEngine::isCustomModelEngine() const { return d->isCustomModelEngine(); }

void XunfeiSpeechEngine::setConfig(const std::string &config) { d->setConfig(config); }

bool XunfeiSpeechEngine::setCurrentModel(const std::string &model) { return d->setCurrentModel(model); }

std::string XunfeiSpeechEngine::modelInfo() const { return d->modelInfo(); }

void XunfeiSpeechEngine::setRecognitionResultCallback(ai_engine::core_ai::speech::RecognitionResultCallback callback) {
    d->setRecognitionResultCallback(callback);
}

void XunfeiSpeechEngine::setSynthesisResultCallback(ai_engine::core_ai::speech::SynthesisResultCallback callback) {
    d->setSynthesisResultCallback(callback);
}

ai_engine::result::AiEngineBaseResult XunfeiSpeechEngine::initContinuousRecognitionModule() {
    return d->initContinuousRecognitionModule();
}

ai_engine::result::AiEngineBaseResult XunfeiSpeechEngine::startContinuousRecognition(const std::string &params) {
    return d->startContinuousRecognition(params);
}

ai_engine::result::AiEngineBaseResult
XunfeiSpeechEngine::writeContinuousRecognitionAudioData(const std::vector<uint8_t> &audioData) {
    return d->writeContinuousRecognitionAudioData(audioData);
}

ai_engine::result::AiEngineBaseResult XunfeiSpeechEngine::stopContinuousRecognition() {
    return d->stopContinuousRecognition();
}

ai_engine::result::AiEngineBaseResult XunfeiSpeechEngine::destroyContinuousRecognitionModule() {
    return d->destroyContinuousRecognitionModule();
}

ai_engine::result::AiEngineBaseResult
XunfeiSpeechEngine::setCurrentContinuousRecognitionModel(const std::string &model) {
    return d->setCurrentContinuousRecognitionModel(model);
}

std::string XunfeiSpeechEngine::currentContinuousRecognitionModel() const {
    return d->currentContinuousRecognitionModel();
}

ai_engine::result::AiEngineBaseResult XunfeiSpeechEngine::initRecognizeOnceModule() {
    return d->initRecognizeOnceModule();
}

ai_engine::result::AiEngineBaseResult XunfeiSpeechEngine::recognizeOnce(const std::string &params,
                                                                        const std::vector<uint8_t> &audioData) {
    return d->recognizeOnce(params, audioData);
}

ai_engine::result::AiEngineBaseResult XunfeiSpeechEngine::destroyRecognizeOnceModule() {
    return d->destroyRecognizeOnceModule();
}

ai_engine::result::AiEngineBaseResult XunfeiSpeechEngine::stopRecognizeOnce() { return d->stopRecognizeOnce(); }

ai_engine::result::AiEngineBaseResult XunfeiSpeechEngine::setCurrentRecognizeOnceModel(const std::string &model) {
    return d->setCurrentRecognizeOnceModel(model);
}

std::string XunfeiSpeechEngine::currentRecognizeOnceModel() const { return d->currentRecognizeOnceModel(); }

ai_engine::result::AiEngineBaseResult XunfeiSpeechEngine::initSynthesizeModule() { return d->initSynthesizeModule(); }

ai_engine::result::AiEngineBaseResult XunfeiSpeechEngine::synthesizeText(const std::string &params,
                                                                         const std::string &text) {
    return d->synthesizeText(params, text);
}

ai_engine::result::AiEngineBaseResult XunfeiSpeechEngine::destroySynthesizeModule() {
    return d->destroySynthesizeModule();
}

ai_engine::result::AiEngineBaseResult XunfeiSpeechEngine::stopSynthesize() { return d->stopSynthesize(); }

ai_engine::result::AiEngineBaseResult XunfeiSpeechEngine::setCurrentSynthesizeModel(const std::string &model) {
    return d->setCurrentSynthesizeModel(model);
}

std::string XunfeiSpeechEngine::currentSynthesizeModel() const { return d->currentSynthesizeModel(); }