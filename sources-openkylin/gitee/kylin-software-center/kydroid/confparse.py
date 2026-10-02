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

import configparser
from models.apkinfo import ApkInfo
from models.enums import UKSC_CACHE_DIR
from models.globals import *
import os

#FT1500 添加不能使用的安卓应用黑名单
FT1500CPU = False
if os.popen("cat /proc/cpuinfo  |grep -i 'ft.*1500'").read() != '':
    FT1500CPU = True

blacklist = [
    "com.tencent.tmgp.speedmobile",
    "com.smile.gifmaker"
]


#
#设置apk的属性值
#
def set_apk_information(app_dict, apk,appmgr):
    apk.displayname = app_dict["displayname_cn"]
    apk.app_id = app_dict["id"]
    apk.summary = app_dict["summary"]
    apk.description = app_dict["description"]
    apk.ratings_average = app_dict["rating_avg"]
    apk.ratings_total = app_dict["rating_total"]
    apk.review_total = app_dict["review_total"]
    apk.downloadcount = app_dict["download_total"]
    apk.category_name = app_dict["categories"]
    apk.icon_path = app_dict["icon"]
    apk.down_cover_image_path = app_dict["cover_image"]
    apk.down_preview_image_path = app_dict["preview_image"]
    apk.arch = app_dict["arch"]
    apk.system_name = app_dict["system_name"]
    apk.depends = app_dict["depends"]
    apk.certSubjectOu = app_dict["certSubjectOu"]
    apk.softStyle = app_dict["softStyle"]
    apk.manufacturers = app_dict["manufacturers"]
    apk.pinyinName = app_dict["pinyinName"]
    apk.initialsName = app_dict["initialsName"]
    apk.anotherName = app_dict["anotherName"]
    apk.popularity = app_dict["popularity"]
    apk.from_ukscdb = True
    category_list = apk.category_name.split(",")
    try:
        num = 0
        category = category_cn = ""
        for i in category_list:
            # if(not i.isdigit()):
            #     continue
            if (int(i) != 1 and int(i) != 2 and int(i) != 15 and int(i) != 18 and int(i) != 16 and int(i) != 17):
                get_category, get_category_cn = appmgr.get_category_name_by_id(int(i))
                if (num == 0):
                    if (i not in Globals.KYDROID_CATEGORY):
                        Globals.KYDROID_CATEGORY.append(i)
                    category = get_category
                    category_cn = get_category_cn
                    num = num + 1
                else:
                    if (i not in Globals.KYDROID_CATEGORY):
                        Globals.KYDROID_CATEGORY.append(i)
                    category = category + "、" + get_category
                    category_cn = category_cn + "、" + get_category_cn
                    num = num + 1
                # break
            # elif (int(i) == 17):
            #     if (num == 0):
            #         category = "kydroid"
            #         category_cn = "安卓软件"
            #     else:
            #         category = category + "、kydroid"
            #         category_cn = category_cn + "、安卓软件"
            else:
                pass
                # category = category
                # category_cn = category_cn
            # num = num + 1
            if (num > 3):
                break
    except:
        category = category_cn = ""
    apk.category_name = category
    apk.category_name_cn = category_cn
    for i in Globals.INSTALLED_KYDROID_APK:
        if (apk.pkgname == i["appname"]):
            apk.is_installed = True

#
# 函数：获取安卓兼容源列表
#
def getApks(lists_header,appmgr_apk_list,appmgr):
    apklist = []
    ratting_apklist = []
    popularity_apklist = []
    for i in Globals.KYDROID_APP_LIST:
        for pkgname in lists_header.keys():
            if FT1500CPU and pkgname in blacklist :
                continue
            else:
                if (pkgname == i["name"]):
                    in_list = False
                    for get_apk in appmgr_apk_list:
                        if (get_apk.pkgname == pkgname):
                            in_list = True
                            get_apk.file_path = lists_header[pkgname]['file']
                            get_apk.candidate_version = lists_header[pkgname]['version']
                            get_apk.version = lists_header[pkgname]['version']
                            get_apk.size = int(float(lists_header[pkgname]['size']))
                            apklist.append(get_apk)
                            break
                    if(in_list == False):
                        apkinfo = ApkInfo(lists_header[pkgname]["packageName"],lists_header[pkgname]['name'], lists_header[pkgname]['version'], lists_header[pkgname]['size'], lists_header[pkgname]['file'], i['summary'])
                        appmsg = i
                        set_apk_information(appmsg,apkinfo,appmgr)
                        apkinfo.from_ukscdb = True
                        apklist.append(apkinfo)
                    break
    for i in Globals.KYDROID_APP_LIST_BY_RATING:
        for apk in apklist:
            if(i["name"] == apk.name):
                ratting_apklist.append(apk)

    for i in Globals.KYDROID_APP_LIST_BY_POPULARITY:
        for apk in apklist:
            if(i["name"] == apk.name):
                popularity_apklist.append(apk)
    return apklist,ratting_apklist,popularity_apklist


#
# 函数：获取安卓兼容源列表
#
def getApks1():
    config = configparser.ConfigParser()
    config.read(os.path.join(UKSC_CACHE_DIR,"kydroid-sourcelist"), encoding="utf-8")
    lists_header = config.sections()  # 获取所有配置组名：['luzhuo.me', 'mysql'] 注意 不含'DEFAULT'组

    apklist = []
    for pkgname in lists_header:
        if FT1500CPU and pkgname in blacklist :
            continue
        apkinfo = ApkInfo(pkgname, config[pkgname]['name'], config[pkgname]['version'], config[pkgname]['size'], config[pkgname]['file'], config[pkgname]['summary'])
        apkinfo.from_ukscdb = False
        apklist.append(apkinfo)

    # for apk in apklist:
    #     print(apk.pkg_name, apk.display_name, apk.version, apk.size, apk.file_path)

    return apklist


if __name__ == "__main__":
    getApks()
