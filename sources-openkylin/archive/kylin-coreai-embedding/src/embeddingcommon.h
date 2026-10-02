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

#ifndef EMBEDDINGCOMMON_H
#define EMBEDDINGCOMMON_H

#include <error.h>

#include <string>
#include <vector>

typedef struct _EmbeddingResult {
    std::vector<float> vector_result = {};
    int error_code = 0;
    std::string error_message = "";

    _EmbeddingResult() {}
    _EmbeddingResult(std::vector<float> vector_result)
        : vector_result(vector_result),
          error_code(AI_COMMON_SUCCESS),
          error_message("Success") {}

    _EmbeddingResult(int error_code, std::string error_message)
        : vector_result({}),
          error_code(error_code),
          error_message(error_message) {}
} EmbeddingResult;

#endif