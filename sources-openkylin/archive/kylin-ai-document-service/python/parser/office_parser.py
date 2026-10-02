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

import sys
import platform
arch = platform.machine()
sys.path.append(f'/usr/lib/{arch}-linux-gnu/kylin-ai-parser-extention/')

from .base_parser import BaseParser
import importlib.util
from log_config import get_logger
logger = get_logger()

class OfficeParser(BaseParser):
    def __init__(self) -> None:
        self.module_available = self._module_exists('libparser_extension')

        if self.module_available:
            import libparser_extension
            self.object = libparser_extension.ParserExtension()
        else:
            logger.warning(f"Warning: The required module 'libparser_extension' is not found. Parsing functionality will be limited.")

    def _module_exists(self, module_name: str) -> bool:
        """
        检查指定的模块是否存在。
        """
        spec = importlib.util.find_spec(module_name)
        return spec is not None

    def filter_special_characters(self,text):
        """
        过滤解析内容中的特殊控制字符和Word字段代码

        参数:
            text: 输入的字符串

        返回:
            过滤后的干净字符串
        """
        if not isinstance(text, str):
            return text
        import re
        # 第一步：移除完整的Word字段代码模式
        # 匹配 \u0013 EMBED * MERGEFORMAT \u0014\u0001\u0015 这种完整模式
        word_field_pattern = r'\u0013\s*EMBED\s*\\\\\*\s*MERGEFORMAT\s*\u0014\u0001\u0015'
        text = re.sub(word_field_pattern, '', text)

        # 第二步：移除其他Word字段代码变体（更通用的匹配）
        # 匹配任何以 \u0013 开始，以 \u0015 结束的字段代码
        generic_field_pattern = r'\u0013[^\u0015]*\u0015'
        text = re.sub(generic_field_pattern, '', text)

        # 第三步：移除指定的单个控制字符
        # 要过滤的特定控制字符列表
        control_chars_to_remove = [
            '\u0001',  # Start of Header  对应原始文档内容是图片
            '\u0005',  # Enquiry          对应原始文档内容是标注
            '\u0007',  # Bell             表格中有一个单元格，解析结果内容就会有两个\u0007 
            '\u000b',  # Vertical Tab
            '\u0013',  # Device Control Three (字段开始)
            '\u0014',  # Device Control Four (字段分隔)
            '\u0015',  # Negative Acknowledge (字段结束)
            '\u001E',  #日期/时间分隔符
            '\u001F',  #可选连字符
            '\u0009',  #Tab制表符
            '\u00A0',  #不间断空格
            '\u2002',  #En空格
            '\u2003',  #Em空格
            '\u0002',  #自动编号占位符
            '\u0008'   #退格
        ]

        for char in control_chars_to_remove:
            text = text.replace(char, '')

        return text


    def is_valid_parser_result(self,text:str) -> bool:
        '''
        判断解析结果是否有效 无效返回False 有效返回True
        (以下是当前搜索解析库的验证情况)无效场景：
        1.加密文档
            doc,wps,ppt 解析结果是一个空格
        2.损坏文档
            doc,wps,ppt 解析结果是一个空格
        3.空文档
            doc,wps 解析结果是一个换行符'\r'和一个空格 (换行符是因为doc和wps自带的)
            ppt 解析结果是一个空格
        4.文档中只有图片
            doc,wps,ppt 解析结果是一个空格
        '''
        if not text:
            return False

        # 移除所有空白字符（包括空格、换行符、制表符等）后，如果还有内容，说明有效
        return bool(text.strip()) # 空字符为False 非空字符为True

    def parser(self, file_path) -> str:
        """
        解析 .doc, .wps, .ppt 文件
        """
        if not self.module_available:
            logger.error(f"Error: Cannot parse the file because 'libparser_extension' is not available.")
            return None

        import os
        parser_support_format = ['doc', 'dot', 'wps', 'ppt', 'pps', 'dps', 'et', 'xls']
        try:
            # 由于office解析库不支持修改扩展名后的文件解析，先校验文件的扩展名是否是解析库支持的扩展名类型
            extension = os.path.splitext(file_path)[1][1:]
            # 对解析库不支持的扩展名，不进行解析
            if extension not in parser_support_format:
                return None

            result = self.object.parserOffice(file_path)
            # 将所有的\r替换成\n
            result = result.replace('\r', '\n')
            result = self.filter_special_characters(result)
            if not self.is_valid_parser_result(result):
                logger.warning(f"File is encrypted/corrupted/empty,file_path:{file_path}")
                return None
            return result

        except Exception as e:
            logger.error(f"Error during parsing: {e}")
            raise

    def is_paragraph_recognition_supported(self,file_type:str) -> bool:
        '''
        wps和doc
        file_type: 文件类型
        return: 是否支持段落识别
        '''
        if file_type in ['wps', 'doc']:
            logger.info(f"File type:{file_type} is support semantic chunking")
            return True
        else:
            logger.warning(f"File type:{file_type} is not support semantic chunking")
            return False

if __name__ == "__main__":
    parser = OfficeParser()
    result = parser.parser('/home/kylin/桌面/文档处理服务/文档解析服务测试文档/公司报销制度及流程.wps')
    print(result)