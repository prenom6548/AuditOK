#!/bin/python3

#  Copyright (C) 2020, KylinSoft Co., Ltd.
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <https://www.gnu.org/licenses/>.

import sys
import os
import csv
import xml.sax.saxutils
from string import Template

pwd = os.path.split(os.path.realpath(__file__))[0]

# 指定CSV文件的路径
csv_file_path = sys.argv[1]
cpp_template_file_path = pwd + r'/dictionary_for_chinese_printer.cpp.template'
cpp_file_path          = pwd + r'/../dictionary/dictionary_for_chinese_printer.cpp'
ts_template_file_path  = pwd + r'/kylin-printer_dict_zh_CN.ts.template'
ts_file_path           = pwd + r'/../translations/kylin-printer_dict_zh_CN.ts'


def xmlescape(data):
    return xml.sax.saxutils.escape(data)

def write(template_file_path, out_file_path, data):
    blocks = []
    template_file = open(template_file_path,'r')
    blocks.append(template_file.read())
    template_file.close()

    sstr = "\n"
    tmpl = Template(sstr.join(blocks))

    res = []
    res.append(tmpl.substitute(
                PROPERTY = data))

    file = open(out_file_path,'w')
    file.writelines(res)
    file.close()


cpp_data = ''
ts_data = ''

# 打开CSV文件
with open(csv_file_path, 'r') as file:
    # 创建CSV阅读器对象
    csv_reader = csv.reader(file)

    # 使用next()跳过第一行
    header = next(csv_reader)

    # 遍历CSV文件的每一行（从第二行开始）
    linenum = 37
    for row in csv_reader:
        # 在这里，row是一个包含CSV行数据的列表
        # 您可以根据需要处理每一行的数据

        cpp_data += ('{("%s"),tr("%s"),"%s"},\n' %(row[0], row[0], row[1]))

        ts_data += ('    <message>\n')
        ts_data += ('        <location filename="../dictionary/dictionary_for_chinese_printer.cpp" line="%d"/>\n' % linenum)
        ts_data += ('        <source>%s</source>\n'           % xmlescape(row[0]))
        ts_data += ('        <translation>%s</translation>\n' % xmlescape(row[1]))
        ts_data += ('    </message>\n')
        linenum = linenum + 1

cpp_data = cpp_data.replace('\u00A0', ' ')
ts_data = ts_data.replace('\u00A0', ' ')

write(cpp_template_file_path, cpp_file_path, cpp_data)
write(ts_template_file_path,  ts_file_path,  ts_data)