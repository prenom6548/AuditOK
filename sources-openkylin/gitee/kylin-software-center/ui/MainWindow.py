# -*- coding: utf-8 -*-
import urllib.parse

from PyQt5.QtGui import *
from PyQt5.QtCore import *
from PyQt5 import QtCore, QtGui, QtWidgets
from PyQt5.QtWidgets import *
from service.appmanager import AppManager
from models.enums import STORE_DEB_PATH, WGET_DOWN_PATH, PageStates, KYDROID_STARTAPP_ENV, KYDROID_VERSION, \
    KYDROID_VERSION_D, KYDROID_CONF_PATH, DB_ITEM, PkgStates, KYDROID_DOWNLOAD_PATH, CONF_PATH, PKG_NAME, \
    KWIN_CONF_PATH, SYSTEM_VERSION, ARCH,KYLIN_ID_CONF_PATH
import glob
from dbus.mainloop.glib import DBusGMainLoop

mainloop = DBusGMainLoop(set_as_default=True)
from backend.utildbus import *
from backend.service.save_password import *
from ui.title_bar import *
from ui.detailwidget import *
from ui.card_widget import *
from ui.inside_card_widget import *
from ui.messagebox import *
from ui.download_ranking_card import *
from ui.win_replace_card import *
from service.dbus_service import *
from ui.login import *
from ui.tip_widget import *
from ui.menubar import *
from ui.contact_us_widget import *
from ui.confirmdialog import *
from ui.search_records import *
from ui.configure_widget import *
from ui.comment_pop_up_widget import *
# zouzou
from backend.service.save_password import password_read
from utils.log_manager import LogManager
from models.apkinfo import *
from models.application import *
from utils import run
from utils.access_db_thread import *
from utils.access_dbus_thread import *
from utils.record_history_installation import *
from utils.record_history_installation import *
import os
import fcntl
import sip
import dbus
import re
import sys
import time
import traceback
from concurrent.futures import ThreadPoolExecutor
import math
import base64
import logging
import subprocess
import configparser
from PyQt5.QtNetwork import QHostAddress, QNetworkAccessManager, QNetworkInterface
from utils.log_manager import LogManager
from utils.aescrypt import *
from ui.set_widget import *
import datetime
from gi.repository import Gio
from multiprocessing import Process

# Globals.LOG_INFO = LogManager(INFO_LOG, level='info')
# Globals.LOG_ERROR = LogManager(ERR_LOG, level='error')

import gettext

gettext.bindtextdomain("kylin-software-center", "/usr/share/locale")
gettext.textdomain("kylin-software-center")
_ = gettext.gettext

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    def _fromUtf8(s):
        return s
Left, Top, Right, Bottom, LeftTop, RightTop, LeftBottom, RightBottom = range(8)
pool = ThreadPoolExecutor(max_workers=15)


class MainWindow(QMainWindow):
    Margins = 10
    space = 3
    finish_resize = True
    resize_window = False
    move_window = False
    mouse_press = False
    mouse_release = False
    click_num = 0
    first_show_tip_widget = True
    is_first_click_menubar = True
    is_first_click_second_level_menubar = True
    is_clicked_second_menu = False
    is_first_click_contact_us = True
    is_first_show_widget = True
    is_remove = False
    click_close_btn = False
    is_downloading = False
    remove_app_num = 0
    down_app_msg_dict = []
    remove_app_dict = []
    need_copy_file_list = []
    wait_download_app_list = []
    need_down_file_list = {}
    wait_install_app_list = []
    send_download_progress = pyqtSignal(str, int)
    signal_reload_btn = pyqtSignal(str)
    signal_uninstall = pyqtSignal(str, str)
    copy_file_to_archives = pyqtSignal(str)
    copy_file_to_apklocal = pyqtSignal(str)
    start_install_apk = pyqtSignal(ApkInfo)
    sigenal_show_downloading_widget = pyqtSignal(str, bool, bool)
    show_applist = pyqtSignal(list, QWidget, bool)
    show_inside_applist = pyqtSignal(bool,str,list,list,list, QWidget, QWidget, bool, bool)
    upgrade_app = pyqtSignal(str)
    change_card_state = pyqtSignal(str, int)
    remove_card_signal = pyqtSignal(QWidget, str)
    set_dark_mode = pyqtSignal()
    signal_installed = pyqtSignal(str)
    download_count_change = pyqtSignal(str, int ,str)
    # app_sign_state = pyqtSignal(str, bool)
    show_image = pyqtSignal()
    tip_download_error_signal = pyqtSignal(str,str)
    tip_network_error_signal = pyqtSignal(str)
    get_screen_shot_signal = pyqtSignal(dict)
    set_ad_list_signal = pyqtSignal(list)
    all_continue_download = pyqtSignal(QWidget)
    add_app_to_downloading_widget = pyqtSignal(Application)
    add_apk_to_downloading_widget = pyqtSignal(ApkInfo)
    change_kydroid_card_btn_state = pyqtSignal()
    send_notice_click = pyqtSignal(str)
    app_list_msg = []
    pkg_list_msg = []
    win_list_msg = []
    path_of_particle = []
    now_category = ""
    card_width = 0
    is_downloading_app = False
    init_all_search = True
    add_inside_card_widget = None
    get_inside_list_over = True
    init_widget = False
    #检测安装失败是否需要进行apt修复
    need_check_apt = True

    def __init__(self, parent=None):

        QWidget.__init__(self, parent)
        sys.excepthook = self.global_except_hook
        self.category = ''
        self.Direction = None
        # password_read()
        self.check_conf_path()
        try:
            cf = configparser.ConfigParser()
            cf.read(CONF_PATH)
            try:
                light = "NO"
                dark = "NO"
                auto = "NO"
                light = cf.get("theme-color", "light")
                dark = cf.get("theme-color", "dark")
                auto = cf.get("theme-color", "auto")
            except:
                try:
                    cf.set("theme-color", "light", "YES")
                    with open(CONF_PATH, "w+") as f:
                        cf.write(f)
                    cf.set("theme-color", "dark", "NO")
                    with open(CONF_PATH, "w+") as f:
                        cf.write(f)
                    cf.set("theme-color", "auto", "NO")
                    with open(CONF_PATH, "w+") as f:
                        cf.write(f)
                except:
                    pass
                light = "YES"
            if (light == "YES"):
                Globals.THEME_COLOR = 0
                self.theme_style(False)
            if (dark == "YES"):
                Globals.THEME_COLOR = 1
                self.theme_style(False)
            if (auto == "YES"):
                try:
                    self.theme_style()
                except:
                    Globals.THEME_COLOR = 0
        except:
            Globals.THEME_COLOR = 0
        Globals.LAN = QLocale.system().name()
        Globals.DEVICE = QGuiApplication.primaryScreen().devicePixelRatio()

        # print("start_init_ui", datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
        self.init_ui()
        # print("init_ui_over", datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
        #QApplication.processEvents()
        # print("init_ui_over", datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
        # self.show()
        # print("show_ui_over", datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
        pool.submit(self.detailwidget.init_cache)
        # print("init_cache_over", datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
        self.get_infomation()
        if (Globals.NEED_SHOW_KYLINID):
            self.title_bar.login_btn.show()
        else:
            self.title_bar.login_btn.hide()
        # print("get_infomation_over", datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
        self.show()

    def get_infomation(self):
        try:
            cf = configparser.ConfigParser()
            cf.read(CONF_PATH)
            service_url = cf.get("server-address", "server")
            service_url = cf.get("server-address", "server")
            if (service_url != ""):
                Globals.SERVICE_URL = service_url
            else:
                cf.set("server-address", "server", Globals.SERVICE_URL)
                with open(CONF_PATH, "w+") as f:
                    cf.write(f)
            kylin_id_path = cf.get("server-address", "kylin_id_path")
            if (kylin_id_path != ""):
                Globals.KYLIN_ID_PATH = kylin_id_path
            else:
                cf.set("server-address", "kylin_id_path", Globals.KYLIN_ID_PATH)
                with open(CONF_PATH, "w+") as f:
                    cf.write(f)
            register_account = cf.get("server-address", "register_account")
            if (register_account != ""):
                Globals.REGISTER_ACCOUNT = register_account
            find_pwd = cf.get("server-address", "find_pwd")
            if (find_pwd != ""):
                Globals.FIND_PASSWORD = find_pwd
            kydroid_server = cf.get("server-address", "kydroid_server")
            if (kydroid_server != ""):
                Globals.KYDROID_SOURCE_SERVER = kydroid_server
        except:
            try:
                cf.set("server-address", "kylin_id_path", Globals.KYLIN_ID_PATH)
                with open(CONF_PATH, "w+") as f:
                    cf.write(f)
                cf.set("server-address", "register_account", Globals.REGISTER_ACCOUNT)
                with open(CONF_PATH, "w+") as f:
                    cf.write(f)
                cf.set("server-address", "find_pwd", Globals.FIND_PASSWORD)
                with open(CONF_PATH, "w+") as f:
                    cf.write(f)
            except:
                pass
        try:
            need_kylin_id = cf.get("function-configuration", "show_popularity")
            if(need_kylin_id == "False"):
                Globals.NEED_SHOW_POPULARITY = False
            else:
                Globals.NEED_SHOW_POPULARITY = True
        except:
            Globals.NEED_SHOW_POPULARITY = False
        try:
            need_kylin_id = cf.get("function-configuration", "show_kylinID")
            if(need_kylin_id == "False"):
                Globals.NEED_SHOW_KYLINID = False
            else:
                Globals.NEED_SHOW_KYLINID = True
        except:
            Globals.NEED_SHOW_KYLINID = True
        try:
            Globals.SYSTEM_VERSION = cf.get("essential-information", "system_version")
            Globals.ARCH = cf.get("essential-information", "arch")
            Globals.IP = cf.get("essential-information", "ip")
            ip = self.aesdecrypt(Globals.IP)
            Globals.IP = ip
            Globals.MAC = cf.get("essential-information", "mac")
            mac = self.aesdecrypt(Globals.MAC)
            Globals.MAC = mac
            Globals.SERIALNUMBER = cf.get("essential-information", "serialNumber")
            Globals.CPU = cf.get("essential-information", "cpu")
            if("kunpeng" in Globals.CPU.lower() and "920" in Globals.CPU.lower()):
                Globals.CPU = "Kunpeng920"
        except Exception as e:
            pass
        if (Globals.SYSTEM_VERSION == "" or Globals.ARCH == "" or Globals.IP == "" or Globals.MAC == "" or Globals.SERIALNUMBER == "" or Globals.CPU == ""):
            Globals.ARCH = get_cpuArchitecture()
            try:
                read_system_version = os.popen("lsb_release -d").readline()
                # read_system_version = "Description:	Ubuntu 20.04.3 LTS"
                res_list = read_system_version.split(":")
                res_list1 = res_list[1].strip("\t").strip("\n").strip(" ")
                res_list2 = res_list1.split("Kylin")
                res = res_list2[1].strip(" ").strip("\n").strip("\t")
                if(res == ""):
                    res = res_list1
                Globals.SYSTEM_VERSION = res
            except:
                read_system_version = os.popen("lsb_release -d").readline()
                # read_system_version = "Description:	Ubuntu 20.04.3 LTS"
                res_list = read_system_version.split(":")
                res_list1 = res_list[1].strip("\t").strip("\n").strip(" ")
                Globals.SYSTEM_VERSION  = res_list1
            get_desktop_dict(Globals.SYSTEM_VERSION)
            Globals.IP, Globals.MAC = self.get_ip_and_mac()
            conf = configparser.ConfigParser()
            conf.read(CONF_PATH)
            try:
                conf.set("essential-information", "system_version", Globals.SYSTEM_VERSION)
                with open(CONF_PATH, "w+") as f:
                    conf.write(f)
                conf.set("essential-information", "arch", Globals.ARCH)
                with open(CONF_PATH, "w+") as f:
                    conf.write(f)
                ip = self.aesencrypt(Globals.IP)
                conf.set("essential-information", "ip", str(ip))
                with open(CONF_PATH, "w+") as f:
                    conf.write(f)
                mac = self.aesencrypt(Globals.MAC)
                conf.set("essential-information", "mac", str(mac))
                with open(CONF_PATH, "w+") as f:
                    conf.write(f)
            except Exception as e:
                print(str(e))
                pass
            kyinfo_conf = configparser.ConfigParser()
            serialNumber = ""
            try:
                kyinfo_conf.read("/etc/.kyinfo")
                serialNumber = kyinfo_conf.get("servicekey", "key")
                conf.set("essential-information", "serialNumber", serialNumber)
                with open(CONF_PATH, "w+") as f:
                    conf.write(f)
            except Exception as e:
                pass
                # Globals.LOG_ERROR.logger.error("remoter daemon: get serialNumber: %s" % str(e))
            Globals.SERIALNUMBER = serialNumber
            # if(CPU != ""):
            #     Globals.CPU = CPU
            # else:
            # if(os.path.exists("/sys/class/hwmon")):
            #     for path in os.listdir("/sys/class/hwmon"):
            #         if(os.path.exists("/sys/class/hwmon/"+ path+"/name")):
            #             read_name = os.popen("cat /sys/class/hwmon/"+ path+"/name").readline()
            #             if ("huawei_qs" in read_name.strip("\n")):
            #                 Globals.CPU = "huawei_qs"
            if(Globals.CPU == ""):
                try:
                    read_cpu = os.popen("cat /proc/cpuinfo |grep Hardware|cut -f2 -d :").readline()
                    # read_cpu = "HUAWEI Kirin 9006C"
                    # read_cpu = "Kirin990"
                    if (read_cpu.strip("\n").strip(" ") != ""):
                        if ("Kirin990".lower() in read_cpu.replace(" ", "").replace("\n", "").lower() or (
                                "kirin" in read_cpu.lower() and "990" in read_cpu)):
                            Globals.CPU = "kirin990"
                        elif ("HUAWEIKirin9006C".lower() in read_cpu.replace(" ", "").replace("\n", "").lower() or (
                                "Kirin" in read_cpu and "9006C" in read_cpu)):
                            Globals.CPU = "kirin9006c"
                        else:
                            Globals.CPU = read_cpu.replace(" ", "").replace("\n", "").lower()
                    else:
                        read_cpu = os.popen("cat /proc/cpuinfo |grep model|grep name|cut -f2 -d :").readline()
                        # read_cpu = ""
                        if (read_cpu.strip("\n").strip(" ") != ""):
                            if ("Intel" in read_cpu):
                                Globals.CPU = "Intel"
                            elif ("AMD" in read_cpu):
                                Globals.CPU = "AMD"
                            elif ("Hygon" in read_cpu):
                                Globals.CPU = "HYGON"
                            elif ("ZHAOXIN" in read_cpu):
                                Globals.CPU = "ZHAOXIN"
                            elif(("FT2000A" in read_cpu and "4-Core" in read_cpu) or ("FT-2000" in read_cpu)):
                                Globals.CPU = "FT2000/4"
                            else:
                                if ("D2000" in read_cpu and "Phytium" in read_cpu):
                                    Globals.CPU = "D2000"
                                else:
                                    if (len(read_cpu.split(",")) > 1):
                                        Globals.CPU = read_cpu.split(",")[1].strip(" ").strip("\n")
                                    else:
                                        Globals.CPU = read_cpu.strip(" ").strip("\n")
                except:
                    pass
            if ("kunpeng" in Globals.CPU.lower() and "920" in Globals.CPU.lower()):
                Globals.CPU = "Kunpeng920"
            try:
                conf.set("essential-information", "cpu", Globals.CPU)
                with open(CONF_PATH, "w+") as f:
                    conf.write(f)
            except:
                pass
        # Globals.SYSTEM_VERSION = "V10 SP1"
        if ("kunpeng" in Globals.CPU.lower() and "920" in Globals.CPU.lower()):
            Globals.CPU = "Kunpeng920"
        # Globals.LAN = QLocale.system().name()
        # self.init_ui()
        # # QApplication.processEvents()
        # self.show()
        # pool.submit(self.detailwidget.init_cache)
        # self.init_dbus_thread = init_dbus_Thread(self)
        # self.init_dbus_thread.start()


    def slot_init_cache_over(self):
        self.init_dbus_thread = init_dbus_Thread(self)
        self.init_dbus_thread.start()

    #敏感信息加密
    def aesencrypt(self, text):
        passwd = "kylin123"
        iv = os.urandom(16)
        # aescryptor = Aescrypt(passwd, AES.MODE_CBC, iv)  # CBC模式
        aescryptor = Aescrypt(passwd,AES.MODE_ECB,iv) # ECB模式
        en_text = aescryptor.aesencrypt(text)
        base64EncodedStr = base64.b64encode(en_text)
        base64EncodedStr = bytes.decode(base64EncodedStr)
        return base64EncodedStr

    # 敏感信息解密
    def aesdecrypt(self, text):
        passwd = "kylin123"
        iv = os.urandom(16)
        # result = re.findall("'([^']*)'", text)
        en_text = text.encode()
        # aescryptor = Aescrypt(passwd, AES.MODE_CBC, iv)  # CBC模式
        aescryptor = Aescrypt(passwd, AES.MODE_ECB, iv)  # ECB模式
        base64DecodedStr = base64.b64decode(en_text)
        de_text = aescryptor.aesdecrypt(base64DecodedStr)
        return bytes.decode(de_text)

    def get_ip_and_mac(self):
        try:
            mac = uuid.UUID(int=uuid.getnode()).hex[-12:].upper()
            mac = ":".join([mac[e:e + 2] for e in range(0, 11, 2)])
            ipAddressesList = QNetworkInterface.allAddresses()
            ipAddress = ""
            for i in range(len(ipAddressesList)):
                if (ipAddressesList[i] != QHostAddress.LocalHost and ipAddressesList[i].toIPv4Address()):
                    ipAddress = ipAddressesList[i].toString()
                    break
            ip = ipAddress
        except:
            ip = ""
            mac = ""
        return ip, mac

    def check_conf_path(self):
        if (not os.path.exists(CONF_PATH)):
            os.system("cp /usr/share/kylin-software-center/kylin-software-center.conf " + UKSC_CACHE_DIR)
        fd = open(CONF_PATH, "r")
        re = fd.readlines()
        theme = False
        update = False
        address = False
        info = False
        for i in re:
            if ("[theme-color]" in i):
                theme = True
            if ("[update-data]" in i):
                update = True
            if ("[server-address]" in i):
                address = True
            if ("[essential-information]" in i):
                info = True
        if (theme == False or update == False or address == False or info == False):
            os.system("cp /usr/share/kylin-software-center/kylin-software-center.conf " + UKSC_CACHE_DIR)


    def init_ui(self):
        self.setObjectName("kylin-software-center")
        self.setAttribute(QtCore.Qt.WA_TranslucentBackground)
        self.setWindowFlags(Qt.FramelessWindowHint)
        self.setProperty("useCustomShadow", True)
        self.setProperty("customShadowDarkness", 1.0)
        self.setProperty("customShadowWidth", 40)
        self.setProperty("customShadowRadius", QVector4D(12, 12, 12, 12))
        self.setProperty("customShadowMargins", QVector4D(35, 35, 35, 35))
        #print("0000000", datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
        try:
            # res = os.popen("LC_ALL=en_US apt-cache policy ukui-kwin|grep Installed|awk '{print $2}'")
            res = os.popen("dpkg -l |grep ukui-kwin |awk '$2==\"ukui-kwin\" {print $3}'")
            version = res.readline()
            if (version):
                if (version.strip("\n") == Globals.NEED_UKUI_KWIN_VERSION):
                    Globals.HAVE_THEME = True
                else:
                    version = version.replace("kylin", ".").strip("\n")
                    version_installed = version.split(".")

                    cmp_version = Globals.NEED_UKUI_KWIN_VERSION.replace("kylin", ".").strip("\n")
                    cmp_version_list = cmp_version.split(".")
                    sub = len(version_installed) - len(cmp_version_list)
                    if (sub > 0):
                        for i in range(sub):
                            version_installed.append("0")
                    else:
                        for i in range(sub):
                            cmp_version_list.append("0")
                    for i in range(len(cmp_version_list)):
                        if (cmp_version_list[i] < version_installed[i]):
                            Globals.HAVE_THEME = True
                            break
            else:
                Globals.HAVE_THEME = False
        except:
            Globals.HAVE_THEME = False
        if (Globals.HAVE_THEME == False):
            self.space = 5
            self.paintEvent = self.set_paintEvent
        else:
            try:
                get_type = os.popen("env |grep XDG_SESSION_TYPE|awk -F '[=]' '{print $2}'")
                xdg_session_type = get_type.readline()
                if (xdg_session_type.strip("\n") != "wayland"):
                    if (os.path.exists(KWIN_CONF_PATH)):
                        cf2 = configparser.ConfigParser()
                        cf2.read(KWIN_CONF_PATH)
                        try:
                            OpenGLIsUnsafe = cf2.get("Compositing", "OpenGLIsUnsafe")
                            if (OpenGLIsUnsafe == "true"):
                                Globals.RIGHT_ANGLE = True
                            else:
                                Globals.RIGHT_ANGLE = False
                        except:
                            try:
                                OpenGLIsUnsafe = cf2.get("Compositing", "Backend")
                                if (OpenGLIsUnsafe == "XRender"):
                                    Globals.RIGHT_ANGLE = True
                                else:
                                    Globals.RIGHT_ANGLE = False
                            except:
                                try:
                                    OpenGLIsUnsafe = cf2.get("Compositing", "Enabled")
                                    if (OpenGLIsUnsafe == "false"):
                                        Globals.RIGHT_ANGLE = True
                                    else:
                                        Globals.RIGHT_ANGLE = False
                                except:
                                    Globals.RIGHT_ANGLE = False
                    else:
                        Globals.RIGHT_ANGLE = False
                else:
                    Globals.RIGHT_ANGLE = False
                    self.paintEvent = self.set_paintEvent
            except:
                Globals.RIGHT_ANGLE = False
                self.paintEvent = self.set_paintEvent
        #print("111111", datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
        self.setWindowTitle(_("Software Store"))
        # print(QIcon.fromTheme("kylin-sotfware-center").hasThemeIcon("kylin-sotfware-center"))
        # if(QIcon.fromTheme("kylin-sotfware-center").hasThemeIcon("kylin-sotfware-center") == False):
        if (Globals.SYSTEM_VERSION.lower() == "v10"):
            self.setWindowIcon(QIcon("./res/logo.png"))
        else:
            iconpath = commontools.get_big_icon_path(Globals.APPNAME)
            self.setWindowIcon(QIcon.fromTheme("kylin-sotfware-center",QIcon(iconpath)))
        self.setFocusPolicy(Qt.NoFocus)
        self.setMouseTracking(True)
        self.installEventFilter(self)
        self.resize(1340 + self.space * 2, 918 + self.space * 2)
        Globals.INIT_DETAIL_WIDTH = self.width() - self.space * 2
        Globals.INIT_WIDTH = self.width()
        Globals.INIT_HEIGHT = self.height()
        desktop = QApplication.desktop()
        screen = desktop.availableGeometry()
        Globals.DESKTOP_WIDTH = screen.width()
        Globals.DESKTOP_HEIGHT = screen.height()
        if (self.width() > screen.width() or self.height() > screen.height()):
            self.resize(Globals.MINIWIDTH, Globals.MINIHIGHT)
        Globals.REMEMBER_WIDTH = self.width()
        Globals.REMEMBER_HEIGHT = self.height()
        self.setStyleSheet("#QWidget{background-color:rgba(55,167,215,0);border: 1px solid rgba(38, 38, 38, 0.15);}")
        self.frame_widget = QWidget(self)
        self.frame_widget.setAttribute(Qt.WA_TranslucentBackground)
        self.frame_widget.setWindowOpacity(1)
        self.frame_widget.installEventFilter(self)
        #self.frame_widget.show()

        self.title_frame_widget = QWidget(self)
        self.title_frame_widget.setGeometry(QtCore.QRect(self.space, self.space, self.width() - self.space, 86))
        self.title_frame_widget.setStyleSheet(".QWidget{background-color:rgba(55,167,215,0);}")
        # self.title_frame_widget.installEventFilter(self)
        #self.title_frame_widget.show()
        self.frame_widget.setGeometry(
            QtCore.QRect(self.space, int(self.title_frame_widget.y()+self.title_frame_widget.height()-1), int(self.width() - self.space * 2), int(self.height() - self.space - 86)))

        self.title_bar = TitleBar(self.width() - self.space * 2, self, self.title_frame_widget)
        self.title_bar.move(0, 0)
        self.title_bar.show()
        #print("444444", datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
        self.detailwidget = DetailWidget(self, self.frame_widget.width(), self.frame_widget.height() - self.space,
                                         self.frame_widget)
        self.detailwidget.move(0,0)
        self.detailwidget.refresh_background_color()
        #print("5555555", datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
        self.timer_hide_login = QTimer(self)
        #QApplication.processEvents()
        #print("6666666", datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
        # self.messageBox = MessageBox(self)
        self.is_max_size = True

    # 读取配置文件
    def theme_style(self,follow_the_theme = True):
        stylelist = ["ukui-dark", "ukui-black"]
        iconstylelist = ["ukui-default",""]
        if (os.path.exists("/usr/share/glib-2.0/schemas/org.ukui.style.gschema.xml")):
            self.settings = Gio.Settings.new('org.ukui.style')
            if(follow_the_theme == True):
                if self.settings["style-name"] in stylelist:
                    if (Globals.THEME_COLOR != 1):
                        Globals.THEME_COLOR = 1
                        # self.set_dark_mode.emit()
                else:
                    if (Globals.THEME_COLOR != 0):
                        Globals.THEME_COLOR = 0
                        # self.set_dark_mode.emit()
            self.settings.connect("changed", self.changed)
            if (self.settings["icon-theme-name"] == "ukui-icon-theme-default"):
                Globals.UK_SYSTEM_ICON_24_PATH = "/usr/share/icons/ukui-icon-theme-default/24x24/apps/"
                Globals.UK_SYSTEM_ICON_48_PATH = "/usr/share/icons/ukui-icon-theme-default/48x48/apps/"
                Globals.UK_SYSTEM_ICON_64_PATH = "/usr/share/icons/ukui-icon-theme-default/64x64/apps/"
                Globals.UK_SYSTEM_ICON_128_PATH = "/usr/share/icons/ukui-icon-theme-default/128x128/apps/"
                Globals.UK_SVG_ICON_PATH = "/usr/share/icons/ukui-icon-theme-default/scalable/apps/"
            elif (self.settings["icon-theme-name"] == "ukui-icon-theme-fashion"):
                Globals.UK_SYSTEM_ICON_24_PATH = "/usr/share/icons/ukui-icon-theme-fashion/24x24/apps/"
                Globals.UK_SYSTEM_ICON_48_PATH = "/usr/share/icons/ukui-icon-theme-fashion/48x48/apps/"
                Globals.UK_SYSTEM_ICON_64_PATH = "/usr/share/icons/ukui-icon-theme-fashion/64x64/apps/"
                Globals.UK_SYSTEM_ICON_128_PATH = "/usr/share/icons/ukui-icon-theme-fashion/128x128/apps/"
                Globals.UK_SVG_ICON_PATH = "/usr/share/icons/ukui-icon-theme-fashion/scalable/apps/"
            elif (self.settings["icon-theme-name"] == "ukui-icon-theme-classical"):
                Globals.UK_SYSTEM_ICON_24_PATH = "/usr/share/icons/ukui-icon-theme-classical/24x24/apps/"
                Globals.UK_SYSTEM_ICON_48_PATH = "/usr/share/icons/ukui-icon-theme-classical/48x48/apps/"
                Globals.UK_SYSTEM_ICON_64_PATH = "/usr/share/icons/ukui-icon-theme-classical/64x64/apps/"
                Globals.UK_SYSTEM_ICON_128_PATH = "/usr/share/icons/ukui-icon-theme-classical/128x128/apps/"
                Globals.UK_SVG_ICON_PATH = "/usr/share/icons/ukui-icon-theme-fashion/classical/apps/"
            else:
                icon_style = self.settings["icon-theme-name"]
                Globals.UK_SYSTEM_ICON_24_PATH = "/usr/share/icons/"+ icon_style+"/24x24/apps/"
                Globals.UK_SYSTEM_ICON_48_PATH = "/usr/share/icons/"+ icon_style+"/48x48/apps/"
                Globals.UK_SYSTEM_ICON_64_PATH = "/usr/share/icons/"+ icon_style+"/64x64/apps/"
                Globals.UK_SYSTEM_ICON_128_PATH = "/usr/share/icons/"+ icon_style+"/128x128/apps/"
                Globals.UK_SVG_ICON_PATH = "/usr/share/icons/"+ icon_style+"/scalable/apps/"
        else:
            Globals.THEME_COLOR = 0

    # 主题发生变化
    def changed(self, settings, key):
        cf = configparser.ConfigParser()
        cf.read(CONF_PATH)
        auto = cf.get("theme-color", "auto")
        if (settings["icon-theme-name"] == "ukui-icon-theme-default"):
            Globals.UK_SYSTEM_ICON_24_PATH = "/usr/share/icons/ukui-icon-theme-default/24x24/apps/"
            Globals.UK_SYSTEM_ICON_48_PATH = "/usr/share/icons/ukui-icon-theme-default/48x48/apps/"
            Globals.UK_SYSTEM_ICON_64_PATH = "/usr/share/icons/ukui-icon-theme-default/64x64/apps/"
            Globals.UK_SYSTEM_ICON_128_PATH = "/usr/share/icons/ukui-icon-theme-default/128x128/apps/"
            Globals.UK_SVG_ICON_PATH = "/usr/share/icons/ukui-icon-theme-default/scalable/apps/"
        elif (settings["icon-theme-name"] == "ukui-icon-theme-fashion"):
            Globals.UK_SYSTEM_ICON_24_PATH = "/usr/share/icons/ukui-icon-theme-fashion/24x24/apps/"
            Globals.UK_SYSTEM_ICON_48_PATH = "/usr/share/icons/ukui-icon-theme-fashion/48x48/apps/"
            Globals.UK_SYSTEM_ICON_64_PATH = "/usr/share/icons/ukui-icon-theme-fashion/64x64/apps/"
            Globals.UK_SYSTEM_ICON_128_PATH = "/usr/share/icons/ukui-icon-theme-fashion/128x128/apps/"
            Globals.UK_SVG_ICON_PATH = "/usr/share/icons/ukui-icon-theme-fashion/scalable/apps/"
        elif (settings["icon-theme-name"] == "ukui-icon-theme-classical"):
            Globals.UK_SYSTEM_ICON_24_PATH = "/usr/share/icons/ukui-icon-theme-classical/24x24/apps/"
            Globals.UK_SYSTEM_ICON_48_PATH = "/usr/share/icons/ukui-icon-theme-classical/48x48/apps/"
            Globals.UK_SYSTEM_ICON_64_PATH = "/usr/share/icons/ukui-icon-theme-classical/64x64/apps/"
            Globals.UK_SYSTEM_ICON_128_PATH = "/usr/share/icons/ukui-icon-theme-classical/128x128/apps/"
            Globals.UK_SVG_ICON_PATH = "/usr/share/icons/ukui-icon-theme-classical/scalable/apps/"
        else:
            icon_style = self.settings["icon-theme-name"]
            Globals.UK_SYSTEM_ICON_24_PATH = "/usr/share/icons/" + icon_style + "/24x24/apps/"
            Globals.UK_SYSTEM_ICON_48_PATH = "/usr/share/icons/" + icon_style + "/48x48/apps/"
            Globals.UK_SYSTEM_ICON_64_PATH = "/usr/share/icons/" + icon_style + "/64x64/apps/"
            Globals.UK_SYSTEM_ICON_128_PATH = "/usr/share/icons/" + icon_style + "/128x128/apps/"
            Globals.UK_SVG_ICON_PATH = "/usr/share/icons/" + icon_style + "/scalable/apps/"
        self.change_icon_show()
        if (Globals.SYSTEM_VERSION.lower() == "v10"):
            self.setWindowIcon(QIcon("./res/logo.png"))
        else:
            iconpath = commontools.get_big_icon_path(Globals.APPNAME)
            self.setWindowIcon(QIcon.fromTheme("kylin-sotfware-center",QIcon(iconpath)))
        if (auto == "YES"):
            stylelist = ["ukui-dark", "ukui-black"]
            if key == "style-name":
                if settings[key] in stylelist:
                    if (Globals.THEME_COLOR != 1):
                        Globals.THEME_COLOR = 1
                        self.set_mode()
                    # Globals.THEME_COLOR = 1
                else:
                    # Globals.THEME_COLOR = 0
                    if (Globals.THEME_COLOR != 0):
                        Globals.THEME_COLOR = 0
                        self.set_mode()

    def change_icon_show(self):
        self.title_bar.change_icon_show()
        if(hasattr(self,"contact_us_widget")):
            self.contact_us_widget.change_icon_show()

    def emit_change_color_signal(self):
        self.set_dark_mode.emit()
        if (Globals.NOWPAGE == PageStates.KYDROID):
            card_list = self.detailwidget.kydroid_widget.app_widget.children()
            for i in card_list:
                i.slot_set_dark_mode()
                QApplication.processEvents()

    def set_mode(self):
        # print("mmmmmm", datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
        self.hide_tip_widget()
        pool.submit(self.emit_change_color_signal)
        # print("tttttttt", datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
        self.title_bar.refresh_background_color()
        # print("huhuhuhu",datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
        self.detailwidget.refresh_background_color()
        # print("iiiiiiii", datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
        self.set_title_btn_stylesheet(Globals.NOWPAGE)
        if (hasattr(self, "menubar")):
            self.menubar.slot_show_dark_mode()
        if (hasattr(self, "second_lavel_menubar")):
            self.second_lavel_menubar.slot_show_dark_mode()
        if (hasattr(self, "search_tip_widget")):
            self.search_tip_widget.slot_show_dark_mode()
        if (hasattr(self, "contact_us_widget")):
            self.contact_us_widget.slot_show_dark_mode()
        if (hasattr(self.title_bar, "range_selection_widget")):
            self.title_bar.range_selection_widget.slot_show_dark_mode()
        if (hasattr(self, "download_error_tip_widget")):
            self.download_error_tip_widget.slot_show_dark_mode()
        if (hasattr(self, "network_error_tip_widget")):
            self.network_error_tip_widget.slot_show_dark_mode()
        if (hasattr(self, "install_error_tip_widget")):
            self.network_error_tip_widget.install_error_tip_widget()
        if (hasattr(self, "loginwidget")):
            self.loginwidget.slot_show_dark_mode()
            if (hasattr(self.loginwidget, "search_user_widget")):
                self.loginwidget.search_user_widget.slot_show_dark_mode()
            if (hasattr(self.loginwidget, "senior_search")):
                self.loginwidget.senior_search.slot_show_dark_mode()
        # print("@@@@@@@@@@@", datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
        QApplication.processEvents()
        # print("%%%%%%%%%%", datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))



    def init_connect(self):
        self.title_bar.close_button.clicked.connect(self.slot_click_close_button)
        self.title_bar.min_button.clicked.connect(self.slot_click_min_button)
        # self.title_bar.set_button.clicked.connect(self.slot_click_set_button)
        self.title_bar.return_btn.clicked.connect(self.slot_click_return_btn)
        self.title_bar.max_button.clicked.connect(self.slot_click_max_btn)
        self.title_bar.recovery_button.clicked.connect(self.slot_click_recovery_btn)
        self.title_bar.home_page_btn.clicked.connect(self.slot_click_homepage_btn)
        self.title_bar.category_btn.clicked.connect(self.slot_click_category_btn)
        self.title_bar.drive_btn.clicked.connect(self.slot_click_drive_btn)
        self.title_bar.kydroid_btn.clicked.connect(self.slot_click_kydroid_btn)
        self.title_bar.my_btn.clicked.connect(self.slot_click_my_btn)
        self.title_bar.login_btn.btn.clicked.connect(self.slot_login_account)
        # self.title_bar.exit_login_btn.btn.clicked.connect(self.slot_exit_login)
        self.title_bar.search_line.textChanged.connect(self.slot_get_search_tips)
        self.title_bar.search_line.returnPressed.connect(self.slot_textChanged)
        self.title_bar.search_image.clicked.connect(self.slot_textChanged)
        self.set_dark_mode.connect(self.title_bar.refresh_background_color)
        # self.title_bar.windowMoved.connect(self.window_move_to)
        # self.title_bar.search_line.editingFinished.connect(self.title_bar.slot_editingFinished)
        self.detailwidget.tip_error.connect(self.show_error_tip_widget)
        self.detailwidget.mywidget.history_widget.click_history_widget_btn.connect(self.slot_clicked_my_widget_btn)
        self.at.access_over.connect(self.slot_work_thread0_over)
        self.at.get_list_over.connect(self.slot_get_list_over)
        self.at.get_drive_list_over.connect(self.slot_get_drive_list_over)
        self.detailwidget.get_list_over.connect(self.slot_get_list_over)
        self.at.get_search_applist.connect(self.show_search_app)
        self.at.check_kydroid_envrun.connect(self.slot_kydroid_envrun_over)
        self.copy_file_to_archives.connect(self.slot_copy_file_to_archives)
        self.show_applist.connect(self.add_card_to_widget)
        self.show_inside_applist.connect(self.add_card_to_inside_widget)
        self.remove_card_signal.connect(self.remove_card)
        self.start_install_apk.connect(self.slot_start_install_apk)

        self.tip_download_error_signal.connect(self.tip_download_error)
        self.tip_network_error_signal.connect(self.tip_network_error)
        self.all_continue_download.connect(self.slot_all_continue_download)
        self.add_app_to_downloading_widget.connect(self.add_card_to_downloading_widget)
        self.add_apk_to_downloading_widget.connect(self.add_card_to_downloading_widget)
        self.set_ad_list_signal.connect(self.set_ad_list)
        self.send_notice_click.connect(self.slot_send_notice_click)
        # self..connect(self.slot_app_reviews_ready)
        # self.worker_thread0.appmgr.app_reviews_ready.connect(self.slot_app_reviews_ready)

        # self.app_reviews_ready.connect(self.slot_app_reviews_ready)

    #
    # 函数：初始化线性dbus
    #
    def init_sessionbus(self):
        try:
            bus = dbus.SessionBus(mainloop)
        except:
            Globals.LOG_ERROR.logger.error("could not initiate dbus")
            sys.exit(0)
        bus_name = dbus.service.BusName('com.kylin.softwarecenter', bus)
        self.dbusControler = UtilDbus(self, bus_name)

    #
    # 函数：全局异常捕获
    # etype：异常类型
    # value：异常值
    # trace：堆栈信息
    #
    def global_except_hook(self, etype, value, trace):
        msg = traceback.format_exception(etype, value, trace)
        sys.__excepthook__(etype, value, trace)
        self.slot_click_close_button()

    #
    # 函数：点击账户按钮
    #
    def slot_click_account_btn(self, item):
        get_item_widget = self.title_bar.exit_login_btn.item_list_widget.itemWidget(item)
        t = get_item_widget.appname_label.text()
        if (t == _("Exit login")):
            self.slot_exit_login()
        elif (t == _("User settings")):
            url = Globals.KYLIN_ID_PATH
            webbrowser_open_url(url)
            # QDesktopServices.openUrl(QUrl(url))

    #
    # 函数：初始化dbus后的操作
    #
    def slot_init_dbus_over(self, res):
        if res == False or self.detailwidget.ca == None:
            self.show_tip_widget(_("Initialization service exception, please restart the software store. If the restart is invalid, please click the error code to see the solution."), True, False,"("+_("Error Code")+"#0001)")
            self.tip_widget.btn.setText(_("Restart"))
            self.tip_widget.btn.clicked.connect(self.just_restart_ksc)
            self.title_bar.close_button.clicked.connect(self.slot_click_close_button)
            self.title_bar.min_button.clicked.connect(self.slot_click_min_button)
            self.title_bar.return_btn.clicked.connect(self.slot_click_return_btn)
            self.title_bar.max_button.clicked.connect(self.slot_click_max_btn)
        else:
            self.kydroid_service = KydroidService()
            self.kydroid_service.check_has_kydroid()
            self.at = AccessDbThread(self, self.init_dbus_thread.backend, self.detailwidget.ca)
            self.at.start()
            # self.kydroid_service.hasKydroid = False
            self.init_connect()

    #
    # 函数:初始访问服务器线程
    #
    def slot_init_appmger_over(self):
        try:
            cf = configparser.ConfigParser()
            cf.read(CONF_PATH)
            newer_list = cf.get("update-data", "update_app_list")
            if (newer_list == "True"):
                cf.set("update-data", "update_app_list", "False")
                with open(CONF_PATH, "w+") as f:
                    cf.write(f)
        except:
            pass
        self.ask_thread = Ask_Service_Thread(self, self.at.appmgr)
        self.ask_thread.start()
        # item = {}
        # item["action"] = "get_kydroid_source_list"
        # self.ask_thread.mutex.acquire()
        # self.ask_thread.work_list.append(item)
        # self.ask_thread.mutex.release()
        at_item = {}
        at_item["action"] = "get_installed_kydroid_app"
        self.at.mutex.acquire()
        self.at.work_list.append(at_item)
        self.at.mutex.release()
        item = {}
        item["action"] = "get_kydroid_sevice"
        self.ask_thread.mutex.acquire()
        self.ask_thread.work_list.append(item)
        self.ask_thread.mutex.release()
        # item2 = {}
        # item2["action"] = "get_cannot_remove_app"
        # self.ask_thread.mutex.acquire()
        # self.ask_thread.work_list.append(item2)
        # self.ask_thread.mutex.release()
        # item3 = {}
        # item3["action"] = "get_desktop_list"
        # self.ask_thread.mutex.acquire()
        # self.ask_thread.work_list.append(item3)
        # self.ask_thread.mutex.release()

    #
    # 函数：链接数据库失败
    #
    def slot_connect_db_false(self):
        self.show_tip_widget(_("Database connection failed, please restart the software store."), False, False,"("+_("Error Code")+"#0005)")
        self.tip_widget.btn.setText(_("Restart"))
        self.tip_widget.btn.clicked.connect(self.just_restart_ksc)

    #
    # 函数名: 检测更新软件商店新版本
    # Function:Detect new versions of updated software stores
    #
    def check_uksc_update(self):
        self.title_bar.home_page_btn.setEnabled(True)
        self.title_bar.my_btn.setEnabled(True)
        self.title_bar.drive_btn.setEnabled(True)
        self.title_bar.kydroid_btn.setEnabled(True)
        self.title_bar.category_btn.setEnabled(True)
        software_store_auto_update = "False"
        try:
            cf = configparser.ConfigParser()
            cf.read(CONF_PATH)
            software_store_auto_update = cf.get("option-settings", "software-store-auto-update")
        except:
            pass
        if(software_store_auto_update == "False"):
            uksc_dict = Globals.UKSC_APPMSG
            if (uksc_dict != None):
                try:
                    self.uksc = self.get_appmsg(uksc_dict)
                except:
                    self.uksc = None
                if (self.uksc != None):
                    if (self.uksc.is_upgradable == True):
                        self.show_tip_widget(_("Software store has new version, whether to upgrade?"))
                        self.tip_widget.btn.clicked.connect(self.update_uksc)

    #
    # 函数名: 更新数据
    # Function:update data
    #
    def update_uksc(self):
        self.tip_widget.close()
        num = 0
        for i in Globals.APPLICATION_LIST:
            if (i["name"] == self.uksc.pkgname):
                break
            num = num + 1
        res = self.init_dbus_thread.backend.call_auth_with_policykit(_("install app or update app or remove app"),
                                                                     "com.kylin.softwarecenter.install")
        is_ksc, is_kum, is_ki, read_list,is_start_remote = self.check_lock_file()
        if (is_ksc == True):
            self.fd = open(Globals.APT_LOCK_FILE, "w")
            try:
                fcntl.flock(self.fd.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)
                self.fd.write(
                    Globals.UID + "\n" + "kylin-software-center\n" + "install\n" + time.strftime("%Y-%m-%d %H:%M:%SZ",
                                                                                                 time.localtime()) + "\n\n")
            except:
                pass
            # fd = open(Globals.APT_LOCK_FILE, "a")
            self.slot_install_app(self.uksc, num, res)
            self.slot_all_puase_btn_state()
        else:
            self.fd = open(Globals.APT_LOCK_FILE, "w")
            try:
                fcntl.flock(self.fd.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)
                self.fd.write(
                    Globals.UID + "\n" + "kylin-software-center\n" + "install\n" + time.strftime("%Y-%m-%d %H:%M:%SZ",
                                                                                                 time.localtime()) + "\n\n")
                # fd.close()
                self.slot_install_app(self.uksc, num, res)
                self.slot_all_puase_btn_state()
            except:
                # fd.close()
                for i in read_list:
                    self.fd.write(i)
                self.fd.close()
                self.read_which_app_using(is_kum, is_ki)

    #
    # 函数：检测首页是否初始化完成
    #
    def check_home_page_ready(self):
        if(self.init_widget == False):
            self.init_widget = True
            # at_item = {}
            # at_item["action"] = "init_recommend"
            # self.at.mutex.acquire()
            # self.at.work_list.append(at_item)
            # self.at.mutex.release()
            # at_item1 = {}
            # at_item1["action"] = "init_android_recommend"
            # self.at.mutex.acquire()
            # self.at.work_list.append(at_item1)
            # self.at.mutex.release()
            # self.check_init_ready()

    def init_homepage(self):
        at_item0 = {}
        at_item0["action"] = "init_new_arrivals"
        self.at.mutex.acquire()
        self.at.work_list.append(at_item0)
        self.at.mutex.release()
        at_item01 = {}
        at_item01["action"] = "init_download_rank"
        self.at.mutex.acquire()
        self.at.work_list.append(at_item01)
        self.at.mutex.release()
        at_item02 = {}
        at_item02["action"] = "init_hot_app"
        self.at.mutex.acquire()
        self.at.work_list.append(at_item02)
        self.at.mutex.release()
        at_item = {}
        at_item["action"] = "init_recommend"
        self.at.mutex.acquire()
        self.at.work_list.append(at_item)
        self.at.mutex.release()
        at_item1 = {}
        at_item1["action"] = "init_android_recommend"
        self.at.mutex.acquire()
        self.at.work_list.append(at_item1)
        self.at.mutex.release()
        self.check_init_ready()
        at_item03 = {}
        at_item03["action"] = "init_all_app_list"
        self.at.mutex.acquire()
        self.at.work_list.append(at_item03)
        self.at.mutex.release()
        saerch_item = {"action": "get_search_association_list"}
        self.at.mutex.acquire()
        self.at.work_list.append(saerch_item)
        self.at.mutex.release()
        set_item = {"action": "", "appname": ""}
        set_item["action"] = "get_update_num"
        self.at.mutex.acquire()
        self.at.work_list.append(set_item)
        self.at.mutex.release()
        set_item = {"action": "", "appname": ""}
        set_item["action"] = "get_cannot_remove_list"
        self.at.mutex.acquire()
        self.at.work_list.append(set_item)
        self.at.mutex.release()
        # at_item = {}
        # at_item["action"] = "init_recommend"
        # self.at.mutex.acquire()
        # self.at.work_list.append(at_item)
        # self.at.mutex.release()
        # at_item1 = {}
        # at_item1["action"] = "init_android_recommend"
        # self.at.mutex.acquire()
        # self.at.work_list.append(at_item1)
        # self.at.mutex.release()

    #
    # 函数：获取广告就绪
    # list：获取到的广告列表
    # applist：获取到广告列表中软件对应application表中的信息
    #
    def slot_get_ad_ready(self, dblist, applist):

        # print("广告就绪",time.strftime("%Y-%m-%dT%H:%M:%SZ", time.localtime()))
        self.init_homepage()
        self.clear_all_card(self.detailwidget.ad_widget.ad_list_widget)
        self.detailwidget.ad_widget.get_lists(dblist, applist)
        num = 0
        self.detailwidget.ad_widget.card_w = 864
        self.detailwidget.ad_widget.card_h = 263
        count = 0
        big_banner = False
        small_banner = False
        for i in dblist:
            # QApplication.processEvents()
            if(i[2] != "0"):
                count = count + 1
                continue
            app_msg = self.get_appmsg(applist[count])
            if (i[2] == "0"):
                path_list = i[1].split("/")
                if ("_small" in path_list[len(path_list) - 1] and small_banner == False):
                    if (app_msg == None):
                        # self.detailwidget.ad_widget.add_ad_widget(None, None, len(dblist), 0)
                        pass
                    else:
                        small_banner = True
                        self.detailwidget.ad_widget.add_ad_widget(i[1], app_msg, len(dblist), 0)
                        num = num + 1
                if ("_big" in path_list[len(path_list) - 1] and big_banner == False):
                    if (app_msg == None):
                        # self.detailwidget.ad_widget.add_ad_widget(None, None, len(dblist), 1)
                        pass
                    else:
                        big_banner = True
                        self.detailwidget.ad_widget.add_ad_widget(i[1], app_msg, len(dblist), 1)
                        num = num + 1
            if (num >= 2):
                break
            count = count + 1
        if(num < 2):
            if(small_banner == False):
                self.detailwidget.ad_widget.add_ad_widget(None, None, len(dblist), 0)
            else:
                self.detailwidget.ad_widget.add_ad_widget(None, None, len(dblist), 1)
        if (len(dblist) != 0):
            Globals.GET_HOME_PGE_READY["AD"] = True
        self.check_home_page_ready()
        QApplication.processEvents()
        # print("广告绘制结束", time.strftime("%Y-%m-%dT%H:%M:%SZ", time.localtime()))

    #
    # 函数：win替换就位
    #
    def slot_win_replace_ready(self, win_list, applist):
        Globals.GET_HOME_PGE_READY["NEW"] = True
        # self.check_home_page_ready()
        pass

    #
    # 函数：获取新品上架就绪
    #
    def slot_new_arrivals_ready(self, dblist, applist):
        # print("new_arrival", time.strftime("%Y-%m-%dT%H:%M:%SZ", time.localtime()))
        self.detailwidget.new_arrivals_widget.start_move_time.stop()
        self.detailwidget.new_arrivals_widget.num = 0
        self.clear_all_card(self.detailwidget.new_arrivals_widget.card_list_widget)
        self.clear_all_card(self.detailwidget.new_arrivals_widget.card_list_widget2)
        self.detailwidget.new_arrivals_widget.card_list_widget.move(0,int(self.detailwidget.new_arrivals_widget.titellabel.height() + 10))
        self.detailwidget.new_arrivals_widget.card_list_widget2.move(int(self.detailwidget.new_arrivals_widget.card_list_widget2.width() ),
            int(self.detailwidget.new_arrivals_widget.titellabel.height() + 10))
        self.detailwidget.new_arrivals_widget.get_lists(dblist, applist)
        num = 0
        self.detailwidget.ad_widget.card_w = 864
        self.detailwidget.ad_widget.card_h = 263
        for i in dblist:
            if (num >= len(applist)):
                pass
            else:
                app_msg = self.get_appmsg(applist[num])
                if (app_msg != None):
                    if (i[2] != "" and i[2] != None):
                        path = i[2].split("/")[-1]
                    else:
                        path = ""
                    self.detailwidget.new_arrivals_widget.add_ad_widget(i[1], app_msg, len(dblist),path)
            num = num + 1

        QApplication.processEvents()
        # if (len(dblist) != 0):
        Globals.GET_HOME_PGE_READY["NEW"] = True
        if(num == 2 or self.detailwidget.new_arrivals_widget.card_list_widget.width() <= self.detailwidget.new_arrivals_widget.width()):
            self.detailwidget.new_arrivals_widget.card_list_widget2.hide()
            self.detailwidget.new_arrivals_widget.start_move_time.stop()
        else:
            self.detailwidget.new_arrivals_widget.start_move_time.start()
            self.detailwidget.new_arrivals_widget.card_list_widget2.show()
        self.detailwidget.new_arrivals_widget.card_list_widget.show()
        # self.detailwidget.recommended_android_list_widget.move(30,
        #                                                        self.detailwidget.recommended_list_widget.y() + self.detailwidget.recommended_list_widget.height() + 36)
        # self.detailwidget.show_homepage_widget.setGeometry(0, 0, self.detailwidget.width(),
        #                                                    self.detailwidget.recommended_android_list_widget.y() + self.detailwidget.recommended_android_list_widget.height() + 30)
        self.check_home_page_ready()

        # print("新品上架绘制结束", time.strftime("%Y-%m-%dT%H:%M:%SZ", time.localtime()))
        pass

    #
    #函数：处理点击新品上架中的更多
    #
    def slot_show_more_newapp(self):
        if(hasattr(self.detailwidget.new_arrivals_widget,"applist") and self.detailwidget.new_arrivals_widget.applist != []):
            pass
        else:
            return
        self.detailwidget.more_widget_title_label.setText(_("New Arrivals"))
        Globals.NOWPAGE = PageStates.MOREPAGE
        Globals.SEARCH_NOWPAGE = PageStates.HOMEPAGE
        page = {"pagetype": PageStates.MOREPAGE, "title_text": self.detailwidget.more_widget_title_label.text(),"now_page":PageStates.HOMEPAGE}
        if (len(Globals.RETURN_PATH_LIST) >= 10):
            Globals.RETURN_PATH_LIST.pop(0)
        Globals.RETURN_PATH_LIST.append(page)
        self.detailwidget.more_widget_title_label.show()
        self.detailwidget.scrollArea.hide()
        self.detailwidget.show_homepage_widget.hide()
        self.detailwidget.scrollArea2.hide()
        self.detailwidget.show_homepage_widget2.hide()
        self.detailwidget.show_homepage_widget3.show()
        self.detailwidget.scrollArea3.show()
        self.clear_all_card(self.detailwidget.show_more_app_widget)
        self.app_list_msg.clear()
        self.app_list_msg.extend(self.detailwidget.new_arrivals_widget.applist)
        self.slot_card_list("",self.detailwidget.new_arrivals_widget.applist,self.detailwidget.show_homepage_widget3,self.detailwidget.show_more_app_widget,False,True)
        # self.show_inside_applist.emit(self.detailwidget.new_arrivals_widget.applist,self.detailwidget.show_homepage_widget3,self.detailwidget.show_more_app_widget,False,True)



    #
    # 函数：热门应用就绪
    #
    def slot_hot_apps_ready(self, lists,path_list):
        # print("热门应用就绪", time.strftime("%Y-%m-%dT%H:%M:%SZ", time.localtime()))
        # QApplication.processEvents()
        self.clear_all_card(self.detailwidget.hot_app_widget.app_list_widget)
        self.clear_all_card(self.detailwidget.hot_app_widget.app_list_widget2)
        self.detailwidget.hot_app_widget.app_list_widget.move(0, int(self.detailwidget.hot_app_widget.titel_label.height()))
        self.detailwidget.hot_app_widget.app_list_widget2.move(0, int(self.detailwidget.hot_app_widget.titel_label.height()))
        self.detailwidget.hot_app_widget.app_num = 0
        self.detailwidget.hot_app_widget.get_lists(lists)
        num = 0
        for item in lists:
            app_msg = self.get_appmsg(item)
            if (app_msg != None):
                self.detailwidget.hot_app_widget.add_app_widget(app_msg,path_list[num])
                # QApplication.processEvents()
            num = num + 1
        if (self.detailwidget.hot_app_widget.app_list_widget.width() < self.detailwidget.hot_app_widget.width()):
            self.detailwidget.hot_app_widget.app_list_widget2.hide()
        self.detailwidget.hot_app_widget.app_list_widget.setFixedWidth(
            self.detailwidget.hot_app_widget.app_list_widget.width())
        self.detailwidget.hot_app_widget.app_list_widget2.setFixedWidth(
            self.detailwidget.hot_app_widget.app_list_widget.width())
        if (self.detailwidget.hot_app_widget.app_list_widget.isHidden() == False):
            if (self.detailwidget.hot_app_widget.app_list_widget.x() < self.detailwidget.hot_app_widget.width() or (
                    self.detailwidget.hot_app_widget.app_list_widget.x() + self.detailwidget.hot_app_widget.app_list_widget.width() < self.detailwidget.hot_app_widget.width())):
                for card in self.detailwidget.hot_app_widget.app_list_widget.children():
                    if (card.x() + self.detailwidget.hot_app_widget.app_list_widget.x() <= self.detailwidget.hot_app_widget.width() and card.x() + self.detailwidget.hot_app_widget.app_list_widget.x() >= 0):
                        if(card.appmsg.app_id not in self.detailwidget.hot_app_widget.show_card_list):
                            self.detailwidget.hot_app_widget.show_card_list.append(card.appmsg.app_id)
                            self.save_hot_exposure(card.appmsg.app_id)
                        else:
                            b = [val for val in self.detailwidget.hot_app_widget.add_card_list if val not in self.detailwidget.hot_app_widget.show_card_list]
                            if (b == []):
                                Globals.NEED_SEND_HOT_EXPOSURE = False
                                self.detailwidget.hot_app_widget.show_card_list.clear()
        if (self.detailwidget.hot_app_widget.app_list_widget2.isHidden() == False):
            if (self.detailwidget.hot_app_widget.app_list_widget2.x() < self.detailwidget.hot_app_widget.width() or (
                    self.detailwidget.hot_app_widget.app_list_widget2.x() + self.detailwidget.hot_app_widget.app_list_widget2.width() < self.width())):
                for card in self.detailwidget.hot_app_widget.app_list_widget2.children():
                    if (card.x() + self.detailwidget.hot_app_widget.app_list_widget2.x() <= self.detailwidget.hot_app_widget.width() and card.x() + self.detailwidget.hot_app_widget.app_list_widget2.x() >= 0):
                        if(card.appmsg.app_id not in self.detailwidget.hot_app_widget.show_card_list):
                            self.detailwidget.hot_app_widget.show_card_list.append(card.appmsg.app_id)
                            self.save_hot_exposure(card.appmsg.app_id)
                        else:
                            b = [val for val in self.detailwidget.hot_app_widget.add_card_list if val not in self.detailwidget.hot_app_widget.show_card_list]
                            if (b == []):
                                Globals.NEED_SEND_HOT_EXPOSURE = False
                                self.detailwidget.hot_app_widget.show_card_list.clear()
        #检测热门应用每次滚动几个卡片
        if (int(self.detailwidget.hot_app_widget.width() / (self.detailwidget.hot_app_widget.card_width + 20)) < 3 and int(self.detailwidget.hot_app_widget.app_list_widget.width() / (self.detailwidget.hot_app_widget.card_width + 20)) > 0):
            self.detailwidget.hot_app_widget.num = int(self.detailwidget.hot_app_widget.width() / (self.detailwidget.hot_app_widget.card_width + 20))
        if (len(lists) != 0):
            Globals.GET_HOME_PGE_READY["HOT"] = True
        self.check_home_page_ready()
        QApplication.processEvents()
        # print("热门应用绘制结束", time.strftime("%Y-%m-%dT%H:%M:%SZ", time.localtime()))
        # self.detailwidget.recommended_android_list_widget.move(30,self.detailwidget.recommended_list_widget.y() + self.detailwidget.recommended_list_widget.height() + 36)
        # self.detailwidget.show_homepage_widget.setGeometry(0, 0, self.detailwidget.width(),
        #                                                    self.detailwidget.recommended_android_list_widget.y() + self.detailwidget.recommended_android_list_widget.height() + 30)


    #
    #函数：获取显示热门应用更多
    #
    def slot_show_more_popularapp(self):
        if(hasattr(self.detailwidget.hot_app_widget,"applist") and self.detailwidget.hot_app_widget.applist != []):
            pass
        else:
            return
        self.detailwidget.more_widget_title_label.setText(_("Popular apps"))
        Globals.NOWPAGE = PageStates.MOREPAGE
        Globals.SEARCH_NOWPAGE = PageStates.HOMEPAGE
        page = {"pagetype": PageStates.MOREPAGE, "title_text": self.detailwidget.more_widget_title_label.text(),
                "now_page": PageStates.HOMEPAGE}
        if (len(Globals.RETURN_PATH_LIST) >= 10):
            Globals.RETURN_PATH_LIST.pop(0)
        Globals.RETURN_PATH_LIST.append(page)
        self.detailwidget.more_widget_title_label.show()
        self.detailwidget.scrollArea.hide()
        self.detailwidget.show_homepage_widget.hide()
        self.detailwidget.scrollArea2.hide()
        self.detailwidget.show_homepage_widget2.hide()
        self.detailwidget.show_homepage_widget3.show()
        self.detailwidget.scrollArea3.show()
        self.clear_all_card(self.detailwidget.show_more_app_widget)
        self.app_list_msg.clear()
        self.app_list_msg.extend(self.detailwidget.hot_app_widget.applist)
        # self.show_inside_applist.emit(self.detailwidget.hot_app_widget.applist,
        #                               self.detailwidget.show_homepage_widget3, self.detailwidget.show_more_app_widget,
        #                               False, True)
        self.slot_card_list("",self.detailwidget.hot_app_widget.applist,
                                      self.detailwidget.show_homepage_widget3, self.detailwidget.show_more_app_widget,
                                      False, True)

    #
    # 函数名: 推荐应用程序就绪
    # Function: slot recommend apps ready
    #
    def slot_recommend_apps_ready(self, applist, bysignal, first=True):
        # QApplication.processEvents()
        self.clear_all_card(self.detailwidget.recommended_list_widget.card_widget)
        self.detailwidget.recommended_list_widget.cardcount = 0
        num = 0
        h_num = int(self.detailwidget.recommended_list_widget.width() / 305)
        for app in applist:
            # if(num >=h_num*3):
            #     break
            # else:
            if app is None:
                continue
            else:
                in_widget = False
                for child in self.detailwidget.recommended_list_widget.card_widget.children():
                    if(str(child.app.app_id) == str(app["id"])):
                        in_widget = True
                        break
                if(in_widget == True):
                    continue
                app_msg = self.get_appmsg(app)
                if (app_msg != None):
                    recommend = Card_Widget(305, 190, 38, app_msg, self, self.detailwidget, self.detailwidget.ca,self.detailwidget.recommended_list_widget.card_widget)
                    self.set_dark_mode.connect(recommend.slot_set_dark_mode)
                    self.send_download_progress.connect(recommend.slot_get_download_progress)
                    self.signal_reload_btn.connect(recommend.reload_btn)
                    self.signal_uninstall.connect(recommend.slot_uninstall)
                    self.change_card_state.connect(recommend.change_btn_state)
                    self.change_kydroid_card_btn_state.connect(recommend.change_kydroid_btn_statue)
                    recommend.puase_download.connect(self.slot_puase_download_pkg)
                    recommend.puase_apk_download.connect(self.slot_puase_download_pkg)
                    recommend.check_all_check_statue.connect(self.slot_all_puase_btn_state)
                    recommend.check_btn.hide()
                    self.detailwidget.recommended_list_widget.add_card(recommend)
                    # if (num >=h_num*2):
                    #     recommend.hide()
                    num = num + 1
                    QApplication.processEvents()
        if (first):
            self.rec_ready = True
        self.detailwidget.recommended_android_list_widget.move(20,int(self.detailwidget.recommended_list_widget.y() + self.detailwidget.recommended_list_widget.height() + 36))
        self.detailwidget.show_homepage_widget.setGeometry(0, 0, int(self.detailwidget.width()),
                                                           int(self.detailwidget.recommended_android_list_widget.y() + self.detailwidget.recommended_android_list_widget.height() + 30))
        if (len(applist) != 0):
            Globals.GET_HOME_PGE_READY["RECOMMEND"] = True
        self.check_home_page_ready()
        # QApplication.processEvents()

    #
    #函数：推荐安卓应用程序就绪
    #
    def slot_android_recommend_ready(self,dblist,applist):
        # QApplication.processEvents()
        self.clear_all_card(self.detailwidget.recommended_android_list_widget.card_widget)
        self.detailwidget.recommended_android_list_widget.cardcount = 0
        num = 0
        h_num = int(self.detailwidget.recommended_android_list_widget.width() / 305)
        for app in applist:
            # QApplication.processEvents()
            # if(num >=h_num*3):
            #     break
            # else:
            app_msg = None
            if app is None:
                continue
            else:
                in_apk_list = False
                for get_apk in self.at.appmgr.apk_list:
                    if (app["name"] == get_apk.pkgname):
                        in_apk_list = True
                        app_msg = get_apk
                        break
                if (in_apk_list == False):
                    for i in Globals.KYDROID_SOURCE_LIST:
                        if(str(Globals.KYDROID_SOURCE_LIST[i]["packageName"]) == str(app["name"])):
                            app_msg = ApkInfo(app["name"], app["displayname_cn"], Globals.KYDROID_SOURCE_LIST[i]["version"],
                                          Globals.KYDROID_SOURCE_LIST[i]["size"], Globals.KYDROID_SOURCE_LIST[i]["file"], app["description"])
                            self.at.appmgr.set_apk_information(app, app_msg)
                            if (app["initialsName"] != None):
                                app_msg.initialsName = app["initialsName"]
                            if (app["pinyinName"] != None):
                                app_msg.pinyinName = app["pinyinName"]
                            if (app["anotherName"] != None):
                                app_msg.anotherName = app["anotherName"]
                            self.at.appmgr.apk_list.append(app_msg)
                # app_msg = self.get_appmsg(app)
                if (app_msg != None):
                    in_list = False
                    for i in Globals.APPLICATION_LIST:
                        if (app_msg.pkgname == i["name"]):
                            app_msg = i["appmsg"]
                            in_list = True
                    if (in_list == False):
                        Globals.APPLICATION_LIST.append({"name": app_msg.pkgname, "appmsg": app_msg})
                    recommend = Card_Widget(305, 190, 38, app_msg, self, self.detailwidget, self.detailwidget.ca,
                                            self.detailwidget.recommended_android_list_widget.card_widget)
                    self.set_dark_mode.connect(recommend.slot_set_dark_mode)
                    self.send_download_progress.connect(recommend.slot_get_download_progress)
                    self.signal_reload_btn.connect(recommend.reload_btn)
                    self.signal_uninstall.connect(recommend.slot_uninstall)
                    self.change_card_state.connect(recommend.change_btn_state)
                    self.change_kydroid_card_btn_state.connect(recommend.change_kydroid_btn_statue)
                    recommend.puase_download.connect(self.slot_puase_download_pkg)
                    recommend.puase_apk_download.connect(self.slot_puase_download_pkg)
                    recommend.check_all_check_statue.connect(self.slot_all_puase_btn_state)
                    recommend.check_btn.hide()
                    self.detailwidget.recommended_android_list_widget.add_card(recommend)
                    # if (num >=h_num*2):
                    #     recommend.hide()
                    num = num + 1
                    if(num == 1):
                        self.detailwidget.recommended_android_list_widget.show()

                    QApplication.processEvents()
        if ((dblist == None or len(dblist) == 0) and (applist == None or len(applist) == 0) or num == 0):
            self.detailwidget.recommended_android_list_widget.resize(self.detailwidget.width() - 60, 0)
            self.detailwidget.recommended_android_list_widget.move(20,
                                                                   int(self.detailwidget.recommended_list_widget.y() + self.detailwidget.recommended_list_widget.height() + 36))
            self.detailwidget.show_homepage_widget.resize(self.detailwidget.width(),
                                                          int(self.detailwidget.recommended_list_widget.y() + self.detailwidget.recommended_list_widget.height() + 30))

        else:
            self.detailwidget.recommended_android_list_widget.move(20,
                                                                   int(self.detailwidget.recommended_list_widget.y() + self.detailwidget.recommended_list_widget.height() + 36))
            # self.detailwidget.recommended_android_list_widget.resize(self.detailwidget.width() - 60, 450)
            self.detailwidget.show_homepage_widget.resize(self.detailwidget.width(),
                                                          int(self.detailwidget.recommended_android_list_widget.y() + self.detailwidget.recommended_android_list_widget.height() + 30))
        Globals.GET_HOME_PGE_READY["ANDROID_RECOMMEND"] = True
        self.check_home_page_ready()
        # QApplication.processEvents()

    #
    # 函数：将数据库中获取的数据转换成Appmsg类型后，将应用数据存储至Globals.APPLICATION_LIST
    #
    def get_appmsg(self, app,is_notice=False):
        if (app == None):
            return None
        if (type(app) == ApkInfo):
            return self.get_apkmsg(app)
        if (app["categories"] == None):
            category_list = [11]
        else:
            category_list = app["categories"].split(",")
        if("100" in category_list):
            is_notice = True
        category = category_cn = ""
        get_category = get_category_cn = ""
        try:
            num = 0
            if(str(app["softStyle"]) == "2"):
                for cate in category_list:
                    for i in Globals.DRIVE_CATEGORIES:
                        if(str(cate) == str(i["id"])):
                            if (num == 0):
                                category = i["name"]
                                category_cn = i["displayname"]
                            else:
                                category = category + "、" + i["name"]
                                category_cn = category_cn + "、" + i["displayname"]
                            num = num + 1
                            break
                    if(num >= 3):
                        break
            else:
                for i in category_list:
                    if (int(i) != 1 and int(i) != 2 and int(i) != 15 and int(i) != 18 and int(i) != 16 and int(i) != 100):
                        get_category, get_category_cn = self.at.appmgr.get_category_name_by_id(int(i))
                        if (num == 0):
                            category = get_category
                            category_cn = get_category_cn
                        else:
                            category = category + "、" + get_category
                            category_cn = category_cn + "、" + get_category_cn
                        num = num + 1
                        # break
                    else:
                        category = category
                        category_cn = category_cn
                    if (num > 3):
                        break
        except:
            category = category_cn = ""
        in_list = False
        for i in Globals.APPLICATION_LIST:
            if (app["name"] == i["name"]):
                app_msg = i["appmsg"]
                in_list = True
                break
        if (in_list == False):
            app_msg = Application(app["id"], app["name"], app["displayname_cn"], category, category_cn,
                                  self.at.appmgr.apt_cache,is_notice)
            Globals.APPLICATION_LIST.append({"name": app_msg.pkgname, "appmsg": app_msg})
        if (app_msg.package == None and is_notice == False):
            return None
        app_msg.from_ukscdb = app["from_db"]
        app_msg.orig_name = app["name"]  # zx2015.01.26
        app_msg.orig_summary = app["summary"]
        app_msg.orig_description = app["description"]
        app_msg.displayname = app["displayname_cn"]
        app_msg.summary = app["summary"]
        app_msg.description = app["description"]
        rating_average = app["rating_avg"]
        rating_total = app["rating_total"]
        review_total = app["review_total"]
        app_msg.downloadcount = app["download_total"]
        app_msg.icon_path = app["icon"]
        app_msg.down_preview_image_path = app["preview_image"]
        app_msg.down_cover_image_path = app["cover_image"]
        app_msg.arch = app["arch"]
        app_msg.system_name = app["system_name"]
        app_msg.depends = app["depends"]
        app_msg.cpu_model = app["cpu_model"]
        app_msg.no_cpu_model = app["no_cpu_model"]
        app_msg.certSubjectOu = app["certSubjectOu"]
        app_msg.softStyle = app["softStyle"]
        app_msg.manufacturers = app["manufacturers"]
        app_msg.popularity = app["popularity"]
        app_msg.website = app["website"]
        if rating_average is not None:
            app_msg.ratings_average = float(rating_average)
        if rating_total is not None:
            app_msg.ratings_total = int(rating_total)
        if review_total is not None:
            app_msg.review_total = int(review_total)
        return app_msg

    #
    # 函数：获取大小
    #
    def get_human_size(self, size):
        size_text = ""
        sizek = size / 1024
        if (sizek <= 1):
            size_text = "1 KB"
        elif (sizek > 1 and sizek < 1024):
            size_text = str('%.0f' % (sizek)) + " KB"
        elif (sizek >= 1024 and sizek < 1024 * 1024):
            if (len(str('%.2f' % (sizek / 1024.0)) + " MB") > 7):
                size_text = str('%.0f' % (sizek / 1024.0)) + " MB"
            else:
                size_text = str('%.2f' % (sizek / 1024.0)) + " MB"
        else:
            if (len(str('%.2f' % (sizek / (1024.0 * 1024.0))) + " GB") > 7):
                size_text = str('%.0f' % (sizek / (1024.0 * 1024.0))) + "GB"
            else:
                size_text = str('%.2f' % (sizek / (1024.0 * 1024.0))) + " GB"
        return size_text

    #
    # 函数：获取apk的软件信息
    #
    def get_apkmsg(self, apk):
        # apk.category_name = "kydroid"
        # apk.category_name_cn = "安卓软件"
        in_list = False
        for i in Globals.APPLICATION_LIST:
            if (apk.pkgname == i["name"]):
                app_msg = i["appmsg"]
                in_list = True
                if (type(app_msg) != ApkInfo):
                    Globals.APPLICATION_LIST.remove(i)
                    in_list = False
        if (in_list == False):
            Globals.APPLICATION_LIST.append({"name": apk.pkgname, "appmsg": apk})
        return apk

    # check base init
    #
    # 函数名: 初始化标题栏
    # Function: Initialize the title bar
    #
    def check_init_ready(self, bysignal=False):
        if (Globals.DEBUG_SWITCH):
            # print("init_check_init_ready")
            Globals.LOG_INFO.logger.info(_("init_check_init_ready"))
        page = {"pagetype": PageStates.HOMEPAGE}
        if (len(Globals.RETURN_PATH_LIST) >= 10):
            Globals.RETURN_PATH_LIST.pop(0)
        Globals.RETURN_PATH_LIST.append(page)
        self.title_bar.search_line_back.setText(_("Please enter appname"))
        Globals.NOWPAGE = PageStates.HOMEPAGE
        Globals.SEARCH_NOWPAGE = PageStates.HOMEPAGE
        self.detailwidget.loadinglabel.hide()
        # if (len(Globals.DRIVE_CATEGORIES) == 0):
        #     self.title_bar.drive_btn.hide()
        #     self.title_bar.drive_label.hide()
        #     self.title_bar.kydroid_btn.move(self.title_bar.drive_btn.x(), 57)
        #     self.title_bar.kydroid_label.move(self.title_bar.kydroid_btn.x()+int((self.title_bar.kydroid_btn.width()-self.title_bar.kydroid_label.width())/2),81)
        #     self.title_bar.my_btn.move(self.title_bar.kydroid_btn.x() + self.title_bar.kydroid_btn.width() + 30, 57)
        #     self.title_bar.my_label.move(self.title_bar.my_btn.x() + 3, 81)
        #     self.title_bar.my_btn_tip.move(self.title_bar.my_btn.x() + self.title_bar.my_btn.width() + 3 , self.title_bar.my_btn.y()-3)
        # else:
        self.title_bar.drive_btn.show()
        try:
            has_kmre = self.at.appmgr.apt_cache["kmre"]
        except:
            has_kmre = None
        if (self.kydroid_service.hasKydroid == False and has_kmre == None):
            # try:
            #     kmre = self.at.appmgr.apt_cache["kmre"]
            # except:
            #     kmre = None
            # if(kmre == None):
            self.title_bar.kydroid_btn.hide()
            self.title_bar.kydroid_label.hide()
            self.title_bar.my_btn.move(int(self.title_bar.kydroid_btn.x()), int(self.title_bar.my_btn.y()))
            self.title_bar.my_label.move(int(self.title_bar.my_btn.x() + 3), 81)
            self.title_bar.my_btn_tip.move(int(self.title_bar.my_btn.x() + self.title_bar.my_btn.width() + 3),
                                           int(self.title_bar.my_btn.y() - 3))
            # else:
            #     self.title_bar.kydroid_btn.show()
        else:
            self.title_bar.kydroid_btn.show()
            self.title_bar.my_btn.move(int(self.title_bar.kydroid_btn.x() + self.title_bar.kydroid_btn.width() + 30), 57)
            self.title_bar.my_label.move(int(self.title_bar.my_btn.x()+3),81)
            self.title_bar.my_btn_tip.move(int(self.title_bar.my_btn.x() + self.title_bar.my_btn.width() + 3),
                                           int(self.title_bar.my_btn.y() - 3))
        self.title_bar.my_btn.show()
        self.title_bar.return_btn.show()
        self.title_bar.home_page_btn.show()
        self.title_bar.category_btn.show()
        self.title_bar.search_label.show()
        self.detailwidget.no_app_tip_label.hide()
        self.detailwidget.text_prompt_label.hide()
        self.detailwidget.no_found_tip.hide()
        self.detailwidget.show_homepage_widget.show()
        self.detailwidget.download_ranking_widget.show()
        self.detailwidget.new_arrivals_widget.show()
        self.detailwidget.ad_widget.show()
        self.detailwidget.hot_app_widget.show()
        self.detailwidget.scrollArea.show()
        self.set_title_btn_stylesheet(PageStates.HOMEPAGE)
        self.detailwidget.new_arrivals_widget.add_card_over()
        #
        # self.read_token_login()

        if (self.detailwidget.show_homepage_widget in self.path_of_particle):
            pass
        else:
            self.path_of_particle.append(self.detailwidget.show_homepage_widget)

        for ad in self.detailwidget.ad_widget.ad_list_widget.children():
            if(hasattr(ad,"appmsg") and ad.appmsg != None):
                self.save_ad_exposure(ad.appmsg.app_id,0)
        for ad in self.detailwidget.ad_widget.ad_list_widget2.children():
            if(hasattr(ad,"appmsg") and ad.appmsg != None):
                self.save_ad_exposure(ad.appmsg.app_id,0)

        if (Globals.SEARCH_SOFTWARE != ""):
            self.click_search_app(Globals.SEARCH_SOFTWARE)
            Globals.SEARCH_SOFTWARE = ""
        # if(Globals.INIT_HOME_PAGE_OVER == False):
        # item = {"action":"submit_pingback_app"}
        #

        # set_item = {"action": "", "appname": ""}
        # set_item["action"] = "get_update_num"
        # self.at.mutex.acquire()
        # self.at.work_list.append(set_item)
        # self.at.mutex.release()

    #
    # 函数：显示下载排行
    #
    def slot_download_ranking_ready(self,applist):
        # print("下载排行就绪", time.strftime("%Y-%m-%dT%H:%M:%SZ", time.localtime()))
        # applist = Globals.GET_ALL_APP_LIST
        self.detailwidget.download_ranking_widget.get_applist(applist)
        self.detailwidget.download_ranking_widget.download_ranking_listWidget.clear()
        #由于下载排行前面的数字从1开始
        num = 1
        for i in applist:
            num = self.download_ranking_widget_additem(i, num)
            # QApplication.processEvents()
            if (num == 8 ):
                break
        Globals.GET_HOME_PGE_READY["RANK"] = True
        # self.detailwidget.recommended_android_list_widget.move(30,
        #                                                        self.detailwidget.recommended_list_widget.y() + self.detailwidget.recommended_list_widget.height() + 36)
        # self.detailwidget.show_homepage_widget.setGeometry(0, 0, self.detailwidget.width(),
        #                                                    self.detailwidget.recommended_android_list_widget.y() + self.detailwidget.recommended_android_list_widget.height() + 30)
        self.check_home_page_ready()
        # print("下载排行结束", time.strftime("%Y-%m-%dT%H:%M:%SZ", time.localtime()))

    #
    #函数：获取下载排行更多
    #
    def slot_show_more_downrankapp(self):
        if(hasattr(self.detailwidget.download_ranking_widget,"applist") and self.detailwidget.download_ranking_widget.applist != []):
            pass
        else:
            return
        self.detailwidget.more_widget_title_label.setText(_("Down Rank"))
        Globals.NOWPAGE = PageStates.MOREPAGE
        Globals.SEARCH_NOWPAGE = PageStates.HOMEPAGE
        page = {"pagetype": PageStates.MOREPAGE, "title_text": self.detailwidget.more_widget_title_label.text(),
                "now_page": PageStates.HOMEPAGE}
        if (len(Globals.RETURN_PATH_LIST) >= 10):
            Globals.RETURN_PATH_LIST.pop(0)
        Globals.RETURN_PATH_LIST.append(page)
        self.detailwidget.more_widget_title_label.show()
        self.detailwidget.scrollArea.hide()
        self.detailwidget.show_homepage_widget.hide()
        self.detailwidget.scrollArea2.hide()
        self.detailwidget.show_homepage_widget2.hide()
        self.detailwidget.show_homepage_widget3.show()
        self.detailwidget.scrollArea3.show()
        self.clear_all_card(self.detailwidget.show_more_app_widget)
        self.app_list_msg.clear()
        self.app_list_msg.extend(self.detailwidget.download_ranking_widget.applist)
        self.slot_card_list("",self.detailwidget.download_ranking_widget.applist,
                                      self.detailwidget.show_homepage_widget3, self.detailwidget.show_more_app_widget,
                                      False, True)

    #
    # 函数：加入item到下载排行QListWidget列表
    #
    def download_ranking_widget_additem(self, item, num):
        app_msg = self.get_appmsg(item)
        try:
            if (app_msg.package != None):
                item = QListWidgetItem(self.detailwidget.download_ranking_widget.download_ranking_listWidget)
                itemwidget = Download_Ranking_card(app_msg, num, self.detailwidget.ca, self,
                                                   self.detailwidget.download_ranking_widget)
                if (num == 1):
                    itemwidget.num_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 500;color: #F72213;}")
                if (num == 2):
                    itemwidget.num_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 500;color: #FE651A;}")
                if (num == 3):
                    itemwidget.num_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 500;color: #FFA01E;}")
                if (num == 7):
                    itemwidget.line_label.hide()
                self.set_dark_mode.connect(itemwidget.slot_set_dark_mode)
                self.send_download_progress.connect(itemwidget.slot_get_download_progress)
                self.signal_reload_btn.connect(itemwidget.reload_btn)
                self.change_card_state.connect(itemwidget.change_btn_state)
                itemwidget.puase_apk_download.connect(self.slot_puase_download_pkg)
                itemwidget.puase_download.connect(self.slot_puase_download_pkg)
                itemwidget.check_all_check_statue.connect(self.slot_all_puase_btn_state)
                itemwidget.btn.clicked.connect(
                    lambda: self.slot_click_action_btn(itemwidget, itemwidget.btn.text(), itemwidget.appmsg))
                self.detailwidget.download_ranking_widget.download_ranking_listWidget.addItem(item)
                self.detailwidget.download_ranking_widget.download_ranking_listWidget.setItemWidget(item, itemwidget)
                num = num + 1
        except:
            pass
        return num

    #
    # 函数：点击返回按钮
    #Globals.RETURN_PATH_LIST: "pagetype" 为所属页面 "sort_way" 排序方式 "Type" 分类 "now_page" 点击进详情页面前所属的位置 "title_text" 点击更多按钮的位置
    #
    def slot_click_return_btn(self):
        self.title_bar.search_line.clear()
        self.detailwidget.text_prompt_label.hide()
        self.detailwidget.no_found_tip.hide()
        self.detailwidget.no_app_tip_label.hide()
        if (len(Globals.RETURN_PATH_LIST) >= 2):
            hide_widget = Globals.RETURN_PATH_LIST.pop(len(Globals.RETURN_PATH_LIST) - 1)
            show_widget = Globals.RETURN_PATH_LIST.pop(len(Globals.RETURN_PATH_LIST) - 1)
            if hide_widget["pagetype"] == PageStates.HOMEPAGE:
                self.detailwidget.scrollArea.hide()
            elif hide_widget["pagetype"] == PageStates.KYDROID:
                self.detailwidget.kydroid_widget.hide()
            elif hide_widget["pagetype"] == PageStates.SEARCHPAGE:
                self.detailwidget.search_title_label.hide()
                self.detailwidget.search_widget_line.hide()
                self.detailwidget.search_app_widget.hide()
                self.detailwidget.all_search_tip_label.hide()
                self.detailwidget.show_homepage_widget2.hide()
                self.detailwidget.scrollArea2.hide()
            elif hide_widget["pagetype"] == PageStates.KYDROID:
                if (show_widget["pagetype"] == PageStates.KYDROID and show_widget["Type"] == hide_widget["Type"]):
                    pass
                else:
                    self.detailwidget.kydroid_widget.hide()
            elif hide_widget["pagetype"] == PageStates.CAREGORYPAGE:
                if (show_widget["pagetype"] == PageStates.CAREGORYPAGE and show_widget["Type"] == hide_widget["Type"]):
                    pass
                else:
                    self.detailwidget.category_widget.hide()
            elif hide_widget["pagetype"] == PageStates.DRIVE:
                self.detailwidget.drive_widget.hide()
            elif hide_widget["pagetype"] == PageStates.DOWNLOADINGPAGE:
                self.detailwidget.mywidget.hide()
            elif hide_widget["pagetype"] == PageStates.MYPAGE:
                self.detailwidget.mywidget.hide()
            elif hide_widget["pagetype"] == PageStates.HISTORY:
                self.detailwidget.mywidget.hide()
            elif hide_widget["pagetype"] == PageStates.APPDETAIL:
                if Globals.GO_SHORT_REVIEWS == True:
                    self.detailwidget.app_detail_widget.slot_change_shortreviewWindow()
                    Globals.RETURN_PATH_LIST.append(show_widget)
                    Globals.RETURN_PATH_LIST.append(hide_widget)
                    return
                else:
                    self.detailwidget.app_detail_widget.hide()
            elif hide_widget["pagetype"] == PageStates.MOREPAGE:
                self.detailwidget.show_homepage_widget3.hide()
                self.detailwidget.scrollArea3.hide()
            if show_widget["pagetype"] == PageStates.HOMEPAGE:
                self.title_bar.search_line_back.setText(_("Please enter appname"))
                if (Globals.INIT_DATA_OVER == True):
                    self.init_widget = False
                    item = {}
                    item["action"] = "reload_home_page"
                    self.at.mutex.acquire()
                    self.at.work_list.append(item)
                    self.at.mutex.release()
                    self.detailwidget.loadinglabel.show()
                    self.detailwidget.scrollArea.hide()
                    self.detailwidget.show_homepage_widget.hide()
                    Globals.INIT_DATA_OVER = False
                    Globals.INIT_HOME_PAGE_OVER = True
                else:
                    self.slot_click_homepage_btn()
            elif show_widget["pagetype"] == PageStates.KYDROID:
                self.set_title_btn_stylesheet(PageStates.KYDROID)
                self.title_bar.search_line_back.setText(_("Please enter android appname"))
                if (hide_widget["pagetype"] == PageStates.APPDETAIL):
                    self.detailwidget.app_detail_widget.hide()
                    self.detailwidget.kydroid_widget.show()
                    self.detailwidget.kydroid_widget.scrollArea.show()
                    self.detailwidget.kydroid_widget.app_widget.show()
                    Globals.RETURN_PATH_LIST.append(show_widget)
                else:
                    if (hide_widget["pagetype"] == PageStates.KYDROID and show_widget["Type"] != hide_widget["Type"]):
                        if (show_widget["sort_way"] == 0):
                            Globals.KYDROID_SORTING_METHOD = 0
                            self.detailwidget.kydroid_widget.slot_click_sorting_btn(0, False)
                        elif(show_widget["sort_way"] == 2):
                            Globals.KYDROID_SORTING_METHOD = 2
                            self.detailwidget.kydroid_widget.slot_click_sorting_btn(2, False)
                        else:
                            Globals.KYDROID_SORTING_METHOD = 1
                            self.detailwidget.kydroid_widget.slot_click_sorting_btn(1, False)

                        self.detailwidget.kydroid_widget.check_choose_btn_type()
                        btns = self.detailwidget.kydroid_widget.btn_widget.btnGroup.buttons()
                        for b in btns:
                            if (b.text_label.text() == show_widget["Type"]):
                                self.detailwidget.kydroid_widget.btn_widget.slot_btn_clicked(b, True)
                                break
                    else:
                        if (show_widget["sort_way"] == 0):
                            Globals.KYDROID_SORTING_METHOD = 0
                            self.detailwidget.kydroid_widget.slot_click_sorting_btn(0, False)
                        elif (show_widget["sort_way"] == 2):
                            Globals.KYDROID_SORTING_METHOD = 2
                            self.detailwidget.kydroid_widget.slot_click_sorting_btn(2, False)
                        else:
                            Globals.KYDROID_SORTING_METHOD = 1
                            self.detailwidget.kydroid_widget.slot_click_sorting_btn(1, False)
                        self.detailwidget.kydroid_widget.check_choose_btn_type()
                        btns = self.detailwidget.kydroid_widget.btn_widget.btnGroup.buttons()
                        for b in btns:
                            if (b.text_label.text() == show_widget["Type"]):
                                self.detailwidget.kydroid_widget.btn_widget.slot_btn_clicked(b, False)
                                break
                        self.detailwidget.slot_click_kydroid_type_btn(show_widget["Type"], False)
            elif show_widget["pagetype"] == PageStates.SEARCHPAGE:
                Globals.SEARCH_NOWPAGE = show_widget["search_nowpage"]
                if (hide_widget["pagetype"] == PageStates.APPDETAIL):
                    self.detailwidget.app_detail_widget.hide()
                    self.detailwidget.show_homepage_widget2.show()
                    self.detailwidget.scrollArea2.show()
                    Globals.RETURN_PATH_LIST.append(show_widget)
                else:
                    self.title_bar.search_line.setText(show_widget["search_text"])
                    self.slot_textChanged("", show_widget["type"])
            elif show_widget["pagetype"] == PageStates.DOWNLOADINGPAGE:
                self.title_bar.search_line_back.setText(_("Please enter onload appname"))
                self.set_title_btn_stylesheet(PageStates.MYPAGE)
                self.detailwidget.mywidget.downloading_btn.show_which_btn()
            elif show_widget["pagetype"] == PageStates.MYPAGE:
                self.set_title_btn_stylesheet(PageStates.MYPAGE)
                if (hide_widget["pagetype"] == PageStates.APPDETAIL):
                    self.detailwidget.app_detail_widget.hide()
                    self.detailwidget.mywidget.show()
                    self.detailwidget.mywidget.app_widget.show()
                    self.detailwidget.mywidget.scrollArea.show()
                    Globals.RETURN_PATH_LIST.append(show_widget)
                else:
                    if (show_widget["Type"] == "update"):
                        self.title_bar.search_line_back.setText(_("Please enter update appname"))
                        self.detailwidget.mywidget.update_btn.show_which_btn()
                    else:
                        self.title_bar.search_line_back.setText(_("Please enter uninstall appname"))
                        self.detailwidget.mywidget.uninstall_btn.show_which_btn()
            elif show_widget["pagetype"] == PageStates.HISTORY:
                self.title_bar.search_line_back.setText(_("Please enter installation appname"))
                self.title_bar.search_line_back.adjustSize()
                self.set_title_btn_stylesheet(PageStates.MYPAGE)
                if (hide_widget["pagetype"] == PageStates.APPDETAIL):
                    self.detailwidget.app_detail_widget.hide()
                    self.detailwidget.mywidget.show()
                    self.detailwidget.mywidget.app_widget.show()
                    self.detailwidget.mywidget.scrollArea.show()
                    Globals.RETURN_PATH_LIST.append(show_widget)
                else:
                    if (show_widget["Type"] == "local"):
                        self.detailwidget.mywidget.history_widget.now_btn_name = "local"
                    else:
                        self.detailwidget.mywidget.history_widget.now_btn_name = "other"
                    self.detailwidget.mywidget.installed_btn.show_which_btn()
            elif show_widget["pagetype"] == PageStates.CAREGORYPAGE:
                self.title_bar.search_line_back.setText(_("Please enter appname"))
                self.set_title_btn_stylesheet(PageStates.CAREGORYPAGE)
                if (hide_widget["pagetype"] == PageStates.APPDETAIL):
                    self.detailwidget.app_detail_widget.hide()
                    self.detailwidget.category_widget.show()
                    self.detailwidget.category_widget.scrollArea.show()
                    self.detailwidget.category_widget.type_show_card_widget.show()
                    self.detailwidget.category_widget.type_widget.show()
                    Globals.RETURN_PATH_LIST.append(show_widget)
                else:
                    if (hide_widget["pagetype"] == PageStates.CAREGORYPAGE and show_widget["Type"] != hide_widget["Type"]):
                        if (show_widget["sort_way"] == 0):
                            Globals.SORTING_METHOD = 0
                            self.detailwidget.category_widget.check_choose_btn_type()
                            self.detailwidget.category_widget.slot_click_sorting_btn(0, False)
                        elif(show_widget["sort_way"] == 2):
                            Globals.SORTING_METHOD = 2
                            self.detailwidget.category_widget.slot_click_sorting_btn(2, False)
                        else:
                            Globals.SORTING_METHOD = 1
                            self.detailwidget.category_widget.slot_click_sorting_btn(1, False)

                        self.detailwidget.category_widget.check_choose_btn_type()
                        btns = self.detailwidget.category_widget.btn_widget.btnGroup.buttons()
                        for b in btns:
                            if (b.text_label.text() == show_widget["Type"]):
                                self.detailwidget.category_widget.btn_widget.slot_btn_clicked(b, True)
                                break
                    else:
                        if (show_widget["sort_way"] == 0):
                            Globals.SORTING_METHOD = 0
                            self.detailwidget.category_widget.slot_click_sorting_btn(0, False)
                        elif (show_widget["sort_way"] == 2):
                            Globals.SORTING_METHOD = 2
                            self.detailwidget.category_widget.slot_click_sorting_btn(2, False)
                        else:
                            Globals.SORTING_METHOD = 1
                            self.detailwidget.category_widget.slot_click_sorting_btn(1, False)

                        self.detailwidget.category_widget.check_choose_btn_type()
                        btns = self.detailwidget.category_widget.btn_widget.btnGroup.buttons()
                        for b in btns:
                            if (b.text_label.text() == show_widget["Type"]):
                                self.detailwidget.category_widget.btn_widget.slot_btn_clicked(b, False)
                                break
                        self.detailwidget.slot_click_type_btn(show_widget["Type"], False)
                        # self.detailwidget.slot_click_type_btn(show_widget["Type"], False)
            elif show_widget["pagetype"] == PageStates.DRIVE:
                self.set_title_btn_stylesheet(PageStates.DRIVE)
                if (hide_widget["pagetype"] == PageStates.APPDETAIL):
                    self.detailwidget.app_detail_widget.hide()
                    self.detailwidget.drive_widget.show()
                    self.detailwidget.drive_widget.scrollArea.show()
                    self.detailwidget.drive_widget.type_show_card_widget.show()
                    self.detailwidget.drive_widget.type_widget.show()
                    Globals.RETURN_PATH_LIST.append(show_widget)
                else:
                    if (hide_widget["pagetype"] == PageStates.DRIVE and show_widget["Type"] != hide_widget["Type"]):
                        btns = self.detailwidget.drive_widget.btn_widget.btnGroup.buttons()
                        for b in btns:
                            if (b.text_label.text() == show_widget["Type"]):
                                self.detailwidget.drive_widget.btn_widget.slot_btn_clicked(b, True)
                                break
                    else:
                        # self.detailwidget.category_widget.slot_click_sorting_btn(False, False)
                        btns = self.detailwidget.drive_widget.btn_widget.btnGroup.buttons()
                        for b in btns:
                            if (b.text_label.text() == show_widget["Type"]):
                                self.detailwidget.drive_widget.btn_widget.slot_btn_clicked(b, False)
                                break
                        self.detailwidget.slot_drive_type_click(show_widget["Type"])
            elif show_widget["pagetype"] == PageStates.APPDETAIL:
                if(show_widget["now_page"] == PageStates.HOMEPAGE):
                    self.title_bar.search_line_back.setText(_("Please enter appname"))
                elif(show_widget["now_page"] == PageStates.KYDROID):
                    self.title_bar.search_line_back.setText(_("Please enter android appname"))
                elif(show_widget["now_page"] == PageStates.CAREGORYPAGE):
                    self.title_bar.search_line_back.setText(_("Please enter appname"))
                elif (show_widget["now_page"] == PageStates.DOWNLOADINGPAGE):
                    self.title_bar.search_line_back.setText(_("Please enter onload appname"))
                elif (show_widget["now_page"] == PageStates.MYPAGE):
                    if(Globals.RETURN_PATH_LIST[len(Globals.RETURN_PATH_LIST) - 1]["Type"] == "update"):
                        self.title_bar.search_line_back.setText(_("Please enter update appname"))
                    else:
                        self.title_bar.search_line_back.setText(_("Please enter uninstall appname"))
                elif(show_widget["now_page"] == PageStates.HISTORY):
                    self.title_bar.search_line_back.setText(_("Please enter installation appname"))
                    self.title_bar.search_line_back.adjustSize()
                Globals.SEARCH_NOWPAGE = show_widget["now_page"]
                self.set_title_btn_stylesheet(show_widget["now_page"])
                self.detailwidget.slot_clicked_card(show_widget["app_msg"])
            elif show_widget["pagetype"] == PageStates.MOREPAGE:
                self.title_bar.search_line_back.setText(_("Please enter appname"))
                if(show_widget["title_text"] == _("New Arrivals")):
                    self.slot_show_more_newapp()
                elif(show_widget["title_text"] == _("Popular apps")):
                    self.slot_show_more_popularapp()
                elif (show_widget["title_text"] == _("Down Rank")):
                    self.slot_show_more_downrankapp()
                self.set_title_btn_stylesheet(show_widget["now_page"])
        elif (len(Globals.RETURN_PATH_LIST) >= 0):
            if(len(Globals.RETURN_PATH_LIST) > 0):
                hide_widget = Globals.RETURN_PATH_LIST.pop(len(Globals.RETURN_PATH_LIST) - 1)
                if hide_widget["pagetype"] == PageStates.HOMEPAGE:
                    self.detailwidget.scrollArea.hide()
                elif hide_widget["pagetype"] == PageStates.KYDROID:
                    self.detailwidget.kydroid_widget.hide()
                elif hide_widget["pagetype"] == PageStates.SEARCHPAGE:
                    self.detailwidget.search_title_label.hide()
                    self.detailwidget.search_widget_line.hide()
                    self.detailwidget.search_app_widget.hide()
                    self.detailwidget.all_search_tip_label.hide()
                    self.detailwidget.scrollArea2.hide()
                elif hide_widget["pagetype"] == PageStates.KYDROID:
                    self.detailwidget.kydroid_widget.hide()
                elif hide_widget["pagetype"] == PageStates.CAREGORYPAGE:
                    self.detailwidget.category_widget.hide()
                elif hide_widget["pagetype"] == PageStates.DOWNLOADINGPAGE:
                    self.detailwidget.mywidget.hide()
                elif hide_widget["pagetype"] == PageStates.MYPAGE:
                    self.detailwidget.mywidget.hide()
                elif hide_widget["pagetype"] == PageStates.HISTORY:
                    self.detailwidget.mywidget.hide()
                elif hide_widget["pagetype"] == PageStates.APPDETAIL:
                    self.detailwidget.app_detail_widget.hide()
                elif hide_widget["pagetype"] == PageStates.MOREPAGE:
                    self.detailwidget.show_homepage_widget3.hide()
                    self.detailwidget.scrollArea3.hide()
                self.slot_click_homepage_btn()
            else:
                self.slot_click_homepage_btn()

    #
    # 函数：点击首页按钮
    #
    def slot_click_homepage_btn(self):
        if (Globals.DEBUG_SWITCH):
            # print("click_home_page")
            Globals.LOG_INFO.logger.info(_("click_home_page"))
        self.title_bar.search_line_back.setText(_("Please enter appname"))
        if(Globals.NOWPAGE != PageStates.HOMEPAGE):
            Globals.NEED_SEND_EXPOSURE = True
            Globals.NEED_SEND_HOT_EXPOSURE = True
        Globals.NOWPAGE = PageStates.HOMEPAGE
        Globals.SEARCH_NOWPAGE = PageStates.HOMEPAGE
        self.set_title_btn_stylesheet(PageStates.HOMEPAGE)
        self.title_bar.search_line.clear()
        self.detailwidget.text_prompt_label.hide()
        self.detailwidget.no_found_tip.hide()
        self.detailwidget.no_app_tip_label.hide()
        self.detailwidget.scrollArea2.verticalScrollBar().setValue(0)
        if (self.detailwidget.first_click_card == False):
            self.detailwidget.app_detail_widget.hide()
        if (self.detailwidget.first_show_mywidget == False):
            self.detailwidget.mywidget.hide()
        if (self.detailwidget.first_show_category == False):
            self.detailwidget.category_widget.hide()
        if (hasattr(self.detailwidget,"kydroid_widget")):
            self.detailwidget.kydroid_widget.hide()
        if(hasattr(self.detailwidget,"drive_widget")):
            self.detailwidget.drive_widget.hide()
        self.detailwidget.show_homepage_widget2.hide()
        self.detailwidget.scrollArea2.hide()
        self.detailwidget.show_homepage_widget3.hide()
        self.detailwidget.scrollArea3.hide()
        self.path_of_particle.clear()
        page = {"pagetype": PageStates.HOMEPAGE}
        if (len(Globals.RETURN_PATH_LIST) >= 10):
            Globals.RETURN_PATH_LIST.pop(0)
        Globals.RETURN_PATH_LIST.append(page)
        self.path_of_particle.append(self.detailwidget.show_homepage_widget)
        self.detailwidget.new_arrivals_widget.card_list_widget.move(0,int(self.detailwidget.new_arrivals_widget.titellabel.height() + 10))
        self.detailwidget.new_arrivals_widget.card_list_widget2.move( int(self.detailwidget.new_arrivals_widget.card_list_widget2.width()),int(self.detailwidget.new_arrivals_widget.titellabel.height() + 10))
        self.detailwidget.new_arrivals_widget.final_x_position = 0
        self.detailwidget.new_arrivals_widget.final_x_position2 = 0
        self.detailwidget.new_arrivals_widget.card_list_widget.show()
        self.detailwidget.new_arrivals_widget.card_list_widget2.hide()
        if (self.detailwidget.new_arrivals_widget.card_list_widget.width() <= self.detailwidget.new_arrivals_widget.width()):
            self.detailwidget.new_arrivals_widget.card_list_widget2.hide()
            self.detailwidget.new_arrivals_widget.start_move_time.stop()
        else:
            self.detailwidget.new_arrivals_widget.start_move_time.start()
            self.detailwidget.new_arrivals_widget.card_list_widget2.show()
            # self.detailwidget.new_arrivals_widget.start_move_time.start(4000)

        if (Globals.INIT_DATA_OVER == True):
            item = {}
            item["action"] = "reload_home_page"
            self.at.mutex.acquire()
            self.at.work_list.append(item)
            self.at.mutex.release()
            self.detailwidget.loadinglabel.show()
            self.detailwidget.scrollArea.hide()
            self.detailwidget.show_homepage_widget.hide()
            Globals.INIT_DATA_OVER = False
            Globals.INIT_HOME_PAGE_OVER = True
        else:
            for i in range(self.detailwidget.download_ranking_widget.download_ranking_listWidget.count()):
                get_item = self.detailwidget.download_ranking_widget.download_ranking_listWidget.item(i)
                get_item_widget = self.detailwidget.download_ranking_widget.download_ranking_listWidget.itemWidget(get_item)
                get_item_widget.change_btn_state(get_item_widget.appmsg.pkgname, get_item_widget.appmsg.status)

            for child in self.detailwidget.recommended_list_widget.card_widget.children():
                child.change_btn_state(child.app.pkgname, child.app.status)

            for child in self.detailwidget.recommended_android_list_widget.card_widget.children():
                child.change_btn_state(child.app.pkgname, child.app.status)


            self.detailwidget.scrollArea.show()
            self.detailwidget.show_homepage_widget.show()
        for ad in self.detailwidget.ad_widget.ad_list_widget.children():
            if(hasattr(ad,"appmsg") and ad.appmsg != None):
                self.save_ad_exposure(ad.appmsg.app_id,0)
        for ad in self.detailwidget.ad_widget.ad_list_widget2.children():
            if(hasattr(ad,"appmsg") and ad.appmsg != None):
                self.save_ad_exposure(ad.appmsg.app_id,0)


    #
    # 函数：点击分类按钮
    #
    def slot_click_category_btn(self):
        Globals.NOWPAGE = PageStates.CAREGORYPAGE
        if (self.at.init_all_data == False):
            return
        self.title_bar.search_line_back.setText(_("Please enter appname"))
        Globals.NOWPAGE = PageStates.CAREGORYPAGE
        Globals.SEARCH_NOWPAGE = PageStates.CAREGORYPAGE
        self.set_title_btn_stylesheet(PageStates.CAREGORYPAGE)
        if (hasattr(self.detailwidget, "drive_widget")):
            self.detailwidget.drive_widget.hide()
        # if(hasattr(self.detailwidget,"category_widget")):
        #     page = {"pagetype": PageStates.CAREGORYPAGE, "Type":self.detailwidget.category_widget.type_name_label.text(),"sort_way":Globals.SORTING_METHOD}
        #     if (len(Globals.RETURN_PATH_LIST) >= 10):
        #         Globals.RETURN_PATH_LIST.pop(0)
        #     Globals.RETURN_PATH_LIST.append(page)
        self.title_bar.search_line.clear()
        self.detailwidget.text_prompt_label.hide()
        self.detailwidget.no_found_tip.hide()
        self.detailwidget.no_app_tip_label.hide()
        self.detailwidget.scrollArea.verticalScrollBar().setValue(0)
        self.detailwidget.show_category_widget()
        self.detailwidget.scrollArea.hide()
        self.detailwidget.show_homepage_widget.hide()
        self.detailwidget.show_homepage_widget3.hide()
        self.detailwidget.scrollArea3.hide()

    #
    #函數:点击驱动按钮
    #
    def slot_click_drive_btn(self):
        Globals.NOWPAGE = PageStates.DRIVE
        if (self.at.init_all_data == False):
            return
        self.title_bar.exit_login_btn.hide()
        self.title_bar.senior_search.hide()
        self.title_bar.search_line_back.setText(_("Please enter drive appname"))
        self.title_bar.search_line_back.adjustSize()
        if (hasattr(self, "title_bar")):
            self.title_bar.search_line.setFocus(False)
            self.title_bar.search_line.clearFocus()
        if (hasattr(self, "search_tip_widget")):
            self.search_tip_widget.hide()
        Globals.NOWPAGE = PageStates.DRIVE
        Globals.SEARCH_NOWPAGE = PageStates.DRIVE
        self.set_title_btn_stylesheet(PageStates.DRIVE)
        self.title_bar.search_line.clear()
        self.detailwidget.text_prompt_label.hide()
        self.detailwidget.no_found_tip.hide()
        self.detailwidget.no_app_tip_label.hide()
        self.detailwidget.scrollArea.verticalScrollBar().setValue(0)
        self.detailwidget.show_drive_widget()
        self.detailwidget.scrollArea.hide()
        self.detailwidget.show_homepage_widget.hide()
        self.detailwidget.show_homepage_widget3.hide()
        self.detailwidget.scrollArea3.hide()

    #
    #函数：获取驱动分类列表结束
    #
    def slot_get_drive_list_over(self,item):
        self.get_inside_list_over = True
        self.detailwidget.drive_widget.all_check_btn.setText(_("Select all"))
        self.detailwidget.drive_widget.one_click_download.setText(_("all down"))
        self.detailwidget.drive_widget.one_click_download.setEnabled(False)
        self.detailwidget.drive_widget.one_click_download.setStyleSheet("QPushButton{font-size:14px;font-weight: 500;color: #8f9399;line-height: 20px;border:0px solid #ff0000;}")
        applist = []
        applist.extend(Globals.DRIVE_CATEGORIES_APP_LIST[item])
        Globals.NOWPAGE = PageStates.DRIVE
        Globals.SEARCH_NOWPAGE = PageStates.DRIVE
        Globals.FRIST_LEVEL_BTN_SCROLLAREA["DRIVE"].clear()
        Globals.FRIST_LEVEL_BTN_SCROLLAREA["DRIVE"].extend(applist)
        self.app_list_msg.clear()
        self.app_list_msg.extend(applist)
        self.clear_all_card(self.detailwidget.drive_widget.type_show_card_widget)
        self.detailwidget.drive_widget.scrollArea.verticalScrollBar().setValue(0)
        self.slot_card_list(self.detailwidget.drive_widget.type_name_label.text(),applist, self.detailwidget.drive_widget.type_widget,
                                      self.detailwidget.drive_widget.type_show_card_widget, False, True)
        self.detailwidget.drive_widget.type_widget.show()
        self.detailwidget.drive_widget.scrollArea.show()
        self.detailwidget.drive_widget.loadinglabel.hide()


    #
    #函数：保存分类界面的曝光率
    #
    def save_ad_exposure(self,appid,category_id):
        if(str(appid)+"_"+str(category_id) not in Globals.AD_EXPOSURE_RATE.keys()):
            Globals.AD_EXPOSURE_RATE[str(appid)+"_"+str(category_id)] = {"appid":appid,"category_id":category_id,"show_count":1,"click_count":0,"down_count":0}
        else:
            show_count = Globals.AD_EXPOSURE_RATE[str(appid)+"_"+str(category_id)]["show_count"] + 1
            Globals.AD_EXPOSURE_RATE[str(appid)+"_"+str(category_id)]["show_count"] = show_count
        item = {}
        item["action"] = "save_db_exposure_rate"
        self.at.mutex.acquire()
        self.at.work_list.append(item)
        self.at.mutex.release()

    #
    #函数：保存点击卡片曝光率
    #
    def save_click_ad_card(self,appid,category_id):
        if (str(appid)+"_"+str(category_id) not in Globals.AD_EXPOSURE_RATE.keys()):
            Globals.AD_EXPOSURE_RATE[str(appid)+"_"+str(category_id)] = {"appid":appid,"category_id":category_id,"show_count": 0, "click_count": 1, "down_count": 0}
        else:
            show_count = Globals.AD_EXPOSURE_RATE[str(appid)+"_"+str(category_id)]["click_count"] + 1
            Globals.AD_EXPOSURE_RATE[str(appid)+"_"+str(category_id)]["click_count"] = show_count
        item = {}
        item["action"] = "save_db_exposure_rate"
        self.at.mutex.acquire()
        self.at.work_list.append(item)
        self.at.mutex.release()

    #
    #函数：保存下载-曝光率次数
    #
    def save_down_ad_card(self,appid,category_id):
        if (str(appid)+"_"+str(category_id) not in Globals.AD_EXPOSURE_RATE.keys()):
            Globals.AD_EXPOSURE_RATE[str(appid)+"_"+str(category_id)] = {"appid": appid, "category_id": category_id, "show_count": 0,
                                                    "click_count": 0, "down_count": 1}
        else:
            show_count = Globals.AD_EXPOSURE_RATE[str(appid)+"_"+str(category_id)]["down_count"] + 1
            Globals.AD_EXPOSURE_RATE[str(appid)+"_"+str(category_id)]["down_count"] = show_count
        item = {}
        item["action"] = "save_db_exposure_rate"
        self.at.mutex.acquire()
        self.at.work_list.append(item)
        self.at.mutex.release()

    #
    # 函数：保存新品上架界面的曝光率
    #
    def save_new_arrivals_exposure(self, appid):
        if (str(appid) not in Globals.NEW_ARRIVALS_EXPOSURE_RATE.keys()):
            Globals.NEW_ARRIVALS_EXPOSURE_RATE[str(appid)] = {"appid": appid,"show_count": 1, "click_count": 0,"down_count": 0}
        else:
            show_count = Globals.NEW_ARRIVALS_EXPOSURE_RATE[str(appid)]["show_count"] + 1
            Globals.NEW_ARRIVALS_EXPOSURE_RATE[str(appid)]["show_count"] = show_count
        item = {}
        item["action"] = "save_new_arrivals_exposure_rate"
        self.at.mutex.acquire()
        self.at.work_list.append(item)
        self.at.mutex.release()

    #
    # 函数：保存点击卡片曝光率
    #
    def save_click_new_arrivals_card(self, appid):
        if (str(appid) not in Globals.NEW_ARRIVALS_EXPOSURE_RATE.keys()):
            Globals.NEW_ARRIVALS_EXPOSURE_RATE[str(appid)] = {"appid": appid,"show_count": 0, "click_count": 1,"down_count": 0}
        else:
            show_count = Globals.NEW_ARRIVALS_EXPOSURE_RATE[str(appid)]["click_count"] + 1
            Globals.NEW_ARRIVALS_EXPOSURE_RATE[str(appid)]["click_count"] = show_count
        item = {}
        item["action"] = "save_new_arrivals_exposure_rate"
        self.at.mutex.acquire()
        self.at.work_list.append(item)
        self.at.mutex.release()

    #
    #函数：获取点击新品上架下载曝光
    #
    def save_down_new_arrival_card(self,appid):
        if (str(appid) not in Globals.NEW_ARRIVALS_EXPOSURE_RATE.keys()):
            Globals.NEW_ARRIVALS_EXPOSURE_RATE[str(appid)] = {"appid": appid,  "show_count": 0,"click_count": 0, "down_count": 1}
        else:
            show_count = Globals.NEW_ARRIVALS_EXPOSURE_RATE[str(appid)]["down_count"] + 1
            Globals.NEW_ARRIVALS_EXPOSURE_RATE[str(appid)]["down_count"] = show_count
        item = {}
        item["action"] = "save_new_arrivals_exposure_rate"
        self.at.mutex.acquire()
        self.at.work_list.append(item)
        self.at.mutex.release()

    #
    # 函数：保存热门应用界面的曝光率
    #
    def save_hot_exposure(self, appid):
        if (str(appid) not in Globals.HOT_EXPOSURE_RATE.keys()):
            Globals.HOT_EXPOSURE_RATE[str(appid)] = {"appid": appid, "show_count": 1, "click_count": 0,
                                                              "down_count": 0}
        else:
            show_count = Globals.HOT_EXPOSURE_RATE[str(appid)]["show_count"] + 1
            Globals.HOT_EXPOSURE_RATE[str(appid)]["show_count"] = show_count
        item = {}
        item["action"] = "save_hot_exposure_rate"
        self.at.mutex.acquire()
        self.at.work_list.append(item)
        self.at.mutex.release()

    #
    # 函数：保存点击热门应用卡片曝光率
    #
    def save_click_hot_card(self, appid):
        if (str(appid) not in Globals.HOT_EXPOSURE_RATE.keys()):
            Globals.HOT_EXPOSURE_RATE[str(appid)] = {"appid": appid, "show_count": 0, "click_count": 1,
                                                              "down_count": 0}
        else:
            show_count = Globals.HOT_EXPOSURE_RATE[str(appid)]["click_count"] + 1
            Globals.HOT_EXPOSURE_RATE[str(appid)]["click_count"] = show_count
        item = {}
        item["action"] = "save_hot_exposure_rate"
        self.at.mutex.acquire()
        self.at.work_list.append(item)
        self.at.mutex.release()

    #
    # 函数：获取点击热门应用下载曝光
    #
    def save_down_hot_card(self, appid):
        if (str(appid) not in Globals.HOT_EXPOSURE_RATE.keys()):
            Globals.HOT_EXPOSURE_RATE[str(appid)] = {"appid": appid, "show_count": 0, "click_count": 0,
                                                              "down_count": 1}
        else:
            show_count = Globals.HOT_EXPOSURE_RATE[str(appid)]["down_count"] + 1
            Globals.HOT_EXPOSURE_RATE[str(appid)]["down_count"] = show_count
        item = {}
        item["action"] = "save_hot_exposure_rate"
        self.at.mutex.acquire()
        self.at.work_list.append(item)
        self.at.mutex.release()

    #
    #函数：检测是否要显示kmre推送模块
    #
    def detect_whether_to_display_kmre(self):
        try:
            kmre = self.at.appmgr.apt_cache[Globals.KYDROID_PACKAGE_NAME]
        except:
            kmre = None
        # kmre = None
        # if(self.kydroid.hasKydroid == True):
        #     need_show_kmre = False
        # else:
        num = 0
        for i in Globals.GET_ALL_APP_LIST:
            if (i["name"] == Globals.KYDROID_PACKAGE_NAME):
                break
            num = num + 1
        if(num > len(Globals.GET_ALL_APP_LIST)-1):
            return False
        need_show_kmre = False     #是否需要显示kmre推送
        if(kmre != None):
            if(kmre.is_installed == False):
                need_show_kmre = True
            else:
                conf = configparser.ConfigParser()
                conf.read(CONF_PATH)
                startup_time = os.popen("date -d \"$(awk -F. '{print $1}' /proc/uptime) second ago\" +\"%Y-%m-%d %H:%M:%S\"").readline().strip("\n")
                install_kmre = ""
                try:
                    install_kmre = conf.get("essential-information", "install_kmre")
                except:
                    pass
                if(install_kmre != ""):
                    format_pattern = "%Y-%m-%d %H:%M:%S"
                    difference = (datetime.datetime.strptime(startup_time, format_pattern) - datetime.datetime.strptime(install_kmre, format_pattern))
                    if(difference.days < 0):
                        need_show_kmre = True
        else:
            if(self.kydroid_service.hasKydroid == True):
                need_show_kmre = False
        return need_show_kmre

    #
    # 函数：点击安卓按钮
    #
    def slot_click_kydroid_btn(self):
        # if(not hasattr(self.detailwidget, "kydroid_widget")):
        #     self.detailwidget.kydroid_widget = Kydroid_Widget(self.width(), self.height() - 20, self.detailwidget)
        #     self.detailwidget.kydroid_widget.scrollArea.verticalScrollBar().valueChanged.connect(self.slot_scrollbar_valuechange)
        #     self.detailwidget.kydroid_widget.btn_widget.click_categoy.connect(self.detailwidget.slot_click_kydroid_type_btn)
        #     self.detailwidget.kydroid_widget.move(0, 10)
        #     self.detailwidget.kydroid_widget.hide()
        if(self.at.init_all_data == True):
            need_show_kmre = self.detect_whether_to_display_kmre()
            if(need_show_kmre == True):
                Globals.SEARCH_NOWPAGE = PageStates.KYDROID
                self.set_title_btn_stylesheet(PageStates.KYDROID)
                self.click_search_app(Globals.KYDROID_PACKAGE_NAME)
                self.set_title_btn_stylesheet(PageStates.KYDROID)
            else:
                Globals.NOWPAGE = PageStates.KYDROID
                self.slot_use_kmre_env()


    #
    #函数：使用安卓环境
    #
    def slot_use_kmre_env(self):
        self.title_bar.exit_login_btn.hide()
        self.title_bar.senior_search.hide()
        self.title_bar.search_line_back.setText(_("Please enter android appname"))
        self.title_bar.search_line_back.adjustSize()
        if (hasattr(self, "title_bar")):
            self.title_bar.search_line.setFocus(False)
            self.title_bar.search_line.clearFocus()
        if (hasattr(self, "search_tip_widget")):
            self.search_tip_widget.hide()
        if(hasattr(self.detailwidget,"kydroid_widget") and hasattr(self.detailwidget.kydroid_widget,"select_widget")):
            self.detailwidget.kydroid_widget.select_widget.hide()
        Globals.NOWPAGE = PageStates.KYDROID
        Globals.SEARCH_NOWPAGE = PageStates.KYDROID
        self.set_title_btn_stylesheet(PageStates.KYDROID)
        self.title_bar.search_line.clear()
        self.detailwidget.text_prompt_label.hide()
        self.detailwidget.no_found_tip.hide()
        self.detailwidget.no_app_tip_label.hide()
        if (hasattr(self.detailwidget, "app_detail_widget")):
            self.detailwidget.app_detail_widget.hide()
        if (hasattr(self.detailwidget, "category_widget")):
            self.detailwidget.category_widget.hide()
        if (hasattr(self.detailwidget, "drive_widget")):
            self.detailwidget.drive_widget.hide()
        self.detailwidget.mywidget.hide()
        self.detailwidget.show_homepage_widget.hide()
        self.detailwidget.show_homepage_widget2.hide()
        self.detailwidget.show_homepage_widget3.hide()
        self.detailwidget.scrollArea3.hide()
        self.detailwidget.scrollArea.hide()
        self.detailwidget.scrollArea2.hide()
        # page = {"pagetype": PageStates.KYDROID}
        # if (len(Globals.RETURN_PATH_LIST) >= 10):
        #     Globals.RETURN_PATH_LIST.pop(0)
        # Globals.RETURN_PATH_LIST.append(page)
        self.title_bar.home_page_btn.setEnabled(False)
        self.title_bar.my_btn.setEnabled(False)
        self.title_bar.drive_btn.setEnabled(False)
        self.title_bar.kydroid_btn.setEnabled(False)
        self.title_bar.category_btn.setEnabled(False)
        self.title_bar.search_line.setReadOnly(True)
        self.title_bar.senior_search.setEnabled(False)
        self.title_bar.search_image.setEnabled(False)
        self.title_bar.return_btn.setEnabled(False)
        self.title_bar.login_btn.setEnabled(False)
        # 709显卡 检测，临时使用
        if os.popen("lspci -n|awk '{print $3}' |grep '0709:'").read() != '':
            if (KYDROID_VERSION == "kydroid2"):
                self.show_no_kydroid(_("709 graphics card optimization, please look forward to!"))
                return
        if self.kydroid_service.hasKydroid == False:
            self.show_no_kydroid(_("Android running environment is not installed, please install it first!"))
            return
        if (not self.at.appmgr.check_kydroid_envrun()):
            Globals.apkpagefirst = True
            Globals.isOnline = True
            self.pkg_list_msg.clear()
            if(hasattr(self.detailwidget,"kydroid_widget")):
                self.clear_all_card(self.detailwidget.kydroid_widget.app_widget)
        if (Globals.apkpagefirst or Globals.isOnline == False):
            if not self.at.appmgr.check_kydroid_envrun():
                self.detailwidget.loading_kydroid_evn.show()
                self.detailwidget.loadinglabel.show()
                self.slot_kydroid_envrun()
            else:
                if self.kydroid_service.hasKydroid != False:
                    self.detailwidget.loading_kydroid_evn.show()
                    self.detailwidget.loadinglabel.show()
                    self.detailwidget.show_homepage_widget.hide()
                    self.detailwidget.scrollArea.hide()
                    self.detailwidget.show_homepage_widget2.hide()
                    self.detailwidget.show_homepage_widget3.hide()
                    self.detailwidget.scrollArea3.hide()
                    self.detailwidget.scrollArea2.hide()
                    # if(self.detailwidget.mywidget in self.path_of_particle):
                    #     self.path_of_particle.remove(self.detailwidget.mywidget)
                    self.detailwidget.mywidget.hide()
                    if (hasattr(self.detailwidget, "category_widget")):
                        # if(self.detailwidget.category_widget in self.path_of_particle):
                        #     self.path_of_particle.remove(self.detailwidget.category_widget)
                        self.detailwidget.category_widget.hide()
                    if (hasattr(self.detailwidget, "drive_widget")):
                        self.detailwidget.drive_widget.hide()
                    item = {}
                    item["action"] = "apk_page_create_emit"
                    item["need_show_widget"] = True
                    self.ask_thread.mutex.acquire()
                    self.ask_thread.work_list.append(item)
                    self.ask_thread.mutex.release()
        else:
            if (len(self.at.appmgr.apk_list) == 0):
                self.detailwidget.loading_kydroid_evn.show()
                self.detailwidget.loadinglabel.show()
                item = {}
                item["action"] = "download_kydroid_sl"
                self.at.mutex.acquire()
                self.at.work_list.append(item)
                self.at.mutex.release()
                # if (len(self.detailwidget.kydroid_widget.app_widget.children()) < len(self.at.appmgr.apk_list)):
                #     self.pkg_list_msg.extend(self.at.appmgr.apk_list)
                #     for i in range(len(self.detailwidget.kydroid_widget.app_widget.children())):
                #         if(len(self.pkg_list_msg) != 0):
                #             self.pkg_list_msg.pop(0)
                self.detailwidget.loading_kydroid_evn.hide()
                self.detailwidget.loadinglabel.hide()
                self.title_bar.home_page_btn.setEnabled(True)
                self.title_bar.my_btn.setEnabled(True)
                self.title_bar.drive_btn.setEnabled(True)
                self.title_bar.kydroid_btn.setEnabled(True)
                self.title_bar.category_btn.setEnabled(True)
                self.title_bar.search_line.setReadOnly(False)
                self.title_bar.senior_search.setEnabled(True)
                self.title_bar.search_image.setEnabled(True)
                self.title_bar.return_btn.setEnabled(True)
                self.title_bar.login_btn.setEnabled(True)
                self.detailwidget.show_homepage_widget.hide()
                self.detailwidget.scrollArea.hide()
                self.detailwidget.show_homepage_widget2.hide()
                self.detailwidget.scrollArea2.hide()
                self.detailwidget.show_homepage_widget3.hide()
                self.detailwidget.scrollArea3.hide()
                # if(self.detailwidget.mywidget in self.path_of_particle):
                #     self.path_of_particle.remove(self.detailwidget.mywidget)
                self.detailwidget.mywidget.hide()
                if (hasattr(self.detailwidget, "category_widget")):
                    self.detailwidget.category_widget.hide()
                    # if(self.detailwidget.category_widget in self.path_of_particle):
                    #     self.path_of_particle.remove(self.detailwidget.category_widget)
                # if(len(self.pkg_list_msg) == 0):
                #     self.pkg_list_msg.extend(self.at.appmgr.apk_list)
                if (hasattr(self.detailwidget, "kydroid_widget")):
                    self.detailwidget.kydroid_widget.show()
                if (hasattr(self.detailwidget, "drive_widget")):
                    self.detailwidget.drive_widget.hide()
                if (len(self.at.appmgr.apk_list) != 0):
                    self.slot_show_kydroid_widget(True)
            else:
                self.slot_show_kydroid_widget(True)

        # self.path_of_particle.append(self.detailwidget.kydroid_widget)
        pass

    def show_no_kydroid(self, str):
        self.title_bar.home_page_btn.setEnabled(True)
        self.title_bar.my_btn.setEnabled(True)
        self.title_bar.drive_btn.setEnabled(True)
        self.title_bar.kydroid_btn.setEnabled(True)
        self.title_bar.category_btn.setEnabled(True)
        self.title_bar.search_line.setReadOnly(False)
        self.title_bar.senior_search.setEnabled(True)
        self.title_bar.search_image.setEnabled(True)
        self.title_bar.return_btn.setEnabled(True)
        self.title_bar.login_btn.setEnabled(True)
        if (Globals.THEME_COLOR == 0):
            self.detailwidget.no_app_tip_label.setStyleSheet(
                "QLabel{border-radius: 12px;background-image:url(./res/no_download.png);}")
        else:
            self.detailwidget.no_app_tip_label.setStyleSheet(
                "QLabel{border-radius: 12px;background-image:url(./res/no_download_dark.png);}")
        self.detailwidget.text_prompt_label.setText(
            _("Android running environment is not installed, please install it first!"))
        # self.detailwidget.no_found_env_text1.setText(_("Kydroid running environment is not installed, click "))
        # self.detailwidget.no_found_env_text1.adjustSize()
        # self.detailwidget.down_env_btn.setText(_("\"here\""))
        # self.detailwidget.down_env_btn.adjustSize()
        # self.detailwidget.no_found_env_text2.setText("to download android environment")
        # self.detailwidget.no_found_env_text2.adjustSize()
        self.detailwidget.text_prompt_label.adjustSize()
        self.detailwidget.text_prompt_label.move(
            int(((self.detailwidget.width() - self.detailwidget.text_prompt_label.width()) / 2) - 15),
            int(self.detailwidget.no_app_tip_label.y() + self.detailwidget.no_app_tip_label.height() + 10))
        self.detailwidget.no_app_tip_label.move(
            int(((self.detailwidget.width() - self.detailwidget.no_app_tip_label.width()) / 2) - 15), int(((self.detailwidget.height() - self.detailwidget.no_app_tip_label.height() - self.detailwidget.text_prompt_label.height() + 10) / 2)))
        #
        # self.detailwidget.no_found_env_text1.move(int((self.detailwidget.width() - 144 * 2 - (
        #             self.detailwidget.no_found_env_text1.width() + self.detailwidget.down_env_btn.width() + self.detailwidget.no_found_env_text2.width())) / 2),
        #                                           self.detailwidget.no_app_tip_label.y() + self.detailwidget.no_app_tip_label.height() + 10)
        # print(self.detailwidget.no_found_env_text1.x(),self.detailwidget.no_found_env_text1.y(),self.detailwidget.no_app_tip_label.y())
        self.detailwidget.text_prompt_label.show()

        # self.detailwidget.no_found_tip.show()
        self.detailwidget.no_app_tip_label.show()
        self.detailwidget.scrollArea.hide()
        self.detailwidget.show_homepage_widget2.hide()
        self.detailwidget.show_homepage_widget3.hide()
        self.detailwidget.scrollArea3.hide()
        self.detailwidget.scrollArea2.hide()
        # self.path_of_particle.append(self.detailwidget.kydroid_widget)
        return

    #
    # 函数名: 安卓环境相关
    # Function: Android envrun
    #
    def slot_kydroid_envrun(self):
        os.system(KYDROID_STARTAPP_ENV + " &")
        item = {}
        item["action"] = "cycle_check_kydroid_envrun"
        item["need_show_kyroid_widget"] = True
        self.at.mutex.acquire()
        self.at.work_list.append(item)
        self.at.mutex.release()
        self.detailwidget.show_homepage_widget.hide()
        self.detailwidget.scrollArea.hide()
        self.detailwidget.show_homepage_widget2.hide()
        self.detailwidget.scrollArea2.hide()
        self.detailwidget.show_homepage_widget3.hide()
        self.detailwidget.scrollArea3.hide()
        self.detailwidget.loading_kydroid_evn.show()
        self.detailwidget.loadinglabel.show()
        if (hasattr(self.detailwidget, "category_widget")):
            self.detailwidget.category_widget.hide()
        if (hasattr(self.detailwidget, "drive_widget")):
            self.detailwidget.drive_widget.hide()

    #
    # 函数名: 安卓环境
    # Function: Android envrun
    #
    def slot_kydroid_envrun_over(self, envrun,need_show_widget):
        if envrun:
            # self.clear_all_card(self.detailwidget.kydroid_widget)
            if (Globals.INIT_APK_LIST_OVER == False):
                item = {}
                item["action"] = "apk_page_create_emit"
                item["need_show_widget"] = need_show_widget
                self.ask_thread.mutex.acquire()
                self.ask_thread.work_list.append(item)
                self.ask_thread.mutex.release()
            else:
                # if(need_show_widget):
                self.slot_show_kydroid_widget(need_show_widget)
        else:
            # self.messageBox.alert_msg("安卓环境启动异常，操作失败！")
            # self.messageBox.alert_msg(_("Android environment starts abnormally, operation fails！"))
            # self.show_tip_widget(_("Android environment starts abnormally, operation fails！"))
            # self.tip_widget.btn.clicked.connect(self.slot_close_tipwidget)
            if (Globals.THEME_COLOR == 0):
                self.slot_network_error("./res/no_network.png",
                                        _("Unable to connect to Android software source. Please try again later!"))
            else:
                self.slot_network_error("./res/no_network_dark.png",
                                        _("Unable to connect to Android software source. Please try again later!"))

    #
    # 函数:显示安卓软件界面
    #
    def slot_show_kydroid_widget(self,need_show_widget):
        self.change_kydroid_card_btn_state.emit()
        if(need_show_widget):
            if (not hasattr(self.detailwidget, "kydroid_widget")):
                self.detailwidget.kydroid_widget = Kydroid_Widget(self.detailwidget.width(), self.detailwidget.height() - 20, self.detailwidget)
                self.detailwidget.kydroid_widget.scrollArea.verticalScrollBar().valueChanged.connect( self.slot_scrollbar_valuechange)
                self.detailwidget.kydroid_widget.btn_widget.click_categoy.connect(self.detailwidget.slot_click_kydroid_type_btn)
                self.detailwidget.kydroid_widget.click_type_btn.connect(self.detailwidget.slot_click_kydroid_type_btn)
                self.detailwidget.kydroid_widget.all_check_btn.clicked.connect(lambda :self.slot_category_all_select(self.detailwidget.kydroid_widget))
                self.detailwidget.kydroid_widget.one_click_download.clicked.connect(self.slot_category_one_click_download)
                self.detailwidget.kydroid_widget.move(0, 10)
                self.detailwidget.kydroid_widget.hide()
            # if (len(self.detailwidget.kydroid_widget.app_widget.children()) < len(self.at.appmgr.apk_list)):
            #     self.pkg_list_msg.clear()
            #     self.pkg_list_msg.extend(self.at.appmgr.apk_list)
            #     for i in range(len(self.detailwidget.kydroid_widget.app_widget.children())):
            #         if (len(self.pkg_list_msg) != 0):
            #             self.pkg_list_msg.pop(0)
            # if(need_show_widget):
            #     self.add_card_to_inside_widget(self.pkg_list_msg, self.detailwidget.kydroid_widget.app_widget,
            #                                    self.detailwidget.kydroid_widget.app_widget, False)
            #     self.detailwidget.loadinglabel.hide()
            #     self.detailwidget.loading_kydroid_evn.hide()
            #     self.detailwidget.text_prompt_label.hide()
            #     self.detailwidget.no_found_tip.hide()
            #     self.detailwidget.no_app_tip_label.hide()
            #     self.detailwidget.kydroid_widget.show()
            #     self.detailwidget.kydroid_widget.scrollArea.show()
                Globals.NOW_KYDROID_CATEGORY = self.detailwidget.kydroid_widget.btn_widget.first_btn
                self.detailwidget.slot_click_kydroid_type_btn(self.detailwidget.kydroid_widget.btn_widget.first_btn)
            else:
                self.detailwidget.slot_click_kydroid_type_btn(self.detailwidget.kydroid_widget.type_name_label.text())
        self.title_bar.home_page_btn.setEnabled(True)
        self.title_bar.my_btn.setEnabled(True)
        self.title_bar.drive_btn.setEnabled(True)
        self.title_bar.kydroid_btn.setEnabled(True)
        self.title_bar.category_btn.setEnabled(True)
        self.title_bar.search_line.setReadOnly(False)
        self.title_bar.senior_search.setEnabled(True)
        self.title_bar.search_image.setEnabled(True)
        self.title_bar.return_btn.setEnabled(True)
        self.title_bar.login_btn.setEnabled(True)
        pass

    #
    # 函数：点击my_btn,获取需要更新的软件数量
    #
    def slot_click_my_btn(self):
        if (hasattr(self.detailwidget, "drive_widget")):
            self.detailwidget.drive_widget.hide()
        if (self.detailwidget.mywidget.type_name_label.text() == _("Onload")):
            self.title_bar.search_line_back.setText(_("Please enter onload appname"))
            self.title_bar.search_line_back.adjustSize()
            Globals.NOWPAGE = PageStates.DOWNLOADINGPAGE
            Globals.SEARCH_NOWPAGE = PageStates.DOWNLOADINGPAGE
            page = {"pagetype": PageStates.DOWNLOADINGPAGE}
            if (len(Globals.RETURN_PATH_LIST) >= 10):
                Globals.RETURN_PATH_LIST.pop(0)
            if (len(Globals.RETURN_PATH_LIST) >= 1 and Globals.RETURN_PATH_LIST[len(Globals.RETURN_PATH_LIST) - 1] != page):
                Globals.RETURN_PATH_LIST.append(page)
        elif (self.detailwidget.mywidget.type_name_label.text() == _("Installation")):
            self.title_bar.search_line_back.setText(_("Please enter installation appname"))
            self.title_bar.search_line_back.adjustSize()
            Globals.NOWPAGE = PageStates.HISTORY
            Globals.SEARCH_NOWPAGE = PageStates.HISTORY
            page = {"pagetype": PageStates.HISTORY, "Type": self.detailwidget.mywidget.history_widget.now_btn_name}
            if (len(Globals.RETURN_PATH_LIST) >= 10):
                Globals.RETURN_PATH_LIST.pop(0)
            if (len(Globals.RETURN_PATH_LIST) >= 1 and Globals.RETURN_PATH_LIST[len(Globals.RETURN_PATH_LIST) - 1] != page):
                Globals.RETURN_PATH_LIST.append(page)
        self.set_title_btn_stylesheet(PageStates.MYPAGE)
        self.title_bar.search_line.clear()
        self.detailwidget.text_prompt_label.hide()
        self.detailwidget.no_found_tip.hide()
        self.detailwidget.no_app_tip_label.hide()
        self.detailwidget.mywidget.scrollArea1.verticalScrollBar().setValue(0)
        self.detailwidget.show_my_widget()
        self.detailwidget.scrollArea.hide()
        self.detailwidget.show_homepage_widget.hide()
        self.detailwidget.show_homepage_widget2.hide()
        self.detailwidget.show_homepage_widget3.hide()
        self.detailwidget.scrollArea3.hide()
        self.detailwidget.scrollArea2.hide()
        item = {"action": ""}
        item["action"] = "get_update_num"
        self.at.mutex.acquire()
        self.at.work_list.append(item)
        self.at.mutex.release()
        # applist, apklist = self.worker_thread0.appmgr.get_history_install_app(True)
        # self.detailwidget.mywidget.update_num_label.setText(str(len(applist) + len(apklist)))
        self.detailwidget.mywidget.update_tip_spot.hide()

    #
    # 函数：修改tital界面按钮状态
    #
    def set_title_btn_stylesheet(self, widget_name):
        if (Globals.THEME_COLOR == 0):
            if (widget_name == PageStates.HOMEPAGE):
                self.title_bar.home_page_btn.setStyleSheet(
                    "QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 500;line-height: 20px;text-align: center;color:#3790FA;}")
                self.title_bar.home_page_label.show()
                self.title_bar.category_btn.setStyleSheet(
                    "QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 400;color: #535353;text-align: center;line-height: 20px;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
                self.title_bar.category_label.hide()
                self.title_bar.drive_btn.setStyleSheet(
                    "QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 400;color: #535353;text-align: center;line-height: 20px;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
                self.title_bar.drive_label.hide()
                self.title_bar.kydroid_btn.setStyleSheet(
                    "QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 400;color: #535353;text-align: center;line-height: 20px;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
                self.title_bar.kydroid_label.hide()
                self.title_bar.my_btn.setStyleSheet(
                    "QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 400;color: #535353;text-align: center;line-height: 20px;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
                self.title_bar.my_label.hide()
            elif (widget_name == PageStates.CAREGORYPAGE):
                self.title_bar.home_page_btn.setStyleSheet(
                    "QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 400;color: #535353;text-align: center;line-height: 20px;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
                self.title_bar.home_page_label.hide()
                self.title_bar.category_btn.setStyleSheet(
                    "QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 500;line-height: 20px;color:#3790FA;text-align: center;}")
                self.title_bar.category_label.show()
                self.title_bar.drive_btn.setStyleSheet(
                    "QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 400;color: #535353;text-align: center;line-height: 20px;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
                self.title_bar.drive_label.hide()
                self.title_bar.kydroid_btn.setStyleSheet(
                    "QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 400;color: #535353;line-height: 20px;text-align: center;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
                self.title_bar.kydroid_label.hide()
                self.title_bar.my_btn.setStyleSheet(
                    "QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 400;color: #535353;line-height: 20px;text-align: center;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
                self.title_bar.my_label.hide()
            elif (widget_name == PageStates.DRIVE):
                self.title_bar.home_page_btn.setStyleSheet(
                    "QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 400;color: #535353;text-align: center;line-height: 20px;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
                self.title_bar.home_page_label.hide()
                self.title_bar.category_btn.setStyleSheet(
                    "QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 400;color: #535353;text-align: center;line-height: 20px;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
                self.title_bar.category_label.hide()
                self.title_bar.drive_btn.setStyleSheet(
                    "QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 500;line-height: 20px;color:#3790FA;text-align: center;}")
                self.title_bar.drive_label.show()
                self.title_bar.kydroid_btn.setStyleSheet(
                    "QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 400;color: #535353;line-height: 20px;text-align: center;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
                self.title_bar.kydroid_label.hide()
                self.title_bar.my_btn.setStyleSheet(
                    "QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 400;color: #535353;line-height: 20px;text-align: center;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
                self.title_bar.my_label.hide()
            elif (widget_name == PageStates.KYDROID):
                self.title_bar.home_page_btn.setStyleSheet(
                    "QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 400;color: #535353;line-height: 20px;text-align: center;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
                self.title_bar.home_page_label.hide()
                self.title_bar.category_btn.setStyleSheet(
                    "QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 400;color: #535353;line-height: 20px;text-align: center;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
                self.title_bar.category_label.hide()
                self.title_bar.drive_btn.setStyleSheet(
                    "QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 400;color: #535353;text-align: center;line-height: 20px;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
                self.title_bar.drive_label.hide()
                self.title_bar.kydroid_btn.setStyleSheet(
                    "QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 500;line-height: 20px;color:#3790FA;text-align: center;}")
                self.title_bar.kydroid_label.show()
                self.title_bar.my_btn.setStyleSheet(
                    "QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 400;color: #535353;line-height: 20px;text-align: center;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
                self.title_bar.my_label.hide()
            elif (widget_name == PageStates.MYPAGE):
                self.title_bar.home_page_btn.setStyleSheet(
                    "QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 400;color: #535353;line-height: 20px;text-align: center;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
                self.title_bar.home_page_label.hide()
                self.title_bar.category_btn.setStyleSheet(
                    "QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 400;color: #535353;line-height: 20px;text-align: center;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
                self.title_bar.category_label.hide()
                self.title_bar.drive_btn.setStyleSheet(
                    "QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 400;color: #535353;text-align: center;line-height: 20px;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
                self.title_bar.drive_label.hide()
                self.title_bar.kydroid_btn.setStyleSheet(
                    "QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 400;color: #535353;line-height: 20px;text-align: center;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
                self.title_bar.kydroid_label.hide()
                self.title_bar.my_btn.setStyleSheet(
                    "QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 500;line-height: 20px;color:#3790FA;text-align: center;}")
                self.title_bar.my_label.show()
        else:
            if (widget_name == PageStates.HOMEPAGE):
                self.title_bar.home_page_btn.setStyleSheet(
                    "QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 500;line-height: 20px;text-align: center;color:#3790FA;}")
                self.title_bar.home_page_label.show()
                self.title_bar.category_btn.setStyleSheet(
                    "QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 400;color: #C0C4CC;text-align: center;line-height: 20px;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
                self.title_bar.category_label.hide()
                self.title_bar.drive_btn.setStyleSheet(
                    "QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 400;color: #C0C4CC;text-align: center;line-height: 20px;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
                self.title_bar.drive_label.hide()
                self.title_bar.kydroid_btn.setStyleSheet(
                    "QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 400;color: #C0C4CC;text-align: center;line-height: 20px;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
                self.title_bar.kydroid_label.hide()
                self.title_bar.my_btn.setStyleSheet(
                    "QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 400;color: #C0C4CC;text-align: center;line-height: 20px;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
                self.title_bar.my_label.hide()
            elif (widget_name == PageStates.CAREGORYPAGE):
                self.title_bar.home_page_btn.setStyleSheet(
                    "QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 400;color: #C0C4CC;text-align: center;line-height: 20px;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
                self.title_bar.home_page_label.hide()
                self.title_bar.category_btn.setStyleSheet(
                    "QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 500;line-height: 20px;color:#3790FA;text-align: center;}")
                self.title_bar.category_label.show()
                self.title_bar.drive_btn.setStyleSheet(
                    "QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 400;color: #C0C4CC;text-align: center;line-height: 20px;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
                self.title_bar.drive_label.hide()
                self.title_bar.kydroid_btn.setStyleSheet(
                    "QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 20px;text-align: center;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
                self.title_bar.kydroid_label.hide()
                self.title_bar.my_btn.setStyleSheet(
                    "QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 20px;text-align: center;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
                self.title_bar.my_label.hide()
            elif (widget_name == PageStates.DRIVE):
                self.title_bar.home_page_btn.setStyleSheet(
                    "QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 20px;text-align: center;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
                self.title_bar.home_page_label.hide()
                self.title_bar.category_btn.setStyleSheet(
                    "QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 20px;text-align: center;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
                self.title_bar.category_label.hide()
                self.title_bar.drive_btn.setStyleSheet(
                    "QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 500;line-height: 20px;color:#3790FA;text-align: center;}")
                self.title_bar.drive_label.show()
                self.title_bar.kydroid_btn.setStyleSheet(
                    "QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 20px;text-align: center;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
                self.title_bar.kydroid_label.hide()
                self.title_bar.my_btn.setStyleSheet(
                    "QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 20px;text-align: center;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
                self.title_bar.my_label.hide()
            elif (widget_name == PageStates.KYDROID):
                self.title_bar.home_page_btn.setStyleSheet(
                    "QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 20px;text-align: center;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
                self.title_bar.home_page_label.hide()
                self.title_bar.category_btn.setStyleSheet(
                    "QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 20px;text-align: center;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
                self.title_bar.category_label.hide()
                self.title_bar.drive_btn.setStyleSheet(
                    "QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 400;color: #C0C4CC;text-align: center;line-height: 20px;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
                self.title_bar.drive_label.hide()
                self.title_bar.kydroid_btn.setStyleSheet(
                    "QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 500;line-height: 20px;color:#3790FA;text-align: center;}")
                self.title_bar.kydroid_label.show()
                self.title_bar.my_btn.setStyleSheet(
                    "QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 20px;text-align: center;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
                self.title_bar.my_label.hide()
            elif (widget_name == PageStates.MYPAGE):
                self.title_bar.home_page_btn.setStyleSheet(
                    "QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 20px;text-align: center;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
                self.title_bar.home_page_label.hide()
                self.title_bar.category_btn.setStyleSheet(
                    "QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 20px;text-align: center;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
                self.title_bar.category_label.hide()
                self.title_bar.drive_btn.setStyleSheet(
                    "QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 400;color: #C0C4CC;text-align: center;line-height: 20px;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
                self.title_bar.drive_label.hide()
                self.title_bar.kydroid_btn.setStyleSheet(
                    "QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 20px;text-align: center;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
                self.title_bar.kydroid_label.hide()
                self.title_bar.my_btn.setStyleSheet(
                    "QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 500;line-height: 20px;color:#3790FA;text-align: center;}")
                self.title_bar.my_label.show()

    #
    # 函数：点击登录界面
    #
    def slot_login_account(self):
        if (hasattr(self, "title_bar")):
            self.title_bar.search_line.setFocus(False)
            self.title_bar.search_line.clearFocus()
        if (hasattr(self, "search_tip_widget")):
            self.search_tip_widget.hide()
        self.title_bar.senior_search.hide()
        # self.title_bar.search_line.clear()
        # self.detailwidget.text_prompt_label.hide()
        # self.detailwidget.no_app_tip_label.hide()
        if (hasattr(self, "loginwidget")):
            self.loginwidget.ui.lesource_2.setText("")
            self.loginwidget.close()
        if (Globals.LOGIN_SUCCESS == False):
            if (self.ask_thread.kylinid_iface == None):
                self.loginwidget = Login(self, self.frame_widget)
                self.show_image.connect(self.loginwidget.slot_show_image)
                self.loginwidget.setWindowModality(Qt.ApplicationModal)
                # self.loginwidget.close_button.clicked.connect(self.slot_click_loginwidget_close_button)
                # self.loginwidget.move(int(self.x()+(self.width()-self.loginwidget.width())/2),int(self.y()+(self.height()-self.loginwidget.height())/2))
                self.loginwidget.move(int(self.x() + (self.width() - self.loginwidget.width()) / 2),
                                      int(self.y() + (self.height() - self.loginwidget.height()) / 2))
                self.loginwidget.slot_show_dark_mode()
                self.loginwidget.show()

                # caoliang
                self.loginwidget.ui_login.connect(self.slot_ui_login)
                self.loginwidget.ui_login_phone_number.connect(self.slot_ui_login_phone_num)
                self.loginwidget.ui_login_get_image_code.connect(self.slot_ui_login_get_image_code)
                try:
                    self.at.appmgr.get_ui_login_over.disconnect()
                    self.at.appmgr.get_ui_login_over_fail.disconnect()
                except:
                    pass
                self.at.appmgr.get_ui_login_over.connect(self.loginwidget.slot_get_ui_login_over)
                self.at.appmgr.get_ui_login_over_fail.connect(self.loginwidget.slot_get_ui_login_over)
                # self.at.appmgr.get_ui_login_over.connect(self.submit_pinback)
                self.loginwidget.ui_login_success.connect(self.slot_ui_login_success)
                # self.loginwidget.ui_login_success.connect(self.loginwidget.slot_rem_user_pwd)
                if Globals.USER_AND_PWD == []:

                    self.loginwidget.save_usrname_password.connect(self.at.appmgr.db.save_password_at_mysql)
                    # self.loginwidget.save_user_pwd = self.at.appmgr.db.select_username_password()
                    Globals.USER_AND_PWD = self.at.appmgr.db.select_username_password()

                    self.loginwidget.decode_user_pwd()
                else:
                    self.loginwidget.show_save_user()
            else:
                item = {"action": "show_kylinId_login"}
                self.ask_thread.mutex.acquire()
                self.ask_thread.work_list.append(item)
                self.ask_thread.mutex.release()
        else:
            self.title_bar.exit_login_btn.setWindowModality(Qt.ApplicationModal)
            self.title_bar.exit_login_btn.show()
            pass

    #
    # 函数名: 登录成功
    # Function: login success
    #
    def slot_ui_login_success(self):
        Globals.USER = ""
        Globals.PASSWORD = ""
        Globals.USER_AND_PWD =[]
        if (hasattr(self, "loginwidget")):
            self.loginwidget.ui.lesource_2.setText("")
            self.loginwidget.save_user_pwd = []
            self.loginwidget.listlogin = []
        if not Globals.RE_LOGIN:
            if (hasattr(self, "loginwidget")):
                if not self.loginwidget.ui.lesource_1.isHidden():
                    Globals.LOGIN_SUCCESS = True
                    if (hasattr(self.detailwidget, "app_detail_widget")):
                        self.detailwidget.app_detail_widget.Login_status_check()
                        if (self.detailwidget.app_detail_widget.isHidden() == False):
                            self.detailwidget.app_detail_widget.get_my_rating.emit(
                                str(self.detailwidget.app_detail_widget.app_msg.app_id),
                                self.detailwidget.app_detail_widget.app_msg.pkgname)
                    self.loginwidget.slot_click_close()
                    # print("name1",Globals.LOGIN_ACCOUNT)
                    self.title_bar.login_btn.setLongTextToElideFormat(self.title_bar.login_btn.user_name_label,
                                                                      Globals.LOGIN_ACCOUNT)
                    # self.title_bar.login_btn.user_name_label.setText(Globals.LOGIN_ACCOUNT)
                    user_profile = UBUNTUKYLIN_DATA_PATH + "user_profile/" + Globals.USER_PROFILE
                    pix2 = QPixmap(user_profile)
                    fitpixmap_userIcon = pix2.scaled(20, 20, Qt.IgnoreAspectRatio, Qt.SmoothTransformation)
                    fitpixmap_userIcon = self.PixmapToRound(fitpixmap_userIcon, 20,
                                                            20)
                    self.title_bar.login_btn.head_portrait_label.setPixmap(fitpixmap_userIcon)
                    # self.title_bar.login_btn.head_portrait_label.setStyleSheet("QLabel{border:0px;border-radius: 10px;border-image: url('"+user_profile+"') 0 0 0 0 stretch strectch;background-color:transparent;}")
                    self.title_bar.login_btn.user_name_label.adjustSize()

                    self.title_bar.login_btn.setLongTextToElideFormat(self.title_bar.login_btn.user_name_label,
                                                                      Globals.LOGIN_ACCOUNT)
                    self.title_bar.login_btn.btn.resize(
                        self.title_bar.login_btn.user_name_label.width() + 4 + self.title_bar.login_btn.head_portrait_label.width(),
                        20)
                    if ("…" in self.title_bar.login_btn.user_name_label.text()):
                        self.title_bar.login_btn.user_name_label.setToolTip(Globals.LOGIN_ACCOUNT)
                else:
                    self.timer_set()
            else:
                Globals.LOGIN_SUCCESS = True
                if (hasattr(self.detailwidget, "app_detail_widget")):
                    self.detailwidget.app_detail_widget.Login_status_check()
                    if (self.detailwidget.app_detail_widget.isHidden() == False):
                        self.detailwidget.app_detail_widget.get_my_rating.emit(
                            str(self.detailwidget.app_detail_widget.app_msg.app_id),
                            self.detailwidget.app_detail_widget.app_msg.pkgname)
                # print("name1",Globals.LOGIN_ACCOUNT)
                self.title_bar.login_btn.setLongTextToElideFormat(self.title_bar.login_btn.user_name_label,
                                                                  Globals.LOGIN_ACCOUNT)
                # self.title_bar.login_btn.user_name_label.setText(Globals.LOGIN_ACCOUNT)
                user_profile = UBUNTUKYLIN_DATA_PATH + "user_profile/" + Globals.USER_PROFILE
                pix2 = QPixmap(user_profile)
                fitpixmap_userIcon = pix2.scaled(20, 20, Qt.IgnoreAspectRatio, Qt.SmoothTransformation)
                fitpixmap_userIcon = self.PixmapToRound(fitpixmap_userIcon, 20,
                                                        20)
                self.title_bar.login_btn.head_portrait_label.setPixmap(fitpixmap_userIcon)
                # self.title_bar.login_btn.head_portrait_label.setStyleSheet("QLabel{border:0px;border-radius: 10px;border-image: url('"+user_profile+"') 0 0 0 0 stretch strectch;background-color:transparent;}")
                self.title_bar.login_btn.user_name_label.adjustSize()

                self.title_bar.login_btn.setLongTextToElideFormat(self.title_bar.login_btn.user_name_label,
                                                                  Globals.LOGIN_ACCOUNT)
                self.title_bar.login_btn.btn.resize(
                    self.title_bar.login_btn.user_name_label.width() + 4 + self.title_bar.login_btn.head_portrait_label.width(),
                    20)
                if ("…" in self.title_bar.login_btn.user_name_label.text()):
                    self.title_bar.login_btn.user_name_label.setToolTip(Globals.LOGIN_ACCOUNT)
        else:
            Globals.LOGIN_SUCCESS = True
            if (hasattr(self.detailwidget, "app_detail_widget")):
                self.detailwidget.app_detail_widget.Login_status_check()
                if (self.detailwidget.app_detail_widget.isHidden() == False):
                    self.detailwidget.app_detail_widget.get_my_rating.emit(
                        str(self.detailwidget.app_detail_widget.app_msg.app_id),
                        self.detailwidget.app_detail_widget.app_msg.pkgname)
            # self.loginwidget.slot_click_close()
            self.title_bar.login_btn.setLongTextToElideFormat(self.title_bar.login_btn.user_name_label,Globals.LOGIN_ACCOUNT)

            user_profile = UBUNTUKYLIN_DATA_PATH + "user_profile/" + Globals.USER_PROFILE
            pix2 = QPixmap(user_profile)
            fitpixmap_userIcon = pix2.scaled(20, 20, Qt.IgnoreAspectRatio, Qt.SmoothTransformation)
            fitpixmap_userIcon = self.PixmapToRound(fitpixmap_userIcon, 20,
                                                    20)
            self.title_bar.login_btn.head_portrait_label.setPixmap(fitpixmap_userIcon)
            # self.title_bar.login_btn.head_portrait_label.setStyleSheet("QLabel{border:0px;border-radius: 10px;border-image: url('"+user_profile+"') 0 0 0 0 stretch strectch;background-color:transparent;}")
            self.title_bar.login_btn.user_name_label.adjustSize()

            self.title_bar.login_btn.setLongTextToElideFormat(self.title_bar.login_btn.user_name_label,
                                                              Globals.LOGIN_ACCOUNT)
            self.title_bar.login_btn.btn.resize(
                self.title_bar.login_btn.user_name_label.width() + 4 + self.title_bar.login_btn.head_portrait_label.width(),
                20)
            self.title_bar.login_btn.setLongTextToElideFormat(self.title_bar.login_btn.user_name_label,
                                                              Globals.LOGIN_ACCOUNT)
            if ("…" in self.title_bar.login_btn.user_name_label.text()):
                self.title_bar.login_btn.user_name_label.setToolTip(Globals.LOGIN_ACCOUNT)

    # 函数名:
    # Function:oprate password
    #
    def operate(self):

        self.loginwidget.slot_click_close()
        # self.title_bar.login_btn.setEnabled(False)
        # self.ui.tips_user_password.hide()
        self.timer_hide_login.stop()

    # 函数：退出登录
    #
    def slot_exit_login(self):
        self.title_bar.login_btn.setEnabled(False)
        self.title_bar.exit_login_btn.setEnabled(False)
        # self.title_bar.login_btn.user_name_label.setText(_("Exiting..."))
        # self.title_bar.login_btn.user_name_label.adjustSize()
        self.title_bar.login_btn.setLongTextToElideFormat(self.title_bar.login_btn.user_name_label,
                                                          _("Exiting..."))
        self.title_bar.login_btn.user_name_label.resize(self.title_bar.login_btn.user_name_label.width() + 3, 20)
        self.title_bar.login_btn.btn.resize(
            self.title_bar.login_btn.user_name_label.width() + 4 + self.title_bar.login_btn.head_portrait_label.width(),
            20)

        if (hasattr(self.title_bar, "exit_login_btn")):
            self.title_bar.exit_login_btn.hide()
        self.title_bar.senior_search.hide()
        if (hasattr(self, "title_bar")):
            self.title_bar.search_line.setFocus(False)
            self.title_bar.search_line.clearFocus()
        if (hasattr(self, "search_tip_widget")):
            self.search_tip_widget.hide()
        # self.title_bar.setWindowModality(Qt.NonModal)
        item = {"action": "exit_login"}
        self.ask_thread.mutex.acquire()
        self.ask_thread.work_list.append(item)
        self.ask_thread.mutex.release()
        if Globals.RE_LOGIN:
            Globals.RE_LOGIN = False
            # pass
        # else:
        #     self.loginwidget.hide()

    # 函数名:设置延时
    # Function:set timer
    #
    def timer_set(self):
        Globals.LOGIN_SUCCESS = True
        if (hasattr(self.detailwidget, "app_detail_widget")):
            self.detailwidget.app_detail_widget.Login_status_check()
            if (self.detailwidget.app_detail_widget.isHidden() == False):
                self.detailwidget.app_detail_widget.get_my_rating.emit(
                    str(self.detailwidget.app_detail_widget.app_msg.app_id),
                    self.detailwidget.app_detail_widget.app_msg.pkgname)

        self.title_bar.login_btn.setLongTextToElideFormat(self.title_bar.login_btn.user_name_label,
                                                          Globals.LOGIN_ACCOUNT)
        # self.title_bar.login_btn.user_name_label.setText(Globals.LOGIN_ACCOUNT)
        user_profile = UBUNTUKYLIN_DATA_PATH + "user_profile/" + Globals.USER_PROFILE
        pix2 = QPixmap(user_profile)
        fitpixmap_userIcon = pix2.scaled(20, 20, Qt.IgnoreAspectRatio, Qt.SmoothTransformation)
        fitpixmap_userIcon = self.PixmapToRound(fitpixmap_userIcon, 20,
                                                20)
        self.title_bar.login_btn.head_portrait_label.setPixmap(fitpixmap_userIcon)
        # self.title_bar.login_btn.head_portrait_label.setStyleSheet("QLabel{border:0px;border-radius: 10px;border-image: url('"+user_profile+"') 0 0 0 0 stretch strectch;background-color:transparent;}")
        self.title_bar.login_btn.user_name_label.adjustSize()

        self.title_bar.login_btn.setLongTextToElideFormat(self.title_bar.login_btn.user_name_label,
                                                          Globals.LOGIN_ACCOUNT)
        self.title_bar.login_btn.btn.resize(
            self.title_bar.login_btn.user_name_label.width() + 4 + self.title_bar.login_btn.head_portrait_label.width(),
            20)
        self.title_bar.login_btn.setLongTextToElideFormat(self.title_bar.login_btn.user_name_label,
                                                          Globals.LOGIN_ACCOUNT)
        if ("…" in self.title_bar.login_btn.user_name_label.text()):
            self.title_bar.login_btn.user_name_label.setToolTip(Globals.LOGIN_ACCOUNT)

        self.timer_hide_login.timeout.connect(self.operate)  # 计时结束调用operate()方法
        self.timer_hide_login.start(1500)
        # 设置计时间隔并启动

    # caoliang
    # 函数名:登录界面
    # Function: login page
    #
    def slot_ui_login(self, login_list):
        feture4 = pool.submit(self.at.appmgr.ui_login, (login_list))

    def slot_ui_login_get_image_code(self, code_dict):
        item = {"action": "get_image_code", "code_dict": code_dict}
        self.ask_thread.mutex.acquire()
        self.ask_thread.work_list.append(item)
        self.ask_thread.mutex.release()

    # 函数：得到token返回值
    # 参数：无
    # 返回：无
    def read_token_login(self):
        token_login_value = {}
        local_login_token = self.at.appmgr.get_local_token()
        if not local_login_token:
            Globals.RE_LOGIN = False
            pass
        else:
            # token_login_value["account"] = local_login_token
            # print("local_login_token", local_login_token)
            item = {"action": "read_token"}
            self.ask_thread.mutex.acquire()
            self.ask_thread.work_list.append(item)
            self.ask_thread.mutex.release()

    def slot_read_token_over(self, login_list):
        self.title_bar.login_btn.btn.setEnabled(True)
        # self.title_bar.login_btn.user_name_label.setText(_("login"))
        # self.title_bar.login_btn.user_name_label.adjustSize()
        self.title_bar.login_btn.setLongTextToElideFormat(self.title_bar.login_btn.user_name_label,
                                                          _("login"))
        self.title_bar.login_btn.btn.resize(
            self.title_bar.login_btn.user_name_label.width() + 4 + self.title_bar.login_btn.head_portrait_label.width(),
            20)

        if login_list == False:
            Globals.RE_LOGIN = False
            if (os.path.exists(os.path.join(CACHE_DIR, "kylinId/token"))):
                os.remove(os.path.join(CACHE_DIR, "kylinId/token"))
            return False
        else:
            if (login_list != {}):
                if login_list['code'] == 200:
                    Globals.RE_LOGIN = True
                    user_data = login_list['data']
                    Globals.LOGIN_ACCOUNT = user_data['name']
                    Globals.USER_PROFILE = user_data['avatar']
                    Globals.USER_ID = user_data["id"]
                    Globals.ACCOUNT = user_data["account"]

                    self.slot_ui_login_success()
                # self.at.appmgr.ui_login(res_list)
                else:
                    if (os.path.exists(os.path.join(CACHE_DIR, "kylinId/token"))):
                        os.remove(os.path.join(CACHE_DIR, "kylinId/token"))
                    Globals.RE_LOGIN = False
            else:
                if (os.path.exists(os.path.join(CACHE_DIR, "kylinId/token"))):
                    os.remove(os.path.join(CACHE_DIR, "kylinId/token"))
                Globals.RE_LOGIN = False
        login_list.clear()
        # print("local_login_token",local_login_token)
        # return local_login_token

    #
    # 函数：获取图片验证
    #
    def slot_get_image_over(self, res):
        # caodandan
        if res == False:
            self.loginwidget.ui.tips_user_password.setText(_("request server exception"))
            self.loginwidget.ui.tips_user_password.show()
            self.loginwidget.timer_set()
        else:
            if (res["code"] == 200 and res["data"] != None):
                self.loginwidget.account_login_values['imgId'] = res['data']['imgId']
                self.show_image.emit()
            else:
                self.loginwidget.next_to_phone_login()

    #
    # 函数：发送手机号，获取验证码
    #
    def slot_ui_login_phone_num(self, phone_num):
        # feture4 = pool.submit(self.at.appmgr.get_captcha, (phone_num))
        mcode_res = self.at.appmgr.get_captcha(phone_num)
        self.loginwidget.phone_code_res = mcode_res

    #
    # 函数：处理win替换中的更多按钮
    #
    def slot_click_win_more_btn(self):
        Globals.NOWPAGE = PageStates.WINPAGE
        self.title_bar.search_line.clear()
        self.detailwidget.text_prompt_label.hide()
        self.detailwidget.no_found_tip.hide()
        self.detailwidget.no_app_tip_label.hide()
        self.detailwidget.show_category_widget()
        self.detailwidget.category_widget.scrollArea.verticalScrollBar().setValue(0)
        # self.detailwidget.category_widget.win_btn.show_which_btn()
        # self.detailwidget.category_widget.win_btn.clicked_this_btn.emit(self.detailwidget.category_widget.win_btn.text())
        pass

    #
    # 函数：处理搜索
    #
    def slot_textChanged(self, t="", ADVANCED_SEARCH=True):
        Globals.ADVANCED_SEARCH = ADVANCED_SEARCH
        if (hasattr(self, "search_tip_widget")):
            self.search_tip_widget.hide()

        self.detailwidget.scrollArea2.verticalScrollBar().setValue(0)
        text = self.title_bar.search_line.text()
        if (text != ""):
            self.title_bar.search_line_back.hide()
            ask_item = {"action":"send_search_keyword","keyword":self.title_bar.search_line.text()}
            self.ask_thread.mutex.acquire()
            self.ask_thread.work_list.append(ask_item)
            self.ask_thread.mutex.release()
            item = self.get_search_app_list()
            Globals.NOWPAGE = PageStates.SEARCHPAGE
            self.detailwidget.new_arrivals_widget.ad_move_left_time.stop()
            page = {"pagetype": PageStates.SEARCHPAGE, "search_text": self.title_bar.search_line.text(),
                    "type": Globals.ADVANCED_SEARCH,"search_nowpage":Globals.SEARCH_NOWPAGE}
            if (len(Globals.RETURN_PATH_LIST) >= 10):
                Globals.RETURN_PATH_LIST.pop(0)
            if (len(Globals.RETURN_PATH_LIST) >= 1 and Globals.RETURN_PATH_LIST[len(Globals.RETURN_PATH_LIST) - 1] != page):
                Globals.RETURN_PATH_LIST.append(page)
            self.detailwidget.no_app_tip_label.hide()
            self.detailwidget.text_prompt_label.hide()
            self.detailwidget.no_found_tip.hide()
            self.detailwidget.search_title_label.hide()
            self.detailwidget.search_widget_line.hide()
            self.detailwidget.search_app_widget.hide()
            self.detailwidget.all_search_tip_label.hide()
            if (hasattr(self.detailwidget, "app_detail_widget")):
                self.detailwidget.app_detail_widget.hide()
            if (hasattr(self.detailwidget, "category_widget")):
                self.detailwidget.category_widget.hide()
            if (hasattr(self.detailwidget, "kydroid_widget")):
                self.detailwidget.kydroid_widget.hide()
            if (hasattr(self.detailwidget, "drive_widget")):
                self.detailwidget.drive_widget.hide()
            self.detailwidget.mywidget.hide()
            self.detailwidget.loadinglabel.show()
            self.detailwidget.show_homepage_widget2.show()
            self.detailwidget.scrollArea2.show()
            self.detailwidget.search_app_widget.hide()
            self.title_bar.return_btn.setEnabled(False)
            self.title_bar.home_page_btn.setEnabled(False)
            self.title_bar.kydroid_btn.setEnabled(False)
            self.title_bar.category_btn.setEnabled(False)
            self.title_bar.my_btn.setEnabled(False)
            self.title_bar.drive_btn.setEnabled(False)
            # if(self.detailwidget.scrollArea.isHidden() == False):
            #     self.detailwidget.show_homepage_widget.hide()
            #     self.detailwidget.scrollArea.hide()
            self.at.mutex.acquire()
            self.at.work_list.append(item)
            self.at.mutex.release()
            if (hasattr(self, "search_tip_widget")):
                self.search_tip_widget.hide()
        else:
            pass
        time.sleep(0.1)

    #
    #函数：获取各个页面对应的list
    #
    def get_search_app_list(self,is_tip = False):
        item = {}
        if(is_tip):
            item["action"] = "search_app_tips"
        else:
            item["action"] = "search_app"
        if(Globals.SEARCH_NOWPAGE == PageStates.HOMEPAGE):
            applist = []
            applist.extend(Globals.GET_ALL_APP_LIST)
            applist.extend(Globals.GET_ALL_DRIVE_APP_LIST)
            item["search_app_list"] = applist
            item["is_remove"] = False
            item["need_all_search"] = True
            item["search_apk_list"] = None
        elif (Globals.SEARCH_NOWPAGE == PageStates.CAREGORYPAGE):
            btn_name = self.detailwidget.category_widget.type_name_label.text()
            # if(Globals.SORTING_METHOD == 0):
            #     item["search_app_list"] = Globals.ALL_CATEGORY_APP_LIST[btn_name]
            # else:
            #     item["search_app_list"] = Globals.ALL_CATEGORY_APP_LIST_BY_RATING[btn_name]
            # applist = []
            # applist.extend(Globals.GET_ALL_APP_LIST)
            item["search_app_list"] = None
            item["is_remove"] = False
            item["need_all_search"] = True
            item["search_apk_list"] = []
        elif(Globals.SEARCH_NOWPAGE == PageStates.MYPAGE):
            item["search_app_list"] = Globals.SEARCH_APP_LIST["MY"]
            if(self.detailwidget.mywidget.type_name_label.text() == _("Uninstall")):
                item["is_remove"] = True
            else:
                item["is_remove"] = False
            item["need_all_search"] = False
            item["search_apk_list"] = Globals.SEARCH_APP_LIST["MY"]
        elif(Globals.SEARCH_NOWPAGE == PageStates.HISTORY):
            item["search_app_list"] = Globals.SEARCH_APP_LIST["HISTORY"]
            item["is_remove"] = False
            item["need_all_search"] = False
            item["search_apk_list"] = Globals.SEARCH_APP_LIST["HISTORY"]
        elif(Globals.SEARCH_NOWPAGE == PageStates.SEARCHPAGE):
            item["search_app_list"] = None
            item["is_remove"] = self.detailwidget.search_is_remove
            item["search_apk_list"] = []
        elif(Globals.SEARCH_NOWPAGE == PageStates.KYDROID):
            item["search_app_list"] = []
            item["is_remove"] = False
            item["need_all_search"] = False
            item["search_apk_list"] = self.at.appmgr.apk_list
        elif(Globals.SEARCH_NOWPAGE == PageStates.DOWNLOADINGPAGE):
            item["search_app_list"] = Globals.SEARCH_APP_LIST["DOWNLOADING"]
            item["is_remove"] = False
            item["need_all_search"] = False
            item["search_apk_list"] = Globals.SEARCH_APP_LIST["DOWNLOADING"]
        elif(Globals.SEARCH_NOWPAGE == PageStates.DRIVE):
            item["search_app_list"] = Globals.GET_ALL_DRIVE_APP_LIST
            item["is_remove"] = False
            item["need_all_search"] = False
            item["search_apk_list"] = []
        return item

    #
    # 函数：获取搜索提示
    #
    def slot_get_search_tips(self, te):
        if (te != "" and self.title_bar.click_item == False):
            item = self.get_search_app_list(True)
            self.at.mutex.acquire()
            self.at.work_list.append(item)
            self.at.mutex.release()
            self.title_bar.search_cancel_btn.show()
        else:
            self.title_bar.search_cancel_btn.hide()
            self.title_bar.search_line_back.show()
            self.title_bar.click_item = False
            if (hasattr(self, "search_tip_widget")):
                self.search_tip_widget.hide()

    #
    # 函数：处理搜索结果部分提示
    #
    def slot_get_search_applist_over(self, applist):
        if(self.title_bar.search_line.text() == ""):
            if (hasattr(self, "search_tip_widget")):
                self.search_tip_widget.hide()
            return
        show_list = []
        for i in applist:
            # app = self.get_appmsg(i)
            if (i not in show_list):
                show_list.append(i)
        if (hasattr(self, "search_tip_widget") == False):
            self.search_tip_widget = Search_records(self.title_bar.search_label.width(), self)
            self.search_tip_widget.move(self.title_bar.search_label.x()-self.search_tip_widget.space,
                                        self.title_bar.search_label.height() + self.title_bar.search_label.y())
            self.search_tip_widget.item_list_widget.itemClicked.connect(self.slot_click_search_item)
        else:
            while (self.search_tip_widget.item_list_widget.count() != 0):
                item = self.search_tip_widget.item_list_widget.takeItem(0)
        self.search_tip_widget.enter_item = None
        for pkgname in show_list:
            item = QListWidgetItem()
            list_item = line_widget(227, self.search_tip_widget)
            list_item.enter_widget.connect(self.search_tip_widget.slot_change_list_style)
            list_item.appname_label.hide()
            # list_item.appname_label.setText(pkgname)
            list_item.setLongTextToElideFormat(list_item.appname_label,pkgname)
            # list_item.appname_label.adjustSize()
            list_item.appname_label.move(list_item.appname_label.x(),
                                         int((list_item.height() - list_item.appname_label.height()) / 2))
            list_item.appname_label.show()
            self.search_tip_widget.item_list_widget.addItem(item)
            self.search_tip_widget.item_list_widget.setItemWidget(item, list_item)
        if (len(show_list) != 0):
            self.search_tip_widget.item_list_widget.setFixedHeight(len(show_list) * list_item.height())
            self.search_tip_widget.widget.setFixedHeight(self.search_tip_widget.item_list_widget.height() + 4)
            self.search_tip_widget.setFixedHeight(self.search_tip_widget.widget.height() + 20)
            self.search_tip_widget.show()
            self.title_bar.senior_search.hide()
            # self.detailwidget.search_app_widget.show()
            self.detailwidget.scrollArea2.show()
        else:
            self.search_tip_widget.hide()
        # for i in applist:

    #
    # 函数：处理点击搜索的item
    #
    def slot_click_search_item(self, item):
        self.detailwidget.new_arrivals_widget.ad_move_left_time.stop()
        self.title_bar.return_btn.setEnabled(True)
        self.title_bar.home_page_btn.setEnabled(True)
        self.title_bar.kydroid_btn.setEnabled(True)
        self.title_bar.drive_btn.setEnabled(True)
        self.title_bar.category_btn.setEnabled(True)
        self.title_bar.my_btn.setEnabled(True)
        self.title_bar.drive_btn.setEnabled(True)
        self.search_tip_widget.hide()
        get_item_widget = self.search_tip_widget.item_list_widget.itemWidget(item)
        self.title_bar.search_line.hide()
        self.title_bar.click_item = True
        self.title_bar.search_line.setText(get_item_widget.appname)
        self.title_bar.search_cancel_btn.show()
        self.title_bar.search_line.setFocus(True)
        self.title_bar.search_line.end(True)
        self.title_bar.search_line.setFocus(False)
        self.title_bar.search_line.show()
        self.slot_textChanged("", Globals.ADVANCED_SEARCH)

    #
    # 函数：显示搜索结果
    #
    def show_search_app(self, applist,is_remove = False,need_all_search = True):
        self.title_bar.return_btn.setEnabled(True)
        self.title_bar.home_page_btn.setEnabled(True)
        self.title_bar.kydroid_btn.setEnabled(True)
        self.title_bar.category_btn.setEnabled(True)
        self.title_bar.drive_btn.setEnabled(True)
        self.title_bar.my_btn.setEnabled(True)
        self.title_bar.drive_btn.setEnabled(True)
        # for i in self.path_of_particle:
        #     i.hide()
        self.detailwidget.search_app_widget.hide()
        self.detailwidget.show_homepage_widget3.hide()
        self.detailwidget.scrollArea3.hide()
        self.detailwidget.new_arrivals_widget.ad_move_left_time.stop()
        self.detailwidget.scrollArea2.show()
        self.detailwidget.loadinglabel.hide()
        self.clear_all_card(self.detailwidget.search_app_widget)
        self.app_list_msg.clear()
        self.app_list_msg.extend(applist)
        if (len(applist) == 0):
            if (Globals.THEME_COLOR == 0):
                self.detailwidget.no_app_tip_label.setStyleSheet(
                    "QLabel{border-radius: 12px;background-image:url(./res/no_search.png);}")
            else:
                self.detailwidget.no_app_tip_label.setStyleSheet(
                    "QLabel{border-radius: 12px;background-image:url(./res/no_search_dark.png);}")
            self.detailwidget.text_prompt_label.setText(_("Sorry, we didn't find the app you want!"))
            self.detailwidget.text_prompt_label.adjustSize()
            self.detailwidget.no_app_tip_label.move(
                int((self.detailwidget.width() - self.detailwidget.no_app_tip_label.width()) / 2), int((self.detailwidget.height() - self.detailwidget.no_app_tip_label.height() - self.detailwidget.text_prompt_label.height() + 10) / 2))
            self.detailwidget.text_prompt_label.move(
                int((self.detailwidget.width() - self.detailwidget.text_prompt_label.width()) / 2),
                self.detailwidget.no_app_tip_label.y() + self.detailwidget.no_app_tip_label.height() + 10)
            self.detailwidget.no_found_tip.move(
                int((self.detailwidget.width() - self.detailwidget.no_found_tip.width()) / 2),
                self.detailwidget.no_app_tip_label.y() + self.detailwidget.no_app_tip_label.height() + 10)
            if (Globals.ADVANCED_SEARCH == False):
                self.detailwidget.text_prompt_label.show()
                self.detailwidget.no_found_tip.hide()
            else:
                if(need_all_search):
                    self.detailwidget.no_found_tip.show()
                    self.detailwidget.text_prompt_label.hide()
                else:
                    self.detailwidget.text_prompt_label.show()
                    self.detailwidget.no_found_tip.hide()
            self.detailwidget.no_app_tip_label.show()
            self.detailwidget.search_title_label.hide()
            self.detailwidget.search_widget_line.hide()
            self.detailwidget.search_app_widget.hide()
            self.detailwidget.show_homepage_widget2.show()
            self.detailwidget.scrollArea2.hide()
            Globals.ADVANCED_SEARCH = True
        else:
            self.detailwidget.search_is_remove = is_remove
            self.detailwidget.need_search_full_library = need_all_search
            self.slot_card_list(self.title_bar.search_line.text(),applist, self.detailwidget.show_homepage_widget2,
                                          self.detailwidget.search_app_widget, is_remove, True)
            self.detailwidget.show_homepage_widget2.show()
            self.detailwidget.scrollArea2.show()
            self.detailwidget.search_widget_line.show()
            self.detailwidget.search_title_label.show()
            self.detailwidget.text_prompt_label.hide()
            self.detailwidget.no_found_tip.hide()
            self.detailwidget.no_app_tip_label.hide()
        if (Globals.ADVANCED_SEARCH == True):
            self.detailwidget.search_title_label.setText(_("search result"))
            self.detailwidget.search_title_label.adjustSize()
            self.detailwidget.all_search_tip_label.hide()
        else:
            self.detailwidget.search_title_label.setText(_("\"all\" result"))
            self.detailwidget.search_title_label.adjustSize()
            self.detailwidget.all_search_tip_label.move(
                self.detailwidget.search_title_label.x() + self.detailwidget.search_title_label.width() + 10,
                self.detailwidget.search_title_label.y() + self.detailwidget.search_title_label.height() - self.detailwidget.all_search_tip_label.height() - 5)
            self.detailwidget.all_search_tip_label.show()
            self.detailwidget.scrollArea2.resize(self.detailwidget.scrollArea2.width(),
                                                 self.detailwidget.height() - self.detailwidget.search_widget_line.y() - self.detailwidget.search_widget_line.height() - 30)
        # if(self.detailwidget.show_homepage_widget2 in self.path_of_particle):
        #     self.path_of_particle.remove(self.detailwidget.show_homepage_widget2)
        # self.path_of_particle.append(self.detailwidget.show_homepage_widget2)

    #
    # 函数：显示在开始菜单点击搜索结果的详情界面
    #
    def click_search_app(self, appname):
        search_list = []
        num = 0
        in_which_list = 0 #0表示Globals.GET_ALL_APP_LIST，1表示Globals.KYDROID_APP_LIST，2表示Globals.GET_ALL_DRIVE_APP_LIST
        for i in Globals.GET_ALL_APP_LIST:
            if (i["name"] == appname or i["displayname_cn"] == appname):
                in_which_list = 0
                break
            num = num + 1
        if(num > len(Globals.GET_ALL_APP_LIST)-1):
            num = 0
            for i in Globals.KYDROID_APP_LIST:
                if (i["name"] == appname or i["displayname_cn"] == appname):
                    in_which_list = 1
                    break
                num = num + 1
            if(num > len(Globals.KYDROID_APP_LIST)-1):
                num = 0
                for i in Globals.GET_ALL_DRIVE_APP_LIST:
                    if (i["name"] == appname or i["displayname_cn"] == appname):
                        in_which_list = 2
                        break
                    num = num + 1
        if(in_which_list == 0):
            if (num <= len(Globals.GET_ALL_APP_LIST) - 1):
                app_msg = self.get_appmsg(Globals.GET_ALL_APP_LIST[num])
                if (type(app_msg) == Application):
                    self.set_title_btn_stylesheet(PageStates.CAREGORYPAGE)
                try:
                    for i in os.popen("wmctrl -l |grep " + _("Software Store")):
                        wm = i.split()
                        if (_("Software Store") in wm):
                            os.system("wmctrl -i -R " + wm[0])
                except:
                    pass
                self.detailwidget.slot_clicked_card(app_msg)
        elif(in_which_list == 1):
            app = Globals.KYDROID_APP_LIST[num]
            for i in Globals.KYDROID_SOURCE_LIST:
                if (str(Globals.KYDROID_SOURCE_LIST[i]["packageName"]) == str(app["name"])):
                    apk = ApkInfo(app["name"], app["displayname_cn"], Globals.KYDROID_SOURCE_LIST[i]["version"],
                                  Globals.KYDROID_SOURCE_LIST[i]["size"], Globals.KYDROID_SOURCE_LIST[i]["file"],
                                  app["description"])
                    self.at.appmgr.set_apk_information(app, apk)
                    if (app["initialsName"] != None):
                        apk.initialsName = app["initialsName"]
                    if (app["pinyinName"] != None):
                        apk.pinyinName = app["pinyinName"]
                    if (app["anotherName"] != None):
                        apk.anotherName = app["anotherName"]
                    self.at.appmgr.apk_list.append(apk)
                    try:
                        for i in os.popen("wmctrl -l |grep " + _("Software Store")):
                            wm = i.split()
                            if (_("Software Store") in wm):
                                os.system("wmctrl -i -R " + wm[0])
                    except:
                        pass
                    self.set_title_btn_stylesheet(PageStates.KYDROID)
                    self.detailwidget.slot_clicked_card(apk)
        else:
            if (num <= len(Globals.GET_ALL_DRIVE_APP_LIST) - 1):
                app_msg = self.get_appmsg(Globals.GET_ALL_DRIVE_APP_LIST[num])
                if (type(app_msg) == Application):
                    self.set_title_btn_stylesheet(PageStates.DRIVE)
                try:
                    for i in os.popen("wmctrl -l |grep " + _("Software Store")):
                        wm = i.split()
                        if (_("Software Store") in wm):
                            os.system("wmctrl -i -R " + wm[0])
                except:
                    pass
                self.detailwidget.slot_clicked_card(app_msg)
        return True

    #
    # 函数：处理滚动条到底部
    #
    def slot_scrollbar_valuechange(self, n):
        if (hasattr(self, "search_tip_widget")):
            self.search_tip_widget.hide()
        list_widget, scrollarea = self.get_current_listWidget()
        if(int(scrollarea.verticalScrollBar().maximum() <= 0)):
            return
        if (n in range(int(scrollarea.verticalScrollBar().maximum() / 4 * 3) - 100,int(scrollarea.verticalScrollBar().maximum())) or n == scrollarea.verticalScrollBar().maximum()):
            if (Globals.NOWPAGE == PageStates.HISTORY):
                # self.check_card_num(list_widget)
                self.slot_card_list(self.detailwidget.mywidget.type_name_label.text(),self.app_list_msg, self.detailwidget.mywidget.history_widget, list_widget,
                                              False, True)
            elif (Globals.NOWPAGE == PageStates.SEARCHPAGE):
                self.slot_card_list(self.title_bar.search_line.text(),[], self.detailwidget.show_homepage_widget2,
                                              self.detailwidget.search_app_widget, self.detailwidget.search_is_remove, True)
            elif(Globals.NOWPAGE == PageStates.MOREPAGE):
                self.slot_card_list("",[], self.detailwidget.show_homepage_widget3,
                                              self.detailwidget.show_more_app_widget, False,
                                              True)
            else:
                if (self.is_remove == False):
                    if (hasattr(self.detailwidget, "category_widget")):
                        if (list_widget == self.detailwidget.category_widget.type_widget):
                            # self.check_card_num(self.detailwidget.category_widget.type_show_card_widget,self.app_list_msg)
                            self.slot_card_list(self.detailwidget.category_widget.type_name_label.text(),self.app_list_msg, list_widget,
                                                          self.detailwidget.category_widget.type_show_card_widget,
                                                          False, True)
                        else:
                            if (Globals.NOWPAGE == PageStates.KYDROID):
                                self.slot_card_list(self.detailwidget.kydroid_widget.type_name_label.text(),self.pkg_list_msg, list_widget, list_widget, False, True)
                            elif(Globals.NOWPAGE == PageStates.DRIVE):
                                self.slot_card_list(self.detailwidget.drive_widget.type_name_label.text(),
                                                    self.app_list_msg, list_widget, list_widget, False, True)
                            else:
                                self.slot_card_list(self.detailwidget.mywidget.type_name_label.text(),self.app_list_msg, list_widget, list_widget, False, True)
                    else:
                        if (Globals.NOWPAGE == PageStates.KYDROID):
                            self.slot_card_list(self.detailwidget.kydroid_widget.type_name_label.text(),self.pkg_list_msg, list_widget, list_widget, False, True)
                        elif (Globals.NOWPAGE == PageStates.DRIVE):
                            self.slot_card_list(self.detailwidget.drive_widget.type_name_label.text(),self.app_list_msg, list_widget, list_widget, False, True)
                        else:
                            # self.check_card_num(list_widget)
                            self.slot_card_list(self.detailwidget.mywidget.type_name_label.text(),self.app_list_msg, list_widget, list_widget, False, True)
                        # self.show_inside_applist.emit(self.app_list_msg, list_widget, list_widget, False)
                else:
                    if (hasattr(self.detailwidget, "category_widget")):
                        if (list_widget == self.detailwidget.category_widget.type_widget):
                            self.check_card_num(self.detailwidget.category_widget.type_show_card_widget)
                            self.slot_card_list(self.detailwidget.category_widget.type_name_label.text(),self.app_list_msg, list_widget,self.detailwidget.category_widget.type_show_card_widget, True,
                                                          True)
                        else:
                            if (Globals.NOWPAGE == PageStates.KYDROID):
                                self.slot_card_list(self.detailwidget.kydroid_widget.type_name_label.text(),self.pkg_list_msg, list_widget, list_widget, True, True)
                            elif(Globals.NOWPAGE == PageStates.DRIVE):
                                self.slot_card_list(self.detailwidget.drive_widget.type_name_label.text(),self.app_list_msg, list_widget, list_widget, True, True)
                            else:
                                self.slot_card_list(self.detailwidget.mywidget.type_name_label.text(),self.app_list_msg, list_widget, list_widget, True, True)
                            # self.check_card_num(list_widget)
                            # self.slot_card_list(self.app_list_msg, list_widget, list_widget, True, True)
                    else:
                        # self.check_card_num(list_widget)
                        if (Globals.NOWPAGE == PageStates.KYDROID):
                            self.slot_card_list(self.detailwidget.kydroid_widget.type_name_label.text(),
                                                self.pkg_list_msg, list_widget, list_widget, True, True)
                        elif (Globals.NOWPAGE == PageStates.DRIVE):
                            self.slot_card_list(self.detailwidget.drive_widget.type_name_label.text(),
                                                self.app_list_msg, list_widget, list_widget, True, True)
                        else:
                            self.slot_card_list(self.detailwidget.mywidget.type_name_label.text(), self.app_list_msg,
                                                list_widget, list_widget, True, True)

    def resize_window_show(self):
        try:
            if (len(self.pkg_list_msg) != 0 and hasattr(self.detailwidget, "kydroid_widget")):
                self.slot_card_list(self.detailwidget.kydroid_widget.type_name_label.text(),self.pkg_list_msg, self.detailwidget.kydroid_widget.app_widget,
                                              self.detailwidget.kydroid_widget.app_widget, False, False)
            if (hasattr(self.detailwidget, "category_widget")):
                if (Globals.NOWPAGE == PageStates.CAREGORYPAGE):
                    self.slot_card_list(self.detailwidget.category_widget.type_name_label.text(),self.app_list_msg, self.detailwidget.category_widget.type_widget,
                                                  self.detailwidget.category_widget.type_show_card_widget,
                                                  False, True)
                    # self.show_inside_applist.emit(self.app_list_msg, self.detailwidget.category_widget.type_widget,
                    #                               self.detailwidget.category_widget.type_show_card_widget,
                    #                               False, True)
                else:
                    self.slot_card_list(self.detailwidget.category_widget.type_name_label.text(),Globals.FRIST_LEVEL_BTN_SCROLLAREA["CATEGORY"],
                                                  self.detailwidget.category_widget.type_widget,
                                                  self.detailwidget.category_widget.type_show_card_widget,
                                                  False, False)
            if (Globals.NOWPAGE == PageStates.MYPAGE):
                if (self.detailwidget.mywidget.type_name_label.text() == _("Uninstall")):
                    self.slot_card_list(self.detailwidget.mywidget.type_name_label.text(),Globals.FRIST_LEVEL_BTN_SCROLLAREA["MY"],
                                                  self.detailwidget.mywidget.app_widget,
                                                  self.detailwidget.mywidget.app_widget,
                                                  True, True)
                else:
                    self.slot_card_list(self.detailwidget.mywidget.type_name_label.text(),Globals.FRIST_LEVEL_BTN_SCROLLAREA["MY"],
                                                  self.detailwidget.mywidget.app_widget,
                                                  self.detailwidget.mywidget.app_widget,
                                                  False, True)
            elif (Globals.NOWPAGE == PageStates.HISTORY):
                self.slot_card_list(self.detailwidget.mywidget.type_name_label.text(),Globals.FRIST_LEVEL_BTN_SCROLLAREA["MY"],
                                              self.detailwidget.mywidget.history_widget,
                                              self.detailwidget.mywidget.history_widget.app_card_widget,
                                              False, True)
            elif (Globals.NOWPAGE == PageStates.SEARCHPAGE):
                self.slot_card_list(self.title_bar.search_line.text(),self.app_list_msg, self.detailwidget.show_homepage_widget2,
                                              self.detailwidget.search_app_widget, False, True)
            else:
                if (self.detailwidget.mywidget.type_name_label.text() == _("Installation")):
                    self.slot_card_list(self.detailwidget.mywidget.type_name_label.text(),Globals.FRIST_LEVEL_BTN_SCROLLAREA["MY"],
                                                  self.detailwidget.mywidget.history_widget,
                                                  self.detailwidget.mywidget.history_widget.app_card_widget,
                                                  False, False)
                elif (self.detailwidget.mywidget.type_name_label.text() == _("Uninstall")):
                    self.slot_card_list(self.detailwidget.mywidget.type_name_label.text(),Globals.FRIST_LEVEL_BTN_SCROLLAREA["MY"],
                                                  self.detailwidget.mywidget.app_widget,
                                                  self.detailwidget.mywidget.app_widget,
                                                  True, False)
                elif (self.detailwidget.mywidget.type_name_label.text() == _("Updates")):
                    self.slot_card_list(self.detailwidget.mywidget.type_name_label.text(),Globals.FRIST_LEVEL_BTN_SCROLLAREA["MY"],
                                                  self.detailwidget.mywidget.app_widget,
                                                  self.detailwidget.mywidget.app_widget,
                                                  False, False)

            if (Globals.NOWPAGE == PageStates.MYPAGE and Globals.NOWPAGE == PageStates.KYDROID or Globals.NOWPAGE == PageStates.CAREGORYPAGE):
                return
        except:
            pass

    #
    # 函数：判断还需要显示的card数量
    #
    def check_card_num(self, widget, list):
        if (len(widget.children()) < len(list)):
            for i in range(len(widget.children())):
                if (len(list) != 0):
                    list.pop(0)

    #
    # 函数：向界面中添加卡片
    #
    def add_card_to_widget(self, applist, widget, is_remove_app, is_kydroid=False):
        app_msg_list = []
        package_is_none = False
        width = widget.width()
        cards = widget.children()
        count = len(cards)
        h_num = int((widget.width() - 10) / 293)
        space = int(((widget.width() - 10) % 293) / (h_num - 1))
        v_num = int(count / h_num)
        num = count % h_num
        n = 0
        for item in applist:
            if (type(item) == ApkInfo):
                is_kydroid = True
            else:
                is_kydroid = False
            if (is_kydroid == False):
                app_msg = self.get_appmsg(item)
            else:
                app_msg = self.get_apkmsg(item)
            if (app_msg != None):
                app_msg_list.append(app_msg)
        if (len(app_msg_list) - count >= h_num * 5):
            sub_n = h_num * 5
        else:
            sub_n = len(app_msg_list) - count
        for i in range(sub_n):
            app_msg = app_msg_list[count + i]
            try:
                if (app_msg.package == None and type(app_msg) != ApkInfo):
                    package_is_none = True
                if (package_is_none == False):
                    card_widget = Card_Widget(293, 158, 32, app_msg, self, self.detailwidget, self.detailwidget.ca,
                                              widget)
                    self.send_download_progress.connect(card_widget.slot_get_download_progress)
                    self.signal_reload_btn.connect(card_widget.reload_btn)
                    self.signal_uninstall.connect(card_widget.slot_uninstall)
                    self.change_card_state.connect(card_widget.change_btn_state)
                    card_widget.puase_download.connect(self.slot_puase_download_pkg)
                    card_widget.check_all_check_statue.connect(self.slot_all_puase_btn_state)
                    card_widget.puase_apk_download.connect(self.slot_puase_download_pkg)
                    for i in self.down_app_msg_dict:
                        if (i["name"] == app_msg.pkgname):
                            if (i["is_downloading"] == True):
                                card_widget.btn.hide()
                                card_widget.cancel_btn.show()
                                card_widget.down_progress.show()
                            elif (i["is_installing"] == True):
                                card_widget.cancel_btn.hide()
                                card_widget.btn.setText(_("installing"))
                                card_widget.btn.show()
                                card_widget.down_progress.hide()
                            elif (i["is_puase"] == True):
                                pass
                            break
                    # card_widget.clicked_card.connect(lambda :self.detailwidget.slot_clicked_card(card_widget.app))
                    if (num < h_num):
                        if (num == 0):
                            card_widget.move(5, 20 + (20 + card_widget.height()) * v_num)
                            widget.resize(width, 20 + (20 + card_widget.height()) * (v_num + 1))
                        else:
                            card_widget.move(num * (card_widget.width() + space),
                                             20 + (20 + card_widget.height()) * v_num)
                        num = num + 1
                    else:
                        num = 0
                        v_num = v_num + 1
                        if (num == 0):
                            card_widget.move(5, 20 + (20 + card_widget.height()) * v_num)
                            widget.resize(width, 20 + (20 + card_widget.height()) * (v_num + 1))
                        else:
                            card_widget.move(num * (card_widget.width() + space),
                                             20 + (20 + card_widget.height()) * v_num)
                        num = num + 1
                    # card_widget.btn.clicked.connect(lambda :self.slot_click_action_btn(card_widget.btn.text(),card_widget.app.APPNAME))
                    if (is_remove_app == True):
                        card_widget.btn.setText(_("uninstall"))
                        card_widget.set_btn_stylesheet(True)
            except:
                pass
        widget.show()

    #
    # 函数：清除所有卡片
    #
    def clear_all_card(self, widget):
        if(widget == self.detailwidget.search_app_widget):
            self.init_all_search = True
        widget.children_count = 0
        cards = widget.children()
        for card in cards:
            sip.delete(card)

    #
    # 函数名:重新设置卡片的位置
    # Function:reload cards
    #
    def reload_cards(self, widget):
        cards = widget.children()
        # self.clear_all_card(widget)
        self.add_card(cards, widget)
        # self.detailwidget.mywidget.reload_card(self.detailwidget.mywidget.downloading_widget,self.detailwidget.mywidget.scrollArea1)

    #
    # 函数：添加已有的card
    #
    def add_card(self, c, widget):
        cards = widget.children()
        if (len(cards) != 0):
            h_num = int(widget.width() / (self.detailwidget.mywidget.card_width + self.detailwidget.card_space))
            get_card_width = int((widget.width() - (self.detailwidget.card_space * (h_num - 1)) - 20) / h_num)
            proportion = get_card_width / widget.width()
            get_card_height = self.detailwidget.mywidget.card_height
            v_num = 0
            num = 0
            for i in range(len(cards)):
                card = cards[i]
                card.resize(get_card_width, get_card_height)
                card.re_move()
                if (num < h_num):
                    if (num == 0):
                        card.move(0, 5 + (20 + card.height()) * v_num)
                        widget.resize(widget.width(), 5 + (20 + card.height()) * (v_num + 1))
                    else:
                        card.move(num * (card.width() + self.detailwidget.card_space),
                                  5 + (20 + card.height()) * v_num)
                    num = num + 1
                else:
                    num = 0
                    v_num = v_num + 1
                    if (num == 0):
                        card.move(0, 5 + (20 + card.height()) * v_num)
                        widget.resize(widget.width(), (20 + card.height()) * (v_num + 1))
                    else:
                        card.move(num * (card.width() + self.detailwidget.card_space),
                                  5 + (20 + card.height()) * v_num)
                    num = num + 1
        if (widget == self.detailwidget.mywidget.downloading_widget):
            card_num = len(self.detailwidget.mywidget.downloading_widget.children())
            if (card_num == 0):
                self.detailwidget.mywidget.download_all_check.setEnabled(False)
                self.detailwidget.mywidget.download_all_check.setStyleSheet(
                    "QPushButton{font-size:14px;font-weight: 500;color: #8f9399;line-height: 20px;border:0px solid #ff0000;}")
                if (Globals.THEME_COLOR == 0):
                    self.detailwidget.mywidget.no_app_tip_label.setStyleSheet(
                        "QLabel{border-radius: 12px;background-image:url(./res/no_download.png);}")
                else:
                    self.detailwidget.mywidget.no_app_tip_label.setStyleSheet(
                        "QLabel{border-radius: 12px;background-image:url(./res/no_download_dark.png);}")
                self.detailwidget.mywidget.text_prompt_label.setText(_("No software downloading!"))
                self.detailwidget.mywidget.text_prompt_label.adjustSize()
                if (int(self.detailwidget.mywidget.update_num_label.text()) > 0):
                    self.title_bar.my_btn_tip.show()
                else:
                    self.title_bar.my_btn_tip.hide()
                if (Globals.NOWPAGE == PageStates.DOWNLOADINGPAGE):
                    self.detailwidget.mywidget.text_prompt_label.show()
                    self.detailwidget.mywidget.no_app_tip_label.show()
                self.detailwidget.mywidget.scrollArea1.hide()
                self.clear_all_card(self.detailwidget.mywidget.downloading_widget)
                self.detailwidget.mywidget.downloading_num_label.hide()
                self.detailwidget.mywidget.no_app_tip_label.move(
                    int((self.detailwidget.width() - self.detailwidget.no_app_tip_label.width()) / 2), int((
                                                                                                                       self.detailwidget.height() - self.detailwidget.no_app_tip_label.height() - self.detailwidget.text_prompt_label.height() + 10) / 2))
                self.detailwidget.mywidget.text_prompt_label.move(
                    int((self.detailwidget.width() - self.detailwidget.text_prompt_label.width()) / 2),
                    self.detailwidget.no_app_tip_label.y() + self.detailwidget.no_app_tip_label.height() + 10)
                self.detailwidget.mywidget.downloading_num_label.hide()
            else:
                self.detailwidget.mywidget.border.show()
                self.detailwidget.mywidget.type_name_label.show()
                if (card_num > 99):
                    self.detailwidget.mywidget.download_all_check.setEnabled(True)
                    self.detailwidget.mywidget.download_all_check.setStyleSheet(
                        "QPushButton{border:0px;font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;border:0px solid #ff0000;}QPushButton:hover{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}QPushButton:pressed{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}")
                    self.detailwidget.mywidget.downloading_num_label.setText("99+")
                    self.detailwidget.mywidget.downloading_num_label.adjustSize()
                    self.detailwidget.mywidget.downloading_num_label.resize(32, 18)
                    self.detailwidget.mywidget.downloading_num_label.show()
                elif (card_num > 0 and card_num <= 99):
                    self.detailwidget.mywidget.download_all_check.setEnabled(True)
                    self.detailwidget.mywidget.download_all_check.setStyleSheet(
                        "QPushButton{border:0px;font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;border:0px solid #ff0000;}QPushButton:hover{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}QPushButton:pressed{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}")
                    self.detailwidget.mywidget.downloading_num_label.setText(str(card_num))
                    self.detailwidget.mywidget.downloading_num_label.adjustSize()
                    self.detailwidget.mywidget.downloading_num_label.show()
                else:
                    if (int(self.detailwidget.mywidget.update_num_label.text()) > 0):
                        self.title_bar.my_btn_tip.show()
                    else:
                        self.title_bar.my_btn_tip.hide()
            # widget.hide()
            # widget.show()

    #
    # 函数名: 删除卡片
    # Function:remove card
    #
    def remove_card(self, widget, appname):
        cards = widget.children()
        count = len(cards)
        for i in range(count):
            if cards[i].app.pkgname == appname:
                app_msg = cards[i].app
                cards[i].close()
                sip.delete(cards[i])
                del cards[i]
                num = 0
                app_dict = None
                if(type(app_msg) == ApkInfo):
                    if (app_msg in Globals.SEARCH_APP_LIST["DOWNLOADING"]):
                        Globals.SEARCH_APP_LIST["DOWNLOADING"].remove(app_msg)
                else:
                    for i in Globals.GET_ALL_APP_LIST:
                        if (i["name"] == app_msg.pkgname):
                            break
                        num = num + 1
                    if (num > len(Globals.GET_ALL_APP_LIST)-1):
                        pass
                    else:
                        app_dict = Globals.GET_ALL_APP_LIST[num]
                        if(app_dict != None and app_dict in Globals.SEARCH_APP_LIST["DOWNLOADING"]):
                            Globals.SEARCH_APP_LIST["DOWNLOADING"].remove(app_dict)
                break
        self.reload_cards(widget)

    #
    # 函数：点击分类中win替换的按钮
    #
    def add_win_replace_card(self, win_list, app_list, widget):
        Globals.NOWPAGE = PageStates.WINPAGE
        self.detailwidget.category_widget.scrollArea.verticalScrollBar().setValue(0)
        cards = widget.children()
        widget.resize(self.detailwidget.category_widget.type_show_card_widget.width(),
                      self.detailwidget.category_widget.type_widget.height() - self.detailwidget.category_widget.type_name_label.height())
        width = widget.width()
        h_num = int((widget.width() - 20) / 413)
        space = int(((widget.width() - 20) % 413) / (h_num - 1))
        num = 0
        subscript = 0
        v_num = 0
        for item in app_list:
            if (win_list[subscript][1] == item["name"]):
                if (num < h_num):
                    app_msg = self.get_appmsg(item)
                    if (app_msg != None):
                        card_widget = Win_Replace_Card(self.detailwidget, win_list[subscript], app_msg, widget)
                        if (num == 0):
                            card_widget.move(10, 20 + (20 + card_widget.height()) * v_num)
                            widget.resize(width, 20 + (20 + card_widget.height()) * (v_num + 1))
                        else:
                            card_widget.move(num * (card_widget.width() + space) + 10,
                                             20 + (20 + card_widget.height()) * v_num)
                        num = num + 1
                else:
                    num = 0
                    v_num = v_num + 1
                    app_msg = self.get_appmsg(item)
                    if (app_msg != None):
                        card_widget = Win_Replace_Card(self.detailwidget, win_list[subscript], app_msg, widget)
                        if (num == 0):
                            card_widget.move(10, 20 + (20 + card_widget.height()) * v_num)
                            widget.resize(width, 20 + (20 + card_widget.height()) * (v_num + 1))
                        else:
                            card_widget.move(num * (card_widget.width() + space) + 10,
                                             20 + (20 + card_widget.height()) * v_num)
                        num = num + 1
            subscript = subscript + 1


    #
    # 函数：显示提示界面
    #
    def show_tip_widget(self, text, is_remove=False, need_cancel_btn=False,has_error_num = ""):
        if (not hasattr(self, "tip_widget")):
            self.tip_widget = Tip_Widget(text, self.frame_widget)
            self.tip_widget.slot_show_dark_mode()
            self.tip_widget.setWindowModality(Qt.ApplicationModal)
            self.set_dark_mode.connect(self.tip_widget.slot_show_dark_mode)
            self.tip_widget.close_btn.clicked.connect(self.slot_close_tipwidget)
            self.tip_widget.cancel_btn.clicked.connect(self.slot_close_tipwidget)
            if (is_remove == True):
                self.tip_widget.btn.move(344, 110)
                self.tip_widget.tips_label.move(44, 55)
                self.tip_widget.cancel_btn.move(self.tip_widget.btn.x() - self.tip_widget.cancel_btn.width() - 16,
                                                self.tip_widget.btn.y())
                if (need_cancel_btn):
                    self.tip_widget.cancel_btn.show()
                else:
                    self.tip_widget.cancel_btn.hide()
                self.tip_widget.warning_label.show()
            else:
                self.tip_widget.btn.move(344, 110)
                self.tip_widget.tips_label.move(16, 55)
            self.tip_widget.btn.clicked.connect(self.slot_close_tipwidget)
            self.tip_widget.move(self.x() + int((self.width() - self.tip_widget.width()) / 2),
                                 self.y() + int((self.height() - self.tip_widget.height()) / 2))
            if(has_error_num != ""):
                self.tip_widget.tips_label.setWordWrap(True)
                self.tip_widget.tips_label.setText(self.tip_widget.tips_label.text() + '<a href="'+has_error_num+'\"'
                                                                         'style=\"color:#595959;\">'
                                                                         ''+has_error_num+'</a>')
                self.tip_widget.tips_label.linkActivated.connect(self.click_error_code)
            self.tip_widget.show()
        else:
            if (self.tip_widget.isHidden() == True):
                self.tip_widget.close()
                self.tip_widget = Tip_Widget(text, self.frame_widget)
                self.tip_widget.slot_show_dark_mode()
                self.tip_widget.setWindowModality(Qt.ApplicationModal)
                self.set_dark_mode.connect(self.tip_widget.slot_show_dark_mode)
                self.tip_widget.close_btn.clicked.connect(self.slot_close_tipwidget)
                self.tip_widget.cancel_btn.clicked.connect(self.slot_close_tipwidget)
                self.tip_widget.btn.clicked.connect(self.slot_close_tipwidget)
                # self.tip_widget.btn.clicked.connect(self.slot_close_tipwidget)
                if (is_remove == True):
                    self.tip_widget.btn.move(344, 110)
                    self.tip_widget.tips_label.move(44, 55)
                    self.tip_widget.cancel_btn.move(self.tip_widget.btn.x() - self.tip_widget.cancel_btn.width() - 16,
                                                    self.tip_widget.btn.y())
                    if (need_cancel_btn):
                        self.tip_widget.cancel_btn.show()
                    else:
                        self.tip_widget.cancel_btn.hide()
                    self.tip_widget.warning_label.show()
                else:
                    self.tip_widget.btn.move(344, 110)
                    self.tip_widget.tips_label.move(16, 55)
                    # self.tip_widget.btn.move(int((self.tip_widget.widget.width()-self.tip_widget.btn.width())/2),110)
                    # self.tip_widget.tips_label.move(int((self.tip_widget.widget.width()-self.tip_widget.tips_label.width())/2),55)
                    self.tip_widget.cancel_btn.hide()
                    self.tip_widget.warning_label.hide()
                self.tip_widget.tips_label.setWordWrap(True)
                self.tip_widget.tips_label.setText(text)
                self.tip_widget.tips_label.setAlignment(Qt.AlignLeft)
                self.tip_widget.move(self.x() + int((self.width() - self.tip_widget.width()) / 2),
                                     self.y() + int((self.height() - self.tip_widget.height()) / 2))
                if (has_error_num != ""):
                    self.tip_widget.tips_label.setWordWrap(True)
                    self.tip_widget.tips_label.setText(
                        self.tip_widget.tips_label.text() + '<a href="' + has_error_num + '\"'
                                                                                          'style=\"color:#595959;\">'
                                                                                          '' + has_error_num + '</a>')
                    self.tip_widget.tips_label.linkActivated.connect(self.click_error_code)

                self.tip_widget.show()

    #
    #函数：处理点击错误码
    #
    def click_error_code(self):
        Dbus_Service.show_user_guide("software-store/error-code")

    #
    # 函数：提示下载失败
    #
    def tip_download_error(self, text=_("Download failed, service exception. Please click the error code to see the solution."),has_error_num =""):
        if (not hasattr(self, "download_error_tip_widget")):
            self.download_error_tip_widget = Tip_Widget(text, self.frame_widget)
            self.download_error_tip_widget.setWindowModality(Qt.ApplicationModal)
            self.download_error_tip_widget.tips_label.setAlignment(Qt.AlignLeft)
            self.download_error_tip_widget.btn.move(
                self.download_error_tip_widget.widget.width() - self.download_error_tip_widget.btn.width() - 16,
                self.download_error_tip_widget.widget.height() - self.download_error_tip_widget.btn.height() - 24)
            self.download_error_tip_widget.btn.show()
            self.download_error_tip_widget.tips_label.move(44, 55)
            self.download_error_tip_widget.warning_label.show()
            self.download_error_tip_widget.move(
                self.x() + int((self.width() - self.download_error_tip_widget.width()) / 2),
                self.y() + int((self.height() - self.download_error_tip_widget.height()) / 2))
            self.set_dark_mode.connect(self.download_error_tip_widget.slot_show_dark_mode)
            self.download_error_tip_widget.btn.clicked.connect(self.slot_hide_download_tip_widget)
            self.download_error_tip_widget.close_btn.clicked.connect(self.slot_hide_download_tip_widget)
            if(has_error_num != ""):
                if(Globals.THEME_COLOR == 0):
                    self.download_error_tip_widget.tips_label.setWordWrap(True)
                    self.download_error_tip_widget.tips_label.setText(self.download_error_tip_widget.tips_label.text() + '<a href="' + has_error_num + '\"'
                                                                                         'style=\"color:#595959;\">'
                                                                                         '' + has_error_num + '</a>')
                else:

                    self.download_error_tip_widget.tips_label.setWordWrap(True)
                    self.download_error_tip_widget.tips_label.setText(
                        self.download_error_tip_widget.tips_label.text() + '<a href="' + has_error_num + '\"'
                                                                                                         'style=\"color:#C0C4CC;\">'
                                                                                                         '' + has_error_num + '</a>')
                self.download_error_tip_widget.tips_label.linkActivated.connect(self.click_error_code)
            self.download_error_tip_widget.show()
        else:
            self.download_error_tip_widget.btn.move(self.download_error_tip_widget.widget.width() - self.download_error_tip_widget.btn.width() - 16,self.download_error_tip_widget.widget.height() - self.download_error_tip_widget.btn.height() - 24)
            self.download_error_tip_widget.tips_label.setWordWrap(True)
            self.download_error_tip_widget.tips_label.setText(text)
            if(has_error_num != ""):
                if(Globals.THEME_COLOR == 0):
                    self.download_error_tip_widget.tips_label.setWordWrap(True)
                    self.download_error_tip_widget.tips_label.setText(self.download_error_tip_widget.tips_label.text() + '<a href="' + has_error_num + '\"'
                                                                                         'style=\"color:#595959;\">'
                                                                                         '' + has_error_num + '</a>')
                else:
                    self.download_error_tip_widget.tips_label.setWordWrap(True)
                    self.download_error_tip_widget.tips_label.setText(
                        self.download_error_tip_widget.tips_label.text() + '<a href="' + has_error_num + '\"'
                                                                                                         'style=\"color:#C0C4CC;\">'
                                                                                                         '' + has_error_num + '</a>')
                # self.download_error_tip_widget.tips_label.linkActivated.connect(self.click_error_code)
            self.download_error_tip_widget.show()

    def slot_hide_download_tip_widget(self):
        self.download_error_tip_widget.hide()

    #
    # 函数：提示安装失败
    #
    def tip_install_error(self,name):
        has_error_num = "(" + _("Error Code") + "#0003)"
        if (not hasattr(self, "install_error_tip_widget")):
            self.install_error_tip_widget = Tip_Widget(name + " " +_("Installation failed, service exception.Please click the error code to see the solution."), self.frame_widget)
            self.install_error_tip_widget.setWindowModality(Qt.ApplicationModal)
            self.install_error_tip_widget.tips_label.setAlignment(Qt.AlignLeft)
            self.install_error_tip_widget.btn.move(
                self.install_error_tip_widget.widget.width() - self.install_error_tip_widget.btn.width() - 16,
                self.install_error_tip_widget.widget.height() - self.install_error_tip_widget.btn.height() - 24)
            self.install_error_tip_widget.btn.show()
            self.install_error_tip_widget.tips_label.move(44, 55)
            self.install_error_tip_widget.warning_label.show()
            self.install_error_tip_widget.move(
                self.x() + int((self.width() - self.install_error_tip_widget.width()) / 2),
                self.y() + int((self.height() - self.install_error_tip_widget.height()) / 2))
            self.set_dark_mode.connect(self.install_error_tip_widget.slot_show_dark_mode)
            self.install_error_tip_widget.btn.clicked.connect(self.slot_hide_install_tip_widget)
            self.install_error_tip_widget.close_btn.clicked.connect(self.slot_hide_install_tip_widget)
            if(Globals.THEME_COLOR == 0):
                self.install_error_tip_widget.tips_label.setWordWrap(True)
                self.install_error_tip_widget.tips_label.setText(name + " " +
                    _("Installation failed, service exception.Please click the error code to see the solution.") + '<a href="' + has_error_num + '\"'
                                                                                                    'style=\"color:#595959;\">'
                                                                                                    '' + has_error_num + '</a>')
            else:
                self.install_error_tip_widget.tips_label.setWordWrap(True)
                self.install_error_tip_widget.tips_label.setText(name + " " + _("Installation failed, service exception.Please click the error code to see the solution.") + '<a href="' + has_error_num + '\"'
                                                                                                                                               'style=\"color:#C0C4CC;\">'
                                                                                                                                               '' + has_error_num + '</a>')
            self.install_error_tip_widget.tips_label.linkActivated.connect(self.click_error_code)
            self.install_error_tip_widget.show()
        else:
            if(Globals.THEME_COLOR == 0):
                self.install_error_tip_widget.tips_label.setWordWrap(True)
                self.install_error_tip_widget.tips_label.setText(name + " " + _("Installation failed, service exception.Please click the error code to see the solution.") + '<a href="' + has_error_num + '\"'
                                                                                                    'style=\"color:#595959;\">'
                                                                                                    '' + has_error_num + '</a>')
            else:
                self.install_error_tip_widget.tips_label.setWordWrap(True)
                self.install_error_tip_widget.tips_label.setText(
                    name + " " + _("Installation failed, service exception.Please click the error code to see the solution.") + '<a href="' + has_error_num + '\"'
                                                                                                             'style=\"color:#C0C4CC;\">'
                                                                                                             '' + has_error_num + '</a>')
            # self.install_error_tip_widget.tips_label.linkActivated.connect(self.click_error_code)
            self.install_error_tip_widget.show()

    def slot_hide_install_tip_widget(self):
        self.install_error_tip_widget.hide()


    #
    # 函数：安卓安卓环境没有起来
    #
    def tip_kydroid_env_error(self, text=_("Android environment is not started, do you want to start now?"),has_error_num =""):
        if (not hasattr(self, "kydroidenv_error_tip_widget")):
            self.kydroidenv_error_tip_widget = Tip_Widget(text, self.frame_widget)
            self.kydroidenv_error_tip_widget.setWindowModality(Qt.ApplicationModal)
            self.kydroidenv_error_tip_widget.tips_label.setAlignment(Qt.AlignLeft)
            self.kydroidenv_error_tip_widget.btn.move(
                self.kydroidenv_error_tip_widget.widget.width() - self.kydroidenv_error_tip_widget.btn.width() - 16,
                self.kydroidenv_error_tip_widget.widget.height() - self.kydroidenv_error_tip_widget.btn.height() - 24)
            self.kydroidenv_error_tip_widget.btn.show()
            self.kydroidenv_error_tip_widget.tips_label.move(44, 55)
            self.kydroidenv_error_tip_widget.warning_label.show()
            self.kydroidenv_error_tip_widget.move(
                self.x() + int((self.width() - self.kydroidenv_error_tip_widget.width()) / 2),
                self.y() + int((self.height() - self.kydroidenv_error_tip_widget.height()) / 2))
            self.set_dark_mode.connect(self.kydroidenv_error_tip_widget.slot_show_dark_mode)
            self.kydroidenv_error_tip_widget.btn.clicked.connect(self.start_kydroid_env)
            self.kydroidenv_error_tip_widget.close_btn.clicked.connect(self.slot_hide_kydroid_env_tip_widget)
            if(has_error_num != ""):
                if(Globals.THEME_COLOR == 0):
                    self.kydroidenv_error_tip_widget.tips_label.setWordWrap(True)
                    self.kydroidenv_error_tip_widget.tips_label.setText(self.kydroidenv_error_tip_widget.tips_label.text() + '<a href="' + has_error_num + '\"'
                                                                                         'style=\"color:#595959;\">'
                                                                                         '' + has_error_num + '</a>')
                else:
                    self.kydroidenv_error_tip_widget.tips_label.setWordWrap(True)
                    self.kydroidenv_error_tip_widget.tips_label.setText(
                        self.kydroidenv_error_tip_widget.tips_label.text() + '<a href="' + has_error_num + '\"'
                                                                                                           'style=\"color:#C0C4CC;\">'
                                                                                                           '' + has_error_num + '</a>')
                self.kydroidenv_error_tip_widget.tips_label.linkActivated.connect(self.click_error_code)
            self.kydroidenv_error_tip_widget.show()
        else:
            self.kydroidenv_error_tip_widget.tips_label.setText(text)
            if(has_error_num != ""):
                if(Globals.THEME_COLOR == 0):
                    self.kydroidenv_error_tip_widget.tips_label.setWordWrap(True)
                    self.kydroidenv_error_tip_widget.tips_label.setText(self.kydroidenv_error_tip_widget.tips_label.text() + '<a href="' + has_error_num + '\"'
                                                                                         'style=\"color:#595959;\">'
                                                                                         '' + has_error_num + '</a>')
                else:
                    self.kydroidenv_error_tip_widget.tips_label.setWordWrap(True)
                    self.kydroidenv_error_tip_widget.tips_label.setText(
                        self.kydroidenv_error_tip_widget.tips_label.text() + '<a href="' + has_error_num + '\"'
                                                                                                           'style=\"color:#C0C4CC;\">'
                                                                                                           '' + has_error_num + '</a>')
                # self.kydroidenv_error_tip_widget.tips_label.linkActivated.connect(self.click_error_code)
            self.kydroidenv_error_tip_widget.show()

    def slot_hide_kydroid_env_tip_widget(self):
        self.kydroidenv_error_tip_widget.hide()

    #
    #函数：启动安卓环境
    #
    def start_kydroid_env(self):
        self.kydroidenv_error_tip_widget.hide()
        if(Globals.IS_INIT_KYDROID_ENVIRONMENT == True):
            self.show_tip_widget(_("Starting Android environment, please try again later!"),False,False,"")
            self.tip_widget.btn.clicked.connect(self.hide_tip_widget)
        else:
            Globals.IS_INIT_KYDROID_ENVIRONMENT = True
            self.title_bar.home_page_btn.setEnabled(False)
            self.title_bar.my_btn.setEnabled(False)
            self.title_bar.drive_btn.setEnabled(False)
            self.title_bar.kydroid_btn.setEnabled(False)
            self.title_bar.category_btn.setEnabled(False)
            self.title_bar.search_line.setReadOnly(True)
            self.title_bar.senior_search.setEnabled(False)
            self.title_bar.search_image.setEnabled(False)
            self.title_bar.return_btn.setEnabled(False)
            self.title_bar.login_btn.setEnabled(False)
            os.system(KYDROID_STARTAPP_ENV + " &")
            item = {}
            item["action"] = "cycle_check_kydroid_envrun"
            item["need_show_kyroid_widget"] = False
            self.at.mutex.acquire()
            self.at.work_list.append(item)
            self.at.mutex.release()

    #
    # 函数：提示网络异常
    #
    def tip_network_error(self,string = _("Network disconnection or server access failure!")):
        if (not hasattr(self, "network_error_tip_widget")):
            self.network_error_tip_widget = Tip_Widget(string,self.frame_widget)
            self.network_error_tip_widget.setWindowModality(Qt.ApplicationModal)
            self.network_error_tip_widget.tips_label.setAlignment(Qt.AlignLeft)
            self.network_error_tip_widget.btn.move(
                self.network_error_tip_widget.widget.width() - self.network_error_tip_widget.btn.width() - 16,
                self.network_error_tip_widget.widget.height() - self.network_error_tip_widget.btn.height() - 24)
            self.network_error_tip_widget.btn.show()
            self.network_error_tip_widget.tips_label.move(44, 55)
            self.network_error_tip_widget.warning_label.show()
            self.network_error_tip_widget.move(
                self.x() + int((self.width() - self.network_error_tip_widget.width()) / 2),
                self.y() + int((self.height() - self.network_error_tip_widget.height()) / 2))
            self.set_dark_mode.connect(self.network_error_tip_widget.slot_show_dark_mode)
            self.network_error_tip_widget.btn.clicked.connect(self.slot_hide_net_tip_widget)
            self.network_error_tip_widget.close_btn.clicked.connect(self.slot_hide_net_tip_widget)
            if(Globals.THEME_COLOR == 0):
                self.network_error_tip_widget.tips_label.setWordWrap(True)
                self.network_error_tip_widget.tips_label.setText(self.network_error_tip_widget.tips_label.text())
            else:
                self.network_error_tip_widget.tips_label.setWordWrap(True)
                self.network_error_tip_widget.tips_label.setText(self.network_error_tip_widget.tips_label.text())
            self.network_error_tip_widget.tips_label.linkActivated.connect(self.click_error_code)
            self.network_error_tip_widget.tips_label.setWordWrap(True)
            self.network_error_tip_widget.show()
        else:
            if(Globals.THEME_COLOR == 0):
                self.network_error_tip_widget.tips_label.setWordWrap(True)
                self.network_error_tip_widget.tips_label.setText(string)
            else:
                self.network_error_tip_widget.tips_label.setWordWrap(True)
                self.network_error_tip_widget.tips_label.setText(string)
            # self.network_error_tip_widget.tips_label.linkActivated.connect(self.click_error_code)
            self.network_error_tip_widget.show()

    def slot_hide_net_tip_widget(self):
        self.network_error_tip_widget.hide()

    #
    # 函数：显示错误提示框
    #
    def show_error_tip_widget(self, text):
        self.show_tip_widget(text, False)
        self.tip_widget.btn.clicked.connect(self.slot_close_tipwidget)

    #
    # 函数：关闭提示框
    #
    def slot_close_tipwidget(self):
        self.detailwidget.mywidget.all_check.setEnabled(True)
        self.detailwidget.mywidget.all_select_btn.setEnabled(True)
        if (Globals.NOWPAGE == PageStates.MYPAGE):
            cards = self.detailwidget.mywidget.app_widget.children()
            for card in cards:
                card.check_btn.setEnabled(True)
            self.check_btn_state()
        self.tip_widget.close()

    #
    # 函数：检测是否有kylin-update.lock文件
    #
    def check_lock_file(self):
        is_ksc = False
        is_kum = False
        is_ki = False
        is_start_remote = False
        read_list = []
        if (os.path.exists(Globals.APT_LOCK_FILE)):
            fd = open(Globals.APT_LOCK_FILE, "r")
            read_list = fd.readlines()
            for i in read_list:
                if ("kylin-software-center" in i.split("\n")):
                    is_ksc = True
                    break
                elif ("kylin-update-manager" in i.split("\n")):
                    is_kum = True
                elif ("kylin-installer" in i.split("\n")):
                    is_ki = True
                elif("kylin-software-center-auto-update" in i.split("\n")):
                    is_start_remote = True
            fd.close()
            pass
        else:
            if not os.path.isdir("/tmp/lock"):  # 无文件夹时创建
                os.makedirs("/tmp/lock")
                os.system("chmod 777 " + "/tmp/lock")
            if not os.path.isfile(Globals.APT_LOCK_FILE):  # 无文件时创建
                os.system("touch " + Globals.APT_LOCK_FILE)
                os.system("chmod 666 " + Globals.APT_LOCK_FILE)
            else:
                pass
        return is_ksc, is_kum, is_ki, read_list ,is_start_remote

    #
    #函数：检测剩余空间
    #
    def check_remaining_space(self,need_use_space):
        #存储以kb为单位的
        try:
            root_space = False
            data_space = False
            root_size = 0
            data_size = 0
            if(os.path.exists("/")):
                storage_info = QStorageInfo("/")
                root_size = storage_info.bytesAvailable()
                if (int(need_use_space) + 1024 * 1024 * 1024 < int(root_size)):
                    root_space = True
                else:
                    root_space = False
            else:
                root_space = True
            # if(os.path.exists("/data")):
            #     storage_info1 = QStorageInfo("/data")
            #     data_size = storage_info1.bytesAvailable()
            #     if (int(need_use_space) + 1024 * 1024 * 1024 < int(data_size)):
            #         data_space = True
            #     else:
            #         data_space = False
            # else:
            #     data_space = True
            if(root_space == True):
                return True
            else:
                return False
        except:
            return True


    #
    # 函数：点击安装、卸载、更新、启动按钮
    #
    def slot_click_action_btn(self, widget, btn_name, app_msg):
        appname = app_msg.pkgname
        num = 0
        for i in Globals.APPLICATION_LIST:
            if (i["name"] == app_msg.pkgname):
                break
            num = num + 1
        if (btn_name != _("Open") and btn_name != _("uninstall") and btn_name != _("start env") and btn_name != _("website")):
            is_ksc, is_kum, is_ki, read_list ,is_start_remote= self.check_lock_file()
            if (is_ksc == True or is_start_remote == True):
                self.fd = open(Globals.APT_LOCK_FILE, "w")
                try:
                    fcntl.flock(self.fd.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)
                    self.fd.write(
                        Globals.UID + "\n" + "kylin-software-center\n" + "install\n" + time.strftime(
                            "%Y-%m-%d %H:%M:%SZ",
                            time.localtime()) + "\n\n")
                except:
                    pass
                if (type(app_msg) == ApkInfo):
                    # if (((Globals.INIT_APK_LIST_OVER == False) and Globals.IS_INIT_KYDROID_ENVIRONMENT == False) or self.at.appmgr.check_kydroid_envrun() == False):
                    if (self.at.appmgr.check_kydroid_envrun() == False):
                        self.tip_kydroid_env_error()
                        return
                if (btn_name == _("update")):
                    res = self.init_dbus_thread.backend.call_auth_with_policykit(
                        _("install app or update app or remove app"), "com.kylin.softwarecenter.install")
                    self.slot_install_app(app_msg, num, res)
                    self.slot_all_puase_btn_state()
                elif (btn_name == _("down")):
                    res = self.init_dbus_thread.backend.call_auth_with_policykit(
                     _("install app or update app or remove app"), "com.kylin.softwarecenter.install")
                    self.slot_all_puase_btn_state()
                    self.slot_install_app(app_msg, num, True)
                    self.slot_all_puase_btn_state()
                elif (btn_name == _("cont")):
                    widget.btn.hide()
                    self.slot_all_puase_btn_state()
                    res = self.init_dbus_thread.backend.call_auth_with_policykit(
                        _("install app or update app or remove app"), "com.kylin.softwarecenter.install")
                    self.slot_install_app(app_msg, num, res)
                    self.slot_all_puase_btn_state()
                elif (btn_name == _("download experience")):
                    # fd.close()
                    res = self.init_dbus_thread.backend.call_auth_with_policykit(
                        _("install app or update app or remove app"), "com.kylin.softwarecenter.install")
                    self.slot_all_puase_btn_state()
                    self.slot_install_app(app_msg, num, res)
                    self.slot_all_puase_btn_state()
            else:
                self.fd = open(Globals.APT_LOCK_FILE, "w")
                try:
                    fcntl.flock(self.fd.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)
                    apt_lock = False
                    if (type(app_msg) == ApkInfo):
                        # if (((Globals.INIT_APK_LIST_OVER == False) and Globals.IS_INIT_KYDROID_ENVIRONMENT == False) or self.at.appmgr.check_kydroid_envrun() == False):
                        if (self.at.appmgr.check_kydroid_envrun() == False):
                            self.tip_kydroid_env_error()
                            return
                    if (btn_name == _("update")):
                        self.fd.write(Globals.UID + "\n" + "kylin-software-center\n" + "install\n" + time.strftime(
                            "%Y-%m-%d %H:%M:%SZ", time.localtime()) + "\n\n")
                        self.fd.flush()
                        res = self.init_dbus_thread.backend.call_auth_with_policykit(
                            _("install app or update app or remove app"), "com.kylin.softwarecenter.install")
                        self.slot_install_app(app_msg, num, res)
                        self.slot_all_puase_btn_state()
                    elif (btn_name == _("down")):
                        self.fd.write(Globals.UID + "\n" + "kylin-software-center\n" + "install\n" + time.strftime(
                            "%Y-%m-%d %H:%M:%SZ", time.localtime()) + "\n\n")
                        self.fd.flush()
                        res = self.init_dbus_thread.backend.call_auth_with_policykit(
                            _("install app or update app or remove app"), "com.kylin.softwarecenter.install")
                        self.slot_all_puase_btn_state()
                        self.slot_install_app(app_msg, num, res)
                        self.slot_all_puase_btn_state()
                    elif (btn_name == _("cont")):
                        widget.btn.hide()
                        self.slot_all_puase_btn_state()
                        res = self.init_dbus_thread.backend.call_auth_with_policykit(
                            _("install app or update app or remove app"), "com.kylin.softwarecenter.install")
                        self.slot_install_app(app_msg, num, res)
                        self.slot_all_puase_btn_state()
                    elif (btn_name == _("download experience")):
                        self.fd.write(Globals.UID + "\n" + "kylin-software-center\n" + "install\n" + time.strftime(
                            "%Y-%m-%d %H:%M:%SZ", time.localtime()) + "\n\n")
                        self.fd.flush()
                        res = self.init_dbus_thread.backend.call_auth_with_policykit(
                            _("install app or update app or remove app"), "com.kylin.softwarecenter.install")
                        self.slot_all_puase_btn_state()
                        self.slot_install_app(app_msg, num, res)
                        self.slot_all_puase_btn_state()
                    else:
                        pass
                except:
                    # fd.close()
                    apt_lock = True
                    for i in read_list:
                        self.fd.write(i)
                    self.fd.close()
                    self.read_which_app_using(is_kum, is_ki)
        elif(btn_name == _("start env")):
            if ((Globals.INIT_APK_LIST_OVER == False) and Globals.IS_INIT_KYDROID_ENVIRONMENT == False):
                self.tip_kydroid_env_error()
            else:
                if (Globals.IS_INIT_KYDROID_ENVIRONMENT):
                    self.show_tip_widget(_("Starting Android environment, please try again later!"), False,
                                         False, "")
                    self.tip_widget.btn.clicked.connect(self.hide_tip_widget)
        if (btn_name == _("uninstall")):
            self.slot_uninstall_app(app_msg, num)
        if(btn_name == _("website")):
            webbrowser_open_url(app_msg.website)
            self.send_notice_click.emit(str(app_msg.app_id))
        if (btn_name == _("Open")):
            if (type(app_msg) != ApkInfo):
                app = self.detailwidget.ca[app_msg.pkgname]
                if (not hasattr(app_msg, "RUN")):
                    try:
                        cmd = run.run_app(app.name)
                        self.ask_thread.run_iface.call_run_app(cmd)
                    except:
                        pass
                else:
                    try:
                        app.run()
                    except:
                        pass
            else:
                if ((Globals.INIT_APK_LIST_OVER == False) and Globals.IS_INIT_KYDROID_ENVIRONMENT == False):
                    self.tip_kydroid_env_error()
                else:
                    if (Globals.IS_INIT_KYDROID_ENVIRONMENT):
                        self.show_tip_widget(_("Starting Android environment, please try again later!"), False,
                                             False, "")
                        self.tip_widget.btn.clicked.connect(self.hide_tip_widget)
                    else:
                        try:
                            cmd = self.run_apk(app_msg.pkgname)
                            self.ask_thread.run_iface.call_run_app(cmd)
                        except:
                            pass

    def run_apk(self, apkname):
        if self.kydroid_service is not None:
            try:
                cmd = self.kydroid_service.launch_app(apkname)
            except:
                if (Globals.DEBUG_SWITCH):
                    # print("apk %s run error" % apkname)
                    Globals.LOG_ERROR.logger.error(_("apk %s run error" % apkname))
        return cmd

    #
    #函数：处理发送预上架应用点击信息
    #
    def slot_send_notice_click(self,appid):
        ask_item = {"action": "get_preview_exposure", "appid": appid}
        self.ask_thread.mutex.acquire()
        self.ask_thread.work_list.append(ask_item)
        self.ask_thread.mutex.release()
        pass

    #
    # 函数：获取预装应用点击信息
    #
    def slot_get_getPreviewNum(self,appid):
        ask_item = {"action": "get_getPreviewNum", "appid": appid}
        self.ask_thread.mutex.acquire()
        self.ask_thread.work_list.append(ask_item)
        self.ask_thread.mutex.release()

    #
    #函数：处理获取点击次数失败
    #
    def slot_get_previe_num_fail(self,appid):
        self.detailwidget.app_detail_widget.slot_get_previe_num_over(appid)

    #
    #函数：处理获取点击次数结束
    #
    def slot_send_previe_num(self,appid,count):
        self.detailwidget.app_detail_widget.slot_get_previe_num_over(appid,count)

    #
    # 函数：更新多个应用
    #
    def slot_update_multiple_app(self, cards):
        res = self.init_dbus_thread.backend.call_auth_with_policykit(_("install app or update app or remove app"),
                                                                     "com.kylin.softwarecenter.install")
        if res == True:
            self.detailwidget.mywidget.all_check.setEnabled(True)
            self.detailwidget.mywidget.all_select_btn.setEnabled(True)
            is_ksc, is_kum, is_ki, read_list ,is_start_remote= self.check_lock_file()
            if (is_ksc == True):
                self.fd = open(Globals.APT_LOCK_FILE, "w")
                try:
                    fcntl.flock(self.fd.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)
                    self.fd.write(
                        Globals.UID + "\n" + "kylin-software-center\n" + "install\n" + time.strftime(
                            "%Y-%m-%d %H:%M:%SZ",
                            time.localtime()) + "\n\n")
                except:
                    pass
                for card in cards:
                    num = 0
                    for i in Globals.APPLICATION_LIST:
                        if (i["name"] == card.app.pkgname):
                            break
                        num = num + 1
                    card.check_btn.hide()
                    card.check_btn.setChecked(False)
                    self.slot_install_app(card.app, num, res)
                    # pool.submit(self.slot_install_app,card.app, num, res)
                self.slot_all_puase_btn_state()
            else:
                self.fd = open(Globals.APT_LOCK_FILE, "w")
                try:
                    fcntl.flock(self.fd.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)
                    self.fd.write(Globals.UID + "\n" + "kylin-software-center\n" + "install\n" + time.strftime(
                        "%Y-%m-%d %H:%M:%SZ",
                        time.localtime()) + "\n\n")
                    self.fd.flush()
                    for card in cards:
                        num = 0
                        for i in Globals.APPLICATION_LIST:
                            if (i["name"] == card.app.pkgname):
                                break
                            num = num + 1
                        card.check_btn.hide()
                        card.check_btn.setChecked(False)
                        self.slot_install_app(card.app, num, res)
                        # pool.submit(self.slot_install_app, card.app, num, res)
                    self.slot_all_puase_btn_state()
                except:
                    # fd.close()
                    apt_lock = True
                    for i in read_list:
                        self.fd.write(i)
                    self.fd.close()
                    self.read_which_app_using(is_kum, is_ki)
        else:
            self.detailwidget.mywidget.all_check.setEnabled(True)
            self.detailwidget.mywidget.all_select_btn.setEnabled(True)
            for card in cards:
                card.check_btn.setEnabled(True)

    #
    # 函数：全部继续下载
    #
    def slot_all_continue_download(self, widget):
        cards = widget.children()
        res = self.init_dbus_thread.backend.call_auth_with_policykit(_("install app or update app or remove app"),
                                                                     "com.kylin.softwarecenter.install")
        if (res == True):
            is_ksc, is_kum, is_ki, read_list ,is_start_remote= self.check_lock_file()
            if (is_ksc == True):
                for card in cards:
                    num = 0
                    for i in Globals.APPLICATION_LIST:
                        if (i["name"] == card.app.pkgname and card.btn.text() != _("Open")):
                            if (card.app.status != PkgStates.INSTALLING):
                                self.fd = open(Globals.APT_LOCK_FILE, "w")
                                try:
                                    fcntl.flock(self.fd.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)
                                    self.fd.write(
                                        Globals.UID + "\n" + "kylin-software-center\n" + "install\n" + time.strftime(
                                            "%Y-%m-%d %H:%M:%SZ",
                                            time.localtime()) + "\n\n")
                                except:
                                    pass
                                self.slot_install_app(card.app, num, res)
                            break
                        num = num + 1
                self.slot_all_puase_btn_state()
            else:
                self.fd = open(Globals.APT_LOCK_FILE, "w")
                try:
                    fcntl.flock(self.fd.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)
                    self.fd.write(Globals.UID + "\n" + "kylin-software-center\n" + "install\n" + time.strftime(
                        "%Y-%m-%d %H:%M:%SZ", time.localtime()) + "\n\n")
                    self.fd.flush()
                    for card in cards:
                        num = 0
                        for i in Globals.APPLICATION_LIST:
                            if (i["name"] == card.app.pkgname and card.btn.text() != _("Open")):
                                if (card.app.status != PkgStates.INSTALLING):
                                    self.slot_install_app(card.app, num, res)
                                break
                            num = num + 1
                    self.slot_all_puase_btn_state()
                    # fd.close()
                except:
                    # fd.close()
                    for i in read_list:
                        self.fd.write(i)
                    self.read_which_app_using()

    #
    # 函数：安装软件
    #
    def slot_install_app(self, app_msg, num, res):
        n = self.at.appmgr.check_kydroid_envrun()
        is_downloading = False
        if (res == True):
            num = 0
            for i in Globals.APPLICATION_LIST:
                if (i["name"] == app_msg.pkgname):
                    break
                num = num + 1
            self.change_global_info(num, PkgStates.DOWNLOADING, 0)
            cards = self.detailwidget.mywidget.downloading_widget.children()
            in_download_widget = False
            for i in cards:
                if (i.app.pkgname == app_msg.pkgname):
                    in_download_widget = True
                    i.down_progress.btn.show()
                    i.down_progress.cont_btn.hide()
                    break

            if (in_download_widget == False):
                if (type(app_msg) == ApkInfo):
                    # if (((Globals.INIT_APK_LIST_OVER == False) and Globals.IS_INIT_KYDROID_ENVIRONMENT == False) or self.at.appmgr.check_kydroid_envrun() == False):
                    if (self.at.appmgr.check_kydroid_envrun() == False):
                        self.tip_kydroid_env_error()
                    else:
                        if (Globals.IS_INIT_KYDROID_ENVIRONMENT):
                            self.show_tip_widget(_("Starting Android environment, please try again later!"), False,
                                                 False, "")
                            self.tip_widget.btn.clicked.connect(self.hide_tip_widget)
                        else:
                            self.change_global_info(num, PkgStates.DOWNLOADING, 0)
                            self.add_apk_to_downloading_widget.emit(app_msg)
                            # for i in self.down_app_msg_dict:
                            if (self.is_downloading == True):
                                if (app_msg not in self.wait_download_app_list):
                                    self.wait_download_app_list.append(app_msg)
                                # is_downloading = True
                                # break
                            if (self.is_downloading == False):
                                if (app_msg in self.wait_download_app_list):
                                    self.wait_download_app_list.remove(app_msg)
                                self.downloading_thread = pool.submit(self.slot_down_app, app_msg, num)
                                self.is_downloading = True
                else:
                    self.change_global_info(num, PkgStates.DOWNLOADING, 0)
                    self.add_app_to_downloading_widget.emit(app_msg)
                    # for i in self.down_app_msg_dict:
                    if (self.is_downloading == True):
                        if (app_msg not in self.wait_download_app_list):
                            self.wait_download_app_list.append(app_msg)
                        # is_downloading = True
                        # break
                    if (self.is_downloading == False):
                        if (app_msg in self.wait_download_app_list):
                            self.wait_download_app_list.remove(app_msg)
                        self.downloading_thread = pool.submit(self.slot_down_app, app_msg, num)
                        self.is_downloading = True
            else:
                # for i in self.down_app_msg_dict:
                if (self.is_downloading == True):
                    if (app_msg not in self.wait_download_app_list):
                        self.wait_download_app_list.append(app_msg)
                    # is_downloading = True
                    # break
                if (self.is_downloading == False):
                    if (app_msg in self.wait_download_app_list):
                        self.wait_download_app_list.remove(app_msg)
                    self.downloading_thread = pool.submit(self.slot_down_app, app_msg, num)
                    self.is_downloading = True
        else:
            pass
            # if(res == None):
            #     print("后台服务异常请重启软件商店")

    #
    # 函数：卸载软件
    #
    def slot_uninstall_app(self, app_msg, num):
        self.show_tip_widget(_(
            "Are you sure you want to uninstall the app? You can download it again later in the historical installation page"),
                             True, True)
        self.tip_widget.btn.clicked.connect(lambda: self.tip_widget_button_click(app_msg, num))

    #
    # 函数：进行软件的卸载
    #
    def do_remove_app(self, app_msg, num, res):
        if (hasattr(self, "tip_widget")):
            self.tip_widget.close()
        if (res == True):
            if (type(app_msg) != ApkInfo):
                dict = {"appname": app_msg.pkgname, "need_check": True}
                self.remove_app_dict.append(dict)
                self.signal_uninstall.emit(app_msg.pkgname, "uninstalling")
                action = {"appname": app_msg.pkgname, "action": "remove"}
                self.init_dbus_thread.mutex.acquire()
                self.init_dbus_thread.dbus_work_list.append(action)
                self.init_dbus_thread.mutex.release()
                self.remove_app_num = self.remove_app_num + 1
                self.change_global_info(num, PkgStates.REMOVING, 0)
                # pool.submit(self.init_dbus_thread.backend.remove_package,appname)
            else:
                # if ((Globals.INIT_APK_LIST_OVER == False) and Globals.IS_INIT_KYDROID_ENVIRONMENT == False):
                if (self.at.appmgr.check_kydroid_envrun() == False):
                    self.tip_kydroid_env_error()
                else:
                    if(Globals.IS_INIT_KYDROID_ENVIRONMENT):
                        self.show_tip_widget(_("Starting Android environment, please try again later!"), False, False, "")
                        self.tip_widget.btn.clicked.connect(self.hide_tip_widget)
                    else:
                        self.signal_uninstall.emit(app_msg.pkgname, "uninstalling")
                        action = {"appname": app_msg.pkgname, "action": "remove_apk"}
                        self.at.mutex.acquire()
                        self.at.work_list.append(action)
                        self.at.mutex.release()
                        self.remove_app_num = self.remove_app_num + 1
                        self.change_global_info(num, PkgStates.REMOVING, 0)
        pass

    #
    # 函数：点击提示框按钮
    #
    def tip_widget_button_click(self, app_msg, num):
        res = self.init_dbus_thread.backend.call_auth_with_policykit(_("install app or update app or remove app"),
                                                                     "com.kylin.softwarecenter.remove")
        if (res):
            is_ksc, is_kum, is_ki, read_list ,is_start_remote= self.check_lock_file()
            if (is_ksc == True):
                self.fd = open(Globals.APT_LOCK_FILE, "w")
                try:
                    fcntl.flock(self.fd.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)
                    self.fd.write(
                        Globals.UID + "\n" + "kylin-software-center\n" + "install\n" + time.strftime(
                            "%Y-%m-%d %H:%M:%SZ",
                            time.localtime()) + "\n\n")
                except:
                    pass
                self.do_remove_app(app_msg, num, res)
            else:
                self.fd = open(Globals.APT_LOCK_FILE, "w")
                try:
                    fcntl.flock(self.fd.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)
                    self.fd.write(Globals.UID + "\n" + "kylin-software-center\n" + "remove\n" + time.strftime(
                        "%Y-%m-%d %H:%M:%SZ", time.localtime()) + "\n\n")
                    self.fd.flush()
                    self.do_remove_app(app_msg, num, res)
                except:
                    # fd.close()
                    for i in read_list:
                        self.fd.write(i)
                    self.fd.close()
                    self.read_which_app_using(is_kum, is_ki)
        else:
            pass

    #
    # 函数“：卸载多个应用
    #
    def slot_uninstall_multiple_app(self, cards):
        self.detailwidget.mywidget.all_check.setStyleSheet(
            "QPushButton{font-size: 14px;font-weight: 500;color: #F56C6C;line-height: 20px;border:0px solid #ff0000;}QPushButton:hover{background: rgba(245, 108, 108, 0.12);border-radius: 12px;}QPushButton:pressed{background: rgba(245, 108, 108, 0.12);border-radius: 12px;}")
        self.show_tip_widget(_(
            "Are you sure you want to uninstall the app? You can download it again later in the historical installation page"),
                             True, True)
        self.tip_widget.btn.clicked.connect(lambda: self.do_remove_multiple_app(cards))

    #
    # 函数：进行多个应用的卸载
    #
    def do_remove_multiple_app(self, cards):
        self.tip_widget.close()
        self.detailwidget.mywidget.all_check.setStyleSheet(
            "QPushButton{font-size: 14px;font-weight: 500;color: #F56C6C;line-height: 20px;border:0px solid #ff0000;}QPushButton:hover{background: rgba(245, 108, 108, 0.12);border-radius: 12px;}QPushButton:pressed{background: rgba(245, 108, 108, 0.12);border-radius: 12px;}")
        res = self.init_dbus_thread.backend.call_auth_with_policykit(_("install app or update app or remove app"),
                                                                     "com.kylin.softwarecenter.remove")
        self.detailwidget.mywidget.all_check.setStyleSheet(
            "QPushButton{font-size: 14px;font-weight: 500;color: #F56C6C;line-height: 20px;border:0px solid #ff0000;}QPushButton:hover{background: rgba(245, 108, 108, 0.12);border-radius: 12px;}QPushButton:pressed{background: rgba(245, 108, 108, 0.12);border-radius: 12px;}")
        if res == True:
            self.detailwidget.mywidget.all_check.setEnabled(True)
            self.detailwidget.mywidget.all_select_btn.setEnabled(True)
            is_ksc, is_kum, is_ki, read_list ,is_start_remote= self.check_lock_file()
            is_ksc = False
            if (is_ksc == True):
                for card in cards:
                    num = 0
                    for i in Globals.APPLICATION_LIST:
                        if (i["name"] == card.app.pkgname):
                            break
                        num = num + 1
                    self.fd = open(Globals.APT_LOCK_FILE, "w")
                    try:
                        fcntl.flock(self.fd.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)
                        self.fd.write(
                            Globals.UID + "\n" + "kylin-software-center\n" + "install\n" + time.strftime(
                                "%Y-%m-%d %H:%M:%SZ",
                                time.localtime()) + "\n\n")
                    except:
                        pass
                    self.do_remove_app(card.app, num, res)
            else:
                # if(hasattr((self)))
                self.fd = open(Globals.APT_LOCK_FILE, "w")
                try:
                    fcntl.flock(self.fd.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)
                    self.fd.write(Globals.UID + "\n" + "kylin-software-center\n" + "remove\n" + time.strftime(
                        "%Y-%m-%d %H:%M:%SZ", time.localtime()) + "\n\n")
                    self.fd.flush()
                    for card in cards:
                        num = 0
                        for i in Globals.APPLICATION_LIST:
                            if (i["name"] == card.app.pkgname):
                                break
                            num = num + 1
                        self.do_remove_app(card.app, num, res)
                    # fd.close()
                except:
                    # fd.close()
                    for i in read_list:
                        self.fd.write(i)
                    self.fd.close()
                    self.read_which_app_using(is_kum, is_ki)

        else:
            self.detailwidget.mywidget.all_check.setEnabled(True)
            self.detailwidget.mywidget.all_select_btn.setEnabled(True)
            for card in cards:
                card.check_btn.setEnabled(True)
        pass

    #
    # 函数：修改Globals.APPLICATION的信息
    #
    def change_global_info(self, num, state, percent, need_change_btn=True, has_dektop=True):
        if(num != None):
            if (num < len(Globals.APPLICATION_LIST)):
                Globals.APPLICATION_LIST[num]["appmsg"].status = state
                if (state != PkgStates.PUASE):
                    Globals.APPLICATION_LIST[num]["appmsg"].percent = percent
                if (need_change_btn == True):
                    self.change_card_state.emit(Globals.APPLICATION_LIST[num]["appmsg"].pkgname, state)
                    pass

    #
    # 函数：检测是否还有需要下载的app
    #
    def check_has_app_need_download(self):
        # is_downloading = False

        self.is_downloading = False
        for i in self.down_app_msg_dict:
            if (i["is_downloading"] == True):
                self.is_downloading = True
        if (len(self.wait_download_app_list) != 0):
            if (self.is_downloading == False):
                down_appmsg = self.wait_download_app_list.pop(0)
                num = 0
                for i in Globals.APPLICATION_LIST:
                    if (i["name"] == down_appmsg.pkgname):
                        break
                    num = num + 1
                if (num < len(Globals.APPLICATION_LIST)):
                    self.slot_install_app(down_appmsg, num, True)
                    self.slot_all_puase_btn_state()
        else:
            all_puase = True
            for card in self.detailwidget.mywidget.downloading_widget.children():
                if (card.down_progress.btn.isHidden() == False):
                    all_puase = False
            if(all_puase == True):
                self.unlock_file(True,False)

    #
    # 函数：获取应用下载信息（包括下载路径、安装时的包全名、大小）
    # 通过命令：apt-get --print-uris install xxxx
    #
    def get_app_download_msg(self, appname, depends,first=True):
        dep_list = []
        res = []
        url_list = []
        i_list = []
        need_use_space = 0
        all_size = 0
        get_depends = False
        is_installed = False
        depends_list = None
        need_install_list = []
        if(depends != None and depends != []):
            for i in depends.split(","):
                try:
                    dep = self.detailwidget.ca[i]
                    if(dep.candidate != None and (dep.is_installed == False or dep.is_upgradable == True)):
                        depends_list = []
                        depends_list.append(i)
                        need_install_list.append(i)
                except:
                    continue
            need_install_list.append(appname)
            # need_install_list.extend(depends.split(","))
        else:
            need_install_list.append(appname)
        # print("need_install",need_install_list)
        for app in need_install_list:
            app_msg = {"name": "", "app_fullname": "", "size": "", "url": ""}
            try:
                res = os.popen("LC_ALL=en_US apt-get install --print-uris %s -y" % app).readlines()
            except:
                return dep_list, all_size, False, False, depends_list,need_use_space
            num = 0
            for i in res:
                if ("is already the newest version" in i and app == appname):
                    is_installed = True
                    break
                if("Need to get" in i and "of archives" in i):
                    for count in range(len(i.split(" "))-1):
                        pattern = re.compile('[0-9]+')
                        match = pattern.findall(i.split(" ")[count])
                        if(match):
                            size = 0.0
                            str_size = ""
                            for get_size in match:
                                if (str_size == ""):
                                    str_size = get_size
                                else:
                                    str_size = str_size + "." + get_size
                            size = float(str_size)
                            if(i.split(" ")[count + 1] == "MB"):
                                need_use_space = need_use_space + size * 1024 * 1024
                            elif(i.split(" ")[count + 1] == "KB"):
                                need_use_space = need_use_space + size * 1024
                            elif(i.split(" ")[count + 1] == "GB"):
                                need_use_space = need_use_space + size * 1024 * 1024 * 1024
                            # elif(i):
                            break
                if("After this operation" in i and "of additional disk space will be used" in i):
                    for count in range(len(i.split(" "))-1):
                        pattern = re.compile('[0-9]+')
                        match = pattern.findall(i.split(" ")[count])
                        if(match):
                            size = 0.0
                            str_size = ""
                            for get_size in match:
                                if(str_size == ""):
                                    str_size = get_size
                                else:
                                    str_size = str_size + "." + get_size
                            size = float(str_size)
                            if(i.split(" ")[count + 1] == "MB"):
                                need_use_space = need_use_space + size * 1024
                            elif(i.split(" ")[count + 1] == "KB"):
                                need_use_space = need_use_space + size
                            elif(i.split(" ")[count + 1] == "GB"):
                                need_use_space = need_use_space + size * 1024 * 1024
                            break
                if ("After this operation," in i):
                    get_depends = True
                    num = num + 1
                    while (num < len(res)):
                        i = res[num]
                        i_list = i.strip("\n").split(" ")
                        app_msg["name"] = i_list[1].split("_")[0]
                        app_msg["size"] = int(i_list[2])
                        app_msg["url"] = i_list[0].strip("'")
                        app_msg["app_fullname"] = i_list[1]
                        if(app_msg not in dep_list):
                            dep_list.append(app_msg)
                            all_size = all_size + int(i_list[2])
                        i_list = []
                        app_msg = {"name": "", "app_fullname": "", "size": "", "url": ""}
                        num = num + 1
                    break
                num = num + 1
        # print(dep_list, all_size, get_depends, is_installed,depends_list)
        if (get_depends == False):
            if (first == True):
                self.get_app_download_msg(appname, None,False)
            else:
                return dep_list, all_size, get_depends, is_installed,depends_list,need_use_space

        return dep_list, all_size, get_depends, is_installed,depends_list,need_use_space

    #
    # 函数：下载软件
    # self.down_app_msg_dict:用于存储点了更新和安装的软件信息
    # appname:软件包名(不带版本)  down_load_size:需要下载的大小  is_downloading:是否正在下载  is_puasee:是否点击了暂停  is_installing:是否正在安装
    # 参数：app_msg,application_num:是这个appmsg在Globals.APPPLICATION_LIST中的位置
    # ！！！！函数内的num 不能随便修改
    #
    def slot_down_app(self, app_msg, application_num):
        if (Globals.DEBUG_SWITCH):
            # print("dowm_deb_pkg")
            Globals.LOG_INFO.logger.info("dowm_deb_pkg:  " + app_msg.pkgname)
        self.is_downloading_app = True
        self.need_copy_file_list = []
        # self.need_down_file_li?st = []
        self.need_down_file_list[app_msg.pkgname] = []
        dep_list = []
        all_size = 0
        need_use_space = 0
        get_depends_success = True
        depends = None
        is_apk = False
        down_size = 0
        if (type(app_msg) == ApkInfo):
            is_apk = True
        if (os.path.isdir(WGET_DOWN_PATH) != True):
            os.system("mkdir " + WGET_DOWN_PATH)
        if (os.path.isdir(STORE_DEB_PATH) != True):
            os.system("mkdir " + STORE_DEB_PATH)
        log_file = os.path.join(WGET_DOWN_PATH, "wget_log")
        if (os.path.exists(log_file)):
            fp = open(log_file, "w")
            fp.close()
        else:
            os.system("touch " + log_file)
        if (is_apk == False):
            dep_list, all_size, get_depends_success, is_installed,depends,need_use_space = self.get_app_download_msg(app_msg.pkgname,app_msg.depends)
            if (get_depends_success == False):
                if (is_installed):
                    self.sigenal_show_downloading_widget.emit(app_msg.pkgname, True, False)
                    # widget.show_downloading_widget(True)
                    self.init_dbus_thread.backend.dbus_apt_process.emit(app_msg.pkgname, "apt", "install", -12, "")
                    # self.detailwidget.tip_error.emit(_("Failed to download the software. The software source is incomplete or does not contain the software"))
                    self.check_has_app_need_download()
                    # 软件安装
                    self.tip_download_error_signal.emit(_("The software is installed!"),"")
                else:
                    # self.sigenal_show_downloading_widget.emit(app_msg.pkgname, True, False)
                    # widget.show_downloading_widget(True)
                    need_check_apt = self.need_check_apt
                    self.init_dbus_thread.backend.dbus_apt_process.emit(app_msg.pkgname, "apt", "install", -11, "")
                    # self.detailwidget.tip_error.emit(_("Failed to download the software. The software source is incomplete or does not contain the software"))
                    if(need_check_apt == False):
                        self.check_has_app_need_download()
                        if(Globals.LAN == "zh_CN"):
                            if(app_msg.is_upgradable):
                                self.tip_download_error_signal.emit(app_msg.displayname_cn + " " + _(
                                    "Update failed, service exception. Please click the error code to see the solution."),
                                                                    "(" + _("Error Code") + "#0002)")
                            else:
                                self.tip_download_error_signal.emit(app_msg.displayname_cn + " " + _("Download failed, service exception. Please click the error code to see the solution."),"("+_("Error Code")+"#0002)")
                        else:
                            if (app_msg.is_upgradable):
                                self.tip_download_error_signal.emit(app_msg.pkgname + " " + _(
                                    "Update failed, service exception. Please click the error code to see the solution."),
                                                                    "(" + _("Error Code") + "#0002)")
                            else:
                                self.tip_download_error_signal.emit(app_msg.pkgname + " " + _("Download failed, service exception. Please click the error code to see the solution."),
                                                                "(" + _("Error Code") + "#0002)")
                pass
            else:
                has_space = self.check_remaining_space(need_use_space)
                # has_space = False
                if(has_space == False):
                    self.init_dbus_thread.backend.dbus_apt_process.emit(app_msg.pkgname, "apt", "install", -13, "")
                    # self.detailwidget.tip_error.emit(_("Failed to download the software. The software source is incomplete or does not contain the software"))
                    self.check_has_app_need_download()
                    if (Globals.LAN == "zh_CN"):
                        if (app_msg.is_upgradable):
                            self.tip_download_error_signal.emit(app_msg.displayname_cn + " " + _("Update failed,please free up disk space."))
                        else:
                            self.tip_download_error_signal.emit(app_msg.displayname_cn + " " + _("Download failed,please free up disk space."))
                    else:
                        if (app_msg.is_upgradable):
                            self.tip_download_error_signal.emit(app_msg.pkgname + " " + _("Update failed,please free up disk space."))
                        else:
                            self.tip_download_error_signal.emit(app_msg.pkgname + " " + _("Download failed,please free up disk space."))
                    return
                app_install_state = {"name": app_msg.pkgname, "is_installing": False, "is_downloading": True,
                                     "is_puase": False, "install_finish": False, "down_load_size": 0,
                                     "need_copy_file_list": [], "is_apk": False, "need_check": True,"depends":depends}
                num = 0
                in_list = False
                for i in self.down_app_msg_dict:
                    if i["name"] == app_msg.pkgname:
                        self.down_app_msg_dict[num]["is_installing"] = False
                        self.down_app_msg_dict[num]["is_downloading"] = True
                        self.down_app_msg_dict[num]["is_puase"] = False
                        self.down_app_msg_dict[num]["install_finish"] = False
                        in_list = True
                        break
                    num = num + 1
                if (in_list == False):
                    app_install_state["down_load_size"] = all_size
                    self.down_app_msg_dict.insert(num, app_install_state)
                    self.all_download_size = all_size
                else:
                    self.all_download_size = self.down_app_msg_dict[num]["down_load_size"]
                if (dep_list == []):
                    self.down_app_msg_dict[num]["is_installing"] = True
                    self.down_app_msg_dict[num]["is_downloading"] = False
                    self.change_global_info(application_num, PkgStates.INSTALLING, 100)
                    # self.send_download_progress.emit(app_msg.pkgname, int(100))
                    self.wait_install_app_list.append(app_msg.pkgname)
                    self.is_downloading = False
                    self.slot_copy_file_finish("", app_msg.pkgname)
                else:
                    for app in dep_list:
                        self.need_down_file_list[app_msg.pkgname].append(app["app_fullname"])
                        if (os.path.isfile(os.path.join(STORE_DEB_PATH, app["app_fullname"]))):
                            down_size = down_size + app["size"]
                            self.change_global_info(application_num, PkgStates.DOWNLOADING,
                                                    int((down_size / self.all_download_size) * 100))
                            self.send_download_progress.emit(app_msg.pkgname,
                                                             int((down_size / self.all_download_size) * 100))
                            self.down_app_msg_dict[num]["need_copy_file_list"].append(app["app_fullname"])
                            if (int((down_size / self.all_download_size) * 100) == 100):
                                self.change_global_info(application_num, PkgStates.INSTALLING, 100)
                                self.wait_install_app_list.append(app_msg.pkgname)
                                self.copy_file_to_archives.emit(app_msg.pkgname)
                        else:
                            for i in self.down_app_msg_dict:
                                if i["name"] == app_msg.pkgname:
                                    in_list = True
                                    break
                            if (in_list):
                                self.down_load_progress = subprocess.Popen(
                                    ["wget", "-c", "-P", WGET_DOWN_PATH, app["url"], "-O",
                                     WGET_DOWN_PATH + "/" + app["app_fullname"], "-o", log_file,"-T", "5"])
                                self.th = pool.submit(self.tail_f, app, down_size, app_msg, application_num, app["url"],
                                                      app["app_fullname"], log_file)
                                get_size = self.th.result()
                                if (int(get_size) == -1):
                                    self.init_dbus_thread.backend.dbus_apt_process.emit(app_msg.pkgname, "apt",
                                                                                        "install", -14, "")
                                    for i in self.down_app_msg_dict:
                                        if (i["name"] == app_msg.pkgname):
                                            in_dict = True
                                            self.down_app_msg_dict.remove(i)
                                    self.check_has_app_need_download()
                                    if(app_msg.is_upgradable):
                                        self.tip_network_error_signal.emit(_(
                                            "Update failed.Please check whether you are connected to the network or reconnect to the network."))
                                    else:
                                        self.tip_network_error_signal.emit(_("Download failed.Please check whether you are connected to the network or reconnect to the network."))
                                    return
                                else:
                                    down_size = down_size + int(get_size)
                            else:
                                self.slot_puase_download_pkg(app_msg)
                                return
        else:
            has_space = self.check_remaining_space(int(app_msg.packageSize))
            # has_space = False
            if (has_space == False):
                self.init_dbus_thread.backend.dbus_apt_process.emit(app_msg.pkgname, "apt", "install", -13, "")
                # self.detailwidget.tip_error.emit(_("Failed to download the software. The software source is incomplete or does not contain the software"))
                self.check_has_app_need_download()
                if (Globals.LAN == "zh_CN"):
                    if (app_msg.is_upgradable):
                        self.tip_download_error_signal.emit(app_msg.displayname_cn + " " + _("Update failed,please free up disk space."))
                    else:
                        self.tip_download_error_signal.emit(app_msg.displayname_cn + " " + _("Download failed,please free up disk space."))
                else:
                    if (app_msg.is_upgradable):
                        self.tip_download_error_signal.emit(app_msg.pkgname + " " + _("Update failed,please free up disk space."))
                    else:
                        self.tip_download_error_signal.emit(app_msg.pkgname + " " + _("Download failed,please free up disk space."))
                return
            app_install_state = {"name": app_msg.pkgname, "is_installing": False, "is_downloading": True,
                                 "is_puase": False, "install_finish": False, "down_load_size": 0,
                                 "need_copy_file_list": [], "depends":depends,"is_apk": True}
            num = 0
            in_list = False
            for i in self.down_app_msg_dict:
                if i["name"] == app_msg.pkgname:
                    self.down_app_msg_dict[num]["is_installing"] = False
                    self.down_app_msg_dict[num]["is_downloading"] = True
                    self.down_app_msg_dict[num]["is_puase"] = False
                    self.down_app_msg_dict[num]["install_finish"] = False
                    in_list = True
                    break
                num = num + 1
            if (in_list == False):
                app_install_state["down_load_size"] = app_msg.packageSize
                self.down_app_msg_dict.insert(num, app_install_state)
                self.all_download_size = app_msg.packageSize
            else:
                self.all_download_size = self.down_app_msg_dict[num]["down_load_size"]
            apkurl = os.path.join(Globals.KYDROID_SOURCE_SERVER, app_msg.file_path)
            filename = app_msg.file_path.split('/')[-1]
            self.need_down_file_list[app_msg.pkgname].append(filename)
            app = {"name": app_msg.pkgname, "app_fullname": filename, "size": app_msg.size, "url": apkurl}
            if (os.path.isfile(os.path.join(KYDROID_DOWNLOAD_PATH, filename))):
                self.down_app_msg_dict[num]["is_installing"] = True
                self.down_app_msg_dict[num]["is_downloading"] = False
                self.change_global_info(application_num, PkgStates.INSTALLING, 100)
                # self.remove_card(self.detailwidget.mywidget.downloading_widget,app_msg.pkgname)
                # self.send_download_progress.emit(app_msg.pkgname, int(100))
                self.wait_install_app_list.append(app_msg.pkgname)
                self.is_downloading = False
                self.slot_copy_file_finish("", app_msg.pkgname)
            else:
                if (os.path.isfile(os.path.join(STORE_DEB_PATH, filename))):
                    down_size = down_size + app["size"]
                    self.down_app_msg_dict[num]["need_copy_file_list"].append(filename)
                    if (int((down_size / self.all_download_size) * 100) == 100):
                        self.is_downloading_app = False
                        self.change_global_info(application_num, PkgStates.INSTALLING, 100)
                        self.wait_install_app_list.append(app_msg.pkgname)
                        self.down_app_msg_dict[num]["is_downloading"] = False
                        self.down_app_msg_dict[num]["is_installing"] = True
                        self.copy_file_to_archives.emit(app_msg.pkgname)
                        if (len(self.wait_download_app_list) != 0):
                            appmsg = self.wait_download_app_list.pop(0)
                            num = 0
                            for i in Globals.APPLICATION_LIST:
                                if (i["name"] == appmsg.pkgname):
                                    break
                                num = num + 1
                            if (num < len(Globals.APPLICATION_LIST)):
                                res = self.init_dbus_thread.backend.call_auth_with_policykit(
                                    _("install app or update app or remove app"), "com.kylin.softwarecenter.install")
                                is_ksc, is_kum, is_ki, read_list ,is_start_remote= self.check_lock_file()
                                if (is_ksc == True):
                                    self.fd = open(Globals.APT_LOCK_FILE, "w")
                                    try:
                                        fcntl.flock(self.fd.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)
                                        self.fd.write(
                                            Globals.UID + "\n" + "kylin-software-center\n" + "install\n" + time.strftime(
                                                "%Y-%m-%d %H:%M:%SZ",
                                                time.localtime()) + "\n\n")
                                    except:
                                        pass
                                    self.slot_install_app(appmsg, num, res)
                                    self.slot_all_puase_btn_state()
                                else:
                                    self.fd = open(Globals.APT_LOCK_FILE, "w")
                                    try:
                                        fcntl.flock(self.fd.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)
                                        self.fd.write(
                                            Globals.UID + "\n" + "kylin-software-center\n" + "install\n" + time.strftime(
                                                "%Y-%m-%d %H:%M:%SZ", time.localtime()) + "\n\n")
                                        # fd.close()
                                        self.slot_install_app(appmsg, num, res)
                                        self.slot_all_puase_btn_state()
                                    except:
                                        for i in read_list:
                                            self.fd.write(i)
                                        self.fd.close()
                                        self.read_which_app_using(is_kum, is_ki)
                else:
                    in_list = False
                    for i in self.down_app_msg_dict:
                        if i["name"] == app_msg.pkgname:
                            in_list = True
                            break
                    if (in_list):
                        cmd = ["wget", "-c", "-P", WGET_DOWN_PATH, apkurl, "-O", WGET_DOWN_PATH + "/" + filename, "-o",
                               log_file]
                        self.down_load_progress = subprocess.Popen(
                            ["wget", "-c", "-P", WGET_DOWN_PATH, apkurl, "-O", WGET_DOWN_PATH + "/" + filename, "-o",
                             log_file, "-T", "5"])
                        self.th = pool.submit(self.tail_f, app, down_size, app_msg, application_num, apkurl, filename,
                                              log_file)
                        get_size = 0
                        get_size = self.th.result()
                        if (int(get_size) == -1):
                            self.init_dbus_thread.backend.dbus_apt_process.emit(app_msg.pkgname, "apt", "install", -14, "")
                            for i in self.down_app_msg_dict:
                                if (i["name"] == app_msg.pkgname):
                                    in_dict = True
                                    self.down_app_msg_dict.remove(i)
                            self.check_has_app_need_download()
                            if(app_msg.is_upgradable):
                                self.tip_network_error_signal.emit(_(
                                    "Update failed.Please check whether you are connected to the network or reconnect to the network."))
                            else:
                                self.tip_network_error_signal.emit(_("Download failed.Please check whether you are connected to the network or reconnect to the network."))
                            return
                        else:
                            down_size = down_size + int(get_size)

    #
    # 函数：获取当前下载大小，下载进度，以及是否下载结束
    # app:正在下载包的信息字典
    # _down_size:下载这个包之前下载的大小
    # downloading_appname:正在下载的主包包名
    #
    def tail_f(self, app, _down_size, app_msg, application_num, down_url, downapp_fullname, log_file):
        try_count = 0
        error = False
        down_file_size = int(app["size"])
        down_size = int(_down_size)
        pre_size = down_size
        downloading_appname = app_msg.pkgname
        down_load_path = os.path.expandvars('$HOME') + "/.cache/uksc/wget_down"
        deb_tmp_dir = os.path.expandvars('$HOME') + "/.cache/uksc/wget_down/deb"
        size = 0
        while (1):
            if (self.click_close_btn == True):
                break
            num = 0
            in_list = False
            for i in self.down_app_msg_dict:
                if (i["name"] == downloading_appname):
                    in_list = True
                    break
                num = num + 1
            if ( in_list == True and self.down_app_msg_dict[num]["is_downloading"] == True and self.down_app_msg_dict[num][
                "is_puase"] == False and Globals.APPLICATION_LIST[application_num]["appmsg"].status == PkgStates.DOWNLOADING):
                try:
                    size = os.path.getsize(os.path.join(down_load_path, app["app_fullname"]))
                except:
                    continue
                if (size == down_file_size):
                    down_size = pre_size + size
                    percent = int((down_size / self.all_download_size) * 100)
                    if (percent == 100):
                        application_num = 0
                        for i in Globals.APPLICATION_LIST:
                            if (i["name"] == app_msg.pkgname):
                                break
                            application_num = application_num + 1
                        self.change_global_info(application_num, PkgStates.INSTALLING, 100)
                    else:
                        application_num = 0
                        for i in Globals.APPLICATION_LIST:
                            if (i["name"] == app_msg.pkgname):
                                break
                            application_num = application_num + 1
                        self.change_global_info(application_num, PkgStates.DOWNLOADING, percent)
                    for i in self.down_app_msg_dict:
                        if i["name"] == app_msg.pkgname:
                            in_list = True
                            break
                    if (in_list):
                        self.send_download_progress.emit(downloading_appname, percent)
                    else:
                        self.slot_puase_download_pkg(app_msg)
                        return 0
                    file_deb_path = os.path.join(down_load_path, app["app_fullname"])
                    tmp_deb_path = os.path.join(deb_tmp_dir, app["app_fullname"])
                    cmd = "mv " + file_deb_path + " " + tmp_deb_path
                    os.rename(file_deb_path, tmp_deb_path)
                    # os.system(cmd)
                    self.down_app_msg_dict[num]["need_copy_file_list"].append(app["app_fullname"])
                    if (int((down_size / self.all_download_size) * 100) >= 100):
                        self.wait_install_app_list.append(downloading_appname)
                        self.down_app_msg_dict[num]["is_downloading"] = False
                        self.down_app_msg_dict[num]["is_installing"] = True
                        self.copy_file_to_archives.emit(downloading_appname)
                        # self.check_has_app_need_download()
                    break
                else:
                    down_size = pre_size + size
                    application_num = 0
                    for i in Globals.APPLICATION_LIST:
                        if (i["name"] == app_msg.pkgname):
                            break
                        application_num = application_num + 1
                    self.change_global_info(application_num, PkgStates.DOWNLOADING,
                                            int((down_size / self.all_download_size) * 100))
                    for i in self.down_app_msg_dict:
                        if (i["name"] == downloading_appname):
                            in_list = True
                            break
                    if (in_list):
                        self.send_download_progress.emit(downloading_appname,
                                                         int((down_size / self.all_download_size) * 100))
                    else:
                        self.slot_puase_download_pkg(app_msg)
                        return 0
                    pass
                if (self.down_load_progress.poll() != None and ( self.down_load_progress.poll() == 4 or self.down_load_progress.poll() == 8) and size != down_file_size):
                    if (try_count < 5):
                        self.down_load_progress = subprocess.Popen(["wget", "-c", "-P", WGET_DOWN_PATH, down_url, "-O",
                                                                    WGET_DOWN_PATH + "/" + downapp_fullname, "-o",
                                                                    log_file, "-T", "10"])
                        try_count = try_count + 1
                    else:
                        return -1
                    # return -1
                # 防止获取到的文件大小有问题
                if (self.down_load_progress.poll() == 0):
                    down_file_size = size
                    self.all_download_size = size
            else:
                break
            # pre_size = down_size
            time.sleep(0.5)
        return app["size"]

    #
    # 函数：处理拷贝文件到安装目录
    #
    def slot_copy_file_to_archives(self, appname):
        copy_file_list = []
        is_apk = False
        for i in self.down_app_msg_dict:
            if (i["name"] == appname):
                copy_file_list = i["need_copy_file_list"]
                is_apk = i["is_apk"]
                break
        if (is_apk == False):
            while (len(copy_file_list) != 0):
                app_fullname = copy_file_list.pop(0)
                if len(copy_file_list) != 0:
                    self.init_dbus_thread.backend.call_copy_file_to_archives(app_fullname, STORE_DEB_PATH, appname,
                                                                             False)
                else:
                    self.init_dbus_thread.backend.call_copy_file_to_archives(app_fullname, STORE_DEB_PATH, appname,
                                                                             True)
        else:
            while (len(copy_file_list) != 0):
                app_fullname = copy_file_list.pop(0)
                self.init_dbus_thread.backend.call_copy_file_to_apk_tmp(app_fullname, STORE_DEB_PATH,
                                                                        KYDROID_DOWNLOAD_PATH, appname)

    #
    # 函数：处理拷贝文件结束信号，判断是否开始安装
    #
    def slot_copy_file_finish(self, app_name, install_appname):
        copy_file_list = []
        is_apk = False
        for i in self.down_app_msg_dict:
            if (i["name"] == install_appname):
                copy_file_list = i["need_copy_file_list"]
                is_apk = i["is_apk"]
                break
        if (len(copy_file_list) == 0):
            if (len(self.wait_install_app_list) != 0):
                install_appname = self.wait_install_app_list.pop(0)
                num = 0
                for i in self.down_app_msg_dict:
                    if (i["name"] == install_appname):
                        self.down_app_msg_dict[num]["is_downloading"] = False
                        self.down_app_msg_dict[num]["is_installing"] = True
                        break
                    num = num + 1
                action = {"appname": install_appname, "action": "install"}
                if (is_apk == False):
                    action = {"appname": install_appname, "action": "install"}
                    self.init_dbus_thread.mutex.acquire()
                    self.init_dbus_thread.dbus_work_list.append(action)
                    self.init_dbus_thread.mutex.release()
                    if (self.down_app_msg_dict[num]["depends"] != None and self.down_app_msg_dict[num]["depends"] != []):
                        for dep_name in self.down_app_msg_dict[num]["depends"]:
                            action = {"appname": dep_name, "action": "install"}
                            self.init_dbus_thread.mutex.acquire()
                            self.init_dbus_thread.dbus_work_list.append(action)
                            self.init_dbus_thread.mutex.release()
                else:
                    for i in self.at.appmgr.apk_list:
                        if (i.pkgname == install_appname):
                            self.start_install_apk.emit(i)
            self.check_has_app_need_download()

    #
    # 函数:开始安装apk
    #
    def slot_start_install_apk(self, app):
        item = {}
        item["action"] = "install_apk"
        item["app"] = app
        self.init_dbus_thread.mutex.acquire()
        self.init_dbus_thread.dbus_work_list.append(item)
        self.init_dbus_thread.mutex.release()

    #
    # 函数：处理删除下载软件字典以及修改application,返回该软件在application中的下标
    #
    def rm_app_msg_and_dict(self, appname):
        num = 0
        for i in Globals.APPLICATION_LIST:
            if (i["name"] == appname):
                break
            num = num + 1
        for i in self.down_app_msg_dict:
            if (i["name"] == appname):
                self.down_app_msg_dict.remove(i)
        for i in self.remove_app_dict:
            if (i["appname"] == appname):
                self.remove_app_dict.remove(i)
        #删除在“我的”界面搜索提示
        if("MY" in Globals.SEARCH_APP_LIST.keys() and Globals.SEARCH_APP_LIST["MY"] != None):
            for i in Globals.SEARCH_APP_LIST["MY"]:
                if(type(i) != ApkInfo):
                    if (i["name"] == appname):
                        Globals.SEARCH_APP_LIST["MY"].remove(i)
                        break
                else:
                    if(i.name == appname):
                        Globals.SEARCH_APP_LIST["MY"].remove(i)
        if (num > len(Globals.APPLICATION_LIST)-1):
            return None
        else:
            return num

    #
    # 函数：判断软件的desktop文件
    #
    def check_app_desktop(self, pkgname):
        # 对一些特殊软件单独处理

        fullcmd = ""

        desktopfile = "/usr/share/applications/" + pkgname + ".desktop"
        desktopfile1 = "/usr/share/applications/kde4/" + pkgname + ".desktop"

        if os.path.exists(desktopfile):
            return True
        elif os.path.exists(desktopfile1):
            return True
        else:
            if pkgname in Globals.PKG_NAME:
                pkgname = Globals.PKG_NAME[pkgname]

                desktopfile = "/usr/share/applications/" + pkgname + ".desktop"

                if os.path.exists(desktopfile):
                    return True
                else:
                    desktopfile = "/usr/share/applications/kde4/" + pkgname + ".desktop"
                    if (os.path.exists(desktopfile)):
                        return True
                    else:
                        return False
            else:
                return False

    #
    # 函数：获取后台dbus工作个数
    #
    def slot_get_dbus_workitem_over(self, workcount):
        if (workcount == 0):
            self.unlock_file(False, True)

    #
    # 函数：解锁文件锁
    #
    def unlock_file(self, install, remove):
        if (install == True):
            # fd = open(Globals.APT_LOCK_FILE,"a+")
            if (len(self.wait_download_app_list) == 0 and len(self.wait_install_app_list) == 0):
                self.check_btn_state()
                try:
                    if (hasattr(self, "fd") and os.path.exists(Globals.APT_LOCK_FILE)):
                        fcntl.flock(self.fd.fileno(), fcntl.LOCK_UN | fcntl.LOCK_NB)
                        self.fd.close()
                        os.system("chmod 666 " + Globals.APT_LOCK_FILE)
                except:
                    if (hasattr(self, "fd")):
                        self.fd.close()
                    if (os.path.exists("/tmp/lock/kylin-update.lock")):
                        os.popen("rm /tmp/lock/kylin-update.lock")
                    pass
        if (remove and self.remove_app_num == 0):
            try:
                if (hasattr(self, "fd") and os.path.exists(Globals.APT_LOCK_FILE)):
                    fcntl.flock(self.fd.fileno(), fcntl.LOCK_UN | fcntl.LOCK_NB)
                    self.fd.close()
                    os.system("chmod 666 " + Globals.APT_LOCK_FILE)
            except:
                if (hasattr(self, "fd")):
                    self.fd.close()
                if (os.path.exists("/tmp/lock/kylin-update.lock")):
                    os.popen("rm /tmp/lock/kylin-update.lock")
                pass

    def read_which_app_using(self, is_kum, is_ki):
        app_name = ""
        if (is_kum):
            if (Globals.LAN == "zh_CN"):
                app_name = "麒麟更新管理器"
            else:
                app_name = "kylin update manager"
        if (is_ki):
            if (Globals.LAN == "zh_CN"):
                app_name = "麒麟安装管理器"
            else:
                app_name = "kylin installer"
        if(is_kum == False and is_ki == False):
            if (Globals.LAN == "zh_CN"):
                app_name = "其他应用"
            else:
                app_name = "Other application"
        if (app_name != ""):
            self.show_tip_widget(app_name + _(" are installing or uninstalling software, please try again later!"))
            self.tip_widget.btn.clicked.connect(self.slot_close_tipwidget)
        else:
            self.show_tip_widget(
                _("Other applications are installing or uninstalling software, please try again later!"))
            self.tip_widget.btn.clicked.connect(self.slot_close_tipwidget)

    #
    # 函數：修复apt后进行的操作
    #
    def slot_check_apt_lock_finish(self, appname, action,percent):
        if(percent != -11):
            action = {"appname": appname, "action": action}
            self.init_dbus_thread.mutex.acquire()
            self.init_dbus_thread.dbus_work_list.append(action)
            self.init_dbus_thread.mutex.release()
        else:
            #为-11表示没有走过下载
            num = 0
            for i in Globals.APPLICATION_LIST:
                if (i["name"] == appname):
                    break
                num = num + 1
            if(num < len(Globals.APPLICATION_LIST)):
                self.slot_down_app(Globals.APPLICATION_LIST[num]["appmsg"],num)

    #
    #函数：修改应用卸载的搜索列表
    #
    def change_search_remove_list(self,appname):
        if (Globals.NOWPAGE == PageStates.MYPAGE):
            if (self.detailwidget.mywidget.type_name_label.text() == _("Uninstall")):
                for i in Globals.SEARCH_APP_LIST["MY"]:
                    if (type(i) == ApkInfo):
                        if (appname == i.pkgname):
                            Globals.SEARCH_APP_LIST["MY"].remove(i)
                    else:
                        if (appname == i["name"]):
                            Globals.SEARCH_APP_LIST["MY"].remove(i)

    #
    # 函数：处理后台发送信号
    #
    def slot_dbus_apt_process(self, appname, sendType, action, percent, sendMsg):
        if (Globals.DEBUG_SWITCH):
            Globals.LOG_INFO.logger.info(_(
                "#########" + str(appname) + "," + str(sendType) + "," + str(action) + "," + str(percent) + "," + str(
                    sendMsg) + "#########"))
        # print("#########"+str(appname)+","+str(sendType)+","+str(action)+","+str(percent)+","+str(sendMsg)+"#########")
        if (sendType == "apt" and action == "install" and (percent == 200 or sendMsg == "软件安装完成" or sendMsg == "Software Installation"+"Perfection!" or sendMsg == _("Software Installation")+_("Perfection!"))):
            Globals.NEED_REFRESH_CACHE = True
            num = self.rm_app_msg_and_dict(appname)
            try:
                del self.need_down_file_list[appname]
            except:
                pass
            installed_db = Record_History_Installation()
            installed_db.insert(appname)
            if(num != None):
                if(Globals.APPLICATION_LIST[num]["appmsg"].is_upgradable == True and (Globals.NOWPAGE == PageStates.MYPAGE or Globals.NOWPAGE == PageStates.DOWNLOADINGPAGE or Globals.NOWPAGE == PageStates.HISTORY) and hasattr(self.detailwidget,"mywidget")):
                    need_update_num = self.detailwidget.mywidget.update_num_label.text()
                    if(need_update_num != "" and need_update_num.isdigit()):
                        need_update_num = str(int(need_update_num) - 1)
                        if(need_update_num == "0"):
                            self.detailwidget.mywidget.update_num_label.setText(need_update_num)
                            self.detailwidget.mywidget.update_num_label.hide()
                        else:
                            self.detailwidget.mywidget.update_num_label.setText(need_update_num)
                            self.detailwidget.mywidget.update_num_label.adjustSize()
                Globals.APPLICATION_LIST[num]["appmsg"].is_removed = False
                self.download_count_change.emit(appname, Globals.APPLICATION_LIST[num]["appmsg"].downloadcount + 1,Globals.APPLICATION_LIST[num]["appmsg"].screenshots_updatetime)
                self.slot_submit_down_over(appname, Globals.APPLICATION_LIST[num]["appmsg"].app_id ,Globals.APPLICATION_LIST[num]["appmsg"].version)
            if (appname == 'kylin-software-center'):
                if action == AppActions.INSTALL:
                    # cd = ConfirmDialog("软件中心升级完成\n点击【确认】按钮重启软件中心\n重启将取消处于等待状态的任务", self)
                    self.show_tip_widget(_("Update successful, please restart!"))
                    self.tip_widget.btn.clicked.connect(self.restart_software_now)
            elif(appname == Globals.KYDROID_PACKAGE_NAME):
                conf = configparser.ConfigParser()
                conf.read(CONF_PATH)
                SHA_TZ = datetime.timezone(datetime.timedelta(hours=8),name='Asia/Shanghai',)
                # 协调世界时
                utc_now = datetime.datetime.utcnow().replace(tzinfo=datetime.timezone.utc)
                beijing_now = utc_now.astimezone(SHA_TZ)
                install_time = str(beijing_now).split(".")[0]

                # install_time = datetime.datetime.strftime(datetime.datetime.now(),'%Y-%m-%d %H:%M:%S')
                try:
                    conf.set("essential-information", "install_kmre", install_time)
                    with open(CONF_PATH, "w+") as f:
                        conf.write(f)
                except:
                    pass
                if action == AppActions.INSTALL:
                    self.show_tip_widget(_("The installation is successful. Please restart the system and use it!"))
            item = {}
            item["appname"] = appname
            item["action"] = "reflesh cache"
            item["apt_action"] = action
            item["percent"] = int(percent)
            self.at.mutex.acquire()
            self.at.work_list.append(item)
            self.at.mutex.release()
            self.unlock_file(True, False)
            # if(num != None):
            #     self.change_global_info(num,PkgStates.NOTHING,0,True)
        elif (sendType == "apt" and action == "install" and percent > 0 and percent < 200):
            num = self.rm_app_msg_and_dict(appname)
            self.change_global_info(num, PkgStates.INSTALLING, 100)
        elif (sendType == "apt" and action == "install" and percent == (0 - 11)):
            # self.init_dbus_thread.backend.call_check_apt_lock()
            if (self.need_check_apt == True):
                self.need_check_apt = False
                action = {"appname": appname, "action": "check_apt_lock", "app_action": "install","percent":int(percent)}
                self.init_dbus_thread.mutex.acquire()
                self.init_dbus_thread.dbus_work_list.append(action)
                self.init_dbus_thread.mutex.release()
            else:
                self.need_check_apt = True
                num = self.rm_app_msg_and_dict(appname)
                self.remove_card(self.detailwidget.mywidget.downloading_widget, appname)
                item = {}
                item["appname"] = appname
                item["action"] = "reflesh cache"
                item["apt_action"] = action
                item["percent"] = int(percent)
                self.at.mutex.acquire()
                self.at.work_list.append(item)
                self.at.mutex.release()
                self.unlock_file(True, False)
                if (num != None):
                    self.change_global_info(num, PkgStates.NOTHING, 0, False)
        elif (sendType == "apt" and action == "install" and (percent == (0 - 12) or percent == (0 - 13) or percent == (0 - 14))):
            #-12:应用通过其他途径安装成功
            #-13:系统空间不足
            #-14:网络异常
            # self.init_dbus_thread.backend.call_check_apt_lock()
            num = self.rm_app_msg_and_dict(appname)
            self.remove_card(self.detailwidget.mywidget.downloading_widget, appname)
            item = {}
            item["appname"] = appname
            item["action"] = "reflesh cache"
            item["apt_action"] = action
            item["percent"] = int(percent)
            self.at.mutex.acquire()
            self.at.work_list.append(item)
            self.at.mutex.release()
            self.unlock_file(True, False)
            if (num != None):
                self.change_global_info(num, PkgStates.NOTHING, 0, False)
        elif (sendType == "apt" and action == "install" and percent < 0):
            #是否需要进行apt锁检测
            if (self.need_check_apt == True):
                self.need_check_apt = False
                action = {"appname": appname, "action": "check_apt_lock", "app_action": "install","percent":int(percent)}
                self.init_dbus_thread.mutex.acquire()
                self.init_dbus_thread.dbus_work_list.append(action)
                self.init_dbus_thread.mutex.release()
            else:
                self.need_check_apt = True
                num = self.rm_app_msg_and_dict(appname)
                self.remove_card(self.detailwidget.mywidget.downloading_widget, appname)
                item = {}
                item["appname"] = appname
                item["action"] = "reflesh cache"
                item["apt_action"] = action
                item["percent"] = int(percent)
                self.at.mutex.acquire()
                self.at.work_list.append(item)
                self.at.mutex.release()
                self.unlock_file(True, False)
                if (num != None):
                    self.change_global_info(num, PkgStates.NOTHING, 0, True)
                try:
                    pkg = self.at.ca[appname]
                except:
                    pkg = None
                if(pkg != None and (pkg.is_installed == False or pkg.is_upgradable == True)):
                    if(Globals.LAN == "zh_CN"):
                        if(num != None):
                            self.tip_install_error(Globals.APPLICATION_LIST[num]["appmsg"].displayname_cn)
                        else:
                            self.tip_install_error(appname)
                    else:
                        self.tip_install_error(appname)
        elif (sendType == "fetch" and action == "install" and percent > 0 and percent < 100):
            num = 0
            for i in Globals.APPLICATION_LIST:
                if (i["name"] == appname):
                    break
                num = num + 1
            if (num > len(Globals.APPLICATION_LIST) - 1):
                pass
            else:
                self.send_download_progress.emit(appname, percent)
        elif (sendType == "fetch" and sendMsg == _("download failed")):
            num = self.rm_app_msg_and_dict(appname)
            pool.submit(self.remove_card, self.detailwidget.mywidget.downloading_widget, appname)
            item = {}
            item["appname"] = appname
            item["action"] = "reflesh cache"
            item["apt_action"] = action
            item["percent"] = int(percent)
            self.at.mutex.acquire()
            self.at.work_list.append(item)
            self.at.mutex.release()
            self.unlock_file(True, False)
            if (num != None):
                self.change_global_info(num, PkgStates.NOTHING, 0, False)
        elif (sendType == "apt" and action == "remove" and percent > 0 and percent < 200):
            num = 0
            self.signal_uninstall.emit(appname, "uninstalling")
        elif (sendType == "apt" and action == "remove" and (percent == 200 or sendMsg == "软件卸载完成") or sendMsg == "Software uninstall"+"Perfection!" or sendMsg == _("Software uninstall")+_("Perfection!") ):
            Globals.NEED_REFRESH_CACHE = True
            self.remove_app_num = self.remove_app_num - 1
            installed_db = Record_History_Installation()
            installed_db.insert(appname)
            num = self.rm_app_msg_and_dict(appname)
            Globals.APPLICATION_LIST[num]["appmsg"].is_removed = True
            self.slot_submit_remove_over(appname, Globals.APPLICATION_LIST[num]["appmsg"].app_id,Globals.APPLICATION_LIST[num]["appmsg"].version)
            self.change_search_remove_list(appname)
            if (appname == 'kylin-software-center'):
                self.init_dbus_thread.backend.clear_dbus_worklist()
                self.init_dbus_thread.backend.exit_uksc_apt_daemon()
                # self.dbusControler.stop()
                self.unlock_file(False, True)
                self.unlock_uksc_lockfile()
                sys.exit(0)
            else:
                item = {}
                item["appname"] = appname
                item["action"] = "reflesh cache"
                item["apt_action"] = action
                item["percent"] = int(percent)
                self.at.mutex.acquire()
                self.at.work_list.append(item)
                self.at.mutex.release()
                if (num != None):
                    self.change_global_info(num, PkgStates.NOTHING, 0, False)
                self.unlock_file(False, True)
        elif (sendType == "apt" and action == "remove" and percent == -11):
            Globals.NEED_REFRESH_CACHE = True
            num = self.rm_app_msg_and_dict(appname)
            self.show_tip_widget(Globals.APPLICATION_LIST[num]["appmsg"].displayname_cn+ " " +_("The software has been uninstalled from other sources!"), True)
            self.tip_widget.btn.clicked.connect(self.slot_close_tipwidget)
            self.remove_app_num = self.remove_app_num - 1
            installed_db = Record_History_Installation()
            installed_db.insert(appname)
            num = self.rm_app_msg_and_dict(appname)
            Globals.APPLICATION_LIST[num]["appmsg"].is_removed = True
            self.slot_submit_remove_over(appname, Globals.APPLICATION_LIST[num]["appmsg"].app_id,Globals.APPLICATION_LIST[num]["appmsg"].version)
            if (appname == 'kylin-software-center'):
                # self.init_dbus_thread.backend.clear_dbus_worklist()
                # self.init_dbus_thread.backend.exit_uksc_apt_daemon()
                self.check_exit_deamon()
                # self.dbusControler.stop()
                self.unlock_file(False, True)
                self.unlock_uksc_lockfile()
                sys.exit(0)
            else:
                item = {}
                item["appname"] = appname
                item["action"] = "reflesh cache"
                item["apt_action"] = action
                item["percent"] = int(percent)
                self.at.mutex.acquire()
                self.at.work_list.append(item)
                self.at.mutex.release()
                if (num != None):
                    self.change_global_info(num, PkgStates.NOTHING, 0, False)
                self.unlock_file(False, True)
        elif (sendType == "apt" and action == "remove" and percent < 0):
            # need_check = False
            # num = 0
            # for i in self.remove_app_dict:
            #     if (i["appname"] == appname):
            #         need_check = i["need_check"]
            #         break
            #     num = num + 1
            # if (need_check == True):
            #     self.remove_app_dict[num]["need_check"] = False
                # self.init_dbus_thread.backend.call_check_apt_lock()
            if(self.need_check_apt == True):
                self.need_check_apt = False
                action = {"appname": appname, "action": "check_apt_lock", "app_action": "remove","percent":int(percent)}
                self.init_dbus_thread.mutex.acquire()
                self.init_dbus_thread.dbus_work_list.append(action)
                self.init_dbus_thread.mutex.release()
            else:
                self.need_check_apt = True
                num = self.rm_app_msg_and_dict(appname)
                if(Globals.LAN == "zh_CN"):
                    if(num != None):
                        self.show_tip_widget(Globals.APPLICATION_LIST[num]["appmsg"].displayname_cn+ " " +_("Uninstall failed, service exception. Please click the error code to see the solution."), True, False,
                                             "(" + _("Error Code") + "#0004)")
                    else:
                        self.show_tip_widget(appname + " " + _("Uninstall failed, service exception. Please click the error code to see the solution."), True,False,"("+_("Error Code")+"#0004)")
                else:
                    self.show_tip_widget(appname + " " + _("Uninstall failed, service exception. Please click the error code to see the solution."), True, False,
                                         "(" + _("Error Code") + "#0004)")
                self.tip_widget.btn.clicked.connect(self.slot_close_tipwidget)
                item = {}
                item["appname"] = appname
                item["action"] = "reflesh cache"
                item["apt_action"] = action
                item["percent"] = int(percent)
                self.at.mutex.acquire()
                self.at.work_list.append(item)
                self.at.mutex.release()
                if (num != None):
                    self.change_global_info(num, PkgStates.NOTHING, 0, False)
                self.remove_app_num = self.remove_app_num - 1
                self.unlock_file(False, True)
        elif (percent < 0):
            num = self.rm_app_msg_and_dict(appname)
            if (num != None):
                self.change_global_info(num, PkgStates.NOTHING, 0, False)
            self.remove_card(self.detailwidget.mywidget.downloading_widget, appname)

    #
    # 函数：刷新cache后重置按钮
    #
    def slot_reflesh_cache_finish(self, appname):
        self.signal_reload_btn.emit(appname)
        # self.slot_clicked_my_widget_btn(self.detailwidget.mywidget.type_name_label.text())
        num = 0
        for i in Globals.APPLICATION_LIST:
            if (i["name"] == appname):
                break
            num = num + 1
        if (num > len(Globals.APPLICATION_LIST)):
            num = None
        else:
            if (num != None):
                self.remove_card(self.detailwidget.mywidget.downloading_widget, appname)
                self.slot_all_puase_btn_state()
                self.signal_installed.emit(appname)
                self.change_global_info(num, PkgStates.NOTHING, 0, False)

    #
    # 函数：处理apk状态的变化
    #
    def slot_apk_progress(self, name, processtype, action, percent, msg):
        # print("**********",name,processtype,action,percent,msg)
        if (Globals.DEBUG_SWITCH):
            # print("**********",name,processtype,action,percent,msg)
            Globals.LOG_INFO.logger.info(_("**********" + str(name) + "," + str(processtype) + "," + str(action) + "," + str(percent) + "," + str(
                    msg)))
        num = 0
        for i in Globals.APPLICATION_LIST:
            if (i["name"] == name):
                break
            num = num + 1
        app = self.at.appmgr.get_apk_by_name(name)
        if int(percent) >= 200:
            if app is not None:
                app.percent = 0
            if (app and action == AppActions.INSTALL):
                num = self.rm_app_msg_and_dict(name)
                try:
                    del self.need_down_file_list[name]
                except:
                    pass
                installed_db = Record_History_Installation()
                installed_db.insert(name)
                if (app.is_upgradable == True and (Globals.NOWPAGE == PageStates.MYPAGE or Globals.NOWPAGE == PageStates.DOWNLOADINGPAGE or Globals.NOWPAGE == PageStates.HISTORY) and hasattr(self.detailwidget, "mywidget")):
                    need_update_num = self.detailwidget.mywidget.update_num_label.text()
                    if (need_update_num != "" and need_update_num.isdigit()):
                        need_update_num = str(int(need_update_num) - 1)
                        if (need_update_num == "0"):
                            self.detailwidget.mywidget.update_num_label.setText(need_update_num)
                            self.detailwidget.mywidget.update_num_label.hide()
                        else:
                            self.detailwidget.mywidget.update_num_label.setText(need_update_num)
                            self.detailwidget.mywidget.update_num_label.adjustSize()
                self.download_count_change.emit(name, Globals.APPLICATION_LIST[num]["appmsg"].downloadcount + 1,Globals.APPLICATION_LIST[num]["appmsg"].screenshots_updatetime)
                self.slot_submit_down_over(name, Globals.APPLICATION_LIST[num]["appmsg"].app_id ,Globals.APPLICATION_LIST[num]["appmsg"].version)
                for i in self.down_app_msg_dict:
                    if (i["name"] == name):
                        self.down_app_msg_dict.remove(i)
                if (app.candidate_version):
                    app.installed_version = app.candidate_version
                if (not app.is_installed):
                    app.is_installed = True
                if (app.is_upgradable):
                    app.is_upgradable = False
                self.remove_card(self.detailwidget.mywidget.downloading_widget, name)
                self.slot_all_puase_btn_state()
                self.signal_installed.emit(name)
                self.change_global_info(num, PkgStates.INSTALLED, 0)
                if (len(self.wait_download_app_list) == 0 and len(self.wait_install_app_list) == 0):
                    self.check_btn_state()
                if (num != None):
                    self.change_global_info(num, PkgStates.NOTHING, 0, False)
                self.unlock_file(True, False)
                # self.check_has_app_need_download()
            elif (app and action == AppActions.REMOVE):
                self.remove_app_num = self.remove_app_num - 1
                installed_db = Record_History_Installation()
                installed_db.insert(name)
                num = self.rm_app_msg_and_dict(name)
                for installed_apk in Globals.INSTALLED_KYDROID_APK:
                    if (installed_apk["appname"] == name):
                        Globals.INSTALLED_KYDROID_APK.remove(installed_apk)
                        break
                # print(Globals.APPLICATION_LIST[num]["appmsg"])
                Globals.APPLICATION_LIST[num]["appmsg"].is_removed = True
                self.slot_submit_remove_over(name, Globals.APPLICATION_LIST[num]["appmsg"].app_id,Globals.APPLICATION_LIST[num]["appmsg"].version)
                self.change_search_remove_list(name)
                if (app.installed_version):
                    app.installed_version = ''
                if (app.is_installed):
                    app.is_installed = False
                elif (app.is_upgradable):
                    app.is_upgradable = False
                self.change_global_info(num, PkgStates.REMOVED, 0)
                self.unlock_file(False, True)

        elif int(percent) < 0:
            if app is not None:
                app.percent = 0
            if(action == AppActions.REMOVE):
                self.remove_app_num = self.remove_app_num - 1
                if(Globals.LAN == "zh_CN"):
                    if(num != None):
                        self.show_tip_widget(Globals.APPLICATION_LIST[num]["appmsg"].displayname_cn + " " +_("Uninstall failed, service exception. Please click the error code to see the solution."), True, False,
                                             "(" + _("Error Code") + "#0004)")
                    else:
                        self.show_tip_widget(name + " " +_("Uninstall failed, service exception. Please click the error code to see the solution."), True, False,
                                             "(" + _("Error Code") + "#0004)")
                else:
                    self.show_tip_widget(name + " " +_("Uninstall failed, service exception. Please click the error code to see the solution."), True, False,
                                     "(" + _("Error Code") + "#0004)")
                self.unlock_file(False, True)
            if int(percent) == int(-1):
                # self.signal_reload_btn.emit(name)
                self.change_global_info(num, PkgStates.NOTHING, 0)
                self.remove_card(self.detailwidget.mywidget.downloading_widget, name)
            elif int(percent) == int(-2):
                # self.signal_reload_btn.emit(name)
                self.change_global_info(num, PkgStates.NOTHING, 0)
                self.remove_card(self.detailwidget.mywidget.downloading_widget, name)
            elif int(percent) == int(-3):
                # self.messageBox.alert_msg("权限认证失败")
                # self.signal_reload_btn.emit(name)
                self.change_global_info(num, PkgStates.NOTHING, 0)
                self.remove_card(self.detailwidget.mywidget.downloading_widget, name)
            elif int(percent) == int(-11):
                # self.messageBox.alert_msg("卸载软件出错:")
                # self.signal_reload_btn.emit(name)
                self.change_global_info(num, PkgStates.NOTHING, 0)
                self.remove_card(self.detailwidget.mywidget.downloading_widget, name)
            elif int(percent) == int(-20):
                # self.signal_reload_btn.emit(name)
                self.change_global_info(num, PkgStates.NOTHING, 0)
                self.remove_card(self.detailwidget.mywidget.downloading_widget, name)
            else:
                self.signal_reload_btn.emit(name)
                self.remove_card(self.detailwidget.mywidget.downloading_widget, name)
            if (action == AppActions.INSTALL):
                apk_list = self.need_down_file_list[name]
                try:
                    for i in apk_list:
                        os.remove(KYDROID_DOWNLOAD_PATH + "/" + i)
                except:
                    pass
                if (Globals.LAN == "zh_CN"):
                    if (num != None):
                        self.tip_install_error(Globals.APPLICATION_LIST[num]["appmsg"].displayname_cn)
                    else:
                        self.tip_install_error(name)
                else:
                    self.tip_install_error(name)
                # self.tip_install_error()
                self.unlock_file(True, False)
        else:
            if(action == AppActions.REMOVE):
                if int(percent) == int(3):
                    self.remove_app_num = self.remove_app_num - 1
                    if (Globals.LAN == "zh_CN"):
                        if (num != None):
                            self.show_tip_widget(Globals.APPLICATION_LIST[num]["appmsg"].displayname_cn + " " + _("The software has been uninstalled from other sources!"), True)
                        else:
                            self.show_tip_widget(name + " " + _("The software has been uninstalled from other sources!"), True)
                    else:
                        self.show_tip_widget(name + " " + _("The software has been uninstalled from other sources!"), True)
                    self.unlock_file(False, True)
                    self.change_global_info(num, PkgStates.NOTHING, 0)
                    self.remove_card(self.detailwidget.mywidget.downloading_widget, name)
                elif(int(percent) != 50):
                    self.remove_app_num = self.remove_app_num - 1
                    if (Globals.LAN == "zh_CN"):
                        if (num != None):
                            self.show_tip_widget(Globals.APPLICATION_LIST[num]["appmsg"].displayname_cn + " " + _(
                                "Uninstall failed, service exception. Please click the error code to see the solution."), True, False,
                                                 "(" + _("Error Code") + "#0004)")
                        else:
                            self.show_tip_widget(name + " " + _("Uninstall failed, service exception. Please click the error code to see the solution."), True, False,
                                                 "(" + _("Error Code") + "#0004)")
                    else:
                        self.show_tip_widget(name + " " + _("Uninstall failed, service exception. Please click the error code to see the solution."), True, False,
                                             "(" + _("Error Code") + "#0004)")
                    self.unlock_file(False, True)
                    self.change_global_info(num, PkgStates.NOTHING, 0)
                    self.remove_card(self.detailwidget.mywidget.downloading_widget, name)
            # if app is not None:
            #     app.percent = percent
            #     self.send_download_progress.emit(name,percent)


    #
    #函数：仅退出软件商店
    #
    def just_restart_ksc(self):
        run_iface = Run_Dbus_Service()
        run_iface.call_run_ksc_app("kylin-software-center restart &")
        try:
            os.system("pkill -f -9 kylin-software-center/backend/run_app/start_run_dbus.py")
        except:
            pass
        sys.exit(0)

    #
    # 函数名: 重启软件商店
    # Function:restart data show
    #
    def restart_software_now(self):

        # self.dbusControler.stop()
        # os.system("kylin-software-center restart &")
        # self.unlock_uksc_lockfile()
        try:
            self.init_dbus_thread.backend.clear_dbus_worklist()
            self.init_dbus_thread.backend.exit_uksc_apt_daemon()
        except Exception as e:
            if (Globals.DEBUG_SWITCH):
                # print((str(e)))
                Globals.LOG_INFO.logger.info(_(str(e)))

        self.ask_thread.run_iface.call_run_ksc_app("kylin-software-center restart &")
        try:
            os.system("pkill -f -9 kylin-software-center/backend/run_app/start_run_dbus.py")
        except:
            pass
        sys.exit(0)

    #
    # 函数：处理暂停软件下载
    #
    def slot_puase_download_pkg(self, appmsg):
        if (self.detailwidget.mywidget.all_check.text() == _("all update")):
            self.detailwidget.mywidget.all_check.setEnabled(True)
        num = 0
        in_list = False
        for i in self.down_app_msg_dict:
            if i["name"] == appmsg.pkgname:
                self.down_app_msg_dict[num]["is_installing"] = False
                self.down_app_msg_dict[num]["is_downloading"] = False
                self.down_app_msg_dict[num]["is_puase"] = True
                self.down_app_msg_dict[num]["install_finish"] = False
                in_list = True
            num = num + 1
        in_wait_list = False
        for app in self.wait_download_app_list:
            if (appmsg.pkgname == app.pkgname):
                self.wait_download_app_list.remove(app)
                in_wait_list = True
                break
        if (in_wait_list == False):
            try:
                self.down_load_progress.terminate()
            except:
                pass
        in_dict = False
        for i in self.down_app_msg_dict:
            if (i["name"] == appmsg.pkgname):
                in_dict = True
                self.down_app_msg_dict.remove(i)
                if (appmsg in self.wait_download_app_list):
                    self.wait_download_app_list.remove(appmsg)
        if (in_dict == False):
            if (appmsg in self.wait_download_app_list):
                self.wait_download_app_list.remove(appmsg.pkgname)
        this_pkg_num = 0
        for i in Globals.APPLICATION_LIST:
            if (i["name"] == appmsg.pkgname):
                break
            this_pkg_num = this_pkg_num + 1
        if (this_pkg_num < len(Globals.APPLICATION_LIST)):
            self.change_global_info(this_pkg_num, PkgStates.PUASE, 0)
        self.slot_all_puase_btn_state()
        # self.sigenal_show_downloading_widget.emit(appmsg.pkgname,False,True)
        self.check_has_app_need_download()
        pass

    #
    # 函数：继续下载
    #
    def re_download_app(self, app_msg):
        num = 0
        for i in Globals.APPLICATION_LIST:
            if (i["name"] == app_msg.pkgname):
                break
            num = num + 1
        res = self.init_dbus_thread.backend.call_auth_with_policykit(_("install app or update app or remove app"),
                                                                     "com.kylin.softwarecenter.install")
        is_ksc, is_kum, is_ki, read_list,is_start_remote = self.check_lock_file()
        if (is_ksc == True):
            self.fd = open(Globals.APT_LOCK_FILE, "w")
            try:
                fcntl.flock(self.fd.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)
                self.fd.write(
                    Globals.UID + "\n" + "kylin-software-center\n" + "install\n" + time.strftime("%Y-%m-%d %H:%M:%SZ",
                                                                                                 time.localtime()) + "\n\n")
            except:
                pass
            self.slot_install_app(app_msg, num, res)
            self.slot_all_puase_btn_state()
        else:
            self.fd = open(Globals.APT_LOCK_FILE, "w")
            try:
                fcntl.flock(self.fd.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)
                self.fd.write(
                    Globals.UID + "\n" + "kylin-software-center\n" + "install\n" + time.strftime("%Y-%m-%d %H:%M:%SZ",
                                                                                                 time.localtime()) + "\n\n")
                # fd.close()
                self.slot_install_app(app_msg, num, res)
                self.slot_all_puase_btn_state()
            except:
                for i in read_list:
                    self.fd.write(i)
                self.fd.close()
                self.read_which_app_using(is_kum, is_ki)
        # self.slot_all_puase_btn_state()

    #
    # 函数：处理取消按钮
    #
    def slot_click_cancel_btn(self, card, appmsg):
        if (hasattr(card, "need_send_exposure_rate")):
            if (card.need_send_exposure_rate == True):
                for i in Globals.AD_EXPOSURE_RATE.keys():
                    if (str(Globals.AD_EXPOSURE_RATE[i]["appid"]) == str(appmsg.app_id)):
                        if (Globals.RETURN_PATH_LIST[len(Globals.RETURN_PATH_LIST) - 2][
                            "pagetype"] == PageStates.HOMEPAGE):
                            down_count = Globals.AD_EXPOSURE_RATE[i]["down_count"] - 1
                            if(down_count < 0):
                                down_count = 0
                            Globals.AD_EXPOSURE_RATE[i]["down_count"] = down_count
                            break
                        elif (Globals.RETURN_PATH_LIST[len(Globals.RETURN_PATH_LIST) - 2][
                                  "pagetype"] == PageStates.CAREGORYPAGE):
                            down_count = Globals.AD_EXPOSURE_RATE[i]["down_count"] - 1
                            if (down_count < 0):
                                down_count = 0
                            Globals.AD_EXPOSURE_RATE[i]["down_count"] = down_count
                            break
                if (Globals.RETURN_PATH_LIST[len(Globals.RETURN_PATH_LIST) - 2]["pagetype"] == PageStates.HOMEPAGE):
                    if (str(appmsg.app_id) in Globals.NEW_ARRIVALS_EXPOSURE_RATE.keys()):
                        down_count = Globals.NEW_ARRIVALS_EXPOSURE_RATE[str(appmsg.app_id)]["down_count"] - 1
                        if (down_count < 0):
                            down_count = 0
                        Globals.NEW_ARRIVALS_EXPOSURE_RATE[str(appmsg.app_id)]["down_count"] = down_count
        in_wait_list = False
        for app in self.wait_download_app_list:
            if (appmsg.pkgname == app.pkgname):
                self.wait_download_app_list.remove(app)
                in_wait_list = True
                break
        in_app_msg_dict = False
        for i in self.down_app_msg_dict:
            if (i["name"] == appmsg.pkgname):
                in_app_msg_dict = True
                self.down_app_msg_dict.remove(i)
                break
        self.remove_cache(appmsg.pkgname)
        if (in_wait_list == False and in_app_msg_dict == True):
            try:
                self.down_load_progress.terminate()
            except:
                pass
        if (hasattr(self.detailwidget, "app_detail_widget")):
            self.detailwidget.app_detail_widget.pre_progress = 0

        self.remove_card_signal.emit(self.detailwidget.mywidget.downloading_widget, appmsg.pkgname)
        self.slot_all_puase_btn_state()
        this_pkg_num = 0
        for i in Globals.APPLICATION_LIST:
            if (i["name"] == appmsg.pkgname):
                break
            this_pkg_num = this_pkg_num + 1
        self.change_global_info(this_pkg_num, PkgStates.NOTHING, 0)
        is_downloading = False
        for i in self.down_app_msg_dict:
            if (i["name"] == appmsg.pkgname):
                in_dict = True
                self.down_app_msg_dict.remove(i)
                if (appmsg.pkgname in self.wait_download_app_list):
                    self.wait_download_app_list.remove(appmsg.pkgname)
        self.check_has_app_need_download()

    #
    # 函数：清除缓存
    #
    def remove_cache(self, appname):
        if (appname in self.need_down_file_list.keys()):
            for i in self.need_down_file_list[appname]:
                if (os.path.exists(STORE_DEB_PATH + "/" + i)):
                    os.remove(STORE_DEB_PATH + "/" + i)
                elif (os.path.exists(WGET_DOWN_PATH + "/" + i)):
                    os.remove(WGET_DOWN_PATH + "/" + i)
                else:
                    pass
        try:
            del self.need_down_file_list[appname]
        except:
            pass


    #
    #函数：处理卡片初始化结束
    #
    def slot_init_insidecard_over(self,card_widget):
        self.send_download_progress.connect(card_widget.slot_get_download_progress)
        self.change_kydroid_card_btn_state.connect(card_widget.change_kydroid_btn_statue)
        self.signal_reload_btn.connect(card_widget.reload_btn)
        self.signal_uninstall.connect(card_widget.slot_uninstall)
        self.signal_installed.connect(card_widget.slot_installed_over)
        self.change_card_state.connect(card_widget.change_btn_state)
        self.set_dark_mode.connect(card_widget.slot_set_dark_mode)
        card_widget.check_all_check_statue.connect(self.slot_all_puase_btn_state)
        card_widget.puase_download.connect(self.slot_puase_download_pkg)
        card_widget.puase_apk_download.connect(self.slot_puase_download_pkg)
        for i in self.down_app_msg_dict:
            if (i["name"] == card_widget.app.pkgname):

                if (card_widget.app.status == PkgStates.NOTHING):
                    self.down_app_msg_dict.remove(i)
                    break
                if (i["is_downloading"] == True):
                    card_widget.btn.hide()
                    card_widget.cancel_btn.show()
                    card_widget.down_progress.show()
                elif (i["is_installing"] == True):
                    card_widget.cancel_btn.hide()
                    card_widget.btn.setText(_("installing"))
                    card_widget.btn.show()
                    card_widget.down_progress.hide()
                elif (i["is_puase"] == True):
                    pass
                break
        if (Globals.NOWPAGE == PageStates.MYPAGE or Globals.NOWPAGE == PageStates.HISTORY):
            if (self.detailwidget.mywidget.all_select_btn.text() == _("Deselect")):
                if (card_widget.btn.text() != _("Open") and card_widget.check_btn.isEnabled() == True and card_widget.btn.text() != _("installing") and card_widget.btn.text() != _("installed") and card_widget.btn.text() != _("start env")):
                    card_widget.check_btn.setChecked(True)
                    card_widget.check_btn.show()
        else:
            check_widget = None
            if (Globals.NOWPAGE == PageStates.CAREGORYPAGE):
                if(hasattr(self.detailwidget,"category_widget")):
                    check_widget = self.detailwidget.category_widget
            elif (Globals.NOWPAGE == PageStates.KYDROID):
                if(hasattr(self.detailwidget,"kydroid_widget")):
                    check_widget = self.detailwidget.kydroid_widget
            elif (Globals.NOWPAGE == PageStates.DRIVE):
                if (hasattr(self.detailwidget, "drive_widget")):
                    check_widget = self.detailwidget.drive_widget
            if (check_widget != None and check_widget.all_check_btn.text() == _("Deselect")):
                if (card_widget.btn.text() != _("Open") and card_widget.check_btn.isEnabled() == True and card_widget.btn.text() != _("installing") and card_widget.btn.text() != _("installed") and card_widget.btn.text() != _("start env")):
                    card_widget.check_btn.setChecked(True)
                    card_widget.check_btn.show()

    #
    #函数：处理inside_card列表
    #
    def slot_card_list(self,category, applist, scrollArea_widget, widget, is_remove_app, need_change_app_list=True,is_notice = False):
        QApplication.processEvents()
        self.get_inside_list_over = False
        #确保进行界面缩放时触发滚动条到底部的信号，导致卡片的排序错误
        if (len(widget.children()) < widget.children_count):
            return
        if (applist == []):
            applist = self.app_list_msg
        if (hasattr(self.detailwidget, "kydroid_widget") and widget == self.detailwidget.kydroid_widget.app_widget):
            if (len(self.pkg_list_msg) == 0):
                self.get_inside_list_over = True
                return
        else:
            if (len(applist) == 0):
                self.get_inside_list_over = True
                return
        app_msg_list = []
        # 防止在v10上直接传的是列表地址
        get_app_msg = []
        get_app_msg.extend(applist)
        package_is_none = False
        width = widget.width()
        cards = widget.children()
        count = len(cards)
        if (widget == self.detailwidget.show_more_app_widget):
            if (self.detailwidget.more_widget_title_label.text() == _("Down Rank")):
                if (count >= Globals.DISPLAY_DOWNLOAD_RANK_NUMBER):
                    return
        h_num = int(widget.width() / (self.detailwidget.card_width + self.detailwidget.card_space))
        get_card_width = int((widget.width() - (self.detailwidget.card_space * (h_num - 1)) - 20) / h_num)
        get_card_height = self.detailwidget.card_height
        # h_num = int((widget.width() - 10) / 248)
        # space = int(((widget.width() - 10) % 248) / (h_num - 1))
        v_num = int(count / h_num)
        # v_num = int(widget.children_count / h_num)
        num = count % h_num
        # num = widget.children_count % h_num
        n = 0
        while (len(get_app_msg) != 0):
            item = get_app_msg.pop(0)
            if (type(item) != ApkInfo and "100" in item["categories"].split(",")):
                is_notice = True
            else:
                is_notice = False
            if (hasattr(self.detailwidget, "kydroid_widget") and widget == self.detailwidget.kydroid_widget.app_widget):
                if (item in self.pkg_list_msg):
                    self.pkg_list_msg.remove(item)
            else:
                if (item in get_app_msg):
                    get_app_msg.remove(item)
                if (item in self.app_list_msg):
                    self.app_list_msg.remove(item)
            if (type(item) == ApkInfo):
                is_kydroid = True
            else:
                is_kydroid = False
            if (is_kydroid == False):
                if (is_remove_app):
                    if(is_notice == False):
                        try:
                            get_app = self.detailwidget.ca[item["name"]]
                            if (get_app.candidate != None and get_app.is_installed == True):
                                app_msg = self.get_appmsg(item,is_notice)
                            else:
                                app_msg = None
                        except:
                            app_msg = None
                    else:
                        app_msg = self.get_appmsg(item, is_notice)
                else:
                    if (is_notice == False):
                        try:
                            get_app = self.detailwidget.ca[item["name"]]
                            app_msg = self.get_appmsg(item)
                        except Exception as e:
                            continue
                    else:
                        app_msg = self.get_appmsg(item, is_notice)
            else:
                app_msg = self.get_apkmsg(item)
            if (app_msg != None):
                if (widget == self.detailwidget.show_more_app_widget):
                    if (self.detailwidget.more_widget_title_label.text() == _("Down Rank")):
                        if (count + len(app_msg_list) >= Globals.DISPLAY_DOWNLOAD_RANK_NUMBER):
                            break
                app_msg_list.append(app_msg)
            if (len(app_msg_list) >= h_num * 6):
                break
            # if(len(get_app_msg) == 0):
            #     break
        if (hasattr(self.detailwidget, "category_widget")):
            if (scrollArea_widget == self.detailwidget.category_widget.type_widget):
                Globals.FRIST_LEVEL_BTN_SCROLLAREA["CATEGORY"].clear()
                Globals.FRIST_LEVEL_BTN_SCROLLAREA["CATEGORY"].extend(get_app_msg)
        if (hasattr(self.detailwidget, "drive_widget")):
            if (scrollArea_widget == self.detailwidget.drive_widget.type_widget):
                Globals.FRIST_LEVEL_BTN_SCROLLAREA["DRIVE"].clear()
                Globals.FRIST_LEVEL_BTN_SCROLLAREA["DRIVE"].extend(get_app_msg)
        if (widget == self.detailwidget.mywidget.app_widget or widget == self.detailwidget.mywidget.history_widget.app_card_widget):
            Globals.FRIST_LEVEL_BTN_SCROLLAREA["MY"].clear()
            Globals.FRIST_LEVEL_BTN_SCROLLAREA["MY"].extend(get_app_msg)
        if (hasattr(self.detailwidget, "kydroid_widget") and widget == self.detailwidget.kydroid_widget.app_widget):
            pass
        else:
            if (need_change_app_list):
                self.app_list_msg.clear()
                self.app_list_msg.extend(get_app_msg)
        self.add_inside_card_widget = Globals.NOWPAGE
        self.show_inside_applist.emit(is_kydroid,category,get_app_msg, app_msg_list,applist, scrollArea_widget, widget, is_remove_app, need_change_app_list)

    #
    # 函数：向按钮内部界面中添加卡片(分类界面、驱动界面、安卓界面等添加卡片）
    #
    def add_card_to_inside_widget(self,is_kydroid,category,get_app_msg,app_msg_list, applist, scrollArea_widget, widget, is_remove_app, need_change_app_list=True):
        self.now_category = category
        h_num = int(widget.width() / (self.detailwidget.card_width + self.detailwidget.card_space))
        v_num = int(widget.children_count / h_num)
        num = widget.children_count % h_num
        get_card_width = int((widget.width() - (self.detailwidget.card_space * (h_num - 1))) / h_num)
        get_card_height = self.detailwidget.card_height
        self.card_width = get_card_width
        width = widget.width()
        sub_n = len(app_msg_list)
        # widget.children_count = sub_n + widget.children_count
        for i in range(sub_n):
            h_num = int(widget.width() / (self.detailwidget.card_width + self.detailwidget.card_space))
            v_num = int(len(widget.children()) / h_num)
            num = len(widget.children()) % h_num
            get_card_width = int((widget.width() - (self.detailwidget.card_space * (h_num - 1))) / h_num)
            get_card_height = self.detailwidget.card_height
            self.card_width = get_card_width
            width = widget.width()
            app_msg = app_msg_list[i]
            # try:

            package_is_none = False
            if(app_msg.is_notice == False):
                if (app_msg.package == None and type(app_msg) != ApkInfo):
                    package_is_none = True
            if (package_is_none == False):
                if(len(Globals.RETURN_PATH_LIST) >= 2):
                    if(self.add_inside_card_widget != Globals.NOWPAGE):
                        return
                card_widget = Inside_Card_Widget(category,get_card_width, get_card_height, app_msg, is_remove_app, self,
                                                 self.detailwidget, self.detailwidget.ca, widget)
                widget.children_count  = widget.children_count + 1
                # self.slot_init_insidecard_over(card_widget)
                if (num < h_num):
                    if (num == 0):
                        if (is_kydroid):
                            card_widget.move(0, 20 + (20 + card_widget.height()) * v_num)
                            widget.resize(width, 20 + (20 + card_widget.height()) * (v_num + 1))
                        else:
                            card_widget.move(0, 20 + (20 + card_widget.height()) * v_num)
                            widget.resize(width, 20 + (20 + card_widget.height()) * (v_num + 1))
                    else:
                        if (is_kydroid):
                            card_widget.move(num * (card_widget.width() + self.detailwidget.card_space) + 5,
                                             20 + (20 + card_widget.height()) * v_num)
                        else:
                            card_widget.move(num * (card_widget.width() + self.detailwidget.card_space) + 5,
                                             20 + (20 + card_widget.height()) * v_num)
                    num = num + 1
                else:
                    num = 0
                    v_num = v_num + 1
                    if (num == 0):
                        card_widget.move(0, 20 + (20 + card_widget.height()) * v_num)
                        widget.resize(width, 20 + (20 + card_widget.height()) * (v_num + 1))
                    else:
                        card_widget.move(num * (card_widget.width() + self.detailwidget.card_space) + 5,
                                         20 + (20 + card_widget.height()) * v_num)
                    num = num + 1
                # card_widget.btn.clicked.connect(lambda :self.slot_click_action_btn(card_widget.btn.text(),card_widget.app.APPNAME))
                if (is_remove_app == True):
                    # card_widget.btn.setText(_("uninstall"))
                    # card_widget.change_btn_width()
                    card_widget.set_btn_stylesheet(True)
            cards = widget.children()
            has_num = 0
            category_id = ""
            category_name = ""
            app_id_list = []
            if(Globals.NOWPAGE == PageStates.CAREGORYPAGE):
                for i in Globals.ALL_CATEGORIES:
                    if (i["displayname"] == self.detailwidget.category_widget.type_name_label.text() or i[
                        "name"] == self.detailwidget.category_widget.type_name_label.text()):
                        category_id = i["id"]
                        if (Globals.LAN == "zh_CN"):
                            category_name = i["displayname"]
                        else:
                            category_name = i["name"]
                if(category_name in Globals.ALL_CATEGORY_APP_LIST_BY_POPULARITY.keys()):
                    for i in Globals.ALL_CATEGORY_APP_LIST_BY_POPULARITY[category_name]:
                        if(i["id"] not in app_id_list):
                            app_id_list.append(i["id"])
            try:
                for card in cards:
                    if(card.x() == card_widget.x() and card.y() == card_widget.y()):
                        if(Globals.NOWPAGE == PageStates.CAREGORYPAGE):
                            if(str(category_id) != "1" and str(category_id) != "100"):
                                if(self.detailwidget.category_widget.type_name_label.text() != card.category_label.text()):
                                    sip.delete(card)
                                    if (widget.children_count > 0):
                                        widget.children_count = widget.children_count - 1
                            else:
                                if(self.now_category != card.category):
                                    sip.delete(card)
                                    if (widget.children_count > 0):
                                        widget.children_count = widget.children_count - 1
                                if(card.app.app_id not in app_id_list):
                                    sip.delete(card)
                                    if (widget.children_count > 0):
                                        widget.children_count = widget.children_count - 1
                                # else:
                                #     if(has_num == 0 ):
                                #         has_num = has_num + 1
                                #     else:
                                #         sip.delete(card)
                                # if(card != card_widget):
                                #     sip.delete(card)
                        elif(Globals.NOWPAGE == PageStates.KYDROID):
                            if(self.detailwidget.kydroid_widget.type_name_label.text() != _("全部") and self.detailwidget.kydroid_widget.type_name_label.text() != _("all")):
                                if (self.detailwidget.kydroid_widget.type_name_label.text() != card.category_label.text()):
                                    sip.delete(card)
                                    if(widget.children_count > 0):
                                        widget.children_count = widget.children_count - 1
                            else:
                                if (self.now_category != card.category):
                                    sip.delete(card)
                                    if (widget.children_count > 0):
                                        widget.children_count = widget.children_count - 1
                                # else:
                                #     if (has_num == 0):
                                #         has_num = has_num + 1
                                #     else:
                                #         sip.delete(card)
                        elif(Globals.NOWPAGE == PageStates.DRIVE):
                            if (self.detailwidget.drive_widget.type_name_label.text() != card.category_label.text()):
                                sip.delete(card)
                                if (widget.children_count > 0):
                                    widget.children_count = widget.children_count - 1
                        elif(Globals.NOWPAGE == PageStates.MYPAGE):
                            show_widget = Globals.RETURN_PATH_LIST[len(Globals.RETURN_PATH_LIST) - 1]
                            if(show_widget["Type"] == "update"):
                                if(card.is_remove_app == True):
                                    sip.delete(card)
                                    if (widget.children_count > 0):
                                        widget.children_count = widget.children_count - 1
                            else:
                                if (card.is_remove_app == False):
                                    sip.delete(card)
                                    if (widget.children_count > 0):
                                        widget.children_count = widget.children_count - 1
                    #删除重复卡片
                    try:
                        if(card != card_widget and card.app.pkgname == card_widget.app.pkgname):
                            sip.delete(card_widget)
                            if (widget.children_count > 0):
                                widget.children_count = widget.children_count - 1
                            break
                    except:
                        pass
            except:
                pass
            if (hasattr(self.detailwidget, "category_widget")):
                if (scrollArea_widget == self.detailwidget.category_widget.type_widget):
                    # Globals.FRIST_LEVEL_BTN_SCROLLAREA["CATEGORY"].clear()
                    # Globals.FRIST_LEVEL_BTN_SCROLLAREA["CATEGORY"].extend(get_app_msg)
                    scrollArea_widget.resize(scrollArea_widget.width(),self.detailwidget.category_widget.type_name_label.y() + self.detailwidget.category_widget.type_name_label.height() + widget.height())
            if (hasattr(self.detailwidget, "drive_widget")):
                if (scrollArea_widget == self.detailwidget.drive_widget.type_widget):
                    # Globals.FRIST_LEVEL_BTN_SCROLLAREA["DRIVE"].clear()
                    # Globals.FRIST_LEVEL_BTN_SCROLLAREA["DRIVE"].extend(get_app_msg)
                    scrollArea_widget.resize(scrollArea_widget.width(),
                                             self.detailwidget.drive_widget.type_name_label.y() + self.detailwidget.drive_widget.type_name_label.height() + widget.height()+30)
            self.title_bar.home_page_btn.setEnabled(True)
            self.title_bar.kydroid_btn.setEnabled(True)
            self.title_bar.drive_btn.setEnabled(True)
            self.title_bar.category_btn.setEnabled(True)
            self.title_bar.my_btn.setEnabled(True)
            self.title_bar.drive_btn.setEnabled(True)
            if (scrollArea_widget == self.detailwidget.mywidget.history_widget):
                if (self.detailwidget.mywidget.type_name_label.text() == _("Installation")):
                    if (len(self.detailwidget.mywidget.history_widget.app_card_widget.children()) != 0):
                        scrollArea_widget.setFixedHeight(self.detailwidget.mywidget.history_widget.label_local.y() + self.detailwidget.mywidget.history_widget.label_local.height() + widget.height() + 30)
                        scrollArea_widget.show()
                else:
                    self.detailwidget.mywidget.all_check.hide()
                    self.detailwidget.mywidget.all_select_btn.hide()
                    self.detailwidget.mywidget.history_widget.hide()
                    self.detailwidget.mywidget.scrollArea2.hide()
                    widget.hide()
                    scrollArea_widget.hide()
            if (hasattr(self.detailwidget,"kydroid_widget") and widget == self.detailwidget.kydroid_widget.app_widget):
                self.detailwidget.kydroid_widget.type_widget.resize(self.detailwidget.kydroid_widget.type_widget.width(),self.detailwidget.kydroid_widget.app_widget.y()+self.detailwidget.kydroid_widget.app_widget.height())
                if(widget == self.detailwidget.show_more_app_widget):
                    self.detailwidget.show_homepage_widget3.resize(self.detailwidget.show_homepage_widget3.width(),
                                                                 self.detailwidget.show_homepage_widget.height())
                    self.detailwidget.scrollArea3.resize(self.detailwidget.scrollArea3.width(),
                                                         self.detailwidget.height() - self.detailwidget.more_widget_line.y() - self.detailwidget.more_widget_line.height() - 30)
                if (Globals.NOWPAGE != PageStates.SEARCHPAGE):
                    self.detailwidget.search_widget_line.hide()
                    self.detailwidget.search_widget_line.hide()
                    self.detailwidget.show_homepage_widget2.hide()
                    self.detailwidget.scrollArea2.hide()
                    if (Globals.NOWPAGE == PageStates.HOMEPAGE):
                        self.detailwidget.show_homepage_widget.show()
                        self.detailwidget.scrollArea.show()
            if (Globals.NOWPAGE == PageStates.DRIVE):
                self.detailwidget.drive_widget.type_widget.resize(self.detailwidget.drive_widget.type_widget.width(),
                                                                  self.detailwidget.drive_widget.type_name_label.y() + self.detailwidget.drive_widget.type_name_label.height() + self.detailwidget.drive_widget.type_show_card_widget.height())

            # if (widget == self.detailwidget.mywidget.app_widget or widget == self.detailwidget.mywidget.history_widget.app_card_widget):
            #     # Globals.FRIST_LEVEL_BTN_SCROLLAREA["MY"].clear()
            #     # Globals.FRIST_LEVEL_BTN_SCROLLAREA["MY"].extend(get_app_msg)
            # if (hasattr(self.detailwidget,"kydroid_widget") and widget == self.detailwidget.kydroid_widget.app_widget):
            #     pass
            # else:
            #     if (need_change_app_list):
            #         self.app_list_msg.clear()
            #         self.app_list_msg.extend(get_app_msg)
            try:
                card_widget.show()
            except:
                pass
            widget.show()
            scrollArea_widget.show()
            QApplication.processEvents()
        widget.children_count = len(widget.children())
        if (widget == self.detailwidget.search_app_widget):
                self.title_bar.home_page_btn.setEnabled(True)
                self.title_bar.kydroid_btn.setEnabled(True)
                self.title_bar.drive_btn.setEnabled(True)
                self.title_bar.category_btn.setEnabled(True)
                self.title_bar.my_btn.setEnabled(True)
                self.title_bar.drive_btn.setEnabled(True)
                self.search_tip_widget.hide()
                if (len(widget.children()) == 0):
                    if (Globals.THEME_COLOR == 0):
                        self.detailwidget.no_app_tip_label.setStyleSheet(
                            "QLabel{border-radius: 12px;background-image:url(./res/no_search.png);}")
                    else:
                        self.detailwidget.no_app_tip_label.setStyleSheet(
                            "QLabel{border-radius: 12px;background-image:url(./res/no_search_dark.png);}")
                    self.detailwidget.text_prompt_label.setText(_("Sorry, we didn't find the app you want!"))
                    self.detailwidget.text_prompt_label.adjustSize()
                    self.detailwidget.no_app_tip_label.move(
                        int((self.detailwidget.width() - self.detailwidget.no_app_tip_label.width()) / 2), int((self.detailwidget.height() - self.detailwidget.no_app_tip_label.height() - self.detailwidget.text_prompt_label.height() + 10) / 2))
                    self.detailwidget.text_prompt_label.move(
                        int((self.detailwidget.width() - self.detailwidget.text_prompt_label.width()) / 2),
                        self.detailwidget.no_app_tip_label.y() + self.detailwidget.no_app_tip_label.height() + 10)
                    self.detailwidget.no_found_tip.move(
                        int((self.detailwidget.width() - self.detailwidget.no_found_tip.width()) / 2),
                        self.detailwidget.no_app_tip_label.y() + self.detailwidget.no_app_tip_label.height() + 10)
                    if (Globals.ADVANCED_SEARCH == False):
                        self.detailwidget.text_prompt_label.show()
                        self.detailwidget.no_found_tip.hide()
                    else:
                        self.detailwidget.no_found_tip.show()
                        self.detailwidget.text_prompt_label.hide()
                    self.detailwidget.no_app_tip_label.show()
                    self.detailwidget.show_homepage_widget2.resize(self.detailwidget.show_homepage_widget2.width(), 0)
                    self.detailwidget.show_homepage_widget2.hide()
                    self.detailwidget.scrollArea2.resize(self.detailwidget.scrollArea2.width(),
                                                         self.detailwidget.show_homepage_widget2.height())
                    self.detailwidget.scrollArea2.hide()
                else:
                    if (Globals.ADVANCED_SEARCH == True):
                        if (len(get_app_msg) == 0 and widget.children_count == len(widget.children())):
                            if (self.detailwidget.need_search_full_library):
                                self.init_all_search_btn()
                        if (self.detailwidget.search_app_widget.height() < self.detailwidget.scrollArea2.height()):
                            self.detailwidget.scrollArea2.resize(self.detailwidget.scrollArea2.width(),
                                                                 self.detailwidget.search_app_widget.height() + 5)
                            self.detailwidget.show_homepage_widget2.resize(self.detailwidget.show_homepage_widget2.width(),self.detailwidget.height())
                        else:
                            self.detailwidget.show_homepage_widget2.resize(self.detailwidget.show_homepage_widget2.width(),
                                                                           self.detailwidget.height())
                            self.detailwidget.scrollArea2.resize(self.detailwidget.scrollArea2.width(),
                                                                 self.detailwidget.height() - self.detailwidget.search_widget_line.y() - self.detailwidget.search_widget_line.height()- 30)
                    else:
                        self.detailwidget.show_homepage_widget2.resize(self.detailwidget.show_homepage_widget2.width(),
                                                                       self.detailwidget.height())
                        self.detailwidget.scrollArea2.resize(self.detailwidget.scrollArea2.width(),
                                                             self.detailwidget.height() - self.detailwidget.search_widget_line.y() - self.detailwidget.search_widget_line.height() - 30)
                    self.detailwidget.show_homepage_widget2.show()
                    self.detailwidget.scrollArea2.show()

        # if (Globals.ADVANCED_SEARCH == True):
        #     if (len(get_app_msg) == 0 and widget.children_count == len(widget.children())):
        #         if (self.detailwidget.need_search_full_library):
        #             self.init_all_search_btn()
        if(Globals.NOWPAGE == PageStates.MYPAGE and hasattr(self.detailwidget,"mywidget")):
            if(self.detailwidget.mywidget.app_widget.height() < self.detailwidget.mywidget.scrollArea.height() and self.detailwidget.mywidget.type_name_label.text() == category):
                self.slot_card_list(self.detailwidget.mywidget.type_name_label.text(),self.app_list_msg, self.detailwidget.mywidget.app_widget, self.detailwidget.mywidget.app_widget,is_remove_app, need_change_app_list)
        elif(Globals.NOWPAGE == PageStates.HISTORY and hasattr(self.detailwidget.mywidget,"history_widget")):
            if (self.detailwidget.mywidget.history_widget.app_card_widget.height() < self.detailwidget.mywidget.scrollArea2.height() and self.detailwidget.mywidget.type_name_label.text() == category):
                self.slot_card_list(self.detailwidget.mywidget.type_name_label.text(), self.app_list_msg,self.detailwidget.mywidget.history_widget,
                                    self.detailwidget.mywidget.history_widget.app_card_widget,
                                    is_remove_app, need_change_app_list)
        elif (Globals.NOWPAGE == PageStates.CAREGORYPAGE and hasattr(self.detailwidget,"category_widget")):
            if (self.detailwidget.category_widget.type_widget.height() < self.detailwidget.category_widget.scrollArea.height() and self.detailwidget.category_widget.type_name_label.text() == category):
                self.slot_card_list(self.detailwidget.category_widget.type_name_label.text(),get_app_msg, self.detailwidget.category_widget.type_widget,
                                                          self.detailwidget.category_widget.type_show_card_widget,
                                    is_remove_app, need_change_app_list)
        elif (Globals.NOWPAGE == PageStates.DRIVE and hasattr(self.detailwidget,"drive_widget")):
            if (self.detailwidget.drive_widget.type_widget.height() < self.detailwidget.drive_widget.scrollArea.height() and self.detailwidget.drive_widget.type_name_label.text() == category):
                self.slot_card_list(self.detailwidget.drive_widget.type_name_label.text(),self.app_list_msg, self.detailwidget.drive_widget.type_widget,
                                  self.detailwidget.drive_widget.type_show_card_widget,
                                    is_remove_app, need_change_app_list)
        elif (Globals.NOWPAGE == PageStates.KYDROID and hasattr(self.detailwidget,"kydroid_widget")):
            if (self.detailwidget.kydroid_widget.type_widget.height() < self.detailwidget.kydroid_widget.scrollArea.height() and self.detailwidget.kydroid_widget.type_name_label.text() == category):
                self.slot_card_list(self.detailwidget.kydroid_widget.type_name_label.text(),self.pkg_list_msg, self.detailwidget.kydroid_widget.app_widget,
                                  self.detailwidget.kydroid_widget.app_widget,
                                    is_remove_app, need_change_app_list)
            # except:
            #     pass

        self.get_inside_list_over = True

    #
    # 函数：初始化全库搜索按钮
    #
    def init_all_search_btn(self):
        if(hasattr(self.detailwidget.search_app_widget,"click_all_btn_widget") == False or self.init_all_search == True):
            self.init_all_search = False  #处理清搜索应用列表时将全库搜索这个按钮处delete
            self.detailwidget.search_app_widget.click_all_btn_widget = Click_All_Search(self, self.detailwidget.search_app_widget)
            self.detailwidget.search_app_widget.click_all_btn_widget.resize(self.detailwidget.search_app_widget.click_all_btn_widget.label2.width() + self.detailwidget.search_app_widget.click_all_btn_widget.label2.x() + 10, 60)
            self.detailwidget.search_app_widget.click_all_btn_widget.move(0, self.detailwidget.search_app_widget.height() + 10)
            self.detailwidget.search_app_widget.click_all_btn_widget.show()
            # self.no_found_text2.move(self.no_found_tip.width()-self.no_found_text2.width()-self.all_search_btn.width()-self.no_found_text3.width())
            self.detailwidget.search_app_widget.resize(self.detailwidget.search_app_widget.width(),self.detailwidget.search_app_widget.click_all_btn_widget.y() + self.detailwidget.search_app_widget.click_all_btn_widget.height())
            self.set_dark_mode.connect(self.detailwidget.search_app_widget.click_all_btn_widget.change_color)
        else:
            self.detailwidget.search_app_widget.click_all_btn_widget.move(0,self.detailwidget.search_app_widget.height() + 10)
            self.detailwidget.search_app_widget.resize(self.detailwidget.search_app_widget.width(),
                                                       self.detailwidget.search_app_widget.click_all_btn_widget.y() + self.detailwidget.search_app_widget.click_all_btn_widget.height())
            self.detailwidget.search_app_widget.click_all_btn_widget.show()

    #
    # 函数：向正在下载列表中添加卡片
    #
    def add_card_to_downloading_widget(self, app_msg, is_cancel=False, is_remove=False):
        num = 0
        app_dict = None
        if(type(app_msg) == ApkInfo):
            Globals.SEARCH_APP_LIST["DOWNLOADING"].append(app_msg)
        else:
            for i in Globals.GET_ALL_APP_LIST:
                if (i["name"] == app_msg.pkgname):
                    break
                num = num + 1
            if (num > len(Globals.GET_ALL_APP_LIST)-1):
                pass
            else:
                app_dict = Globals.GET_ALL_APP_LIST[num]
            if(app_dict != None and app_dict not in Globals.SEARCH_APP_LIST["DOWNLOADING"]):
                Globals.SEARCH_APP_LIST["DOWNLOADING"].append(app_dict)
        self.title_bar.my_btn_tip.show()
        # Globals.NOWPAGE = PageStates.DOWNLOADINGPAGE
        self.detailwidget.mywidget.scrollArea1.verticalScrollBar().setValue(0)
        cards = self.detailwidget.mywidget.downloading_widget.children()
        card_num = len(cards)
        h_num = int(self.detailwidget.mywidget.downloading_widget.width() / (
                    self.detailwidget.card_width + self.detailwidget.card_space))
        get_card_width = int((self.detailwidget.mywidget.downloading_widget.width() - (
                    self.detailwidget.card_space * (h_num - 1)) - 20) / h_num)
        get_card_height = self.detailwidget.card_height
        width = self.detailwidget.mywidget.downloading_widget.width()
        v_num = int(card_num / h_num)
        num = card_num % h_num
        card_widget = Inside_Card_Widget("",get_card_width, get_card_height, app_msg, is_remove, self, self.detailwidget,
                                         self.detailwidget.ca, self.detailwidget.mywidget.downloading_widget)
        self.set_dark_mode.connect(card_widget.slot_set_dark_mode)
        card_widget.check_all_check_statue.connect(self.slot_all_puase_btn_state)
        card_widget.puase_download.connect(self.slot_puase_download_pkg)
        card_widget.puase_apk_download.connect(self.slot_puase_download_pkg)
        self.send_download_progress.connect(card_widget.slot_get_download_progress)
        self.signal_reload_btn.connect(card_widget.reload_btn)
        self.signal_uninstall.connect(card_widget.slot_uninstall)
        self.signal_installed.connect(card_widget.slot_installed_over)
        self.change_card_state.connect(card_widget.change_btn_state)

        card_widget.check_btn.hide()
        card_widget.btn.hide()
        card_widget.down_progress.show()
        card_widget.cancel_btn.show()
        # card_widget.clicked_card.connect(lambda :self.detailwidget.slot_clicked_card(card_widget.app))
        if (num < h_num):
            if (num == 0):
                card_widget.move(5, 5 + (20 + card_widget.height()) * v_num)
                self.detailwidget.mywidget.downloading_widget.resize(width,
                                                                     5 + (20 + card_widget.height()) * (v_num + 1))
            else:
                card_widget.move(num * (card_widget.width() + self.detailwidget.card_space) + 5,
                                 5 + (20 + card_widget.height()) * v_num)
            num = num + 1
        else:
            num = 0
            v_num = v_num + 1
            if (num == 0):
                card_widget.move(5, 5 + (20 + card_widget.height()) * v_num)
                self.detailwidget.mywidget.downloading_widget.resize(width,
                                                                     5 + (20 + card_widget.height()) * (v_num + 1))
            else:
                card_widget.move(num * (card_widget.width() + self.detailwidget.card_space) + 5,
                                 5 + (20 + card_widget.height()) * v_num)
            num = num + 1
        card_num = len(self.detailwidget.mywidget.downloading_widget.children())
        if (card_num > 99):
            self.detailwidget.mywidget.downloading_num_label.setText("99+")
            self.detailwidget.mywidget.downloading_num_label.adjustSize()
            self.detailwidget.mywidget.downloading_num_label.resize(32, 18)
            self.detailwidget.mywidget.downloading_num_label.show()
        elif (card_num > 0 and card_num <= 99):
            self.detailwidget.mywidget.downloading_num_label.setText(str(card_num))
            self.detailwidget.mywidget.downloading_num_label.adjustSize()
            if (card_num < 10):
                self.detailwidget.mywidget.downloading_num_label.resize(18, 18)
            else:
                self.detailwidget.mywidget.downloading_num_label.resize(24, 18)
            self.detailwidget.mywidget.downloading_num_label.show()
        else:
            self.detailwidget.mywidget.downloading_num_label.hide()
        # self.detailwidget.mywidget.all_check.setEnabled(True)
        self.detailwidget.mywidget.download_all_check.setStyleSheet(
            "QPushButton{font-size:14px;font-weight: 500;color: #12C188;line-height: 20px;border:0px solid #ff0000;}QPushButton:hover{color: #12C188;}QPushButton:pressed{color: #12C188;}")
        self.detailwidget.mywidget.downloading_widget.show()

    #
    # 函数：点击我的界面的按钮
    #
    def slot_clicked_my_widget_btn(self, btn_name):
        if (hasattr(self, "title_bar")):
            self.title_bar.search_line.setFocus(False)
            self.title_bar.search_line.clearFocus()
        if (hasattr(self, "search_tip_widget")):
            self.search_tip_widget.hide()
        self.detailwidget.mywidget.no_app_tip_label.hide()
        self.detailwidget.mywidget.text_prompt_label.hide()
        self.detailwidget.mywidget.show()
        # self.detailwidget.mywidget.loadinglabel.show()
        self.clear_all_card(self.detailwidget.mywidget.app_widget)
        self.detailwidget.mywidget.all_check.hide()
        self.detailwidget.mywidget.download_all_check.hide()
        self.detailwidget.mywidget.all_select_btn.hide()
        self.detailwidget.mywidget.scrollArea.hide()
        self.detailwidget.show_homepage_widget2.hide()
        self.detailwidget.show_homepage_widget.hide()
        self.detailwidget.show_homepage_widget3.hide()
        self.detailwidget.scrollArea3.hide()
        self.detailwidget.search_widget_line.hide()
        self.detailwidget.search_title_label.hide()
        self.detailwidget.all_search_tip_label.hide()
        self.detailwidget.mywidget.scrollArea1.hide()
        self.detailwidget.mywidget.scrollArea2.hide()
        self.detailwidget.search_title_label.hide()
        self.detailwidget.all_search_tip_label.hide()
        self.detailwidget.search_widget_line.hide()
        self.detailwidget.scrollArea.hide()
        self.detailwidget.show_homepage_widget.hide()
        self.detailwidget.show_homepage_widget2.hide()
        self.detailwidget.scrollArea2.hide()
        if (Globals.THEME_COLOR == 0):
            self.detailwidget.mywidget.all_select_btn.setStyleSheet(
                "QPushButton{font-size: 14px;border:0px;font-weight: 500;color: #606265;line-height: 20px;}QPushButton:hover{background: rgba(55, 144, 250, 0.12);color: #3790FA;border-radius: 12px;}PushButton:pressed{background: rgba(55, 144, 250, 0.12);color: #3790FA;border-radius: 12px;}")
        else:
            self.detailwidget.mywidget.all_select_btn.setStyleSheet(
                "QPushButton{font-size: 14px;border:0px;font-weight: 500;color: #F9F9F9;line-height: 20px;}QPushButton:hover{background: rgba(55, 144, 250, 0.12);color: #3790FA;border-radius: 12px;}PushButton:pressed{background: rgba(55, 144, 250, 0.12);color: #3790FA;border-radius: 12px;}")
        self.detailwidget.mywidget.all_select_btn.setText(_("Select all"))
        if (btn_name == _("Installation")):
            if(self.detailwidget.mywidget.history_widget.now_btn_name != "local"):
                self.detailwidget.mywidget.loadinglabel.show()
            self.app_list_msg.clear()
            self.detailwidget.mywidget.downloading_btn.setEnabled(True)
            self.detailwidget.mywidget.uninstall_btn.setEnabled(True)
            self.detailwidget.mywidget.update_btn.setEnabled(True)
            self.detailwidget.mywidget.installed_btn.setEnabled(False)
            self.title_bar.search_line_back.setText(_("Please enter installation appname"))
            self.title_bar.search_line_back.adjustSize()
            Globals.NOWPAGE = PageStates.HISTORY
            Globals.SEARCH_NOWPAGE = PageStates.HISTORY
            page = {"pagetype": PageStates.HISTORY, "Type": self.detailwidget.mywidget.history_widget.now_btn_name}
            if (len(Globals.RETURN_PATH_LIST) >= 10):
                Globals.RETURN_PATH_LIST.pop(0)
            if (len(Globals.RETURN_PATH_LIST) >= 1 and Globals.RETURN_PATH_LIST[len(Globals.RETURN_PATH_LIST) - 1] != page):
                Globals.RETURN_PATH_LIST.append(page)
            self.clear_all_card(self.detailwidget.mywidget.history_widget.app_card_widget)
            # self.detailwidget.mywidget.scrollArea.verticalScrollBar().setValue(0)
            self.detailwidget.mywidget.scrollArea2.verticalScrollBar().setValue(0)
            self.detailwidget.mywidget.all_check.hide()
            self.detailwidget.mywidget.all_select_btn.hide()
            item = {}
            item["action"] = "get_history_list"
            item["type"] = self.detailwidget.mywidget.history_widget.now_btn_name
            self.at.mutex.acquire()
            self.at.work_list.append(item)
            self.at.mutex.release()
            if (self.detailwidget.mywidget.history_widget.now_btn_name == "local"):
                self.detailwidget.mywidget.history_widget.label_local.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 400;line-height: 20px;text-align: center;background-color:transparent;color: #3790FA;}")
                self.detailwidget.mywidget.history_widget.label_other.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 400;color: #8F9399;line-height: 20px;text-align: center;background-color:transparent;}QPushButton:hover{color: #3790FA;}QPushButton:pressed{color: #3790FA;}")
            else:
                self.detailwidget.mywidget.history_widget.label_other.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 400;line-height: 20px;text-align: center;background-color:transparent;color: #3790FA;}")
                self.detailwidget.mywidget.history_widget.label_local.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 400;color: #8F9399;line-height: 20px;text-align: center;background-color:transparent;}QPushButton:hover{color: #3790FA;}QPushButton:pressed{color: #3790FA;}")
            if (self.detailwidget.mywidget.update_num_label.text() != "" and int(self.detailwidget.mywidget.update_num_label.text()) > 0 and self.detailwidget.mywidget.type_name_label.text() != _("Updates")):
                # self.detailwidget.mywidget.update_tip_spot.show()
                self.detailwidget.mywidget.update_num_label.show()
            else:
                self.detailwidget.mywidget.update_num_label.hide()
            self.detailwidget.mywidget.all_check.setStyleSheet(
                "QPushButton{font-size:14px;font-weight: 500;color: #8f9399;line-height: 20px;border:0px solid #ff0000;}")
            self.detailwidget.mywidget.all_check.setEnabled(False)
            self.detailwidget.mywidget.all_check.setText(_("all down"))
            self.detailwidget.mywidget.all_check.adjustSize()
            self.detailwidget.mywidget.all_select_btn.move(
                self.detailwidget.mywidget.all_check.x() + 5 - self.detailwidget.mywidget.all_select_btn.width(),
                self.detailwidget.mywidget.all_check.y())
            self.detailwidget.mywidget.scrollArea.show()
            self.detailwidget.mywidget.downloading_widget.hide()
            self.detailwidget.mywidget.scrollArea1.hide()
        elif (btn_name == _("Updates")):
            self.title_bar.search_line_back.setText(_("Please enter update appname"))
            self.title_bar.search_line_back.adjustSize()
            Globals.NOWPAGE = PageStates.MYPAGE
            Globals.SEARCH_NOWPAGE = PageStates.MYPAGE
            page = {"pagetype": PageStates.MYPAGE, "Type": "update"}
            if (len(Globals.RETURN_PATH_LIST) >= 10):
                Globals.RETURN_PATH_LIST.pop(0)
            if (len(Globals.RETURN_PATH_LIST) >= 1 and Globals.RETURN_PATH_LIST[len(Globals.RETURN_PATH_LIST) - 1] != page):
                Globals.RETURN_PATH_LIST.append(page)
            self.detailwidget.mywidget.downloading_btn.setEnabled(True)
            self.detailwidget.mywidget.uninstall_btn.setEnabled(True)
            self.detailwidget.mywidget.update_btn.setEnabled(False)
            self.detailwidget.mywidget.installed_btn.setEnabled(True)
            self.detailwidget.mywidget.scrollArea.verticalScrollBar().setValue(0)
            item = {}
            item["action"] = "get_update_list"
            self.at.mutex.acquire()
            self.at.work_list.append(item)
            self.at.mutex.release()
            self.detailwidget.mywidget.all_check.hide()
            self.detailwidget.mywidget.all_select_btn.hide()
            self.detailwidget.mywidget.all_check.setStyleSheet(
                "QPushButton{font-size:14px;font-weight: 500;color: #8f9399;line-height: 20px;border:0px solid #ff0000;}")
            self.detailwidget.mywidget.all_check.setEnabled(False)
            self.detailwidget.mywidget.all_check.setText(_("all update"))
            self.detailwidget.mywidget.all_check.adjustSize()
            self.detailwidget.mywidget.all_select_btn.move(
                self.detailwidget.mywidget.all_check.x() + 5 - self.detailwidget.mywidget.all_select_btn.width(),
                self.detailwidget.mywidget.all_check.y())
            self.detailwidget.mywidget.scrollArea.show()
            self.detailwidget.mywidget.downloading_widget.hide()
            self.detailwidget.mywidget.scrollArea1.hide()
        elif (btn_name == _("Uninstall")):
            self.title_bar.search_line_back.setText(_("Please enter uninstall appname"))
            self.title_bar.search_line_back.adjustSize()
            Globals.NOWPAGE = PageStates.MYPAGE
            Globals.SEARCH_NOWPAGE = PageStates.MYPAGE
            page = {"pagetype": PageStates.MYPAGE, "Type": "uninstall"}
            if (len(Globals.RETURN_PATH_LIST) >= 10):
                Globals.RETURN_PATH_LIST.pop(0)
            if (len(Globals.RETURN_PATH_LIST) >= 1 and Globals.RETURN_PATH_LIST[len(Globals.RETURN_PATH_LIST) - 1] != page):
                Globals.RETURN_PATH_LIST.append(page)
            self.detailwidget.mywidget.downloading_btn.setEnabled(True)
            self.detailwidget.mywidget.uninstall_btn.setEnabled(False)
            self.detailwidget.mywidget.update_btn.setEnabled(True)
            self.detailwidget.mywidget.installed_btn.setEnabled(True)
            self.detailwidget.mywidget.scrollArea.verticalScrollBar().setValue(0)
            item = {}
            item["action"] = "get_uninstall_list"
            self.at.mutex.acquire()
            self.at.work_list.append(item)
            self.at.mutex.release()
            if (self.detailwidget.mywidget.update_num_label.text() != "" and int(self.detailwidget.mywidget.update_num_label.text()) > 0 and self.detailwidget.mywidget.type_name_label.text() != _("Updates")):
                # self.detailwidget.mywidget.update_tip_spot.show()
                self.detailwidget.mywidget.update_num_label.show()
            else:
                self.detailwidget.mywidget.update_num_label.hide()
            self.detailwidget.mywidget.all_check.hide()
            self.detailwidget.mywidget.all_select_btn.hide()
            self.detailwidget.mywidget.all_check.setStyleSheet(
                "QPushButton{font-size:14px;font-weight: 500;color: #8f9399;line-height: 20px;border:0px solid #ff0000;}")
            self.detailwidget.mywidget.all_check.setEnabled(False)
            self.detailwidget.mywidget.all_check.setText(_("all remove"))
            self.detailwidget.mywidget.all_check.adjustSize()
            self.detailwidget.mywidget.all_select_btn.move(
                self.detailwidget.mywidget.all_check.x() + 5 - self.detailwidget.mywidget.all_select_btn.width(),
                self.detailwidget.mywidget.all_check.y())
            self.detailwidget.mywidget.scrollArea.show()
            self.detailwidget.mywidget.downloading_widget.hide()
            self.detailwidget.mywidget.scrollArea1.hide()
        else:
            self.detailwidget.mywidget.loadinglabel.hide()
            self.title_bar.search_line_back.setText(_("Please enter onload appname"))
            self.title_bar.search_line_back.adjustSize()
            Globals.NOWPAGE = PageStates.DOWNLOADINGPAGE
            Globals.SEARCH_NOWPAGE = PageStates.DOWNLOADINGPAGE
            page = {"pagetype": PageStates.DOWNLOADINGPAGE}
            if (len(Globals.RETURN_PATH_LIST) >= 10):
                Globals.RETURN_PATH_LIST.pop(0)
            if (len(Globals.RETURN_PATH_LIST) >= 1 and Globals.RETURN_PATH_LIST[len(Globals.RETURN_PATH_LIST) - 1] != page):
                Globals.RETURN_PATH_LIST.append(page)
            self.detailwidget.mywidget.downloading_btn.setEnabled(False)
            self.detailwidget.mywidget.uninstall_btn.setEnabled(True)
            self.detailwidget.mywidget.update_btn.setEnabled(True)
            self.detailwidget.mywidget.installed_btn.setEnabled(True)
            self.detailwidget.mywidget.scrollArea1.verticalScrollBar().setValue(0)
            self.detailwidget.mywidget.all_check.hide()
            self.detailwidget.mywidget.all_select_btn.hide()
            # self.detailwidget.mywidget.download_all_check.setText(_("all puase"))
            if(self.detailwidget.mywidget.type_name_label.text() != _("Updates")):
                self.detailwidget.mywidget.update_num_label.hide()
            if (self.detailwidget.mywidget.update_num_label.text() != "" and int(self.detailwidget.mywidget.update_num_label.text()) > 0):
                # self.detailwidget.mywidget.update_tip_spot.show()
                self.detailwidget.mywidget.update_num_label.show()
            else:
                self.detailwidget.mywidget.update_num_label.hide()
            self.detailwidget.mywidget.download_all_check.show()
            self.detailwidget.mywidget.download_all_check.setEnabled(True)
            self.detailwidget.mywidget.download_all_check.setStyleSheet(
                "QPushButton{font-size:14px;font-weight: 500;color: #8f9399;line-height: 20px;border:0px solid #ff0000;}")
            self.detailwidget.mywidget.app_widget.hide()
            self.detailwidget.mywidget.scrollArea.hide()
            self.detailwidget.mywidget.scrollArea2.hide()
            self.detailwidget.mywidget.loadinglabel.hide()
            if (len(self.detailwidget.mywidget.downloading_widget.children()) == 0):
                if (Globals.THEME_COLOR == 0):
                    self.detailwidget.mywidget.no_app_tip_label.setStyleSheet(
                        "QLabel{border-radius: 12px;background-image:url(./res/no_download.png);}")
                else:
                    self.detailwidget.mywidget.no_app_tip_label.setStyleSheet(
                        "QLabel{border-radius: 12px;background-image:url(./res/no_download_dark.png);}")
                self.detailwidget.mywidget.text_prompt_label.setText(_("No software downloading!"))
                self.detailwidget.mywidget.download_all_check.setEnabled(False)
                self.detailwidget.mywidget.download_all_check.setStyleSheet(
                    "QPushButton{font-size:14px;font-weight: 500;color: #8f9399;line-height: 20px;border:0px solid #ff0000;}")
                self.detailwidget.mywidget.text_prompt_label.adjustSize()
                self.detailwidget.mywidget.no_app_tip_label.move(
                    int((self.detailwidget.mywidget.width() - self.detailwidget.mywidget.no_app_tip_label.width()) / 2),
                    int((
                                    self.detailwidget.mywidget.height() - self.detailwidget.mywidget.no_app_tip_label.height() - self.detailwidget.mywidget.text_prompt_label.height() + 10) / 2))
                self.detailwidget.mywidget.text_prompt_label.move(int(
                    (self.detailwidget.mywidget.width() - self.detailwidget.mywidget.text_prompt_label.width()) / 2),
                                                                  self.detailwidget.mywidget.no_app_tip_label.y() + self.detailwidget.mywidget.no_app_tip_label.height() + 10)
                self.detailwidget.mywidget.text_prompt_label.show()
                self.detailwidget.mywidget.no_app_tip_label.show()
                self.detailwidget.mywidget.download_all_check.hide()
                self.detailwidget.mywidget.scrollArea1.hide()
            else:
                self.detailwidget.mywidget.download_all_check.setEnabled(True)
                self.detailwidget.mywidget.download_all_check.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;border:0px solid #ff0000;}QPushButton:hover{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}QPushButton:pressed{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}")
                self.detailwidget.mywidget.no_app_tip_label.hide()
                self.detailwidget.mywidget.border.show()
                self.detailwidget.mywidget.type_name_label.show()
                self.detailwidget.mywidget.download_all_check.show()
                self.detailwidget.mywidget.downloading_widget.show()
                self.detailwidget.mywidget.scrollArea1.show()

    #
    # 函数：点击checkbutton
    #
    def slot_clicked_check_btn(self, card):
        card.check_btn.setStyleSheet(
            "QCheckBox{border-radius: 8px;spacing: 0px;color:#666666;font-size:14px;background-color:rgba(0,0,0,0);border: 0px solid #8F9399;}" \
            "QCheckBox:hover{background-color:rgba(0,0,0,0);border-radius: 8px;image: url(./res/circular_uncheck.png);}" \
            "QCheckBox::indicator{width: 16px;height: 16px;border-radius:8px;}" \
            "QCheckBox::indicator:unchecked{background-color:rgba(0,0,0,0);border:0px solid #bfbfbf;image: url(./res/circular_uncheck.png);}" \
            "QCheckBox::indicator:unchecked:hover{background-color:rgba(0,0,0,0);border: 0px solid #8F9399;image: url(./res/circular_uncheck.png);}" \
            "QCheckBox::indicator:unchecked:pressed{background-color:rgba(0,0,0,0);border:0px solid #415FC4;image: url(./res/circular_uncheck.png);}" \
            "QCheckBox::indicator{width: 16px;height: 16px;}" \
            "QCheckBox::indicator:checked{border-radius: 8px;background-color: rgba(0,0,0,0);border:0px solid #415FC4;image: url(./res/circular_check.png);}" \
            "QCheckBox::indicator:checked:hover{border-radius: 8px;background-color:rgba(0,0,0,0);border:0px solid #415FC4;image: url(./res/circular_check.png);}" \
            "QCheckBox::indicator:checked:pressed{border-radius: 8px;background-color:rgba(0,0,0,0);border:0px solid #415FC4;image: url(./res/circular_check.png);}")
        self.check_btn_state()
        if(Globals.NOWPAGE == PageStates.CAREGORYPAGE or Globals.NOWPAGE == PageStates.KYDROID or Globals.NOWPAGE == PageStates.DRIVE):
            self.check_category_btn_statue()

    def check_btn_state(self):
        all_check = True
        has_check = False
        if (Globals.NOWPAGE == PageStates.HISTORY):
            cards = self.detailwidget.mywidget.history_widget.app_card_widget.children()
        else:
            cards = self.detailwidget.mywidget.app_widget.children()
        for card in cards:
            if (card.check_btn.isChecked() == False):
                all_check = False
            else:
                if (card.check_btn.isEnabled() == True):
                    has_check = True
        if (all_check == True):
            self.detailwidget.mywidget.all_select_btn.setText(_("Deselect"))
            self.detailwidget.mywidget.all_select_btn.adjustSize()
            self.detailwidget.mywidget.all_check.setEnabled(True)
            if (self.detailwidget.mywidget.type_name_label.text() == _("Uninstall")):
                self.detailwidget.mywidget.all_check.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 500;color: #F56C6C;line-height: 20px;border:0px solid #ff0000;}QPushButton:hover{background: rgba(245, 108, 108, 0.12);border-radius: 12px;}QPushButton:pressed{background: rgba(245, 108, 108, 0.12);border-radius: 12px;}")
            else:
                self.detailwidget.mywidget.all_check.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;border:0px solid #ff0000;}QPushButton:hover{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}QPushButton:pressed{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}")
        else:
            self.detailwidget.mywidget.all_select_btn.setText(_("Select all"))
            self.detailwidget.mywidget.all_select_btn.adjustSize()
            if (has_check == True):
                self.detailwidget.mywidget.all_check.setEnabled(True)
                if (self.detailwidget.mywidget.type_name_label.text() == _("Uninstall")):
                    self.detailwidget.mywidget.all_check.setStyleSheet(
                        "QPushButton{font-size: 14px;font-weight: 500;color: #F56C6C;line-height: 20px;border:0px solid #ff0000;}QPushButton:hover{background: rgba(245, 108, 108, 0.12);border-radius: 12px;}QPushButton:pressed{background: rgba(245, 108, 108, 0.12);border-radius: 12px;}")
                else:
                    self.detailwidget.mywidget.all_check.setStyleSheet(
                        "QPushButton{font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;border:0px solid #ff0000;}QPushButton:hover{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}QPushButton:pressed{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}")
            else:
                self.detailwidget.mywidget.all_select_btn.setEnabled(True)
                self.detailwidget.mywidget.all_check.setEnabled(False)
                self.detailwidget.mywidget.all_check.setStyleSheet(
                    "QPushButton{font-size:14px;font-weight: 500;color: #8f9399;line-height: 20px;border:0px solid #ff0000;}")

    #
    # 函数：点击全选按钮
    #
    def slot_click_all_check_btn(self):
        if (self.detailwidget.mywidget.all_select_btn.text() == _("Select all")):
            if (Globals.NOWPAGE == PageStates.HISTORY or Globals.NOWPAGE == PageStates.MYPAGE):
                if (self.detailwidget.mywidget.type_name_label.text() == _("Installation")):
                    cards = self.detailwidget.mywidget.history_widget.app_card_widget.children()
                else:
                    cards = self.detailwidget.mywidget.app_widget.children()
            else:
                cards = self.detailwidget.mywidget.app_widget.children()
            checked_num = 0
            for card in cards:
                if (card.btn.text() != _("Open") and card.check_btn.isEnabled() == True and card.btn.text() != _("installing") and card.btn.text() != _("installed") and card.cancel_btn.isHidden() and card.btn.text() != _("start env") and card.btn.isHidden() == False):
                    card.check_btn.setStyleSheet(
                        "QCheckBox{border-radius: 8px;spacing: 0px;color:#666666;font-size:14px;background-color:rgba(0,0,0,0);border: 0px solid #8F9399;}" \
                        "QCheckBox:hover{background-color:rgba(0,0,0,0);border-radius: 8px;}" \
                        "QCheckBox::indicator{width: 16px;height: 16px;border-radius:8px;}" \
                        "QCheckBox::indicator:unchecked{background-color:rgba(0,0,0,0);border:0px solid #bfbfbf;}" \
                        "QCheckBox::indicator:unchecked:hover{background-color:rgba(0,0,0,0);border: 0px solid #8F9399;}" \
                        "QCheckBox::indicator:unchecked:pressed{background-color:rgba(0,0,0,0);border:0px solid #415FC4;}" \
                        "QCheckBox::indicator{width: 16px;height: 16px;}" \
                        "QCheckBox::indicator:checked{border-radius: 8px;background-color: rgba(0,0,0,0);border:0px solid #415FC4;image: url(./res/circular_check.png);}" \
                        "QCheckBox::indicator:checked:hover{border-radius: 8px;background-color:rgba(0,0,0,0);border:0px solid #415FC4;image: url(./res/circular_check.png);}" \
                        "QCheckBox::indicator:checked:pressed{border-radius: 8px;background-color:rgba(0,0,0,0);border:0px solid #415FC4;image: url(./res/circular_check.png);}")
                    card.check_btn.setChecked(True)
                    checked_num = checked_num + 1
                    card.check_btn.show()
            if (checked_num > 0):
                self.detailwidget.mywidget.all_select_btn.setText(_("Deselect"))
                self.detailwidget.mywidget.all_check.setEnabled(True)
                if (self.detailwidget.mywidget.type_name_label.text() == _("Uninstall")):
                    self.detailwidget.mywidget.all_check.setStyleSheet(
                        "QPushButton{font-size: 14px;font-weight: 500;color: #F56C6C;line-height: 20px;border:0px solid #ff0000;}QPushButton:hover{background: rgba(245, 108, 108, 0.12);border-radius: 12px;}QPushButton:pressed{background: rgba(245, 108, 108, 0.12);border-radius: 12px;}")
                else:
                    self.detailwidget.mywidget.all_check.setStyleSheet(
                        "QPushButton{font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;border:0px solid #ff0000;}QPushButton:hover{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}QPushButton:pressed{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}")
            else:
                self.detailwidget.mywidget.all_check.setEnabled(False)
                self.detailwidget.mywidget.all_check.setStyleSheet(
                    "QPushButton{font-size:14px;font-weight: 500;color: #8f9399;line-height: 20px;border:0px solid #ff0000;}")
            self.detailwidget.mywidget.all_select_btn.adjustSize()
        elif (self.detailwidget.mywidget.all_select_btn.text() == _("Deselect")):
            self.detailwidget.mywidget.all_check.setEnabled(False)
            self.detailwidget.mywidget.all_check.setStyleSheet(
                "QPushButton{font-size:14px;font-weight: 500;color: #8f9399;line-height: 20px;border:0px solid #ff0000;}")
            if (Globals.NOWPAGE == PageStates.HISTORY):
                cards = self.detailwidget.mywidget.history_widget.app_card_widget.children()
            else:
                cards = self.detailwidget.mywidget.app_widget.children()
            for card in cards:
                if (card.check_btn.isEnabled() == True):
                    card.check_btn.setChecked(False)
                    if (card.btn.text() != _("Open") and card.check_btn.isEnabled() == True and card.btn.text() != _("installing") and card.btn.text() != _("installed") and card.btn.text() != _("start env") and card.cancel_btn.isHidden() and card.btn.isHidden() == False):
                        card.check_btn.show()
                    else:
                        card.check_btn.setEnabled(False)
                        card.check_btn.hide()
            self.detailwidget.mywidget.all_select_btn.setText(_("Select all"))
            self.detailwidget.mywidget.all_select_btn.adjustSize()

    #
    #函数：检测分类界面“一键下载”按钮状态
    #
    def check_category_btn_statue(self):
        all_check = True
        has_check = False
        if (Globals.NOWPAGE == PageStates.CAREGORYPAGE):
            widget = self.detailwidget.category_widget
            cards = self.detailwidget.category_widget.type_show_card_widget.children()
        elif(Globals.NOWPAGE == PageStates.KYDROID):
            widget = self.detailwidget.kydroid_widget
            cards = self.detailwidget.kydroid_widget.app_widget.children()
        elif (Globals.NOWPAGE == PageStates.DRIVE):
            widget = self.detailwidget.drive_widget
            cards = self.detailwidget.drive_widget.type_show_card_widget.children()
        else:
            widget = self.detailwidget.mywidget
            cards = self.detailwidget.mywidget.app_widget.children()
        for card in cards:
            if (card.check_btn.isChecked() == False):
                all_check = False
            else:
                if (card.check_btn.isEnabled() == True):
                    has_check = True
        if (all_check == True):
            widget.all_check_btn.setText(_("Deselect"))
            widget.all_check_btn.adjustSize()
            widget.one_click_download.setEnabled(True)
            widget.all_check_btn.setText(_("Deselect"))
            widget.one_click_download.setEnabled(True)
            if (Globals.THEME_COLOR == 0):
                widget.one_click_download.setStyleSheet(
                    "QPushButton{border:0px;font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;border:0px solid #ff0000;}QPushButton:hover{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}QPushButton:pressed{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}")
            else:
                widget.one_click_download.setStyleSheet(
                    "QPushButton{border:0px;font-size: 14px;font-weight: 500;color: #62666C;line-height: 20px;border:0px solid #ff0000;}QPushButton:hover{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}QPushButton:pressed{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}")
        else:
            widget.all_check_btn.setText(_("Select all"))
            widget.all_check_btn.adjustSize()
            if (has_check == True):
                widget.one_click_download.setEnabled(True)
                if (Globals.THEME_COLOR == 0):
                    widget.one_click_download.setStyleSheet(
                        "QPushButton{border:0px;font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;border:0px solid #ff0000;}QPushButton:hover{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}QPushButton:pressed{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}")
                else:
                    widget.one_click_download.setStyleSheet(
                        "QPushButton{border:0px;font-size: 14px;font-weight: 500;color: #62666C;line-height: 20px;border:0px solid #ff0000;}QPushButton:hover{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}QPushButton:pressed{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}")
            else:
                widget.all_check_btn.setEnabled(True)
                widget.one_click_download.setEnabled(False)
                widget.one_click_download.setStyleSheet("QPushButton{font-size:14px;font-weight: 500;color: #8f9399;line-height: 20px;border:0px solid #ff0000;}")


    #
    #函数：处理分类界面点击全选
    #
    def slot_category_all_select(self,widget):
        if(widget.all_check_btn.text() == _("Select all")):
            if (Globals.NOWPAGE == PageStates.CAREGORYPAGE):
                widget = self.detailwidget.category_widget
                cards = self.detailwidget.category_widget.type_show_card_widget.children()
            elif(Globals.NOWPAGE == PageStates.KYDROID):
                widget = self.detailwidget.kydroid_widget
                cards = self.detailwidget.kydroid_widget.app_widget.children()
            elif(Globals.NOWPAGE == PageStates.DRIVE):
                widget = self.detailwidget.drive_widget
                cards = self.detailwidget.drive_widget.type_show_card_widget.children()
            checked_num = 0
            for card in cards:
                if (card.btn.text() != _("Open") and card.check_btn.isEnabled() == True and card.btn.text() != _("installing") and card.btn.text() != _("installed") and card.cancel_btn.isHidden() and card.btn.text() != _("start env")):
                    card.check_btn.setStyleSheet(
                        "QCheckBox{border-radius: 8px;spacing: 0px;color:#666666;font-size:14px;background-color:rgba(0,0,0,0);border: 0px solid #8F9399;}" \
                        "QCheckBox:hover{background-color:rgba(0,0,0,0);border-radius: 8px;}" \
                        "QCheckBox::indicator{width: 16px;height: 16px;border-radius:8px;}" \
                        "QCheckBox::indicator:unchecked{background-color:rgba(0,0,0,0);border:0px solid #bfbfbf;}" \
                        "QCheckBox::indicator:unchecked:hover{background-color:rgba(0,0,0,0);border: 0px solid #8F9399;}" \
                        "QCheckBox::indicator:unchecked:pressed{background-color:rgba(0,0,0,0);border:0px solid #415FC4;}" \
                        "QCheckBox::indicator{width: 16px;height: 16px;}" \
                        "QCheckBox::indicator:checked{border-radius: 8px;background-color: rgba(0,0,0,0);border:0px solid #415FC4;image: url(./res/circular_check.png);}" \
                        "QCheckBox::indicator:checked:hover{border-radius: 8px;background-color:rgba(0,0,0,0);border:0px solid #415FC4;image: url(./res/circular_check.png);}" \
                        "QCheckBox::indicator:checked:pressed{border-radius: 8px;background-color:rgba(0,0,0,0);border:0px solid #415FC4;image: url(./res/circular_check.png);}")
                    card.check_btn.setChecked(True)
                    checked_num = checked_num + 1
                    card.check_btn.show()
            if (checked_num > 0):
                widget.all_check_btn.setText(_("Deselect"))
                widget.one_click_download.setEnabled(True)
                if(Globals.THEME_COLOR == 0):
                    widget.one_click_download.setStyleSheet("QPushButton{border:0px;font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;border:0px solid #ff0000;}QPushButton:hover{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}QPushButton:pressed{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}")
                else:
                    widget.one_click_download.setStyleSheet("QPushButton{border:0px;font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;border:0px solid #ff0000;}QPushButton:hover{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}QPushButton:pressed{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}")
            else:
                widget.one_click_download.setEnabled(False)
                widget.one_click_download.setStyleSheet(
                    "QPushButton{font-size:14px;font-weight: 500;color: #8f9399;line-height: 20px;border:0px solid #ff0000;}")
            widget.all_check_btn.adjustSize()
        elif (widget.all_check_btn.text() == _("Deselect")):
            widget.one_click_download.setEnabled(False)
            widget.one_click_download.setStyleSheet(
                "QPushButton{font-size:14px;font-weight: 500;color: #8f9399;line-height: 20px;border:0px solid #ff0000;}")
            if (Globals.NOWPAGE == PageStates.CAREGORYPAGE):
                cards = self.detailwidget.category_widget.type_show_card_widget.children()
            elif (Globals.NOWPAGE == PageStates.KYDROID):
                cards = self.detailwidget.kydroid_widget.app_widget.children()
            elif (Globals.NOWPAGE == PageStates.DRIVE):
                cards = self.detailwidget.drive_widget.type_show_card_widget.children()
            else:
                cards = self.detailwidget.mywidget.app_widget.children()
            for card in cards:
                if (card.check_btn.isEnabled() == True):
                    card.check_btn.setChecked(False)
                    if (card.btn.text() != _("Open") and card.check_btn.isEnabled() == True and card.btn.text() != _("installing") and card.btn.text() != _("installed") and card.btn.text() != _("start env") and card.cancel_btn.isHidden()):
                        card.check_btn.show()
                    else:
                        card.check_btn.setEnabled(False)
                        card.check_btn.hide()
            widget.all_check_btn.setText(_("Select all"))
            widget.all_check_btn.adjustSize()

    #
    # 函数：处理点击卡片按钮后下载界面btn显示
    #
    def slot_all_puase_btn_state(self):
        cards = self.detailwidget.mywidget.downloading_widget.children()
        if (len(cards) == 0):
            self.detailwidget.mywidget.download_all_check.setText(_("all puase"))
            self.detailwidget.mywidget.download_all_check.setEnabled(False)
            self.detailwidget.mywidget.download_all_check.setStyleSheet(
                "QPushButton{font-size:14px;font-weight: 500;color: #8f9399;line-height: 20px;border:0px solid #ff0000;}")
            self.detailwidget.mywidget.download_all_check.hide()
        else:
            all_cont = True
            has_cont = False
            for card in cards:
                if (card.down_progress.cont_btn.isHidden() == False and card.down_progress.btn.isHidden() and card.btn.isHidden() == True):
                    has_cont = True
                else:
                    all_cont = False
            if (all_cont == False and has_cont == False):
                self.detailwidget.mywidget.download_all_check.setText(_("all puase"))
                self.detailwidget.mywidget.download_all_check.setEnabled(True)
                self.detailwidget.mywidget.download_all_check.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;border:0px solid #ff0000;}QPushButton:hover{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}QPushButton:pressed{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}")
            else:
                self.detailwidget.mywidget.download_all_check.setText(_("all start"))
                self.detailwidget.mywidget.download_all_check.setEnabled(True)
                self.detailwidget.mywidget.download_all_check.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;border:0px solid #ff0000;}QPushButton:hover{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}QPushButton:pressed{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}")

    #
    # 函数：处理全部暂停
    #
    def slot_download_check_all_operation(self):
        self.detailwidget.mywidget.all_check.setEnabled(False)
        self.detailwidget.mywidget.all_select_btn.setEnabled(False)
        if (self.detailwidget.mywidget.download_all_check.text() == _("all puase")):
            self.wait_download_app_list = []
            try:
                self.down_load_progress.terminate()
            except:
                pass
            cards = self.detailwidget.mywidget.downloading_widget.children()
            for card in cards:
                for i in Globals.APPLICATION_LIST:
                    if (i["name"] == card.app.pkgname):
                        if (i["appmsg"].status != PkgStates.INSTALLING):
                            card.slot_puase_download()
            self.detailwidget.mywidget.download_all_check.hide()
            self.detailwidget.mywidget.download_all_check.setText(_("all start"))
            self.detailwidget.mywidget.download_all_check.show()
            self.detailwidget.mywidget.download_all_check.adjustSize()
        elif (self.detailwidget.mywidget.download_all_check.text() == _("all start")):
            self.detailwidget.mywidget.download_all_check.hide()
            self.detailwidget.mywidget.download_all_check.setText(_("all puase"))
            self.detailwidget.mywidget.download_all_check.show()
            self.all_continue_download.emit(self.detailwidget.mywidget.downloading_widget)

    #
    # 函数：处理全部暂停、一键下载、一键卸载、全部更新
    #
    def slot_check_all_operation(self):
        self.detailwidget.mywidget.all_check.setEnabled(False)
        self.detailwidget.mywidget.all_select_btn.setEnabled(False)
        if (self.detailwidget.mywidget.all_check.text() == _("all remove")):
            need_remove_card_list = []
            cards = self.detailwidget.mywidget.app_widget.children()
            for card in cards:
                if (card.check_btn.isChecked() == True and card.btn.isHidden() == False):
                    card.check_btn.setEnabled(False)
                    need_remove_card_list.append(card)
            self.detailwidget.mywidget.all_check.setStyleSheet(
                "QPushButton{font-size: 14px;font-weight: 500;color: #F56C6C;line-height: 20px;border:0px solid #ff0000;}QPushButton:hover{background: rgba(245, 108, 108, 0.12);border-radius: 12px;}QPushButton:pressed{background: rgba(245, 108, 108, 0.12);border-radius: 12px;}")
            self.slot_uninstall_multiple_app(need_remove_card_list)
            # self.check_btn_state()
        elif (self.detailwidget.mywidget.all_check.text() == _("all update")):
            need_update_card_list = []
            cards = self.detailwidget.mywidget.app_widget.children()
            for card in cards:
                if (card.check_btn.isChecked() == True and card.btn.isHidden() == False and card.down_progress.isHidden() == True and card.btn.text() != _(
                        "installing") and card.btn.text() != _("Open") and card.btn.text() != _("installed") and card.btn.text() != _("start env")):
                    card.check_btn.setEnabled(False)
                    need_update_card_list.append(card)
            self.slot_update_multiple_app(need_update_card_list)
        elif (self.detailwidget.mywidget.all_check.text() == _("all down")):
            need_download_card_list = []
            cards = self.detailwidget.mywidget.history_widget.app_card_widget.children()
            for card in cards:
                if (card.check_btn.isChecked() == True):
                    need_download_card_list.append(card)
            self.slot_update_multiple_app(need_download_card_list)

    #
    #函数：处理分类界面的一键下载
    #
    def slot_category_one_click_download(self):
        need_download_card_list = []
        if(Globals.NOWPAGE == PageStates.CAREGORYPAGE):
            cards = self.detailwidget.category_widget.type_show_card_widget.children()
        elif(Globals.NOWPAGE == PageStates.KYDROID):
            cards = self.detailwidget.kydroid_widget.app_widget.children()
        elif(Globals.NOWPAGE == PageStates.DRIVE):
            cards = self.detailwidget.drive_widget.type_show_card_widget.children()
        for card in cards:
            if(card.btn.isHidden() == False and card.down_progress.isHidden() == True and card.btn.text() != _("installing") and card.btn.text() != _("Open") and card.btn.text() != _("installed") and card.btn.text() != _("start env") and card.check_btn.isChecked() == True):
                need_download_card_list.append(card)
        self.slot_update_multiple_app(need_download_card_list)

    #
    # 函数：点击登录界面的退出按钮
    #
    def slot_click_loginwidget_close_button(self):
        self.loginwidget.hide()

    #
    # 函数：处理work_thread0工作结束后的显示
    #由于为了绘制卡片，将普通应用和移动应用合并了，因此更新数目只需要显示合并后的应用列表长度即可
    #
    def slot_work_thread0_over(self, item, app_list, apklist):
        if (item != "get_update_num"):
            self.app_list_msg.clear()
        self.detailwidget.loadinglabel.hide()
        self.detailwidget.mywidget.loadinglabel.hide()
        applist = []
        show_app_list = []
        applist.extend(app_list)
        if (item != "get_history_list" and item != "get_update_num"):
            for i in apklist:
                if (i not in applist):
                    applist.append(i)
        self.detailwidget.mywidget.all_select_btn.setEnabled(True)
        if (item == "get_update_num"):
            self.detailwidget.mywidget.update_num_label.setText(str(len(applist) + len(apklist)))
            self.detailwidget.mywidget.update_num_label.adjustSize()
            if (len(applist) + len(apklist) > 0):
                self.title_bar.my_btn_tip.show()
                # self.detailwidget.mywidget.update_tip_spot.show()
                self.detailwidget.mywidget.update_num_label.show()
            else:
                self.detailwidget.mywidget.update_tip_spot.hide()
                self.detailwidget.mywidget.update_num_label.hide()
                if (len(self.detailwidget.mywidget.downloading_widget.children()) == 0):
                    self.title_bar.my_btn_tip.hide()
        elif (item == "get_history_list"):
            if(Globals.NOWPAGE != PageStates.HISTORY):
                return
            self.is_remove = False
            if (self.detailwidget.mywidget.history_widget.now_btn_name == "local"):
                show_app_list = app_list
                Globals.FRIST_LEVEL_BTN_SCROLLAREA["MY"].clear()
                Globals.FRIST_LEVEL_BTN_SCROLLAREA["MY"].extend(applist)
                Globals.SEARCH_APP_LIST["HISTORY"].clear()
                Globals.SEARCH_APP_LIST["HISTORY"].extend(applist)
            else:
                show_app_list = apklist
                Globals.FRIST_LEVEL_BTN_SCROLLAREA["MY"].clear()
                Globals.FRIST_LEVEL_BTN_SCROLLAREA["MY"].extend(apklist)
                Globals.SEARCH_APP_LIST["HISTORY"].clear()
                Globals.SEARCH_APP_LIST["HISTORY"].extend(apklist)
            self.app_list_msg.extend(show_app_list)
            self.is_remove = False
            self.clear_all_card(self.detailwidget.mywidget.app_widget)
            # self.slot_card_list(self.detailwidget.mywidget.type_name_label.text(),self.app_list_msg, self.detailwidget.mywidget.history_widget,
            #                               self.detailwidget.mywidget.history_widget.app_card_widget, False, True)
            self.detailwidget.mywidget.all_check.show()
            self.detailwidget.mywidget.history_widget.label_local.show()
            if(Globals.NEED_SHOW_KYLINID == False):
                self.detailwidget.mywidget.history_widget.label_other.hide()
            else:
                self.detailwidget.mywidget.history_widget.label_other.show()
            self.detailwidget.mywidget.all_select_btn.show()
            self.detailwidget.mywidget.loadinglabel.hide()
            if (int(self.detailwidget.mywidget.update_num_label.text()) > 0 and self.detailwidget.mywidget.type_name_label.text() != _("Updates")):
                # self.detailwidget.mywidget.update_tip_spot.show()
                self.detailwidget.mywidget.update_num_label.show()
            else:
                self.detailwidget.mywidget.update_num_label.hide()
            if (len(show_app_list) == 0 and self.detailwidget.mywidget.type_name_label.text() == _("Installation")):
                self.detailwidget.mywidget.all_select_btn.setEnabled(False)
                self.detailwidget.mywidget.history_widget.reload_size()
                if Globals.THEME_COLOR == 0:
                    self.detailwidget.mywidget.history_widget.no_app_tip_label.setStyleSheet(
                        "QLabel{border-radius: 12px;background-image:url(./res/no_download.png);}")
                else:
                    self.detailwidget.mywidget.history_widget.no_app_tip_label.setStyleSheet(
                        "QLabel{border-radius: 12px;background-image:url(./res/no_download_dark.png);}")
                if (self.detailwidget.mywidget.history_widget.now_btn_name == "local"):
                    self.detailwidget.mywidget.history_widget.text_prompt_label.setText(_("No installation record!"))
                else:
                    if (Globals.LOGIN_SUCCESS):
                        self.detailwidget.mywidget.history_widget.text_prompt_label.setText(
                            _("No installation record!"))
                    else:
                        self.detailwidget.mywidget.history_widget.text_prompt_label.setText(
                            _("No installation record,Please log in and check！"))
                self.detailwidget.mywidget.history_widget.text_prompt_label.adjustSize()
                # self.detailwidget.mywidget.history_widget.text_prompt_label.move(int((self.detailwidget.mywidget.history_widget.width() - self.detailwidget.mywidget.history_widget.text_prompt_label.width()) / 2),self.detailwidget.mywidget.history_widget.no_app_tip_label.y() + self.detailwidget.mywidget.history_widget.no_app_tip_label.height() + 10)
                # self.detailwidget.mywidget.history_widget.setFixedHeight(self.detailwidget.mywidget.history_widget.text_prompt_label.height()+self.detailwidget.mywidget.history_widget.text_prompt_label.y())
                # self.detailwidget.mywidget.border.hide()
                # self.detailwidget.mywidget.type_name_label.hide()
                self.detailwidget.mywidget.all_select_btn.hide()
                self.detailwidget.mywidget.all_check.hide()
                self.detailwidget.mywidget.history_widget.app_card_widget.hide()
                # self.detailwidget.mywidget.history_widget.label_local.hide()
                # self.detailwidget.mywidget.history_widget.label_other.hide()
                self.detailwidget.mywidget.history_widget.re_move()
                self.detailwidget.mywidget.history_widget.text_prompt_label.show()
                self.detailwidget.mywidget.history_widget.no_app_tip_label.show()
                # self.detailwidget.mywidget.history_widget.no_app_tip_label.move(int(((self.width()-20)-self.detailwidget.mywidget.history_widget.no_app_tip_label.width())/2)+10-self.detailwidget.mywidget.history_widget.x(),int(((self.height()-20)-(self.detailwidget.mywidget.history_widget.no_app_tip_label.height()+self.detailwidget.mywidget.history_widget.text_prompt_label.height()+19))/2)+10-self.detailwidget.mywidget.y()-self.detailwidget.mywidget.history_widget.y())
                # self.detailwidget.mywidget.history_widget.text_prompt_label.move(int(((self.width()-20)-self.detailwidget.mywidget.history_widget.no_app_tip_label.width())/2)+10-self.detailwidget.mywidget.history_widget.x(),self.detailwidget.mywidget.history_widget.no_app_tip_label.y()+self.detailwidget.mywidget.history_widget.no_app_tip_label.height()+20)
            elif (len(show_app_list) > 0 and self.detailwidget.mywidget.type_name_label.text() == _("Installation")):
                self.detailwidget.mywidget.all_select_btn.setEnabled(True)
                self.detailwidget.mywidget.type_name_label.show()
                self.detailwidget.mywidget.border.show()
                self.detailwidget.mywidget.all_check.show()
                self.detailwidget.mywidget.all_select_btn.show()
                self.detailwidget.mywidget.history_widget.text_prompt_label.hide()
                self.detailwidget.mywidget.history_widget.no_app_tip_label.hide()
                self.detailwidget.mywidget.history_widget.app_card_widget.show()
            self.detailwidget.mywidget.history_widget.show()
            self.detailwidget.mywidget.scrollArea2.show()
            self.detailwidget.mywidget.scrollArea.hide()
            self.detailwidget.mywidget.scrollArea1.hide()
            # self.detailwidget.mywidget.installed_btn.setEnabled(True)
            if (self.detailwidget.mywidget.type_name_label.text() != _("Installation")):
                self.detailwidget.mywidget.all_check.hide()
                self.detailwidget.mywidget.all_select_btn.hide()
                self.detailwidget.mywidget.history_widget.hide()
                self.detailwidget.mywidget.scrollArea2.hide()
            self.slot_card_list(self.detailwidget.mywidget.type_name_label.text(), self.app_list_msg,
                                self.detailwidget.mywidget.history_widget,
                                self.detailwidget.mywidget.history_widget.app_card_widget, False, True)
        elif (item == "get_update_list"):
            if (Globals.NOWPAGE != PageStates.MYPAGE and self.detailwidget.mywidget.type_name_label.text() != _("Updates")):
                return
            self.is_remove = False
            Globals.FRIST_LEVEL_BTN_SCROLLAREA["MY"].clear()
            Globals.FRIST_LEVEL_BTN_SCROLLAREA["MY"].extend(applist)
            # Globals.SEARCH_APP_LIST["MY"]["apklist"].clear()
            # Globals.SEARCH_APP_LIST["MY"]["applist"].clear()
            # Globals.SEARCH_APP_LIST["MY"]["applist"].extend(applist)
            # Globals.SEARCH_APP_LIST["MY"]["apklist"].extend(apklist)
            Globals.SEARCH_APP_LIST["MY"].clear()
            Globals.SEARCH_APP_LIST["MY"].extend(applist)
            self.app_list_msg.extend(applist)
            show_app_list = applist
            self.is_remove = False
            self.detailwidget.mywidget.update_num_label.setText(str(len(applist)))
            self.detailwidget.mywidget.update_num_label.adjustSize()
            self.detailwidget.mywidget.update_num_label.show()
            self.detailwidget.mywidget.update_tip_spot.hide()
            self.clear_all_card(self.detailwidget.mywidget.app_widget)
            self.slot_card_list(self.detailwidget.mywidget.type_name_label.text(),self.app_list_msg, self.detailwidget.mywidget.app_widget,
                                          self.detailwidget.mywidget.app_widget, False, True)
            self.detailwidget.mywidget.loadinglabel.hide()
            if (len(show_app_list) == 0):
                self.detailwidget.mywidget.all_select_btn.setEnabled(False)
                if (Globals.THEME_COLOR == 0):
                    self.detailwidget.mywidget.no_app_tip_label.setStyleSheet(
                        "QLabel{border-radius: 12px;background-image:url(./res/no_update.png);}")
                else:
                    self.detailwidget.mywidget.no_app_tip_label.setStyleSheet(
                        "QLabel{border-radius: 12px;background-image:url(./res/no_update_dark.png);}")
                self.detailwidget.mywidget.text_prompt_label.setText(_("No software update!"))
                self.detailwidget.mywidget.text_prompt_label.adjustSize()
                self.detailwidget.mywidget.no_app_tip_label.move(
                    int((self.detailwidget.mywidget.width() - self.detailwidget.mywidget.no_app_tip_label.width()) / 2),
                    int((
                                    self.detailwidget.mywidget.height() - self.detailwidget.mywidget.no_app_tip_label.height() - self.detailwidget.mywidget.text_prompt_label.height() + 10) / 2))
                self.detailwidget.mywidget.text_prompt_label.move(int(
                    (self.detailwidget.mywidget.width() - self.detailwidget.mywidget.text_prompt_label.width()) / 2),
                                                                  self.detailwidget.mywidget.no_app_tip_label.y() + self.detailwidget.mywidget.no_app_tip_label.height() + 10)
                self.detailwidget.mywidget.text_prompt_label.show()
                self.detailwidget.mywidget.no_app_tip_label.show()
                self.detailwidget.mywidget.all_check.hide()
                self.detailwidget.mywidget.all_select_btn.hide()
                self.detailwidget.mywidget.update_tip_spot.hide()
                self.detailwidget.mywidget.update_num_label.hide()
                if (len(self.detailwidget.mywidget.downloading_widget.children()) == 0):
                    self.title_bar.my_btn_tip.hide()
                self.detailwidget.mywidget.scrollArea.hide()
            else:
                self.detailwidget.mywidget.type_name_label.show()
                self.detailwidget.mywidget.border.show()
                if (Globals.NOWPAGE != PageStates.MYPAGE and self.detailwidget.mywidget.type_name_label.text() != _("Updates")):
                    return
                self.detailwidget.mywidget.all_check.show()
                self.detailwidget.mywidget.all_select_btn.show()
                self.detailwidget.mywidget.text_prompt_label.hide()
                self.detailwidget.mywidget.no_app_tip_label.hide()
                self.detailwidget.mywidget.scrollArea.show()
            self.detailwidget.mywidget.scrollArea1.hide()
            self.detailwidget.mywidget.scrollArea2.hide()
        elif (item == "get_uninstall_list"):
            if (Globals.NOWPAGE != PageStates.MYPAGE and self.detailwidget.mywidget.type_name_label.text() != _("Uninstall")):
                return
            Globals.FRIST_LEVEL_BTN_SCROLLAREA["MY"].clear()
            Globals.FRIST_LEVEL_BTN_SCROLLAREA["MY"].extend(applist)
            # Globals.SEARCH_APP_LIST["MY"]["apklist"].clear()
            # Globals.SEARCH_APP_LIST["MY"]["applist"].clear()
            # Globals.SEARCH_APP_LIST["MY"]["applist"].extend(applist)
            # Globals.SEARCH_APP_LIST["MY"]["apklist"].extend(apklist)
            Globals.SEARCH_APP_LIST["MY"].clear()
            Globals.SEARCH_APP_LIST["MY"].extend(applist)
            self.app_list_msg.extend(applist)
            show_app_list = applist
            self.clear_all_card(self.detailwidget.mywidget.app_widget)
            self.is_remove = True
            self.slot_card_list(self.detailwidget.mywidget.type_name_label.text(),self.app_list_msg, self.detailwidget.mywidget.app_widget,
                                          self.detailwidget.mywidget.app_widget, True, True)
            if(self.detailwidget.mywidget.type_name_label.text() != _("Updates")):
                self.detailwidget.mywidget.update_num_label.hide()
            if (int(self.detailwidget.mywidget.update_num_label.text()) > 0):
                # self.detailwidget.mywidget.update_tip_spot.show()
                self.detailwidget.mywidget.update_num_label.show()
            else:
                self.detailwidget.mywidget.update_num_label.hide()
            self.detailwidget.mywidget.loadinglabel.hide()
            if (len(show_app_list) == 0):
                self.detailwidget.mywidget.all_select_btn.setEnabled(False)
                if (Globals.THEME_COLOR == 0):
                    self.detailwidget.mywidget.no_app_tip_label.setStyleSheet(
                        "QLabel{border-radius: 12px;background-image:url(./res/no_uninstall.png);}")
                else:
                    self.detailwidget.mywidget.no_app_tip_label.setStyleSheet(
                        "QLabel{border-radius: 12px;background-image:url(./res/no_uninstall_dark.png);}")
                self.detailwidget.mywidget.text_prompt_label.setText(_("No software remove!"))
                self.detailwidget.mywidget.text_prompt_label.adjustSize()
                self.detailwidget.mywidget.no_app_tip_label.move(
                    int((self.detailwidget.mywidget.width() - self.detailwidget.mywidget.no_app_tip_label.width()) / 2),
                    int((self.detailwidget.mywidget.height() - self.detailwidget.mywidget.no_app_tip_label.height() - self.detailwidget.mywidget.text_prompt_label.height() + 10) / 2))
                self.detailwidget.mywidget.text_prompt_label.move(int((self.detailwidget.mywidget.width() - self.detailwidget.mywidget.text_prompt_label.width()) / 2),
                                                                  self.detailwidget.mywidget.no_app_tip_label.y() + self.detailwidget.mywidget.no_app_tip_label.height() + 10)
                self.detailwidget.mywidget.text_prompt_label.show()
                self.detailwidget.mywidget.no_app_tip_label.show()
                self.detailwidget.mywidget.all_check.hide()
                self.detailwidget.mywidget.all_select_btn.hide()
                self.detailwidget.mywidget.scrollArea.hide()
            else:
                if (Globals.NOWPAGE != PageStates.MYPAGE and self.detailwidget.mywidget.type_name_label.text() != _("Uninstall")):
                    return
                self.detailwidget.mywidget.type_name_label.show()
                self.detailwidget.mywidget.all_check.show()
                self.detailwidget.mywidget.border.show()
                self.detailwidget.mywidget.all_select_btn.show()
                self.detailwidget.mywidget.text_prompt_label.hide()
                self.detailwidget.mywidget.no_app_tip_label.hide()
                self.detailwidget.mywidget.scrollArea.show()
            self.detailwidget.mywidget.scrollArea1.hide()
            self.detailwidget.mywidget.scrollArea2.hide()

    #
    # 函数：处理线程从数据库获取信息后发送的信号
    #
    def slot_get_list_over(self, item, winlist, applist, need_all_flesh):
        self.get_inside_list_over = True
        self.detailwidget.category_widget.all_check_btn.setText(_("Select all"))
        self.detailwidget.category_widget.one_click_download.setText(_("all down"))
        self.detailwidget.category_widget.one_click_download.setEnabled(False)
        self.detailwidget.category_widget.one_click_download.setStyleSheet("QPushButton{font-size:14px;font-weight: 500;color: #8f9399;line-height: 20px;border:0px solid #ff0000;}")
        category_id = self.at.appmgr.get_category_id_by_name(item)
        if (item == _("replace Win")):
            Globals.NOWPAGE = PageStates.WINPAGE
            self.app_list_msg.clear()
            Globals.FRIST_LEVEL_BTN_SCROLLAREA["CATEGORY"].clear()
            Globals.FRIST_LEVEL_BTN_SCROLLAREA["CATEGORY"].extend(applist)
            self.app_list_msg.extend(applist)
            self.win_list_msg = []
            self.win_list_msg = winlist
            cards = self.detailwidget.category_widget.type_show_card_widget.children()
            for card in cards:
                sip.delete(card)
            self.detailwidget.category_widget.scrollArea.verticalScrollBar().setValue(0)
            self.add_win_replace_card(winlist, applist, self.detailwidget.category_widget.type_show_card_widget)
            self.detailwidget.category_widget.type_widget.show()
            self.detailwidget.category_widget.scrollArea.show()
            self.detailwidget.category_widget.loadinglabel.hide()
            pass
        elif (item == _("Theme")):
            Globals.NOWPAGE = PageStates.CAREGORYPAGE
            Globals.SEARCH_NOWPAGE = PageStates.CAREGORYPAGE
            self.detailwidget.category_widget.scrollArea.verticalScrollBar().setValue(0)
            pass
        elif (item == _("Currency")):
            Globals.NOWPAGE = PageStates.CAREGORYPAGE
            Globals.SEARCH_NOWPAGE = PageStates.CAREGORYPAGE
            self.detailwidget.category_widget.scrollArea.verticalScrollBar().setValue(0)
            pass
        else:
            Globals.NOWPAGE = PageStates.CAREGORYPAGE
            Globals.SEARCH_NOWPAGE = PageStates.CAREGORYPAGE
            Globals.FRIST_LEVEL_BTN_SCROLLAREA["CATEGORY"].clear()
            Globals.FRIST_LEVEL_BTN_SCROLLAREA["CATEGORY"].extend(applist)
            self.clear_all_card(self.detailwidget.category_widget.type_show_card_widget)
            self.detailwidget.category_widget.loadinglabel.hide()
            self.detailwidget.category_widget.type_widget.show()
            self.detailwidget.category_widget.scrollArea.show()
            self.set_ad_list_signal.emit(winlist)
            self.app_list_msg.clear()
            self.app_list_msg.extend(applist)
            if (need_all_flesh):
                self.detailwidget.category_widget.scrollArea.verticalScrollBar().setValue(0)

            # self.show_inside_applist.emit(applist, self.detailwidget.category_widget.type_widget,
            #                               self.detailwidget.category_widget.type_show_card_widget, False, True)
            if(str(category_id) != "100"):
                self.slot_card_list(self.detailwidget.category_widget.type_name_label.text(),applist, self.detailwidget.category_widget.type_widget,
                                              self.detailwidget.category_widget.type_show_card_widget, False, True)
            else:
                self.slot_card_list(self.detailwidget.category_widget.type_name_label.text(), applist,
                                    self.detailwidget.category_widget.type_widget,
                                    self.detailwidget.category_widget.type_show_card_widget, False, True,True)
        for ad in self.detailwidget.category_widget.category_ad_card.ad_list_widget.children():
            if (hasattr(ad, "appmsg") and ad.appmsg != None):
                for i in Globals.ALL_CATEGORIES:
                    if(i["name"] == self.detailwidget.category_widget.type_name_label.text()):
                        self.save_ad_exposure(ad.appmsg.app_id, i["id"])
                        break
                    elif(i["displayname"] == self.detailwidget.category_widget.type_name_label.text()):
                        self.save_ad_exposure(ad.appmsg.app_id,i["id"])
                        break
        for ad in self.detailwidget.category_widget.category_ad_card.ad_list_widget2.children():
            if (hasattr(ad, "appmsg") and ad.appmsg != None):
                for i in Globals.ALL_CATEGORIES:
                    if (i["name"] == self.detailwidget.category_widget.type_name_label.text()):
                        self.save_ad_exposure(ad.appmsg.app_id, i["id"])
                        break
                    elif (i["displayname"] == self.detailwidget.category_widget.type_name_label.text()):
                        self.save_ad_exposure(ad.appmsg.app_id, i["id"])
                        break

    #
    #函数：显示安卓的分类
    #
    def slot_show_kydroid_category_widget(self,applist,need_all_flesh = True):
        self.get_inside_list_over = True
        Globals.NOWPAGE = PageStates.KYDROID
        Globals.SEARCH_NOWPAGE = PageStates.KYDROID
        # Globals.FRIST_LEVEL_BTN_SCROLLAREA["CATEGORY"].clear()
        # Globals.FRIST_LEVEL_BTN_SCROLLAREA["CATEGORY"].extend(applist)

        self.clear_all_card(self.detailwidget.kydroid_widget.app_widget)
        self.detailwidget.kydroid_widget.loadinglabel.hide()
        self.detailwidget.kydroid_widget.type_widget.show()
        self.detailwidget.kydroid_widget.scrollArea.show()
        self.pkg_list_msg.clear()
        self.pkg_list_msg.extend(applist)
        if (need_all_flesh):
            self.detailwidget.kydroid_widget.scrollArea.verticalScrollBar().setValue(0)
        self.slot_card_list(self.detailwidget.kydroid_widget.type_name_label.text(),applist, self.detailwidget.kydroid_widget.app_widget,
                                      self.detailwidget.kydroid_widget.app_widget, False, True)

    #
    # 函数：设置广告界面
    #
    def set_ad_list(self, adlist):
        applist = []
        adlist_bak = []
        adlist_bak.extend(adlist)
        for ad in adlist_bak:
            app = self.at.appmgr.get_one_app_info_by_appname(ad[0])
            if (app != None):
                applist.append(app)
            else:
                adlist.remove(ad)
        self.clear_all_card(self.detailwidget.category_widget.category_ad_card.ad_list_widget)
        self.clear_all_card(self.detailwidget.category_widget.category_ad_card.ad_list_widget2)
        self.detailwidget.category_widget.category_ad_card.get_lists(adlist, applist)
        num = 0
        # self.detailwidget.category_widget.category_ad_card.card_w = self.detailwidget.category_widget.type_widget.width() - 50*2
        count = 0
        if (len(adlist) == 0):
            if(len(self.detailwidget.category_widget.category_ad_card.ad_list_widget.children()) == 0):
                self.detailwidget.category_widget.category_ad_card.add_ad_widget(None, None, len(adlist), 0)
            else:
                self.detailwidget.category_widget.category_ad_card.change_ad_image(None, None, len(adlist), 0)
            if (len(self.detailwidget.category_widget.category_ad_card.ad_list_widget2.children()) == 0):
                self.detailwidget.category_widget.category_ad_card.add_ad_widget(None, None, len(adlist), 1)
            else:
                self.detailwidget.category_widget.category_ad_card.change_ad_image(None, None, len(adlist), 1)
        small = False
        big = False
        for i in adlist:
            app_msg = self.get_appmsg(applist[count])
            path_list = i[1].split("/")
            if ("_small" in path_list[len(path_list) - 1] and small == False):
                if(len(self.detailwidget.category_widget.category_ad_card.ad_list_widget.children()) == 0):
                    self.detailwidget.category_widget.category_ad_card.add_ad_widget(i[1], app_msg, len(adlist), 0)
                else:
                    self.detailwidget.category_widget.category_ad_card.change_ad_image(i[1], app_msg, len(adlist), 0)
                num = num + 1
                small = True
                if (len(adlist) == 1):
                    if (len(self.detailwidget.category_widget.category_ad_card.ad_list_widget2.children()) == 0):
                        self.detailwidget.category_widget.category_ad_card.add_ad_widget(None, None, len(adlist), 1)
                    else:
                        self.detailwidget.category_widget.category_ad_card.change_ad_image(None, None, len(adlist), 1)
            if ("_big" in path_list[len(path_list) - 1] and big == False):
                if (len(self.detailwidget.category_widget.category_ad_card.ad_list_widget2.children()) == 0):
                    self.detailwidget.category_widget.category_ad_card.add_ad_widget(i[1], app_msg, len(adlist), 1)
                else:
                    self.detailwidget.category_widget.category_ad_card.change_ad_image(i[1], app_msg, len(adlist), 1)
                num = num + 1
                big = True
                if (len(adlist) == 1):
                    if(len(self.detailwidget.category_widget.category_ad_card.ad_list_widget.children()) == 0):
                        self.detailwidget.category_widget.category_ad_card.add_ad_widget(None, None, len(adlist), 0)
                    else:
                        self.detailwidget.category_widget.category_ad_card.change_ad_image(None, None, len(adlist), 0)
            if (num >= 2):
                break
            count = count + 1
        self.detailwidget.category_widget.category_ad_card.show()

    #
    # 函数名: 获取当前列表
    # Function: Get current list
    #
    def get_current_listWidget(self):
        listWidget = ''
        scrollarea = ""
        if (Globals.NOWPAGE == PageStates.HOMEPAGE):
            self.is_remove = False
            listWidget = self.detailwidget.show_homepage_widget
            scrollarea = self.detailwidget.scrollArea
        elif (Globals.NOWPAGE == PageStates.SEARCHPAGE):
            self.is_remove = False
            listWidget = self.detailwidget.show_homepage_widget2
            scrollarea = self.detailwidget.scrollArea2
        elif (Globals.NOWPAGE == PageStates.DOWNLOADINGPAGE):
            self.is_remove = False
            listWidget = self.detailwidget.mywidget.downloading_widget
            scrollarea = self.detailwidget.mywidget.scrollArea1
        elif (Globals.NOWPAGE == PageStates.MYPAGE):
            listWidget = self.detailwidget.mywidget.app_widget
            scrollarea = self.detailwidget.mywidget.scrollArea
        elif (Globals.NOWPAGE == PageStates.WINPAGE or Globals.NOWPAGE == PageStates.CAREGORYPAGE):
            self.is_remove = False
            if(hasattr(self.detailwidget,"category_widget")):
                listWidget = self.detailwidget.category_widget.type_widget
                scrollarea = self.detailwidget.category_widget.scrollArea
        elif (Globals.NOWPAGE == PageStates.KYDROID):
            self.is_remove = False
            if(hasattr(self.detailwidget,"kydroid_widget")):
                listWidget = self.detailwidget.kydroid_widget.app_widget
                scrollarea = self.detailwidget.kydroid_widget.scrollArea
        elif (Globals.NOWPAGE == PageStates.HISTORY):
            self.is_remove = False
            listWidget = self.detailwidget.mywidget.history_widget.app_card_widget
            scrollarea = self.detailwidget.mywidget.scrollArea2
        elif (Globals.NOWPAGE == PageStates.MOREPAGE):
            self.is_remove = False
            listWidget = self.detailwidget.show_homepage_widget3
            scrollarea = self.detailwidget.scrollArea3
        elif(Globals.NOWPAGE == PageStates.DRIVE):
            self.is_remove = False
            if(hasattr(self.detailwidget,"drive_widget")):
                listWidget = self.detailwidget.drive_widget.type_show_card_widget
                scrollarea = self.detailwidget.drive_widget.scrollArea
        else:
            pass
        return listWidget, scrollarea

    #
    # 函数：设置界面
    #
    def slot_click_set_button(self):
        if (hasattr(self, "title_bar")):
            self.title_bar.search_line.setFocus(False)
            self.title_bar.search_line.clearFocus()
        if (hasattr(self.title_bar, "exit_login_btn")):
            self.title_bar.exit_login_btn.hide()
        self.title_bar.senior_search.hide()
        if (hasattr(self, "search_tip_widget")):
            self.search_tip_widget.hide()
        # item_name_list =[_("contact us"),_("Theme")]
        # item_name_list = [_("Theme"),_("Auto Update"),_("Help"), _("About"),  _("Exit")]
        item_name_list = [_("Theme"),_("Set"),_("Help"), _("About"),  _("Exit")]
        if (self.is_first_click_menubar == True):
            self.menubar = MenuBar(160, 36, len(item_name_list), item_name_list, self, self)
            # self.menubar.item_list.itemClicked.connect(self.slot_click_first_level)
            self.menubar.click_item.connect(self.slot_click_first_level)
            if (((self.x() + self.width() <= Globals.DESKTOP_WIDTH and self.x()>=0) or self.x()+self.title_bar.set_button.x() > Globals.DESKTOP_WIDTH) or (Globals.HAVE_THEME == True and self.x() <0) ):
                if (self.y() < 0):
                    self.menubar.move(self.x() + self.title_bar.set_button.x() + self.title_bar.set_button.width() - 10,
                                      self.title_bar.set_button.y() + self.title_bar.set_button.height() - 10)
                elif(Globals.HAVE_THEME == False and self.y()+self.height()>Globals.DESKTOP_HEIGHT):
                    self.menubar.move(self.x() + self.title_bar.set_button.x() + self.title_bar.set_button.width() - 10,
                                      Globals.DESKTOP_HEIGHT-self.height() +self.title_bar.set_button.y() + self.title_bar.set_button.height() - 10)
                else:
                    self.menubar.move(self.x() + self.title_bar.set_button.x() + self.title_bar.set_button.width() - 10,
                                      self.y() + self.title_bar.set_button.y() + self.title_bar.set_button.height() - 10)
            elif(Globals.HAVE_THEME == False and self.x() <0):
                if (self.y() < 0):
                    self.menubar.move(self.title_bar.set_button.x() + self.title_bar.set_button.width() - 10,
                                      self.title_bar.set_button.y() + self.title_bar.set_button.height() - 10)
                elif(Globals.HAVE_THEME == False and self.y()+self.height()>Globals.DESKTOP_HEIGHT):
                    self.menubar.move(self.title_bar.set_button.x() + self.title_bar.set_button.width() - 10,
                                      Globals.DESKTOP_HEIGHT-self.height() +self.title_bar.set_button.y() + self.title_bar.set_button.height() - 10)
                else:
                    self.menubar.move(self.title_bar.set_button.x() + self.title_bar.set_button.width() - 10,
                                      self.y() + self.title_bar.set_button.y() + self.title_bar.set_button.height() - 10)
            else:
                if (self.y() < 0):
                    self.menubar.move(Globals.DESKTOP_WIDTH - self.width() + self.title_bar.set_button.x(),
                                      self.title_bar.set_button.y() + self.title_bar.set_button.height() - 10)
                elif (Globals.HAVE_THEME == False and self.y() + self.height() > Globals.DESKTOP_HEIGHT):
                    self.menubar.move(Globals.DESKTOP_WIDTH - self.width() + self.title_bar.set_button.x(),
                                      Globals.DESKTOP_HEIGHT - self.height()  + self.title_bar.set_button.y() + self.title_bar.set_button.height() - 10)
                else:
                    self.menubar.move(Globals.DESKTOP_WIDTH - self.width() + self.title_bar.set_button.x(),
                                      self.y() + self.title_bar.set_button.y() + self.title_bar.set_button.height() - 10)
            self.menubar.show()
            self.is_first_click_menubar = False
        else:
            # self.menubar.move(self.x()+self.title_bar.set_button.x()+self.title_bar.set_button.width()-10,self.y()+self.title_bar.set_button.y()+self.title_bar.set_button.height()-10)
            if (((self.x() + self.width() <= Globals.DESKTOP_WIDTH and self.x() >= 0) or self.x()+self.title_bar.set_button.x() > Globals.DESKTOP_WIDTH) or (Globals.HAVE_THEME == True and self.x() <0)):

                if (self.y() < 0):
                    self.menubar.move(self.x() + self.title_bar.set_button.x() + self.title_bar.set_button.width() - 10,
                                      self.title_bar.set_button.y() + self.title_bar.set_button.height() - 10)
                elif (Globals.HAVE_THEME == False and self.y() + self.height() > Globals.DESKTOP_HEIGHT):
                    self.menubar.move(self.x() + self.title_bar.set_button.x() + self.title_bar.set_button.width() - 10,
                                      Globals.DESKTOP_HEIGHT - self.height()  + self.title_bar.set_button.y() + self.title_bar.set_button.height() - 10)
                else:
                    self.menubar.move(self.x() + self.title_bar.set_button.x() + self.title_bar.set_button.width() - 10,
                                      self.y() + self.title_bar.set_button.y() + self.title_bar.set_button.height() - 10)
            elif (Globals.HAVE_THEME == False and self.x() < 0):
                if (self.y() < 0):
                    self.menubar.move(self.title_bar.set_button.x() + self.title_bar.set_button.width() - 10,
                                      self.title_bar.set_button.y() + self.title_bar.set_button.height() - 10)
                elif (Globals.HAVE_THEME == False and self.y() + self.height() > Globals.DESKTOP_HEIGHT):
                    self.menubar.move(self.title_bar.set_button.x() + self.title_bar.set_button.width() - 10,
                                      Globals.DESKTOP_HEIGHT - self.height() + self.title_bar.set_button.y() + self.title_bar.set_button.height() - 10)
                else:
                    self.menubar.move(self.title_bar.set_button.x() + self.title_bar.set_button.width() - 10,
                                      self.y() + self.title_bar.set_button.y() + self.title_bar.set_button.height() - 10)
            else:
                if (self.y() < 0):
                    self.menubar.move(Globals.DESKTOP_WIDTH - self.width() + (self.width() - self.menubar.width()),
                                      self.title_bar.set_button.height())
                elif (Globals.HAVE_THEME == False and self.y() + self.height() > Globals.DESKTOP_HEIGHT):
                    self.menubar.move(Globals.DESKTOP_WIDTH - self.width() + self.title_bar.set_button.x(),
                                      Globals.DESKTOP_HEIGHT - self.height()  + self.title_bar.set_button.y() + self.title_bar.set_button.height() - 10)
                else:
                    self.menubar.move(Globals.DESKTOP_WIDTH - self.width() + (self.width() - self.menubar.width()),
                                      self.y() + self.title_bar.set_button.y() + self.title_bar.set_button.height() - 10)
            if(self.menubar.isHidden()):
                self.menubar.show()
            else:
                self.menubar.hide()
                if(hasattr(self,"second_lavel_menubar")):
                    self.second_lavel_menubar.hide()
        QApplication.setActiveWindow(self.menubar)

    #
    # 函数：处理点击一级目录
    #
    def slot_click_first_level(self, item):
        self.is_clicked_second_menu = True
        get_item_widget = self.menubar.item_list.itemWidget(item)
        text = get_item_widget.item_name_label.text()
        if (text == _("Theme")):
            if (self.is_first_click_second_level_menubar == True):
                item_name_list = [_("Light mode"), _("Dark Mode"), _("Automatic")]
                self.second_lavel_menubar = MenuBar(160, 36, 3, item_name_list, self, self)
                # self.second_lavel_menubar.item_list.itemClicked.connect(self.slot_click_second_level)
                self.second_lavel_menubar.click_item.connect(self.slot_click_second_level)
                self.second_lavel_menubar.move(self.menubar.x() + self.menubar.width() - 20 + 6,
                                               self.menubar.y())
                if (self.second_lavel_menubar.x() + self.second_lavel_menubar.width() <= Globals.DESKTOP_WIDTH or self.title_bar.set_button.x()+self.x() > Globals.DESKTOP_WIDTH):
                    if self.second_lavel_menubar.x() - self.menubar.x() >= self.menubar.width() - 20 + 6:
                        self.second_lavel_menubar.move(self.menubar.x() + self.menubar.width() - 20 + 6,
                                                       self.menubar.y())
                    else:
                        self.second_lavel_menubar.move(self.menubar.x() - self.second_lavel_menubar.width() + 20 - 6,
                                                       self.menubar.y())
                else:
                    self.second_lavel_menubar.move(self.menubar.x() - self.second_lavel_menubar.width() + 20 - 6,
                                                   self.menubar.y())
                self.second_lavel_menubar.show()
                self.second_lavel_menubar.raise_()
                self.is_first_click_second_level_menubar = False
            else:
                self.second_lavel_menubar.move(self.menubar.x() + self.menubar.width() - 20 + 6,
                                               self.menubar.y())
                if (self.second_lavel_menubar.x() - self.menubar.x() >= self.menubar.width() - 20 + 6 and self.second_lavel_menubar.x() + self.second_lavel_menubar.width() <= Globals.DESKTOP_WIDTH) or self.title_bar.set_button.x()+self.x() > Globals.DESKTOP_WIDTH:
                    self.second_lavel_menubar.move(self.menubar.x() + self.menubar.width() - 20 + 6,
                                                   self.menubar.y())
                else:
                    self.second_lavel_menubar.move(self.menubar.x() - self.second_lavel_menubar.width() + 20 - 6,
                                                   self.menubar.y())
                self.second_lavel_menubar.raise_()
                self.second_lavel_menubar.show()
        elif (text == _("Auto Update")):
            self.slot_need_auto_update()
        elif (text == _("Help")):
            if (self.is_first_click_second_level_menubar == False):
                self.second_lavel_menubar.hide()
            Dbus_Service.show_user_guide("software-store")
        elif (text == _("Exit")):
            self.slot_click_close_button()
        elif(text == _("Server address")):
            self.slot_click_configure_button()
        elif(text == _("Set")):
            self.slot_click_set_btn()
        else:
            if (self.is_first_click_second_level_menubar == False):
                self.second_lavel_menubar.hide()
            if (self.is_first_click_contact_us == True):
                self.contact_us_widget = Contact_Us_Widget(self, self.frame_widget)
                self.contact_us_widget.setWindowModality(Qt.ApplicationModal)
                self.contact_us_widget.move(self.x() + int((self.width() - self.contact_us_widget.width()) / 2),
                                            self.y() + int((self.height() - self.contact_us_widget.height()) / 2))
                self.is_first_click_contact_us = False
            else:
                self.contact_us_widget.move(self.x() + int((self.width() - self.contact_us_widget.width()) / 2),
                                            self.y() + int((self.height() - self.contact_us_widget.height()) / 2))
            self.contact_us_widget.slot_show_dark_mode()
            self.contact_us_widget.show()
            self.menubar.hide()
            # 弹出联系我们的界面
        self.is_clicked_second_menu = False

    #
    #函数：点击设置选项
    #
    def slot_click_set_btn(self):
        if (self.is_first_click_second_level_menubar == False):
            self.second_lavel_menubar.hide()
        self.menubar.hide()
        software_store_auto_update = "False"
        software_auto_update = "False"
        notice_software_update = "False"
        try:
            cf = configparser.ConfigParser()
            cf.read(CONF_PATH)
            software_store_auto_update = cf.get("option-settings", "software-store-auto-update")
            software_auto_update = cf.get("option-settings", "software-auto-update")
            notice_software_update = cf.get("option-settings", "notice-software-update")
        except:
            pass
        if(not hasattr(self,"set_widget")):
            self.set_widget = Set_Widget(software_store_auto_update,software_auto_update,notice_software_update,self.frame_widget)
            self.set_widget.setWindowModality(Qt.ApplicationModal)
            self.set_widget.close_btn.clicked.connect(self.slot_click_setwidget_close_btn)
            self.set_widget.btn.clicked.connect(self.slot_save_setings)
            self.set_dark_mode.connect(self.set_widget.slot_show_dark_mode)
            self.set_widget.cancel_btn.clicked.connect(self.slot_click_setwidget_close_btn)
            self.set_widget.move(self.x() + int((self.width() - self.set_widget.width()) / 2),
                                 self.y() + int((self.height() - self.set_widget.height()) / 2))
            self.set_widget.show()
        else:
            self.set_widget.close()
            self.set_widget = Set_Widget(software_store_auto_update,software_auto_update,notice_software_update,self.frame_widget)
            self.set_widget.setWindowModality(Qt.ApplicationModal)
            self.set_widget.close_btn.clicked.connect(self.slot_click_setwidget_close_btn)
            self.set_widget.btn.clicked.connect(self.slot_save_setings)
            self.set_widget.cancel_btn.clicked.connect(self.slot_click_setwidget_close_btn)
            self.set_dark_mode.connect(self.set_widget.slot_show_dark_mode)
            self.set_widget.move(self.x() + int((self.width() - self.set_widget.width()) / 2),
                                 self.y() + int((self.height() - self.set_widget.height()) / 2))
            self.set_widget.show()

    #
    #函数：处理勾选自动更新软件商店
    #
    def slot_save_setings(self):
        try:
            cf = configparser.ConfigParser()
            cf.read(CONF_PATH)
            try:
                if(self.set_widget.check_btn1.isChecked() == True):
                    cf.set("option-settings", "software-store-auto-update", "True")
                    with open(CONF_PATH, "w+") as f:
                        cf.write(f)
                else:
                    cf.set("option-settings", "software-store-auto-update", "False")
                    with open(CONF_PATH, "w+") as f:
                        cf.write(f)
                if(self.set_widget.check_btn2.isChecked() == True):
                    cf.set("option-settings", "software-auto-update", "True")
                    with open(CONF_PATH, "w+") as f:
                        cf.write(f)
                else:
                    cf.set("option-settings", "software-auto-update", "False")
                    with open(CONF_PATH, "w+") as f:
                        cf.write(f)
                if (self.set_widget.check_btn3.isChecked() == True):
                    cf.set("option-settings", "notice-software-update", "True")
                    with open(CONF_PATH, "w+") as f:
                        cf.write(f)
                else:
                    cf.set("option-settings", "notice-software-update", "False")
                    with open(CONF_PATH, "w+") as f:
                        cf.write(f)
            except:
                pass
        except:
            pass
        self.set_widget.close()


    #
    #函数：关闭设置界面
    #
    def slot_click_setwidget_close_btn(self):
        self.set_widget.close()


    #
    #函数：处理点击配置
    #
    def slot_click_configure_button(self):
        if (self.is_first_click_second_level_menubar == False):
            self.second_lavel_menubar.hide()
        self.menubar.hide()
        if(not hasattr(self,"configure_widget")):
            self.configure_widget = Configure_Widget(self)
            # self.configure_widget.close_btn.clicked.connect(self.slot_click_conf_close_btn)
            self.set_dark_mode.connect(self.configure_widget.slot_show_dark_mode)
            self.configure_widget.btn.clicked.connect(self.slot_change_server_adress)
            self.configure_widget.cancel_btn.clicked.connect(self.slot_click_conf_close_btn)
            self.configure_widget.kylinid_dress_label.textChanged.connect(self.slot_set_kylin_id_dress)
            self.configure_widget.service_dress_label.textChanged.connect(self.slot_set_service_dress)
        url_type,url = urllib.parse.splittype(Globals.SERVICE_URL)
        hosts,rest = urllib.parse.splithost(url)
        server_name = url_type + "://"+hosts
        self.configure_widget.service_dress_label.setText(server_name)
        self.configure_widget.service_dress_back_label.hide()
        url_type, url = urllib.parse.splittype(Globals.KYLIN_ID_PATH)
        hosts, rest = urllib.parse.splithost(url)
        kylinid_server_name = url_type + "://" + hosts
        self.configure_widget.kylinid_dress_label.setText(kylinid_server_name)
        self.configure_widget.kylinid_dress_back_label.hide()
        self.configure_widget.setWindowModality(Qt.ApplicationModal)
        self.configure_widget.move(self.x() + int((self.width() - self.configure_widget.width()) / 2),
                             self.y() + int((self.height() - self.configure_widget.height()) / 2))
        self.configure_widget.show()

    def slot_change_server_adress(self):
        if(self.configure_widget.service_dress_label.text().strip(" ") != None and self.configure_widget.service_dress_label.text().strip(" ") != ""):
            self.configure_widget.service_dress_back_label.hide()
            self.slot_change_uksc_service(self.configure_widget.service_dress_label.text())
        else:
            self.configure_widget.service_dress_back_label.show()
        if (self.configure_widget.kylinid_dress_label.text() != None and self.configure_widget.kylinid_dress_label.text() != ""):
            self.configure_widget.kylinid_dress_back_label.hide()
            kylin_id_adress = self.configure_widget.kylinid_dress_label.text()
            self.slot_change_kylin_id_service(self.configure_widget.kylinid_dress_label.text())
        else:
            self.configure_widget.kylinid_dress_back_label.show()

    def checkip(self,hostip):
        hostip = hostip.strip(" ").split(":")[0]
        pat = re.compile(r'([0-9]{1,3})\.')
        r = re.findall(pat, hostip + ".")
        if len(r) == 4 and len([x for x in r if int(x) >= 0 and int(x) <= 255]) == 4:
            return True
        else:
            return False
        # return True

    def slot_change_uksc_service(self,text):
        try:
            #检测格式是否正常
            url_type, url = urllib.parse.splittype(text)
            hosts, rest = urllib.parse.splithost(url)
            server_name = url_type + "://" + hosts
            if (url_type == None or hosts == None or hosts.strip(" ") == "" or url == None or url == ""):
                self.configure_widget.tip_label.show()
            else:
                service_url = ""
                if(Globals.SERVICE_URL != service_url):
                    Globals.SERVICE_URL = service_url
                    self.ask_thread.appmgr.premoter.service_url = service_url
                    cf = configparser.ConfigParser()
                    cf.read(CONF_PATH)
                    cf.set("server-address", "server", service_url)
                    with open(CONF_PATH, "w+") as f:
                        cf.write(f)

                    # self.configure_widget.hide()
                    # self.configure_widget.service_dress_label.setText("")
                    os.system("systemctl --global daemon-reload")
                    os.system("systemctl --global restart kylin-software-center.service")
        except:
            self.configure_widget.tip_label.show()
            pass

    def slot_change_kylin_id_service(self,text):
        try:
            url_type, url = urllib.parse.splittype(text)
            hosts, rest = urllib.parse.splithost(url)
            if (url_type == None or hosts == None or url == None or url == "" or hosts.strip(" ") == ""):
                self.configure_widget.tip_label2.show()
            else:
                login_url =  text + "/login?userSource=softwareStore"
                register_account = text + "/registered?userSource=softwareStore"
                find_pwd = text + "/find"
                if(Globals.KYLIN_ID_PATH != login_url or Globals.REGISTER_ACCOUNT != register_account or Globals.FIND_PASSWORD != find_pwd):
                    Globals.KYLIN_ID_PATH = login_url
                    Globals.REGISTER_ACCOUNT = register_account
                    Globals.FIND_PASSWORD = find_pwd
                    cf = configparser.ConfigParser()
                    cf.read(CONF_PATH)
                    cf.set("server-address", "kylin_id_path", login_url)
                    with open(CONF_PATH, "w+") as f:
                        cf.write(f)
                    cf.set("server-address", "register_account", register_account)
                    with open(CONF_PATH, "w+") as f:
                        cf.write(f)
                    cf.set("server-address", "find_pwd", find_pwd)
                    with open(CONF_PATH, "w+") as f:
                        cf.write(f)

                    if (os.path.exists(KYLIN_ID_CONF_PATH)):
                        url_type, url = urllib.parse.splittype(text)
                        hosts, rest = urllib.parse.splithost(url)
                        id_conf = QSettings(KYLIN_ID_CONF_PATH,QSettings.IniFormat)
                        id_conf.sync()
                        # id_cf = configparser.ConfigParser()
                        # id_cf.read(KYLIN_ID_CONF_PATH)
                        id_conf.setValue("UrlSettings/baseUrl",hosts)
                        id_conf.setValue("UrlSettings/prefix", "/v1/api/")
                        # id_cf.set("UrlSettings", "baseUrl", hosts)
                        # id_cf.set("UrlSettings", "prefix", "/v1/api/")
                        if (url_type == "http"):
                            id_conf.setValue("UrlSettings/isUseSsl", "false")
                            # id_cf.set("UrlSettings", "isUseSsl", "false")
                        else:
                            # id_cf.set("UrlSettings", "isUseSsl", "true")
                            id_conf.setValue("UrlSettings/isUseSsl", "true")
                        # with open(CONF_PATH, "w+") as f:
                        #     id_cf.write(f)
                        id_conf.sync()
                        func = getattr(self.ask_thread.kylinid_iface, "reloadApi")
                        if func is None:
                            return False
                        re = func()

                if(self.configure_widget.tip_label.isHidden() == True):
                    self.configure_widget.hide()
                    self.configure_widget.kylinid_dress_label.setText("")
        except Exception as e:
            self.configure_widget.tip_label2.show()


    def slot_set_service_dress(self):
        if (self.configure_widget.service_dress_label.text() != None and self.configure_widget.service_dress_label.text() != ""):
            self.configure_widget.service_dress_back_label.hide()
            self.configure_widget.btn.setEnabled(True)
            self.configure_widget.btn.setStyleSheet("QPushButton{background: #3790FA;border-radius: 4px;font-size: 14px;font-weight: 400;color: #FFFFFF;text-align: center;line-height: 12px;}QPushButton:hover{background: #40A9FB;;border-radius: 4px;color: #ffffff;}QPushButton:pressed{background: #296CD9;border-radius: 4px;color: #ffffff;}")
        else:
            self.configure_widget.service_dress_back_label.show()
            self.configure_widget.btn.setEnabled(False)
            self.configure_widget.btn.setStyleSheet(
                "QPushButton{background: #E9E9E9;border-radius: 4px;font-size: 14px;font-weight: 400;color: #B3B3B3;text-align: center;line-height: 12px;}")
        self.configure_widget.tip_label.hide()

    def slot_set_kylin_id_dress(self,text):
        if (self.configure_widget.kylinid_dress_label.text() != None and self.configure_widget.kylinid_dress_label.text() != ""):
            self.configure_widget.kylinid_dress_back_label.hide()
            self.configure_widget.btn.setEnabled(True)
            self.configure_widget.btn.setStyleSheet("QPushButton{background: #3790FA;border-radius: 4px;font-size: 14px;font-weight: 400;color: #FFFFFF;text-align: center;line-height: 12px;}QPushButton:hover{background: #40A9FB;;border-radius: 4px;color: #ffffff;}QPushButton:pressed{background: #296CD9;border-radius: 4px;color: #ffffff;}")
        else:
            self.configure_widget.kylinid_dress_back_label.show()
            self.configure_widget.btn.setEnabled(False)
            self.configure_widget.btn.setStyleSheet("QPushButton{background: #E9E9E9;border-radius: 4px;font-size: 14px;font-weight: 400;color: #B3B3B3;text-align: center;line-height: 12px;}")
        self.configure_widget.tip_label2.hide()

    def slot_click_conf_close_btn(self):
        self.configure_widget.close_btn.setChecked(False)
        self.configure_widget.hide()

    #
    # 函数：处理点击第二层目录的itemis_clicked_second_menu:如果是点击第二级目录则不会影藏一级目录
    #
    def slot_click_second_level(self, item):
        self.menubar.hide()
        if(hasattr(self,"second_lavel_menubar")):
            self.second_lavel_menubar.hide()
        get_item_widget = self.second_lavel_menubar.item_list.itemWidget(item)
        text = get_item_widget.item_name_label.text()
        item_count = self.second_lavel_menubar.item_list.count()
        for i in range(item_count):
            ergodic_item = self.second_lavel_menubar.item_list.item(i)
            ergodic_item_widget = self.second_lavel_menubar.item_list.itemWidget(ergodic_item)
            get_ergodic_text = ergodic_item_widget.item_name_label.text()
            if (text == get_ergodic_text):
                get_item_widget.icon_label.show()
                cf = configparser.ConfigParser()
                cf.read(CONF_PATH)
                if (text == _("Dark Mode")):
                    cf.set("theme-color", "dark", "YES")
                    cf.set("theme-color", "light", "NO")
                    cf.set("theme-color", "auto", "NO")
                    with open(CONF_PATH, "w+") as f:
                        cf.write(f)
                    Globals.THEME_COLOR = 1
                elif (text == _("Light mode")):
                    cf.set("theme-color", "dark", "NO")
                    cf.set("theme-color", "light", "YES")
                    cf.set("theme-color", "auto", "NO")
                    with open(CONF_PATH, "w+") as f:
                        cf.write(f)
                    Globals.THEME_COLOR = 0
                else:
                    cf.set("theme-color", "dark", "NO")
                    cf.set("theme-color", "light", "NO")
                    cf.set("theme-color", "auto", "YES")
                    with open(CONF_PATH, "w+") as f:
                        cf.write(f)
                    self.theme_style()
                # self.set_dark_mode.emit()
                self.set_mode()
                # self.menubar.slot_show_dark_mode()
                # self.second_lavel_menubar.slot_show_dark_mode()
                # self.title_bar.refresh_background_color()
                # self.detailwidget.refresh_background_color()
                # self.set_title_btn_stylesheet(Globals.NOWPAGE)
            else:
                ergodic_item_widget.icon_label.hide()

    #
    #函数：处理是否自动更新软件
    #
    def slot_need_auto_update(self):
        print("%^%^%^%^")

    #
    # 函数：处理发送我的评分
    #
    def slot_submit_my_rating(self, grade, app_msg):
        item = {"action": "submit_my_rating", "appname": app_msg.pkgname, "app_msg": app_msg, "grade": grade}
        self.ask_thread.mutex.acquire()
        self.ask_thread.work_list.append(item)
        self.ask_thread.mutex.release()

    #
    # 函数：处理需要重新获取平均分和各分所占百分比
    #
    def slot_need_get_new_rating(self, id, appname,softstyle):
        item = {"action": "get_rating_avg", "appname": appname, "id": id,"softstyle":softstyle}
        self.ask_thread.mutex.acquire()
        self.ask_thread.work_list.append(item)
        self.ask_thread.mutex.release()

    #
    # 函数：返回获取的评分
    #
    def slot_return_rating(self, count, scoreDetail, score, appname):
        if (score != -1):
            for i in Globals.APPLICATION_LIST:
                if (appname == i["name"]):
                    i["appmsg"].ratings_average = score
        if (hasattr(self.detailwidget, "app_detail_widget")):
            self.detailwidget.app_detail_widget.slot_change_rating(count, scoreDetail, appname, score)

    # 函数：处理获取软件签名状态
    # 参数：id，appname
    # 返回：无
    def slot_need_get_app_sign_state(self, id, appname):
        item = {"action": "get_app_sign", "appname": appname, "id": id}
        self.ask_thread.mutex.acquire()
        self.ask_thread.work_list.append(item)
        self.ask_thread.mutex.release()

    #
    # 函数：处理获取签名结束
    #
    def slot_return_app_sign_state(self, appname, app_sign_state):

        # for i in Globals.APPLICATION_LIST:
        #     if (appname == i["name"]):
        #         i["appmsg"].downloadcount = count
        if (hasattr(self.detailwidget, "app_detail_widget")):
            self.detailwidget.app_detail_widget.slot_app_sign_state(appname, app_sign_state)

    #
    # 函数：处理提交评论异常
    #
    def slot_return_submit_review(self, review_res):

        # for i in Globals.APPLICATION_LIST:
        #     if (appname == i["name"]):
        #         i["appmsg"].downloadcount = count
        if (hasattr(self.detailwidget, "app_detail_widget")):
            self.detailwidget.app_detail_widget.get_submit_review_state(review_res)

    # 函数：处理获取软件下载次数
    #
    def slot_need_get_download_count(self, id, appname,softstyle):
        item = {"action": "get_download_count", "appname": appname, "id": id,"softstyle":softstyle}
        self.ask_thread.mutex.acquire()
        self.ask_thread.work_list.append(item)
        self.ask_thread.mutex.release()

    #
    # 函数：处理获取软件下载次数结束
    #
    def slot_return_download_count(self, count, appname,update_time):
        for i in Globals.APPLICATION_LIST:
            if (appname == i["name"]):
                i["appmsg"].downloadcount = count
                i["appmsg"].screenshots_updatetime = update_time
        if (hasattr(self.detailwidget, "app_detail_widget")):
            self.detailwidget.app_detail_widget.slot_change_download_count(appname, count , update_time)

    #
    # 函数：获取我的评分
    #
    def slot_get_my_rating(self, id, appname):
        item = {"action": "get_my_rating", "appname": appname, "id": id}
        self.ask_thread.mutex.acquire()
        self.ask_thread.work_list.append(item)
        self.ask_thread.mutex.release()

    #
    # 函数：处理获取我的评分结束
    #
    def slot_get_my_reating_over(self, score, id, appname):
        if (hasattr(self.detailwidget, "app_detail_widget")):
            self.detailwidget.app_detail_widget.slot_change_my_rating(score, id, appname)

    #
    # 函数：获取软件截图
    #
    def slot_get_screen_shot(self, down_url, id, appname, update_time):
        item = {"action": "download_screen_shot", "appname": appname, "id": id, "down_url": down_url,
                "update_time": update_time}
        self.ask_thread.mutex.acquire()
        self.ask_thread.work_list.append(item)
        self.ask_thread.mutex.release()

    #
    # 函数：获取软件截图结束
    #
    def slot_get_screen_shot_over(self, appname, update_time):
        if (hasattr(self.detailwidget, "app_detail_widget") and appname != ""):
            self.detailwidget.app_detail_widget.check_screen_shot_file(appname, update_time)

    #
    # 函数：获取软件截图
    #
    def slot_get_screen_shot_msg(self, id, appname):
        item = {"action": "get_screen_shot", "id": id, "appname": appname}
        self.ask_thread.mutex.acquire()
        self.ask_thread.work_list.append(item)
        self.ask_thread.mutex.release()

    #
    # 函数：获取软件截图信息结束
    #
    def slot_get_shot_screen_list(self, app_detail):
        if (app_detail != False):
            if (app_detail["code"] == 200 and app_detail["data"] != None):
                # self.detailwidget.app_detail_widget.get_screen_shot_msg(app_detail["data"])
                app_detail["data"]["updateTime"] = app_detail["data"]["updateTime"].split(".")[0]
                self.get_screen_shot_signal.emit(app_detail["data"])

    #
    # 函数：分页获取软件评论
    #
    def slot_get_app_reviews(self, app_msg, appname, page):
        item = {"action": "get_app_reviews", "appname": appname, "id": app_msg.app_id, "page": page}
        self.ask_thread.mutex.acquire()
        self.ask_thread.work_list.append(item)
        self.ask_thread.mutex.release()

    #
    # 函数：点击撰写评论按钮
    #
    def slot_click_edit_btn(self,appmsg,my_rating):
        if(hasattr(self,"comment_pop_up") == False):
            self.comment_pop_up = Comment_Pop_Up(appmsg,my_rating,self.detailwidget.app_detail_widget,self.frame_widget)
            self.comment_pop_up.setWindowModality(Qt.ApplicationModal)
            self.set_dark_mode.connect(self.comment_pop_up.slot_show_dark_mode)
            self.comment_pop_up.close_btn.clicked.connect(self.slot_click_comment_pop_up_closebtn)
            self.comment_pop_up.cancel_btn.clicked.connect(self.slot_click_comment_pop_up_closebtn)
            self.comment_pop_up.move(self.x() + int((self.width() - self.comment_pop_up.width()) / 2),
                             self.y() + int((self.height() - self.comment_pop_up.height()) / 2))
        else:
            self.comment_pop_up.close()
            self.comment_pop_up = Comment_Pop_Up(appmsg, my_rating, self.detailwidget.app_detail_widget,
                                                 self.frame_widget)
            self.comment_pop_up.setWindowModality(Qt.ApplicationModal)
            self.set_dark_mode.connect(self.comment_pop_up.slot_show_dark_mode)
            self.comment_pop_up.close_btn.clicked.connect(self.slot_click_comment_pop_up_closebtn)
            self.comment_pop_up.cancel_btn.clicked.connect(self.slot_click_comment_pop_up_closebtn)
            self.comment_pop_up.move(self.x() + int((self.width() - self.comment_pop_up.width()) / 2),
                                     self.y() + int((self.height() - self.comment_pop_up.height()) / 2))

    #
    #函数：处理关闭撰写评论弹窗
    #
    def slot_click_comment_pop_up_closebtn(self):
        if(hasattr(self,"comment_pop_up")):
            self.comment_pop_up.close()

    #
    #函数：评论点击点赞
    #
    def slot_click_agree_with_btn(self,reviewlist,appid,comment_widget):
        if(Globals.LOGIN_SUCCESS):
            item = {}
            item["action"] = "give_the_thumbs_up"
            item["data"] = reviewlist
            item["widget"] = comment_widget
            item["appid"] = appid
            if(comment_widget.agree_with == True):
                item["behaviorLike"] = 0
            else:
                item["behaviorLike"] = 1
            self.ask_thread.mutex.acquire()
            self.ask_thread.work_list.append(item)
            self.ask_thread.mutex.release()
        else:
            self.show_tip_widget(_("Please log in to your account first!"), False)

    #
    # 函数：处理评论点赞结束
    #
    def slot_send_like_finish(self, res, comment_widget,behaviorlike):
        if (res["code"] == 200 and res["data"] != None):
            if (behaviorlike == 0):
                comment_widget.agree_with = False
                comment_widget.disagree = False
                comment_widget.init_btn_statue()
            else:
                comment_widget.agree_with = True
                comment_widget.disagree = False
                comment_widget.click_agree_over()
            comment_widget.agree_with_count_label.setText(str(res["data"]["likeCount"]))
            comment_widget.agree_with_count_label.adjustSize()
            comment_widget.disagree_count_label.setText(str(res["data"]["unlikeCount"]))
            comment_widget.disagree_count_label.adjustSize()
        if (res["code"] == -1):
            comment_widget.click_agree_over()


    #
    # 函数：评论点击点踩
    #
    def slot_click_disagree_with_btn(self, reviewlist, appid,comment_widget):
        if(Globals.LOGIN_SUCCESS):
            item = {}
            item["action"] = "disagree_comment"
            item["data"] = reviewlist
            item["widget"] = comment_widget
            item["appid"] = appid
            if (comment_widget.disagree == True):
                item["behaviorUnlike"] = 0
            else:
                item["behaviorUnlike"] = 1
            self.ask_thread.mutex.acquire()
            self.ask_thread.work_list.append(item)
            self.ask_thread.mutex.release()
        else:
            self.show_tip_widget(_("Please log in to your account first!"), False)

    #
    #函数：处理评论点踩结束
    #
    def slot_send_dislike_finish(self,res,comment_widget,behaviorUnlike):
        if(res["code"] == 200 and res["data"] != None):
            if(behaviorUnlike == 0):
                comment_widget.agree_with = False
                comment_widget.disagree = False
                comment_widget.init_btn_statue()
            else:
                comment_widget.agree_with = False
                comment_widget.disagree = True
                comment_widget.click_disagree_over()
            comment_widget.agree_with_count_label.setText(str(res["data"]["likeCount"]))
            comment_widget.agree_with_count_label.adjustSize()
            comment_widget.disagree_count_label.setText(str(res["data"]["unlikeCount"]))
            comment_widget.disagree_count_label.adjustSize()
        if(res["code"] == -1):
            comment_widget.click_disagree_over()

    #
    #函数：提示删除评论
    #
    def show_delete_user_comment_tip(self,reviewlist,appname,text = _("You are about to delete this comment!")):
        if (not hasattr(self, "delete_comment_tip_widget")):
            self.delete_comment_tip_widget = Tip_Widget(text, self.frame_widget)
            self.delete_comment_tip_widget.slot_show_dark_mode()
            self.delete_comment_tip_widget.setWindowModality(Qt.ApplicationModal)
            self.set_dark_mode.connect(self.delete_comment_tip_widget.slot_show_dark_mode)
            self.delete_comment_tip_widget.close_btn.clicked.connect(self.slot_close_deletecommenttipwidget)
            self.delete_comment_tip_widget.cancel_btn.clicked.connect(self.slot_close_deletecommenttipwidget)
            self.delete_comment_tip_widget.btn.clicked.connect(self.slot_close_deletecommenttipwidget)
            self.delete_comment_tip_widget.btn.clicked.connect(lambda :self.slot_delete_user_commemt(reviewlist,appname))
            self.delete_comment_tip_widget.move(self.x() + int((self.width() - self.delete_comment_tip_widget.width()) / 2),
            self.y() + int((self.height() - self.delete_comment_tip_widget.height()) / 2))
            self.delete_comment_tip_widget.show()
        else:
            if (self.delete_comment_tip_widget.isHidden() == True):
                self.delete_comment_tip_widget.close()
                self.delete_comment_tip_widget = Tip_Widget(_("You are about to delete this comment!"), self.frame_widget)
                self.delete_comment_tip_widget.slot_show_dark_mode()
                self.delete_comment_tip_widget.setWindowModality(Qt.ApplicationModal)
                self.set_dark_mode.connect(self.delete_comment_tip_widget.slot_show_dark_mode)
                self.delete_comment_tip_widget.close_btn.clicked.connect(self.slot_close_deletecommenttipwidget)
                self.delete_comment_tip_widget.cancel_btn.clicked.connect(self.slot_close_deletecommenttipwidget)
                self.delete_comment_tip_widget.btn.clicked.connect(self.slot_close_deletecommenttipwidget)
                self.delete_comment_tip_widget.btn.clicked.connect(lambda :self.slot_delete_user_commemt(reviewlist,appname))
                # self.delete_comment_tip_widget.btn.clicked.connect(self.slot_close_tipwidget)
                self.delete_comment_tip_widget.move(
                    self.x() + int((self.width() - self.delete_comment_tip_widget.width()) / 2),
                    self.y() + int((self.height() - self.delete_comment_tip_widget.height()) / 2))
                self.delete_comment_tip_widget.show()

    def slot_close_deletecommenttipwidget(self):

        self.delete_comment_tip_widget.close()


    #
    #函数：处理删除评论
    #
    def slot_delete_user_commemt(self,reviewlist,appname):
        item = {}
        #{"commentId":"","userId":"",}
        item["action"] = "delete_user_comment"
        item["data"] = reviewlist
        item["appname"] = appname
        self.ask_thread.mutex.acquire()
        self.ask_thread.work_list.append(item)
        self.ask_thread.mutex.release()

    #
    #函数：处理删除评论结束
    #
    def slot_User_operation(self,tip):
        self.show_tip_widget(tip)

    #
    # 函数：发送我的评论
    #
    # def slot_submit_my_review(self, id, appname, my_review,app_version):
    def slot_submit_my_review(self,app_msg,my_review):
        item = {"action": "submit_my_review", "appname": app_msg.pkgname, "id": app_msg.app_id, "review": my_review,"version":app_msg.version,"app_msg":app_msg}
        self.ask_thread.mutex.acquire()
        self.ask_thread.work_list.append(item)
        self.ask_thread.mutex.release()

    #
    # 函数：发送我发评论结束
    #
    def slot_submit_my_review_over(self, appname):
        if (hasattr(self.detailwidget, "app_detail_widget")):
            self.detailwidget.app_detail_widget.slot_submit_my_review_over(appname)

    #
    # 函数：收集软件使用信息
    #
    def submit_pinback(self):
        item = {"action": "submit_pingback"}
        self.ask_thread.mutex.acquire()
        self.ask_thread.work_list.append(item)
        self.ask_thread.mutex.release()

    #
    # 函数：下载后发送下载记录到服务器
    #
    def slot_submit_down_over(self, appname, id,version):
        item = {"action": "submit_download_msg", "id": id,"version":version}
        self.ask_thread.mutex.acquire()
        self.ask_thread.work_list.append(item)
        self.ask_thread.mutex.release()

    #
    # 函数：卸载软件后发送卸载记录到服务器
    #
    def slot_submit_remove_over(self, appname, id,version):
        self.signal_uninstall.emit(appname, _("uninstalled"))
        item = {"action": "subimt_app_remove_msg", "id": id ,"version":version}
        self.ask_thread.mutex.acquire()
        self.ask_thread.work_list.append(item)
        self.ask_thread.mutex.release()

    #
    # 函数：登录失效
    #
    def slot_login_failure(self, msg):
        self.title_bar.login_btn.setEnabled(True)
        self.title_bar.exit_login_btn.setEnabled(True)
        self.title_bar.login_btn.user_name_label.setToolTip("")

        if msg == 'False':
            if (os.path.exists(os.path.join(CACHE_DIR, "kylinId/token"))):
                os.remove(os.path.join(CACHE_DIR, "kylinId/token"))
            self.show_tip_widget(_("Network disconnection or server access failure!"), True,False)
            self.tip_widget.tips_label.setAlignment(Qt.AlignLeft)
            self.tip_widget.btn.clicked.connect(self.slot_close_tipwidget)
            # self.title_bar.login_btn.user_name_label.setText(Globals.LOGIN_ACCOUNT)
            self.title_bar.login_btn.setLongTextToElideFormat(self.title_bar.login_btn.user_name_label,
                                                              Globals.LOGIN_ACCOUNT)
            user_profile = UBUNTUKYLIN_DATA_PATH + "user_profile/" + Globals.USER_PROFILE
            pix2 = QPixmap(user_profile)
            fitpixmap_userIcon = pix2.scaled(20, 20, Qt.IgnoreAspectRatio, Qt.SmoothTransformation)
            fitpixmap_userIcon = self.PixmapToRound(fitpixmap_userIcon, 20,
                                                    20)
            self.title_bar.login_btn.head_portrait_label.setPixmap(fitpixmap_userIcon)
            # self.title_bar.login_btn.head_portrait_label.setStyleSheet("QLabel{border:0px;border-radius: 10px;border-image: url('"+user_profile+"') 0 0 0 0 stretch strectch;background-color:transparent;}")
            self.title_bar.login_btn.user_name_label.adjustSize()

            self.title_bar.login_btn.setLongTextToElideFormat(self.title_bar.login_btn.user_name_label,
                                                              Globals.LOGIN_ACCOUNT)
            self.title_bar.login_btn.btn.resize(
                self.title_bar.login_btn.user_name_label.width() + 4 + self.title_bar.login_btn.head_portrait_label.width(),
                20)
            self.title_bar.login_btn.setLongTextToElideFormat(self.title_bar.login_btn.user_name_label,
                                                              Globals.LOGIN_ACCOUNT)
            if ("…" in self.title_bar.login_btn.user_name_label.text()):
                self.title_bar.login_btn.user_name_label.setToolTip(Globals.LOGIN_ACCOUNT)

        else:
            if (os.path.exists(os.path.join(CACHE_DIR, "kylinId/token"))):
                os.remove(os.path.join(CACHE_DIR, "kylinId/token"))
            self.slot_exit_login_finish()
            # self.slot_login_account()
            if (hasattr(self, "loginwidget")):
                self.loginwidget.ui.tips_user_password.hide()
                self.loginwidget.ui.tips_user_password.setText(msg)
                # self.loginwidget.ui.tips_user_password.setText(_("Login failure"))
                self.loginwidget.ui.tips_user_password.show()
                self.loginwidget.timer_set()

    #
    #
    #
    def slot_login_finish(self, res):
        res_list = json.loads(str(res))
        self.slot_get_ui_login_over(res_list)

    #
    # 函数名:点击登录之后的操作和错误提示
    # Function:click login over
    #
    def slot_get_ui_login_over(self, login_over_list):
        # try:
        if login_over_list == False:
            pass
        else:
            code = login_over_list['code']
            if code == 200 or code == '200':
                Globals.LOGIN_SUCCESS = True
                Globals.LOGIN_ACCOUNT = login_over_list['data']['entity']['userName']
                Globals.USER_PROFILE = login_over_list['data']['entity']['avatar']
                Globals.ACCOUNT = login_over_list['data']['entity']['kylinID']

            self.slot_ui_login_success()
        # except:
        #     return False

    #
    # 函数：退出登录结束
    #
    def slot_exit_login_finish(self):
        if (os.path.exists(os.path.join(CACHE_DIR, "kylinId/token"))):
            os.remove(os.path.join(CACHE_DIR, "kylinId/token"))
        Globals.LOGIN_ACCOUNT = ''
        Globals.USER_PROFILE = ''
        Globals.ACCOUNT = ''
        Globals.USER_ID = ''
        Globals.LOGIN_SUCCESS = False
        Globals.USER_AND_PWD = []
        Globals.USER_AND_PWD_DECODE = []
        self.title_bar.login_btn.setEnabled(True)
        # self.title_bar.login_btn.user_name_label.setText(_("login"))
        # self.title_bar.login_btn.user_name_label.adjustSize()
        self.title_bar.login_btn.setLongTextToElideFormat(self.title_bar.login_btn.user_name_label,
                                                          _("login"))
        if (Globals.THEME_COLOR == 0):
            user_profile = UBUNTUKYLIN_RES_PATH + "unlogin.png"
            pix2 = QPixmap(user_profile)
            fitpixmap_userIcon = pix2.scaled(20, 20, Qt.IgnoreAspectRatio, Qt.SmoothTransformation)
            fitpixmap_userIcon = self.PixmapToRound(fitpixmap_userIcon, 20,
                                                    20)
            self.title_bar.login_btn.head_portrait_label.setPixmap(fitpixmap_userIcon)
            # self.title_bar.login_btn.head_portrait_label.setStyleSheet(
            #     "QLabel{border:0px;border-radius: 10px;background-image:url(./res/unlogin.png)}")
        else:
            user_profile = UBUNTUKYLIN_RES_PATH + "unlogin_dark.png"
            pix2 = QPixmap(user_profile)
            fitpixmap_userIcon = pix2.scaled(20, 20, Qt.IgnoreAspectRatio, Qt.SmoothTransformation)
            fitpixmap_userIcon = self.PixmapToRound(fitpixmap_userIcon, 20,
                                                    20)
            self.title_bar.login_btn.head_portrait_label.setPixmap(fitpixmap_userIcon)
            # self.title_bar.login_btn.head_portrait_label.setStyleSheet(
            #     "QLabel{border:0px;border-radius: 10px;background-image:url(./res/unlogin_dark.png);background-position:center;}")
        self.title_bar.login_btn.user_name_label.adjustSize()
        self.title_bar.login_btn.user_name_label.resize(self.title_bar.login_btn.user_name_label.width() + 3, 20)
        self.title_bar.login_btn.user_name_label.setToolTip("")
        self.title_bar.show_exit_login_widget()
        self.title_bar.exit_login_btn.setEnabled(True)
        if (hasattr(self.detailwidget, "app_detail_widget")):
            self.detailwidget.app_detail_widget.Login_status_check()
            self.detailwidget.app_detail_widget.ratings_widget.refresh_star()

    #
    # 函数：处理网路出错显示
    #
    def slot_network_error(self, image, test):
        if (Globals.THEME_COLOR == 1):
            if (image == "./res/no_network.png"):
                image = "./res/no_network_dark.png"
        self.detailwidget.loading_kydroid_evn.hide()
        self.title_bar.home_page_btn.setEnabled(True)
        self.title_bar.my_btn.setEnabled(True)
        self.title_bar.drive_btn.setEnabled(True)
        self.title_bar.kydroid_btn.setEnabled(True)
        self.title_bar.category_btn.setEnabled(True)
        self.title_bar.search_line.setReadOnly(False)
        self.title_bar.senior_search.setEnabled(True)
        self.title_bar.search_image.setEnabled(True)
        self.title_bar.return_btn.setEnabled(True)
        self.title_bar.login_btn.setEnabled(True)
        self.detailwidget.loading_kydroid_evn.hide()
        self.detailwidget.loadinglabel.hide()
        self.detailwidget.no_app_tip_label.setStyleSheet(
            "QLabel{border-radius: 12px;background-image:url('" + image + "');}")
        self.detailwidget.text_prompt_label.setText(test)
        self.detailwidget.text_prompt_label.adjustSize()
        self.detailwidget.no_app_tip_label.move(
            int((self.detailwidget.width() - self.detailwidget.no_app_tip_label.width()) / 2), int((self.detailwidget.height() - self.detailwidget.no_app_tip_label.height() - self.detailwidget.text_prompt_label.height() + 10) / 2))
        self.detailwidget.text_prompt_label.move(
            int((self.detailwidget.width() - self.detailwidget.text_prompt_label.width()) / 2),
            self.detailwidget.no_app_tip_label.y() + self.detailwidget.no_app_tip_label.height() + 10)

        # self.detailwidget.show_homepage_widget.hide()
        # self.detailwidget.scrollArea.hide()
        self.detailwidget.show_homepage_widget2.hide()
        self.detailwidget.scrollArea2.hide()
        self.detailwidget.show_homepage_widget3.hide()
        self.detailwidget.scrollArea3.hide()
        self.detailwidget.text_prompt_label.show()
        self.detailwidget.no_app_tip_label.show()

    #
    # 函数：处理修改分辨率
    #
    def show_new_widget(self, size):
        Globals.DESKTOP_HAS_CHANGE = True
        desktop = QApplication.desktop()
        rect = desktop.availableGeometry()
        Globals.DESKTOP_WIDTH = rect.width()
        Globals.DESKTOP_HEIGHT = rect.height()
        if(size.width() < Globals.INIT_WIDTH or size.height() < Globals.INIT_HEIGHT):
            Globals.REMEMBER_HEIGHT = Globals.MINIHIGHT
            Globals.REMEMBER_WIDTH = Globals.MINIWIDTH
            if (self.title_bar.max_button.isHidden() == True):
                self.slot_click_max_btn()
            else:
                self.re_move(Globals.MINIWIDTH, Globals.MINIHIGHT)
                self.setGeometry(int(self.x()), int(self.y()), Globals.MINIWIDTH, Globals.MINIHIGHT)

        else:
            Globals.REMEMBER_HEIGHT = Globals.INIT_HEIGHT
            Globals.REMEMBER_WIDTH = Globals.INIT_WIDTH
            if (self.title_bar.max_button.isHidden() == True):
                self.slot_click_max_btn()
            else:
                self.re_move(Globals.INIT_WIDTH, Globals.INIT_HEIGHT)
                self.setGeometry(int(self.x()), int(self.y()), Globals.INIT_WIDTH, Globals.INIT_HEIGHT)

    # 将图片切割成圆角
    def PixmapToRound(self, src, x, y):
        pixmapa = QPixmap(src)
        pixmap = QPixmap(x, y)
        pixmap.fill(Qt.transparent)
        painter = QPainter(pixmap)
        painter.setRenderHint(QPainter.Antialiasing, True)
        # painter.setRenderHints(QPainter.Antialiasing | QPainter.SmoothPixmapTransform)
        path = QPainterPath()
        rect = QRectF(0, 0, x, y)
        path.addRoundedRect(rect, 10.0, 10.0)
        painter.setClipPath(path)
        painter.drawPixmap(0, 0, x, y, pixmapa)

        return pixmap

    #
    # 函数：最小化界面
    #
    def slot_click_min_button(self):
        if (hasattr(self, "title_bar")):
            self.title_bar.search_line.setFocus(False)
            self.title_bar.search_line.clearFocus()
        if (hasattr(self.title_bar, "exit_login_btn")):
            self.title_bar.exit_login_btn.hide()
        self.title_bar.senior_search.hide()
        if (hasattr(self, "search_tip_widget")):
            self.search_tip_widget.hide()
        self.showMinimized()

    #
    # 函数：点击最大化按钮
    #
    def slot_click_max_btn(self):
        # self.showMaximized()
        # self.hide()
        desktop = QApplication.desktop()
        rect = desktop.availableGeometry()
        if (Globals.REMEMBER_WIDTH != rect.width() and Globals.REMEMBER_HEIGHT != rect.height()):
            Globals.REMEMBER_WIDTH = self.width()
            Globals.REMEMBER_HEIGHT = self.height()
        Globals.STATUS = 0
        # print(rect)
        # self.setGeometry(rect)
        # self.show_max_window()
        self.showMaximized()

    def show_max_window(self):
        # self.hide()
        self.hide_tip_widget()
        self.space = 0
        self.title_bar.max_button.hide()
        if (Globals.THEME_COLOR == 0):
            self.title_bar.recovery_button.setStyleSheet(
                "QPushButton{border-radius: 4px;border:0px;border-image:url('res/recovery1.png')}")
        else:
            self.title_bar.recovery_button.setStyleSheet(
                "QPushButton{border-radius: 4px;border:0px;border-image:url('res/common-restore-white.png')}")
        self.title_bar.recovery_button.show()
        self.title_frame_widget.setGeometry(QtCore.QRect(0, 0, self.width(), 86))
        self.title_bar.resize(self.width(), 86)
        self.frame_widget.setGeometry(QtCore.QRect(0, self.title_frame_widget.y()+self.title_frame_widget.height()-1, self.width(), self.height() - 86))
        self.detailwidget.proportion = self.frame_widget.width() / (Globals.INIT_WIDTH - self.space * 2)
        self.detailwidget.resize(self.frame_widget.width(), self.frame_widget.height() - self.space)
        self.detailwidget.move(0, 0)
        self.title_bar.re_move()
        self.detailwidget.re_move()
        self.layout().setContentsMargins(0, 0, 0, 0)
        if (hasattr(self, "search_tip_widget")):
            self.search_tip_widget.move(self.title_bar.search_label.x(),
                                        self.title_bar.search_label.height() + self.title_bar.search_label.y())
        # # self.hide()
        # self.show()

    def slot_click_recovery_btn(self):
        Globals.STATUS = 1
        self.showNormal()
        # Globals.STATUS = 0
        # desktop = QApplication.desktop()
        # rect = desktop.availableGeometry()
        # print("UUUUUUUUUUUU",Globals.REMEMBER_WIDTH,Globals.REMEMBER_HEIGHT)
        # if(Globals.REMEMBER_WIDTH < Globals.MINIWIDTH or Globals.REMEMBER_HEIGHT < Globals.MINIHIGHT):
        #     self.setGeometry(0, 0, Globals.MINIWIDTH, Globals.MINIHIGHT)
        # else:
        #     screen = QDesktopWidget().screenGeometry()
        #     if(Globals.REMEMBER_WIDTH != rect.width() and  Globals.REMEMBER_HEIGHT != rect.height()):
        #         self.setGeometry(0, 0, Globals.REMEMBER_WIDTH, Globals.REMEMBER_HEIGHT)
        #     else:
        #         pass
        # print("pys333", self.width(), self.height())
        # self.show_recovery_window()

    def show_recovery_window(self):
        self.hide_tip_widget()
        desktop = QApplication.desktop()
        rect = desktop.availableGeometry()
        # self.move(int((rect.width() - self.width()) / 2), int((rect.height() - self.height()) / 2))
        if (Globals.HAVE_THEME):
            self.space = 3
        else:
            self.space = 5
        self.title_bar.max_button.show()
        self.title_bar.recovery_button.hide()
        self.title_frame_widget.setGeometry(QtCore.QRect(self.space, self.space, self.width() - self.space * 2, 86))
        self.title_bar.resize(self.width() - self.space * 2, 86)
        self.frame_widget.setGeometry(
            QtCore.QRect(self.space, self.title_frame_widget.y()+self.title_frame_widget.height()-1, self.width() - self.space * 2, self.height() - self.space - 86))
        self.detailwidget.proportion = self.frame_widget.width() / (Globals.INIT_WIDTH - self.space * 2)
        self.detailwidget.resize(self.frame_widget.width(), self.frame_widget.height() - self.space)
        self.title_bar.re_move()
        self.detailwidget.re_move()
        self.detailwidget.move(0, 0)
        if (hasattr(self, "search_tip_widget")):
            self.search_tip_widget.move(self.title_bar.search_label.x(),
                                        self.title_bar.search_label.height() + self.title_bar.search_label.y())

        Globals.REMEMBER_WIDTH = self.width()
        Globals.REMEMBER_HEIGHT = self.height()
        if (self.isMaximized() and Globals.DESKTOP_HAS_CHANGE == True):
            self.show_max_window()

    #
    # 函数：修改界面大小
    #
    def re_move(self, w, h):
        # pass
        self.title_frame_widget.setGeometry(QtCore.QRect(self.space, self.space, w - self.space * 2, 86))
        self.title_bar.setGeometry(self.title_bar.x(), self.title_bar.y(), w - self.space * 2, 86)
        self.frame_widget.setGeometry(QtCore.QRect(self.space, self.title_frame_widget.y()+self.title_frame_widget.height()-1, w - self.space * 2, h - self.space - 86))
        # self.frame_widget.resize(w- self.space * 2, h - self.space * 2 - 88)
        value1 = self.detailwidget.scrollArea.verticalScrollBar().value()
        value2 = self.detailwidget.scrollArea2.verticalScrollBar().value()
        self.detailwidget.proportion = self.frame_widget.width() / (Globals.INIT_WIDTH - self.space * 2)
        self.detailwidget.setGeometry(0, 0, self.frame_widget.width(), self.frame_widget.height())
        # if(self.title_bar.home_page_btn.isHidden() == False):
        self.title_bar.re_move()
        self.detailwidget.re_move()
        # self.detailwidget.move(0, self.space-2)
        self.detailwidget.scrollArea.verticalScrollBar().setValue(value1)
        self.detailwidget.scrollArea2.verticalScrollBar().setValue(value2)
        if (hasattr(self, "search_tip_widget")):
            self.search_tip_widget.move(self.title_bar.search_label.x(),
                                        self.title_bar.search_label.height() + self.title_bar.search_label.y())


    #
    # 函数：点击关闭
    #
    def slot_click_close_button(self):
        # item = {}
        # item["action"] = "save_db_exposure_rate"
        # self.at.mutex.acquire()
        # self.at.work_list.append(item)
        # self.at.mutex.release()
        all_suspend = True
        for card in self.detailwidget.mywidget.downloading_widget.children():
            if(card.down_progress.btn.isHidden() == False):
                all_suspend = False
        if (all_suspend == True and self.remove_app_num <= 0):
            self.exit_app()
        else:
            self.show_tip_widget(_("There are still tasks in progress. Are you sure you want to exit?"), True, True)
            self.tip_widget.btn.clicked.connect(self.slot_sure_close)

    def check_exit_deamon(self):
        # try:
        #     fcntl.flock(Globals.APT_LOCK_FILE, fcntl.LOCK_UN | fcntl.LOCK_NB)
        # except:
        #     pass
        self.click_close_btn = True
        try:
            try:
                os.system("pkill -f -9 kylin-software-center/backend/run_app/start_run_dbus.py")
            except:
                pass
            self.fd = open(Globals.APT_LOCK_FILE, "r")
            try:
                fcntl.flock(self.fd.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)
                try:
                    fcntl.flock(Globals.APT_LOCK_FILE, fcntl.LOCK_UN | fcntl.LOCK_NB)
                except:
                    pass
                self.init_dbus_thread.backend.clear_dbus_worklist()
                self.init_dbus_thread.backend.uksc_exit()
                try:
                    if (hasattr(self, "fd") and os.path.exists(Globals.APT_LOCK_FILE)):
                        fcntl.flock(self.fd.fileno(), fcntl.LOCK_UN | fcntl.LOCK_NB)
                        self.fd.close()
                        os.system("chmod 666 " + Globals.APT_LOCK_FILE)
                except:
                    if (hasattr(self, "fd")):
                        self.fd.close()
                    if (os.path.exists("/tmp/lock/kylin-update.lock")):
                        os.popen("rm /tmp/lock/kylin-update.lock")
                    pass
            except:
                is_ksc, is_kum, is_ki, read_list ,is_start_remote= self.check_lock_file()
                if(is_start_remote == True):
                    pass
                else:
                    try:
                        fcntl.flock(Globals.APT_LOCK_FILE, fcntl.LOCK_UN | fcntl.LOCK_NB)
                    except:
                        pass
                    self.init_dbus_thread.backend.exit_uksc_apt_daemon()
                    self.init_dbus_thread.backend.exit_dbus()
        except:
            pass

    def exit_app(self):
        # item = {}
        # item["action"] = "save_db_exposure_rate"
        # self.at.mutex.acquire()
        # self.at.work_list.append(item)
        # self.at.mutex.release()
        # self.at.appmgr.db.save_db_exposure_rate()
        # self.at.appmgr.db.save_new_arrivals_exposure_rate()
        self.check_exit_deamon()
        try:
            self.down_load_progress.terminate()
        except Exception as e:
            pass

        self.close()
        # self.unlock_file()
        self.unlock_uksc_lockfile()
        sys.exit()

    #
    # 函数：处理确认关闭
    #
    def slot_sure_close(self):
        self.remove_app_num = 0
        self.clear_all_card(self.detailwidget.mywidget.downloading_widget)
        self.exit_app()

    #
    # 函数：处理窗口状态改变时影藏菜单界面
    #
    def changeEvent(self, event):
        if (hasattr(self, "title_bar")):
            self.title_bar.search_line.setFocus(False)
            self.title_bar.search_line.clearFocus()
            self.title_bar.exit_login_btn.hide()
            self.title_bar.senior_search.hide()
        if (hasattr(self, "search_tip_widget")):
            self.search_tip_widget.hide()
        if (event.type() == QEvent.ActivationChange):
            if (hasattr(self.title_bar, "range_selection_widget")):
                if (QApplication.activeWindow() != self and QApplication.activeWindow() != self.title_bar.range_selection_widget):
                    self.title_bar.range_selection_widget.hide()
                else:
                    QApplication.setActiveWindow(self)
            # if(hasattr(self,"menubar")):
            #     if(hasattr(self,"second_lavel_menubar")):
            #         if(QApplication.activeWindow()!=self and QApplication.activeWindow()!=self.menubar and QApplication.activeWindow()!=self.second_lavel_menubar):
            #             self.menubar.hide()
            #             self.second_lavel_menubar.hide()
            #         else:
            #             QApplication.setActiveWindow(self)
            #     else:
            #         if (QApplication.activeWindow()!=self and QApplication.activeWindow() != self and QApplication.activeWindow() != self.menubar):
            #             self.menubar.hide()
            #         else:
            #             QApplication.setActiveWindow(self)

    ##########################################################3
    #
    # 函数名: 评分就绪
    # Function: rating ready
    #
    def slot_rating_reviews_ready(self, rnrlist):
        if (Globals.DEBUG_SWITCH):
            # LOG.debug("receive ratings and reviews ready, count is %d", len(rnrlist))
            Globals.LOG_INFO.logger.info(_("receive ratings and reviews ready, count is %d :" + str(len(rnrlist))))

        self.rnr_ready = True

    #
    # 函数名: 评论就绪
    # Function: reviews ready
    #
    def slot_app_reviews_ready(self, total, reviewlist, pkgname):

        # review_list = reviewlist
        self.detailwidget.app_detail_widget.get_reviewlist_to_init_review_ui(total, reviewlist, pkgname)

        # Globals.ALL_REVIEWLIST.clear()
        # self.detailwidget.app_detail_widget.show()

    ###############################################################

    # 函数：重绘函数
    #
    #
    def set_paintEvent(self, event):
        painter = QPainter(self)
        m_defaultBackgroundColor = QColor(qRgba(192, 192, 192, 1))
        m_defaultBackgroundColor.setAlpha(50)
        path = QPainterPath()
        path.setFillRule(Qt.WindingFill)
        path.addRoundedRect(self.space, self.space, self.width() - 20, self.height() - 20, 12, 12)

        painter.setRenderHint(QPainter.Antialiasing, True)
        painter.fillPath(path, QBrush(QColor(m_defaultBackgroundColor.red(),
                                             m_defaultBackgroundColor.green(),
                                             m_defaultBackgroundColor.blue(),
                                             m_defaultBackgroundColor.alpha())))

        color = QColor(0, 0, 0, 16)
        i = 0
        while i < 4:
            path = QPainterPath()
            path.setFillRule(Qt.WindingFill)
            path.addRoundedRect(self.space - i, self.space - i, self.width() - (self.space - i) * 2,
                                self.height() - (self.space - i) * 2, 12, 12)
            color.setAlpha(int(100 - math.sqrt(i) * 50))
            painter.setPen(color)
            painter.drawPath(path)
            i = i + 1

        painter.setRenderHint(QPainter.Antialiasing)

    def window_move_to(self, pos):
        if self.windowState() == Qt.WindowMaximized or self.windowState() == Qt.WindowFullScreen:
            # 最大化或者全屏则不允许移动
            return
        self.setWindowOpacity(1)
        self.move(pos)

    #
    # 函数：实现无标题栏窗口拖动
    #
    def mousePressEvent(self, event):
        if (event.button() == Qt.LeftButton):
            self._mpos = event.pos()
            if ((0 <= self._mpos.x() <= self.width() and 0 <= self._mpos.y() <= 7) or (
                    0 <= self._mpos.x() <= self.width() and self.height() - 7 <= self._mpos.y() <= self.height()) or (
                    0 <= self._mpos.x() <= 7 and 0 <= self._mpos.y() <= self.height()) or (
                    self.width() - 7 <= self._mpos.x() <= self.width() and 0 <= self._mpos.y() <= self.height())):
                self.resize_window = True
            else:
                self.move_window = True
            self.title_bar.exit_login_btn.hide()
            self.title_bar.senior_search.hide()
            self.title_bar.exit_login_btn.setWindowModality(Qt.NonModal)
            if (hasattr(self, "title_bar")):
                self.title_bar.search_line.setFocus(False)
                self.title_bar.search_line.clearFocus()
            if (hasattr(self, "search_tip_widget")):
                self.search_tip_widget.hide()
            if (hasattr(self, "menubar")):
                self.menubar.hide()
            if (hasattr(self, "second_lavel_menubar")):
                self.second_lavel_menubar.hide()
            if (hasattr(self.title_bar, "range_selection_widget")):
                self.title_bar.range_selection_widget.hide()
            self.mouse_press = True
            self.drag_pos = event.globalPos() - self.pos()
            event.accept()

    def mouseReleaseEvent(self, event):
        self.mouse_press = False
        self.move_window = False
        self.mouse_release = True
        self.resize_window = False
        self.setCursor(Qt.ArrowCursor)
        self.setWindowOpacity(1)

    def mouseDoubleClickEvent(self, e):
        if (Globals.STATUS):
            self.slot_click_recovery_btn()
        else:
            self.slot_click_max_btn()

    def mouseMoveEvent(self, event):
        pos = event.pos()
        xPos, yPos = pos.x(), pos.y()
        wm, hm = self.width() - self.Margins, self.height() - self.Margins
        if self.title_bar.max_button.isHidden():
            if(hasattr(self,"Direction")):
                self.Direction = None
            self.setCursor(Qt.ArrowCursor)
            return
        else:
            desktop = QApplication.desktop()
            rect = desktop.availableGeometry()
            if (Globals.REMEMBER_WIDTH != rect.width() or Globals.REMEMBER_HEIGHT != rect.height()):
                Globals.REMEMBER_WIDTH = self.width()
                Globals.REMEMBER_HEIGHT = self.height()
        if event.buttons() == Qt.LeftButton and self.mouse_press:
            if (self.resize_window == True):
                if (self.finish_resize):
                    self.finish_resize = False
                    self._resizeWidget(event.pos())
                return
        if xPos <= self.Margins and yPos <= self.Margins:
            # 左上角
            self.Direction = LeftTop
            self.setCursor(Qt.SizeFDiagCursor)
        elif wm <= xPos <= self.width() and hm <= yPos <= self.height():
            # 右下角
            self.Direction = RightBottom
            self.setCursor(Qt.SizeFDiagCursor)
        elif wm <= xPos and yPos <= self.Margins:
            # 右上角
            self.Direction = RightTop
            self.setCursor(Qt.SizeBDiagCursor)
        elif xPos <= self.Margins and hm <= yPos:
            # 左下角
            self.Direction = LeftBottom
            self.setCursor(Qt.SizeBDiagCursor)
        elif 0 <= xPos <= self.Margins and self.Margins <= yPos <= hm:
            # 左边
            self.Direction = Left
            self.setCursor(Qt.SizeHorCursor)
        elif wm <= xPos <= self.width() and self.Margins <= yPos <= hm:
            # 右边
            self.Direction = Right
            self.setCursor(Qt.SizeHorCursor)
        elif self.Margins <= xPos <= wm and 0 <= yPos <= self.Margins:
            # 上面
            self.Direction = Top
            self.setCursor(Qt.SizeVerCursor)
        elif self.Margins <= xPos <= wm and hm <= yPos <= self.height():
            # 下面
            self.Direction = Bottom
            self.setCursor(Qt.SizeVerCursor)
        else:
            self.setCursor(Qt.ArrowCursor)
        # if (self.mouse_press and self.move_window == True):
        #     device = QGuiApplication.primaryScreen().devicePixelRatio()
        #     self.move_pos = event.globalPos()
        #     pos = self.move_pos - self.drag_pos
        #     self.move(int(pos.x()*device) , int(pos.y()*device))
        #     # self.move(int(pos.x()) , int(pos.y()))
        #     self.setWindowOpacity(1)
        #     event.accept()



    def _resizeWidget(self, pos):
        """调整窗口大小"""
        if hasattr(self,"Direction") and self.Direction == None:
            self.finish_resize = True
            return
        desktop = QApplication.desktop()
        rect = desktop.availableGeometry()
        mpos = pos - self._mpos
        xPos, yPos = mpos.x(), mpos.y()
        geometry = self.geometry()
        x, y, w, h = geometry.x(), geometry.y(), geometry.width(), geometry.height()
        if self.Direction == LeftTop:  # 左上角
            if ((xPos > self.Margins or xPos < 0 - self.Margins) or (yPos > self.Margins or yPos < 0 - self.Margins)):
                if w - xPos > Globals.MINIWIDTH:
                    x += xPos
                    w -= xPos
                if h - yPos > Globals.MINIHIGHT:
                    y += yPos
                    h -= yPos
        elif self.Direction == RightBottom:  # 右下角
            if ((xPos > self.Margins or xPos < 0 - self.Margins) or (yPos > self.Margins or yPos < 0 - self.Margins)):
                if w + xPos > Globals.MINIWIDTH:
                    w += xPos
                    self._mpos = pos
                if h + yPos > Globals.MINIHIGHT:
                    h += yPos
                    self._mpos = pos
        elif self.Direction == RightTop:  # 右上角
            if ((yPos > self.Margins or yPos < 0 - self.Margins) or (xPos > self.Margins or xPos < 0 - self.Margins)):
                if h - yPos > Globals.MINIHIGHT:
                    y += yPos
                    h -= yPos
                if w + xPos > Globals.MINIWIDTH:
                    w += xPos
                    self._mpos.setX(pos.x())
        elif self.Direction == LeftBottom:  # 左下角
            if ((yPos > self.Margins or yPos < 0 - self.Margins) or (xPos > self.Margins or xPos < 0 - self.Margins)):
                if w - xPos > Globals.MINIWIDTH:
                    x += xPos
                    w -= xPos
                if h + yPos > Globals.MINIHIGHT:
                    h += yPos
                    self._mpos.setY(pos.y())
        elif self.Direction == Left:  # 左边
            if w - xPos > Globals.MINIWIDTH and (xPos > self.Margins or xPos < 0 - self.Margins):
                x += xPos
                w -= xPos
            else:
                self.finish_resize = True
                return
        elif self.Direction == Right:  # 右边
            if w + xPos > Globals.MINIWIDTH and (xPos > self.Margins or xPos < 0 - self.Margins):
                w += xPos
                self._mpos = pos
            else:
                self.finish_resize = True
                return
        elif self.Direction == Top:  # 上面
            if h - yPos > Globals.MINIHIGHT and (yPos > self.Margins or yPos < 0 - self.Margins):
                y += yPos
                h -= yPos
            else:
                self.finish_resize = True
                return
        elif self.Direction == Bottom:  # 下面
            if h + yPos > Globals.MINIHIGHT and (yPos > self.Margins or yPos < 0 - self.Margins):
                h += yPos
                self._mpos = pos
            else:
                self.finish_resize = True
                return
        value1 = self.detailwidget.scrollArea.verticalScrollBar().value()
        value2 = self.detailwidget.scrollArea2.verticalScrollBar().value()
        # self.resize(w, h)
        # self.move(x, y)
        if (w > rect.width() or h > rect.height()):
            pass
        else:
            self.setGeometry(x, y, w, h)
            # self.re_move(w, h)
        # if xPos == 0 :
        #     pass
        # else:
        #     if(w <= rect.width() and h <=rect.height()):
        #         self.re_move(w, h)
        #         self.setGeometry(x, y, w, h)
        #         Globals.REMEMBER_WIDTH = self.width()
        #         Globals.REMEMBER_HEIGHT = self.height()
        #         # self.detailwidget.scrollArea.verticalScrollBar().setValue(value1)
        #         # self.detailwidget.scrollArea2.verticalScrollBar().setValue(value2)
        #     elif (w > rect.width() and h <= rect.height()):
        #         self.re_move(rect.width(), h)
        #         self.setGeometry(x, y, rect.width(), h)
        #         Globals.REMEMBER_WIDTH = self.width()
        #         Globals.REMEMBER_HEIGHT = self.height()
        #         # self.detailwidget.scrollArea.verticalScrollBar().setValue(value1)
        #         # self.detailwidget.scrollArea2.verticalScrollBar().setValue(value2)
        #     elif (w <= rect.width() and h > rect.height()):
        #         self.re_move(w, rect.height())
        #         self.setGeometry(x, y, w, rect.height())
        #         Globals.REMEMBER_WIDTH = self.width()
        #         Globals.REMEMBER_HEIGHT = self.height()
        #         # self.detailwidget.scrollArea.verticalScrollBar().setValue(value1)
        #         # self.detailwidget.scrollArea2.verticalScrollBar().setValue(value2)
        #     else:
        #         Globals.REMEMBER_WIDTH = self.width()
        #         Globals.REMEMBER_HEIGHT = self.height()

        self.finish_resize = True

    def hoverMoveEvent(self, Q):
        self.setCursor(Qt.ArrowCursor)

    # def closeEvent(self,event):
    #     pass

    def keyPressEvent(self, keyevent):
        key = keyevent.key()
        if(key == Qt.Key_Down or key == Qt.Key_Up):
            if(hasattr(self,"search_tip_widget") and self.search_tip_widget.isHidden() == False):
                self.search_tip_widget.setFocus()
                if (key == Qt.Key_Up):
                    if (self.search_tip_widget.enter_item == None):
                        self.search_tip_widget.key_up()
                elif (key == Qt.Key_Down):
                    self.search_tip_widget.key_down()
        elif keyevent.key() == Qt.Key_F1:
            Dbus_Service.show_user_guide("software-store")

    def eventFilter(self, obj, event):
        if (obj == self):
            if (event.type() == event.Close):
                event.ignore()
                self.slot_click_close_button()
                return True
            if (event.type() == event.MouseButtonRelease):
                self.hide_tip_widget()
        if (hasattr(self, "frame_widget")):
            if (obj == self.frame_widget):
                if (event.type() == event.MouseButtonPress):
                    self.hide_tip_widget()
        if isinstance(event, QEnterEvent):
            self.setCursor(Qt.ArrowCursor)
        if (obj == self):
            if (event.type() == event.Resize):
                if(self.is_first_show_widget == True):
                    if self.isMaximized() and Globals.STATUS == 0:
                        self.show_max_window()
                        Globals.STATUS = 1
                        self.mouse_release = False
                        return False
                    elif(self.width() == Globals.DESKTOP_WIDTH and self.height() == Globals.DESKTOP_HEIGHT):
                        self.show_max_window()
                        Globals.STATUS = 1
                        self.mouse_release = False
                        return False
                    else:
                        if (Globals.STATUS == 1):
                            Globals.STATUS = 0
                            self.mouse_release = False
                            self.show_recovery_window()
                            return False
                        else:
                            if(hasattr(self,"detailwidget")):
                                self.re_move(self.width(),self.height())
                else:
                    self.is_first_show_widget = False
            # elif(event.type() == event.MouseMove):
            #     self.re_mouseMoveEvent(event)
        # window = QApplication.activeWindow()
        # if (QApplication.activeWindow() != self):
        #     if (hasattr(self, "menubar")):
        #         if (QApplication.activeWindow() != self.menubar):
        #             if (hasattr(self, "second_lavel_menubar")):
        #                 if (QApplication.activeWindow() != self.second_lavel_menubar):
        #                     print("22222")
        #                     self.hide_tip_widget()
        #             else:
        #                 print("33333")
        #                 self.hide_tip_widget()
        #     else:
        #         print("44444")
        #         self.hide_tip_widget()
        return False

    def hide_tip_widget(self):
        if (hasattr(self, "title_bar")):
            # self.title_bar.search_line.setFocus(Falsm
            self.title_bar.exit_login_btn.hide()
            self.title_bar.senior_search.hide()
            self.title_bar.exit_login_btn.setWindowModality(Qt.NonModal)
            if (hasattr(self.title_bar, "range_selection_widget")):
                self.title_bar.range_selection_widget.hide()
        if (hasattr(self, "search_tip_widget")):
            self.search_tip_widget.hide()
        if (hasattr(self, "menubar")):
            self.menubar.hide()
        if (hasattr(self, "second_lavel_menubar")):
            self.second_lavel_menubar.hide()

    # 手动解锁单例文件锁
    def unlock_uksc_lockfile(self):
        if(Globals.LOCAL_SERVER != None):
            localserver = Globals.LOCAL_SERVER
            localserver.close()
        pidfile = Globals.PIDFILE
        try:
            fcntl.flock(pidfile, fcntl.LOCK_UN | fcntl.LOCK_NB)
        except Exception as e:
            Globals.LOG_ERROR.logger.error("unlock the file lock failed : %s" % str(e))

