# UpdateManager.py
# -*- Mode: Python; indent-tabs-mode: nil; tab-width: 4; coding: utf-8 -*-
import os
import apt_pkg
import sys
import time
import shutil
import dbus
import logging
import dbus.service
import threading
import subprocess
import traceback
from apt import Cache
from gettext import gettext as _
from apt.debfile import DebPackage
from dbus.mainloop.glib import DBusGMainLoop
DBusGMainLoop(set_as_default=True)

from .Core.errors import *
from .Core.enums import *
from .Core.MyCache import MyCache
from .UpdateManagerDbus import UpdateManagerDbusController,UpdateManagerDbusControllerUtils,UPDATER_DBUS_INTERFACE,UPDATER_DBUS_PATH,UPDATER_DBUS_SERVICE
from .Core.UpdateList import UpdateList
from .Core.PluginManager import *
from .backend import InstallBackend,get_backend
from .Core.Database import Sqlite3Server
from .Core.loop import mainloop
from .Core.DataAcquisition import UpdateMsgCollector

from SystemUpdater.Core.LocalUpgradeData import LocalUpgradeDataList
from SystemUpdater.Core.UpdaterConfigParser import UpgradeConfig
from SystemUpdater.Core.JsonConfigParser import JsonConfig as json_config
from SystemUpdater.Core.utils import kill_process
from SystemUpdater.Core.DpkgInstallProgress import LogInstallProgress
from SystemUpdater.Core.utils import deb_verify,PolicyKit_Authority,get_proc_from_dbus_name,whether_to_quit_uu,get_dist
from SystemUpdater.packageManager.pmWorker import PmWorker

