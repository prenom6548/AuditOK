#!/usr/bin/python3
# -*- coding: utf-8 -*-

import sys
# reload(sys)
# sys.setdefaultencoding('utf8')
#from gevent import monkey;monkey.patch_all()
from PyQt5.QtCore import QThread
from PyQt5.QtCore import pyqtSignal
import locale
from service.appmanager import *
from service.dbus_service import *
import time
from models.enums import KYDROID_DOWNLOAD_PATH
import threading
import gettext
gettext.bindtextdomain("kylin-software-center", "/usr/share/locale")
gettext.textdomain("kylin-software-center")
_ = gettext.gettext

class  init_dbus_Thread(QThread):
    dbus_work_list = []
    dbus_init_over = pyqtSignal(bool)
    get_dbus_workitem_over = pyqtSignal(int)
    def __init__(self,mw):
        super(init_dbus_Thread, self).__init__()
        self.mw = mw
        self.backend = Dbus_Service(True)
        self.get_dbus_workitem_over.connect(self.mw.slot_get_dbus_workitem_over)
        self.backend.check_apt_lock_finish.connect(self.mw.slot_check_apt_lock_finish)
        # self.dbus_init_over.connect(self.mw.slot_init_dbus_over)
        # res = self.backend.init_dbus_ifaces()
        # self.dbus_init_over.emit(res)
        # self.backend.copy_finish.connect(self.mw.slot_copy_file_finish)
        # self.backend.dbus_apt_process.connect(self.mw.slot_dbus_apt_process)
        # self.mutex = threading.RLock()

    def run(self):
        # init dbus backend
        #初始化dbus后台
        self.dbus_init_over.connect(self.mw.slot_init_dbus_over)
        res = self.backend.init_dbus_ifaces()
        self.dbus_init_over.emit(res)
        self.backend.copy_finish.connect(self.mw.slot_copy_file_finish)
        self.backend.dbus_apt_process.connect(self.mw.slot_dbus_apt_process)
        self.mutex = threading.RLock()
        # if (os.path.exists(KYDROID_DOWNLOAD_PATH)):
        #     if (os.path.isdir(KYDROID_DOWNLOAD_PATH)):
        #         pass
        #     else:
        #         action = {"action": "mkdir_kydroid_download_path","path":KYDROID_DOWNLOAD_PATH}
        #         self.mutex.acquire()
        #         self.dbus_work_list.append(action)
        #         self.mutex.release()
        # else:
        #     action = {"action": "mkdir_kydroid_download_path", "path": KYDROID_DOWNLOAD_PATH}
        #     self.mutex.acquire()
        #     self.dbus_work_list.append(action)
        #     self.mutex.release()
        while(True):
            if len(self.dbus_work_list) == 0:
                time.sleep(0.2)
                continue
            self.mutex.acquire()
            item = self.dbus_work_list.pop(0)
            self.mutex.release()
            if (item["action"] == "install"):
                self.backend.install_package(item["appname"])
            # elif(item["action"] == "mkdir_kydroid_download_path"):
            #     self.backend.mk_dir(item["path"])
            elif (item["action"] == "remove"):
                self.backend.remove_package(item["appname"])
            elif (item["action"] == "raising rights"):
                self.backend.call_auth_with_policykit(item["appname"],item["action_id"])
            elif (item["action"] == "clear_dbus_worklist"):
                self.backend.clear_dbus_worklist()
            elif (item["action"] == "check_work_item"):
                res = self.backend.check_dbus_workitem()
                self.get_dbus_workitem_over.emit(res[0])
            elif(item["action"] == "check_apt_lock"):
                self.backend.call_check_apt_lock(item["appname"],item["app_action"],item["percent"])
            elif (item["action"] == "install_apk"):
                self.mw.at.appmgr.start_download_apk(item["app"])
            elif (item["action"] == 'run'):
                self.backend.run_app(item["cmd"])
            time.sleep(0.2)
