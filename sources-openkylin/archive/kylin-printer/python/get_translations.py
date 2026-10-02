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
destDir = os.path.abspath(workspaceDir + '/../../language-packs/ukui/kylin-printer/')


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
        move_file(old_file, new_file)

        with open(new_file, 'r') as file:
            lines = file.readlines()
        new_line = lines[2]
        new_line = replace_whole_word(new_line, old_locale, new_locale)
        lines[2] = new_line

        with open(new_file, 'w') as file:
            file.writelines(lines)

def export_trans():
    for root, dirs, files in os.walk(workspaceDir):
        for file in files:
            file_extension = os.path.splitext(file)[-1][1:]
            if file_extension == 'ts' :
                outputDir = destDir
                if 'dict' in file:
                    outputDir += '/dict'

                copy_file(root + '/' + file, outputDir + '/' + file)
                file_language_conversion(outputDir + '/' + file, 'zh_HK', 'zh_Hant')
                file_language_conversion(outputDir + '/' + file, 'kk_KZ', 'kk')
                file_language_conversion(outputDir + '/' + file, 'ug_CN', 'ug')
                file_language_conversion(outputDir + '/' + file, 'ky_KG', 'ky')

def import_trans():
    for root, dirs, files in os.walk(destDir):
        for file in files:
            file_extension = os.path.splitext(file)[-1][1:]
            if file_extension == 'ts' :
                outputDir = workspaceDir

                copy_file(root + '/' + file, outputDir + '/' + file)
                file_language_conversion(outputDir + '/' + file, 'zh_Hant', 'zh_HK')
                file_language_conversion(outputDir + '/' + file, 'kk',      'kk_KZ')
                file_language_conversion(outputDir + '/' + file, 'ky',      'ky_KG')
                file_language_conversion(outputDir + '/' + file, 'ug',      'ug_CN')



if len(sys.argv) != 2:
    print("使用方法: python3 get_translations.py import/export")
    sys.exit(1)

param = sys.argv[1]
if param != 'import' and param != 'export':
    print("使用方法: python3 get_translations.py import/export")
    sys.exit(1)
if param == 'import':
    import_trans()
if param == 'export':
    export_trans()
