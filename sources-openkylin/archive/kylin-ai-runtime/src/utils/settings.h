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
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef UTILS_SETTINGS_H
#define UTILS_SETTINGS_H

#include <filesystem>
#include <map>
#include <string>

namespace util {

// 不考虑存储方式
class Settings {
public:
    Settings();

    void beginReadArray(const std::string& name);

    void beginWriteArray(const std::string& name);

    void endArray();

    void setArrayIndex(int index);

    void setValue(const std::string& key, const std::string& value);

    std::string value(const std::string& key);

    void print();

    size_t count();

private:
    std::map<std::string, std::map<int, std::map<std::string, std::string>>> map_{};
    std::string currentName_{};
    int currentIndex_{-1};
};

}  // namespace util

#endif
