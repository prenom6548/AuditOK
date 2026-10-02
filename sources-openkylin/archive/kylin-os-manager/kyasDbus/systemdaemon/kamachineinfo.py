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
import platform
import binascii

import json
import subprocess

from pprint import pprint

from kajsondef import *
from kathread import *
from systemboardthread import *

from karealization import DetailInfo
from memoryinfothread import MemoryInfo
from networkadaptorthread import NetworkAdaptor
from audioadaptorthread import AudioAdaptor
from kamonitorthread import KAMonitor
from kaharddiskinfo import KAHarddiskInfo
from kagraphicscardinfo import KAGraphicsCardInfo
from systemboardthread import *
from kakeyboardinfo import KAKeyboardInfo
from kamouseinfo import KAMouseInfo
from kacdrominfo import KACDRomInfo

class KAMachineInfo():

    def __init__(self, cmdtool):
        self.osname = self.read_os_name()
        self.cmdTool = cmdtool
        pass

    def ctoascii(self,buf):
        ch = bytes(buf.encode('utf-8'))
        asci = binascii.b2a_hex(ch)
        asci = int(asci,16)
        return asci

    def strip(self, s):
        if(len(s) == 0):
            return s
        while len(s) > 0 and self.ctoascii(s[0]) <= 32:
            s = s[1: ]
        k = len(s)
        while len(s) > 0 and self.ctoascii(s[k-1]) <= 32:
            s = s[ :k-1]
            k = len(s)
        i = 0
        while i < len(s):
            if self.ctoascii(s[i]) < 32:
                s = s[ :i] + s[i+1: ]
                i -= 1
            i += 1
        return s

    def get_url(self,v,p):
        vendors = {
#CPU产商
           "INTEL":["Intel"],
           "AMD":["AMD"],
           "VIMICRO":["Vimicro"],
#显卡产商
           "ATI":["ATI"],
           "1002":["ATI"],
           "SIS":["SIS"],
           "1039":["SIS"],
           "NVIDIA":["Nvidia"],
           "VIA":["VIA"],
           "XFX":["XFX"],
           "SUPERGRAPHIC":["Supergraphic"],
           "JINGJIA":["JJM"],
           "Wuhan Digital Engineering":["WDE"],
#显示器产商
           "AUO":["AUO"],
           "AOC":["AOC"],
           "PHILIPS":["Philips"],
           "PHL":["Philips"],
           "LEN":["Lenovo"],
           "SEC":["SAMSUNG"],
#电脑品牌
           "HASEE":["Hasee"],
           "FOUNDER":["Founder"],
           "TONGFANG":["Tongfang"],
           "TSINGHUA":["Tongfang"],
           "ACER":["Acer"],
           "LENOVO":["Lenovo"],
           "ASUSTEK":["ASUS"],
           "NEC":["NEC"],
           "HP":["HP"],
           "HEWLETT-PACKARD":["HP"],
           "SAMSUNG":["SAMSUNG"],
           "TOSHIBA":["TOSHIBA"],
           "APPLE":["Apple"],
           "DELL":["DELL"],
           "FUJITSU":["FUJITSU"],
           "PANASONIC":["Panasonic"],
           "SONY":["SONY"],
           "IBM":["IBM"],
#虚拟机
           "INNOTEK":["VirtualBox"],
           "VBOX":["VirtualBox"],
           "VIRTUALBOX":["VirtualBox"],
#网卡产商
           "3COM":["3COM"],
           "D-LINK":["D-LINK"],
           "RALINK":["Ralink"],
           "ATHEROS":["Atheros"],
           "MARVELL":["Marvell"],
           "BROADCOM":["Broadcom"],
#硬盘产商
           "EXCELSTOR":["Excelstor"],
           "HITACHI":["Hitachi"],
           "MAXTOR":["Maxtor"],
           "WESTERN":["Western Digital"],
           "LITEON":["Liteon"],
           "SEAGATE":["Seagate"],
           "QUANTUM":["Quantum"],
#光驱产商
           "PLDS":["PLDS"],
           "PBDS":["PLDS"],
           "HL-DT-ST":["LG"],
           "OPTIARC":["SONY"],
           "TSSTCORP":["TSSTcorp"],
           "PIONEER":["Pioneer"],
           "MATSHITA":["Panasonic"],
#声卡产商
           "REALTEK":["Realtek"],
           "CREATIVE":["Creative"],
           "LOONGSON":["Loongson"],
           "HISILICON":["HiSilicon"],
#摄像头
           "SONIX":["Sonix"],
           "ETRON":["Etron"],
           "AVEO":["Aveo"],
           "SYNTEK":["Syntek"],
           "EMPIA":["Empia"],
           "CHICONY":["Chicony"],
           "OMNIVISION":["OmniVision"],
#鼠标产商
           "LOGITECH":["Logitech"],
           "SUNPLUS":["Sunplus"],
           "PRIMAX":["Primax"],
           "PIXART":["Pixart"],
           "TRUST":["Trust"],
           "1BCF":["Rapoo"],
           "AVAGO":["Avago"],
           "MICROSOFT":["Microsoft"],
#键盘产商
           "RAPOO":["Rapoo"],
#主板产商
           "GIGABYTE":["Gigabyte"],
           "BIOSTAR":["Biostar"],
           "COLORFUL":["Colorful"],
           "YESTON":["Yeston"],
#指纹识别
           "UPEK":["Authentec"],
           "AUTHENTEC":["Authentec"],
#闪存产商
           "KINGSTON":["Kingston"],
           "KINGMAX":["Kingmax"],
           "KINGBOX":["Kingbox"],
           "HYNIX":["Hynix"],
           "HYUNDAI":["Hynix"],
           "MICRON":["Micron"],
           "06C1":["Asint"],
           "ADATA":["ADATA"],
           "ZTE":["ZTE"],
           "EAGET":["Eaget"],
           "TEXAS":["Texas Instruments"],
           "MOTOROLA":["Motorola"],
#电源产商
           "SMP":["SMP"],
           "SIMPLO":["SMP"],
#BIOS产商
           "AMERICAN":["AMI"],
           "AWARD":["Phoenix"],
           "PHOENIX":["Phoenix"]
            }
        tmp = v.split(" ")[0]
        tmp = re.findall("([a-zA-Z0-9-]+)", tmp)
        if tmp :
            url = vendors.get(tmp[0].upper()) 
            if url:
                return url[0]
            else :
                k = p.split(" ")[0]
                url = vendors.get(k.upper())
                if url:
                    return url[0]
        else:
            tmp = p.split(" ")[0]
            url = vendors.get(tmp.upper())
            if url:
                return url[0]

        tmp = re.findall("JingJia", p)
        if tmp :
            url = vendors.get(tmp[0].upper())
            if url:
                return url[0]
        tmp = re.findall("Loongson", p, flags=re.IGNORECASE)
        if tmp :
            url = vendors.get(tmp[0].upper())
            if url:
                return url[0]

        tmp = re.findall("Wuhan Digital Engineering", p)
        if tmp :
            url = vendors.get(tmp[0].upper())
            if url:
                return url[0]

        tmp = re.findall("ATI", v)
        if tmp :
            url = vendors.get(tmp[0].upper())
            if url:
                return url[0]
        tmp = re.findall("SIS", v)
        if tmp :
            url = vendors.get(tmp[0].upper())
            if url:
                return url[0]
        tmp = re.findall("Intel", v)
        if tmp :
            url = vendors.get(tmp[0].upper())
            if url:
                return url[0]
        tmp = re.findall("ATI", p)
        if tmp :
            url = vendors.get(tmp[0].upper())
            if url:
                return url[0]
        tmp = re.findall("SIS", p)
        if tmp :
            url = vendors.get(tmp[0].upper())
            if url:
                return url[0]
        tmp = re.findall("Intel", p)
        if tmp :
            url = vendors.get(tmp[0].upper())
            if url:
                return url[0]
        return v

    def read_os_name(self):
        platValue = platform.platform()
        if os.path.exists("/etc/ubuntukylin-release"):
            with open("/etc/ubuntukylin-release", "r") as fsys:
                for line in fsys:
                    if line.startswith("DISTRIB_DESCRIPTION"):
                        tmp = line
                        break
            # kobe: remove '"' and '\n'
            front = tmp.split('=')[1].replace('"', '').replace('\n', '')
            platValue = front
        elif os.path.exists("/etc/os-release"):
            tmp = tmp1 = ""
            # 优先获取version 和 控制面板保持一致
            with open("/etc/os-release", "r" ,encoding="utf-8") as fsys:
                for line in fsys:
                    if line.startswith("VERSION"):
                        tmp = line
                    if tmp != "":
                        platValue = tmp.split('=')[1].replace('"', '').replace('\n', '');
                        return platValue
            with open("/etc/os-release", "r") as fsys:
                for line in fsys:
                    if line.startswith("NAME"):
                        tmp = line
                    if line.startswith("PRETTY_NAME"):
                        tmp1 = line
                    # 跳出循环规则调整
                    if tmp != "" and tmp1 != "":
                        break			
            # kobe: remove '"' and '\n'
            front = tmp.split('=')[1].replace('"', '').replace('\n', '')
            front1 = tmp1.split('=')[1].replace('"', '').replace('\n', '')
            
            if front == "Kylin":
                platValue = front
            else:
                platValue = front1

            if front == "Kylin" or front == "YHKylin":
                with open("/etc/lsb-release", "r") as fp:
                    for line in fp:
                        if line.startswith("DISTRIB_VERSION_TYPE"):
                            tmp = line
                            # kobe: remove '"' and '\n'
                            id = tmp.split('=')[1].replace('"', '').replace('\n', '')
                            if id == "community":
                                platValue = "YHKylin community"
                            else:
                                platValue = front1
                            break
        else:
            community = ""
            id = ""
            with open("/etc/lsb-release", "r") as fp:
                for line in fp:
                    if line.startswith("DISTRIB_ID"):#if line.startswith("DISTRIB_DESCRIPTION"):
                        tmp = line
                        # kobe: remove '"' and '\n'
                        id = tmp.split('=')[1].replace('"', '').replace('\n', '')
                    elif line.startswith("DISTRIB_VERSION_TYPE"):
                        tmp = line
                        # kobe: remove '"' and '\n'
                        community = tmp.split('=')[1].replace('"', '').replace('\n', '')
                    elif line.startswith("DISTRIB_DESCRIPTION"):
                        tmp = line
                        # kobe: remove '"' and '\n'
                        community1 = tmp.split('=')[1].replace('"', '').replace('\n', '')
