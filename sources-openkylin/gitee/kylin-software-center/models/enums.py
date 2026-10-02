
import os
import pwd
from PyQt5.QtGui import *
from PyQt5.QtCore import *
import dbus.service
import sys
from models.globals import Globals
from utils.log_manager import LogManager
import getpass
import uuid
import json
from xdg import BaseDirectory as xdg
import configparser
import gettext
gettext.textdomain("kylin-software-center")
_ = gettext.gettext

Specials = ["\"%c\"", "%f","%F","%u","%U","%d","%D","%n","%N","%i","%c","%k","%v","%m","%M", "-caption", "/bin/sh", "sh", "-c", "STARTED_FROM_MENU=yes"]

#鼠标状态
BUTTON_LEAVE = 0
BUTTON_ENTER = 1
BUTTON_PRESSED = 2
BUTTON_DISABLE = 3

###############################安装的目录####################

UBUNTUKYLIN_RES_PATH = (os.path.abspath(os.path.curdir) + "/res/")
UBUNTUKYLIN_DATA_PATH = (os.path.abspath(os.path.curdir) + "/data/")
CACHE_DIR = xdg.xdg_cache_home
UKSC_CACHE_DIR = os.path.join(xdg.xdg_cache_home, "uksc")
if not os.path.exists(UKSC_CACHE_DIR):
        try:
            os.makedirs(UKSC_CACHE_DIR)
        except OSError as e:
            pass
            # if e.errno == errno.EEXIST:
            #     # it seems that another process has already created this
            #     # directory in the meantime, that's ok
            #     pass
            # else:
            #     # the error is due to something else, so we want to raise it
            #     raise
else:
    if(os.path.isdir(UKSC_CACHE_DIR) == False):
        try:
            os.remove(UKSC_CACHE_DIR)
            os.makedirs(UKSC_CACHE_DIR)
        except OSError as e:
            pass

UKSC_CACHE_DIR_LOG = os.path.join(UKSC_CACHE_DIR,"log")
if not os.path.exists(UKSC_CACHE_DIR_LOG):
        try:
            os.makedirs(UKSC_CACHE_DIR_LOG)
        except OSError as e:
            pass

#日志文件
ERR_LOG = os.path.join(UKSC_CACHE_DIR_LOG,"kylin-software-center-error.log")
INFO_LOG = os.path.join(UKSC_CACHE_DIR_LOG,"kylin-software-center-info.log")
DEBUG_LOG = os.path.join(UKSC_CACHE_DIR_LOG,"kylin-software-center-debug.log")
Globals.LOG_INFO = LogManager(INFO_LOG, level='info')
Globals.LOG_ERROR = LogManager(ERR_LOG, level='error')

#图片缓存路径
UBUNTUKYLIN_CACHE_ICON_PATH = os.path.join(UKSC_CACHE_DIR, "icons/")
UBUNTUKYLIN_CACHE_SETADS_PATH =os.path.join(UKSC_CACHE_DIR, "ads/")
UBUNTUKYLIN_CACHE_COVERIMAGE_PATH = os.path.join(UKSC_CACHE_DIR,"coverimage/")
UBUNTUKYLIN_CACHE_NEW_ARRIVALS_PATH = os.path.join(UKSC_CACHE_DIR,"new_arrivals/")
UBUNTUKYLIN_CACHE_PREVIEWIMAGE_PATH = os.path.join(UKSC_CACHE_DIR,"previewImage/")

UBUNTUKYLIN_RES_ICON_PATH = UBUNTUKYLIN_DATA_PATH + "icons/"
UBUNTUKYLIN_RES_COVERIMAGE_PATH = UBUNTUKYLIN_DATA_PATH + "coverimage/"
UBUNTUKYLIN_RES_PREVIEWIMAGE_PATH = UBUNTUKYLIN_DATA_PATH + "previewImage/"
UBUNTUKYLIN_RES_NEW_ARRIVALS_PATH = UBUNTUKYLIN_DATA_PATH + "new_arrivals/"
UBUNTUKYLIN_RES_AD_PATH = UBUNTUKYLIN_DATA_PATH + "ads/"

