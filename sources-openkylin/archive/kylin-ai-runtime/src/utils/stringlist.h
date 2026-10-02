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

#ifndef UTILS_STRING_LIST_H
#define UTILS_STRING_LIST_H

#include <string>
#include <vector>

namespace util {

class StringList {
public:
    StringList();

    ~StringList();

    void append(const std::string &str);

    bool removeAll(const std::string &str);

    size_t count() const;

    std::string at(size_t index) const;

    void clear();

    bool contains(const std::string &str);

private:
    std::vector<std::string> list_;
};

}  // namespace util

#endif
