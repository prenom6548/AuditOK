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

#ifndef XUNFEISPEECHENGINE_H
#define XUNFEISPEECHENGINE_H

#include <kylin-ai/ai-engine/speechengine.h>

class XunfeiSpeechEnginePrivate;

class XunfeiSpeechEngine : public ai_engine::core_ai::speech::AbstractSpeechEngine {
public:
    XunfeiSpeechEngine();
    ~XunfeiSpeechEngine() override;
    std::string engineName() const override;
    bool isCloud() const override;
    bool isBuiltInEngine() const override;
    bool isCustomModelEngine() const override;
    std::string currentModel() const override;
    std::string currentModelName() const override;

    void setConfig(const std::string &config) override;
    bool setCurrentModel(const std::string &model) override;
    std::string modelInfo() const override;

    void setRecognitionResultCallback(ai_engine::core_ai::speech::RecognitionResultCallback callback) override;

    void setSynthesisResultCallback(ai_engine::core_ai::speech::SynthesisResultCallback callback) override;

    // 语音识别
    ai_engine::result::AiEngineBaseResult initContinuousRecognitionModule() override;
    ai_engine::result::AiEngineBaseResult startContinuousRecognition(const std::string &params) override;
    ai_engine::result::AiEngineBaseResult
    writeContinuousRecognitionAudioData(const std::vector<uint8_t> &audioData) override;
    ai_engine::result::AiEngineBaseResult stopContinuousRecognition() override;
    ai_engine::result::AiEngineBaseResult destroyContinuousRecognitionModule() override;
    ai_engine::result::AiEngineBaseResult setCurrentContinuousRecognitionModel(const std::string &model) override;
    std::string currentContinuousRecognitionModel() const override;

    ai_engine::result::AiEngineBaseResult initRecognizeOnceModule() override;
    ai_engine::result::AiEngineBaseResult recognizeOnce(const std::string &params,
                                                        const std::vector<uint8_t> &audioData) override;
    ai_engine::result::AiEngineBaseResult destroyRecognizeOnceModule() override;
    ai_engine::result::AiEngineBaseResult stopRecognizeOnce() override;
    ai_engine::result::AiEngineBaseResult setCurrentRecognizeOnceModel(const std::string &model) override;
    std::string currentRecognizeOnceModel() const override;

    // 语音合成
    ai_engine::result::AiEngineBaseResult initSynthesizeModule() override;
    ai_engine::result::AiEngineBaseResult synthesizeText(const std::string &params, const std::string &text) override;
    ai_engine::result::AiEngineBaseResult destroySynthesizeModule() override;
    ai_engine::result::AiEngineBaseResult stopSynthesize() override;
    ai_engine::result::AiEngineBaseResult setCurrentSynthesizeModel(const std::string &model) override;
    std::string currentSynthesizeModel() const override;

private:
    XunfeiSpeechEnginePrivate *d;
};

#endif // XUNFEISPEECHENGINE_H
