/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

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