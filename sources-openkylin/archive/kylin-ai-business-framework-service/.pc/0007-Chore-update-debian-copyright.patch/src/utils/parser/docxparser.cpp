#include "docxparser.h"
#include "officepyparserwrapper.h"

FileInfo DocxParser::parse(const std::string &filePath)
{
    auto* parser = OfficePyParserWrapper::getInstance();
    return parser->parse(filePath, "docxparser", "DocxParser");
}
