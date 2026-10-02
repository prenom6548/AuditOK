#ifndef PPTXPARSER_H
#define PPTXPARSER_H

#include "parser.h"

class PptxParser : public Parser {
public:
    PptxParser() = default;
    ~PptxParser() = default;

    FileInfo parse(const std::string &filePath) override;
};

#endif // PPTXPARSER_H
