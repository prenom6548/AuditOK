# /*
#  * Copyright (C) 2020, KylinSoft Co., Ltd.
#  *
#  * This program is free software: you can redistribute it and/or modify
#  * it under the terms of the GNU General Public License as published by
#  * the Free Software Foundation, either version 3 of the License, or
#  * (at your option) any later version.
#  *
#  * This program is distributed in the hope that it will be useful,
#  * but WITHOUT ANY WARRANTY; without even the implied warranty of
#  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  * GNU General Public License for more details.
#  *
#  * You should have received a copy of the GNU General Public License
#  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
#  */

import os
import ctypes
import logging
import subprocess

from enum import Enum
from SystemUpdater.packageManager.pmUtils import *

class UpdaterSDKInterface:
    def __init__(self, libraryPath = '', archRelatedLib = ''):

        self.sdkLib     = None
        self.sdkLoaded  = False

        try:
            if archRelatedLib:
                libraryPath = self.arch_related_library_path(archRelatedLib)

            if os.path.exists(libraryPath):
                self.sdkLib     = ctypes.cdll.LoadLibrary(libraryPath)
                self.sdkLoaded  = True
            else:
                logging.warning("%s dose not exist.", libraryPath)
        except Exception as e:
            logging.error("UpdaterSDKInterface init error: %s", e)

    def arch_related_library_path(self, archRelatedLib):
        archRelatedLibPath = ''

        try:
            multiarch = subprocess.check_output(['dpkg-architecture', '-qDEB_HOST_MULTIARCH'], timeout=3).decode().strip()
            archRelatedLibPath = os.path.join("/usr/lib", multiarch, archRelatedLib)
        except Exception as e:
            logging.error("arch_related_library_path error: %s", e)

        return archRelatedLibPath

    def function_exists(self, funcName):
        try:
            getattr(self.sdkLib, funcName)
            return True
        except AttributeError:
            return False


class KSettingsPrivate(ctypes.Structure):
    GMainContext = ctypes.c_void_p
    KSettingsSchema = ctypes.c_void_p
    GDBusConnection = ctypes.c_void_p

    _fields_ = [
        ("main_context", ctypes.POINTER(GMainContext)),
        ("schema", ctypes.POINTER(KSettingsSchema)),
        ("conn", ctypes.POINTER(GDBusConnection)),
        ("sub_id", ctypes.c_uint * 2)
    ]

class KSettings(ctypes.Structure):
    _fields_ = [
        ("parent_instance", ctypes.c_void_p),   # GObject parent_instance
        ("priv", ctypes.POINTER(KSettingsPrivate))
    ]

