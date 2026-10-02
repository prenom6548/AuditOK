#!/usr/bin/python3
# -*- coding: utf-8 -*-

### BEGIN LICENSE
 # Copyright (C) 2021 KylinSoft Co., Ltd.
 #
 # Authors:
 #  Yang Min yangmin@kylinos.cn
 #
 # This program is free software; you can redistribute it and/or modify
 # it under the terms of the GNU General Public License as published by
 # the Free Software Foundation, either version 3 of the License, or
 # (at your option) any later version.
 #
 # This program is distributed in the hope that it will be useful,
 # but WITHOUT ANY WARRANTY; without even the implied warranty of
 # MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 # GNU General Public License for more details.
 #
 # You should have received a copy of the GNU General Public License
 # along with this program.  If not, see <http://www.gnu.org/licenses/>.
### END LICENSE

import os
import re
import subprocess
import hashlib
import ast
import xml.etree.ElementTree as ET

from pprint import pprint

from kajsondef import *

class KAConfig():

    __instance = None

    def __init__(self):
        self.key = "kylin"
        # 需要replace进行检索的模块
        self.replaceConfigModuleList = ["memory","graphicsCard","networkCard",
            "hardDisk", "monitor", "soundCard", "keyboard", "mouse", "bluetooth","camera"]
        # 配置即显示的模块
        self.directConfigModuleList = ["battery","motherboard","processor"]
        self.filePath = "/etc/youker_assistant_config.xml";
        if KAConfig.__instance != None:
            raise Exception("This KAConfig is a singleton!")
        else:
            KAConfig.__instance = self
        pass
    
    # Set函数
    def setFilePath(self, filePath):
        self.filePath = filePath

    # Get函数
    def getFilePath(self):
        return self.filePath

    @staticmethod 
    def getInstance():
        if KAConfig.__instance is None:
            KAConfig.__instance = KAConfig()
        return KAConfig.__instance

    # 验证配置文件
    def vertifyConfigFile(self,filePath):
        data = ""
        
        if os.path.exists(filePath):
        # 文件存在，获取xml中的md5标签
            with open(filePath,'r',encoding="utf-8") as f:
                data = f.readlines()
                pattern = re.compile(r'<md5>.*</md5>')
                count = 0
                for i in range(len(data)):
                    if "md5" in data[i]:
                        count += 1
                if count == 1:
                    for i in range(len(data)):
                        if "md5" in data[i]:
                            md5 = re.findall(pattern, data[i])[0].replace("<md5>", "").replace("</md5>", "")

                    # 计算xml文件移除md5标签值后文件的md5值
                    with open(filePath,'r',encoding="utf-8") as f:
                        for i in range(len(data)) :
                            if "md5" in data[i]:
                                data[i] = re.sub(pattern, "<md5></md5>", data[i])
                        data.append(self.key)
                        result = "".join(data)
                        md5hash = hashlib.md5(result.encode('utf-8'))
                        calMd5 = md5hash.hexdigest()

                    if md5 == calMd5:
                        return True
                    else:
                        # print('md5校验失败')
                        return False
                else:
                    # print('xml文件读取失败')
                    return False
        else:
            # print('配置文件未找到')
            return False
        return False

    def formatCheck(self,filePath):
        # 使用xml.etree.ElementTree模块解析xml文件，如果解析成功则说明xml符合格式
        try:
            tree = ET.parse(filePath)
            root = tree.getroot()
            return True
        except ET.ParseError:
            return False

    def visibleCheck(self, filePath, module):
        try:
            tree = ET.parse(filePath)
            root = tree.getroot()
            # Find the Module tag with the specified id
            moduleTag = root.find("./Module[@id='{}']".format(module))
            if moduleTag is None:
                # print('未找到模块')
                return None
            
            # Find the Visible tag
            visibleTag = moduleTag.find("Visible")
            # print('-----------')
            # print(visibleTag)
            if visibleTag is not None:
                try:
                    # 转换true，True，TRUE为ast.literal_eval支持转换的类型
                    visibleTag.text = visibleTag.text.lower() 
                    visibleTag.text = ' '.join(word.capitalize() for word in visibleTag.text.split())
                    visible = ast.literal_eval(visibleTag.text) 
                    # print("visible 输出："+str(visible))
                    # visible = ast.literal_eval(visibleTag.text)
                    return visible
                except ValueError:
                    # print('ValueError')
                    # Handle the case where visibleTag.text is not a valid Python expression
                    return None
            else:
                # print("visibleTag 输出：" + visibleTag.text)
                return None
        except ET.ParseError:
            # print('ET.ParseError')
            return None

    def readConfig(self,filePath,module):
        # Parse the configuration file and find the Module tag with the specified id
        tree = ET.parse(filePath)
        root = tree.getroot()
        moduleTag = root.find("./Module[@id='{}']".format(module))

        # If the Module tag is found, iterate over the Devices and Items tags to create a dictionary list
        # 这里区分模块（其中内存、网卡、主板模块字典值带空格，需要转换一下）
        if module in self.directConfigModuleList:
            # print('-------direct')
            if moduleTag is not None:
                devices = moduleTag.findall("Devices")
                result = []
                for device in devices:
                    items = device.findall("Items")
                    for item in items:
                        itemDict = {}
                        for child in item:
                            if child.text != None:
                                if "motherboard" == module :
                                    itemDict[child.tag.replace("_", " ")] = child.text
                                else:
                                    itemDict[child.tag] = child.text
                        result.append(itemDict)
                return result
            else:
                return None
        elif module in self.replaceConfigModuleList:
            # 解析配置文件Module id为module变量下面，遍历多个Devices，并且Devices下面必须要有replace标签且有值不然返回none，存在replace标签值后，将replace和Devices下面Items且不为空标签返回一个字典列表
            if moduleTag is not None:
                devices = moduleTag.findall("Devices")
                result = []
                for device in devices:
                    replaceTag = device.find("replace")
                    if replaceTag is not None and replaceTag.text is not None:
                        items = device.findall("Items")
                        for item in items:
                            if len(item) > 0:
                                itemDict = {}
                                for child in item:
                                    if child.text != None:
                                        if "networkCard" == module or "soundCard" == module:
                                            itemDict[child.tag.replace("_", " ")] = child.text
                                        else:
                                            itemDict[child.tag] = child.text
                                itemDict["replace"] = replaceTag.text
                                result.append(itemDict)
                if len(result) > 0:
                    return result
                else:
                    return None
            else:
                return None

            pass
        else:
            return None

    def replaceModuleInfo(self,module,configInfo,moduleInfo):
        # 判断moudule是否在directConfigModuleList里面，如果在列表里面则遍历
        if module in self.directConfigModuleList:
             # 判断configInfo字典列表是否为空，如果不为空，则遍历configInfo字典列表，将信息补充或替换到moduleInfo列表字典中
            if configInfo is not None and len(configInfo) > 0:
                for moduleDict in moduleInfo:
                    for configDict in configInfo:
                        # 如果键值存在则替换，不存在则新增
                        for key, value in configDict.items():
                            if key not in moduleDict:
                                if value is not None:
                                    for moduleDict in moduleInfo:
                                        if value in list(moduleDict.values()):
                                            newDict = moduleDict.copy()
                                            newDict.update(configDict)
                                            moduleInfo[moduleInfo.index(moduleDict)] = newDict
                                        else:
                                            moduleDict[key] = value 
                            else:
                                moduleDict[key] = value                     
        elif module in self.replaceConfigModuleList:
            # 遍历configInfo，获取replace键的值，使用值检索moduleInfo字典值，找到对应字典后将configInfo替换或新增到module对应的字典中
            if configInfo is not None and len(configInfo) > 0:
                for configDict in configInfo:
                    replaceValue = configDict.get("replace")
                    if replaceValue is not None:
                        # print('---------------------')
                        # print(replaceValue)
                        for moduleDict in moduleInfo:
                            if replaceValue in list(moduleDict.values()) :
                                moduleDict.update(configDict)
                                # break
        
        # print(moduleInfo)
        return moduleInfo

    def execute(self,module,infoList):
        # 将流程统一封装起来
        # 先校验可配置化文件
        if KAConfig.getInstance().vertifyConfigFile(self.filePath) :
            # 读取是否可视
            # 判断是不可视，还是文件存在问题
            isVisible = KAConfig.getInstance().visibleCheck(self.filePath,module)
            if isVisible:
                configInfo = KAConfig.getInstance().readConfig(self.filePath,module)
                # print('configInfo-----')
                # print(configInfo)
                # 判断configInfo是否为空，不为空则进行替换
                if configInfo:
                    print(configInfo,infoList)
                    infoList = KAConfig.getInstance().replaceModuleInfo(module,configInfo,infoList)
                else:
                    # print("可配置文件解析后返回值为空")
                    return "configFileIsNULL"
            else:
                if False == isVisible:
                    # print("模块页签不可视")
                    return "invisible"
                else:
                    # print("配置文件解析存在问题")
                    return "configFileError"
        else:
            # print("配置文件未找到,或者不符合xml标准(md5)")  
            return "configFileNotFind"
        # 成功返回 "success"
        return "success"

if __name__ == "__main__":
    pass
    # KAConfig.getInstance()
    # 先校验可配置化文件
    # print(KAConfig.getInstance().vertifyConfigFile(filePath))
    # 读取是否可视
    # module = "battery"
    # module = "hardDisk"
    # print(type(KAConfig.getInstance().visibleCheck(filePath,module)))
    # 读取模块信息
    # configInfo = KAConfig.getInstance().readConfig(filePath,module)
    # print(configInfo)
    # 替换数据
    # configInfo = [{'manufacturer': '飞腾', 'replace': '9999'},{'model': '七彩虹', 'replace': '8888888','manufacturer': '我就是厂商'}]
    # moduleInfo = [{'manufacturer': '这是测试','mi':'lll'}]
    # KAConfig.getInstance().replaceModuleInfo(module,configInfo,moduleInfo)
    # 即换即显，多个如何处理
    