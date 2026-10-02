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
        poppler::page* page = doc->create_page(i);
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
