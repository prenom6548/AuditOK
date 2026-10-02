#!/usr/bin/python3
# -*- coding: UTF-8 -*-

import dbus
import dbus.service
import logging
from gettext import gettext as _

from RevisionsManager.Core.RMVersionManager import RMVersionManager
from RevisionsManager.Core.RMOsInfo import RMOsInfo


RM_DBUS_INTERFACE = 'com.kylin.RevisionsManager.interface'
RM_DBUS_PATH = '/com/kylin/RevisionsManager'
RM_DBUS_SERVICE = 'com.kylin.RevisionsManager'

# method 加锁防止调用混乱
class RevisionsManagerDbusController(dbus.service.Object):
    """ Provide the RevisionsManagerIFace """
    def __init__(self, parent, bus_name, object_path=RM_DBUS_PATH):

        self.methodLock = False             # Dbus method lock
        self.runningMethod = None           # Dbus method running name
        dbus.service.Object.__init__(self, bus_name, object_path)
        self.parent = parent
        self.bus = dbus.SystemBus()

        self.VersionManagerHD = RMVersionManager()
        self.VersionManagerHD.reg_up2_process_func(self.upgradeProcessSig2)
        self.VersionManagerHD.reg_up_process_func(self.upgradeProcessSig)
        self.VersionManagerHD.reg_upCheck_process_func(self.upgradeCheckSig)
        self.VersionManagerHD.reg_back_process_func(self.fallbackProcessSig)
        self.VersionManagerHD.reg_backCheck_process_func(self.fallbackCheckSig)
        self.osInfoHD = RMOsInfo()
        logging.info("[DBUS]: init finished")


    # 升级第二阶段进度信号
    @dbus.service.signal(RM_DBUS_INTERFACE, signature="si")
    def upgradeProcessSig2(self, itermStr, process):
        logging.info("Emitting upgradeProcess2 item[%s] process[%d].\n",itermStr, process)
    
    @dbus.service.signal(RM_DBUS_INTERFACE, signature="siis")
    def upgradeCheckSig(self, itermStr, state, process, suggestion):
        logging.info("Emitting upgradeCheck item[%s] state[%d] process[%d] suggestion[%s].\n",itermStr, state, process, suggestion)

    @dbus.service.signal(RM_DBUS_INTERFACE, signature="siis")
    def upgradeProcessSig(self, itermStr, state, process, suggestion):
        logging.info("Emitting upgradeProcess item[%s] state[%d] process[%d] suggestion[%s].\n",itermStr, state, process, suggestion)

    @dbus.service.signal(RM_DBUS_INTERFACE, signature="siis")
    def fallbackCheckSig(self, itermStr, state, process, suggestion):
        logging.info("Emitting fallbackCheck item[%s] state[%d] process[%d] suggestion[%s].\n",itermStr, state, process, suggestion)

    @dbus.service.signal(RM_DBUS_INTERFACE, signature="siis")
    def fallbackProcessSig(self, itermStr, state, process, suggestion):
        logging.info("Emitting fallbackProcess item[%s] state[%d] process[%d] suggestion[%s].\n",itermStr, state, process, suggestion)

    #升级前检测
    @dbus.service.method(RM_DBUS_INTERFACE, in_signature="s")
    def preUpgradeCheck(self, isoPath):
        if not isoPath.startswith("/"):
            logging.error("[DBUS]: isoPath[%s] is not abs path.", isoPath)
            return

        if self.methodLock:
            logging.error("[DBUS]: Please wait. Last method[%s] unfinished.", self.runningMethod)
            return
        self.methodLock = True
        self.runningMethod = "preUpgradeCheck"

        self.VersionManagerHD.up_pre_check(isoPath)

        self.methodLock = False
        self.runningMethod = None

    #执行升级
    @dbus.service.method(RM_DBUS_INTERFACE, in_signature="s")
    def upgradeVersion(self, isoPath):
        if not isoPath.startswith("/"):
            logging.error("[DBUS]: isoPath[%s] is not abs path.", isoPath)
            return
        if self.methodLock:
            logging.error("[DBUS]: Please wait. Last method[%s] unfinished.", self.runningMethod)
            return
        self.methodLock = True
        self.runningMethod = "upgradeVersion"

        self.VersionManagerHD.up_act(isoPath)

        self.methodLock = False
        self.runningMethod = None


    # 获取当前版本
    @dbus.service.method(RM_DBUS_INTERFACE, out_signature='s')
    def getCurOsVersion(self):
        return self.osInfoHD.get_pretty_name()

    #还原前检测 un
    @dbus.service.method(RM_DBUS_INTERFACE)
    def preFallbackCheck(self):
        # 是否有还原文件系统
        if self.methodLock:
            logging.error("[DBUS]: Please wait. Last method[%s] unfinished.", self.runningMethod)
            return
        self.methodLock = True
        self.runningMethod = "preFallbackCheck"

        self.VersionManagerHD.back_pre_check()
        self.methodLock = False
        self.runningMethod = None


    #执行还原
    @dbus.service.method(RM_DBUS_INTERFACE)
    def fallbackVersion(self):
        if self.methodLock:
            logging.error("[DBUS]: Please wait. Last method[%s] unfinished.", self.runningMethod)
            return
        self.methodLock = True
        self.runningMethod = "fallbackVersion"

        self.VersionManagerHD.back_act()

        self.methodLock = False
        self.runningMethod = None

    # 解锁 for test
    @dbus.service.method(RM_DBUS_INTERFACE)
    def TEST_unlockMethod(self):
        self.methodLock = False
        self.runningMethod = None

    # 获取线程状态，
    # 阻塞 -- True
    # 在运行 -- False
    @dbus.service.method(RM_DBUS_INTERFACE)
    def isblocked(self):
        logging.info("[DBUS]: IS NOT BLOCKED")
        return False

    # 重启后，获取升级或还原的结果
    # 升级：成功、完成、失败
    # 还原：成功、失败
    @dbus.service.method(RM_DBUS_INTERFACE, out_signature='ss')
    def getState(self):
        return self.VersionManagerHD.get_state(), ""

    # 旧系统是否存在
    @dbus.service.method(RM_DBUS_INTERFACE, out_signature='i')
    def isOldOsExists(self):
        ret = self.VersionManagerHD.back_check_old_os_exists()
        logging.debug("[DBUS]: check old os exist [%d].", ret)
        return ret

    @dbus.service.method(RM_DBUS_INTERFACE, out_signature='s')
    def getOldOs(self):
        return self.VersionManagerHD.get_old_os()

    # 获取当前第三方应用列表
    # 返回值：应用列表
    @dbus.service.method(RM_DBUS_INTERFACE, out_signature='as')
    def getAppList(self):       
        logging.debug("[DBUS]: get App List.")
        return self.VersionManagerHD.up_getAppList()

    # 设置不需要迁移的第三方应用列表
    # 参数：列表
    # 返回值：0--成功， 其他--失败
    @dbus.service.method(RM_DBUS_INTERFACE, in_signature='as')
    def setMoveAppList(self, appList):
        logging.debug("[DBUS]: set not move App List.")
        self.VersionManagerHD.up_setAppList(appList)

    # 清除不需要迁移的第三方应用列表
    @dbus.service.method(RM_DBUS_INTERFACE)
    def clearMoveAppList(self):
        logging.debug("[DBUS]: clear move App List.")
        self.VersionManagerHD.up_clearMoveAppList()
    
    # 获取升级不兼容应用列表
    @dbus.service.method(RM_DBUS_INTERFACE, out_signature='as')
    def getUncompatPackageList(self):
        if self.VersionManagerHD.get_app_move_flag():
            return self.VersionManagerHD.get_uncompat_package()
        else:
            return []

    # 软件是否迁移的开关，默认为否
    @dbus.service.method(RM_DBUS_INTERFACE, in_signature='u')
    def switchAppMove(self, state):
        logging.debug("[DBUS]: apps move switch [%s].", state)
        if state != True:
            state = False
        self.VersionManagerHD.switch_apps_move(state)


    # 获取是否超出还原时间
    @dbus.service.method(RM_DBUS_INTERFACE, out_signature='i')
    def isOverTime(self):
        return False

    # 前端显示了结果界面
    @dbus.service.method(RM_DBUS_INTERFACE)
    def addNoticeCount(self):
        self.VersionManagerHD.add_notice()

    ## 以下无用 ##

    # 设置离线源文件
    # 参数：可以是空 或 绝对路径
    # 返回值：0--成功， 其他--失败
    @dbus.service.method(RM_DBUS_INTERFACE, in_signature='s')
    def setOfflineSource(self, path):
        logging.debug("[DBUS]: offline source is [%s].", path)

    # 设置授权文件路径
    # 参数：绝对路径
    # 返回值：0--成功， 其他--失败
    @dbus.service.method(RM_DBUS_INTERFACE, in_signature='ss')
    def setActivationFiles(self, iPath, lPath):
        logging.debug("[DBUS]: kyinfo is [%s] and LICENSE is [%s].", iPath, lPath)
    
    # 获取升级第三方应用安装失败列表
    @dbus.service.method(RM_DBUS_INTERFACE, out_signature='as')
    def getFailedPackageList(self):
        return []

