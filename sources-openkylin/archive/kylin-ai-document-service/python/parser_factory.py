# /*
#  * Copyright 2024 KylinSoft Co., Ltd.
#  *
#  * This program is free software: you can redistribute it and/or modify it under
#  * the terms of the GNU General Public License as published by the Free Software
#  * Foundation, either version 3 of the License, or (at your option) any later
#  * version.
#  *
#  * This program is distributed in the hope that it will be useful, but WITHOUT
#  * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
#  * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
#  * details.
#  *
#  * You should have received a copy of the GNU General Public License along with
#  * this program. If not, see <https://www.gnu.org/licenses/>.
#  */

from parser.base_parser import BaseParser
from parser.txt_parser import TXTParser
from parser.json_parser import JSONParser
from parser.html_parser import HTMLParser
from parser.xml_parser import XMLParser
from parser.md_parser import MDParser
from parser.ofd_parser import OFDParser

class ParserFactory:
    def __init__(self) -> None:
        pass
    @staticmethod
    def create_parser(format_type: str) -> BaseParser:
        if format_type == "txt":
            return TXTParser()
        elif format_type == "pdf":
            from parser.pdf_parser import PDFParser
            return PDFParser()
        elif format_type == "docx":
            from parser.docx_parser import DOCXParser
            return DOCXParser()
        elif format_type == "pptx":
            from parser.pptx_parser import PPTXParser
            return PPTXParser()
        # 搜索的解析库支持的格式有doc, wps, ppt, ofd, uof, uop, uot, uos, dot, pps, dps, et, xls
        # 解析服务目前使用到搜索的库解析格式有doc, wps, ppt，ofd
        elif format_type == "doc" or format_type == "wps" or format_type == "ppt" or \
            format_type == "uof" or \
            format_type == "uop" or \
            format_type == "uot" or \
            format_type == "uos" or \
            format_type == "dot" or \
            format_type == "pps" or \
            format_type == "dps" or \
            format_type == "et" or \
            format_type == "xls" :
            from parser.office_parser import OfficeParser
            return OfficeParser()
        elif format_type == "json":
            return TXTParser()  # txt类型的parser也能处理json文件
        elif format_type == "html":
            return HTMLParser()
        elif format_type == "xml":
            return XMLParser()
        elif format_type == "md":
            return MDParser()
        elif format_type == "xlsx":
            from parser.excel_parser import EXCELParser
            return EXCELParser()
        elif format_type == "ofd":
            return OFDParser()
        else:
            raise ValueError(f"Unsupported format: {format_type}")

if __name__ == "__main__":
    import os
    filepath = "/usr/share/kylin-user-guide/data/guide/accessories-text-editor/zh_CN/index.md"
    file_extension = os.path.splitext(filepath)[1]
    file_format = file_extension[1:]
    parser = ParserFactory.create_parser(file_format)
    print(parser.parser(filepath))