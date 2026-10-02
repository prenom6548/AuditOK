#ifndef DOCXPARSER_H
#define DOCXPARSER_H

#include "parser.h"

class DocxParser : public Parser {
public:
    DocxParser() = default;
    ~DocxParser() = default;

    FileInfo parse(const std::string &filePath) override;
};

#endif // DOCXPARSER_H
