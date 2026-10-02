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

#include <uuid.h>

#include <numeric>

#include "log.h"

std::string Utils::generateUUID() {
    uuid_t uuid;
    char uuidStr[37];

    uuid_generate_random(uuid);
    uuid_unparse(uuid, uuidStr);

    return uuidStr;
}

std::string Utils::camelCaseToKebab(const std::string& str) {
    std::string result;
    for (char c : str) {
        if (std::isupper(c)) {
            if (!result.empty()) {
                result += '-';
            }
            result += static_cast<char>(std::tolower(c));
        } else {
            result += c;
        }
    }
    return result;
}

std::string Utils::joinIds(const std::unordered_set<int64_t>& ids) {
    std::string result =
        std::accumulate(ids.begin(), ids.end(), std::string(), [](const std::string& params, int64_t id) {
            return params.empty() ? std::to_string(id) : params + ", " + std::to_string(id);
        });
    return result;
}
