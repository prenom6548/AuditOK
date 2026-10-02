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

#include "utils.h"

#include <uuid/uuid.h>

#include <cmath>
namespace ai_engine::core_ai::embedding {
std::string generateUUID() {
    uuid_t uuid;
    uuid_generate(uuid);

    char uuidStr[37];
    uuid_unparse(uuid, uuidStr);

    return std::string(uuidStr);
}

// 向量取模
float norm(const std::vector<float>& vec) {
    float norm = 0.0f;
    for (const float& element : vec) {
        norm += element * element;
    }
    norm = std::sqrt(norm);

    return norm;
}

void normalize(std::vector<float>& vec) {
    if (vec.empty()) {
        return;
    }

    float normNum = norm(vec);
    if (normNum == 0.0) {
        return;
    }

    for (int i = 0; i < vec.size(); ++i) {
        vec[i] = vec[i] / normNum;
    }
}

}  // namespace ai_engine::core_ai::embedding
