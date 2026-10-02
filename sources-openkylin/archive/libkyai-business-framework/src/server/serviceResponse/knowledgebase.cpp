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

#include "knowledgebase.h"

#include <iostream>

#include "log.h"

void knowledgebaseSyncResponse(const std::string& method, GObject* delegate, GDBusMethodInvocation* invocation,
                               const std::string& output) {
    if (method == "createKnowledgeBase") {
        ai_business_knowledgebase_complete_create_knowledge_base(reinterpret_cast<AiBusinessKnowledgebase*>(delegate),
                                                                 invocation, output.c_str());
        PRINT_DEBUG("create knowledgebase method.\n");
        return;
    }

    if (method == "deleteKnowledgeBase") {
        ai_business_knowledgebase_complete_delete_knowledge_base(reinterpret_cast<AiBusinessKnowledgebase*>(delegate),
                                                                 invocation, output.c_str());
        PRINT_DEBUG("delete knowledgebase method.\n");
        return;
    }

    if (method == "similaritySearch") {
        ai_business_knowledgebase_complete_similarity_search(reinterpret_cast<AiBusinessKnowledgebase*>(delegate),
                                                             invocation, output.c_str());
        PRINT_DEBUG("similarity search method.\n");
        return;
    }

    if (method == "addTextFiles") {
        ai_business_knowledgebase_complete_add_text_files(reinterpret_cast<AiBusinessKnowledgebase*>(delegate),
                                                          invocation, output.c_str());
        PRINT_DEBUG("add text file method.\n");
        return;
    }

    if (method == "getTextFileContents") {
        ai_business_knowledgebase_complete_get_text_file_contents(reinterpret_cast<AiBusinessKnowledgebase*>(delegate),
                                                                  invocation, output.c_str());
        PRINT_DEBUG("get text file contents method.\n");
        return;
    }

    PRINT_ERROR("unknown method: %s\n", method.c_str());
}