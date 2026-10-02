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

#include "settings.h"

#include "logger.h"

namespace util {

Settings::Settings() = default;

void Settings::beginReadArray(const std::string& name) { currentName_ = name; }

void Settings::beginWriteArray(const std::string& name) { currentName_ = name; }

void Settings::endArray() {
    currentName_.clear();
    currentIndex_ = -1;
}

void Settings::setArrayIndex(int index) { currentIndex_ = index; }

void Settings::setValue(const std::string& key, const std::string& value) {
    if (currentName_.empty()) {
        return;
    }
    map_[currentName_][currentIndex_][key] = value;
}

std::string Settings::value(const std::string& key) {
    if (currentName_.empty()) {
        return std::string{};
    }
    return map_[currentName_][currentIndex_][key];
}

void Settings::print() {
    for (auto& [name, array] : map_) {
        for (auto& [index, item] : array) {
            for (auto& [key, value] : item) {
                LOGD("name {} index [{}] key:{} value:{}", name, index, key, value);
            }
        }
    }
}

size_t Settings::count() { return map_[currentName_].size(); }

}  // namespace util
