#   Copyright 2024 KylinSoft Co., Ltd.
#
#   This program is free software: you can redistribute it and/or modify it under
#   the terms of the GNU General Public License as published by the Free Software
#   Foundation, either version 3 of the License, or (at your option) any later
#   version.
#
#   This program is distributed in the hope that it will be useful, but WITHOUT
#   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
#   FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License along with
#   this program. If not, see <https://www.gnu.org/licenses/>.

from transformers import AutoTokenizer

import os
os.environ["TOKENIZERS_PARALLELISM"] = "false"
tokenizer = None

def init_tokenizer(tokenizer_file_path):
    global tokenizer

    tokenizer = AutoTokenizer.from_pretrained(tokenizer_file_path)

def tokenize_text(input_text):
    global tokenizer

    if tokenizer is None:
        print("Don't init tokenizer, please run init_tokenizer befor")
        return {}

    # 将文本转换为 tokens
    tokens = tokenizer(input_text)
    # print(tokens)

    input_ids = tokens['input_ids']
    attention_mask = tokens['attention_mask']

    # 合并 input_ids 和 attention_mask
    merged_dict = {'input_ids': input_ids, 'attention_mask': attention_mask}

    return merged_dict

# # 调用函数，传入输入文本和 tokenizer 文件路径
# input_texts = { "杰尼龟", "皮卡丘", "小火龙", "妙蛙种子" }
# tokenizer_file_path = "/home/wangweinan/models/bge-m3-onnx/"

# init_tokenizer(tokenizer_file_path)
# for text in input_texts:
#     tokens = tokenize_text(text)
#     print (tokens)
