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

#!/usr/bin/env python

import gi
import traceback
import logging
import os,subprocess
gi.require_version('OSTree', '1.0')
from gi.repository import GLib, Gio, OSTree,GObject
from SystemUpdater.Core.errors import *
from SystemUpdater.Core.enums import *
from SystemUpdater.configs import settings
from SystemUpdater.constants import OFFLINE_REPO_POST,KYLIN_OS_NAME
from gettext import gettext as _
from SystemUpdater.Core.diskManager import check_free_space
import SystemUpdater.modules.self_upgrade as self_upgrade

RETRY_LIMIT = 5
DEFAULT_REPO_NAME = "kylin"

class BackendBaseOstree(GObject.GObject):    
    __gsignals__ = {
                    "transaction-done": (GObject.SignalFlags.RUN_FIRST,
                                         None,
                                         (GObject.TYPE_PYOBJECT,)),
                    "download-info": (GObject.SIGNAL_RUN_FIRST,
                                GObject.TYPE_NONE,
                                (   GObject.TYPE_INT,
                                    GObject.TYPE_INT,
                                    GObject.TYPE_ULONG,
                                    GObject.TYPE_ULONG,
                                    GObject.TYPE_ULONG
                                )),
                    "deploy-progress": (GObject.SIGNAL_RUN_FIRST,
                                GObject.TYPE_NONE,
                                (   GObject.TYPE_INT,
                                )),
                    }
    
    def __init__(self,parent,sysroot_path=None,osname=KYLIN_OS_NAME):
        GObject.GObject.__init__(self)
        self._async_progress = None
        if sysroot_path:
            sysroot_path = Gio.File.new_for_path(sysroot_path)
            
        self.sysroot = OSTree.Sysroot.new(sysroot_path)
        self.sysroot.set_mount_namespace_in_use()
        self.sysroot.initialize()
        self.sysroot.load()

        booted_deployment = self.sysroot.get_booted_deployment()
        if booted_deployment:
            self.osname = booted_deployment.get_osname()
        else:
            logging.warning("Booted deployment is None and use to defalut os name(%s)...",osname)
            self.osname = osname

        self.cur_deployment = self.sysroot.get_merge_deployment(self.osname)
        if not self.cur_deployment:
            logging.error("Now system deployment is None...")
            
        self.origin = self.cur_deployment.get_origin()

        # 加载插件机制
        # 获取当前运行程序的插件目录位置
        # self.plugins_dir = os.path.join(os.getcwd(),"SystemUpdater/plugins")
        # # 基础插件 所有插件的位置
        # test = os.path.join(self.plugins_dir,"base")
        # self.plugin_base = PluginBase(package='SystemUpdater.plugins',searchpath=[os.path.join(self.plugins_dir,"base")])

    def run(self,trans=None):
        logging.info("Processing transaction")
        # self._run_transaction_idle(trans)
        GLib.idle_add(self._run_transaction_idle,trans)

    def _run_transaction_idle(self,trans):
        """Run the transaction"""
        try:
            self._async_progress = OSTree.AsyncProgress.new()
            self._async_progress.connect("changed",self._on_async_progress)
            self._try_lock()
            self._run_transaction(self.sysroot,trans)
        except GLib.Error as error:
            logging.error(str(error))
            trans.success = False
            self._parse_ostree_error(error,trans)
        except UpdateBaseError as excep:
            trans.success = False
            trans.error = excep.code
        except Exception as excep:
            logging.error(str(excep))
            trans.success = False

            tbk = traceback.format_exc()
            logging.error(str(tbk))
        else:
            trans.success = True
        
        finally:
            self._try_unlock()
            self._async_progress.finish()
            logging.info("Finished transaction")

            self._emit_transaction_done(trans)

        return False

    def _run_transaction(self,sysroot,trans):
        """This method needs to be implemented by the backends."""
        pass
        # raise errors.TransactionFailed(enums.ERROR_NOT_SUPPORTED)

    def _try_lock(self):
        ret,out_acquired = self.sysroot.try_lock()
        if not out_acquired:
            raise UpdateBaseError(ERROR_OTHER_PROGRESS_OCCUPATION)
    
    def _try_unlock(self):
        self.sysroot.unlock()

    def _emit_transaction_done(self,trans):
        """Emit the transaction-done signal.

        Keyword argument:
        trans -- the finished transaction
        """
        # logging.debug("Emitting transaction-done: %d",)
        self.emit("transaction-done",trans)

    def _on_async_progress(self,obj):
        try:
            start_time = obj.get_uint64("start-time")
            bytes_transferred = obj.get_uint64("bytes-transferred") 

            fetched = obj.get_uint("fetched")
            requested = obj.get_uint("requested")

            # 下载速度
            elapsed_secs=(GLib.get_monotonic_time()-start_time)/GLib.USEC_PER_SEC
            bytes_sec = int(bytes_transferred / elapsed_secs)

            if requested <= 2:
                return
            
            self.emit("download-info",fetched,requested,bytes_transferred,0,bytes_sec)
        except Exception as excep:
            tbk = traceback.format_exc()
            logging.error(str(tbk))

    def _parse_ostree_error(self,error,trans):
        if trans.role == ROLE_DEPLOY:
            trans.error = ERROR_NOT_DEPLOYMENT
        else:
            if error.code in (0,37,39):
                trans.error = ERROR_NETWORK_FAILED
            #取消
            elif error.code == 19:
                trans.cancellable.reset()
                trans.error = EXIT_CANCELLED
            elif error.code == 1:
                # 在远程分支 未找到 对应的 分支 
                if "Server returned status 404" in error.message:
                    trans.error = ERROR_PUSH_BRANCH_EXCEPTION
                # 未找到 对应的仓库信息 
                elif "Remote" in error.message and "not found" in error.message:
                    trans.success = True
    
    def _open_offline_repo(self):
        # 检查是否存在离线仓库
        offline_flag = OFFLINE_REPO_POST + "/config"
        if os.path.exists(offline_flag):
            g_file= Gio.File.new_for_path(OFFLINE_REPO_POST)
            offline_repo = OSTree.Repo.new(g_file)
            ret = offline_repo.open()
            return offline_repo
        else:
            return None

    def register_formatter(self, name, formatter):
        """A function a plugin can use to register a formatter."""
        self.formatters[name] = formatter

    def _load_plugins(self,identifier):
        # 每次formatters将被刷新
        self.formatters = {}
        plugin_dir = os.path.join(self.plugins_dir,identifier)
        plugins_sources = self.plugin_base.make_plugin_source(searchpath=[plugin_dir],identifier=identifier)

        # 初始化插件
        for plugin_name in plugins_sources.list_plugins():
            plugin = plugins_sources.load_plugin(plugin_name)
            plugin.setup(self)
        return self.formatters

