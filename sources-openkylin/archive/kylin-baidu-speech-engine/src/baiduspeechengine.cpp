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

#include "baiduspeechengine.h"
#include "baiduspeechengine_p.h"

BaiduSpeechEngine::BaiduSpeechEngine() : d(new BaiduSpeechEnginePrivate()) {}

BaiduSpeechEngine::~BaiduSpeechEngine() { delete d; }

std::string BaiduSpeechEngine::engineName() const { return d->engineName(); }

std::string BaiduSpeechEngine::currentModel() const { return d->currentModel(); }

std::string BaiduSpeechEngine::currentModelName() const { return d->currentModelName(); }

bool BaiduSpeechEngine::isCloud() const { return d->isCloud(); }

bool BaiduSpeechEngine::isBuiltInEngine() const { return d->isBuiltInEngine(); }

bool BaiduSpeechEngine::isCustomModelEngine() const { return d->isCustomModelEngine(); }

void BaiduSpeechEngine::setConfig(const std::string &config) { d->setConfig(config); }

bool BaiduSpeechEngine::setCurrentModel(const std::string &model) { return d->setCurrentModel(model); }

std::string BaiduSpeechEngine::modelInfo() const { return d->modelInfo(); }

void BaiduSpeechEngine::setRecognitionResultCallback(ai_engine::core_ai::speech::RecognitionResultCallback callback) {
    d->setRecognitionResultCallback(callback);
}

void BaiduSpeechEngine::setSynthesisResultCallback(ai_engine::core_ai::speech::SynthesisResultCallback callback) {
    d->setSynthesisResultCallback(callback);
}

ai_engine::result::AiEngineBaseResult BaiduSpeechEngine::initContinuousRecognitionModule() {
    return d->initContinuousRecognitionModule();
}

ai_engine::result::AiEngineBaseResult BaiduSpeechEngine::startContinuousRecognition(const std::string &params) {
    return d->startContinuousRecognition(params);
}

ai_engine::result::AiEngineBaseResult
BaiduSpeechEngine::writeContinuousRecognitionAudioData(const std::vector<uint8_t> &audioData) {
    return d->writeContinuousRecognitionAudioData(audioData);
}

ai_engine::result::AiEngineBaseResult BaiduSpeechEngine::stopContinuousRecognition() {
    return d->stopContinuousRecognition();
}

ai_engine::result::AiEngineBaseResult BaiduSpeechEngine::destroyContinuousRecognitionModule() {
    return d->destroyContinuousRecognitionModule();
}

ai_engine::result::AiEngineBaseResult
BaiduSpeechEngine::setCurrentContinuousRecognitionModel(const std::string &model) {
    return d->setCurrentContinuousRecognitionModel(model);
}

std::string BaiduSpeechEngine::currentContinuousRecognitionModel() const {
    return d->currentContinuousRecognitionModel();
}

ai_engine::result::AiEngineBaseResult BaiduSpeechEngine::initRecognizeOnceModule() {
    return d->initRecognizeOnceModule();
}

ai_engine::result::AiEngineBaseResult BaiduSpeechEngine::recognizeOnce(const std::string &params,
                                                                       const std::vector<uint8_t> &audioData) {
    return d->recognizeOnce(params, audioData);
}

ai_engine::result::AiEngineBaseResult BaiduSpeechEngine::destroyRecognizeOnceModule() {
    return d->destroyRecognizeOnceModule();
}

ai_engine::result::AiEngineBaseResult BaiduSpeechEngine::stopRecognizeOnce() { return d->stopRecognizeOnce(); }

ai_engine::result::AiEngineBaseResult BaiduSpeechEngine::setCurrentRecognizeOnceModel(const std::string &model) {
    return d->setCurrentRecognizeOnceModel(model);
}

std::string BaiduSpeechEngine::currentRecognizeOnceModel() const { return d->currentRecognizeOnceModel(); }

ai_engine::result::AiEngineBaseResult BaiduSpeechEngine::initSynthesizeModule() { return d->initSynthesizeModule(); }

ai_engine::result::AiEngineBaseResult BaiduSpeechEngine::synthesizeText(const std::string &params,
                                                                        const std::string &text) {
    return d->synthesizeText(params, text);
}

ai_engine::result::AiEngineBaseResult BaiduSpeechEngine::destroySynthesizeModule() {
    return d->destroySynthesizeModule();
}

ai_engine::result::AiEngineBaseResult BaiduSpeechEngine::stopSynthesize() { return d->stopSynthesize(); }

ai_engine::result::AiEngineBaseResult BaiduSpeechEngine::setCurrentSynthesizeModel(const std::string &model) {
    return d->setCurrentSynthesizeModel(model);
}

std::string BaiduSpeechEngine::currentSynthesizeModel() const { return d->currentSynthesizeModel(); }