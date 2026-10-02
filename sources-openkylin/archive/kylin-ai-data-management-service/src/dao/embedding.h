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

#pragma onec

#include <rpc/embedding/embeddingtask.h>

#include <string>
#include <vector>

enum class DataManagementEmbeddingDataType { Text = 1, ImageFlie, ImageBase64, ImageText };

class Embedding {
public:
    Embedding(DataManagementEmbeddingDataType dataType, scene::TaskPriority priority);

    std::vector<float> getEmbedding(const std::string& data);

    ~Embedding() = default;

private:
    DataManagementEmbeddingDataType dataType_;
    scene::TaskPriority priority_;
    bool inited_;
};