class UpdaterSDKConf2:
    KCONF2_LIBRARY   = 'libkyconf2.so'
    APPLICATION_NAME = 'kylin-update-frontend'
    GROUP_TO_LISTEN  = [
        'kylin-update-frontend.notification',
        'kylin-update-frontend.downloadlimit',
        'kylin-update-frontend.autodownload',
        'kylin-update-frontend.autobackup',
        'kylin-update-frontend.downloadsettings'
        ]
    GROUP_SETTINGS   = {}

    def __init__(self, parent = None):
        self.conf2Loaded      = False
        self.strategieService = parent
        self.sdkInterface     = UpdaterSDKInterface(archRelatedLib = self.KCONF2_LIBRARY)

        if not self.sdkInterface.sdkLoaded:
            logging.warning("'%s' load error", self.KCONF2_LIBRARY)
        else:
            self.conf2_interface_init()

    def conf2_interface_init(self):
        try:
            if self.sdkInterface.function_exists("kdk_conf2_new"):
                self.sdkInterface.sdkLib.kdk_conf2_new.argtypes  = [ctypes.c_char_p, ctypes.c_char_p]
                self.sdkInterface.sdkLib.kdk_conf2_new.restype   = ctypes.POINTER(KSettings)

                for gtl in self.GROUP_TO_LISTEN:
                    self.GROUP_SETTINGS.update({gtl:self.sdkInterface.sdkLib.kdk_conf2_new(gtl.encode('utf-8'), None)})

            else:
                logging.warning("kdk_conf2_new do not find in '%s'", self.KCONF2_LIBRARY)
                self.conf2Loaded = False
                return

            if self.sdkInterface.function_exists("kdk_conf2_get_string"):
                self.sdkInterface.sdkLib.kdk_conf2_get_string.argtypes = [ ctypes.POINTER(KSettings),
                                                                         ctypes.c_char_p]
                self.sdkInterface.sdkLib.kdk_conf2_get_string.restype  = ctypes.c_char_p
            else:
                self.conf2Loaded = False
                return

            if self.sdkInterface.function_exists("kdk_conf2_set_value"):
                self.sdkInterface.sdkLib.kdk_conf2_set_value.argtypes = [ ctypes.POINTER(KSettings),
                                                                        ctypes.c_char_p,
                                                                        ctypes.c_char_p]
                self.sdkInterface.sdkLib.kdk_conf2_set_value.restype  = ctypes.c_int
            else:
                self.conf2Loaded = False
                return

            if self.sdkInterface.function_exists("kdk_conf2_ksettings_destroy"):
                pass
            else:
                self.conf2Loaded = False
                return


            # TODO: ctypes使用kdk_conf2_connect_signal时回调无法触发...
            # KCallBack = ctypes.CFUNCTYPE(None, ctypes.c_void_p, ctypes.c_char_p, ctypes.c_void_p)

            # @KCallBack
            # def on_key_changed(setting, key, user_data):
            #     key_str = key.decode('utf-8')
            #     logging.info("'%s' change detected", key_str)

            # self.sdkInterface.sdkLib.kdk_conf2_connect_signal.argtypes = [ ctypes.POINTER(KSettings),
            #                                                   ctypes.c_char_p,
            #                                                   KCallBack,
            #                                                   ctypes.c_void_p]
            # self.sdkInterface.sdkLib.kdk_conf2_connect_signal.restype  = ctypes.c_ulong
            # self.sdkInterface.sdkLib.kdk_conf2_connect_signal(ksettings_ptr, b"changed", on_key_changed, None)

            self.conf2_listen_key_changed()

            self.conf2Loaded = True
        except Exception as e:
            logging.error("conf2_interface_init error: %s", e)

    def conf2_listen_key_changed(self):
        try:
            def conf2_key_changed_handler(schema, version, key):
                if schema in self.GROUP_TO_LISTEN:
                    changed_value = self.conf2_get_key_value(schema, key).decode('utf-8')

                    logging.info("key_changed signal: %s(%s):%s -> %s",
                                 schema, version, key, changed_value)

                    if schema == 'kylin-update-frontend.notification':

                        if "enable" == key:
                            self.strategieService.dbus_send.AutocheckStatusChanged(
                                                True if str(changed_value).lower() == 'true' else False)

                    elif schema == 'kylin-update-frontend.downloadlimit':

                        if "enable" == key:
                            downloadlimitValue = self.conf2_get_key_value(schema, 'value')
                            self.strategieService.dbus_send.DownloadStatusChanged(
                                                True if str(changed_value).lower() == 'true' else False,
                                                int(downloadlimitValue))
                        
                        elif "value" == key:
                            downloadspeedLimitStatus = self.conf2_get_key_value(schema, 'enable')
                            self.strategieService.dbus_send.DownloadStatusChanged(
                                                True if downloadspeedLimitStatus.lower() == 'true' else False,
                                                int(str(changed_value)) if str(changed_value).isdigit() else int(0))

                    elif schema == 'kylin-update-frontend.autodownload':

                        if "enable" == key:
                            self.strategieService.dbus_send.AutoUpgradeStatusChanged(
                                                True if str(changed_value).lower() == 'true' else False)
            
            # conf2_service_proxy = self.strategieService.bus.get_object('com.kylin.kysdk.conf2',
            #                                                            '/com/kylin/kysdk/conf2',
            #                                                            follow_name_owner_changes=True)
            # conf2_service_proxy.connect_to_signal("key_changed", conf2_key_changed_handler)   
            self.strategieService.bus.add_signal_receiver(
                                            conf2_key_changed_handler,
                                            dbus_interface='com.kylin.kysdk.conf2',
                                            signal_name='key_changed'
                                        )

        except Exception as e:
            logging.error("conf2_listen_key_changed error: %s", e)

    def conf2_get_key_value(self, schema, key):
        updater_kconf2_value = ''

        try:
            updater_kconf2_value = self.sdkInterface.sdkLib.kdk_conf2_get_string(
                                                                self.GROUP_SETTINGS[schema],
                                                                key.encode('utf-8'))

            logging.info("conf2_get_key_value: %s:%s -> %s.", schema, key, updater_kconf2_value)
        except Exception as e:
            logging.error("conf2_get_key_value: %s",str(e))

        return updater_kconf2_value

    def conf2_set_key_value(self, schema, key, updater_kconf2_value):
        setStatus = True
        try:
            setStatus = self.sdkInterface.sdkLib.kdk_conf2_set_value(
                                                            self.GROUP_SETTINGS[schema],
                                                            key.encode('utf-8'),
                                                            updater_kconf2_value.encode('utf-8'))
            setStatus = True if setStatus == int(0) else False
            logging.info("conf2_set_key_value: %s:%s -> %s.", schema, key, updater_kconf2_value)
        except Exception as e:
            logging.error("conf2_set_key_value: %s",str(e))
            setStatus = False

        return setStatus

    def quitKConf2Manager(self):
        pass


