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

class BaseParser:
    def parser(self, file_path) -> str:
        """
        虚函数，用于解析指定路径的文件。

        :return: 解析后的字符串结果
        """
        raise NotImplementedError("Subclasses should implement this method.")

    def is_paragraph_recognition_supported(self,file_type:str) -> bool:
        """
        是否支持段落识别 解析子类需要根据解析器的特点重写该函数
        file_type: 文件类型 有的解析器支持很多类型 但只有部分类型可以提取出来段落信息 用这个参数来判断
        return: 是否支持段落识别
        """
        raise NotImplementedError("Subclasses should implement this method.")