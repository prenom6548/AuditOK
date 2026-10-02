#!/usr/bin/python3
# -*- coding: utf-8 -*-

from PyQt5.QtGui import *
from PyQt5.QtCore import *
from PyQt5.QtWidgets import QFrame,QWidget,QLabel
import math
import gettext
from ui.systembutton import *
from models.globals import *
from utils.commontools import *
from utils.run import *
from models.enums import UKSC_CACHE_DIR
gettext.bindtextdomain("kylin-software-center", "/usr/share/locale")
gettext.textdomain("kylin-software-center")
_ = gettext.gettext


class Contact_Us_Widget(QFrame):
    def __init__(self,mw,parent=None):
        QFrame.__init__(self,parent)
        self.mw = mw
        self.mouse_press = False
        self.setFixedSize(420+20,560+20)
        self.paintEvent = self.set_paintEvent
        self.setWindowFlags(Qt.FramelessWindowHint|Qt.Tool)
        self.setWindowTitle(_("Software Store"))
        self.setAttribute(Qt.WA_TranslucentBackground)

        self.widget = QWidget(self)
        self.widget.resize(self.width()-20,self.height()-20)
        self.widget.move(10,10)
        # self.widget.setStyleSheet("QWidget{border-radius:6px;background-color:#ffffff;border: 1px solid rgba(38, 38, 38, 0.15);}")

        self.logo_icon_label = QPushButton(self.widget)
        self.logo_icon_label.setFixedSize(24, 24)
        # self.logo_icon_label.setFrameShape(QFrame.NoFrame)
        self.logo_icon_label.setAutoFillBackground(True)
        # self.logo_icon_label.setScaledContents(True)
        iconpath = get_title_icon(Globals.APPNAME)
        self.logo_icon_label.setIcon(QIcon(iconpath))
        self.logo_icon_label.setIconSize(QSize(24, 24))
        self.logo_icon_label.setStyleSheet("QPushButton{background:transparent;border:0px;}")
        self.logo_icon_label.move(8, 8)
        self.logo_icon_label.show()

        self.titel_label = QLabel(self.widget)
        self.titel_label.setFixedSize(200,24)
        # self.titel_label.adjustSize()
        # self.titel_label.setFixedWidth(160)
        self.titel_label.setAttribute(Qt.WA_TranslucentBackground)
        self.titel_label.setStyleSheet("QLabel{font-size: 14px;color: #1B1B1B;}")
        self.titel_label.setText(_("Software Store"))
        self.titel_label.setAlignment(Qt.AlignVCenter)
        self.titel_label.adjustSize()
        self.titel_label.move(self.logo_icon_label.x()+self.logo_icon_label.width()+8,8)
        self.titel_label.show()

        self.close_btn = QPushButton(self.widget)
        self.close_btn.resize(30,30)
        self.close_btn.setFocusPolicy(Qt.NoFocus)
        self.close_btn.move(self.widget.width()-4-self.close_btn.width(),4)
        # self.close_btn.clicked.connect(self.slot_click_close_btn)
        self.close_btn.installEventFilter(self)
        self.close_btn.setToolTip(_("close"))

        self.big_logo = QLabel(self.widget)
        self.big_logo.setFixedSize(96,96)
        iconpath = get_big_icon_path(Globals.APPNAME)
        # label_pixmap = QPixmap(iconpath)
        # self.big_logo.setPixmap(label_pixmap)
        self.big_logo.setStyleSheet("QLabel{border-image: url("+iconpath+") 0 0 0 0 stretch strectch;}")
        # self.big_logo.setStyleSheet("QLabel{background:transparent;border:0px;}")
        self.big_logo.move(int((self.widget.width()-self.big_logo.width())/2),60)

        self.app_name_label = QLabel(self.widget)
        self.app_name_label.setFixedHeight(28)
        self.app_name_label.setStyleSheet("QLabel{font-size: 18px;font-weight: 500;color: #262626;line-height: 28px;}")
        self.app_name_label.setText(_("Software Store"))
        self.app_name_label.adjustSize()
        self.app_name_label.setAlignment(Qt.AlignVCenter)
        self.app_name_label.move(int((self.widget.width()-self.app_name_label.width())/2),self.big_logo.height()+self.big_logo.y()+16)

        self.app_version_label = QLabel(self.widget)
        self.app_version_label.setFixedHeight(34)
        self.app_version_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #595959;line-height: 24px;}")
        self.app_version_label.setText(_("versions:") + Globals.UKSC_VERSION.split("kylin")[0])
        self.app_version_label.adjustSize()
        self.app_version_label.setAlignment(Qt.AlignVCenter)
        self.app_version_label.move(int((self.widget.width() - self.app_version_label.width())/2),self.app_name_label.y()+self.app_name_label.height()+12)

        self.discription_label = QLabel(self.widget)
        self.discription_label.setFixedWidth(self.widget.width()-32*2)
        self.discription_label.setWordWrap(True)
        # self.discription_label.setAlignment(Qt.AlignHCenter)
        # self.discription_label.setAlignment(Qt.AlignLeft | Qt.AlignVCenter)
        self.discription_label.setStyleSheet("QLabel{font-size: 14px;color: #595959;line-height: 24px;text-align:justify;}")
        # self.discription_label.setText(_("软件商店是一款内容丰富的图形化软件管理工具，支持新品上架、热门应用推荐功能，提供软件搜索、下载、安装、更新、卸载等一站式软件服务。"))
        self.discription_label.setText(_("Software store is a graphical software management tool with rich content. It supports the recommendation of new products and popular applications, and provides one-stop software services such as software search, download, installation, update and uninstall."))
        # self.discription_label.setText("软件商店是一款内容丰富的图形化软件管理工具，支持新品上架、热门应用等推荐功能，提供软件搜索、下载、安装、更新、卸载等一站式软件服务。")
        self.discription_label.adjustSize()
        self.discription_label.move(int((self.widget.width()-self.discription_label.width())/2),self.app_version_label.y()+self.app_version_label.height()+12)


        self.contact_us_label = QLabel(self.widget)
        self.contact_us_label.setStyleSheet("QLabel{font-size: 14px;color: #595959;line-height: 24px;text-align:justify;}")
        # self.contact_us_label.setFixedWidth(self.widget.width()-32*2)
        # self.contact_us_label.setWordWrap(True)
        self.contact_us_label.setText(_("Service and support team: ") +  '<a href="mailto://sopport@kylinos.cn\"'
                                                                         'style=\"color:#595959;\">'
                                                                         'support@kylinos.cn</a>')
        self.contact_us_label.adjustSize()
        self.contact_us_label.linkActivated.connect(self.link_handler)
        self.contact_us_label.setContextMenuPolicy(Qt.NoContextMenu)
        self.contact_us_label.move(32, self.discription_label.y() + self.discription_label.height() + 20)
        # self.contact_us_label.move(int((self.widget.width()-self.contact_us_label.width())/2), self.discription_label.y() + self.discription_label.height() + 12)
        # self.contact_us_label.setOpenExternalLinks(True)
        # self.contact_us_label.setTextInteractionFlags(Qt.TextSelectableByMouse)
        # self.contact_us_label.setText(_("Service and support team: ") + Globals.SUPPORT_MAILBOX)

        self.disclaimers_label = QLabel(self.widget)
        self.disclaimers_label.setStyleSheet(
            "QLabel{font-size: 14px;color: #595959;line-height: 24px;text-align:justify;}")
        # self.disclaimers_label.setFixedWidth(self.widget.width() - 32 * 2)
        # self.disclaimers_label.setWordWrap(True)
        self.disclaimers_label.setText(_("Disclaimers:") + '<a href="Disclaimer\"''style=\"color:#595959;\">'
                                                                        '' + "《" + _("Kylin Software Store Disclaimer") + "》" + ' </a>')
        self.disclaimers_label.linkActivated.connect(self.click_disclaimers)
        self.disclaimers_label.adjustSize()
        # self.disclaimers_label.move(int((self.widget.width()-self.disclaimers_label.width())/2),self.contact_us_label.y()+self.contact_us_label.height()+5)
        self.disclaimers_label.move(self.contact_us_label.x(),self.contact_us_label.y()+self.contact_us_label.height()+5)
        # self.disclaimers_label.hide()

        self.widget.setFixedHeight(self.disclaimers_label.y()+self.disclaimers_label.height()+40)
        self.setFixedHeight(self.widget.height()+20)
        self.slot_show_dark_mode()

    #
    # 函数：重新设置icon
    #
    def change_icon_show(self):
        iconpath = get_title_icon(Globals.APPNAME)
        self.logo_icon_label.setIcon(QIcon(iconpath))
        self.logo_icon_label.setIconSize(QSize(24, 24))
        self.logo_icon_label.setStyleSheet("QPushButton{background:transparent;border:0px;}")
        # self.logo_icon_label.setIcon(QIcon.fromTheme("kylin-software-center"))
        iconpath = get_big_icon_path(Globals.APPNAME)
        self.big_logo.setStyleSheet("QLabel{border-image: url(" + iconpath + ") 0 0 0 0 stretch strectch;}")
        self.big_logo.hide()
        self.big_logo.show()


    #
    #函数：点击免责说明
    #
    def click_disclaimers(self,text):
        url = Globals.DISCLAIMERS
        webbrowser_open_url(url)

    #
    #函数：点击发送邮件
    #
    def link_handler(self,url):
        # webbrowser_open_url(url)
        # os.popen("evolution")
        # QDesktopServices.openUrl(QUrl(url))
        try:
            config_home = UKSC_CACHE_DIR.rsplit("/", 2)[0] + "/.config/"
            mimeapps_path = config_home + "mimeapps.list"
            with open(mimeapps_path, "r", encoding="utf-8") as mine_file:
                for line in mine_file.readlines():
                    if line.find("x-scheme-handler/mailto") != -1:
                        print(line)
                        mailto_name = (line.rsplit("=", 1)[1]).rsplit(".", 1)[0]
                        if Globals.DEBUG_SWITCH:
                            print("broswer name: %s" % mailto_name)
                        break
            if (mailto_name == "org.gnome.Evolution"):
                try:
                    app = self.mw.detailwidget.ca["evolution"]
                except:
                    app = None
                if(app != None):
                    if (app.candidate != None and app.is_installed == True):
                        self.mw.ask_thread.run_iface.call_run_app("evolution mailto:")
                        return
            elif(mailto_name == "thunderbird"):
                try:
                    app = self.mw.detailwidget.ca["evolution"]
                except:
                    app = None
                if(app != None):
                    if (app.candidate != None and app.is_installed == True):
                        self.mw.ask_thread.run_iface.call_run_app("thunderbird -compose")
                        return
            elif (mailto_name == "claws-mail"):
                try:
                    app = self.mw.detailwidget.ca["claws-mail"]
                except:
                    app = None
                if (app != None):
                    if (app.candidate != None and app.is_installed == True):
                        self.mw.ask_thread.run_iface.call_run_app("claws-mail")
                        return
        except:
                pass
        try:
            app = self.mw.detailwidget.ca["evolution"]
            if(app.candidate != None and app.is_installed == True):
                # cmd = run_app("evolution")
                # self.mw.ask_thread.run_iface.call_run_app(cmd)
                self.mw.ask_thread.run_iface.call_run_app("evolution mailto:")
            else:
                try:
                    app = self.mw.detailwidget.ca["thunderbird"]
                    if (app.candidate != None and app.is_installed == True):
                        # cmd =  run_app("thunderbird")
                        self.mw.ask_thread.run_iface.call_run_app("thunderbird -compose")
                    else:
                        try:
                            app = self.mw.detailwidget.ca["claws-mail"]
                            if (app.candidate != None and app.is_installed == True):
                                # cmd = run_app("claws-mail")
                                # self.mw.ask_thread.run_iface.call_run_app(cmd)
                                self.mw.ask_thread.run_iface.call_run_app("claws-mail")
                        except:
                            pass
                except:
                    try:
                        app = self.mw.detailwidget.ca["claws-mail"]
                        if (app.candidate != None and app.is_installed == True):
                            # cmd = run_app("claws-mail")
                            # self.mw.ask_thread.run_iface.call_run_app(cmd)
                            self.mw.ask_thread.run_iface.call_run_app("claws-mail")
                    except:
                        pass
        except:
            try:
                app = self.mw.detailwidget.ca["thunderbird"]
                if (app.candidate != None and app.is_installed == True):
                    # cmd = run_app("thunderbird")
                    # self.mw.ask_thread.run_iface.call_run_app(cmd)
                    self.mw.ask_thread.run_iface.call_run_app("thunderbird -compose")
                else:
                    try:
                        app = self.mw.detailwidget.ca["claws-mail"]
                        if (app.candidate != None and app.is_installed == True):
                            # cmd = run_app("claws-mail")
                            # self.mw.ask_thread.run_iface.call_run_app(cmd)
                            self.mw.ask_thread.run_iface.call_run_app("claws-mail")
                    except:
                        pass
            except:
                try:
                    app = self.mw.detailwidget.ca["claws-mail"]
                    if (app.candidate != None and app.is_installed == True):
                        # cmd = run_app("claws-mail")
                        self.mw.ask_thread.run_iface.call_run_app("claws-mail")
                except:
                    pass

    #
    #函数：处理主题颜色的显示
    #
    def slot_show_dark_mode(self):
        if(Globals.THEME_COLOR == 0 ):
            if(Globals.RIGHT_ANGLE):
                self.widget.setStyleSheet("QWidget{background-color:#ffffff;}")
                self.setStyleSheet("QFrame{background-color:#ffffff;}")
            else:
                self.widget.setStyleSheet("QWidget{border-radius:12px;background-color:#ffffff;}")
                self.setStyleSheet("QFrame{border-radius:12px;background-color:#ffffff;}")
            self.titel_label.setStyleSheet("QLabel{font-size: 14px;color: #1B1B1B;}")
            self.app_name_label.setStyleSheet("QLabel{font-size: 18px;color: #1B1B1B;line-height: 28px;}")
            self.app_version_label.setStyleSheet("QLabel{font-size: 14px;color: #595959;line-height: 24px;}")
            self.discription_label.setStyleSheet("QLabel{font-size: 14px;color: #595959;line-height: 24px;text-align:justify;}")
            self.contact_us_label.setStyleSheet("QLabel{font-size: 14px;color: #595959;line-height: 24px;text-align:justify;}")
            self.contact_us_label.setText(_("Service and support team: ") + '<a href="mailto://sopport@kylinos.cn\"'
                                                                            'style=\"color:#595959;\">'
                                                                            'support@kylinos.cn</a>')
            self.close_btn.setStyleSheet("QPushButton{border-radius: 4px;border:0px;border-image:url('res/close1.png')}")
            self.disclaimers_label.setStyleSheet(
                "QLabel{font-size: 14px;color: #595959;line-height: 24px;text-align:justify;}")
            self.disclaimers_label.setText(_("Disclaimers:") + '<a href="Disclaimer\"'
                                                                'style=\"color:#595959;\">'
                                                                '' + "《" + _("Kylin Software Store Disclaimer") + "》" + ' </a>')
        else:
            if (Globals.RIGHT_ANGLE):
                self.widget.setStyleSheet("QWidget{background-color:#3D3D41;}")
                self.setStyleSheet("QFrame{background-color:#3D3D41;}")
            else:
                self.setStyleSheet("QFrame{border-radius:12px;background-color:#3D3D41;}")
                self.widget.setStyleSheet("QWidget{border-radius:12px;background-color:#3D3D41;}")
            self.titel_label.setStyleSheet("QLabel{font-size: 14px;color: #8F9399;}")
            self.app_name_label.setStyleSheet("QLabel{font-size: 18px;color: #C0C4CC;line-height: 28px;}")
            self.app_version_label.setStyleSheet("QLabel{font-size: 14px;color: #C0C4CC;line-height: 24px;}")
            self.discription_label.setStyleSheet("QLabel{font-size: 14px;color: #C0C4CC;line-height: 24px;text-align:justify;}")
            self.contact_us_label.setStyleSheet("QLabel{font-size: 14px;color: #C0C4CC;line-height: 24px;text-align:justify;}")
            self.contact_us_label.setText(_("Service and support team: ") + '<a href="mailto://sopport@kylinos.cn\"'
                                                                            'style=\"color:#C0C4CC;\">'
                                                                            'support@kylinos.cn</a>')
            self.disclaimers_label.setStyleSheet(
                "QLabel{font-size: 14px;color: #C0C4CC;line-height: 24px;text-align:justify;}")
            self.disclaimers_label.setText(_("Disclaimers:") + '<a href="Disclaimer\"'
                                                                        'style=\"color:#C0C4CC;\">'
                                                                        ''+"《"+ _("Kylin Software Store Disclaimer")+"》"+' </a>')

            self.close_btn.setStyleSheet("QPushButton{border-radius: 4px;border:0px;border-image:url('res/common-close-white.png')}")
        # QApplication.processEvents()


    #
    #函数：处理点击关闭按钮
    #
    def slot_click_close_btn(self):
        if (Globals.THEME_COLOR == 0):
            self.close_btn.setStyleSheet(
                "QPushButton{border-radius: 4px;border:0px;border-image:url('res/close1.png')}")
        else:
            self.close_btn.setStyleSheet(
                "QPushButton{border-radius: 4px;border:0px;border-image:url('res/res/common-close-white.png.png')}")
        self.hide()
        QApplication.setActiveWindow(self.mw)

    def eventFilter(self, widget, event):
        if(hasattr(self,"close_btn")):
            if (widget == self.close_btn and event.type() == event.MouseButtonRelease):
                self.slot_click_close_btn()
                # return True
            elif (widget == self.close_btn and event.type() == event.HoverEnter):
                if (Globals.THEME_COLOR == 0):
                    self.close_btn.setStyleSheet("QPushButton:hover{border-radius: 4px;border:0px;border-image:url('res/close2.png')}")
                else:
                    self.close_btn.setStyleSheet("QPushButton{border-radius: 4px;border:0px;border-image:url('res/hover-close.png')}")
            elif (widget == self.close_btn and event.type() == event.HoverLeave):
                if (Globals.THEME_COLOR == 0):
                    self.close_btn.setStyleSheet("QPushButton{border-radius: 4px;border:0px;border-image:url('res/close1.png')}")
                else:
                    self.close_btn.setStyleSheet("QPushButton{border-radius: 4px;border:0px;border-image:url('res/common-close-white.png')}")
            elif (widget == self.close_btn and event.type() == event.MouseButtonPress):
                if (Globals.THEME_COLOR == 0):
                    self.close_btn.setStyleSheet("QPushButton:hover{border-radius: 4px;border:0px;border-image:url('res/close3.png')}")
                else:
                    self.close_btn.setStyleSheet("QPushButton{border-radius: 4px;border:0px;border-image:url('res/clicked-close.png')}")
        return False


    # 函数：重绘函数
    # is_clicked_second_menu:如果是点击第二级目录则不会影藏一级目录
    #
    def set_paintEvent(self, event):
        painter = QPainter(self)
        m_defaultBackgroundColor = QColor(qRgb(192, 192, 192))
        m_defaultBackgroundColor.setAlpha(50)
        path = QPainterPath()
        path.setFillRule(Qt.WindingFill)
        path.addRoundedRect(10, 10, self.width() - 20, self.height() - 20, 12, 12)

        painter.setRenderHint(QPainter.Antialiasing, True)
        painter.fillPath(path, QBrush(QColor(m_defaultBackgroundColor.red(),
                                             m_defaultBackgroundColor.green(),
                                             m_defaultBackgroundColor.blue())))

        color = QColor(0, 0, 0, 16)
        i = 0
        while i < 4:
            path = QPainterPath()
            path.setFillRule(Qt.WindingFill)
            path.addRoundedRect(10 - i, 10 - i, self.width() - (10 - i) * 2, self.height() - (10 - i) * 2, 12, 12)
            color.setAlpha(int(100 - math.sqrt(i) * 50))
            painter.setPen(color)
            painter.drawPath(path)
            i = i + 1

        painter.setRenderHint(QPainter.Antialiasing)

    #
    # 函数：实现无标题栏窗口拖动
    #
    def mousePressEvent(self, event):
        if (event.button() == Qt.LeftButton):
            self.mouse_press = True
            self.drag_pos = event.globalPos() - self.pos()
            event.accept()

    def mouseReleaseEvent(self, event):
        self.mouse_press = False
        self.setWindowOpacity(1)

    # def mouseMoveEvent(self, event):
    #     if (self.mouse_press):
    #         self.move_pos = event.globalPos()
    #         self.move(self.move_pos - self.drag_pos)
    #         self.setWindowOpacity(1)
    #         event.accept()
