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

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "log.h"

class Configuration {
public:
    static bool parseConfFile(const std::string& path, const std::string& filename);
    static nlohmann::json getAllValues();
    static std::string getString(const std::string& key, const std::string& defaultValue = "");
    static int getInt(const std::string& key, int defaultValue = 0);
    static std::vector<int> getIntArray(const std::string& key, const std::vector<int>& defaultValue);
    static std::vector<std::string> getStringArray(const std::string& key,
                                                   const std::vector<std::string>& defaultValue);
    static LogLevel getLogLevel();
    static std::string getServiceName();

private:
    static nlohmann::json values;
};
