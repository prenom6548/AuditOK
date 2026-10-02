#!/usr/bin/python3
# -*- coding: utf-8 -*-

### BEGIN LICENSE

# Copyright (C) 2013 National University of Defense Technology(NUDT) & Kylin Ltd

# Author:     
#     kobe Lee <xiangli@ubuntukylin.com>
#     maclin <majun@ubuntukylin.com>
# Maintainer:
#     maclin <majun@ubuntukylin.com>

# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 3, as published
# by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranties of
# MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
# PURPOSE.  See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program.  If not, see <http://www.gnu.org/licenses/>.

### END LICENSE

import sys
import os
import signal
import glob
import fcntl
import shutil
# import logging
import tempfile
import subprocess
import re
import dbus
import dbus.service
import sqlite3
import dbus.mainloop.glib
from gi.repository import GObject
from PyQt5.QtCore import *
import apt
import aptsources.sourceslist
import apt_pkg
import time
import fcntl
from kylin_software_center_daemon.aescrypt import *

from kylin_software_center_daemon.apt_daemon import AptDaemon, AppActions
from kylin_software_center_daemon.apt_daemon import WorkitemError

import threading
lock = threading.Lock()
# log = logging.getLogger('Daemon')

INTERFACE = 'com.kylin.softwarecenter'
UKPATH = '/'

HTTP_SOURCE_UBUNTUKYLIN = "http://archive.ubuntukylin.com:10006/ubuntukylin"
DEB_SOURCE_UBUNTUKYLIN = "deb " + HTTP_SOURCE_UBUNTUKYLIN
UBUNTUKYLIN_SOFTWARECENTER_ACTION = 'com.kylin.softwarecenter.action'

LIB_PATH = "/var/lib/"
DPKG_PATH = "dpkg/lock"


class WorkItem:
    def __init__(self, pkgname, action, kwargs):
        self.pkgname = pkgname
        self.action = action
        self.kwargs = kwargs


class WorkThread(threading.Thread):

    def __init__(self, dbus_service):
        threading.Thread.__init__(self)
        self.dbus_service = dbus_service
        self.thread_is_working = 0
        self.uksc_is_working = 0
        self.uksc_exit = False

    def run(self):
        #        print "The backend start the work thread..."
        while (True):
            if self.thread_is_working < 100:
                self.thread_is_working += 1
            else:
                self.thread_is_working = 0
            if len(self.dbus_service.worklist) == 0:
                if(self.uksc_exit == True):
                    self.dbus_service.mainloop.quit()
                    break
                else:
                    time.sleep(0.5)
                    continue

            if is_file_locked(LIB_PATH + DPKG_PATH) is True or 1 == self.uksc_is_working:
                time.sleep(0.5)
                continue

            self.dbus_service.mutex.acquire()
            item = self.dbus_service.worklist.pop(0)  # pop(0) is get first item and remove it from list
            self.dbus_service.mutex.release()
            self.uksc_is_working = 1

            try:
                func = getattr(self.dbus_service.daemonApt, item.action)
                if func is None:
                    if(self.DEBUG_SWITCH):
                        print("Error action: ", item)

                res = func(item.pkgname, item.kwargs)
                if res is False:
                    if(self.DEBUG_SWITCH):
                        print("Action exec failed...")
            except WorkitemError as e:
                # print(e.errornum)
                self.uksc_is_working = 0
                kwarg = {"apt_appname": item.pkgname,
                         "apt_percent": str(-e.errornum),
                         "action": str(item.action),
                         }
                self.dbus_service.software_apt_signal("apt_error", kwarg)
            except:
                self.uksc_is_working = 0
                kwarg = {"apt_appname": item.pkgname,
                         "apt_percent": str(-6.6),
                         "action": str(item.action),
                         }
                self.dbus_service.software_apt_signal("apt_error", kwarg)


#            print "finish one acion....."
# time.sleep(0.5)


