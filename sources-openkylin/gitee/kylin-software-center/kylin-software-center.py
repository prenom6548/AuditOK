#!/usr/bin/python3
# -*- coding: utf-8 -*-

from PyQt5.QtGui import *
from PyQt5.QtCore import *
from PyQt5 import QtCore, QtGui, QtWidgets
from ui.MainWindow import *
from ui.messagebox import *
from ui.download_ranking_card import *
import os
import fcntl
from PyQt5.QtNetwork import QLocalServer,QLocalSocket
import dbus
import sys
import signal
from models.globals import Globals
import logging
#LOG = logging.getLogger(__name__)

import gettext
gettext.textdomain("kylin-software-center")
_ = gettext.gettext
try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    def _fromUtf8(s):
        return s

#
#函数名：单例
#
#
pidfile=0
def app_instance():

    serverName = "kylinsoftwarecenter"
    socket = QLocalSocket()
    socket.connectToServer(serverName)
    if socket.waitForConnected(500):
        try:
            for i in os.popen("wmctrl -l |grep " + _("Software Store")):
                wm = i.split()
                if (_("Software Store") in wm):
                    os.system("wmctrl -i -R " + wm[0])
        except:
            pass
        if (Globals.DEBUG_SWITCH):
            print('Software Store has been started.')
        sys.exit(0)
    global  localserver
    localserver = QLocalServer()
    localserver.listen(serverName)
    Globals.LOCAL_SERVER = localserver
    global pidfile
    homePath = QStandardPaths.standardLocations(QStandardPaths.HomeLocation)
    pidfile = open(str(homePath[0]+ "/.config/kylin-software-center%1.lock").replace("%1",os.getenv("DISPLAY")),"w+")
    Globals.PIDFILE = pidfile
    try:
        fcntl.flock(pidfile, fcntl.LOCK_EX | fcntl.LOCK_NB)
    except:
        try:
            for i in os.popen("wmctrl -l |grep " + _("Software Store")):
                wm = i.split()
                if (_("Software Store") in wm):
                    os.system("wmctrl -i -R " + wm[0])
        except:
            pass
        # app.setActiveWindow()
        if (Globals.DEBUG_SWITCH):
            print('Software Store has been started.')
        sys.exit(0)

#
#函数名: 本地包文件
#Function: local debfile
#
def check_local_deb_file(url):
    return os.path.isfile(url)

#
#函数名: 退出
#Function: quit
#
def quit(signum, frame):
    if (Globals.DEBUG_SWITCH):
        print('You choose to stop software-center.')

    # 尝试解锁
    if (Globals.DEBUG_SWITCH):
        Globals.LOG_INFO.logger.info("unlock the file lock.")
    pidfile = Globals.PIDFILE
    try:
        fcntl.flock(pidfile, fcntl.LOCK_UN | fcntl.LOCK_NB)
    except Exception as e:
        Globals.LOG_ERROR.logger.error("unlock the file lock failed : %s" % str(e))
        
    sys.exit()

#
#函数名: 主函数
#Function: main function
#
def main():
    # print("----start-----",datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
    #app = QApplication(sys.argv)
    app_instance()
    try:
        conf = configparser.ConfigParser()
        conf.read(CONF_PATH)
        SHA_TZ = datetime.timezone(datetime.timedelta(hours=8), name='Asia/Shanghai', )
        # 协调世界时
        utc_now = datetime.datetime.utcnow().replace(tzinfo=datetime.timezone.utc)
        beijing_now = utc_now.astimezone(SHA_TZ)
        now_time = str(beijing_now).split(".")[0]
        startup_clients_time = conf.set("essential-information", "start_client_time",now_time)
        with open(CONF_PATH, "w+") as f:
            conf.write(f)
    except:
        pass
    try:
        cmd = "dpkg -l |grep libqt5gui |awk '{print $3}'"
        version = os.popen(cmd).readline().strip("\n")
        if version.startswith("5.6"):
            if ("~" in version):
                if ("k" in version.split("~")[1]):
                    if (int(version.split("~")[1].split("k")[1]) >= 4):
                        QCoreApplication.setAttribute(Qt.AA_EnableHighDpiScaling)
                        QCoreApplication.setAttribute(Qt.AA_UseHighDpiPixmaps)
        else:
            QCoreApplication.setAttribute(Qt.AA_EnableHighDpiScaling)
            QCoreApplication.setAttribute(Qt.AA_UseHighDpiPixmaps)
    except:
        QCoreApplication.setAttribute(Qt.AA_EnableHighDpiScaling)
        QCoreApplication.setAttribute(Qt.AA_UseHighDpiPixmaps)
    app = QApplication(sys.argv)

    app.setStyle("ukui")
    app.setWindowIcon(QIcon.fromTheme("kylin-sotfware-center"))