class InstallMode(Enum):
    KDK_PACKAGE_AUTO    = 0
    KDK_PACKAGE_KARE    = 1
    KDK_PACKAGE_KAIMING = 2
    KDK_PACKAGE_DEBIAN  = 3

class UpdaterSDKPkgInstall:
    SDK_LIBRARY = 'libkypackage.so'
    KCallBack   = ctypes.CFUNCTYPE(None, ctypes.c_int, ctypes.c_int, ctypes.c_char_p)

    KDK_PACKAGE_INSTALL_ERROR       =  0
    KDK_PACKAGE_INSTALL_SUCCESS     =  1

    def __init__(self, parent = None):
        self.installLoaded    = False
        self.upgradeService   = parent
        self.sdkInterface     = UpdaterSDKInterface(archRelatedLib = self.SDK_LIBRARY)

        if not self.sdkInterface.sdkLoaded:
            logging.warning("'%s' load error", self.SDK_LIBRARY)
        else:
            self.package_interface_init()

        self.purge_status_changed           = self.KCallBack(self._purge_callback_wrapper)
        self.install_status_changed         = self.KCallBack(self._install_callback_wrapper)
        self.batch_install_status_changed   = self.KCallBack(self._batch_install_callback_wrapper)

        self.purgeFinished      = False
        self.installFinished    = False
        self.batchInstFinished  = False

        self.progress_restore   = 0
        self.batchInstPkgs      = []
        self.batchInstPkgsNum   = 0
        self.batchInstPkgsIndex = 0

    # state状态
    # 0: 卸载成功
    # 1: 正在卸载包
    # other: 错误
    def _purge_callback_wrapper(self, progress, state, loginfo):
        try:
            _loginfo = loginfo.decode("utf-8")
            # logging.info("purge_status_changed, progress: %d, state: %d, loginfo: %s.", progress, state, _loginfo)

            if state == 0:
                if progress == 100:
                    self.purgeFinished = True
                    self.upgradeService.dbus_send.PurgePackagesFinished(True, _loginfo,'')
                    self.upgradeService.dbus_send.PurgePackagesFinishedWithPkgname(True, _loginfo, '', '')
                    self.upgradeService.dbus_send.PurgePackagesFinishedWithErrCode(True, _loginfo, '', '', '')

            elif state == 1:
                if progress == 100:
                    self.purgeFinished = True
                    self.upgradeService.dbus_send.PurgePackagesFinished(True, _loginfo,'')
                    self.upgradeService.dbus_send.PurgePackagesFinishedWithPkgname(True, _loginfo, '', '')
                    self.upgradeService.dbus_send.PurgePackagesFinishedWithErrCode(True, _loginfo, '', '', '')
                else:
                    self.upgradeService.dbus_send.PurgePkgStatusChanged(progress, str(state), _loginfo)

            elif state < 0:
                self.purgeFinished = True
                self.upgradeService.dbus_send.PurgePackagesFinished(False, _loginfo,'')
                self.upgradeService.dbus_send.PurgePackagesFinishedWithPkgname(False, _loginfo, '', '')
                self.upgradeService.dbus_send.PurgePackagesFinishedWithErrCode(False, _loginfo, '', '', get_error_num_from_enum(ERROR_PURGE_PKG_DPKG))

            else:
                logging.info("purge_status_changed other, progress: %d, state: %d, loginfo: %s.", progress, state, _loginfo)

        except Exception as e:
            logging.error("_install_callback_wrapper error: %s", e)

    # state状态
    # 0: 安装成功
    # 1: 正在安装包
    # 2: 正在构建环境
    # other: 错误
    def _install_callback_wrapper(self, progress, state, loginfo):
        try:
            _loginfo = loginfo.decode("utf-8")
            # logging.info("install_status_changed, progress: %d, state: %d, loginfo: %s.", progress, state, _loginfo)

            if state == 0:
                if progress == 100:
                    self.installFinished = True
                    self.upgradeService.dbus_send.InstalldebFinished (True, '', _loginfo)
                    self.upgradeService.dbus_send.InstalldebFinishedWithErrCode (True, '', _loginfo, '')

            elif state == 1:
                if progress == 100:
                    self.installFinished = True
                    self.upgradeService.dbus_send.InstalldebFinished (True, '', _loginfo)
                    self.upgradeService.dbus_send.InstalldebFinishedWithErrCode (True, '', _loginfo, '')
                else:
                    self.upgradeService.dbus_send.InstalldebStatusChanged(progress, str(state), _loginfo)

            elif state == 2:
                if progress == 100:
                    self.upgradeService.dbus_send.EnvBuildStatusChanged(progress, str(_loginfo))
                    self.upgradeService.dbus_send.EnvBuildStatusFinished(True, str(_loginfo))
                else:
                    self.upgradeService.dbus_send.EnvBuildStatusChanged(progress, str(_loginfo))

            elif state < 0:
                self.installFinished = True
                self.upgradeService.dbus_send.InstalldebFinished (False, '', _loginfo)
                self.upgradeService.dbus_send.InstalldebFinishedWithErrCode (False, '', _loginfo, '')

            else:
                logging.info("install_status_changed other, progress: %d, state: %d, loginfo: %s.", progress, state, _loginfo)

        except Exception as e:
            logging.error("_install_callback_wrapper error: %s", e)

    def _batch_install_callback_wrapper(self, progress, state, loginfo):
        try:
            _loginfo = loginfo.decode("utf-8")
            _progress = int((progress * 100.0 / self.batchInstPkgsNum + self.batchInstPkgsIndex * 100 * 100.0 / self.batchInstPkgsNum) / 100)

            if _progress < self.progress_restore:
                _progress = self.progress_restore
            else:
                self.progress_restore = _progress

            logging.info("_batch_install_callback_wrapper(%d/%d[%d]), progress: %d, state: %d, loginfo: %s.", 
                         self.batchInstPkgsIndex + 1, self.batchInstPkgsNum, _progress, progress, state, _loginfo)

            if state == 0:
                if progress == 100:
                    if self.batchInstPkgsIndex == self.batchInstPkgsNum:
                        self.batchInstFinished = True
                        self.upgradeService.dbus_send.UpdateInstallFinished(True, self.batchInstPkgs, '', _loginfo)
                    else:
                        self.upgradeService.dbus_send.InstalldebStatusChanged(_progress, str(state), _loginfo)

            elif state == 1:
                if progress == 100:
                    if self.batchInstPkgsIndex == self.batchInstPkgsNum:
                        self.batchInstFinished = True
                        self.upgradeService.dbus_send.UpdateInstallFinished(True, self.batchInstPkgs, '', _loginfo)
                    else:
                        self.upgradeService.dbus_send.InstalldebStatusChanged(_progress, str(state), _loginfo)

                else:
                    self.upgradeService.dbus_send.InstalldebStatusChanged(_progress, str(state), _loginfo)

            elif state == 2:
                if progress == 100:
                    self.upgradeService.dbus_send.EnvBuildStatusChanged(progress, str(_loginfo))
                    self.upgradeService.dbus_send.EnvBuildStatusFinished(True, str(_loginfo))
                else:
                    self.upgradeService.dbus_send.EnvBuildStatusChanged(progress, str(_loginfo))

            elif state < 0:
                if self.batchInstPkgsIndex == self.batchInstPkgsNum:
                    self.batchInstFinished = True
                    self.upgradeService.dbus_send.UpdateInstallFinished(False, self.batchInstPkgs, '', _loginfo)
                else:
                    self.upgradeService.dbus_send.InstalldebStatusChanged(_progress, str(state), _loginfo)

            else:
                logging.info("_batch_install_callback_wrapper other, progress: %d, state: %d, loginfo: %s.", progress, state, _loginfo)

        except Exception as e:
            logging.error("_batch_install_callback_wrapper error: %s", e)

    def package_interface_init(self):
        try:
            if self.sdkInterface.function_exists("kdk_package_install_package"):
                self.sdkInterface.sdkLib.kdk_package_install_package.argtypes  = [ctypes.c_char_p, ctypes.c_int, self.KCallBack]
                self.sdkInterface.sdkLib.kdk_package_install_package.restype   = ctypes.c_int
            else:
                logging.warning("kdk_package_install_package do not find in '%s'", self.SDK_LIBRARY)
                self.conf2Loaded = False
                return

            if self.sdkInterface.function_exists("kdk_package_install_package_online"):
                self.sdkInterface.sdkLib.kdk_package_install_package_online.argtypes  = [ctypes.c_char_p, ctypes.c_int, self.KCallBack]
                self.sdkInterface.sdkLib.kdk_package_install_package_online.restype   = ctypes.c_int
            else:
                logging.warning("kdk_package_install_package_online do not find in '%s'", self.SDK_LIBRARY)
                self.conf2Loaded = False
                return

            if self.sdkInterface.function_exists("kdk_package_install_package_offline"):
                self.sdkInterface.sdkLib.kdk_package_install_package_offline.argtypes  = [ctypes.c_char_p, ctypes.c_int, self.KCallBack]
                self.sdkInterface.sdkLib.kdk_package_install_package_offline.restype   = ctypes.c_int
            else:
                logging.warning("kdk_package_install_package_offline do not find in '%s'", self.SDK_LIBRARY)
                self.conf2Loaded = False
                return

            if self.sdkInterface.function_exists("kdk_package_remove_package"):
                self.sdkInterface.sdkLib.kdk_package_remove_package.argtypes  = [ctypes.c_char_p, self.KCallBack]
                self.sdkInterface.sdkLib.kdk_package_remove_package.restype   = ctypes.c_int
            else:
                logging.warning("kdk_package_remove_package do not find in '%s'", self.SDK_LIBRARY)
                self.conf2Loaded = False
                return

            # 以下接口不影响sdk安装卸载操作
            if self.sdkInterface.function_exists("kdk_package_get_name"):
                self.sdkInterface.sdkLib.kdk_package_get_name.argtypes  = [ctypes.c_char_p]
                self.sdkInterface.sdkLib.kdk_package_get_name.restype   = ctypes.c_char_p
            else:
                logging.warning("kdk_package_get_name do not find in '%s'", self.SDK_LIBRARY)

            if self.sdkInterface.function_exists("kdk_package_is_installed"):
                self.sdkInterface.sdkLib.kdk_package_is_installed.argtypes  = [ctypes.c_char_p, ctypes.c_char_p]
                self.sdkInterface.sdkLib.kdk_package_is_installed.restype   = ctypes.c_int
            else:
                logging.warning("kdk_package_is_installed do not find in '%s'", self.SDK_LIBRARY)

            self.installLoaded = True
        except Exception as e:
            logging.error("conf2_interface_init error: %s", e)

    def package_interface_install(self, uri):
        _success = False

        try:
            if not self.installLoaded:
                logging.error("kdk_package_install_package load library error")
                return _success

            logging.info("kdk_package_install_package try to install: %s", uri)

            iRet = self.sdkInterface.sdkLib.kdk_package_install_package (uri.encode('utf-8'),
                                                        InstallMode.KDK_PACKAGE_AUTO.value,
                                                        self.install_status_changed)

            logging.info("kdk_package_install_package iRet:%d", iRet)

            if iRet == self.KDK_PACKAGE_INSTALL_ERROR:
                logging.info("kdk_package_install_package KDK_PACKAGE_INSTALL_ERROR")
            elif iRet == self.KDK_PACKAGE_INSTALL_SUCCESS:
                _success = True
                logging.info("kdk_package_install_package KDK_PACKAGE_INSTALL_SUCCESS")

        except Exception as e:
            logging.error("package_interface_install error: %s", e)
        
        return _success

    def package_interface_batch_install(self, pkgs_list):
        _success = False
        iRet     = self.KDK_PACKAGE_INSTALL_ERROR

        try:
            self.batchInstPkgs = pkgs_list

            if not self.installLoaded:
                logging.error("kdk_package_install_package load library error")
                return _success

            logging.info("kdk_package_install_package try to install: %s", ",".join(pkgs_list))
            self.batchInstPkgsNum   = len(pkgs_list)

            for pl in pkgs_list:
                iRet = self.sdkInterface.sdkLib.kdk_package_install_package (pl.encode('utf-8'),
                                                            InstallMode.KDK_PACKAGE_AUTO.value,
                                                            self.batch_install_status_changed)
                self.batchInstPkgsIndex += 1

                if iRet == self.KDK_PACKAGE_INSTALL_ERROR:
                    break

            logging.info("kdk_package_install_package iRet:%d", iRet)

            if iRet == self.KDK_PACKAGE_INSTALL_ERROR:
                logging.info("kdk_package_install_package KDK_PACKAGE_INSTALL_ERROR")
            elif iRet == self.KDK_PACKAGE_INSTALL_SUCCESS:
                _success = True
                logging.info("kdk_package_install_package KDK_PACKAGE_INSTALL_SUCCESS")

            self.package_interface_install_clear()
        except Exception as e:
            logging.error("package_interface_install error: %s", e)
        
        return _success

    def package_interface_remove(self, uri):
        _success      = False
        _package_name = ''

        try:
            if not self.installLoaded:
                logging.error("kdk_package_install_package load library error")
                return _success

            if os.path.exists(uri) and uri.endswith('.desktop'):
                _package_name = self.package_name_from_desktop (uri)
            else:
                _package_name = uri

            logging.info("kdk_package_remove_package try to remove: %s", _package_name)
            iRet = self.sdkInterface.sdkLib.kdk_package_remove_package (_package_name.encode('utf-8'),
                                                        self.purge_status_changed)

            logging.info("kdk_package_remove_package iRet:%d", iRet)

            if iRet == self.KDK_PACKAGE_INSTALL_ERROR:
                logging.info("kdk_package_remove_package KDK_PACKAGE_REMOVE_ERROR")
            elif iRet == self.KDK_PACKAGE_INSTALL_SUCCESS:
                _success = True
                logging.info("kdk_package_remove_package KDK_PACKAGE_REMOVE_SUCCESS")

        except Exception as e:
            logging.error("package_interface_remove error: %s", e)

        return _success

    def package_name_from_desktop(self, desktop_path):
        _package_name = ''

        try:
            if self.sdkInterface.sdkLib.kdk_package_get_name:
                logging.info("package_name_from_desktop try to get _package_name: %s", desktop_path)
                _package_name = self.sdkInterface.sdkLib.kdk_package_get_name (desktop_path.encode('utf-8')).decode('utf-8').strip()
            else:
                logging.error("kdk_package_get_name load library error")

            logging.info("package_name_from_desktop _package_name: %s", _package_name)

        except Exception as e:
            logging.error("package_name_from_desktop error: %s", e)

        return _package_name

    def package_interface_check_status(self, _name, _version):
        install_status = -100

        try:
            if self.sdkInterface.sdkLib.kdk_package_is_installed:
                if _name:
                    _name = _name.encode('utf-8')
                if _version:
                    _version = _version.encode('utf-8')
                logging.info("package_interface_check_status try to check _name: %s, _version:%s""", _name, _version)
                install_status = self.sdkInterface.sdkLib.kdk_package_is_installed (_name, _version)
            else:
                logging.error("kdk_package_is_installed load library error")

            logging.info("package_interface_check_status _name: %s, _version:%s, install_status:%s", 
                         _name, _version, install_status)

        except Exception as e:
            logging.error("package_interface_check_status error: %s", e)

        return install_status

    def package_interface_install_clear(self):
        try:
            self.progress_restore   = 0
            self.batchInstPkgs      = []
            self.batchInstPkgsNum   = 0
            self.batchInstPkgsIndex = 0

        except Exception as e:
            logging.error("package_interface_check_status error: %s", e)


