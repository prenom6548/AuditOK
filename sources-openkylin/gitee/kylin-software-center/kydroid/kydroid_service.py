#!/usr/bin/python3
# -*- coding: utf-8 -*-

### BEGIN LICENSE

# Copyright (C) 2013 National University of Defense Technology(NUDT) & Kylin Ltd

# Author:
#     Shine Huang<shenghuang@ubuntukylin.com>
# Maintainer:
#     Shine Huang<shenghuang@ubuntukylin.com>

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

from ctypes import *
import json
import os
import xdg.DesktopEntry
import configparser
from models.enums import ERR_LOG,INFO_LOG,DEBUG_LOG,CONF_PATH
from models.globals import Globals
from utils.log_manager import LogManager
import gettext
import datetime
gettext.textdomain("kylin-software-center")
_ = gettext.gettext

# Globals.LOG_INFO = LogManager(INFO_LOG, level='info')
# Globals.LOG_ERROR = LogManager(ERR_LOG, level='error')

class KydroidService:

    libKydroid = None
    hasKydroid = False

    # check if has kydroid
    def check_has_kydroid(self):
        kydroidEnv = os.getenv("ANDROID_ENV")
        if kydroidEnv is not None:
            from ctypes import cdll
            try:
                if os.path.exists('/usr/lib/libkmre.so'):
                    self.libKydroid = cdll.LoadLibrary('/usr/lib/libkmre.so')
                    self.libKydroid.get_installed_applist.restype = c_char_p  # 设置返回值类型为char* 否则默认为int
                    res = self.libKydroid.is_android_env_installed()
                    if (res == True):
                        # need_show_kmre = True
                        # try:
                        #     conf = configparser.ConfigParser()
                        #     conf.read(CONF_PATH)
                        #     startup_time = os.popen("date -d \"$(awk -F. '{print $1}' /proc/uptime) second ago\" +\"%Y-%m-%d %H:%M:%S\"").readline().strip("\n")
                        #     install_kmre = ""
                        #     try:
                        #         install_kmre = conf.get("essential-information", "install_kmre")
                        #     except:
                        #         pass
                        #     if (install_kmre != ""):
                        #         format_pattern = "%Y-%m-%d %H:%M:%S"
                        #         difference = (datetime.datetime.strptime(startup_time,
                        #                                                  format_pattern) - datetime.datetime.strptime(
                        #             install_kmre, format_pattern))
                        #         if (difference.days < 0):
                        #             need_show_kmre = False
                        # except:
                        #     pass
                        # if(need_show_kmre == True):
                        #     self.hasKydroid = True
                        # else:
                        #     self.hasKydroid = False
                        self.hasKydroid = True
                    else:
                        self.hasKydroid = False
                else:
                    if os.path.exists('/usr/lib/libkydroidrequest.so'):
                        self.libKydroid = cdll.LoadLibrary('/usr/lib/libkydroidrequest.so')
                        self.libKydroid.get_installed_applist.restype = c_char_p #设置返回值类型为char* 否则默认为int
                        self.hasKydroid = True
                    else:
                        self.hasKydroid = False
            except:
                if os.path.exists('/usr/lib/libkmre.so'):
                    self.libKydroid = cdll.LoadLibrary('/usr/lib/libkmre.so')
                    self.libKydroid.get_installed_applist.restype = c_char_p  # 设置返回值类型为char* 否则默认为int
                    need_show_kmre = True
                    try:
                        conf = configparser.ConfigParser()
                        conf.read(CONF_PATH)
                        startup_time = os.popen(
                            "date -d \"$(awk -F. '{print $1}' /proc/uptime) second ago\" +\"%Y-%m-%d %H:%M:%S\"").readline().strip(
                            "\n")
                        install_kmre = ""
                        try:
                            install_kmre = conf.get("essential-information", "install_kmre")
                        except:
                            pass
                        if (install_kmre != ""):
                            format_pattern = "%Y-%m-%d %H:%M:%S"
                            difference = (datetime.datetime.strptime(startup_time,
                                                                     format_pattern) - datetime.datetime.strptime(
                                install_kmre, format_pattern))
                            if (difference.days < 0):
                                need_show_kmre = False
                    except:
                        pass
                    if (need_show_kmre == True):
                        self.hasKydroid = True
                    else:
                        self.hasKydroid = False
                    # self.hasKydroid = True
                else:
                    if os.path.exists('/usr/lib/libkydroidrequest.so'):
                        self.libKydroid = cdll.LoadLibrary('/usr/lib/libkydroidrequest.so')
                        self.libKydroid.get_installed_applist.restype = c_char_p #设置返回值类型为char* 否则默认为int
                        self.hasKydroid = True
                    else:
                        self.hasKydroid = False
        else:
            self.hasKydroid = False

    # launch kydroid app
    def launch_app(self, appname):
        fullcmd = ""
        pkgname = appname
        # pkgname = c_char_p(bytes(appname, 'utf-8'))
        # a = self.libKydroid.launch_app(p)
        #获取desktop文件路径
        user_desktop_path = os.path.join(os.path.expanduser("~"), ".local", "share", "applications")
        if(os.path.exists(user_desktop_path) == False):
            #user_desktop_path = os.path.join(os.path.expanduser("~"), '桌面')
            user_desktop_path = os.path.join(os.path.expanduser("~"),_("Desktop"))

        desktopfile = user_desktop_path + "/" + pkgname + ".desktop"
        if os.path.exists(desktopfile):
            DeskTopEntry = xdg.DesktopEntry.DesktopEntry(desktopfile)
            fullcmd = DeskTopEntry.getExec()
            if (Globals.DEBUG_SWITCH):
                #print("launch app: ", fullcmd)
                Globals.LOG_INFO.logger.info(_("launch app: "+str(fullcmd)))
            # try:
            #     os.system(fullcmd + " &")
            #     return True
            # except Exception as detail:
            #     if (Globals.DEBUG_SWITCH):
            #         #print(detail)
            #         Globals.LOG_INFO.logger.info(_(str(detail)))
            #     return False
        return fullcmd

    # install kydroid apk
    #com.xxx.apk
    #apkname:com.xxx.apk, appname:com.xxx pkgname:xxx
    def install_app(self, apkname, appname,pkgname=None):
        apk = c_char_p(bytes(apkname, 'utf-8'))
        app = c_char_p(bytes(appname, 'utf-8'))
        if(pkgname == None):
            a = self.libKydroid.install_app(apk, app)
        else:
            pkg = c_char_p(bytes(pkgname,'utf-8'))
            a = self.libKydroid.install_app(apk, app, pkg)
        if (Globals.DEBUG_SWITCH):
           # print("install apk: ", apkname,appname,a)
            Globals.LOG_INFO.logger.info(_("install apk: "+str(apkname)+";"+str(appname)+";"+str(pkgname)+";"+str(a)))
        return a

    # uninstall kydroid app
    def uninstall_app(self, appname):
        p = c_char_p(bytes(appname, 'utf-8'))
        a = self.libKydroid.uninstall_app(p)
        if (Globals.DEBUG_SWITCH):
            #print("uninstall app: ", a)
            Globals.LOG_INFO.logger.info(_("uninstall app: "+str(a)))
        return a

    # get installed kydroid app list
    def get_installed_applist(self):
        a = self.libKydroid.get_installed_applist()
        rtnstr = a.decode("utf-8")
        rtnobj = json.loads(rtnstr)
        #for obj in rtnobj:
            #print(obj['app_name'])
            #print(obj['package_name'])
            #print(obj['app_info'])
        return rtnobj


if __name__ == "__main__":
    ks = KydroidService()
    ks.check_has_kydroid()
    if (Globals.DEBUG_SWITCH):
        #print(ks.hasKydroid)
        Globals.LOG_INFO.logger.info(_(str(ks.hasKydroid)))
