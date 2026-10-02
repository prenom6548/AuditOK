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

#ifndef __KYLIN_AI_PRIVATE_OS_ASSISTANT_H__
#define __KYLIN_AI_PRIVATE_OS_ASSISTANT_H__

#include <functional>
#include <list>
#include <memory>
#include <vector>

#include "error.h"
#include "kyaiexport.h"

namespace kyai {
namespace assistant {

// json chat result
using ChatResultCallback = std::function<void(const std::string)>;

enum PromptContentAccessType { ReadOnly = 1, ReadWrite, NoAccess };

struct KYAI_ASSISTANT_EXPORT Prompt {
    int id;
    std::string name;
    std::string content;
    PromptContentAccessType type;
    bool active : 8;
};

struct KYAI_ASSISTANT_EXPORT Message {
    std::string user;
    std::string assistant;
};

class KYAI_ASSISTANT_EXPORT Features {
public:
    enum Feature {
        TextChat = 1,
        TextToImage = 2,
        ImageProcessing = 4,
        DocumentQA = 8
    };

    explicit Features(int features) : features_(features) {}

    bool containsFeature(Feature feature) const { return features_ & feature; }

private:
    int features_;
};

class OsAssistantPrivate;
class KYAI_ASSISTANT_EXPORT OsAssistant {
public:
    OsAssistant();
    ~OsAssistant();

    // blocked init
    Error init();
    Error initWithChatHistory(std::vector<Message> messages, int promptId = -1);

    void setChatAsyncCallback(ChatResultCallback callback);

    Features supportedFeatures() const;

    // json message
    void chatAsync(const std::string &message);
    void stopChat();

    void setPromptId(int promptId);

    void clearContext();

    std::vector<Prompt> prompts();
    bool setPromptOrder(std::list<int> promptIdList);

    // 创建失败返回-1
    int createPrompt(std::string name, std::string content);
    bool deletePrompt(int promptId);
    bool updatePrompt(int promptId, std::string name, std::string content);

    std::string getConfiguredNlpModelInfos() const;
    void setCurrentNlpModel(const std::string &modelName) const;

private:
    std::unique_ptr<OsAssistantPrivate> assistantPrivate_;
};

}  // namespace assistant

}  // namespace kyai

#endif  // __KYLIN_AI_PRIVATE_OS_ASSISTANT_H__
