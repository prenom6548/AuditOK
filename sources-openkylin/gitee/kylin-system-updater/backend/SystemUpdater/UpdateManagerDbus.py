#!/usr/bin/python3
import os
import dbus
import time
import dbus.service
import logging

import apt_pkg
from gettext import gettext as _
from .backend import InstallBackend
from .Core.loop import mainloop
import SystemUpdater.Core.enums as enums
from .Core.errors import *
from .Core.utils import humanize_size
from SystemUpdater.Core.utils import get_proc_from_dbus_name,PolicyKit_Authority
import locale
from xml.etree import ElementTree
from .Core.MyCache import MyCache
from importlib import reload

UPDATER_DBUS_INTERFACE = 'com.kylin.systemupgrade.interface'
UPDATER_DBUS_PATH = '/com/kylin/systemupgrade'
UPDATER_DBUS_SERVICE = 'com.kylin.systemupgrade'
RUN_UNATTENDED_UPGRADE = '/var/run/unattended-upgrades.pid'
SYSTEM_VERSION = '/etc/kylin-version/kylin-system-version.conf'
UPDATER_DBUS_PATH_UTILS = '/com/kylin/systemupgrade/utils'

#颜色设置
COLORLOG_SUFFIX = "\033[0m"

# Define some foreground colors
BLACK = 30
RED = 31
GREEN = 32
YELLOW = 33
BLUE = 34
MAGENTA = 35
CYAN = 36
WHITE = 37

#字体颜色
FRONT_COLOR_SEQ = "\033[1;%dm"
#背景颜色
BACK_COLOR_SEQ = "\033[%d;1m"

COLORLOG_PREFIX     = FRONT_COLOR_SEQ % GREEN                                                                 
COLORMETHOR_PREFIX  = FRONT_COLOR_SEQ % CYAN

UU_UPGRADE_MODE_TIMING = 0
UU_UPGRADE_MODE_BEFORE_SHUTDOWN = 1

