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
from subprocess import PIPE
import dbus
import dbus.service
import dbus.mainloop.glib
from gi.repository import GLib

import subprocess

from kathread import *
from utils import *

from audioadaptorthread import AudioAdaptor
from kacmdtool import KACmdTool

OBJPATH = '/com/kylin/assistant/sessiondaemon'
INTERFACE = 'com.kylin.assistant.sessiondaemon'

THREAD_INDEX_OUTLINE = 0
THREAD_INDEX_HARDWARE = 1
THREAD_INDEX_MEMORY = 2
THREAD_INDEX_NETWORK = 3
THREAD_INDEX_AUDIOADAPTER = 4
THREAD_INDEX_SYSTEMBOARD = 5
THREAD_INDEX_RELOADLSHW = 6
THREAD_INDEX_MAX = 7

class HardwareInfo(dbus.service.Object):
    def __init__(self, system_bus, mainloop):
        # Init dbus service.
        dbus.service.Object.__init__(self, system_bus, OBJPATH)
        self.mainloop = mainloop
        self.cmdtool = KACmdTool()

        self.threadPool = [None] * THREAD_INDEX_MAX

    @dbus.service.method(INTERFACE, in_signature='', out_signature='', sender_keyword='sender')
    def exit(self, sender = None):
        self.mainloop.quit()

    @dbus.service.method(INTERFACE, in_signature='', out_signature='s', sender_keyword='sender')
    def getSoundCardsExt(self, sender=None):
        self.threadPool[THREAD_INDEX_AUDIOADAPTER] = KAThread(AudioAdaptor(self.cmdtool).getAudioAdaptorInfoExt, None, args=())
        #改为信号的方式
        self.threadPool[THREAD_INDEX_AUDIOADAPTER].start()
        self.threadPool[THREAD_INDEX_AUDIOADAPTER].join()
        return self.threadPool[THREAD_INDEX_AUDIOADAPTER].getResult()

if __name__ == '__main__':
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
    mainloop = GLib.MainLoop()
    # Init dbus.
    system_bus = dbus.SessionBus()
    bus_name = dbus.service.BusName("com.kylin.assistant.sessiondaemon", system_bus)
    HardwareInfo(system_bus, mainloop)
    mainloop.run()

