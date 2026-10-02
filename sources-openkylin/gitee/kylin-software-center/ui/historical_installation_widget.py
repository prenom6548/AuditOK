#!/usr/bin/python3
# -*- coding: utf-8 -*-


from PyQt5.QtGui import *
from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
from models.globals import *

import gettext
gettext.bindtextdomain("kylin-software-center", "/usr/share/locale")
gettext.textdomain("kylin-software-center")
_ = gettext.gettext

class Historical_Insatallation_widget(QWidget):
    click_history_widget_btn = pyqtSignal(str)
    def __init__(self, w, h, parent=None):
        QWidget.__init__(self, parent)
        self.w = w
        self.h = h
        self.resize(self.w,self.h)
        self.m_parent = parent
        self.now_btn_name = "local"

        self.label_local = QPushButton(self)
        self.label_local.setFocusPolicy(Qt.NoFocus)
        self.label_local.setStyleSheet("QPushButton{font-size: 14px;font-weight: 400;color: #8F9399;line-height: 20px;text-align: center;background-color:transparent;}QPushButton:hover{color: #3790FA;}QPushButton:pressed{color: #3790FA;}")
        #self.label_local.setText(_("本机安装历史"))
        self.label_local.setText(_("Installation history of the machine"))
        self.label_local.adjustSize()
        self.label_local.move(20,0)
        self.label_local.show()
        self.label_local.clicked.connect(self.slot_click_local_btn)

        self.label_other = QPushButton(self)
        self.label_other.setFocusPolicy(Qt.NoFocus)
        self.label_other.setStyleSheet("QPushButton{font-size: 14px;font-weight: 400;color: #8F9399;line-height: 20px;text-align: center;background-color:transparent;}QPushButton:hover{color: #3790FA;}QPushButton:pressed{color: #3790FA;}")
        #self.label_other.setText(_("其他设备安装历史"))
        self.label_other.setText(_("Cloud installation history"))
        self.label_other.adjustSize()
        self.label_other.move(self.label_local.x()+self.label_local.width()+20,self.label_local.y())
        self.label_other.show()
        self.label_other.clicked.connect(self.slot_click_other_btn)
        if(Globals.NEED_SHOW_KYLINID == False):
            self.label_other.hide()

        self.app_card_widget = QWidget(self)
        self.app_card_widget.resize(self.width(), self.height()-self.label_other.height()-20)
        self.app_card_widget.move(int((self.width() - self.app_card_widget.width()) / 2), self.label_other.y()+self.label_other.height())
        self.app_card_widget.setStyleSheet("QWidget{border-radius: 0px;background-color:transparent;}")
        self.app_card_widget.hide()

        self.text_prompt_label = QLabel(self)
        self.text_prompt_label.setAttribute(Qt.WA_TranslucentBackground)
        # self.text_prompt_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #BBBDCA;line-height: 24px;background-color:transparent;}")
        self.text_prompt_label.hide()

        self.no_app_tip_label = QLabel(self)
        self.no_app_tip_label.resize(240, 240)
        if(Globals.THEME_COLOR == 0):
            self.no_app_tip_label.setStyleSheet("QLabel{border-radius: 12px;background-image:url(./res/no_uninstall.png);background-color:transparent;}")
        else:
            self.no_app_tip_label.setStyleSheet("QLabel{border-radius: 12px;background-image:url(./res/no_uninstall_dark.png);background-color:transparent;}")
        self.no_app_tip_label.hide()

        self.set_dark_mode()

    def set_dark_mode(self):
        if(Globals.THEME_COLOR == 0):
            self.label_local.setStyleSheet(
                "QPushButton{font-size: 14px;font-weight: 400;color: #8F9399;line-height: 20px;text-align: center;background-color:transparent;}QPushButton:hover{color: #3790FA;}QPushButton:pressed{color: #3790FA;}")
            self.label_other.setStyleSheet(
                "QPushButton{font-size: 14px;font-weight: 400;color: #8F9399;line-height: 20px;text-align: center;background-color:transparent;}QPushButton:hover{color: #3790FA;}QPushButton:pressed{color: #3790FA;}")
            self.text_prompt_label.setStyleSheet( "QLabel{font-size: 14px;font-weight: 400;color: #BBBDCA;line-height: 24px;background-color:transparent;}")
        else:
            self.label_local.setStyleSheet(
                "QPushButton{font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 20px;text-align: center;background-color:transparent;}QPushButton:hover{color: #3790FA;}QPushButton:pressed{color: #3790FA;}")
            self.label_other.setStyleSheet(
                "QPushButton{font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 20px;text-align: center;background-color:transparent;}QPushButton:hover{color: #3790FA;}QPushButton:pressed{color: #3790FA;}")
            self.text_prompt_label.setStyleSheet(
                "QLabel{font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 24px;background-color:transparent;}")

    def re_move(self):
        self.label_local.move(20, 20)
        self.label_other.move(self.label_local.x() + self.label_local.width() + 20, self.label_local.y())
        self.app_card_widget.resize(self.width(), self.height() - self.label_other.height() - 20)
        self.app_card_widget.move(int((self.width() - self.app_card_widget.width()) / 2),
                                  self.label_other.y() + self.label_other.height())
        self.no_app_tip_label.move(int(self.app_card_widget.x() + int((self.app_card_widget.width() - self.no_app_tip_label.width()) / 2)),int(((self.height() - self.no_app_tip_label.height()) / 2)-((self.m_parent.btn_widget.y()+self.m_parent.btn_widget.height())/2)))
        self.text_prompt_label.move(int((self.width() - self.text_prompt_label.width()) / 2),self.no_app_tip_label.y() + self.no_app_tip_label.height() + 10)

    def reload_size(self):
        self.setFixedHeight(self.h)
        self.no_app_tip_label.move(self.app_card_widget.x() + int((self.app_card_widget.width() - self.no_app_tip_label.width()) / 2),int((self.height() - self.no_app_tip_label.height()) / 2)-((self.m_parent.btn_widget.y()+self.m_parent.btn_widget.height())/2))

    def slot_click_local_btn(self):
        self.now_btn_name = "local"
        self.label_local.setStyleSheet("QPushButton{font-size: 14px;font-weight: 400;line-height: 20px;text-align: center;background-color:transparent;color: #3790FA;}")
        self.label_other.setStyleSheet("QPushButton{font-size: 14px;font-weight: 400;color: #8F9399;line-height: 20px;text-align: center;background-color:transparent;}QPushButton:hover{color: #3790FA;}QPushButton:pressed{color: #3790FA;}")
        self.click_history_widget_btn.emit(_("Installation"))

    def slot_click_other_btn(self):
        self.now_btn_name = "other"
        self.label_other.setStyleSheet(
            "QPushButton{font-size: 14px;font-weight: 400;line-height: 20px;text-align: center;background-color:transparent;color: #3790FA;}")
        self.label_local.setStyleSheet(
            "QPushButton{font-size: 14px;font-weight: 400;color: #8F9399;line-height: 20px;text-align: center;background-color:transparent;}QPushButton:hover{color: #3790FA;}QPushButton:pressed{color: #3790FA;}")
        self.click_history_widget_btn.emit(_("Installation"))
