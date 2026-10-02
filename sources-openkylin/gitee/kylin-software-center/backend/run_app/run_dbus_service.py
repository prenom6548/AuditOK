#!/usr/bin/python3
# -*- coding: utf-8 -*
import dbus
import os
import sys
import shutil
import locale
import datetime
import time
from gi.repository import GObject
import logging

from PyQt5.QtCore import *
from PyQt5 import QtDBus
UBUNTUKYLIN_SOFTWARECENTER_ACTION = 'com.kylin.softwarecenter.action'


import multiprocessing

from dbus.mainloop.glib import DBusGMainLoop, threads_init
from models.globals import Globals
import gettext
gettext.textdomain("kylin-software-center")
_ = gettext.gettext
UBUNTUKYLIN_SERVICE_PATH = "com.kylin.softwarecenter.runapp"
UBUNTUKYLIN_INTERFACE_PATH = "com.kylin.softwarecenter.runapp"


class Run_Dbus_Service(QObject):

    def __init__(self):
        QObject.__init__(self)
        locale.setlocale(locale.LC_ALL, "zh_CN.UTF-8")

        self.iface = None
        self.init_dbus_ifaces()

    #
    # 函数：初始化dbus接口
    #
    def init_dbus_ifaces(self):
        try:
            Globals.LOG_INFO.logger.info("InstallBackend init_run_dbus_ifaces (" + datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f") + ")")
            DBusGMainLoop(set_as_default=True)
            threads_init()
            bus = dbus.SessionBus()
        except Exception as e:
            Globals.LOG_ERROR.logger.error("dbus exception:%s" % str(e))
            return False

        try:
            Globals.LOG_INFO.logger.info("InstallBackend init_dbus_ifaces call blocking (%s)" % datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
            bus.call_blocking(UBUNTUKYLIN_SERVICE_PATH, '/com/kylin/softwarecenter/runapp', UBUNTUKYLIN_INTERFACE_PATH, 'wakeup', None, (), timeout=5)
        except dbus.DBusException as e:
            Globals.LOG_ERROR.logger.error("InstallBackend DBusConnection call blocking exception: %s" % str(e))
            return False

        try:
            Globals.LOG_INFO.logger.info("InstallBackend init_dbus_ifaces bus.get_object (%s)" % datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
            obj = bus.get_object(UBUNTUKYLIN_SERVICE_PATH, '/com/kylin/softwarecenter/runapp')
            Globals.LOG_INFO.logger.info("InstallBackend init_dbus_ifaces dbus.Interface (%s)" % datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
            self.iface = dbus.Interface(obj, dbus_interface=UBUNTUKYLIN_INTERFACE_PATH)

            Globals.LOG_INFO.logger.info("InstallBackend init_dbus_ifaces finished (%s)" % datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
        except dbus.DBusException as e:
            Globals.LOG_ERROR.logger.error("InstallBackend DBusConnection exception: %s" % str(e))
            return False

        return True

    def call_exit(self):
        self.iface.stop()

    def call_run_app(self,cmd):
        self.iface.run_app(cmd)

    def call_run_ksc_app(self,cmd):
        self.iface.run_ksc_app(cmd)