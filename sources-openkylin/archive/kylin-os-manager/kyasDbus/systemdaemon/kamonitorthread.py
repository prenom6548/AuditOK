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
                # 初始化 EDID 解析的厂商信息和名称变量，确保在整个处理过程中都可以访问
                edid_manufacturer = ""
                edid_name = ""  # 保存从 EDID 解析的名称，优先使用
                localinfo = xrandrDat.split("\n%s connected" % monitor)[1].split("connected")[0]
                mnInterface = monitor
                if "primary" in localinfo :
                    mnIsMain = "1"
                else :
                    mnIsMain = "0"
                curResolution = re.findall(r"\s*(\d+)x(\d+)\+", localinfo)
                if curResolution :
                    curResolutionWidth = int(curResolution[0][0])
                    curResolutionHeight = int(curResolution[0][1])
                    mnResolution = str(curResolutionWidth)+"X"+str(curResolutionHeight)
                     # 新增分辨率
                    refreshRate  = re.findall(r'(\d*).(\d*)\*', localinfo)
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

                # 为每个显示器使用独立的临时文件，避免多个显示器共享同一个文件导致数据被覆盖
                edid_file_path = "/tmp/edid_%s.dat" % mnInterface.replace("-", "_")
                result = re.findall(r'EDID: \s*(\w*)\s*(\w*)\s*(\w*)\s*\s*(\w*)\s*\s*(\w*)\s*\s*(\w*)\s*\s*(\w*)\s*\s*(\w*)\s*', localinfo)
                if result :
                    with open(edid_file_path, "w") as fp:
                        for edidinfoline in result[0] :
                            fp.write(edidinfoline)
                else:
                    edid_files = glob.glob('/sys/class/drm/*/edid')
                    if edid_files:
                        for edid_file in edid_files:
                            drm_device = edid_file.split("/")[-2]
                            interface_name = drm_device.split("-", 1)[-1]

                            if mnInterface in interface_name:
                                shutil.copy(edid_file, edid_file_path)
                                break
                #   1920x1080     59.93*+
                result = re.findall(r'\s*(\d*)x(\d*)\s\s\s', localinfo)
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
                # 为每个显示器使用独立的临时文件，避免多个显示器共享同一个文件导致数据被覆盖
                edid_file_path = "/tmp/edid_%s.dat" % mnInterface.replace("-", "_")
                
                # 优先直接从 /sys/class/drm 读取，避免使用可能被覆盖的临时文件
                edid_source_file = None
                if os.path.exists("/sys/class/drm"):
                    dirList = os.listdir("/sys/class/drm")
                    for dirItem in dirList:
                        if (r"-"+mnInterface) in dirItem:
                            potential_edid = "/sys/class/drm/"+dirItem+"/edid"
                            if os.path.exists(potential_edid):
                                edid_source_file = potential_edid
                                break
                
                status, ediddecret = "", ""
                if edid_source_file:
                    # 直接从 /sys/class/drm 读取，最可靠
                    status, ediddecret = subprocess.getstatusoutput('/usr/bin/edid-decode %s' % edid_source_file)
                elif os.path.exists(edid_file_path):
                    # 使用为这个显示器创建的独立临时文件
                    status, ediddecret = subprocess.getstatusoutput('/usr/bin/edid-decode %s' % edid_file_path)
                elif os.path.exists("/tmp/edid.dat"):
                    # 兼容旧代码，但应该避免这种情况
                    status, ediddecret = subprocess.getstatusoutput('/usr/bin/edid-decode /tmp/edid.dat')
                elif os.path.exists("/sys/devices/platform/hldrm/drm/"):
                    status, ediddecret = subprocess.getstatusoutput("/usr/bin/edid-decode /sys/devices/platform/hldrm/drm/card*/*-"+monitor+"/edid")
                
                # 只有在成功获取到 EDID 数据时才进行解析
                if len(ediddecret) > 0:
                    #Display Product Name: LEN T2224rbA
                    #Manufacturer: LEN Model 24810 Serial Number 16843009
                    # 改进制造商解析：支持多种格式
                    
                    # 查找完整的 Manufacturer 行，避免匹配到其他包含 "Manufacturer" 的行
                    # edid-decode 输出格式: "    Manufacturer: AOC" (有4个空格缩进)
                    # 格式可能是: Manufacturer: XXX Model YYY Serial Number ZZZ
                    # 或者: Manufacturer: XXX
                    manufacturer_lines = re.findall(r"^\s+Manufacturer:\s*(.+)$", ediddecret, re.MULTILINE)
                    
                    # 匹配格式: "    Manufacturer: AOC Model 12802" (有缩进)
                    # 或者: "Manufacturer: TMA Model 2067 Serial Number 0" (可能有不同的缩进)
                    # 优先匹配包含 Model 的完整格式
                    result_bak = re.findall(r"^\s*Manufacturer:\s*(\w+)\s+Model\s+(\w+)", ediddecret, re.MULTILINE)
                    if not result_bak:
                        # 尝试另一种格式：Manufacturer: XXX (单独一行，可能有不同的缩进)
                        # 使用更宽松的匹配，允许 Manufacturer: 后面有任意空白字符
                        result_bak = re.findall(r"^\s*Manufacturer:\s*(\w+)(?:\s|$)", ediddecret, re.MULTILINE)
                        if result_bak:
                            # 接受所有匹配到的制造商名称（包括 SDC，因为它是有效的制造商）
                            # SDC = Samsung Display Company，是合法的制造商
                            # AOC 也是合法的制造商
                            # TMA 也是合法的制造商
                            # 每个显示器应该使用自己的 EDID 数据，不应该被过滤
                            result_bak = [(result_bak[0], "")]
                        else:
                            # 最后尝试：在整个字符串中查找 Manufacturer: XXX 模式（不限制行首）
                            result_bak = re.findall(r"Manufacturer:\s*(\w+)(?:\s+Model|\s|$)", ediddecret, re.MULTILINE)
                            if result_bak:
                                result_bak = [(result_bak[0], "")]
                            else:
                                result_bak = []
                    
                    # 匹配格式: "    Display Product Name: 'U32P2WG6'" (有缩进，可能有引号)
                    result = re.findall(r"^\s*Display Product Name:\s*['\"]?([^\r\n'\"]+)['\"]?", ediddecret, re.MULTILINE)
                    if result and len(result) > 0 and len(result[0]) > 0 : ### 笔记本没有Monitor name
                        display_name = result[0].strip()
                        mnName = display_name
                        # 如果 Display Product Name 看起来像型号（包含字母数字和连字符，如 ATNA40YK14-0）
                        if re.match(r'^[A-Z0-9]+[A-Z0-9\-]+$', display_name):
                            mnModel = display_name
                    
                    # 优先使用从 EDID 解析的制造商信息，避免被后续逻辑覆盖
                    # 这个值应该在整个处理过程中保持不变，除非 EDID 中没有获取到
                    # edid_manufacturer 已在循环开始处初始化
                    if result_bak and len(result_bak) > 0: 
                        edid_manufacturer = result_bak[0][0]
                        # 优先使用 EDID 解析的厂商信息，确保不被后续逻辑覆盖
                        mnManufacturer = edid_manufacturer
                        # 不要使用 Manufacturer + Model 的组合作为名称
                        # 名称应该优先从 Alphanumeric Data String 获取（如 TM140VDXP02）
                        # 这里只设置厂商信息，不设置名称
                    #Made in week 26 of 2020
                    result = re.findall(r"Made in week\s*(\d*)\s*of\s*(\d*)", ediddecret)
                    if result:
                        mnYear = result[0][1]
                        mnWeek = result[0][0]
                    
                    #Alphanumeric Data String: BOE CQ
                    #Alphanumeric Data String: NE140FHM-N61
                    #Alphanumeric Data String: ATNA40YK14-0
                    # 匹配格式: "    Alphanumeric Data String: 'ATNA40YK14-0 '" (有缩进，值在引号中)
                    result = re.findall(r"^\s*Alphanumeric Data String:\s*['\"]?([^\r\n'\"]+)['\"]?", ediddecret, re.MULTILINE)
                    if result and len(result) > 0:
                        # 清理字符串，去除引号和空格
                        cleaned_result = []
                        for r in result:
                            cleaned = r.strip().strip("'\"")
                            if cleaned:
                                cleaned_result.append(cleaned)
                        result = cleaned_result
                        
                        # 优先查找类似型号格式的字符串（如 ATNA40YK14-0 或 TM140VDXP02）
                        # 对于 TMA 厂商，优先使用 Alphanumeric Data String 作为名称
                        for alphanumeric_str in result:
                            alphanumeric_str = alphanumeric_str.strip()
                            # 匹配型号格式：字母数字组合，可能包含连字符（如 ATNA40YK14-0 或 TM140VDXP02）
                            if re.match(r'^[A-Z0-9]+[A-Z0-9\-]+$', alphanumeric_str) and len(alphanumeric_str) >= 8:
                                # 如果还没有设置型号，使用这个字符串
                                if len(mnModel) == 0:
                                    mnModel = alphanumeric_str
                                # 优先使用 Alphanumeric Data String 作为名称，覆盖之前可能设置的名称
                                # 保存到 edid_name，确保优先使用 EDID 解析的名称
                                edid_name = alphanumeric_str
                                mnName = edid_name
                                break
                        # 保存从 EDID 头部获取的制造商信息（优先级最高，不应被 Alphanumeric Data String 覆盖）
                        # 使用之前保存的 edid_manufacturer，确保不会被覆盖
                        manufacturer_from_edid_header = edid_manufacturer
                        
                        # 如果已经从 EDID 头部获取到制造商，优先使用并保持不变
                        if manufacturer_from_edid_header:
                            # 保持从 EDID 头部获取的制造商信息（如 "IVO"），确保不被后续逻辑覆盖
                            mnManufacturer = manufacturer_from_edid_header
                            
                            # 第二个 Alphanumeric Data String 通常是型号（如 "M140NWHE R0"）
                            if len(result) >= 2 and len(result[1].strip()) > 0:
                                # 使用第二个字符串作为型号/名称
                                mnName = result[1].strip()
                                # 同时设置型号字段
                                if len(mnModel) == 0:
                                    mnModel = result[1].strip()
                            # 如果只有一个 Alphanumeric Data String，优先使用它作为名称
                            elif len(result) > 0 and len(result[0].strip()) > 0:
                                # 优先使用 Alphanumeric Data String 作为名称，覆盖之前可能设置的名称
                                edid_name = result[0].strip()
                                mnName = edid_name
                        elif len(result) >= 2 and len(result[0]) > 0 and len(edid_manufacturer) == 0:
                            # 只有在没有从 EDID 头部获取到制造商时，才使用 Alphanumeric Data String
                            # 但要确保第一个字符串不是型号格式（型号格式不应该作为制造商）
                            potential_manufacturer = result[0].strip()
                            # 如果第一个字符串看起来像型号（如 ATNA40YK14-0），则跳过，使用第二个
                            if not re.match(r'^[A-Z0-9]+[A-Z0-9\-]+$', potential_manufacturer) or len(potential_manufacturer) < 8:
                                mnManufacturer = potential_manufacturer
                            elif len(result) >= 2:
                                # 如果第一个是型号格式，尝试使用第二个（可能是制造商）
                                potential_manufacturer2 = result[1].strip()
                                if not re.match(r'^[A-Z0-9]+[A-Z0-9\-]+$', potential_manufacturer2) or len(potential_manufacturer2) < 8:
                                    mnManufacturer = potential_manufacturer2
                        elif len(edid_manufacturer) > 0:
                            # 保持从 EDID 头部获取的制造商信息，确保不被覆盖
                            mnManufacturer = edid_manufacturer
                        elif result_bak and len(result_bak) > 0:
                            # 如果 edid_manufacturer 为空，但 result_bak 有值，使用它
                            mnManufacturer = result_bak[0][0]
                            
                            # 第一个 Alphanumeric Data String 通常是显示器名称
                            # 优先使用 Alphanumeric Data String 作为名称，覆盖之前可能设置的名称
                            if len(result) > 0 and len(result[0]) > 0:
                                edid_name = result[0].strip()
                                mnName = edid_name
                            
                            # 只有在 EDID 中没有获取到制造商信息时，才使用 hwinfoList 中的信息
                            # 避免 hwinfoList 覆盖从 EDID 中正确解析出的制造商信息
                            # 优先使用 EDID 解析的厂商信息（edid_manufacturer），只有在 EDID 完全没有解析到厂商时才使用 hwinfo
                            if len(edid_manufacturer) == 0 and len(mnManufacturer) == 0 and hwinfoList.__len__() != 0:
                                if "Vendor" in hwinfoList[nIndex]:
                                    mnManufacturer = hwinfoList[nIndex]["Vendor"]
                            elif len(edid_manufacturer) > 0:
                                # 确保 EDID 解析的厂商信息不被覆盖
                                mnManufacturer = edid_manufacturer
                            if "BOE" == mnManufacturer:
                                if len(result) > 0:
                                    mnName = result[0].strip()
                            elif len(mnName) == 0 and hwinfoList.__len__() != 0:
                                # 只有在名称为空时才使用 hwinfoList 中的 Vendor 作为名称
                                if "Vendor" in hwinfoList[nIndex]:
                                    mnName = hwinfoList[nIndex]["Vendor"]

                        # 对于 TMA 厂商，如果还没有设置名称，优先使用 Alphanumeric Data String 作为名称
                        if "TMA" == mnManufacturer:
                            if len(result) > 0 and len(edid_name) == 0:
                                # 对于 TMA 厂商，优先使用 Alphanumeric Data String 作为名称
                                edid_name = result[0].strip()
                                mnName = edid_name

                        # 第二个 Alphanumeric Data String 通常是型号
                        if len(result) >= 2 and len(result[1]) > 0:
                            # 如果第一个字符串包含制造商信息（如 "CEC"），则第二个是型号
                            if result_bak and result[0].strip() in result_bak[0] or "CEC" in result[0]:
                                mnName = result[1].strip()
                            # else:
                            #     #Alphanumeric Data String: N140HCE-EN2
                            #     #Alphanumeric Data String: CMN
                            #     #Alphanumeric Data String: N140HCE-EN2
                            #     mnManufacturer = result[1]
                            #     mnName = result[0]

                    # Maximum image size: 48 cm x 27 cm
                    # Detailed mode: Clock 148.500 MHz, 527 mm x 296 mm（edid-decode 中物理尺寸，优先使用）
                    result_cm = re.findall(r"Maximum image size:\s*(\d+)\s*cm\s*x\s*(\d+)\s*cm", ediddecret)
                    # 兼容 "Detailed mode: Clock 148.500 MHz, 527 mm x 296 mm" 或 "Clock 148 MHz, 527 mm x 296 mm"
                    result2 = re.findall(r"Detailed mode:\s*Clock\s*\d+(?:\.\d+)?\s*MHz,\s*(\d+)\s*mm\s*x\s*(\d+)\s*mm", ediddecret)
                    # 兜底：任意位置的 "XXX mm x YYY mm"（如仅出现在 Detailed mode 行时已被 result2 匹配）
                    result_mm_fallback = re.findall(r"(\d+)\s*mm\s*x\s*(\d+)\s*mm", ediddecret)
                    isTN133A2 = Judgment_TN133A2()
                    is323F = Judgment_323F()

                    def set_size_from_mm(mx_mm, my_mm):
                        md = math.sqrt(float(mx_mm)**2 + float(my_mm)**2) / 25.4
                        if md >= 65:
                            return str(round(md)), (str(mx_mm) + " X " + str(my_mm) + " mm")
                        return str("%.1f" % md), (str(mx_mm) + " X " + str(my_mm) + " mm")

                    def set_size_from_cm(mx_cm, my_cm):
                        md = math.sqrt(float(mx_cm)**2 + float(my_cm)**2) / 2.54
                        if md >= 65:
                            return str(round(md)), (str(mx_cm) + " X " + str(my_cm) + " cm")
                        return str("%.1f" % md), (str(mx_cm) + " X " + str(my_cm) + " cm")

                    # 优先使用 Detailed mode 中的 mm（如 527 mm x 296 mm），其次 Maximum image size 的 cm，最后兜底 mm
                    if result2:
                        mx, my = result2[0][0], result2[0][1]
                        mnSize, mnVisibleArea = set_size_from_mm(mx, my)
                    elif result_cm:
                        mx, my = result_cm[0][0], result_cm[0][1]
                        mnSize, mnVisibleArea = set_size_from_cm(mx, my)
                    elif result_mm_fallback:
                        # 取第一组合理的 mm x mm（物理尺寸），过滤明显非尺寸的数值（如时钟等）
                        for (mx, my) in result_mm_fallback:
                            mx, my = int(mx), int(my)
                            if 50 <= mx <= 3000 and 30 <= my <= 2000:
                                mnSize, mnVisibleArea = set_size_from_mm(mx, my)
                                break
                        else:
                            mx, my = result_mm_fallback[0][0], result_mm_fallback[0][1]
                            mnSize, mnVisibleArea = set_size_from_mm(mx, my)

                        
                    #Gamma: 2.20
                    result = re.findall(r"Gamma:\s*(\S*)", ediddecret)
                    if result:
                        mnGamma = result[0]

                    #Display Product Serial Number: U1610C5W
                    result = re.findall(r"Display Product Serial Number:\s*(\S*)", ediddecret)
                    if result:
                        mnSerial = result[0]

                    #greatwall
                    if len(mnName) > 0 and "24AL60" == mnName :
                        mnSize = "23.8"  

                    #X100
                    if len(mnName) > 0 and "LM133LF-8L02" == mnName :
                        mnSize = "13.3"

                    #Display Product Name: 2220EH  edid v1.3
                    #Display Product Name: ATNA40YK14-0
                    # 再次尝试匹配 Display Product Name（作为备用，如果之前没有匹配到）
                    # 匹配格式: "    Display Product Name: 'U32P2WG6'" (有缩进，值在引号中)
                    result = re.findall(r"^\s*Display Product Name:\s*['\"]?([^\r\n'\"]+)['\"]?", ediddecret, re.MULTILINE)
                    if result and len(result) > 0:
                        display_name = result[0].strip()
                        # 如果之前没有设置名称，使用 Display Product Name
                        if len(mnName) == 0:
                            mnName = display_name
                        # 如果 Display Product Name 看起来像型号（如 ATNA40YK14-0），优先使用
                        if re.match(r'^[A-Z0-9]+[A-Z0-9\-]+$', display_name) and len(display_name) >= 8:
                            if len(mnModel) == 0:
                                mnModel = display_name
                        elif len(mnModel) == 0:
                            # 如果不符合型号格式，但 mnModel 为空，也设置
                            mnModel = display_name
                else:
                    match = re.search(r'(\d+)mm x (\d+)mm', localinfo)
                    if match:
                        width_mm = int(match.group(1))
                        height_mm = int(match.group(2))
                        diagonal_mm = math.sqrt(width_mm**2 + height_mm**2)
                        # 将毫米转换为英寸
                        diagonal_inches = diagonal_mm / 25.4
                        mnSize = str("%.1f" %diagonal_inches)

                # 使用 hwinfo 结果补充 EDID 解析缺失的信息
                # 优先级：EDID 解析的信息 > hwinfo 的信息
                # 只有在 EDID 中没有获取到相应信息时，才使用 hwinfo 的结果
                # 确保 EDID 解析的厂商信息（edid_manufacturer）优先，不被 hwinfo 覆盖
                if len(hwinfoList) > nIndex:
                    # hwinfoList[nIndex] 可能是一个字典，也可能是一个列表
                    if isinstance(hwinfoList[nIndex], dict):
                        # 如果是字典，直接使用
                        mapInfo = hwinfoList[nIndex]
                        if "Model" in mapInfo and "Vendor" in mapInfo:
                            # 如果 EDID 中没有获取到制造商信息，使用 hwinfoList 中的 Vendor
                            # 优先检查 edid_manufacturer，确保 EDID 解析的厂商信息不被覆盖
                            if len(edid_manufacturer) == 0 and len(mnManufacturer) == 0:
                                if "Vendor" in mapInfo and isinstance(mapInfo["Vendor"], str):
                                    mnManufacturer = mapInfo["Vendor"]
                            elif len(edid_manufacturer) > 0:
                                # 确保 EDID 解析的厂商信息不被覆盖
                                mnManufacturer = edid_manufacturer
                            
                            # 如果 EDID 中没有获取到名称信息，使用 hwinfoList 中的 Model
                            # 优先检查 edid_name，确保 EDID 解析的名称不被覆盖
                            if len(edid_name) == 0 and len(mnName) == 0:
                                if "Model" in mapInfo and isinstance(mapInfo["Model"], str):
                                    hwinfo_model = mapInfo["Model"].split("\"")
                                    if len(hwinfo_model) > 1:
                                        hwinfo_model = hwinfo_model[0]
                                    else:
                                        hwinfo_model = mapInfo["Model"]
                                    if len(hwinfo_model) > 0:
                                        mnName = hwinfo_model
                            elif len(edid_name) > 0:
                                # 如果 EDID 解析到了名称，确保不被覆盖
                                mnName = edid_name
                            
                            # 如果 EDID 中没有获取到型号信息，使用 hwinfoList 中的 Model
                            if len(mnModel) == 0:
                                if "Model" in mapInfo and isinstance(mapInfo["Model"], str):
                                    hwinfo_model = mapInfo["Model"].split("\"")
                                    if len(hwinfo_model) > 1:
                                        hwinfo_model = hwinfo_model[0]
                                    else:
                                        hwinfo_model = mapInfo["Model"]
                                    if len(hwinfo_model) > 0:
                                        mnModel = hwinfo_model
                            
                            # 如果制造商匹配，且名称为空，也可以使用 hwinfoList 中的 Model
                            # 但前提是 EDID 中没有解析到厂商信息，或者 EDID 解析的厂商和 hwinfo 的厂商匹配
                            # 如果 EDID 解析到了厂商信息，优先使用 EDID 的厂商，不要被 hwinfo 覆盖
                            if len(edid_manufacturer) == 0 and len(mnManufacturer) > 0 and "Vendor" in mapInfo and isinstance(mapInfo["Vendor"], str) and mnManufacturer == mapInfo["Vendor"]:
                                # 优先检查 edid_name，确保 EDID 解析的名称不被覆盖
                                if len(edid_name) == 0 and len(mnName) == 0 and "Model" in mapInfo and isinstance(mapInfo["Model"], str):
                                    hwinfo_model = mapInfo["Model"].split("\"")
                                    if len(hwinfo_model) > 1:
                                        hwinfo_model = hwinfo_model[0]
                                    else:
                                        hwinfo_model = mapInfo["Model"]
                                    if len(hwinfo_model) > 0:
                                        mnName = hwinfo_model
                                elif len(edid_name) > 0:
                                    # 如果 EDID 解析到了名称，确保不被覆盖
                                    mnName = edid_name
                            elif len(edid_manufacturer) > 0:
                                # 如果 EDID 解析到了厂商信息，确保不被覆盖
                                mnManufacturer = edid_manufacturer
                                # 同时确保名称也不被覆盖
                                if len(edid_name) > 0:
                                    mnName = edid_name
                        elif "Vendor" in mapInfo:
                            # 如果只有 Vendor 信息，且 EDID 中没有制造商，使用它
                            # 优先检查 edid_manufacturer，确保 EDID 解析的厂商信息不被覆盖
                            if len(edid_manufacturer) == 0 and len(mnManufacturer) == 0 and isinstance(mapInfo["Vendor"], str):
                                mnManufacturer = mapInfo["Vendor"]
                            elif len(edid_manufacturer) > 0:
                                # 确保 EDID 解析的厂商信息不被覆盖
                                mnManufacturer = edid_manufacturer
                    elif isinstance(hwinfoList[nIndex], list):
                        # 如果是列表，遍历列表中的每个字典
                        for mapInfo in hwinfoList[nIndex]:
                            if isinstance(mapInfo, dict):
                                if "Model" in mapInfo and "Vendor" in mapInfo:
                                    # 如果 EDID 中没有获取到制造商信息，使用 hwinfoList 中的 Vendor
                                    # 优先检查 edid_manufacturer，确保 EDID 解析的厂商信息不被覆盖
                                    if len(edid_manufacturer) == 0 and len(mnManufacturer) == 0:
                                        if "Vendor" in mapInfo and isinstance(mapInfo["Vendor"], str):
                                            mnManufacturer = mapInfo["Vendor"]
                                    elif len(edid_manufacturer) > 0:
                                        # 确保 EDID 解析的厂商信息不被覆盖
                                        mnManufacturer = edid_manufacturer
                                        break  # 已经使用 EDID 的厂商信息，不需要继续遍历
                                    
                                    # 如果 EDID 中没有获取到名称信息，使用 hwinfoList 中的 Model
                                    # 优先检查 edid_name，确保 EDID 解析的名称不被覆盖
                                    if len(edid_name) == 0 and len(mnName) == 0:
                                        if "Model" in mapInfo and isinstance(mapInfo["Model"], str):
                                            hwinfo_model = mapInfo["Model"].split("\"")
                                            if len(hwinfo_model) > 1:
                                                hwinfo_model = hwinfo_model[0]
                                            else:
                                                hwinfo_model = mapInfo["Model"]
                                            if len(hwinfo_model) > 0:
                                                mnName = hwinfo_model
                                    elif len(edid_name) > 0:
                                        # 如果 EDID 解析到了名称，确保不被覆盖
                                        mnName = edid_name
                                        break  # 已经使用 EDID 的名称信息，不需要继续遍历
                                    
                                    # 如果 EDID 中没有获取到型号信息，使用 hwinfoList 中的 Model
                                    if len(mnModel) == 0:
                                        if "Model" in mapInfo and isinstance(mapInfo["Model"], str):
                                            hwinfo_model = mapInfo["Model"].split("\"")
                                            if len(hwinfo_model) > 1:
                                                hwinfo_model = hwinfo_model[0]
                                            else:
                                                hwinfo_model = mapInfo["Model"]
                                            if len(hwinfo_model) > 0:
                                                mnModel = hwinfo_model
                                    
                                    # 如果制造商匹配，且名称为空，也可以使用 hwinfoList 中的 Model
                                    # 但前提是 EDID 中没有解析到厂商信息，或者 EDID 解析的厂商和 hwinfo 的厂商匹配
                                    # 如果 EDID 解析到了厂商信息，优先使用 EDID 的厂商，不要被 hwinfo 覆盖
                                    if len(edid_manufacturer) == 0 and len(mnManufacturer) > 0 and "Vendor" in mapInfo and isinstance(mapInfo["Vendor"], str) and mnManufacturer == mapInfo["Vendor"]:
                                        # 优先检查 edid_name，确保 EDID 解析的名称不被覆盖
                                        if len(edid_name) == 0 and len(mnName) == 0 and "Model" in mapInfo and isinstance(mapInfo["Model"], str):
                                            hwinfo_model = mapInfo["Model"].split("\"")
                                            if len(hwinfo_model) > 1:
                                                hwinfo_model = hwinfo_model[0]
                                            else:
                                                hwinfo_model = mapInfo["Model"]
                                            if len(hwinfo_model) > 0:
                                                mnName = hwinfo_model
                                        elif len(edid_name) > 0:
                                            # 如果 EDID 解析到了名称，确保不被覆盖
                                            mnName = edid_name
                                    elif len(edid_manufacturer) > 0:
                                        # 如果 EDID 解析到了厂商信息，确保不被覆盖
                                        mnManufacturer = edid_manufacturer
                                        # 同时确保名称也不被覆盖
                                        if len(edid_name) > 0:
                                            mnName = edid_name
                                        break  # 已经使用 EDID 的厂商信息，不需要继续遍历
                                elif "Vendor" in mapInfo:
                                    # 如果只有 Vendor 信息，且 EDID 中没有制造商，使用它
                                    # 优先检查 edid_manufacturer，确保 EDID 解析的厂商信息不被覆盖
                                    if len(edid_manufacturer) == 0 and len(mnManufacturer) == 0 and isinstance(mapInfo["Vendor"], str):
                                        mnManufacturer = mapInfo["Vendor"]
                                    elif len(edid_manufacturer) > 0:
                                        # 确保 EDID 解析的厂商信息不被覆盖
                                        mnManufacturer = edid_manufacturer
                                        break  # 已经使用 EDID 的厂商信息，不需要继续遍历
                # 最终赋值前，确保优先使用 EDID 解析的厂商信息和名称
                # 这是最后的保障，确保 EDID 解析的信息不会被覆盖
                if len(edid_manufacturer) > 0:
                    # 优先使用 EDID 解析的厂商信息
                    monitorOne[MNI_MANUFACTURER] = edid_manufacturer
                    # 同时更新 mnManufacturer，确保一致性
                    mnManufacturer = edid_manufacturer
                elif len(mnManufacturer) > 0 :
                    monitorOne[MNI_MANUFACTURER] = mnManufacturer
                # 优先使用 EDID 解析的名称信息
                if len(edid_name) > 0:
                    # 优先使用 EDID 解析的名称信息
                    monitorOne[MNI_NAME] = edid_name
                    # 同时更新 mnName，确保一致性
                    mnName = edid_name
                elif len(mnName) > 0 :
                    monitorOne[MNI_NAME] = mnName
                if len(mnSize) > 0 :
                    monitorOne[MNI_SIZE] = mnSize
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
