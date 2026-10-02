#!/usr/bin/python3
# -*- coding: utf-8 -*-

import os
import signal
import dbus
import dbus.mainloop.glib
from gi.repository import GObject
import sys
from run_utildbus import RunUtilDbus

if __name__ == '__main__':
    # os.environ["TERM"] = "xterm"
    os.environ["PATH"] = "/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games"
    # os.environ["DEBIAN_FRONTEND"] = "noninteractive"
    os.environ["QT_QPA_PLATFORMTHEME"] = "ukui"
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
    GObject.threads_init()
    mainloop = GObject.MainLoop()
    try:
        bus = dbus.SessionBus(mainloop)
    except:
        sys.exit(0)
    bus_name = dbus.service.BusName('com.kylin.softwarecenter.runapp', bus)
    dbusControler = RunUtilDbus(None, bus_name,mainloop)

    mainloop.run()