SVG_ICON_PATH = "/usr/share/icons/hicolor/scalable/apps/"
ICON_48_PATH = "/usr/share/icons/hicolor/48x48/"
ICON_64_PATH = "/usr/share/icons/hicolor/64x64/"
ICON_128_PATH = "/usr/share/icons/hicolor/128x128/"

#商业版系统默认图标路径
KYLIN_SYSTEM_ICON_24_PATH = "/usr/share/icons/kylin-icon-theme/24x24/apps/"
KYLIN_SYSTEM_ICON_48_PATH = "/usr/share/icons/kylin-icon-theme/48x48/apps/"
KYLIN_SYSTEM_ICON_64_PATH = "/usr/share/icons/kylin-icon-theme/64x64/apps/"
KYLIN_SYSTEM_ICON_128_PATH = "/usr/share/icons/kylin-icon-theme/128x128/apps/"
KYLIN_SVG_ICON_PATH = "/usr/share/icons/kylin-icon-theme/scalable/apps/"
#社区版系统默认图标路径
UK_SYSTEM_ICON_24_PATH = "/usr/share/icons/ukui-icon-theme-default/24x24/apps/"
UK_SYSTEM_ICON_48_PATH = "/usr/share/icons/ukui-icon-theme-default/48x48/apps/"
UK_SYSTEM_ICON_64_PATH = "/usr/share/icons/ukui-icon-theme-default/64x64/apps/"
UK_SYSTEM_ICON_128_PATH = "/usr/share/icons/ukui-icon-theme-default/128x128/apps/"
UK_SVG_ICON_PATH = "/usr/share/icons/ukui-icon-theme-default/scalable/apps/"

UBUNTUKYLIN_CACHE_SETSCREENSHOTS_PATH=os.path.join(UKSC_CACHE_DIR, "previewImage/")
if(os.path.exists(UBUNTUKYLIN_CACHE_SETSCREENSHOTS_PATH)):
    pass
else:
    if(os.path.exists(UKSC_CACHE_DIR)):
        os.makedirs(UBUNTUKYLIN_CACHE_SETSCREENSHOTS_PATH)
    else:
        os.makedirs(UKSC_CACHE_DIR)
        os.makedirs(UBUNTUKYLIN_CACHE_SETSCREENSHOTS_PATH)

UBUNTUKYLIN_RES_SCREENSHOT_PATH = os.path.join("/usr/share/kylin-software-center/data/", "previewImage/")
UBUNTUKYLIN_USER_TOKEN_PATH = os.path.join(CACHE_DIR, "kylinId/")

UBUNTUKYLIN_HTTP_WIN_RES_PATH = os.path.join(UKSC_CACHE_DIR,"uk-win/")
UBUNTUKYLIN_RES_WIN_PATH = UBUNTUKYLIN_DATA_PATH + "winicons/"

WGET_DOWN_PATH = os.path.join(os.path.expandvars('$HOME'), ".cache/uksc/wget_down")
STORE_DEB_PATH = os.path.join(os.path.expandvars('$HOME'), ".cache/uksc/wget_down/deb")

KWIN_CONF_PATH = os.path.join(os.path.expandvars('$HOME'), ".config/ukui-kwinrc")
KYLIN_ID_CONF_PATH = os.path.join(os.path.expandvars('$HOME'), ".config/ukui/kylin-id.conf")

CONF_PATH =UKSC_CACHE_DIR + "/kylin-software-center.conf"
if( not os.path.exists(CONF_PATH)):
    os.system("cp /usr/share/kylin-software-center/kylin-software-center.conf "+UKSC_CACHE_DIR)
