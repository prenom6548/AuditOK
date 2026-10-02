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

#include "conf.h"

#include <fstream>
#include <stdexcept>

#include "constant.h"

nlohmann::json Configuration::values = nlohmann::json();

bool Configuration::parseConfFile(const std::string& path, const std::string& filename) {
    // 打开文件流
    std::string fullFile = path + filename;
    std::ifstream ifs(fullFile);
    if (!ifs.is_open()) {
        PRINT_ERROR("%s open file failed.\n", fullFile.c_str());
        return false;
    }

    try {
        Configuration::values = nlohmann::json::parse(ifs);
        ifs.close();
        PRINT_INFO("%s parse success. %s\n", fullFile.c_str(), values.dump().c_str());
        return true;
    } catch (const std::exception& e) {
        PRINT_ERROR("%s parse file failed. error: %s\n", fullFile.c_str(), e.what());
        ifs.close();
        return false;
    }
}

nlohmann::json Configuration::getAllValues() { return values; }

std::string Configuration::getString(const std::string& key, const std::string& defaultValue) {
    return values.contains(key) ? values[key].get<std::string>() : defaultValue;
}

int Configuration::getInt(const std::string& key, int defaultValue) {
    return values.contains(key) ? values[key].get<int>() : defaultValue;
}

std::vector<int> Configuration::getIntArray(const std::string& key, const std::vector<int>& defaultValue) {
    std::vector<int> output;
    if (!values.contains(key) || !values[key].is_array()) {
        PRINT_ERROR("Configuration parse input:[%s] as int array failed.\n", values.dump().c_str());
        return std::vector<int>();
    }

    for (const auto& item : values[key]) {
        if (!item.is_number_integer()) {
            PRINT_ERROR("Configuration parse input:[%s] as int array failed.\n", values.dump().c_str());
            break;
        }
        output.push_back(item.get<int>());
    }
    return output;
}

std::vector<std::string> Configuration::getStringArray(const std::string& key,
                                                       const std::vector<std::string>& defaultValue) {
    std::vector<std::string> output;
    if (!values.contains(key) || !values[key].is_array()) {
        PRINT_ERROR("Configuration parse input:[%s] as string array failed.\n", values.dump().c_str());
        return std::vector<std::string>();
    }

    for (const auto& item : values[key]) {
        if (!item.is_string()) {
            PRINT_ERROR("Configuration parse input:[%s] as string array failed.\n", values.dump().c_str());
            break;
        }
        output.push_back(item.get<std::string>());
    }
    return output;
}

LogLevel Configuration::getLogLevel() { return static_cast<LogLevel>(getInt(KEY_LOGLEVEL, LOG_LEVEL_INFO)); }

std::string Configuration::getServiceName() { return getString(KEY_SERVICE_NAME, ""); }
