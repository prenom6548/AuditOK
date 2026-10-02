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
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "utils.h"

#include <uuid/uuid.h>
#include <iostream>
#include <cmath>

namespace utils {

std::string generateUUID()
{
    uuid_t uuid;
    char uuid_str[37];

    uuid_generate_random(uuid);
    uuid_unparse(uuid, uuid_str);
    std::string id = uuid_str;

    return id;
}

}

namespace math
{
void normalize(std::vector<float> &vec)
{
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

// 向量取模
float norm(const std::vector<float>& vec)
{
    float norm = 0.0f;
    for (const float& element : vec) {
        norm += element * element;
    }
    norm = std::sqrt(norm);

    return norm;
}

void normalizeTokens(std::vector<int64_t> &tokens, int length)
{
    if (tokens.size() == length) {
        return;
    } else if (tokens.size() > length) {
        tokens[length - 1] = tokens.back();
        tokens.resize(length);
        std::cerr << "Input tokens exceed " << length
                  << ", the tokens will be cropped" << std::endl;
    } else {
        tokens.resize(52, 0);
    }
}
} // namespace math