fd = open(CONF_PATH,"r")
re = fd.readlines()
theme = False
update = False
for i in re:
    if("[theme-color]" in i):
        theme = True
    if("[update-data]" in i):
        update = True
if(theme == False or update == False):
    os.popen("cp /usr/share/kylin-software-center/kylin-software-center.conf " + UKSC_CACHE_DIR)


# 安卓兼容源自动判断
kydroid_source = {
    "V10":{
        "kydroid2": {
            "amd64":"http://archive.kylinos.cn/kylin/kydroid/2/x86/",
            "arm64":"http://archive.kylinos.cn/kylin/kydroid/2/arm64/"
        },
        "kydroid3": {
            # "amd64":"http://archive.kylinos.cn/kylin/kydroid/3/x86/",
            "arm64":"http://archive.kylinos.cn/kylin/kydroid/3/arm64/",
            "kunpeng":"http://archive.kylinos.cn/kylin/kydroid/3/kunpeng/",
            "other":"http://archive.kylinos.cn/kylin/kydroid/3/other/"  #709和景嘉微显卡源
        },
        "kydroid4": {
            "amd64":"http://archive.kylinos.cn/kylin/kydroid/4/x86/",
        },
        "kmre2": {
            # "amd64":"http://archive.kylinos.cn/kylin/kydroid/3/x86/",
            "arm64": "http://archive.kylinos.cn/kylin/kmre/2/arm64/",
            "kunpeng": "http://archive.kylinos.cn/kylin/kmre/2/kunpeng/",
            "other": "http://archive.kylinos.cn/kylin/kmre/2/other/"  # 709和景嘉微显卡源
        }
    },
    "V101":{
        "kydroid3": {
            # "amd64":"http://archive.kylinos.cn/kylin/kydroid/3/x86/",
            "arm64":"http://archive.kylinos.cn/kylin/kydroid/3/101/arm64/",
            "kunpeng":"http://archive.kylinos.cn/kylin/kydroid/3/101/kunpeng/",
            "other":"http://archive.kylinos.cn/kylin/kydroid/3/101/other/",  #709和景嘉微显卡源
            "kyrin990":"http://archive.kylinos.cn/kylin/kydroid/3/101/kyrin990/"
        },
        "kmre2": {
            # "amd64":"http://archive.kylinos.cn/kylin/kydroid/3/x86/",
            "arm64": "http://archive.kylinos.cn/kylin/kmre/2/101/arm64/",
            "kunpeng": "http://archive.kylinos.cn/kylin/kmre/2/101/kunpeng/",
            "other": "http://archive.kylinos.cn/kylin/kmre/2/101/other/"  # 709和景嘉微显卡源
        }
    }
}

#判断和兼容安卓兼容版本
if(os.path.isfile('/usr/share/kydroid/kydroid.conf') and os.path.exists('/usr/lib/libkydroidrequest.so')): # kydroid3和之后版本
    KYDROID_VERSION = "kydroid"
    KYDROID_VERSION_D = "Kydroid"
    KYDROID_CONF_PATH = "/usr/share/kydroid/kydroid.conf"
elif(os.path.isfile('/usr/share/kmre/kmre.conf') and os.path.exists('/usr/lib/libkmre.so')):
    KYDROID_VERSION = "kmre"
    KYDROID_VERSION_D = "Kmre"
    KYDROID_CONF_PATH = "/usr/share/kmre/kmre.conf"
else:
    KYDROID_VERSION = "kydroid2"
    KYDROID_VERSION_D = "Kydroid2"
    KYDROID_CONF_PATH = "/usr/share/kydroid2/kydroid2.conf"

