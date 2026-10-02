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
import dbus.service
import logging
import subprocess
from gettext import gettext as _
from .Core.loop import mainloop
from SystemUpdater.Core.DbusControl import DbusControl
from SystemUpdater.packageManager.pmUtils import *
from SystemUpdater.packageManager.pmWorker import *

UPDATER_DBUS_INTERFACE = 'com.kylin.UpgradeStrategies.interface'
UPDATER_DBUS_PATH = '/com/kylin/UpgradeStrategies'
UPDATER_DBUS_SERVICE = 'com.kylin.UpgradeStrategies'
RUN_UNATTENDED_UPGRADE = '/var/run/unattended-upgrades.pid'
SYSTEM_VERSION = '/etc/kylin-version/kylin-system-version.conf'

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

UU_UPGRADE_MODE_AUTOMATIC_DOWNLOAD = 0
UU_UPGRADE_MODE_MANUAL = 1
UU_UPGRADE_MODE_AUTOMATIC_INSTALL = 2
UU_UPGRADE_MODE_BEFORE_SHUTDOWN = 3


#dbus 建立
class UpgradeStrategiesDbusController(dbus.service.Object):
    """ this is a helper to provide the UpdateManagerIFace """

    ACTION_DEFUALT_STATUS   =-1
    ACTION_UPDATE           = 0
    ACTION_DOWNLOADONLY     = 1
    ACTION_DEPLOY           = 2
    ACTION_ROLLBACK         = 3

    RETURN_BUSY_STATE       = 1
    RETURN_BUSY_DESC        = "In the process of updating or Upgrading..."

    P2P_DEDAULT_PATH        = "/etc/default/apt-p2p"
    RETURN_SUCCESS_CODE     = 0
    RETURN_SUCCESS_DESC     = ""

    RETURN_UNKNOWN_CODE     =-1
    RETURN_UNKNOWN_DESC     = ""

    def __init__(self, parent, bus_name,
                object_path=UPDATER_DBUS_PATH):
        dbus.service.Object.__init__(self, bus_name, object_path)
        self.parent         = parent
        self.bus            = dbus.SystemBus()

        self.now_working    = self.ACTION_DEFUALT_STATUS

        self.prohibit_list  = []
        if not self.parent.options.debug:
            self.prohibit_list = ["dbus-send","gdbus"]
        
        self.dbus_control = DbusControl(self.bus)
    
    def _check_prohibit_user(self, sender_name):
        if sender_name in self.prohibit_list:
            raise dbus.exceptions.DBusException("ERROR: You are not allowed to perform this action.")

    def __check_change__(self, _config = None, _section = "", _option = "", _value = ""):
        if _config == None:
            return False
        if _value == _config.getWithDefault(_section,_option,_value," "):
            return True
        return False

    @dbus.service.method(UPDATER_DBUS_INTERFACE,
                        in_signature="", out_signature="",
                        sender_keyword="caller_name")
    def Quit(self, caller_name):
        """Request a shutdown of the daemon."""
        #如果在下载就请求 取消
        logging.info("UpgradeStrategies service quitting...")
        mainloop.quit()
        logging.debug("Exit")

    ## dbus接口: 开启或关闭预下载功能
    @dbus.service.method(UPDATER_DBUS_INTERFACE, in_signature='s', out_signature='is',sender_keyword='sender')
    def ChangingP2PStatus(self,_status,sender = None):
        status = str(_status)
        sender_name = self.dbus_control.check_sender(sender)
        logging.info(COLORMETHOR_PREFIX+'method'+COLORLOG_SUFFIX+' ChangingP2PStatus, _status = %s , sender name: %s',status,sender_name)
        
        if os.path.exists(self.P2P_DEDAULT_PATH):
            if status == "enable":
                with open(self.P2P_DEDAULT_PATH, 'w+') as configfile:
                    configfile.write("#enable=true\n")
                #第一次进行检查是否已经开启
                args = ["systemctl","is-enabled","apt-p2p.service"]
                p = subprocess.run(args, stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
                if p.returncode != 0:
                    logging.info("Apt-p2p service is not runing and will be enable...")
                    #第二次进行重启
                    args = ["systemctl","enable","apt-p2p.service"]
                    p = subprocess.run(args, stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
                    if p.returncode == 0:
                        logging.info("Service Enable Execute successfully")
                        #第三次进行重启
                        args = ["systemctl","restart","apt-p2p.service"]
                        p = subprocess.run(args, stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
                        if p.returncode == 0:
                            logging.info("Restart Execute successfully")
                            return self.RETURN_SUCCESS_CODE,self.RETURN_SUCCESS_DESC
                        else:
                            logging.error(str(p.stdout))
                            logging.error("Failed to execute reboot")
                            return self.RETURN_UNKNOWN_CODE,str(p.stdout)
                    else:
                        logging.error(str(p.stdout))
                        logging.error("Failed to execute enable")
                        return self.RETURN_UNKNOWN_CODE,str(p.stdout)
                else:
                    logging.info("Apt-p2p service has been enabled and not need to reabled...")
                    return self.RETURN_SUCCESS_CODE,self.RETURN_SUCCESS_DESC

            elif status == "disable":
                with open(self.P2P_DEDAULT_PATH, 'w+') as configfile:
                    configfile.write("enable=false\n")

                args = ["systemctl","is-enabled","apt-p2p.service"]
                p = subprocess.run(args, stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
                if p.returncode == 0:
                    logging.info("Apt-p2p service is runing and will be disable...")
                    args = ["systemctl","disable","apt-p2p.service"]
                    p = subprocess.run(args, stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
                    if p.returncode == 0:
                        logging.info("Service disable Execute successfully")

                        args = ["systemctl","stop","apt-p2p.service"]
                        p = subprocess.run(args, stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)

                        if p.returncode == 0:
                            logging.info("Stop Execute successfully")
                            return self.RETURN_SUCCESS_CODE,self.RETURN_SUCCESS_DESC
                        else:
                            logging.error(str(p.stdout))
                            logging.error("Failed to execute Stop")
                            return self.RETURN_UNKNOWN_CODE,str(p.stdout)
                    else:
                        logging.error(str(p.stdout))
                        logging.error("Failed to execute disable")
                        return self.RETURN_UNKNOWN_CODE,str(p.stdout)
                else:
                    logging.info("Apt-p2p service has been disabled and not need to redisabled...")
                    return self.RETURN_SUCCESS_CODE,self.RETURN_SUCCESS_DESC
            else:
                logging.warning("error: input value _status=%s",status)
        else:
            logging.warning("apt-p2p function is not install...")

    @dbus.service.method(UPDATER_DBUS_INTERFACE, in_signature='b', out_signature='b',sender_keyword='sender')
    def SetUpgradeStrategyState(self,state,sender=None):
        state=bool(state)
        sender_name = self.dbus_control.check_sender(sender)
        logging.info(COLORMETHOR_PREFIX+'method'+COLORLOG_SUFFIX+' SetUpdateStrategyState, upgrade strategy state is %r, sender name: %s .',state,sender_name)
        try:
            if state:
                if not self.__check_change__(self.parent.uuconfigs, "updateStrategiesManager", "strategiesState", "True"):
                    self.parent.uuconfigs.setValue("updateStrategiesManager", "strategiesState", "True", True)                    
                    self.StrategyChanged("strategiesState","True")            
            else:
                if not self.__check_change__(self.parent.uuconfigs, "updateStrategiesManager", "strategiesState", "False"):
                    self.parent.uuconfigs.setValue("updateStrategiesManager", "strategiesState", "False", True)
                    self.StrategyChanged("strategiesState","False")
        except Exception as e:
            logging.error(str(e))
            return False
        return True
    
        ## 设置更新周期
    @dbus.service.method(UPDATER_DBUS_INTERFACE, in_signature='s', out_signature='b',sender_keyword='sender')
    def SetUpgradeMode(self, mode, sender = None):
        _mode = str(mode)
        sender_name = self.dbus_control.check_sender(sender)
        logging.info(COLORMETHOR_PREFIX+'method'+COLORLOG_SUFFIX+' SetUpgradeMode, mode: %s , sender:%s .',_mode,sender_name)                        
        try:
            if not self.__check_change__(self.parent.uuconfigs, "updateStrategiesManager", "installType", _mode):
                self.parent.uuconfigs.setValue("updateStrategiesManager", "installType", _mode, True)
                self.StrategyChanged("installType",_mode)
        except Exception as e:
            logging.error(str(e))
            return False
        return True
    
    ## dbus接口: 开启或关闭预下载功能
    @dbus.service.method(UPDATER_DBUS_INTERFACE, in_signature='bs', out_signature='b',sender_keyword='sender')
    def SetPreDownloadState(self, _state, _time, sender = None):
        state = bool(_state)
        time  = str(_time)
        sender_name = self.dbus_control.check_sender(sender)
        logging.info(COLORMETHOR_PREFIX+'method'+COLORLOG_SUFFIX+' SetPreDownloadState, state is %r, time: %s, sender name: %s .',state,time,sender_name)
        try:
            if state:
                if not self.__check_change__(self.parent.uuconfigs, "autoUpgradePolicy", "preDownload", "on"):
                    self.parent.uuconfigs.setValue("autoUpgradePolicy", "preDownload", "on", True)
                    self.PropertyChanged("preDownload","on")
                if not self.__check_change__(self.parent.uuconfigs, "autoUpgradePolicy", "preDownloadTime", time):
                    self.parent.uuconfigs.setValue("autoUpgradePolicy", "preDownloadTime", time, True)
                    self.PropertyChanged("preDownloadTime",time)
            else:
                if not self.__check_change__(self.parent.uuconfigs, "autoUpgradePolicy", "preDownload", "off"):
                    self.parent.uuconfigs.setValue("autoUpgradePolicy", "preDownload", "off", True)
                    self.PropertyChanged("preDownload","off")
        except Exception as e:
            logging.error(str(e))
            return False
        return True

    ## 设置更新周期
    @dbus.service.method(UPDATER_DBUS_INTERFACE, in_signature='i', out_signature='b',sender_keyword='sender')
    def SetUpdateDays(self, days, sender = None):
        _days = int(days)
        sender_name = self.dbus_control.check_sender(sender)
        logging.info(COLORMETHOR_PREFIX+'method'+COLORLOG_SUFFIX+' SetUpdateDays, days: %d , sender:%s .'\
                        ,_days,sender_name)
        try:
            if not self.__check_change__(self.parent.uuconfigs, "autoUpgradePolicy", "updateDays", _days):
                self.parent.uuconfigs.setValue("autoUpgradePolicy", "updateDays", str(_days), True)
                self.PropertyChanged("updateDays",str(_days))
        except Exception as e:
            logging.error(str(e))
            return False
        return True
    
    # 设置自动更新时间
    @dbus.service.method(UPDATER_DBUS_INTERFACE, in_signature='s', out_signature='bs',sender_keyword='sender')
    def SetAutoUpgradeRandomRange(self,randomRange,sender=None):
        _randomRange = str(randomRange)
        sender_name = self.dbus_control.check_sender(sender)
        logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' SetAutoUpgradeRandomRange will be set value %s, sender: %s .',\
                            _randomRange,sender_name)
        try:
            if not self.__check_change__(self.parent.uuconfigs, "autoUpgradePolicy", "randomRange", _randomRange):
                self.parent.uuconfigs.setValue("autoUpgradePolicy", "randomRange", _randomRange, True)
                self.PropertyChanged("randomRange",_randomRange)
        except Exception as e:
            logging.error(str(e))
        return True,"success"

    ## 设置是否开启自动重启,以及设置自动重启的时间
    @dbus.service.method(UPDATER_DBUS_INTERFACE, in_signature='bs', out_signature='b',sender_keyword='sender')
    def SetAutomaticReboot(self, status, reboot_time, sender = None):
        _state = bool(status)
        _reboot_time  = str(reboot_time)
        sender_name = self.dbus_control.check_sender(sender)
        logging.info(COLORMETHOR_PREFIX+'method'+COLORLOG_SUFFIX+' SetAutomaticReboot, status is %r, reboot_time: %s, sender name: %s .'\
                        ,_state,_reboot_time,sender_name)
        try:
            if _state:
                if not self.__check_change__(self.parent.uuconfigs, "autoUpgradePolicy", "automaticReboot", "on"):
                    self.parent.uuconfigs.setValue("autoUpgradePolicy", "automaticReboot", "on", True)
                    self.PropertyChanged("automaticReboot","on")
                if not self.__check_change__(self.parent.uuconfigs, "autoUpgradePolicy", "automaticRebootTime", _reboot_time):
                    self.parent.uuconfigs.setValue("autoUpgradePolicy", "automaticRebootTime", _reboot_time, True)
                    self.PropertyChanged("automaticRebootTime",_reboot_time)
            else:
                if not self.__check_change__(self.parent.uuconfigs, "autoUpgradePolicy", "automaticReboot", "off"):
                    self.parent.uuconfigs.setValue("autoUpgradePolicy", "automaticReboot", "off", True)
                    self.PropertyChanged("automaticReboot","off")
        except Exception as e:
            logging.error(str(e))
            return False
        return True

    ## dbus接口: 开启关闭自动更新功能
    @dbus.service.method(UPDATER_DBUS_INTERFACE, in_signature='b', out_signature='b', sender_keyword='sender')
    def SetAutoUpgradeState(self, _state, sender):
        state = bool(_state)
        sender_name = self.dbus_control.check_sender(sender)
        logging.info(COLORMETHOR_PREFIX+'method'+COLORLOG_SUFFIX+' SetAutoUpgradeState, state is %r ...',state)
        try:
            if state:
                if not self.__check_change__(self.parent.uuconfigs, "autoUpgradePolicy", "autoUpgradeState", "on"):
                    self.parent.uuconfigs.setValue("autoUpgradePolicy", "autoUpgradeState", "on", True)
                    self.parent.sqlite3_server.insert_into_display("autoupdate_allow", "true")
                    self.PropertyChanged("autoUpgradeState","on")
                self.ButtonStatusChange("autoUpgradeStatus", "true")
            else :
                if not self.__check_change__(self.parent.uuconfigs, "autoUpgradePolicy", "autoUpgradeState", "off"):
                    self.parent.uuconfigs.setValue("autoUpgradePolicy", "autoUpgradeState", "off", True)
                    self.parent.sqlite3_server.insert_into_display("autoupdate_allow", "false")
                    self.PropertyChanged("autoUpgradeState","off")
                self.ButtonStatusChange("autoUpgradeStatus", "false")
        except Exception as e:
            logging.error(str(e))
            return False
        return True

    ## dbus接口: 设置自动更新策略
    @dbus.service.method(UPDATER_DBUS_INTERFACE, in_signature='is', out_signature='b', sender_keyword='sender')
    def SetAutoUpgradeMode(self, mode, time, sender):
        _time = str(time)
        sender_name = self.dbus_control.check_sender(sender)
        logging.info(COLORMETHOR_PREFIX+'method'+COLORLOG_SUFFIX+' SetAutoUpgradeMode, mode is %s, time is %s ...',mode,_time)
        try:
            if mode == UU_UPGRADE_MODE_AUTOMATIC_DOWNLOAD:
                if not self.__check_change__(self.parent.uuconfigs, "autoUpgradePolicy", "downloadMode", "timing"):
                    self.parent.uuconfigs.setValue("autoUpgradePolicy", "downloadMode", "timing", True)
                    self.PropertyChanged("downloadMode","timing")
                if not self.__check_change__(self.parent.uuconfigs, "autoUpgradePolicy", "downloadTime", str(_time)):
                    self.parent.uuconfigs.setValue("autoUpgradePolicy", "downloadTime", str(_time), True)
                    self.PropertyChanged("downloadTime",str(_time))
                self.ButtonStatusChange("autoUpgradeTime", str(_time))
            elif mode == UU_UPGRADE_MODE_AUTOMATIC_INSTALL:
                if not self.__check_change__(self.parent.uuconfigs, "autoUpgradePolicy", "installMode", "timing"):
                    self.parent.uuconfigs.setValue("autoUpgradePolicy", "installMode", "timing", True)
                    self.PropertyChanged("installMode","timing")
                if not self.__check_change__(self.parent.uuconfigs, "autoUpgradePolicy", "installTime", str(_time)):
                    self.parent.uuconfigs.setValue("autoUpgradePolicy", "installTime", str(_time), True)
                    self.PropertyChanged("installTime",str(_time))
            elif mode == UU_UPGRADE_MODE_BEFORE_SHUTDOWN:
                if not self.__check_change__(self.parent.uuconfigs, "autoUpgradePolicy", "installTime", "bshutdown"):
                    self.parent.uuconfigs.setValue("autoUpgradePolicy", "installMode", "bshutdown", True)
                    self.PropertyChanged("installMode","bshutdown")
            elif mode == UU_UPGRADE_MODE_MANUAL:
                if not self.__check_change__(self.parent.uuconfigs, "autoUpgradePolicy", "downloadMode", "manual"):
                    self.parent.uuconfigs.setValue("autoUpgradePolicy", "downloadMode", "manual", True)
                    self.PropertyChanged("downloadMode","manual")
        except Exception as e:
            logging.error(str(e))
            return False
        return True

    # # dbus接口：改变apt下载速度
    @dbus.service.method(UPDATER_DBUS_INTERFACE, in_signature='sb', out_signature='b',sender_keyword='sender')
    def SetDownloadspeedMax(self, speed, set,sender = None):
        sender_name = self.dbus_control.check_sender(sender)
        logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' SetDownloadspeedMax, speed:%s, set:%r, sender name: %s .'%(speed, set, sender_name))
        #来重启Aptdeamon
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
                    return True
                except Exception as e:
                    logging.error(e)
                    return False
        else:
            if os.path.exists("/etc/apt/apt.conf.d/80apt-download"):
                os.remove("/etc/apt/apt.conf.d/80apt-download")
                self.parent.sqlite3_server.insert_into_display("download_limit","false")
                self.ButtonStatusChange("speed", "0")
                return True
            else:
                self.parent.sqlite3_server.insert_into_display("download_limit","false")
                self.ButtonStatusChange("speed", "0")
                return True

    # # dbus接口：获取apt下载速度
    @dbus.service.method(UPDATER_DBUS_INTERFACE, out_signature='bs',sender_keyword='sender')
    def GetDownloadspeedLimitValue(self,sender = None):
        sender_name = self.dbus_control.check_sender(sender)
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
            
    # 是否允许关机前更新
    @dbus.service.method(UPDATER_DBUS_INTERFACE, in_signature='ss', out_signature='bs', sender_keyword='sender')
    def UnattendedUpgradeValue(self, operation, value="false", sender=None):
        sender_name = self.dbus_control.check_sender(sender)
        logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' UnattendedUpgradeValue sender:%s ', sender_name)
        if operation.lower() != "get" and operation.lower() != "set":
            return False, 'Please input [\"set\", \"value\"] to set. \nor [\"get\"] to get whether updates are allowed before shutdown.'
        if operation == "set":
            try:
                logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' UnattendedUpgradeValue is going to %s [allow_unattended_upgrades_shutdown] value to %s.'%(operation,value))
                self.parent.sqlite3_server.insert_into_display("allow_unattended_upgrades_shutdown", value.lower())
            except Exception as e:
                logging.error(str(e))
                return False,str(e)
        else:
            logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' UnattendedUpgradeValue is going to %s [allow_unattended_upgrades_shutdown] value.'%(operation))
            try:
                value = self.parent.sqlite3_server.select_from_display("allow_unattended_upgrades_shutdown")
                logging.info("[allow_unattended_upgrades_shutdown] value is %s."%(value))
            except Exception as e:
                logging.error(str(e))
                return False,str(e)
        return True,"success"

    # # 设置自动更新时间
    # @dbus.service.method(UPDATER_DBUS_INTERFACE, in_signature='s', out_signature='bs',sender_keyword='sender')
    # def SetAutoUpgradePeriod(self, period, sender = None):
    #     sender_name = get_proc_from_dbus_name(sender)
    #     logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' SetAutoUpgradePeriod will be set value %s, SetAutoUpgradePeriod sender: %s.'%(period, sender_name))
    #     try:
    #         self.parent.sqlite3_server.insert_into_display("update_period", period.lower())
    #     except Exception as e:
    #         logging.error(str(e))
    #     return True,"success"

    # 获取数据库值
    @dbus.service.method(UPDATER_DBUS_INTERFACE, in_signature='bss', out_signature='s', sender_keyword='sender')
    def GetSetDatabaseInfo(self, gs, table, field, sender):
        Text = 'NULL'
        sender_name = self.dbus_control.check_sender(sender)
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

    ## dbus接口: 发送立即更新的信号
    @dbus.service.method(UPDATER_DBUS_INTERFACE, out_signature='b', sender_keyword='sender')
    def AutoUpgradeAllNow(self, sender):
        sender_name = self.dbus_control.check_sender(sender)
        logging.info(COLORMETHOR_PREFIX+'method'+COLORLOG_SUFFIX+' AutoUpgradeAllNow ...')
        try:
            self.UpgradeAllNow()
        except Exception as e:
            logging.error(str(e))
            return False
        return True

    #设置数据库配置信息
    @dbus.service.method(UPDATER_DBUS_INTERFACE,in_signature='ss',out_signature='b',sender_keyword='sender')
    def DatabaseInfoSet(self,field_name,field_value,sender=None):
        Status = False
        sender_name = self.dbus_control.check_sender(sender)
        logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' SetDatabaseInfo,field_name:%s,field_value:%s,caller:%s .',\
                field_name,field_value,sender_name)
        Status = self.parent.sqlite3_server.insert_into_display(field_name,field_value)
        return bool(Status)

    #数据库获取配置信息
    @dbus.service.method(UPDATER_DBUS_INTERFACE,in_signature='s',out_signature='s',sender_keyword='sender')
    def DatabaseInfoGet(self,field_name,sender=None):
        field_value = ''
        sender_name = self.dbus_control.check_sender(sender)
        logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' GetDatabaseInfo field_name:%s caller:%s',field_name,sender_name)
        field_value = self.parent.sqlite3_server.select_from_display(str(field_name))
        logging.info("Get field_value:%s",field_value)
        return field_value

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
        try:
            sender_name = self.dbus_control.check_sender(sender)

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
        try:
            sender_name = self.dbus_control.check_sender(sender)
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

    @dbus.service.method(UPDATER_DBUS_INTERFACE,in_signature='ss',out_signature='i',sender_keyword='sender')
    def PackageInstallStatus(self, name, version, sender=None):
        try:
            sender_name = self.dbus_control.check_sender(sender)
            logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' PackageInstallStatus, name:%s, version:%s, sender:%s .',
                        name, version, sender_name)

            if not hasattr(self.parent, 'pmworker') or self.parent.pmworker is None:
                self.parent.pmworker = PmWorker(self.parent)

            return self.parent.pmworker.check_package_status(name, version, sender)
        except Exception as e:
            logging.error("PackageInstallStatus err:%s", e)
            return -100

    @dbus.service.method(UPDATER_DBUS_INTERFACE,out_signature='b',sender_keyword='sender')
    def GetAutocheckStatus(self,sender=None):
        sender_name = self.dbus_control.check_sender(sender)
        logging.info(COLORMETHOR_PREFIX+'Method'+COLORLOG_SUFFIX+' GetAutocheckStatus sender: %s .', sender_name)

        autocheckStatus = True

        try:
            autocheckStatus = self.parent.sqlite3_server.select_from_display("auto_check")
            logging.info("[auto_check] value is %s.", autocheckStatus)
            autocheckStatus = True if autocheckStatus.lower() == "true" else False
        except Exception as e:
            logging.error("GetAutocheckStatus: %s.", str(e))

        return autocheckStatus

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


    #限速修改信号
    @dbus.service.signal(UPDATER_DBUS_INTERFACE,signature='ss')
    def ButtonStatusChange(self, signal_types = '', value=''):
        logging.info(COLORLOG_PREFIX + "Emitting"+ COLORLOG_SUFFIX + " ButtonStatusChange signal_types = %s, value = %s.",signal_types, value)

    @dbus.service.signal(UPDATER_DBUS_INTERFACE,signature='basss')
    def UpdateInstallFinished(self, success, upgrade_group, error_string='', error_desc=''):
        logging.info(COLORLOG_PREFIX + "Emitting" + COLORLOG_SUFFIX +" UpdateInstallFinished success = %r , upgrade_group = %a, error_string = %s , error_desc = %s ",\
                     success, upgrade_group, error_string, error_desc)

    # signal：属性发生改变
    @dbus.service.signal(dbus_interface=UPDATER_DBUS_INTERFACE,
                         signature="ss")
    def PropertyChanged(self, property, value):
        logging.info(COLORLOG_PREFIX + "Emitting" + COLORLOG_SUFFIX +" PropertyChanged: ( %s, %s )" % (property, value))

    # signal：属性发生改变
    @dbus.service.signal(dbus_interface=UPDATER_DBUS_INTERFACE,
                         signature="ss")
    def StrategyChanged(self, property, value):
        logging.info(COLORLOG_PREFIX + "Emitting" + COLORLOG_SUFFIX +" StrategyChanged: ( %s, %s )" % (property, value))

    # WRITABLE_PROPERTIES = ()

    # # pylint: disable-msg=C0103,C0322
    # @dbus.service.signal(dbus_interface=dbus.PROPERTIES_IFACE,
    #                     signature="sa{sv}as")
    # def PropertiesChanged(self, interface, changed_properties,
    #                     invalidated_properties):
    #     """The signal gets emitted if a property of the object's
    #     interfaces changed.

    #     :param property: The name of the interface.
    #     :param changed_properties: A dictrionary of changed
    #         property/value pairs
    #     :param invalidated_properties: An array of property names which
    #         changed but the value isn't conveyed.

    #     :type interface: s
    #     :type changed_properties: a{sv}
    #     :type invalidated_properties: as
    #     """
    #     logging.info("Emitting PropertiesChanged: %s, %s, %s" %
    #             (interface, changed_properties, invalidated_properties))

    # # pylint: disable-msg=C0103,C0322
    # @dbus.service.method(dbus.INTROSPECTABLE_IFACE,
    #                     in_signature='', out_signature='s',
    #                     path_keyword='object_path',
    #                     connection_keyword='connection')
    # def Introspect(self, object_path, connection):
    #     # Inject the properties into the introspection xml data
    #     data = dbus.service.Object.Introspect(self, object_path, connection)
    #     xml = ElementTree.fromstring(data)
    #     for iface in xml.findall("interface"):
    #         props = self._get_properties(iface.attrib["name"])
    #         for key, value in props.items():
    #             attrib = {"name": key}
    #             if key in self.WRITABLE_PROPERTIES:
    #                 attrib["access"] = "readwrite"
    #             else:
    #                 attrib["access"] = "read"
    #             if isinstance(value, dbus.String):
    #                 attrib["type"] = "s"
    #             elif isinstance(value, dbus.UInt32):
    #                 attrib["type"] = "u"
    #             elif isinstance(value, dbus.Int32):
    #                 attrib["type"] = "i"
    #             elif isinstance(value, dbus.UInt64):
    #                 attrib["type"] = "t"
    #             elif isinstance(value, dbus.Int64):
    #                 attrib["type"] = "x"
    #             elif isinstance(value, dbus.Boolean):
    #                 attrib["type"] = "b"
    #             elif isinstance(value, dbus.Struct):
    #                 attrib["type"] = "(%s)" % value.signature
    #             elif isinstance(value, dbus.Dictionary):
    #                 attrib["type"] = "a{%s}" % value.signature
    #             elif isinstance(value, dbus.Array):
    #                 attrib["type"] = "a%s" % value.signature
    #             else:
    #                 raise Exception("Type %s of property %s isn't "
    #                                 "convertable" % (type(value), key))
    #             iface.append(ElementTree.Element("property", attrib))
    #     new_data = ElementTree.tostring(xml, encoding="UTF-8")
    #     return new_data

    # # pylint: disable-msg=C0103,C0322
    # @dbus.service.method(dbus.PROPERTIES_IFACE,
    #                     in_signature="ssv", out_signature="",
    #                     sender_keyword="sender")
    # def Set(self, iface, name, value, sender):
    #     """Set a property.

    #     Only the user who intiaited the transaction is
    #     allowed to modify it.

    #     :param iface: The interface which provides the property.
    #     :param name: The name of the property which should be modified.
    #     :param value: The new value of the property.

    #     :type iface: s
    #     :type name: s
    #     :type value: v
    #     """
    #     logging.info("Set() was called: %s, %s" % (name, value))
    #     return self._set_property(iface, name, value, sender)

    # # pylint: disable-msg=C0103,C0322
    # @dbus.service.method(dbus.PROPERTIES_IFACE,
    #                     in_signature="s", out_signature="a{sv}")
    # def GetAll(self, iface):
    #     """Get all available properties of the given interface."""
    #     logging.info("GetAll() was called: %s" % iface)
    #     return self._get_properties(iface)

    # # pylint: disable-msg=C0103,C0322
    # @dbus.service.method(dbus.PROPERTIES_IFACE,
    #                     in_signature="ss", out_signature="v")
    # def Get(self, iface, property):
    #     """Return the value of the given property provided by the given
    #     interface.
    #     """
    #     logging.info("Get() was called: %s, %s" % (iface, property))
    #     return self._get_properties(iface)[property]
