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

#include <kylin-ai/ai-engine/aiengine.h>

#include "ocr_triton_client/client.h"
#include "ondevicevisioncapabilityengine.h"

namespace ai_engine::core_ai::vision {

OndeviceVisionTextRecognitionEngine::OndeviceVisionTextRecognitionEngine()
    : ocrClient_(new OcrTritonClient("localhost:8000")) {}

OndeviceVisionTextRecognitionEngine::~OndeviceVisionTextRecognitionEngine() { delete ocrClient_; }

void OndeviceVisionTextRecognitionEngine::setTextRecognitionResultCallback(TextRecognitionResultCallback callback) {
    ocrClient_->setTextRecognitionResultCallback(callback);
}
result::AiEngineBaseResult OndeviceVisionTextRecognitionEngine::recognizeTextFromFile(
    const std::string& imageFilePath) {
    result::AiEngineBaseResult error = ocrClient_->OcrRequestByPath(imageFilePath);

    return error;
}

result::AiEngineBaseResult OndeviceVisionTextRecognitionEngine::recognizeTextFromBase64ImageData(
    const std::string& imageBase64Data) {
    result::AiEngineBaseResult error = ocrClient_->OcrRequestByBase64(imageBase64Data.c_str());

    return error;
}

result::AiEngineBaseResult OndeviceVisionTextRecognitionEngine::init() {
    result::AiEngineBaseResult error;
    error.success = true;
    ocrClient_->init();

    return error;
}

result::AiEngineBaseResult OndeviceVisionTextRecognitionEngine::destroy() {
    result::AiEngineBaseResult error;
    error.success = true;
    return error;
}

void OndeviceVisionTextRecognitionEngine::stopTextRecognition() { ocrClient_->stopTextRecognition(); }

std::string OndeviceVisionTextRecognitionEngine::modelInfo() const {
    const char* info = R"(
    {
        "vendor": "",
        "models": [
            {
                "modelName": "kylin-ocr_ppocr",
                "displayName": "麒麟-ocr_ppocr",
                "model": "ocr_ppocr",
                "subConfig": [],
                "debName": "",
                "is3rdModel": false
            }
        ]
    }
    )";
    return info;
}

std::string OndeviceVisionTextRecognitionEngine::currentTextRecognitionModel() const {
    return ocrClient_->currentTextRecognitionModel();
}

void OndeviceVisionTextRecognitionEngine::setCurrentTextRecognitionModel(const std::string& model) {
    ocrClient_->setCurrentTextRecognitionModel(model);
    return;
}

}  // namespace ai_engine::core_ai::vision