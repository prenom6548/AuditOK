#!/usr/bin/python3

import dbus
import dbus.service
import logging
import sys
from gi.repository import GLib

from .RevisionsManagerDbus import RevisionsManagerDbusController,RM_DBUS_SERVICE


class RevisionsManager():

    def __init__(self):
        # 建立dbus
        self.dbusController = self._setup_dbus()
        if self.dbusController == None:
            logging.critical("Dbus start failed")
            self.quit()

    def run(self):
        try:
            GLib.MainLoop().run()
        except KeyboardInterrupt:
            self.quit()

    def quit(self):
        """Request a shutdown of the daemon."""
        logging.debug("Daemon Quit")
        sys.exit(0)

    def _setup_dbus(self):
        # check if there is another g-a-i already and if not setup one
        # listening on dbus
        bus = dbus.SystemBus()
        try:
            bus_name = dbus.service.BusName(RM_DBUS_SERVICE,
                                            bus,
                                            do_not_queue=True)
            logging.info("initiate dbus success ...")
            return  RevisionsManagerDbusController(self, bus_name)
        except dbus.exceptions.NameExistsException:
            logging.critical("Another RevisionsManagerDaemon is already running")
            sys.exit(1)

            