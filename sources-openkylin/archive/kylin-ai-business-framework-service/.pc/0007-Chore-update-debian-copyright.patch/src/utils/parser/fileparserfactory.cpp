#include "fileparserfactory.h"
#include "parser.h"

#include "docxparser.h"
#include "pptxparser.h"
#include "pdfparser.h"
#include "txtparser.h"

std::unique_ptr<Parser> FileParserFactory::createParser(const std::string &format)
{
    std::unique_ptr<Parser> parser;
    if (format == "txt") {
        parser.reset(new TxtParser);
    } else if (format == "pdf") {
        parser.reset(new PDFParser);
    } else if (format == "docx") {
        parser.reset(new DocxParser);
    } else if (format == "pptx") {
        parser.reset(new PptxParser);
    }
    return parser;
}