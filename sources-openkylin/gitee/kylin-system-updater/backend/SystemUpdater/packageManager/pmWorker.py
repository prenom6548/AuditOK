import logging
import os
import re
import signal
import shutil
import subprocess
from multiprocessing import Process, Value

import dbus
from gi.repository import GLib
from gettext import gettext as _

from SystemUpdater.Core.utils import (
    PolicyKit_Authority,
    deb_verify,
    get_proc_from_dbus_name,
)
from SystemUpdater.packageManager.UpdaterSDKInterface import (
    InstallMode,
    UpdaterSDKPkgInstall,
)


class TransactionManager:
    TRANSTYPE_NONE = 'TRANSTYPE_NONE'
    TRANSTYPE_INSTALL = 'TRANSTYPE_INSTALL'
    TRANSTYPE_PURGE = 'TRANSTYPE_PURGE'
    TRANSTIMEOUT = {
        TRANSTYPE_NONE: 1,
        TRANSTYPE_INSTALL: 60 * 60 * 2,
        TRANSTYPE_PURGE: 20 * 60,
    }

    def __init__(self, update_daemon):
        self.updateDaemon = update_daemon
        self.transProcess = None
        self.transTimerID = None
        self.trans_is_runing = Value('b', False)
        self.trans_send_finished = Value('b', False)
        self.transInfo = {
            'package': [],
            'type': self.TRANSTYPE_NONE,
            'timeout': self.TRANSTIMEOUT[self.TRANSTYPE_NONE],
        }

    def transaction_timeout_checking(self):
        def check_timeout():
            if self.transProcess is None or not self.transProcess.is_alive():
                self.transaction_finished()
                return False
            if self.transInfo['timeout'] <= 0:
                logging.warning('SDK install transaction timeout')
                self.transaction_finished()
                return False
            self.transInfo['timeout'] -= 1
            return True

        self.transTimerID = GLib.timeout_add_seconds(1, check_timeout)

    def transaction_finished(self):
        try:
            self.trans_is_runing.value = False
            if self.transTimerID:
                GLib.source_remove(self.transTimerID)
                self.transTimerID = None
            if self.transProcess and self.transProcess.is_alive():
                self.transProcess.terminate()
                self.transProcess.join()
            if not self.trans_send_finished.value:
                if self.transInfo['type'] == self.TRANSTYPE_PURGE:
                    self.updateDaemon.dbus_send.PurgePackagesFinished(
                        False, '', _('Uninstallation timeout'))
                else:
                    self.updateDaemon.dbus_send.InstalldebFinished(
                        False, '', _('Installation timeout'))
            self.transInfo = {
                'package': [],
                'type': self.TRANSTYPE_NONE,
                'timeout': self.TRANSTIMEOUT[self.TRANSTYPE_NONE],
            }
        except Exception as error:
            logging.error('transaction_finished error: %s', error)


