# -*- coding: utf-8 -*-

from officeparser import OfficeParser
from docx import Document

class DocxParser(OfficeParser):
    def __init__(self, file_path):
        self.doc = Document(file_path)

    def contents(self):
        content_list = []
        for paragraph in self.doc.paragraphs:
            content_list.append(paragraph.text)
        return content_list

    def title(self):
        return self.doc.core_properties.title

    def author(self):
        return self.doc.core_properties.author

    def pages(self):
        return -1

# 使用示例
# if __name__ == "__main__":
#     file_path = "/home/wangweinan/test-search/申请文件对照页.docx"
#     parser = DocxParser(file_path)
#     content = parser.content()
#     # num_pages = parser.num_pages()
#     title = parser.title()
#     author = parser.author()

#     print("Content:")
#     for page_content in content:
#         print(page_content)
#     print("Title:", title)
#     print("Author:", author)
