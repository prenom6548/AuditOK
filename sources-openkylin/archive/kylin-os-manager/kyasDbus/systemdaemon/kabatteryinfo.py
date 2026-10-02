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


import sys
import os
import re

import json
import subprocess

from pprint import pprint

from kajsondef import *
from kathread import *
from systemboardthread import *
from kaconfig import *


class KABatteryInfo():

    def __init__(self):
        pass

    def isGreatWallMachine(self):
        """判断是否为长城机器"""
        try:
            # 执行命令：dmidecode | sed 's/[[:space:]]//g' | grep -i -c greatwall
            # 进程本身已经是 root 权限，不需要 sudo
            args = ["dmidecode"]
            pipe = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            output = pipe.stdout.read()
            pipe.wait()
            
            # 移除所有空白字符
            output_str = output.decode('utf-8', errors='ignore')
            output_str = re.sub(r'\s+', '', output_str)
            
            # 查找 greatwall（不区分大小写）
            count = len(re.findall(r'greatwall', output_str, re.IGNORECASE))
            return count > 0
        except Exception as e:
            return False

    def formatEnergyValue(self, value):
        """格式化电量值，保留一位小数"""
        if not value or value == "":
            return value
        
        try:
            # 提取数字部分和单位，例如 "73.7235 Wh" -> 73.7235 和 "Wh"
            match = re.match(r'([\d.]+)\s*([A-Za-z]+)?', value.strip())
            if match:
                number_str = match.group(1)
                unit = match.group(2) if match.group(2) else ""
                number = float(number_str)
                formatted_number = "{:.1f}".format(number)
                if unit:
                    return formatted_number + " " + unit
                else:
                    return formatted_number
            return value
        except (ValueError, AttributeError):
            return value

    def getBatteryInfo(self, sysProductName = ""):
        summary = {"list":[]}
        
        # 判断是否为长城机器
        isGreatWall = self.isGreatWallMachine()

        args = ["upower", "--dump"]
        pipe = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        output = pipe.stdout.readlines()

        oddindex = -1
        pattern = re.compile(r'Device: ')
        batSerailNum,batState,batPercentage,batTimeToEmpty,batEnergyFull,batEnergyFullDesign,batEnergy,batUsedTimes,batManufacturer,batModel,batSupply =  \
            "","","","","","","","","","",False
        batOne = {}

        for line in [ bytes.decode(l.strip(),"utf-8","ignore") for l in output]:
            if pattern.match(line):
                if "battery" in line.lower() :
                    oddindex = 1
                else :
                    oddindex = -1
                
                if len(batOne) > 0 and batSupply :
                    if len(sysProductName) > 0 :
                        if "TN140A2" in sysProductName or "TN133A2" in sysProductName :   ##长城笔记本
                            # batOne[BTI_MANUFACTURER] = "飞毛腿（福建）电子有限公司"
                            # batOne[BTI_MODEL] = "SNGW001"
                            batOne[BTI_ENERGY] = ""
                            batOne[BTI_ENERGYFULL] = "60Wh"
                        elif "F860-T1" in sysProductName :   ##清华同方笔记本
                            batOne[BTI_MANUFACTURER] = "飞毛腿"
                            batOne[BTI_MODEL] = "SNBL001"
                            batOne[BTI_ENERGY] = ""
                            batOne[BTI_ENERGYFULL] = "63Wh"
                    
                    # 格式化电量值，保留一位小数
                    if BTI_ENERGY in batOne and len(batOne[BTI_ENERGY]) > 0:
                        batOne[BTI_ENERGY] = self.formatEnergyValue(batOne[BTI_ENERGY])
                    if BTI_ENERGYFULL in batOne and len(batOne[BTI_ENERGYFULL]) > 0:
                        batOne[BTI_ENERGYFULL] = self.formatEnergyValue(batOne[BTI_ENERGYFULL])
                    if BTI_ENERGYFULLDESIGN in batOne and len(batOne[BTI_ENERGYFULLDESIGN]) > 0:
                        batOne[BTI_ENERGYFULLDESIGN] = self.formatEnergyValue(batOne[BTI_ENERGYFULLDESIGN])
                    
                    # 长城机器特殊处理
                    if isGreatWall:
                        # 确保满电容量和当前电量为空
                        batOne[BTI_ENERGY] = ""
                        batOne[BTI_ENERGYFULL] = ""
                    
                    if BTI_MANUFACTURER not in batOne :
                        batOne[BTI_NAME] = "Name"
                    summary["list"].append(batOne)
                batSerailNum,batState,batPercentage,batTimeToEmpty,batEnergyFull,batEnergyFullDesign,batEnergy,batUsedTimes,batManufacturer,batModel,batSupply =  \
                    "","","","","","","","","","",False
                batOne = {}
            else:
                if oddindex == -1:
                    continue
                if ":" not in line:
                    continue
                
                results = [ v.strip() for v in line.split(":", 1)]
                
                #厂商
                if line.startswith("vendor:") and "kylin" != results[1] :
                    batManufacturer = results[1]
                #序列号
                if line.startswith("serial:") and len(results[1]) > 0 and "0" != results[1] and "None" != results[1]:
                    batSerailNum = results[1]
                #型号
                if line.startswith("model:"):
                    batModel = results[1]
                #状态
                if line.startswith("state:"):
                    batState = results[1]
                #电量
                if line.startswith("percentage:"):
                    batPercentage = results[1]
                #当前容量
                if line.startswith("energy:"):
                    if isGreatWall:
                        # 长城机器：即使获取到也设置为空
                        batEnergy = ""
                    else:
                        batEnergy = results[1]
                #满电容量
                if line.startswith("energy-full:"):
                    if isGreatWall:
                        # 长城机器：即使获取到也设置为空
                        batEnergyFull = ""
                    else:
                        batEnergyFull = results[1]
                #额定电量
                if line.startswith("energy-full-design:"):
                    batEnergyFullDesign = results[1]
                #预计使用时间
                if line.startswith("time to empty:"):
                    batTimeToEmpty = results[1]
                #是否供电
                if line.startswith("power supply:"):
                    if results[1] == "yes" :
                        batSupply = True
                    else :
                        batSupply = False

                if len(batManufacturer) > 0 :
                    batOne[BTI_MANUFACTURER] = batManufacturer
                if len(batSerailNum) > 0 :
                    batOne[BTI_SERAILNUM] = batSerailNum
                if len(batState) > 0 :
                    batOne[BTI_STATE] = batState
                if len(batPercentage) > 0 :
                    batOne[BTI_PERCENTAGE] = batPercentage
                if len(batEnergy) > 0 :
                    # 格式化电量值，保留一位小数
                    formattedEnergy = self.formatEnergyValue(batEnergy)
                    batOne[BTI_ENERGY] = formattedEnergy
                elif isGreatWall:
                    # 长城机器：确保为空
                    batOne[BTI_ENERGY] = ""
                if len(batEnergyFull) > 0 :
                    # 格式化电量值，保留一位小数
                    formattedEnergyFull = self.formatEnergyValue(batEnergyFull)
                    batOne[BTI_ENERGYFULL] = formattedEnergyFull
                elif isGreatWall:
                    # 长城机器：确保为空
                    batOne[BTI_ENERGYFULL] = ""
                if len(batEnergyFullDesign) > 0 :
                    # 格式化电量值，保留一位小数
                    formattedEnergyFullDesign = self.formatEnergyValue(batEnergyFullDesign)
                    batOne[BTI_ENERGYFULLDESIGN] = formattedEnergyFullDesign
                if len(batTimeToEmpty) > 0 :
                    batOne[BTI_TTE] = batTimeToEmpty
                if len(batUsedTimes) > 0 :
                    batOne[BTI_USEDTIMES] = batUsedTimes
                if len(batModel) > 0 :
                    batOne[BTI_MODEL] = batModel
        if len(batOne) > 0 and batSupply :
            if len(sysProductName) > 0 :
                if "TN140A2" in sysProductName or "TN133A2" in sysProductName :   ##长城笔记本
                    # batOne[BTI_MANUFACTURER] = "飞毛腿（福建）电子有限公司"
                    # batOne[BTI_MODEL] = "SNGW001"
                    batOne[BTI_ENERGY] = ""
                    batOne[BTI_ENERGYFULL] = "60Wh"
                elif "F860-T1" in sysProductName :   ##清华同方笔记本
                    batOne[BTI_MANUFACTURER] = "飞毛腿"
                    batOne[BTI_MODEL] = "SNBL001"
                    batOne[BTI_ENERGY] = ""
                    batOne[BTI_ENERGYFULL] = "63Wh"
            
            # 格式化电量值，保留一位小数
            if BTI_ENERGY in batOne and len(batOne[BTI_ENERGY]) > 0:
                batOne[BTI_ENERGY] = self.formatEnergyValue(batOne[BTI_ENERGY])
            if BTI_ENERGYFULL in batOne and len(batOne[BTI_ENERGYFULL]) > 0:
                batOne[BTI_ENERGYFULL] = self.formatEnergyValue(batOne[BTI_ENERGYFULL])
            if BTI_ENERGYFULLDESIGN in batOne and len(batOne[BTI_ENERGYFULLDESIGN]) > 0:
                batOne[BTI_ENERGYFULLDESIGN] = self.formatEnergyValue(batOne[BTI_ENERGYFULLDESIGN])
            
            # 长城机器特殊处理
            if isGreatWall:
                # 确保满电容量和当前电量为空
                batOne[BTI_ENERGY] = ""
                batOne[BTI_ENERGYFULL] = ""
            
            if BTI_MANUFACTURER not in batOne :
                batOne[BTI_NAME] = "Name"
            summary["list"].append(batOne)
        if len(summary["list"]) == 0:
            return ""

        # 传入需要配置化的模块名称
        module = "battery"
        if "list" in summary:
            status = KAConfig.getInstance().execute(module,summary["list"])
            if "invisible" == status:
                return ""

        return json.dumps(summary)

if __name__ == "__main__":
    cc = KABatteryInfo()
    pprint(cc.getBatteryInfo())