#dbus 建立
class UpdateManagerDbusController(dbus.service.Object):
    """ this is a helper to provide the UpdateManagerIFace """

    RETURN_SUCCESS_CODE = 0
    RETURN_SUCCESS_DESC = ""

    RETURN_UNKNOWN_CODE = -1
    RETURN_UNKNOWN_DESC = ""

    RETURN_BUSY_STATE = 1
    RETURN_BUSY_DESC = "In the process of updating or Upgrading..."


    def __init__(self, parent, bus_name,
                object_path=UPDATER_DBUS_PATH):
        dbus.service.Object.__init__(self, bus_name, object_path)
        self.parent = parent
        self.bus = dbus.SystemBus()

        self.transaction = None


    def _update_important_reply(self,retval):
        if bool(retval) == False:
            self.InstallDetectStatus(False,enums.get_error_code_from_enum(enums.ERROR_UPDATE_SOURCE_FAILED))
            self.UpdateDetectFinished(False,[''],enums.get_error_string_from_enum(enums.ERROR_UPDATE_SOURCE_FAILED),\
                    enums.get_error_description_from_enum(enums.ERROR_UPDATE_SOURCE_FAILED))
        else:
            self.parent.start_update_backend()

    def _update_important_error(self,retval):
        logging.error(str(retval))
        self.InstallDetectStatus(False,enums.get_error_code_from_enum(enums.ERROR_UPDATE_SOURCE_TIMEOUT))
        self.UpdateDetectFinished(False,[''],enums.get_error_string_from_enum(enums.ERROR_UPDATE_SOURCE_TIMEOUT),\
                enums.get_error_description_from_enum(enums.ERROR_UPDATE_SOURCE_TIMEOUT))

    #更新important.list的本次升级的列表
    def on_update_important_list(self):
        self.UpdateDetectStatusChanged(10,_("Updating Source Template"))
        obj = self.bus.get_object('com.kylin.software.properties', '/com/kylin/software/properties')
        interface = dbus.Interface(obj, dbus_interface='com.kylin.software.properties.interface')
        interface.updateSourceTemplate(timeout=20,reply_handler=self._update_important_reply,error_handler=self._update_important_error)

    def is_reboot_required(self):
        """If a reboot is required to get all changes into effect."""
        return os.path.exists(os.path.join(apt_pkg.config.find_dir("Dir"),
                                        "var/run/reboot-required"))

    def is_logout_required(self):
        """If a logout is required to get all changes into effect."""
        return os.path.exists(os.path.join(apt_pkg.config.find_dir("Dir"),
                                        "var/run/logout-required"))

    #重启aptdeamon后台服务
    def make_aptdeamon_restart(self):
        try:
            obj = self.bus.get_object('org.debian.apt', '/org/debian/apt')
            interface = dbus.Interface(obj, dbus_interface='org.debian.apt')
            logging.info("Now start to restart Aptdeamon...")
            interface.Quit(timeout=0.5)
        except Exception as e:
            logging.error(str(e))

    #设置aptdeamon的环境变量
    def set_aptdeamon_environ(self,key,value):
        try:
            logging.info("Set aptdeaom environment variables %s = %s...",key,value)
            obj = self.bus.get_object('org.debian.apt', '/org/debian/apt')
            interface = dbus.Interface(obj, dbus_interface='org.debian.apt')
            retval = interface.SetEnviron(key,value)
            return retval
        except Exception as e:
            error_name = getattr(e, 'get_dbus_name', lambda: type(e).__name__)()
            logging.warning("Set aptdaemon environment failed: %s", error_name)
            if key == "init" and value == "config":
                self.make_aptdeamon_restart()
                time.sleep(0.5)
            return False

    def check_connectivity(self):
        try:
            self.UpdateDetectStatusChanged(5,_("Checking network connection"))
            obj = self.bus.get_object("org.freedesktop.NetworkManager","/org/freedesktop/NetworkManager")
            interface = dbus.Interface(obj, "org.freedesktop.DBus.Properties")
            retval = interface.Get("org.freedesktop.NetworkManager","Connectivity",timeout=0.5)
        except Exception:
            retval = 4
        #1 表示没有网卡可以使用
        if retval == 1:
            raise UpdateBaseError(enums.ERROR_NETWORK_FAILED)

    def check_connectivity_pro(self):
        try:
            obj = self.bus.get_object("org.freedesktop.NetworkManager","/org/freedesktop/NetworkManager")
            interface = dbus.Interface(obj, "org.freedesktop.NetworkManager")
            retval = interface.CheckConnectivity(timeout=15)
        except Exception as e:
            logging.error("Network status is Exception:" + str(e))
            return False

        if retval == 4:
            logging.info("Network status is Success...")
            return True
        else:
            logging.info("Network status(%d) is Exception...",retval)
            return False

    def _check_prohibit_user(self, sender_name):
        prohibit_list = ["dbus-send","gdbus"]
        if sender_name in prohibit_list:
            raise dbus.exceptions.DBusException("ERROR: You are not allowed to perform this action.")

    @dbus.service.method(UPDATER_DBUS_INTERFACE,
                        in_signature="", out_signature="",
                        sender_keyword="caller_name")
    def Quit(self, caller_name):
        """Request a shutdown of the daemon."""
        #如果在下载就请求 取消
        self.CancelDownload()
        logging.info("Quitting was requested")
        logging.debug("Quitting main loop...")
        mainloop.quit()
        logging.debug("Exit")

    #检查是否需要安装或者重启安装的请求
    @dbus.service.method(UPDATER_DBUS_INTERFACE,out_signature='i')
    def CheckInstallRequired(self):
        try:
            logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' CheckInstallRequired ...')
            return self.parent.install_mode.check_install_required()
        except Exception as e:
            logging.error(str(e))
            return 0

    #set config value
    @dbus.service.method(UPDATER_DBUS_INTERFACE,in_signature='sss',out_signature='b')
    def SetConfigValue(self,section, option, value):
        try:
            logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' SetConfigValue ...')
            if self.parent.configs_uncover.has_section(str(section)) and self.parent.configs_uncover.has_option(str(section),str(option)):
                if self.parent.configs_uncover.setValue(str(section), str(option),str(value)) == True:
                    return True
                else:
                    return False
            elif self.parent.sqlite3_server.ucconfigs.has_section(str(section)) and self.parent.sqlite3_server.ucconfigs.has_option(str(section),str(option)):
                if self.parent.sqlite3_server.ucconfigs.setValue(str(section), str(option),str(value)) == True:
                    return True
                else:
                    return False
        except Exception as e:
            logging.error(e)
            return False

    #get config value
    @dbus.service.method(UPDATER_DBUS_INTERFACE,in_signature='ss',out_signature='bs')
    def GetConfigValue(self,section, option):
        try:
            if self.parent.configs_cover.has_section(str(section)) and self.parent.configs_cover.has_option(str(section),str(option)):
                value = str(self.parent.configs_cover.get(str(section), str(option)))
                # logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+" GetConfigValue section:%s option:%s value:%s ...",section,option,value)
                return True,value
            if self.parent.configs_uncover.has_section(str(section)) and self.parent.configs_uncover.has_option(str(section),str(option)):
                value = str(self.parent.configs_uncover.get(str(section), str(option)))
                # logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+" GetConfigValue section:%s option:%s value:%s ...",section,option,value)
                return True,value
            elif self.parent.sqlite3_server.ucconfigs.has_section(str(section)) and self.parent.sqlite3_server.ucconfigs.has_option(str(section),str(option)):
                value = str(self.parent.sqlite3_server.ucconfigs.get(str(section), str(option)))
                # logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+" GetConfigValue section:%s option:%s value:%s ...",section,option,value)
                return True,value
            else:
                logging.warning("Warning: Can't found section:%s option:%s ... ",section, option)
        except Exception as e:
            logging.error("Error: GetConfigValue section:%s option:%s, %s.",section, option, e)
            return False,''

    #Remove all downloaded files.
    # @dbus.service.method(UPDATER_DBUS_INTERFACE,out_signature='b')
    # def Clean(self):
    #     try:
    #         #处于更新和升级中的话 不进行更新
    #         if self.parent.now_working != InstallBackend.ACTION_DEFUALT_STATUS:
    #             logging.warning('Clean In the process of updating or Upgrading...')
    #             return False,'In the process of updating or Upgrading...'
    #         else:
    #             logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' Clean ...')
    #             self.parent.start_clean()
    #             return True
    #     except Exception:
    #         return False

    #获取后端现在的状态
    @dbus.service.method(UPDATER_DBUS_INTERFACE,in_signature='s',out_signature='i')
    def GetBackendStatus(self,user_lang):
        try:
            logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' GetBackendStatus and user_lang = %s ...',str(user_lang))
            #当传入为空时 直接返回
            if str(user_lang) == '':
                logging.info("The incoming language is null...")
            else:
                (input_lang, encoding) = locale._parse_localename(str(user_lang))
                (local_lang, encoding) = locale._parse_localename(str(os.environ["LANG"]))
                (local_language, encoding) = locale._parse_localename(str(os.environ["LANGUAGE"]))
                
                #合法性的校验
                if input_lang == None or input_lang == '':
                    logging.info("The incoming language is Illegal...")
                    return self.parent.now_working

                #处于更新和升级中的话 不进行更新
                if input_lang != local_language or input_lang != local_lang:
                    logging.warning("LANG: switched %s to %s",os.environ["LANG"],user_lang)
                    logging.warning("LANGUAGE: switched %s to %s",os.environ["LANGUAGE"],user_lang)
                    os.environ["LANG"] = user_lang
                    os.environ["LANGUAGE"] = user_lang
                    self.parent.aptd_lang_switch = True
                    reload(enums)

            return self.parent.now_working
        except Exception as e:
            logging.error(str(e))
            return self.parent.now_working

    #apt install -f
    @dbus.service.method(UPDATER_DBUS_INTERFACE,out_signature='is')
    def FixBrokenDepends(self):
        try:
            #处于更新和升级中的话 不进行更新
            if self.parent.now_working != InstallBackend.ACTION_DEFUALT_STATUS:
                logging.warning('FixBrokenDepends In the process of updating or Upgrading...')
                return self.RETURN_BUSY_STATE,self.RETURN_BUSY_DESC
            else:
                logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' FixBrokenDepends ...')
                self.parent.start_fix_broken()
                return self.RETURN_SUCCESS_CODE,self.RETURN_SUCCESS_DESC
        except Exception as e:
            logging.error(str(e))
            return self.RETURN_UNKNOWN_CODE,str(e)

    #dpkg configure -a 
    @dbus.service.method(UPDATER_DBUS_INTERFACE,out_signature='is')
    def FixIncompleteInstall(self):
        try:
            #处于更新和升级中的话 不进行更新
            if self.parent.now_working != InstallBackend.ACTION_DEFUALT_STATUS:
                logging.warning('FixIncompleteInstall In the process of updating or Upgrading...')
                return self.RETURN_BUSY_STATE,self.RETURN_BUSY_DESC
            else:
                logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' FixIncompleteInstall ...')
                self.parent.start_fix_incomplete()
                return self.RETURN_SUCCESS_CODE,self.RETURN_SUCCESS_DESC
        except Exception as e:
            logging.error(str(e))
            return self.RETURN_UNKNOWN_CODE,str(e)
    
    #更新的dbus
    @dbus.service.method(UPDATER_DBUS_INTERFACE,out_signature='is',sender_keyword='sender')
    def UpdateDetect(self,sender=None):
        try:
            #处于更新和升级中的话 不进行更新
            if self.parent.now_working != InstallBackend.ACTION_DEFUALT_STATUS:
                logging.warning('UpdateDetect In the process of Updating or Upgrading...')
                return self.RETURN_BUSY_STATE,self.RETURN_BUSY_DESC
            else:
                sender_name = get_proc_from_dbus_name(sender)
                logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' UpdateDetect sender:%s...',sender_name)
                self._check_prohibit_user(sender_name)
                self.parent.start_update()
                return self.RETURN_SUCCESS_CODE,self.RETURN_SUCCESS_DESC
        except Exception as e:
            logging.error(str(e))
            return self.RETURN_UNKNOWN_CODE,str(e)

    #部分升级
    @dbus.service.method(UPDATER_DBUS_INTERFACE,in_signature='bas',out_signature='is',sender_keyword='sender')
    def DistUpgradePartial(self,_is_install,_partial_upgrade_list,sender=None):
        try:
            not_resolver = bool(_is_install)
            if not self.parent.update_list:
                logging.warning('Perform \"UpdateDetect\" first')
                return self.RETURN_BUSY_STATE,self.RETURN_BUSY_DESC
            #处于更新和升级中的话 不进行升级
            if self.parent.now_working != InstallBackend.ACTION_DEFUALT_STATUS:
                logging.warning('DistUpgradePartial In the process of updating or Upgrading...')
                return self.RETURN_BUSY_STATE,self.RETURN_BUSY_DESC
            else:
                sender_name = get_proc_from_dbus_name(sender)
                partial_upgrade_list = [str(i) for i in _partial_upgrade_list]
                logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' DistUpgradePartial sender:%s and not_resolver:%r, partial_upgrade_list:%s. ',sender_name,not_resolver,",".join(partial_upgrade_list))
                self._check_prohibit_user(sender_name)
                
                if partial_upgrade_list:
                    self.parent.start_install(InstallBackend.MODE_INSTALL_PARTIAL,not_resolver,partial_upgrade_list)
                    return self.RETURN_SUCCESS_CODE,self.RETURN_SUCCESS_DESC
                else:
                    return self.RETURN_BUSY_STATE,self.RETURN_BUSY_DESC
        except Exception as e:
            logging.info(str(e))
            return self.RETURN_UNKNOWN_CODE,str(e)

    #全部升级
    @dbus.service.method(UPDATER_DBUS_INTERFACE,in_signature='b',out_signature='is',sender_keyword='sender')
    def DistUpgradeAll(self,_is_install,sender=None):
        try:
            not_resolver = bool(_is_install)
            if not self.parent.update_list:
                logging.warning('Perform \"UpdateDetect\" first')
                return self.RETURN_BUSY_STATE,self.RETURN_BUSY_DESC
            #处于更新和升级中的话 不进行升级
            if self.parent.now_working != InstallBackend.ACTION_DEFUALT_STATUS:
                logging.warning('DistUpgradeAll In the process of updating or Upgrading...')
                return self.RETURN_BUSY_STATE,self.RETURN_BUSY_DESC
            else:
                sender_name = get_proc_from_dbus_name(sender)
                self._check_prohibit_user(sender_name)
                logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' DistUpgradeAll and not_resolver:%r...',not_resolver)
                self.parent.start_install(InstallBackend.MODE_INSTALL_ALL,not_resolver)
                return self.RETURN_SUCCESS_CODE,self.RETURN_SUCCESS_DESC
        except Exception as e:
            logging.info(str(e))
            return self.RETURN_UNKNOWN_CODE,str(e)
            
    # 全盘升级 也就是 apt dist-upgrade 升级的方式
    @dbus.service.method(UPDATER_DBUS_INTERFACE,in_signature='b',out_signature='is',sender_keyword='sender')
    def DistUpgradeSystem(self,_is_install,sender=None):
        try:
            not_resolver = bool(_is_install)
            #处于更新和升级中的话 不进行升级
            if self.parent.now_working != InstallBackend.ACTION_DEFUALT_STATUS:
                logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' DistUpgradeSystem and not_resolver:%r...',not_resolver)
                logging.warning('DistUpgradeSystem In the process of updating or Upgrading...')
                return self.RETURN_BUSY_STATE,self.RETURN_BUSY_DESC
            else:
                sender_name = get_proc_from_dbus_name(sender)
                self._check_prohibit_user(sender_name)
                logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' DistUpgradeSystem and not_resolver:%r...',not_resolver)
                self.parent.start_install(InstallBackend.MODE_INSTALL_SYSTEM,not_resolver)
                return self.RETURN_SUCCESS_CODE,self.RETURN_SUCCESS_DESC
        except Exception as e:
            logging.error(False, str(e))
            return self.RETURN_UNKNOWN_CODE,str(e)

    #卸载包
    @dbus.service.method(UPDATER_DBUS_INTERFACE,in_signature='asss',out_signature='is',sender_keyword='sender')
    def PurgePackages(self,_purge_list,cur_user,user_lang = '', sender=None):
        try:
            #当传入为空时 直接返回
            if str(user_lang) == '':
                logging.info("The incoming language is null...")
            else:
                (input_lang, encoding) = locale._parse_localename(str(user_lang))
                (local_lang, encoding) = locale._parse_localename(str(os.environ["LANG"]))
                (local_language, encoding) = locale._parse_localename(str(os.environ["LANGUAGE"]))
                
                #合法性的校验
                if input_lang == None or input_lang == '':
                    logging.info("The incoming language is Illegal...")
                    return self.RETURN_UNKNOWN_CODE,"The incoming language is Illegal..."

                #处于更新和升级中的话 不进行更新
                if input_lang != local_language or input_lang != local_lang:
                    logging.warning("LANG: switched %s to %s",os.environ["LANG"],user_lang)
                    logging.warning("LANGUAGE: switched %s to %s",os.environ["LANGUAGE"],user_lang)
                    os.environ["LANG"] = user_lang
                    os.environ["LANGUAGE"] = user_lang
                    self.parent.aptd_lang_switch = True
                    reload(enums)

            purge_list = [str(pkg) for pkg in _purge_list]
            sender_name = get_proc_from_dbus_name(sender)
            logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' DistPurgePackages Sender:%s and purge list is:%s...',sender_name, purge_list)
            
            if not self.parent.sqlite3_server.purge_policy_keep:
                (status, details) = PolicyKit_Authority(get_source_name_from_enum(sender_name)+_(" requires authentication to uninstall software packages."),
                                                        sender = sender, InstPolicy = False, 
                                                        authentication = self.parent.configs_uncover.getWithDefault("InstallAndPurge","purge_authority",False),  
                                                        source=sender_name)
                if not status:
                    self.PurgePackagesFinished(False,details,'')
                    return self.RETURN_UNKNOWN_CODE,details
                else:
                    logging.info("Start check purge policy timeout...")
                    self.parent.sqlite3_server.purge_policy_keep = True
                    def _check_purge_policy():
                        if self.parent.sqlite3_server.purge_policy_timestamp % 10 == 0:
                            logging.info("Checking for purge policy timeout(%d)...",self.parent.sqlite3_server.purge_policy_timestamp)
                        if (self.parent.sqlite3_server.purge_policy_timestamp <= 0):
                            logging.warning("Purge policy timeout")
                            self.parent.sqlite3_server.purge_policy_keep = False
                            return False
                        else:
                            self.parent.sqlite3_server.purge_policy_timestamp = self.parent.sqlite3_server.purge_policy_timestamp - 1
                        return True

                    from gi.repository import GLib
                    self.parent.sqlite3_server.purge_policy_timestamp = 60 * 5
                    GLib.timeout_add_seconds(1,_check_purge_policy)
            else:
                self.parent.sqlite3_server.purge_policy_timestamp = 60 * 5
                logging.info("Purge policy keep, ignore...")

            #目前只有360使用这个环境变量 当其他包也使用时 可以将这个权限放开
            if True:
                #需要对aptdeamon加这两个环境变量 才可以提示弹窗
                self.set_aptdeamon_environ("XAUTHORITY","/home/"+str(cur_user)+"/.Xauthority")
                self.set_aptdeamon_environ("DISPLAY",":0")
                
            # 处于更新和升级中的话 不进行升级
            if self.parent.now_working != InstallBackend.ACTION_DEFUALT_STATUS:
                # self.PurgePackagesFinished(False,_("Other tasks are being updated and upgraded, please uninstall them later."),''," ".join(purge_list))
                self.PurgePackagesFinished(False,_("Other tasks are being updated and upgraded, please uninstall them later."),'')
                logging.warning('PurgePackages In the process of updating or Upgrading...')
                return self.RETURN_BUSY_STATE,self.RETURN_BUSY_DESC
            else:
                self._check_prohibit_user(sender_name)

                self.parent.sqlite3_server.current_purge_pkgs = purge_list
                self.parent.start_purge_pkgs(purge_list)
                return self.RETURN_SUCCESS_CODE,self.RETURN_SUCCESS_DESC
        except Exception as e:
            logging.error(str(e))
            return self.RETURN_UNKNOWN_CODE,str(e)
            
    # 取消transaction 
    @dbus.service.method(UPDATER_DBUS_INTERFACE, out_signature='b')
    def CancelDownload(self):
        status = False
        try:
            logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' CancelDownload...')
            
            #空时直接返回
            if self.transaction == None:
                logging.info("transaction is none")
                return False
            
            if self.transaction.cancellable == True:
                self.transaction.cancel()
                status = True
                logging.info("dbus-mothod cancel task Success")
            else:
                status = False
                logging.info("cancel task Failed")
                
        except Exception as e:
            logging.error("cancel task Failed" + str(e))
            return status
        return status


    # 安装本地deb包
    @dbus.service.method(UPDATER_DBUS_INTERFACE,in_signature='ssbbs',out_signature='is',sender_keyword='sender')
    def InstallDebFile(self,source = "unKnown", path = "", _check_local_dep = False, _auto_satisfy = False, user_lang = '', sender=None):
        try:
            #当传入为空时 直接返回
            if str(user_lang) == '':
                logging.info("The incoming language is null...")
            else:
                (input_lang, encoding) = locale._parse_localename(str(user_lang))
                (local_lang, encoding) = locale._parse_localename(str(os.environ["LANG"]))
                (local_language, encoding) = locale._parse_localename(str(os.environ["LANGUAGE"]))
                
                #合法性的校验
                if input_lang == None or input_lang == '':
                    logging.info("The incoming language is Illegal...")
                    return self.RETURN_UNKNOWN_CODE,"The incoming language is Illegal..."

                #处于更新和升级中的话 不进行更新
                if input_lang != local_language or input_lang != local_lang:
                    logging.warning("LANG: switched %s to %s",os.environ["LANG"],user_lang)
                    logging.warning("LANGUAGE: switched %s to %s",os.environ["LANGUAGE"],user_lang)
                    os.environ["LANG"] = user_lang
                    os.environ["LANGUAGE"] = user_lang
                    self.parent.aptd_lang_switch = True
                    reload(enums)
            sender_name = get_proc_from_dbus_name(sender)
            self._check_prohibit_user(sender_name)

            check_local_dep = bool(_check_local_dep)
            auto_satisfy = bool(_auto_satisfy)
            deb_path = str(path)
            logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' InstallDebFile and check_local_dep:%r, auto_satisfy:%r, current_lang:%s , InstallDebFile sender: %s .',\
                                        check_local_dep,auto_satisfy,user_lang,sender_name)
            logging.info("Will install: %s.",path)
            self.parent.start_deb_install(deb_path, _check_local_dep, _auto_satisfy, source, sender)
            return self.RETURN_SUCCESS_CODE,self.RETURN_SUCCESS_DESC
        except Exception as e:
            logging.error(str(e))
            return self.RETURN_UNKNOWN_CODE,str(e)
    
    # commitpackages
    @dbus.service.method(UPDATER_DBUS_INTERFACE,in_signature='as',out_signature='is',sender_keyword='sender')
    def InstallPackages(self, pkg_list = [], sender=None):
        try:
            sender_name = get_proc_from_dbus_name(sender)
            self._check_prohibit_user(sender_name)
            logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' InstallPackages : [%s],  InstallPackages sender: %s .', ", ".join(list(pkg_list)), sender_name)
            self.parent.start_back_upgrade(pkg_list)
            return self.RETURN_SUCCESS_CODE,self.RETURN_SUCCESS_DESC
        except Exception as e:
            logging.error(str(e))
            return self.RETURN_UNKNOWN_CODE,str(e)

    #更新的dbus
    @dbus.service.method(UPDATER_DBUS_INTERFACE,out_signature='is',sender_keyword='sender')
    def UpdateCache(self,sender=None):
        try:
            #处于更新和升级中的话 不进行更新
            if self.parent.now_working != InstallBackend.ACTION_DEFUALT_STATUS:
                logging.warning('UpdateDetect In the process of Updating or Upgrading...')
                return self.RETURN_BUSY_STATE,self.RETURN_BUSY_DESC
            else:
                sender_name = get_proc_from_dbus_name(sender)
                logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' UpdateCache sender:%s...',sender_name)
                self._check_prohibit_user(sender_name)
                self.parent.start_update_backend(InstallBackend.MODE_UPDATE_CACHE)
                return self.RETURN_SUCCESS_CODE,self.RETURN_SUCCESS_DESC
        except Exception as e:
            logging.error(str(e))
            return self.RETURN_UNKNOWN_CODE,str(e)

    # dbus接口：向数据库display表中插入数据
    @dbus.service.method(UPDATER_DBUS_INTERFACE, in_signature='ss', out_signature='is', sender_keyword='sender')
    def InsertInstallState(self, item, value, sender=None):
        try:
            sender_name = get_proc_from_dbus_name(sender)
            logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' InsertInstallState, options:%s，value:%s,  InsertInstallState sender: %s .' % (item, value, sender_name))
            self.parent.sqlite3_server.insert_into_display(item, value)
            logging.info("Database inserted successfully，options:%s，value:%s" % (item, value))
            return self.RETURN_SUCCESS_CODE,self.RETURN_SUCCESS_DESC
        except Exception as e:
            logging.error("Database insert failed，options:%s，value:%s" % (item, value))
            return self.RETURN_UNKNOWN_CODE,str(e)

    # TODO 已迁移
    # 是否允许关机前更新
    @dbus.service.method(UPDATER_DBUS_INTERFACE, in_signature='ss', out_signature='is', sender_keyword='sender')
    def UnattendedUpgradeValue(self, operation, value="false", sender=None):
        sender_name = get_proc_from_dbus_name(sender)
        logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' UnattendedUpgradeValue sender:%s ', sender_name)
        if operation.lower() != "get" and operation.lower() != "set":
            return self.RETURN_UNKNOWN_CODE, 'Please input [\"set\", \"value\"] to set. \nor [\"get\"] to get whether updates are allowed before shutdown.'
        if operation == "set":
            try:
                logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' UnattendedUpgradeValue is going to %s [allow_unattended_upgrades_shutdown] value to %s.'%(operation,value))
                self.parent.sqlite3_server.insert_into_display("allow_unattended_upgrades_shutdown", value.lower())
            except Exception as e:
                logging.error(str(e))
                return self.RETURN_UNKNOWN_CODE,str(e)
        else:
            logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' UnattendedUpgradeValue is going to %s [allow_unattended_upgrades_shutdown] value.'%(operation))
            try:
                value = self.parent.sqlite3_server.select_from_display("allow_unattended_upgrades_shutdown")
                logging.info("[allow_unattended_upgrades_shutdown] value is %s."%(value))
            except Exception as e:
                logging.error(str(e))
                return self.RETURN_UNKNOWN_CODE,str(e)
        return self.RETURN_SUCCESS_CODE,self.RETURN_SUCCESS_DESC
    
    # TODO 已迁移
    # 设置自动更新时间
    @dbus.service.method(UPDATER_DBUS_INTERFACE, in_signature='s', out_signature='is',sender_keyword='sender')
    def SetAutoUpgradePeriod(self, period, sender = None):
        sender_name = get_proc_from_dbus_name(sender)
        logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' SetAutoUpgradePeriod will be set value %s, SetAutoUpgradePeriod sender: %s.'%(period, sender_name))
        try:
            self.parent.sqlite3_server.insert_into_display("update_period", period.lower())
            return self.RETURN_SUCCESS_CODE,self.RETURN_SUCCESS_DESC
        except Exception as e:
            logging.error(str(e))
            return self.RETURN_UNKNOWN_CODE,str(e)
    
    # TODO 已迁移
    # # dbus接口：改变apt下载速度
    @dbus.service.method(UPDATER_DBUS_INTERFACE, in_signature='sb', out_signature='is',sender_keyword='sender')
    def SetDownloadspeedMax(self, speed, set,sender = None):
        sender_name = get_proc_from_dbus_name(sender)
        logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' SetDownloadspeedMax, speed:%s, set:%r, sender name: %s .'%(speed, set, sender_name))
        #来重启Aptdeamon
        self.parent.init_config_aptdeamon = True
        if set:
            with open("/etc/apt/apt.conf.d/80apt-download", "w+") as f:
                try:
                    f.write("Acquire::http::Dl-Limit" + " \"" + "%s" % str(speed) + "\";\n")
                    f.write("Acquire::https::Dl-Limit" + " \"" + "%s" % str(speed) + "\";\n")
                    #更改数据库值
                    self.parent.sqlite3_server.insert_into_display("download_limit","true")
                    self.parent.sqlite3_server.insert_into_display("download_limit_value",str(speed))
                    #发送信号
                    self.ButtonStatusChange("speed" , str(speed))
                except Exception as e:
                    logging.error(e)
                    return self.RETURN_UNKNOWN_CODE,str(e)
        else:
            if os.path.exists("/etc/apt/apt.conf.d/80apt-download"):
                os.remove("/etc/apt/apt.conf.d/80apt-download")
                self.parent.sqlite3_server.insert_into_display("download_limit","false")
                self.ButtonStatusChange("speed", "0")
            else:
                self.parent.sqlite3_server.insert_into_display("download_limit","false")
                self.ButtonStatusChange("speed", "0")
        return self.RETURN_SUCCESS_CODE,self.RETURN_SUCCESS_DESC

    # TODO 已迁移
    # # dbus接口：获取apt下载速度
    @dbus.service.method(UPDATER_DBUS_INTERFACE, out_signature='bs',sender_keyword='sender')
    def GetDownloadspeedLimitValue(self,sender = None):
        sender_name = get_proc_from_dbus_name(sender)
        logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' GetDownloadspeedLimitValue sender: %s .', sender_name)
        try:
            download_limit = self.parent.sqlite3_server.select_from_display("download_limit")
            if download_limit == "true":
                download_limit_value = self.parent.sqlite3_server.select_from_display("download_limit_value")
                return True,str(download_limit_value)
            else:
                return False,str("0")
        except:
            return False, "0"
    

    ## dbus接口: 开启或关闭预下载功能
    @dbus.service.method(UPDATER_DBUS_INTERFACE, in_signature='b', out_signature='b',sender_keyword='sender')
    def SetPreDownloadState(self, _state,sender = None):
        state = bool(_state)
        sender_name = get_proc_from_dbus_name(sender)
        logging.info(COLORMETHOR_PREFIX+'method'+COLORLOG_SUFFIX+' SetPreDownloadState, state is %r, sender name: %s .',state,sender_name)
        try:
            if state:
                self.parent.uuconfigs.setValue("autoUpgradePolicy", "preDownload", "on", True)
            else :
                self.parent.uuconfigs.setValue("autoUpgradePolicy", "preDownload", "off", True)
            self.ChangeUpgradePolicy()
        except Exception as e:
            logging.error(str(e))
            return False
        return True

    ## dbus接口: 发送立即更新的信号
    @dbus.service.method(UPDATER_DBUS_INTERFACE, out_signature='b')
    def AutoUpgradeAllNow(self):
        logging.info(COLORMETHOR_PREFIX+'method'+COLORLOG_SUFFIX+' AutoUpgradeAllNow ...')
        try:
            self.UpgradeAllNow()
        except Exception as e:
            logging.error(str(e))
            return False
        return True

    ## dbus接口: 开启关闭自动更新功能
    @dbus.service.method(UPDATER_DBUS_INTERFACE, in_signature='b', out_signature='b')
    def SetAutoUpgradeState(self, _state):
        state = bool(_state)
        logging.info(COLORMETHOR_PREFIX+'method'+COLORLOG_SUFFIX+' SetAutoUpgradeState, state is %r ...',state)
        try:
            if state:
                self.parent.uuconfigs.setValue("autoUpgradePolicy", "autoUpgradeState", "on", True)
                self.parent.sqlite3_server.insert_into_display("autoupdate_allow", "true")
                self.ButtonStatusChange("autoUpgradeStatus", "true")
            else :
                self.parent.uuconfigs.setValue("autoUpgradePolicy", "autoUpgradeState", "off", True)
                # self.parent.uuconfigs.setValue("autoUpgradePolicy", "downloadMode", "manual", True)
                # self.parent.uuconfigs.setValue("autoUpgradePolicy", "installMode", "manual", True)
                self.parent.sqlite3_server.insert_into_display("autoupdate_allow", "false")
                self.ButtonStatusChange("autoUpgradeStatus", "false")
            self.ChangeUpgradePolicy()
            
        except Exception as e:
            logging.error(str(e))
            return False
        return True

    ## dbus接口: 设置自动更新策略
    @dbus.service.method(UPDATER_DBUS_INTERFACE, in_signature='is', out_signature='b')
    def SetAutoUpgradeMode(self, mode, time):
        logging.info(COLORMETHOR_PREFIX+'method'+COLORLOG_SUFFIX+' SetAutoUpgradeMode, mode is %s, time is %s ...',mode, time)
        try:
            if mode == UU_UPGRADE_MODE_TIMING:
                self.parent.uuconfigs.setValue("autoUpgradePolicy", "downloadMode", "timing", True)
                self.parent.uuconfigs.setValue("autoUpgradePolicy", "downloadTime", str(time), True)
                self.parent.uuconfigs.setValue("autoUpgradePolicy", "installMode", "timing", True)
                self.ButtonStatusChange("autoUpgradeTime", str(time))
            elif mode == UU_UPGRADE_MODE_BEFORE_SHUTDOWN:
                self.parent.uuconfigs.setValue("autoUpgradePolicy", "downloadMode", "timing", True)
                self.parent.uuconfigs.setValue("autoUpgradePolicy", "installMode", "bshutdown", True)
            self.ChangeUpgradePolicy()
        except Exception as e:
            logging.error(str(e))
            return False
        return True

    ## dbus接口: 获取自动更新属性信息
    @dbus.service.method(UPDATER_DBUS_INTERFACE, in_signature='s', out_signature='s')
    def Get(self, propertyName):
        propertyValue = ''
        retvalue = ''
        logging.info(COLORMETHOR_PREFIX+'method'+COLORLOG_SUFFIX+' Get, propertyName is %s ...',propertyName)
        try:
            if propertyName == "PreDownloadState":
                retvalue = self.parent.uuconfigs.getWithDefault("autoUpgradePolicy", "preDownload", "False")
                if retvalue == "on":
                    propertyValue = "on"
                elif retvalue == "off":
                    propertyValue = "off"
            elif propertyName == "AutoUpgradeState":
                retvalue = self.parent.uuconfigs.getWithDefault("autoUpgradePolicy", "autoUpgradeState", "False")
                if retvalue == "on":
                    propertyValue = "on"
                elif retvalue == "off":
                    propertyValue = "off"
            elif propertyName == "AutoUpgradeMode":
                retvalue = self.parent.uuconfigs.getWithDefault("autoUpgradePolicy", "downloadtime", "False")
                if retvalue == "before_shutdown": #关机前更新
                    propertyValue = "before_shutdown"
                elif ":" in retvalue: #定时更新
                    propertyValue = "timing+"+retvalue
            logging.info("Get '%s': %s",propertyName, propertyValue)
        except Exception as e:
            logging.error(str(e))
        return propertyValue

    # 监控是否需要重启的操作
    @dbus.service.method(UPDATER_DBUS_INTERFACE, in_signature='s', out_signature='b')
    def CheckRebootRequired(self, model):
        logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' CheckRebootRequired is be %s',str(model))
        try:
            #更新完成之后检查是否需要重启
            if self.is_reboot_required() == True:
                self.RebootLogoutRequired("reboot")
                logging.info("System need to Reboot...")
            elif self.is_logout_required() == True:
                self.RebootLogoutRequired("logout")
                logging.info("System need to Logout...")
            else:
                logging.info("System not need to Reboot or Logout...")
        except Exception as e:
            logging.error(str(e))
            return False
        return True

    # TODO 已迁移
    # kill 进程
    @dbus.service.method(UPDATER_DBUS_INTERFACE, in_signature='i', out_signature='b')
    def KillProcessSignal(self, pid):
        logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' KillProcessSignal is %d', pid)
        try:
            # 判断文件是否存在
            if (os.path.exists(RUN_UNATTENDED_UPGRADE)):
                os.kill(int(pid), 9)
                logging.info('%s has been killed', pid)
            else:
                logging.warning('%s is not exist.', RUN_UNATTENDED_UPGRADE)
        except Exception as e:
            logging.error(str(e))
            return False
        return True
    
    # TODO 已迁移
    # 获取数据库值
    @dbus.service.method(UPDATER_DBUS_INTERFACE, in_signature='bss', out_signature='s')
    def GetSetDatabaseInfo(self, gs, table, field):
        Text = 'NULL'
        try:
            if gs: #get 
                if table == 'display':
                    Text = self.parent.sqlite3_server.select_from_display(str(field))
                    logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' GetDatabaseInfo Table:%s Field:%s Text:%s',table,field,Text)
            else: #set 
                if table == 'display' and "=" in field:
                    field, value = str(field).split("=")
                    logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' SetDatabaseInfo Table:%s Field:%s', table, field)
                    self.parent.sqlite3_server.insert_into_display(field, value)
                    return "success"
        except Exception as e:
            logging.error(str(e))
            return Text
        return Text
    
    # 获取系统版本号
    @dbus.service.method(UPDATER_DBUS_INTERFACE, out_signature='ss',sender_keyword='sender')
    def GetSystemUpdateVersion(self,sender=None):
        sender_name = get_proc_from_dbus_name(sender)
        logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' GetSystemUpdateVersion sender: %s .', sender_name)
        os_version = ''
        update_version = ''
        try:
            self.parent.sqlite3_server._ensure_system_version_config()
            if not os.path.exists("/etc/kylin-version/kylin-system-version.conf"): 
                logging.warning("System version file doesn't exist.")
                update_version,os_version = self.parent.sqlite3_server.get_default_version()
                return str(os_version),str(update_version)
            os_version = str(self.parent.sqlite3_server.ucconfigs.get("SYSTEM","os_version"))
            update_version = str(self.parent.sqlite3_server.ucconfigs.get("SYSTEM","update_version"))
        except Exception as e:
            logging.error(str(e))
            return str(e),''
        logging.info('Current os_version: %s, release_id: %s .', os_version, update_version)
        return os_version,update_version

    # @dbus.service.method(UPDATER_DBUS_INTERFACE, in_signature='s', out_signature='bs')
    # def MountSquashfsSource(self, mount_source):
    #     logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' MountSquashfsSource %s',str(mount_source))
    #     try:
    #         ret,dsc = self.parent.source_info.mount_squashfs(str(mount_source))
    #         logging.info(COLORLOG_PREFIX+"Emitting"+COLORLOG_SUFFIX+" FixBrokenStatusChanged finished = %r dec = %s",ret,dsc)
    #         return ret,dsc
    #     except Exception as e:
    #         logging.error(str(e))
    #         return False,str(e)

    #更新进度信息 0~100 进度信息 101为非预期的信号
    @dbus.service.signal(UPDATER_DBUS_INTERFACE,signature='is')
    def UpdateDetectStatusChanged(self,progress,status):
        logging.info(COLORLOG_PREFIX+"Emitting"+COLORLOG_SUFFIX+" UpdateDetectStatusChanged progress = %d , status = %s",progress,status)
    
    #更新完成的信号
    @dbus.service.signal(UPDATER_DBUS_INTERFACE,signature='basss')
    def UpdateDetectFinished(self, success, upgrade_group,error_string='',error_desc=''):
        logging.info(COLORLOG_PREFIX + "Emitting"+ COLORLOG_SUFFIX + " UpdateDetectFinished success = %r , upgrade_group = %a, error_string = %s , error_desc = %s ",\
                    success,upgrade_group,error_string,error_desc)

    #升级的进度信息 0~100 进度信息 101为非预期的信号
    @dbus.service.signal(UPDATER_DBUS_INTERFACE,signature='asiss')
    def UpdateDloadAndInstStaChanged(self,groups_list,progress,status,current_details):
        logging.info(COLORLOG_PREFIX + "Emitting" + COLORLOG_SUFFIX +" UpdateDloadAndInstStaChanged upgrade groups_list = %s progress = %d , status = %s ,current_details = %s\033[0m",\
            groups_list,progress,status,current_details)

    #升级完成的信号
    @dbus.service.signal(UPDATER_DBUS_INTERFACE,signature='basss')
    def UpdateInstallFinished(self, success, upgrade_group,error_string='',error_desc=''):
        logging.info(COLORLOG_PREFIX + "Emitting" + COLORLOG_SUFFIX +" UpdateInstallFinished success = %r , upgrade_group = %a, error_string = %s , error_desc = %s ",\
                    success,upgrade_group, error_string,error_desc)

    #分步更新安装完成后的弹窗
    @dbus.service.signal(UPDATER_DBUS_INTERFACE,signature='bs')
    def PopupStepsInstalled(self, need_reboot,details_desc=''):
        logging.info(COLORLOG_PREFIX + "Emitting" + COLORLOG_SUFFIX +" PopupStepsInstalled success = %r ,details_desc = %s ",need_reboot,details_desc)

    #分步更新安装中的提示
    @dbus.service.signal(UPDATER_DBUS_INTERFACE,signature='bs')
    def NotifyStepsInstalled(self, reservations,details_desc=''):
        logging.info(COLORLOG_PREFIX + "Emitting" + COLORLOG_SUFFIX +" NotifyStepsInstalled reservations = %r ,details_desc = %s ",reservations,details_desc)

    #升级完成的信号
    @dbus.service.signal(UPDATER_DBUS_INTERFACE,signature='basss')
    def UpdateDownloadFinished(self, success, upgrade_group,error_string='',error_desc=''):
        logging.info(COLORLOG_PREFIX + "Emitting" + COLORLOG_SUFFIX +" UpdateDownloadFinished success = %r , upgrade_group = %a, error_string = %s , error_desc = %s ",\
                    success,upgrade_group, error_string,error_desc)

    #发送下载包信息 fix bug 字节大小改成u 无符号32位
    @dbus.service.signal(UPDATER_DBUS_INTERFACE, signature='asiiuui')
    def UpdateDownloadInfo(self,upgrade_group,current_items, total_items, currenty_bytes, total_bytes, current_cps):
        logging.info(COLORLOG_PREFIX + "Emitting" + COLORLOG_SUFFIX +" UpdateDownloadInfo upgrade_group = %a,current_items = %d, total_items = %d, currenty_bytes = %s, total_bytes = %s, current_cps = %s/s",\
                    upgrade_group,\
                    current_items, total_items, \
                    humanize_size(currenty_bytes), humanize_size(total_bytes),\
                    humanize_size(current_cps))

    #查询解决依赖 信号
    @dbus.service.signal(UPDATER_DBUS_INTERFACE,signature='bbasasasss')
    def UpdateDependResloveStatus(self, resolver_status, remove_status,remove_pkgs,pkg_raw_description,delete_desc,error_string='',error_desc=''):
        logging.info(COLORLOG_PREFIX + "Emitting" + COLORLOG_SUFFIX +" UpdateDependResloveStatus:resolver_status = %r , remove_status = %r , remove_pkgs = %a,pkg_raw_description = %s ,delete_desc = %s,error_string = %s , error_desc = %s ",\
            resolver_status,remove_status,remove_pkgs,pkg_raw_description,delete_desc,error_string,error_desc)

    #查询解决依赖 信号
    @dbus.service.signal(UPDATER_DBUS_INTERFACE,signature='bbasasasss')
    def UpdateFixBrokenStatus(self, resolver_status, remove_status,remove_pkgs,pkg_raw_description,delete_desc,error_string='',error_desc=''):
        logging.info(COLORLOG_PREFIX + "Emitting" + COLORLOG_SUFFIX +" UpdateFixBrokenStatus:resolver_status = %r , remove_status = %r , remove_pkgs = %a,pkg_raw_description = %s ,delete_desc = %s,error_string = %s , error_desc = %s ",\
            resolver_status,remove_status,remove_pkgs,pkg_raw_description,delete_desc,error_string,error_desc)

    #查询dist-upgrade解决依赖
    @dbus.service.signal(UPDATER_DBUS_INTERFACE,signature='bbasasasss')
    def DistupgradeDependResloveStatus(self, resolver_status, remove_status,remove_pkgs,pkg_raw_description,delete_desc,error_string='',error_desc=''):
        logging.info(COLORLOG_PREFIX + "Emitting" + COLORLOG_SUFFIX +" DistupgradeDependResloveStatus:resolver_status = %r , remove_status = %r , remove_pkgs = %a,pkg_raw_description = %s ,delete_desc = %s ,error_string = %s , error_desc = %s ",\
            resolver_status,remove_status,remove_pkgs,pkg_raw_description,delete_desc,error_string,error_desc)

    # 信号是否可取消
    @dbus.service.signal(UPDATER_DBUS_INTERFACE, signature='b')
    def Cancelable(self, Cancelable):
        pass
        # logging.info(COLORLOG_PREFIX + "Emitting" + COLORLOG_SUFFIX +" Cancelable: %r",Cancelable)
    
    # 插入数据库完成后发送
    @dbus.service.signal(UPDATER_DBUS_INTERFACE, signature='ss')
    def UpdateSqlitSingle(self, appname, date):
        logging.info(COLORLOG_PREFIX + "Emitting" + COLORLOG_SUFFIX +" UpdateSqlitSingle: [ %s ]: date: %s .",\
                    appname, date)

    #更新进度信息 0~100 进度信息 101为非预期的信号
    @dbus.service.signal(UPDATER_DBUS_INTERFACE,signature='bbisss')
    def FixBrokenStatusChanged(self,finished,success,progress,status,error_string='',error_desc=''):
        logging.info(COLORLOG_PREFIX+"Emitting"+COLORLOG_SUFFIX+" FixBrokenStatusChanged finished = %r , success = %r,progress = %d , status = %s,error_string = %s , error_desc = %s",\
            finished,success,progress,status,error_string,error_desc)

    #卸载完成的信号
    @dbus.service.signal(UPDATER_DBUS_INTERFACE,signature='bss')
    def PurgePackagesFinished(self, success,error_string='',error_desc=''):
        logging.info(COLORLOG_PREFIX + "Emitting"+ COLORLOG_SUFFIX + " PurgePackagesFinished success = %r , error_string = %s , error_desc = %s ",\
                    success,error_string,error_desc)

    #卸载进度信息 0~100 进度信息 101为非预期的信号
    @dbus.service.signal(UPDATER_DBUS_INTERFACE,signature='iss')
    def PurgePkgStatusChanged(self,progress,status,current_details):
        logging.info(COLORLOG_PREFIX + "Emitting" + COLORLOG_SUFFIX +" PurgePkgStatusChanged progress = %d , status = %s ,current_details = %s",\
            progress,status,current_details)

    #安装deb包完成的信号
    @dbus.service.signal(UPDATER_DBUS_INTERFACE,signature='bss')
    def InstalldebFinished(self, success,error_string='',error_desc=''):
        logging.info(COLORLOG_PREFIX + "Emitting"+ COLORLOG_SUFFIX + " InstalldebFinished success = %r , error_string = %s , error_desc = %s ",\
                    success,error_string,error_desc)
        
    #安装进度信息 0~100 进度信息 101为非预期的信号
    @dbus.service.signal(UPDATER_DBUS_INTERFACE,signature='iss')
    def InstalldebStatusChanged(self,progress,status,current_details):
        logging.info(COLORLOG_PREFIX + "Emitting" + COLORLOG_SUFFIX +" InstalldebStatusChanged progress = %d , status = %s ,current_details = %s",\
            progress,status,current_details)

    # dbus 信号：用于发送本地安装snap包的结果
    @dbus.service.signal(UPDATER_DBUS_INTERFACE, signature='sa{sv}')
    def KumSnapSignal(self, type_msg, msg):
        logging.info(COLORLOG_PREFIX + "Emitting" + COLORLOG_SUFFIX +" KumSnapSignal type_msg = %s,msg = %s",type_msg,str(msg))  

    #重启和注销请求信号
    @dbus.service.signal(UPDATER_DBUS_INTERFACE,signature='s')
    def RebootLogoutRequired(self,required_status=''):
        logging.info(COLORLOG_PREFIX + "Emitting"+ COLORLOG_SUFFIX + " RebootLogoutRequired required_status = %s",required_status)\
        
    #限速修改信号
    @dbus.service.signal(UPDATER_DBUS_INTERFACE,signature='ss')
    def ButtonStatusChange(self, signal_types = '', value=''):
        logging.info(COLORLOG_PREFIX + "Emitting"+ COLORLOG_SUFFIX + " ButtonStatusChange signal_types = %s, value = %s.",signal_types, value)

    # dbus 信号：用于发送立即更新信号
    @dbus.service.signal(UPDATER_DBUS_INTERFACE)
    def UpgradeAllNow(self):
        logging.info(COLORLOG_PREFIX + "Emitting" + COLORLOG_SUFFIX +" UpgradeAllNow") 
    
    # dbus 信号：用于发送自动更新配置更改信号
    @dbus.service.signal(UPDATER_DBUS_INTERFACE)
    def ChangeUpgradePolicy(self):
        logging.info(COLORLOG_PREFIX + "Emitting" + COLORLOG_SUFFIX +" ChangeUpgradePolicy") 

    #下载安装前的状态检查信号
    @dbus.service.signal(UPDATER_DBUS_INTERFACE,signature='bs')
    def InstallDetectStatus(self, success,error_code=''):
        logging.info(COLORLOG_PREFIX + "Emitting"+ COLORLOG_SUFFIX + " InstallDetectStatus success = %r , error_code = %s",\
                    success,error_code)

    WRITABLE_PROPERTIES = ()

    # pylint: disable-msg=C0103,C0322
    @dbus.service.signal(dbus_interface=dbus.PROPERTIES_IFACE,
                        signature="sa{sv}as")
    def PropertiesChanged(self, interface, changed_properties,
                        invalidated_properties):
        """The signal gets emitted if a property of the object's
        interfaces changed.

        :param property: The name of the interface.
        :param changed_properties: A dictrionary of changed
            property/value pairs
        :param invalidated_properties: An array of property names which
            changed but the value isn't conveyed.

        :type interface: s
        :type changed_properties: a{sv}
        :type invalidated_properties: as
        """
        logging.info("Emitting PropertiesChanged: %s, %s, %s" %
                (interface, changed_properties, invalidated_properties))

    # pylint: disable-msg=C0103,C0322
    @dbus.service.method(dbus.INTROSPECTABLE_IFACE,
                        in_signature='', out_signature='s',
                        path_keyword='object_path',
                        connection_keyword='connection')
    def Introspect(self, object_path, connection):
        # Inject the properties into the introspection xml data
        data = dbus.service.Object.Introspect(self, object_path, connection)
        xml = ElementTree.fromstring(data)
        for iface in xml.findall("interface"):
            props = self._get_properties(iface.attrib["name"])
            for key, value in props.items():
                attrib = {"name": key}
                if key in self.WRITABLE_PROPERTIES:
                    attrib["access"] = "readwrite"
                else:
                    attrib["access"] = "read"
                if isinstance(value, dbus.String):
                    attrib["type"] = "s"
                elif isinstance(value, dbus.UInt32):
                    attrib["type"] = "u"
                elif isinstance(value, dbus.Int32):
                    attrib["type"] = "i"
                elif isinstance(value, dbus.UInt64):
                    attrib["type"] = "t"
                elif isinstance(value, dbus.Int64):
                    attrib["type"] = "x"
                elif isinstance(value, dbus.Boolean):
                    attrib["type"] = "b"
                elif isinstance(value, dbus.Struct):
                    attrib["type"] = "(%s)" % value.signature
                elif isinstance(value, dbus.Dictionary):
                    attrib["type"] = "a{%s}" % value.signature
                elif isinstance(value, dbus.Array):
                    attrib["type"] = "a%s" % value.signature
                else:
                    raise Exception("Type %s of property %s isn't "
                                    "convertable" % (type(value), key))
                iface.append(ElementTree.Element("property", attrib))
        new_data = ElementTree.tostring(xml, encoding="UTF-8")
        return new_data

    # pylint: disable-msg=C0103,C0322
    @dbus.service.method(dbus.PROPERTIES_IFACE,
                        in_signature="ssv", out_signature="",
                        sender_keyword="sender")
    def Set(self, iface, name, value, sender):
        """Set a property.

        Only the user who intiaited the transaction is
        allowed to modify it.

        :param iface: The interface which provides the property.
        :param name: The name of the property which should be modified.
        :param value: The new value of the property.

        :type iface: s
        :type name: s
        :type value: v
        """
        logging.info("Set() was called: %s, %s" % (name, value))
        return self._set_property(iface, name, value, sender)

    # pylint: disable-msg=C0103,C0322
    @dbus.service.method(dbus.PROPERTIES_IFACE,
                        in_signature="s", out_signature="a{sv}")
    def GetAll(self, iface):
        """Get all available properties of the given interface."""
        logging.info("GetAll() was called: %s" % iface)
        return self._get_properties(iface)

    # pylint: disable-msg=C0103,C0322
    @dbus.service.method(dbus.PROPERTIES_IFACE,
                        in_signature="ss", out_signature="v")
    def Get(self, iface, property):
        """Return the value of the given property provided by the given
        interface.
        """
        logging.info("Get() was called: %s, %s" % (iface, property))
        return self._get_properties(iface)[property]

    def _set_property(self, iface, name, value, sender):
        """Helper to set a property on the properties D-Bus interface."""
        if iface == UPDATER_DBUS_INTERFACE:
            if name == "ShutdownInstall":
                self.parent.configs_uncover.setValue("InstallMode","shutdown_install",str(bool(value)))
            elif name == "P2pBootstrap":
                self.parent.apt_p2p_config.set_bootstrap(str(value))
            else:
                raise dbus.exceptions.DBusException("Unknown or read only "
                                                    "property: %s" % name)
        else:
            raise dbus.exceptions.DBusException("Unknown interface: %s" %
                                                iface)

    def _get_properties(self, iface):
        """Helper get the properties of a D-Bus interface."""
        if iface == UPDATER_DBUS_INTERFACE:
            return {
                "ShutdownInstall": dbus.Boolean(
                    self.parent.configs_uncover.getWithDefault("InstallMode", "shutdown_install", False)),

                "P2pBootstrap": dbus.String(self.parent.apt_p2p_config.get_bootstrap())
                    }
        else:
            return {}

