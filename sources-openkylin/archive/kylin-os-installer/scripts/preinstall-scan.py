# Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, see <http://www.gnu.org/licenses/&gt;.

#!/usr/bin/env python3
# -*- coding:utf-8 -*-

import configparser
import json
import os

Directory = "/cdrom/apps-third/resources"
#Directory = "/home/kylin/apps-third/resources"
auto_cfg = "/var/log/installer/ky-installer.cfg"
#auto_cfg = "ky-installer.cfg"
info_list = "/opt/apps-third/install.list"
#info_list = "install.list"

def get_autocfg_settings():
    applists = []
    config = configparser.ConfigParser()
    config.read(auto_cfg,encoding='utf-8')
    if "preinstallapps" in config.options('config'):
        applists = config.get('config', 'PreinstallApps').strip().split('=', 1)[-1]
        if len(applists) != 0:
            applists = applists.split(',')
    lang = config.get('config', 'language')
    return(applists,lang)

def get_default_apps():
    paths = os.walk(Directory)
    applists = [] 
    for root,dirs,files in paths:
        for appdir in dirs:
            if (appdir):
                apppath = os.path.join(Directory, appdir)
                appcfg = apppath + "/" + appdir + ".cfg"
                f = open(appcfg, 'r')
                datas=f.readlines()
                for data in datas:
                    res = "".join(data)
                    res = res.strip()
                    if res.startswith('Default'):
                        if (res.endswith('yes')):
                            applists.append(appdir)
                f.close()
    return applists

def dump_to_json(applists, lang):
    infolists = dict()
    install_list = []
    for app in applists:
        if os.path.exists(Directory + "/" + app):
            print(app + " app exists")
            name_zh = ""
            name_bo = ""
            name_en = ""
            app_type = ""
            f = open(Directory + "/" + app + "/" + app + ".cfg", 'r')
            datas = f.readlines()
            for data in datas:
                data = data.strip()
                res = "".join(data)
                res = res.strip()
                if res.startswith('Name='):
                    name_zh = res.split('=', 1)[-1].strip()
                elif res.startswith('Name_en='):
                    name_en = res.split('=', 1)[-1].strip()
                elif res.startswith('Name_bo='):
                    name_bo = res.split('=', 1)[-1].strip()
                elif res.startswith('Type='):
                    if res.endswith('deb'):
                        app_type = "1"
                    elif res.endswith('apk'):
                        app_type = "2"
            f.close()
            if app_type and ( name_zh or name_en or name_bo):
                if lang == "bo_CN":
                    if name_bo:
                        app_name = name_bo
                    elif name_zh:
                        app_name = name_zh
                    else:
                        app_name = name_en
                elif lang == "zh_CN":
                    if name_zh:
                        app_name = name_zh
                    else:
                        app_name = name_en
                elif lang == "en_US":
                    if name_en:
                        app_name = name_en
                    else:
                        app_name = name_zh
                infolists.clear()
                infolists.update({'app_name': app_name, 'package_name': app, 'type': app_type})
                install_list.append(infolists.copy())
    print(install_list) 
    with open(info_list, 'a', encoding='utf-8') as f1:
        json.dump(install_list, f1, ensure_ascii=False)

    print("done")

def main():
    applists = []
    applists,lang = get_autocfg_settings()
    if len(applists) == 0:
        applists = get_default_apps()
    print(applists)
    dump_to_json(applists, lang)

if __name__=='__main__':
    print("start preinstall app scans...")
    main()
