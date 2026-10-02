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

class Request {
public:
    explicit Request(const std::string& input);

    nlohmann::json getValues() const { return values; }

    std::string getString(const std::string& key) const;

    std::vector<std::string> getStringArray(const std::string& key) const;

    int getInt(const std::string& key) const;

    std::vector<int> getIntArray(const std::string& key) const;

    double getDouble(const std::string& key) const;

    float getFloat(const std::string& key) const;

    std::string toString() const;

private:
    nlohmann::json values;
};
