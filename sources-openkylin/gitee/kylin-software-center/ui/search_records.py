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

class Search_records(QFrame):
    clicked_item = pyqtSignal(QListWidgetItem)
    space = 8
    def __init__(self,w,parent = None):
        QFrame.__init__(self,parent)
        self.w = w
        self.init_ui()
        self.mw = parent
        self.enter_item = None
        self.clicked_item.connect(self.mw.slot_click_search_item)
        self.paintEvent = self.set_paintevent

    def init_ui(self):
        self.resize(self.w+20,0)
        self.setWindowFlags(Qt.FramelessWindowHint)
        self.setAttribute(Qt.WA_TranslucentBackground)
        # self.setStyleSheet("QFrame{width: 232px;height: 106px;background: #FFFFFF;border-radius: 4px;}")
        self.widget = QWidget(self)
        # self.widget.setStyleSheet("QWidget{background: #FFFFFF;border-radius: 4px;}")
        self.widget.resize(self.w,0)
        self.widget.move(10,10)
        #self.setAttribute(Qt.WA_TranslucentBackground)

        self.item_list_widget = QListWidget(self.widget)
        self.item_list_widget.resize(self.w-4,0)
        self.item_list_widget.move(2,2)
        # self.item_list_widget.setStyleSheet("QListWidget{background-color:#ffffff;border:0px solid #cfcfcf;border-radius: 4px;}QListWidget::item{height: 30px;border-radius: 2px;background:#FFFFFF;}QListWidget::item:hover{background:#296CD9;}")
        self.slot_show_dark_mode()

    def slot_show_dark_mode(self):
        if(Globals.THEME_COLOR == 0):
            if(Globals.RIGHT_ANGLE):
                self.setStyleSheet("QFrame{background: transparent;}")
                self.widget.setStyleSheet("QWidget{background: #FFFFFF;}")
            else:
                self.setStyleSheet("QFrame{background: transparent;border-radius: 8px;}")
                self.widget.setStyleSheet("QWidget{background: #FFFFFF;border-radius: 8px;}")
            self.item_list_widget.setStyleSheet(
                "QListWidget{background-color:#ffffff;border:0px solid #cfcfcf;border-radius: 8px;}QListWidget::item{height: 36px;background:#FFFFFF;border-radius: 6px;}QListWidget::item:hover{background:#3790FA;border-radius: 6px;}")
        else:
            if (Globals.RIGHT_ANGLE):
                self.setStyleSheet("QFrame{background: #3D3D41;}")
                self.widget.setStyleSheet("QWidget{background: #3D3D41;}")
            else:
                self.setStyleSheet("QFrame{background: #3D3D41;border-radius: 8px;}")
                self.widget.setStyleSheet("QWidget{background: #3D3D41;border-radius: 8px;}")
            self.item_list_widget.setStyleSheet("QListWidget{background-color:#3D3D41;border:0px solid #cfcfcf;border-radius: 8px;}QListWidget::item{height: 36px;background:#3D3D41;border-radius: 6px;}QListWidget::item:hover{background:#296CD9;border-radius: 6px;}")
        for i in range(self.item_list_widget.count()):
            item = self.item_list_widget.item(i)
            get_widget = self.item_list_widget.itemWidget(item)
            get_widget.slot_show_dark_mode()

    def slot_change_list_style(self,enter_widget):
        for i in range(self.item_list_widget.count()):
            item = self.item_list_widget.item(i)
            get_widget = self.item_list_widget.itemWidget(item)
            if(get_widget != None):
                if(get_widget == enter_widget):
                    get_widget.appname_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #ffffff;border-radius:6px;}")
                    get_widget.setStyleSheet("QFrame{border-radius:6px;background:#296CD9;}")
                else:
                    if (Globals.THEME_COLOR == 0):
                        get_widget.setStyleSheet("QFrame{border-radius:6px;background:#FFFFFF;}")
                        get_widget.appname_label.setStyleSheet(
                            "QLabel{font-size: 14px;font-weight: 400;color: #303133;border-radius:6px;}")
                    else:
                        get_widget.setStyleSheet("QFrame{border-radius:6px;background:#3D3D41;}")
                        get_widget.appname_label.setStyleSheet(
                            "QLabel{font-size: 14px;font-weight: 400;color: #F9F9F9;border-radius:6px;}")

    def key_up(self):
        if (self.enter_item == None):
            self.enter_item = self.item_list_widget.count() - 1
            item = self.item_list_widget.item(self.enter_item)
            get_item_widget = self.item_list_widget.itemWidget(item)
            get_item_widget.setStyleSheet("QFrame{border-radius:6px;background:#3790FA;}")
            get_item_widget.appname_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #ffffff;}")
        else:
            if (self.enter_item - 1 >= 0):
                pre_item_num = self.enter_item
                self.enter_item = self.enter_item - 1
            else:
                pre_item_num = self.enter_item
                self.enter_item = self.item_list_widget.count() - 1
            pre_item = self.item_list_widget.item(pre_item_num)
            get_pre_item_widget = self.item_list_widget.itemWidget(pre_item)
            if (Globals.THEME_COLOR == 0):
                get_pre_item_widget.setStyleSheet("QFrame{border-radius:2px;background:#FFFFFF;}")
                get_pre_item_widget.appname_label.setStyleSheet(
                    "QLabel{font-size: 14px;font-weight: 400;color: #303133;}")
            else:
                get_pre_item_widget.setStyleSheet("QFrame{border-radius:6px;background:#3D3D41;}")
                get_pre_item_widget.appname_label.setStyleSheet(
                    "QLabel{font-size: 14px;font-weight: 400;color: #F9F9F9;}")
            item = self.item_list_widget.item(self.enter_item)
            get_item_widget = self.item_list_widget.itemWidget(item)
            get_item_widget.appname_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #ffffff;}")
            get_item_widget.setStyleSheet("QFrame{border-radius:6px;background:#3790FA;}")


    def key_down(self):
        if (self.enter_item == None):
            self.enter_item = 0
            item = self.item_list_widget.item(self.enter_item)
            get_item_widget = self.item_list_widget.itemWidget(item)
            get_item_widget.setStyleSheet("QFrame{border-radius:6px;background-color:#3790FA;}")
            get_item_widget.appname_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #ffffff;}")

        else:
            if (self.enter_item + 1 <= self.item_list_widget.count() - 1):
                pre_item_num = self.enter_item
                self.enter_item = self.enter_item + 1
            else:
                pre_item_num = self.enter_item
                self.enter_item = 0
            pre_item = self.item_list_widget.item(pre_item_num)
            get_pre_item_widget = self.item_list_widget.itemWidget(pre_item)
            if (Globals.THEME_COLOR == 0):
                get_pre_item_widget.setStyleSheet("QFrame{border-radius:6px;background:#FFFFFF;}")
                get_pre_item_widget.appname_label.setStyleSheet(
                    "QLabel{font-size: 14px;font-weight: 400;color: #303133;}")
            else:
                get_pre_item_widget.setStyleSheet("QFrame{border-radius:6px;background-color:#3D3D41;}")
                get_pre_item_widget.appname_label.setStyleSheet(
                    "QLabel{font-size: 14px;font-weight: 400;color: #F9F9F9;}")
            item = self.item_list_widget.item(self.enter_item)
            get_item_widget = self.item_list_widget.itemWidget(item)
            get_item_widget.appname_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #ffffff;}")
            get_item_widget.setStyleSheet("QFrame{border-radius:6px;background:#3790FA;}")

    def keyPressEvent(self, keyevent):
        key = keyevent.key()
        if(key == Qt.Key_Up):
            self.key_up()
        elif(key == Qt.Key_Down):
            self.key_down()
        elif(key == Qt.Key_Enter or key == Qt.Key_Return):
            if(self.enter_item != None):
                self.clicked_item.emit(self.item_list_widget.item(self.enter_item))
        elif(key >= Qt.Key_A and key <= Qt.Key_Z):
            self.mw.title_bar.search_line.setFocus()
            self.mw.title_bar.search_line.setText(self.mw.title_bar.search_line.text()+keyevent.text())
        elif(key == Qt.Key_Backspace):
            self.mw.title_bar.search_line.setFocus()
            self.mw.title_bar.search_line.backspace()


    def set_paintevent(self,event):
        painter = QPainter(self)
        m_defaultBackgroundColor = QColor(qRgb(255, 255, 255))
        m_defaultBackgroundColor.setAlpha(50)
        path = QPainterPath()
        path.setFillRule(Qt.WindingFill)
        path.addRoundedRect(10, 10, self.width() - 20, self.height() - 20, 8, 8)

        painter.setRenderHint(QPainter.Antialiasing, True)
        painter.fillPath(path, QBrush(QColor(m_defaultBackgroundColor.red(),
                                             m_defaultBackgroundColor.green(),
                                             m_defaultBackgroundColor.blue())))

        color = QColor(0, 0, 0, 20)
        i = 0
        while i < 4:
            path = QPainterPath()
            path.setFillRule(Qt.WindingFill)
            path.addRoundedRect(10 - i, 10 - i, self.width() - (10 - i) * 2, self.height() - (10 - i) * 2, 8, 8)
            color.setAlpha(int(100 - math.sqrt(i) * 50))
            painter.setPen(color)
            painter.drawPath(path)
            i = i + 1

        painter.setRenderHint(QPainter.Antialiasing)


