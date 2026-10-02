/*
 * Copyright 2025 KylinSoft Co., Ltd.
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

#ifndef SERVICES_ASSISTANT_IMAGE_HANDLER_H
#define SERVICES_ASSISTANT_IMAGE_HANDLER_H

#include <future>

#include "assistant/sessionbase.h"
#include "imagehandlerinterface.h"

class ImageHandler : public ImageHandlerInterface {
public:
    ImageHandler() = default;

    ~ImageHandler() override;

    // 开始 - 文生图接口部分
    DialogueData::CallSuccessIdentifier textToImage(const std::string &prompt) override;

    DialogueData::CallSuccessIdentifier imageMatting(const std::string &imagePath) override;

    DialogueData::CallSuccessIdentifier imageBinarization(const std::string &imagePath) override;
    // 结束 - 文生图接口部分

    void initImageGenerationEngine(const std::shared_ptr<assistant::ImageGenerationEngine> &visionEngine);

    void destroyImageGenerationEngine();

    void initImageProcessEngine(const std::shared_ptr<assistant::ImageProcessEngine> &imageProcessEngine);

    void destroyImageProcessEngine();

    std::string getFeedbackUrl(const std::string &imageModelName) override;

    void stopImageBinarization() override;

    void stopImageMatting() override;

private:
    void initImageMattingModule();

    void destroyImageMattingModule();

    void initImageBinaryModule();

    void destroyImageBinaryModule();

    void waitForImageMattingModuleInited();

    void waitForImageBinaryModuleInited();

private:
    std::shared_ptr<assistant::ImageGenerationEngine> imageGenerationEngine_{nullptr};

    std::shared_ptr<assistant::ImageProcessEngine> imageProcessEngine_{nullptr};

    bool imageGenerationEngineInited_{false};
    bool imageMattingEngineInited_{false};
    bool imageBinaryEngineInited_{false};

    std::future<void> imageMattingInitFuture_;
    std::future<void> imageBinaryInitFuture_;
};

#endif  // SERVICES_ASSISTANT_IMAGE_HANDLER_H
