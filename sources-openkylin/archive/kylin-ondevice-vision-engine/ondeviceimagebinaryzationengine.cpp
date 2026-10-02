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

#include "image_binaryzation/client.h"
#include "ondevicevisioncapabilityengine.h"

namespace ai_engine::core_ai::vision {
void OndeviceVisionImageProcessEngine::setImageBinaryResultCallback(ImageResultCallback callback) {
    binImage_->setImageBinaryResultCallback(callback);
}

result::AiEngineBaseResult OndeviceVisionImageProcessEngine::binarizeImageFromImageFile(
    const std::string& imageFilePath) {
    return binImage_->binaryImageRequestByPath(imageFilePath);
}

result::AiEngineBaseResult OndeviceVisionImageProcessEngine::binarizeImageFromBase64ImageData(
    const std::string& imageBase64Data) {
    return binImage_->binaryImageRequestByBase64(imageBase64Data);
}

std::string OndeviceVisionImageProcessEngine::modelInfo() const {
    const char* info = R"(
    {
        "model_catalog": {
            "matting_portrait": [{
                "models": [
                    "matting_portrait"
                ],
                "name": "matting_portrait"
            }],
            "matting_universal": [{
                "models": [
                    "matting_universal"
                ],
                "name": "matting_universal"
            }],
            "seg_sam": [{
                "models": [
                    "seg_sam",
                    "seg_sam-decoder",
                    "seg_sam-encoder"
                ],
                "name": "seg_sam"
            }]
        },
        "models": [{
            "debName": "",
            "displayName": "端侧图像处理",
            "is3rdModel": "false",
            "model": "端侧图像处理模型",
            "modelName": "kylin-端侧图像处理模型",
            "subConfig": []
        }],
        "vendor": ""
    }
    )";
    return info;
}

result::AiEngineBaseResult OndeviceVisionImageProcessEngine::initImageBinaryModule() {
    result::AiEngineBaseResult error;
    error.success = true;

    return error;
}

result::AiEngineBaseResult OndeviceVisionImageProcessEngine::destroyImageBinaryModule() {
    result::AiEngineBaseResult error;
    error.success = true;

    return error;
}

void OndeviceVisionImageProcessEngine::stopImageBinarization() { binImage_->stopImageBinarization(); }

}  // namespace ai_engine::core_ai::vision