class line_widget(QFrame):
    enter_widget = pyqtSignal(QFrame)
    def __init__(self,w,parent = None):
        QFrame.__init__(self,parent)
        self.w = w
        self.init_ui()

    def init_ui(self):
        self.resize(self.w,36)
        self.setWindowFlags(Qt.FramelessWindowHint)
        self.setStyleSheet("QFrame{background-color:transparent;border-radius:6px;}")

        self.appname_label = QLabel(self)
        self.appname_label.move(20,0)
        self.appname_label.setFixedSize(self.width()-20,self.height())
        self.appname_label.setAttribute(Qt.WA_TranslucentBackground)
        self.appname_label.setAlignment(Qt.AlignVCenter)
        self.appname_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #303133;border-radius:6px;}")
        self.slot_show_dark_mode()

    def setLongTextToElideFormat(self,label, text):
        if text[len(text) - 1] == '\n':
            text = text.rstrip()
        metrics = QFontMetrics(label.font())
        elidedText = metrics.elidedText(text, Qt.ElideRight, label.maximumWidth())
        if "…" in elidedText:
            label.setToolTip(text)
        self.appname = text
        label.setText(elidedText)

    def slot_show_dark_mode(self):
        if (Globals.THEME_COLOR == 0):
            self.appname_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #303133;border-radius:6px;}")
            self.setStyleSheet("QFrame{background-color:#FFFFFF;border-radius:6px;}")
        else:
            self.appname_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #F9F9F9;border-radius:6px;}")
            self.setStyleSheet("QFrame{background-color:transparent;border-radius:6px;}")

    def enterEvent(self, event):
        self.enter_widget.emit(self)
        self.appname_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #ffffff;border-radius:6px;}")
        self.setStyleSheet("QFrame{border-radius:6px;background-color:#3790FA;}")

    def leaveEvent(self,event):
        if (Globals.THEME_COLOR == 0):
            self.appname_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #303133;border-radius:6px;}")
            self.setStyleSheet("QFrame{background-color:#FFFFFF;border-radius:6px;}")
        else:
            self.appname_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #F9F9F9;border-radius:6px;}")
            self.setStyleSheet("QFrame{background-color:transparent;border-radius:6px;}")
