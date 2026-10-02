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

#include "osassistant/osassistant.h"

#include <jsoncpp/json/value.h>

#include <tuple>

#include "assistantserviceproxy.h"

namespace kyai {
namespace assistant {

class OsAssistantPrivate {
public:
    OsAssistantPrivate();
    Error init();
    Error initWithChatHistory(std::vector<Message> messages, int promptId);
    ChatResultCallback chatResultCallabck;
    AssistantServiceProxy proxy;

private:
    std::tuple<std::string, std::string, std::string, std::string>
    getNlpAndVisionConfigs();
};

OsAssistantPrivate::OsAssistantPrivate() {}

Error OsAssistantPrivate::init() { return proxy.init(); }

Error OsAssistantPrivate::initWithChatHistory(std::vector<Message> messages,
                                              int promptId) {
    return proxy.initWithChatHistory(messages, promptId);
}

OsAssistant::OsAssistant()
    : assistantPrivate_(std::make_unique<OsAssistantPrivate>()) {}

OsAssistant::~OsAssistant() = default;

Error OsAssistant::init() { return assistantPrivate_->init(); }

Error OsAssistant::initWithChatHistory(std::vector<Message> messages,
                                       int promptId) {
    return assistantPrivate_->initWithChatHistory(messages, promptId);
}

void OsAssistant::setChatAsyncCallback(ChatResultCallback callback) {
    assistantPrivate_->chatResultCallabck = std::move(callback);
    assistantPrivate_->proxy.setChatResultCallback(
        assistantPrivate_->chatResultCallabck);
}

Features OsAssistant::supportedFeatures() const {
    return assistantPrivate_->proxy.supportedFeatures();
}

void OsAssistant::chatAsync(const std::string &message) {
    assistantPrivate_->proxy.chatAsync(message);
}

void OsAssistant::stopChat() { assistantPrivate_->proxy.stopChat(); }

void OsAssistant::setPromptId(int promptId) {
    assistantPrivate_->proxy.setPromptId(promptId);
}

void OsAssistant::clearContext() { assistantPrivate_->proxy.clearContext(); }

std::vector<Prompt> OsAssistant::prompts() {
    return assistantPrivate_->proxy.prompts();
}

bool OsAssistant::setPromptOrder(std::list<int> promptIdList) {
    return assistantPrivate_->proxy.setPromptOrder(promptIdList);
}

int OsAssistant::createPrompt(std::string name, std::string content) {
    return assistantPrivate_->proxy.createPrompt(name, content);
}

bool OsAssistant::deletePrompt(int promptId) {
    return assistantPrivate_->proxy.deletePrompt(promptId);
}

bool OsAssistant::updatePrompt(int promptId, std::string name,
                               std::string content) {
    return assistantPrivate_->proxy.updatePrompt(promptId, name, content);
}

std::string OsAssistant::getConfiguredNlpModelInfos() const {
    return assistantPrivate_->proxy.getConfiguredNlpModelInfos();
}

void OsAssistant::setCurrentNlpModel(const std::string &modelName) const {
    assistantPrivate_->proxy.setCurrentNlpModel(modelName);
}

}  // namespace assistant

}  // namespace kyai
