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

from gi.repository import GLib

import os
import dbus
import signal
import dbus.service
import dbus.mainloop.glib

from kahwinfo import *

DBUSNAME = "com.kylin.assistant.sessiondaemon"

if __name__ == '__main__':
    os.environ["TERM"] = "xterm"
    os.environ["PATH"] = "/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/X11R6/bin"

    dbus.mainloop.glib.DBusGMainLoop(set_as_default = True)
    mainloop = GLib.MainLoop()

    bus = dbus.SessionBus()
    name = dbus.service.BusName(DBUSNAME, bus)
    signal.signal(signal.SIGINT, lambda : mainloop.quit())
    ##obj = TestObject(bus, mainloop)
    obj = HardwareInfo(bus, mainloop)

    print("Running kyas_session_service...\n")
    mainloop.run()
    print("Ending kyas_session_service...\n")
