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
import sys
import signal
import os
import logging
import dbus.service
from gi.repository import GObject
from dbus.mainloop.glib import DBusGMainLoop
mainloop = DBusGMainLoop(set_as_default=True)
LOG = logging.getLogger(__name__)

class UtilDbus(dbus.service.Object):

    def __init__(self, parent, bus_name,
                 object_path='/com/kylin/softwarecenter'):
        dbus.service.Object.__init__(self, bus_name, object_path)
        self.parent = parent

    def stop(self):
        """ stop the dbus controller and remove from the bus """
        self.remove_from_connection()

    @dbus.service.method('com.kylin.utiliface',in_signature='s',out_signature='b')
    def show_search_result(self,appname):
        res = self.parent.click_search_app(appname)
        return res
        #带参数启动软件商店(1.没有启动软件商店则启动软件商店，并且显示到软件详情界面。2.启动了软件商店显示结果的详情界面)

if __name__ == '__main__':
    try:
        bus = dbus.SessionBus(mainloop)
    except:
        sys.exit(0)
    bus_name = dbus.service.BusName('com.kylin.softwarecenter', bus)
    dbusControler = UtilDbus(None,bus_name)