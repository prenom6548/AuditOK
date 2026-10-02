#!/usr/bin/python3
# -*- coding: utf-8 -*-

import os
from PyQt5.QtGui import *
from PyQt5.QtCore import *
from PyQt5.QtWidgets import QWidget, QHBoxLayout, QLabel, QFrame, QApplication
from models.enums import setLongTextToElideFormat,BUTTON_PRESSED,BUTTON_LEAVE,BUTTON_ENTER
from ui.systembutton import *
from models.enums import UBUNTUKYLIN_RES_WIN_PATH,UBUNTUKYLIN_HTTP_WIN_RES_PATH,ITEM_LABEL_STYLE,AppMsg
from models.application import *
from utils import commontools

class Win_Replace_Card(QFrame):
    clicked_card = pyqtSignal(Application)
    def __init__(self,detail,win_msg,app_msg,parent = None):
        QFrame.__init__(self,parent)
        self.win_msg = win_msg
        self.app = app_msg
        self.detail_widget = detail
        self.m_parent = parent
        self.init_ui()
        self.init_style()
        self.clicked_card.connect(self.detail_widget.slot_clicked_card)
        self.show()

    def init_ui(self):
        self.resize(413,114)
        self.setStyleSheet("QFrame{background-color: #e6e6e6;border:0px solid #ff0000;border-radius:10px;}")

        self.icon1_label = QLabel(self)
        self.icon1_label.resize(40,40)
        self.icon1_label.move(20,29)
        self.appname1_label = QLabel(self)
        self.appname1_label.resize(84,20)
        self.appname1_label.setStyleSheet("QLabel{border:0px;font-size: 14px;font-weight: 400;color: #303133;line-height: 20px;}")
        self.appname1_label.move(74,25)
        self.category1_label = QLabel(self)
        self.category1_label.resize(24,17)
        self.category1_label.setStyleSheet("QLabel{border:0px;font-size: 12px;font-weight: 400;color: #8F9399;line-height: 17px;}")
        self.category1_label.move(74,59)
        self.category1_label.show()

        self.arrowhead_label = QLabel(self)
        self.arrowhead_label.resize(50,31)
        self.arrowhead_label.setStyleSheet("QLabel{border:0px;background-image:url(./res/arrowhead.png);background-color:transparent;}")
        self.arrowhead_label.move((self.width()-self.arrowhead_label.width())/2 ,(self.height()-self.arrowhead_label.height())/2)
        self.arrowhead_label.show()

        self.icon2_label = QLabel(self)
        self.icon2_label.resize(40, 40)
        self.icon2_label.move(247, 29)
        self.appname2_label = QLabel(self)
        self.appname2_label.resize(84, 20)
        self.appname2_label.setStyleSheet("QLabel{border:0px;font-size: 14px;font-weight: 400;color: #303133;line-height: 20px;}")
        self.appname2_label.move(301, 25)
        self.category2_label = QLabel(self)
        self.category2_label.resize(24, 17)
        self.category2_label.setStyleSheet("QLabel{border:0px;font-size: 12px;font-weight: 400;color: #8F9399;line-height: 17px;}")
        self.category2_label.move(301, 59)
        self.category2_label.show()

    #
    #函数：初始化界面
    #
    def init_style(self):
        if (os.path.isfile(UBUNTUKYLIN_HTTP_WIN_RES_PATH + str(self.win_msg[4]) + ".png")):
            self.icon1_label.setStyleSheet(
                ITEM_LABEL_STYLE % (UBUNTUKYLIN_HTTP_WIN_RES_PATH + str(self.win_msg[4]) + ".png"))
        elif (os.path.isfile(UBUNTUKYLIN_HTTP_WIN_RES_PATH + str(self.win_msg[4]) + ".jpg")):
            self.icon1_label.setStyleSheet(
                ITEM_LABEL_STYLE % (UBUNTUKYLIN_HTTP_WIN_RES_PATH + str(self.win_msg[4]) + ".jpg"))
        elif (os.path.isfile(UBUNTUKYLIN_RES_WIN_PATH + str(self.win_msg[4]) + ".png")):
            self.icon1_label.setStyleSheet(
                ITEM_LABEL_STYLE % (UBUNTUKYLIN_RES_WIN_PATH + str(self.win_msg[4]) + ".png"))
        elif (os.path.isfile(UBUNTUKYLIN_RES_WIN_PATH + str(self.win_msg[4]) + ".jpg")):
            self.icon1_label.setStyleSheet(
                ITEM_LABEL_STYLE % (UBUNTUKYLIN_RES_WIN_PATH + str(self.win_msg[4]) + ".jpg"))
        else:
            self.icon1_label.setStyleSheet(ITEM_LABEL_STYLE % (UBUNTUKYLIN_RES_WIN_PATH + "default.png"))
        self.icon1_label.show()
        if(self.win_msg[4]!=""):
            self.appname1_label.setText(self.win_msg[4])
        else:
            self.appname1_label.setText(self.win_msg[3])
        self.appname1_label.adjustSize()
        self.appname1_label.show()
        if self.app.category_name_cn != '' and self.app.category_name_cn is not None and self.app.category_name_cn != 'None':
            self.category1_label.setText(self.app.category_name_cn)
            self.category2_label.setText(self.app.category_name_cn)
        else:
            self.category2_label.setText(self.app.category_name)
            self.category1_label.setText(self.app.category_name)
        self.category1_label.adjustSize()
        self.category2_label.adjustSize()

        iconpath = commontools.get_icon_path(self.app.pkgname)
        self.icon2_label.setStyleSheet("QLabel{background-image:url('" + iconpath + "');background-color:transparent;}")
        self.icon2_label.show()
        if self.app.displayname_cn != '' and self.app.displayname_cn is not None and self.app.displayname_cn != 'None':
            self.appname2_label.setText(self.app.displayname_cn)
            self.appname2_label.adjustSize()
        else:
            self.appname2_label.setText(self.app.pkgname)
            self.appname2_label.adjustSize()

    # 鼠标进入事件
    def enterEvent(self, event):
        self.status = BUTTON_ENTER
        self.setGeometry(self.x(),self.y(), self.width() + 5, self.height() + 5)
        self.shadow = QGraphicsDropShadowEffect(self)
        self.shadow.setOffset(0, 0)
        self.shadow.setColor(Qt.black)
        self.shadow.setBlurRadius(10)
        self.setGraphicsEffect(self.shadow)
    #
    # 函数：鼠标离开事件
    #

    def leaveEvent(self, event):
        self.status = BUTTON_LEAVE
        self.resize(413,114)
        self.shadow = QGraphicsDropShadowEffect(self)
        self.shadow.setBlurRadius(0)
        self.shadow.setOffset(0, 0)
        self.setGraphicsEffect(self.shadow)

    #
    # 函数：鼠标按下事件
    #
    def mousePressEvent(self, event):
        self.clicked_card.emit(self.app)
        if event.button() == Qt.LeftButton:
            self.status = BUTTON_PRESSED
        else:
            event.ignore()
