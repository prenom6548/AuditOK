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

import re
from .base_chunker import BaseChunker

class TextChunker(BaseChunker):
    def split_sentence_into_chunks(self,sentence:str, max_chunk_size:int = 100) -> list:
        '''
        按照句子切分，保证一定的语义
        1.按照标点符号切分成多个句子
        2.拼接后的句子长度不超过max_chunk_size
        3.但会出现最后一个句子是一个超长句子的情况，split_sentence_length函数进行第二次切分，就是用来处理这种情况
        '''
        punctuation = r'.+?[.?!,;，。！？：]+'

        chunks = []
        current_chunk = ""
        for part in re.split(f"({punctuation})", sentence):
            part = part.strip()
            if not part:
                continue
            if len(current_chunk) + len(part) <= max_chunk_size:
                current_chunk += part
            else:
                if current_chunk:
                    chunks.append(current_chunk.strip())
                current_chunk = part
        if current_chunk:
            chunks.append(current_chunk.strip())

        return chunks

    def split_sentence_length(self, sentence:list, max_chunk_size:int = 100) -> list:
        '''
        按照长度切分，保证每个chunk的长度不超过max_chunk_size
        这里强行按长度切分，防止句子切分时，存在超长句子
        '''
        sentence_list = []
        for sen in sentence:
            sentence_list.extend([sen[i:i+max_chunk_size] for i in range(0, len(sen), max_chunk_size)])
        return sentence_list

    def _formatted_chunks(self, chunks: list, file_path: str) -> list:
        """
        输出格式：
        - 和语义切分保持一致
        需要注意点是 普通切分的title_path和metadata为空
            - title_path 为空
            - content为分块后的内容
            - file_path为文件路径
            - metadata 为空

        Args:
            chunks: 普通切分后的内容
            file_path: 文件路径

        Returns:
            切分后的 chunk 列表，格式与语义切分一致
        """
        import json

        # 格式化输出结果和语义切分一致
        formatted_chunks = []
        for i, chunk_content in enumerate(chunks, start=1):
            if not chunk_content or not chunk_content.strip():
                continue

            chunk = {
                "title_path": [],
                "content": chunk_content.strip(),
                "file_path": file_path,
                "metadata": {}
            }
            chunk_str = json.dumps(chunk, ensure_ascii=False)
            formatted_chunks.append(chunk_str)

        return formatted_chunks

    def segment_tokenize(self, texts:str, max_length:int,file_path:str) -> list:
        sentence = self.split_sentence_into_chunks(sentence=texts,max_chunk_size=max_length)
        sentence_list = self.split_sentence_length(sentence=sentence,max_chunk_size=max_length)
        return self._formatted_chunks(sentence_list,file_path)

if __name__ == "__main__":
    import os
    from parser_factory import ParserFactory
    filepath = "/home/kylin/桌面/文档处理服务/文档解析服务测试文档/各类文档本质/ofd/正常段落的文档.ofd"
    file_extension = os.path.splitext(filepath)[1]
    file_format = file_extension[1:]
    parser = ParserFactory.create_parser(file_format)
    parser_content = parser.parser(filepath)
    # print(f"解析结果：{parser_content} ")

    chunker = TextChunker()
    chunks = chunker.segment_tokenize(parser_content,200,filepath)
    print(f"切分结果: {chunks}")