#   #QTextCodec.setCodecForTr(QTextCodec.codecForName("UTF-8"))
#   #QTextCodec.setCodecForCStrings(QTextCodec.codecForName("UTF-8"))

    globalfont = QFont()
    # globalfont.setFamily("")
    # 文泉驿微米黑
    # 文泉驿等宽微米黑
    # 方正书宋_GBK
    # 方正仿宋_GBK
    # 方正姚体_GBK
    # 方正宋体S-超大字符集
    # 方正宋体S-超大字符集(SIP)
    # 方正小标宋_GBK
    # 方正楷体_GBK
    # 方正细黑一_GBK
    # 方正行楷_GBK
    # 方正超粗黑_GBK
    # 方正隶书_GBK
    # 方正魏碑_GBK
    # 方正黑体_GBK
    globalfont.setPointSize(11)
    app.setFont(globalfont)

    trans = QTranslator()
    trans.load('po/qt_zh_CN.qm')
    QApplication.installTranslator(trans)
    trans2 = QTranslator()
    trans2.load('po/widgets_zh_CN.qm')
    QApplication.installTranslator(trans2)

    # check show quiet
    argn = len(sys.argv)
    if(argn == 1):
        Globals.LAUNCH_MODE = 'normal'
    elif(argn > 1):
        arg = sys.argv[1]
        if(arg == '-quiet'):
            Globals.LAUNCH_MODE = 'quiet'
        elif(arg == '-remove'):
            if(sys.argv[2]):
                Globals.LAUNCH_MODE = 'normal'
                Globals.REMOVE_SOFT = sys.argv[2]
            else:
                if (Globals.LOCAL_SERVER != None):
                    localserver = Globals.LOCAL_SERVER
                    localserver.close()
                sys.exit(0)
        elif(arg == '-find'):
            if (sys.argv[2]):
                Globals.SEARCH_SOFTWARE = sys.argv[2]
            else:
                if (Globals.LOCAL_SERVER != None):
                    localserver = Globals.LOCAL_SERVER
                    localserver.close()
                sys.exit(0)
        else:
            Globals.LAUNCH_MODE = 'normal'
            if(check_local_deb_file(arg)):
                Globals.LOCAL_DEB_FILE = arg
                '''MessageBox = File_window()
                MessageBox.setText(_("Opening files is not supported"))
                MessageBox.exec()'''
                if (Globals.LOCAL_SERVER != None):
                    localserver = Globals.LOCAL_SERVER
                    localserver.close()
                sys.exit(0)
            else:
                if (Globals.LOCAL_SERVER != None):
                    localserver = Globals.LOCAL_SERVER
                    localserver.close()
                sys.exit(0)
    # app_instance()
    # app.setStyle("ukui")
    # print("init_mw", datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
    mw = MainWindow()
    # print("init_mw_over", datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
    # app.desktop().resized.connect(mw.show_new_widget)
    windowWidth = QApplication.desktop().screenGeometry(0).width()
    qScreen = QApplication.primaryScreen()
    qScreen.availableGeometryChanged.connect(mw.show_new_widget)
    windowHeight = QApplication.desktop().screenGeometry(0).height()
    mw.move(int((windowWidth - mw.width()) / 2), int((windowHeight - mw.height()) / 2))
    # app.setStyle("ukui")

    sys.exit(app.exec_())
    os.system("pause")

if __name__ == '__main__':
    main()
