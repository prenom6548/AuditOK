#ifndef FILEPARSERFACTORY_H
#define FILEPARSERFACTORY_H

#include <string>
#include <memory>

#include "parser.h"

class FileParserFactory {
public:
    FileParserFactory() = default;
    ~FileParserFactory() {}
    static std::unique_ptr<Parser> createParser(const std::string &format);
};

#endif // FILEPARSERFACTORY_H