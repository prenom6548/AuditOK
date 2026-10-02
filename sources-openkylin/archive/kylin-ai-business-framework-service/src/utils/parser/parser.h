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

#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>
#include <iostream>
#include <memory>

struct FileInfo {
    std::string author;
    std::string title;
    int pages = -1;
    time_t modifyTime = -1;
    std::vector<std::string> contents;

    void print() {
        std::cout << "Contents: " << std::endl;
        for (int i = 0; i < contents.size(); ++i) {
            std::cout << "Page " << i + 1 << ":\n" << contents[i] << std::endl;
        }

        std::cout << "Author: " << author << std::endl;
        std::cout << "Title: " << title << std::endl;
        std::cout << "Pages: " << pages << std::endl;
        std::cout << "Modify time: " << modifyTime << std::endl;
    }
};

class Parser {
public:
    Parser() = default;
    virtual ~Parser() {}

    virtual FileInfo parse(const std::string& filePath) = 0;

    static time_t modifyTime(const std::string& filePath);
};

#endif // PARSER_H
