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
