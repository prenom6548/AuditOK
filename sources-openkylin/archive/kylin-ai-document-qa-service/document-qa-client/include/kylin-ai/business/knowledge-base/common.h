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
#include <string>
namespace knowledgebase {

typedef enum : int {
    KNOWLEDGEBASE_SUCCESS = 0,
    KNOWLEDGEBASE_CONNECTION_ERROR,
    KNOWLEDGEBASE_PARSE_PARAM_FAILED,
    KNOWLEDGEBASE_EMBEDDING_FAILED,
    KNOWLEDGEBASE_CHUNK_FAILED,
    KNOWLEDGEBASE_DOCUMENT_EXCEEDS_LIMIT,
    KNOWLEDGEBASE_SEARCH_FILE_INFO_FAILED,
    KNOWLEDGEBASE_VECTOR_DATABASE_FAILED,
    KNOWLEDGEBASE_VECTOR_DATABASE_DATA_UNAVAILABLE,

    KNOWLEDGEBASE_ERROR_UNKNOWN = 99,
} KnowledgebaseErrorCode;

struct Status {
    bool success;
    KnowledgebaseErrorCode errorCode;
    std::string errorMsg;
};
}  // namespace knowledgebase