class UpdateManagerDbusControllerUtils(dbus.service.Object):
    """ this is a helper to provide the UpdateManagerIFace """

    def __init__(self, parent, bus_name,
                object_path=UPDATER_DBUS_PATH_UTILS):
        try:
            dbus.service.Object.__init__(self, bus_name, object_path)
        except Exception as e:
            # pass
            logging.warning(e)
        self.parent = parent
        self.bus = dbus.SystemBus()

        self.transaction = None

    def _check_prohibit_user(self, sender_name):
        prohibit_list = ["dbus-send","gdbus"]
        if sender_name in prohibit_list:
            raise dbus.exceptions.DBusException("ERROR: You are not allowed to perform this action.")

    # # dbus接口: 后端大数据采集
    @dbus.service.method(UPDATER_DBUS_INTERFACE, in_signature='ss', out_signature='b',sender_keyword='sender')
    def DataBackendCollect(self, messageType, uploadMessage, sender=None):
        sender_name = get_proc_from_dbus_name(sender)
        logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' DataBackendCollect, messageType is %s, called by: %s .',messageType,sender_name)
        logging.debug("uploadMessage: %s.", uploadMessage)
        try:
            self.parent.collector.UpdateMsg(messageType, uploadMessage)
        except Exception as e:
            logging.error(str(e))
    
    # 检查更新
    @dbus.service.method(UPDATER_DBUS_INTERFACE, in_signature='as', out_signature='as', sender_keyword='sender')
    def CheckInstalledOrUpgrade(self, pkgs, sender=None):
        sender_name = get_proc_from_dbus_name(sender)
        try:
            pkglist = []
            cache = MyCache(None)
            self.parent.collector.cache = cache
            for i in pkgs:
                name = str(i).strip().replace("\n", "")
                if name in cache:
                    pkg = cache[name]
                    if pkg.is_installed == False or pkg.is_upgradable == True:
                        pkglist.append(pkg.name)
                        pkglist.append(str(pkg.candidate.size))
                        self.parent.collector.make_background_version(pkg)
                        self.parent.collector.background_upgradable.append(pkg.name)
        except:
            logging.error("CheckInstalledOrUpgrade: Failed to obtain package information: %s" % str(i), exc_info=True)
        # if len(pkglist) != 0:
        #     logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' CheckInstalledOrUpgrade, called by: %s, Upgrade list: %s .', sender_name, ", ".join(pkglist))
        return pkglist

    # 提供插入更新历史的接口
    @dbus.service.method(UPDATER_DBUS_INTERFACE, in_signature='a{sv}', out_signature='b', sender_keyword='sender')
    def InsertUpgradeHistory(self, arg = {}, sender=None):
        try:
            #    {"appname":GLib.Variant("s", "kylin-system-updater"), "version":GLib.Variant("s", "string value")}
            #    "description":GLib.Variant("s", "Update Manager for Kylin"), "date":GLib.Variant("s", "2022-07-27 15:23:51")
            #    "status":GLib.Variant("s", "failed"), "keyword":GLib.Variant("s", "1")
            #    "errorcode":GLib.Variant("s", "System upgrade is complete. "), "appname_cn":GLib.Variant("s", "音乐")
            sender_name = get_proc_from_dbus_name(sender)
            logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' Call InsertUpgradeHistory from %s.', sender_name)
            if not self.parent.sqlite3_server.insert_upgrade_history(arg, sender_name) :
                return False
        except Exception as e:
            logging.error("InsertUpgradeHistory Failed: %s" % str(e), exc_info=True)
            return False
        return True

    WRITABLE_PROPERTIES = ()

    # pylint: disable-msg=C0103,C0322
    @dbus.service.signal(dbus_interface=dbus.PROPERTIES_IFACE,
                        signature="sa{sv}as")
    def PropertiesChanged(self, interface, changed_properties,
                        invalidated_properties):
        """The signal gets emitted if a property of the object's
        interfaces changed.

        :param property: The name of the interface.
        :param changed_properties: A dictrionary of changed
            property/value pairs
        :param invalidated_properties: An array of property names which
            changed but the value isn't conveyed.

        :type interface: s
        :type changed_properties: a{sv}
        :type invalidated_properties: as
        """
        logging.info("Emitting PropertiesChanged: %s, %s, %s" %
                (interface, changed_properties, invalidated_properties))

    # pylint: disable-msg=C0103,C0322
    @dbus.service.method(dbus.INTROSPECTABLE_IFACE,
                        in_signature='', out_signature='s',
                        path_keyword='object_path',
                        connection_keyword='connection')
    def Introspect(self, object_path, connection):
        # Inject the properties into the introspection xml data
        data = dbus.service.Object.Introspect(self, object_path, connection)
        xml = ElementTree.fromstring(data)
        for iface in xml.findall("interface"):
            props = self._get_properties(iface.attrib["name"])
            for key, value in props.items():
                attrib = {"name": key}
                if key in self.WRITABLE_PROPERTIES:
                    attrib["access"] = "readwrite"
                else:
                    attrib["access"] = "read"
                if isinstance(value, dbus.String):
                    attrib["type"] = "s"
                elif isinstance(value, dbus.UInt32):
                    attrib["type"] = "u"
                elif isinstance(value, dbus.Int32):
                    attrib["type"] = "i"
                elif isinstance(value, dbus.UInt64):
                    attrib["type"] = "t"
                elif isinstance(value, dbus.Int64):
                    attrib["type"] = "x"
                elif isinstance(value, dbus.Boolean):
                    attrib["type"] = "b"
                elif isinstance(value, dbus.Struct):
                    attrib["type"] = "(%s)" % value.signature
                elif isinstance(value, dbus.Dictionary):
                    attrib["type"] = "a{%s}" % value.signature
                elif isinstance(value, dbus.Array):
                    attrib["type"] = "a%s" % value.signature
                else:
                    raise Exception("Type %s of property %s isn't "
                                    "convertable" % (type(value), key))
                iface.append(ElementTree.Element("property", attrib))
        new_data = ElementTree.tostring(xml, encoding="UTF-8")
        return new_data

    # pylint: disable-msg=C0103,C0322
    @dbus.service.method(dbus.PROPERTIES_IFACE,
                        in_signature="ssv", out_signature="",
                        sender_keyword="sender")
    def Set(self, iface, name, value, sender):
        """Set a property.

        Only the user who intiaited the transaction is
        allowed to modify it.

        :param iface: The interface which provides the property.
        :param name: The name of the property which should be modified.
        :param value: The new value of the property.

        :type iface: s
        :type name: s
        :type value: v
        """
        logging.info("Set() was called: %s, %s" % (name, value))
        return self._set_property(iface, name, value, sender)

    # pylint: disable-msg=C0103,C0322
    @dbus.service.method(dbus.PROPERTIES_IFACE,
                        in_signature="s", out_signature="a{sv}")
    def GetAll(self, iface):
        """Get all available properties of the given interface."""
        logging.info("GetAll() was called: %s" % iface)
        return self._get_properties(iface)

    # pylint: disable-msg=C0103,C0322
    @dbus.service.method(dbus.PROPERTIES_IFACE,
                        in_signature="ss", out_signature="v")
    def Get(self, iface, property):
        """Return the value of the given property provided by the given
        interface.
        """
        logging.info("Get() was called: %s, %s" % (iface, property))
        return self._get_properties(iface)[property]

    def _set_property(self, iface, name, value, sender):
        """Helper to set a property on the properties D-Bus interface."""
        if iface == UPDATER_DBUS_INTERFACE:
            if name == "UploadUpgradeLog":
                self.parent.configs_uncover.setValue("SystemStatus","upload_upgrade_log",str(bool(value)))
            if name == "UploadInstallerLog":
                self.parent.configs_uncover.setValue("SystemStatus","upload_installer_log",str(bool(value)))
            else:
                raise dbus.exceptions.DBusException("Unknown or read only "
                                                    "property: %s" % name)
        else:
            raise dbus.exceptions.DBusException("Unknown interface: %s" %
                                                iface)

    def _get_properties(self, iface):
        """Helper get the properties of a D-Bus interface."""
        if iface == UPDATER_DBUS_INTERFACE:
            return {
                "UploadUpgradeLog": dbus.Boolean(
                    self.parent.configs_uncover.getWithDefault("SystemStatus", "upload_upgrade_log", True)),
                "UploadInstallerLog": dbus.Boolean(
                    self.parent.configs_uncover.getWithDefault("SystemStatus", "upload_installer_log", False))
                    }
        else:
            return {}
