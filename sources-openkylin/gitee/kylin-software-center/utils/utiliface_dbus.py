#!/usr/bin/python3
# -*- coding: utf-8 -*-

### BEGIN LICENSE

# Copyright (C) 2013 National University of Defense Technology(NUDT) & Kylin Ltd

# Author:
#     Shine Huang<shenghuang@ubuntukylin.com>
# Maintainer:
#     Shine Huang<shenghuang@ubuntukylin.com>

# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 3, as published
# by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranties of
# MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
# PURPOSE.  See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program.  If not, see <http://www.gnu.org/licenses/>.

import dbus
import dbus.service
import os
import dbus.mainloop.glib
import getpass
import signal
from gi.repository import GObject
from gi.repository import GLib

INTERFACE = 'com.kylin.utiliface'
DBUSPATH = '/'

class UtilIfaceDbusService(dbus.service.Object):

    def __init__(self, bus, mainloop):

        self.bus = bus
        self.bus_name = dbus.service.BusName(INTERFACE, bus=bus)

        dbus.service.Object.__init__(self, self.bus_name, DBUSPATH)
        self.mainloop = mainloop

    @dbus.service.method(INTERFACE, in_signature='', out_signature='')
    def exit(self):
        self.mainloop.quit()

    # 获取安全配置文件存放路径
    @dbus.service.method('com.kylin.utiliface')
    def GetSecurityConfigPath(self):
        SOFTWARE_BLACK="/home/"+getpass.getuser()+"/.config/kylin-software-center-security-config.json"
        return SOFTWARE_BLACK

    # 重新加载安全配置（软件商店只提供接口，动态重载后续开发）
    @dbus.service.method('com.kylin.utiliface')
    def ReloadSecurityConfig(self):
        pass
    


if __name__ == '__main__':
    os.environ["TERM"] = "xterm"
    os.environ["PATH"] = "/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/X11R6/bin"
    os.environ["DEBIAN_FRONTEND"] = "noninteractive"

    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
    # GObject.threads_init()
    # mainloop = GObject.MainLoop()
    mainloop = GLib.MainLoop()
    signal.signal(signal.SIGINT, lambda: mainloop.quit())
    UtilIfaceDbusService(dbus.SessionBus(), mainloop)
    mainloop.run()
