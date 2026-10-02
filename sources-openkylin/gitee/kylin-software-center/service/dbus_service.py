#!/usr/bin/python3
# -*- coding: utf-8 -*
import dbus
import os
import sys
import shutil
import locale
import datetime
import time
import base64
from utils.aescrypt import *
from gi.repository import GObject
import logging
import json

from PyQt5.QtCore import *
from PyQt5 import QtDBus
UBUNTUKYLIN_SOFTWARECENTER_ACTION = 'com.kylin.softwarecenter.action'
from models.enums import (UBUNTUKYLIN_SERVICE_PATH,
                          UBUNTUKYLIN_INTERFACE_PATH,
                          Signals,
                          AppActions,
                          AptProcessMsg,
                          AptActionMsg,
                          ERR_LOG,
                          INFO_LOG,
                          DEBUG_LOG,
                          KYDROID_VERSION_D)

import multiprocessing

from dbus.mainloop.glib import DBusGMainLoop, threads_init
from models.globals import Globals
#from dbus.mainloop.qt import DBusQtMainLoop
#mainloop = DBusQtMainLoop()
from utils.log_manager import LogManager
import gettext
gettext.textdomain("kylin-software-center")
_ = gettext.gettext
# LOG = logging.getLogger("uksc")


class Dbus_Service(QObject,Signals):

    dbus_apt_process = pyqtSignal(str,str,str,int,str)
    copy_finish = pyqtSignal(str,str)
    check_apt_lock_finish = pyqtSignal(str,str,int)

    def __init__(self,need_init_connect):
        QObject.__init__(self)
        locale.setlocale(locale.LC_ALL, "zh_CN.UTF-8")

        self.iface = None
        self.need_init_connect = need_init_connect

    #
    # 函数：初始化dbus接口
    #
    def init_dbus_ifaces(self):
        try:
            Globals.LOG_INFO.logger.info("InstallBackend init_dbus_ifaces (" + datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f") + ")")
            DBusGMainLoop(set_as_default=True)
            threads_init()
            bus = dbus.SystemBus()
        except Exception as e:
            Globals.LOG_ERROR.logger.error("dbus exception:%s" % str(e))
            #self.init_models_ready.emit("fail","初始化失败!")
            self.init_models_ready.emit("fail", _("Initialization failed"))
            return False

        try:
            Globals.LOG_INFO.logger.info("InstallBackend init_dbus_ifaces call blocking (%s)" % datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
            bus.call_blocking(UBUNTUKYLIN_SERVICE_PATH, '/', UBUNTUKYLIN_INTERFACE_PATH, 'wakeup', None, (), timeout=5)
        except dbus.DBusException as e:
            Globals.LOG_ERROR.logger.error("InstallBackend DBusConnection call blocking exception: %s" % str(e))
            return False

        try:
            Globals.LOG_INFO.logger.info("InstallBackend init_dbus_ifaces bus.get_object (%s)" % datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
            obj = bus.get_object(UBUNTUKYLIN_SERVICE_PATH, '/')
            Globals.LOG_INFO.logger.info("InstallBackend init_dbus_ifaces dbus.Interface (%s)" % datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
            self.iface = dbus.Interface(obj, dbus_interface=UBUNTUKYLIN_INTERFACE_PATH)
            Globals.LOG_INFO.logger.info("InstallBackend init_dbus_ifaces finished (%s)" % datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))

            # lixiang: Repeated starts dbus service may cause 25 seconds stuck because of bus.get_object or connect_to_signal
            #if self.iface is not None:
            #    LOG.info("InstallBackend start to connect signal named software_apt_signal (%s)" % datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
            #    self.iface.connect_to_signal("software_apt_signal",self._on_software_apt_signal, dbus_interface=UBUNTUKYLIN_INTERFACE_PATH)
            #    LOG.info("InstallBackend start to connect signal named software_fetch_signal (%s)" % datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
            #    self.iface.connect_to_signal("software_fetch_signal",self._on_software_fetch_signal, dbus_interface=UBUNTUKYLIN_INTERFACE_PATH)
            #    LOG.info("InstallBackend start to connect signal named software_auth_signal (%s)" % datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
            #    self.iface.connect_to_signal("software_auth_signal",self._on_software_auth_signal, dbus_interface=UBUNTUKYLIN_INTERFACE_PATH)
            #    LOG.info("InstallBackend connect signals finished (%s)" % datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
            #else:
            #    LOG.info("InstallBackend dbus interface in none (%s)" % datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
        except dbus.DBusException as e:
#            bus_name = dbus.service.BusName('com.ubuntukylin.softwarecenter', bus)
#            self.dbusControler = SoftwarecenterDbusController(self, bus_name)
#           self.init_models_ready.emit("fail","初始化失败!")
            self.init_models_ready.emit("fail",_("Initialization failed"))
            Globals.LOG_ERROR.logger.error("InstallBackend DBusConnection exception: %s" % str(e))
            return False
        if(self.need_init_connect == True):
            # lixiang: QTimer no response ???
            GObject.timeout_add(2000, self.slotTimeout)

        return True

    #lixiang
    def slotTimeout(self):
        Globals.LOG_INFO.logger.info("InstallBackend slotTimeout Responsed (%s)" % datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
        if self.iface is not None:
            self.iface.connect_to_signal("software_apt_signal", self._on_software_apt_signal, dbus_interface=UBUNTUKYLIN_INTERFACE_PATH)
            self.iface.connect_to_signal("software_fetch_signal", self._on_software_fetch_signal,dbus_interface=UBUNTUKYLIN_INTERFACE_PATH)
            self.iface.connect_to_signal("software_auth_signal", self._on_software_auth_signal,dbus_interface=UBUNTUKYLIN_INTERFACE_PATH)
            self.iface.connect_to_signal("copy_finish",self.call_copy_finish,dbus_interface=UBUNTUKYLIN_INTERFACE_PATH)
        else:
            Globals.LOG_INFO.logger.info("InstallBackend dbus interface in none (%s)" % datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
        return False

    # 安卓环境启动检测dbus接口
    def kydroid_dbus_ifaces(self):
        self.kydroid_iface = None
        try:
            bus = dbus.SystemBus()
        except Exception as e:
            Globals.LOG_ERROR.logger.error("dbus exception:%s" % str(e))
            # self.init_models_ready.emit("fail","安卓dbus初始化失败!")
            self.init_models_ready.emit("fail", _("Android dbus initialization failed"))
            return False
        dbus_server = "cn.kylinos." + KYDROID_VERSION_D
        dbus_path = "/cn/kylinos/" + KYDROID_VERSION_D
        try:
            obj = bus.get_object(dbus_server,
                                 dbus_path,
                                 dbus_server)
            self.kydroid_iface = dbus.Interface(obj, dbus_server)

        #            self.call_dbus_iface("check_source_ubuntukylin")

        # self.iface.connect_to_signal("software_fetch_signal",self._on_software_fetch_signal)
        # self.iface.connect_to_signal("software_apt_signal",self._on_software_apt_signal)
        # self.iface.connect_to_signal("software_auth_signal",self._on_software_auth_signal)
        except dbus.DBusException as e:
            #            bus_name = dbus.service.BusName('com.ubuntukylin.softwarecenter', bus)
            #            self.dbusControler = SoftwarecenterDbusController(self, bus_name)
            #           self.init_models_ready.emit("fail","安卓dbus初始化失败!")
            self.init_models_ready.emit("fail", _("Android dbus initialization failed"))
            Globals.LOG_ERROR.logger.error("dbus exception:%s" % str(e))
            return False

        return True



    #
    # 函数：获取kydroid环境运行状态
    #
    def get_kydroid_evnrun(self,name,uid,prop):
        return self.call_kydroid_dbus_iface("GetPropOfContainer",name,uid,prop)

    #
    #函数：退出dbus后台
    #
    def exit_dbus(self):
        self.iface.exit()

    #
    #函数：进行私钥加密
    #
    def add_key(self,kwargs):
        get_string = base64.b64decode(Globals.STRING).decode()
        res = QByteArray()
        if(kwargs != None):
            for i in kwargs:
                res.append(i)
        else:
            res.append(kwargs)
        #生成一个md5的散列
        cryptographichash = QCryptographicHash(QCryptographicHash.Md5)
        #计算出哈希值
        cryptographichash.addData(res)
        #获取最终的哈希值
        md5 = cryptographichash.result()
        aescryptor = Aescrypt()
        get_string = "-----BEGIN RSA PRIVATE KEY-----\n" + get_string + "\n-----END RSA PRIVATE KEY-----"
        #进行加签
        re_text = aescryptor.add_key_encrypt(get_string,md5.data())  #结果为byte类型
        return re_text.decode()

    # call the dbus functions by function name
    def call_dbus_iface(self, funcname, kwargs=None):
        signature_data = self.add_key(kwargs)
        if self.iface is None:
            return None
        func = getattr(self.iface, funcname)
        if func is None:
            return None

        res = None
        try:
            if(funcname == AppActions.REMOVE or funcname == AppActions.INSTALL):
                res = func(kwargs,signature_data)
            else:
                res = func(kwargs)
        except dbus.DBusException as e:
            self.init_dbus_ifaces()
            try:
                self.call_dbus_iface(funcname)
            except:
                pass
            Globals.LOG_ERROR.logger.error("apt-daemon dbus exception:%s" % str(e))
            return None
        return res

    #
    # 函数：安装本地deb包调用
    #
    def install_debfile(self, path):
        debcache_dir = os.path.join(os.path.expanduser("~"), ".cache", "uksc", "debfile")
        if(os.path.exists(debcache_dir) == False):
            os.makedirs(debcache_dir)
        if(os.path.exists(path)):
            shutil.copy(path, debcache_dir)
        debcache_path = os.path.join(debcache_dir,os.path.split(path)[1])

        return self.call_dbus_iface(AppActions.INSTALLDEBFILE, debcache_path)

    def install_package(self,pkgname):
        return self.call_dbus_iface(AppActions.INSTALL,pkgname)

    # def mk_dir(self,path):
    #     self.iface.mkdir(path)

    def run_app(self,cmd):
        self.iface.run_app(cmd)

    def remove_package(self,pkgname):
        return self.call_dbus_iface(AppActions.REMOVE,pkgname)

    def upgrade_package(self,pkgname):
        return self.call_dbus_iface(AppActions.UPGRADE,pkgname)

    def cancel_package(self,cancelinfo):
        return self.call_dbus_iface(AppActions.CANCEL,cancelinfo)

    def update_source(self,quiet=False):
        return self.call_dbus_iface(AppActions.UPDATE,False)

    def update_source_first_os(self,quiet=False):
        return self.call_dbus_iface(AppActions.UPDATE_FIRST,False)

    def add_source(self,text):
        return self.call_dbus_iface(AppActions.ADD_SOURCE,text)

    def remove_source(self,text):
        return self.call_dbus_iface(AppActions.REMOVE_SOURCE,text)

    def clear_dbus_worklist(self):
        return self.call_dbus_iface("clear_all_work_item")

    def check_dbus_workitem(self):
        return self.call_dbus_iface("check_work_item")

    def check_uksc_is_working(self):
        return self.call_dbus_iface("check_uksc_is_working")

    def set_uksc_not_working(self):
        return self.call_dbus_iface("set_uksc_not_working")

    def exit_uksc_apt_daemon(self):
        self.call_dbus_iface("exit")

    def uksc_exit(self):
        re = self.call_dbus_iface("uksc_exit")

    def call_check_apt_lock(self,appname,action,percent):
        self.call_dbus_iface("check_apt_lock")
        self.check_apt_lock_finish.emit(appname,action,percent)


    def check_dpkg_statu(self):
        self.call_dbus_iface("check_dpkg_statu")

    def call_kydroid_policykit(self):
        return self.call_dbus_iface("kydroid_policykit")

    def call_auth_with_policykit(self,text,action_id):
        return True
        # return  self.call_dbus_iface("auth_with_policykit",(text,action_id))
        list = [text,action_id]
        signature_data = self.add_key(list)
        try:
            return self.iface.auth_with_policykit(text,action_id,signature_data)
        except:
            self.init_dbus_ifaces()
            try:
                return self.iface.auth_with_policykit(text,action_id,signature_data)
            except:
                return False
        # return True

    def call_copy_file_to_archives(self,app_fullname,src_path,install_appname,need_emit_signal):
        try:
            return self.iface.copy_file_to_install(app_fullname,src_path,install_appname,need_emit_signal)
        except:
            self.init_dbus_ifaces()
            try:
                self.iface.copy_file_to_install(app_fullname, src_path, install_appname, need_emit_signal)
            except:
                pass
            return False

    def call_copy_file_to_apk_tmp(self,app_fullname,src_path,dest_path,install_appname):
        # def copy_apk_to_install(self, app_fullname, src_path,dest_path,install_appname):
        cmd = "mv " + src_path + "/" + app_fullname +" "+ dest_path
        os.system(cmd)
        # self.copy_finish(app_fullname,install_appname)
        self.call_copy_finish(app_fullname,install_appname)
        # return self.iface.copy_apk_to_install(app_fullname, src_path,dest_path,install_appname)
        return

    def call_copy_finish(self,app_fullname,install_appname):
        self.copy_finish.emit(app_fullname,install_appname)

   #
    # 函数：软件状态的信号响应
    #
    def _on_software_fetch_signal(self, type, kwarg):
        sendType = "fetch"
        appname = str(kwarg['download_appname'])
        percent = float(str(kwarg['download_percent']))
        action = str(kwarg['action'])
        sendMsg = AptProcessMsg[str(type)]
        if( type == "down_pulse"):
            sizepercent = str(int(kwarg['download_bytes'])/1024) + "/" + str(int(kwarg['total_bytes'])/1024)
            itempercent = str(int(kwarg['download_items'])) + "/" + str(int(kwarg['total_items']))
            sendMsg = sendMsg + "(" + sizepercent + ")" + "(" + itempercent + ")"
        # if type == "down_fetch":
        #     if (Globals.DEBUG_SWITCH):
        #         LogManager(INFO_LOG,level='info').logger.info(_("downloading"+str(kwarg["uri"])))

        self.dbus_apt_process.emit(appname,sendType,action,percent,sendMsg)

    #
    # 函数：apt调用的返回信号响应
    #
    def _on_software_apt_signal(self,type, kwarg):
        sendType = "apt"
        appname = str(kwarg['apt_appname'])
        sendMsg  = ""
        percent = float(str(kwarg['apt_percent']))
        action = str(kwarg['action'])
        sendMsg = AptActionMsg[action] + AptProcessMsg[str(type)]

        self.dbus_apt_process.emit(appname,sendType,action,percent,sendMsg)


    #
    # 函数：auth信号响应
    #
    def _on_software_auth_signal(self,type, kwarg):

        sendType = "auth"
        appname = str(kwarg['appname'])
        #sendMsg  = "操作取消"
        sendMsg = _("Operation canceled")
        action = str(kwarg['action'])
        if type == "auth_cancel":
            sendType = "cancel"

        self.dbus_apt_process.emit(appname,sendType,action,0,sendMsg)

    #
    # 函数：调用kydroid的dbus接口
    #
    def call_kydroid_dbus_iface(self, funcname, kwargs=None,kwargs2=None,kwargs3=None):
        if self.kydroid_iface is None:
            return None
        func = getattr(self.kydroid_iface,funcname)
        if func is None:
            return None

        res = None
        try:
            res = func(kwargs,kwargs2,kwargs3)
        except dbus.DBusException as e:
            Globals.LOG_ERROR.logger.error("apt-daemon kydroid dbus exception:%s" % str(e))
            return None
        return res

    #
    # 函数：打开软件商店用户手册
    #
    @staticmethod
    def show_user_guide(msg):

        dbus_service = "com.kylinUserGuide.hotel" + "_" + str(os.getuid())
        dbus_path = "/"
        dbus_interface = "com.guide.hotel"

        try:
            message = QtDBus.QDBusMessage.createMethodCall(dbus_service, dbus_path, dbus_interface, "showGuide")
            message << msg
            QtDBus.QDBusConnection.sessionBus().call(message)

        except Exception as e:
            Globals.LOG_ERROR.logger.error("dbus exception:%s" % str(e))
            return False

        return True


if __name__ == "__main__":
    # dbus_service = Dbus_Service()

    Dbus_Service.show_user_guide()

