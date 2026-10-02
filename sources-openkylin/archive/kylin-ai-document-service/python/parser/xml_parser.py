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

from bs4 import BeautifulSoup, Comment
from .base_parser import BaseParser
from .utils import detect_and_decode_file

class XMLParser(BaseParser):

    def parser(self, file_path) -> str:
        # 读取文件内容
        content = detect_and_decode_file(file_path)

        soup = BeautifulSoup(content, 'lxml-xml')  # 使用 XML 解析器
        texts = []
        for element in soup.find_all(text=True):
            # 处理普通文本节点
            stripped = element.strip()
            if stripped:
                texts.append(stripped)

        # 合并并清理文本
        clean_text = ' '.join(texts)
        clean_text = ''.join(clean_text.split())  # 去除多余空格
        return clean_text

    def is_paragraph_recognition_supported(self,file_type:str) -> bool:
        '''
        todo 待测试是否支持段落识别，目前先返回false
        '''
        return False

if __name__ == '__main__':
    file_path = '/home/kylin/文档/test.xml'
    html_parser = XMLParser()
    html_content = html_parser.parser(file_path)
    # print(html_parser.parser_tag(file_path, 'keys'))
    print(html_content)