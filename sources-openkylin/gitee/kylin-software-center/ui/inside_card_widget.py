#!/usr/bin/python3
# -*- coding: utf-8 -*-


from PyQt5.QtGui import *
from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
from models.enums import setLongTextToElideFormat,BUTTON_PRESSED,BUTTON_LEAVE,BUTTON_ENTER,AppMsg,PKG_NAME
from models.application import *
from models.apkinfo import *
from utils import commontools
from ui.circular_progress_bar import *
from ui.uninstall_btn import *
from ui.shot_detail import *
# import time
import gettext
gettext.bindtextdomain("ubuntu-kylin-software-center", "/usr/share/locale")
gettext.textdomain("ubuntu-kylin-software-center")
_ = gettext.gettext


class Inside_Card_Widget(QFrame):
    is_remove = False
    clicked_card = pyqtSignal(Application)
    clicked_kycard = pyqtSignal(ApkInfo)
    send_download_progress = pyqtSignal(int)
    puase_download = pyqtSignal(Application)
    puase_apk_download = pyqtSignal(ApkInfo)
    check_action_btn_state = pyqtSignal(str)
    send_progress = pyqtSignal(int)
    init_detail_display = pyqtSignal()
    init_card_over = pyqtSignal(QWidget)
    check_all_check_statue = pyqtSignal()
    btn_text = ""

    def __init__(self, category,width,hight,app,is_remove_app,mw,detail,ca,parent=None):
        QFrame.__init__(self, parent)
        self.app = app
        self.mw = mw
        self.category = category
        self.x_width = width
        self.y_height = hight
        self.detail_widget = detail
        self.pre_progress = 0
        self.m_parent = parent
        self.narrow_btn_time = QTimer()
        self.is_remove_app = is_remove_app
        self.font = QFont()
        self.font.setPixelSize(14)
        self.m_font = QFontMetrics(self.font)
        self.ca = ca
        self.init_ui()
        self.init_card_over.connect(self.detail_widget.mw.slot_init_insidecard_over)
        self.clicked_card.connect(self.detail_widget.slot_clicked_card)
        self.clicked_kycard.connect(self.detail_widget.slot_clicked_card)
        self.mw.sigenal_show_downloading_widget.connect(self.show_downloading_widget)
        self.check_action_btn_state.connect(self.detail_widget.slot_check_action_btn_state)
        self.narrow_btn_time.timeout.connect(self.move_btn)
        self.init_detail_display.connect(self.slot_init_detail_widget)
        self.init_detail_display.emit()
        self.show()

    def init_ui(self):
        self.resize(self.x_width,self.y_height)
        # self.setAutoFillBackground(True)
        self.setFocusPolicy(Qt.NoFocus)
        self.setWindowFlags(Qt.FramelessWindowHint)
        if(Globals.THEME_COLOR == 0):
            self.setStyleSheet("QFrame{border-radius: 12px;}")
            palette1 = QPalette()
            palette1.setColor(self.backgroundRole(), QColor(247, 248, 249))  # 设置背景颜色
            self.setPalette(palette1)
            self.setAutoFillBackground(True)
        else:
            self.setStyleSheet("QFrame{border-radius: 12px;}")
            palette1 = QPalette()
            palette1.setColor(self.backgroundRole(), QColor(31, 32, 34))  # 设置背景颜色
            self.setPalette(palette1)
            self.setAutoFillBackground(True)
            # self.setStyleSheet("QFrame{background: #2B2B2C;border-radius: 12px;}")


    def slot_init_detail_widget(self):
        self.check_btn = QCheckBox(self)
        self.check_btn.resize(16,16)
        self.check_btn.setFocusPolicy(Qt.NoFocus)
        self.check_btn.setAttribute(Qt.WA_TranslucentBackground)
        self.check_btn.setCheckState(Qt.Unchecked)
        self.check_btn.setStyleSheet(
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
        self.check_btn.move(self.width()-self.check_btn.width()-20,20)
        self.check_btn.clicked.connect(lambda:self.mw.slot_clicked_check_btn(self))
        self.check_btn.hide()


        self.icon_label = QLabel(self)
        self.icon_label.resize(48,48)
        self.icon_label.move(20,20)
        iconpath = commontools.get_big_icon_path(self.app.pkgname)
        photo = Photo(iconpath, self.icon_label.width(),
                      self.icon_label.height(),
                      self.icon_label)
        photo.shot_photo_change(iconpath)
        # photo.resize_image()
        photo.show()
        self.icon_label.setStyleSheet("QLabel{border-radius: 8px;background-color:transparent;}")
        # self.icon_label.setStyleSheet("QLabel{border-radius: 8px;border-image: url('"+iconpath+"') 0 0 0 0 stretch strectch;background-color:transparent;}")
        #self.icon_label.setStyleSheet("QLabel{background-image:url('" + iconpath + "');background-color:transparent;background-position:center;background-repeat:no-repeat;}")

        self.name_label = QLabel(self)
        self.name_label.setAttribute(Qt.WA_TranslucentBackground)
        self.name_label.move(self.icon_label.x()+self.icon_label.width()+10,self.icon_label.y()+4)
        self.name_label.setFixedWidth(self.width()-self.name_label.x()-40)
        # self.name_label.setMaximumWidth(120)
        self.name_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #303133;line-height: 17px;}")
        if(Globals.LAN == "zh_CN"):
            if self.app.displayname_cn != '' and self.app.displayname_cn is not None and self.app.displayname_cn != 'None':
                setLongTextToElideFormat(self.name_label, self.app.displayname_cn)
                if("…" in self.name_label.text()):
                    self.name_label.setToolTip(self.app.displayname_cn)
            else:
                setLongTextToElideFormat(self.name_label, self.app.pkgname)
                if ("…" in self.name_label.text()):
                    self.name_label.setToolTip(self.app.pkgname)
        else:
            setLongTextToElideFormat(self.name_label, self.app.pkgname)
            if ("…" in self.name_label.text()):
                self.name_label.setToolTip(self.app.pkgname)
        self.name_label.adjustSize()
        self.category_label = QLabel(self)
        self.category_label.setAttribute(Qt.WA_TranslucentBackground)
        self.category_label.setAlignment(Qt.AlignLeft)
        self.category_label.setFixedHeight(24)
        self.category_label.setStyleSheet("QLabel{font-size: 12px;font-weight: 400;color: #8F9399;}")
        if (str(self.app.softStyle) == "3"):
            if (hasattr(self.mw.detailwidget,"kydroid_widget") and self.m_parent == self.mw.detailwidget.kydroid_widget.app_widget):
                if ( self.mw.detailwidget.kydroid_widget.type_name_label.text() not in self.app.category_name_cn and self.mw.detailwidget.kydroid_widget.type_name_label.text() not in self.app.category_name):
                    if self.app.category_name_cn != '' and self.app.category_name_cn is not None and self.app.category_name_cn != 'None' and Globals.LAN == "zh_CN":
                        self.category_label.setText(self.app.category_name_cn.split("、")[0])
                    else:
                        self.category_label.setText(self.app.category_name.split("、")[0])
                else:
                    self.category_label.setText(self.mw.detailwidget.kydroid_widget.type_name_label.text())
            else:
                if self.app.category_name_cn != '' and self.app.category_name_cn is not None and self.app.category_name_cn != 'None' and Globals.LAN == "zh_CN":
                    self.category_label.setText(self.app.category_name_cn.split("、")[0])
                else:
                    self.category_label.setText(self.app.category_name.split("、")[0])
        else:
            if (hasattr(self.mw.detailwidget,"category_widget") and self.m_parent == self.mw.detailwidget.category_widget.type_show_card_widget):
                if (self.mw.detailwidget.category_widget.type_name_label.text() not in self.app.category_name_cn and self.mw.detailwidget.category_widget.type_name_label.text() not in self.app.category_name):
                    if self.app.category_name_cn != '' and self.app.category_name_cn is not None and self.app.category_name_cn != 'None' and Globals.LAN == "zh_CN":
                        self.category_label.setText(self.app.category_name_cn.split("、")[0])
                    else:
                        self.category_label.setText(self.app.category_name.split("、")[0])
                else:
                    self.category_label.setText(self.mw.detailwidget.category_widget.type_name_label.text())
            else:
                if self.app.category_name_cn != '' and self.app.category_name_cn is not None and self.app.category_name_cn != 'None' and Globals.LAN == "zh_CN":
                    self.category_label.setText(self.app.category_name_cn.split("、")[0])
                else:
                    self.category_label.setText(self.app.category_name.split("、")[0])
        # else:
        #     # if(str(self.app.softStyle) == "2"):
        #     #     self.category_label.setText(self.app.manufacturers)
        #     # else:
        #     if self.app.category_name_cn != '' and self.app.category_name_cn is not None and self.app.category_name_cn != 'None' and Globals.LAN == "zh_CN":
        #         self.category_label.setText(self.app.category_name_cn.split("、")[0])
        #     else:
        #         self.category_label.setText(self.app.category_name.split("、")[0])
        self.category_label.adjustSize()
        self.category_label.move(self.name_label.x(),self.name_label.y()+self.name_label.height()+4)
        self.btn = QPushButton(self)
        self.btn.setFocusPolicy(Qt.NoFocus)
        self.btn.setStyleSheet("QPushButton{font-size: 14px;font-weight: 400;color: #FFFFFF;line-height: 12px;border-radius: 12px;border: 0px solid #3790FA;background: #3790FA;}QPushButton:hover{background: #40A9FB;border-radius: 12px;color: #ffffff;font-weight: 400;border:0px;}QPushButton:pressed{background: #296CD9;border-radius: 12px;color: #ffffff;font-weight: 400;border:0px;}")
        # self.btn.resize(52,24)
        self.btn.move(self.width()-self.btn.width()-20,self.height()-self.btn.height()-20)
        self.btn.hide()
        self.btn.clicked.connect(lambda: self.mw.slot_click_action_btn(self, self.btn.text(), self.app))

        self.down_progress = Circular_Progress_Bar(22, 22, self.app.pkgname,self)
        self.down_progress.update_progress(self.app.percent)
        self.down_progress.move(self.width() - self.down_progress.width() - 20,
                                self.height() - self.down_progress.height() - 20)
        self.send_progress.connect(self.down_progress.GenUpdate)
        self.down_progress.hide()
        self.cancel_btn = QPushButton(self)
        self.cancel_btn.setFocusPolicy(Qt.NoFocus)
        self.cancel_btn.resize(22, 22)
        self.cancel_btn.setStyleSheet("QPushButton{background-image:url(./res/cancel1.png);border-radius:11px;}QPushButton:hover{background-image:url(./res/cancel1_hover.png);border-radius:11px;}")
        # self.cancel_btn.setStyleSheet("QPushButton{font-size: 14px;font-weight: 400;color: #303133;line-height: 12px;text-align: center;background: rgba(55, 144, 250, 0.08);border-radius: 12px;}QPushButton:hover{background: #3790FA;border-radius: 12px;color: #FFFFFF;font-weight: 400;}QPushButton:pressed{background: #296CD9;color: #FFFFFF;font-weight: 500;}")
        # self.cancel_btn.setText(_("cancel"))
        self.cancel_btn.move(self.down_progress.x() - self.cancel_btn.width() - 8,
                             self.height() - self.cancel_btn.height() - 20)
        self.cancel_btn.hide()
        self.cancel_btn.clicked.connect(lambda: self.mw.slot_click_cancel_btn(self, self.app))
        self.remove_btn = uninstall_btn(self)
        self.remove_btn.move(self.width() - self.remove_btn.width() - 20, self.height() - self.remove_btn.height() - 20)
        self.remove_btn.hide()
        self.down_progress.btn.clicked.connect(self.slot_puase_download)
        self.down_progress.cont_btn.clicked.connect(lambda :self.mw.re_download_app(self.app))
        self.change_btn_state(self.app.pkgname,self.app.status)
        self.slot_set_dark_mode()
        self.init_card_over.emit(self)

    #
    #函数：修改界面大小后重新设置界面控件的大小和位置
    #
    def re_move(self):
        self.name_label.setFixedWidth(self.width() - self.name_label.x()-40)
        if (Globals.LAN == "zh_CN"):
            if self.app.displayname_cn != '' and self.app.displayname_cn is not None and self.app.displayname_cn != 'None':
                setLongTextToElideFormat(self.name_label, self.app.displayname_cn)
                if ("…" in self.name_label.text()):
                    self.name_label.setToolTip(self.app.displayname_cn)
                else:
                    self.name_label.setToolTip("")
            else:
                setLongTextToElideFormat(self.name_label, self.app.pkgname)
                if ("…" in self.name_label.text()):
                    self.name_label.setToolTip(self.app.pkgname)
        else:
            setLongTextToElideFormat(self.name_label, self.app.pkgname)
            if ("…" in self.name_label.text()):
                self.name_label.setToolTip(self.app.pkgname)
        self.btn.move(self.width()-self.btn.width()-20,self.height()-self.btn.height()-20)
        self.remove_btn.move(self.width() - self.remove_btn.width() - 20, self.height() - self.remove_btn.height() - 20)
        self.down_progress.move(self.width() - self.down_progress.width() - 20,
                                self.height() - self.down_progress.height() - 20)
        self.cancel_btn.move(self.down_progress.x() - self.cancel_btn.width() - 8,
                             self.height() - self.cancel_btn.height() - 20)
        self.check_btn.move(self.width() - self.check_btn.width() - 20, 20)

    #
    #函数：设置深色模式
    #
    def slot_set_dark_mode(self):
        if (Globals.THEME_COLOR == 0):
            # palette1 = QPalette()
            # palette1.setColor(self.backgroundRole(), QColor(247, 248, 249))  # 设置背景颜色
            # self.setPalette(palette1)
            # self.setAutoFillBackground(True)
            self.setStyleSheet("QFrame{border:0px solid #ff0000;border-radius: 12px;background-color:#F7F8F9;}")
            self.name_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #303133;line-height: 17px;}")
            self.category_label.setStyleSheet("QLabel{font-size: 12px;font-weight: 400;color: #8F9399;line-height: 17px;}")
            self.cancel_btn.setStyleSheet("QPushButton{background-image:url(./res/cancel1.png);border-radius:11px;}QPushButton:hover{background-image:url(./res/cancel1_hover.png);border-radius:11px;}")
        else:
            # painter = QPalette()
            # painter.setColor(QPalette.Background, QColor(46, 46, 46))
            # self.setPalette(painter)
            # self.setAutoFillBackground(True)
            self.setStyleSheet("QFrame{background: rgba(46, 46, 46, 0.88);border-radius: 12px;}")
            self.name_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #F9F9F9;line-height: 17px;}")
            self.category_label.setStyleSheet("QLabel{font-size: 12px;font-weight: 400;color: #8F9399;line-height: 17px;}")
            self.cancel_btn.setStyleSheet(
                "QPushButton{background-image:url(./res/cancel_dark.png);border-radius:11px;}QPushButton:hover{background-image:url(./res/cancel_hover_dark.png);border-radius:11px;}")
        if(self.btn.text() == _("uninstall")):
            self.set_btn_stylesheet(True)
        elif(self.btn.text() == _("uninstalled")):
            self.set_btn_stylesheet()
        self.down_progress.reflash_color()
        # self.show()
        # QApplication.processEvents()
        # time.sleep(0.1)


    #
    #函数：设置按钮的样式
    #
    def set_btn_stylesheet(self,is_uninstall = False):
        if(is_uninstall == True):
            self.btn.setEnabled(True)
            if(Globals.THEME_COLOR == 0):
                self.btn.setStyleSheet("QPushButton{font-size: 14px;font-weight: 400;color: #3790FA;line-height: 12px;text-align: center;background: #EBEBEB;border-radius: 12px;}QPushButton:hover{border:0px;background: #3790FA;border-radius: 12px;color: #ffffff;font-weight: 400;}QPushButton:pressed{border:0px;background: #296CD9;border-radius: 12px;color: #ffffff;font-weight: 400;}")
            else:
                self.btn.setStyleSheet("QPushButton{font-size: 14px;font-weight: 400;color: #3790FA;line-height: 12px;text-align: center;background: #454545;border-radius: 12px;}QPushButton:hover{border:0px;background: #3790FA;border-radius: 12px;color: #ffffff;font-weight: 400;}QPushButton:pressed{border:0px;background: #296CD9;border-radius: 12px;color: #ffffff;font-weight: 400;}")
        if(self.btn.text() == _("uninstalled")):
            self.check_btn.hide()
            self.check_btn.setChecked(False)
            self.check_btn.setEnabled(False)
            if (Globals.THEME_COLOR == 0):
                self.btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 400;color: #B3B3B3;line-height: 12px;background: #F5F5F5;border-radius: 12px;}")
            else:
                self.btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 400;color: #8F9399;line-height: 12px;background: rgba(245,245,245,0.19);border-radius: 12px;}")


    #
    # 函数：处理点击暂停
    #
    def slot_puase_download(self):
        if (type(self.app) != ApkInfo):
            self.puase_download.emit(self.app)
        else:
            self.puase_apk_download.emit(self.app)


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
    #函数：获取按钮的名字
    #
    def check_btn_text(self):
        self.btn.resize(52, 24)
        self.btn.move(self.width() - self.btn.width() - 20, self.height() - self.btn.height() - 20)
        self.remove_btn.stop_loading()
        self.remove_btn.hide()
        self.cancel_btn.hide()
        self.down_progress.hide()
        self.check_btn.setEnabled(True)
        self.btn.setEnabled(True)
        self.btn.show()
        if(self.app.status == PkgStates.REMOVING):
            self.slot_uninstall(self.app.pkgname,"uninstalling")
            return
        if (type(self.app) != ApkInfo):
            try:
                self.package = self.ca[self.app.pkgname]
            except:
                self.package = None
            if (self.package != None):
                if (self.package.is_installed == True and self.package.is_upgradable == True):
                    self.btn.setText(_("update"))
                    self.change_btn_width()
                    self.btn.setStyleSheet("QPushButton{font-size: 14px;font-weight: 400;color: #3790FA;line-height: 12px;border-radius: 12px;background: rgba(55, 144, 250, 0.12);}QPushButton:hover{background: #3790FA;border-radius: 12px;color: #ffffff;font-weight: 400;border:0px;}QPushButton:pressed{background: #296CD9;border-radius: 12px;color: #ffffff;font-weight: 400;border:0px;}")
                elif (self.package.is_installed == False):
                    self.btn.setText(_("down"))
                    self.change_btn_width()
                    self.btn.setStyleSheet("QPushButton{font-size: 14px;font-weight: 400;color: #FFFFFF;line-height: 12px;border-radius: 12px;border: 0px solid #3790FA;background: #3790FA;}QPushButton:hover{background: #40A9FB;border-radius: 12px;color: #ffffff;font-weight: 400;border:0px;}QPushButton:pressed{background: #296CD9;border-radius: 12px;color: #ffffff;font-weight: 400;border:0px;}")
                elif (self.package.is_installed == True and self.package.is_upgradable == False):
                    #self.btn.setText(_("run"))
                    if (self.app.pkgname == "kylin-installer"):
                        has_desktop = False
                    else:
                        has_desktop = self.check_app_desktop(self.app.pkgname)
                    if (has_desktop == True):
                        self.check_btn.hide()
                        self.check_btn.setChecked(False)
                        self.btn.setText(_("Open"))
                        self.change_btn_width()
                        self.btn.setStyleSheet("QPushButton{font-size: 14px;font-weight: 400;color: #3790FA;line-height: 12px;border-radius: 12px;background: rgba(55, 144, 250, 0.12);}QPushButton:hover{background: #3790FA;border-radius: 12px;color: #ffffff;font-weight: 400;border:0px;}QPushButton:pressed{background: #296CD9;border-radius: 12px;color: #ffffff;font-weight: 400;border:0px;}")
                        # self.mw.check_btn_state()
                    else:
                        self.check_btn.hide()
                        self.check_btn.setChecked(False)
                        self.btn.setText(_("installed"))
                        self.change_btn_width()
                        self.btn.setEnabled(False)
                        self.btn.setStyleSheet("QPushButton{font-size: 14px;font-weight: 400;color: #3790FA;line-height: 12px;background: rgba(55, 144, 250, 0.12);border-radius: 12px;}")
                        # self.mw.check_btn_state()
                    pass
            else:
                if(self.app.is_notice == False):
                    self.btn.setText(_("down"))
                    self.change_btn_width()
                else:
                    self.btn.setText(_("website"))
                    self.change_btn_width()
                self.btn.setStyleSheet("QPushButton{font-size: 14px;font-weight: 400;color: #FFFFFF;line-height: 12px;border-radius: 12px;border: 0px solid #3790FA;background: #3790FA;}QPushButton:hover{background: #40A9FB;border-radius: 12px;color: #ffffff;font-weight: 400;border:0px;}QPushButton:pressed{background: #296CD9;border-radius: 12px;color: #ffffff;font-weight: 400;border:0px;}")
        else:
            if (self.app != None):
                # if ((self.mw.at.appmgr.apk_list == None or self.mw.at.appmgr.apk_list == [])):
                if(Globals.APK_EVNRUN == 0):
                    self.btn.setText(_("start env"))
                    self.change_btn_width()
                    self.btn.setStyleSheet(
                        "QPushButton{font-size: 14px;font-weight: 400;color: #3790FA;line-height: 12px;border-radius: 12px;background: rgba(55, 144, 250, 0.12);}QPushButton:hover{background: #3790FA;border-radius: 12px;color: #ffffff;font-weight: 400;border:0px;}QPushButton:pressed{background: #296CD9;border-radius: 12px;color: #ffffff;font-weight: 400;border:0px;}")
                else:
                    if (self.app.is_installed == True and self.app.is_upgradable == True):
                        self.btn.setText(_("update"))
                        self.change_btn_width()
                        self.btn.setStyleSheet("QPushButton{font-size: 14px;font-weight: 400;color: #3790FA;line-height: 12px;border-radius: 12px;background: rgba(55, 144, 250, 0.12);}QPushButton:hover{background: #3790FA;border-radius: 12px;color: #ffffff;font-weight: 400;border:0px;}QPushButton:pressed{background: #296CD9;border-radius: 12px;color: #ffffff;font-weight: 400;border:0px;}")
                    elif (self.app.is_installed == False):
                        self.btn.setText(_("down"))
                        self.change_btn_width()
                        self.btn.setStyleSheet("QPushButton{font-size: 14px;font-weight: 400;color: #FFFFFF;line-height: 12px;border-radius: 12px;border: 0px solid #3790FA;background: #3790FA;}QPushButton:hover{background: #40A9FB;border-radius: 12px;color: #ffffff;font-weight: 400;border:0px;}QPushButton:pressed{background: #296CD9;border-radius: 12px;color: #ffffff;font-weight: 400;border:0px;}")
                    elif (self.app.is_installed == True and self.app.is_upgradable == False):
                        self.check_btn.hide()
                        self.btn.setText(_("Open"))
                        self.change_btn_width()
                        self.btn.setStyleSheet("QPushButton{font-size: 14px;font-weight: 400;color: #3790FA;line-height: 12px;border-radius: 12px;background: rgba(55, 144, 250, 0.12);}QPushButton:hover{background: #3790FA;border-radius: 12px;color: #ffffff;font-weight: 400;border:0px;}QPushButton:pressed{background: #296CD9;border-radius: 12px;color: #ffffff;font-weight: 400;border:0px;}")
                        pass
            else:
                self.btn.setText(_("down"))
                self.change_btn_width()
                self.btn.setStyleSheet("QPushButton{font-size: 14px;font-weight: 400;color: #FFFFFF;line-height: 12px;border-radius: 12px;border: 0px solid #3790FA;background: #3790FA;}QPushButton:hover{background: #40A9FB;border-radius: 12px;color: #ffffff;font-weight: 400;border:0px;}QPushButton:pressed{background: #296CD9;border-radius: 12px;color: #ffffff;font-weight: 400;border:0px;}")
        self.set_btn_stylesheet()

    #
    #函数：重置按钮
    #
    def reload_btn(self,appname):
        if(appname == self.app.pkgname):
            if (self.app.is_removed == True):
                self.check_btn.hide()
                self.check_btn.setChecked(False)
                self.check_btn.setEnabled(False)
                self.slot_uninstall(appname, "uninstalled")
            else:
                if(self.is_remove_app):
                    self.btn.setText(_("uninstall"))
                    self.change_btn_width()
                    self.btn.show()
                    self.remove_btn.hide()
                    self.cancel_btn.hide()
                    self.down_progress.hide()
                    self.set_btn_stylesheet(True)
                else:
                    self.check_btn_text()

    #
    #函数：进度显示
    #
    def slot_get_download_progress(self,appname,progress):
        if(appname == self.app.pkgname):
            if(int(progress) == 100):
                if(self.is_remove == False):
                    self.narrow_btn_time.stop()
                    self.btn.resize(52, 24)
                    self.btn.move(self.width() - self.btn.width() - 20, self.height() - self.btn.height() - 20)
                    self.cancel_btn.hide()
                    self.down_progress.hide()
                    self.btn.setText(_("installing"))
                    self.change_btn_width()
                    self.btn.setStyleSheet("QPushButton{font-size: 14px;font-weight: 400;color: #3790FA;line-height: 12px;background: rgba(55, 144, 250, 0.12);border-radius: 12px;}")
                    self.btn.show()
                    self.remove_btn.hide()
                else:
                    self.slot_uninstall(appname,"uninstalled")
            else:
                if(self.app.status == PkgStates.DOWNLOADING):
                    if(progress-self.pre_progress >=2):
                        self.app.percent = progress
                        self.send_progress.emit(progress)
                        self.pre_progress = progress
                    else:
                        self.send_progress.emit(self.pre_progress)


    #
    #函数：显示下载时卡片界面的显示
    #
    def show_downloading_widget(self, appname,is_cancel=False,is_puase = False):
        if(appname == self.app.pkgname):
            if (self.app.status != PkgStates.NOTHING):
                if (is_cancel == False):
                    if (is_puase == False):
                        if(self.app.status != PkgStates.INSTALLING):
                            self.btn.hide()
                            self.cancel_btn.show()
                            self.down_progress.show()
                            self.down_progress.btn.show()
                            self.down_progress.cont_btn.hide()
                            self.cancel_btn.move(self.down_progress.x() - self.cancel_btn.width() - 10,self.height() - self.cancel_btn.height() - 20)
                    else:
                        self.btn.setText(_("cont"))
                        self.change_btn_width()
                        self.btn.setStyleSheet("QPushButton{font-size: 14px;font-weight: 400;color: #3790FA;line-height: 12px;border-radius: 12px;background: rgba(55, 144, 250, 0.12);}QPushButton:hover{background: #3790FA;border-radius: 12px;color: #ffffff;font-weight: 400;border:0px;}QPushButton:pressed{background: #3790FA;border-radius: 12px;color: #ffffff;font-weight: 400;border:0px;}")
                        self.btn.show()
                        self.remove_btn.hide()
                        self.cancel_btn.show()
                        self.down_progress.hide()
                        self.cancel_btn.move(self.btn.x() - self.cancel_btn.width() - 10,self.height() - self.cancel_btn.height() - 20)
                else:
                    self.btn.resize(52, 24)
                    self.btn.move(self.width() - self.btn.width() - 20, self.height() - self.btn.height() - 20)
                    self.remove_btn.hide()
                    self.reload_btn(appname)
                    self.cancel_btn.hide()
                    self.down_progress.hide()
                    self.cancel_btn.move(self.down_progress.x() - self.cancel_btn.width() - 10,self.height() - self.cancel_btn.height() - 20)
                    self.btn.show()
                self.send_download_progress.emit(self.app.percent)
            else:
                self.check_btn_text()

    def slot_installed_over(self,appname):
        if (appname == self.app.pkgname):
            if(self.is_remove):
                self.check_btn.hide()
                self.check_btn.setChecked(False)
                self.check_btn.setEnabled(False)

    def change_btn_width(self):
        s = self.m_font.boundingRect(self.btn.text()).size().width()
        if (s+24 < 52):
            self.btn.setFixedSize(52,24)
        else:
            self.btn.setFixedSize(s+24,24)
        self.btn.move(self.width()-self.btn.width()-18,self.btn.y())

    def slot_uninstall(self,appname,state):
        if (appname == self.app.pkgname):
            self.btn.resize(52, 24)
            self.btn.move(self.width() - self.btn.width() - 20, self.height() - self.btn.height() - 20)
            self.cancel_btn.hide()
            self.down_progress.hide()
            if (state == "uninstalling"):
                self.is_remove = True
                self.check_btn.hide()
                self.check_btn.setEnabled(False)
                self.btn.setText(_("uninstalling"))
                self.change_btn_width()
                self.btn.hide()
                self.remove_btn.start_loading()
                self.remove_btn.show()
            else:
                if(self.m_parent == self.mw.detailwidget.mywidget.app_widget):
                    self.check_btn.hide()
                    self.check_btn.setChecked(False)
                    self.check_btn.setEnabled(False)
                    self.remove_btn.stop_loading()
                    self.remove_btn.hide()
                    self.btn.setText(_("uninstalled"))
                    self.change_btn_width()
                    if (Globals.THEME_COLOR == 0):
                        self.btn.setStyleSheet("QPushButton{font-size: 14px;font-weight: 400;color: #B3B3B3;line-height: 12px;background: #F5F5F5;border-radius: 12px;}")
                    else:
                        self.btn.setStyleSheet(
                            "QPushButton{font-size: 14px;font-weight: 400;color: #8F9399;line-height: 12px;background: rgba(245,245,245,0.19);border-radius: 12px;}")
                    self.btn.setEnabled(False)
                    self.btn.show()
                    self.remove_btn.hide()
                    self.down_progress.hide()
                    self.check_action_btn_state.emit(appname)
                else:
                    self.is_remove = False
                    # self.app.is_removed = False
                    self.is_remove_app = False
                    # if(self.m_parent == self.mw.detailwidget.mywidget.history_widget.app_card_widget):
                    self.change_btn_state(appname,PkgStates.NOTHING)

    def change_kydroid_btn_statue(self):
        if(self.is_remove_app == False):
            if(type(self.app) == ApkInfo):
                for i in self.mw.at.appmgr.apk_list:
                    if(i.pkgname == self.app.pkgname):
                        self.app.is_installed = i.is_installed
                        self.app.is_upgradable = i.is_upgradable
                if(self.app.status == PkgStates.NOTHING):
                    self.check_btn_text()

    def change_btn_state(self,appname,state):
        if(appname == self.app.pkgname):
            if(state == PkgStates.NOTHING):
                if(self.app.is_removed == True and self.is_remove):
                    self.slot_uninstall(appname,"uninstalled")
                else:
                    if(self.is_remove_app == True):
                        self.btn.setText(_("uninstall"))
                        self.change_btn_width()
                        self.btn.show()
                        self.cancel_btn.hide()
                        self.remove_btn.hide()
                        self.down_progress.hide()
                        self.set_btn_stylesheet(True)
                    else:
                        self.check_btn_text()
                self.check_all_check_statue.emit()
            elif(state == PkgStates.REMOVED):
                self.app.status = PkgStates.NOTHING
                self.slot_uninstall(appname, "uninstalled")
            elif(state == PkgStates.INSTALLING):
                self.down_progress.hide()
                self.slot_get_download_progress(appname,100)
            elif(state == PkgStates.REMOVING):
                self.is_remove = True
                self.slot_uninstall(appname,"uninstalling")
            elif(state == PkgStates.INSTALLED):
                self.app.is_installed = True
                self.app.is_upgradable = False
                self.app.status = PkgStates.NOTHING
                self.reload_btn(appname)
                self.check_all_check_statue.emit()
            elif(state == PkgStates.DOWNLOADING):
                self.check_btn.setChecked(False)
                self.check_btn.hide()
                if (self.btn.isHidden() == False):
                    self.down_progress.init_down = True
                    self.down_progress.persent = 0
                    self.pre_progress = 0
                if (self.app.status != PkgStates.NOTHING):
                    if(self.btn.text() != ""):
                        self.narrow_btn_time.start(1)
                    else:
                        self.btn.hide()
                        self.cancel_btn.show()
                        self.down_progress.btn.show()
                        self.down_progress.cont_btn.hide()
                        self.down_progress.show()
            elif (state == PkgStates.PUASE):
                # self.show_downloading_widget(appname,False,True)
                self.app.status = PkgStates.PUASE
                self.btn.hide()
                self.cancel_btn.show()
                self.down_progress.show()
                self.down_progress.puase_download(self.app.percent)
                self.check_all_check_statue.emit()

    def move_btn(self):
        self.narrow_btn_time.stop()
        self.btn_text = self.btn.text()
        self.btn.setText("")
        w = self.btn.width()
        h = self.btn.height()
        x = self.btn.x()
        y = self.btn.y()
        if (self.app.status == PkgStates.DOWNLOADING):
            if(self.btn.width()>self.btn.height()):
                self.btn.hide()
                self.btn.setFixedSize(w-1,h)
                self.btn.move(x+1,y)
                self.btn.show()
                self.remove_btn.hide()
                self.down_progress.hide()
                self.narrow_btn_time.start(4)
            else:
                self.show_downloading_widget(self.app.pkgname)
                self.btn.setText(self.btn_text)
        else:
            self.btn.setText(self.btn_text)

    # 鼠标进入事件
    def enterEvent(self, event):
        self.status = BUTTON_ENTER
        self.setGeometry(self.x(), self.y(), self.width() , self.height())

        if (Globals.THEME_COLOR == 0):
            # painter = QPalette()
            # painter.setColor(QPalette.Background, QColor(55, 144, 250, 0.1))
            # self.setPalette(painter)
            # self.setAutoFillBackground(True)
            self.setStyleSheet("QFrame{border:0px solid #ff0000;border-radius: 12px;background: rgba(55,144,250,0.1);}")
        else:
            # painter = QPalette()
            # painter.setColor(QPalette.Background, QColor(255, 255, 255,0.1))
            # self.setPalette(painter)
            # self.setAutoFillBackground(True)
            self.setStyleSheet("QFrame{background: rgba(255,255,255,0.1);border-radius: 12px;}")
        if(self.check_btn.isEnabled() == True):
            if(self.check_btn.isChecked() == False and self.btn.text() != _("installing") and self.btn.text() != _("Open") and self.btn.text() != _("installed") and self.btn.text() != _("uninstalled") and self.cancel_btn.isHidden() and self.btn.text() != _("start env") and self.remove_btn.isHidden()):
                if(hasattr(self.detail_widget,"category_widget")):
                    if(self.m_parent == self.detail_widget.mywidget.app_widget or self.m_parent == self.detail_widget.mywidget.history_widget.app_card_widget or self.m_parent == self.detail_widget.category_widget.type_show_card_widget or (hasattr(self.detail_widget,"kydroid_widget") and self.m_parent == self.detail_widget.kydroid_widget.app_widget) or (hasattr(self.detail_widget,"drive_widget") and self.m_parent == self.detail_widget.drive_widget.type_show_card_widget)):
                        self.check_btn.setStyleSheet(
                            "QCheckBox{border-radius: 8px;spacing: 0px;color:#666666;font-size:14px;background-color:rgba(0,0,0,0);border: 0px solid #8F9399;background-image: url(./res/circular_uncheck.png);}" \
                            "QCheckBox:hover{background-color:rgba(0,0,0,0);border-radius: 8px;image: url(./res/circular_uncheck.png);}" \
                            "QCheckBox::indicator{width: 16px;height: 16px;border-radius:8px;}" \
                            "QCheckBox::indicator:unchecked{background-color:rgba(0,0,0,0);border:0px solid #bfbfbf;image: url(./res/circular_uncheck.png);}" \
                            "QCheckBox::indicator:unchecked:hover{background-color:rgba(0,0,0,0);border: 0px solid #8F9399;image: url(./res/circular_uncheck.png);}" \
                            "QCheckBox::indicator:unchecked:pressed{background-color:rgba(0,0,0,0);border:0px solid #415FC4;image: url(./res/circular_uncheck.png);}" \
                            "QCheckBox::indicator{width: 16px;height: 16px;}" \
                            "QCheckBox::indicator:checked{border-radius: 8px;background-color: rgba(0,0,0,0);border:0px solid #415FC4;image: url(./res/circular_check.png);}" \
                            "QCheckBox::indicator:checked:hover{border-radius: 8px;background-color:rgba(0,0,0,0);border:0px solid #415FC4;image: url(./res/circular_check.png);}" \
                            "QCheckBox::indicator:checked:pressed{border-radius: 8px;background-color:rgba(0,0,0,0);border:0px solid #415FC4;image: url(./res/circular_check.png);}")
                        self.check_btn.show()
                else:
                    if (self.m_parent == self.detail_widget.mywidget.app_widget or self.m_parent == self.detail_widget.mywidget.history_widget.app_card_widget or (hasattr(self.detail_widget,"kydroid_widget") and self.m_parent == self.detail_widget.kydroid_widget.app_widget) or (hasattr(self.detail_widget,"drive_widget") and self.m_parent == self.detail_widget.drive_widget.type_show_card_widget)):
                        self.check_btn.setStyleSheet(
                            "QCheckBox{border-radius: 8px;spacing: 0px;color:#666666;font-size:14px;background-color:rgba(0,0,0,0);border: 0px solid #8F9399;background-image: url(./res/circular_uncheck.png);}" \
                            "QCheckBox:hover{background-color:rgba(0,0,0,0);border-radius: 8px;image: url(./res/circular_uncheck.png);}" \
                            "QCheckBox::indicator{width: 16px;height: 16px;border-radius:8px;}" \
                            "QCheckBox::indicator:unchecked{background-color:rgba(0,0,0,0);border:0px solid #bfbfbf;image: url(./res/circular_uncheck.png);}" \
                            "QCheckBox::indicator:unchecked:hover{background-color:rgba(0,0,0,0);border: 0px solid #8F9399;image: url(./res/circular_uncheck.png);}" \
                            "QCheckBox::indicator:unchecked:pressed{background-color:rgba(0,0,0,0);border:0px solid #415FC4;image: url(./res/circular_uncheck.png);}" \
                            "QCheckBox::indicator{width: 16px;height: 16px;}" \
                            "QCheckBox::indicator:checked{border-radius: 8px;background-color: rgba(0,0,0,0);border:0px solid #415FC4;image: url(./res/circular_check.png);}" \
                            "QCheckBox::indicator:checked:hover{border-radius: 8px;background-color:rgba(0,0,0,0);border:0px solid #415FC4;image: url(./res/circular_check.png);}" \
                            "QCheckBox::indicator:checked:pressed{border-radius: 8px;background-color:rgba(0,0,0,0);border:0px solid #415FC4;image: url(./res/circular_check.png);}")
                        self.check_btn.show()


    #
    # 函数：鼠标离开事件
    #

    def leaveEvent(self, event):
        self.status = BUTTON_LEAVE
        self.resize(self.width() , self.height())
        if (Globals.THEME_COLOR == 0):
            # painter = QPalette()
            # painter.setColor(QPalette.Background, QColor(247, 248, 249))
            # self.setPalette(painter)
            # self.setAutoFillBackground(True)
            self.setStyleSheet("QFrame{border:0px solid #ff0000;border-radius: 12px;background: #F7F8F9;}")
        else:
            # painter = QPalette()
            # painter.setColor(QPalette.Background, QColor(46, 46, 46,0.88))
            # self.setPalette(painter)
            # self.setAutoFillBackground(True)
            self.setStyleSheet("QFrame{background:rgba(46, 46, 46, 0.88) ;border-radius: 12px;}")
        if(self.check_btn.isChecked() == False):
            if(hasattr(self.detail_widget,"category_widget")):
                if(self.m_parent == self.detail_widget.mywidget.app_widget or self.m_parent == self.detail_widget.mywidget.history_widget.app_card_widget or self.m_parent == self.detail_widget.category_widget.type_show_card_widget or (hasattr(self.detail_widget,"kydroid_widget") and self.m_parent == self.detail_widget.kydroid_widget.app_widget) or (hasattr(self.detail_widget,"drive_widget") and self.m_parent == self.detail_widget.drive_widget.type_show_card_widget)):
                    self.check_btn.hide()
            else:
                if (self.m_parent == self.detail_widget.mywidget.app_widget or self.m_parent == self.detail_widget.mywidget.history_widget.app_card_widget or (hasattr(self.detail_widget,"kydroid_widget") and self.m_parent == self.detail_widget.kydroid_widget.app_widget) or (hasattr(self.detail_widget,"drive_widget") and self.m_parent == self.detail_widget.drive_widget.type_show_card_widget)):
                    self.check_btn.hide()

    #
    # 函数：鼠标按下事件
    #
    def mousePressEvent(self, event):
        if event.button() == Qt.LeftButton:
            self.status = BUTTON_PRESSED
            if(type(self.app) == ApkInfo):
                self.clicked_kycard.emit(self.app)
            else:
                self.clicked_card.emit(self.app)
        else:
            event.ignore()