class UpdateBackend(BackendBaseOstree):
    def __init__(self,parent,sysroot_path=None,osname=None):
        """Initialize a new AptWorker instance."""
        BackendBaseOstree.__init__(self,parent,sysroot_path,osname)
        
        #更新前插件
        # self.update_formatters = self._load_plugins("update")

    def _run_transaction(self, sysroot,trans):
        g_options = { 
                        "n-network-retries":GLib.Variant("u",RETRY_LIMIT),\
                        "flags":GLib.Variant("i",OSTree.RepoPullFlags.COMMIT_ONLY),\
                        "max-speed-limit-kilobyte": GLib.Variant("u",int(os.environ.get("DOWNLOAD_SPEED_LIMIT",0))),\
                        "depth":GLib.Variant("i",1)
                    }
        push_revision = None
        ostree_repo = sysroot.repo()

        # 获取当前部署分支的checksum与当前部署的分支
        trans.from_revision = self.cur_deployment.get_csum()
        refspec = self.origin.get_string("origin","refspec")
        ret,trans.available_repo,trans.deployed_refs = OSTree.parse_refspec(refspec)
        if not trans.available_repo:
            logging.error("Now system deployment repo name is None...")
            raise UpdateBaseError(ERROR_REPO_IS_NONE)

        logging.info("Now system deployment info(%s:%s@%s)...",trans.available_repo,trans.deployed_refs,trans.from_revision)

        # 加载推送更新 当推送为空时 直接去服务端 拉取当前分支最新的commit
        if trans.push_list:
            for push in trans.push_list:
                trans.available_repo = push.repo_name
                trans.available_refs = push.branch
                # 指定拉取的commit id，不指定时拉取当前分支最新的
                if push.revision:
                    g_options["override-commit-ids"] = GLib.Variant("as",[push.revision])
                    push_revision = push.revision
                break
        else:
        # 未推送情况下 获取当前部署的分支
            trans.available_refs = trans.deployed_refs
                
        g_options["refs"] = GLib.Variant("as",[trans.available_refs])
        logging.info("Start pull metadata and options:%s...",g_options)
        ostree_repo.pull_with_options(trans.available_repo,GLib.Variant("a{sv}",g_options),self._async_progress,trans.cancellable)

        # 没有指定commit情况下 检索当前分支最新commit
        if not push_revision:
            ret,trans.new_revision = ostree_repo.resolve_rev(trans.available_repo+":"+trans.available_refs,True)
        else:
            trans.new_revision = push_revision

        ret,metadata = ostree_repo.load_variant(OSTree.ObjectType.COMMIT,trans.new_revision)
        trans.update_metadata = metadata[0]

        # 检查是否当前分支已经终止，切换到新的分支
        if OSTree.COMMIT_META_KEY_ENDOFLIFE_REBASE in trans.update_metadata and not trans.push_list:
            # 拉取新分支的元数据
            new_ref = trans.update_metadata[OSTree.COMMIT_META_KEY_ENDOFLIFE_REBASE]
            logging.info("From origin ref:%s To new refs:%s...",trans.available_refs,new_ref)

            trans.available_refs = new_ref
            logging.info("Start pull new metadata refs:%s...",trans.available_refs)
            g_options["refs"] = GLib.Variant("as",[trans.available_refs])
            ret = ostree_repo.pull_with_options(trans.available_repo,GLib.Variant("a{sv}",g_options),self._async_progress,trans.cancellable)
            
            ret,trans.new_revision = ostree_repo.resolve_rev(trans.available_repo+":"+trans.available_refs,True)
            
        ret,metadata = ostree_repo.load_variant(OSTree.ObjectType.COMMIT,trans.new_revision)
        trans.update_metadata = metadata[0]

        # 执行优选升级流程
        if trans.push_list and settings.get("enable_self_update",True) and \
                        self_upgrade.run_priority_upgrade(ostree_repo,trans):
            raise UpdateBaseError(PRIORITY_UPGRADE_SUCCCESSED)

    def _get_commit_recursive(self,ostree_repo,checksum):
        # 如果 start 不在 all_deltas 中，返回空列表
        ret,metadata = ostree_repo.load_variant(OSTree.ObjectType.COMMIT,checksum)
        if not metadata:
            return []
        parent = OSTree.commit_get_parent(metadata)
        if parent:
            return [parent] + self._get_commit_recursive(ostree_repo, parent)
        else:
            return []

    def _get_all_commit(self,ostree_repo):
        commit_and_refs = {}
        ret,out_all_refs = ostree_repo.list_refs(None,None)
        
        for refs in out_all_refs:
            commit_list = self._get_commit_recursive(ostree_repo,out_all_refs[refs])
            for commit in commit_list:
                commit_and_refs[commit] = refs

        logging.info(commit_and_refs)

