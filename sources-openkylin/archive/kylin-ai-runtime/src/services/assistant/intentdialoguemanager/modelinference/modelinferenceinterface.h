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

#ifndef SERVICES_ASSISTANT_MODEL_INFERENCE_INTERFACE_H
#define SERVICES_ASSISTANT_MODEL_INFERENCE_INTERFACE_H

#include <future>
#include <string>

#include "assistant/intentdialoguemanager/dialoguemessage.h"

class ModelInferenceInterface {
public:
    virtual ~ModelInferenceInterface() = default;

    // 调用对话模型
    virtual std::future<bool> callChatModelInference(const std::string& modelInput) = 0;

    // 调用意图模型
    virtual DialogueData::CallSuccessIdentifier callIntentionModelInference(const std::string& modelInput) = 0;

    virtual std::string intentModelName() = 0;

    virtual std::string intentModelDisplayName() = 0;

    virtual std::string chatModelName() = 0;

    void setChatModelCallback(const DialogueCallback::ChatModelCallback& callback) { chatModelCallback_ = callback; }

    virtual int getCurrentModelMaxTokens() = 0;

    virtual bool isCloudModel() = 0;
    virtual bool isChatModelConfigured() = 0;
    virtual bool isIntentModelNotLoaded() = 0;

    virtual void stopChat() = 0;

protected:
    void callChatModelCallback(const DialogueCallback::ChatData& chatData) {
        if (chatModelCallback_ != nullptr) {
            chatModelCallback_(chatData);
        }
    }

protected:
    DialogueCallback::ChatModelCallback chatModelCallback_{nullptr};
};

#endif  // SERVICES_ASSISTANT_MODEL_INFERENCE_INTERFACE_H
