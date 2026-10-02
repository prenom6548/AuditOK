#!/usr/bin/python3
# -*- coding: utf-8 -*-


from PyQt5.QtGui import *
from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
from models.globals import Globals
import gettext
gettext.bindtextdomain("kylin-software-center", "/usr/share/locale")
gettext.textdomain("kylin-software-center")
_ = gettext.gettext

class Download_Ranking_Widget(QWidget):
    def __init__(self,parent = None):
        QWidget.__init__(self,parent)
        self.init_ui()
        self.init_style()

    def init_ui(self):
        self.resize(237,550)
        self.setWindowFlags(Qt.FramelessWindowHint)
        self.setAttribute(Qt.WA_TranslucentBackground)
        self.local_lan = QLocale.system().name()
        self.titel_label = QLabel(self)
        self.titel_label.setAttribute(Qt.WA_TranslucentBackground)
        if self.local_lan == 'zh_CN':
            self.titel_label.resize(96,26)
        else:
            self.titel_label.resize(170,26)
        self.titel_label.move(0,0)

        self.more_btn = QPushButton(self)
        self.more_btn.setAttribute(Qt.WA_TranslucentBackground)
        self.more_btn.setFocusPolicy(Qt.NoFocus)
        self.more_btn.setStyleSheet(
            "QPushButton{border: 0px;font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;}")
        self.more_btn.setText(_("more"))
        self.more_btn.adjustSize()
        self.more_btn.move(self.width() - self.more_btn.width(),
                           self.titel_label.y() + int((self.titel_label.height() - self.more_btn.height()) / 2))
        self.more_btn.show()

        self.download_ranking_listWidget = QListWidget(self)
        self.download_ranking_listWidget.setFocusPolicy(Qt.NoFocus)
        self.download_ranking_listWidget.resize(237,508)
        self.download_ranking_listWidget.move(0,30)

    def init_style(self):
        if(Globals.THEME_COLOR == 0):
            self.titel_label.setStyleSheet("QLabel{font-size: 24px;font-weight: 500;color: #303133;line-height: 33px;}")
        else:
            self.titel_label.setStyleSheet( "QLabel{font-size: 24px;font-weight: 500;color: #F9F9F9;line-height: 33px;}")
        self.titel_label.setText(_("Down Rank"))
        # self.titel_label.adjustSize()
        self.download_ranking_listWidget.setStyleSheet("QListWidget{background-color:rgba(0,0,0,0);border:0px solid #cfcfcf;}QListWidget::item{background-color:rgba(0,0,0,0);height:65px;margin-top:0px;border:0px solid #cfcfcf;}QListWidget::item:hover{background-color:background-color:rgba(0,0,0,0);}")
        self.download_ranking_listWidget.verticalScrollBar().setStyleSheet("QScrollBar:vertical{margin:0px 0px 0px 0px;background-color:rgb(255,255,255);border:0px;width:6px;}\
                                                                                 QScrollBar::sub-line:vertical{subcontrol-origin:margin;border:0px solid red;height:13px}\
                                                                                 QScrollBar::up-arrow:vertical{subcontrol-origin:margin;background-color:rgba(0,0,0,0);height:13px}\
                                                                                 QScrollBar::sub-page:vertical{background-color:#EEEDF0;}\
                                                                                 QScrollBar::handle:vertical{background-color:rgba(0,0,0,20%);width:6px;border-radius:6px;} QScrollBar::handle:vertical:hover{background-color:rgb(0,0,0,10%);opacity:0.1;width:6px;border-radius:6px;}  QScrollBar::handle:vertical:pressed{background-color:rgb(0,0,0,50%);width:6px;border-radius:6px;}\
                                                                                 QScrollBar::add-page:vertical{background-color:#EEEDF0;}\
                                                                                 QScrollBar::down-arrow:vertical{background-color:rgba(0,0,0,0);}\
                                                                                 QScrollBar::add-line:vertical{subcontrol-origin:margin;border:0px solid green;height:13px}")

    def get_applist(self,applist):
        self.applist = []
        self.applist.extend(applist)
