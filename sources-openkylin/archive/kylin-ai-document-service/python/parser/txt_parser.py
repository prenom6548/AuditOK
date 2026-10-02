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

from .base_parser import BaseParser
from .utils import detect_and_decode_file

class TXTParser(BaseParser):
    def parser(self, file_path) -> str:
        content = detect_and_decode_file(file_path)
        return content

    def is_paragraph_recognition_supported(self,file_type:str) -> bool:
        '''
        txt解析器不支持段落识别
        '''
        return False

if __name__ == '__main__':
    parser = TXTParser()
    print(parser.parser('/home/kylin/下载/生物医学工程学对医学的作用.txt'))