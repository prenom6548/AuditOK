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

#ifndef SERVICES_ASSISTANT_IMAGE_HANDLER_INTERFACE_H
#define SERVICES_ASSISTANT_IMAGE_HANDLER_INTERFACE_H

#include <string>

#include "assistant/intentdialoguemanager/dialoguemessage.h"

class ImageHandlerInterface {
public:
    virtual ~ImageHandlerInterface() = default;

    // TODO 这个接口返回值用处不大的话理应删除
    virtual DialogueData::CallSuccessIdentifier textToImage(const std::string &prompt) = 0;  // 文生图

    // 抠图
    virtual DialogueData::CallSuccessIdentifier imageMatting(const std::string &imagePath) = 0;

    // 二值化
    virtual DialogueData::CallSuccessIdentifier imageBinarization(const std::string &imagePath) = 0;

    void setTextToImageCallback(const DialogueCallback::TextToImageCallback &callback) {
        textToImageCallback_ = callback;
    }

    void setImageMattingCallback(const DialogueCallback::ImageMattingCallback &callback) {
        imageMattingCallback_ = callback;
    }

    void setImageBinarizationCallback(const DialogueCallback::ImageBinarizationCallback &callback) {
        imageBinarizationCallback_ = callback;
    }

    virtual std::string getFeedbackUrl(const std::string &imageModelName) = 0;

    virtual void stopImageMatting() = 0;
    virtual void stopImageBinarization() = 0;

protected:
    void callTextToImageCallback(const DialogueCallback::TextToImageResult &message) {
        if (textToImageCallback_ != nullptr) {
            textToImageCallback_(message);
        }
    }

    void callImageMattingCallback(const DialogueCallback::ImageMattingResult &result) {
        if (imageMattingCallback_ != nullptr) {
            imageMattingCallback_(result);
        }
    }

    void callImageBinarizationCallback(const DialogueCallback::ImageBinarizationResult &result) {
        if (imageBinarizationCallback_ != nullptr) {
            imageBinarizationCallback_(result);
        }
    }

protected:
    DialogueCallback::TextToImageCallback textToImageCallback_{nullptr};
    DialogueCallback::ImageMattingCallback imageMattingCallback_{nullptr};
    DialogueCallback::ImageBinarizationCallback imageBinarizationCallback_{nullptr};
};

#endif  // SERVICES_ASSISTANT_IMAGE_HANDLER_INTERFACE_H
