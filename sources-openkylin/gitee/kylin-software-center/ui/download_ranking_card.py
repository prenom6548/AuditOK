from PyQt5.QtGui import *
from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
from models.enums import  setLongTextToElideFormat,PkgStates,PKG_NAME
from models.apkinfo import *
from models.application import *
from utils import commontools
from ui.circular_progress_bar import *
from ui.uninstall_btn import *
from ui.shot_detail import *
import gettext
gettext.bindtextdomain("kylin-software-center", "/usr/share/locale")
gettext.textdomain("kylin-software-center")
_ = gettext.gettext

class Download_Ranking_card(QWidget):
    send_download_progress = pyqtSignal(int)
    puase_download = pyqtSignal(Application)
    puase_apk_download = pyqtSignal(ApkInfo)
    check_all_check_statue = pyqtSignal()

    def __init__(self, appmsg,num,ca,mw,parent=None):
        QWidget.__init__(self, parent)
        self.appmsg = appmsg
        self.app_dict = None
        self.pre_progress = 0
        self.num = num
        self.ca = ca
        self.mw = mw
        self.font = QFont()
        self.font.setPixelSize(14)
        self.m_font = QFontMetrics(self.font)
        self.narrow_btn_time = QTimer()
        self.init_ui()
        self.narrow_btn_time.timeout.connect(self.move_btn)
        self.mw.sigenal_show_downloading_widget.connect(self.show_downloading_widget)


    def init_ui(self):
        self.resize(237, 65)
        self.setWindowFlags(Qt.FramelessWindowHint)
        self.setAttribute(Qt.WA_TranslucentBackground)
        self.setStyleSheet("QFrame{border:0px solid #ff0000;border-radius: 12px;background-color:transparent;}")
        # self.setAutoFillBackground(True)
        # palette = QPalette()
        # palette.setColor(self.backgroundRole(), QColor(0, 255, 255))
        # self.setPalette(palette)

        self.icon_label = QLabel(self)
        self.icon_label.setFixedSize(48,48)
        self.icon_label.move(0,13)
        iconpath = commontools.get_big_icon_path(self.appmsg.pkgname)
        photo = Photo(iconpath, self.icon_label.width(),
                      self.icon_label.height(),
                      self.icon_label)
        photo.shot_photo_change(iconpath)
        # photo.resize_image()
        photo.show()
        self.icon_label.setStyleSheet("QLabel{border-radius: 8px;background-color:transparent;}")
        # self.icon_label.setStyleSheet("QLabel{border-radius: 8px;border-image: url('"+iconpath+"') 0 0 0 0 stretch strectch;background-color:transparent;}")

        self.num_label = QLabel(self)
        self.num_label.setAttribute(Qt.WA_TranslucentBackground)
        self.num_label.setFixedSize(10,14)
        self.num_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 500;color: #303133;}")
        self.num_label.setText(str(self.num))
        self.num_label.move(self.icon_label.x()+self.icon_label.width()+6,self.icon_label.y()+6)

        self.appname_label = QLabel(self)
        self.appname_label.setAttribute(Qt.WA_TranslucentBackground)
        self.appname_label.setAlignment(Qt.AlignVCenter)
        self.appname_label.resize(110,19)
        self.appname_label.setMaximumWidth(110)
        if(Globals.LAN == "zh_CN"):
            if self.appmsg.displayname_cn != '' and self.appmsg.displayname_cn is not None and self.appmsg.displayname_cn != 'None':
                setLongTextToElideFormat(self.appname_label, self.appmsg.displayname_cn)
                if ("…" in self.appname_label.text()):
                    self.appname_label.setToolTip(self.appmsg.displayname_cn)
            else:
                setLongTextToElideFormat(self.appname_label, self.appmsg.pkgname)
                if ("…" in self.appname_label.text()):
                    self.appname_label.setToolTip(self.appmsg.pkgname)
        else:
            setLongTextToElideFormat(self.appname_label, self.appmsg.pkgname)
            if ("…" in self.appname_label.text()):
                self.appname_label.setToolTip(self.appmsg.pkgname)
        # self.appname_label.adjustSize()
        self.appname_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 500;color: #303133;text-align: left;}")
        self.appname_label.move(self.num_label.x()+self.num_label.width()+6,self.num_label.y()-3)

        self.app_categories = QLabel(self)
        self.app_categories.setAttribute(Qt.WA_TranslucentBackground)
        self.app_categories.setFixedSize(65,15)
        self.app_categories.setAlignment(Qt.AlignVCenter)
        self.app_categories.setStyleSheet("QLabel{font-size: 12px;font-weight: 400;color: #8F9399;}")
        if(self.appmsg.category_name_cn !="" and self.appmsg.category_name_cn !="None" and self.appmsg.category_name_cn !=None  and Globals.LAN == "zh_CN"):
            self.app_categories.setText(self.appmsg.category_name_cn.split("、")[0])
        else:
            self.app_categories.setText(self.appmsg.category_name.split("、")[0])
        # self.app_categories.adjustSize()
        self.app_categories.move(self.appname_label.x(),self.appname_label.y()+self.appname_label.height()+8)

        self.btn = QPushButton(self)
        self.btn.setFocusPolicy(Qt.NoFocus)
        self.btn.setStyleSheet("QPushButton{font-size: 12px;font-weight: 400;color: #FFFFFF;line-height: 12px;border-radius: 11px;border: 0px solid #3790FA;background: #3790FA;}QPushButton:hover{background: #40A9FB;border-radius: 11px;color: #ffffff;font-weight: 400;border:0px;}QPushButton:pressed{background: #296CD9;border-radius: 11px;color: #ffffff;font-weight: 400;border:0px;}")
        self.btn.resize(48, 22)
        self.btn.move(int(self.width()-self.btn.width()),int((self.icon_label.height()-self.btn.height())/2) + self.icon_label.y())

        self.line_label = QLabel(self)
        self.line_label.resize(self.width(),1)
        # self.line_label.setStyleSheet("QLabel{border: 1px solid rgba(151,151,151,0.2);}")
        self.line_label.move(0,self.height()-2)
        self.line_label.show()

        self.down_progress = Circular_Progress_Bar(22,22,self.appmsg.pkgname,self)
        self.down_progress.update_progress(self.appmsg.percent)
        self.send_download_progress.connect(self.down_progress.GenUpdate)
        self.down_progress.btn.clicked.connect(self.slot_puase_download)
        self.down_progress.cont_btn.clicked.connect(lambda :self.mw.re_download_app(self.appmsg))
        self.down_progress.move(int(self.width()-self.down_progress.width()), self.btn.y())
        self.down_progress.hide()
        self.remove_btn = uninstall_btn(self)
        self.remove_btn.move(self.btn.x() + int((self.btn.width()-self.remove_btn.width())/2), self.btn.y())
        self.remove_btn.hide()
        self.slot_set_dark_mode()
        self.check_btn_text()

    #
    # 函数：设置深色模式
    #
    def slot_set_dark_mode(self):
        if (Globals.THEME_COLOR == 0):
            # self.setStyleSheet("QFrame{border:0px solid #ff0000;border-radius: 12px;background-color:#ffffff;}")
            self.num_label.setStyleSheet(
                "QLabel{font-size: 14px;font-weight: 500;color: #303133;line-height: 20px;}")
            self.appname_label.setStyleSheet(
                "QLabel{font-size: 14px;font-weight: 400;color: #303133;line-height: 17px;}")
            self.app_categories.setStyleSheet(
                "QLabel{font-size: 12px;font-weight: 400;color: #8F9399;line-height: 17px;}")
            self.line_label.setStyleSheet("QLabel{border: 1px solid rgba(151,151,151,0.2);}")
        else:
            # self.setStyleSheet("QFrame{background: rgba(46, 46, 46, 0.88);border-radius: 12px;}")
            self.num_label.setStyleSheet(
                "QLabel{font-size: 14px;font-weight: 500;color: #F9F9F9;line-height: 20px;}")
            self.appname_label.setStyleSheet(
                "QLabel{font-size: 14px;font-weight: 400;color: #F9F9F9;line-height: 17px;}")
            self.app_categories.setStyleSheet(
                "QLabel{font-size: 12px;font-weight: 400;color: #8F9399;line-height: 17px;}")
            self.line_label.setStyleSheet("QLabel{border: 1px solid #393939;}")
        self.down_progress.reflash_color()
        # QApplication.processEvents()
        # self.get_btn_color()

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

    def change_btn_width(self):
        s = self.m_font.boundingRect(self.btn.text()).size().width()
        if (s +18< 48):
            self.btn.setFixedSize(48,22)
        else:
            self.btn.setFixedSize(s+18,22)
        # print("00000",self.btn.width())
        self.btn.move(int(self.width()-self.btn.width()),int((self.icon_label.height()-self.btn.height())/2) + self.icon_label.y())

    def check_btn_text(self):
        self.btn.resize(48, 22)
        self.btn.move(int(self.width() - self.btn.width()),
                      int((self.icon_label.height() - self.btn.height()) / 2) + self.icon_label.y())
        self.remove_btn.stop_loading()
        self.remove_btn.hide()
        self.down_progress.hide()
        self.btn.setCheckable(True)
        self.btn.setEnabled(True)
        if (self.appmsg.status == PkgStates.REMOVING):
            self.slot_uninstall(self.appmsg.pkgname, "uninstalling")
            return
        if (type(self.appmsg) != ApkInfo):
            try:
                self.package = self.ca[self.appmsg.pkgname]
            except:
                self.package = None
            if (self.package != None):
                if (self.package.is_installed == True and self.package.is_upgradable == True):
                    self.btn.setText(_("update"))
                    self.change_btn_width()
                    self.btn.setStyleSheet("QPushButton{font-size: 12px;font-weight: 400;color: #3790FA;line-height: 12px;border-radius: 11px;background: rgba(55, 144, 250, 0.12);}QPushButton:hover{background: #3790FA;border-radius: 11px;color: #ffffff;font-weight: 400;border:0px;}QPushButton:pressed{background: #296CD9;border-radius: 11px;color: #ffffff;font-weight: 400;border:0px;}")
                elif (self.package.is_installed == False):
                    self.btn.setText(_("down"))
                    self.change_btn_width()
                    self.btn.setStyleSheet("QPushButton{font-size: 12px;font-weight: 400;color: #FFFFFF;line-height: 12px;border-radius: 11px;border: 0px solid #3790FA;background: #3790FA;}QPushButton:hover{background: #40A9FB;border-radius: 11px;color: #ffffff;font-weight: 400;border:0px;}QPushButton:pressed{background: #296CD9;border-radius: 11px;color: #ffffff;font-weight: 400;border:0px;}")
                elif (self.package.is_installed == True and self.package.is_upgradable == False):
                    if (self.appmsg.pkgname == "kylin-installer"):
                        has_desktop = False
                    else:
                        has_desktop = self.check_app_desktop(self.appmsg.pkgname)
                    if (has_desktop == True):
                        self.btn.setText(_("Open"))
                        self.change_btn_width()
                        self.btn.setStyleSheet("QPushButton{font-size: 12px;font-weight: 400;color: #3790FA;line-height: 12px;border-radius: 11px;background: rgba(55, 144, 250, 0.12);}QPushButton:hover{background: #3790FA;border-radius: 11px;color: #ffffff;font-weight: 400;border:0px;}QPushButton:pressed{background: #296CD9;border-radius: 11px;color: #ffffff;font-weight: 400;border:0px;}")
                    else:
                        self.btn.setText(_("installed"))
                        self.change_btn_width()
                        self.btn.setEnabled(False)
                        self.btn.setStyleSheet("QPushButton{font-size: 12px;font-weight: 400;color: #3790FA;line-height: 12px;background: rgba(55, 144, 250, 0.12);border-radius: 11px;}")
                    pass
            else:
                self.btn.setText(_("down"))
                self.change_btn_width()
                self.btn.setStyleSheet("QPushButton{font-size: 12px;font-weight: 400;color: #FFFFFF;line-height: 12px;border-radius: 11px;border: 0px solid #3790FA;background: #3790FA;}QPushButton:hover{background: #40A9FB;border-radius: 11px;color: #ffffff;font-weight: 400;border:0px;}QPushButton:pressed{background: #296CD9;border-radius: 11px;color: #ffffff;font-weight: 400;border:0px;}")
        else:
            if (self.appmsg != None):
                if (self.appmsg.is_installed == True and self.appmsg.is_upgradable == True):
                    self.btn.setText(_("update"))
                    self.change_btn_width()
                    self.btn.setStyleSheet("QPushButton{font-size: 12px;font-weight: 400;color: #3790FA;line-height: 12px;border-radius: 11px;background: rgba(55, 144, 250, 0.12);}QPushButton:hover{background: #3790FA;border-radius: 11px;color: #ffffff;font-weight: 400;border:0px;}QPushButton:pressed{background: #296CD9;border-radius: 11px;color: #ffffff;font-weight: 400;border:0px;}")
                elif (self.appmsg.is_installed == False):
                    self.btn.setText(_("down"))
                    self.change_btn_width()
                    self.btn.setStyleSheet("QPushButton{font-size: 12px;font-weight: 400;color: #FFFFFF;line-height: 12px;border-radius: 11px;border: 0px solid #3790FA;background: #3790FA;}QPushButton:hover{background: #40A9FB;border-radius: 11px;color: #ffffff;font-weight: 400;border:0px;}QPushButton:pressed{background: #296CD9;border-radius: 11px;color: #ffffff;font-weight: 400;border:0px;}")
                elif (self.appmsg.is_installed == True and self.appmsg.is_upgradable == False):
                    self.btn.setText(_("Open"))
                    self.change_btn_width()
                    self.btn.setStyleSheet("QPushButton{font-size: 12px;font-weight: 400;color: #3790FA;line-height: 12px;border-radius: 11px;background: rgba(55, 144, 250, 0.12);}QPushButton:hover{background: #3790FA;border-radius: 11px;color: #ffffff;font-weight: 400;border:0px;}QPushButton:pressed{background: #296CD9;border-radius: 11px;color: #ffffff;font-weight: 400;border:0px;}")
                    pass
            else:
                self.btn.setText(_("installed"))
                self.change_btn_width()
        self.btn.show()
        # self.get_btn_color()
    #
    #函数：获取btn的颜色
    #
    def get_btn_color(self):
        if(self.btn.text() == _("down")):
            if (Globals.THEME_COLOR == 0):
                self.btn.setStyleSheet("QPushButton{font-size: 12px;font-weight: 400;color: #FFFFFF;line-height: 12px;border-radius: 11px;border: 0px solid #3790FA;background: #3790FA;}QPushButton:hover{background: #40A9FB;border-radius: 11px;color: #ffffff;font-weight: 400;border:0px;}QPushButton:pressed{background: #296CD9;border-radius: 11px;color: #ffffff;font-weight: 400;border:0px;}")
            else:
                self.btn.setStyleSheet("QPushButton{font-size: 12px;font-weight: 400;color: #3790FA;line-height: 12px;border-radius: 11px;border: 1px solid #3790FA;}QPushButton:hover{background: #3790FA;border-radius: 11px;color: #ffffff;font-weight: 400;border:0px;}QPushButton:pressed{background: #296CD9;border-radius: 11px;color: #ffffff;font-weight: 400;border:0px;}")
        elif(self.btn.text() == _("cont")):
            self.btn.setStyleSheet("QPushButton{font-size: 12px;font-weight: 400;color: #3790FA;line-height: 12px;border-radius: 11px;background: rgba(55, 144, 250, 0.12);}QPushButton:hover{background: #3790FA;border-radius: 11px;color: #ffffff;font-weight: 400;border:0px;}QPushButton:pressed{background: #3790FA;border-radius: 11px;color: #ffffff;font-weight: 400;border:0px;}")
        elif(self.btn.text() == _("installing") or self.btn.text() == _("installed")):
            self.btn.setStyleSheet("QPushButton{font-size: 12px;font-weight: 400;color: #3790FA;line-height: 12px;background: rgba(55, 144, 250, 0.12);border-radius: 11px;}")
        else:
            if (Globals.THEME_COLOR == 0):
                self.btn.setStyleSheet("QPushButton{font-size: 12px;font-weight: 400;color: #3790FA;line-height: 12px;border-radius: 11px;background: rgba(55, 144, 250, 0.12);}QPushButton:hover{background: #3790FA;border-radius: 11px;color: #ffffff;font-weight: 400;border:0px;}QPushButton:pressed{background: #296CD9;border-radius: 11px;color: #ffffff;font-weight: 400;border:0px;}")
            else:
                self.btn.setStyleSheet("QPushButton{font-size: 12px;font-weight: 400;color: #3790FA;line-height: 12px;border-radius: 11px;border: 1px solid #3790FA;}QPushButton:hover{background: #3790FA;border-radius: 11px;color: #ffffff;font-weight: 400;border:0px;}QPushButton:pressed{background: #296CD9;border-radius: 11px;color: #ffffff;font-weight: 400;border:0px;}")

    def reload_btn(self,appname):
        if(appname == self.appmsg.pkgname):
            self.check_btn_text()

    def show_downloading_widget(self,appname,is_cancel = False,is_puase = False):
        if(appname == self.appmsg.pkgname):
            if( self.appmsg.status != PkgStates.NOTHING):
                if(is_cancel == False):
                    if (is_puase == False):
                        if (self.appmsg.status != PkgStates.INSTALLING):
                            self.btn.hide()
                            self.down_progress.show()
                            self.down_progress.btn.show()
                            self.down_progress.cont_btn.hide()
                    else:
                        self.btn.setText(_("cont"))
                        self.change_btn_width()
                        self.btn.setStyleSheet("QPushButton{font-size: 12px;font-weight: 400;color: #3790FA;line-height: 12px;border-radius: 11px;background: rgba(55, 144, 250, 0.12);}QPushButton:hover{background: #3790FA;border-radius: 11px;color: #ffffff;font-weight: 400;border:0px;}QPushButton:pressed{background: #3790FA;border-radius: 11px;color: #ffffff;font-weight: 400;border:0px;}")
                        self.btn.show()
                        self.down_progress.show()
                else:
                    self.btn.resize(52, 24)
                    self.btn.move(self.width() - self.btn.width() - 20, self.height() - self.btn.height() - 20)
                    self.btn.show()
                    self.down_progress.hide()
            else:
                self.check_btn_text()
        # self.get_btn_color()

    def slot_get_download_progress(self,appname,progress):
        if(appname == self.appmsg.pkgname):
            if(progress == 100):
                self.btn.setEnabled(False)
                self.narrow_btn_time.stop()
                self.btn.resize(48, 22)
                self.btn.move(int(self.width() - self.btn.width()),
                              int((self.icon_label.height() - self.btn.height()) / 2) + self.icon_label.y())
                self.down_progress.hide()
                self.btn.setCheckable(False)
                self.btn.setText(_("installing"))
                self.change_btn_width()
                self.btn.setStyleSheet("QPushButton{font-size: 12px;font-weight: 400;color: #3790FA;line-height: 12px;border-radius: 11px;background: rgba(55, 144, 250, 0.12);}QPushButton:hover{background: #3790FA;border-radius: 11px;color: #ffffff;font-weight: 400;border:0px;}QPushButton:pressed{background: #296CD9;border-radius: 11px;color: #ffffff;font-weight: 400;border:0px;}")
                self.btn.show()
            else:
                if (self.appmsg.status != PkgStates.PUASE):
                    if (progress - self.pre_progress >= 2):
                        self.send_download_progress.emit(progress)
                        self.pre_progress = progress
                    else:
                        self.send_download_progress.emit(self.pre_progress)

    # 处理：卸载界面的显示
    def slot_uninstall(self, appname, state):
        if (appname == self.appmsg.pkgname):
            self.btn.resize(48, 22)
            self.btn.move(int(self.width() - self.btn.width()),
                          int((self.icon_label.height() - self.btn.height()) / 2) + self.icon_label.y())
            if (state == "uninstalling"):
                self.btn.setText(_("uninstalling"))
                self.change_btn_width()
                self.btn.hide()
                self.remove_btn.start_loading()
                self.remove_btn.show()
            else:
                self.btn.setText(_("uninstalled"))
                self.change_btn_width()
                self.btn.setStyleSheet(
                    "QPushButton{font-size: 12px;font-weight: 400;color: #F8F8F8;line-height: 12px;background: #C0C4CC;border-radius: 11px;}")

    def change_btn_state(self,appname,state):
        if(appname == self.appmsg.pkgname):
            if(state == PkgStates.NOTHING or state == PkgStates.REMOVED):
                self.appmsg.status = PkgStates.NOTHING
                self.check_btn_text()
                self.check_all_check_statue.emit()
            if(state == PkgStates.INSTALLING):
                self.slot_get_download_progress(appname,100)
            if(state == PkgStates.REMOVING):
                self.slot_uninstall(appname,"uninstalling")
            if(state == PkgStates.INSTALLED):
                self.appmsg.is_installed = True
                self.appmsg.is_upgradable = False
                self.appmsg.status = PkgStates.NOTHING
                self.reload_btn(appname)
                self.check_all_check_statue.emit()
            if(state == PkgStates.DOWNLOADING):
                if(self.btn.isHidden() == False):
                    self.down_progress.init_down = True
                    self.down_progress.persent = 0
                    self.pre_progress = 0
                if (self.appmsg.status != PkgStates.NOTHING):
                    if (self.btn.text() != ""):
                        self.narrow_btn_time.start(1)
                    else:
                        self.btn.hide()
                        self.down_progress.btn.show()
                        self.down_progress.cont_btn.hide()
                        self.down_progress.show()
                # self.show_downloading_widget(appname)
            if (state == PkgStates.PUASE):
                # self.show_downloading_widget(appname,False,True)
                self.appmsg.status = PkgStates.PUASE
                self.btn.hide()
                self.down_progress.show()
                self.down_progress.puase_download(self.appmsg.percent)
                self.check_all_check_statue.emit()

    def move_btn(self):
        self.narrow_btn_time.stop()
        self.btn.setText("")
        w = self.btn.width()
        h = self.btn.height()
        x = self.btn.x()
        y = self.btn.y()
        if(self.appmsg.status != PkgStates.NOTHING):
            if(self.btn.width()>self.btn.height()):
                self.btn.hide()
                self.btn.setFixedSize(w-1,h)
                self.btn.move(x+1,y)
                self.btn.show()
                self.narrow_btn_time.start(4)
            else:
                self.show_downloading_widget(self.appmsg.pkgname)


    #
    # 函数：处理点击暂停
    #
    def slot_puase_download(self):
        if (type(self.appmsg) != ApkInfo):
            self.puase_download.emit(self.appmsg)
        else:
            self.puase_apk_download.emit(self.appmsg)