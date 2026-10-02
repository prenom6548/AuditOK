#!/usr/bin/env python
# -*- Mode: Python; indent-tabs-mode: nil; tab-width: 4; coding: utf-8 -*-

"""Integration of package managers into SystemUpdater"""

import os
import apt
import apt_pkg
import json
import logging
import subprocess
import traceback
import shutil
import fcntl
import dbus
import threading
from apt import Cache
from gettext import gettext as _
from SystemUpdater.Core.errors import *
from SystemUpdater.Core.enums import *
from SystemUpdater.Core.DataAcquisition import get_east_8_time
from SystemUpdater.Core.DistUpgradeCache import NotEnoughFreeSpaceError
from SystemUpdater.Core.utils import get_config_patch
from SystemUpdater.Core.JsonConfigParser import JsonConfig as json_config
from aptdaemon.enums import ERROR_PACKAGE_DOWNLOAD_FAILED

class NowUpgradeMeta:
    """
    Represent the (potentially partial) results of an unattended-upgrades
    run
    """
    def __init__(self,_upgrade_groups = [],_single_pkgs = []):
        self.push_content = []
        #组列表中包含的包
        self.upgrade_groups = _upgrade_groups
        #推送的可升级的单包
        self.single_pkgs = _single_pkgs
        self.version_upgrade = True
    
    def get_content(self):
        return self.single_pkgs + self.upgrade_groups

class UpdateEssentialItem():
    def __init__(self,parent):
        self.essential_pkgs = []
        self.remove_white = []

        self.read_path = get_config_patch() + 'kylin-update-desktop-system.json'
        self.update()

    def update(self):
        if os.path.exists(self.read_path):
            with open(self.read_path,'r') as f:
                try:
                    data = json.load(f)
                    self.essential_pkgs = data['install_list']
                except Exception as exc:
                    logging.warning(exc)
                try:
                    self.remove_white = data['remove_white_list']
                except Exception as exc:
                    pass
                    # logging.warning(exc)
    
    def check_essential(self,remove_pkgs):
        logging.info("Check: For remove of Essential Packages...")
        for pkg in remove_pkgs:
            if pkg in self.essential_pkgs:
                logging.error("Essential Packages(%s) will be Removed...",pkg)
                raise UpdateBaseError(ERROR_REMOVE_ESSENTIAL_PACKAGES)
    
    def check_white(self,remove_pkgs):
        logging.info("Check: remove pkg in White Packages...")
        new_upgrade_list = list(set(remove_pkgs) & set(self.remove_white))

        for pkg in new_upgrade_list:
            if pkg in remove_pkgs:
                logging.info("%s will be remove in remove pkg...",pkg)
                remove_pkgs.remove(pkg)


class AptdBaseInformation:
    """
        进度 状态 状态细节信息
    """
    def __init__(self,
                progress = 0,
                status='',
                details='',
                cancelable = False,
                error_details = '',
                ):

        self.progress = progress
        self.status = status
        self.details = details
        self.cancelable = cancelable
        self.error_details = error_details