class UpdateManager():
    BACKEND_PKG_NAME = 'kylin-system-updater'
    FRONTEND_PKG_NAME = "kylin-update-frontend"
    GROUPS_PKG_NAME = 'kylin-update-desktop-config'
    SOURCES_UPDATE_NAME = "kylin-software-properties"
    APTD_PKG_NAME = "aptdaemon"
    RUN_UNATTENDED_UPGRADE = '/var/run/unattended-upgrades.pid'
    RETRY_LIMIT_NUM = 2

    def __init__(self,options):
        try:
            self.options = options
            self.cache = None
            self.update_list = None
            self.init_config_aptdeamon = False
            self.aptd_lang_switch = False
            self.retry_limit = self.RETRY_LIMIT_NUM
            self.now_working = InstallBackend.ACTION_DEFUALT_STATUS
            #dbus
            self.dbusController = self._setup_dbus()
            self.dbusControllerUtils = self._setup_dbus_utils()
            #config
            self.configs_uncover = UpgradeConfig(defaults_dir="system-updater-defaults.conf")
            self.configs_cover = UpgradeConfig(name = "system-updater-coverable.conf")
            self.uuconfigs = UpgradeConfig(datadir = "/var/lib/unattended-upgrades/", name = "unattended-upgrades-policy.conf")
            #数据采集器
            self.collector = UpdateMsgCollector(self)
            #连接数据库
            self.sqlite3_server = Sqlite3Server(self)
            self.pm_class = pluginManagerClass()
            self.simulate_mode = SimulateTerminal()
            self.install_mode = UpdateInstallMode(self)
            self.apt_p2p_config = AptP2pConfigManager()
            self.main_meta = LocalUpgradeDataList()
            json_config()
            self._refresh_cache_only()

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

    #进行清空所有下载的文件
    def start_clean(self):
        clean_backend = get_backend(self, InstallBackend.ACTION_CLEAN)
        clean_backend.start()

    #进行修复破损的包的操作 apt install -f 
    def start_fix_broken(self):
        fix_backend = get_backend(self, InstallBackend.ACTION_FIX_BROKEN)
        fix_backend.start()
    
    #进行 dpkg --configure
    def start_fix_incomplete(self):
        fix_backend = get_backend(self, InstallBackend.ACTION_FIX_INCOMPLETE)
        fix_backend.start()

    #进行升级的操作
    def start_install(self,upgrade_mode = InstallBackend.MODE_DEFAULT_STATUS,not_resolver = False,push_content = []):
        try:
            if self.install_mode.shutdown_mode() == True and upgrade_mode != InstallBackend.MODE_INSTALL_SINGLE:
                
                #部分升级的方式 计算的时候 补上上次更新的内容一起计算
                if upgrade_mode == InstallBackend.MODE_INSTALL_PARTIAL:
                    push_content += self.install_mode.tmp_content

                if not_resolver == True:
                    # if whether_to_quit_uu():
                    #     kill_process(self.RUN_UNATTENDED_UPGRADE)

                    self.check_config_aptdeamon()
                    install_backend = get_backend(self, InstallBackend.ACTION_DOWNLOADONLY,upgrade_mode)
                    install_backend.start(push_content)
                else:
                    resolver_backend = get_backend(self, InstallBackend.ACTION_CHECK_RESOLVER,upgrade_mode)
                    resolver_backend.start_resolver(push_content)
            else:
                if self.main_meta.is_steps(push_content) == True and \
                                upgrade_mode != InstallBackend.MODE_INSTALL_SYSTEM:
                    upgrade_mode = InstallBackend.MODE_INSTALL_STEP
                    
                if not_resolver == True:
                    # if whether_to_quit_uu():
                    #     kill_process(self.RUN_UNATTENDED_UPGRADE)

                    self.check_config_aptdeamon()
                    install_backend = get_backend(self, InstallBackend.ACTION_INSTALL,upgrade_mode)
                    install_backend.start(push_content)
                else:
                    resolver_backend = get_backend(self, InstallBackend.ACTION_CHECK_RESOLVER,upgrade_mode)
                    resolver_backend.start_resolver(push_content)
        except Exception as e:
            logging.error(e)

    #进行更新的操作
    def start_update(self,update_mode = InstallBackend.MODE_UPDATE_ALL):
        try:
            #更新前的准备
            # self.install_mode.reset_shutdown_mode()

            self.install_mode.check_source()

            if self.install_mode.check_network() == True:
                self.dbusController.check_connectivity()

            if self.install_mode.update_important() == True:
                self.dbusController.on_update_important_list()
                return

            #不进行update
            if self.options.no_update:
                self.start_available()
                return

            self.start_update_backend(update_mode = update_mode)
        
        except UpdateBaseError as excep:
            self.dbusController.InstallDetectStatus(False,get_error_code_from_enum(excep.code))
            self.dbusController.UpdateDetectFinished(False,[''],excep.header,excep.desc)
        except UpdateProgressExit:
            pass
        except Exception as e:
            logging.error(e)
            traceback.print_exc()
            self.dbusController.InstallDetectStatus(False,get_error_code_from_enum(ERROR_PROGRAM_EXCEPTION))
            self.dbusController.UpdateDetectFinished(False,[''],get_error_string_from_enum(ERROR_PROGRAM_EXCEPTION),\
                                                                get_error_description_from_enum(ERROR_PROGRAM_EXCEPTION))

    def start_update_backend(self,update_mode = InstallBackend.MODE_UPDATE_ALL):
        #调用aptdeamon进行update
        update_backend = get_backend(self, InstallBackend.ACTION_UPDATE,update_mode)
        update_backend.start()

    def start_available(self):
        try:
            self.configs_cover.reRead()

            self.retry_limit = self.RETRY_LIMIT_NUM

            json_config().read()

            self.refresh_cache()

            self.main_meta.refresh()

            self.update_list = UpdateList(self)

            if self.configs_uncover.getWithDefault("SystemStatus", "abnormal_reboot", False) == True:
                self.configs_uncover.setValue("SystemStatus","abnormal_reboot",str(False),True)
                logging.warning("start fix Abnormal Reboot broken pkgs...")
                self.start_fix_broken()
                return 
            
            self._check_system_broken(self.cache)
            
            self._check_config_upgrade(self.cache)

            self._check_self_upgrade(self.cache)

            self.update_list.update_kylin(self.cache,self.install_mode.get_important_data(),self.install_mode.is_openkylin_desktop())

            self.dbusController.UpdateDetectFinished(True,self.main_meta.get_push(),'','')

        except UpdateBaseError as excep:
            self.dbusController.InstallDetectStatus(False,get_error_code_from_enum(excep.code))
            self.dbusController.UpdateDetectFinished(False,[''],excep.header,excep.desc)
        except UpdateProgressExit as excep:
            pass
        except Exception as e:
            logging.error(e)
            traceback.print_exc()
            self.dbusController.UpdateDetectFinished(False,[''],get_error_string_from_enum(ERROR_PROGRAM_EXCEPTION),\
                                                                get_error_description_from_enum(ERROR_PROGRAM_EXCEPTION))

    def refresh_cache(self):
        try:
            if self.cache is None:
                self.cache = MyCache(None)
            else:
                self.cache.open(None)
                self.cache._initDepCache()
        except AssertionError:
            raise UpdateBaseError(ERROR_SOFTWARE_INDEX_RROKEN)

        except SystemError as e:
            logging.error(str(e))
            raise UpdateBaseError(ERROR_NOT_INIT_PACKAGESINFIO)

    def _refresh_cache_only(self):
        self._reload_options_config()

        if self.cache is None:
            self.cache = MyCache(None)
        else:
            self.cache.open(None)
            self.cache._initDepCache()

    def _check_config_upgrade(self,cache):
        need_upgrade = False
        self_upgrade = []

        #config包
        for pkg_name in [self.GROUPS_PKG_NAME]:
            if pkg_name in cache:
                self_pkg = cache[pkg_name]
                if self_pkg.is_installed:
                    if self_pkg.is_upgradable:
                        logging.info("Check: groups JSON ConfigPkgs(%s) start upgrading From %s to %s...",pkg_name,\
                                        self_pkg.installed.source_version,self_pkg.candidate.source_version)
                        try:
                            self_pkg.mark_install(True, False, True)
                            self_upgrade.append(pkg_name)
                            need_upgrade = True
                        except SystemError:
                            logging.error("Check: mark %s to upgrade Failed...",pkg_name)
                            self.simulate_mode.thread_install([pkg_name])
                            raise UpdateBaseError(ERROR_NOT_CONFIGPKG_DEPENDENCIES)
                    else:
                        logging.info("Check: ConfigPkgs(%s:%s) No need to upgrade...",pkg_name,self_pkg.installed.source_version)
                else:
                    logging.info("Check: groups JSON ConfigPkgs(%s) start new installing...",pkg_name)
                    try:
                        self_pkg.mark_install(True, False, True)
                        self_upgrade.append(pkg_name)
                        need_upgrade = True
                    except SystemError:
                        logging.error("Check: mark %s to install Failed...",pkg_name)
                        self.simulate_mode.thread_install([pkg_name])
                        raise UpdateBaseError(ERROR_NOT_CONFIGPKG_DEPENDENCIES)
            else:
                logging.error("Check: groups JSON ConfigPkgs(%s) is not in Cache...",pkg_name)
                raise UpdateBaseError(ERROR_NOT_GROUPS_CONFIG)

        if need_upgrade == True:
            self.dbusController.UpdateDetectStatusChanged(95,_("Priority Upgrade Package being updated"))
            self.start_install(InstallBackend.MODE_INSTALL_SINGLE,True,push_content=self_upgrade)
            raise UpdateProgressExit()

    def _check_self_upgrade(self,cache):
        self_upgrade = []
        
        channel_config = json_config().getWithDefault("update_self_upgrade",default = None)
        if channel_config == None:
            logging.warning("Json: update_self_upgrade item is None...")
            upgrade_list =  [self.BACKEND_PKG_NAME,self.APTD_PKG_NAME,self.FRONTEND_PKG_NAME]
        else:
            upgrade_list = channel_config.get("upgrade_list",[self.BACKEND_PKG_NAME,self.APTD_PKG_NAME,self.FRONTEND_PKG_NAME])

        for pkg_name in upgrade_list:
            if pkg_name in cache:
                self_pkg = cache[pkg_name]
                if self_pkg.is_installed:
                    if self_pkg.is_upgradable:
                        logging.info("Check: (%s) will upgrading From %s to %s...",pkg_name,\
                                        self_pkg.installed.source_version,self_pkg.candidate.source_version)
                        self_upgrade.append(pkg_name)
                    else:
                        logging.info("Check: (%s:%s) No need to upgrade...",pkg_name,self_pkg.installed.source_version)
                else:
                    logging.info("Check: (%s:%s) start new installing...",pkg_name,self_pkg.candidate.version)
                    self_upgrade.append(pkg_name)
            else:
                logging.info("Check: (%s) The upgrade package is not in Cache...",pkg_name)

        if self_upgrade:
            try:
                cache.make_resolver_self_upgrade(self_upgrade)
            except Exception:
                raise UpdateBaseError(ERROR_NOT_SELFPKG_DEPENDENCIES)
            self.dbusController.UpdateDetectStatusChanged(95,_("Priority Upgrade Package being updated"))
            self.start_install(InstallBackend.MODE_INSTALL_SINGLE,True,push_content=self_upgrade)
            raise UpdateProgressExit()

    def _check_system_broken(self,cache):
        if cache.get_changes():
            cache.clear()
        if cache._depcache.broken_count or cache._depcache.del_count > 0 or \
            cache._depcache.inst_count > 0:
            self.simulate_mode.start_caculate(["apt-get", "install","-f","--simulate"],thread=True)
        else:
            logging.info("Check: System Apt Cache for Broken Successfully...")
            return
        fix_backend = get_backend(self, InstallBackend.ACTION_CHECK_BROKEN)
        fix_backend.start()
        raise UpdateProgressExit()

    def _setup_dbus(self):
        # check if there is another g-a-i already and if not setup one
        # listening on dbus
        bus = dbus.SystemBus()
        try:
            bus_name = dbus.service.BusName(UPDATER_DBUS_SERVICE,
                                            bus,
                                            do_not_queue=True)
            logging.info("Initiate dbus success ...")
            UpdateManagerDbusControllerUtils(self, bus_name)
            return  UpdateManagerDbusController(self, bus_name)
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
                    return  UpdateManagerDbusController(self, bus_name)
                except dbus.exceptions.NameExistsException:
                    the_other_guy = bus.get_object(UPDATER_DBUS_SERVICE,
                                                    UPDATER_DBUS_PATH)
                    the_other_guy.Quit(dbus_interface=UPDATER_DBUS_INTERFACE,
                                        timeout=300)
                    logging.error("Dbus has not withdrawn and retry reboot times:%d...",retry_reboot_times)
                    time.sleep(1)
    
    def _setup_dbus_utils(self):
        # check if there is another g-a-i already and if not setup one
        # listening on dbus
        bus = dbus.SystemBus()
        try:
            bus_name = dbus.service.BusName(UPDATER_DBUS_SERVICE,
                                            bus,
                                            do_not_queue=True)
            return  UpdateManagerDbusControllerUtils(self, bus_name)
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
                    return  UpdateManagerDbusController(self, bus_name)
                except dbus.exceptions.NameExistsException:
                    the_other_guy = bus.get_object(UPDATER_DBUS_SERVICE,
                                                    UPDATER_DBUS_PATH)
                    the_other_guy.Quit(dbus_interface=UPDATER_DBUS_INTERFACE,
                                        timeout=300)
                    logging.error("Dbus has not withdrawn and retry reboot times:%d...",retry_reboot_times)
                    time.sleep(1)

    def start_back_upgrade(self, pkglist):
        try:
            install_backend = get_backend(self, InstallBackend.ACTION_BACKGROUND_UPGRADE)
            install_backend.start_alone(push_content = pkglist)
        except Exception as e:
            logging.error(str(e))

    def start_deb_resolver(self, pkglist = []):
        try:
            install_backend = get_backend(self, InstallBackend.ACTION_INSTALL_DEB_RESOLVER)
            install_backend.start_alone(push_content = pkglist)
        except Exception as e:
            logging.error(str(e))

    # 进行本地deb包安装的操作
    # _check_local_dep : 是否查询本地依赖
    # _auto_satisfy    : 是否通过网络下载依赖
    def start_deb_install(self, deb_path = "", _check_local_dep = False, _auto_satisfy = False, source = '', sender=None):
        if str(deb_path).lower().endswith('.ok'):
            if not hasattr(self, 'pmworker') or self.pmworker is None:
                self.pmworker = PmWorker(self)
            if self.pmworker.trans_is_runing.value:
                logging.warning("SDK install transaction is running, ignore: %s", deb_path)
                return
            self.pmworker.start_deb_install(
                deb_path, _check_local_dep, _auto_satisfy, source, sender)
            return

        header = ''
        desc = ''
        absolute_path, debname = os.path.split(deb_path)
        self.sqlite3_server.deb_metadata.update({"current_install_debfile":deb_path})
        self.sqlite3_server.deb_metadata.update({"absolute_path":absolute_path})
        self.sqlite3_server.deb_metadata.update({"source":source})
        
        self.deb_obj = {}
        UpdateMsg = {}
        try:
            if not os.path.isfile(deb_path):
                logging.info("No such file or directory: %s.",deb_path)
                self.dbusController.InstalldebFinished(False,'No such file or directory .','')
                return
            # 验签提权
            sender_name = get_proc_from_dbus_name(sender)
            caller = get_caller_from_enum(sender_name)
            caller_trans = get_source_name_from_enum(sender_name)

            if not deb_verify(deb_path): #验签失败，提权
                if not self.sqlite3_server.deb_policy_keep:
                    (status,error_string) = PolicyKit_Authority(caller_trans+_(" requires authentication to install software packages."),
                                            sender = sender, InstPolicy = True,
                                            authentication = self.configs_uncover.getWithDefault("InstallAndPurge","install_authority",True),
                                            source=source)
                    if not status:
                        self.dbusController.InstalldebFinished(False,error_string,'')
                        return 
                    else:
                        logging.info("Start check deb policy timeout...")
                        
                        if error_string == "USI-policy-close" or error_string == "USI-policy-deter":
                            self.sqlite3_server.deb_policy_keep = False
                            self.sqlite3_server.deb_policy_timestamp = 0
                        else:
                            self.sqlite3_server.deb_policy_keep = True
                            def _check_deb_policy():
                                if self.sqlite3_server.deb_policy_timestamp % 10 == 0:
                                    logging.info("Checking for deb policy timeout(%d)...",self.sqlite3_server.deb_policy_timestamp)
                                if (self.sqlite3_server.deb_policy_timestamp <= 0):
                                    logging.warning("Deb policy timeout")
                                    self.sqlite3_server.deb_policy_keep = False
                                    return False
                                else:
                                    self.sqlite3_server.deb_policy_timestamp = self.sqlite3_server.deb_policy_timestamp - 1
                                return True

                            from gi.repository import GLib
                            self.sqlite3_server.deb_policy_timestamp = 60 * 5
                            GLib.timeout_add_seconds(1,_check_deb_policy)

                else:
                    self.sqlite3_server.deb_policy_timestamp = 60 * 5
                    logging.info("Deb policy keep, ignore...")

            self.deb_obj.update({"debname":str(debname)})
            self.deb_obj.update({"old_version":""})
            self.deb_obj.update({"source":str(caller)})
            UpdateMsg.update({"source":str(self.deb_obj.get("source","kylin-system-updater"))})
            deb_cache, ins, _isinstall = self._suit_install_mode(deb_path)
            UpdateMsg.update({"appname":str(self.debName)})
            self.sqlite3_server.deb_metadata.update({"debname":str(self.debName)})
            self.sqlite3_server.deb_metadata['deblist'].append(str(self.debName))
            UpdateMsg.update({"new_version":str(self.debVersion)})
            if self._is_broken > 0 or not self.cacheSatisfy or self._need_downgrade:
            # 走 dpkg 安装流程，说明本地apt环境已经损坏,or dep not satisfied or need downgrade
                dep_satisfy, header, desc = self._deb_install(deb_cache, deb_path, _check_local_dep, ins)
                if dep_satisfy:
                    self.dbusController.InstalldebFinished(True, header, desc)
                    UpdateMsg.update({"status":"success"})
                    UpdateMsg.update({"errorCode":" "})
                else:
                    self.dbusController.InstalldebFinished(False, header, desc)
                    UpdateMsg.update({"status":"failed"})
                    UpdateMsg.update({"errorCode":str(header+"， "+desc)})
                #dpkg发送数据
                if self.configs_uncover.getWithDefault("SystemStatus", "upload_installer_log", False) == True:
                    UpdateMsg.update({"old_version":str(self.deb_obj.get("old_version","None"))})
                    self.collector.Upgrade_Process_Msg(InstallBackend.ACTION_INSTALL_DEB, UpdateMsg.copy())
                    self.deb_obj = {}
            else:
            # apt 安装流程
                dep_satisfy, header, desc = self._attempt_depends(deb_cache, deb_path, _check_local_dep,_auto_satisfy, ins)
                if dep_satisfy:
                    install_backend = get_backend(self, InstallBackend.ACTION_INSTALL_DEB)
                    logging.info("source name: %s.", source)
                    install_backend.start_alone(push_content=deb_path,_is_install=_auto_satisfy,caller=source)
                else:
                    self.dbusController.InstalldebFinished(False, header, desc)
        except UpdateBaseError as excep:
            self.dbusController.InstalldebFinished(False,excep.header,excep.desc)
            UpdateMsg.update({"old_version":str(self.deb_obj.get("old_version","None"))})
            UpdateMsg.update({"errorCode":str(excep.header+"， "+excep.desc)})
            UpdateMsg.update({"status":"failed"})
            self.collector.Upgrade_Process_Msg(InstallBackend.ACTION_INSTALL_DEB, UpdateMsg.copy())
        except Exception as e:
            logging.info(str(e))
            traceback.print_exc()
            self.dbusController.InstalldebFinished(False, str(e), desc)
            UpdateMsg.update({"old_version":str(self.deb_obj.get("old_version","None"))})
            UpdateMsg.update({"errorCode":str(e)})
            UpdateMsg.update({"status":"failed"})
            self.collector.Upgrade_Process_Msg(InstallBackend.ACTION_INSTALL_DEB, UpdateMsg.copy())

    #进行删除的操作
    def start_purge_pkgs(self,pkgs_list):
        try:
            if not hasattr(self, 'pmworker') or self.pmworker is None:
                self.pmworker = PmWorker(self)
            if self.pmworker.start_purge_pkgs(pkgs_list):
                return

            # 是否有破损的包
            deb_cache = Cache()
            broken_count = deb_cache._depcache.broken_count
            if broken_count > 0:
                _success,header,desc = self._dpkg_purge_pkgs(pkgs_list)
                if _success == True:
                    logging.info(header)
                    # self.dbusController.PurgePackagesFinished(_success,'',desc," ".join(pkgs_list))
                    self.dbusController.PurgePackagesFinished(_success,'',desc)
                else:
                    # self.dbusController.PurgePackagesFinished(_success,header,desc," ".join(pkgs_list))
                    self.dbusController.PurgePackagesFinished(_success,header,desc)
            else:
                purge_backend = get_backend(self, InstallBackend.ACTION_REMOVE_PACKAGES)
                purge_backend.start(push_content = pkgs_list)
            
            deb_cache.close()
        except Exception as e:
            logging.error(e)
            traceback.print_exc()

    def _dpkg_purge_pkgs(self,pkgs_list):
        success = False
        pkg_list_str = " ".join(pkgs_list)
        args = ["dpkg", "-r"]
        args.extend([pkg_list_str])

        p = subprocess.run(args, stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
        success = p.returncode == 0
        return success,p.stdout,''

    def _reload_options_config(self):
        apt_pkg.config["DPkg::Options::"] = "--force-confold"
        options_new = list(set(apt_pkg.config.value_list("DPkg::Options")))
        for option in ("--force-confnew","--force-confdef"):
            if option in options_new:
                options_new.remove(option)
        apt_pkg.config.clear("DPkg::Options")
        for option in options_new:
            apt_pkg.config["DPkg::Options::"] = option
        if apt_pkg.config.find_b("APT::Install-Recommends",False) == True:
            apt_pkg.config.clear("APT::Install-Recommends")
        if apt_pkg.config.find_b("APT::Install-Suggests",False) == True:
            apt_pkg.config.clear("APT::Install-Suggests")

    def check_config_aptdeamon(self):
    #检查是否需要重新启动aptdeamon 目前需要重启的有限速功能
        if self.init_config_aptdeamon == True:
            self.init_config_aptdeamon = False
            self.dbusController.set_aptdeamon_environ("init","config")

    # 是否查找本地依赖
    def _attempt_depends(self,deb_cache, deb_path,_check_local_dep,_auto_satisfy, _install):  
        depends_list = []
        depends_pkg = []
        satisfy_list = []
        depends_count = 0
        _local_satisfy = False
        error_string = ''
        error_desc = ''
        absolute_path, debname = os.path.split(deb_path)
        # 依赖不满足的情况
        if len(_install) > 0:
            if _check_local_dep: #查找本地
                # 需要查找本地依赖
                if len(_install) > 0:
                    for pkg in deb_cache:
                        if pkg.marked_install and pkg.name != str(debname.split("_")[0]):
                            depends_pkg.append(pkg)
                        elif pkg.marked_upgrade and pkg.name != str(debname.split("_")[0]):
                            depends_pkg.append(pkg)
                if len(depends_pkg)>0: #查找本地deb包
                    depends_list = [debfile for debfile in os.listdir(absolute_path) if debfile.endswith(".deb")]
                    for depends in depends_pkg:
                        for debfile in depends_list:
                            if depends.name in debfile and depends.candidate.version in debfile:
                                #FIXME:检查depends包的合法性
                                depends_count += 1
                                satisfy_list.append(debfile)
                    if depends_count < len(depends_pkg):
                        #本地依赖不满足 判断源下载
                        if _auto_satisfy:
                            _local_satisfy = True
                        elif not _auto_satisfy:
                            _local_satisfy = False
                            error_string = str(debname.split("_")[0])+_("dependency is not satisfied")
                            error_desc   = ",".join(_install)
                            logging.error(error_string+ error_desc)
                    else:
                        #将应用包与依赖包拷贝至archive目录安装
                        try:
                            if debname not in satisfy_list:
                                satisfy_list.append(debname)
                            for satisfy in satisfy_list:
                                shutil.copy(os.path.join(absolute_path,satisfy),"/var/cache/apt/archives/")
                                logging.info("move debpkg: %s",satisfy)
                            _local_satisfy = True
                        except Exception as e:
                            logging.info(str(e))
                return _local_satisfy,error_string,error_desc
            elif not _check_local_dep and _auto_satisfy:
                _local_satisfy = True
                if _install:
                    error_string = str(debname.split("_")[0])+_("dependency is not satisfied will download")
                    error_desc   = ",".join(_install)
                    logging.error(error_string+error_desc)
                    return _local_satisfy,error_string,error_desc
            elif not _check_local_dep and not _auto_satisfy:
                _local_satisfy = False
                if _install:
                    error_string = str(debname.split("_")[0])+_("dependency is not satisfied")
                    error_desc   = ",".join(_install)
                    logging.error(error_string+error_desc)
                    return _local_satisfy,error_string,error_desc
        # 依赖满足
        else:
            _local_satisfy = True
            error_string = ''
            error_desc=''
            return _local_satisfy,error_string,error_desc
    
    def _deb_install(self, deb_cache, deb_path, _check_local_dep, _install):
        depends_list = []
        depends_pkg = []
        satisfy_list = []
        noSatisfyList = []
        depends_count = 0
        error_string = ''
        header = ''
        desc = ''
        absolute_path, debname = os.path.split(deb_path)
        # 依赖不满足的情况
        if len(_install) > 0 or len(self.noSatisfyList) > 0:
            if _check_local_dep: #查找本地
                # 需要查找本地依赖
                if len(self.noSatisfyList) > 0:
                    for nS in self.noSatisfyList:
                        if len(nS[0]) == 1:
                            noSatisfyList.append(str(nS[0][0]))
                        else :
                            noSatisfyList.append(str(nS[0][0]))
                if len(_install) > 0:
                    for pkg in deb_cache:
                        if pkg.marked_install and pkg.name != str(debname.split("_")[0]):
                            depends_pkg.append(pkg)
                        elif pkg.marked_upgrade and pkg.name != str(debname.split("_")[0]):
                            depends_pkg.append(pkg)
                if len(depends_pkg)>0 or len(noSatisfyList)>0: #查找本地deb包
                    depends_list = [debfile for debfile in os.listdir(absolute_path) if debfile.endswith(".deb")]
                    for depends in depends_pkg:
                        for debfile in depends_list:
                            if "%3a" in debfile:
                                debfile=debfile.replace("%3a",":")
                            if depends.name in debfile and depends.candidate.version in debfile:
                                depends_count += 1
                                satisfy_list.append(debfile)
                    for depends in noSatisfyList:
                        for debfile in depends_list:
                            if "%3a" in debfile:
                                debfile=debfile.replace("%3a",":")
                            if depends.split('_')[0] == debfile.split('_')[0] and depends.split('_')[1] == debfile.split('_')[1] and debfile not in satisfy_list:
                                depends_count += 1
                                satisfy_list.append(debfile)
                    if depends_count < len(noSatisfyList) or depends_count < len(depends_pkg):
                        #本地依赖不满足
                        error_string = str(debname.split("_")[0])+_("dependency is not satisfied")+", ".join(noSatisfyList)
                        logging.error(error_string)
                        header = error_string
                        return False,header, desc
                    else:
                        #安装依赖包
                        if debname not in satisfy_list:
                            satisfy_list.append(debname)
                        for satisfy in satisfy_list:
                            try:
                                deb = DebPackage(os.path.join(absolute_path,satisfy))
                                iprogress = LogInstallProgress(deb_path)
                                ret = deb.install(install_progress=iprogress)
                                if ret != 0:
                                    return False, iprogress.errormsg, desc
                            except Exception as e:
                                logging.error(e)
                                return False, str(e), desc
                        return True, header, desc
            elif not _check_local_dep:
                if _install:
                    error_string = str(debname.split("_")[0])+_("dependency is not satisfied")+":".join(_install)
                    logging.error(error_string)
                    header = error_string
                    return False,header, desc
        # 依赖满足
        else:
            try:
                deb = DebPackage(deb_path)
                iprogress = LogInstallProgress(deb_path)
                ret = deb.install(install_progress=iprogress)
                if ret != 0:
                    return False, iprogress.errormsg, desc
            except Exception as e:
                logging.error(e)
                return False, str(e), desc
            return True, header, desc

    def _suit_install_mode(self, deb_path):
        self._is_broken = False
        self.cacheSatisfy = False
        _is_install=False
        absolute_path, debname = os.path.split(deb_path)
        # 检查本地破损
        try:
            deb_cache = Cache()
            logging.info("Install deb_package, check broken")
            broken_count = deb_cache._depcache.broken_count
            self.debVersion = 0
            debPackage = DebPackage(deb_path, deb_cache)
            debPackage.check(allow_downgrade=True)
            self.debVersion = debPackage._sections["Version"]
            self.debName = debPackage.pkgname
            logging.info("Install deb_package, required changes")
            (install, remove, unauth) = debPackage.required_changes    # need in cach
            if broken_count > 0:
                self._is_broken = True
            else :
                self._is_broken = False
            if debname.split("_")[0] in deb_cache:
                pkg = deb_cache[debname.split("_")[0]]
                if pkg.is_installed:
                    self.deb_obj.update({"old_version":str(pkg.installed.version)})
        except apt_pkg.Error as e:
            logging.error(str(e))
            if "E:read" in str(e):
                raise UpdateBaseError(ERROR_READ_LOCAL_DEB)
            else:
                raise UpdateBaseError(ERROR_LOCAL_DEB_FORMAT)
        except Exception as e:
            logging.error(str(e))
            raise UpdateBaseError(ERROR_INSTALL_DEB_BASE)
        self._need_downgrade = False
        # 不满足的依赖列表
        depends = debPackage.depends
        self.noSatisfyList = self._gen_noSatisfyList(depends, deb_cache)
        _list = []
        # cache是否满足
        if len(install) == 0 and len(self.noSatisfyList) == 0:
            self.cacheSatisfy = True
        else:
            for ns in self.noSatisfyList:
                for or_group in ns:
                    for pkg in install:
                        if pkg == or_group[0]:
                            _list.append(ns)
            if len(_list) == len(self.noSatisfyList):
                self.cacheSatisfy = True
            else:
                self.cacheSatisfy = False
        logging.info("Cache satisfy is %r.",self.cacheSatisfy)
        return deb_cache,install,_is_install
    
    def _gen_noSatisfyList(self, depends, deb_cache):
        _noSatisfyList = []
        _group_satify=False
        providers = []
        for or_group in depends:
            for deb_info in or_group:
                debname,ver,oper = deb_info
                if ":" in debname:
                    debname=debname.split(":")[0]
                try:
                    if debname not in deb_cache:
                        if  deb_cache.is_virtual_package(debname):
                            logging.warning("The cache has no package named '%s', this is a virtual package...", debname)
                            providers = deb_cache.get_providing_packages(debname)
                        # hear!
                        if len(providers) < 1:
                            logging.error("Can not found depend %s.", debname)
                            continue
                        depname = providers[0].name
                        pkg = deb_cache[depname]
                        cand = deb_cache._depcache.get_candidate_ver(pkg._pkg)
                        if not cand:
                            _group_satify = False
                        if not apt_pkg.check_dep(cand.ver_str, oper, ver):
                            _group_satify = False
                        else:
                            _group_satify = True
                    else:
                        pkg = deb_cache[debname]
                        if (apt_pkg.check_dep(pkg.candidate.source_version, oper, ver)) or (pkg.installed and apt_pkg.check_dep(pkg.installed.version, oper, ver)) or (not pkg.installed and apt_pkg.check_dep(pkg.candidate.version, oper, ver)) and _group_satify == False:
                            # if not apt_pkg.check_dep(cand, oper, ver): #no candidate
                            _group_satify = True
                except Exception as e:
                    logging.error(str(e))
            if _group_satify == False and or_group not in _noSatisfyList:
                _noSatisfyList.append(or_group)
            _group_satify = False
        return _noSatisfyList

class MakeSourceInit():
    DIR_MRDIA = "/media/"
    MOUNT_SQUASHFS_PATH = "/media/kylin/kylin-test-upgrade/upgrade-pool/"
    def __init__(self):
        self.is_disc = False
        self.is_mounted = False

    def mount_squashfs(self,mount_source):
        args = ["mount", "-o","loop",mount_source,self.MOUNT_SQUASHFS_PATH]

        p = subprocess.run(args, stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
        logging.info(str(p.stdout))
        if p.returncode == 0:
            self.is_mounted = True
            return True,' '
        elif p.returncode == 1:
            self.is_mounted = True
            return True,' '
        else:
            self.is_mounted = False
            return False,str(p.stdout)

    def check_mount(self):
        if self.is_mounted == True:
            args = ["umount",self.MOUNT_SQUASHFS_PATH]
            logging.info("Will be to umount the offlinesource...")
            p = subprocess.run(args, stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
            logging.info(str(p.stdout))
            if p.returncode == 0:
                self.is_mounted = False
                return True
            else:
                return False

    #判断是否为光盘源
    #光盘源格式 deb file:///home/someone/packs/
    def check_source(self):
        logging.info("Check: Whether to use CD-ROM source updates Successfully...")
        if os.path.exists(self.DIR_MRDIA):
            for first_dir in os.listdir(self.DIR_MRDIA):
                #到/media/x 
                check_dir_one = self.DIR_MRDIA + first_dir + "/"
                
                if not os.path.isdir(check_dir_one):
                    continue
                
                for second_dir in os.listdir(check_dir_one):
                    #到/media/x/test
                    check_dir_two = check_dir_one + second_dir + "/"
                    
                    if not os.path.isdir(check_dir_two):
                        continue
                    check_file = check_dir_two + "ss.map"

                    logging.info("Check: CD-ROM source File(%s)",check_file)
                    if os.path.exists(check_file):
                        self.is_disc = True
                        logging.info("Use to CD-Source and Turn off NetworkCheck and CloseFiter...")
                        return
            #没有返回存在光盘源就说明不存在
            self.is_disc = False
            return
        else:
            self.is_disc = False
            return

class AptP2pConfigManager():
    APT_P2P_FILE = "/etc/apt-p2p/apt-p2p.conf"
    HEADER_DSC = "apt-p2p config(/etc/apt-p2p/apt-p2p.conf) is not exists..."
    def __init__(self):
        if os.path.exists(self.APT_P2P_FILE):
            self.p2pConfigs = UpgradeConfig(datadir = "/etc/apt-p2p/", name = "apt-p2p.conf")
        else:
            self.p2pConfigs = None
    
    def get_bootstrap(self):
        if self.p2pConfigs == None:
            return self.HEADER_DSC

        return self.p2pConfigs.getWithDefault("apt_p2p_Khashmir", "BOOTSTRAP", "Failed")

    def set_bootstrap(self,value):
        if self.p2pConfigs == None:
            return self.HEADER_DSC
        self.p2pConfigs.setValue("apt_p2p_Khashmir","BOOTSTRAP",str(value))

class SimulateTerminal():
    ZH_UNMET_DEPENDENCIES = '下列软件包有未满足的依赖关系：'
    EN_UNMET_DEPENDENCIES = 'The following packages have unmet dependencies:'
    def __init__(self):
        self.update_args = ["apt-get", "update"]
        self.install_args = ["apt-get", "install","--simulate"]

    def _sub_emulate(self,args):
        p = subprocess.run(args, stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
        logging.info(str(p.stdout))
        return p.stdout

    def start_caculate(self,args = [],thread = False):        
        tmp_str = ''
        if thread is True:
            threading_emulate = threading.Thread(target=self._sub_emulate,args=(args,))
            threading_emulate.start()
        else:
            p = subprocess.run(args, stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
            tmp_str = p.stdout
        return tmp_str

    def _emulate_install(self,pkgs):
        args = ["apt-get", "install","--simulate"]
        args = args + pkgs

        p = subprocess.run(args, stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
        logging.info(str(p.stdout))
        return p.stdout

    def emulate_update(self):
        args = ["apt-get", "update"]

        p = subprocess.run(args, stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
        logging.info(str(p.stdout))
        return p.stdout

    def thread_install(self,pkgs):
        threading_emulate = threading.Thread(target=self.dependencies_broken,args=(pkgs,True))
        threading_emulate.start()

    def thread_update(self):
        threading_emulate = threading.Thread(target=self.emulate_update)
        threading_emulate.start()
    
    def dependencies_broken(self,pkgs,thread=False):
        terminal_msg = self._emulate_install(pkgs)

        if len(terminal_msg) > 500:
            terminal_msg = ''
        if self.ZH_UNMET_DEPENDENCIES in terminal_msg:
            terminal_msg = '\n' + self.ZH_UNMET_DEPENDENCIES + terminal_msg.split(self.ZH_UNMET_DEPENDENCIES)[1]
        elif self.EN_UNMET_DEPENDENCIES in terminal_msg:
            terminal_msg = '\n' + self.EN_UNMET_DEPENDENCIES + terminal_msg.split(self.EN_UNMET_DEPENDENCIES)[1]
        else:
            terminal_msg = ''
        
        if thread == True:
            logging.info(terminal_msg)

        return terminal_msg

class UpdateInstallMode():
    OPENKYLIN_DISTTRIBUTOR = "Openkylin"
    KYLIN_DISTTRIBUTOR = "Kylin"
    SYSTEM_UPDATE_GROUPS = "kylin-update-desktop-system"
    DIR_MRDIA = "/media/"
    MOUNT_SQUASHFS_PATH = "/media/kylin/kylin-test-upgrade/upgrade-pool/"
    IMPORTANT_LIST_PATH = '/var/lib/kylin-software-properties/template/important.list'

    def __init__(self,parent):
        self.parent = parent
        self.is_disc = False
        self.is_mounted = False
        self.dist = get_dist()

        if self.shutdown_mode() == True:
            logging.info("Initialize Shutdown Install Model...")
            self.bus = dbus.SystemBus()
            self.logind_proxy = self.bus.get_object('org.freedesktop.login1', '/org/freedesktop/login1')
            self._prepare_shutdown_model()
            self.tmp_content = []
            self.inhibit_lock = None

    def is_openkylin_desktop(self):
        return True

    def check_network(self):
        if self.parent.options.no_check_network is False and self.is_disc == False:
            return True
        else:
            return False

    def update_important(self):
        if self.parent.options.no_update_source is False and self.is_openkylin_desktop() == False:
            return True
        else:
            return False

    def get_important_data(self):
        important_list = []
        if self.is_openkylin_desktop() == False:
            with open(self.IMPORTANT_LIST_PATH, 'r') as f:
                data = f.read()
                important_list = data.split()
        else:
            important_list = [self.SYSTEM_UPDATE_GROUPS]
        
        #去除重复内容
        important_list = list(set(important_list))

        return important_list
        
    def _plymouth_splash(self):
        if os.path.exists("/bin/plymouth"):
            logging.debug("Running plymouth --splash")
            subprocess.run(["/sbin/plymouthd", "--mode=shutdown","--attach-to-session"])
            subprocess.Popen(["/bin/plymouth", "show-splash","--wait"])
            subprocess.call(["/bin/plymouth","system-update","--progress=0"])

    def _set_inhibit_delay(self,delay_time):
        try:
            #首先设置systemd默认延长时间为1800
            obj = self.bus.get_object('org.freedesktop.login1', '/org/freedesktop/login1')
            getter_interface = dbus.Interface(
                        self.logind_proxy,
                        dbus_interface='org.freedesktop.login1.Manager')
            ret = getter_interface.SetExtraInhibitShutdownDelaySec(delay_time)
        except Exception as e:
            logging.error(e)

    def _inhibit_sleep(self):
        """
        Send a dbus signal to logind to not suspend the system, it will be
        released when the return value drops out of scope
        """
        try:
            from gi.repository import Gio, GLib
            connection = Gio.bus_get_sync(Gio.BusType.SYSTEM)

            self._set_inhibit_delay(1800)

            var, fdlist = connection.call_with_unix_fd_list_sync(
                'org.freedesktop.login1', '/org/freedesktop/login1',
                'org.freedesktop.login1.Manager', 'Inhibit',
                GLib.Variant('(ssss)',
                            ('shutdown',
                            'Kylin System Updater', 'Updating System',
                            'delay')),
                None, 0, -1, None, None)
            inhibitor = Gio.UnixInputStream(fd=fdlist.steal_fds()[var[0]])
            return inhibitor
        except Exception as e:
            logging.error(e)
            return None

    def _prompt_in_boot(self):
        #关机安装完成之后开机时进行提醒
        popconfigs = UpgradeConfig(datadir = "/opt/apt_result/", name = "ota_result")
        popconfigs.setValue("OTA","status","success")
        popconfigs.setValue("OTA","upgrade","1")

    def _start_install_shutdown(self):
        if self.manual_install() == True:
            self._plymouth_splash()
            install_backend = get_backend(self.parent, InstallBackend.ACTION_INSTALL_SHUTDOWN)
            install_backend.start()
        elif self.inhibit_lock != None:
            self.inhibit_lock.close()
            logging.info("No packages to be installed and Releasing the shutdown lock...")

    def _prepare_shutdown_model(self):
        def prepare_for_shutdown_handler(active):
            """ Handle PrepareForShutdown() """
            if not active:
                logging.warning("PrepareForShutdown(false) received, "
                                "this should not happen")

            logging.info("Prepare For Shutdown arrived, starting final iterations....")
            
            self._start_install_shutdown()

        self.logind_proxy.connect_to_signal(
                "PrepareForShutdown", prepare_for_shutdown_handler)
    
    def auto_install(self):
        return self.parent.configs_uncover.getWithDefault("InstallMode", "auto_install", False)

    def manual_install(self):
        return self.parent.configs_uncover.getWithDefault("InstallMode", "manual_install", False)

    def shutdown_mode(self):
        return self.parent.configs_uncover.getWithDefault("InstallMode", "shutdown_install", False)
    
    def set_shutdown_install(self,status=False):
        self.parent.configs_uncover.setValue("InstallMode","manual_install",str(status))

    def reset_shutdown_mode(self):
        if self.shutdown_mode() == True:
            self.tmp_content = []
            self.parent.configs_uncover.setValue("InstallMode","manual_install",str(False))

            #释放锁 更新完成时重新那锁
            if self.inhibit_lock != None:
                self.inhibit_lock.close()
                self.inhibit_lock = None

    def install_finished(self):
        self.parent.configs_uncover.setValue("InstallMode","manual_install",str(False))
        self.parent.configs_uncover.setValue("InstallMode","auto_install",str(False))

        self._prompt_in_boot()

    def check_install_required(self):
        if self.shutdown_model() == True:
            if self.manual_install() == True:
                logging.info("Now need to shutdown install in manual install model...")
                return 1
            elif self.auto_install() == True:
                logging.info("Now need to shutdown install in auto install model...")
                return 2
            else:
                return 0
        else:
            logging.info("No need to shutdown install in the normal model...")
            return 0

    def get_inhibit_lock(self):
        #只要是关机更新模式就监听抑制关机
        if self.inhibit_lock == None:
            self.inhibit_lock = self._inhibit_sleep()
            if self.inhibit_lock == None:
                    logging.error("Prepare inhibit lock failed...")

    #判断是否为光盘源
    #光盘源格式 deb file:///home/someone/packs/
    def check_source(self):
        logging.info("Check: Whether to use CD-ROM source updates Successfully...")
        if os.path.exists(self.DIR_MRDIA):
            for first_dir in os.listdir(self.DIR_MRDIA):
                #到/media/x 
                check_dir_one = self.DIR_MRDIA + first_dir + "/"
                
                if not os.path.isdir(check_dir_one):
                    continue
                
                for second_dir in os.listdir(check_dir_one):
                    #到/media/x/test
                    check_dir_two = check_dir_one + second_dir + "/"
                    
                    if not os.path.isdir(check_dir_two):
                        continue
                    check_file = check_dir_two + "ss.map"

                    logging.info("Check: CD-ROM source File(%s)",check_file)
                    if os.path.exists(check_file):
                        self.is_disc = True
                        logging.info("Use to CD-Source and Turn off NetworkCheck and CloseFiter...")
                        return
            #没有返回存在光盘源就说明不存在
            self.is_disc = False
            return
        else:
            self.is_disc = False
            return

    # #当检查失败时 再切换到ping再进行一次检查
    # def mount_squashfs(self,mount_source):
    #     args = ["mount", "-o","loop",mount_source,self.MOUNT_SQUASHFS_PATH]

    #     p = subprocess.run(args, stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
    #     logging.info(str(p.stdout))
    #     if p.returncode == 0:
    #         self.is_mounted = True
    #         return True,' '
    #     elif p.returncode == 1:
    #         self.is_mounted = True
    #         return True,' '
    #     else:
    #         self.is_mounted = False
    #         return False,str(p.stdout)

    # def check_mount(self):
    #     if self.is_mounted == True:
    #         args = ["umount",self.MOUNT_SQUASHFS_PATH]
    #         logging.info("Will be to umount the offlinesource...")
    #         p = subprocess.run(args, stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
    #         logging.info(str(p.stdout))
    #         if p.returncode == 0:
    #             self.is_mounted = False
    #             return True
    #         else:
    #             return False
