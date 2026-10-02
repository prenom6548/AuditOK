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

#include <scene.h>

#include "database.h"

Response handleDeleteKnowledgeBase(const Request& request) {
    PRINT_DEBUG("handleDeleteKnowledgeBase request: %s\n", request.toString().c_str());
    nlohmann::json values = request.getValues();
    if (values.empty() || !values.contains("knowledgebase_name")) {
        PRINT_ERROR("handleDeleteKnowledgeBase request param error.\n");
        return Response(PARSE_PARAM_FAILED);
    }

    std::string knowledgeBaseName = values["knowledgebase_name"];

    KnowledgebaseVectorDatabase database;
    auto status = database.deleteCollection(knowledgeBaseName);
    if (!status.success) {
        if (status.code == VectorDB::StatusCode::DATABASE_INIT_FAILED) {
            PRINT_ERROR("handleDeleteKnowledgeBase failed: Database init error - %s\n", status.message.c_str());
            return Response(VECTOR_DATABASE_DATA_UNAVAILABLE);
        }
        PRINT_ERROR("handleDeleteKnowledgeBase failed: Unknown database delete error - %s\n", status.message.c_str());
        return Response(DELETE_VECTOR_DATABASE_FAILED);
    }

    return Response::success();
}

REGISTER_SERVICE_HANDLER("deleteKnowledgeBase", handleDeleteKnowledgeBase)