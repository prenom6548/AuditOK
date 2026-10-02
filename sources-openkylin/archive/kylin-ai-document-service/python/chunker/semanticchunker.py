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

import json
import re
import numpy as np
from .base_chunker import BaseChunker
from log_config import get_logger
logger = get_logger()

class SemanticChunker(BaseChunker):
    def __init__(self):
        from .embedder import embedding_instance
        self.client_ = embedding_instance

    def split_chinese_paragraph(self,paragraph: str, max_sent_len: int = 500) -> list:
        '''
        按照句末标点符号将段落分割成句子，当句子大于max_sent_len时，再按照，、,.符号进一步分割
        paragraph: 段落文本
        max_sent_len: 句子最大长度
        '''
        # pattern = r'(.*?[；。！？])'
        # rough_sentences = re.findall(pattern, paragraph)
        rough_sentences = self.split_sentences(paragraph)
        sentences = []
        for s in rough_sentences:
            s = s.strip()
            if len(s) <= max_sent_len:
                sentences.append(s)
            else:
                subs = re.split(r'[，、,.]', s)
                temp = ""
                for part in subs:
                    if len(temp) + len(part) <= max_sent_len:
                        temp += part + "，"
                    else:
                        sentences.append(temp.strip("，"))
                        temp = part + "，"
                if temp:
                    sentences.append(temp.strip("，"))
        return [s for s in sentences if s]

    def split_sentences(self,paragraph):
        # 处理引号内的标点不分割（如：他说："你好！" 不会被分割）
        pattern = r'''
            (?:
                # 处理引号内的内容，保持完整性
                [‘“](?:\\\\.|[^‘”])*?[’”][；。！？]?
                |
                # 普通文本内容，允许非贪婪匹配任意字符直到句末标点
                .*?[；。！？](?!['"]?[^'"<>]*>['"]?)
            )
        '''
        # 预编译正则表达式提高效率
        regex = re.compile(pattern, re.DOTALL | re.VERBOSE)

        # 执行匹配
        sentences = regex.findall(paragraph)

        # 处理可能的末尾未完成内容
        last_pos = 0
        for s in sentences:
            last_pos = paragraph.find(s, last_pos) + len(s)

        # 提取末尾未匹配的内容
        remaining = paragraph[last_pos:].strip()
        if remaining:
            sentences.append(remaining)

        # 清理多余空格和空句子
        return [s.strip() for s in sentences if s.strip()]

    def triton_encode_sentences(self, sentences, batch_size=100):
        '''
        将句子列表中的每个句子编码成向量
        sentences: 句子列表
        batch_size: 批量大小
        return: 一个包含所有句子向量的列表
        '''
        all_embeddings = []
        for i in range(0, len(sentences)):
            embedding = self.client_.embed_text(sentences[i])
            if not embedding:
                logger.error(f"句子{i}编码失败: {sentences[i]}")
                continue

            all_embeddings.append(embedding)

        return np.vstack(all_embeddings)

    def _cosine_similarity(self,a: np.ndarray, b: np.ndarray) -> float:
        """手动计算两个向量之间的余弦相似度"""
        dot_product = np.dot(a, b)
        norm_a = np.linalg.norm(a)
        norm_b = np.linalg.norm(b)

        # 处理零向量情况
        if norm_a == 0 or norm_b == 0:
            return 0.0

        return dot_product / (norm_a * norm_b)

    def _split_by_similarity(self, sentences: list, embeddings: np.ndarray,
                             similarities: list, max_chunk_len: int,
                             sim_threshold: float) -> list:
        """
        基于语义相似度将句子切分成 chunks

        切分规则：
        1. 如果相邻句子相似度 < sim_threshold：在两句之间切分
        2. 如果合并后长度 > max_chunk_len：在两句之间切分
        3. 否则：将句子合并到当前 chunk

        Args:
            sentences: 句子列表
            embeddings: 句子向量列表
            similarities: 相邻句子相似度列表（长度为 len(sentences) - 1）
            max_chunk_len: 最大 chunk 长度
            sim_threshold: 相似度阈值

        Returns:
            切分后的 chunk 列表（字符串列表）
        """
        chunks = []
        current_chunk = [sentences[0]]  # 从第一个句子开始
        current_chunk_len = len(sentences[0])

        # 遍历每个相似度，决定是否切分
        for i, sim in enumerate(similarities):
            next_sentence = sentences[i + 1]
            next_sentence_len = len(next_sentence)

            # 判断是否需要切分
            should_split = (sim < sim_threshold) or (current_chunk_len + next_sentence_len > max_chunk_len)

            if should_split:
                # 切分：保存当前 chunk，开始新的 chunk
                chunks.append("".join(current_chunk))
                current_chunk = []
                current_chunk_len = 0

            # 将下一个句子添加到当前 chunk（无论是否切分）
            current_chunk.append(next_sentence)
            current_chunk_len += next_sentence_len

        # 处理最后一个 chunk（循环结束后剩余的 chunk）
        if current_chunk:
            chunks.append("".join(current_chunk))

        return chunks

    def _merge_small_chunks(self, chunks: list, min_chunk_len: int) -> list:
        """
        合并过小的 chunks，确保所有 chunk 都 >= min_chunk_len

        Args:
            chunks: 待合并的 chunk 列表
            min_chunk_len: 最小 chunk 长度
 
        Returns:
            合并后的 chunk 列表
        """
        if not chunks:
            return []

        merged_chunks = []
        buffer = ""

        for chunk in chunks:
            if not buffer:
                # 第一个 chunk，直接作为 buffer
                buffer = chunk
            elif len(buffer) + len(chunk) < min_chunk_len:
                # 合并后仍然小于最小长度，继续合并
                buffer += chunk
            else:
                # buffer 已经足够大，保存并开始新的 buffer
                merged_chunks.append(buffer)
                buffer = chunk

        # 处理最后一个 buffer
        if buffer:
            merged_chunks.append(buffer)

        return merged_chunks

    def semantic_split(self, content: str, title_path: list, file_path: str,
                      max_sent_len: int = 500, max_chunk_len: int = 800,
                      min_chunk_len: int = 200, sim_threshold: float = 0.97,
                      level: int = 2) -> list:
        """
        对内容进行语义切分

        核心流程：
        1. 将内容分割成句子
        2. 对每个句子进行向量化
        3. 计算相邻句子的语义相似度
        4. 基于相似度切分（相似度低或长度超限时切分）
        5. 合并过小的 chunks（确保所有 chunk >= min_chunk_len）
        6. 返回格式化的 chunk 列表

        Args:
            content: 待切分的文本内容
            title_path: 标题路径（用于标识 chunk 的层级位置）
            file_path: 文件路径
            max_sent_len: 句子最大长度（超过此长度会进一步切分）
            max_chunk_len: 最大 chunk 长度（超过此长度会切分）
            min_chunk_len: 最小 chunk 长度（小于此长度的 chunk 会被合并）
            sim_threshold: 语义相似度阈值（低于此值会切分，默认 0.97）
            level: 当前层级（用于 metadata）

        Returns:
            切分后的 chunk 列表，每个 chunk 包含：
            - title_path: 标题路径（会添加"语义子段 X"）
            - content: chunk 内容
            - file_path: 文件路径
            - metadata: 元数据（包含 level）
        """
        # 步骤1：将内容分割成句子
        sentences = self.split_chinese_paragraph(content, max_sent_len=max_sent_len)

        # 如果只有一个或没有句子，直接返回
        if len(sentences) <= 1:
            return [{
                "title_path": title_path,
                "content": content.strip(),
                "file_path": file_path,
                "metadata": {
                    "level": level - 1  # fallback to previous level
                }
            }]

        # 步骤2：对每个句子进行向量化
        embeddings = self.triton_encode_sentences(sentences)

        # 步骤3：计算相邻句子之间的余弦相似度
        # 注意：相似度列表长度 = 句子列表长度 - 1
        similarities = [
            self._cosine_similarity(embeddings[i], embeddings[i + 1])
            for i in range(len(embeddings) - 1)
        ]

        # 步骤4：基于相似度切分句子
        chunks = self._split_by_similarity(
            sentences=sentences,
            embeddings=embeddings,
            similarities=similarities,
            max_chunk_len=max_chunk_len,
            sim_threshold=sim_threshold
        )

        # 步骤5：合并过小的 chunks
        merged_chunks = self._merge_small_chunks(chunks, min_chunk_len)

        # 步骤6：格式化返回结果
        # 注意：真正进入到了语义切分，title_path 层级才会有"语义子段"
        return [{
            "title_path": title_path + [f"语义子段 {i + 1}"],
            "content": chunk.strip(),
            "file_path": file_path,
            "metadata": {
                "level": level
            }
        } for i, chunk in enumerate(merged_chunks)]


    def split_content_if_needed(self, content: str, title_path: list, file_path: str,
                                max_chunk_len: int = 800, min_chunk_len: int = 200,
                                max_sent_len: int = 500, sim_threshold: float = 0.97,
                                level: int = 1) -> list:
        """
        判断内容是否需要切分，如果需要则进行语义切分

        核心逻辑：
        1. 如果内容长度 <= max_chunk_len：直接返回，不需要切分
        2. 如果内容长度 > max_chunk_len：调用 semantic_split 进行语义切分

        Args:
            content: 待处理的内容
            title_path: 标题路径（用于标识 chunk 的层级位置）
            file_path: 文件路径
            max_chunk_len: 最大 chunk 长度，超过此长度需要切分
            min_chunk_len: 最小 chunk 长度
            max_sent_len: 句子最大长度
            sim_threshold: 语义相似度阈值
            level: 当前层级（切分后 level + 1）

        Returns:
            切分后的 chunk 列表，每个 chunk 包含 title_path、content、file_path 和 metadata
        """
        content_stripped = content.strip()

        # 如果内容长度不超过最大长度，直接返回，不需要切分
        if len(content_stripped) <= max_chunk_len:
            # 缓冲区+下一个段落大于max_chunk_len 但是缓冲区小于max_chunk_len
            return [{
                "title_path": title_path,
                "content": content_stripped,
                "file_path": file_path,
                "metadata": {
                    "level": level
                }
            }]

        # 当前缓冲区已经超过了max_chunk_len 进行切分
        return self.semantic_split(
            content=content,
            title_path=title_path,
            file_path=file_path,
            max_sent_len=max_sent_len,
            max_chunk_len=max_chunk_len,
            min_chunk_len=min_chunk_len,
            sim_threshold=sim_threshold,
            level=level + 1
        )

    def _should_merge_paragraph(self, buffer: str, para: str, max_chunk_len: int, min_chunk_len: int) -> bool:
        """
        判断是否应该将新段落合并到当前 buffer 中

        合并条件：
        1. 合并后长度 <= max_chunk_len：可以合并
        2. buffer 长度 < min_chunk_len：即使合并后会超过 max_chunk_len，也要合并（避免产生过小的 chunk）

        Args:
            buffer: 当前缓冲区内容
            para: 新段落内容
            max_chunk_len: 最大 chunk 长度
            min_chunk_len: 最小 chunk 长度

        Returns:
            True 表示应该合并，False 表示不应该合并
        """
        tentative_length = len(buffer + "\n" + para)

        # 条件1：合并后不超过最大长度，可以合并
        if tentative_length <= max_chunk_len:
            return True

        # 条件2：当前 buffer 太小，即使合并后会超过最大长度，也要先合并（后续会切分）
        if len(buffer) < min_chunk_len:
            return True

        # 其他情况：不合并，需要先处理当前 buffer
        return False

    def _process_buffer(self, buffer: str, paragraph_idx: int, file_path: str,
                       max_chunk_len: int, min_chunk_len: int, max_sent_len: int,
                       sim_threshold: float) -> list:
        """
        处理当前 buffer 中的内容，进行切分并返回 chunks

        Args:
            buffer: 待处理的内容
            paragraph_idx: 段落索引
            file_path: 文件路径
            max_chunk_len: 最大 chunk 长度
            min_chunk_len: 最小 chunk 长度
            max_sent_len: 句子最大长度
            sim_threshold: 相似度阈值

        Returns:
            切分后的 chunk 列表
        """
        title_path = [f"段落 {paragraph_idx}"]
        return self.split_content_if_needed(
            content=buffer,
            title_path=title_path,
            file_path=file_path,
            max_chunk_len=max_chunk_len,
            min_chunk_len=min_chunk_len,
            max_sent_len=max_sent_len,
            sim_threshold=sim_threshold,
            level=1
        )

    def build_rag_chunks_from_string_by_paragraph(self,
        md_string: str,
        file_path: str,
        max_chunk_len: int = 800,
        min_chunk_len: int = 200,
        max_sent_len: int = 500,
        sim_threshold: float = 0.97
    ) -> list:
        """
        语义切分核心逻辑：
        1. 按换行符分割段落
        2. 遍历段落，尝试合并到 buffer
        3. 合并规则：
           - 合并后 <= max_chunk_len：合并
           - buffer < min_chunk_len：合并（避免过小的 chunk）
           - 否则：处理当前 buffer，新段落作为新 buffer
        4. 处理最后一个 buffer

        注意：语义切分强依赖段落，解析后的内容没有换行符'\n'时，要谨慎使用

        Args:
            md_string: 待切分的文本内容
            file_path: 文件路径
            max_chunk_len: 最大 chunk 长度
            min_chunk_len: 最小 chunk 长度
            max_sent_len: 句子最大长度
            sim_threshold: 语义相似度阈值

        Returns:
            切分后的 chunk 列表，每个 chunk 包含 title_path、content、file_path 和 metadata（这个字段还没有用到）
        """
        # 按换行符分割段落，过滤空段落
        raw_paragraphs = [p.strip() for p in re.split(r'\n+', md_string) if p.strip()]
        logger.info(f"Start split chunks from string by paragraph, raw_paragraphs length: {len(raw_paragraphs)}")

        if not raw_paragraphs:
            return []

        all_chunks = []
        paragraph_idx = 1
        buffer = ""

        # 遍历每个段落，尝试合并到 buffer
        for para in raw_paragraphs:
            if not buffer:
                # 第一个段落，直接作为 buffer
                buffer = para
            else:
                # 判断是否应该合并新段落到当前 buffer
                if self._should_merge_paragraph(buffer, para, max_chunk_len, min_chunk_len):
                    # 合并段落
                    buffer = buffer + "\n" + para
                else:
                    # 不合并：先处理当前 buffer，然后新段落作为新 buffer
                    chunks = self._process_buffer(
                        buffer=buffer,
                        paragraph_idx=paragraph_idx,
                        file_path=file_path,
                        max_chunk_len=max_chunk_len,
                        min_chunk_len=min_chunk_len,
                        max_sent_len=max_sent_len,
                        sim_threshold=sim_threshold
                    )
                    all_chunks.extend(chunks)
                    paragraph_idx += 1
                    buffer = para

        # 处理最后一个 buffer（循环结束后剩余的 buffer）
        if buffer:
            chunks = self._process_buffer(
                buffer=buffer,
                paragraph_idx=paragraph_idx,
                file_path=file_path,
                max_chunk_len=max_chunk_len,
                min_chunk_len=min_chunk_len,
                max_sent_len=max_sent_len,
                sim_threshold=sim_threshold
            )
            all_chunks.extend(chunks)

        return all_chunks

    def _post_process_chunks(self,chunks: list) -> list:
        '''
        将chunks中的每个chunk包装成字符串 因为通信协议中chunks要是一个字符串列表
        chunks: 包含chunk的列表
        wrapper_str: 包装字符串
        return: 包装后的字符串列表
        '''
        wrapped_chunks = []
        for chunk in chunks:
            # 为确保分块结果为字符串列表，所以对chunk中每一个对象包装字符串
            chunk_str = json.dumps(chunk, ensure_ascii=False)
            wrapped_chunks.append(chunk_str)
        return wrapped_chunks

    def segment_tokenize(self, texts: str, max_length: int, file_path: str) -> list:
        """
        语义切分
        Args:
            texts: 待切分的文本内容
            max_length: 每个 chunk 的最大长度
            file_path: 文件路径

        Returns:
        返回字符串列表
        """

        chunks = self.build_rag_chunks_from_string_by_paragraph(
            texts,
            file_path=file_path,
            max_chunk_len=max_length
        )
        return self._post_process_chunks(chunks)

if __name__ == "__main__":
    import os
    from parser_factory import ParserFactory
    filepath = "/home/kylin/桌面/文档处理服务/文档解析服务测试文档/各类文档本质/ofd/加密.ofd"
    file_extension = os.path.splitext(filepath)[1]
    file_format = file_extension[1:]
    parser = ParserFactory.create_parser(file_format)
    parser_content = parser.parser(filepath)
    print(f"解析结果：{parser_content} ")

    chunker = SemanticChunker()
    chunks = chunker.segment_tokenize(parser_content,200,filepath)
    print(f"语义切分结果: {chunks}")
