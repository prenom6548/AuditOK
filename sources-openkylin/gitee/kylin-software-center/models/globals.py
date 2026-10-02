#!/usr/bin/python3
# -*- coding: utf-8 -*-

### BEGIN LICENSE

# Copyright (C) 2013 National University of Defense Technology(NUDT) & Kylin Ltd

# Author:
#     maclin <majun@ubuntukylin.com>
#     Shine Huang<shenghuang@ubuntukylin.com>
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


import os
import platform
from pwd import getpwnam
import getpass
# from models.enums import ERR_LOG,DEBUG_LOG,INFO_LOG

import gettext
gettext.bindtextdomain("kylin-software-center", "/usr/share/locale")
gettext.textdomain("kylin-software-center")
_ = gettext.gettext


class Globals:

    if(platform.machine() == 'mips64' or platform.machine() == 'mips64el'):
        MIPS64 = True
    else:
        MIPS64 = False
    # mainwindow size
    # mainwindow size
    MAIN_WIDTH_NORMAL = 1000
    MAIN_HEIGHT_NORMAL = 710
    MAIN_WIDTH = 1000
    MAIN_HEIGHT = 710

    # card format
    NORMALCARD_WIDTH = 200
    NORMALCARD_HEIGHT = 115

    # how many softwares show in a setp
    SOFTWARE_STEP_NUM = 24

    # uksc launch mode: normal / quiet
    LAUNCH_MODE = 'quiet'

    SEARCH_SOFTWARE = ''

    APPNAME = "kylin-software-center"
    KUM = "kylin-update-manager"
    OLDNAME = "ubuntu-kylin-software-center"
    # uksc version number
    UKSC_VERSION = '1.3.10'

    SERVICE_URL = ""

    PUBLIC_NET_SOURCE = ""

    FIND_PASSWORD = ""

    KYLIN_ID_PATH = ""

    REGISTER_ACCOUNT = ""

    DISCLAIMERS = ""

    KYDROID_SOURCE_SERVER = ""

    # open with local deb file
    LOCAL_DEB_FILE = None

    #LOG.debug and print Switch
    DEBUG_SWITCH = True

    INIT_DATA_OVER = False
    INIT_HOME_PAGE_OVER = False
    SERVER_STATE = True
    # user name
    OS_USER=''
    USER = ''
    USER_DISPLAY = ''
    MODE_PWD_REM = ''
    PASSWORD = ''
    EMAIL = ''
    LAST_LOGIN = ''
    USER_IDEN = ''
    USER_LEVEL = ''
    SET_REM = ''
    SET_SM_REM = ''
    AUTO_LOGIN = ''
    SHOW_LOGIN = 0
    TOKEN = ''

    BLACKLIST = []

    NEED_SHOW_KYLINID = True

    USER_AND_PWD = []  #加密后的账号密码，是字节类型
    USER_AND_PWD_DECODE = []

    ADVANCED_SEARCH = True

    NOWPAGE = -1
    SEARCH_NOWPAGE = -1

    NOW_CATEGORY = _("Feature")
    NOW_KYDROID_CATEGORY = ""

    UPDATE_HOM = 0

    ALL_APPS = {}

    APK_EVNRUN = 0

    MAIN_CHECKBOX = 0

    SOURCE_ITEMWIDTH=0

    CHEK_SAVE_PASSWORD = "0"

    SOURCE_LIST=[]

    list_chk=[]

    Denot = 0
    Dleft=0
    ADS_NUM=0

    LINDIT=0

    REMOVE_SOFT = None

    JUMP_SCRENN=False

    ERAN_SCRENN=0

    # SQLITR_DB=''

    isOnline = True
    apkpagefirst = True

    TASK_LIST=[]

    STOP_DOWNLOAD=False

    LOGIN_SUCCESS=False

    RE_LOGIN = False

    LOGIN_ACCOUNT = ''

    USER_PROFILE = ''

    USER_ID = ""

    ACCOUNT = '' #邮箱

    UPNUM = False

    SORTING_METHOD = 2  #0表示下载排序，1表示评分排序 2表示热度排行

    KYDROID_SORTING_METHOD = 2 #0表示下载排序，1表示评分排序 2表示热度排行

    DATAUNUM=""

    UTFDET = False
    #zouzou
    CHECK_LOGIN_WAY = True

    GET_HOME_PGE_READY = {"AD":False,"NEW":False,"HOT":False,"RANK":False,"RECOMMEND":False,"ANDROID_RECOMMEND":False}

    GET_ALL_APP_LIST = [] #储存数据库中所有普通应用列表

    GET_ALL_DRIVE_APP_LIST = [] #储存数据库中所有驱动应用列表

    GET_ALL_KYDROID_APP_LIST = [] #储存数据库中所有移动应用列表

    UKSC_APPMSG = {}

    GET_RATING_APP_LIST = []

    GET_RATING_DRIVE_APP_LIST = []

    GET_RATING_KYDROID_APP_LIST = []

    RETURN_PATH_LIST = []

    NOW_CARD = []

    ALL_CATEGORY_APP_LIST = {}

    ALL_CATEGORY_APP_LIST_BY_RATING = {}

    ALL_CATEGORY_APP_LIST_BY_POPULARITY = {}

    WIN_REPLACE_LIST = {"Win":[],"Linux":[]}

    MY_APP_LIST = {_("update app"): {"applist":[],"apklist":[]},_("hist app"):{"applist":[],"apklist":[]}}

    RECOMMEND_APP_LIST = ["indicator-china-weather","kylin-video","linuxqq","gimp","sogouimebs","vlc","electronic-wechat","wps-office","gparted"]

    HOT_APP_LIST = ["browser360-cn-stable","edrawmax","edrawproject","fcitx-baidupinyin","qaxbrowser-safe-stable","hardinfo","xmind-vana","youker-assistant","electronic-wechat","uget","qqbrowser","mindmaster"]

    CAN_NOT_REMOVE_SOFT = ["kylin-boot-repair","kylin-calculator","kylin-recorder","kylin-camera","ukui-biometric-manager","yhkylin-backup-tools","kylin-update-manager","kylin-installer","kylin-log-viewer","kylin-service-support","kylin-user-guide","kylin-screenshot","ksc-defender","ukui-system-monitor","box-manager","ukui-clock","ukui-notebook","engrampa","caja"]

    ALL_CATEGORIES = []

    DRIVE_CATEGORIES = []

    APPLICATION_LIST = []

    AD_LIST = []

    CATEGORIES_AD_LIST = {}

    SEARCH_APP_LIST = {"MY":[],"HISTORY":[],"DOWNLOADING":[],"DRIVE":[]}

    FRIST_LEVEL_BTN_SCROLLAREA = {"MY":[],"KYDROID":[],"CATEGORY":[],"DRIVE":[]}

    #0：表示浅色模式 1：表示深色模式 2：表示自动
    THEME_COLOR = 1

    RIGHT_ANGLE = False

    #判断评论页的当前页面，三级评论页面为True
    GO_SHORT_REVIEWS = False
    #为设置深色模式所创建的两个评论模块的很多个标志位。。。。
    #存左右--widget底色
    ADD_WIDGET = []
    #存左右--用户名
    ADD_WIDGET_USERNAME = []
    #存左右--评论时间--目前用不到
    # ADD_WIDGET_DATE = []
    #存左右--评论内容
    ADD_WIDGET_COMMENT = []
    # 存展开--widget底色
    ALL_WIDGET = []
    # 存展开--用户名
    ALL_WIDGET_USERNAME = []
    # 存展开--评论时间--目前用不到
    # ALL_WIDGET_DATE = []
    # 存展开--评论内容
    ALL_WIDGET_COMMENT = []

    HOTLINE = "400-089-1870"
    SUPPORT_MAILBOX = "support@kylinos.cn"
    SALES_MAILBOX = "sales@kylinos.cn"
    MARKET_MAILBOX = "market@kylinos.cn"
    WEBSITE = "www.kylinos.cn"
    ADDRESS =(_( "4th Floor, Gongmei Building, 156 Sanyi Avenue, Kaifu District, Changsha"))
    PHONE = "0731-88280170"
    FAX = "0731-88280166"
    POSTCODE = "410073"
   
    ALL_REVIEWLIST= []

    IS_GETTING_REVIEW = False

    POS = 0
    PKGNAME = []

    MAINWINDOW_WIDTH = 1340

    REMEMBER_WIDTH = 0
    REMEMBER_HEIGHT = 0

    STATUS = 0 #0表示非全屏 1表示全屏

    MINIWIDTH = 1024
    MINIHIGHT = 680

    INIT_WIDTH = 0
    INIT_HEIGHT = 0

    DESKTOP_WIDTH = 0
    DESKTOP_HEIGHT = 0

    DESKTOP_HAS_CHANGE = False

    INIT_DETAIL_WIDTH = 0


    PIDFILE = None

    LOCAL_SERVER = None

    NEED_REFRESH_CACHE = False

    HAVE_THEME = False

    NEED_UKUI_KWIN_VERSION = "1.0.1kylin28"

    APT_LOCK_FILE = "/tmp/lock/kylin-update.lock"

    GET_USER_NAME = getpass.getuser()
    UID = str(getpwnam(GET_USER_NAME).pw_uid)
    LAN = "zh_CN"
    DEVICE = 1

    LOG_INFO = None
    LOG_ERROR = None

    PKG_NAME = {}

    CONNECT_DB = True

    SYSTEM_VERSION = ""
    ARCH = ""
    CPU = ""
    IP = ""
    MAC = ""
    SERIALNUMBER = ""
    ADEV = ""

    IS_INIT_KYDROID_ENVIRONMENT = False

    KYDROID_SOURCE_LIST = {}

    INSTALLED_KYDROID_APK = []

    KYDROID_CATEGORY = []

    ORDINARY_CATEGORY = []

    SEARCH_ASSOCIATION = {}

    UK_SYSTEM_ICON_24_PATH = "/usr/share/icons/ukui-icon-theme-default/24x24/apps/"
    UK_SYSTEM_ICON_48_PATH = "/usr/share/icons/ukui-icon-theme-default/48x48/apps/"
    UK_SYSTEM_ICON_64_PATH = "/usr/share/icons/ukui-icon-theme-default/64x64/apps/"
    UK_SYSTEM_ICON_128_PATH = "/usr/share/icons/ukui-icon-theme-default/128x128/apps/"
    UK_SVG_ICON_PATH = "/usr/share/icons/ukui-icon-theme-default/scalable/apps/"

    INIT_APK_LIST_OVER = False

    AD_EXPOSURE_RATE = {}

    NEW_ARRIVALS_EXPOSURE_RATE = {}

    HOT_EXPOSURE_RATE = {}

    DISPLAY_DOWNLOAD_RANK_NUMBER = 100

    NEED_SEND_EXPOSURE = True

    NEED_SEND_HOT_EXPOSURE = True

    #用于点击驱动中的分类获取对应列表
    DRIVE_CATEGORIES_APP_LIST = {}

    DRIVE_CATEGORIES_APP_LIST_BY_RATING = {}

    KYDROID_APP_LIST = []

    KYDROID_APP_LIST_BY_RATING = []

    KYDROID_PACKAGE_NAME = "kmre"

    NEED_SHOW_KMRE = False

    NEED_SHOW_POPULARITY = False

    KYDROID_APP_LIST_BY_POPULARITY = []

    GET_POPULARITY_DRIVE_APP_LIST = []

    GET_POPULARITY_APP_LIST = []

    STRING = b'TUlJQ1hBSUJBQUtCZ1FDVDE4YmxXYmtLY0hSR2cyZ20zU2xRT3dJcStUTjY5cUErREduWmsxZ1VNdDZmTktaeTBUVExYK3hjMUZMT0lkdDFWWitTTWthb21pZE4wRHh5OElXS3NicSt5aVAxSUxtQ2w3dktITTV6Q0hQakpsWFhyUnRXWTVGQm5xWHBaTWxwaVdMUXVsaWUrYzlJam9SeGo0dnlKa0NvNUV5Qk1aSlliR2l0TGpobHB3SURBUUFCQW9HQUpHMkNZSGlmRTJkcC93WFozOW1XbUdJeHdvenNsbHUyV0JPTzhVRlE3b2txT1dncnF4YkpQMk0vUmhBNjV2ZUlyM3lOZU9SU2JMQ3ZCb2xNbTdXUDhhREFmNkN2SnQ2Rm15NFptbUdEN1NIOGpvZjZqTnJoTDdVaDVZUXFCaEwyeDVxT3VRd2pXanpncjM4QkxjVGJ3RnI3WVdYYVk4RFF3TnRvOE5UUXd1RUNRUURFenR4c3VKclBpcDdyWVBNY3NKM0IyZ21HSGFoOVdRMGNuZWZYczBoOFJsdzc0TUdLdHpmUm56aHgyU0pwbUVVSGZDSFR1MkNJYUhBLzAvOGdFNzF0QWtFQXdFN2QyQVJRSlBuNXZ1d3RBTWNLbzdwLzgyYVcrTDlEeVc0VHZHMnRzNzBJMTdaaHRIbXJJZTFJWFZjVmVTQk0wTFcvQ2dubm1lKzY5VEUrYTNabTR3SkFSOXlveStJVVZSZnVzMCt4TlphZjQvcXpZNDlNeUloTnExZFZJb1BLUWVhSEl5NE1QWlJ0QTFPNkRvZ1VjcTR3OW15ekJWYjBCcHJyOGNKODg3K0JlUUpCQUtieGtEcWYrK2N0cnZaa1Z1a080WmlqSmNKcU0vTjdMdUNsazdLSFVEenhoTVNteW9qKyttWTRkQks3cjZONE85Y1BmK202RVhtY0dLUVVmTWlUZjlzQ1FDenZteStGNnM3dFhsMHNCTW4xbFl2UHpDTGNUTldsZGg4OUh4am0xRkY5alRoa1lIRXdVU0xMd1RNaFdYbi9WZm1ZaDR4MnZoN1FpdGIyNlFZaXpRQT0='