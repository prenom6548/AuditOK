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

import json
import subprocess

from pprint import pprint

from kajsondef import *
from kathread import *
from utils import *

subsystem_id = "LANG=en lspci -d 0731:7200 -v | sed -n '/Subsystem:/p' | awk -F: '{ print $NF }'"
subsystem_id_old = "lspci -d 0731:7200 | awk -F: '{ print $NF }'"        #兼容旧驱动
subsystem_id_re = re.compile(r'Subsystem:(.*?\d{3,5})')
subsystem_id_re_old = re.compile(r'.*')

def get_interface(com, pci_str):
    "输入想要的命令，并获取想要内容的函数,第一个参数是命令，第二个参数是正则表达式"
    res = subprocess.Popen(com, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, bufsize=10)
    inter1 = res.stdout.read()
    inter = inter1.decode('utf-8')
    result = inter
    return result

class KAGraphicsCardInfo() :

    def __init__(self, cmdtool):
        self.cmdTool = cmdtool
        pass

    def get_display_hw990(self):
        displayInfo = {}
        wayland_sock = glob.glob('/run/user/*/wayland-0')[0]
        xdg_runtime_dir = wayland_sock[:-10] # /run/user/1000
        gpuinfo_env = {'XDG_RUNTIME_DIR' : xdg_runtime_dir}
        # 判断是否有gpuinfo命令
        try:
            process = subprocess.run('gpuinfo', env=gpuinfo_env, capture_output=True)
            output = process.stdout.decode()
            ret = process.returncode
        except:
            return displayInfo
            
        lines = output.strip().split('\n')
        for line in lines:
            if line.find(':') > 0:
                if line.find('GPU vendor') >= 0: # GPU vendor: ARM
                    displayInfo[GSI_MANUFACTURER] = line.split(':')[1].strip()
                elif line.find('GPU type') >= 0: # GPU type: Mali-G76
                    displayInfo[GSI_NAME] = line.split(':')[1].strip()
                elif "GDDR capacity" in line:
                    displayInfo[GSI_GDDRCAPACITY] = line.split(':')[1].strip()
                elif "EGL version" in line:
                    displayInfo[GSI_EGLVERSION] = line.split(':')[1].strip()
                elif "EGL client APIs" in line:
                    displayInfo[GSI_EGLAPIS] = line.split(':')[1].strip()
                elif "GL version" in line:
                    displayInfo[GSI_GLVERSION] = line.split(':')[1].strip()
                elif "GLSL version" in line:
                    displayInfo[GSI_GLSLVERSION] = line.split(':')[1].strip()
                else:
                    pass
            else: # integrated graphics controller
                displayInfo[GSI_DESC] = line
        return displayInfo

    def getGraphicsCardInfo(self) :
        summary = {"list":[]}

        self.cmdTool.loadGPUInfoFromDmesg(False)
        self.cmdTool.loadInfoFromLshw(False)
        self.cmdTool.loadGPUInfoFromNvidiaSmi(False)
        graphicsCardList = self.cmdTool.lshwDisplayList
        cardList = self.cmdTool.loadInfoFromHwinfo("--display")["list"]
        length = len(graphicsCardList)
        for i in range(length) :
            if "bus info" in graphicsCardList[i] :
                words = graphicsCardList[i]["bus info"].split("@")
                if len(words) != 2 :
                    continue
                key = words[1].strip()
                #key = key.replace(".","" in mapInfo 
                for mapInfo in cardList :
                    if "SysFS BusID" in mapInfo :
                        if key != mapInfo["SysFS BusID"] :
                            continue
                        graphicsCardList[i].update(mapInfo)
                result = key.split(":", 1)
                if result and len(result) == 2 :
                    key = result[1]
                    for mapInfo in self.cmdTool.nvidiaGpuList :
                        if "nvidia_id" in mapInfo :
                            if key != mapInfo["nvidia_id"] :
                                continue
                            graphicsCardList[i].update(mapInfo)
            if len(self.cmdTool.dmesgGPUList) > 0 :
                graphicsCardList[i].update(self.cmdTool.dmesgGPUList[0])

        index = 0
        for mapInfo in graphicsCardList :
            if len(mapInfo) > 0 :
                pciAddr = ""
                summary["list"].append({})
                isJJM = False
                isLingjiu = False
                isInnosilicon = False
                isIcube = False
                ##名称
                if "product" in mapInfo :
                    summary["list"][index][GSI_NAME] = mapInfo["product"]

                ##制造商
                if "Vendor" in mapInfo :
                    cdVendor = mapInfo["Vendor"].split("\"")
                    if len(cdVendor) > 1 :
                        summary["list"][index][GSI_MANUFACTURER] = cdVendor[1]
                    else :
                        summary["list"][index][GSI_MANUFACTURER] = mapInfo["Vendor"]
                if "SubVendor" in mapInfo :
                    cdVendor = mapInfo["SubVendor"].split("\"")
                    if len(cdVendor) > 1 :
                        summary["list"][index][GSI_MANUFACTURER] = cdVendor[1]
                    else :
                        summary["list"][index][GSI_MANUFACTURER] = mapInfo["SubVendor"]
                if "vendor" in mapInfo :
                    summary["list"][index][GSI_MANUFACTURER] = mapInfo["vendor"]

                    # JJW显卡的特殊处理
                    tmpVendor = mapInfo["vendor"]
                    if str(tmpVendor).lower().find("jingjia") != -1:
                        isJJM = True
                        tmpResult = get_interface(subsystem_id, subsystem_id_re)
                        summary["list"][index][GSI_NAME] = tmpResult.strip()
                        if not tmpResult:
                            summary["list"][index][GSI_NAME] = get_interface(subsystem_id_old, subsystem_id_re_old).strip()

                    # 凌久
                    if str(tmpVendor).lower().find("lingjiu") != -1:
                        isLingjiu = True

                    # 芯动显卡的特殊处理
                    if str(tmpVendor).lower().find("innosilicon") != -1:
                        isInnosilicon = True

                    # 中微电显卡的特殊处理
                    if str(tmpVendor).lower().find("icube") != -1:
                        isIcube = True


                ##类型
                if "description" in mapInfo :
                    summary["list"][index][GSI_DESC] = mapInfo["description"]

                ##版本
                if "version" in mapInfo :
                    summary["list"][index][GSI_VERSION] = mapInfo["version"]

                ##总线信息
                if "bus info" in mapInfo :
                    summary["list"][index][GSI_DBUSINFO] = mapInfo["bus info"]
                    pciResult = re.findall(r"pci@(\S+)", mapInfo["bus info"])
                    if pciResult :
                        pciAddr = pciResult[0]

                ##功能
                if "capabilities" in mapInfo :
                    summary["list"][index][GSI_FUNCTION] = mapInfo["capabilities"]

                ##位宽 (不准确，暂时不显示)
                # if "width" in mapInfo :
                #     summary["list"][index][GSI_BITWIDTH] = mapInfo["width"]

                ##物理ID sw机器物理id 普通用户和root不一致
                if "physical id" in mapInfo and not Judgment_SW():
                    summary["list"][index][GSI_PHYSICS] = mapInfo["physical id"]

                ##时钟频率
                if "clock" in mapInfo :
                    summary["list"][index][GSI_CLOCK] = mapInfo["clock"]
                ##NVIDIA显卡
                if "nvidia_maxclock" in mapInfo :
                    summary["list"][index][GSI_CLOCK] = mapInfo["nvidia_maxclock"]

                ##中断
                if "irq" in mapInfo :
                    summary["list"][index][GSI_IRQ] = mapInfo["irq"]

                ##内存
                #if "memory" in mapInfo :
                #    summary["list"][index][GSI_MEMORY] = mapInfo["memory"]

                ##io端口
                if "ioport" in mapInfo :
                    summary["list"][index][GSI_IOPORT] = mapInfo["ioport"]

