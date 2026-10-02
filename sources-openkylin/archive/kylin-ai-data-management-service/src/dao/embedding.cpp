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

 #include "embedding.h"

#include <log.h>
#include <rpc/embedding/imageembeddingprocessor.h>
#include <rpc/embedding/textembeddingprocessor.h>

Embedding::Embedding(DataManagementEmbeddingDataType dataType, scene::TaskPriority priority)
    : dataType_(dataType), priority_(priority) {
    switch (dataType_) {
        case DataManagementEmbeddingDataType::Text:
            if (!scene::TextEmbeddingProcessor::getInstance().init()) {
                PRINT_ERROR("TextEmbeddingProcessor init failed\n");
                inited_ = false;
                return;
            }
            break;
        case DataManagementEmbeddingDataType::ImageFlie:
        case DataManagementEmbeddingDataType::ImageBase64:
        case DataManagementEmbeddingDataType::ImageText:
            if (!scene::ImageEmbeddingProcessor::getInstance().init()) {
                PRINT_ERROR("ImageEmbeddingProcessor init failed.\n");
                inited_ = false;
                return;
            }
            break;
        default:
            PRINT_ERROR("Embedding input unknown data type: %d\n", dataType_);
            inited_ = false;
            return;
    }
    inited_ = true;
}

std::vector<float> Embedding::getEmbedding(const std::string& data) {
    if (!inited_) {
        PRINT_ERROR("EmbeddingProcessor not inited.\n");
        return std::vector<float>();
    }

    if (data.empty()) {
        PRINT_ERROR("EmbeddingProcessor input data is empty.\n");
        return std::vector<float>();
    }

    scene::TaskID taskID;
    std::vector<float> ret;

    switch (dataType_) {
        case DataManagementEmbeddingDataType::Text:
            taskID =
                scene::TextEmbeddingProcessor::getInstance().addTask(data, scene::EmbeddingDataType::Text, priority_);
            ret = scene::TextEmbeddingProcessor::getInstance().getResult(taskID);
            break;
        case DataManagementEmbeddingDataType::ImageFlie:
        case DataManagementEmbeddingDataType::ImageBase64:
            taskID = scene::ImageEmbeddingProcessor::getInstance().addTask(data, scene::EmbeddingDataType::ImageFlie,
                                                                           priority_);
            ret = scene::ImageEmbeddingProcessor::getInstance().getResult(taskID);
            break;
        case DataManagementEmbeddingDataType::ImageText:
            taskID =
                scene::ImageEmbeddingProcessor::getInstance().addTask(data, scene::EmbeddingDataType::Text, priority_);
            ret = scene::ImageEmbeddingProcessor::getInstance().getResult(taskID);
            break;
        default:
            PRINT_ERROR("Embedding input unknown data type: %d\n", dataType_);
            break;
    }

    if (ret.empty()) {
        PRINT_ERROR("Embedding result is empty, data: %s.\n", data.c_str());
    }
    return ret;
}