class DownloadBackend(BackendBaseOstree):
    def __init__(self,parent,sysroot_path=None,osname=None):
        """Initialize a new AptWorker instance."""
        BackendBaseOstree.__init__(self,parent,sysroot_path,osname)
        self.download_size = 0

        self.g_options = {
                            "n-network-retries":GLib.Variant("u",RETRY_LIMIT),\
                            # "timestamp-check":GLib.Variant("b",True),\
                            # "low-speed-limit-bytes":GLib.Variant("u",10),\
                            "max-speed-limit-kilobyte": GLib.Variant("u",int(os.environ.get("DOWNLOAD_SPEED_LIMIT",0))),\
                            "flags":GLib.Variant("i",OSTree.RepoPullFlags.NONE),\
                            "depth":GLib.Variant("i",1)
                        }

    def _get_offline_deltas(self):
        all_deltas = []
        to_refs = []
        offline_repo = self._open_offline_repo()
        if offline_repo:
            all_deltas = {}
            ret,deltas_list = offline_repo.list_static_delta_names(None)
            logging.info("Offline Repo(%s) has delta files:%r",OFFLINE_REPO_POST,deltas_list)
            for delta_info in deltas_list:
                deltas_splite = delta_info.split("-")
                from_rev = deltas_splite[0]
                to_rev = deltas_splite[1]
                all_deltas[from_rev] = to_rev

                # 解析
                if len(deltas_splite) == 3:
                    refs_repo = deltas_splite[2]
                    if ":" in refs_repo:
                        to_refs[to_rev] = refs_repo.split(":")[1]
                    else:
                        to_refs[to_rev] = refs_repo

        return all_deltas,to_refs

    def _run_transaction(self, sysroot,trans):
        ostree_repo = sysroot.repo()
        
        remote_repo = trans.available_repo
        if trans.update_metadata:
            self.download_size = int(trans.update_metadata.get("download-size",0))

        check_free_space(self.download_size)

        self.g_options["refs"] = GLib.Variant("as",[trans.available_refs])

        all_deltas,to_refs = self._get_offline_deltas()
        if all_deltas:
            pull_deltas = self._get_deltas_recursive(trans.from_revision,all_deltas)
            self.g_options["require-static-deltas"] = GLib.Variant("b",True)

            # 循环拉取所有的 deltas
            for delta in pull_deltas:
                if delta in to_refs:
                    pull_ref = to_refs[delta]
                    self.g_options["refs"] = GLib.Variant("as",[pull_ref])
                self.g_options["override-commit-ids"] = GLib.Variant("as",[delta])
                logging.info("Start pull data and options:%s...",self.g_options)
                ostree_repo.pull_with_options(remote_repo,GLib.Variant("a{sv}",self.g_options),self._async_progress,trans.cancellable)
        else:
            self.g_options["override-commit-ids"] = GLib.Variant("as",[trans.new_revision])
            logging.info("Start pull data and options:%s...",self.g_options)
            ostree_repo.pull_with_options(remote_repo,GLib.Variant("a{sv}",self.g_options),self._async_progress,trans.cancellable)

        # 下载完成后检查下载的元数据的时间戳
        # upgrader.check_timestamps(ostree_repo,from_revision,trans.new_revision)
    
    def _get_deltas_recursive(self,start, all_deltas):
        # 如果 start 不在 all_deltas 中，返回空列表
        if start not in all_deltas:
            return []
        # 获取下一个值并递归处理
        next_value = all_deltas[start]
        return [next_value] + self._get_deltas_recursive(next_value, all_deltas)

    def _on_async_progress(self,obj):
        try:
            start_time = obj.get_uint64("start-time")
            bytes_transferred = obj.get_uint64("bytes-transferred") 

            fetched = obj.get_uint("fetched")
            requested = obj.get_uint("requested")

            # 下载速度
            elapsed_secs=(GLib.get_monotonic_time()-start_time)/GLib.USEC_PER_SEC
            bytes_sec = int(bytes_transferred / elapsed_secs)

            if requested <= 2:
                return
            
            self.emit("download-info",fetched,requested,bytes_transferred,self.download_size,bytes_sec)
        except Exception as excep:
            tbk = traceback.format_exc()
            logging.error(str(tbk))