#               ##驱动
#               if "driver" in mapInfo :
#                   summary["list"][index][GSI_DRIVER] = mapInfo["driver"]

                ##延迟
                if "latency" in mapInfo :
                    summary["list"][index][GSI_DELAY] = mapInfo["latency"]

                ##显存 景嘉微和凌久都通过gpuinfo_0获取显存（优先）
                gpuinfo_path = "/proc/gpuinfo_{}".format(index)
                
                if os.path.exists(gpuinfo_path):
                    args4 = ["cat", gpuinfo_path]
                    pipe4 = subprocess.Popen(args4, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                    output4 = pipe4.stdout.readlines()

                    for line in [ bytes.decode(l.strip(),"utf-8","ignore") for l in output4]:
                        if "Memory Size" in line:
                            results = line.strip().split(":")
                            if len(results) == 2 :
                                temSize = results[1].strip()
                                summary["list"][index][GSI_CAPCITY] = temSize.replace(" ","")

                        if "Vendor Name" in line:
                            results = line.strip().split(":")
                            if len(results) == 2 :
                                vendor = results[1].strip()
                                summary["list"][index][GSI_MANUFACTURER] = vendor.replace(" ","")

                        if "Product Name" in line:
                            results = line.strip().split(":")
                            if len(results) == 2 :
                                model = results[1].strip()
                                summary["list"][index][GSI_MODEL] = model.replace(" ","")
                                summary["list"][index][GSI_DESC] = model.replace(" ","")

                # JJW显卡：如果通过gpuinfo没有获取到显存，尝试通过dmesg获取
                if isJJM and GSI_CAPCITY not in summary["list"][index] and len(pciAddr) > 0:
                    try:
                        cmd = "dmesg | grep {} | grep VRAM".format(pciAddr)
                        result = get_interface(cmd, None)
                        if result:
                            # 解析类似 "[    1.483349] mwv207 0000:81:00.0: 4096M VRAM, 128M aperture, 128M visible VRAM"
                            vram_match = re.search(r'(\d+)M\s+VRAM', result)
                            if vram_match:
                                vram_size_mb = int(vram_match.group(1))
                                if vram_size_mb >= 1024:
                                    summary["list"][index][GSI_CAPCITY] = str(float("%0.1f" % (float(vram_size_mb) / 1024))) + "GB"
                                else:
                                    summary["list"][index][GSI_CAPCITY] = str(vram_size_mb) + "MB"
                    except:
                        pass

                if isInnosilicon and os.path.exists("/sys/bus/platform/devices/sysdbg.0/gpu_static_info"):
                    args5 = ["cat", "/sys/bus/platform/devices/sysdbg.0/gpu_static_info"]
                    pip5 = subprocess.Popen(args5, stdout=subprocess.PIPE ,stderr = subprocess.PIPE)
                    output5 = pip5.stdout.readlines()

                    for line in [ bytes.decode(l.strip(),"utf-8","ignore") for l in output5]:
                        if "memory total size" not in line:
                            continue

                        results = line.strip().split(":")
                        if len(results) == 2 :
                            temSize = results[1].strip()
                            summary["list"][index][GSI_CAPCITY] = temSize.replace(" ","")
                
                if isIcube and os.path.exists("/sys/kernel/debug/dri/0/total_vram"):
                    args6 = ["cat", "/sys/kernel/debug/dri/0/total_vram"]
                    pip6 = subprocess.Popen(args6, stdout=subprocess.PIPE ,stderr = subprocess.PIPE)
                    output6 = pip6.stdout.readlines()

                    for line in [ bytes.decode(l.strip(),"utf-8","ignore") for l in output6]:
                        if not line:
                            continue
                        else:
                            summary["list"][index][GSI_CAPCITY] = line
                
                if (GSI_NAME in summary["list"][index] and "X100" in summary["list"][index][GSI_NAME]) or \
                    (GSI_MANUFACTURER in summary["list"][index] and "X100" in summary["list"][index][GSI_MANUFACTURER]):    ## X100
                    if os.path.exists("/sys/kernel/debug/pvr/physical_vram_size"):
                        args3 = ["cat", "/sys/kernel/debug/pvr/physical_vram_size"]
                        pipe3 = subprocess.Popen(args3, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                        output3 = pipe3.stdout.readlines()
                        totalMem = 0
                        for line in [ bytes.decode(l.strip(),"utf-8","ignore") for l in output3]:
                            if line.strip().isdigit():
                                totalMem = int(line)
                        if totalMem > 0 :
                            # 单位为字节 转换成 MB
                            totalMem /= 1024*1024
                        if totalMem > 1024 :
                            summary["list"][index][GSI_CAPCITY] = str(float("%0.1f"%(float(totalMem)/1024)))+"GB"
                        else :
                            summary["list"][index][GSI_CAPCITY] = str(totalMem)+"MB"
                    else:
                        totalMem = 0
                        for line in [ bytes.decode(l.strip(),"utf-8","ignore") for l in output3]:
                            if "1db7:dc20" not in line and "1db7:dc21" not in line and "1db7:dc22" not in line :
                                continue
                            infoItems = line.split(" ")
                            if len(infoItems) != 3 :
                                continue
                            args4 = ["lspci", "-vs"]
                            args4.append(infoItems[0].strip())
                            pipe4 = subprocess.Popen(args4, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                            output4 = pipe4.stdout.readlines()
                            singleMem = 0
                            for line1 in [ bytes.decode(l.strip(),"utf-8","ignore") for l in output4]:
                                    if "Memory" not in line1:
                                        continue

                                    results = [ v.strip().strip("\)") for v in line1.split() ]
                                    if "prefetchable" in results :
                                        ## Memory at a0000000 (64-bit, prefetchable) [size=256M]
                                        for session in results :
                                            if "size" not in session :
                                                continue
                                            strMem = session.strip("[").strip("]").split("=")
                                            if len(strMem) == 2 :
                                                sizeN = re.findall("([0-9]+)M.*",strMem[1])
                                                if sizeN and len(sizeN) > 0 and sizeN[0].isdigit() :
                                                    if int(sizeN[0]) > singleMem :
                                                        singleMem = int(sizeN[0])
                                                sizeN = re.findall("([0-9]+)G.*",strMem[1])
                                                if sizeN and len(sizeN) > 0 and sizeN[0].isdigit() :
                                                    if int(sizeN[0])*1024 > singleMem :
                                                        singleMem = int(sizeN[0])*1024
                        totalMem += singleMem
                        if totalMem > 0 :
                            if totalMem > 1024 :
                                summary["list"][index][GSI_CAPCITY] = str(float("%0.1f"%(float(totalMem)/1024)))+"GB"
                            else :
                                summary["list"][index][GSI_CAPCITY] = str(totalMem)+"MB"
                # 摩尔线程
                if (GSI_MANUFACTURER in summary["list"][index] and "Moore" in summary["list"][index][GSI_MANUFACTURER]): 
                    if len(pciAddr) > 0 :
                        args3 = ["lspci", "-vs"]
                        args3.append(pciAddr)
                        pipe3 = subprocess.Popen(args3, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                        output3 = pipe3.stdout.readlines()
                        totalMem = 0

                        for line in [ bytes.decode(l.strip(),"utf-8","ignore") for l in output3]:
                            if "Memory" not in line:
                                continue
                        
                            results = [ v.strip().strip("\)") for v in line.split() ]
                            if "prefetchable" in results and len(results) >=6 :
                                ## Memory at a0000000 (64-bit, prefetchable) [size=256M]
                                strMem = results[5].strip("[").strip("]").split("=")
                                if len(strMem) == 2 :
                                    sizeN = re.findall("([0-9]+)M.*",strMem[1])
                                    if sizeN and len(sizeN) > 0 and sizeN[0].isdigit() :
                                        if int(sizeN[0]) > totalMem :
                                            totalMem = int(sizeN[0])
                                    sizeN = re.findall("([0-9]+)G.*",strMem[1])
                                    if sizeN and len(sizeN) > 0 and sizeN[0].isdigit() :
                                        if int(sizeN[0])*1024 > totalMem :
                                            totalMem = int(sizeN[0])*1024

                        if totalMem > 0 :
                            if totalMem > 1024 :
                                summary["list"][index][GSI_CAPCITY] = str(float("%0.1f"%(float(totalMem)/1024)))+"GB"
                            else :
                                summary["list"][index][GSI_CAPCITY] = str(totalMem)+"MB"

                if GSI_CAPCITY not in summary["list"][index] :
                    if "size" in mapInfo and len(mapInfo["size"]) > 0:
                        summary["list"][index][GSI_CAPCITY] = mapInfo["size"]
                    else :
                        if len(pciAddr) > 0 :
                            args3 = ["lspci", "-vs"]
                            args3.append(pciAddr)
                            pipe3 = subprocess.Popen(args3, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                            output3 = pipe3.stdout.readlines()
                            totalMem = 0

                            for line in [ bytes.decode(l.strip(),"utf-8","ignore") for l in output3]:
                                if "Memory" not in line:
                                    continue
                            
                                results = [ v.strip().strip("\)") for v in line.split() ]
                                if "prefetchable" in results and len(results) >=6 :
                                    ## Memory at a0000000 (64-bit, prefetchable) [size=256M]
                                    strMem = results[5].strip("[").strip("]").split("=")
                                    if len(strMem) == 2 :
                                        sizeN = re.findall("([0-9]+)M.*",strMem[1])
                                        if sizeN and len(sizeN) > 0 and sizeN[0].isdigit() :
                                            if int(sizeN[0]) > totalMem :
                                                totalMem = int(sizeN[0])
                                        sizeN = re.findall("([0-9]+)G.*",strMem[1])
                                        if sizeN and len(sizeN) > 0 and sizeN[0].isdigit() :
                                            if int(sizeN[0])*1024 > totalMem :
                                                totalMem = int(sizeN[0])*1024

                            if totalMem > 0 :
                                for lines in output3:
                                    if "Zhaoxin".encode('utf-8') in lines:
                                        with open("/sys/class/drm/card0/device/zx_info/fb_size", 'r') as file:
                                            content = file.read().strip()
                                            if '4096' in content:
                                                summary["list"][index][GSI_CAPCITY] = "4GB"
                                                break
                                            if '2048' in content :
                                                summary["list"][index][GSI_CAPCITY] = "2GB"
                                                break
                                            if '1024' in content:
                                                summary["list"][index][GSI_CAPCITY] = "1G"
                                                break
                                            if '512' in content:
                                                summary["list"][index][GSI_CAPCITY] = "512MB"
                                                break
                                            if '256' in content:
                                                summary["list"][index][GSI_CAPCITY] = "256MB"
                                                break
                                            if '128' in content:
                                                summary["list"][index][GSI_CAPCITY] = "128MB"
                                                break
                                if totalMem >= 1024 and GSI_CAPCITY not in summary["list"][index]:
                                    summary["list"][index][GSI_CAPCITY] = str(float("%0.1f"%(float(totalMem)/1024)))+"GB"
                                elif totalMem < 1024 and GSI_CAPCITY not in summary["list"][index]:
                                    summary["list"][index][GSI_CAPCITY] = str(totalMem)+"MB"
                if "nvidia_mem" in mapInfo and len(mapInfo["nvidia_mem"]) > 0 :
                    summary["list"][index][GSI_CAPCITY] = mapInfo["nvidia_mem"]

                #显存类型
                if GSI_CAPCITY in summary["list"][index] and "mem_type" in mapInfo and len(mapInfo["mem_type"]) > 0 :
                    summary["list"][index][GSI_MEMTYPE] = mapInfo["mem_type"]

                #子制造商和VID/DID/SUBVID/SUBDID
                if len(pciAddr) > 0 :
                    # 从sysfs读取vid、did、subvid、subdid
                    pci_sysfs_base = "/sys/bus/pci/devices/{}".format(pciAddr)
                    vid_path = "{}/vendor".format(pci_sysfs_base)
                    did_path = "{}/device".format(pci_sysfs_base)
                    subvid_path = "{}/subsystem_vendor".format(pci_sysfs_base)
                    subdid_path = "{}/subsystem_device".format(pci_sysfs_base)
                    
                    # 读取vid
                    if os.path.exists(vid_path):
                        try:
                            with open(vid_path, 'r') as f:
                                vid = f.read().strip()
                                # 转换为16进制格式（去掉0x前缀，转为小写）
                                if vid.startswith("0x") or vid.startswith("0X"):
                                    vid = vid[2:].lower()
                                else:
                                    # 如果是十进制，转换为16进制
                                    try:
                                        vid_int = int(vid)
                                        vid = format(vid_int, '04x')
                                    except ValueError:
                                        vid = vid.lower()
                                summary["list"][index][GSI_VID] = vid
                        except Exception:
                            pass
                    
                    # 读取did
                    if os.path.exists(did_path):
                        try:
                            with open(did_path, 'r') as f:
                                did = f.read().strip()
                                # 转换为16进制格式（去掉0x前缀，转为小写）
                                if did.startswith("0x") or did.startswith("0X"):
                                    did = did[2:].lower()
                                else:
                                    # 如果是十进制，转换为16进制
                                    try:
                                        did_int = int(did)
                                        did = format(did_int, '04x')
                                    except ValueError:
                                        did = did.lower()
                                summary["list"][index][GSI_DID] = did
                        except Exception:
                            pass
                    
                    # 读取subvid
                    if os.path.exists(subvid_path):
                        try:
                            with open(subvid_path, 'r') as f:
                                subvid = f.read().strip()
                                # 转换为16进制格式（去掉0x前缀，转为小写）
                                if subvid.startswith("0x") or subvid.startswith("0X"):
                                    subvid = subvid[2:].lower()
                                else:
                                    # 如果是十进制，转换为16进制
                                    try:
                                        subvid_int = int(subvid)
                                        subvid = format(subvid_int, '04x')
                                    except ValueError:
                                        subvid = subvid.lower()
                                summary["list"][index][GSI_SUBVID] = subvid
                        except Exception:
                            pass
                    
                    # 读取subdid
                    if os.path.exists(subdid_path):
                        try:
                            with open(subdid_path, 'r') as f:
                                subdid = f.read().strip()
                                # 转换为16进制格式（去掉0x前缀，转为小写）
                                if subdid.startswith("0x") or subdid.startswith("0X"):
                                    subdid = subdid[2:].lower()
                                else:
                                    # 如果是十进制，转换为16进制
                                    try:
                                        subdid_int = int(subdid)
                                        subdid = format(subdid_int, '04x')
                                    except ValueError:
                                        subdid = subdid.lower()
                                summary["list"][index][GSI_SUBDID] = subdid
                        except Exception:
                            pass
                    
                    # 从lspci获取子制造商信息（保持原有逻辑）
                    args3 = ["lspci", "-vvs"]
                    args3.append(pciAddr)
                    pipe3 = subprocess.Popen(args3, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                    output3 = pipe3.stdout.readlines()
                    subSystemInfo = ""
                    pciStatusClock = ""

                    for line in [ bytes.decode(l.strip(),"utf-8","ignore") for l in output3]:
                        words = line.split(":")
                        if len(words) == 2 :
                            keyWord = words[0].strip()
                            valWord = words[1].strip()
                            if keyWord == "Subsystem" :
                                subSystemInfo = valWord
                            # 采用lshw 里面的显卡时钟频率，不采用pci里面的
                            # elif keyWord == "Status" :
                            #     result = re.findall("[\s]*([0-9]+MHz)-.*", valWord)
                            #     if result and len(result) > 0 :
                            #         pciStatusClock = result[0]
                    if len(subSystemInfo) > 0 :
                        summary["list"][index][GSI_SUBSYSTEM] = subSystemInfo
                    if len(pciStatusClock) > 0 :
                        summary["list"][index][GSI_CLOCK] = pciStatusClock

                ##型号
                if "Model" in mapInfo and GSI_MODEL not in summary["list"][index]:
                    summary["list"][index][GSI_MODEL] = mapInfo["Model"].strip("\"")
                    if GSI_NAME in summary["list"][index] :
                        if GSI_MANUFACTURER in summary["list"][index] and \
                            summary["list"][index][GSI_MANUFACTURER] != summary["list"][index][GSI_NAME] :
                            summary["list"][index][GSI_MODEL] = summary["list"][index][GSI_NAME]
                    if "vendor" in mapInfo:
                        # subsytem 放到 product里面（Jingjia Microelectronics Co Ltd JM7201）--- 根据制造商裁剪型号，作为设备名称和型号
                        if get_url(mapInfo["vendor"],mapInfo["vendor"]).find("jingjia") != -1:
                            tmpName = summary["list"][index][GSI_SUBSYSTEM].split(summary["list"][index][GSI_MANUFACTURER])[-1].strip();
                            summary["list"][index][GSI_NAME] = tmpName;
                            summary["list"][index][GSI_MODEL] = tmpName;
                else :
                    if GSI_NAME in summary["list"][index] :
                        summary["list"][index][GSI_MODEL] = summary["list"][index][GSI_NAME]

                ##NVIDIA显卡
                if "nvidia_name" in mapInfo :
                    summary["list"][index][GSI_MODEL] = mapInfo["nvidia_name"]

                if isLingjiu :
                    vid = mapInfo["Vendor"].split('0x')[-1]
                    device_filter = f"{vid}::0300"
                    cmd = ["lspci", "-d", device_filter, "-m", "-v"]
                    output = subprocess.run(
                        cmd,
                        check=True,
                        stdout=subprocess.PIPE,
                        text=True,
                        encoding="utf-8",
                    ).stdout
                    for line in output.splitlines():
                        if "SDevice" in line:
                            # 按制表符分割字段（-m 模式下为机器可读格式）
                            fields = line.strip().split("\t")
                            summary["list"][index][GSI_MODEL] = fields[1]

                ##配置状态
                if "Config Status" in mapInfo :
                    summary["list"][index][GSI_CONFSTATUS] = mapInfo["Config Status"]

                ##驱动
                if "Driver" in mapInfo :
                    summary["list"][index][GSI_DRIVER] = mapInfo["Driver"].strip("\"")

                ##驱动模块
                if "Driver Modules" in mapInfo :
                    summary["list"][index][GSI_DRIVERMODULE] = mapInfo["Driver Modules"]

                ##设备  hwinfo中获取的device id在更新hwinfo里面的pci.ids后会出现型号的显示，和型号的显示重复故移除
                # if "Device" in mapInfo :
                #     summary["list"][index][GSI_DEVICE] = mapInfo["Device"]

                index += 1
        
        if Judgment_HW990():
            displayList = self.get_display_hw990()
            if len(displayList) > 0 :
                summary["list"].append(displayList)

        if len(summary["list"]) == 0 and os.path.exists("/sys/kernel/debug/gc/info"):
            with open("/sys/kernel/debug/gc/info", 'r') as f:
                content = f.read()
                if "FTG340" in content:
                    cardInfo = {}
                    cardInfo[GSI_MANUFACTURER] = "Phytium"
                    cardInfo[GSI_NAME] = "Phytium FTG340"
                    cardInfo[GSI_MODEL] = "Phytium FTG340"
                    cardInfo[GSI_CAPCITY] = self.get_total_memory_with_unit()
                    cardInfo[GSI_GLVERSION] = "4.6"
                    summary["list"].append(cardInfo)

        module = "graphicsCard"
        if "list" in summary:
            status = KAConfig.getInstance().execute(module,summary["list"])
            if "invisible" == status:
                # summary["list"] = []
                return ""
        return json.dumps(summary)

    #概要信息
    def getOutline(self, customedType) :
        if "" == self.getGraphicsCardInfo():
            return ""
        outLine = self.getGraphicsCardInfo()
        outLineObj = json.loads(outLine)
        outLine = ""
        if 1 == customedType : ## lenovo
            if len(outLineObj) > 0 and "list" in outLineObj :
                nIndex = 0
                for info in outLineObj["list"] :
                    oneOutLine = ""
                    if (GSI_MANUFACTURER in info and len(info[GSI_MANUFACTURER]) > 0) :
                        gaManufacturer = exchangeManufacturer(info[GSI_MANUFACTURER])
                        if gaManufacturer == "AMD" :
                            oneOutLine += gaManufacturer+"-R520"
                        elif gaManufacturer == "NV" :
                            oneOutLine += gaManufacturer+"-GT730"
                        elif gaManufacturer == "JJW" :
                            oneOutLine += gaManufacturer+"-7201"
                        elif gaManufacturer == "GLF" :
                            oneOutLine += gaManufacturer+"-10C0"

                    if len(oneOutLine) > 0 :
                        if nIndex == 0 :
                            outLine += oneOutLine
                        else :
                            outLine += " / " + oneOutLine
                    else :
                        if GSI_NAME in info and len(info[GSI_NAME]) > 0:
                            if nIndex == 0 :
                                outLine += info[GSI_NAME]
                            else :
                                outLine += " / " + info[GSI_NAME]
                        elif MMI_MODEL in info :
                            if nIndex == 0 :
                                outLine += info[MMI_MODEL]
                            else :
                                outLine += " / " + info[MMI_MODEL]
                    nIndex = nIndex + 1
        else :
            if len(outLineObj) > 0 and "list" in outLineObj :
                nIndex = 0
                for info in outLineObj["list"] :
                    if GSI_NAME in info and len(info[GSI_NAME]) > 0:
                        if nIndex == 0 :
                            outLine += info[GSI_NAME]
                        else :
                            outLine += " / " + info[GSI_NAME]
                    elif MMI_MODEL in info :
                            if nIndex == 0 :
                                outLine += info[MMI_MODEL]
                            else :
                                outLine += " / " + info[MMI_MODEL]
                    nIndex = nIndex + 1
        return outLine

    def get_total_memory_with_unit(self):
        with open('/sys/kernel/debug/gc/total_mem', 'r') as file:
            content = file.read().strip()
            if '0x' in content:
                hex_str = content.split('0x')[-1].split()[0]  # 提取十六进制部>分
                mb = int(hex_str, 16) / (1024 * 1024)
                gb = mb / 1024
                return f"{gb:.1f} GB"
            else:
                mb = content.split(':')[-1].split()[0]  # 提取十六进制部分
                gb = int(mb) / 1024
                return f"{gb:.1f} GB"

if __name__ == "__main__":
    cmdTool = KACmdTool()
    cmdTool.loadInfoFromLshw()
    cc = KAGraphicsCardInfo(cmdTool)
    pprint(cc.getGraphicsCardInfo())
    #pprint(cc.getOutline())
