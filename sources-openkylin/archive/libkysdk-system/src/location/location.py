# 
#  libkysdk-system's Library
#  
#  Copyright (C) 2023-2025, KylinSoft Co., Ltd.
# 
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public
#  License as published by the Free Software Foundation; either
#  version 3 of the License, or (at your option) any later version.
# 
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  Lesser General Public License for more details.
# 
#  You should have received a copy of the GNU General Public License
#  along with this library.  If not, see <https://www.gnu.org/licenses/>.
# 
#  Authors: ShaoShuai Tian <tianshaoshuai@kylinos.cn>
# 
# 

# -*- coding: UTF-8 -*- 
from urllib.request import urlopen
import requests
import json
import re

import dbus
import dbus.service
from dbus.mainloop.glib import DBusGMainLoop
from gi.repository import GLib

class AddressBus(dbus.service.Object):

    def __init__(self, conn=None, object_path=None):
        dbus.service.Object.__init__(self, conn, object_path)

    _IP_SOURCES = [
        "https://api.ipify.org",
        "https://ifconfig.me/ip",
        "https://api.ip.sb/ip",
        "https://ipinfo.io/ip"
    ]

    def getaddr(self):
        for source in self._IP_SOURCES:
            try:
                text = urlopen(source, timeout=5).read().decode('utf-8').strip()
                ip = re.findall(r'\d+\.\d+\.\d+\.\d+', text)
                if ip:
                    return ip[0]
            except Exception:
                continue
        return None

    @dbus.service.method('com.kysdk.location', out_signature='s')
    def getAddress(self):
        try:
            ip = self.getaddr()
            if not ip:
                return "Get public ip failed"

            url = "http://api.map.baidu.com/location/ip"
            myParams = {"ak":"4DPAeozisfSCUgSLuC14Ux4iXjeEYNGa","ip":ip,"coor":"bd09ll"}
            res = requests.get(url=url, params=myParams, timeout=5)
            if res.status_code != 200:
                return "Error Code " + str(res.status_code)

            data = json.loads(res.text)
            if data["status"] == 0:
                return json.dumps(data, ensure_ascii=False)
            else:
                return data["message"]
        except Exception as e:
            return "Get address failed: " + str(e)

if __name__ == '__main__' :
    try:
        DBusGMainLoop(set_as_default=True)

        system_bus = dbus.SystemBus()
        name = dbus.service.BusName("com.kysdk.location", system_bus)
        object_path = "/com/kysdk/location"
        obj = AddressBus(system_bus, object_path)

        mainloop = GLib.MainLoop()
        mainloop.run()
    except Exception as e:
        print("Error occurred: " + str(e))
