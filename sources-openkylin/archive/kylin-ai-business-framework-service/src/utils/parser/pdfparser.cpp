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

#include "pdfparser.h"

#include <poppler-document.h>
#include <poppler-page.h>

#include <filesystem>
#include <iostream>
#include <memory>

FileInfo PDFParser::parse(const std::string &filePath)
{
    FileInfo info;
    if (!std::filesystem::exists(filePath)) {
        std::cerr << "File " << filePath << " don't exist, "
                  << "PDFParser can't parse." << std::endl;
        return info;
    }

    auto doc = std::unique_ptr<poppler::document>(poppler::document::load_from_file(filePath));

    if (doc.get() == nullptr) {
        std::cerr << "Poppler open file: " << filePath << " failed." << std::endl;
        return info;
    }
    if (doc->is_locked()) {
        std::cout << "Document " << filePath << " is locked, can't parser it" << std::endl;
        return info;
    }

    info.author = vectorCharToString(doc->get_author().to_utf8());
    info.title = vectorCharToString(doc->get_title().to_utf8());
    info.pages = doc->pages();
    info.modifyTime = modifyTime(filePath);

    for (int i = 0; i < doc->pages(); ++i) {
        auto page = std::unique_ptr<poppler::page>(doc->create_page(i));
        if (!page)
            continue;

        std::string text = vectorCharToString(page->text().to_utf8());
        info.contents.emplace_back(std::move(text));
    }

    return info;
}

std::string PDFParser::vectorCharToString(std::vector<char> &&vectorChar) const
{
    return std::string(std::make_move_iterator(vectorChar.begin()), std::make_move_iterator(vectorChar.end()));
}
