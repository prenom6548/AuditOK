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

from bs4 import BeautifulSoup
from .base_parser import BaseParser
from .utils import detect_and_decode_file

class HTMLParser(BaseParser):

    def parser(self, file_path) -> str:
        """
        :param file_path: 文件路径
        :return: 按行保存的文本内容列表
        """
        html_content = detect_and_decode_file(file_path)
        soup = BeautifulSoup(html_content, 'html.parser')
        text = soup.get_text(separator=' ')
        text = ''.join(text.split())
        return text


    def is_paragraph_recognition_supported(self,file_type:str) -> bool:
        '''
        todo 待测试是否支持段落识别，目前先返回false
        '''
        return False

if __name__ == '__main__':
    file_path = '/home/kylin/文档/test.html'
    html_parser = HTMLParser()
    html_content = html_parser.parser(file_path)
    # print(html_parser.parser_tag(file_path, 'keys'))
    print(html_content)