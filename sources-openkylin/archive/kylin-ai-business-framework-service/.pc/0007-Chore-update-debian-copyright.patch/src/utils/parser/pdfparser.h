#ifndef PDFPARSER_H
#define PDFPARSER_H

#include "parser.h"

class PDFParser : public Parser {
public:
    PDFParser() = default;
    ~PDFParser() = default;

    FileInfo parse(const std::string& filePath) override;

private:
    std::string vectorCharToString(std::vector<char>&& vectorChar) const;
};


#endif // PDFPARSER_H