def is_file_locked(lockfile):
    """
    Check whether ``apt-get`` or ``dpkg`` is currently active by check the lock file.

    This works by checking whether the lock file like ``/var/lib/
    dpkg/lock``
    ``/var/lib/apt/lists/lock`` is locked by an ``apt-get`` or ``dpkg`` process,
    which in turn is done by momentarily trying to acquire the lock.
     This means that the current process needs to have sufficient privileges.

    :returns: ``True`` when the lock is already taken (``apt-get`` or ``dpkg``
              is running), ``False`` otherwise.
    :raises: :py:exc:`exceptions.IOError` if the required privileges are not
             available.

    .. note:: ``apt-get`` doesn't acquire this lock until it needs it, for
              example an ``apt-get update`` run consists of two phases (first
              fetching updated package lists and then updating the local
              package index) and only the second phase claims the lock (because
              the second phase writes the local package index which is also
              read from and written to by ``dpkg``).
    """
    with open(lockfile, 'w') as handle:
        try:
            fcntl.lockf(handle, fcntl.LOCK_EX | fcntl.LOCK_NB)
            return False
        except IOError:
            return True


class SoftwarecenterDbusService(dbus.service.Object):

    DE_STRING = "LS0tLS1CRUdJTiBQVUJMSUMgS0VZLS0tLS0KTUlHZk1BMEdDU3FHU0liM0RRRUJBUVVBQTRHTkFEQ0JpUUtCZ1FDVDE4YmxXYmtLY0hSR2cyZ20zU2xRT3dJcQorVE42OXFBK0RHblprMWdVTXQ2Zk5LWnkwVFRMWCt4YzFGTE9JZHQxVlorU01rYW9taWROMER4eThJV0tzYnErCnlpUDFJTG1DbDd2S0hNNXpDSFBqSmxYWHJSdFdZNUZCbnFYcFpNbHBpV0xRdWxpZStjOUlqb1J4ajR2eUprQ28KNUV5Qk1aSlliR2l0TGpobHB3SURBUUFCCi0tLS0tRU5EIFBVQkxJQyBLRVktLS0tLQo="

    def __init__(self, bus, mainloop):
        self.DEBUG_SWITCH = False
        self.daemonApt = AptDaemon(self)
        self.bus = bus
        self.bus_name = dbus.service.BusName(INTERFACE, bus=bus)
        #        print "SoftwarecenterDbusService:",self.bus_name
        dbus.service.Object.__init__(self, self.bus_name, UKPATH)
        self.mainloop = mainloop
        self.worklist = []
        self.cancel_name_list = []
        self.mutex = threading.RLock()
        self.cancelmutex = threading.RLock()
        self.worker_thread = WorkThread(self)
        self.worker_thread.setDaemon(True)
        self.worker_thread.start()

    #
    # 函数：操作policykit提权
    #
    @dbus.service.method(INTERFACE, in_signature='sss', out_signature='b', sender_keyword='sender')
    def auth_with_policykit(self, text,get_action_id,signature_data, sender=None):
        res = QByteArray()
        res.append(text)
        res.append(get_action_id)
        cryptographichash = QCryptographicHash(QCryptographicHash.Md5)
        cryptographichash.addData(res)
        md5 = cryptographichash.result()
        aescryptor = Aescrypt()  # ECB模式
        is_verify = aescryptor.add_key_dncrypt(base64.b64decode(self.DE_STRING), signature_data.encode(), md5.data())
        # return True
        if(is_verify == True):
            action = get_action_id
            if text == "":
                text = "要安装或卸载软件"
            if not sender:
                raise ValueError('sender == None')

            #        print "auth_with_policykit:", sender
            granted = False
            try:
                obj = dbus.SystemBus().get_object('org.freedesktop.PolicyKit1',
                                                  '/org/freedesktop/PolicyKit1/Authority')
                policykit = dbus.Interface(obj, 'org.freedesktop.PolicyKit1.Authority')
                subject = ('system-bus-name', {'name': sender})
                flags = dbus.UInt32(1)  # AllowUserInteraction flag
                msg = text + "，您需要进行验证。"
                details = {'polkit.message': msg}
                cancel_id = ''  # No cancellation id
                (granted, notused, details) = policykit.CheckAuthorization(
                    subject, action, details, flags, cancel_id)
            except:
                print("auth with except......")
                granted = False

            return granted
        else:
            return False

    # @dbus.service.method(INTERFACE, in_signature='s', out_signature='b')
    def run_app(self,cmd):
        user = os.popen("who|sed -e 's/[[:space:]][[:space:]]*/ /g'|awk '{print $1}'").readline().strip("\n").strip("\t")
        try:
            os.system("su - "+user+" -c '"+ cmd +"'" )
        except:
            return False
        return True


    #
    # 函数：添加线程任务
    #
    def add_worker_item(self, item):
        if(self.DEBUG_SWITCH):
            print("####add_worker_item:", item)
        self.mutex.acquire()
        self.worklist.append(item)
        self.mutex.release()
        if (self.DEBUG_SWITCH):
            print("####add_worker_item finished!")

    #
    # 函数：删除线程任务
    #
    def del_worker_item_by_name(self, cancelinfo):
        if (self.DEBUG_SWITCH):
            print("####del_worker_item_by_name:", cancelinfo[0])

        del_work_item = None
        self.mutex.acquire()
        try:
            for item in self.worklist:
                if item.pkgname == cancelinfo[0] and item.action == cancelinfo[1]:
                    self.worklist.remove(item)
                    del_work_item = item
                    break
        except:
            pass
        self.mutex.release()

        self.cancelmutex.acquire()
        if del_work_item != None:
            self.cancel_name_list.append(del_work_item)
        self.cancelmutex.release()
        if (self.DEBUG_SWITCH):
            print("####del_worker_item_by_name finished!")

    def check_cancel_worker_item(self, cancelinfo):
        cancel = False
        self.cancelmutex.acquire()
        #        print "check_cancel_worker_item:",len(self.cancel_name_list)

        for item in self.cancel_name_list:
            if item.pkgname == cancelinfo[0] and item.action == cancelinfo[1]:
                self.cancel_name_list.remove(item)
                cancel = True
                break
        self.cancelmutex.release()

        #        print "####check_cancel_worker_item finished!:",cancel
        return cancel

    @dbus.service.method(INTERFACE, in_signature='', out_signature='')
    def uksc_exit(self):
        self.worker_thread.uksc_exit = True

    @dbus.service.method(INTERFACE, in_signature='', out_signature='b')
    def check_apt_lock(self):
        result = os.popen("ps -u root | awk '{print $4}'").readlines()
        for i in result:
            if i.strip("\n") == "apt" or i.strip("\n") == "apt-get" or i.strip("\n") == "dpkg":
                return False
            #当遍历到最后一个时
            if i == result[len(result) - 1]:
                if os.path.exists("/var/cache/apt/archives/lock"):
                    os.remove("/var/cache/apt/archives/lock")
                elif os.path.exists("/var/lib/dpkg/lock"):
                    os.remove("/var/lib/dpkg/lock")
                os.system("dpkg --configure -a")
                os.system("apt-get install -f -y")
                return True


    @dbus.service.method(INTERFACE, in_signature='', out_signature='')
    def exit(self):
        self.mainloop.quit()

    # lixiang: wakeup dbus with timeout
    @dbus.service.method(INTERFACE, in_signature='', out_signature='s')
    def wakeup(self):
        return "kylin"

    # check ubuntukylin source is in /etc/apt/sources.list or not
    #@dbus.service.method(INTERFACE, in_signature='', out_signature='b', sender_keyword='sender')
    # def check_source_ubuntukylin(self, sender=None):
    #     source = aptsources.sourceslist.SourcesList()
    #     for item in source.list:
    #         if (item.str().find(DEB_SOURCE_UBUNTUKYLIN) != -1):
    #             return True
    #     return False

    # add ubuntukylin source in /etc/apt/sources.list
    #@dbus.service.method(INTERFACE, in_signature='s', out_signature='b', sender_keyword='sender')
    # def add_source_ubuntukylin(self, version, sender=None):
    #
    #     granted = self.auth_with_policykit("要增加软件源","com.kylin.softwarecenter.action", sender)
    #     if not granted:
    #         return False
    #
    #     source = aptsources.sourceslist.SourcesList(())
    #     # ????the check option should include version
    #     if (self.check_source_ubuntukylin() is True):
    #         return False
    #     osversion = str(version) + (" main")
    #     source.add("deb", HTTP_SOURCE_UBUNTUKYLIN, osversion, "")
    #     source.save()
    #     return True

    # add source in /etc/apt/sources.list
    #@dbus.service.method(INTERFACE, in_signature='s', out_signature='s', sender_keyword='sender')
    # def add_source(self, text, sender=None):
    #
    #     granted = self.auth_with_policykit("要增加软件源","com.kylin.softwarecenter.action", sender)
    #     if not granted:
    #         return '0'
    #
    #     return self.daemonApt.add_source(text)

    # remove source from /etc/apt/sources.list
    #@dbus.service.method(INTERFACE, in_signature='s', out_signature='b', sender_keyword='sender')
    # def remove_source(self, text, sender=None):
    #
    #     granted = self.auth_with_policykit("","com.kylin.softwarecenter.action", sender)
    #     if not granted:
    #         return False
    #
    #     return self.daemonApt.remove_source(text)

    # check ubuntukylin source is in /etc/apt/sources.list or not
    #@dbus.service.method(INTERFACE, in_signature='b', out_signature='as', sender_keyword='sender')
    # def get_sources(self, except_ubuntu, sender=None):
    #
    #     return self.daemonApt.get_sources(except_ubuntu)

    # -------------------------software-center-------------------------

    # install deb file
    #@dbus.service.method(INTERFACE, in_signature='s', out_signature='b', sender_keyword='sender')
    # def install_debfile(self, path, sender=None):
    #     if (self.DEBUG_SWITCH):
    #         print("####install deb file: ", path)
    #     # path = "".join([chr(character) for character in path]) # add by zhangxin for chinese .deb path 11.19
    #     granted = self.auth_with_policykit("","com.kylin.softwarecenter.action", sender)
    #     if not granted:
    #         kwarg = {"appname": path,
    #                  "action": AppActions.INSTALLDEBFILE,
    #                  }
    #         self.software_auth_signal("auth_cancel", kwarg)
    #         return False
    #
    #     item = WorkItem(path, AppActions.INSTALLDEBFILE, None)
    #     self.add_worker_item(item)
    #     return True

    # install deb file's deps
    #@dbus.service.method(INTERFACE, in_signature='s', out_signature='b', sender_keyword='sender')
    # def install_deps(self, path, sender=None):
    #     if (self.DEBUG_SWITCH):
    #         print("####install deps: ", path)
    #     # path = "".join([chr(character) for character in path])
    #     granted = self.auth_with_policykit("", "com.kylin.softwarecenter.action",sender)
    #     if not granted:
    #         kwarg = {"appname": path,
    #                  "action": AppActions.INSTALLDEPS,
    #                  }
    #         self.software_auth_signal("auth_cancel", kwarg)
    #         return False
    #
    #     item = WorkItem(path, AppActions.INSTALLDEPS, None)
    #     self.add_worker_item(item)
    #     return True

    # install package sa:software_fetch_signal() and software_apt_signal()
    @dbus.service.method(INTERFACE, in_signature='ss', out_signature='b', sender_keyword='sender')
    def install(self, pkgName, signature_data, sender=None):
        res = QByteArray()
        res.append(pkgName)
        cryptographichash = QCryptographicHash(QCryptographicHash.Md5)
        cryptographichash.addData(res)
        md5 = cryptographichash.result()
        aescryptor = Aescrypt()  # ECB模式
        is_verify = aescryptor.add_key_dncrypt(base64.b64decode(self.DE_STRING), signature_data.encode(), md5.data())
        if(is_verify == True):
            if (self.DEBUG_SWITCH):
                print("####install: ", pkgName)

            item = WorkItem(pkgName, AppActions.INSTALL, None)

            self.add_worker_item(item)

            #        self.daemonApt.install_pkg(pkgName)
            if (self.DEBUG_SWITCH):
                print("####install return")
            return True
        else:
            return False

    # uninstall package sa:software_apt_signal()
    @dbus.service.method(INTERFACE, in_signature='ss', out_signature='b', sender_keyword='sender')
    def remove(self, pkgName,signature_data, sender=None):
        res = QByteArray()
        res.append(pkgName)
        cryptographichash = QCryptographicHash(QCryptographicHash.Md5)
        cryptographichash.addData(res)
        md5 = cryptographichash.result()
        aescryptor = Aescrypt()  # ECB模式
        is_verify = aescryptor.add_key_dncrypt(base64.b64decode(self.DE_STRING), signature_data.encode(), md5.data())
        if(is_verify):
            list = []
            user = os.popen("who|sed -e 's/[[:space:]][[:space:]]*/ /g'|awk '{print $1}'").readline()
            if(user.strip("\n").strip(" ").strip("\t") != "root"):
                destFile = "/home/"+user.strip("\n").strip(" ").strip("\t")+"/.cache/uksc/uksc.db"
            else:
                destFile = "/" + user.strip("\n").strip(" ").strip("\t")+"/.cache/uksc/uksc.db"
            try:
                self.connect = sqlite3.connect(destFile, timeout=30.0, check_same_thread=False)
                self.cursor = self.connect.cursor()
                try:
                    lock.acquire(True)
                    self.cursor.execute("select app_name from application")
                    res = self.cursor.fetchall()
                finally:
                    lock.release()
                if len(res) == 0:
                    list = []
                else:
                    list = res
            except Exception as e:
                return False
            in_list = False
            for i in list:
                if(i[0] == pkgName):
                    in_list = True
            if(in_list == False):
                return False
            if (self.DEBUG_SWITCH):
                print("####remove: ", pkgName)

            item = WorkItem(pkgName, AppActions.REMOVE, None)

            self.add_worker_item(item)

            #        self.daemonApt.uninstall_pkg(pkgName)
            if (self.DEBUG_SWITCH):
                print("####remove return")
            return True
        else:
            return False

    # update package sa:software_fetch_signal() and software_apt_signal()
    #@dbus.service.method(INTERFACE, in_signature='s', out_signature='b', sender_keyword='sender')
    # def upgrade(self, pkgName, sender=None):
    #     if (self.DEBUG_SWITCH):
    #         print("####upgrade: ", pkgName)
    #
    #     granted = self.auth_with_policykit("","com.kylin.softwarecenter.action", sender)
    #     if not granted:
    #         kwarg = {"appname": pkgName,
    #                  "action": AppActions.UPGRADE,
    #                  }
    #         self.software_auth_signal("auth_cancel", kwarg)
    #         return False
    #
    #     item = WorkItem(pkgName, AppActions.UPGRADE, None)
    #
    #     self.add_worker_item(item)
    #
    #     #        self.daemonApt.upgrade_pkg(pkgName)
    #     if (self.DEBUG_SWITCH):
    #         print("####upgrade return")
    #     return True

    #@dbus.service.method(INTERFACE, in_signature='as', out_signature='s', sender_keyword='sender')
    # def cancel(self, cancelinfo, sender=None):
    #     if (self.DEBUG_SWITCH):
    #         print("####cancel: ", cancelinfo[0])
    #
    #     granted = self.auth_with_policykit("","com.kylin.softwarecenter.action", sender)
    #     if not granted:
    #         return "False"
    #
    #     self.del_worker_item_by_name(cancelinfo)
    #     if self.check_cancel_worker_item(cancelinfo) is True:
    #         if (self.DEBUG_SWITCH):
    #             print("####cancel return")
    #         return "True"
    #     else:
    #         return "False"

    # apt-get update sa:software_fetch_signal()
    #z@dbus.service.method(INTERFACE, in_signature='b', out_signature='s', sender_keyword='sender')
    # def update(self, quiet, sender=None):
    #     if (self.DEBUG_SWITCH):
    #         print("####update: ")
    #
    #     granted = self.auth_with_policykit("要更新软件源","com.kylin.softwarecenter.action", sender)
    #     if not granted:
    #         return "False"
    #
    #     if is_file_locked("/var/lib/apt/lists/lock") is True:
    #         return "Locked"
    #
    #     kwargs = {"quiet": str(quiet),
    #               }
    #
    #     item = WorkItem("#update", AppActions.UPDATE, kwargs)
    #
    #     self.add_worker_item(item)
    #
    #     #        self.daemonApt.update()
    #     # print "####update return"
    #     return "True"

    # ????????????????????????????
    # apt-get update sa:software_fetch_signal()
    #@dbus.service.method(INTERFACE, in_signature='b', out_signature='s', sender_keyword='sender')
    # def update_first(self, quiet, sender=None):
    #     if (self.DEBUG_SWITCH):
    #         print("####update first: ")
    #
    #     granted = self.auth_with_policykit( "要更新软件源","com.kylin.softwarecenter.action", sender)
    #     if not granted:
    #         return "False"
    #
    #     kwargs = {"quiet": str(quiet),
    #               }
    #
    #     item = WorkItem("#update", AppActions.UPDATE_FIRST, kwargs)
    #
    #     self.add_worker_item(item)
    #
    #     #        self.daemonApt.update()
    #
    #     return "True"

    # check packages status by pkgNameList sa:software_check_status_signal()
    @dbus.service.method(INTERFACE, in_signature='as', out_signature='')
    def check_pkgs_status(self, pkgNameList):
        self.daemonApt.check_pkgs_status_rtn_list(pkgNameList)

    # check one package status by pkgName
    @dbus.service.method(INTERFACE, in_signature='s', out_signature='s')
    def check_pkg_status(self, pkgName):
        return self.daemonApt.check_pkg_status(pkgName)

    # ------------------get info about apt depend----------add 20140926----
    def _common_get_marked_cache(self, pkgName=None, cache=None):
        """"Return the cache that pkg had been marked."""
        if cache is None:
            cache = apt.cache.Cache()
        else:
            cache.clear()

        if pkgName is None:
            return False
        pkg = cache[pkgName]

        if pkg.is_installed:
            pkg.mark_delete()
        else:
            pkg.mark_install()

        return cache

    @dbus.service.method(INTERFACE, in_signature='s', out_signature='i')
    def require_totally_pkg_size(self, pkgName=None):
        """Return the size of the additional reuquired space on the fs."""
        markedCache = self._common_get_marked_cache(pkgName, )
        if not markedCache:
            return False

        # markedCache.clear()
        return markedCache.required_space

    @dbus.service.method(INTERFACE, in_signature='s', out_signature='i')
    def download_totally_pkg_size(self, pkgName=None):
        """Return the size of the package that are required to download."""
        markedCache = self._common_get_marked_cache(pkgName, )
        if not markedCache:
            return False

        return markedCache.required_download

    @dbus.service.method(INTERFACE, in_signature='s', out_signature='')
    def require_totally_pkg_list(self, pkgName=None):
        """Return a dictionary about dep-pkg should been install or delete """
        dic = {"install": [], "update": [], "delete": [], "broken": []}
        markedCache = self._common_get_marked_cache(pkgName, )
        if not markedCache:
            return False

        temp = markedCache.get_changes()
        for i in temp:
            if (i.marked_install):
                dic["install"].append(i.name)
            elif (i.marked_delete):
                dic["delete"].append(i.name)
            elif (i.marked_upgrade):
                if (i.is_installed):
                    dic["update"].append(i.name)
            elif (i.is_inst_broken):
                dic["broken"].append(i.name)
        if (self.DEBUG_SWITCH):
            print(dic)
        # return dic

    # package download status signal
    '''parm mean
        type:
            start:start download
            stop:all work is finish
            done:all items download finished
            fail:download failed
            fetch:one item download finished
            pulse:download status, this msg given a string like dict
        msg:
            a message of type, sometimes is None
    '''

    @dbus.service.signal(INTERFACE, signature='sa{ss}')
    def software_fetch_signal(self, type, msg):
        pass

    @dbus.service.signal(INTERFACE, signature='sa{ss}')
    def software_auth_signal(self, type, msg):
        pass

    # package install/update/remove signal
    '''parm mean
        type:
            start:start work
            stop:work finish
            error:got a error
            pulse:work status, this msg given a string like dict
        msg:
            a message of type, sometimes is None
    '''

    @dbus.service.signal(INTERFACE, signature='sa{ss}')
    def software_apt_signal(self, type, msg):
        pass

    # get packages status signal
    '''parm mean
        dict{packageName, packageStatus}
        packageStatus:
            i:installed
            u:installed and can update
            n:notinstall
    '''

    @dbus.service.signal(INTERFACE, signature='as')
    def software_check_status_signal(self, statusList):
        pass

    @dbus.service.signal(INTERFACE, signature='s')
    def software_signal_test(self, msg):
        pass

    @dbus.service.method(INTERFACE, in_signature='', out_signature='ai')
    def check_work_item(self):
        dpkg_is_running = 0
        workitemcount = 0
        self.mutex.acquire()
        if len(self.worklist) != 0:
            workitemcount = len(self.worklist)
        self.mutex.release()
        dpkg_is_running = is_file_locked(LIB_PATH + DPKG_PATH)
        if dpkg_is_running is True:
            dpkg_is_running = 1
        else:
            dpkg_is_running = 0
        res = [workitemcount, dpkg_is_running]
        return res

    @dbus.service.method(INTERFACE, in_signature='', out_signature='')
    def clear_all_work_item(self):
        self.worklist = []
        self.cancel_name_list = []
        self.worker_thread.uksc_is_working = 0

    #@dbus.service.method(INTERFACE, in_signature='', out_signature='i')
    # def check_dbus_thread_is_working(self):
    #     return self.worker_thread.thread_is_working

    #@dbus.service.method(INTERFACE, in_signature='', out_signature='i')
    # def check_uksc_is_working(self):
    #     return self.worker_thread.uksc_is_working

    @dbus.service.method(INTERFACE, in_signature='', out_signature='')
    def set_uksc_not_working(self):
        self.worker_thread.uksc_is_working = 0

    @dbus.service.method(INTERFACE, in_signature='', out_signature='')
    def exit(self):
        self.mainloop.quit()

    # @dbus.service.method(INTERFACE, in_signature='', out_signature='')
    # def check_dpkg_statu(self):
    #     os.system("dpkg --configure -a")

    # @dbus.service.method(INTERFACE, in_signature='', out_signature='b', sender_keyword='sender')
    # def kydroid_policykit(self, sender=None):
    #     granted = self.auth_with_policykit("移动应用操作","com.kylin.softwarecenter.action", sender)
    #     if not granted:
    #         return False
    #     else:
    #         return True

    @dbus.service.method(INTERFACE, in_signature='sssb', out_signature='')
    def copy_file_to_install(self, app_fullname ,src_path,install_appname,need_emit_signal):
        if(app_fullname.endswith(".deb")):
            dest_path = " /var/cache/apt/archives/"
            cmd = "mv " + src_path + "/"+ app_fullname + dest_path
            os.system(cmd)
            if(need_emit_signal):
                self.copy_finish(app_fullname,install_appname)

    # @dbus.service.method(INTERFACE, in_signature='ssss', out_signature='')
    # def copy_apk_to_install(self, app_fullname, src_path,dest_path,install_appname):
    #     cmd = "mv " + src_path + "/" + app_fullname +" "+ dest_path
    #     os.system(cmd)
    #     self.copy_finish(app_fullname,install_appname)

    #
    # 函数:判断拷贝结束
    #
    @dbus.service.signal(INTERFACE, signature="ss")
    def copy_finish(self, app_fullname,install_appname):
        pass

    # @dbus.service.method(INTERFACE, in_signature='s', out_signature='')
    # def mkdir(self,path):
    #     try:
    #         os.remove(path)
    #     except:
    #         pass
    #     try:
    #         os.mkdir(path)
    #         os.system("chmod 777 "+path)
    #     except:
    #         pass

if __name__ == '__main__':
    os.environ["TERM"] = "xterm"
    os.environ["PATH"] = "/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/X11R6/bin"
    os.environ["DEBIAN_FRONTEND"] = "noninteractive"
    if os.path.exists("/var/lib/apt/lists/lock"):
        os.remove("/var/lib/apt/lists/lock")
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
    GObject.threads_init()
    mainloop = GObject.MainLoop()
    signal.signal(signal.SIGINT, lambda: mainloop.quit())
    SoftwarecenterDbusService(dbus.SystemBus(), mainloop)
    mainloop.run()
