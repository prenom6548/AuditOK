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

const char* FILE_TAGS_COLLECTION_NAME = "files_tags";
const char* FILE_SUMMARY_COLLECTION_NAME = "files_summary";  // 摘要向量暂时未用
const char* VISION_FILE_CONTENT_COLLECTION_NAME = "vision_files_content_vector";
const char* TEXT_FILE_CONTENT_COLLECTION_NAME = "text_files_content_vector";

extern const int TEXT_DIMENSION = 768;    // 文本模型维度 1024，目前先写死
extern const int IMAGE_DIMENSION = 1024;  // 图片模型维度 512，目前先写死

extern const float TEXT_SEARCH_THRESHOLD = 0.75;
extern const float VISION_SEARCH_THRESHOLD = 0.5;
extern const float TAG_SEARCH_THRESHOLD = 0.7;

const char* CHUNKS = "chunks";
const char* CONTENT = "content";
const char* FILEPATH = "filePath";
