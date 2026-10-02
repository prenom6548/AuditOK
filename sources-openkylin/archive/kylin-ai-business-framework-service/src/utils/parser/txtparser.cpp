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

#include "txtparser.h"

#include <fstream>
#include <iostream>
#include <filesystem>

FileInfo TxtParser::parse(const std::string &filePath)
{
    FileInfo info;

    if (!std::filesystem::exists(filePath)) {
        std::cerr << "File " << filePath << " don't exist, "
                  << "TxtParser can't parse." << std::endl;
        return info;
    }

    std::ifstream file(filePath);
    std::string content;

    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            for (char c : line) {
                if (c != '\n' && c != '\t') {
                    content += c;
                }
            }
        }
        file.close();
    } else {
        std::cerr << "Can't to open file: " << filePath << std::endl;
    }

    info.contents.emplace_back(std::move(content));
    info.modifyTime = modifyTime(filePath);

    return info;
}
