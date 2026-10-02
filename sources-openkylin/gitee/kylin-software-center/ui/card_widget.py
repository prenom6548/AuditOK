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
import gettext
gettext.bindtextdomain("kylin-software-center", "/usr/share/locale")
gettext.textdomain("kylin-software-center")
_ = gettext.gettext


class Card_Widget(QFrame):
    clicked_card = pyqtSignal(Application)
    clicked_kycard = pyqtSignal(ApkInfo)
    card_width = 357
    card_height = 223
    send_download_progress = pyqtSignal(int)
    puase_download = pyqtSignal(Application)
    puase_apk_download = pyqtSignal(ApkInfo)
    check_all_check_statue = pyqtSignal()

    def __init__(self, width,hight,btn_height,app,mw,detail,ca,parent=None):
        QWidget.__init__(self, parent)
        self.app = app
        self.mw = mw
        self.x_width = width
        self.y_height = hight
        self.pre_progress = 0
        self.detail_widget = detail
        self.down_widget_hight = btn_height
        self.ca = ca
        self.m_parent = parent
        self.font = QFont()
        self.font.setPixelSize(12)
        self.m_font = QFontMetrics(self.font)
        self.narrow_btn_time = QTimer()
        self.narrow_btn_time.timeout.connect(self.move_btn)
        self.init_ui()
        self.clicked_card.connect(self.detail_widget.slot_clicked_card)
        self.clicked_kycard.connect(self.detail_widget.slot_clicked_card)
        self.mw.sigenal_show_downloading_widget.connect(self.show_downloading_widget)
        self.show()

    def init_ui(self):
        self.resize(self.x_width,self.y_height)
        self.setFocusPolicy(Qt.NoFocus)
        self.setWindowFlags(Qt.FramelessWindowHint)
        self.setAttribute(Qt.WA_TranslucentBackground)
        self.setStyleSheet("QFrame{border:0px solid #ff0000;border-radius: 12px;background-color:#999999;}")

        self.icon_widget = QWidget(self)
        self.icon_widget.setAttribute(Qt.WA_TranslucentBackground)
        self.icon_widget.resize(self.width(),self.height()-6)
        self.icon_widget.setStyleSheet("QWidget{border-radius: 12px;}")
        self.icon_widget.move(0,0)

        self.hover_widget = QWidget(self)
        self.hover_widget.resize(self.width(), self.height())
        self.hover_widget.setWindowFlags(Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint)
        # self.card_list_widget.setAttribute(Qt.WA_TranslucentBackground)
        self.hover_widget.setStyleSheet("QWidget{background: rgba(0, 0, 0, 0.1);border-radius: 12px;}")
        self.hover_widget.move(0, 0)
        self.hover_widget.hide()

        self.check_btn = QCheckBox(self.icon_widget)
        self.check_btn.setFixedSize(16,16)
        self.check_btn.setFocusPolicy(Qt.NoFocus)
        self.check_btn.setCheckState(Qt.Unchecked)
        self.check_btn.setStyleSheet(
            "QCheckBox{border-radius: 10px;spacing: 0px;color:#666666;font-size:14px;background-color:#FFFFFF;background-image:url(./res/circular_uncheck.png);}" \
            "QCheckBox:hover{background-color:#DAE3FA;background-image:url(./res/circular_uncheck.png);}" \
            "QCheckBox::indicator{width: 20px;height: 20px;border-radius:10px;}" \
            "QCheckBox::indicator:unchecked{background-color:#FFFFFF;border:1px solid #bfbfbf;}" \
            "QCheckBox::indicator:unchecked:hover{background-color:#DAE3FA;border:1px solid #6B8EEB;}" \
            "QCheckBox::indicator:unchecked:pressed{background-color:#BCCCF6;border:1px solid #415FC4;}" \
            "QCheckBox::indicator{width: 20px;height: 20px;}" \
            "QCheckBox::indicator:checked{background-color:#3D6BE5;border:1px solid #415FC4;image: url(./res/check);}" \
            "QCheckBox::indicator:checked:hover{background-color:#6B8EEB;border:1px solid #415FC4;image: url(./res/check);}" \
            "QCheckBox::indicator:checked:pressed{background-color:#415FC4;border:1px solid #415FC4;image: url(./res/check);}")
        self.check_btn.move(10,10)
        self.check_btn.hide()

        self.icon_label = QLabel(self.icon_widget)
        self.icon_label.resize(self.width(),self.height()-6)
        self.icon_label.move(0,0)
        iconpath = commontools.get_coverimage_path(self.app.pkgname)
        # self.icon_label.setStyleSheet("QLabel{border-radius: 12px;border-image: url('"+iconpath+"') 0 0 0 0 stretch strectch;background-color:transparent;}")
        if(Globals.DEVICE == 1):
            self.icon_label.setStyleSheet("QLabel{border-radius: 12px;background-color:transparent;}")
            self.pix2 = QPixmap(iconpath)
            #图片按照要求提供尺寸修改大小，Qt.IgnoreAspectRatio(忽略图片自身横纵比)
            self.fitpixmap_userIcon = self.pix2.scaled(self.card_width, self.card_height, Qt.IgnoreAspectRatio, Qt.SmoothTransformation)
            self.fitpixmap_userIcon = self.PixmapToRound(self.fitpixmap_userIcon, self.card_width,self.card_height)
            self.icon_label.setPixmap(self.fitpixmap_userIcon)
        else:
            self.icon_label.setStyleSheet("QLabel{border-image:url('" + iconpath + "');border-radius: 12px;}")
        # self.icon_label.setStyleSheet("QLabel{background-image:url('" + iconpath + "');border-radius: 12px;background-repeat: repeat-xy;background-position: center;background-attachment: fixed;background-clip: padding;}")
        self.icon_label.setScaledContents(True)

        self.down_widget = QFrame(self)
        self.down_widget.resize(self.width(), self.down_widget_hight)
        self.down_widget.move(0, self.height()-self.down_widget_hight)
        self.down_widget.setStyleSheet("QFrame{background-color: #F4F4F4;border-bottom-left-radius:12px;border-bottom-right-radius:12px;border-top-left-radius:0px;border-top-right-radius:0px;}")
        self.down_widget.show()

        self.name_label = QLabel(self.down_widget)
        self.name_label.setAttribute(Qt.WA_TranslucentBackground)
        self.name_label.setMaximumWidth(150)
        self.name_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #303133;line-height: 17px;}")
        if(Globals.LAN == "zh_CN"):
            if self.app.displayname_cn != '' and self.app.displayname_cn is not None and self.app.displayname_cn != 'None':
                setLongTextToElideFormat(self.name_label, self.app.displayname_cn)
                # setLongTextToElideFormat(self.ui.named, self.app.displayname_cn)
            else:
                setLongTextToElideFormat(self.name_label, self.app.pkgname)
        else:
            setLongTextToElideFormat(self.name_label, self.app.pkgname)
        self.name_label.adjustSize()
        self.name_label.move(10,int((self.down_widget.height()-self.name_label.height())/2))

        self.line_label = QLabel(self.down_widget)
        self.line_label.setFixedSize(1,17)
        self.line_label.setStyleSheet("QLabel{background-color: #303133;}")
        self.line_label.move(self.name_label.x()+self.name_label.width()+10,int((self.down_widget.height()-self.line_label.height())/2))

        self.category_label = QLabel(self.down_widget)
        self.category_label.setAttribute(Qt.WA_TranslucentBackground)
        self.category_label.resize(28,20)
        self.category_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #8F9399;line-height: 17px;}")
        if self.app.category_name_cn != '' and self.app.category_name_cn is not None and self.app.category_name_cn != 'None' and Globals.LAN == "zh_CN":
            self.category_label.setText(self.app.category_name_cn.split("、")[0])
        else:
            self.category_label.setText(self.app.category_name.split("、")[0])
        self.category_label.adjustSize()
        self.category_label.move(self.line_label.width()+self.line_label.x()+4,int((self.down_widget.height()-self.category_label.height())/2))

        self.btn = QPushButton(self.down_widget)
        self.btn.setFocusPolicy(Qt.NoFocus)
        self.btn.setStyleSheet("QPushButton{font-size: 14px;font-weight: 400;color: #FFFFFF;line-height: 12px;border-radius: 12px;border: 0px solid #3790FA;background: #3790FA;}QPushButton:hover{background: #40A9FB;border-radius: 12px;color: #ffffff;font-weight: 400;border:0px;}QPushButton:pressed{background: #296CD9;border-radius: 12px;color: #ffffff;font-weight: 400;border:0px;}")
        self.btn.resize(52,24)
        self.btn.move(self.down_widget.width()-self.btn.width()-10,int((self.down_widget.height()-self.btn.height())/2))
        self.btn.show()
        self.btn.clicked.connect(lambda: self.mw.slot_click_action_btn(self, self.btn.text(), self.app))

        self.down_progress = Circular_Progress_Bar(22, 22,self.app.pkgname,self.down_widget)
        self.down_progress.update_progress(self.app.percent)
        self.down_progress.move(self.down_widget.width() - self.down_progress.width() - 10,
                                int((self.down_widget.height() - self.down_progress.height()) / 2))
        self.send_download_progress.connect(self.down_progress.GenUpdate)
        self.down_progress.hide()

        self.cancel_btn = QPushButton(self.down_widget)
        self.cancel_btn.setFocusPolicy(Qt.NoFocus)
        self.cancel_btn.resize(22, 22)
        self.cancel_btn.setStyleSheet("QPushButton{background-image:url(./res/cancel1.png);border-radius:11px;}QPushButton:hover{background-image:url(./res/cancel1_hover.png);border-radius:11px;}")
        # self.cancel_btn.resize(48, 22)
        # self.cancel_btn.setStyleSheet("QPushButton{font-size: 14px;font-weight: 400;color: #FFFFFF;line-height: 14px;text-align: center;background: rgba(0, 0, 0, 0.7);border-radius: 11px;}QPushButton:hover{background: #000000;}QPushButton:pressed{background: #000000;}")
        # self.cancel_btn.setText(_("cancel"))
        self.cancel_btn.move(self.down_progress.x() - self.cancel_btn.width() - 8, self.down_progress.y())
        self.cancel_btn.hide()

        self.remove_btn = uninstall_btn(self.down_widget)

        self.remove_btn.move(self.width() - self.remove_btn.width() - 20, int((self.down_widget.height() - self.remove_btn.height())/2))
        self.remove_btn.hide()
        self.cancel_btn.clicked.connect(lambda: self.mw.slot_click_cancel_btn(self, self.app))
        self.down_progress.btn.clicked.connect(self.slot_puase_download)
        self.down_progress.cont_btn.clicked.connect(lambda :self.mw.re_download_app(self.app))
        self.change_btn_state(self.app.pkgname,self.app.status)
        self.slot_set_dark_mode()


    def re_move(self):
        self.hover_widget.resize(self.width(),self.height())
        self.icon_widget.resize(self.width(),self.height())
        self.icon_widget.move(0,0)
        self.icon_label.resize(self.width(), self.height() - 2)
        iconpath = commontools.get_coverimage_path(self.app.pkgname)
        if (Globals.DEVICE == 1):
            self.icon_label.setStyleSheet("QLabel{border-radius: 12px;background-color:transparent;}")
            self.pix2 = QPixmap(iconpath)
            fitpixmap_userIcon = self.pix2.scaled(self.width(), self.height() - 2, Qt.IgnoreAspectRatio, Qt.SmoothTransformation)
            fitpixmap_userIcon = self.PixmapToRound(fitpixmap_userIcon, self.width(), self.height() - 2)
            self.icon_label.setPixmap(fitpixmap_userIcon)
            self.icon_label.setScaledContents(True)
        else:
            self.icon_label.setStyleSheet("QLabel{border-image:url('" + iconpath + "');border-radius: 12px;}")
        # self.icon_label.setStyleSheet(
            # "QLabel{background-image:url('" + iconpath + "');border-radius: 12px;background-repeat: repeat-xy;background-position: center;background-attachment: fixed;background-clip: padding;}")

        self.icon_label.move(0, 0)
        self.down_widget.resize(self.width(), self.down_widget_hight)
        self.down_widget.move(0, self.height() - self.down_widget_hight)
        self.name_label.move(10, int((self.down_widget.height() - self.name_label.height()) / 2))
        self.line_label.move(self.name_label.x() + self.name_label.width() + 10,
                             int((self.down_widget.height() - self.line_label.height()) / 2))
        self.category_label.move(self.line_label.width() + self.line_label.x() + 4,
                                 int((self.down_widget.height() - self.category_label.height()) / 2))
        self.btn.move(self.down_widget.width() - self.btn.width() - 10,
                      int((self.down_widget.height() - self.btn.height()) / 2))
        self.down_progress.move(self.down_widget.width() - self.down_progress.width() - 10,
                                int((self.down_widget.height() - self.down_progress.height()) / 2))
        self.cancel_btn.move(self.down_progress.x() - self.cancel_btn.width() - 8, self.down_progress.y())
        self.remove_btn.move(self.width() - self.remove_btn.width() - 20,
                             int((self.down_widget.height() - self.remove_btn.height()) / 2))

    #将图片绘制为圆角
    def PixmapToRound(self,src,x,y):
        pixmapa = QPixmap(src)
        pixmap = QPixmap(x, y)
        pixmap.fill(Qt.transparent)
        painter = QPainter(pixmap)
        painter.setRenderHint(QPainter.Antialiasing, True)
        painter.setRenderHint(QPainter.TextAntialiasing, True)  #文字尽量抗锯齿
        # painter.setRenderHint(QPainter.SmoothPixmapTransform, True)
        # painter.setRenderHints(QPainter.Antialiasing | QPainter.SmoothPixmapTransform)
        path = QPainterPath()
        rect = QRectF(0,0,x,y)
        path.addRoundedRect(rect, 12.0, 12.0)
        painter.setClipPath(path)
        painter.drawPixmap(0, 0, x, y, pixmapa)

        return pixmap


    #
    #函数：设置按钮的样式
    #
    def set_btn_stylesheet(self,is_uninstall = False):
        if(is_uninstall == True):
            self.btn.setEnabled(True)
            self.btn.setStyleSheet("QPushButton{font-size: 14px;font-weight: 400;color: #3790FA;line-height: 12px;text-align: center;background: rgba(55, 144, 250, 0.08);border-radius: 12px;background:transparent;}QPushButton:hover{background: #F56C6C;;border-radius: 12px;color: #ffffff;}QPushButton:pressed{background: #F56C6C;border-radius: 12px;color: #ffffff;}")

    #
    # 函数：设置深色模式
    #
    def slot_set_dark_mode(self):
        if (Globals.THEME_COLOR == 0):
            self.setStyleSheet("QFrame{border:0px solid #ff0000;border-radius: 12px;background-color:#ffffff;}")
            self.name_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #303133;line-height: 17px;}")
            self.category_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #8F9399;line-height: 17px;}")
            self.down_widget.setStyleSheet("QFrame{background-color: #F4F4F4;border-bottom-left-radius:12px;border-bottom-right-radius:12px;border-top-left-radius:0px;border-top-right-radius:0px;}")
            self.line_label.setStyleSheet("QLabel{background-color: #303133;}")
            self.cancel_btn.setStyleSheet("QPushButton{background-image:url(./res/cancel1.png);border-radius:11px;}QPushButton:hover{background-image:url(./res/cancel1_hover.png);border-radius:11px;}")
        else:
            self.setStyleSheet("QFrame{background: rgba(46, 46, 46, 0.88);border-radius: 12px;}")
            self.name_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #F9F9F9;line-height: 17px;}")
            self.category_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #8F9399;line-height: 17px;}")
            self.down_widget.setStyleSheet("QFrame{background: #2E2E2E;border-bottom-left-radius:12px;border-bottom-right-radius:12px;border-top-left-radius:0px;border-top-right-radius:0px;}")
            self.line_label.setStyleSheet("QLabel{background-color: #8F9399;}")
            self.cancel_btn.setStyleSheet(
                "QPushButton{background-image:url(./res/cancel_dark.png);border-radius:11px;}QPushButton:hover{background-image:url(./res/cancel_hover_dark.png);border-radius:11px;}")
        self.down_progress.reflash_color()
        # QApplication.processEvents()


    #
    #函数：处理点击暂停
    #
    def slot_puase_download(self):
        if(type(self.app) != ApkInfo):
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

    def change_btn_width(self):
        s = self.m_font.boundingRect(self.btn.text()).size().width()
        if (s +18< 52):
            self.btn.setFixedSize(52,24)
        else:
            self.btn.setFixedSize(s+24,24)
        # print("00000",self.btn.width())
        self.btn.move(self.down_widget.width()-self.btn.width()-10,int((self.down_widget.height()-self.btn.height())/2))

    def change_kydroid_btn_statue(self):
        if(type(self.app) == ApkInfo):
            for i in self.mw.at.appmgr.apk_list:
                if(i.pkgname == self.app.pkgname):
                    self.app.is_installed = i.is_installed
                    self.app.is_upgradable = i.is_upgradable
            self.check_btn_text()

    #
    #函数：获取按钮的名字
    #
    def check_btn_text(self):
        self.btn.resize(52, 24)
        self.btn.move(self.down_widget.width() - self.btn.width() - 10,
                      int((self.down_widget.height() - self.btn.height()) / 2))
        self.remove_btn.stop_loading()
        self.btn.setCheckable(True)
        self.btn.setEnabled(True)
        self.remove_btn.hide()
        self.cancel_btn.hide()
        self.down_progress.hide()
        if (self.app.status == PkgStates.REMOVING):
            self.slot_uninstall(self.app.pkgname, "uninstalling")
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
                    if (self.app.pkgname == "kylin-installer"):
                        has_desktop = False
                    else:
                        has_desktop = self.check_app_desktop(self.app.pkgname)
                    if(has_desktop == True):
                        self.btn.setText(_("Open"))
                        self.change_btn_width()
                        self.btn.setStyleSheet("QPushButton{font-size: 14px;font-weight: 400;color: #3790FA;line-height: 12px;border-radius: 12px;background: rgba(55, 144, 250, 0.12);}QPushButton:hover{background: #3790FA;border-radius: 12px;color: #ffffff;font-weight: 400;border:0px;}QPushButton:pressed{background: #296CD9;border-radius: 12px;color: #ffffff;font-weight: 400;border:0px;}")
                    else:
                        self.btn.setText(_("installed"))
                        self.change_btn_width()
                        self.btn.setEnabled(False)
                        self.btn.setStyleSheet("QPushButton{font-size: 14px;font-weight: 400;color: #3790FA;line-height: 12px;background: rgba(55, 144, 250, 0.12);border-radius: 12px;}")
                    pass
            else:
                self.btn.setText(_("installed"))
                self.change_btn_width()
        else:
            if (self.app != None):
                if (Globals.APK_EVNRUN == 0):
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
                        self.btn.setText(_("Open"))
                        self.change_btn_width()
                        self.btn.setStyleSheet("QPushButton{font-size: 14px;font-weight: 400;color: #3790FA;line-height: 12px;border-radius: 12px;background: rgba(55, 144, 250, 0.12);}QPushButton:hover{background: #3790FA;border-radius: 12px;color: #ffffff;font-weight: 400;border:0px;}QPushButton:pressed{background: #296CD9;border-radius: 12px;color: #ffffff;font-weight: 400;border:0px;}")
                        pass
            else:
                self.btn.setText(_("down"))
                self.change_btn_width()
                self.btn.self.btn.setStyleSheet("QPushButton{font-size: 14px;font-weight: 400;color: #FFFFFF;line-height: 12px;border-radius: 12px;border: 0px solid #3790FA;background: #3790FA;}QPushButton:hover{background: #40A9FB;border-radius: 12px;color: #ffffff;font-weight: 400;border:0px;}QPushButton:pressed{background: #296CD9;border-radius: 12px;color: #ffffff;font-weight: 400;border:0px;}")
        self.btn.show()
        self.set_btn_stylesheet()

    #
    #函数：重置按钮
    #
    def reload_btn(self,appname):
        if(appname == self.app.pkgname):
            self.check_btn_text()

    #
    #函数：进度显示
    #
    def slot_get_download_progress(self,appname,progress):
        if(appname == self.app.pkgname):
            if(progress == 100):
                self.narrow_btn_time.stop()
                self.cancel_btn.hide()
                self.btn.resize(52, 24)
                self.btn.move(self.down_widget.width() - self.btn.width() - 10,
                              int((self.down_widget.height() - self.btn.height()) / 2))
                self.btn.setCheckable(False)
                self.btn.setText(_("installing"))
                self.change_btn_width()
                self.btn.setStyleSheet("QPushButton{font-size: 14px;font-weight: 400;color: #3790FA;line-height: 12px;background: rgba(55, 144, 250, 0.12);border-radius: 12px;}")
                self.btn.show()
                self.down_progress.hide()
            else:
                self.cancel_btn.show()
                self.btn.hide()
                if (self.app.status != PkgStates.PUASE):
                    if (progress - self.pre_progress >= 2):
                        self.send_download_progress.emit(progress)
                        self.pre_progress = progress
                    else:
                        self.send_download_progress.emit(self.pre_progress)

    #
    #函数：显示下载时卡片界面的显示
    #
    def show_downloading_widget(self, appname,is_cancel=False,is_puase = False):
        if(appname == self.app.pkgname):
            if (self.app.status != PkgStates.NOTHING):
                if (is_cancel == False):
                    if(is_puase == False):
                        if (self.app.status != PkgStates.INSTALLING):
                            self.btn.hide()
                            self.cancel_btn.show()
                            self.down_progress.show()
                            self.down_progress.btn.show()
                            self.down_progress.cont_btn.hide()
                    else:
                        self.btn.setText(_("cont"))
                        self.change_btn_width()
                        self.btn.setStyleSheet("QPushButton{font-size: 14px;font-weight: 400;color: #3790FA;line-height: 12px;border-radius: 12px;background: rgba(55, 144, 250, 0.12);}QPushButton:hover{background: #3790FA;border-radius: 12px;color: #ffffff;font-weight: 400;border:0px;}QPushButton:pressed{background: #3790FA;border-radius: 12px;color: #ffffff;font-weight: 400;border:0px;}")
                        self.btn.hide()
                        self.cancel_btn.show()
                        self.down_progress.show()
                else:
                    self.btn.resize(52, 24)
                    self.btn.move(self.width() - self.btn.width() - 20, self.height() - self.btn.height() - 20)
                    self.btn.show()
                    self.reload_btn(appname)
                    self.cancel_btn.hide()
                    self.down_progress.hide()
                self.send_download_progress.emit(self.app.percent)
            else:
                self.check_btn_text()

    def slot_uninstall(self,appname,state):
        if (appname == self.app.pkgname):
            self.btn.resize(52, 24)
            self.btn.move(self.down_widget.width() - self.btn.width() - 10,
                          int((self.down_widget.height() - self.btn.height()) / 2))
            self.cancel_btn.hide()
            self.down_progress.hide()
            if (state == "uninstalling"):
                self.btn.setText(_("uninstalling"))
                self.change_btn_width()
                self.btn.hide()
                self.remove_btn.start_loading()
                self.remove_btn.show()
            else:
                self.btn.setText(_("uninstalled"))
                self.change_btn_width()
                self.btn.setStyleSheet("QPushButton{font-size: 14px;font-weight: 400;color: #F8F8F8;line-height: 12px;text-align: center;background: #C0C4CC;border-radius: 12px;}")

    def change_btn_state(self,appname,state):
        if(appname == self.app.pkgname):
            if(state == PkgStates.NOTHING or state == PkgStates.REMOVED):
                self.app.status = PkgStates.NOTHING
                self.check_btn_text()
                self.check_all_check_statue.emit()
            if(state == PkgStates.INSTALLING):
                self.slot_get_download_progress(appname,100)
            if(state == PkgStates.REMOVING):
                self.slot_uninstall(appname,"uninstalling")
            if(state == PkgStates.INSTALLED):
                self.app.is_installed = True
                self.app.is_upgradable = False
                self.app.status = PkgStates.NOTHING
                self.reload_btn(appname)
                self.check_all_check_statue.emit()
            if(state == PkgStates.DOWNLOADING):
                if (self.btn.isHidden() == False):
                    self.down_progress.init_down = True
                    self.down_progress.persent = 0
                    self.pre_progress = 0
                if (self.app.status != PkgStates.NOTHING):
                    if (self.btn.text() != ""):
                        self.narrow_btn_time.start(1)
                    else:
                        self.btn.hide()
                        self.cancel_btn.show()
                        self.down_progress.btn.show()
                        self.down_progress.cont_btn.hide()
                        self.down_progress.show()
                # self.show_downloading_widget(appname)
            if (state == PkgStates.PUASE):
                # self.show_downloading_widget(appname,False,True)
                self.app.status = PkgStates.PUASE
                self.btn.hide()
                self.cancel_btn.show()
                self.down_progress.show()
                self.down_progress.puase_download(self.app.percent)
                self.check_all_check_statue.emit()

    def move_btn(self):
        self.narrow_btn_time.stop()
        self.btn.setText("")
        w = self.btn.width()
        h = self.btn.height()
        x = self.btn.x()
        y = self.btn.y()
        if (self.app.status != PkgStates.NOTHING):
            if(self.btn.width()>self.btn.height()):
                self.btn.hide()
                self.btn.setFixedSize(w-1,h)
                self.btn.move(x+1,y)
                self.cancel_btn.hide()
                self.down_progress.hide()
                self.btn.show()
                self.narrow_btn_time.start(4)
            else:
                self.show_downloading_widget(self.app.pkgname)

    #
    # 函数：刷新背景颜色
    #
    def refresh_background_color(self):

        if (Globals.THEME_COLOR == 0):
            self.setStyleSheet("QFrame{border:0px solid #ff0000;border-radius: 12px;background-color:#999999;}")
        else:

            self.setStyleSheet("QFrame{border:0px solid #ff0000;border-radius: 12px;background-color:#999999;}")

    # 鼠标进入事件
    def enterEvent(self, event):
        self.status = BUTTON_ENTER
        self.setGeometry(self.x(), self.y(), self.width() , self.height())
        self.hover_widget.show()
        self.shadow = QGraphicsDropShadowEffect(self)
        self.shadow.setOffset(0, 0)
        self.shadow.setColor(QColor(qRgba(149,149,149,0)))
        self.shadow.setBlurRadius(20)
        self.setGraphicsEffect(self.shadow)

    #
    # 函数：鼠标离开事件
    #

    def leaveEvent(self, event):
        self.status = BUTTON_LEAVE
        self.hover_widget.hide()
        self.resize(self.width(),self.height())
        self.shadow = QGraphicsDropShadowEffect(self)
        self.shadow.setBlurRadius(0)
        self.shadow.setOffset(0, 0)
        self.setGraphicsEffect(self.shadow)

    #
    # 函数：鼠标按下事件
    #
    def mousePressEvent(self, event):
        if event.button() == Qt.LeftButton:
            self.status = BUTTON_PRESSED
            if (type(self.app) == ApkInfo):
                self.clicked_kycard.emit(self.app)
            else:
                self.clicked_card.emit(self.app)
        else:
            event.ignore()