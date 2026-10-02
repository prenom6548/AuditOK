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

#include <kylin-ai-document-service/include/client.h>

#include <nlohmann/json.hpp>

struct ChunkResult {
    std::string filePath;
    std::vector<std::string> chunk;
};

class Parser {
public:
    Parser();
    std::vector<ChunkResult> chunkFiles(const std::vector<std::string>& filePaths);

    ~Parser() = default;

private:
    nlohmann::json syncChunkDocument(const std::vector<std::string>& filePaths);
    std::vector<ChunkResult> parseChunkResult(nlohmann::json& data);

private:
    std::shared_ptr<document::DocumentChunkClient> client_;
};
