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

#include "_toolchoice.h"

#include <common/error.h>

#include "_genaitextsession.h"

GenAiToolChoiceConfig* genai_text_create_tool_choice_config(GenAiToolChoiceMode mode) {
    auto config = new _GenAiToolChoiceConfig;
    if (mode >= GENAI_TEXT_TOOL_CHOICE_AUTO && mode <= GENAI_TEXT_TOOL_CHOICE_REQUIRED) {
        config->mode = mode;
    }
    return config;
}

void genai_text_tool_choice_config_set_specific_tool(GenAiToolChoiceConfig* config, const char* tool_name) {
    if (config == nullptr) {
        return;
    }
    if (tool_name == nullptr) {
        config->toolName.clear();
        return;
    }
    config->toolName = tool_name;
}

void genai_text_destroy_tool_choice_config(GenAiToolChoiceConfig** config) {
    if (config == nullptr) {
        return;
    }
    delete *config;
    *config = nullptr;
}

int genai_text_register_tool(GenAiTextSession* session, const char* tool_name, GenAiToolType tool_type,
                             const char* json_schema) {
    if (session == nullptr) {
        return AI_COMMON_INVALID_SESSION;
    }
    if (tool_name == nullptr || json_schema == nullptr) {
        return AI_COMMON_INVALID_PARAMETER;
    }
    return session->registerTool(tool_name, tool_type, json_schema);
}

int genai_text_clear_tools(GenAiTextSession* session) {
    if (session == nullptr) {
        return AI_COMMON_INVALID_SESSION;
    }
    return session->clearTools();
}

void genai_text_set_tool_handler(GenAiTextSession* session, GenAiToolHandler handler, void* user_data) {
    if (session == nullptr) {
        return;
    }
    session->setToolHandler(handler, user_data);
}

int genai_text_submit_tool_result(GenAiTextSession* session, const char* tool_calls_json, const char* result_json) {
    if (session == nullptr) {
        return AI_COMMON_INVALID_SESSION;
    }
    return session->submitToolResult(tool_calls_json, result_json);
}

void genai_text_set_tool_choice_config(GenAiTextSession* session, const GenAiToolChoiceConfig* config) {
    if (session == nullptr) {
        return;
    }
    if (config == nullptr) {
        return;
    }
    session->setToolChoiceConfig(*config);
}

int getToolChoiceModeType(const _GenAiToolChoiceConfig& toolChoiceConfig) { return toolChoiceConfig.mode; }

Json::Value getToolChoiceConfig(const _GenAiToolChoiceConfig& toolChoiceConfig) {
    Json::Value root;
    root["tool_name"] = toolChoiceConfig.toolName;
    root["tool_mode"] = getToolChoiceModeType(toolChoiceConfig);
    return root;
}