class DeployBackend(BackendBaseOstree):
    def __init__(self,parent,sysroot_path=None,osname=None):
        """Initialize a new AptWorker instance."""
        BackendBaseOstree.__init__(self,parent,sysroot_path,osname)
        
    def _deployment_sanitycheck_true(self,rootfs_path):
        try:
            pid = os.fork()
            if pid == 0:
                os.chroot(rootfs_path)
                os.chdir("/")
                # 执行命令
                result = subprocess.run(["/usr/bin/true"],capture_output=True, text=True)
                os._exit(result.returncode)
            else:
                # 等待子进程结束
                _, exit_code = os.waitpid(pid, 0)
                logging.info("Command exited with status: %d",os.WEXITSTATUS(exit_code))
                if os.WIFEXITED(exit_code) != 0:
                    logging.error("Command did not exit normally.")
        except Exception as e:
            logging.error(str(e))

    def _on_async_progress(self,obj):
        try:
            current_progress = obj.get_uint("current-progress")
            self.emit("deploy-progress",int(current_progress))
        except Exception as excep:
            tbk = traceback.format_exc()
            logging.error(str(excep)+str(tbk))

    def _run_transaction(self, sysroot,trans):
        ostree_repo = sysroot.repo()

        new_revision = trans.new_revision
        refs = trans.available_refs

        # 获取当前origin文件
        remote_repo = trans.available_repo

        # 修改origin 为当前部署新分支
        origin_refspec = remote_repo+":"+refs
        # 创建新的origin文件
        deploy_origin = sysroot.origin_new_from_refspec(origin_refspec)
        # 进行部署时 保留历史部署
        flags = OSTree.SysrootSimpleWriteDeploymentFlags.RETAIN
        logging.info("Start deploy and available_refs:%s new_revision:%s...",refs,new_revision)
        try:
            sysroot.prepare_cleanup(self._async_progress,None)
            ret,upgrade_deployment = sysroot.deploy_tree(self._async_progress,self.osname,new_revision,deploy_origin,self.cur_deployment,None,None)
            sysroot.simple_write_deployment(self._async_progress,self.osname,upgrade_deployment,self.cur_deployment,flags,None)
        except TypeError as e:
            logging.info("Switch to using APIs from the previous of OStree and error:%s",str(e))
            self.emit("deploy-progress",20)
            sysroot.prepare_cleanup(None)
            self.emit("deploy-progress",30)
            ret,upgrade_deployment = sysroot.deploy_tree(self.osname,new_revision,deploy_origin,self.cur_deployment,None,None)
            self.emit("deploy-progress",50)
            sysroot.simple_write_deployment(self.osname,upgrade_deployment,self.cur_deployment,flags,None)
        
        self.emit("deploy-progress",100)
        ret,metadata = ostree_repo.load_variant(OSTree.ObjectType.COMMIT,new_revision) 

        trans.new_revision = new_revision
        trans.update_metadata = metadata[0]

