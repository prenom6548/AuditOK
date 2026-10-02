#!/usr/bin/python3
# -*- coding: UTF-8 -*-

import logging
import os
from gettext import gettext as _
from enum import IntEnum
from time import sleep
import shutil
import subprocess
import dbus
import copy
import threading
from datetime import datetime

from RevisionsManager.Core.RMConf import RMConf
from RevisionsManager.Core.RMGrub import RMGrub
from RevisionsManager.Core.RMMoveEtc import RMMoveEtc
from RevisionsManager.Core.RMApp import RMApp
from RevisionsManager.Core.RMCom import RMCom, RMSUCCESS, RMFAILURE, get_sys_call, str_to_list, update_grub
from RevisionsManager.Core.RMScript import RMScriptDir


# 升级前检测项
UCHandlerIterms=(_("Please prepare more than 12G space in the root partition"),
                _("Please use right archive ISO"))

UCIterms=(  _("Check root free space"),
            _("Check iso right"))

class UCItermsEnum(IntEnum):
    UCRootFree = 0
    UCIsoRight = 1
    UCItermsMax = 2

# 升级项
UPHandlerIterms=(_("Please run \"sudo update-initramfs -u\", and try again."),
                _("Please reboot, and try again"),
                "",
                _("Please reboot, and try again"),
                _("Please reboot, and try again"),
                _("Please reboot, and try again"))

UPIterms=(  _("Create orginal system initrd"),
            _("Create new system"),
            _("Create new applications deb"),
            _("Move etc"),
            _("Create new system initrd"),
            _("Create new system grub"))

class UPItermsEnum(IntEnum):
    UPCreateOrgFsInitrd = 0
    UPCreateNewFs = 1
    UPCreateNewDeb = 2
    UPMoveEtc = 3
    UPCreateNewFsInitrd = 4
    UPCreateNewGrub = 5
    UPItermsMax = 6

# 还原前检测项
FCHandlerIterms=(_("Please check whether the old os exists"),
                _("Please check whether the old initrd exists"))
FCIterms=(  _("Check old os"),
            _("Check org Initrd"))
class FCItermsEnum(IntEnum):
    FCCheckOldOs = 0
    FCCheckOrgInitrd = 1
    FCItermsMax = 2

# 还原项
FPHandlerIterms=(_("Please reboot, and try again"))
FPIterms=(_("Create Fallback Grub"),)
class FPItermsEnum(IntEnum):
    FPCreateGrub = 0
    FPItermsMax = 1

