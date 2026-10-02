#ifndef TXTPARSER_H
#define TXTPARSER_H

#include "parser.h"

class TxtParser : public Parser {
public:
    TxtParser() = default;
    ~TxtParser() = default;

    FileInfo parse(const std::string& filePath) override;
};

#endif // TXTPARSER_H
