#!/usr/bin/python3
# -*- coding: utf-8 -*-


from PyQt5.QtGui import *
from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
import math
from models.globals import Globals
import gettext
gettext.bindtextdomain("kylin-software-center", "/usr/share/locale")
gettext.textdomain("kylin-software-center")
_ = gettext.gettext

class Login_serch(QFrame):
    def __init__(self,w,parent = None):
        QFrame.__init__(self,parent)
        self.w = w
        self.init_ui()
        self.paintEvent = self.set_paintevent

    def init_ui(self):
        self.resize(self.w+20,0)
        self.setWindowFlags(Qt.FramelessWindowHint)
        self.setAttribute(Qt.WA_TranslucentBackground)
        # self.setFrameShape(QFrame.NoFrame)
        # self.setStyleSheet("QFrame{background:red;}")
        self.widget = QWidget(self)
        # self.widget.setStyleSheet("QWidget{background: #FFFFFF;border-radius: 4px;}")
        self.widget.resize(self.w,0)
        self.widget.move(10,10)
        #self.setAttribute(Qt.WA_TranslucentBackground)

        self.item_list_widget = QListWidget(self.widget)
        self.item_list_widget.resize(self.w-8,0)
        self.item_list_widget.move(4,2)
        # self.item_list_widget.setStyleSheet("QListWidget{background-color:#ffffff;border:0px solid #cfcfcf;border-radius: 4px;}QListWidget::item{height: 30px;border-radius: 2px;background:#FFFFFF;}QListWidget::item:hover{background:#F7F7F7;}QListWidget::item:selected{background:#F7F7F7;}")
        self.slot_show_dark_mode()

    def slot_show_dark_mode(self):
        if (Globals.THEME_COLOR == 0):
            # self.setStyleSheet("QFrame{background: transparent;}")
            if(Globals.RIGHT_ANGLE):
                self.widget.setStyleSheet(".QWidget{background: #FFFFFF;}")
            else:
                self.widget.setStyleSheet(".QWidget{background: #FFFFFF;border-radius: 4px;}")
            self.item_list_widget.setStyleSheet(
                "QListWidget{background-color:#ffffff;border:0px solid #cfcfcf;border-radius: 4px;}QListWidget::item{height: 30px;border-radius: 2px;background:#FFFFFF;}QListWidget::item:hover{background:#296CD9;}")
        else:
            # self.setStyleSheet("QFrame{background:red;}")
            if(Globals.RIGHT_ANGLE):
                self.widget.setStyleSheet(".QWidget{background-color: #3D3D41;border-radius: 4px;}")
            else:
                self.widget.setStyleSheet(".QWidget{background-color: #3D3D41;border-radius: 4px;}")
            self.item_list_widget.setStyleSheet("QListWidget{background-color:#3D3D41;border:0px solid #cfcfcf;border-radius: 4px;}QListWidget::item{height: 30px;border-radius: 2px;background:#3D3D41;}QListWidget::item:hover{background:#296CD9;}")
        for i in range(self.item_list_widget.count()):
            item = self.item_list_widget.item(i)
            get_widget = self.item_list_widget.itemWidget(item)
            get_widget.slot_show_dark_mode()
        # QApplication.processEvents()

    def set_paintevent(self,event):
        painter = QPainter(self)
        m_defaultBackgroundColor = QColor(qRgb(255, 255, 255))
        m_defaultBackgroundColor.setAlpha(50)
        path = QPainterPath()
        path.setFillRule(Qt.WindingFill)
        path.addRoundedRect(10, 10, self.width() - 20, self.height() - 20, 4, 4)

        painter.setRenderHint(QPainter.Antialiasing, True)
        painter.fillPath(path, QBrush(QColor(m_defaultBackgroundColor.red(),
                                             m_defaultBackgroundColor.green(),
                                             m_defaultBackgroundColor.blue())))

        color = QColor(0, 0, 0, 20)
        i = 0
        while i < 4:
            path = QPainterPath()
            path.setFillRule(Qt.WindingFill)
            path.addRoundedRect(10 - i, 10 - i, self.width() - (10 - i) * 2, self.height() - (10 - i) * 2, 4, 4)
            color.setAlpha(int(100 - math.sqrt(i) * 50))
            painter.setPen(color)
            painter.drawPath(path)
            i = i + 1

        painter.setRenderHint(QPainter.Antialiasing)


class line_widget(QWidget):
    def __init__(self,w,parent = None):
        QWidget.__init__(self,parent)
        self.w = w
        self.init_ui()

    def init_ui(self):
        self.resize(self.w,30)
        self.setWindowFlags(Qt.FramelessWindowHint)
        self.setStyleSheet("QWidget{background-color:transparent;border-radius:2px;}")

        self.username_label = QLabel(self)

        # self.username_label.setAttribute(Qt.WA_TranslucentBackground)
        self.username_label.setFixedHeight(30)
        self.username_label.move(20, 0)
        self.username_label.setAttribute(Qt.WA_TranslucentBackground)
        self.username_label.setAlignment(Qt.AlignVCenter)
        self.username_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #303133;}")
        # self.username_label.move(9, int((self.height() - self.username_label.height())/ 2))

        self.pwd_label = QLabel(self)
        # self.pwd_label.setAttribute(Qt.WA_TranslucentBackground)
        self.pwd_label.setStyleSheet("QLabel{background-color:transparent;}")
        self.pwd_label.hide()

        self.state_label = QLabel(self)
        # self.state_label.setAttribute(Qt.WA_TranslucentBackground)
        self.state_label.setStyleSheet("QLabel{background-color:transparent;}")
        self.state_label.hide()
        self.slot_show_dark_mode()

    def slot_show_dark_mode(self):
        if (Globals.THEME_COLOR == 0):
            self.username_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #303133;}")
        else:
            self.username_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #F9F9F9;}")
        # QApplication.processEvents()

    def enterEvent(self, event):
        self.username_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #ffffff;}")

    def leaveEvent(self,event):
        if (Globals.THEME_COLOR == 0):
            self.username_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #303133;}")
        else:
            self.username_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #F9F9F9;}")
