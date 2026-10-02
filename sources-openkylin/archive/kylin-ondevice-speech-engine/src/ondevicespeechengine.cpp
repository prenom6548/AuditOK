#include "ondevicespeechengine.h"

#include "ondevicespeechengine_p.h"

namespace ai_engine::core_ai::speech {

OndeviceSpeechEngine::OndeviceSpeechEngine() : dPtr_(new OndeviceSpeechEnginePrivate()) {}

OndeviceSpeechEngine::~OndeviceSpeechEngine() {
    delete dPtr_;
    dPtr_ = nullptr;
}

std::string OndeviceSpeechEngine::engineName() const { return dPtr_->engineName(); }

bool OndeviceSpeechEngine::isCloud() const { return dPtr_->isCloud(); }

bool OndeviceSpeechEngine::isBuiltInEngine() const { return dPtr_->isBuiltInEngine(); }

bool OndeviceSpeechEngine::isCustomModelEngine() const { return dPtr_->isCustomModelEngine(); }

std::string OndeviceSpeechEngine::modelInfo() const { return dPtr_->modelInfo(); }

bool OndeviceSpeechEngine::setCurrentModel(const std::string &model) { return dPtr_->setCurrentModel(model); }

std::string OndeviceSpeechEngine::currentModel() const { return dPtr_->currentModel(); }

void OndeviceSpeechEngine::setConfig(const std::string &config) { dPtr_->setConfig(config); }

void OndeviceSpeechEngine::setRecognitionResultCallback(RecognitionResultCallback callback) {
    dPtr_->setRecognitionResultCallback(callback);
}

void OndeviceSpeechEngine::setSynthesisResultCallback(SynthesisResultCallback callback) {
    dPtr_->setSynthesisResultCallback(callback);
}

result::AiEngineBaseResult OndeviceSpeechEngine::initContinuousRecognitionModule() {
    return dPtr_->initContinuousRecognitionModule();
}

result::AiEngineBaseResult OndeviceSpeechEngine::destroyContinuousRecognitionModule() {
    return dPtr_->destroyContinuousRecognitionModule();
}

result::AiEngineBaseResult OndeviceSpeechEngine::startContinuousRecognition(const std::string &params) {
    return dPtr_->startContinuousRecognition(params);
}

result::AiEngineBaseResult OndeviceSpeechEngine::writeContinuousRecognitionAudioData(
    const std::vector<uint8_t> &audioData) {
    return dPtr_->writeContinuousRecognitionAudioData(audioData);
}

result::AiEngineBaseResult OndeviceSpeechEngine::stopContinuousRecognition() {
    return dPtr_->stopContinuousRecognition();
}

result::AiEngineBaseResult OndeviceSpeechEngine::setCurrentContinuousRecognitionModel(const std::string& model) {
    return dPtr_->setCurrentContinuousRecognitionModel(model);
}

std::string OndeviceSpeechEngine::currentContinuousRecognitionModel() const {
    return dPtr_->currentContinuousRecognitionModel();
}

result::AiEngineBaseResult OndeviceSpeechEngine::initRecognizeOnceModule() { return dPtr_->initRecognizeOnceModule(); }

result::AiEngineBaseResult OndeviceSpeechEngine::destroyRecognizeOnceModule() {
    return dPtr_->destroyRecognizeOnceModule();
}

result::AiEngineBaseResult OndeviceSpeechEngine::recognizeOnce(const std::string &params,
                                                               const std::vector<uint8_t> &audioData) {
    return dPtr_->recognizeOnce(params, audioData);
}

result::AiEngineBaseResult OndeviceSpeechEngine::stopRecognizeOnce() { return dPtr_->stopRecognizeOnce(); }

result::AiEngineBaseResult OndeviceSpeechEngine::setCurrentRecognizeOnceModel(const std::string& model) {
    return dPtr_->setCurrentRecognizeOnceModel(model);
}

std::string OndeviceSpeechEngine::currentRecognizeOnceModel() const { return dPtr_->currentRecognizeOnceModel(); }

result::AiEngineBaseResult OndeviceSpeechEngine::initSynthesizeModule() { return dPtr_->initSynthesizeModule(); }

result::AiEngineBaseResult OndeviceSpeechEngine::destroySynthesizeModule() { return dPtr_->destroySynthesizeModule(); }

result::AiEngineBaseResult OndeviceSpeechEngine::synthesizeText(const std::string &params, const std::string &text) {
    return dPtr_->synthesizeText(params, text);
}

result::AiEngineBaseResult OndeviceSpeechEngine::stopSynthesize() { return dPtr_->stopSynthesize(); }

result::AiEngineBaseResult OndeviceSpeechEngine::setCurrentSynthesizeModel(const std::string& model) {
    return dPtr_->setCurrentSynthesizeModel(model);
}

std::string OndeviceSpeechEngine::currentSynthesizeModel() const { return dPtr_->currentSynthesizeModel(); }

}  // namespace ai_engine::core_ai::speech