system_version =  ""
CPU = ""
GPU = ""
try:
    try:
        bus = dbus.SessionBus()
    except:
        sys.exit(0)
    if (os.path.exists("/usr/share/kmre/kmre.conf")):
        proxy_obj = bus.get_object('cn.kylinos.Kmre.Manager', '/cn/kylinos/Kmre/Manager')
        iface = dbus.Interface(proxy_obj, 'cn.kylinos.Kmre.Manager')
    else:
        proxy_obj = bus.get_object('com.kylin.kydroid', '/com/kylin/kydroid')
        iface = dbus.Interface(proxy_obj, 'com.kylin.kydroid')
    dest = iface.getDisplayInformation()
    adev=json.loads(dest)
    # arch = os.popen("dpkg --print-architecture").readline().splitlines()[0]
    kydroid_config = configparser.ConfigParser()
    if "gpu_vendor" in adev.keys() and "gpu_model" in adev.keys():
        GPU = adev["gpu_vendor"]
        if (GPU == "JJM" and "gpu_model" in adev):
            if (adev["gpu_model"] == "JJM_7200"):
                GPU = "JJM"
            else:
                GPU = ""
        elif (GPU == "GP101" and "gpu_model" in adev):
            if (adev["gpu_model"] == "GP101_0001"):
                GPU = "GP101"
            else:
                GPU = ""
        if("cpu_type" in adev.keys()) :
            CPU = adev["cpu_type"]
    if "gpu_vendor" in adev:
        if(GPU == ""):
            GPU = adev["gpu_vendor"]
        # if arch == "amd64":
        #     pass
        # else:
        #     if adev["gpu_vendor"] == "AMD" or adev["gpu_vendor"] == "NVIDIA":
        #         arch = "arm64"
        #     elif adev["gpu_vendor"] == "GP101" or adev["gpu_vendor"] == "JJM" or adev["gpu_vendor"] == "MALI" or adev[
        #         "gpu_vendor"] == "VIRTUAL" or adev["gpu_vendor"] == "UNKNOWN" or adev["gpu_vendor"] == "ZC716":
        #         arch = "other"

        Globals.ADEV = adev["gpu_vendor"]
    # if ((os.popen("lspci -n|awk '{print $3}' |grep '0709:'").read() != '') or (
    #         os.popen("cat /proc/fb |grep -i MWV206").read() != '')):  # 709和景嘉微特殊处理
    #     arch = "other"
    # elif (os.popen("lscpu|grep -i kunpeng").read() != ''):
    #     arch = "kunpeng"
    # elif (CPU == "HUAWEI_KIRIN990"):
    #     arch = "kirin990"
    # read_system_version = os.popen("lsb_release -d").readline()
    # res_list = read_system_version.split(":")
    # res_list1 = res_list[1].strip("\t").strip("\n").strip(" ")
    # res_list2 = res_list1.split("Kylin")
    # res = res_list2[1].strip(" ").strip("\n").strip("\t")
    # system_version = res

    kydroid_config.read(KYDROID_CONF_PATH)
    kydroid_version = kydroid_config['image']['repo']
    #传参：系统版本、安卓版本、架构给服务器获取返回
    # SYSTEM_VERSION = system_version
    GET_KYDROID_VERSION = kydroid_version
    # KYDROID_ARCH = arch
    # KYDROID_SOURCE_SERVER = kydroid_source[system_version][kydroid_version][arch]
except:
    KYDROID_SOURCE_SERVER = ""
    # SYSTEM_VERSION = "1"
    GET_KYDROID_VERSION = "1"
    # KYDROID_ARCH = "1"
    # caoliang
    UBUNTUKYLIN_ARCH = "all"

SOFTWARE_BLACK ="/home/"+getpass.getuser()+"/"+".config/kylin-software-center-security-config.json"

def software_center_noshow():
    try:
        blacklist=[]
        file =open(SOFTWARE_BLACK,"r")
        dest=json.load(file)
        ntdit=dest['ukui-kylin-software-center']['blacklist']
        for i in ntdit:
            blacklist.append(i['pkgname'])
        return blacklist
    except:
        return []

Globals.BLACKLIST = software_center_noshow()

