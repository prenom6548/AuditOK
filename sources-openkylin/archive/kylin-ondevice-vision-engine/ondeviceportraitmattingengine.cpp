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

#include <kylin-ai/ai-engine/aiengine.h>

#include "matting_triton_client/client.h"
#include "matting_triton_client/portrait_matting_client.h"
#include "matting_triton_client/universal_matting_client.h"
#include "ondevicevisioncapabilityengine.h"

namespace ai_engine::core_ai::vision {

void OndeviceVisionImageProcessEngine::setMattingResultCallback(ImageResultCallback callback) {
    portraitMattingClient_->setMattingResultCallback(callback);
    universalMattingClient_->setMattingResultCallback(callback);
}

result::AiEngineBaseResult OndeviceVisionImageProcessEngine::mattingFromImageFile(const std::string& imageFilePath,
                                                                                  MattingMode mode) {
    result::AiEngineBaseResult error;
    switch (mode) {
        case MattingMode::MATTING_MODE_PORTRAIT:
            error = portraitMattingClient_->mattingRequestByPath(imageFilePath);
            break;
        case MattingMode::MATTING_MODE_GENERAL:
            error = universalMattingClient_->mattingRequestByPath(imageFilePath);
            break;
        default:
            printf("[MattingTriton]: Matting mode error:%d\n", mode);
            error.success = false;
            error.error = ai_engine::EngineError((int)VisionEngineErrorCode::MattingModelError,
                                                 "[MattingTriton]: Matting mode error.");
            return error;
    }

    return error;
}

result::AiEngineBaseResult OndeviceVisionImageProcessEngine::mattingFromBase64ImageData(
    const std::string& imageBase64Data, MattingMode mode) {
    result::AiEngineBaseResult error;
    switch (mode) {
        case MattingMode::MATTING_MODE_PORTRAIT:
            error = portraitMattingClient_->mattingRequestByBase64(imageBase64Data.c_str());
            break;
        case MattingMode::MATTING_MODE_GENERAL:
            error = universalMattingClient_->mattingRequestByBase64(imageBase64Data.c_str());
            break;
        default:
            printf("[MattingTriton]: Matting mode error:%d\n", mode);
            error.success = false;
            error.error = ai_engine::EngineError((int)VisionEngineErrorCode::MattingModelError,
                                                 "[MattingTriton]: Matting mode error.");
            return error;
    }

    return error;
}

result::AiEngineBaseResult OndeviceVisionImageProcessEngine::initMattingsModule() {
    result::AiEngineBaseResult error;
    error = portraitMattingClient_->initMattingsModule();
    if (!error.success) {
        return error;
    }
    error = universalMattingClient_->initMattingsModule();
    if (!error.success) {
        return error;
    }
    return error;
}

result::AiEngineBaseResult OndeviceVisionImageProcessEngine::destroyMattingsModule() {
    result::AiEngineBaseResult error;
    error = portraitMattingClient_->destroyMattingsModule();
    if (!error.success) {
        return error;
    }
    error = universalMattingClient_->destroyMattingsModule();
    if (!error.success) {
        return error;
    }
    return error;
}

std::string OndeviceVisionImageProcessEngine::currentPortraitSegmentationModel() const {
    return portraitMattingClient_->currentModel();
}
std::string OndeviceVisionImageProcessEngine::currentGeneralSegmentationModel() const {
    return universalMattingClient_->currentModel();
}
void OndeviceVisionImageProcessEngine::setCurrentPortraitSegmentationModel(const std::string& model) {
    return portraitMattingClient_->setCurrentModel(model);
}
void OndeviceVisionImageProcessEngine::setCurrentGeneralSegmentationModel(const std::string& model) {
    return universalMattingClient_->setCurrentModel(model);
}

void OndeviceVisionImageProcessEngine::stopMattingProcess() { return portraitMattingClient_->stopMattingProcess(); }

}  // namespace ai_engine::core_ai::vision
