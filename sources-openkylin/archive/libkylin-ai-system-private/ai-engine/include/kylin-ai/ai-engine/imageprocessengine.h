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

#ifndef COREAIIMAGEPROCESSENGINE_H
#define COREAIIMAGEPROCESSENGINE_H

#include <functional>
#include <string>

#include "aiengine.h"
#include "aiengineerror.h"
#include "aiengineresult.h"

namespace ai_engine {
namespace core_ai {
namespace vision {

struct ImageResult : public result::AiEngineModelResult {
    std::string imageFormat;
    std::string imageBase64Data;
};

using ImageResultCallback = std::function<void(const ImageResult &result)>;

class AbstractImageProcessEngine : public AbstractAiEngine {
public:
    virtual ~AbstractImageProcessEngine() {}

    // 抠图模块
    virtual result::AiEngineBaseResult initMattingsModule() = 0;
    virtual result::AiEngineBaseResult destroyMattingsModule() = 0;
    virtual void setMattingResultCallback(ImageResultCallback callback) = 0;
    virtual result::AiEngineBaseResult mattingFromBase64ImageData(
        const std::string &imageBase64Data) = 0;
    virtual result::AiEngineBaseResult mattingFromImageFile(
        const std::string &imageFilePath) = 0;
    virtual void stopMattingProcess() = 0;

    // 图像二值化模块
    virtual result::AiEngineBaseResult initImageBinaryModule() = 0;
    virtual result::AiEngineBaseResult destroyImageBinaryModule() = 0;
    virtual void setImageBinaryResultCallback(ImageResultCallback callback) = 0;
    virtual result::AiEngineBaseResult binarizeImageFromBase64ImageData(
        const std::string &imageBase64Data) = 0;
    virtual result::AiEngineBaseResult binarizeImageFromImageFile(
        const std::string &imageFilePath) = 0;
    virtual void stopImageBinarization() = 0;
};

}  // namespace vision
}  // namespace core_ai
}  // namespace ai_engine

#endif