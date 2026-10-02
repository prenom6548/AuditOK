import ctypes
import logging
import os
import subprocess
from enum import Enum


class InstallMode(Enum):
    KDK_PACKAGE_AUTO = 0
    KDK_PACKAGE_DEBIAN = 3


class UpdaterSDKInterface:
    def __init__(self, library_path=''):
        self.sdkLib = None
        self.sdkLoaded = False
        try:
            multiarch = subprocess.check_output(
                ['dpkg-architecture', '-qDEB_HOST_MULTIARCH'], timeout=3
            ).decode().strip()
            library_path = os.path.join('/usr/lib', multiarch, library_path)
            if os.path.exists(library_path):
                self.sdkLib = ctypes.CDLL(library_path)
                self.sdkLoaded = True
            else:
                logging.warning("%s does not exist.", library_path)
        except Exception as error:
            logging.error("UpdaterSDKInterface init error: %s", error)

    def function_exists(self, name):
        try:
            getattr(self.sdkLib, name)
            return True
        except AttributeError:
            return False


class UpdaterSDKPkgInstall:
    SDK_LIBRARY = 'libkypackage.so'
    KCallBack = ctypes.CFUNCTYPE(None, ctypes.c_int, ctypes.c_int, ctypes.c_char_p)
    KDK_PACKAGE_INSTALL_ERROR = 0
    KDK_PACKAGE_INSTALL_SUCCESS = 1

    def __init__(self, parent=None):
        self.upgradeService = parent
        self.installPkgName = ''
        self.installLoaded = False
        self.sdkInterface = UpdaterSDKInterface(self.SDK_LIBRARY)
        if self.sdkInterface.sdkLoaded and self.sdkInterface.function_exists(
                'kdk_package_install_package'):
            install = self.sdkInterface.sdkLib.kdk_package_install_package
            install.argtypes = [ctypes.c_char_p, ctypes.c_int, self.KCallBack]
            install.restype = ctypes.c_int
            self.installLoaded = True
        self.removeLoaded = False
        if self.sdkInterface.sdkLoaded and self.sdkInterface.function_exists(
                'kdk_package_remove_package'):
            remove = self.sdkInterface.sdkLib.kdk_package_remove_package
            remove.argtypes = [ctypes.c_char_p, self.KCallBack]
            remove.restype = ctypes.c_int
            self.removeLoaded = True

    def _status_changed(self, progress, state, loginfo):
        try:
            details = loginfo.decode('utf-8', errors='replace') if loginfo else ''
            self.upgradeService.dbus_send.InstalldebStatusChanged(
                int(progress), str(state), details)
        except Exception as error:
            logging.error('SDK install status callback error: %s', error)

    def package_interface_install(self, uri, install_mode):
        self.installPkgName = os.path.basename(uri)
        if not self.installLoaded:
            logging.error('kdk_package_install_package load library error')
            return False
        try:
            callback = self.KCallBack(self._status_changed)
            result = self.sdkInterface.sdkLib.kdk_package_install_package(
                str(uri).encode('utf-8'), int(install_mode), callback)
            logging.info('kdk_package_install_package result: %d', result)
            return result == self.KDK_PACKAGE_INSTALL_SUCCESS
        except Exception as error:
            logging.error('package_interface_install error: %s', error)
            return False

    def package_interface_send_interrupt_signal(self):
        try:
            interrupt = getattr(self.sdkInterface.sdkLib,
                                'kdk_package_interrupt_signal_handler')
            interrupt.argtypes = []
            interrupt.restype = None
            interrupt()
        except Exception as error:
            logging.warning('send SDK interrupt signal error: %s', error)

    def package_interface_remove(self, package_name):
        if not self.removeLoaded:
            logging.error('kdk_package_remove_package load library error')
            return False
        try:
            callback = self.KCallBack(self._remove_status_changed)
            result = self.sdkInterface.sdkLib.kdk_package_remove_package(
                str(package_name).encode('utf-8'), callback)
            logging.info('kdk_package_remove_package result: %d', result)
            return result == self.KDK_PACKAGE_INSTALL_SUCCESS
        except Exception as error:
            logging.error('package_interface_remove error: %s', error)
            return False

    def _remove_status_changed(self, progress, state, loginfo):
        try:
            details = loginfo.decode('utf-8', errors='replace') if loginfo else ''
            self.upgradeService.dbus_send.PurgePkgStatusChanged(
                int(progress), str(state), details)
        except Exception as error:
            logging.error('SDK remove status callback error: %s', error)
