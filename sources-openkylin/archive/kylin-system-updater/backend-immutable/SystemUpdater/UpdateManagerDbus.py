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

#!/usr/bin/python3
import os
import dbus
import psutil
import dbus.service
import logging
from gettext import gettext as _
from .Core.loop import mainloop
from SystemUpdater.Core.errors import *
from SystemUpdater.Core.enums import *
from xml.etree import ElementTree
import locale
from SystemUpdater.UpdatesAvailable import Transaction
from gettext import ngettext
from math import ceil
import subprocess 
from SystemUpdater.configs import settings
from SystemUpdater.modules.update_history import get_last_deploy_result
from SystemUpdater.packageManager.pmWorker import *
from SystemUpdater.Core.DbusControl import DbusControl

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

ACTION_DEFUALT_STATUS = -1
ACTION_DOWNLOADONLY = 1
ACTION_DEPLOY = 2

def humanize_size(bytes):
    """
    Convert a given size in bytes to a nicer better readable unit
    """

    if bytes < 1000 * 1000:
        # to have 0 for 0 bytes, 1 for 0-1000 bytes and for 1 and above
        # round up
        size_in_kb = int(ceil(bytes / float(1000)))
        # TRANSLATORS: download size of small updates, e.g. "250 kB"
        return ngettext("%(size).0f kB", "%(size).0f kB", size_in_kb) % {
            "size": size_in_kb}
    else:
        # TRANSLATORS: download size of updates, e.g. "2.3 MB"
        return locale.format_string(_("%.1f MB"), bytes / 1000.0 / 1000.0)

