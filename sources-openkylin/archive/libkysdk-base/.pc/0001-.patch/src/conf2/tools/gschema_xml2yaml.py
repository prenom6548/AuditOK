#!/usr/bin/python3
# 
#  libkysdk-system's Library
#  
#  Copyright (C) 2023, KylinSoft Co., Ltd.
# 
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public
#  License as published by the Free Software Foundation; either
#  version 3 of the License, or (at your option) any later version.
# 
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  Lesser General Public License for more details.
# 
#  You should have received a copy of the GNU General Public License
#  along with this library.  If not, see <https://www.gnu.org/licenses/>.
# 
#  Authors: tianshaoshuai <tianshaoshuai@kylinos.cn>
# 
# 

import os
import re
import sys
import xml.etree.ElementTree as ET
import yaml

def convert_one_file(data:dict, path:str, file:str):

    global organization

    # 解析XML文件到树中
    if os.path.exists(f'{path}/{file}'):

        tree = ET.parse(f'{path}/{file}')
        root = tree.getroot()

        #处理枚举
        for enum in root.findall('.//enum'):
            list = []
            id = enum.attrib.get('id')
            group_list = id[len(organization) + 1:].split('.')
            if group_list[0] not in data:
                data[group_list[0]] = {version:{}}
            tmp = data[group_list[0]][version]
            for value in enum:
                item = {}
                item['_nick'] = value.get('nick')
                item['_value'] = value.get('value')
                list.append(item)
            tmp[f'enum_{id}'] = list

        #处理schema下的key， schema不管
        schema_list = root.findall('.//schema')

        for schema in schema_list:
            id = schema.attrib.get('id')
            group_list = id[len(organization) + 1:].split('.')
            if group_list[0] not in data:
                data[group_list[0]] = {version:{}}
            tmp = data[group_list[0]][version]
            for group in group_list[1:]:
                if group not in tmp:
                    tmp[group] = {}
                tmp = tmp[group]

            for key in schema.findall('.//key'):
                item = {}
                key_name = key.attrib.get('name')
                key_type = key.attrib.get('type')
                #type 不存在 就是枚举类型
                if key_type is None:
                    enumid = key.attrib.get('enum')
                    item['_range'] = f'@enum_{enumid}'
                    item['_type'] = 'enum'
                else:
                    item['_type'] = key_type
                for child in key:
                    tag = child.tag
                    text = '' if child.text is None else child.text
                    text = text.strip()
                    text = text.strip('\'') #部分xml空字符串默认值会写''
                    text = text.strip('\"') #部分xml空字符串默认值会写""
                    text = text.replace('\n\t','') #删除原xml中使下一行缩进而使用的制表符
                    if tag == 'range':
                        text = f'{child.attrib.get("min")},{child.attrib.get("max")}'
                    item[f'_{tag}'] = text
                tmp[key_name] = item
    else:
        print(f'{path}/{file} not exists')

if __name__ == '__main__': 
    app = sys.argv[1]
    version = '2.0.0-0k0.0'
    if 3 == len(sys.argv):
        version = sys.argv[2]
    path = '/usr/share/glib-2.0/schemas'

    if len(sys.argv) > 3:
        path = sys.argv[3]

    organization = app.rsplit('.', 1)[0]

    result = {}

    # 读取app的所有配置文件
    file_list = []
    pattren = re.compile(r'^{}.*\.gschema\.xml$'.format(re.escape(app)))
    for root, dirs, files in os.walk(path):
        for file in files:
            if pattren.match(file):
                file_list.append(file)

    # 按照每个元素分割后的字符串个数对列表进行降序排序
    file_list = sorted(file_list, key=lambda x: len(x.split('.')), reverse=False)
    for file in file_list:
        convert_one_file(result, path, file)
    
    # 生成yaml文件
    yaml_data = yaml.safe_dump(result, allow_unicode = True)

    # 将YAML数据写入文件
    home = os.getenv('HOME')
    file = f"{home}/yaml/{app}.yaml"
    if not os.path.exists(os.path.dirname(file)):
        os.makedirs(os.path.dirname(file), exist_ok=True)

    with open(file, 'w') as yaml_file:
        yaml_file.write(yaml_data)
