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

#include "stringlist.h"

#include <algorithm>

namespace util {
StringList::StringList() {}

StringList::~StringList() {}

void StringList::append(const std::string &str) { list_.push_back(str); }

bool StringList::removeAll(const std::string &str) {
    auto iter = std::remove_if(list_.begin(), list_.end(), [str](const std::string &theStr) { return theStr == str; });
    if (iter == list_.end()) {
        return false;
    }
    list_.erase(iter, list_.end());
    return true;
}

size_t StringList::count() const { return list_.size(); }

std::string StringList::at(size_t index) const {
    if (index >= list_.size()) {
        return std::string{};
    }
    return list_[index];
}

void StringList::clear() { list_.clear(); }

bool StringList::contains(const std::string &str) {
    auto iter = std::find(list_.begin(), list_.end(), str);
    return iter != list_.end();
}

}  // namespace util
