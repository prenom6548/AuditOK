# UpdateManager.py
# -*- Mode: Python; indent-tabs-mode: nil; tab-width: 4; coding: utf-8 -*-
import os
import sys
import time
import dbus
import logging
import dbus.service
import traceback
from gettext import gettext as _
from dbus.mainloop.glib import DBusGMainLoop
from gi.repository import GLib
DBusGMainLoop(set_as_default=True)

from .UpgradeStrategiesDbus import UpgradeStrategiesDbusController,UPDATER_DBUS_INTERFACE,UPDATER_DBUS_PATH,UPDATER_DBUS_SERVICE
from .Core.Database import Sqlite3Server
from .Core.loop import mainloop

from SystemUpdater.Core.UpdaterConfigParser import UpgradeConfig

STRATEGY_IDLE_INTERVAL = 2*60
STRATEGY_IDLE_TIMEOUT = 6*60
class UpgradeStrategies():
    def __init__(self,options):
        try:
            self.options = options
            #dbus
            self.dbusController = self._setup_dbus()
            #config
            self.uuconfigs = UpgradeConfig(datadir = "/var/lib/unattended-upgrades/", name = "unattended-upgrades-policy.conf")
            self.sqlite3_server = Sqlite3Server(self, _no_DataMigration=True)
            #策略配置接口的超时退出机制
            self.strategy_timestamp = 0
            GLib.timeout_add_seconds(STRATEGY_IDLE_INTERVAL,
                self._check_strategy_inactivity)
        except Exception as e:
            logging.error(e)
            traceback.print_exc()
    
    def run(self):
        """Start the daemon and listen for calls."""
        logging.info("Waiting for calls...")
        try:
            mainloop.run()
        except KeyboardInterrupt:
            self.dbusController.Quit(None)

    def _setup_dbus(self):
        # check if there is another g-a-i already and if not setup one
        # listening on dbus
        bus = dbus.SystemBus()
        try:
            bus_name = dbus.service.BusName(UPDATER_DBUS_SERVICE,
                                            bus,
                                            do_not_queue=True)
            logging.info("Initiate dbus success ...")
            return  UpgradeStrategiesDbusController(self, bus_name)
        except dbus.exceptions.NameExistsException:
            if self.options.replace is False:
                logging.critical("Another daemon is already running")
                sys.exit(1)
            logging.warning("Replacing already running daemon")
            
            retry_reboot_times = 0
            the_other_guy = bus.get_object(UPDATER_DBUS_SERVICE,
                                            UPDATER_DBUS_PATH)
            the_other_guy.Quit(dbus_interface=UPDATER_DBUS_INTERFACE,
                                timeout=300)
            time.sleep(1)
            while True:
                retry_reboot_times = retry_reboot_times + 1
                #当重试次数超过5次时退出程序
                if retry_reboot_times > 5:
                    logging.critical("Reboot backend is Failed...")
                    sys.exit(1)
                try:
                    bus_name = dbus.service.BusName(UPDATER_DBUS_SERVICE,
                                                    bus,
                                                    do_not_queue=True)
                    logging.warning("Replacing already running daemon to Success...")
                    return  UpgradeStrategiesDbusController(self, bus_name)
                except dbus.exceptions.NameExistsException:
                    the_other_guy = bus.get_object(UPDATER_DBUS_SERVICE,
                                                    UPDATER_DBUS_PATH)
                    the_other_guy.Quit(dbus_interface=UPDATER_DBUS_INTERFACE,
                                        timeout=300)
                    logging.error("Dbus has not withdrawn and retry reboot times:%d...",retry_reboot_times)
                    time.sleep(1)
    
    def _check_strategy_inactivity(self):
        logging.info("Checking for inactivity in Strategies daemon ...")
        timestamp = self.strategy_timestamp
        if timestamp == 0:
            self.strategy_timestamp = time.time()
            return True
        #超时退出
        if self.strategy_timestamp != 0 and  time.time() - self.strategy_timestamp > STRATEGY_IDLE_TIMEOUT:
            logging.warning("Quitting due to inactivity")
            self.dbusController.Quit(None)
            return False
        return True