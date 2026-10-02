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

import fitz  # PyMuPDF
from .base_parser import BaseParser
from log_config import get_logger
logger = get_logger()

class PDFParser(BaseParser):
    def parser(self, file_path) -> str:
        """
        同时解析PDF文件中的文本和表格，并将它们合并到一个列表中。
        :return: 包含每页内容（文本和表格）的列表
        加密文档直接返回None
        """
        pdf = fitz.open(file_path)

        # pdf处理之前先判断是否加密，对于已经加密的文档，就不去解析了，直接抛出异常
        if pdf.is_encrypted:
            logger.error(f"File is encrypted,file_path:{file_path}")
            return None

        content = []
        for page_num in range(pdf.page_count):
            page = pdf.load_page(page_num)
            results = self._parse_page(page)
            if results:
                content.extend(results)

        texts = ",".join(content)
        return texts

    def _parse_page(self, page) -> list:
        """
        解析单个页面的文本和表格。
        :param page: PyMuPDF 页面对象
        :return: 包含文本和表格数据的列表
        """
        blocks = page.get_text("blocks")
        if not blocks:   # 如果是全图pdf，block为空
            logger.debug("Block is empty. This PDF file is an image.")
            return []

        page_blocks = page.get_text("dict")["blocks"]
        # 获取水印块起始位置的索引
        offset = self.watermark_offset(page_blocks)
        if offset:
            text = self.clean_incorrect_linebreaks(self.get_block_text(blocks[:offset]))
        # 无水印的情况，直接提取页面中的全部文本
        else:
            text = self.clean_incorrect_linebreaks(page.get_text("text"))

        # 提取表格
        tables = []
        # blocks = page.get_text("dict")["blocks"]
        # for block in blocks:
        #     if block["type"] == 0:
        #         table_data = []
        #         for line in block["lines"]:
        #             row_data = [span["text"] for span in line["spans"]]
        #             table_data.append(", ".join(row_data))
        #         tables.extend(table_data)
        # 将文本和表格合并到一个列表中
        combined_data = [text] + tables
        result = "".join(combined_data)
        return [result]

    def watermark_offset(self, blocks_dict) -> int:
        """
        水印文本块开始的块索引。PyMuPDF解析出的页面，会将水印文本块集中在最后的块中。
        这是由PDF页面的渲染、绘制标准流程决定的，PDF绘制的内容流基本按照由底层到顶层的顺序绘制出整个页面，
        为了能够显示出位于顶层的水印，这些绘制水印的内容流位置在最后。
        解析PDF页面的过程是按照内容流顺序进行的，所以水印文本最后被解析。（参考Adobe PDF Reference Manual 或 ISO32000标准）
        :param blocks_dict: 由包含每个block详细信息的字典为元素，组成的列表
        :return: 水印文本块相对于最后的偏移量，可以理解为正文和水印文本块的分界。
        """
        watermark_offset = 0
        # 水印所在的块被识别在最后的位置，倒序遍历blocks能够更快速找到
        for block in reversed(blocks_dict):
            """
            type: 0代表文本类型块，1代表图像类型块
            dir: (cosA, -sinA), A代表该行与x轴正方向的夹角。水平方向从左到右书写的文本，A=0，故dir=(1, 0)
            """
            if block["type"] == 0 and block["lines"][0]["dir"] != (1, 0):
                watermark_offset += 1
            # 水印块是连续的，如果出现了非水印块，直接跳出
            else:
                break

        # 由于是倒序查找，偏移量是相对于最后一个块确定的
        return -watermark_offset

    def get_block_text(self, blocks) -> str:
        """
        获取块中的全部文本。
        :param blocks: 文本块列表
        :return: 文本字符串
        """
        all_text = ""
        for block in blocks:
            # block元组的索引为4的元素是该块包含的文本内容
            text = block[4].strip()
            all_text += text

        return all_text

    def clean_incorrect_linebreaks(self,text):
        """
        清理字符串中不符合规则的换行符

        规则：
        1. 换行符前必须是以下中文或英文结束标点：
        中文：句号（。）、问号（？）、感叹号（！）、省略号（……）、后引号（"）、后书名号（》）
        英文：句号(.)、问号(?)、感叹号(!)、后引号(")、后括号())
        2. 换行符后不能有任何中英文标点符号
        3. 特殊情况：如果是冒号（：）后面有换行符，且换行符后面有◦符号，则保留该换行符

        参数:
        text (str): 需要处理的文本

        返回:
        str: 处理后的文本
        """
        import re

        # 定义有效的前置标点和无效的后置标点
        valid_before = r'[。？！……"》\.?!\"\)]'
        invalid_after = r'[，。？！：；""''「」『』《》【】（）{}〈〉[]\.\,\?\!\:\;\'\"\-\+\=\*\/\\\&\|\@\#\$\%\^\&\*\(\)\{\}\[\]\<\>]'

        # 匹配所有换行符，然后根据前后文决定是否保留
        def replace_func(match):
            prev_char = text[match.start()-1] if match.start() > 0 else ''
            next_char = text[match.start()+1] if match.start() + 1 < len(text) else ''

            # 特殊情况：冒号后换行符且后面有◦符号，保留换行符
            if prev_char == '：' and next_char == '◦':
                return '\n'  # 保留这个特殊的换行符

            # 检查常规规则：
            # 1. 前面必须是有效结束标点
            # 2. 后面不能是任何标点符号
            if re.match(valid_before, prev_char) and not re.match(invalid_after, next_char):
                return '\n'  # 保留正确的换行符
            else:
                return ''    # 删除错误的换行符

        # 使用回调函数处理每个换行符
        pattern = r'\n'
        cleaned_text = re.sub(pattern, replace_func, text)

        return cleaned_text

    def is_paragraph_recognition_supported(self,file_type:str) -> bool:
        '''
        pdf 解析结果的换行不是真正的换行 但是对解析结果进行了后处理，删掉了一些不符合规则的换行
        保证了大部分场景下的段落是正确的，认为可以识别到段落信息
        '''
        return True

if __name__ == '__main__':
    pdf_path = r"/home/kylink/pdf_files/embedding_wmark.pdf"
    myParser = PDFParser()
    result = myParser.parser(pdf_path)

    print(result)
