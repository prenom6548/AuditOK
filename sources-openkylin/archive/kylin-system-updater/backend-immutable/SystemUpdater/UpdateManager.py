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

# UpdateManager.py
# -*- Mode: Python; indent-tabs-mode: nil; tab-width: 4; coding: utf-8 -*-
import os
import json
import dbus,time,sys
import traceback
import logging
from .Core.errors import *
from .Core.enums import *
from gi.repository import GObject
from .Core.loop import mainloop
from gettext import gettext as _
from dbus.mainloop.glib import DBusGMainLoop
DBusGMainLoop(set_as_default=True)
from .Core.Database import Sqlite3Server
from SystemUpdater.constants import OUTPUT_JSON_PATH,SYSTEM_UPDATE_GROUPS,KYLIN_OS_NAME,UPDATER_PACKAGE_NAME
from SystemUpdater.modules.system import get_dist,system_shutdown
from .push_update import update_important,get_push_content
from SystemUpdater.Core.PlymouthManager import PlymouthManager
from SystemUpdater.configs import settings
from SystemUpdater.UpdatesAvailable import DeployState
from .backend.BackendOstreeNext import UpdateBackend,DownloadBackend,DeployBackend,RollbackBackend,LoadCacheBackend
from .UpdateManagerDbus import UpdateManagerDbusController,UPDATER_DBUS_INTERFACE,UPDATER_DBUS_PATH,UPDATER_DBUS_SERVICE
from SystemUpdater.Core.DataAcquisition import create_time_stamp