class InstallBackend():
    INSTALL_OUTPUT_JSON = "/var/lib/kylin-system-updater/json/requires_upgrade_meta.json"

    ACTION_DEFUALT_STATUS = -1
    ACTION_UPDATE = 0
    ACTION_INSTALL = 1
    ACTION_INSTALL_DEB = 2
    ACTION_CHECK_RESOLVER = 3
    ACTION_DOWNLOADONLY = 4
    ACTION_FIX_BROKEN = 5
    ACTION_REMOVE_PACKAGES = 6
    ACTION_FIX_INCOMPLETE = 7
    ACTION_CLEAN = 8
    ACTION_INSTALL_SHUTDOWN = 9
    ACTION_BACKGROUND_UPGRADE = 10
    ACTION_CHECK_BROKEN = 11
    ACTION_INSTALL_DEB_RESOLVER = 12

    MODE_DEFAULT_STATUS = -1
    
    MODE_INSTALL_PARTIAL = 0
    MODE_INSTALL_ALL = 1
    MODE_INSTALL_SYSTEM = 2
    MODE_INSTALL_SINGLE = 3
    MODE_INSTALL_STEP = 4

    MODE_UPDATE_CACHE = 0
    MODE_UPDATE_ALL = 1

    def __init__(self, window_main, action,mode = MODE_DEFAULT_STATUS):
        self.window_main = window_main
        self.cache = window_main.cache
        self.action = action
        self.action_mode = mode

        self.window_main.now_working = action

        self.aptd_base = AptdBaseInformation()
        self.collector = window_main.collector

        self.backend_data = window_main.main_meta
        self.dbus_send = window_main.dbusController
        self.database = window_main.sqlite3_server
        self.meta_upgrade = NowUpgradeMeta()

        if self.action != self.ACTION_UPDATE:
            self.safe_manager = UpdateSafeManager()
            self.update_essential = UpdateEssentialItem(self)
            self.inhibit_shutdown = InhibitShutdownLock()
            self.scripts = window_main.pm_class
        
    def start(self,push_content = []):
        try:
            #安装升级包 首先必须调用ACTION_CHECK_RESOLVER 计算依赖解决方便 标记cache 进行升级
            if self.action == self.ACTION_INSTALL or self.action == self.ACTION_DOWNLOADONLY:
                self.meta_upgrade.push_content = push_content
                if self.action_mode == self.MODE_INSTALL_SINGLE:
                    pkgs_install,pkgs_upgrade,pkgs_remove = self.get_mark_from_cache(self.cache)

                    self.meta_upgrade.single_pkgs = push_content
                elif self.action_mode == self.MODE_INSTALL_STEP:
                    self.meta_upgrade.single_pkgs,self.meta_upgrade.upgrade_groups = self.backend_data.classify_content(push_content)

                    pkgs_install,pkgs_upgrade,pkgs_remove = self.backend_data.get_resolver_upgrade()
                elif self.action_mode == self.MODE_INSTALL_SYSTEM:
                    self.meta_upgrade.upgrade_groups = ["dist-upgrade"]
                    pkgs_install,pkgs_upgrade,pkgs_remove = self.backend_data.get_resolver_upgrade()
                else:
                    #单独下载和下载安装同时走此分支
                    self.meta_upgrade.single_pkgs,self.meta_upgrade.upgrade_groups = self.backend_data.classify_content(push_content)
                    pkgs_install,pkgs_upgrade,pkgs_remove = self.backend_data.get_resolver_upgrade()

                logging.info("INSTALL install:%d , upgrade:%d remove:%d",len(pkgs_install),len(pkgs_upgrade),len(pkgs_remove))

                self.update_essential.check_essential(pkgs_remove)

                for ul in self.collector.upgrade_list:
                    self.collector.Upgrade_Process_Msg(self.action, {"appname":ul})

                if self.action_mode != self.MODE_INSTALL_SINGLE:
                    self._update_to_config(self.meta_upgrade,pkgs_install,pkgs_upgrade,pkgs_remove)

                if len(pkgs_install) + len(pkgs_upgrade) > 100:
                    logging.info("Open a major version upgrade and Retry mechanism on...")
                    self.meta_upgrade.version_upgrade = True

                if self.action == self.ACTION_DOWNLOADONLY:
                    self.commit_only(self.action,pkgs_install, pkgs_upgrade, pkgs_remove)
                    return

                if self.action_mode != self.MODE_INSTALL_STEP and self.action_mode != self.MODE_INSTALL_SINGLE:
                    #更新前执行脚本的操作
                    for groups in self.meta_upgrade.upgrade_groups:
                        if self.run_steps_script(50,'',groups) == False:
                            raise UpdateBaseError(ERROR_RUN_SCRIPTS_FAILED)

                if self.action_mode == self.MODE_INSTALL_STEP:
                    logging.info("Will be enter to step install model...")
                    self.commit_steps(self.ACTION_DOWNLOADONLY,pkgs_install, pkgs_upgrade, pkgs_remove,0,50)
                else:
                    self.commit(self.action,pkgs_install, pkgs_upgrade, pkgs_remove)
            elif self.action == self.ACTION_INSTALL_SHUTDOWN:
                self.meta_upgrade,pkgs_install,pkgs_upgrade,pkgs_remove = self._config_to_upgrade()
                logging.info("ACTION_INSTALL_SHUTDOWN install:%d , upgrade:%d remove:%d",len(pkgs_install),len(pkgs_upgrade),len(pkgs_remove))

                # self._start_install_lock()
                self.commit(self.action,pkgs_install, pkgs_upgrade, pkgs_remove)

            #修复未满足的依赖关系 与apt-get -f install匹配
            elif self.action == self.ACTION_CHECK_BROKEN:
                try:
                    #计算依赖的解决方案
                    self.cache.fix_broken()
                except Exception as e:
                    logging.error(e)
                    raise UpdateBaseError(ERROR_RESOLVER_FAILED)

                pkgs_install,pkgs_upgrade,pkgs_remove = self._get_pkgs_from_cache(self.cache)
                logging.warning("Fix broken Packages is need to install:%a and upgrade:%a and remove:%a",pkgs_install,pkgs_upgrade,pkgs_remove)

                #当存在需要卸载包的情况时 不进行修复
                if len(pkgs_remove) == 0:
                    logging.warning("start fix system broken pkgs...")
                    self.fix_broken()
                else:
                    raw_description = []
                    delete_desc = []
                    #添加关于删除包的描述信息
                    for pkg in pkgs_remove:
                        delete_desc.append('')
                        pkg_obj = self.cache[pkg]
                        raw_description.append(getattr(pkg_obj.candidate, "summary", ''))
                    
                    self.dbus_send.UpdateFixBrokenStatus(True,True,pkgs_remove,raw_description,delete_desc,'','')
                    self._action_done(self.action,False,True,'','')
                    
            elif self.action == self.ACTION_FIX_BROKEN:
                pkgs_install,pkgs_upgrade,pkgs_remove = self._get_pkgs_from_cache(self.cache)

                self.update_essential.check_essential(pkgs_remove)
                self.fix_broken()
            # 修复不完整的安装dpkg configure -a
            elif self.action == self.ACTION_FIX_INCOMPLETE:
                self.fix_incomplete()
            #卸载包
            elif self.action == self.ACTION_REMOVE_PACKAGES:
                self._start_install_lock(_("Kylin System Updater"))
                self.purge_packages(push_content)
            elif self.action == self.ACTION_CLEAN:
                self.clean()
            #更新cache
            elif self.action == self.ACTION_UPDATE:
                #检查依赖之前 判断是否需要重启aptdeamon 目前为了生效限速功能
                self.window_main.check_config_aptdeamon()
                self.update()
        except UpdateBaseError as excep:
            self._action_done(self.action,True,False,excep.header,excep.desc,excep.code)
        except UpdateProgressExit as excep:
            pass
        except Exception as e:
            logging.error(e) 
            traceback.print_exc()

    def start_resolver(self,push_content):
        try:
            #被删除包的描述
            raw_description = []
            total_remove = []
            
            if self.action_mode != self.MODE_INSTALL_SYSTEM:
                all_install,all_upgrade = self.backend_data.get_push_pkgs(push_content)
                logging.info("ALLs install:%d , upgrade:%d",len(all_install),len(all_upgrade))

                self._make_problem_resolver(self.cache,all_install,all_upgrade)
                pkgs_install,pkgs_upgrade,pkgs_remove = self.get_mark_from_cache(self.cache)
                total_remove += pkgs_remove

                self.backend_data.resolver_groups(pkgs_install,pkgs_upgrade,pkgs_remove)
                self.check_free_space(self.cache)

                #分步更新处理过程
                if self.action_mode == self.MODE_INSTALL_STEP:
                    steps_reboot = False
                    for step_name in self.backend_data.get_steps():
                        step_install,step_upgrade = self.backend_data.get_steps_pkgs(step_name)
                        logging.info("Steps(%s): install:%d , upgrade:%d",step_name,len(step_install),len(step_upgrade))
                        self._make_problem_resolver(self.cache,step_install,step_upgrade)

                        step_install,step_upgrade,step_remove = self.get_mark_from_cache(self.cache)
                        logging.info("RESOLVER(%s):Steps install:%d , upgrade:%d remove:%d ",step_name,len(step_install),\
                                                                                        len(step_upgrade),len(step_remove))

                        if len(step_install) == 0 and len(step_upgrade) == 0 and len(step_remove) == 0:
                            logging.warning("Steps(%s) This update cannot detect the upgradeable package.",step_name)

                        self.backend_data.resolver_steps(step_name,step_install,step_upgrade,step_remove)

                        total_remove += step_remove

                        self.backend_data.steps_queue.insert(0,step_name)

                        steps_reboot = self.backend_data.check_steps_reboot(step_name)
                        if steps_reboot == True:
                            break
                    
                    if steps_reboot == False:
                        self.backend_data.steps_queue.insert(0,self.backend_data.resolver_name)
            else:
                # 使用全盘升级 类似 apt dist-upgrade
                if self.cache.get_changes():
                    self.cache.clear()
                self.cache._depcache.upgrade(True)
                pkgs_install,pkgs_upgrade,pkgs_remove = self.get_mark_from_cache(self.cache)
                self.backend_data.resolver_groups(pkgs_install,pkgs_upgrade,pkgs_remove)

                self.check_free_space(self.cache)

                total_remove += pkgs_remove

            #可能存在重复，去除
            total_remove = list(set(total_remove))

            logging.warning("ProblemResolver of the deletion package list:%s",str(total_remove))                
            self.update_essential.check_white(total_remove)
            
            logging.info("RESOLVER:ALL install:%d , upgrade:%d remove:%d ",len(pkgs_install),len(pkgs_upgrade),\
                                                                        len(total_remove))
            
            is_remove_pkgs = len(total_remove) != 0

            # 数据上报
            self.collector.Generate_Msg(self.meta_upgrade.get_content(), self.action_mode)
            errorCode = ""
            if is_remove_pkgs:
                errorCode = _("Need remove pkgs: ")+", ".join(total_remove)
            for ul in self.collector.upgrade_list:
                self.collector.Upgrade_Process_Msg(self.action, {"appname":ul, "status":is_remove_pkgs, "errorCode":errorCode})
            
            #补充删除包的描述信息,删除描述
            delete_desc = []
            for pkg in total_remove:
                pkg_obj = self.cache[pkg]
                raw_description.append(getattr(pkg_obj.candidate, "summary", ''))
                delete_desc.append('')

            if self.action_mode != self.MODE_INSTALL_SYSTEM:
                self.dbus_send.UpdateDependResloveStatus(True,is_remove_pkgs,total_remove,raw_description,delete_desc,'','')
            else:
                self.dbus_send.DistupgradeDependResloveStatus(True,is_remove_pkgs,total_remove,raw_description,delete_desc,'','')

            self._action_done(self.action,False,True,'','')
            
            if is_remove_pkgs:
                threading_emulate = threading.Thread(target=self._emulate_calcul_delete,args=((pkgs_upgrade + pkgs_install),))
                threading_emulate.start()

        except UpdateBaseError as excep:
            self.dbus_send.InstallDetectStatus(False,get_error_code_from_enum(excep.code))
            self._action_done(self.action,True,False,excep.header,excep.desc,excep.code)
        except UpdateProgressExit as excep:
            pass
        except Exception as e:
            logging.error(e) 
            traceback.print_exc()

    def start_alone(self,push_content = [],_is_install = False, caller=''):
        # 安装本地deb包的接口
        if self.action == self.ACTION_INSTALL_DEB: 
            try:
                self._start_install_lock(caller=caller)
                self.install_deb(install_path = push_content, install_force = _is_install)
            except Exception as e:
                logging.error(str(e))
        elif self.action == self.ACTION_INSTALL_DEB_RESOLVER:
            try:
                self._make_problem_resolver(self.cache,push_content,[])
                pkgs_install,pkgs_upgrade,pkgs_remove = self.get_mark_from_cache(self.cache)
                self._start_install_lock(caller=caller)
                self.commit(self.ACTION_INSTALL,pkgs_install,pkgs_upgrade,pkgs_remove)
            except Exception as e:
                logging.error(str(e))
        # 安装在线包的接口
        elif self.action == self.ACTION_BACKGROUND_UPGRADE:
            try:
                pkgs_install = [ str(pkg) for pkg in push_content]
                logging.info("Install deb package, open cache")
                cache = Cache()
                for pkg_name in pkgs_install:
                    if pkg_name not in cache:
                        if "=" not in pkg_name or ("=" in pkg_name and pkg_name.split("=")[0] not in cache):
                        # 没找到包或格式不正确
                            self.dbus_send.UpdateInstallFinished(False, pkgs_install, "'"+pkg_name+"' is not in cache", "")
                            return
                self._start_install_lock(caller=caller)
                self.commit(self.ACTION_INSTALL,pkgs_install,[],[])
            except Exception as e:
                logging.error(str(e))
                self.dbus_send.UpdateInstallFinished(False, pkgs_install, str(e), "")

    def update(self):
        """Run a update to refresh the package list"""
        raise NotImplementedError

    def clean(self):
        """Remove all downloaded files.t"""
        raise NotImplementedError

    def commit(self,model,pkgs_install, pkgs_upgrade, pkgs_remove,pkgs_downgrade = []):
        """Commit the cache changes """
        raise NotImplementedError

    def install_deb(self,install_path,install_force):
        """install_deb the cache changes """
        raise NotImplementedError

    def fix_broken(self):
        """fix_broken_depends"""
        raise NotImplementedError

    def fix_incomplete(self):
        """fix_incomplete_install"""
        raise NotImplementedError

    def purge_packages(self,pkgs_purge):
        """purge_packages"""
        raise NotImplementedError

    def _get_pkgs_from_cache(self,cache):
        pkgs_install = []
        pkgs_upgrade = []
        pkgs_remove = []

        #计算修复破损包 方案 需要删除 新装那些包
        for pkg in cache:
            try:
                if pkg.marked_install :
                    pkgs_install.append(pkg.name)
                elif pkg.marked_upgrade:
                    pkgs_upgrade.append(pkg.name)
                elif pkg.marked_delete:
                    pkgs_remove.append(pkg.name)
            except KeyError:
                # pkg missing from fresh_cache can't be modified
                pass
        return pkgs_install,pkgs_upgrade,pkgs_remove

    def get_mark_from_cache(self,cache):
        '''从cache中拿到标记的升级新装和删除的包

        入参：
        cache - 当前cache
        出参：
        pkgs_install - 当前要安装的包列表
        pkgs_upgrade - 当前要安装的包列表
        pkgs_remove - 当前要卸载的包列表
        '''
        pkgs_install = []
        pkgs_upgrade = []
        pkgs_remove = []

        for pkg in cache:
            try:
                if pkg.marked_install:
                    pkgname = pkg.name
                    if pkg.is_auto_installed:
                        pkgname += "#auto"
                        pkgs_install.append(pkgname)
                    else:
                        pkgs_install.append(pkgname)
                elif pkg.marked_upgrade:
                        pkgs_upgrade.append(pkg.name)
                elif pkg.marked_delete:
                    pkgs_remove.append(pkg.name)
            except KeyError:
                pass
        return pkgs_install,pkgs_upgrade,pkgs_remove

    #从本地中获取本次升级需要升级的包 部分升级和全部升级使用 全盘升级不适用
    def _make_pkgs_list(self,cache,groups_pkgs,groups_list,pkg_list):
        pkgs_install = []
        pkgs_upgrade = []
        
        try:
            #单包的升级方式
            for pkg in pkg_list:
                if cache[pkg].is_installed:
                    pkgs_upgrade.append(pkg)
                else:
                    pkgs_install.append(pkg) 

            #遍历升级组列表
            for group_name in groups_list:
                pkgs_install += groups_pkgs.get(group_name,[]).get('pkgs_install',[])
                pkgs_upgrade += groups_pkgs.get(group_name,[]).get('pkgs_upgrade',[]) 
            
            return pkgs_install,pkgs_upgrade
        except Exception as e:
            logging.error(str(e))
            return [],[]
    
    def _installed_dependencies(self, pkg_name, all_deps=None):
        """Recursively return all installed dependencies of a given package."""
        # FIXME: Should be part of python-apt, since it makes use of non-public
        #       API. Perhaps by adding a recursive argument to
        #       apt.package.Version.get_dependencies()
        if not all_deps:
            all_deps = set()
        if pkg_name not in self.cache:
            return all_deps
        cur = self.cache[pkg_name]._pkg.current_ver
        if not cur:
            return all_deps
        for sec in ("PreDepends", "Depends", "Recommends"):
            try:
                for dep in cur.depends_list[sec]:
                    dep_name = dep[0].target_pkg.name
                    if dep_name not in all_deps:
                        all_deps.add(dep_name)
                        all_deps |= self._installed_dependencies(dep_name,
                                                                 all_deps)
            except KeyError:
                pass
        return all_deps

    def _split_package_id(self,package):
        """Return the name, the version number and the release of the
        specified package."""
        if "=" in package:
            name, version = package.split("=", 1)
            release = None
        elif "/" in package:
            name, release = package.split("/", 1)
            version = None
        else:
            name = package
            version = release = None
        return name, version, release

    def _make_problem_resolver(self,cache,pkgs_install = [],pkgs_upgrade = []):
        try:
            logging.info("Start calculating dependencies...")

            with cache.actiongroup():
                if cache.get_changes():
                    cache.clear()
                resolver = apt.cache.ProblemResolver(cache)

                for pkg in pkgs_upgrade + pkgs_install:
                    pkg_cache = cache[pkg]
                    if pkg_cache.is_upgradable == False and pkg_cache.is_installed == True:
                        logging.warning("pkg(%s) not upgrade or install and will be continue...",pkg)
                        continue

                    pkg_cache.mark_install(False, True, True)

                    if pkg_cache.is_upgradable == True:
                        auto = pkg_cache.is_auto_installed
                        pkg_cache.mark_auto(auto)

                    if pkg_cache.marked_keep == True:
                        pkg_cache.mark_install(False, False, True)
                        if pkg_cache.marked_keep == True:
                            raise

                    resolver.clear(pkg_cache)
                    resolver.protect(pkg_cache)

                resolver.resolve()
            return
        except Exception as e:
            logging.error(str(e))
            pkg_string = ''
            for pkg in pkgs_install + pkgs_upgrade:
                pkg_string = pkg_string + ' ' + str(pkg)
            logging.error('Resolver calculation Packages List: '+pkg_string+'\n')
            terminal_msg = self.window_main.simulate_mode.dependencies_broken(pkgs_install + pkgs_upgrade)
            logging.error(terminal_msg)
            raise UpdateBaseError(ERROR_RESOLVER_FAILED,
                        desc= terminal_msg)

    def _emulate_calcul_delete(self,total_pkg):
        logging.info("[DELETE_DETAILS] Start calculating delete dependencies...")
        delete_pkgs = []
        last_delete_count = 0
        cache = Cache()

        with cache.actiongroup():
            if cache.get_changes():
                cache.clear()
            for pkg in total_pkg:
                if '#' in pkg:
                    pkg = pkg.split("#")[0]
                elif '=' in pkg:
                    pkg = pkg.split("=")[0]

                try:
                    cache[pkg].mark_install()
                except Exception as e:
                    logging.error(str(e))
                    break

                if cache.delete_count != last_delete_count:
                    last_delete_count = cache.delete_count
                    for pkg_obj in cache.get_changes():
                        if pkg_obj.marked_delete and pkg_obj.name not in delete_pkgs:
                            delete_pkgs.append(pkg_obj.name)
                            delete_pkgs.append(pkg)
                            logging.info("[DELETE_DETAILS] %s Will be deleted Due to Install or Upgrade %s",pkg_obj.name,pkg)
        
        logging.info("[DELETE_DETAILS] End calculating delete dependencies...")
        cache.close()

    def _send_error_code(self,error_code):
        if error_code == ERROR_NOT_DISK_SPACE:
            self.dbus_send.InstallDetectStatus(False,error_code)

    def _self_upgrade_finished(self,success,error_string='',error_desc='',error_code=''):
        if success:
            restart_ui = False

            channel_config = json_config().getWithDefault("update_self_upgrade",default = {})

            restart_list = channel_config.get("restart_service_list",[self.window_main.APTD_PKG_NAME,self.window_main.BACKEND_PKG_NAME])
            notify_list = channel_config.get("restart_panel_list",[self.window_main.FRONTEND_PKG_NAME])

            error_string = get_error_string_from_enum(PRIORITY_UPGRADE_SUCCCESSED)

            if (self.window_main.APTD_PKG_NAME in self.meta_upgrade.get_content() and self.window_main.BACKEND_PKG_NAME not in self.meta_upgrade.get_content()) \
                        and self.window_main.APTD_PKG_NAME in restart_list:
                self.dbus_send.make_aptdeamon_restart()

            if self.window_main.FRONTEND_PKG_NAME in self.meta_upgrade.get_content() and \
                    self.window_main.FRONTEND_PKG_NAME in notify_list:
                self.dbus_send.UpdateDetectFinished(False,[self.window_main.BACKEND_PKG_NAME],error_string,'')
                restart_ui = True

            # 异常情况 需要强制重启ui提示
            if "force_restart_ui" in notify_list:
                self.dbus_send.UpdateDetectFinished(False,[self.window_main.BACKEND_PKG_NAME],error_string,'')
                restart_ui = True

            if self.window_main.BACKEND_PKG_NAME in self.meta_upgrade.get_content() and \
                    self.window_main.BACKEND_PKG_NAME in restart_list:
                if restart_ui == False:
                    self.window_main.configs_uncover.setValue("SystemStatus","reboot_update",str(True))
                self.dbus_send.Quit(None)
                return 

            if restart_ui != True:
                self.window_main.start_available()
        else:
            self.dbus_send.InstallDetectStatus(False,get_error_code_from_enum(error_code))
            self.dbus_send.UpdateDetectFinished(False,[''],get_error_string_from_enum(ERROR_UPDATE_DEFAULT_FAILED),\
                                                                    error_string +' '+error_desc)

    def run_steps_script(self,progress,last_steps,steps):
        if progress == 50:
            script_list = self.backend_data.get_pre_script(steps)
            for scr in script_list:
                if self.scripts.run_plugin(scr) == False:
                    return True
        elif progress == 100:
            script_list = self.backend_data.get_post_script(last_steps)
            for scr in script_list:
                if self.scripts.run_plugin(scr) == False:
                    return True
        else:
            script_list = self.backend_data.get_post_script(last_steps) + self.backend_data.get_pre_script(steps)
            for scr in script_list:
                if self.scripts.run_plugin(scr) == False:
                    return True
        return True

    def _steps_done(self):
        #分步更新任务队列
        if self.backend_data.steps_queue != []: 
            now_steps = self.backend_data.steps_queue.pop()
            progress_begin,progress_end = self.backend_data.get_progress(now_steps)
            pkgs_install,pkgs_upgrade,pkgs_remove = self.backend_data.get_resolver_steps(now_steps)

            if self.run_steps_script(progress_begin,self.backend_data.last_task,now_steps) == False:
                self._action_done(self.action,False,False,get_error_string_from_enum(ERROR_RUN_SCRIPTS_FAILED),\
                                            get_error_description_from_enum(ERROR_RUN_SCRIPTS_FAILED))
                return

            #记录上一次的任务
            self.backend_data.last_task = now_steps

            mid_info = self.backend_data.get_mid_notify(now_steps)
            if mid_info != '':
                self.dbus_send.NotifyStepsInstalled(True,mid_info)

            logging.info("INSTALL(%s) install:%d , upgrade:%d remove:%d",now_steps,len(pkgs_install),len(pkgs_upgrade),len(pkgs_remove))
            self.commit_steps(self.action,pkgs_install, pkgs_upgrade, pkgs_remove,\
                                        progress_begin,progress_end,self.backend_data.steps_queue == [])
            return

    #调用aptdeamon结束之后处理的地方 不管是出错还是正常都在此处理
    def _action_done(self, action, is_cancelled,success, error_string='',error_desc='',error_code=''):    
        self.window_main.now_working = self.ACTION_DEFUALT_STATUS
        
        if action == self.ACTION_INSTALL:
            #整个安装执行完毕 退出
            self._send_error_code(error_code)
            self.exit_installed()

            if self.action_mode == self.MODE_INSTALL_STEP:
                if self.run_steps_script(100,self.backend_data.last_task,'') == False:
                    error_string = get_error_string_from_enum(ERROR_RUN_SCRIPTS_FAILED)
                    error_desc = get_error_description_from_enum(ERROR_RUN_SCRIPTS_FAILED)
                    success = False
                
                #弹窗的处理过程 在全部安装时不会进行弹窗
                if success and self.backend_data.last_task != self.backend_data.resolver_name:
                    post_info = self.backend_data.get_post_notify()
                    if post_info != '':
                        self.dbus_send.PopupStepsInstalled(True,post_info)
                    self.dbus_send.RebootLogoutRequired("reboot")

                if success == False or self.backend_data.last_task == self.backend_data.resolver_name:
                    self.database.insert_info(action,self.meta_upgrade.single_pkgs,\
                                                                self.meta_upgrade.upgrade_groups,[],success,error_string,error_desc)
                
                self.dbus_send.InstallDetectStatus(False,get_error_code_from_enum(error_code))
                self.dbus_send.UpdateInstallFinished(success,self.meta_upgrade.get_content(),error_string,error_desc)

                self.window_main.refresh_cache()

            elif self.action_mode == self.MODE_INSTALL_SINGLE:
                self._self_upgrade_finished(success,error_string,error_desc,error_code)
            else:
                #下载重试机制的判断
                if self.meta_upgrade.version_upgrade == True and success == False and error_code == ERROR_PACKAGE_DOWNLOAD_FAILED:
                    if self.dbus_send.check_connectivity_pro() == True:
                        if self.window_main.retry_limit != 0:
                            logging.warning("Retry the upgrade and installaton and retry number: %d",self.window_main.retry_limit)
                            self.window_main.start_install(self.action_mode,True,self.meta_upgrade.push_content)
                            self.window_main.retry_limit = self.window_main.retry_limit - 1
                            return
                            
                #更新完毕后执行脚本的操作
                for groups in self.meta_upgrade.upgrade_groups:
                    if self.run_steps_script(100,groups,'') == False:
                        error_string = get_error_string_from_enum(ERROR_RUN_SCRIPTS_FAILED)
                        error_desc = get_error_description_from_enum(ERROR_RUN_SCRIPTS_FAILED)
                        success = False

                self.database.insert_info(action,self.meta_upgrade.single_pkgs,self.meta_upgrade.upgrade_groups\
                                                                            ,[],success,error_string,error_desc)
                self.dbus_send.InstallDetectStatus(False,get_error_code_from_enum(error_code))
                self.dbus_send.UpdateInstallFinished(success,self.meta_upgrade.get_content(),error_string,error_desc)

                if success:
                    self.backend_data.pop(self.meta_upgrade.get_content())
                    self.dbus_send.CheckRebootRequired("self")
                    logging.info("Install or Upgrade successful, so Now update Cache...")
                    self.window_main.refresh_cache()
            
        elif action == self.ACTION_INSTALL_SHUTDOWN:
            # self._release_install_lock()
            self._send_error_code(error_code)
            #插入数据库
            self.database.insert_info(self.ACTION_INSTALL,self.meta_upgrade.single_pkgs,\
                                                        self.meta_upgrade.upgrade_groups,[],success,error_string,error_desc)
            if success == True:
                #当升级完成时 将手动和自动安装的标志位全部清空
                self.window_main.install_mode.install_finished()
                
            self.dbus_send.UpdateInstallFinished(success,self.meta_upgrade.get_content(),error_string,error_desc)

            #释放锁 允许关机
            if self.window_main.install_mode.inhibit_lock != None:
                self.window_main.install_mode.inhibit_lock.close()
                logging.info("Install Packages Finished and Releasing the shutdown lock...")
            else:
                logging.error("Install Packages Finished and Releasing the shutdown lock Failed...")
                #升级本身完成后 来释放关机锁
                self.dbus_send.Quit(None)

        elif action == self.ACTION_CHECK_RESOLVER:
            if success == False:
                self.database.insert_info(self.action_mode,self.meta_upgrade.single_pkgs,\
                                                            self.meta_upgrade.upgrade_groups,[],success,error_string,error_desc)

                if self.action_mode != self.MODE_INSTALL_SYSTEM:
                    self.dbus_send.UpdateDependResloveStatus(success,False,[''],[''],[''],error_string,error_desc)
                else:
                    self.dbus_send.DistupgradeDependResloveStatus(success,False,[''],[''],[''],error_string,error_desc)

        elif action == self.ACTION_DOWNLOADONLY:
            if success == True and self.action_mode == self.MODE_INSTALL_PARTIAL:
                self.window_main.install_mode.tmp_content += self.meta_upgrade.get_content()
            
            if success == False:
                self.database.insert_info(self.action_mode,self.meta_upgrade.single_pkgs,\
                                                            self.meta_upgrade.upgrade_groups,[],success,error_string,error_desc)
            
            #如果下载成功 就标志需要 安装重启
            if success == True:
                #安装成功的话再获取关机锁
                self.window_main.install_mode.get_inhibit_lock()

            self.window_main.install_mode.set_shutdown_install(success)

            self.dbus_send.UpdateDownloadFinished(success,self.meta_upgrade.get_content(),error_string,error_desc)

        elif action == self.ACTION_UPDATE and self.action_mode == self.MODE_UPDATE_ALL:
            if success == False:
                # threading_emulate = threading.Thread(target=self.emulate_update)
                # threading_emulate.start()

                if ERROR_UPDATE_KEY_SIGNATURES in self.aptd_base.error_details:
                    error_desc = get_error_description_from_enum(ERROR_UPDATE_KEY_SIGNATURES)
                elif ERROR_UPDATE_NET_AUTHENTICATION in self.aptd_base.error_details:
                    error_desc = get_error_description_from_enum(ERROR_UPDATE_NET_AUTHENTICATION)
                elif ERROR_UPDATE_NOTREAD_SOURCES in self.aptd_base.error_details:
                    error_desc = get_error_description_from_enum(ERROR_UPDATE_NOTREAD_SOURCES)
                elif ERROR_UPDATE_INVALID_TIME in self.aptd_base.error_details:
                    error_desc = get_error_description_from_enum(ERROR_UPDATE_INVALID_TIME)
            
            if success:
                #开始生成列表
                self.window_main.start_available()
            else:
                self.dbus_send.InstallDetectStatus(False,get_error_code_from_enum(error_code))
                self.dbus_send.UpdateDetectFinished(False,[''],get_error_string_from_enum(ERROR_UPDATE_DEFAULT_FAILED),\
                                                                        error_string +' '+error_desc)
            self.database.insert_into_display("check_time",get_east_8_time()[0:-4])

        elif action == self.ACTION_UPDATE and self.action_mode == self.MODE_UPDATE_CACHE:
            self.dbus_send.InstallDetectStatus(False,get_error_code_from_enum(error_code))
            self.dbus_send.UpdateDetectFinished(success,[''],error_string,error_desc)
        elif action == self.ACTION_FIX_BROKEN:
            self.dbus_send.FixBrokenStatusChanged(True,success,100,'',error_string,error_desc)
            logging.warning("fix broken packages is complete...")
            
            if success:
                logging.info("Fix broken packages is complete to success...")
                self.window_main.start_available()
            else:
                self.dbus_send.InstallDetectStatus(False,get_error_code_from_enum(ERROR_NOT_FIX_SYSTEM))
                self.dbus_send.UpdateDetectFinished(success,[''],\
                                            get_error_string_from_enum(ERROR_NOT_FIX_SYSTEM),error_string+' '+error_desc)
                logging.error("fix broken packages is complete to failed...")

        elif action == self.ACTION_FIX_INCOMPLETE:
            if success:
                logging.info("fix incomplete install success.")
            else:
                logging.warning("fix incomplete install failed.")

        elif action == self.ACTION_REMOVE_PACKAGES:
            self._release_install_lock()
            # self.dbus_send.PurgePackagesFinished(success,error_string,error_desc," ".join(self.window_main.sqlite3_server.current_purge_pkgs))
            self.dbus_send.PurgePackagesFinished(success,error_string,error_desc)
            
        elif action == self.ACTION_INSTALL_DEB or action == self.ACTION_INSTALL_DEB_RESOLVER:
            self._release_install_lock()
            if success == False and '\r\n: \r\n\r\n' in self.aptd_base.error_details:
                error_string = _("Package validation failed and installation was rejected.")
                error_desc = ''
            if error_code != None and str(error_code) != 'error-dep-resolution-failed':
                if error_desc != '':
                    error_string = ",".join((error_string,error_desc))
                self.dbus_send.InstalldebFinished(success,error_string,'')
                UpdateMsg = {}
                if success:
                    status = 'success'
                    UpdateMsg.update({"errorCode":" "})
                else:
                    status = 'failed'
                    UpdateMsg.update({"errorCode":str(error_string+" "+error_desc)})
                #apt发送数据
                if self.window_main.configs_uncover.getWithDefault("SystemStatus", "upload_installer_log", False) == True:
                    
                    UpdateMsg.update({"appname":str(self.window_main.deb_obj.get("debname","None").split("_")[0])})
                    UpdateMsg.update({"source":str(self.window_main.deb_obj.get("source","kylin-system-updater"))})
                    UpdateMsg.update({"status":str(status)})
                    UpdateMsg.update({"new_version":str(self.window_main.deb_obj.get("debname","None").split("_")[1])})
                    UpdateMsg.update({"old_version":str(self.window_main.deb_obj.get("old_version","None"))})
                    self.collector.Upgrade_Process_Msg(self.action, UpdateMsg.copy())
                    self.window_main.deb_obj = {}
        elif action == self.ACTION_BACKGROUND_UPGRADE:
            self._release_install_lock()
            UpdateMsg = {}
            if success:
                status = 'success'
            else:
                status = 'failed'
            self.collector.Generate_Msg(self.collector.background_list, self.action_mode)
            for bl in self.collector.background_list:
                pkg = self.collector.cache[bl]
                UpdateMsg.update({"appname":str(bl)})
                UpdateMsg.update({"source":"Kylin Background upgrade"})
                UpdateMsg.update({"status":str(status)})
                UpdateMsg.update({"errorCode":str(error_string+" "+error_desc)})
                if self.collector.background_version[bl]:
                    UpdateMsg.update({"old_version":self.collector.background_version[bl]})
                UpdateMsg.update({"new_version":self.collector.cache[bl].candidate.source_version})
                # self.collector.Upgrade_Process_Msg(self.action, UpdateMsg.copy())
            
            self.dbus_send.UpdateInstallFinished(success,self.meta_upgrade.get_content(),error_string,error_desc)
    
    def entering_installed(self):
        '''进入安装的状态
            1、禁止关机的锁
            2、关闭安全机制
        '''
        logging.info("The process is now in the installtion phase")
        # self.safe_manager.shutdown_safe()
        self._start_install_lock(_("Kylin System Updater"))

    def exit_installed(self):
        '''退出安装的状态
            1、释放禁止关机的锁
            2、还原安全的配置
        '''
        # self.safe_manager.reset_safe()
        self._release_install_lock()

    def _start_install_lock(self, caller='kylin-system-updater'):
        self.window_main.configs_uncover.setValue("SystemStatus","abnormal_reboot",str(True))
        self.inhibit_shutdown.lock(caller=caller)

    def _release_install_lock(self):
        self.window_main.configs_uncover.setValue("SystemStatus","abnormal_reboot",str(False))
        self.inhibit_shutdown.unlock()

    def _message_to_plymouth(self,message):
        subprocess.call(["/bin/plymouth", "message", "--text", message])

    def _progress_to_plymouth(self,progress):
        tmp = ("--progress=%d"%progress)
        run_cmd = ["/bin/plymouth","system-update",tmp]
        subprocess.call(run_cmd)

    def _update_to_config(self,meta_upgrade,pkgs_install,pkgs_upgrade,pkgs_remove):
        try:
            output_upgrade = {}

            output_upgrade.update({"push_groups":meta_upgrade.upgrade_groups})
            output_upgrade.update({"push_pkgs":meta_upgrade.single_pkgs})
            output_upgrade.update({"pkgs_install":pkgs_install})
            output_upgrade.update({"pkgs_upgrade":pkgs_upgrade})
            output_upgrade.update({"pkgs_remove":pkgs_remove})

            #6 产生JSON文件
            with open(self.INSTALL_OUTPUT_JSON, 'w', encoding='utf-8') as f:
                json.dump(output_upgrade, f, ensure_ascii=False, indent=4)

            logging.info("Generate Jsonfile(%s) to complete... ",self.INSTALL_OUTPUT_JSON)
            logging.info("Update Packages list to config file...")
        except Exception as exc:
            logging.error(exc)

    def _config_to_upgrade(self):
        #读取组JSON文件
        with open(self.INSTALL_OUTPUT_JSON,'r') as f:
            upgrade_groups = []
            single_pkgs = []
            pkgs_install = []
            pkgs_upgrade = []
            pkgs_remove = []

            try:
                data = json.load(f)
                upgrade_groups = data["push_groups"]
                single_pkgs = data["push_pkgs"]
                pkgs_install = data["pkgs_install"]
                pkgs_upgrade = data["pkgs_upgrade"]
                pkgs_remove = data["pkgs_remove"]
            except Exception as exc:
                logging.error(exc)
        
        return NowUpgradeMeta(_upgrade_groups=upgrade_groups,_single_pkgs=single_pkgs),pkgs_install,pkgs_upgrade,pkgs_remove

    def emulate_calcul_broken(self,pkgs):
        args = ["apt-get", "install","--simulate"]
        args = args + pkgs

        p = subprocess.run(args, stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
        # logging.error(str(p.stdout))
        return p.stdout

    def emulate_update(self):
        args = ["apt-get", "update"]

        p = subprocess.run(args, stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
        logging.warning(str(p.stdout))
        # return p.stdout

    def check_free_space(self,cache):
        err_sum = _("Not enough free disk space")
        err_msg = _("The upgrade needs a total of %s free space on "
                    "disk '%s'. "
                    "Please free at least an additional %s of disk "
                    "space on '%s'. %s")
        # specific ways to resolve lack of free space
        remedy_archivedir = _("Remove temporary packages of former "
                                "installations using 'sudo apt clean'.")
        remedy_boot = _("You can remove old kernels using "
                        "'sudo apt autoremove', and you could also "
                        "set COMPRESS=xz in "
                        "/etc/initramfs-tools/initramfs.conf to "
                        "reduce the size of your initramfs.")
        remedy_root = _("Empty your trash and remove temporary "
                        "packages of former installations using "
                        "'sudo apt clean'.")
        remedy_tmp = _("Reboot to clean up files in /tmp.")
        remedy_usr = _("")
        # check free space and error if its not enough
        try:
            cache.checkFreeSpace()
        except NotEnoughFreeSpaceError as e:
            # CheckFreeSpace examines where packages are cached
            archivedir = apt_pkg.config.find_dir("Dir::Cache::archives")
            err_long = ""
            for req in e.free_space_required_list:
                if err_long != "":
                    err_long += " "
                if req.dir == archivedir:
                    err_long += err_msg % (req.size_total, req.dir,
                                            req.size_needed, req.dir,
                                            remedy_archivedir)
                elif req.dir == "/boot":
                    err_long += err_msg % (req.size_total, req.dir,
                                            req.size_needed, req.dir,
                                            remedy_boot)
                elif req.dir == "/":
                    err_long += err_msg % (req.size_total, req.dir,
                                            req.size_needed, req.dir,
                                            remedy_root)
                elif req.dir == "/tmp":
                    err_long += err_msg % (req.size_total, req.dir,
                                            req.size_needed, req.dir,
                                            remedy_tmp)
                elif req.dir == "/usr":
                    err_long += err_msg % (req.size_total, req.dir,
                                            req.size_needed, req.dir,
                                            remedy_usr)
            # #当单包安装检查内存不够的时候，报错到控制面板 单包安装属于更新才使用的所以直接赋值为更新 再进行之后的报错处理
            if self.action_mode == self.MODE_INSTALL_SINGLE:
                self.action = self.ACTION_UPDATE
            raise UpdateBaseError(ERROR_NOT_DISK_SPACE,desc=err_sum)
        except SystemError:
            logging.exception("free space check failed")
        logging.info("Disk Check finished...")

# try aptdaemon
if os.path.exists("/usr/sbin/aptd") \
   and "UPDATE_MANAGER_FORCE_BACKEND_SYNAPTIC" not in os.environ:
    # check if the gtkwidgets are installed as well
    try:
        from .InstallBackendAptdaemon import InstallBackendAptdaemon
    except ImportError:
        logging.exception("importing aptdaemon")

def get_backend(*args, **kwargs):
    """Select and return a package manager backend."""
    # try aptdaemon
    if (os.path.exists("/usr/sbin/aptd")
            and "UPDATE_MANAGER_FORCE_BACKEND_SYNAPTIC" not in os.environ):
        # check if the gtkwidgets are installed as well
        try:
            return InstallBackendAptdaemon(*args, **kwargs)
        except NameError:
            logging.exception("using aptdaemon failed")

    # nothing found, raise
    raise Exception("No working backend found, please try installing "
                    "aptdaemon or synaptic")

class InhibitShutdownLock():
    # 禁止关机锁文件路径
    FILELOCK_PATH = "/tmp/lock/"
    SHUTDOWN_BLOCK_FILELOCK = "kylin-update.lock"
    
    def __init__(self):
        self.inhibit_lock = None
        self.pidfile = None

    #安装时禁止关机 进行加锁
    def lock(self, caller='kylin-system-updater'):
        """
        Send a dbus signal to logind to not suspend the system, it will be
        released when the return value drops out of scope
        """
        try:
            from gi.repository import Gio, GLib
            connection = Gio.bus_get_sync(Gio.BusType.SYSTEM)
            
            var, fdlist = connection.call_with_unix_fd_list_sync(
                'org.freedesktop.login1', '/org/freedesktop/login1',
                'org.freedesktop.login1.Manager', 'Inhibit',
                GLib.Variant('(ssss)',
                            ('shutdown:sleep',
                            caller, _("Installing or removing software packages."),
                            'block')),
                None, 0, -1, None, None)
            self.inhibit_lock = Gio.UnixInputStream(fd=fdlist.steal_fds()[var[0]])

            self.LockedPreventShutdown()
            logging.info("Shutdown Has been locked...")
        except Exception as e:
            logging.error(e)

    #解锁禁止关机 
    def unlock(self):
        try:
            self.unLockedEnableShutdown()
            if self.inhibit_lock != None:
                self.inhibit_lock.close()
                self.inhibit_lock == None
                logging.info("Shutdown Has been unlocked...")
            else:
                logging.info("Not locked and Quitting ...")
        except Exception as e:
            logging.error("unlock failed." + str(e))

    #安装时禁止关机 进行加锁
    def LockedPreventShutdown(self):

        #不为空是表示以及被锁
        if self.pidfile != None:
            logging.error("self.pidfile file disc not is None,Has been locked...")
            return False

        if not os.path.exists(self.FILELOCK_PATH):
            #不存在创建
            logging.info("File(%s) is not exists and will be create",self.FILELOCK_PATH)
            os.makedirs(self.FILELOCK_PATH)
        else:
            #当目录存在时进行删除 不删除进行创建文件的话会报错 
            # file cannot be locked.[Errno 11] Resource temporarily unavailable
            # 资源被占用报错
            shutil.rmtree(self.FILELOCK_PATH)
            logging.info("File(%s) is exists and will be delete and create",self.FILELOCK_PATH)
            os.makedirs(self.FILELOCK_PATH)        

        try:
            self.pidfile = open(os.path.join(self.FILELOCK_PATH, self.SHUTDOWN_BLOCK_FILELOCK), "w+")
            fcntl.flock(self.pidfile, fcntl.LOCK_EX | fcntl.LOCK_NB)
            return True
        except Exception as e:
            logging.error("file cannot be locked." + str(e))
            self.pidfile.close()
            self.pidfile = None
            return False

    #解锁禁止关机 
    def unLockedEnableShutdown(self):
        #未加锁退出
        if not self.pidfile:
            logging.info("Not locked and Quitting ...")
            return False
        try:
            fcntl.flock(self.pidfile, fcntl.LOCK_UN)
            self.pidfile.close()
            self.pidfile = None

            # Fix 修复权限问题 当普通用户无法使用 所以直接删除目录
            if os.path.exists(self.FILELOCK_PATH):
                shutil.rmtree(self.FILELOCK_PATH)
                logging.info('Emptying the lockPath(%s) is complete...',self.FILELOCK_PATH)
            else:
                logging.info("Emptying the lockPath(%s) is Failed...",self.FILELOCK_PATH)

            return True
        except Exception as e:
            logging.error("unlock failed." + str(e))
            self.pidfile.close()
            self.pidfile = None
            return False   

class UpdateSafeManager():
    KYSEC_STATUS = "/sys/kernel/security/kysec/status"
    KYSEC_EXECTL = "/sys/kernel/security/kysec/exectl"

    KYSEC_EXECUT_CONTROL = 0
    KYSEC_NETWORK_CONTROL = 1
    KYSEC_SHUTDOWN_CODE = 0

    def __init__(self):
        self.bus = dbus.SystemBus()
        self.safe_status = False
        self.safe_exectl_code = self.KYSEC_SHUTDOWN_CODE
        self.check_status()

    def check_status(self):
        self._check_safe_status()
        self._check_safe_exectl()

    def _check_safe_status(self):
        if os.path.exists(self.KYSEC_STATUS):
            with open(self.KYSEC_STATUS, 'r') as f:
                data = f.read()
                if data != "0":
                    self.safe_status = True
    
    def _check_safe_exectl(self):
        if self.safe_status == True:
            if os.path.exists(self.KYSEC_EXECTL):
                with open(self.KYSEC_EXECTL, 'r') as f:
                    data = f.read()
                    self.safe_exectl_code = int(data)
                    logging.info("Now kylin Sec has opened and exectl status:%s...",data)

    def reset_safe(self):
        if self.safe_status == True:
            self._set_kysec_status(self.KYSEC_EXECUT_CONTROL,self.safe_exectl_code)
            self._set_kysec_status(self.KYSEC_NETWORK_CONTROL,2)

    def shutdown_safe(self):
        if self.safe_status == True:
            self._set_kysec_status(self.KYSEC_EXECUT_CONTROL,self.KYSEC_SHUTDOWN_CODE)
            self._set_kysec_status(self.KYSEC_NETWORK_CONTROL,self.KYSEC_SHUTDOWN_CODE)

    def _set_kysec_status(self,fun,value):
        try:
            obj = self.bus.get_object('com.kylin.kysec', '/xattr')
            interface = dbus.Interface(obj,dbus_interface='com.kylin.kysec.xattr')
            retval = interface.kysec_xattr_set_func_status(fun,value,timeout=0.5)
            logging.info("Set kysec_xattr_set_func_status %s...",str(value))
            return True
        except Exception as e:
            logging.warning("Set kylin Sec Failed and fun:%d value:%d Error msg:" + str(e),fun,value)
            return False
