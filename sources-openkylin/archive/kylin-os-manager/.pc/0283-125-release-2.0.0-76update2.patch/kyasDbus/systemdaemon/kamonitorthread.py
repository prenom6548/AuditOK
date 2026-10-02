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
import math
import glob
import shutil
import json
import subprocess

from pprint import pprint

from kajsondef import *
from kathread import *
from utils import *

class KAMonitor() :

    def __init__(self, cmdtool):
        self.cmdTool = cmdtool
        pass

    def getMonitorsInfo(self) :
        infoList = {"list":[]}
        xrandrDat = ""
        if os.path.exists("/tmp/youker-assistant-monitorinfo.dat"):
            status, xrandrDat = subprocess.getstatusoutput('cat /tmp/youker-assistant-monitorinfo.dat')
        value = re.findall("(.*) connected", xrandrDat)
        connectList =[]
        hwinfoList = self.cmdTool.loadInfoFromHwinfo("--monitor")["list"]
        nIndex = 0
        for monitor in value :
            if Judgment_HW990():
                if "eDP-1" == monitor:
                    continue
            if Judgment_M900():
                if "DSI-1" == monitor:
                    continue
            if Judgment_HW9000C():
                if "DP-1" == monitor:
                    continue
            p = re.compile(r'\n%s connected' % monitor)
            for m in p.finditer(xrandrDat):
                #去重
                connectInterface=m.group()				
                if connectInterface in connectList:
                   continue
                else:
                   connectList.append(connectInterface)
                monitorOne = {}
                mnManufacturer,mnName,mnSize,mnRatio,mnResolution,mnMaxResolution,mnIsMain,mnGamma,mnInterface, \
                    mnModel,mnVisibleArea,mnYear,mnWeek,mnSerial = "","","","","","","","","","","","","",""
                localinfo = xrandrDat.split("\n%s connected" % monitor)[1].split("connected")[0]
                mnInterface = monitor
                if "primary" in localinfo :
                    mnIsMain = "1"
                else :
                    mnIsMain = "0"
                curResolution = re.findall("\s*(\d+)x(\d+)\+", localinfo)
                if curResolution :
                    curResolutionWidth = int(curResolution[0][0])
                    curResolutionHeight = int(curResolution[0][1])
                    mnResolution = str(curResolutionWidth)+"X"+str(curResolutionHeight)
                     # 新增分辨率
                    refreshRate  = re.findall('(\d*).(\d*)\*', localinfo)
                    if refreshRate:
                        for item in refreshRate :
                            if item[0].isdigit() and item[1].isdigit():
                                mnResolution += "@"+item[0]+"."+item[1]+"Hz"
                    if curResolutionHeight > 0 :
                        ratio = int(curResolutionWidth*100/curResolutionHeight)
                        if ratio == 125 : #5:4
                            mnRatio = "5:4"
                        elif ratio == 133 : #4:3
                            mnRatio = "4:3"
                        elif ratio == 160 : #16:10
                            mnRatio = "16:10"
                        elif ratio == 177 : #16:9
                            mnRatio = "16:9"

                result = re.findall('EDID: \s*(\w*)\s*(\w*)\s*(\w*)\s*\s*(\w*)\s*\s*(\w*)\s*\s*(\w*)\s*\s*(\w*)\s*\s*(\w*)\s*', localinfo)
                if result :
                    with open("/tmp/edid.dat", "w") as fp:
                        for edidinfoline in result[0] :
                            fp.write(edidinfoline)
                else:
                    edid_files = glob.glob('/sys/class/drm/*/edid')
                    if edid_files:
                        for edid_file in edid_files:
                            drm_device = edid_file.split("/")[-2]
                            interface_name = drm_device.split("-", 1)[-1]

                            if mnInterface in interface_name:
                                shutil.copy(edid_file, '/tmp/edid.dat')
                                break
                #   1920x1080     59.93*+
                result = re.findall('\s*(\d*)x(\d*)\s\s\s', localinfo)
                refreshRateMax = ""
                if result:
                    nMaxRes = 0
                    for item in result :
                        if item[0].isdigit() and item[1].isdigit() :
                            tempRes = int(item[0])*int(item[1])
                            if tempRes > nMaxRes :
                                nMaxRes = tempRes
                                for line in localinfo.split('\n'):
                                    if line.strip().startswith(item[0]+"x"+item[1]):
                                        if '*' in line:
                                            line = line.replace('*', "")
                                        if '+' in line:
                                            line = line.replace('+', "")
                                        tmp = line.strip().split(" ")
                                        tmp = [i for i in tmp if i != '']
                                        if(len(tmp) > 1):
                                            refreshRateMax = self.getMaxRefreshRate(tmp)
                                mnMaxResolution = (item[0] + "X" + item[1])

                if refreshRateMax != "":
                    if "+" in refreshRateMax:
                        refreshRateMax = refreshRateMax.replace("+","")
                    if "*" in refreshRateMax:
                        refreshRateMax = refreshRateMax.replace("*","")
                    mnMaxResolution += "@"+refreshRateMax+"Hz"
                is323F = Judgment_323F()
                if os.path.exists("/tmp/edid.dat") or os.path.exists("/sys/devices/platform/hldrm/drm/"):
                    status, ediddecret = "", ""
                    if os.path.exists("/tmp/edid.dat"):
                        status, ediddecret = subprocess.getstatusoutput('/usr/bin/edid-decode /tmp/edid.dat')
                    else:
                        status, ediddecret = subprocess.getstatusoutput("/usr/bin/edid-decode /sys/devices/platform/hldrm/drm/card*/*-"+monitor+"/edid")
                    if is323F and os.path.exists("/sys/class/drm"):
                        dirList = os.listdir("/sys/class/drm")
                        monDir = ""
                        for dirItem in dirList:
                            if (r"-"+mnInterface) in dirItem:
                                monDir = dirItem
                                break
                        if "" != monDir and os.path.exists("/sys/class/drm/"+monDir+"/edid"):
                            status, ediddecret = subprocess.getstatusoutput('/usr/bin/edid-decode %s' % ("/sys/class/drm/"+monDir+"/edid"))
                    #Display Product Name: LEN T2224rbA
                    #Manufacturer: LEN Model 24810 Serial Number 16843009
                    result_bak = re.findall("Manufacturer:\s*(\w*)\s*Model\s*(\w*)", ediddecret)
                    result = re.findall('Display Product Name:\s*(.*)', ediddecret)
                    if result and len(result) > 0 and len(result[0]) > 0 : ### 笔记本没有Monitor name
                        mnName = result[0].strip()
                    if result_bak : 
                        mnManufacturer = result_bak[0][0]
                        if len(mnName) == 0 :
                            if len(result_bak[0]) >= 2 :
                                mnName = result_bak[0][0]+" "+result_bak[0][1]
                            else :
                                mnName = result_bak[0][0]
                    #Made in week 26 of 2020
                    result = re.findall("Made in week\s*(\d*)\s*of\s*(\d*)", ediddecret)
                    if result:
                        mnYear = result[0][1]
                        mnWeek = result[0][0]
                    
                    #Alphanumeric Data String: BOE CQ
                    #Alphanumeric Data String: NE140FHM-N61
                    result = re.findall("Alphanumeric Data String:\s*(\S*)", ediddecret)
                    if result and len(result) > 0:
                        if len(result) >= 2 and len(result[0]) > 0 :
                            mnManufacturer = result[0]
                        elif result_bak:
                            # Alphanumeric Data String: NE140WUM-N63
                            mnManufacturer = result_bak[0][0]
                            if "BOE" == mnManufacturer:
                                mnName = result[0]
                            elif hwinfoList.__len__() != 0:
                                if "Vendor" in hwinfoList[nIndex]:
                                    mnName = hwinfoList[nIndex]["Vendor"]
                        if len(result) >= 2 and len(result[1]) > 0 :
                                #CEC model 和 Alphanumeric Data String 不一致
                            if result_bak and result[0] in result_bak[0] or "CEC" in result[0]:
                                mnName = result[1]
                            # else:
                            #     #Alphanumeric Data String: N140HCE-EN2
                            #     #Alphanumeric Data String: CMN
                            #     #Alphanumeric Data String: N140HCE-EN2
                            #     mnManufacturer = result[1]
                            #     mnName = result[0]

                    #Maximum image size: 48 cm x 27 cm
                    result = re.findall("Maximum image size:\s*(\d*)\s*cm\s*x\s*(\d*)\s*cm", ediddecret)
                    result2 = re.findall("Detailed mode: Clock (\d*\.\d*) MHz,\s*(\d*)\s*mm\s*x\s*(\d*)\s*mm", ediddecret)
                    isTN133A2 = Judgment_TN133A2()
                    is323F = Judgment_323F()
                    # 优先使用Detailed mode 里面的mm
                    if result2:
                        mx = float(result2[0][1]); my = float(result2[0][2])
                        md = math.sqrt(mx**2 + my**2)/25.4
                        mnVisibleArea = (str(mx) + " X " + str(my) + " mm")
                        # 默认根据可视大小进计算
                        # if md > 13 and md <= 14 and len(mnName) > 0 and True != isTN133A2 and True != is323F:
                        #      md = 14
                        mnSize = str("%.1f" %md)
                    elif result:
                        mx = float(result[0][0]); my = float(result[0][1])
                        md = math.sqrt(mx**2 + my**2)/2.54
                        mnVisibleArea = (str(mx) + " X " + str(my) + " cm")
                        # 默认根据可视大小进计算
                        # if md > 13 and md <= 14 and len(mnName) > 0 and True != isTN133A2 and True != is323F :
                        #      md = 14
                        mnSize = str("%.1f" %md)

                    #Gamma: 2.20
                    result = re.findall("Gamma:\s*(\S*)", ediddecret)
                    if result:
                        mnGamma = result[0]

                    #Display Product Serial Number: U1610C5W
                    result = re.findall("Display Product Serial Number:\s*(\S*)", ediddecret)
                    if result:
                        mnSerial = result[0]

                    #greatwall
                    if len(mnName) > 0 and "24AL60" == mnName :
                        mnSize = "23.8"  

                    #X100
                    if len(mnName) > 0 and "LM133LF-8L02" == mnName :
                        mnSize = "13.3"

                    #Display Product Name: 2220EH  edid v1.3
                    result = re.findall("Display Product Name:\s*(.+)", ediddecret)
                    if result and "" == mnModel:
                        mnModel = result[0]
                else:
                    match = re.search(r'(\d+)mm x (\d+)mm', localinfo)
                    if match:
                        width_mm = int(match.group(1))
                        height_mm = int(match.group(2))
                        diagonal_mm = math.sqrt(width_mm**2 + height_mm**2)
                        # 将毫米转换为英寸
                        diagonal_inches = diagonal_mm / 25.4
                        mnSize = str("%.1f" %diagonal_inches)

                if len(mnManufacturer) > 0 :
                    monitorOne[MNI_MANUFACTURER] = mnManufacturer
                if len(mnName) > 0 :
                    monitorOne[MNI_NAME] = mnName
                if len(mnSize) > 0 :
                    monitorOne[MNI_SIZE] = mnSize
                # hwinfo 有可能获取不到显示器信息，hwinfo只是用于最后做一个优化
                if len(hwinfoList) > nIndex:
                    for mapInfo in hwinfoList[nIndex]:
                        if "Model" in mapInfo and "Vendor" in mapInfo:
                            if mnManufacturer == mapInfo["Vendor"]:
                                mnModel = mapInfo["Model"].split("\"")
                                if len(mnModel) > 1 :
                                    mnModel = mnModel[0]
                                else :
                                    mnModel = mapInfo["Model"]
                                monitorOne[MNI_NAME] = mnModel
                            elif  mnManufacturer == "BOE":
                                pass
                if len(mnRatio) > 0 :
                    monitorOne[MNI_RATIO] = mnRatio
                if len(mnResolution) > 0 :
                    monitorOne[MNI_RESOLUTION] = mnResolution
                if len(mnMaxResolution) > 0 :
                    monitorOne[MNI_MAXAVARES] = mnMaxResolution
                if len(mnIsMain) > 0 :
                    monitorOne[MNI_ISMAIN] = mnIsMain
                if len(mnGamma) > 0 :
                    monitorOne[MNI_GAMMA] = mnGamma
                if len(mnInterface) > 0 :
                    monitorOne[MNI_INTERFACE] = mnInterface
                if len(mnModel) > 0 :
                    monitorOne[MNI_MODEL] = mnModel
                if len(mnVisibleArea) > 0 :
                    monitorOne[MNI_VISIBLEAREA] = mnVisibleArea
                if len(mnYear) > 0 :
                    monitorOne[MNI_YEAR] = mnYear
                if len(mnWeek) > 0 :
                    monitorOne[MNI_WEEK] = mnWeek
                if len(mnSerial) > 0 :
                    monitorOne[MNI_SERIALNUM] = mnSerial

                nIndex = nIndex + 1

                if len(monitorOne) > 0 :
                    infoList["list"].append(monitorOne)

        if Judgment_HW990():
            if os.path.exists("/sys/class/drm"):
                edid_files = glob.glob("/sys/class/drm/*/edid")
                for edid_file in edid_files:
                    if edid_file.find("card0-eDP-1") != -1:
                        monitorOne = {}
                        monitorOne[MNI_SIZE] = "14"
                        monitorOne[MNI_MAXAVARES] = "2160X1440"
                        monitorOne[MNI_MANUFACTURER] = "LCD"
                        monitorOne[MNI_INTERFACE] = "eDP-1"
                        infoList["list"].append(monitorOne)

        if Judgment_M900():
            if os.path.exists("/sys/class/drm"):
                 edid_files = glob.glob("/sys/class/drm/*/edid")
                 for edid_file in edid_files:
                    if edid_file.find("card0-DSI-1") != -1:
                        monitorOne = {}
                        monitorOne[MNI_SIZE] = "23.8"
                        monitorOne[MNI_MAXAVARES] = "2160X1440"
                        monitorOne[MNI_MANUFACTURER] = "HKC"
                        monitorOne[MNI_INTERFACE] = "DSI"
                        infoList["list"].append(monitorOne)

        if Judgment_HW9000C():
            if os.path.exists("/sys/class/drm"):
                edid_files = glob.glob("/sys/class/drm/*/edid")
                for edid_file in edid_files:
                    if edid_file.find("card0-DP-1") != -1:
                        monitorOne = {}
                        monitorOne[MNI_SIZE] = "14.2"
                        monitorOne[MNI_MAXAVARES] = "2880X1920"
                        monitorOne[MNI_INTERFACE] = "DP-1"
                        monitorOne[MNI_RATIO] = "3:2"
                        infoList["list"].append(monitorOne)

        if "list" in infoList and len(infoList["list"]) == 1 :
            infoList["list"][0][MNI_ISMAIN] = "1"

        #  hw设备如果存在插入外屏但禁用的情况，则需要使用kscreen-doctor -o 去获取显示器信息
        if Judgment_HW9000C() or Judgment_HW990() or Judgment_HW9A0():
            status, result = subprocess.getstatusoutput('cat /tmp/youker-assistant-monitorinfo2.dat')

            # 按 Output: 分割所有输出块
            outputs = result.split('Output:')[1:]  # 第一个元素是空字符串，跳过
            disabled_outputs = []
            for output in outputs:
                clean_output = self.clean_ansi_codes(output)
                if "disabled" in clean_output and "connected" in clean_output:
                    # 清理格式，并重新加上 "Output:"
                    cleaned_output = "Output:" + clean_output.strip()
                    pattern = r"Output:\s*(\d+)\s+(\S+)\s+disabled\s+connected"
                    matches = re.findall(pattern, cleaned_output)
                    monitorOne = {}
                    monitorOne[MNI_SIZE] = "unknow"
                    monitorOne[MNI_MAXAVARES] = "unknow"
                    monitorOne[MNI_INTERFACE] = matches[0][1]
                    monitorOne[MNI_RATIO] = "unknow"
                    if monitorOne[MNI_INTERFACE] != "DP-1" and monitorOne[MNI_INTERFACE] != "eDP-1":
                        infoList["list"].append(monitorOne)

        module = "monitor"
        if "list" in infoList:
            status = KAConfig.getInstance().execute(module,infoList["list"])
            if "invisible" == status:
                # infoList["list"] = []
                return ""
        return json.dumps(infoList)

    def clean_ansi_codes(self, text):
        """去除 ANSI 颜色/控制字符"""
        ansi_escape = re.compile(r'\x1B(?:[@-Z\\-_]|\[[0-?]*[ -/]*[@-~])')
        return ansi_escape.sub('', text)

    #概要信息
    def getOutline(self) :
        if "" == self.getMonitorsInfo():
            return ""
        outLine = self.getMonitorsInfo()
        outLineObj = json.loads(outLine)
        outLine = ""
        if len(outLineObj) > 0 and "list" in outLineObj :
            nIndex = 0
            for info in outLineObj["list"] :
                if MNI_NAME in info :
                    if nIndex == 0 :
                        outLine += info[MNI_NAME]
                    else :
                        outLine += " / " + info[MNI_NAME]
                # 华为机型如果没有型号则用厂商+屏幕尺寸
                elif Judgment_PANGUW() or Judgment_HW990() or Judgment_HW9A0():
                    if MNI_MANUFACTURER in info:
                        if nIndex == 0 :
                            outLine += info[MNI_MANUFACTURER]
                        else:
                            outLine += " / " + info[MNI_MANUFACTURER]
                if MNI_SIZE in info :
                    outLine += "("+info[MNI_SIZE]+")"
                nIndex = nIndex + 1
        return outLine

    def getMaxRefreshRate(self, refreshrates):
        try:
            max_value = float(refreshrates[1])
        except (ValueError, IndexError, TypeError):
            return '0'

        for value in refreshrates[1:]:
            if float(value) > max_value:
                max_value = float(value)

        formatted_num = "%.2f" % max_value
        return str(formatted_num)

if __name__ == "__main__":
    cmdtool = KACmdTool()
    cc = KAMonitor(cmdtool)
    pprint(cc.getMonitorsInfo())
    #pprint(cc.getOutline())