#                        break
            if id == "Kylin":
                platValue = id
            else:
                platValue = community1

            if id == "Kylin" or id == "YHKylin":
                if community == "community":
                    platValue = "YHKylin community"
                else:
                    platValue = community1

        return platValue

    def getOutline(self):
        summary = {}
        sysBoardInfo = SystemBoard()
        sysInfo = sysBoardInfo.getSystemInfo()
        ComVendor,ComProduct,ComVersion,ComSerial, ComOSName = '','','','',''
        if System_Manufacturer in sysInfo and sysInfo[System_Manufacturer] != "XXXX":
            ComVendor = sysInfo[System_Manufacturer]
        else :
            ComVendor = "Wrong info from firmware"
        
        if System_Product in sysInfo and sysInfo[System_Product] != "XXXX":
            ComProduct = sysInfo[System_Product]
        else :
            ComProduct = "Wrong info from firmware"

        if System_Version in sysInfo and sysInfo[System_Version] != "XXXX":
            ComVersion = sysInfo[System_Version]
        else :
            ComVersion = "Wrong info from firmware"

        if System_SerialNumber in sysInfo and sysInfo[System_SerialNumber] != "XXXX":
            ComSerial = sysInfo[System_SerialNumber]
        else :
            ComSerial = "Wrong info from firmware"
        ComVendor = self.get_url(ComVendor,ComProduct)
        
        ComVendor,ComProduct,ComVersion,ComSerial = self.strip(ComVendor),self.strip(ComProduct),self.strip(ComVersion),self.strip(ComSerial)

        with open('/proc/uptime') as f:
            for line in f:
                string = line.split('.')[0]
                if string.isdigit() :
                    seconds = int(string)
                else :
                    seconds = 0
                minutes = seconds / 60
                uptime = str(minutes)

        ComOSName = self.osname

        summary[MI_MANUFACTURER] = ComVendor
        summary[MI_MACHINEMODEL] = ComProduct
        summary[MI_SERIALNUM] = ComSerial
        summary[MI_VERSION] = ComVersion
        summary[MI_SYSTEMBITS] = platform.architecture()[0]
        summary[MI_KERNELARCH] = platform.machine()
        summary[MI_KERNELVERSION] = platform.release()
        # 采用hostname命令处理，platform.node暂不重启不会进行更新
        output = subprocess.check_output(['hostname'])
        hostname = output.decode('utf-8').strip()
        summary[MI_HOSTNAME] = hostname
        summary[MI_OSVERSION] = ComOSName
        summary[MI_OSTYPES] = platform.system()
        summary[MI_RUNINGTIME] = uptime

        baseBoardInfo = sysBoardInfo.getBaseBoardInfo()
        customedType = 0   ## 0 通用，1 联想 主板名称:
        if Board_Manufacturer in baseBoardInfo and "lenovo" in baseBoardInfo[Board_Manufacturer].lower() \
            and Board_Product in baseBoardInfo and ("FD2000ZX200MB1" in baseBoardInfo[Board_Product]) :
            customedType = 1
        processorInfo = DetailInfo()
        summary[MI_PROCESSOR] = processorInfo.getOutline()
        memInfo = MemoryInfo(self.cmdTool)
        summary[MI_MEMORY] = memInfo.getOutline(customedType)
        motherBoardInfo = SystemBoard()
        summary[MI_MAINBOARD] = motherBoardInfo.getOutline()
        harddiskInfo = KAHarddiskInfo(self.cmdTool)
        summary[MI_HARDDISK] = harddiskInfo.getOutline(customedType)
        graphicsCardInfo = KAGraphicsCardInfo(self.cmdTool)
        summary[MI_GRAPHICSCARD] = graphicsCardInfo.getOutline(customedType)
        monitorInfo = KAMonitor(self.cmdTool)
        summary[MI_MONITOR] = monitorInfo.getOutline()
        networkInfo = NetworkAdaptor(self.cmdTool)
        summary[MI_NETWORKCARD] = networkInfo.getOutline()
        soundInfo = AudioAdaptor(self.cmdTool)
        tempSoundInfo = soundInfo.getOutline()
        if len(tempSoundInfo) > 0 :
            summary[MI_SOUNDCARD] = tempSoundInfo

        return json.dumps(summary)

if __name__ == "__main__":
    cmdTool = KACmdTool()
    cmdTool.loadInfoFromLshw()
    cc = KAMachineInfo(cmdTool)
    pprint(cc.getOutline())