#dbus 建立
class UpdateManagerDbusController(dbus.service.Object):
    """ this is a helper to provide the UpdateManagerIFace """

    ACTION_DEFUALT_STATUS = -1
    ACTION_UPDATE = 0
    ACTION_DOWNLOADONLY = 1
    ACTION_DEPLOY = 2
    ACTION_ROLLBACK = 3

    RETURN_SUCCESS_CODE = 0
    RETURN_SUCCESS_DESC = ""

    RETURN_UNKNOWN_CODE = -1
    RETURN_UNKNOWN_DESC = ""

    RETURN_BUSY_STATE = 1
    RETURN_BUSY_DESC = "In the process of updating or Upgrading..."

    def __init__(self, parent,bus,bus_name,
                object_path=UPDATER_DBUS_PATH):
        dbus.service.Object.__init__(self, bus_name, object_path)
        self.parent = parent
        self.bus = bus
        self.database = self.parent.sqlite3_server
        self.now_working = self.ACTION_DEFUALT_STATUS
        self.dbus_control = DbusControl(self.bus)

    @dbus.service.method(UPDATER_DBUS_INTERFACE,
                        in_signature="", out_signature="",
                        sender_keyword="caller_name")
    def Quit(self, caller_name=""):
        """Request a shutdown of the daemon."""
        logging.info("Quitting was requested")
        logging.debug("Quitting main loop...")
        mainloop.quit()
        logging.debug("Exit")

    #  =============================== 检查更新 =============================== */

    @dbus.service.method(UPDATER_DBUS_INTERFACE,out_signature='is',sender_keyword='sender')
    def UpdateDetect(self,sender=None):
        sender_name = self.dbus_control.check_sender(sender)
        try:
            #处于更新和升级中的话 不进行更新
            if self.now_working != ACTION_DEFUALT_STATUS:
                logging.warning('UpdateDetect In the process of Updating or Upgrading...')
                return self.RETURN_BUSY_STATE,self.RETURN_BUSY_DESC
            else:
                logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' UpdateDetect sender:%s...',sender_name)
                self.parent.run_transaction(Transaction(role=ROLE_UPDATE_CACHE, sender=sender_name))
                return self.RETURN_SUCCESS_CODE,self.RETURN_SUCCESS_DESC
        except Exception as e:
            logging.error(str(e))
            return self.RETURN_UNKNOWN_CODE,str(e)

    #更新 信号
    def UpdateDetectFinishedNext(self,success,upgrade_group,error_code=''):
        self.UpdateDetectFinished(success,upgrade_group,get_error_num_from_enum(error_code),get_error_description_from_enum(error_code))

    @dbus.service.signal(UPDATER_DBUS_INTERFACE,signature='basss')
    def UpdateDetectFinished(self, success, upgrade_group,error_code='',error_desc=''):
        self.now_working = self.ACTION_DEFUALT_STATUS
        logging.info(COLORLOG_PREFIX + "Emitting"+ COLORLOG_SUFFIX + " UpdateDetectFinished success = %r , upgrade_group = %a, error_code = %s , error_desc = %s ",\
                    success,upgrade_group,error_code,error_desc)

    #  =============================== 下载 =============================== */

    @dbus.service.method(UPDATER_DBUS_INTERFACE,out_signature='is',sender_keyword='sender')
    def UpdateDownloadAll(self,sender=None):
        sender_name = self.dbus_control.check_sender(sender)
        try:
            logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' UpdateDownloadAll sender:%s...',sender_name)
            trans = Transaction(**settings["current_update"])
            trans.role = ROLE_DOWNLOAD
            trans.sender = sender_name
            self.parent.run_transaction(trans)
            return self.RETURN_SUCCESS_CODE,self.RETURN_SUCCESS_DESC
        except Exception as e:
            logging.error(str(e))
            return self.RETURN_UNKNOWN_CODE,str(e)
        
    # 下载信号
    def UpdateDownloadFinishedNext(self,success,upgrade_group,error_code=''):
        self.UpdateDownloadFinished(success,upgrade_group,get_error_num_from_enum(error_code),get_error_description_from_enum(error_code))

    @dbus.service.signal(UPDATER_DBUS_INTERFACE,signature='basss')
    def UpdateDownloadFinished(self, success, upgrade_group,error_code='',error_desc=''):
        self.now_working = self.ACTION_DEFUALT_STATUS
        logging.info(COLORLOG_PREFIX + "Emitting" + COLORLOG_SUFFIX +" UpdateDownloadFinished success = %r , upgrade_group = %a, error_code = %s , error_desc = %s ",\
                    success,upgrade_group, error_code,error_desc)

    #发送下载包信息 fix bug 字节大小改成u 无符号32位
    @dbus.service.signal(UPDATER_DBUS_INTERFACE, signature='iitti')
    def UpdateDownloadInfo(self,current_items, total_items, currenty_bytes, total_bytes, current_cps):
        logging.info(COLORLOG_PREFIX + "Emitting" + COLORLOG_SUFFIX +" UpdateDownloadInfo current_items = %d, total_items = %d, currenty_bytes = %s, total_bytes = %s, current_cps = %s/s",
                    current_items, total_items, \
                    humanize_size(currenty_bytes), humanize_size(total_bytes),\
                    humanize_size(current_cps))

    @dbus.service.method(UPDATER_DBUS_INTERFACE,out_signature='is',sender_keyword='sender')
    def CancelDownload(self,sender=None):
        logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' CancelDownload ')
        
        self.parent.start_cancel()
        return self.RETURN_SUCCESS_CODE,self.RETURN_SUCCESS_DESC

    #  =============================== 部署 =============================== */

    # V10版本旧接口名称，内容和DeployLatestUpdate一样
    @dbus.service.method(UPDATER_DBUS_INTERFACE,in_signature='s',out_signature='is',sender_keyword='sender')
    def TriggerInstallOnShutdown(self,mode,sender=None):
        sender_name = self.dbus_control.check_sender(sender)
        try:
            #处于更新和升级中的话 不进行更新
            if self.now_working != ACTION_DEFUALT_STATUS:
                logging.warning('DeployLatestUpdate In the process of Updating or Upgrading...')
                return self.RETURN_BUSY_STATE,self.RETURN_BUSY_DESC
            else:
                logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' DeployLatestUpdate sender:%s...',sender_name)
                trans = Transaction(**settings["current_update"])

                if str(mode) == "shutdown":
                    trans.reboot_required = False
                    
                trans.role = ROLE_DEPLOY
                trans.sender = sender_name
                self.parent.run_transaction(trans)
                return self.RETURN_SUCCESS_CODE,self.RETURN_SUCCESS_DESC
        except Exception as e:
            logging.error(str(e))
            return self.RETURN_UNKNOWN_CODE,str(e)

    @dbus.service.method(UPDATER_DBUS_INTERFACE,in_signature='s',out_signature='is',sender_keyword='sender')
    def DeployLatestUpdate(self,mode,sender=None):
        sender_name = self.dbus_control.check_sender(sender)
        try:
            #处于更新和升级中的话 不进行更新
            if self.now_working != ACTION_DEFUALT_STATUS:
                logging.warning('DeployLatestUpdate In the process of Updating or Upgrading...')
                return self.RETURN_BUSY_STATE,self.RETURN_BUSY_DESC
            else:
                logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' DeployLatestUpdate sender:%s...',sender_name)
                trans = Transaction(**settings["current_update"])
                trans.role = ROLE_DEPLOY
                trans.sender = sender_name
                self.parent.run_transaction(trans)
                return self.RETURN_SUCCESS_CODE,self.RETURN_SUCCESS_DESC
        except Exception as e:
            logging.error(str(e))
            return self.RETURN_UNKNOWN_CODE,str(e)

    def DeployUpdatFinishedNext(self,success,upgrade_group,error_code=''):
        self.database.insert_info(success,error_code)
        self.DeployUpdatFinished(success,upgrade_group,get_error_num_from_enum(error_code),get_error_description_from_enum(error_code))

    @dbus.service.signal(UPDATER_DBUS_INTERFACE,signature='basss')
    def DeployUpdatFinished(self, success, upgrade_group,error_code='',error_desc=''):
        self.now_working = self.ACTION_DEFUALT_STATUS
        logging.info(COLORLOG_PREFIX + "Emitting" + COLORLOG_SUFFIX +" DeployUpdatFinished success = %r , upgrade_group = %a, error_code = %s , error_desc = %s ",\
                    success,upgrade_group, error_code,error_desc)

    #升级的进度信息 0~100 进度信息 101为非预期的信号
    @dbus.service.signal(UPDATER_DBUS_INTERFACE,signature='asiss')
    def UpdateDloadAndInstStaChanged(self,groups_list,progress,status,current_details):
        logging.info(COLORLOG_PREFIX + "Emitting" + COLORLOG_SUFFIX +" UpdateDloadAndInstStaChanged upgrade groups_list = %s progress = %d , status = %s ,current_details = %s\033[0m",\
            groups_list,progress,status,current_details)

    #  =============================== 回滚 =============================== */

    @dbus.service.method(UPDATER_DBUS_INTERFACE,out_signature='is',sender_keyword='sender')
    def RollbackSysVersion(self,mode,version,sender=None):
        sender_name = self.dbus_control.check_sender(sender)
        try:
            #处于更新和升级中的话 不进行更新
            if self.now_working != ACTION_DEFUALT_STATUS:
                logging.warning('RollbackSysVersion In the process of Updating or Upgrading...')
                return self.RETURN_BUSY_STATE,self.RETURN_BUSY_DESC
            else:
                logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' RollbackSysVersion sender:%s...',sender_name)
                self.parent.start_rollback()
                self.now_working = self.ACTION_ROLLBACK
                return self.RETURN_SUCCESS_CODE,self.RETURN_SUCCESS_DESC
        except Exception as e:
            logging.error(str(e))
            return self.RETURN_UNKNOWN_CODE,str(e)

    def RollbackSysFinishedNext(self, success, version_name,error_code):
        self.RollbackSysFinished(success,version_name,error_code,get_error_description_from_enum(error_code))

    @dbus.service.signal(UPDATER_DBUS_INTERFACE,signature='bsss')
    def RollbackSysFinished(self, success, version_name,error_code='',error_desc=''):
        self.now_working = self.ACTION_DEFUALT_STATUS
        logging.info(COLORLOG_PREFIX + "Emitting" + COLORLOG_SUFFIX +" RollbackSysFinished success = %r , upgrade_group = %s, error_code = %s , error_desc = %s ",\
                    success,version_name, error_code,error_desc)

    #  =============================== 其他 =============================== */
    #重启和注销请求信号
    @dbus.service.signal(UPDATER_DBUS_INTERFACE,signature='s')
    def RebootLogoutRequired(self,required_status=''):
        logging.info(COLORLOG_PREFIX + "Emitting"+ COLORLOG_SUFFIX + " RebootLogoutRequired required_status = %s",required_status)\
    
    #向数据库display表中插入数据
    @dbus.service.method(UPDATER_DBUS_INTERFACE, in_signature='ss', out_signature='is', sender_keyword='sender')
    def InsertInstallState(self, item, value, sender=None):
        try:
            sender_name = self.dbus_control.check_sender(sender)
            logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' InsertInstallState, options:%s，value:%s,  InsertInstallState sender: %s .' % (item, value, sender_name))
            self.database.insert_into_display(item, value)
            logging.info("Database insert to display successfully，options:%s，value:%s" % (item, value))
            return self.RETURN_SUCCESS_CODE,self.RETURN_SUCCESS_DESC
        except Exception as e:
            logging.error("Database insert  to display failed，options:%s，value:%s" % (item, value))
            return self.RETURN_UNKNOWN_CODE,str(e)

    #获取后端现在的状态
    @dbus.service.method(UPDATER_DBUS_INTERFACE,in_signature='s',out_signature='i',sender_keyword='sender')
    def GetBackendStatus(self,user_lang,sender=None):
        sender_name = self.dbus_control.check_sender(sender)
        try:
            logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' GetBackendStatus and user_lang = %s sender_name:%s status:%d...',str(user_lang),sender_name,self.now_working)
            return self.now_working
        except Exception as e:
            logging.error(str(e))
            return self.now_working

    #更新完成后的弹窗
    @dbus.service.method(UPDATER_DBUS_INTERFACE,out_signature='s',sender_keyword='sender')
    def CheckPopupOnPoweroffInstalled(self,sender=None):
        sender_name = self.dbus_control.check_sender(sender)
        try:
            status = get_last_deploy_result()
            logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' ChcekPopupOnPoweroffInstalled sender:%s  status:%s ...',sender_name,status)
            return status
        except Exception as e:
            logging.error(str(e))
            return ""

    # 获取系统版本号
    @dbus.service.method(UPDATER_DBUS_INTERFACE, out_signature='ss',sender_keyword='sender')
    def GetSystemUpdateVersion(self,sender=None):
        sender_name = self.dbus_control.check_sender(sender)
        try:
            os_version = self.parent.sqlite3_server.ucconfigs.getWithDefault("SYSTEM","os_version",'',True)
            update_version = self.parent.sqlite3_server.ucconfigs.getWithDefault("SYSTEM","update_version",'',True)
        except Exception as e:
            logging.error(str(e))
            return str(e),''
        # logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+ \
        #                     ' GetSystemUpdateVersion sender:%s os_version:%s release_id:%s', \
        #                                             sender_name,os_version, update_version)
        return os_version,update_version

    #  =============================== 包安装卸载 =============================== */
    @dbus.service.method(UPDATER_DBUS_INTERFACE, in_signature='ssbbs', out_signature='is', sender_keyword='sender')
    def InstallDebFile(self,source = "unKnown", path = "", _check_local_dep = False, _auto_satisfy = False, user_lang = '', sender=None):
        try:
            sender_name = self.dbus_control.check_sender(sender)

            if self.now_working != self.ACTION_DEFUALT_STATUS:
                logging.warning('Updater In the process of Upgrading, ignore \'%s\' InstallDebFile(%s) request ...', \
                                sender_name, path)
                return self.RETURN_BUSY_STATE,self.RETURN_BUSY_DESC

            if str(user_lang) == '':
                logging.info("The incoming language is null...")

            check_local_dep     = bool(_check_local_dep)
            auto_satisfy        = bool(_auto_satisfy)
            deb_path            = str(path)

            logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' InstallDebFile and check_local_dep:%r, auto_satisfy:%r, current_lang:%s , InstallDebFile sender: %s .',\
                                        check_local_dep, auto_satisfy, user_lang, sender_name)
            logging.info("Will install: %s.", path)

            if not hasattr(self.parent, 'pmworker') or self.parent.pmworker is None:
                self.parent.pmworker = PmWorker(self.parent)

            self.parent.pmworker.start_deb_install(deb_path, _check_local_dep, _auto_satisfy, source, sender)
            return self.RETURN_SUCCESS_CODE,self.RETURN_SUCCESS_DESC
        except Exception as e:
            logging.error(str(e))
            return self.RETURN_UNKNOWN_CODE,str(e)

    @dbus.service.method(UPDATER_DBUS_INTERFACE, in_signature='as', out_signature='is', sender_keyword='sender')
    def InstallPackages(self, pkg_list = [], sender=None):
        sender_name = self.dbus_control.check_sender(sender)
        try:
            if self.now_working != self.ACTION_DEFUALT_STATUS:
                logging.warning('Updater In the process of Upgrading, ignore \'%s\' InstallPackages request ...', \
                                sender_name)
                return self.RETURN_BUSY_STATE,self.RETURN_BUSY_DESC

            logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' InstallPackages: %s .',\
                                                                        ",".join(pkg_list))

            if not hasattr(self.parent, 'pmworker') or self.parent.pmworker is None:
                self.parent.pmworker = PmWorker(self.parent)

            self.parent.pmworker.start_back_upgrade(pkg_list, sender)

            return self.RETURN_SUCCESS_CODE, self.RETURN_SUCCESS_DESC
        except Exception as e:
            logging.error(str(e))
            return self.RETURN_UNKNOWN_CODE, str(e)

    @dbus.service.method(UPDATER_DBUS_INTERFACE, in_signature='asss', out_signature='is', sender_keyword='sender')
    def PurgePackages(self, _purge_list, cur_user, user_lang = '', sender=None):
        sender_name = self.dbus_control.check_sender(sender)
        try:
            purge_list = [str(pkg) for pkg in _purge_list]

            if self.now_working != self.ACTION_DEFUALT_STATUS:
                logging.warning('Updater In the process of Upgrading, ignore \'%s\' PurgePackages(%s) request ...', \
                                sender_name, purge_list)
                return self.RETURN_BUSY_STATE,self.RETURN_BUSY_DESC

            logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' PurgePackages Sender:%s and purge list is:%s...',sender_name, purge_list)

            if not hasattr(self.parent, 'pmworker') or self.parent.pmworker is None:
                self.parent.pmworker = PmWorker(self.parent)

            self.parent.pmworker.start_purge_pkgs(purge_list, sender)
            return self.RETURN_SUCCESS_CODE,self.RETURN_SUCCESS_DESC
        except Exception as e:
            logging.error(str(e))
            return self.RETURN_UNKNOWN_CODE,str(e)

    @dbus.service.signal(UPDATER_DBUS_INTERFACE,signature='basss')
    def UpdateInstallFinished(self, success, upgrade_group, error_string='', error_desc=''):
        logging.info(COLORLOG_PREFIX + "Emitting" + COLORLOG_SUFFIX +" UpdateInstallFinished success = %r , upgrade_group = %a, error_string = %s , error_desc = %s ",\
                     success, upgrade_group, error_string, error_desc)

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
    
    #卸载完成的信号，包含当前正在卸载的包名
    @dbus.service.signal(UPDATER_DBUS_INTERFACE,signature='bsss')
    def PurgePackagesFinishedWithPkgname(self, success,error_string='',error_desc='',current_pkg=''):
        logging.info(COLORLOG_PREFIX + "Emitting"+ COLORLOG_SUFFIX + " PurgePackagesFinishedWithPkgname success = %r , error_string = %s , error_desc = %s , current purge package = %s" ,\
                    success,error_string,error_desc,current_pkg)
    
    #卸载完成的信号
    @dbus.service.signal(UPDATER_DBUS_INTERFACE,signature='bssss')
    def PurgePackagesFinishedWithErrCode(self, success,error_string='',error_desc='',current_pkg='',error_code=''):
        logging.info(COLORLOG_PREFIX + "Emitting"+ COLORLOG_SUFFIX + " PurgePackagesFinishedWithErrCode success = %r , error_string = %s , error_desc = %s , current purge package = %s , error_code = %s",\
                    success,error_string,error_desc,current_pkg,error_code)

    #卸载进度信息 0~100 进度信息 101为非预期的信号，包含当前正在卸载的包名
    @dbus.service.signal(UPDATER_DBUS_INTERFACE,signature='isss')
    def PurgePkgStatusChangedWithPkgname(self,progress,status,current_details,current_pkg=''):
        logging.info(COLORLOG_PREFIX + "Emitting" + COLORLOG_SUFFIX +" PurgePkgStatusChangedWithPkgname progress = %d , status = %s ,current_details = %s, current purge package = %s",\
            progress,status,current_details,current_pkg)

    #安装deb包完成的信号
    @dbus.service.signal(UPDATER_DBUS_INTERFACE,signature='bss')
    def InstalldebFinished(self, success,error_string='',error_desc=''):
        logging.info(COLORLOG_PREFIX + "Emitting"+ COLORLOG_SUFFIX + " InstalldebFinished success = %r , error_string = %s , error_desc = %s ",\
                    success,error_string,error_desc)

    #安装deb包完成的信号
    @dbus.service.signal(UPDATER_DBUS_INTERFACE,signature='bsss')
    def InstalldebFinishedWithErrCode(self, success,error_string='',error_desc='',error_code=''):
        logging.info(COLORLOG_PREFIX + "Emitting"+ COLORLOG_SUFFIX + " InstalldebFinishedWithErrCode success = %r , error_string = %s , error_desc = %s , error_code = %s",\
                    success,error_string,error_desc,error_code)
        
    #安装进度信息 0~100 进度信息 101为非预期的信号
    @dbus.service.signal(UPDATER_DBUS_INTERFACE,signature='iss')
    def InstalldebStatusChanged(self,progress,status,current_details):
        logging.info(COLORLOG_PREFIX + "Emitting" + COLORLOG_SUFFIX +" InstalldebStatusChanged progress = %d , status = %s ,current_details = %s",\
            progress,status,current_details)

    @dbus.service.signal(UPDATER_DBUS_INTERFACE,signature='is')
    def EnvBuildStatusChanged(self, progress, status):
        logging.info(COLORLOG_PREFIX + "Emitting" + COLORLOG_SUFFIX +" EnvBuildStatusChanged progress = %d , status = %s",\
            progress,status)

    @dbus.service.signal(UPDATER_DBUS_INTERFACE,signature='bs')
    def EnvBuildStatusFinished(self, progress, status):
        logging.info(COLORLOG_PREFIX + "Emitting" + COLORLOG_SUFFIX +" EnvBuildStatusFinished progress = %d , status = %s",\
            progress,status)

    WRITABLE_PROPERTIES = ()

    #检查是否需要安装或者重启安装的请求
    @dbus.service.method(UPDATER_DBUS_INTERFACE,out_signature='is',sender_keyword='sender')
    def CheckInstallRequired(self,sender=None):
        sender_name = self.dbus_control.check_sender(sender)
        try:
            need_install,install_time = self.parent.require_shutdown_install()
            logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' CheckInstallRequired need_install:%s install_time:%s sender_name: %s...',\
                                                need_install,install_time,sender_name)

            return need_install,str(install_time)
        except Exception as e:
            logging.error(str(e))
            return 0,"0"

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
            if not hasattr(props,'items'):
                continue
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
        '''
        if iface == UPDATER_DBUS_INTERFACE:
            if name == "ShutdownInstall":
            elif name == "P2pBootstrap":
                self.parent.apt_p2p_config.set_bootstrap(str(value))
            else:
                raise dbus.exceptions.DBusException("Unknown or read only "
                                                    "property: %s" % name)
        else:
            raise dbus.exceptions.DBusException("Unknown interface: %s" %
                                                iface)
        '''

    def _get_properties(self, iface):
        """Helper get the properties of a D-Bus interface."""
        if iface == UPDATER_DBUS_INTERFACE:
            return {
                        "history": dbus.Array(settings.get("history",[])),
                        # "system": dbus.Array([settings.get("system",{})]),
                    }
        else:
            return {}

    @dbus.service.method(UPDATER_DBUS_INTERFACE,in_signature='s',out_signature='b',sender_keyword='sender')
    def MountSquashfs(self,squashfs,sender=None):
        logging.info("mount squashfs:%s"%squashfs)
        ret = True
        if(os.path.exists(squashfs)):
            name=os.path.splitext(squashfs)
            logging.info("file extension name:%s"%name[1])
            if(name[1].lower()==".squashfs"):
                try:                    
                    mountpath = "/media/OfflineSource"
                    cmd="cat /proc/mounts|grep -q %s && umount %s"%(mountpath,mountpath)
                    ret1=subprocess.run(cmd,shell=True).returncode
                    logging.info("umount return code:%d"%ret1)
                    cmd1 = "mkdir -p %s && mount -o loop %s %s"%(mountpath,squashfs,mountpath)
                    ret2=subprocess.run(cmd1,shell=True).returncode                                                     
                    logging.info("mount ret:%d"%ret2)
                    if (ret2==0):
                        pass
                    else:
                        ret=False
                except Exception as e:
                    logging.error(e)
                    ret = False
        return ret