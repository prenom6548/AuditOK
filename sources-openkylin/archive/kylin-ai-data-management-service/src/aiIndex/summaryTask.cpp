/*
 * Copyright (C) 2024 KylinSoft Co., Ltd.
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

#include "summaryTask.h"

#include <kyai-business-framework/conf.h>
#include <kyai-business-framework/log.h>

#include <nlohmann/json.hpp>

namespace DataManagement {
SummaryTask::SummaryTask() : NlpBaseTask("summary-task", TASK_TYPE_SUMMARY) {}

ChatModelConfig* SummaryTask::getChatModelConfig() {
    ChatModelConfig* config = chat_model_config_create();

    if (Configuration::getString("extract_summary_nlp_model_deploy_type") == "PrivateCloud") {
        PRINT_DEBUG("[%s] nlp model deploy type is PrivateCloud.\n", taskName.c_str());
        chat_model_config_set_deploy_type(config, ModelDeployType::PrivateCloud);
    } else {
        PRINT_DEBUG("[%s] nlp model deploy type is OnDevice.\n", taskName.c_str());
        chat_model_config_set_deploy_type(config, ModelDeployType::OnDevice);
    }

    return config;
}

void SummaryTask::setPrompt() {
    std::string prompt = R"(
#Role
你是一个文档摘要撰写助手，你的职责是根据文档内容撰写文档摘要。

#Flow
Step1:仔细阅读文档，理解文档主要内容；
Step2:根据文档主要内容撰写文档摘要；
Step3:格式化输出结果:严格按照下面的json格式进行输出:
{
    "文档摘要":"..."
}

#Rules
1.文档摘要必须包含文档的主要内容， 不能照抄原文档内容；
2.语言简洁，不要解释，不要自由发挥，不要补充内容；

#Initialize
你需要严格按照<Flow>定义的流程，遵循<Rules>的规则，并按照<Flow>中的要求的json格式输出文档摘要，最多30个字。
)";

    genai_text_set_chat_system_prompt(session, prompt.c_str());
}

void SummaryTask::modifyFileSummary(const std::string& summary) {
    std::string sql =
        R"(UPDATE t_file_info SET file_summary = ?, last_modify_time = datetime('now', 'localtime') WHERE file_id = ?)";
    SQLite::Statement statement(db, sql);
    statement.bind(1, summary);
    statement.bind(2, taskDO.fileId);
    int rows = statement.exec();
    PRINT_DEBUG("[%s] SQL: %s, modify summary success %d rows.\n", taskName.c_str(), statement.getExpandedSQL().c_str(),
                rows);
}

bool SummaryTask::worker() {
    if (nlpResult.empty()) {
        modifyFileSummary(nlpResult);
        return true;
    }

    stripThoughtContent();

    std::string result = parseResult();
    if (result.empty()) {
        return false;
    }

    modifyFileSummary(result);
    return true;
}

std::string SummaryTask::parseResult() {
    PRINT_DEBUG("[%s] summary chat parse result start:%s.\n", taskName.c_str(), nlpResult.c_str());
    nlohmann::json jsonData = nlohmann::json::parse(nlpResult, nullptr, false);
    if (jsonData.is_discarded()) {
        PRINT_ERROR("[%s] summary chat parse result json format error.\n", taskName.c_str());
        return "";
    }

    if (!jsonData.contains("文档摘要") || !jsonData["文档摘要"].is_string()) {
        PRINT_ERROR("[%s] Parse summary json is empty, %s.\n", taskName.c_str(), jsonData.dump().c_str());
        return "";
    }
    if (jsonData["文档摘要"].size() >= 200) {
        PRINT_ERROR("[%s] Parse summary json result size %lu > 200 is discarded.\n", taskName.c_str(),
                    jsonData["文档摘要"].size());
        return "";
    }
    PRINT_DEBUG("[%s] chat result parseSummary end, %s.\n", taskName.c_str(), jsonData["文档摘要"].dump().c_str());
    return jsonData["文档摘要"];
}
}  // namespace DataManagement