class PmWorker(TransactionManager):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.parent = parent
        self.dbus_send = parent.dbusController
        self.sqlite3_server = parent.sqlite3_server
        self.configs_uncover = parent.configs_uncover
        self.updater_package = UpdaterSDKPkgInstall(parent=self)

    def _sdk_package_manage(self):
        for signum in (signal.SIGINT, signal.SIGTERM, signal.SIGQUIT,
                       signal.SIGHUP, signal.SIGTSTP):
            signal.signal(signum, signal.SIG_DFL)
        package = self.transInfo['package'][0]
        if self.transInfo['type'] == self.TRANSTYPE_INSTALL and \
                not package.lower().endswith('.ok'):
            logging.warning(
                'Package %s is not an .ok package, keep the original install flow',
                package)
            self.trans_is_runing.value = False
            return

        if self.transInfo['type'] == self.TRANSTYPE_INSTALL:
            mode = InstallMode.KDK_PACKAGE_AUTO.value
            success = self.updater_package.package_interface_install(package, mode)
            if not self.trans_send_finished.value:
                self.trans_send_finished.value = True
                self.dbus_send.InstalldebFinished(success, '', '')
        elif self.transInfo['type'] == self.TRANSTYPE_PURGE:
            success = True
            for package in self.transInfo['package']:
                success = self.updater_package.package_interface_remove(package)
                if not success:
                    break
            if not self.trans_send_finished.value:
                self.trans_send_finished.value = True
                self.dbus_send.PurgePackagesFinished(success, '', '')

    @staticmethod
    def is_kaiming_package(package_name):
        kaiming = shutil.which('kaiming')
        if not kaiming:
            for candidate in ('/usr/bin/kaiming', '/opt/kaiming-tools/bin/kaiming'):
                if os.path.isfile(candidate) and os.access(candidate, os.X_OK):
                    kaiming = candidate
                    break
        if not kaiming:
            logging.warning('kaiming executable not found')
            return False
        try:
            result = subprocess.run(
                [kaiming, 'list'], stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT, text=True, check=False)
            logging.info('kaiming list output:\n%s', result.stdout.rstrip())
            normalized_name = ' '.join(str(package_name).split())
            row_pattern = re.compile(
                r'^\s*(\S+)\s+(.+?)\s+\S+\s+'
                r'(?:x86_64|amd64|arm64|aarch64|loongarch64|mips64el|riscv64)\s+')
            for line in result.stdout.splitlines():
                row = row_pattern.match(line)
                if not row:
                    continue
                identifier, name = row.groups()
                if normalized_name in (identifier, ' '.join(name.split())):
                    return True
        except Exception as error:
            logging.warning('kaiming list failed: %s', error)
        return False

    def is_maintain_mode(self):
        try:
            bus = dbus.SystemBus()
            obj = bus.get_object('com.kylin.MaintainMode',
                                 '/com/kylin/MaintainMode/Object')
            interface = dbus.Interface(obj, 'com.kylin.MaintainMode.interface')
            return str(interface.GetBootMode(timeout=3)) == 'maintain'
        except Exception as error:
            logging.error('Error checking maintain mode: %s', error)
            return False

    def is_supported_debian_install_mode(self):
        try:
            result = subprocess.run(
                ['dpkg-query', '-W', '-f=${Version}', 'libkysdk-package'],
                stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                text=True, check=True)
            import apt_pkg
            apt_pkg.init()
            return apt_pkg.version_compare(
                result.stdout.strip(), '3.0.1.0-0k5.0') >= 0
        except Exception as error:
            logging.error('Error checking libkysdk-package version: %s', error)
            return False

    def start_deb_install(self, deb_uri='', _check_local_dep=False,
                          _auto_satisfy=False, source='', sender=None):
        try:
            sender_name = get_proc_from_dbus_name(sender)
            if not deb_verify(deb_uri):
                authorized, message = PolicyKit_Authority(
                    source=source, sender=sender,
                    authentication=self.configs_uncover.getWithDefault(
                        'InstallAndPurge', 'install_authority', False))
                if not authorized:
                    self.dbus_send.InstalldebFinished(False, message, '')
                    return
            is_installer = sender_name == 'kylin-installer'
            self.transaction_start(
                [str(deb_uri)], self.TRANSTYPE_INSTALL,
                is_installer, self.is_maintain_mode() if is_installer else False)
        except Exception as error:
            logging.error('start_deb_install error: %s', error)
            self.dbus_send.InstalldebFinished(False, str(error), '')

    def transaction_start(self, package, trans_type, is_installer=False,
                          in_maintain_mode=False):
        if self.trans_is_runing.value:
            logging.warning('PmWorker transaction is running, please wait')
            return
        self.trans_is_runing.value = True
        self.trans_send_finished.value = False
        self.transInfo.update({
            'package': package, 'type': trans_type,
            'timeout': self.TRANSTIMEOUT[trans_type],
            'is_kylin_installer': is_installer,
            'in_maintain_mode': in_maintain_mode,
        })
        self.transProcess = Process(target=self._sdk_package_manage)
        self.transaction_timeout_checking()
        self.transProcess.start()

    def start_purge_pkgs(self, packages):
        kaiming_packages = [
            str(package) for package in packages
            if self.is_kaiming_package(str(package))
        ]
        if not kaiming_packages:
            return False
        self.transaction_start(kaiming_packages, self.TRANSTYPE_PURGE)
        return True