class KSystemVersion(ctypes.Structure):
    _fields_ = [
        ("os_version", ctypes.c_char_p),
        ("update_version", ctypes.c_char_p)
    ]

class UpdaterSDKSysInfo:
    SDK_LIBRARY = 'libkysysinfo.so'

    def __init__(self):
        self.sysInfoLoaded    = False
        self.sdkInterface     = UpdaterSDKInterface(archRelatedLib = self.SDK_LIBRARY)

        if not self.sdkInterface.sdkLoaded:
            logging.warning("'%s' load error", self.SDK_LIBRARY)
        else:
            self.sysinfo_interface_init()

    def sysinfo_interface_init(self):
        try:
            if self.sdkInterface.function_exists("kdk_system_get_appScene"):
                self.sdkInterface.sdkLib.kdk_system_get_appScene.restype   = ctypes.c_char_p
            else:
                logging.warning("kdk_system_get_appScene do not find in '%s'", self.SDK_LIBRARY)
                return

            self.sysInfoLoaded = True
        except Exception as e:
            logging.error("package_interface_init error: %s", e)

    def sysinfo_interface_get_appScen(self):
        _sysinfo = ''

        try:
            if not self.sysInfoLoaded:
                logging.error("sysinfo_interface_get_appScen load library error")
                return _sysinfo

            _sysinfo = self.sdkInterface.sdkLib.kdk_system_get_appScene().decode('utf-8').strip()

            logging.info("sysinfo_interface_get_appScen _sysinfo: %s", _sysinfo)

        except Exception as e:
            logging.error("sysinfo_interface_get_appScen error: %s", e)

        return _sysinfo

    def sysinfo_interface_get_version_detaile(self):
        _os_version     = ''
        _update_version = ''

        try:
            if not self.sysInfoLoaded:
                logging.error("sysinfo_interface_get_version_detaile load library error")
                return _os_version, _update_version

            if self.sdkInterface.function_exists("kdk_system_get_version_detaile"):
                self.sdkInterface.sdkLib.kdk_system_get_version_detaile.restype   = KSystemVersion

            version_t = self.sdkInterface.sdkLib.kdk_system_get_version_detaile()
            _os_version     = version_t.os_version.decode('utf-8').strip()
            _update_version = version_t.update_version.decode('utf-8').strip()

            logging.info("sysinfo_interface_get_version_detaile _os_version: %s, _update_version: %s", _os_version, _update_version)

        except Exception as e:
            logging.error("sysinfo_interface_get_version_detaile error: %s", e)

        return _os_version, _update_version
