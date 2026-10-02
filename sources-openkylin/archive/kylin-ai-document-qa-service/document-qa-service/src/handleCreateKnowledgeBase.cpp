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

Response handleCreateKnowledgeBase(const Request& request) {
    PRINT_DEBUG("handleCreateKnowledgeBase request: %s\n", request.toString().c_str());
    std::string collectionName;
    KnowledgebaseVectorDatabase database;
    auto status = database.createCollection(collectionName);
    if (!status.success) {
        if (status.code == VectorDB::StatusCode::DATABASE_INIT_FAILED) {
            PRINT_ERROR("handleCreateKnowledgeBase failed: Database init error - %s\n", status.message.c_str());
            return Response(VECTOR_DATABASE_DATA_UNAVAILABLE);
        }
        PRINT_ERROR("handleCreateKnowledgeBase failed: Unknown database create error - %s\n", status.message.c_str());
        return Response(CREATE_VECTOR_DATABASE_FAILED);
    }

    Response output = Response::success();
    output.add("knowledgebase_name", collectionName);
    return output;
}

REGISTER_SERVICE_HANDLER("createKnowledgeBase", handleCreateKnowledgeBase)