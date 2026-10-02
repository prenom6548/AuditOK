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

#ifndef PROMPTTEMPLATE_H
#define PROMPTTEMPLATE_H

#include <jsoncpp/json/json.h>

#include <map>
#include <string>
#include <unordered_map>
#include <vector>

namespace config {
class PromptTemplate {
public:
    enum class PromptId {
        Default = 0,                       // 默认
        TextExpansion = 1,                 // 文本扩写
        TextPolishing = 2,                 // 文本润色
        TranslateChineseToEnglish = 3,     // 中英互译
        FileQuestionAnswering = 4,         // 系统助手/文档问答
        TodoListGeneration = 5,            // 生成待办
        WeeklyReportGeneration = 6,        // 整理周报
        MeetingInformationExtraction = 7,  // 新建日程
        ErrorCorrection = 8,               // 内容校对
        TextSummary = 9,                   // 总结概括
        EmailReply = 10,                   // 邮件回复
        WorkSummary = 11,                  // 工作总结
        DocTagExtraction = 12,             // 文档标签提取，端侧特有
        IntentionRecognition = 13,         // 系统控制，意图识别
    };

    constexpr static int maxDefaultPromptId = 100;  // 自带PromptId的最大值

    enum PromptContentAccessType { ReadOnly = 1, ReadWrite, NoAccess };

    struct Prompt {
        int id;
        std::string name;
        std::string content;
        PromptContentAccessType type;
    };

    PromptTemplate(const std::string &promptConfigFile, const std::string &promptFilePath);
    Prompt prompt(const int id) const;

    std::vector<Prompt> prompts();
    int createPrompt(const std::string &name, const std::string &content);
    bool deletePrompt(int promptId);
    bool updatePrompt(int promptId, const std::string &name, const std::string &content);

private:
    void dissectPrompts(const std::string &promptFile, const std::string &promptFilePath);
    int getIdleId();

private:
    std::map<int, Prompt> prompts_;
};

}  // namespace config

#endif  // PROMPTTEMPLATE_H
