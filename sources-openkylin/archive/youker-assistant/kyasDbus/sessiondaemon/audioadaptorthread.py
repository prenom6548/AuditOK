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

    def getAudioAdaptorInfoExt(self):
        summary = {"list":[]}
        index = 0
        audioAdaptorList = self.cmdTool.loadSoundInfoFromPacmd(False)

        for mapInfo in audioAdaptorList :
            if len(mapInfo) > 0 :
                summary["list"].append({})

                ##名称
                if "sound_name" in mapInfo :
                    summary["list"][index][AudioAdaptor_Product] = mapInfo["sound_name"]

                ##制造商
                if "sound_vendor" in mapInfo :
                    summary["list"][index][AudioAdaptor_Manufacturer] = mapInfo["sound_vendor"]

                ##类型
                if "sound_description" in mapInfo:
                    summary["list"][index][AudioAdaptor_Type] = mapInfo["sound_description"]

                ##总线信息
                #if "sound_busaddr" in mapInfo:
                #    summary["list"][index][AudioAdaptor_Businfo] = mapInfo["sound_busaddr"]

                ##驱动
                #if "sound_driver" in mapInfo :
                #    summary["list"][index][AudioAdaptor_Driver] = mapInfo["sound_driver"]

                ##型号
                if "sound_model" in mapInfo :
                    summary["list"][index][AudioAdaptor_Model] = mapInfo["sound_model"]

                index += 1

        if len(summary["list"]) == 0 :
            return ""

        return json.dumps(summary)

if __name__ == "__main__":
    cmdTool = KACmdTool()
    cmdTool.loadInfoFromLshw()
    cc = AudioAdaptor(cmdTool)
    pprint(cc.getAudioAdaptorInfoExt())

