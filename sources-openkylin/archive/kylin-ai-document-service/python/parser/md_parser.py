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

from markdown_it import MarkdownIt
from bs4 import BeautifulSoup
from .base_parser import BaseParser
from .utils import detect_and_decode_file

class MDParser(BaseParser):

    def parser(self, file_path) -> str:
        md_content = detect_and_decode_file(file_path)
        md = MarkdownIt()
        html = md.render(md_content)

        soup = BeautifulSoup(html, 'html.parser')
        text = soup.get_text(separator=' ')
        text = ''.join(text.split())
        return text

    def is_paragraph_recognition_supported(self,file_type:str) -> bool:
        '''
        todo 待测试是否支持段落识别，目前先返回false
        '''
        return False

if __name__ == '__main__':
    md_parser = MDParser()
    html_content = md_parser.parser('/home/kylin/文档/test.md')
    print(html_content)
