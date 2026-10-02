#!/usr/bin/env python

import aptdaemon.client as client
import aptdaemon.errors as errors
import aptdaemon.enums as enums
from aptdaemon.enums import (
                            EXIT_SUCCESS,
                            EXIT_FAILED,
                            EXIT_CANCELLED,
                            STATUS_FINISHED,
                            get_error_description_from_enum,
                            get_error_string_from_enum,
                            get_status_string_from_enum
                            )
from defer import inline_callbacks
from SystemUpdater.backend import InstallBackend
import logging
from gettext import gettext as _
from SystemUpdater.Core.utils import humanize_size
import dbus,time
from gi.repository import GLib
import traceback
from importlib import reload

# 超时检测 秒单位
UPDATER_IDLE_CHECK_INTERVAL = 5
#安装的超时检查20分钟 按照5秒检查一次 
INSTALL_IDLE_TIMEOUT = 4 * 60
#更新超时检查 5分钟
UPDATE_IDLE_TIMEOUT = 5 * 60

class InstallBackendAptdaemon(InstallBackend):
    """Makes use of aptdaemon to refresh the cache and to install updates."""

    def __init__(self, window_main, action,mode = InstallBackend.MODE_DEFAULT_STATUS):
        InstallBackend.__init__(self, window_main, action,mode)
        self.window_main = window_main
        #切换aptdaemon的语言 重新导入模块就可以进行切换
        if self.window_main.aptd_lang_switch == True:
            self.window_main.aptd_lang_switch = False
            reload(client)
            reload(errors)
            reload(enums)

        #客户端连接aptdeamon的dbus接口
        self.client = client.AptClient()
        self.trans_failed_msg = None

        self.steps_progress_begin = 0
        self.steps_progress_end = 0

        self.need_retry = False
        
        #是否在安装状态 判断依据进度>50
        self.on_installing = False
        if self.action == self.ACTION_INSTALL_SHUTDOWN:
            self.on_installing = True

        if self.action == self.ACTION_UPDATE:
            #更新的超时检查机制  超时时取消下载
            self.update_timestamp = 0
            GLib.timeout_add_seconds(UPDATER_IDLE_CHECK_INTERVAL,
                self._check_update_inactivity)
            
            #定时模拟发送进度 为了让进度更加线性
            self.simulation_progress = 0
            GLib.timeout_add_seconds(1,self._check_simulation_progress)
        
        elif self.action == self.ACTION_INSTALL or self.action == self.ACTION_INSTALL_SHUTDOWN:
            #安装的超时间检查 超时解除禁止关机
            self.install_timestamp = INSTALL_IDLE_TIMEOUT
            self.check_progress = 0
            GLib.timeout_add_seconds(UPDATER_IDLE_CHECK_INTERVAL,
                self._check_install_inactivity)
        
    def _check_simulation_progress(self):
        self.simulation_progress += 20

        if self.aptd_base.progress >= 90 or self.simulation_progress > 80:
            return False
        else:
            self._dist_status_changed(self.action,self.simulation_progress,self.aptd_base.status,self.aptd_base.details)
            return True

    def _check_install_inactivity(self):
        """Shutdown the daemon if it has been inactive for time specified
        in INSTALL_IDLE_TIMEOUT.
        """
        logging.info("Checking for inactivity in Installing Time:%d...",self.install_timestamp)

        #需要重试下载的话 退出上一个定时器
        if self.need_retry == True:
            logging.info("Need retry:Installing to exit and timeout check quit...")
            return False

        if self.window_main.now_working != self.ACTION_INSTALL and self.window_main.now_working != self.ACTION_INSTALL_SHUTDOWN:
            logging.info("Installing to exit and timeout check quit...")
            return False

        # 进度不同时 更新时间戳
        if self.aptd_base.progress !=  self.check_progress:
            self.check_progress = self.aptd_base.progress
            self.install_timestamp = INSTALL_IDLE_TIMEOUT

        #只有安装的时候启用 下载时候不使用
        if (self.install_timestamp <= 0 and self.on_installing == True):
            logging.error("Quitting due to inactivity(%s)",self.aptd_base.details)
            if self.action == self.ACTION_INSTALL_SHUTDOWN:
                #关机安装模式 解除禁止关机锁
                self.window_main.inhibit_lock.close()
                logging.info("Installtion timeout to exit Due to inactivity and Releasing the shutdown lock...")
            else:
                #超时只单独进行解锁关机
                self.inhibit_shutdown.unlock()
            self._action_done(self.ACTION_INSTALL,
                            is_cancelled=False, success=False,
                            #FIXME: 安装超时退出 
                            error_string=_("Could not install the upgrades"), 
                            error_desc=_("Installtion timeout to exit Due to inactivity") + self.aptd_base.details) 

            # self.dbus_send.Quit(None)
            return False
        else:
            self.install_timestamp = self.install_timestamp - 1

        return True

    def _check_update_inactivity(self):
        logging.info("Checking for inactivity in Updating...")
        #退出定时器 当更新完毕的时候
        if (self.aptd_base.cancelable == False and self.aptd_base.progress >= 90) or self.window_main.now_working != self.ACTION_UPDATE:
            logging.info("Updating to exit and timeout check quit...")
            return False

        #当更新进入下载状态时 记录进去的时间
        timestamp = self.update_timestamp
        if self.aptd_base.cancelable == True and timestamp == 0:
            self.update_timestamp = time.time()
            return True
        
        #超时设置
        if self.update_timestamp != 0 and  time.time() - self.update_timestamp > UPDATE_IDLE_TIMEOUT \
            and self.aptd_base.cancelable == True:

            logging.error("Quitting due to inactivity")
            self.dbus_send.transaction.cancel()
            return False
        
        return True

    @inline_callbacks
    def update(self):
        """刷新包cache"""
        try:
            trans = yield self.client.update_cache(defer=True)
            self.dbus_send.transaction = trans
            # 注册回调函数 接收更新的状态
            yield self._show_transaction(trans, self.ACTION_UPDATE,
                                        _("Checking for updates…"), False)
        except errors.NotAuthorizedError:
            self._action_done(self.ACTION_UPDATE,
                            authorized=False, success=False,
                            error_string='', error_desc='')
        except Exception:
            self._action_done(self.ACTION_UPDATE,
                            is_cancelled=False, success=False,
                            error_string='', error_desc='')
            raise

    @inline_callbacks
    def commit(self,model,pkgs_install, pkgs_upgrade, pkgs_remove,pkgs_downgrade = []):
        """Commit a list of package adds and removes"""
        try:
            reinstall = purge = []
            trans = yield self.client.commit_packages(
                pkgs_install, reinstall, pkgs_remove, purge = purge, upgrade = pkgs_upgrade,
                downgrade = pkgs_downgrade,defer=True)
            self.dbus_send.transaction = trans
            
            yield self._show_transaction(trans, self.action,
                                        _("Installing updates…"), True)
        except errors.NotAuthorizedError:
            self._action_done(self.action,
                            authorized=False, success=False,
                            error_string='', error_desc='')
        except errors.TransactionFailed as e:
            self.trans_failed_msg = str(e)
        except dbus.DBusException as e:
            if e.get_dbus_name() != "org.freedesktop.DBus.Error.NoReply":
                raise
            self._action_done(self.action,
                            authorized=False, success=False,
                            error_string='', error_desc='')
        except Exception:
            self._action_done(self.action,
                            is_cancelled=False, success=False,
                            error_string='', error_desc='')
            raise

    @inline_callbacks
    def commit_only(self,model,pkgs_install, pkgs_upgrade, pkgs_remove,pkgs_downgrade = []):
        """Commit a list of package adds and removes"""
        try:
            reinstall = purge = []
            trans = yield self.client.commit_only(
                pkgs_install, reinstall, pkgs_remove, purge = purge, upgrade = pkgs_upgrade,
                downgrade = pkgs_downgrade,download = model, defer=True)
            self.dbus_send.transaction = trans
            
            yield self._show_transaction(trans, self.action,
                                        _("Installing updates…"), True)
        except errors.NotAuthorizedError:
            self._action_done(self.action,
                            authorized=False, success=False,
                            error_string='', error_desc='')
        except errors.TransactionFailed as e:
            self.trans_failed_msg = str(e)
        except dbus.DBusException as e:
            if e.get_dbus_name() != "org.freedesktop.DBus.Error.NoReply":
                raise
            self._action_done(self.action,
                            authorized=False, success=False,
                            error_string='', error_desc='')
        except Exception:
            self._action_done(self.action,
                            is_cancelled=False, success=False,
                            error_string='', error_desc='')
            raise

    @inline_callbacks
    def commit_steps(self,model,pkgs_install, pkgs_upgrade, pkgs_remove,begin=0,end=100,last_steps=False):
        """Commit a list of package adds and removes"""
        try:
            reinstall = purge = downgrade =  []
            trans = yield self.client.commit_only(
                pkgs_install, reinstall, pkgs_remove, purge = purge, upgrade = pkgs_upgrade,
                downgrade = downgrade,download = model, defer=True)
            self.dbus_send.transaction = trans
            
            self.steps_progress_begin = begin
            self.steps_progress_end = end
            if last_steps == True:
                yield self._show_transaction(trans, self.action,
                                            _("Installing updates…"), True)
            else:
                yield self._show_transaction_steps(trans, self.action,
                                            _("Installing updates…"), True)
                
        except errors.NotAuthorizedError:
            self._action_done(self.action,
                            authorized=False, success=False,
                            error_string='', error_desc='')
        except errors.TransactionFailed as e:
            self.trans_failed_msg = str(e)
        except dbus.DBusException as e:
            if e.get_dbus_name() != "org.freedesktop.DBus.Error.NoReply":
                raise
            self._action_done(self.action,
                            authorized=False, success=False,
                            error_string='', error_desc='')
        except Exception:
            self._action_done(self.action,
                            is_cancelled=False, success=False,
                            error_string='', error_desc='')
            raise

    @inline_callbacks
    def install_deb(self,install_path,install_force):
        """安装deb包 """
        try:
            trans = yield self.client.install_file(path = install_path,force = install_force,defer=True)
            # 注册回调函数 接收更新的状态
            yield self._show_transaction(trans, self.ACTION_INSTALL_DEB,
                                        _("Installing deb packages…"), False)
        except errors.NotAuthorizedError:
            self._action_done(self.ACTION_INSTALL_DEB,
                            authorized=False, success=False,
                            error_string='', error_desc='')
        except Exception as e:
            self._action_done(self.ACTION_INSTALL_DEB,
                            is_cancelled=False, success=False,
                            error_string=str(e), error_desc='', error_code=e.code)
            # raise

    @inline_callbacks
    def fix_broken(self):
        """安装deb包 """
        try:
            trans = yield self.client.fix_broken_depends(defer=True)
            self.dbus_send.transaction = trans
            # 注册回调函数 接收更新的状态
            yield self._show_transaction(trans, self.ACTION_FIX_BROKEN,
                                        _("Installing deb packages…"), False)
        except errors.NotAuthorizedError:
            self._action_done(self.ACTION_FIX_BROKEN,
                            authorized=False, success=False,
                            error_string='', error_desc='')
        except Exception:
            self._action_done(self.ACTION_FIX_BROKEN,
                            is_cancelled=False, success=False,
                            error_string='', error_desc='')
            raise
    
    @inline_callbacks
    def fix_incomplete(self):
        """修复未完成的安装 """
        try:
            trans = yield self.client.fix_incomplete_install(defer=True)
            self.dbus_send.transaction = trans
            # 注册回调函数 接收更新的状态
            yield self._show_transaction(trans, self.ACTION_FIX_INCOMPLETE,
                                        _("fix incomplete install"), False)
        except errors.NotAuthorizedError:
            self._action_done(self.ACTION_FIX_INCOMPLETE,
                            authorized=False, success=False,
                            error_string='', error_desc='')
        except Exception:
            self._action_done(self.ACTION_FIX_INCOMPLETE,
                            is_cancelled=False, success=False,
                            error_string='', error_desc='')
            raise

    @inline_callbacks
    def clean(self):
        """清空所有下载的文件 """
        try:
            trans = yield self.client.clean(defer=True)
            self.dbus_send.transaction = trans
            # 注册回调函数 接收更新的状态
            yield self._show_transaction(trans, self.ACTION_CLEAN,
                                        _("Remove all downloaded files."), False)
        except errors.NotAuthorizedError:
            self._action_done(self.ACTION_CLEAN,
                            authorized=False, success=False,
                            error_string='', error_desc='')
        except Exception:
            self._action_done(self.ACTION_CLEAN,
                            is_cancelled=False, success=False,
                            error_string='', error_desc='')
            raise

    @inline_callbacks
    def purge_packages(self,pkgs_purge):
        """卸载deb包 """
        try:
            # trans = yield self.client.remove_packages(package_names = pkgs_purge,defer=True)
            trans = yield self.client.commit_packages([],[],[],pkgs_purge,[],[],defer=True)
            self.dbus_send.transaction = trans
            # 注册回调函数 接收更新的状态
            yield self._show_transaction(trans, self.ACTION_REMOVE_PACKAGES,
                                        _("Installing deb packages…"), False)
        except errors.NotAuthorizedError:
            self._action_done(self.ACTION_REMOVE_PACKAGES,
                            authorized=False, success=False,
                            error_string='', error_desc='')
        except Exception as e:
            logging.error(str(e))

    #进度回调
    def _on_progress_changed(self, trans,progress,action):
        #不要101这种未知状态
        if progress == 101:
            return

        if self.action_mode == self.MODE_INSTALL_STEP:
            progress = int(self.steps_progress_begin + (progress / 100) *
                           (self.steps_progress_end - self.steps_progress_begin))
        else:
            #过滤掉不是线性的进度 
            if progress > self.aptd_base.progress:
                self.aptd_base.progress = progress
            else:
                return
        
        self.aptd_base.progress = progress
        self._dist_status_changed(action,self.aptd_base.progress,self.aptd_base.status,self.aptd_base.details)

    #同步状态回调
    def _on_status_changed(self, trans, status,action):
        if action == self.ACTION_UPDATE and status == STATUS_FINISHED:
            return

        #转化词条
        self.aptd_base.status = get_status_string_from_enum(status)
        if self.aptd_base.status == None:
            return

        self._dist_status_changed(action,self.aptd_base.progress,self.aptd_base.status,self.aptd_base.details)
    
    #分发进度状态和细节信息
    def _dist_status_changed(self,action,progress = 0,status = '',details = ''):

        if action == self.ACTION_UPDATE: # 更新进度100后推迟发出100%的信号 -- 等待源过滤完成
            if progress == 11:
                progress = 15

            if progress != 100:
                self.dbus_send.UpdateDetectStatusChanged(progress,status)
        elif action == self.ACTION_INSTALL:
            if self.meta_upgrade.version_upgrade == True and progress >= 48 and 'Failed to fetch' in self.aptd_base.error_details: 
                logging.warning("Arise Failed to fetch and Need retry Upgrade...")
                self.need_retry = True
                return

            #50%时候 属于下载状态切换到安装状态的过程 下面的代码只执行一次
            #在安装的阶段的时候就进入安装阶段 禁止关机操作
            #提前进度安装状态
            if progress >= 49 and progress < 90 and self.on_installing == False:
                self.on_installing = True
                self.entering_installed()

            #在下载阶段发送取消信号
            if self.on_installing == False:
                self.dbus_send.Cancelable(self.aptd_base.cancelable)

            self.dbus_send.UpdateDloadAndInstStaChanged(self.meta_upgrade.get_content(),progress,status,details)

        elif action == self.ACTION_DOWNLOADONLY:
            #只处理从下载切换到安装时出现的网络问题
            #当网络波动时下载某些软件包失败时属于异常状态进行重试时 不发送后续进度 等待重试正常是 进行下载安装
            if self.meta_upgrade.version_upgrade == True and 'Failed to fetch' in self.aptd_base.error_details: 
                logging.warning("Arise Failed to fetch and Need retry Upgrade...")
                self.need_retry = True
                return

            self.dbus_send.Cancelable(self.aptd_base.cancelable)
            self.dbus_send.UpdateDloadAndInstStaChanged(self.meta_upgrade.get_content(),progress,status,details)
            
        elif action == self.ACTION_INSTALL_SHUTDOWN:
            # 写入进度 到plymouth
            self._progress_to_plymouth(progress)
            self.dbus_send.UpdateDloadAndInstStaChanged(self.meta_upgrade.get_content(),progress,status,details)
        elif action == self.ACTION_FIX_BROKEN:
            self.dbus_send.FixBrokenStatusChanged(False,True,progress,status,'','')
        elif action == self.ACTION_REMOVE_PACKAGES:
            self.dbus_send.PurgePkgStatusChanged(progress,status,details)
        elif action == self.ACTION_INSTALL_DEB or action == self.ACTION_BACKGROUND_UPGRADE:
            self.dbus_send.InstalldebStatusChanged(progress,status,details)
        else:
            logging.info("Other Action:progress = %d , status = %s ,details = %s",progress,status,details)

    def _on_details_changed(self, trans, details,action):
        self.aptd_base.details = details
        self._dist_status_changed(action,self.aptd_base.progress,self.aptd_base.status,self.aptd_base.details)

    def _on_download_changed(self, trans, details):
        logging.info(details)

    # eta 剩余时间不正确，取消掉
    def _on_progress_download_changed(self,trans,current_items, total_items, currenty_bytes, total_bytes, current_cps, eta):
        if self.action == self.ACTION_INSTALL or self.action == self.ACTION_DOWNLOADONLY or self.action == self.ACTION_BACKGROUND_UPGRADE:
            self.dbus_send.UpdateDownloadInfo(self.meta_upgrade.get_content(),\
                            current_items,  total_items, \
                            currenty_bytes, total_bytes, \
                            current_cps)
        else:
            if self.action == self.ACTION_UPDATE or self.action == self.ACTION_REMOVE_PACKAGES:
                return
            logging.info("Other Action:current_items = %d, total_items = %d, currenty_bytes = %s, total_bytes = %s, current_cps = %s/s",\
            current_items, total_items, \
            humanize_size(currenty_bytes), humanize_size(total_bytes),\
            humanize_size(current_cps))

    def _on_cancellable_changed(self, trans, Cancelable):
        #下面的这些 不发送取消信号
        if self.action == self.ACTION_REMOVE_PACKAGES:
            return

        if self.action != self.ACTION_UPDATE:
            logging.info("\033[1;32m" + "Emitting" + "\033[0m" +" Cancelable: %r",Cancelable) 
        self.dbus_send.Cancelable(Cancelable)
        #增加取消信号的频发机制
        self.aptd_base.cancelable = Cancelable

    def _on_config_file_conflict(self, transaction, old, new):
        logging.info("Config file conflict oldconf = %s , newconf = %s...",str(old),str(new))
        logging.info("Default To Replace Old Configfile...")

        transaction.resolve_config_file_conflict(old, "keep")

    #增加记录当产生错误的时候 详细信息 
    def _on_error_changed(self, trans,error_code, error_details):
        self.aptd_base.error_details = str(error_details)
        logging.error(str(error_details))

    @inline_callbacks
    def _show_transaction_steps(self, trans, action, header, show_details):

        trans.connect("finished", self._on_finished_steps, action)
        #升级和更新的状态信息和进度
        trans.connect("status-changed", self._on_status_changed,action)
        trans.connect("progress-changed", self._on_progress_changed,action)
        #取消升级
        trans.connect("cancellable-changed", self._on_cancellable_changed)
        #下载的进度信息
        trans.connect("progress-details-changed", self._on_progress_download_changed)
        trans.connect("status-details-changed", self._on_details_changed,action)
        trans.connect("error", self._on_error_changed)

        trans.connect("config-file-conflict", self._on_config_file_conflict)

        yield trans.run()

    @inline_callbacks
    def _show_transaction(self, trans, action, header, show_details):
        if action == self.ACTION_INSTALL_DEB:
            trans.connect("finished", self._on_finished_special, action)
        else:
            trans.connect("finished", self._on_finished, action)
        #升级和更新的状态信息和进度
        trans.connect("status-changed", self._on_status_changed,action)
        trans.connect("progress-changed", self._on_progress_changed,action)
        #取消升级
        trans.connect("cancellable-changed", self._on_cancellable_changed)
        #下载的进度信息
        trans.connect("progress-details-changed", self._on_progress_download_changed)
        trans.connect("status-details-changed", self._on_details_changed,action)
        trans.connect("error", self._on_error_changed)

        trans.connect("config-file-conflict", self._on_config_file_conflict)

        yield trans.run()

    def _on_finished_steps(self, trans, status, action):
        try:
            if status == EXIT_SUCCESS:
                self._steps_done()
            else:
                self._on_finished(trans, status, action)
        except Exception as e:
            logging.error(e)
            traceback.print_exc()

    def _on_finished(self, trans, status, action):
        try:
            error_string = ''
            error_desc = ''
            error_code = ''

            if status == EXIT_FAILED:
                # self.log_audit(str(trans.error.code))
                error_code = trans.error.code
                error_string = get_error_string_from_enum(trans.error.code)
                error_desc = get_error_description_from_enum(trans.error.code)
                if self.trans_failed_msg:
                    error_desc = error_desc + "\n" + self.trans_failed_msg
            #取消下载
            elif status == EXIT_CANCELLED:
                error_string = _("Failed to fetch")
                error_desc = _("_Cancel Upgrade")
            elif status == EXIT_SUCCESS and action == self.ACTION_INSTALL:
                pass
            elif status == EXIT_SUCCESS and action == self.ACTION_REMOVE_PACKAGES:
                error_string = _("Uninstallation completed")

            is_success = (status == EXIT_SUCCESS)

            self._action_done(action,
                            is_cancelled=(status == EXIT_CANCELLED), success=is_success,
                            error_string=error_string, error_desc=error_desc,error_code=error_code)
        except Exception as e:
            logging.error(e)
            traceback.print_exc()
    
    def _on_finished_special(self, trans, status, action):
        try:
            error_string = ''
            error_desc = ''
            error_code = ''

            if status == EXIT_FAILED and action == self.ACTION_INSTALL_DEB:
                error_code = trans.error.code
                error_string = get_error_string_from_enum(trans.error.code)
                error_desc = get_error_description_from_enum(trans.error.code)
                if str(error_code) == 'error-dep-resolution-failed':
                    status,detailed_information,err_info = self.window_main.start_deb_resolver(self.window_main.sqlite3_server.deb_metadata['deblist'])

            is_success = ((status == EXIT_SUCCESS) or (status == 0))
            self._action_done(action,
                            is_cancelled=(status == EXIT_CANCELLED), success=is_success,
                            error_string=error_string, error_desc=error_desc,error_code=error_code)
        except Exception as e:
            logging.error(e)
            traceback.print_exc()

    # def log_audit(self,error_code):
    #     if error_code == "error-package-manager-failed":
    #         error_msg = self.check_install_error()
            
    #     with open("/var/log/kylin-system-updater/error_details.log", 'w+') as configfile:
    #         configfile.write(str(error_msg))

    # def check_install_error(self):
    #     locate_error = ''
    #     with open("/var/log/apt/term.log", "r+") as f:
    #         log_start = None
    #         log_end = None
    #         aptterm_log = f.readlines()

    #         reverse_log = aptterm_log[::-1]

    #         for logstr in reverse_log:
    #             if log_end == None and "Log ended:" in logstr:
    #                 log_end = aptterm_log.index(logstr)
    #             if log_start == None and "Log started:" in logstr:
    #                 log_start = aptterm_log.index(logstr)

    #             if log_end != None and log_start != None:
    #                 break

    #         latest_log = aptterm_log[log_start:log_end+1]
    #         error_deb = latest_log[-2].strip()
            
    #         error_msg_line = None
    #         for log_msg in latest_log:
    #             if error_deb in log_msg:
    #                 error_msg_line = latest_log.index(log_msg)
    #                 locate_error = latest_log[error_msg_line-5:error_msg_line+5]
    #                 break
            
    #     return locate_error