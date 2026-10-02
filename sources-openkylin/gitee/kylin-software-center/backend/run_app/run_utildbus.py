#!/usr/bin/python3
# -*- coding: utf-8 -*-


import dbus
import sys
import signal
import os
import logging
import dbus.service
import subprocess
from gi.repository import GObject
from dbus.mainloop.glib import DBusGMainLoop
mainloop = DBusGMainLoop(set_as_default=True)
LOG = logging.getLogger(__name__)

class RunUtilDbus(dbus.service.Object):

    def __init__(self, parent, bus_name,mainloop,
                 object_path='/com/kylin/softwarecenter/runapp'):
        dbus.service.Object.__init__(self, bus_name, object_path)
        self.parent = parent
        self.mainloop = mainloop

    def stop(self):
        """ stop the dbus controller and remove from the bus """
        self.remove_from_connection()
        self.mainloop.quit()

    @dbus.service.method('com.kylin.softwarecenter.runapp', in_signature='s', out_signature='')
    def run_app(self, cmd):
        os.system(cmd+"&")

    @dbus.service.method('com.kylin.softwarecenter.runapp', in_signature='s', out_signature='')
    def run_ksc_app(self, cmd):
        os.system(cmd)

    @dbus.service.method('com.kylin.softwarecenter.runapp', in_signature='', out_signature='s')
    def wakeup(self):
        return "kylin"



if __name__ == '__main__':
    try:
        bus = dbus.SessionBus(mainloop)
    except:
        sys.exit(0)
    bus_name = dbus.service.BusName('com.kylin.softwarecenter.runapp', bus)
    dbusControler = RunUtilDbus(None,bus_name)