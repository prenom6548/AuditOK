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

from docx import Document
from docx.table import Table
from docx.text.paragraph import Paragraph
import time
import re
from .base_parser import BaseParser

class DOCXParser(BaseParser):

    def parser_docx(self, file_path) -> str:
        docx = Document(file_path)

        content_list = []
        body = docx.element.body

        # 正文段落和表格
        for child in body.iterchildren():
            if child.tag.endswith('}p'):  # 段落
                paragraph = Paragraph(child, docx)
                text = paragraph.text.strip()
                if text:
                    content_list.append(text)
            elif child.tag.endswith('}tbl'):  # 表格
                table = Table(child, docx)
                for row in table.rows:
                    row_text = [cell.text.strip() for cell in row.cells]
                    content_list.append(','.join(row_text))

        result = "\n".join(content_list)
        return result

    def parser(self, file_path) -> str:
        """
        解析docx文件
        """
        return self.parser_docx(file_path)

    def is_paragraph_recognition_supported(self,file_type:str) -> bool:
        '''
        docx解析器提取到段落信息
        file_type: 文件类型
        return: 是否支持段落识别
        '''
        return True


if __name__ == '__main__':
    DocxParser = DOCXParser()
    paras = DocxParser.parser('/home/kylin/桌面/文档处理服务/文档解析服务测试文档/损坏的文件/损坏的文件/损坏.docx')
    print(paras)