###############################缓存路径###############################

#KYDROID_DOWNLOAD_PATH = "/var/lib/kydroid/kydroid2-1000-kylin/data/local/tmp"
if(os.path.exists("/usr/share/kmre/kmre.conf")):
    KYDROID_DOWNLOAD_PATH = "/var/lib/kmre/" + KYDROID_VERSION + "-" + str(os.getuid()) + "-" + str(pwd.getpwuid(os.getuid())[0]) + "/data/local/tmp/"
    KYDROID_STARTAPP_ENV = "/usr/bin/startapp start_kmre"
else:
    KYDROID_DOWNLOAD_PATH = "/var/lib/kydroid/" + KYDROID_VERSION + "-" + str(os.getuid()) + "-" + str(pwd.getpwuid(os.getuid())[0]) + "/data/local/tmp/"
    KYDROID_STARTAPP_ENV = "/usr/bin/startapp start_kydroid"

KYDROID_INSTALLED_FILENAME = KYDROID_DOWNLOAD_PATH +"/installed.json"





########################################网址类###########################

UBUNTUKYLIN_SERVICE_PATH = "com.kylin.softwarecenter"
UBUNTUKYLIN_INTERFACE_PATH = "com.kylin.softwarecenter"

UBUNTUKYLIN_SERVER = "http://service.ubuntukylin.com:8001/uksc/"
UK_APP_ICON_URL = ("http://service.ubuntukylin.com:8001/"
    "media/%(pkgname)s")

# screenshot handling
SCREENSHOT_THUMB_URL = ("http://screenshots.ubuntu.com/"
    "thumbnail-with-version/%(pkgname)s/%(version)s")
SCREENSHOT_LARGE_URL = ("http://screenshots.ubuntu.com/"
    "screenshot-with-version/%(pkgname)s/%(version)s")


RESOURCE_SERVER="http://archive.kylinos.cn/kylin/resources/screenshots/"

#####################################样式###################################
ITEM_LABEL_STYLE = ("QLabel{background-image:url(%s);background-color:transparent;}")


#####################################类###################################
class Signals:
    # from models.application import Application
    init_models_ready = pyqtSignal(str,str)

    download_ranking_ready = pyqtSignal(list)
    hot_ready = pyqtSignal(list,list)
    get_ad_ready = pyqtSignal(list,list)
    get_new_arrivals_ready = pyqtSignal(list,list)
    get_android_recommend_ready = pyqtSignal(list,list)
    get_win_replace_ready = pyqtSignal(list,list)
    recommend_ready = pyqtSignal(list,bool,bool)
################
    app_reviews_ready = pyqtSignal(int,list,str)

    get_reviews = pyqtSignal(str)

    # caoliang login signals
    ui_login = pyqtSignal(list)
    get_ui_login_over = pyqtSignal(list)
    get_ui_login_over_fail = pyqtSignal(bool)
    ui_login_success = pyqtSignal()
    save_usrname_password = pyqtSignal(str,str,str)

    #zou
    ui_login_mouse_current_position = pyqtSignal(int)
    #zouzou
    ui_login_get_image_code = pyqtSignal(dict)
    ui_login_image_code_move = pyqtSignal(int)
    #zou手机号
    ui_login_phone_number = pyqtSignal(list)


class AppMsg:
    ID = 0
    APPNAME = ""
    DISPLAY_NAME = ""
    DISPLAY_NAME_CN = ""
    CATEGORIES = ""
    CATEGORIES_CN = ""
    DESCRIPTION = ""
    RATING_AVG = ""
    RATING_TOTAL = 0
    REVIEW_TOTAL = 0
    DOWNLOAD_TOTAL = ""
    VERSION = ""
    IS_FROM_DB = True
    IS_INSTALLED = False
    IS_UPGRADABLE = False
    IS_KYDROID = False
    FILE_PATH = ""
    SIZE = ""
    RUN = None