# iso挂载 ok
# isoPath 绝对路径
# return 挂载路径
def mount_iso(isoPath):
    if not isoPath.startswith("/"):
        logging.error("[mount_iso]: isoPath[%s] is not abs path.", isoPath)
        return None
    logging.info("[mount_iso]: mount start")

    rmNewFs="/media/RevisionsManagerNewFs"
    # 挂载路径是否存在
    flag = False # 是否需要挂载
    for i in range(0,10):
        rmNewFs_i = rmNewFs + str(i)
        logging.debug("[mount_iso]: folder[%s] will mount", rmNewFs_i)
        if not os.path.exists(rmNewFs_i):
            os.makedirs(rmNewFs_i, mode=0o755)
            flag = True
            break
        else:
            cmd = "mount | grep " + rmNewFs_i
            mountinfo = get_sys_call(cmd)
            # TODO: 认为 mountinfo 只有一行信息
            if isoPath in mountinfo.split(" "):
                # 已经挂载
                logging.info("[mount_iso]: folder[%s] has been mounted", rmNewFs_i)
                return rmNewFs_i

    if not flag:
        logging.error("[mount_iso]: folder %s(0-10) is in use , please release one of them",rmNewFs)
        return None
    
    cmd = "mount " + isoPath + " " + rmNewFs_i
    logging.info("[mount_iso]: cmd:%s", cmd)
    cmdRet = subprocess.run(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    mountedStr = "already mounted on"
    mountedStrLen = len(mountedStr)
    cmdRetEnd=len(cmdRet.stderr)
    errInfo = cmdRet.stderr.decode()
    # 挂载是失败处理
    if cmdRet.returncode != RMSUCCESS:
        indexTmp = errInfo.find(mountedStr)
        # iso已挂载
        if -1 != indexTmp:
            indexPath = indexTmp + mountedStrLen + 1
            rmNewFs_i = errInfo[indexPath : cmdRetEnd-2]
            logging.info("[mount_iso]: iso[%s] has been mounted on [%s]",isoPath, rmNewFs_i)
        else:
            logging.error("[mount_iso]: iso[%s] mount [%s] failed. Because [%s]", isoPath, rmNewFs_i, errInfo)
            return None

    logging.info("[mount_iso]: mount end")
    return rmNewFs_i

# 解除挂载 ok
# mountPath 挂载目录
# rmMountPathFlag 是否删除挂载目录
def umount_iso(mountPath, rmMountPathFlag=True):
    cmd = "umount " + mountPath      
    try:
        os.system(cmd)
        if rmMountPathFlag:
            os.rmdir(mountPath)
    except Exception as e:
        logging.error("[umount_iso]: cmd[%s] failed as [%s]", cmd, e)

def handle_depend():
    depends_shell = "/usr/share/revisions-manager/RevisionsManager/scripts/handle_base_depend.sh"
    if not os.path.exists(depends_shell):
        logging.error("Lost depends shell.")
        return
    cmd = "/bin/bash " + depends_shell + " > /var/log/RevisionsManager/depends.log"
    ret = subprocess.run(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if ret.returncode != 0:
        logging.error("Install depends out[%s] failed [%s].",ret.stdout.decode(), ret.stderr.decode())
        return
    
    logging.info("Install depends success.")

class UPPostItermsEnum():
    POSTINSTALL_STEP_OSCONFIG = 0       # 重启后配置
    POSTINSTALL_STEP_SECCONFIG = 1      # 安全迁移
    POSTINSTALL_STEP_THIRDDEB = 2       # 第三方应用安装
    POSTINSTALL_STEP_REMOVE = 3         # 移除临时文件
    POSTINSTALL_STEP_GRUB = 4           # 更新grub
    POSTINSTALL_STEP_INITRD = 5         # 更新initrd
    POSTINSTALL_STEP_NEWSTATE = 6       # 更新升级状态、时间
    POSTINSTALL_STEP_MAX = 7

    _stepToName = {
        POSTINSTALL_STEP_OSCONFIG: _('os config'),
        POSTINSTALL_STEP_SECCONFIG: _('move security config'),
        POSTINSTALL_STEP_THIRDDEB: _('install third party packages'),
        POSTINSTALL_STEP_REMOVE: _('remove temp files'),
        POSTINSTALL_STEP_INITRD: _('update initrd'),
        POSTINSTALL_STEP_GRUB: _('update grub'),
        POSTINSTALL_STEP_NEWSTATE: _('update upgrade state'),
    }
    _nameTostep = {
        _('os config'):POSTINSTALL_STEP_OSCONFIG,
        _('move security config'):POSTINSTALL_STEP_SECCONFIG,
        _('install third party packages'):POSTINSTALL_STEP_THIRDDEB,
        _('remove temp files'):POSTINSTALL_STEP_REMOVE,
        _('update initrd'):POSTINSTALL_STEP_INITRD,
        _('update grub'):POSTINSTALL_STEP_GRUB,
        _('update upgrade state'):POSTINSTALL_STEP_NEWSTATE,
    }

class RMVersionManager(RMCom, UPPostItermsEnum):
    POST_UPPROCESS_LIST = [0,30,40,70,80,85,98,100]  # 升级后处理进度划分 7
    UP_PROCESS_LIST = [0, 6, 30, 70, 75, 90, 100]    # 升级进度划分 6

    # 初始化 
    def __init__(self):
        self.upCheckFlag = RMFAILURE        # 升级前检测状态
        self.backCheckFlag = RMFAILURE      # 还原前检测状态
        self.arch = get_sys_call("uname -m").strip("\n")
        self.curVersion = get_sys_call("uname -r").strip("\n")
        self.users = get_sys_call("ls -ln /home/ | awk  '{if($3>=1000){printf \"%s \", $NF}}'")
        self.newVersion = None # 从新系统iso中获取
        self.oldVersion = None # 升级的系统记录

        if not os.path.exists(self.ETC_DIR):
            os.makedirs(self.ETC_DIR, mode=0o755)
        self._final_fs_name()

        self.rmConf = RMConf(self.CONF_PATH)  # 配置文件中开关
        self.appClass = RMApp()

        self.rmStateConf = RMConf(self.RM_STATE_PATH)
        self.rmStateConf.change_key_value(self.SEC_RUNNING_OS_INFO, self.KEY_KERNEL, self.curVersion)
        self.rmStateConf.change_key_value(self.SEC_HW_INFO, self.KEY_ARCH, self.arch)
        self.rmStateConf.change_key_value(self.SEC_HOME_INFO, self.KEY_USERS, self.users)

        self.thread = threading.Thread(target=self._state_handler)
        self.thread.start()

        self.appClass.find_repack_list()

        logging.info("[VM]: init finished")
    
    def _state_handler(self):
        logging.info("[VM]: state handle start")
        self.versionManagerState = self.rmStateConf.get_key(self.SEC_STATE, self.KEY_VERSIONMANAGER)
        vmStateInt = self.strToInt.get(self.versionManagerState)
        if vmStateInt == None:
            vmStateInt = self.VMINIT
            self.versionManagerState = self.intToStr.get(self.VMINIT)
            self.rmStateConf.change_key_value(self.SEC_STATE,self.KEY_VERSIONMANAGER, self.versionManagerState)
        
        if vmStateInt >= self.VMUP and vmStateInt <= self.VMUPFAILED:
                self.oldVersion = self.rmStateConf.get_key(self.SEC_OLD_OS_INFO,self.KEY_KERNEL)
    
        if vmStateInt == self.VMUPACT2:
            self._up_post_handle()
        elif vmStateInt == self.VMBACKACT2:
            self._back_post_handle()

        forcePath = os.path.join(self.ETC_DIR,"forceback")
        if os.path.exists(forcePath):
            logging.info("[VM-BACK]: post handle for force-back")
            os.remove(forcePath)
            update_grub()

        handle_depend()

        logging.info("[VM]: state handle end")
        return RMSUCCESS

    # 升级第二部分 进度显示函数
    def _update_process(self, index, hasProcess = False, programClass = None):
        stepName = ""
        stepProcess = 0 # 模块的实时进度
        process = 0 

        if index < 0 or index >= self.POSTINSTALL_STEP_MAX:
            logging.error("[VM-UP][process]: index[%d] is err.", index)

        else:
            try:
                # 外部程序 支持上报实时进度
                if hasProcess and programClass is not None:
                    stepName, stepProcess = programClass.get_processState()
                    process = int(self.POST_UPPROCESS_LIST[index] + (self.POST_UPPROCESS_LIST[index + 1] - self.POST_UPPROCESS_LIST[index]) * stepProcess / 100)
                    self.postProcessLastIndex = index
                # 模拟进度
                else:
                    stepName = self._stepToName.get(index)

                    if self.postProcessLastIndex == index:
                        self.postProcessCount += 1
                        process = self.POST_UPPROCESS_LIST[index] + self.postProcessCount
                        if process >= self.POST_UPPROCESS_LIST[index+1]:
                            process = self.POST_UPPROCESS_LIST[index+1]
                    elif self.postProcessLastIndex > index:
                        return
                    else:
                        # 更新
                        self.postProcessLastIndex = index
                        process = self.POST_UPPROCESS_LIST[index]
                        self.postProcessCount = 0
            except Exception as e:
                logging.error("[VM-UP][process]: get process state err[%s].", e)

        
        logging.info("[VM-UP][process]: update name[%s] stepProcess[%d] process[%d].", stepName, stepProcess, process)
        self.up2ProcessCb(stepName, process)

        if stepProcess >= 100 or stepProcess < 0:
            return RMSUCCESS

        if process >= 100 or process < 0:
            return RMSUCCESS

        timerTmp = threading.Timer(2, function=self._update_process, args=[index, hasProcess, programClass])
        timerTmp.start()

    # 升级第二部分处理 -- 第一次重启
    def _up_post_handle(self):
        logging.error("[VM-UP]: up post handle start.")
        self.postProcess = 0

        try:
            postInstallClass = RMScriptDir(self.SCRIPT_POSTISNTALL, os.path.join(self.SCRIPTS_DIR, self.SCRIPT_POSTISNTALL))
            timera = threading.Timer(1, function=self._update_process,args=[self.POSTINSTALL_STEP_OSCONFIG,True,postInstallClass])
            timera.start()
            postInstallClass.run_dir()
            
            # securityClass = RMScriptDir(self.SCRIPT_SECURITY, os.path.join(self.SCRIPTS_DIR, self.SCRIPT_SECURITY))
            # timerb = threading.Timer(0.5, function=self._update_process,args=[self.POSTINSTALL_STEP_SECCONFIG,True,securityClass])
            # timerb.start()
            # securityClass.run_dir()

            # 安装第三方软件包
            # 在ISO 自带包安装之后，防止第三方包安装失败导致都不可以安装
            timerc = threading.Timer(0.5, function=self._update_process,args=[self.POSTINSTALL_STEP_THIRDDEB])
            timerc.start()
            self.appClass.install_third_package()

            # 2-清除多余的标志文件
            timerc = threading.Timer(0.1, function=self._update_process,args=[self.POSTINSTALL_STEP_REMOVE])
            timerc.start()
            os.system("rm -f /mv*Ok")
            if os.path.exists("/cdrom"):
                logging.info("[VM-UP]: remove /cdrom.")
                shutil.rmtree("/cdrom")
            initramfsPath = "/run/initramfs/initramfs.debug"
            if os.path.exists(initramfsPath):
                cmd = "cp " + initramfsPath + " /var/log/RevisionsManager/"
                os.system(cmd)

            # 3-更新grub
            timerc = threading.Timer(0.2, function=self._update_process,args=[self.POSTINSTALL_STEP_GRUB])
            timerc.start()
            ret = update_grub()
            if ret != RMSUCCESS:
                logging.error("[VM-UP] update grub failed.")
                self.rmStateConf.change_key_value(self.SEC_STATE,self.KEY_VERSIONMANAGER, self.intToStr.get(self.VMUPFAILED))

            # 4-更新initrd
            timerc = threading.Timer(0.2, function=self._update_process,args=[self.POSTINSTALL_STEP_INITRD])
            timerc.start()
            logging.info("[VM-UP]: update initrd.")
            os.system("update-initramfs -u -k " + self.curVersion)

            # 5-写入升级成功状态
            timerc = threading.Timer(0.1, function=self._update_process,args=[self.POSTINSTALL_STEP_NEWSTATE])
            timerc.start()

            # 6-写入升级成功的时间
            uptime = datetime.now()
            timeStr = datetime.strftime(uptime,"%Y-%m-%d %H:%M:%S")
            self.rmStateConf.change_key_value(self.SEC_TIME, self.KEY_UPTIEM, timeStr)

            # 7-启动兼容环境
            buildEnvPath = "/usr/bin/buildCompatibilityEnv.sh"
            if os.path.exists(buildEnvPath):
                cmd = "bash " + buildEnvPath
                get_sys_call(cmd)
            else:
                logging.error("[VM-UP]: build docker env shell not exists.")

            logging.info("[VM-UP]: post handle finished, will reboot.")
            self.up2ProcessCb(self._stepToName.get(self.POSTINSTALL_STEP_NEWSTATE), 100)

            self.rmStateConf.change_key_value(self.SEC_STATE,self.KEY_VERSIONMANAGER, self.intToStr.get(self.VMUPSUCCESS))
        except Exception as e:
            logging.error("[VM-UP]: post handle failed[%s].",e)
            self.rmStateConf.change_key_value(self.SEC_STATE,self.KEY_VERSIONMANAGER, self.intToStr.get(self.VMUPFAILED))
        
        # 7-重启
        os.system("reboot")


    def reg_up2_process_func(self, func):
        self.up2ProcessCb = func
    def reg_up_process_func(self, func):
        self.upProcessCb = func
    def reg_upCheck_process_func(self, func):
        self.upCheckProcessCb = func
    def reg_back_process_func(self, func):
        self.backProcessCb = func
    def reg_backCheck_process_func(self, func):
        self.backCheckProcessCb = func


    ############# 升级 #############

    # 检查架构 arch x86
    def _check_iso_arch(self, path):
        efiPath = "/dists/"
        distPath = path + efiPath
        if not os.path.exists(distPath):
            logging.error("[VM-UP]: There is path [%s].", distPath)
            return RMFAILURE
        cmd = "find " + distPath + " -name \"binary-*\""
        ret = get_sys_call(cmd)
        fileInfoList = str_to_list(ret, "\n")
        if len(fileInfoList) == 0:
            logging.error("[VM-UP]: Path [%s] no file.", distPath)
            return RMFAILURE

        archList = []
        for eleList in self.ARCHLIST:
            if self.arch in eleList:
                archList = copy.deepcopy(eleList)
                break
        if len(archList) == 0:
            logging.error("[VM-UP]: not find self arch[%s].", self.arch)
            return RMFAILURE

        for fileInfo in fileInfoList:
            tmpInfoList = fileInfo.split("binary-")
            if len(tmpInfoList) < 2:
                logging.debug("[VM-UP]: Info [%s] not right.", fileInfo)
                continue

            tmpInfo = tmpInfoList[1]

            for ele in archList:
                if ele in tmpInfo:
                    logging.info("[VM-UP]: info [%s]and[%s] is the same", tmpInfo , self.arch)
                    return RMSUCCESS

        logging.info("[VM-UP]: arch [%s]and[%s] is not the same", tmpInfo , self.arch)
        return RMFAILURE

    # 检测ISO正确
    def _check_iso_right(self, isoPath):
        ret = RMSUCCESS        
        if not os.path.exists(isoPath):
            logging.error("[VM-UP]: iso[%s] not exist.", isoPath)
            return RMFAILURE

        mountPath = mount_iso(isoPath)
        if mountPath is None:
            logging.error("[VM-UP]: Mount [%s] failed.", isoPath)
            return RMFAILURE

        ret = self._check_iso_arch(mountPath)

        umount_iso(mountPath)
        logging.info("[VM-UP]: Check iso is ret[%d].", ret)
        return ret

    # 检测根分区空间 ok
    # TODO: 需要空间的阈值 12*SPACE_G 再确认
    def _check_root_free(self):
        sv = os.statvfs("/")
        freeSpace = sv.f_bavail * sv.f_bsize
        if freeSpace < 12*self.SPACE_G:
            logging.warning(_("root free space[%d] is not enough [12G]."), freeSpace)
            return RMFAILURE
        return RMSUCCESS
    
    def up_pre_check(self, isoPath):
        count = 5
        sigSpeed = 9
        sug = ""
        # 检测ISO是否符合要求
        for i in range(count):
            self.upCheckProcessCb(UCIterms[UCItermsEnum.UCIsoRight], 0, i * sigSpeed, "")
            sleep(0.5)
        ret = self._check_iso_right(isoPath)
        self.upCheckFlag = ret
        if ret == RMFAILURE:
            sug = UCHandlerIterms[UCItermsEnum.UCIsoRight]
        self.upCheckProcessCb(UCIterms[UCItermsEnum.UCIsoRight], ret, 50, sug)


        # 检测根分区空间
        for i in range(count):
            self.upCheckProcessCb(UCIterms[UCItermsEnum.UCRootFree], 0, 50 + i * sigSpeed, "")
            sleep(0.5)
        ret = self._check_root_free()
        self.upCheckFlag += ret
        sug = ""
        if ret == RMFAILURE:
            sug = UCHandlerIterms[UCItermsEnum.UCRootFree]
        self.upCheckProcessCb(UCIterms[UCItermsEnum.UCRootFree], ret, 100, sug)

        return self.upCheckFlag


    def _up_pre_act(self):
        cmd = "xmodmap -e \"keycode 133 =\""
        get_sys_call(cmd)
        cmd = "xmodmap -e \"keycode 134 =\""
        get_sys_call(cmd)

        self.rmStateConf.change_key_value(self.SEC_OLD_OS_INFO, self.KEY_FS_NAME, self.ORGFS_DIR)
        try:
            bus = dbus.SystemBus()
            proxy = bus.get_object("org.freedesktop.login1", "/org/freedesktop/login1")
            bus_interface = dbus.Interface(proxy, dbus_interface="org.freedesktop.login1.Manager")
            userList = []
            bM_ListUsers = bus_interface.get_dbus_method("ListUsers")
            if bM_ListUsers != None:
                userList = bM_ListUsers()

            count = len(userList)
            if count == 0:
                logging.error("[VM-UP]: can't get users")
                return
            
            logging.info("[VM-UP]: get user info[%s].", userList)
            for ele in userList:
                dbusPath = ele[2]
                userName = ele[1]
                user_proxy = bus.get_object("org.freedesktop.login1", dbusPath)
                user_inf = dbus.Interface(user_proxy, dbus_interface="org.freedesktop.DBus.Properties")
                getPro = user_inf.get_dbus_method("Get")
                state = getPro("org.freedesktop.login1.User","State")
                logging.info("[VM-UP]: get user[%s] state[%s].", ele, state)
                if state == "active":
                    self.rmStateConf.change_key_value(self.SEC_HOME_INFO, self.KEY_ACT_USER, userName)
                    logging.info("[VM-UP]: record user[%s].", userName)
                    self.appClass.backup_desktop()
                    break
        except Exception as e:
            logging.error("[VM-UP]: get bus err[%s].", e)

    def _upFunc(self,func,*param):
        if func == None:
            logging.error("[FUNC]: in func is None.")
            return RMFAILURE
        ret = RMFAILURE
        try:
            logging.info("[FUNC]: func[%s] param[%s].", func.__name__, param)
            if param == None or len(param) == 0:
                ret = func()
            else:
                ret = func(param[0])
        except Exception as e:
            logging.error("[FUNC]: upgrade err[%s]",e)
            if self.upStep != UPItermsEnum.UPCreateNewDeb:
                self.upProcess = 100
                self.upStepRet = RMFAILURE
                self.upProcessCb(UPIterms[self.upStep], self.upStepRet, self.upProcess, UPHandlerIterms[self.upStep])
                return RMFAILURE
            else:
                ret = RMSUCCESS

        self.upStepRet = ret
        self.upProcess = self.UP_PROCESS_LIST[self.upStep+1]
        return ret


    def _show_process(self, index = 0, failedCount = 0):
        logging.debug("[FUNC]: show info-a [%d %d %d %d]", self.upStep, self.upProcess, self.upStepRet, failedCount)
        if failedCount == 1:
            return
        updateSpeed = 3 # 3秒更新一次

        if self.upStep >= UPItermsEnum.UPItermsMax or self.upStep < 0:
            logging.error("[FUNC]: upstep[%d] is over", self.upStep)

        if self.upProcess < self.UP_PROCESS_LIST[self.upStep+1]:
            self.upProcess = self.upProcess + 1
        
        itermStr = UPIterms[self.upStep]
        
        sug = ""
        if self.upStepRet != RMSUCCESS:
            sug = UPHandlerIterms[self.upStep]
        try:
            self.upProcessCb(itermStr, self.upStepRet, self.upProcess, sug)
        except Exception as e:
            logging.error("[FUNC]: sig err occur[%s].", e)

        if self.upProcess >= 100 or self.upProcess < 0:
            logging.info("[FUNC]: process[%d] is over", self.upProcess)
            return
        if self.upStepRet != RMSUCCESS:
            failedCount = 1
            logging.error("[FUNC]: run [%s] failed.", itermStr)
            return

        tmpTimer = threading.Timer(updateSpeed, function=self._show_process, args=[index, failedCount])
        tmpTimer.start()


    def _up_create_init_and_functions(self, action_up, distPath):
        # 升级
        if action_up:
            srcPath = "/usr/share/revisions-manager/data/initramfs-upgrade/"
        # 还原
        else:
            srcPath = "/usr/share/revisions-manager/data/initramfs-back/"
        initFile = srcPath + "init"
        if not os.path.exists(initFile):
            logging.error("[VM-UP]: %s not exists.", initFile)
            return RMFAILURE
        cmd = "cp " + initFile + " " + distPath
        get_sys_call(cmd)

        funcFile = srcPath + "functions"
        if not os.path.exists(funcFile):
            logging.error("[VM-UP]: %s not exists.", funcFile)
            return RMFAILURE
        cmd = "cp " + funcFile + " " + distPath + "scripts/"
        get_sys_call(cmd)
        return RMSUCCESS


    # 创建原系统还原用的initrd
    # TODO:目前是整体文件替换；希望可以仅在文件中添加修改
    def _up_create_orgfs_initrd(self):
        logging.info("[VM-UP]: create orgfs initrd start.")

        ret = self._up_create_init_and_functions(False, "/usr/share/initramfs-tools/")
        if ret == RMFAILURE:
            logging.error("[VM-UP]: install itself err")
            return ret

        cmd = "update-initramfs -u -k " + self.curVersion
        os.system(cmd)

        # 记录生成的initrd md5
        initrdFileName = "initrd.img-"+self.curVersion
        initrdFilePath = os.path.join("/boot", initrdFileName)
        if os.path.exists(initrdFilePath):
            # 备份
            shutil.copyfile(initrdFilePath, self.CACHE_DIR + initrdFileName)

            cmd = "md5sum " + initrdFilePath
            ret = get_sys_call(cmd)

            with open(self.MD5_PATH, "w+") as fd:
                fd.writelines(ret)
                logging.info("[VM-UP]: add md5 info [%s]", ret)
            fd.close()
        else:
            logging.error("[VM-UP]: File[%s] not exists.",initrdFilePath)
            return RMFAILURE

        logging.info("[VM-UP]: create orgfs initrd finished ver[%s].", self.curVersion)
        return RMSUCCESS

    # 创建新系统
    # 1-解压文件系统
    # 2-拷贝授权文件.kyinfo LICENSE 去掉，不拷贝ISO中的该文件
    # 3-拷贝iso中的源
    # 4-拷贝plymouth脚本
    def _up_create_newfs(self, isoPath):
        logging.info("[VM-UP]: up_create_newfs start")
        if os.path.exists(self.NEWFS_DIR):
            try:
                # TODO:解除挂载
                shutil.rmtree(self.NEWFS_DIR)
            except OSError as e:
                shutil.rmtree(self.NEWFS_DIR)
                logging.error("[VM-UP]: remove [%s] failed as [%s]", self.NEWFS_DIR, e)
            except Exception as e:
                logging.error("[VM-UP]: remove [%s] failed as [%s]", self.NEWFS_DIR, e)
                return RMFAILURE

        mountPath = mount_iso(isoPath)
        if mountPath is None:
            logging.error("[VM-UP]: Mount [%s] failed.", isoPath)
            return RMFAILURE

        squashfsPath = os.path.join(mountPath, "casper/filesystem.squashfs")
        if not os.path.exists(squashfsPath):
            logging.error("[VM-UP]: iso is broken, please check whether exists filesystem.squashfs[%s]", squashfsPath)
            return RMFAILURE
        else:
            cmd = "unsquashfs -d " + self.NEWFS_DIR + " " + squashfsPath
            logging.info("[VM-UP]: cmd: %s", cmd)
            try:
                shellRet = subprocess.run(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                if shellRet.returncode != RMSUCCESS:
                    logging.error("[VM-UP]: cmd[%s] ret[%d] failed as [%s].", cmd, shellRet.returncode, shellRet.stderr.decode())
                    return RMFAILURE
            except Exception as e:
                logging.error("[VM-UP]: cmd[%s] failed as [%s].", cmd, e)

        # iso中的源拷贝并设置
        cdromPath = os.path.join(self.NEWFS_DIR, "cdrom")
        if not os.path.exists(cdromPath):
            os.makedirs(cdromPath, mode=0o755)

        srcPath=os.path.join(mountPath, "pool")
        cmd = "cp -r " + srcPath + " " + cdromPath
        get_sys_call(cmd)

        srcPath=os.path.join(mountPath, "dists")
        cmd = "cp -r " + srcPath + " " + cdromPath
        get_sys_call(cmd)

        srcPath=os.path.join(mountPath, ".kylin-post-packages")
        cmd = "cp -r " + srcPath + " " + cdromPath
        get_sys_call(cmd)

        srcPath="/usr/share/revisions-manager/data/plymouth/ubuntukylin-logo.script"
        distDir="/fs.new/usr/share/plymouth/themes/ubuntukylin-logo/"
        cmd = "cp -r " + srcPath + " " + distDir
        get_sys_call(cmd)

        srcPath="/usr/share/revisions-manager/data/plymouth/openkylin-logo.script"
        distDir="/fs.new/usr/share/plymouth/themes/openkylin-logo/"
        cmd = "cp -r " + srcPath + " " + distDir
        get_sys_call(cmd)
        
        umount_iso(mountPath)
        logging.info("[VM-UP]: create newfs finished")
        return RMSUCCESS

    # 迁移etc
    def _up_move_etc(self):
        logging.info("[VM-UP]: _up_move_etc start")
        moveEtcClass = RMMoveEtc()
        ret = moveEtcClass.move_etc()
        logging.info("[VM-UP]: move etc ret[%d]", ret)
        return ret


    # 创建升级新系统用的initrd
    # 提前安装kylin-revision-manager到新系统
    def _up_inchroot_handle(self):
        logging.info("[VM-UP]: _up_inchroot_handle start")
        new_ker_version=""

        ret = self._up_create_init_and_functions(True, "/fs.new/usr/share/initramfs-tools/")
        if ret == RMFAILURE:
            logging.error("[VM-UP]: install itself err")
            return ret

        newfd=os.open(self.NEWFS_DIR,os.O_RDONLY)
        orgfd=os.open("/",os.O_RDONLY)
        os.chroot(self.NEWFS_DIR)
        os.fchdir(newfd)

        cmd = "linux-version list --paths | linux-version sort --reverse"
        kerVersion = get_sys_call(cmd)
        if len(kerVersion) > 0:
            kerVersionList = str_to_list(kerVersion, "\n")
            KER_VERSION = kerVersionList[0].split(" ")
            new_ker_version = KER_VERSION[0]
        else:
            cmd = "dpkg -l | grep linux-image-[0-9] | awk \'{print $2}\'"
            kerVersion = get_sys_call(cmd)
            kerVersionList = str_to_list(kerVersion, "\n")
            KER_VERSION = kerVersionList[0].split("image-")
            new_ker_version = KER_VERSION[1]

        os.system("mount -t  proc proc /proc")
        os.system("mount -t sysfs sys /sys")
        os.system("mount -t devtmpfs devtmpfs /dev")
        os.system("mount -t devpts devpts /dev/pts")
        get_sys_call("update-initramfs -u")

        # 安装kylin-revision-manager
        ret = self.appClass.install_revisions_manager()

        os.system("umount /dev/pts")
        os.system("umount /dev")
        os.system("umount /sys")
        os.system("umount /proc")

        os.fchdir(orgfd)
        os.chroot(".")

        if ret == RMFAILURE:
            logging.error("[VM-UP]: install itself err")
            return ret

        if os.path.exists("/fs.new/boot/initrd.img-"+new_ker_version):
            os.listdir("/fs.new/boot/")
            cmd = "cp -r /fs.new/boot/*" + new_ker_version + " /boot"
            get_sys_call(cmd)
        else:
            logging.error("[VM-UP]: /fs.new/boot/initrd.img-"+new_ker_version+" not exists")
            return RMFAILURE

        self.rmStateConf.change_key_value(self.SEC_NEW_OS_INFO, self.KEY_KERNEL, new_ker_version)
        logging.info("[VM-UP]: _up_inchroot_handle finished")
        return RMSUCCESS

    # 在 chroot 后执行, 创建升级的GRUB
    def _up_create_newfs_grub(self):
        logging.info("[VM-UP]:_up_create_newfs_grub start.")
        menulist = []
        if self.newVersion is None:
            newBootDir = "/fs.new/boot/"
            kerlist = []
            fileList = os.listdir(newBootDir)
            for ele in fileList:
                if "initrd.img" in ele:
                    if os.path.islink(ele):
                        continue
                    eleSplit = ele.split("-", maxsplit=1)
                    if len(eleSplit) <= 1:
                        continue
                    kerlist.append(eleSplit[1])
            kerlist.sort(reverse=True)
            if len(kerlist) != 0:
                self.newVersion = kerlist[0]

        logging.info("[VM-UP]: new ker version is[%s].", self.newVersion)

        grubClass = RMGrub()
        grubClass.analysis_cfg(grubClass.grubCfgPath)

        # 创建备份
        shutil.copyfile(grubClass.grubCfgPath, os.path.join(self.CACHE_DIR, "grub.cfg_bak-"+self.newVersion))

        grubClass.get_UUID()
        menulist = grubClass.create_upgrade_submenu(self.newVersion)
        if menulist is None or len(menulist) == 0:
            logging.error("[VM-UP]: create menu failed")
            return RMFAILURE

        forcemenulist = grubClass.create_force_org_submenu(self.curVersion)
        if forcemenulist is None or len(forcemenulist) == 0:
            logging.error("[VM-UP]: create force back menu failed")

        menulist.extend(forcemenulist)

        grubClass.add_subMenu(self.GRUB_FILE_PATH, menulist)
        grubClass.remove_hidden()

        logging.info("[VM-UP]: _up_create_newfs_grub success.")
        return RMSUCCESS

    def _up_compat_app(self):
        logging.info("[VM-UP]: _up_compat_app start.")
        try:
            self.appClass.find_repack_list()
            ret = self.appClass.create_new_deb()
        except Exception as e:
            logging.error("[VM-UP]: create new app deb err[%s].", e)

        logging.info("[VM-UP]: _up_compat_app finished.")
        return ret

    def up_act(self, isoPath):
        if self.upCheckFlag != RMSUCCESS:
            logging.error("[VM-UP]: pre upgrade check not success")
            return RMFAILURE

        self._up_pre_act()
        tmpTimer = threading.Timer(1, function=self._show_process)
        tmpTimer.start()

        self.upProcess = 0
        self.upStep = UPItermsEnum.UPCreateOrgFsInitrd
        self.upStepRet = RMSUCCESS
        ret = self._upFunc(self._up_create_orgfs_initrd)
        if ret == RMFAILURE:
            logging.error("[VM-UP]: action failed and return")
            return ret

        self.upStep = UPItermsEnum.UPCreateNewFs
        ret = self._upFunc(self._up_create_newfs, isoPath)
        if ret == RMFAILURE:
            logging.error("[VM-UP]: action failed and return")
            return ret

        self.upStep = UPItermsEnum.UPCreateNewDeb
        ret = self._upFunc(self._up_compat_app)

        value = self.rmConf.get_key(self.rmConf.SEC_SWITCH, self.rmConf.KEY_SW_ETC)
        if value.lower() == "true":
            self.upStep = UPItermsEnum.UPMoveEtc
            ret = self._upFunc(self._up_move_etc)
            if ret == RMFAILURE:
                logging.error("[VM-UP]: action failed and return")
                return ret

        self.upStep = UPItermsEnum.UPCreateNewFsInitrd
        ret = self._upFunc(self._up_inchroot_handle)
        if ret == RMFAILURE:
            logging.error("[VM-UP]: action failed and return")
            return ret

        self.upStep = UPItermsEnum.UPCreateNewGrub
        ret = self._upFunc(self._up_create_newfs_grub)
        if ret == RMFAILURE:
            logging.error("[VM-UP]: action failed and return")
            return ret

        self.upProcessCb(UPIterms[self.upStep], 0, 100, '')
        self.rmStateConf.change_key_value(self.SEC_OLD_OS_INFO, self.KEY_KERNEL, self.curVersion)
        self.versionManagerState = self.intToStr.get(self.VMINIT)
        self.rmStateConf.change_key_value(self.SEC_STATE,self.KEY_VERSIONMANAGER, self.versionManagerState)

    ####################还原####################

    # 还原重启后的处理
    # 更新grub initrd
    def _back_post_handle(self):
        logging.info("[VM-BACK]: post handle start")
        self.versionManagerState = self.intToStr.get(self.VMBACKSUCCESS)
        self.rmStateConf.change_key_value(self.SEC_STATE,self.KEY_VERSIONMANAGER, self.versionManagerState)
        self.appClass.resume_desktop()
        newKerVer = self.rmStateConf.get_key(self.SEC_NEW_OS_INFO,self.KEY_KERNEL)
        if newKerVer != None:
            cmd = "rm -r /boot/*" + newKerVer
            os.system(cmd)

        logging.info("[VM-BACK]: post handle -- update grub")
        update_grub()

        logging.info("[VM-BACK]: post handle -- update initrd")
        os.system("update-initramfs -u -k " + self.curVersion)

        self.rmStateConf.clear_sec(self.SEC_NEW_OS_INFO)
        logging.info("[VM-BACK]: post handle end")


    def back_check_old_os_exists(self):
        oldfs = self.rmStateConf.get_key(self.SEC_OLD_OS_INFO, self.KEY_FS_NAME)
        if oldfs != None and os.path.exists(oldfs):
            logging.info("[VM-BACK]: old os is [%s].", oldfs)
            return True

        logging.error("[VM-BACK]: %s not exists.", oldfs)
        return False
    
    def _back_check_old_os_complete(self):
        oldfs = self.rmStateConf.get_key(self.SEC_OLD_OS_INFO, self.KEY_FS_NAME)
        if not os.path.exists(oldfs):
            logging.error("[VM-BACK]: %s not exists.", oldfs)
            return RMFAILURE
        
        if not os.path.exists("/usr/share/revisions-manager/"):
            logging.error("[VM-BACK]: revisions-manager not complete.")
            return RMFAILURE
        
        return RMSUCCESS


    def _back_check_initrd(self):
        oldfs = self.rmStateConf.get_key(self.SEC_OLD_OS_INFO, self.KEY_FS_NAME)
        kerVer = self.rmStateConf.get_key(self.SEC_OLD_OS_INFO , self.KEY_KERNEL)
        if kerVer == None:
            logging.error("[VM-BACK]: no old ker version.")
            return RMFAILURE

        md5Str = ""
        if os.path.exists(self.MD5_PATH):
            with open(self.MD5_PATH, "r") as fd:
                md5Str = fd.readlines()
            fd.close()
        if len(md5Str) < 1:
            logging.error("[VM-BACK]: md5 info is err.")
            return RMFAILURE
        valuedist = md5Str[0].split()[0]
        ret = ""
        initrdFilePath = oldfs + self.CACHE_DIR + "initrd.img-" + kerVer
        if os.path.exists(initrdFilePath):
            cmd = "md5sum " + initrdFilePath
            ret = get_sys_call(cmd)
            valueSrc = ret.split()[0]
            if valueSrc != valuedist:
                logging.error("[VM-BACK]: [%s][%s] md5 is diff.", valueSrc, valuedist)
                return RMFAILURE

            shutil.copyfile(initrdFilePath, "/boot/" + "initrd.img-" + kerVer)
            return RMSUCCESS

        logging.error("[VM-BACK]: [%s] is moved.", initrdFilePath)
        return RMFAILURE        

    def back_pre_check(self):
        ret = RMSUCCESS
        sug = ""
        count = 9
        sigSpeed = 5

        try:
            for i in range(count):
                self.backCheckProcessCb(FCIterms[FCItermsEnum.FCCheckOldOs], 0, i * sigSpeed, "")
                sleep(0.3)
            ret = self._back_check_old_os_complete()
            if ret == RMFAILURE:
                sug = FCHandlerIterms[FCItermsEnum.FCCheckOldOs]
            self.backCheckProcessCb(FCIterms[FCItermsEnum.FCCheckOldOs], ret, 50, sug)
        except Exception as e:
            logging.error("[VM-BACK]: pre Fallback Check err[%s].", e)
            self.backCheckProcessCb(FCIterms[FCItermsEnum.FCCheckOldOs], RMFAILURE, 50, sug)

        try:
            for i in range(count):
                self.backCheckProcessCb(FCIterms[FCItermsEnum.FCCheckOrgInitrd], 0, i * sigSpeed + 50, "")
                sleep(0.3)
            ret = self._back_check_initrd()
            if ret == RMFAILURE:
                sug = FCHandlerIterms[FCItermsEnum.FCCheckOrgInitrd]
            self.backCheckProcessCb(FCIterms[FCItermsEnum.FCCheckOrgInitrd], ret, 100, sug)
        except Exception as e:
            logging.error("[VM-BACK]: pre Fallback Check err[%s].", e)
            self.backCheckProcessCb(FCIterms[FCItermsEnum.FCCheckOrgInitrd], RMFAILURE, 99, sug)
    
    def _back_create_fallback_grub(self):
        kerver = self.rmStateConf.get_key(self.SEC_OLD_OS_INFO, self.KEY_KERNEL)
        oldfs = self.rmStateConf.get_key(self.SEC_OLD_OS_INFO, self.KEY_FS_NAME)
        if kerver == None:
            logging.error("[VM-BACK]: no old kerver.")
            return RMFAILURE

        grubClass = RMGrub()

        # /boot不是单独分区的处理   单独封装
        cmd = "lsblk | grep \"/boot\$\" | wc -l"
        ret = subprocess.run(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        if ret.returncode != 0:
            logging.error("[VM-BACK]: find boot failed [%s].", ret.stderr.decode())
            return RMFAILURE
        retStr = ret.stdout.decode().strip("\n")
        if retStr == '0':
            initrdPath = grubClass.find_initrd(kerver, oldfs + "/boot/")
            vmlinuzPath = grubClass.find_image(kerver, oldfs + "/boot/")
            cmd = "cp " + initrdPath + " /boot/"
            get_sys_call(cmd)
            cmd = "cp " + vmlinuzPath + " /boot/"
            get_sys_call(cmd)


        try:
            # 创建备份
            shutil.copy2(grubClass.grubCfgPath, os.path.join(self.CACHE_DIR, "grub.cfg_bak-"+kerver))
            
            grubClass.analysis_cfg(grubClass.grubCfgPath)
            ret = grubClass.get_UUID()
            if ret == RMFAILURE:
                logging.info("[VM-BACK]: get UUID failed")
                return RMFAILURE

            menulist = grubClass.create_fallback_submenu(kerver)
            if menulist is None:
                logging.info("[VM-BACK]: create fallback submenu failed")
                return RMFAILURE
            grubClass.add_subMenu(self.GRUB_FILE_PATH, menulist)
        except Exception as e:
            logging.error("[VM-BACK]: create_fallback_grub failed as [%s].",e)
            return RMFAILURE

        logging.info("[VM-BACK]: create_fallback_grub success.")
        return RMSUCCESS


    def back_act(self):
        sug = ""
        count = 20
        sigSpeed = 5
        # 检测ISO是否符合要求
        for i in range(count):
            self.backProcessCb(FPIterms[FPItermsEnum.FPCreateGrub], 0, i * sigSpeed, "")
            sleep(0.3)

        # 创建grub
        ret = self._back_create_fallback_grub()
        if ret == RMFAILURE:
            sug = FPHandlerIterms[FPItermsEnum.FPCreateGrub]
        self.backProcessCb(FPIterms[FPItermsEnum.FPCreateGrub], ret, 100, sug)
        self.versionManagerState = self.intToStr.get(self.VMINIT)
        self.rmStateConf.change_key_value(self.SEC_STATE, self.KEY_VERSIONMANAGER, self.versionManagerState)

    # 应用兼容配置
    def get_uncompat_package(self):
        return self.appClass.get_not_compat_package()

    def get_app_move_flag(self):
        return self.appClass.get_app_move()
    
    # 设置是否迁移第三方应用
    def switch_apps_move(self, enable):
        if enable:
            self.appClass.enable_app_move()
        else:
            self.appClass.disable_app_move()

    def up_getAppList(self):
        self.appClass.find_repack_list()
        return self.appClass.get_app_list()

    def up_setAppList(self, appList):
        return self.appClass.set_app_list(appList)
    
    def up_clearMoveAppList(self):
        return self.appClass.clear_move_app_list()
    

    def get_state(self):
        return self.versionManagerState
    
    def get_old_os(self):
        return self.ORGFS_DIR
    
    def add_notice(self):
        stateInt = self.strToInt.get(self.versionManagerState)
        if stateInt == self.VMUPFAILED or stateInt == self.VMUPSUCCESS:
            self.versionManagerState = self.intToStr.get(self.VMUP)
        elif stateInt == self.VMBACKFAILED or stateInt == self.VMBACKSUCCESS:
            self.versionManagerState = self.intToStr.get(self.VMBACK)
        else:
            logging.info("[VM-ALL]: VM STATE IS %s.", self.versionManagerState)
            return
        
        self.rmStateConf.change_key_value(self.SEC_STATE, self.KEY_VERSIONMANAGER, self.versionManagerState)

if __name__ == "__main__":
    # testClass = RMVersionManager()
    # testClass._up_create_newfs_grub()

    pass
