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