AptActionMsg = {
    "install_deps":_("Install dependencies"),
    "install_debfile":_("Install local package"),
    "install":_("Software Installation"),
    "remove":_("Software uninstall"),
    "upgrade":_("Software upgrade"),
    "update":_("Source update"),
    "update_first":_("Source initialization")
}

AptProcessMsg = {
    "apt_start":_("Start..."),
    "apt_finish":_("Perfection!"),
    "apt_error":_("Failure!"),
    "apt_pulse":_("Processing"),
    "down_start":_("Download begins"),
    "down_stop":_("Download stopped"),
    "down_done":_("Download completed"),
    "down_fail":_("download failed"),
    "down_fetch":_("Single download completed"),
    "down_pulse":_("Download in progress"),
    "down_cancel":_("Download canceled"),
}





# application actions, this should sync with definition in apt_dbus_service
class AppActions:
    INSTALLDEPS = "install_deps"
    INSTALLDEBFILE = "install_debfile"
    DOWNLOADAPK = "download_apk"
    INSTALL = "install"
    REMOVE = "remove"
    UPGRADE = "upgrade"
    CANCEL = "cancel"
    APPLY = "apply_changes"
    PURCHASE = "purchase"
    UPDATE = "update"
    UPDATE_FIRST = "update_first"
    ADD_SOURCE = "add_source"
    REMOVE_SOURCE = "remove_source"
    GET_SOURCES = "get_sources"
    USECDROM = "usecdrom"
    FIND_UP_SERVER = "find_up_server"

class PageStates:
    (
     HOMEPAGE,
     MYPAGE,
     DOWNLOADINGPAGE,
     SEARCHPAGE,
     WINPAGE,
     CAREGORYPAGE,
     KYDROID,
     HISTORY,
     APPDETAIL,
     MOREPAGE,
     DRIVE
     ) = list(range(11))

class ClickCard:
    (
        NEWARRIVALS,
        AD,
        INSIDECARD,
        HOT
    ) = list(range(4))

# pkg action state constants
class PkgStates:
    (
    # current
    INSTALLED,
    UNINSTALLED,
    UPGRADABLE,
    REINSTALLABLE,
    # progress
    INSTALLING,
    REMOVING,
    UPGRADING,
    ENABLING_SOURCE,
    INSTALLING_PURCHASED,
    # special
    NEEDS_SOURCE,
    NEEDS_PURCHASE,
    PURCHASED_BUT_REPO_MUST_BE_ENABLED,
    ERROR,
    FORCE_VERSION,
    # the package is not found in the DB or cache
    NOT_FOUND,
    # its purchased but not found for the current series
    PURCHASED_BUT_NOT_AVAILABLE_FOR_SERIES,
    # this *needs* to be last (for test_appdetails.py) and means
    # something went wrong and we don't have a state for this PKG
    UNKNOWN,
    DOWNLOADING,
    PUASE,
    RUN,
    INSTALL,
    UPDATE,
    UNINSTALL,
    NORUN,
    NOTHING,
    REMOVED,
    ) = list(range(26))

#软件状态
DOWNLOADING = 1
PUASE = 2
INSTALLING = 3
OTHER = 4


def setLongTextToElideFormat(label, text):
    if text[len(text) - 1] == '\n':
        text = text.rstrip()
    metrics = QFontMetrics(label.font())
    elidedText = metrics.elidedText(text, Qt.ElideRight, label.maximumWidth())
    label.setText(elidedText)
    return elidedText

DB_ITEM = {"action":"","appname":""}

get_app_info = {"name":"","displayname":"","displayname_cn":"","categories":"","summary":"","description":"","rating_avg":0,"rating_total":0,"review_total":0,"download_total":0,"from_db":True}

category_dict = {"id":0,"name":"","displayname":""}

PKG_NAME = {}
ARCH = ""
SYSTEM_VERSION = ""
