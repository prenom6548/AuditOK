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

from charset_normalizer import detect

# 常见编码列表（参考 https://en.wikipedia.org/wiki/Popularity_of_text_encodings）
COMMON_ENCODINGS = [
    "utf_8",
    "iso_8859_1",
    "iso_8859_6",
    "iso_8859_8",
    "ascii",
    "big5",
    "utf_16",
    "utf_16_be",
    "utf_16_le",
    "utf_32",
    "utf_32_be",
    "utf_32_le",
    "euc_jis_2004",
    "euc_jisx0213",
    "euc_jp",
    "euc_kr",
    "gb18030",
    "shift_jis",
    "shift_jis_2004",
    "shift_jisx0213",
]

def detect_and_decode_file(file_path: str, confidence_threshold: float = 0.8) -> str:
    """
    检测文件编码格式并根据编码格式进行解码，返回字符串文本

    当编码检测失败或置信度不足时，会尝试遍历常见编码列表进行解码。

    参数:
        file_path: 文件路径
        confidence_threshold: 编码检测置信度阈值，低于此值则尝试常见编码列表（默认0.8）

    返回:
        解码后的字符串文本

    """
    with open(file_path, 'rb') as file:
        raw_data = file.read()  # 以二进制方式读取文件，先判断文档的编码格式，再解码为字符串

    charset_result = detect(raw_data)
    encoding = charset_result.get('encoding')
    confidence = charset_result.get('confidence')

    # 如果编码检测失败或置信度不足，尝试常见编码列表
    if encoding is None or confidence is None or confidence < confidence_threshold:
        # 编码检测失败，回退到预定义的常见编码列表
        for enc in COMMON_ENCODINGS:
            try:
                content = raw_data.decode(enc)
                return content
            except (UnicodeDecodeError, UnicodeError, LookupError):
                continue

        # 所有编码尝试都失败
        raise ValueError(
            f"Unable to determine file encoding after trying all common encodings. "
            f"File may be corrupted or in an unsupported format."
        )
    else:
        # 使用检测到的编码进行解码
        content = raw_data.decode(encoding)
        return content

