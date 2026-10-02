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
import glob

import json
import subprocess

from pprint import pprint

from kajsondef import *
from kathread import *
from kaconfig import *

class KACameraInfo():

    def __init__(self, cmdtool):
        self.cmdTool = cmdtool
        pass

    def _mergeLshwForBusId(self, mapInfo, busId, lshwInfoList):
        words = busId.split(":")
        if len(words) != 2:
            return
        chs = words[0].split("-")
        if len(chs) != 2:
            return
        keyToLshw = "usb@" + chs[0] + ":" + chs[1]
        for info in lshwInfoList:
            if "bus info" in info and info["bus info"] == keyToLshw:
                mapInfo.update(info)
                break

    def _findHwinfoForUsbParent(self, hwinfoList, usbParent):
        matched = None
        for entry in hwinfoList:
            busId = entry.get("SysFS BusID", "")
            if busId.startswith(usbParent + ":"):
                if matched is None or len(entry) > len(matched):
                    matched = entry
        return matched

    def _loadInfoFromVideo4Linux(self, hwinfoList, lshwInfoList):
        """复合 USB 设备的视频接口可能不在 hwinfo --usb 的摄像头筛选结果中，从 V4L2 补全。"""
        v4lRoot = "/sys/class/video4linux"
        if not os.path.isdir(v4lRoot):
            return []

        seenDevices = set()
        result = []
        for videoPath in sorted(glob.glob(os.path.join(v4lRoot, "video*"))):
            nameFile = os.path.join(videoPath, "name")
            deviceLink = os.path.join(videoPath, "device")
            if not os.path.isfile(nameFile) or not os.path.exists(deviceLink):
                continue
            deviceReal = os.path.realpath(deviceLink)
            if deviceReal in seenDevices:
                continue
            seenDevices.add(deviceReal)

            with open(nameFile, "r", encoding="utf-8", errors="ignore") as f:
                devName = f.read().strip()
            if not devName:
                continue

            busId = os.path.basename(deviceReal)
            mapInfo = {
                "Device": devName,
                "Model": devName,
                "SysFS BusID": busId,
                "Hotplug": "USB" if "/usb" in deviceReal else "",
            }

            if ":" in busId:
                usbParent = busId.rsplit(":", 1)[0]
                hwMatch = self._findHwinfoForUsbParent(hwinfoList, usbParent)
                if hwMatch:
                    mapInfo.update(hwMatch)
                    mapInfo["SysFS BusID"] = busId

            self._mergeLshwForBusId(mapInfo, busId, lshwInfoList)

            driverLink = os.path.join(deviceReal, "driver")
            if os.path.islink(driverLink):
                mapInfo["Driver"] = os.path.basename(os.path.realpath(driverLink))

            result.append(mapInfo)
        return result

    def getCameraInfo(self):
        summary = {"list":[]}

        hwinfoList = self.cmdTool.loadInfoFromHwinfo("--usb")["list"]
        self.cmdTool.loadInfoFromLshw(False)
        lshwInfoList = self.cmdTool.lshwUsbList
        validInfoList = []
        length = len(hwinfoList)
        for i in range(length) :
            if len(hwinfoList[i]) < 5 :
                continue
            if "SysFS BusID" not in hwinfoList[i] :
                continue
            if ("Model" in hwinfoList[i] and "camera" in hwinfoList[i]["Model"].lower()) or \
                ("Device" in hwinfoList[i] and "camera" in hwinfoList[i]["Device"].lower()) or \
                ("Driver" in hwinfoList[i] and "uvcvideo" in hwinfoList[i]["Driver"].lower()) or \
                ("Model" in hwinfoList[i] and "webcam" in hwinfoList[i]["Model"].lower()) :
                ## 获取映射到 lshw设备信息的 关键字
                ##1-4:1.0
                words = hwinfoList[i]["SysFS BusID"].split(":")
                if len(words) == 2 :
                    chs = words[0].split("-")
                    if len(words) == 2 :
                        keyToLshw = "usb@"+chs[0]+":"+chs[1]
                        for mapInfo in lshwInfoList :
                            if "bus info" in mapInfo and mapInfo["bus info"] == keyToLshw :
                                hwinfoList[i].update(mapInfo)
                validInfoList.append(hwinfoList[i])

        if len(validInfoList) == 0:
            validInfoList = self._loadInfoFromVideo4Linux(hwinfoList, lshwInfoList)

        for mapInfo in validInfoList :
            if len(mapInfo) < 5 :
                continue
            caOne = {}
            caName,caManufacturer,caModel,caInterface,caDriver,caType,caVersion,caBus,caSpeed = "","","","","","","","",""
            ##名称
            if "Device" in mapInfo :
                caDevice = mapInfo["Device"].split("\"")
                if len(caDevice) > 1 :
                    caName = caDevice[1]
                else :
                    caName = mapInfo["Device"]
            if "product" in mapInfo :
                caName = mapInfo["product"]
            ##类型
            if "Hardware Class" in mapInfo and "unknown" not in mapInfo["Hardware Class"] :
                caType = mapInfo["Hardware Class"]

            ##制造商
            if "Vendor" in mapInfo :
                caVendor = mapInfo["Vendor"].split("\"")
                if len(caVendor) > 1 :
                    caManufacturer = caVendor[1]
                else :
                    caManufacturer = mapInfo["Vendor"]
            if "SubVendor" in mapInfo :
                caVendor = mapInfo["SubVendor"].split("\"")
                if len(caVendor) > 1 :
                    caManufacturer = caVendor[1]
                else :
                    caManufacturer = mapInfo["SubVendor"]
            if "vendor" in mapInfo :
                caManufacturer = mapInfo["vendor"]

            ##接口
            if "Hotplug" in mapInfo :
                caInterface = mapInfo["Hotplug"]

            ##型号
            if "Model" in mapInfo :
                caModel = mapInfo["Model"].strip("\"")
                if "bluetooth" in caModel :
                    caInterface = "BLUETOOTH"

            ##驱动
            if "Driver" in mapInfo :
                caDriver = mapInfo["Driver"].strip("\"")

            ##总线
            if "SysFS BusID" in mapInfo :
                caBus = mapInfo["SysFS BusID"]

            ##版本
            if "Revision" in mapInfo :
                caVersion = mapInfo["Revision"]
            if "version" in mapInfo :
                caVersion = mapInfo["version"]

            ##速度
            if "Speed" in mapInfo :
                caSpeed = mapInfo["Speed"]
            if "speed" in mapInfo :
                caSpeed = mapInfo["speed"]

            if len(caName) > 0 :
                caOne[CAI_NAME] = caName
            if len(caManufacturer) > 0 :
                caOne[CAI_MANUFACTURER] = caManufacturer
            if len(caModel) > 0 :
                caOne[CAI_MODEL] = caModel
            if len(caInterface) > 0 :
                caOne[CAI_INTERFACE] = caInterface
            if len(caDriver) > 0 :
                caOne[CAI_DRIVER] = caDriver
            if len(caType) > 0 :
                caOne[CAI_TYPE] = caType
            if len(caVersion) > 0 :
                caOne[CAI_VERSION] = caVersion
            if len(caBus) > 0 :
                caOne[CAI_BUSINFO] = caBus
            if len(caSpeed) > 0 :
                caOne[CAI_SPEED] = caSpeed

            if len(caOne) > 0 :
                summary["list"].append(caOne)
        if len(summary["list"]) == 0:
            return ""

        module = "camera"
        if "list" in summary:
            status = KAConfig.getInstance().execute(module,summary["list"])
            if "invisible" == status:
                # summary["list"] = []
                return ""
        return json.dumps(summary)

    #概要信息
    def getOutline(self) :
        outLine = self.getMouseInfo()
        outLineObj = json.loads(outLine)
        outLine = ""
        if len(outLineObj) > 0 and "list" in outLineObj :
            nIndex = 0
            for info in outLineObj["list"] :
                if CAI_NAME in info :
                    if nIndex == 0 :
                        outLine += info[CAI_NAME]
                    else :
                        outLine += " / " + info[CAI_NAME]
                if CAI_MODEL in info :
                        outLine += "("+info[CAI_MODEL]+")"
                nIndex = nIndex + 1
        return outLine

if __name__ == "__main__":
    cmdTool = KACmdTool()
    cmdTool.loadInfoFromLshw()
    cc = KACameraInfo(cmdTool)
    pprint(cc.getCameraInfo())
    #pprint(cc.getOutline())

