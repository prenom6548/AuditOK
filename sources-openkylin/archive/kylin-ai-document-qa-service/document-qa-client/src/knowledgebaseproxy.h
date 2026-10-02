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

#pragma once

#include <gio/gio.h>
#include <kylin-ai/business/knowledge-base/common.h>
#include <kylin-ai/business/knowledge-base/knowledgebase.h>

#include <unordered_map>

#include "knowledgebaseglue.h"

namespace knowledgebase {
typedef gboolean (*AiBusinessKnowledgeBaseCallSyncFunc)(AiBusinessKnowledgebase* proxy, const gchar* arg_input,
                                                        GDBusCallFlags call_flags, gint timeout_msec,
                                                        gchar** out_output, GCancellable* cancellable, GError** error);

#define STATUS_SUCCESS InitializeStatus(true, 0, "success")
#define STATUS_FAILURE(errorCode, errorMsg) InitializeStatus(false, errorCode, errorMsg)

Status InitializeStatus(bool success, int errorCode, const std::string& errorMsg);

extern const std::unordered_map<int, KnowledgebaseErrorCode> ServerToClientErrorMap;
class KnowledgebaseProxy {
public:
    KnowledgebaseProxy();
    ~KnowledgebaseProxy();

    Status isSupported(bool& success) const;

    Status createKnowledgebase(std::string& knowledgebaseName);
    Status deleteKnowledgebase(const std::string& knowledgebaseName);

    Status similaritySearch(const std::string& searchCondition, std::string& result);
    Status addTextFiles(const std::string& input);

    Status getTextFileContents(const std::string& input, std::string& result);

private:
    Status connect();
    void destroy();

    std::string syncFuncCall(const std::string& input, gint timeout_msec, AiBusinessKnowledgeBaseCallSyncFunc func);
    std::string handleGError(GError* gError, const std::string& fallbackMsg, KnowledgebaseErrorCode errorCode);
    Status handleDBusError(GError* gError, const std::string& contextMsg, KnowledgebaseErrorCode errorCode);

private:
    bool getConnection_ = false;
    GDBusConnection* connection_ = nullptr;
    AiBusinessKnowledgebase* delegate_ = nullptr;
};
}  // namespace knowledgebase