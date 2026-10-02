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

#include <unordered_map>

// 使用vector为了保证顺序，pair key: filepath, value: similarity
using SimilaritySearchResult = std::vector<std::pair<std::string, float>>;

typedef struct dataManagementTagInfo {
    std::string tagName;
    int64_t tagId;
} DataManagementTagInfo;

typedef struct dataManagementTagFileInfo {
    std::string filePath;
    std::vector<DataManagementTagInfo> tags;
    std::string summary;
} DataManagementTagFileInfo;

typedef struct dataManagementFileInfo {
    std::string filePath;
    std::string modifyTime;
    std::vector<DataManagementTagInfo> tags;
    std::string summary;
    int64_t fileId;
} DataManagementFileInfo;

typedef struct dataManagementChunkFileInfo {
    int64_t fileId;
    std::string filePath;
    std::vector<std::string> chunks;
} DataManagementChunkFileInfo;

extern const char* FILE_TAGS_COLLECTION_NAME;
extern const char* FILE_SUMMARY_COLLECTION_NAME;
extern const char* VISION_FILE_CONTENT_COLLECTION_NAME;
extern const char* TEXT_FILE_CONTENT_COLLECTION_NAME;

extern const int TEXT_DIMENSION;
extern const int IMAGE_DIMENSION;

extern const float TEXT_SEARCH_THRESHOLD;
extern const float VISION_SEARCH_THRESHOLD;
extern const float TAG_SEARCH_THRESHOLD;

extern const char* CHUNKS;
extern const char* CONTENT;
extern const char* FILEPATH;
