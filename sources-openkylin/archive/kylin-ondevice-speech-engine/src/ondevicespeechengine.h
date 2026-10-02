#pragma once

#include <kylin-ai/ai-engine/speechengine.h>

namespace ai_engine::core_ai::speech {

class OndeviceSpeechEnginePrivate;

class OndeviceSpeechEngine : public AbstractSpeechEngine {
public:
    OndeviceSpeechEngine();
    ~OndeviceSpeechEngine() override;

    std::string engineName() const override;
    bool isCloud() const override;
    bool isBuiltInEngine() const override;
    bool isCustomModelEngine() const override;
    std::string modelInfo() const override;
    bool setCurrentModel(const std::string &model) override;
    std::string currentModel() const override;
    void setConfig(const std::string &config) override;

    void setRecognitionResultCallback(RecognitionResultCallback callback) override;
    void setSynthesisResultCallback(SynthesisResultCallback callback) override;

    // 流式语音识别
    result::AiEngineBaseResult initContinuousRecognitionModule() override;
    result::AiEngineBaseResult destroyContinuousRecognitionModule() override;
    result::AiEngineBaseResult startContinuousRecognition(const std::string &params) override;
    result::AiEngineBaseResult writeContinuousRecognitionAudioData(const std::vector<uint8_t> &audioData) override;
    result::AiEngineBaseResult stopContinuousRecognition() override;
    virtual result::AiEngineBaseResult setCurrentContinuousRecognitionModel(const std::string& model) override;
    virtual std::string currentContinuousRecognitionModel() const override;

    // 非流式语音识别
    result::AiEngineBaseResult initRecognizeOnceModule() override;
    result::AiEngineBaseResult destroyRecognizeOnceModule() override;
    result::AiEngineBaseResult recognizeOnce(const std::string &params, const std::vector<uint8_t> &audioData) override;
    virtual result::AiEngineBaseResult stopRecognizeOnce() override;
    virtual result::AiEngineBaseResult setCurrentRecognizeOnceModel(const std::string& model) override;
    virtual std::string currentRecognizeOnceModel() const override;

    // 语音合成
    result::AiEngineBaseResult initSynthesizeModule() override;
    result::AiEngineBaseResult destroySynthesizeModule() override;
    result::AiEngineBaseResult synthesizeText(const std::string &params, const std::string &text) override;
    virtual result::AiEngineBaseResult stopSynthesize() override;
    virtual result::AiEngineBaseResult setCurrentSynthesizeModel(const std::string& model) override;
    virtual std::string currentSynthesizeModel() const override;

private:
    OndeviceSpeechEnginePrivate *dPtr_ = nullptr;
};

}  // namespace ai_engine::core_ai::speech