#include "pptxparser.h"

#include "officepyparserwrapper.h"

FileInfo PptxParser::parse(const std::string &filePath)
{
    auto* parser = OfficePyParserWrapper::getInstance();
    return parser->parse(filePath, "pptxparser", "PptxParser");
}
