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

    _PCI_BDF_RE = re.compile(
        r'^[0-9a-fA-F]{4}:[0-9a-fA-F]{2}:[0-9a-fA-F]{2}\.[0-7]$'
    )

    def __init__(self, cmdtool):
        self.cmdTool = cmdtool
        pass

    def _sound_card_bus_info(self, sound_card_path, bus_type):
        """
        总线信息尽量显示完整地址，与 lshw 的 pci@0000:xx:xx.x 一致。
        PCI：uevent PCI_SLOT_NAME 或 sysfs 路径中的 BDF。
        USB：uevent BUSNUM/DEVNUM 或路径中的 host 端口段。
        其它：platform@设备节点名等。
        """
        uevent_path = os.path.join(sound_card_path, 'device', 'uevent')
        kv = {}
        if os.path.exists(uevent_path):
            try:
                with open(uevent_path, 'r', encoding='utf-8', errors='ignore') as f:
                    for line in f:
                        line = line.strip()
                        if '=' in line:
                            k, v = line.split('=', 1)
                            kv[k] = v
            except OSError:
                pass
        slot = kv.get('PCI_SLOT_NAME', '').strip()
        if slot:
            return 'pci@' + slot
        busnum, devnum = kv.get('BUSNUM', '').strip(), kv.get('DEVNUM', '').strip()
        if busnum and devnum:
            dn = devnum.lstrip('0') or '0'
            return 'usb@%s:%s' % (busnum, dn)
        try:
            dev_real = os.path.realpath(os.path.join(sound_card_path, 'device'))
            parts = dev_real.strip('/').split('/')
            for part in parts:
                if self._PCI_BDF_RE.match(part):
                    return 'pci@' + part
            # USB：路径中常见 1-1.2:1.0 等形式
            for part in parts:
                if re.match(r'^\d+-\d+(?:\.\d+)*(?::\d+\.\d+)?$', part):
                    return 'usb@' + part
            if '/sound/' in dev_real:
                root = dev_real.split('/sound/')[0]
                base = os.path.basename(root)
                if base and base not in ('devices', 'sys'):
                    if bus_type and bus_type != 'pci':
                        return '%s@%s' % (bus_type, base)
                    return base
        except OSError:
            pass
        return bus_type if bus_type else ''

    # ===============================
    # 新增：从 /sys 目录优先获取声卡信息
    # ===============================
    def getAudioFromSys(self):
        summary = {"list": []}
        sound_card_paths = glob.glob('/sys/class/sound/card*')
        processed_cards = set()  # 记录已处理的声卡编号，避免重复

        for sound_card_path in sound_card_paths:
            card_num = sound_card_path.split('/')[-1][4:]
            driver_path = sound_card_path + '/device/driver'
            if not os.path.exists(driver_path):
                continue

            driver_realpath_split = os.path.realpath(driver_path).split('/')
            if len(driver_realpath_split) < 4:
                continue

            bus_type = driver_realpath_split[3]

            audioAdaptor = {}
            audioAdaptor[AudioAdaptor_Businfo] = self._sound_card_bus_info(
                sound_card_path, bus_type)
            vendor = ''
            product = 'Soc Integrated'

            compatible_path = sound_card_path + "/device/of_node/compatible"
            codec_path = "/proc/asound/card%s/codec#0" % card_num

            if os.path.exists(codec_path):
                with open(codec_path, 'r') as f:
                    line = f.readline()
                    while line:
                        if line.startswith('Name:'):
                            product = line.strip().split(':', 1)[-1]
                        elif line.startswith('Vendor:'):
                            vendor = line.strip().split(':', 1)[-1]
                        elif line.strip().startswith('Codec'):
                            # 如果 Codec 字段中包含 HDMI，则跳过这个声卡
                            if 'HDMI' in line.upper():
                                vendor = 'HDMI'  # 设置标志以便后续检查能捕获
                                break
                            vendor = line.split(' ')[1]  # 'Device: name="SN6180 Analog"'
                            product = line.split(' ')[2]
                        line = f.readline()
            elif os.path.exists(compatible_path):
                with open(compatible_path, 'r') as f:
                    compatible_content_split = f.read().split(',')
                    vendor = compatible_content_split[0]
                    if len(compatible_content_split) > 1:
                        product = compatible_content_split[1].strip()
            
            # 如果产品名或厂商名中包含 HDMI，则跳过这个声卡（HDMI 不是真正的声卡）
            if 'HDMI' in product.upper() or 'HDMI' in vendor.upper():
                continue

            flag_9a0 = Judgment_HW9A0()
            if flag_9a0 and "da_combine_v5" in product:
                product = "Hi6405"

            flag_pangu = Judgment_PANGUW()
            if flag_pangu and "Audio" in vendor:
                vendor = "HUAWEI"

            if Judgment_PANGUX():
                vendor = "HUAWEI"
                product = "OnBoard Audio"

            alsa_card_id = ''
            alsa_id_path = '/proc/asound/card%s/id' % card_num
            if os.path.isfile(alsa_id_path):
                try:
                    with open(alsa_id_path, 'r', encoding='utf-8', errors='ignore') as f:
                        alsa_card_id = mystrip((f.readline() or '').strip())
                except OSError:
                    pass

            if len(vendor) > 0:
                audioAdaptor[AudioAdaptor_Manufacturer] = get_url(mystrip(vendor), mystrip(vendor))
            elif alsa_card_id:
                # codec 无 Vendor 时（如 PMDK HDMI），用 ALSA 短名补全，否则会因无 Manufacturer 不入 list
                audioAdaptor[AudioAdaptor_Manufacturer] = get_url(
                    mystrip(alsa_card_id), mystrip(alsa_card_id))
                plow = mystrip(product).lower()
                if plow in ('soc integrated', 'hd-audio generic', 'onboard audio',
                            'generic', 'audio', ''):
                    product = alsa_card_id

            audioAdaptor[AudioAdaptor_Product] = mystrip(product)
            audioAdaptor[AudioAdaptor_Driver] = driver_realpath_split[-1]

            if AudioAdaptor_Manufacturer in audioAdaptor:
                summary["list"].append(audioAdaptor)
                processed_cards.add(card_num)  # 标记该声卡已处理

        # 飞腾等无PCI信息集成声卡方案
        if not Judgment_HW990() and not Judgment_HW9A0() and not Judgment_PANGUW():
            for sound_card_path in sound_card_paths:
                card_num = sound_card_path.split('/')[-1][4:]
                # 如果该声卡已经在第一个循环中处理过，跳过
                if card_num in processed_cards:
                    continue
                uevent_path = sound_card_path + '/device/uevent'
                if os.path.exists(uevent_path):
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
                            if '' != vendor and '' != model and '' != driver:
                                bus_type = 'platform'
                                dp = sound_card_path + '/device/driver'
                                if os.path.exists(dp):
                                    sp = os.path.realpath(dp).split('/')
                                    if len(sp) >= 4:
                                        bus_type = sp[3]
                                summary["list"].append({
                                    AudioAdaptor_Businfo: self._sound_card_bus_info(
                                        sound_card_path, bus_type),
                                    AudioAdaptor_Driver: driver,
                                    AudioAdaptor_Manufacturer: vendor,
                                    AudioAdaptor_Model: model})
                                processed_cards.add(card_num)  # 标记该声卡已处理
        return summary


    # ===============================
    # 原有声卡信息获取主逻辑
    # ===============================
    def getAudioAdaptorInfo(self):
        summary = {"list": []}
        index = 0

        # 1️⃣ 优先从 /sys 获取声卡信息
        sys_summary = self.getAudioFromSys()
        if len(sys_summary["list"]) > 0:
            summary = sys_summary
        else:
            # 2️⃣ 再从 lshw + hwinfo 获取信息
            self.cmdTool.loadInfoFromLshw(True)
            audioAdaptorList = self.cmdTool.lshwMultimediaList
            for audiodev in list(audioAdaptorList):
                if "description" in audiodev:
                    if "Audio" not in audiodev["description"]:
                        audioAdaptorList.remove(audiodev)

            adaptorList = self.cmdTool.loadInfoFromHwinfo("--sound")["list"]
            length = len(audioAdaptorList)
            for i in range(length):
                if "bus info" in audioAdaptorList[i]:
                    words = audioAdaptorList[i]["bus info"].split("@")
                    if len(words) != 2:
                        continue
                    key = words[1].strip()
                    isDone = False
                    for mapInfo in adaptorList:
                        if "SysFS BusID" in mapInfo and key == mapInfo["SysFS BusID"]:
                            audioAdaptorList[i].update(mapInfo)
                            isDone = True
                    if not isDone and ":" in key:
                        key = key.replace(":", "-")
                        for mapInfo in adaptorList:
                            if "SysFS BusID" in mapInfo:
                                sysBusIds = mapInfo["SysFS BusID"].split(":")
                                if key in sysBusIds:
                                    audioAdaptorList[i].update(mapInfo)
                                    isDone = True

            for mapInfo in audioAdaptorList:
                if len(mapInfo) > 0:
                    summary["list"].append({})
                    pciAddr = ""

                    if "product" in mapInfo:
                        summary["list"][index][AudioAdaptor_Product] = mapInfo["product"]

                    if "Vendor" in mapInfo:
                        devVendor = mapInfo["Vendor"].split("\"")
                        summary["list"][index][AudioAdaptor_Manufacturer] = devVendor[1] if len(devVendor) > 1 else mapInfo["Vendor"]
                    if "SubVendor" in mapInfo:
                        devVendor = mapInfo["SubVendor"].split("\"")
                        summary["list"][index][AudioAdaptor_Manufacturer] = devVendor[1] if len(devVendor) > 1 else mapInfo["SubVendor"]
                    if "vendor" in mapInfo:
                        summary["list"][index][AudioAdaptor_Manufacturer] = mapInfo["vendor"]

                    if "description" in mapInfo:
                        summary["list"][index][AudioAdaptor_Type] = mapInfo["description"]

                    if "version" in mapInfo:
                        summary["list"][index][AudioAdaptor_Version] = mapInfo["version"]

                    if "bus info" in mapInfo:
                        summary["list"][index][AudioAdaptor_Businfo] = mapInfo["bus info"]
                        pciResult = re.findall(r"pci@(\S+)", mapInfo["bus info"])
                        if pciResult:
                            pciAddr = pciResult[0]

                    if "capabilities" in mapInfo:
                        summary["list"][index][AudioAdaptor_Capabilities] = mapInfo["capabilities"]

                    if "width" in mapInfo:
                        summary["list"][index][AudioAdaptor_Width] = mapInfo["width"]

                    if "physical id" in mapInfo:
                        summary["list"][index][AudioAdaptor_Physicalid] = mapInfo["physical id"]

                    if "clock" in mapInfo:
                        summary["list"][index][AudioAdaptor_Clock] = mapInfo["clock"]

                    if "irq" in mapInfo:
                        summary["list"][index][AudioAdaptor_IRQ] = mapInfo["irq"]

                    if "memory" in mapInfo:
                        summary["list"][index][AudioAdaptor_Memory] = mapInfo["memory"]

                    if "driver" in mapInfo:
                        summary["list"][index][AudioAdaptor_Driver] = mapInfo["driver"]

                    if "latency" in mapInfo:
                        summary["list"][index][AudioAdaptor_Latency] = mapInfo["latency"]

                    if "Model" in mapInfo:
                        summary["list"][index][AudioAdaptor_Model] = mapInfo["Model"].strip("\"")

                    if "SubVendor" in mapInfo:
                        summary["list"][index][AudioAdaptor_SubVendor] = mapInfo["SubVendor"]

                    if "SubDevice" in mapInfo:
                        summary["list"][index][AudioAdaptor_SubDevice] = mapInfo["SubDevice"]

                    if "Config Status" in mapInfo:
                        summary["list"][index][AudioAdaptor_ConfigStatus] = mapInfo["Config Status"]

                    if "Driver" in mapInfo:
                        summary["list"][index][AudioAdaptor_Driver] = mapInfo["Driver"].strip("\"")

                    if "Driver Modules" in mapInfo:
                        summary["list"][index][AudioAdaptor_DriverMod] = mapInfo["Driver Modules"]

                    if "Driver Status" in mapInfo:
                        summary["list"][index][AudioAdaptor_DriverStatus] = mapInfo["Driver Status"]

                    if "Driver Activation Cmd" in mapInfo:
                        summary["list"][index][AudioAdaptor_DriverActiveCMD] = mapInfo["Driver Activation Cmd"]

                    if len(pciAddr) > 0:
                        args3 = ["lspci", "-vvs", pciAddr]
                        pipe3 = subprocess.Popen(args3, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                        output3 = pipe3.stdout.readlines()
                        pciStatusClock = ""
                        for line in [bytes.decode(l.strip(), "utf-8", "ignore") for l in output3]:
                            words = line.split(":")
                            if len(words) == 2:
                                keyWord = words[0].strip()
                                valWord = words[1].strip()
                        if len(pciStatusClock) > 0:
                            summary["list"][index][AudioAdaptor_Clock] = pciStatusClock
                    index += 1

        if len(summary["list"]) == 0:
            return json.dumps(summary)

        module = "soundCard"
        if "list" in summary:
            status = KAConfig.getInstance().execute(module, summary["list"])
            if "invisible" == status:
                return ""

        return json.dumps(summary)

    # ===============================
    # 概要信息函数保持不变
    # ===============================
    def getOutline(self):
        outLine = self.getAudioAdaptorInfo()
        if len(outLine) == 0:
            return ""
        outLineObj = json.loads(outLine)
        outLine = ""
        if len(outLineObj) > 0 and "list" in outLineObj:
            nIndex = 0
            for info in outLineObj["list"]:
                if AudioAdaptor_Product in info and len(info[AudioAdaptor_Product]) > 0:
                    outLine += (" / " if nIndex > 0 else "") + info[AudioAdaptor_Product]
                elif AudioAdaptor_Model in info:
                    outLine += (" / " if nIndex > 0 else "") + info[AudioAdaptor_Model]
                nIndex += 1
        return outLine


if __name__ == "__main__":
    cmdTool = KACmdTool()
    cmdTool.loadInfoFromLshw()
    cc = AudioAdaptor(cmdTool)
    pprint(cc.getAudioAdaptorInfo())
