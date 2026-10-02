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

import os
import subprocess
import sys
import re

workspaceDir = os.path.abspath(os.path.dirname(__file__) + '/../translations')

def copy_file(source, destination):
    try:
        result = subprocess.run(['cp', source, destination], check=True, text=True, capture_output=True)
        print(f"文件已复制: {source} -> {destination}")
    except subprocess.CalledProcessError as e:
        print(f"错误: {e.stderr}")

def move_file(source, destination):
    try:
        result = subprocess.run(['mv', source, destination], check=True, text=True, capture_output=True)
        print(f"文件已移动: {source} -> {destination}")
    except subprocess.CalledProcessError as e:
        print(f"错误: {e.stderr}")


def replace_whole_word(text, old_word, new_word):
    pattern = r'(?<![a-zA-Z0-9])' + re.escape(old_word) + r'(?![a-zA-Z0-9])'
    return re.sub(pattern, new_word, text)

def is_whole_word_present(text, word):
    pattern = r'(?<![a-zA-Z0-9])' + re.escape(word) + r'(?![a-zA-Z0-9])'
    match = re.search(pattern, text)
    return match is not None


def file_language_conversion(old_file, old_locale, new_locale):
    new_file = old_file
    if is_whole_word_present(new_file, old_locale):
        new_file = replace_whole_word(old_file, old_locale, new_locale)
        copy_file(old_file, new_file)

        with open(new_file, 'r') as file:
            lines = file.readlines()
        new_line = lines[2]
        new_line = replace_whole_word(new_line, old_locale, new_locale)
        lines[2] = new_line

        with open(new_file, 'w') as file:
            file.writelines(lines)

files=[
    'kylin-printer_kk.ts',
    'kylin-printer_ky.ts',
    'kylin-printer_ug.ts',
    'kylin-printer_zh_Hant.ts',
    'kylin-printer_dict_kk.ts',
    'kylin-printer_dict_ky.ts',
    'kylin-printer_dict_ug.ts',
    'kylin-printer_dict_zh_Hant.ts',
]

for file in files:
    file_language_conversion(workspaceDir + '/' + file, 'kk',      'kk_KZ')
    file_language_conversion(workspaceDir + '/' + file, 'ky',      'ky_KG')
    file_language_conversion(workspaceDir + '/' + file, 'ug',      'ug_CN')
    file_language_conversion(workspaceDir + '/' + file, 'zh_Hant', 'zh_HK')