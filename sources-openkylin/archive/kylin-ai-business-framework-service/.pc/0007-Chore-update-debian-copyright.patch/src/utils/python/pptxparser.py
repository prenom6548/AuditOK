# This Python file uses the following encoding: utf-8

from officeparser import OfficeParser
from pptx import Presentation

class PptxParser(OfficeParser):
    def __init__(self, file_path):
        self.presentation = Presentation(file_path)

    def author(self):
        return self.presentation.core_properties.author

    def pages(self):
        return len(self.presentation.slides)

    def title(self):
            return self.presentation.core_properties.title

    def contents(self):
        contents = []
        for slide in self.presentation.slides:
            content = []
            for shape in slide.shapes:
                if hasattr(shape, "text"):
                    content.append(shape.text)
            contents.append("\n".join(content))
        return contents

# 用法示例
# file_path = "/home/wangweinan/test-search/可信设计方法.pptx"
# parser = PptxParser(file_path)

# print("Contents:")
# for i, content in enumerate(parser.contents(), start=1):
#     print("Page", i, ":", content)
# print("Author:", parser.author())
# print("Page Count:", parser.page_count())
# print("Title:", parser.title())