class UpdateManager(GObject.GObject):
    __gsignals__ = {
                    "transaction-stage": (GObject.SignalFlags.RUN_FIRST,
                                         None,
                                         (GObject.TYPE_PYOBJECT,)),
                    }
    def __init__(self,options):
        try:
            GObject.GObject.__init__(self)
            self.cancellable = None
            self.options = options
            self.sqlite3_server = Sqlite3Server(self)

            self._check_exception_status()
            
            self.bus = dbus.SystemBus()
            self.dbus_send = self._setup_dbus(self.bus)

            self.distro = get_dist()

            # 获取当前系统的状态信息并保存到配置文件中
            def _get_state_finished(parent,trans):
                trans.distro = self.distro
                settings["system"] = trans.asdict()
            cache_backend = LoadCacheBackend(self,self.options.sysroot,self.options.os)
            cache_backend.connect("transaction-done", _get_state_finished)
            cache_backend.run(DeployState())

            self._init_function()
        except Exception as e:
            logging.error(e)

    def run(self):
        """Start the daemon and listen for calls."""
        logging.info("Waiting for calls...")
        try:
            mainloop.run()
        except KeyboardInterrupt:
            self.dbus_send.Quit(None)

    def run_transaction(self, trans):
        """Run the worker"""
        try:
            self.cancellable = trans.cancellable
            # lock.wait_for_lock(trans)
            if trans.role == ROLE_UPDATE_CACHE:
                self.start_update(trans)
            elif trans.role == ROLE_DOWNLOAD:
                self.start_download(trans)
            elif trans.role == ROLE_DEPLOY:
                self.start_deploy(trans)
        except UpdateBaseError as excep:
            self._finished_signal(trans.role,excep.code)
        except Exception as e:
            logging.error(e)
            tbk = traceback.format_exc()
            logging.error(str(tbk))
            self._finished_signal(trans.role,ERROR_PROGRAM_EXCEPTION)
        finally:
            pass

    def _finished_signal(self,role,error_code):
        if role == ROLE_UPDATE_CACHE:
            self.dbus_send.UpdateDetectFinishedNext(False,[''],error_code)
        elif role == ROLE_DOWNLOAD:
            self.dbus_send.UpdateDownloadFinishedNext(False,[''],error_code)
        elif role == ROLE_DEPLOY:
            self.dbus_send.DeployUpdatFinishedNext(False,[''],error_code)   

    def _init_function(self):
        # 非麒麟系统不获取推送内容
        if KYLIN_OS_NAME != self.distro:
            settings["enable_push"] = False

    def start_cancel(self):
        self.cancellable.cancel()

    #进行更新的操作
    def start_update(self,trans):
        if settings.get("enable_push",True):
            def _update_important_success(push_list):
                trans.push_list = push_list
                self.start_update_cache(trans)

            update_important(self.bus,reply_handler=_update_important_success,error_handler=self._finished_signal)
            return

        trans.push_list = get_push_content()
        self.start_update_cache(trans)

    #进行更新的操作
    def start_update_cache(self,trans):
        def _download_info(parent,fetched,requested,bytes_transferred,download_all,bytes_sec):
            self.dbus_send.UpdateDownloadInfo(fetched,requested,bytes_transferred,download_all,bytes_sec)

        def _update_finished(parent,trans):
            if trans.success:
                if trans.from_revision != trans.new_revision:
                    # 检查是否为openkylin版本，openkylin版本只要commit不同就可以升级
                    logging.info("Status Info: distro:%s from_revision:%s to new_revision:%s deployed_refs:%s to available_refs:%s"\
                                                ,self.distro,trans.from_revision,trans.new_revision,trans.deployed_refs,trans.available_refs)
                    if KYLIN_OS_NAME == self.distro:
                        if trans.push_list or trans.available_refs != trans.deployed_refs:
                            self._make_meta_for_panel(trans.update_metadata,OUTPUT_JSON_PATH)
                            self.dbus_send.UpdateDetectFinishedNext(True,[SYSTEM_UPDATE_GROUPS],'')
                        else:
                            self.dbus_send.UpdateDetectFinishedNext(trans.success,[],'')
                    else:
                        logging.info("Update available refs:%s old-revision:%s to new-revision:%s ",trans.available_refs,trans.from_revision,trans.new_revision)
                        self._make_meta_for_panel(trans.update_metadata,OUTPUT_JSON_PATH)
                        self.dbus_send.UpdateDetectFinishedNext(True,[SYSTEM_UPDATE_GROUPS],'')
                else:
                    logging.info("System is up-to-date from_revision:%s == new_revision:%s",trans.from_revision,trans.new_revision)
                    self.dbus_send.UpdateDetectFinishedNext(trans.success,[],'')
            else:
                if trans.error == PRIORITY_UPGRADE_SUCCCESSED:
                    self.dbus_send.UpdateDetectFinishedNext(trans.success,[UPDATER_PACKAGE_NAME],trans.error)
                else:
                    self.dbus_send.UpdateDetectFinishedNext(trans.success,[""],trans.error)
            
            # 配置写入到配置文件内
            settings["current_update"] = trans.asdict()
            self.sqlite3_server.insert_into_display("check_time", create_time_stamp()[0:-4])
            self.emit("transaction-stage",trans)

            # 优先升级完成后，重启更新的服务
            if trans.error == PRIORITY_UPGRADE_SUCCCESSED:
                self.dbus_send.Quit()

        update_backend = UpdateBackend(self,self.options.sysroot,self.options.os)
        update_backend.connect("transaction-done", _update_finished)
        update_backend.connect("download-info", _download_info)
        update_backend.run(trans)

    def start_download(self,trans):
        def _download_info(parent,fetched,requested,bytes_transferred,download_all,bytes_sec):
            self.dbus_send.UpdateDownloadInfo(fetched,requested,bytes_transferred,download_all,bytes_sec)

        def _download_finished(parent,trans):
            self.dbus_send.UpdateDownloadFinishedNext(trans.success,[SYSTEM_UPDATE_GROUPS],trans.error)
            # 配置写入到配置文件内
            settings["current_update"] = trans.asdict()
            self.emit("transaction-stage",trans)

        download_backend = DownloadBackend(self,self.options.sysroot,self.options.os)
        download_backend.connect("transaction-done", _download_finished)
        download_backend.connect("download-info", _download_info)
        download_backend.run(trans)

    def start_deploy(self,trans):
        plymouth = PlymouthManager()
        def _deploy_progress(parent,progress):
            self.dbus_send.UpdateDloadAndInstStaChanged([SYSTEM_UPDATE_GROUPS],progress,"","")
            # 调试模式下 禁止开启Plymouth界面
            if not settings.get("prohibit_shutdown",False):
                plymouth.show()
                plymouth.progress_to(progress)
                plymouth.message_to(_("The system is installing updates. Please do not turn off your computer. "))

        def _deploy_finished(parent,trans):
            self.dbus_send.DeployUpdatFinishedNext(trans.success,[SYSTEM_UPDATE_GROUPS],trans.error)                
            trans.status = STATUS_FINISHED
            # 获取历史数据并进行叠加
            history_list = [trans.asdict()] + settings.get("history",[])
            settings["history"] = history_list
            settings["current_update"] = {}
            
            self.emit("transaction-stage",trans)
            if not settings.get("prohibit_shutdown",False):
                plymouth.show_countdown(trans.success,trans.reboot_required)
                plymouth.clean_all()
                system_shutdown(trans.reboot_required)
        
        # 部署时默认状态为 部署失败错误，当部署过程异常关机时保留状态，其他成功或失败刷新错误状态
        trans.status = STATUS_DEPLOYING
        settings["current_update"] = trans.asdict()

        deploy_backend = DeployBackend(self,self.options.sysroot,self.options.os)
        deploy_backend.connect("transaction-done", _deploy_finished)
        deploy_backend.connect("deploy-progress", _deploy_progress)
        deploy_backend.run(trans)

    def start_rollback(self):
        def _rollback_finished(parent,trans):
            self.dbus_send.RollbackSysFinishedNext(parent.exit,trans.available_refs,trans.error)

            if not settings.get("prohibit_shutdown",False):
                system_shutdown()

        rollback_backend = RollbackBackend(self,self.options.sysroot,self.options.os)
        rollback_backend.connect("transaction-done", _rollback_finished)
        rollback_backend.run(self.trans)

    def require_shutdown_install(self):
        current_update = settings.get("current_update",{})
        # 当前需要部署时，更新状态未下载完成
        if current_update and \
                current_update.get("role",ROLE_UNSET)  == ROLE_DOWNLOAD and current_update.get("success",False):
            return 1,0
        else:
            return 0,0

    def _make_meta_for_panel(self,data,output_path):
        groups_base_info = {}
        output_json = {}
        try: 
            if not os.path.exists(output_path):
                os.makedirs(output_path)
                logging.info('making the ConfigPath(%s) is complete...',output_path)

            output_config_name = output_path + SYSTEM_UPDATE_GROUPS + '.json'
            groups_base_info.update({"version":data.setdefault("version","")})
            groups_base_info.update({"update-type":data.setdefault("update-type",SYSTEM_UPDATE_GROUPS)})

            update_name = {"zh_CN":data.setdefault("update-name_zh_CN","系统更新"),"en_US":data.setdefault("update-name_en_US","System update")}
            groups_base_info.update({"update-name":update_name})

            description_name = {"zh_CN":data.setdefault("description",""),"en_US":data.setdefault("description","")}
            groups_base_info.update({"description":description_name})

            changlog = {"zh_CN":data.setdefault("changelog",""),"en_US":data.setdefault("changelog","")}
            groups_base_info.update({"changelog":changlog})
            
            groups_base_info.update({"total_download_size":int(data.setdefault("download-size",0))})
            groups_base_info.update({"total_install_size":int(data.setdefault("install-size",0))})
            groups_base_info.update({"icon":data.setdefault("icon","/usr/share/upgrade/icon/kylin-update-desktop-system.png")})

            output_json.update(groups_base_info)

            with open(output_config_name, 'w', encoding='utf-8') as f:
                json.dump(output_json, f, ensure_ascii=False, indent=4)                   
            logging.info("Generate Jsonfile(%s) to complete... ",output_config_name)
        except Exception as e:
            logging.error(e)

    def _check_exception_status(self):
        # 检查var配置目录是否存在 不存在时进行创建
        VAR_CONFG_DIR = '/var/lib/kylin-system-updater'
        if not os.path.exists(VAR_CONFG_DIR):
            os.makedirs(VAR_CONFG_DIR)
        
        current_update = settings.get("current_update",{})
        # 此种情况标识 当前正在部署过程中异常关机
        if current_update and current_update.get("status",STATUS_FINISHED) == STATUS_DEPLOYING:
            # 补充历史数据
            current_update["success"] = False
            current_update["error"] = ERROR_INSTALL_POWER_OUTAGE
            history_list = [current_update] + settings.get("history",[])

            settings["history"] = history_list
            settings["current_update"] = {}
            self.sqlite3_server.insert_info(False,get_error_description_from_enum(ERROR_INSTALL_POWER_OUTAGE))

    def _setup_dbus(self,bus):
        # check if there is another g-a-i already and if not setup one
        # listening on dbus
        try:
            bus_name = dbus.service.BusName(UPDATER_DBUS_SERVICE,
                                            bus,do_not_queue=True)
            logging.info("Initiate dbus success ...")
            return  UpdateManagerDbusController(self,bus,bus_name)
        except dbus.exceptions.NameExistsException:
            if not settings.get("replace",False):
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
                    return  UpdateManagerDbusController(self,bus,bus_name)
                except dbus.exceptions.NameExistsException:
                    the_other_guy = bus.get_object(UPDATER_DBUS_SERVICE,
                                                    UPDATER_DBUS_PATH)
                    the_other_guy.Quit(dbus_interface=UPDATER_DBUS_INTERFACE,
                                        timeout=300)
                    logging.warning("Dbus has not withdrawn and retry reboot times:%d...",retry_reboot_times)
                    time.sleep(1)