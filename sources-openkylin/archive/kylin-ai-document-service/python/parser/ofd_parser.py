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

class OFDParser(BaseParser):
    def __init__(self) -> None:
        self.module_available = self._module_exists('libparser_extension')

        if self.module_available:
            import libparser_extension
            self.object = libparser_extension.ParserExtension()
        else:
            logger.error("Don't load ofd_parser lib")

    def _module_exists(self, module_name: str) -> bool:
        """
        检查指定的模块是否存在。
        """
        spec = importlib.util.find_spec(module_name)
        return spec is not None

    def is_valid_parser_result(self,text:str) -> bool:
        '''
        判断解析结果是否有效 无效返回False 有效返回True
        (以下是当前搜索解析库的验证情况)无效场景：
        1.加密文档
            ofd 解析结果是一个空格
        3.空文档
            ofd 解析结果是一个空格
        4.文档中只有图片
            ofd 解析结果是一个空格
        '''
        if not text:
            return False

        # 移除所有空白字符（包括空格、换行符、制表符等）后，如果还有内容，说明有效
        return bool(text.strip()) # 空字符为False 非空字符为True

    def parser(self, file_path) -> str:
        """
        解析 ofd文件
        """
        if not self.module_available:
            logger.error("Parser ofd error, because of don't load ofd_parser lib")
            return None

        import os
        try:
            # 搜索解析ofd 不支持修改扩展名后的解析
            extension = os.path.splitext(file_path)[1][1:]
            # 对解析库不支持的扩展名，不进行解析
            if extension != 'ofd':
                return None

            result = self.object.parserOffice(file_path)
            if not self.is_valid_parser_result(result):
                logger.warning(f"File is encrypted/corrupted/empty,file_path:{file_path}")
                return None

            return result
        except Exception as e:
            logger.error(f"Error during parsing ofd: {e}")
            raise

    def is_paragraph_recognition_supported(self,file_type:str) -> bool:
        '''
        ofd解析器不支持语义切分
        '''
        return False

if __name__ == "__main__":
    parser = OFDParser()
    result = parser.parser('/home/kylin/桌面/文档处理服务/文档解析服务测试文档/各类文档本质/ofd/存在段落-表格-图片-页眉-页脚.ofd')
    print(result)