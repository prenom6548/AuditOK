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

#include <nlohmann/json.hpp>

#include "ondevicevisioncapabilityengine.h"
#include "seg_triton_client/client.h"

namespace ai_engine::core_ai::vision {

void OndeviceVisionImageProcessEngine::setSegmentationResultCallback(const SegmentationResultCallback& callback) {
    segClient_->setSegmentationResultCallback(callback);
}
result::AiEngineBaseResult OndeviceVisionImageProcessEngine::encodeSegmentationFromImageFile(
    const std::string& imageFilePath) {
    return segClient_->segEncodeRequestByPath(imageFilePath);
}

result::AiEngineBaseResult OndeviceVisionImageProcessEngine::encodeSegmentationFromBase64ImageData(
    const std::string& imageBase64Data) {
    return segClient_->segEncodeRequestByBase64(imageBase64Data);
}

result::AiEngineBaseResult OndeviceVisionImageProcessEngine::decodeSegmentation(
    const std::vector<SegmentationPoint>& points, uint32_t minArea) {
    std::vector<std::tuple<int, int, bool>> inputPoints;
    for (auto& point : points) {
        inputPoints.push_back(std::make_tuple(point.x, point.y, point.isForeground));
    }

    return segClient_->segDecodeRequest(inputPoints, minArea);
}

result::AiEngineBaseResult OndeviceVisionImageProcessEngine::initSegmentationModule() {
    result::AiEngineBaseResult error;
    error.success = true;
    segClient_->init();
    return error;
}

result::AiEngineBaseResult OndeviceVisionImageProcessEngine::destroySegmentationModule() {
    result::AiEngineBaseResult error;
    error.success = true;
    return error;
}

namespace nl = nlohmann;

result::AiEngineBaseResult OndeviceVisionImageProcessEngine::stopSegmentation() {
    return segClient_->stopSegmentation();
}

void OndeviceVisionImageProcessEngine::clearSegmentationCache() { segClient_->segmentationClearCache(); }

std::string OndeviceVisionImageProcessEngine::currentSegmentationModel() const { return segClient_->currentSegmentationModel(); }
void OndeviceVisionImageProcessEngine::setCurrentSegmentationModel(
    const std::string &model) { segClient_->setCurrentSegmentationModel(model); return; }

}  // namespace ai_engine::core_ai::vision