class RollbackBackend(BackendBaseOstree):
    def __init__(self,parent,sysroot_path=None,osname=None):
        """Initialize a new AptWorker instance."""
        BackendBaseOstree.__init__(self,parent,sysroot_path,osname)

    def _run_transaction(self, sysroot,trans):
        new_deployments = []

        out_pending,out_rollback = sysroot.query_deployments_for(self.osname)

        if out_rollback == None:
            raise UpdateBaseError(ERROR_NOT_ROLLBAK_DEPLOYMENT)

        rollback_csum = out_rollback.get_csum()

        old_deployments = sysroot.get_deployments()

        new_deployments.append(out_rollback)

        # 部署的列表
        for deploys in old_deployments:
            if not out_rollback.equal(deploys):
                new_deployments.append(deploys)

        logging.info("start reollback checksum:%s...",rollback_csum)
        ret = sysroot.write_deployments(new_deployments,None)

        origin = out_rollback.get_origin()
        refspec = origin.get_string("origin","refspec")
        ret,remote_repo,origin_ref = OSTree.parse_refspec(refspec)

        trans.available_refs = origin_ref

class LoadCacheBackend(BackendBaseOstree):
    def __init__(self,parent,sysroot_path=None,osname=None):
        """Initialize a new AptWorker instance."""
        BackendBaseOstree.__init__(self,parent,sysroot_path,osname)

    def _run_transaction(self, sysroot,trans):
        trans.deployed_refs = self._get_refs()
        trans.rollback_deployment = self._rollback_deployment(sysroot,self.osname)
        trans.revision = self.cur_deployment.get_csum()

    def _get_refs(self):
        refspec = self.origin.get_string("origin","refspec")
        ret,remote_repo,origin_ref = OSTree.parse_refspec(refspec)
        if origin_ref == None:
            origin_ref = ''
        return origin_ref
    
    def _rollback_deployment(self,sysroot,osname):
        out_pending,out_rollback = sysroot.query_deployments_for(osname)
        
        if out_rollback and not out_rollback.is_pinned():
            rollback_origin = out_rollback.get_origin()
            refspec = rollback_origin.get_string("origin","refspec")
            ret,remote_repo,origin_ref = OSTree.parse_refspec(refspec)
            
            return [origin_ref]
        else:
            return []