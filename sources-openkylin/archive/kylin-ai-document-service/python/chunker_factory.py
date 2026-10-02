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

from chunker.base_chunker import BaseChunker
from chunker.textchunker import TextChunker
from chunker.semanticchunker import SemanticChunker

class ChunkerFactory:
    def __init__(self) -> None:
        pass

    @staticmethod
    def create_chunker(format_type: str) -> BaseChunker:
        if format_type == "TextChunker":
            return TextChunker()
        if format_type == "SemanticChunker":
            return SemanticChunker()

if __name__ == "__main__":
    #文档解析
    import os
    from parser_factory import ParserFactory
    filepath = r"/home/kylin/文档/操作系统大会演示素材v0.2/智能数据管理素材/文档素材/公司报销制度及流程.docx"
    file_extension = os.path.splitext(filepath)[1]
    file_format = file_extension[1:]
    parser = ParserFactory.create_parser(file_format)
    parser_content = parser.parser(filepath)
    md_string = str(parser_content)
    # print("====================== 解析结果如下：==============================\n",md_string)
    
    # 普通切分
    chunker = ChunkerFactory.create_chunker("TextChunker")
    chunks = chunker.segment_tokenize(md_string, 512)
    print("====================== 普通切分结果如下：==============================\n",chunks)

    # 语义切分
    # chunker = ChunkerFactory.create_chunker("SemanticChunker")
    # chunks = chunker.build_rag_chunks_from_string_by_paragraph(md_string, file_name="demo.md")
    # print("====================== 语义切分结果如下：==============================\n",chunks)