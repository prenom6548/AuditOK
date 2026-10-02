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

#include "request.h"

#include "log.h"
#include "utils.h"

Request::Request(const std::string& input) {
    values = nlohmann::json::parse(input, nullptr, false);
    if (values.is_discarded()) {
        PRINT_ERROR("Request parse input:%s failed.\n", input.c_str());
    }
}

std::string Request::getString(const std::string& key) const {
    return values.contains(key) ? values[key].get<std::string>() : "";
}

std::vector<std::string> Request::getStringArray(const std::string& key) const {
    std::vector<std::string> output;
    if (!values.contains(key) || !values[key].is_array()) {
        PRINT_ERROR("Request parse input:[%s] as string array failed.\n", toString().c_str());
        return std::vector<std::string>();
    }

    for (const auto& item : values[key]) {
        if (!item.is_string()) {
            PRINT_ERROR("Request parse input:[%s] as string array failed.\n", toString().c_str());
            break;
        }
        output.push_back(item.get<std::string>());
    }
    return output;
}

int Request::getInt(const std::string& key) const { return values.contains(key) ? values[key].get<int>() : 0; }

std::vector<int> Request::getIntArray(const std::string& key) const {
    std::vector<int> output;
    if (!values.contains(key) || !values[key].is_array()) {
        PRINT_ERROR("Request parse input:[%s] as int array failed.\n", toString().c_str());
        return std::vector<int>();
    }

    for (const auto& item : values[key]) {
        if (!item.is_number_integer()) {
            PRINT_ERROR("Request parse input:[%s] as int array failed.\n", toString().c_str());
            break;
        }
        output.push_back(item.get<int>());
    }
    return output;
}

double Request::getDouble(const std::string& key) const {
    return values.contains(key) ? values[key].get<double>() : 0.0;
}

float Request::getFloat(const std::string& key) const { return values.contains(key) ? values[key].get<float>() : 0.0f; }

std::string Request::toString() const { return values.dump(); }
