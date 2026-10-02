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
import time

from pprint import pprint

from kajsondef import *
from kathread import *
from utils import *
import glob

AudioAdaptor_Product=         "Product"
AudioAdaptor_Manufacturer=    "Vendor"
AudioAdaptor_Model=           "Model"
AudioAdaptor_Type=            "Description"
AudioAdaptor_Version=         "Version"
AudioAdaptor_Businfo=         "Bus Info"
AudioAdaptor_Capabilities=    "Capabilities"
AudioAdaptor_IRQ=             "IRQ"
AudioAdaptor_Memory=          "Memory"
AudioAdaptor_Width=           "Width"
AudioAdaptor_Clock=           "Clock"
AudioAdaptor_SubVendor=       "SubVendor"
AudioAdaptor_SubDevice=       "SubDevice"
AudioAdaptor_Physicalid=      "Physical ID"
AudioAdaptor_Latency=         "Latency"
AudioAdaptor_ConfigStatus=    "Config Status"
AudioAdaptor_Driver=          "Driver"
AudioAdaptor_DriverMod=       "Driver Modules"
AudioAdaptor_DriverStatus=    "Driver Status"
AudioAdaptor_DriverActiveCMD= "Driver Activation Cmd"


class AudioAdaptor():

    def __init__(self, cmdtool):
        self.cmdTool = cmdtool
        pass

    def getAudioAdaptorInfo(self):
        summary = {"list":[]}
        index = 0
        self.cmdTool.loadInfoFromLshw(True)
        audioAdaptorList = self.cmdTool.lshwMultimediaList
        adaptorList = self.cmdTool.loadInfoFromHwinfo("--sound")["list"]
        length = len(audioAdaptorList)
        for i in range(length) :
            if "bus info" in audioAdaptorList[i] :
                words = audioAdaptorList[i]["bus info"].split("@")
                if len(words) != 2 :
                    continue
                key = words[1].strip()
                isDone = False
                #key = key.replace(".",":")
                for mapInfo in adaptorList :
                    if "SysFS BusID" in mapInfo :
                        if key != mapInfo["SysFS BusID"] :
                            continue
                        audioAdaptorList[i].update(mapInfo)
                        isDone = True
                if not isDone and ":" in key :
                    key = key.replace(":", "-")
                    for mapInfo in adaptorList :
                        if "SysFS BusID" in mapInfo :
                            sysBusIds = mapInfo["SysFS BusID"].split(":")
                            if key not in sysBusIds :
                                continue
                            audioAdaptorList[i].update(mapInfo)
                            isDone = True


        for mapInfo in audioAdaptorList :
            if len(mapInfo) > 0 :
                summary["list"].append({})
                pciAddr = ""
                ##名称
                if "product" in mapInfo :
                    summary["list"][index][AudioAdaptor_Product] = mapInfo["product"]

                ##制造商
                if "Vendor" in mapInfo :
                    devVendor = mapInfo["Vendor"].split("\"")
                    if len(devVendor) > 1 :
                        summary["list"][index][AudioAdaptor_Manufacturer] = devVendor[1]
                    else :
                        summary["list"][index][AudioAdaptor_Manufacturer] = mapInfo["Vendor"]
                if "SubVendor" in mapInfo :
                    devVendor = mapInfo["SubVendor"].split("\"")
                    if len(devVendor) > 1 :
                        summary["list"][index][AudioAdaptor_Manufacturer] = devVendor[1]
                    else :
                        summary["list"][index][AudioAdaptor_Manufacturer] = mapInfo["SubVendor"]
                if "vendor" in mapInfo :
                    summary["list"][index][AudioAdaptor_Manufacturer] = mapInfo["vendor"]

                ##类型
                if "description" in mapInfo:
                    summary["list"][index][AudioAdaptor_Type] = mapInfo["description"]

                ##版本
                if "version" in mapInfo:
                    summary["list"][index][AudioAdaptor_Version] = mapInfo["version"]

                ##总线信息
                if "bus info" in mapInfo:
                    summary["list"][index][AudioAdaptor_Businfo] = mapInfo["bus info"]
                    pciResult = re.findall("pci@(\S+)", mapInfo["bus info"])
                    if pciResult :
                        pciAddr = pciResult[0]

                ##功能
                if "capabilities" in mapInfo:
                    summary["list"][index][AudioAdaptor_Capabilities] = mapInfo["capabilities"]

                ##位宽
                if "width" in mapInfo:
                    summary["list"][index][AudioAdaptor_Width] = mapInfo["width"]

                ##物理ID
                if "physical id" in mapInfo:
                    summary["list"][index][AudioAdaptor_Physicalid] = mapInfo["physical id"]

                ##时钟频率
                if "clock" in mapInfo:
                    summary["list"][index][AudioAdaptor_Clock] = mapInfo["clock"]

                 ##中断
                if "irq" in mapInfo :
                    summary["list"][index][AudioAdaptor_IRQ] = mapInfo["irq"]

                ##内存
                if "memory" in mapInfo :
                    summary["list"][index][AudioAdaptor_Memory] = mapInfo["memory"]
                
                ##驱动
                if "driver" in mapInfo :
                    summary["list"][index][AudioAdaptor_Driver] = mapInfo["driver"]

                ##延迟
                if "latency" in mapInfo :
                    summary["list"][index][AudioAdaptor_Latency] = mapInfo["latency"]

                ##型号
                if "Model" in mapInfo :
                    summary["list"][index][AudioAdaptor_Model] = mapInfo["Model"].strip("\"")

                ##子制造商
                if "SubVendor" in mapInfo :
                    summary["list"][index][AudioAdaptor_SubVendor] = mapInfo["SubVendor"]

                ##子设备
                if "SubDevice" in mapInfo :
                    summary["list"][index][AudioAdaptor_SubDevice] = mapInfo["SubDevice"]

                ##配置状态
                if "Config Status" in mapInfo :
                    summary["list"][index][AudioAdaptor_ConfigStatus] = mapInfo["Config Status"]

                ##驱动
                if "Driver" in mapInfo :
                    summary["list"][index][AudioAdaptor_Driver] = mapInfo["Driver"].strip("\"")

                ##驱动模块
                if "Driver Modules" in mapInfo :
                    summary["list"][index][AudioAdaptor_DriverMod] = mapInfo["Driver Modules"]

                ##驱动状态
                if "Driver Status" in mapInfo :
                    summary["list"][index][AudioAdaptor_DriverStatus] = mapInfo["Driver Status"]

                ##驱动激活命令
                if "Driver Activation Cmd" in mapInfo :
                    summary["list"][index][AudioAdaptor_DriverActiveCMD] = mapInfo["Driver Activation Cmd"]

                #子制造商
                if len(pciAddr) > 0 :
                    args3 = ["lspci", "-vvs"]
                    args3.append(pciAddr)
                    pipe3 = subprocess.Popen(args3, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                    output3 = pipe3.stdout.readlines()
                    pciStatusClock = ""

                    for line in [ bytes.decode(l.strip(),"utf-8","ignore") for l in output3]:
                        words = line.split(":")
                        if len(words) == 2 :
                            keyWord = words[0].strip()
                            valWord = words[1].strip()
                            # 采用lshw 里面的声卡时钟频率，不采用pci里面的
                            # if keyWord == "Status" :
                            #     result = re.findall("[\s]*([0-9]+MHz)-.*", valWord)
                            #     if result and len(result) > 0 :
                            #         pciStatusClock = result[0]
                    if len(pciStatusClock) > 0 :
                        summary["list"][index][AudioAdaptor_Clock] = pciStatusClock

                index += 1

        if len(summary["list"]) == 0 :
            # non-pci bus
            sound_card_paths = glob.glob('/sys/class/sound/card*')
            for sound_card_path in sound_card_paths:
                card_num = sound_card_path.split('/')[-1][4:]
                driver_path = sound_card_path + '/device/driver'
                driver_realpath_split = os.path.realpath(driver_path).split('/')
                # /sys/bus/platform/drivers/hisi-dp-audio
                bus_type = driver_realpath_split[3]
                if bus_type == 'platform':# soc devices
                    audioAdaptor = {}
                    audioAdaptor[AudioAdaptor_Businfo] = bus_type
                    vendor = ''
                    product = 'Soc Integrated'
                    compatible_path = sound_card_path + "/device/of_node/compatible"
                    codec_path = "/proc/asound/card%s/codec#0" % card_num
                    if os.path.exists(codec_path):
                        #Name:da_combine_v5
                        #Vendor:HISILICON
                        #Model:da_combine_v5
                        with open(codec_path, 'r') as f:
                            line = f.readline()
                            while line:
                                if line.startswith('Name:'):
                                    product = line.strip().split(':', 1)[-1]
                                elif line.startswith('Vendor:'):
                                    vendor = line.strip().split(':', 1)[-1]
                                line = f.readline()
                    elif os.path.exists(compatible_path):
                        # hisilicon,hisi-dp-audio-machine\x00
                        with open(compatible_path, 'r') as f:
                            compatible_content_split = f.read().split(',')
                            vendor = compatible_content_split[0]
                            if len(compatible_content_split) > 1:
                                product = compatible_content_split[1].strip()
                    flag_9a0 = Judgment_HW9A0()
                    if flag_9a0 and "da_combine_v5" in product:
                        product = "Hi6405"
                    #audioAdaptor[AudioAdaptor_Product] = mystrip(product)
                    flag_pangu = Judgment_PANGUW()
                    if flag_pangu and "Audio" in vendor:
                        vendor = "HUAWEI"
                    if Judgment_PANGUX():
                        vendor = "HUAWEI"
                        product = "OnBoard Audio"
                    if len(vendor) > 0 :
                        audioAdaptor[AudioAdaptor_Manufacturer] = get_url(mystrip(vendor), mystrip(vendor))
                    audioAdaptor[AudioAdaptor_Product] = mystrip(product)
                    driver_name = driver_realpath_split[-1]
                    audioAdaptor[AudioAdaptor_Driver] = driver_name
                    if AudioAdaptor_Manufacturer in audioAdaptor :
                        summary["list"].append(audioAdaptor)
                else:
                    # pci bus devices have been parsed
                    # other bus device is unsupported
                    pass

        # 添加集成声卡信息集成方案
        # 非990/9A0机型采用此方案
        if not Judgment_HW990() and not Judgment_HW9A0() and not Judgment_PANGUW():
            sound_card_paths = glob.glob('/sys/class/sound/card*')
            for sound_card_path in sound_card_paths:
                card_num = sound_card_path.split('/')[-1][4:]
                uevent_path = sound_card_path + '/device/uevent'
                if os.path.exists(uevent_path):
                    #飞腾集成声卡无pci信息，通过uevent进行筛选
                    with open(uevent_path, 'r', encoding="utf-8") as f:
                        lines = f.readlines()
                        if 'PCI_ID' in "".join(lines):
                            continue
                        else:
                            driver = ''
                            vendor = ''
                            model = '' 
                            for line in lines:
                                if line.startswith('DRIVER='):
                                    driver = line.strip().split('=', 1)[-1]
                            vendor, model = self.cmdTool.getSoundVendorProduct(card_num)
                            if ''!=vendor and ''!=model and ''!=driver:
                                summary["list"].append({
                                    AudioAdaptor_Driver:driver,
                                    AudioAdaptor_Manufacturer:vendor,
                                    AudioAdaptor_Model:model})
            
        # ## 添加集成声卡信息集成方案
        # pacmdSoundList = self.cmdTool.loadSoundInfoFromPacmd(False);
        # for mapInfo in pacmdSoundList :
        #     if len(mapInfo) > 0 :
        #         if "sound_busaddr" in mapInfo:
        #             # 判断当前信息是否已经放入声卡信息中
        #             if mapInfo["sound_busaddr"].replace("-","@") in [mapInfo[AudioAdaptor_Businfo] for mapInfo in summary["list"]]:
        #                 continue;
        #             else:
        #                 soundDict = {};
        #                 if "sound_vendor" in mapInfo:
        #                     soundDict[AudioAdaptor_Manufacturer] = mapInfo["sound_vendor"];
        #                 if "sound_driver" in mapInfo:
        #                     soundDict[AudioAdaptor_Driver] = mapInfo["sound_driver"];
        #                 if "sound_busaddr" in mapInfo:
        #                     soundDict[AudioAdaptor_Businfo] = mapInfo["sound_busaddr"];
        #                 if "sound_name" in mapInfo:
        #                     soundDict[AudioAdaptor_Product] = mapInfo["sound_name"];
        #                 if "sound_model" in mapInfo:
        #                     soundDict[AudioAdaptor_Manufacturer] = mapInfo["sound_model"];
        #                 if len(soundDict) > 0:
        #                     summary["list"].append(soundDict);

        if len(summary["list"]) == 0 :
            return json.dumps(summary)
        
        module = "soundCard"
        if "list" in summary:
            status = KAConfig.getInstance().execute(module,summary["list"])
            if "invisible" == status:
                # summary["list"] = []
                return ""

        return json.dumps(summary)

    #概要信息
    def getOutline(self) :
        outLine = self.getAudioAdaptorInfo()
        if len(outLine) == 0 :
            return ""
        outLineObj = json.loads(outLine)
        outLine = ""
        if len(outLineObj) > 0 and "list" in outLineObj :
            nIndex = 0
            for info in outLineObj["list"] :
                if AudioAdaptor_Product in info and len(info[AudioAdaptor_Product]) > 0:
                    if nIndex == 0 :
                        outLine += info[AudioAdaptor_Product]
                    else :
                        outLine += " / " + info[AudioAdaptor_Product]
                elif AudioAdaptor_Model in info :
                    if nIndex == 0 :
                        outLine += info[AudioAdaptor_Model]
                    else :
                        outLine += " / " + info[AudioAdaptor_Model]
                nIndex = nIndex + 1
        return outLine

    #禁用声卡设备
    def disableSoundCard(self,strProductAndPath) :
        print(strProductAndPath)
        # try:
        if("|" in strProductAndPath):
            strProduct,pciPath = strProductAndPath.split("|")
            sysPciPath = "/sys/devices/pci0000:00/"
            if not os.path.exists(sysPciPath+pciPath+"/remove") :
                return False
            youkerConfigDir = "/etc/youker-assistant"
            if os.path.exists(youkerConfigDir):
                pass
            else:
                os.mkdir(youkerConfigDir)
            #写文件
            with open(youkerConfigDir+"/"+"soundCard", "w+") as file:
                file.write(strProduct + "|" + pciPath)
            #保存设备名称和pci写到文件中    设备名称+pci地址    而且要做区分            
            if validate_command(pciPath):
                device_path = sysPciPath+pciPath+"/remove";
                with open(device_path, "w") as file:
                    file.write("1")
        # except Exception as e:
        #     print(e)       
        return True
    
    #启用声卡设备
    def enableSoundCard(self, productAndPci) :
        print('------------------------------------')
        print(productAndPci)
        configFile = "/etc/youker-assistant/soundCard"
        youkerConfigDir = "/etc/youker-assistant"
        sysPciRescanPath = "/sys/bus/pci/rescan"
        if not os.path.exists(sysPciRescanPath) or not os.path.exists(configFile)  :
            return False
        # 去除禁用信息
        allData = []
        with open(configFile,'r')as fd:
            print(fd.readlines)
        #with open(configFile,'r') as fd:
        file = open(configFile,'r') 
        allInfo = file.readlines()
        print(allInfo)
        print('====================')
        for info in allInfo:
            #info = fd.readline()
            if '' != info:
                if productAndPci in info:
                    pass
                else:
                    allData.append(info)
        fw = open(configFile, 'w')
        fw.writelines(allData)
        fw.close()
        # os.popen('sed -i "s/%s//g" %s'%(productAndPci,configFile))
#       os.popen('echo 1 > %s'%(sysPciRescanPath))
        subprocess.Popen("echo 1 > %s"%(sysPciRescanPath), shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT) 
       # 扫描当前所有需要禁用的设备
        disableList = ["netCard","soundCard"]
        commands = []
        for item in disableList:
            print('-----------------disableList-------------------')
            print(os.path.exists(youkerConfigDir + "/" + item))
            print(youkerConfigDir + "/" + item)
            if os.path.exists(youkerConfigDir + "/" + item):
                with open(youkerConfigDir + "/" + item, "r" ,encoding="utf-8") as file:
                    allLines = file.readlines()
                    for line in set(allLines):
                        print('---------------line')
                        line = line.strip()
                        print(line)
                        if "|" in line :
                            print('-----------in--------')
                            strProduct,pciPath = line.strip().split("|")
                            sysPciPath = "/sys/devices/pci0000:00/"
                            count = 0
                            while True and count < 30 : 
                                if os.path.exists(sysPciPath+pciPath.strip()+"/remove"):
                                    break
                                time.sleep(1)
                                count += 1
                            if not os.path.exists(sysPciPath+pciPath.strip()+"/remove") :
                                return False
                            print('-----------------echo-------------------')
                            print('echo 1 > %s'%(sysPciPath+pciPath.strip()+"/remove"))
                            commands.append("echo 1 > %s"%(sysPciPath+pciPath.strip()+"/remove"))
                            # subprocess.Popen("echo 1 > %s"%(sysPciPath+pciPath.strip()+"/remove"), shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
#                            os.popen('echo 1 > %s'%(sysPciPath+pciPath.strip()+"/remove"))   
        # 遍历命令列表，并逐一执行 
        for cmd in commands:  subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        return True

if __name__ == "__main__":
    cmdTool = KACmdTool()
    cmdTool.loadInfoFromLshw()
    cc = AudioAdaptor(cmdTool)
    #cc.disableSoundCard("Comet Lake PCH-LP cAVS|0000:00:1f.3")
    # cc.enableSoundCard("Zhaoxin Audio device|0000:00:01.1")
    pprint(cc.getAudioAdaptorInfo())

