#!/usr/bin/python
# -*- coding: utf-8 -*-

### BEGIN LICENSE

# Copyright (C) 2013 National University of Defense Technology(NUDT) & Kylin Ltd

# Author:
#     Shine Huang<shenghuang@ubuntukylin.com>
# Maintainer:
#     Shine Huang<shenghuang@ubuntukylin.com>

# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 3, as published
# by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranties of
# MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
# PURPOSE.  See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program.  If not, see <http://www.gnu.org/licenses/>.
import base64

from PyQt5.QtWidgets import *
from PyQt5.QtGui import *
from PyQt5.QtCore import *
import math
from .login_ui import Ui_Login_ui
from models.enums import Signals,UKSC_CACHE_DIR
from ui.confirmdialog import ConfirmDialog, TipsDialog, Update_Source_Dialog
from models.globals import Globals
from backend.service.save_password import password_write, password_read
from utils.aescrypt import *
import re
import os
from PyQt5.QtCore import QTimer
from utils.log_manager import LogManager
from ui.login_search import *
# from tkinter import *
import time
from utils.commontools import webbrowser_open_url
import webbrowser
import gettext
gettext.textdomain("ubuntu-kylin-software-center")
_ = gettext.gettext

class Login(QDialog,Signals):
        
    # listadduser = ["","","",""]
    listlogin = ["","",""]
    res = []
    dragPosition = -1
    picture_position = 0
    login_phone_list =[]
    close_rem_state = True
    first_open = 0
    # login_mode = 0
    #strs = r'^[a-zA-Z0-9_-]+(\.[a-zA-Z0-9_-]+){0,4}@[a-zA-Z0-9_-]+(\.[a-zA-Z0-9_-]+){0,4}$'
    strs = r'^[0-9a-zA-Z_]{0,19}@[t]{0,1}[j]{0,1}[.]{0,1}[0-9a-zA-Z]{1,13}\.[com,cn,net]{1,3}$'
    def __init__(self,mw,parent=None):
        super().__init__(parent)
        self.mw = mw
        self.pre_num = 0
        self.shortcut = 0
        self.ui_init()
        self.setWindowFlags(Qt.FramelessWindowHint|Qt.Tool)
        self.setWindowTitle(_("Software Store"))
        self.setAttribute(Qt.WA_TranslucentBackground)
        self.ui.centralwidget.paintEvent = self.set_paintEvent
        # self.setStyleSheet("QWidget{border:1px solid #cccccc;background-color:#ffffff;border-radius:6px;}")
        # self.ui.bg.lower()
        # self.move(280, 60)
        # self.ui.topWidget.raise_()
        self.ui.sourceWidget.raise_()
        self.setWindowTitle(_("Login"))
        # self.ui.tips_user_password.clear()

        self.ui.register_new_account.setFocusPolicy(Qt.NoFocus)
        self.ui.btn_login.setFocusPolicy(Qt.NoFocus)
        self.ui.btnPassword.setFocusPolicy(Qt.NoFocus)
        self.ui.btnShortMessage.setFocusPolicy(Qt.NoFocus)
        self.ui.captcha.setFocusPolicy(Qt.NoFocus)
        self.ui.checkBox_5.setFocusPolicy(Qt.NoFocus)
        self.ui.checkBox_5.setCheckState(Qt.Unchecked)
        self.ui.btnClose.setFocusPolicy(Qt.NoFocus)

        # self.ui.btnClose.clicked.connect(self.hide)
        self.ui.btnClose.clicked.connect(self.slot_click_close)
        self.ui.btn_login.clicked.connect(self.slot_click_login)
        self.ui.register_new_account.clicked.connect(self.slot_click_adduser)
        self.ui.lesource_2.setEchoMode(QLineEdit.Password)
        self.ui.lesource_2.setContextMenuPolicy(Qt.NoContextMenu)
        self.ui.Login_user_box.show()
        self.ui.Login_phone_box.hide()
        self.ui.Login_sm_box.hide()
        self.ui.Login_pwd_box.show()
        # self.ui.Login_user_box.setStyleSheet("QWidget{border:1px solid #3790FA}")
        # self.ui.Login_pwd_box.setStyleSheet("QWidget{border:1px solid #EBEEF5}")
        # self.ui.Login_phone_box.setStyleSheet("QWidget{border:1px solid #3790FA}")
        # self.ui.Login_sm_box.setStyleSheet("QWidget{border:1px solid #EBEEF5;}")

	    # self.ui.show_search.hide()
        self.ui.show_search.hide()
        self.ui.show_search.setFocusPolicy(Qt.NoFocus)



        #用户名qlabel文字修改信号槽
        self.ui.lesource.textChanged.connect(self.slot_le_input)

        self.ui.lesource.installEventFilter(self)
        #电话号码qlabel文字修改信号槽
        self.ui.lesource1.textChanged.connect(self.slot_le_input1)
        self.ui.lesource1.installEventFilter(self)
        # QRegExp
        self.regx = QRegExp("[0-9]+$")
        self.validator = QRegExpValidator(self.regx,self.ui.lesource1)
        self.ui.lesource1.setValidator( self.validator)
        self.ui.lesource1.setMaxLength(11)
        # self.ui.lesource1.textEdited.connect(self.slot_le_input1)
        #密码qlabel
        self.ui.lesource_2.textChanged.connect(self.slot_le_input2)

        self.ui.lesource_2.installEventFilter(self)
        #验证码qlabel
        self.ui.lesource_1.textChanged.connect(self.slot_le_input3)
        self.ui.lesource_1.installEventFilter(self)
        self.validator_1 = QRegExpValidator(self.regx,self.ui.lesource_1)
        self.ui.lesource_1.setValidator( self.validator_1)

        self.ui.btnShortMessage.clicked.connect(self.slot_click_SM)
        self.ui.btnPassword.clicked.connect(self.slot_click_PW)
        self.ui.captcha.clicked.connect(self.slot_click_time)
        # self.ui.checkBox_5.stateChanged.connect(self.slot_change1)
        # self.ui.checkBox_6.stateChanged.connect(self.slot_change2)
        #忘记密码
        self.ui.forget_pwd.clicked.connect(self.find_password_suc)

        self.ui.lesource.setMaxLength(30)
        self.ui.lesource_2.setMaxLength(30)
        self.ui.lesource1.setMaxLength(11)
        self.ui.lesource_1.setMaxLength(30)
        # self.ui.lesource.setFocus(True)
        # self.ui.lesource_2.setFocus(False)
        # self.ui.lesource_1.setFocus(False)

        #self.check_text_state()
        self.count = 59
        self.timer = QTimer(self)  # 初始化一个定时器
        self.timer1 = QTimer(self)
        self.timer1.setInterval(1000)
        self.timer1.timeout.connect(self.slot_time_Refresh)
        self.timer_user_and_pwd = QTimer(self)

        self.timer_picture = QTimer(self)
        self.timer_picture_correct = QTimer(self)
        self.phone_timer_picture_correct = QTimer(self)
        self.timer_pwd = QTimer(self)
        self.timer_user = QTimer(self)
        self.timer_wait_picWidget_hide = QTimer(self)


        # caoliang
        # 登录模式 0——密码登录，1——验证码登录
        self.login_mode = 0
        self.account_login_values = {}
        self.account_login_res = {}
        self.phone_login_values = {}
        self.phone_login_res = {}

        self.image_code_values = {}
        self.image_code_res = {}
        self.phone_code_values = {}
        self.phone_code_res = {}
        self.tips_code_message = []

        self.rem_click = True

        self.mouse_press = False
        self.save_user_pwd = Globals.USER_AND_PWD

        self.save_user_item = []

        # self.user_list = []
        # self.state_list = []
        # self.pwd_list = []

        self.ui.tips_user.setText(_("User not detected"))
        self.ui.tips_password.setText(_("Pwd_Error"))
        self.ui.tips_num.setText(_("Abnormal number"))
        # self.ui.tips_user.setText(_("未检测到该用户"))
        # self.ui.tips_password.setText(_("密码错误"))
        # self.ui.tips_num.setText(_("号码异常"))
        self.ui.tips_user_password.setAlignment(Qt.AlignCenter)#设置字体居中

        self.ui.register_new_account.setText(_("Reg"))
        # self.ui.register_new_account.setText(_("注册新账号"))
        #self.ui.btn_login.setText("登录")
        self.ui.btn_login.setText(_("Login"))
        self.ui.btn_login.setEnabled(False)
        self.ui.btn_login.clicked.connect(self.choose_login_way)
        # if not self.ui.lesource.isHidden():
        #     self.ui.btn_login.clicked.connect(self.slot_login)
        # else:
        #     self.ui.btn_login.clicked.connect(self.next_to_phone_login)


        self.ui.lesource_M_back.setText(_("User name/Phone/Email"))
        # self.ui.lesource.setPlaceholderText(_("用户名/手机/邮箱"))
        # self.ui.lesource1.setPlaceholderText(_("手机号"))
        self.ui.lesource1_S_back.setText(_("cell-phone number"))

       # self.ui.lesource_2.setPlaceholderText("请输入密码")
        self.ui.lesource_2_M_back.setText(_("Please enter the password"))

        # self.ui.captcha.setText(_("获取验证码"))
        self.ui.captcha.setText(_("Get code"))
        # self.ui.lesource_1.setPlaceholderText(_("验证码"))
        self.ui.lesource_1_S_back.setText(_("Captcha"))

        self.ui.lesource_M_back.setEnabled(False)
        self.ui.lesource_2_M_back.setEnabled(False)
        self.ui.lesource1_S_back.setEnabled(False)
        self.ui.lesource_1_S_back.setEnabled(False)
        self.ui.lesource_M_back.installEventFilter(self)
        self.ui.lesource_2_M_back.installEventFilter(self)
        self.ui.lesource1_S_back.installEventFilter(self)
        self.ui.lesource_1_S_back.installEventFilter(self)
        # self.ui.rmb_pwd.setText("记住密码")
        self.ui.rmb_pwd.setText(_("Rmb pwd"))

        self.ui.rmb_pwd.setFocusPolicy(Qt.NoFocus)
        self.ui.rmb_pwd.clicked.connect(self.rem_password_suc)
        self.ui.rmb_pwd.setEnabled(True)
        # self.ui.rmb_pwd.setStyleSheet()
        #self.ui.text9.setText("自动登录")
        # self.ui.text9.setText(_("Auto login"))
        #self.ui.forget_pwd.setText("找回密码")
        self.ui.forget_pwd.setText(_("Forget Pwd"))
        # self.ui.forget_pwd.setText(_("忘记密码？"))
        self.ui.soft_linedit.setText(_("Kylin Id login Center"))
        # self.ui.soft_linedit.adjustSize()
        # self.ui.soft_linedit.setText(_("应用商店登录中心"))
        self.ui.soft_linedit.setAlignment(Qt.AlignCenter)


        self.ui.show_search.clicked.connect(self.slot_show_searchwidget)

        # self.ui.btnPassword.setText(_("密码登录"))
        self.ui.btnPassword.setText(_("Pwd login"))

        # self.ui.btnShortMessage.setText(_("短信登录"))
        self.ui.btnShortMessage.setText(_("SMS login"))
        # self.ui.groupBox.setStyleSheet("QGroupBox{border:0px;}")

        #self.ui.Login_phone_box.setStyleSheet("QWidget{border:1px solid #EBEEF5}")
        #self.ui.Login_sm_box.setStyleSheet("QWidget{border:1px solid #EBEEF5;}")

         # self.ui.btn_login.setStyleSheet("QPushButton{color:white;border:0px;border-radius:4px;backgroound-color:#2d8ae1;}QPushButton:hover{border:0px;}QPushButton:pressed{border:0px;}")
        # self.ui.btn_login.setStyleSheet("QPushButton{background-color:#E9E9E9;border:0px;font-size:14px;border-radius:16px;color:#B3B3B3}QPushButton:hover{background-color:#40A9FB;border:0px;border-radius:16px;font-size:14px;color:#FFFFFF}QPushButton:pressed{background-color:#296CD9;border:0px;border-radius:16px;font-size:14px;color:#FFFFFF}")
        if self.ui.lesource_2.text() != '':

            self.ui.show_hide_pwd.show()
            self.ui.show_pwd.show()
            self.ui.hide_pwd.hide()

        else:

            self.ui.show_hide_pwd.hide()
            self.ui.show_pwd.show()
            self.ui.hide_pwd.hide()



        self.ui.show_pwd.clicked.connect(self.slot_show_password)


        self.ui.hide_pwd.clicked.connect(self.slot_hide_password)
        self.ui.show_hide_pwd.setFocusPolicy(Qt.NoFocus)
        self.ui.show_pwd.setFocusPolicy(Qt.NoFocus)
        self.ui.hide_pwd.setFocusPolicy(Qt.NoFocus)


        self.ui.forget_pwd.setFocusPolicy(Qt.NoFocus)

        self.ui.picture_test_background.hide()
        self.ui.picture_test.hide()

        self.ui.picture_tip.setText(_("Drag the slider to complete the validation"))

        self.ui.picture_tip.setAlignment(Qt.AlignCenter)
        self.ui.picture_tip.hide()
        self.ui.picture_correct.hide()

        self.slide_widget = SlideBar(self.ui.picture_widget)
        self.slide_widget.resize(320,26)
        self.slide_widget.move(0,186)
        self.slide_widget.hide()
        self.ui.picture_widget.hide()

        self.ui.picture_widget_cover.hide()


        self.slide_widget.picture_slider.ui_login_mouse_current_position.connect(self.next_to_login)
        self.ui.picture_test_small.ui_login_mouse_current_position.connect(self.next_to_login)


        self.slide_widget.picture_slider.ui_login_image_code_move.connect(self.slot_ui_login_image_code_move)
        self.ui.picture_test_small.ui_login_image_code_move.connect(self.slot_ui_login_image_code_move)
        self.ui.centralwidget.paintEvent = self.set_paintEvent
        # self.shadow = QGraphicsDropShadowEffect(self)
        #点击下拉按钮的下拉框
        self.senior_search = Login_serch(183, self)
        self.senior_search.move(119, 188)
        self.senior_search.hide()
        #输入用户名后进行匹配显示的下拉框
        self.search_user_widget = Login_serch(183, self)
        self.search_user_widget.move(119, 188)
        self.search_user_widget.hide()
        self.search_user_widget.item_list_widget.itemClicked.connect(self.slot_click_item_for_search)

        # self.decode_user_pwd()

        if(Globals.SET_REM):
            self.ui.lesource.setText(Globals.USER)
            self.ui.lesource_2.setText(Globals.PASSWORD)
            self.ui.checkBox_5.setChecked(True)
            self.rem_click = False
        else:
            self.rem_click = True

        self.show_or_hide_back()
    #
    # 函数：显示密码登录背景文字
    #
    def show_or_hide_back(self):
        if self.ui.lesource_1.isHidden():
            if self.ui.lesource.text() == '':
                self.ui.lesource_M_back.show()

            else:
                self.ui.lesource_M_back.hide()
            if self.ui.lesource_2.text() == '':
                self.ui.lesource_2_M_back.show()
            else:
                self.ui.lesource_2_M_back.hide()

    #
    # 函数：显示短信登录背景文字
    #
    def show_or_hide_back_S(self):
        if self.ui.lesource.isHidden():
            if self.ui.lesource1.text() == '':
                self.ui.lesource1_S_back.show()

            else:
                self.ui.lesource1_S_back.hide()
            if self.ui.lesource_1.text() == '':
                self.ui.lesource_1_S_back.show()
            else:
                self.ui.lesource_1_S_back.hide()
    # #
    # 函数:选择登录方式,账号密码登录或者手机短信登录picture-widget-cover
    #
    def choose_login_way(self):
        if not self.ui.lesource.isHidden():
            self.slot_login()
        else:
            self.next_to_phone_login()

    def slot_show_dark_mode(self):
        if(Globals.THEME_COLOR == 0):
            
            self.ui.centralwidget.setStyleSheet("QFrame{background-color:#ffffff;}")
            if(Globals.RIGHT_ANGLE):
                self.ui.sourceWidget.setStyleSheet(".QWidget#sourceWidget{border:0px solid #ffffff;background-color:#ffffff;}")
            else:
                self.ui.sourceWidget.setStyleSheet(".QWidget#sourceWidget{border:0px solid #ffffff;background-color:#ffffff;border-radius:4px;}")
            self.ui.btnClose.setStyleSheet("QPushButton{border-radius: 4px;border:0px;border-image:url('res/close1.png')}QPushButton:hover{border:0px;border-image:url('res/close2.png')}QPushButton:pressed{border:0px;border-image:url('res/close3.png')}")

            self.ui.usr_icon.setStyleSheet("QWidget{border-image:url('res/usernameblack.png');background-color:transparent;border:0px}")
            self.ui.password_icon.setStyleSheet("QWidget{border-image:url('res/passwordblack.png');background-color:transparent;border:0px}")

            self.ui.register_new_account.setStyleSheet("QPushButton{border:0px;font-size:14px;no-repeat center left;color:#303133}QPushButton:hover{color: #3790FA;}QPushButton:pressed{color: #3790FA;}")
            self.ui.forget_pwd.setStyleSheet("QPushButton{border:0px;font-size:14px;color:#303133;}QPushButton:hover{border:0px;font-size:13px;color:#303133;}QPushButton:pressed{border:0px;font-size:13px;color:#303133;}")

            self.ui.soft_linedit.setStyleSheet("QLabel{font-size:16px;font-weight:500;color:#303133}")

            self.ui.tips_user_password.setStyleSheet("QLabel{background-color:transparent;font-size:12px;border:none;color:#F56C6C;}")
            self.ui.tips_user.setStyleSheet("QLabel{background-color:transparent;font-size:12px;border:none;color:#F56C6C;}")
            self.ui.tips_password.setStyleSheet("QLabel{background-color:transparent;font-size:12px;border:none;color:#F56C6C;}")
            self.ui.tips_num.setStyleSheet("QLabel{background-color:transparent;font-size:12px;border:none;color:#F56C6C;}")

            # self.ui.btnClose.setStyleSheet("QPushButton{border-radius: 4px;border:0px;background-image:url('res/close1.png')}QPushButton:hover{border:0px;background-image:url('res/close2.png')}QPushButton:pressed{border:0px;background-image:url('res/close3.png')}")
            self.ui.show_search.setStyleSheet("QPushButton{border-radius: 4px;border:0px;border-image:url('res/show_search.png')}QPushButton:hover{border:0px;border-image:url('res/show_search_clicked.png')}QPushButton:pressed{border:0px;border-image:url('res/show_search_clicked.png')}")
            if self.ui.lesource.isHidden():
                self.ui.btnPassword.setStyleSheet(
                    "QPushButton{border-radius: 12px;border: 1px solid #C0C4CC ;font-size: 12px;color:#C0C4CC;}"
                    "QPushButton:hover{border-radius: 12px;border: 1px solid #3790FA;font-size: 12px;color:#3790FA;}"
                    "QPushButton:pressed{border-radius: 12px;border: 1px solid #3790FA;font-size: 12px;color:#3790FA;}")
                self.ui.btnShortMessage.setStyleSheet(
                    "QPushButton{border-radius: 12px;border: 1px solid #3790FA ;font-size: 12px;color:#3790FA;}"
                    "QPushButton:hover{border-radius: 12px;border: 1px solid #3790FA;font-size: 12px;color:#3790FA;}"
                    "QPushButton:pressed{border-radius: 12px;border: 1px solid #3790FA;font-size: 12px;color:#3790FA;}")
            else:
                self.ui.btnPassword.setStyleSheet("QPushButton{border-radius: 12px;border: 1px solid #3790FA ;font-size: 12px;color:#3790FA;}"
                                                  "QPushButton:hover{border-radius: 12px;border: 1px solid #3790FA;font-size: 12px;color:#3790FA;}"
                                                  "QPushButton:pressed{border-radius: 12px;border: 1px solid #3790FA;font-size: 12px;color:#3790FA;}")
                self.ui.btnShortMessage.setStyleSheet("QPushButton{border-radius: 12px;border: 1px solid #C0C4CC ;font-size: 12px;color:#C0C4CC;}"
                                                "QPushButton:hover{border-radius: 12px;border: 1px solid #3790FA;font-size: 12px;color:#3790FA;}"
                                                "QPushButton:pressed{border-radius: 12px;border: 1px solid #3790FA;font-size: 12px;color:#3790FA;}")

            self.ui.separator_line.setStyleSheet("QWidget{border:1px solid #DDDFE7;}")
            self.ui.captcha.setStyleSheet("QPushButton{border:0px;font-size: 12px;color:#8F9399;background:transparent;}")

            self.ui.lesource.setStyleSheet("QLineEdit{background-color:transparent;border:0px;border-radius:2px;color:#C0C4CC;font-size:14px;}QLineEdit:pressed{color:#000000;}")
            self.ui.lesource1.setStyleSheet("QLineEdit{background-color:transparent;border:0px;border-radius:2px;color:#C0C4CC;font-size:14px;}QLineEdit:pressed{color:#000000;}")
            self.ui.lesource_2.setStyleSheet("QLineEdit{background-color:transparent;border:0px;border-radius:2px;color:#C0C4CC;font-size:14px;}QLineEdit:pressed{color:#000000;}")
            self.ui.lesource_1.setStyleSheet("QLineEdit{background-color:transparent;border:0px;border-radius:2px;color:#C0C4CC;font-size:14px;}QLineEdit:pressed{color:#000000;}")

            self.ui.lesource_M_back.setStyleSheet(
                "QLabel{background-color:transparent;border:0px;border-radius:2px;color:#C0C4CC;font-size:14px;}")
            self.ui.lesource1_S_back.setStyleSheet(
                "QLabel{background-color:transparent;border:0px;border-radius:2px;color:#C0C4CC;font-size:14px;}")
            self.ui.lesource_2_M_back.setStyleSheet(
                "QLabel{background-color:transparent;border:0px;border-radius:2px;color:#C0C4CC;font-size:14px;}")
            self.ui.lesource_1_S_back.setStyleSheet(
                "QLabel{background-color:transparent;border:0px;border-radius:2px;color:#C0C4CC;font-size:14px;}")

            self.ui.show_hide_pwd.setStyleSheet("QPushButton{border-radius: 4px;border:0px;}QPushButton:hover{border:0px;}QPushButton:pressed{border:0px;}")
            self.ui.show_pwd.setStyleSheet("QPushButton{border-radius: 4px;border:0px;border-image:url('res/close_pwd.png');}QPushButton:hover{border:0px;border-image:url('res/close_pwd_click.png');}QPushButton:pressed{border:0px;border-image:url('res/close_pwd_click.png');}")
            self.ui.hide_pwd.setStyleSheet("QPushButton{border-radius: 4px;border:0px;border-image:url('res/show_pwd.png');}QPushButton:hover{border:0px;border-image:url('res/show_pwd_click.png');}QPushButton:pressed{border:0px;border-image:url('res/show_pwd_click.png');}")
            if self.ui.lesource.text() != '':
                self.ui.btn_login.setStyleSheet(
                    "QPushButton{background-color:#3790FA;border:0px;font-size:14px;border-radius:16px;color:#ffffff}QPushButton:hover{background-color:#40A9FB;border:0px;border-radius:16px;font-size:14px;color:#ffffff}QPushButton:pressed{background-color:#296CD9;border:0px;border-radius:16px;font-size:14px;color:#ffffff}")
            else:
                self.ui.btn_login.setStyleSheet("QPushButton{background-color:#E9E9E9;border:0px;font-size:14px;border-radius:16px;color:#B3B3B3}")
            if self.ui.lesource1.isHidden():
                self.ui.rmb_pwd.setStyleSheet(
                    "QPushButton{border:0px;font-size:14px;no-repeat center left;color:#303133}QPushButton:hover{font-size:13px;color:#303133;}")

                self.ui.checkBox_5.setStyleSheet("QCheckBox{border-radius:2px;background-color:#FFFFFF;}"
                                                 "QCheckBox::indicator{width: 14px; height: 14px;}"
                                                 "QCheckBox::indicator:unchecked{ image: url(./res/unchecked.png);}"
                                                 "QCheckBox::indicator:unchecked:hover{image: url(./res/unchecked.png);}"
                                                 "QCheckBox::indicator:unchecked:pressed{ image: url(./res/unchecked.png);}"
                                                 "QCheckBox::indicator:checked{image: url(./res/check.png);}"
                                                 "QCheckBox::indicator:checked:hover {image: url(./res/check.png);}"
                                                 "QCheckBox::indicator:checked:pressed {image: url(./res/check.png);}"
                                                 "QCheckBox::indicator:indeterminate {image: url(./res/check.png);}"
                                                 "QCheckBox::indicator:indeterminate:hover {image: url(./res/check.png);}"
                                                 "QCheckBox::indicator:indeterminate:pressed{image: url(./res/check.png);}")
            else:
                self.ui.rmb_pwd.setStyleSheet(
                    "QPushButton{border:0px;font-size:14px;no-repeat center left;color:#D5D8DE;}QPushButton:hover{font-size:14px;color:#D5D8DE;}")
                self.ui.checkBox_5.setStyleSheet("QCheckBox{border-radius:2px;background-color:#FFFFFF;}"
                                                 "QCheckBox::indicator{width: 14px; height: 14px;}"
                                                 "QCheckBox::indicator:unchecked{ image: url(./res/unchecked.png);}"
                                                 "QCheckBox::indicator:unchecked:hover{image: url(./res/unchecked.png);}"
                                                 "QCheckBox::indicator:unchecked:pressed{ image: url(./res/unchecked.png);}"
                                                 "QCheckBox::indicator:checked{image: url(./res/check.png);}"
                                                 "QCheckBox::indicator:checked:hover {image: url(./res/check.png);}"
                                                 "QCheckBox::indicator:checked:pressed {image: url(./res/check.png);}"
                                                 "QCheckBox::indicator:indeterminate {image: url(./res/check.png);}"
                                                 "QCheckBox::indicator:indeterminate:hover {image: url(./res/check.png);}"
                                                 "QCheckBox::indicator:indeterminate:pressed{image: url(./res/check.png);}")
                self.ui.checkBox_5.setChecked(False)
            if not self.ui.lesource1.isHidden():
                ret = re.match(r"^1\d{10}$", self.ui.lesource1.text())
                # if self.ui.lesource1.text().isdigit() and self.ui.lesource1.text() == self.strs:
                if ret:
                    self.ui.btn_login.setEnabled(True)
                    self.ui.captcha.setStyleSheet(
                        "QPushButton{border:0px;font-size: 12px;color:#3790FA;background:transparent;}")
                    self.ui.btn_login.setStyleSheet(
                        "QPushButton{background-color:#3790FA;border:0px;font-size:14px;border-radius:16px;color:#ffffff}QPushButton:hover{background-color:#40A9FB;border:0px;border-radius:16px;font-size:14px;color:#ffffff}QPushButton:pressed{background-color:#296CD9;border:0px;border-radius:16px;font-size:14px;color:#ffffff}")

                else:
                    self.ui.btn_login.setEnabled(False)
                    self.ui.captcha.setStyleSheet(
                        "QPushButton{border:0px;font-size: 12px;color:#8F9399;background:transparent;}")
                    # self.ui.btn_login.setStyleSheet("QPushButton{background-color:#E9E9E9;border:0px;font-size:14px;border-radius:16px;color:#B3B3B3}QPushButton:hover{background-color:#C0C4CC;border:0px;border-radius:16px;font-size:14px;color:#B3B3B3}QPushButton:pressed{background-color:#C0C4CC;border:0px;border-radius:16px;font-size:14px;color:#B3B3B3}")
                    self.ui.btn_login.setStyleSheet(
                        "QPushButton{background-color:#E9E9E9;border:0px;font-size:14px;border-radius:16px;color:#B3B3B3}")

            self.ui.picture_tip.setStyleSheet("QLabel{background-color:transparent;font-size:12px;border:none;color:#C0C4CC;}")
            self.ui.picture_correct.setStyleSheet("QLabel{background-color:transparent;border-image:url('res/picture_correct.png');}")
            self.ui.picture_widget.setStyleSheet("QWidget{background-color:rgba(255,255,255,1);}")
            self.ui.picture_test_background.setStyleSheet("QLabel{background-color:#ffffff;}")
            self.ui.picture_widget_cover.setStyleSheet("QWidget{background-color:rgba(255,255,255,0.5);}")

        else:

            self.ui.centralwidget.setStyleSheet("QFrame{background-color:#3D3D41;}")
            if(Globals.RIGHT_ANGLE):
                self.ui.sourceWidget.setStyleSheet(".QWidget#sourceWidget{background-color:#3D3D41;}")
            else:
                self.ui.sourceWidget.setStyleSheet(".QWidget#sourceWidget{background-color:#3D3D41;border-radius:4px;}")
            # self.ui.btnClose.setStyleSheet("QPushButton{border-radius: 4px;border:0px;background-image:url('res/common-close-white.svg')}QPushButton:hover{border:0px;background-image:url('res/hover-close.svg')}QPushButton:pressed{border:0px;background-image:url('res/clicked-close.svg')}")
            self.ui.btnClose.setStyleSheet("QPushButton{border-radius: 4px;border:0px;border-image:url('res/common-close-white.png')}QPushButton:hover{border:0px;border-image:url('res/hover-close.png')}QPushButton:pressed{border:0px;border-image:url('res/clicked-close.png')}")

            self.ui.usr_icon.setStyleSheet("QWidget{border-image:url('res/username.png');background-color:transparent;border:0px}")
            self.ui.password_icon.setStyleSheet("QWidget{border-image:url('res/password.png');background-color:transparent;border:0px}")

            self.ui.register_new_account.setStyleSheet("QPushButton{border:0px;font-size:14px;no-repeat center left;color:#C0C4CC}QPushButton:hover{color: #3790FA;}QPushButton:pressed{color: #3790FA;}")
            self.ui.forget_pwd.setStyleSheet("QPushButton{border:0px;font-size:14px;color:#C0C4CC;}QPushButton:hover{border:0px;font-size:13px;color#C0C4CC;}QPushButton:pressed{border:0px;font-size:13px;color:#C0C4CC;}")

            self.ui.soft_linedit.setStyleSheet("QLabel{font-size:16px;font-weight:500;color:#FFFFFF}")

            self.ui.tips_user_password.setStyleSheet("QLabel{background-color:transparent;font-size:12px;border:none;color:#F56C6C;}")
            self.ui.tips_user.setStyleSheet("QLabel{background-color:transparent;font-size:12px;border:none;color:#F56C6C;}")
            self.ui.tips_password.setStyleSheet("QLabel{background-color:transparent;font-size:12px;border:none;color:#F56C6C;}")
            self.ui.tips_num.setStyleSheet("QLabel{background-color:transparent;font-size:12px;border:none;color:#F56C6C;}")

            # self.ui.btnClose.setStyleSheet("QPushButton{border-radius: 4px;border:0px;background-image:url('res/close1.png')}QPushButton:hover{border:0px;background-image:url('res/close2.png')}QPushButton:pressed{border:0px;background-image:url('res/close3.png')}")
            self.ui.show_search.setStyleSheet("QPushButton{border-radius: 4px;border:0px;border-image:url('res/show_search_black.png')}QPushButton:hover{border:0px;border-image:url('res/show_search_clicked.png')}QPushButton:pressed{border:0px;border-image:url('res/show_search_clicked.png')}")


            self.ui.separator_line.setStyleSheet("QWidget{border:1px solid #48484C;}")
            self.ui.captcha.setStyleSheet("QPushButton{border:0px;font-size: 12px;color:#C0C4CC;background:transparent;}")

            self.ui.lesource_M_back.setStyleSheet(
                "QLabel{background-color:transparent;border:0px;border-radius:2px;color:#606265;font-size:14px;}")
            self.ui.lesource1_S_back.setStyleSheet(
                "QLabel{background-color:transparent;border:0px;border-radius:2px;color:#606265;font-size:14px;}")
            self.ui.lesource_2_M_back.setStyleSheet(
                "QLabel{background-color:transparent;border:0px;border-radius:2px;color:#606265;font-size:14px;}")
            self.ui.lesource_1_S_back.setStyleSheet(
                "QLabel{background-color:transparent;border:0px;border-radius:2px;color:#606265;font-size:14px;}")

            self.ui.show_hide_pwd.setStyleSheet("QPushButton{border-radius: 4px;border:0px;}QPushButton:hover{border:0px;}QPushButton:pressed{border:0px;}")
            self.ui.show_pwd.setStyleSheet("QPushButton{border-radius: 4px;border:0px;border-image:url('res/close_pwd_black.png');}QPushButton:hover{border:0px;border-image:url('res/close_pwd_click.png');}QPushButton:pressed{border:0px;border-image:url('res/close_pwd_click.png');}")
            self.ui.hide_pwd.setStyleSheet("QPushButton{border-radius: 4px;border:0px;border-image:url('res/show_pwd_black.png');}QPushButton:hover{border:0px;border-image:url('res/show_pwd_click.png');}QPushButton:pressed{border:0px;border-image:url('res/show_pwd_click.png');}")
            self.ui.lesource1.setStyleSheet(
                "QLineEdit{background-color:transparent;border:0px;border-radius:2px;font-size:14px;color:#ffffff;}QLineEdit:pressed{color:#ffffff;}")
            self.ui.lesource_1.setStyleSheet(
                "QLineEdit{background-color:transparent;border:0px;border-radius:2px;color:#ffffff;font-size:14px;}QLineEdit:pressed{color:#ffffff;}")
            self.ui.lesource.setStyleSheet(
                "QLineEdit{background-color:transparent;border:0px;border-radius:2px;color:#ffffff;font-size:14px;}QLineEdit:pressed{color:#ffffff;}")
            self.ui.lesource_2.setStyleSheet(
                "QLineEdit{background-color:transparent;border:0px;border-radius:2px;color:#ffffff;font-size:14px;}QLineEdit:pressed{color:#ffffff;}")
            if self.ui.lesource.isHidden():
                self.ui.btnPassword.setStyleSheet(
                    "QPushButton{border-radius: 12px;border: 1px solid #C0C4CC ;font-size: 12px;color:#C0C4CC;}"
                    "QPushButton:hover{border-radius: 12px;border: 1px solid #3790FA;font-size: 12px;color:#3790FA;}"
                    "QPushButton:pressed{border-radius: 12px;border: 1px solid #3790FA;font-size: 12px;color:#3790FA;}")
                self.ui.btnShortMessage.setStyleSheet(
                    "QPushButton{border-radius: 12px;border: 1px solid #3790FA ;font-size: 12px;color:#3790FA;}"
                    "QPushButton:hover{border-radius: 12px;border: 1px solid #3790FA;font-size: 12px;color:#3790FA;}"
                    "QPushButton:pressed{border-radius: 12px;border: 1px solid #3790FA;font-size: 12px;color:#3790FA;}")
            else:
                self.ui.btnPassword.setStyleSheet("QPushButton{border-radius: 12px;border: 1px solid #3790FA ;font-size: 12px;color:#3790FA;}"
                                                  "QPushButton:hover{border-radius: 12px;border: 1px solid #3790FA;font-size: 12px;color:#3790FA;}"
                                                  "QPushButton:pressed{border-radius: 12px;border: 1px solid #3790FA;font-size: 12px;color:#3790FA;}")
                self.ui.btnShortMessage.setStyleSheet("QPushButton{border-radius: 12px;border: 1px solid #C0C4CC ;font-size: 12px;color:#C0C4CC;}"
                                                      "QPushButton:hover{border-radius: 12px;border: 1px solid #3790FA;font-size: 12px;color:#3790FA;}"
                                                  "QPushButton:pressed{border-radius: 12px;border: 1px solid #3790FA;font-size: 12px;color:#3790FA;}")
            # self.ui.btn_login.setStyleSheet("QPushButton{background-color:#3790FA;border:0px;font-size:14px;border-radius:16px;color:#FFFFFF}")
            if self.ui.lesource.text() != '':

                self.ui.btn_login.setStyleSheet(
                    "QPushButton{background-color:#3790FA;border:0px;font-size:14px;border-radius:16px;color:#ffffff}QPushButton:hover{background-color:#40A9FB;border:0px;border-radius:16px;font-size:14px;color:#ffffff}QPushButton:pressed{background-color:#296CD9;border:0px;border-radius:16px;font-size:14px;color:#ffffff}")
            else:
                self.ui.lesource.setStyleSheet(
                    "QLineEdit{background-color:transparent;border:0px;border-radius:2px;color:#606265;font-size:14px;}QLineEdit:pressed{color:#606265;}")

                self.ui.btn_login.setStyleSheet(
                    "QPushButton{background-color:rgba(255, 255, 255,0.08);border:0px;font-size:14px;border-radius:16px;color:#8F9399}")
            if self.ui.lesource1.isHidden():
                self.ui.rmb_pwd.setStyleSheet(
                    "QPushButton{border:0px;font-size:14px;no-repeat center left;color:#C0C4CC}QPushButton:hover{font-size:13px;color:#C0C4CC;}")
                self.ui.checkBox_5.setStyleSheet("QCheckBox{border-radius:2px;background-color:transparent;}"
                                                 "QCheckBox::indicator{width: 14px; height: 14px;}"
                                                 "QCheckBox::indicator:unchecked{background-color:transparent; image: url(./res/unchecked_black.png);}"
                                                 "QCheckBox::indicator:unchecked:hover{background-color:transparent;image: url(./res/unchecked_black.png);}"
                                                 "QCheckBox::indicator:unchecked:pressed{background-color:transparent; image: url(./res/unchecked_black.png);}"
                                                 "QCheckBox::indicator:checked{background-color:transparent;image: url(./res/check.png);}"
                                                 "QCheckBox::indicator:checked:hover {background-color:transparent;image: url(./res/check.png);}"
                                                 "QCheckBox::indicator:checked:pressed {background-color:transparent;image: url(./res/check.png);}"
                                                 "QCheckBox::indicator:indeterminate {background-color:transparent;image: url(./res/check.png);}"
                                                 "QCheckBox::indicator:indeterminate:hover {background-color:transparent;image: url(./res/check.png);}"
                                                 "QCheckBox::indicator:indeterminate:pressed{background-color:transparent;image: url(./res/check.png);}")
            else:
                self.ui.rmb_pwd.setStyleSheet(
                    "QPushButton{border:0px;font-size:14px;no-repeat center left;color:#5B5B5B}QPushButton:hover{font-size:13px;color:#5B5B5B;}")
                self.ui.checkBox_5.setStyleSheet("QCheckBox{border-radius:2px;background-color:transparent;}"
                                                 "QCheckBox::indicator{width: 14px; height: 14px;}"
                                                 "QCheckBox::indicator:unchecked{background-color:transparent; image: url(./res/uncheck_black_sm.png);}"
                                                 "QCheckBox::indicator:unchecked:hover{background-color:transparent;image: url(./res/uncheck_black_sm.png);}"
                                                 "QCheckBox::indicator:unchecked:pressed{background-color:transparent; image: url(./res/uncheck_black_sm.png);}"
                                                 "QCheckBox::indicator:checked{background-color:transparent;image: url(./res/check.png);}"
                                                 "QCheckBox::indicator:checked:hover {background-color:transparent;image: url(./res/check.png);}"
                                                 "QCheckBox::indicator:checked:pressed {background-color:transparent;image: url(./res/check.png);}"
                                                 "QCheckBox::indicator:indeterminate {background-color:transparent;image: url(./res/check.png);}"
                                                 "QCheckBox::indicator:indeterminate:hover {background-color:transparent;image: url(./res/check.png);}"
                                                 "QCheckBox::indicator:indeterminate:pressed{background-color:transparent;image: url(./res/check.png);}")
            if not self.ui.lesource1.isHidden():
                ret = re.match(r"^1\d{10}$", self.ui.lesource1.text())
                # if self.ui.lesource1.text().isdigit() and self.ui.lesource1.text() == self.strs:
                if ret:
                    self.ui.btn_login.setEnabled(True)
                    self.ui.captcha.setStyleSheet(
                        "QPushButton{border:0px;font-size: 12px;color:#3790FA;background:transparent;}")
                    self.ui.btn_login.setStyleSheet(
                        "QPushButton{background-color:#3790FA;border:0px;font-size:14px;border-radius:16px;color:#FFFFFF}")
                else:
                    self.ui.btn_login.setEnabled(False)
                    self.ui.captcha.setStyleSheet(
                        "QPushButton{border:0px;font-size: 12px;color:#C0C4CC;background:transparent;}")
                    # self.ui.btn_login.setStyleSheet("QPushButton{background-color:#E9E9E9;border:0px;font-size:14px;border-radius:16px;color:#B3B3B3}QPushButton:hover{background-color:#C0C4CC;border:0px;border-radius:16px;font-size:14px;color:#B3B3B3}QPushButton:pressed{background-color:#C0C4CC;border:0px;border-radius:16px;font-size:14px;color:#B3B3B3}")
                    self.ui.btn_login.setStyleSheet("QPushButton{background-color:rgba(255, 255, 255,0.08);border:0px;font-size:14px;border-radius:16px;color:#8F9399}")

            self.ui.picture_tip.setStyleSheet("QLabel{background-color:transparent;font-size:12px;border:none;color:#C0C4CC;}")
            self.ui.picture_correct.setStyleSheet("QLabel{background-color:transparent;background-image:url('res/picture_correct.png');}")
            self.ui.picture_widget.setStyleSheet("QWidget{background-color:#3D3D41;}")
            self.ui.picture_test_background.setStyleSheet("QLabel{background-color:rgba(255,255,255,0.5);}")
            self.ui.picture_widget_cover.setStyleSheet("QWidget{background-color:rgba(255,255,255,0.5);}")
        self.search_user_widget.slot_show_dark_mode()
        self.slide_widget.slot_dark_mode()
        self.show_or_hide_back()
        self.show_or_hide_back_S()
        # QApplication.processEvents()


    #
    # 函数：显示密码可见,并切换按钮
    #
    def slot_show_password(self):
        self.ui.lesource_2.setEchoMode(QLineEdit.Normal)
        self.ui.show_pwd.hide()
        self.ui.hide_pwd.show()

    #
    # 函数：显示密码不可见,并切换按钮
    #
    def slot_hide_password(self):
        self.ui.lesource_2.setEchoMode(QLineEdit.Password)
        self.ui.show_pwd.show()
        self.ui.hide_pwd.hide()



    def slot_click_close(self):
        # self.ui.btnClose.deleteLater()
        self.close()

    def slot_rem_user_pwd(self):
        try:
            if self.ui.checkBox_5.isChecked():

                if self.login_mode == 0:
                    Globals.MODE_PWD_REM = 1
                    auto_login = '1'
                    set_rem_pass = '1'
                    Globals.SET_REM = 1
                    Globals.SET_SM_REM = 0
                    Globals.PASSWORD = self.ui.lesource_2.text()
                    Globals.USER = self.ui.lesource.text()
                    # password_write(set_rem_pass,auto_login,Globals.USER,Globals.PASSWORD)

                else:
                        # Globals.MODE_PWD_REM = 0
                        if self.ui.lesource_2.text() != '' or self.ui.lesource.text() != '':
                            Globals.PASSWORD = self.ui.lesource_2.text()
                            Globals.USER = self.ui.lesource.text()
                            Globals.SET_REM = 1
                        else:
                            Globals.SET_SM_REM = 1
                            f = open('./backend/service/password.txt', "r+")
                            f.truncate()
            else:
                Globals.MODE_PWD_REM = 0
                Globals.SET_REM = 0
                Globals.SET_SM_REM = 0
                self.ui.lesource1.clear()
                self.ui.lesource_2.clear()
                self.ui.lesource_1.clear()
                self.ui.lesource.clear()
                f=open('./backend/service/password.txt', "r+")
                f.truncate()
            # if self.close_rem_state == False:
            #     f = open('./backend/service/password.txt', "r+")
            #     f.truncate()
            # else:
            #     pass


        except:
            pass

    def decode_user_pwd(self):
        save_user_pwd = []
        if len(Globals.USER_AND_PWD) > 5:
            save_user_pwd = Globals.USER_AND_PWD[-5:]
        # for idx in range(0,len(self.save_user_pwd)):
        #
        #     # for user_item in self.save_user_pwd:
        #     self.save_user_pwd[idx] = list(self.save_user_pwd[idx])
        #     self.save_user_pwd[idx][0] = base64.b64decode(self.save_user_pwd[idx][0]).decode()
        #     self.save_user_pwd[idx][1] = base64.b64decode(self.save_user_pwd[idx][1]).decode()
        #     self.save_user_pwd[idx][2] = base64.b64decode(self.save_user_pwd[idx][2]).decode()
        Globals.USER_AND_PWD.extend(save_user_pwd)
        # Globals.USER_AND_PWD_DECODE.extend(save_user_pwd)
        self.show_save_user()

    def show_save_user(self):
        # self.save_user_pwd = Globals.USER_AND_PWD_DECODE
        if len(Globals.USER_AND_PWD) == 0:
            self.senior_search.close()
        else:
            save_user_item = list(reversed(Globals.USER_AND_PWD))
            num = 0
            for i in range(len(Globals.USER_AND_PWD)):
                passwd = "kylin123"
                iv = os.urandom(32)
                aescryptor = Aescrypt("kylin123", AES.MODE_ECB, iv)  # ECB模式
                # de = base64.b64decode(self.save_user_item[i][0])
                # state = aescryptor.aesdecrypt(de)
                # self.list_item.state_label.setText(state.decode())
                # self.list_item.username_label.hide()
                de = base64.b64decode(Globals.USER_AND_PWD[i][1])
                user_name = aescryptor.aesdecrypt(de)
                if(user_name.decode() == ""):
                    continue
                item = QListWidgetItem()
                list_item = line_widget(206, self.senior_search)
                # de = base64.b64decode(self.save_user_item[i][0])
                # state = aescryptor.aesdecrypt(de)
                # self.list_item.state_label.setText(state.decode())
                # self.list_item.username_label.hide()
                de = base64.b64decode(Globals.USER_AND_PWD[i][1])
                user_name = aescryptor.aesdecrypt(de)
                list_item.username_label.setText(user_name.decode())
                list_item.username_label.show()
                de = base64.b64decode(Globals.USER_AND_PWD[i][2])
                passwd = aescryptor.aesdecrypt(de)
                list_item.pwd_label.setText(passwd.decode())

                # self.ui.search_user_widget.addItem(self.save_user_item[i][1])
                # self.state_list.append(self.save_user_item[i][0])
                # self.user_list.append(self.save_user_item[i][1])
                # self.pwd_list.append(self.save_user_item[i][2])

                self.senior_search.item_list_widget.addItem(item)
                self.senior_search.item_list_widget.setItemWidget(item, list_item)
                num = num + 1
                if(num >= 5):
                    break

            self.senior_search.item_list_widget.setFixedHeight(num * list_item.height())
            self.senior_search.item_list_widget.show()
            self.senior_search.widget.setFixedHeight(num * (list_item.height() + 2)+2)
            self.senior_search.setFixedHeight(num * (list_item.height()+2)+2+ 20)
            self.senior_search.show()
            self.senior_search.hide()
            self.senior_search.item_list_widget.itemClicked.connect(self.slot_click_item)

    # 点击用户名下拉框中的item自动填入用户名
    def slot_click_item(self,item):

        self.search_user_widget.hide()
        self.senior_search.hide()
        count = self.senior_search.item_list_widget.currentRow()
        get_item_widget = self.senior_search.item_list_widget.itemWidget(item)

        if self.senior_search.item_list_widget.item(count).isSelected():
            self.senior_search.item_list_widget.item(count).setSelected(False)

        self.ui.lesource.clear()
        self.ui.lesource.setText(get_item_widget.username_label.text())
        self.ui.lesource_2.clear()
        self.ui.lesource_2.setText(get_item_widget.pwd_label.text())
        if get_item_widget.pwd_label.text() == '':
            self.ui.checkBox_5.setChecked(False)
        else:
            self.ui.checkBox_5.setChecked(True)

    # 点击自动填入用户名
    def slot_click_item_for_search(self, item):
        self.search_user_widget.hide()
        get_item_widget = self.search_user_widget.item_list_widget.itemWidget(item)
        self.ui.lesource.clear()
        self.ui.lesource.setText(get_item_widget.username_label.text())
        self.ui.lesource_2.clear()
        self.ui.lesource_2.setText(get_item_widget.pwd_label.text())
        if get_item_widget.pwd_label.text() == '':
            self.ui.checkBox_5.setChecked(False)
        else:
            self.ui.checkBox_5.setChecked(True)
        self.search_user_widget.hide()


    # 展示当前存储的用户
    def slot_show_searchwidget(self):

        if self.senior_search.isHidden():
            self.search_user_widget.close()
            self.senior_search.raise_()
            self.senior_search.show()
        else:
            self.senior_search.hide()
    
    #
    #函数名: 鼠标点击事件
    #Function: Mouse click event
    #
    def mousePressEvent(self, event):
        if(event.button() == Qt.LeftButton):
            self.clickx = event.globalPos().x()
            self.clicky = event.globalPos().y()

            if self.senior_search.isHidden():
                pass
            else:
                self.senior_search.hide()
            if self.search_user_widget.isHidden():
                pass
            else:
                self.search_user_widget.hide()
            self.mouse_press = True
            self.drag_pos = event.globalPos() - self.pos()
            event.accept()
    # #
    # #函数名: 窗口拖动事件
    # #Function: Window drag event
    # #
    # def mouseMoveEvent(self, event):
    #     if (self.mouse_press):
    #         self.move_pos = event.globalPos()
    #         self.move(self.move_pos - self.drag_pos)
    #         self.setWindowOpacity(1)
    #         event.accept()

    #
    # 函数名: 鼠标释放事件
    # Function: Window drag event
    #
    def mouseReleaseEvent(self, event):
        self.mouse_press = False
        self.setWindowOpacity(1)
    #
    # 函数名:找回密码
    # Function:find password
    #
    def find_password_suc(self):
        url = Globals.FIND_PASSWORD
        webbrowser_open_url(url)
        # QDesktopServices.openUrl(QUrl(url))

    #
    # 函数名:记住密码
    # Function:find password
    #
    def rem_password_suc(self):
        if self.ui.checkBox_5.isChecked():
            self.rem_click = False
        else:
            self.rem_click = True
        if self.rem_click == True:
            self.rem_click = False
            self.ui.checkBox_5.setChecked(True)
        else:
            self.rem_click = True
            self.ui.checkBox_5.setChecked(False)


    #
    # 函数名:初始化界面
    # Function:init interface
    #
    def ui_init(self):
        self.ui = Ui_Login_ui()
        self.ui.setupUi(self)
        # self.show()
    #
    # 函数名:点击关闭
    # Function:click close
    #
    #函数名：点击切换短信验证登录
    def slot_click_SM(self):
        if(hasattr(self,"senior_search")):
            self.senior_search.hide()
        self.ui.show_hide_pwd.hide()
        self.ui.register_new_account.show()
        self.ui.lesource_M_back.hide()
        self.ui.lesource_2_M_back.hide()
        self.ui.lesource_1_S_back.show()
        self.ui.lesource1_S_back.show()
        self.ui.forget_pwd.setEnabled(False)
        self.slot_hide_password()
        self.ui.btnPassword.setEnabled(True)
        self.ui.btnShortMessage.setEnabled(False)
        if Globals.THEME_COLOR == 0:
            self.ui.captcha.setStyleSheet(
                "QPushButton{border:0px;font-size: 12px;color:#8F9399;background:transparent;}")
            self.ui.password_icon.setStyleSheet(
                "QWidget{border-image:url('res/message.png');border:0px}")
            self.ui.btnPassword.setStyleSheet(
                "QPushButton{border-radius: 12px;border: 1px solid #C0C4CC ;font-size: 12px;color:#C0C4CC;}"
                "QPushButton:hover{border-radius: 12px;border: 1px solid #3790FA;font-size: 12px;color:#3790FA;}"
                "QPushButton:pressed{border-radius: 12px;border: 1px solid #3790FA;font-size: 12px;color:#3790FA;}")
            self.ui.btnShortMessage.setStyleSheet(
                "QPushButton{border-radius: 12px;border: 1px solid #3790FA ;font-size: 12px;color:#3790FA;}"
                "QPushButton:hover{border-radius: 12px;border: 1px solid #3790FA;font-size: 12px;color:#3790FA;}"
                "QPushButton:pressed{border-radius: 12px;border: 1px solid #3790FA;font-size: 12px;color:#3790FA;}")
            self.ui.rmb_pwd.setStyleSheet("QPushButton{border:0px;font-size:14px;no-repeat center left;color:#D5D8DE;}QPushButton:hover{font-size:14px;color:#D5D8DE;}")
            self.ui.forget_pwd.setStyleSheet("QPushButton{border:0px;font-size:14px;color:#D5D8DE;}QPushButton:hover{border:0px;font-size:13px;color:#303133;}QPushButton:pressed{border:0px;font-size:13px;color:#303133;}")
        else:
            self.ui.captcha.setStyleSheet(
                "QPushButton{border:0px;font-size: 12px;color:#C0C4CC;background:transparent;}")
            self.ui.password_icon.setStyleSheet(
                "QWidget{border-image:url('res/message_black.png');border:0px}")
            self.ui.btnPassword.setStyleSheet(
                "QPushButton{border-radius: 12px;border: 1px solid #C0C4CC ;font-size: 12px;color:#C0C4CC;}"
                "QPushButton:hover{border-radius: 12px;border: 1px solid #3790FA;font-size: 12px;color:#3790FA;}"
                "QPushButton:pressed{border-radius: 12px;border: 1px solid #3790FA;font-size: 12px;color:#3790FA;}")
            self.ui.btnShortMessage.setStyleSheet(
                "QPushButton{border-radius: 12px;border: 1px solid #3790FA ;font-size: 12px;color:#3790FA;}"
                "QPushButton:hover{border-radius: 12px;border: 1px solid #3790FA;font-size: 12px;color:#3790FA;}"
                "QPushButton:pressed{border-radius: 12px;border: 1px solid #3790FA;font-size: 12px;color:#3790FA;}")
            self.ui.rmb_pwd.setStyleSheet("QPushButton{border:0px;font-size:14px;no-repeat center left;color:#5B5B5B}QPushButton:hover{font-size:13px;color:#5B5B5B;}")
            self.ui.forget_pwd.setStyleSheet("QPushButton{border:0px;font-size:14px;color:#5B5B5B;}QPushButton:hover{border:0px;font-size:13px;color:#303133;}QPushButton:pressed{border:0px;font-size:13px;color:#303133;}")
            self.ui.checkBox_5.setStyleSheet("QCheckBox{border-radius:2px;background-color:transparent;}"
                                             "QCheckBox::indicator{width: 14px; height: 14px;}"
                                             "QCheckBox::indicator:unchecked{background-color:transparent; image: url(./res/uncheck_black_sm.png);}"
                                             "QCheckBox::indicator:unchecked:hover{background-color:transparent;image: url(./res/uncheck_black_sm.png);}"
                                             "QCheckBox::indicator:unchecked:pressed{background-color:transparent; image: url(./res/uncheck_black_sm.png);}"
                                             "QCheckBox::indicator:checked{background-color:transparent;image: url(./res/check.png);}"
                                             "QCheckBox::indicator:checked:hover {background-color:transparent;image: url(./res/check.png);}"
                                             "QCheckBox::indicator:checked:pressed {background-color:transparent;image: url(./res/check.png);}"
                                             "QCheckBox::indicator:indeterminate {background-color:transparent;image: url(./res/check.png);}"
                                             "QCheckBox::indicator:indeterminate:hover {background-color:transparent;image: url(./res/check.png);}"
                                             "QCheckBox::indicator:indeterminate:pressed{background-color:transparent;image: url(./res/check.png);}")
        self.ui.rmb_pwd.setEnabled(False)
        if self.ui.checkBox_5.isChecked():
            # self.mode_pwd_rem = 1
            Globals.MODE_PWD_REM = 1
            self.ui.checkBox_5.setChecked(False)
            self.ui.checkBox_5.setEnabled(False)
        else:
            # self.mode_pwd_rem = 0
            Globals.MODE_PWD_REM = 0
            self.ui.checkBox_5.setEnabled(False)

        self.ui.lesource1.clear()
        self.ui.lesource_1.clear()

        self.ui.lesource1.setFocus(True)
        self.ui.picture_widget.hide()
        self.ui.btn_login.show()
        self.ui.Login_user_box.hide()
        self.ui.Login_pwd_box.hide()
        self.ui.Login_sm_box.show()
        self.ui.Login_phone_box.show()
        # self.ui.lesource.clear()
        # self.ui.lesource_2.clear()

        self.ui.lesource1.show()
        self.ui.lesource_1.show()
        self.ui.lesource.hide()
        # self.ui.show_search.hide()
        self.ui.lesource_2.hide()
        # self.
        # self.timer1.stop()
        self.count = 0
        self.slot_time_Refresh()
        self.ui.captcha.show()

        self.ui.separator_line.show()
        length_lesource1 = len(self.ui.lesource1.text())
        self.ui.lesource.setSelection(length_lesource1, length_lesource1)

        # caoliang
        self.login_mode = 1

        ret = re.match(r"^1\d{10}$", self.ui.lesource1.text())
        # if self.ui.lesource1.text().isdigit() and self.ui.lesource1.text() == self.strs:
        if ret:
            self.ui.btn_login.setEnabled(True)
            if Globals.THEME_COLOR == 0:
            # self.ui.Login_phone_box.setStyleSheet("QWidget{border:1px solid #3790FA}")
                self.ui.captcha.setStyleSheet("QPushButton{border:0px;font-size: 12px;color:#3790FA;background:transparent;}")
                self.ui.btn_login.setStyleSheet("QPushButton{background-color:#3790FA;border:0px;font-size:14px;border-radius:16px;color:#ffffff}QPushButton:hover{background-color:#40A9FB;border:0px;border-radius:16px;font-size:14px;color:#ffffff}QPushButton:pressed{background-color:#296CD9;border:0px;border-radius:16px;font-size:14px;color:#ffffff}")
            else:
                self.ui.captcha.setStyleSheet(
                    "QPushButton{border:0px;font-size: 12px;color:#3790FA;background:transparent;}")
                self.ui.btn_login.setStyleSheet(
                    "QPushButton{background-color:#3790FA;border:0px;font-size:14px;border-radius:16px;color:#FFFFFF}")

        else:
            self.ui.btn_login.setEnabled(False)
            self.ui.tips_num.hide()
            self.ui.tips_user.hide()
            if Globals.THEME_COLOR == 0:
                self.ui.captcha.setStyleSheet("QPushButton{border:0px;font-size: 12px;color:#8F9399;background:transparent;}")
                # self.ui.btn_login.setStyleSheet("QPushButton{background-color:#E9E9E9;border:0px;font-size:14px;border-radius:16px;color:#B3B3B3}QPushButton:hover{background-color:#C0C4CC;border:0px;border-radius:16px;font-size:14px;color:#B3B3B3}QPushButton:pressed{background-color:#C0C4CC;border:0px;border-radius:16px;font-size:14px;color:#B3B3B3}")
                self.ui.btn_login.setStyleSheet("QPushButton{background-color:#E9E9E9;border:0px;font-size:14px;border-radius:16px;color:#B3B3B3}")
            else:
                self.ui.captcha.setStyleSheet(
                    "QPushButton{border:0px;font-size: 12px;color:#C0C4CC;background:transparent;}")
                # self.ui.btn_login.setStyleSheet("QPushButton{background-color:#E9E9E9;border:0px;font-size:14px;border-radius:16px;color:#B3B3B3}QPushButton:hover{background-color:#C0C4CC;border:0px;border-radius:16px;font-size:14px;color:#B3B3B3}QPushButton:pressed{background-color:#C0C4CC;border:0px;border-radius:16px;font-size:14px;color:#B3B3B3}")
                self.ui.btn_login.setStyleSheet("QPushButton{background-color:rgba(255, 255, 255,0.08);border:0px;font-size:14px;border-radius:16px;color:#8F9399}")


            # self.ui.Login_phone_box.setStyleSheet("QWidget{border:1px solid #EBEEF5}")


        # if self.ui.lesource1 == '':



    #函数名：点击切换密码登录
    def slot_click_PW(self):
        # print("%^%^%^%")
        #self.check_text_state()
        self.ui.lesource1_S_back.hide()
        self.ui.lesource_1_S_back.hide()
        self.ui.lesource_M_back.show()
        self.ui.lesource_2_M_back.show()
        self.ui.rmb_pwd.setEnabled(True)
        self.ui.forget_pwd.setEnabled(True)
        self.ui.btnPassword.setEnabled(False)
        self.ui.btnShortMessage.setEnabled(True)
        self.listlogin[1] = self.ui.lesource_2.text()
        if Globals.THEME_COLOR == 0:

            self.ui.password_icon.setStyleSheet(
                "QWidget{border-image:url('res/passwordblack.png');border:0px}")

            self.ui.btnPassword.setStyleSheet(
                "QPushButton{border-radius: 12px;border: 1px solid #3790FA ;font-size: 12px;color:#3790FA;}"
                "QPushButton:hover{border-radius: 12px;border: 1px solid #3790FA;font-size: 12px;color:#3790FA;}"
                "QPushButton:pressed{border-radius: 12px;border: 1px solid #3790FA;font-size: 12px;color:#3790FA;}")
            self.ui.btnShortMessage.setStyleSheet(
                "QPushButton{border-radius: 12px;border: 1px solid #C0C4CC ;font-size: 12px;color:#C0C4CC;}"
                "QPushButton:hover{border-radius: 12px;border: 1px solid #3790FA;font-size: 12px;color:#3790FA;}"
                "QPushButton:pressed{border-radius: 12px;border: 1px solid #3790FA;font-size: 12px;color:#3790FA;}")
            self.ui.rmb_pwd.setStyleSheet("QPushButton{border:0px;font-size:14px;no-repeat center left;color:#303133}QPushButton:hover{font-size:13px;color:#303133;}")
            self.ui.forget_pwd.setStyleSheet("QPushButton{border:0px;font-size:14px;color:#303133;}QPushButton:hover{border:0px;font-size:13px;color:#303133;}QPushButton:pressed{border:0px;font-size:13px;color:#303133;}")
        else:
            self.ui.password_icon.setStyleSheet(
                "QWidget{border-image:url('res/password.png');border:0px}")

            self.ui.btnPassword.setStyleSheet(
                "QPushButton{border-radius: 12px;border: 1px solid #3790FA ;font-size: 12px;color:#3790FA;}"
                "QPushButton:hover{border-radius: 12px;border: 1px solid #3790FA;font-size: 12px;color:#3790FA;}"
                "QPushButton:pressed{border-radius: 12px;border: 1px solid #3790FA;font-size: 12px;color:#3790FA;}")
            self.ui.btnShortMessage.setStyleSheet(
                "QPushButton{border-radius: 12px;border: 1px solid #C0C4CC ;font-size: 12px;color:#C0C4CC;}"
                "QPushButton:hover{border-radius: 12px;border: 1px solid #3790FA;font-size: 12px;color:#3790FA;}"
                "QPushButton:pressed{border-radius: 12px;border: 1px solid #3790FA;font-size: 12px;color:#3790FA;}")
            self.ui.rmb_pwd.setStyleSheet("QPushButton{border:0px;font-size:14px;no-repeat center left;color:#C0C4CC}QPushButton:hover{font-size:13px;color:#C0C4CC;}")
            self.ui.forget_pwd.setStyleSheet(
                "QPushButton{border:0px;font-size:14px;color:#C0C4CC;}QPushButton:hover{border:0px;font-size:13px;color#C0C4CC;}QPushButton:pressed{border:0px;font-size:13px;color:#C0C4CC;}")
            self.ui.checkBox_5.setStyleSheet("QCheckBox{border-radius:2px;background-color:transparent;}"
                                             "QCheckBox::indicator{width: 14px; height: 14px;}"
                                             "QCheckBox::indicator:unchecked{background-color:transparent; image: url(./res/unchecked_black.png);}"
                                             "QCheckBox::indicator:unchecked:hover{background-color:transparent;image: url(./res/unchecked_black.png);}"
                                             "QCheckBox::indicator:unchecked:pressed{background-color:transparent; image: url(./res/unchecked_black.png);}"
                                             "QCheckBox::indicator:checked{background-color:transparent;image: url(./res/check.png);}"
                                             "QCheckBox::indicator:checked:hover {background-color:transparent;image: url(./res/check.png);}"
                                             "QCheckBox::indicator:checked:pressed {background-color:transparent;image: url(./res/check.png);}"
                                             "QCheckBox::indicator:indeterminate {background-color:transparent;image: url(./res/check.png);}"
                                             "QCheckBox::indicator:indeterminate:hover {background-color:transparent;image: url(./res/check.png);}"
                                             "QCheckBox::indicator:indeterminate:pressed{background-color:transparent;image: url(./res/check.png);}")
        if Globals.MODE_PWD_REM == 1:
            self.ui.checkBox_5.setChecked(True)
        else:
            self.ui.checkBox_5.setChecked(False)
        # self.ui.lesource.setFocus(True)
        self.ui.checkBox_5.setEnabled(True)
        self.ui.picture_widget.hide()
        self.ui.Login_phone_box.hide()
        self.ui.Login_sm_box.hide()
        self.ui.Login_pwd_box.show()
        self.ui.Login_user_box.show()
        self.ui.btn_login.show()
        # self.ui.lesource1.clear()
        # self.ui.lesource_1.clear()
        self.ui.lesource1.hide()
        self.ui.lesource_1.hide()
        self.ui.lesource_2.show()
        self.ui.lesource.show()
        # self.ui.show_search.show()
        self.ui.captcha.hide()
        self.ui.separator_line.hide()
        self.ui.lesource.setFocus(True)
        length_lesource = len(self.ui.lesource.text())
        self.ui.lesource.setSelection(length_lesource,length_lesource)


        # caoliang
        self.login_mode = 0
        if not self.ui.lesource.text() == '':
            self.ui.lesource_M_back.hide()
            sourcetext = str(self.ui.lesource.text())
            self.listlogin[0] = sourcetext

            self.ui.btn_login.setEnabled(True)
            # self.ui.Login_user_box.setStyleSheet("QWidget{border:1px solid #3790FA}")
            self.ui.btn_login.setStyleSheet("QPushButton{background-color:#3790FA;border:0px;font-size:14px;border-radius:16px;color:#ffffff}QPushButton:hover{background-color:#40A9FB;border:0px;border-radius:16px;font-size:14px;color:#ffffff}QPushButton:pressed{background-color:#296CD9;border:0px;border-radius:16px;font-size:14px;color:#ffffff}")
        else:
            self.ui.lesource_M_back.show()
            if Globals.THEME_COLOR == 0:
            # self.ui.Login_user_box.setStyleSheet("QWidget{border:1px solid #EBEEF5}")
            # self.ui.btn_login.setStyleSheet("QPushButton{background-color:#E9E9E9;border:0px;font-size:14px;border-radius:16px;color:#B3B3B3}QPushButton:hover{background-color:#E9E9E9;border:0px;border-radius:16px;font-size:14px;color:#B3B3B3}QPushButton:pressed{background-color:#E9E9E9;border:0px;border-radius:16px;font-size:14px;color:#B3B3B3}")
                self.ui.btn_login.setStyleSheet("QPushButton{background-color:#E9E9E9;border:0px;font-size:14px;border-radius:16px;color:#B3B3B3}")
            else:
                self.ui.btn_login.setStyleSheet("QPushButton{background-color:rgba(255, 255, 255,0.08);border:0px;font-size:14px;border-radius:16px;color:#8F9399}")

        if not self.ui.lesource_2.text() == '':
            self.ui.lesource_2_M_back.hide()
            self.ui.show_hide_pwd.show()
            self.ui.show_pwd.show()
            self.ui.hide_pwd.hide()
        else:
            self.ui.lesource_2_M_back.show()
            self.ui.show_hide_pwd.hide()
        #
        # if self.ui.lesource1.isHidden():
        #     print("dsgfsgsfdgfdsg")



    # self.paintEvent = self.set_paintEvent
    # # 函数：重绘函数,添加外边框阴影
    #
    def set_paintEvent(self, event):
        painter = QPainter(self.ui.centralwidget)
        m_defaultBackgroundColor = QColor(qRgb(0, 0, 0))
        m_defaultBackgroundColor.setAlpha(16)
        path = QPainterPath()
        path.setFillRule(Qt.WindingFill)
        path.addRoundedRect(10, 10, self.ui.sourceWidget.width() - 20, self.ui.sourceWidget.height() - 20,6, 6)

        painter.setRenderHint(QPainter.Antialiasing, True)

        color = QColor(0, 0, 0, 0.2)
        i = 0
        while i < 4:
            path = QPainterPath()
            path.setFillRule(Qt.WindingFill)
            path.addRoundedRect(10 - i, 10 - i, self.ui.centralwidget.width() - (10 - i) * 2, self.ui.centralwidget.height() - (10 - i) * 2, 6, 6)
            color.setAlpha(int(100 - math.sqrt(i) * 50))
            painter.setPen(color)
            painter.drawPath(path)
            i = i + 1

        painter.setRenderHint(QPainter.Antialiasing)

    #函数名：计时器60s一循环。初步设置为不可被打断
    def slot_time_Refresh(self):

        if self.count > 0:
            # self.string = (_("已发送"))
            self.string = (_("Sent "))
            self.ui.captcha.setText(self.string+str(self.count)+"s")
            self.ui.captcha.setStyleSheet("QPushButton{border:0px;font-size: 12px;color:#8F9399;background:transparent;}")
            self.count -= 1
        else:
            self.timer1.stop()
            self.ui.captcha.setEnabled(True)
            self.ui.captcha.setStyleSheet("QPushButton{border:0px;font-size: 12px;color:#3790FA;background:transparent;}")
            # self.ui.captcha.setText(_('发送验证码'))
            self.ui.captcha.setText(_("Get code"))
            self.count = 59


    # 函数名:点击登录后的样式改变
    # Function:click login
    #
    def slot_click_login(self):
        pass
        # self.ui.groupBox.show()
        # self.ui.register_new_account.setStyleSheet("QPushButton{border:0px;font-size:14px;no-repeat center left;color:#303133}QPushButton:hover{font-size:13px;color:#303133;}")

    #
    # 函数名:注册新用户
    # Function:add user
    #
    def slot_click_adduser(self):
        url = Globals.REGISTER_ACCOUNT
        # webbrowser.open(url)
        webbrowser_open_url(url)
        # QDesktopServices.openUrl(QUrl(url))



    # 函数名:用户名输入
    # Function:user name input
    # 
    def slot_le_input(self,text):
        if(Globals.LOGIN_SUCCESS):
            return
        if(text == ""):
            self.search_user_widget.hide()
        self.senior_search.close()
        self.ui.lesource.adjustSize()
        sourcetext = str(text)
        #输入框输入的用户名字
        username = sourcetext

        self.ui.Login_phone_box.hide()
        self.ui.Login_sm_box.hide()
        self.ui.Login_pwd_box.show()
        self.ui.Login_user_box.show()
        # self.ui.lesource.raise_()
        # self.ui.lesource.show()
        # self.ui.Login_user_box.setStyleSheet("QWidget{border:1px solid #3790FA}")
        self.ui.btn_login.setStyleSheet("QPushButton{background-color:#3790FA;border:0px;font-size:14px;border-radius:16px;color:#ffffff}QPushButton:hover{background-color:#40A9FB;border:0px;border-radius:16px;font-size:14px;color:#ffffff}QPushButton:pressed{background-color:#296CD9;border:0px;border-radius:16px;font-size:14px;color:#ffffff}")
        self.ui.tips_user.hide()
        if not self.ui.lesource.text() == '':
            self.ui.lesource_M_back.hide()
            # self.ui.Login_user_box.setStyleSheet("QWidget{border:1px solid #3790FA}")
            self.ui.btn_login.setStyleSheet("QPushButton{background-color:#3790FA;border:0px;font-size:14px;border-radius:16px;color:#ffffff}QPushButton:hover{background-color:#40A9FB;border:0px;border-radius:16px;font-size:14px;color:#ffffff}QPushButton:pressed{background-color:#296CD9;border:0px;border-radius:16px;font-size:14px;color:#ffffff}")
            self.ui.btn_login.setEnabled(True)
        else:
            self.ui.lesource_M_back.show()
            # self.ui.Login_user_box.setStyleSheet("QWidget{border:1px solid #EBEEF5}")
            # self.ui.btn_login.setStyleSheet("QPushButton{background-color:#E9E9E9;border:0px;font-size:14px;border-radius:16px;color:#B3B3B3}QPushButton:hover{background-color:#E9E9E9;border:0px;border-radius:16px;font-size:14px;color:#B3B3B3}QPushButton:pressed{background-color:#E9E9E9;border:0px;border-radius:16px;font-size:14px;color:#B3B3B3}")
            if Globals.THEME_COLOR == 0:
                self.ui.btn_login.setStyleSheet("QPushButton{background-color:#E9E9E9;border:0px;font-size:14px;border-radius:16px;color:#B3B3B3}")
            else:
                self.ui.btn_login.setStyleSheet("QPushButton{background-color:rgba(255, 255, 255,0.08);border:0px;font-size:14px;border-radius:16px;color:#8F9399}")
        if Globals.THEME_COLOR != 0:
            if self.ui.lesource.text() != '':
                self.ui.lesource_M_back.hide()
                self.ui.lesource.setStyleSheet(
                    "QLineEdit{background-color:transparent;border:0px;border-radius:2px;color:#ffffff;font-size:14px;}QLineEdit:pressed{color:#ffffff;}")

            else:
                self.ui.lesource_M_back.show()
                self.ui.lesource.setStyleSheet(
                    "QLineEdit{background-color:transparent;border:0px;border-radius:2px;color:#606265;font-size:14px;}QLineEdit:pressed{color:#606265;}")

        search_list = []
        search_list.clear()
        text_len = len(username)

        # for i in range(0, len(self.save_user_item)):
        #     if (username in self.save_user_item[i][1]):
        #         if username == self.save_user_item[i][1]:
        #             pass
        #         else:
        #             search_list.append(self.save_user_item[i])
        #     else:
        #         self.search_user_widget.close()
        passwd = "kylin123"
        iv = os.urandom(32)
        aescryptor = Aescrypt("kylin123", AES.MODE_ECB, iv)  # ECB模式
        for i in range(0, len(Globals.USER_AND_PWD)):

            de = base64.b64decode(Globals.USER_AND_PWD[i][1])
            name = aescryptor.aesdecrypt(de)
            get_name = name.decode()
            if username in get_name:

                if username == get_name[0:text_len]:
                    if username == get_name:
                        pass
                    else:
                        search_list.append(Globals.USER_AND_PWD[i])
            else:
                self.search_user_widget.close()
        if self.first_open != 0:

            self.search_user_widget.item_list_widget.clear()
            if len(search_list) == 0:
                self.search_user_widget.close()
            else:
                num = 0
                for i in range(len(search_list)):
                    item1 = QListWidgetItem()
                    list_item1 = line_widget(206, self.search_user_widget)

                    de = base64.b64decode(search_list[i][0])
                    state = aescryptor.aesdecrypt(de)
                    list_item1.state_label.setText(state.decode())
                    list_item1.state_label.adjustSize()
                    list_item1.state_label.hide()
                    de = base64.b64decode(search_list[i][1])
                    user_name = aescryptor.aesdecrypt(de)
                    list_item1.username_label.setText(user_name.decode())
                    list_item1.username_label.adjustSize()
                    list_item1.username_label.show()
                    de = base64.b64decode(search_list[i][2])
                    passwd = aescryptor.aesdecrypt(de)
                    list_item1.pwd_label.adjustSize()
                    list_item1.pwd_label.setText(passwd.decode())
                    # self.list_item1.state_label.setText(search_list[i][0])
                    # self.list_item1.username_label.hide()
                    # self.list_item1.username_label.setText(search_list[i][1])
                    # self.list_item1.username_label.show()
                    # self.list_item1.pwd_label.setText(search_list[i][2])
                    self.search_user_widget.item_list_widget.addItem(item1)
                    self.search_user_widget.item_list_widget.setItemWidget(item1, list_item1)
                    num = num + 1
                    if(num >= 5):
                        break

                self.search_user_widget.item_list_widget.setFixedHeight(len(search_list) * (list_item1.height()+2))
                self.search_user_widget.item_list_widget.show()
                self.search_user_widget.widget.setFixedHeight(len(search_list) * (list_item1.height()+2)+2)
                self.search_user_widget.setFixedHeight(len(search_list) * (list_item1.height() +2)+2 + 20)

                self.search_user_widget.show()


        else:
            self.first_open += 1




    #函数名：手机号输入
    def slot_le_input1(self,text = ""):
        self.ui.Login_user_box.hide()
        self.ui.Login_pwd_box.hide()
        self.ui.Login_sm_box.show()
        self.ui.Login_phone_box.show()
        if self.ui.lesource1.text() != '':
            self.ui.lesource1_S_back.hide()
        else:
            self.ui.lesource1_S_back.show()

        # self.ui.lesource1.adjustSize()
        sourcetext = str(text)
        self.listlogin[0] = sourcetext
        ret = re.match(r"^1\d{10}$", self.ui.lesource1.text())
        # if self.ui.lesource1.text().isdigit() and self.ui.lesource1.text() == self.strs:
        if ret:
            self.ui.btn_login.setEnabled(True)
            # self.ui.Login_phone_box.setStyleSheet("QWidget{border:1px solid #3790FA}")
            self.ui.captcha.setStyleSheet(
                "QPushButton{border:0px;font-size: 12px;color:#3790FA;background:transparent;}")
            self.ui.btn_login.setStyleSheet(
                "QPushButton{background-color:#3790FA;border:0px;font-size:14px;border-radius:16px;color:#ffffff}QPushButton:hover{background-color:#40A9FB;border:0px;border-radius:16px;font-size:14px;color:#ffffff}QPushButton:pressed{background-color:#296CD9;border:0px;border-radius:16px;font-size:14px;color:#ffffff}")
        else:
            self.ui.btn_login.setEnabled(False)
            self.ui.tips_num.hide()
            self.ui.tips_user.hide()
            self.ui.captcha.setStyleSheet(
                "QPushButton{border:0px;font-size: 12px;color:#C0C4CC;background:transparent;}")
            # self.ui.btn_login.setStyleSheet("QPushButton{background-color:#E9E9E9;border:0px;font-size:14px;border-radius:16px;color:#B3B3B3}QPushButton:hover{background-color:#E9E9E9;border:0px;border-radius:16px;font-size:14px;color:#B3B3B3}QPushButton:pressed{background-color:#E9E9E9;border:0px;border-radius:16px;font-size:14px;color:#B3B3B3}")
            if Globals.THEME_COLOR == 0:
                self.ui.btn_login.setStyleSheet(
                    "QPushButton{background-color:#E9E9E9;border:0px;font-size:14px;border-radius:16px;color:#B3B3B3}")
            else:
                self.ui.btn_login.setStyleSheet(
                    "QPushButton{background-color:rgba(255, 255, 255,0.08);border:0px;font-size:14px;border-radius:16px;color:#8F9399}")
        if Globals.THEME_COLOR != 0:
            if self.ui.lesource1.text() != '':
                self.ui.lesource1.setStyleSheet(
                    "QLineEdit{background-color:transparent;border:0px;border-radius:2px;color:#ffffff;font-size:14px;}QLineEdit:pressed{color:#ffffff;}")

            else:
                self.ui.lesource1.setStyleSheet(
                    "QLineEdit{background-color:transparent;border:0px;border-radius:2px;color:#606265;font-size:14px;}QLineEdit:pressed{color:#606265;}")

    #
    # 函数名:密码输入
    # Function:user name input
    #
    def slot_le_input2(self,text):

        #self.ui.lesource_2.adjustSize()
        # self.ui.lesource_2.raise_()
        # self.ui.lesource_2.show()
        sourcetext = str(text)
        self.listlogin[1] = sourcetext
        self.ui.tips_password.hide()
        if not self.ui.lesource_2.text() == '':
            self.ui.show_hide_pwd.show()
            self.ui.lesource_2_M_back.hide()

        else:
            self.ui.show_hide_pwd.hide()
            self.ui.lesource_2_M_back.show()
        if Globals.THEME_COLOR != 0:
            if self.ui.lesource_2.text() != '':
                self.ui.lesource_2.setStyleSheet(
                    "QLineEdit{background-color:transparent;border:0px;border-radius:2px;color:#ffffff;font-size:14px;}QLineEdit:pressed{color:#ffffff;}")

            else:
                self.ui.lesource_2.setStyleSheet(
                    "QLineEdit{background-color:transparent;border:0px;border-radius:2px;color:#606265;font-size:14px;}QLineEdit:pressed{color:#606265;}")




    #验证码输入
    def slot_le_input3(self,text):
        self.ui.lesource_1.adjustSize()
        sourcetext = str(text)
        self.listlogin[1] = sourcetext
        self.ui.tips_password.hide()
        if self.ui.lesource_1.text() != '':
            self.ui.lesource_1_S_back.hide()
        else:
            self.ui.lesource_1_S_back.show()
        if Globals.THEME_COLOR != 0:
            if self.ui.lesource_1.text() != '':
                self.ui.lesource_1.setStyleSheet(
                    "QLineEdit{background-color:transparent;border:0px;border-radius:2px;color:#ffffff;font-size:14px;}QLineEdit:pressed{color:#ffffff;}")

            else:
                self.ui.lesource_1.setStyleSheet(
                    "QLineEdit{background-color:transparent;border:0px;border-radius:2px;color:#606265;font-size:14px;}QLineEdit:pressed{color:#606265;}")


    #
    # 函数名:登录之前，弹出验证码界面
    # Function:login
    # 
    def slot_login(self):
        self.ui.btn_login.setEnabled(False)
        self.ui.captcha.setEnabled(False)
        if not self.ui.lesource.isHidden():
            if (self.ui.lesource.text() == "" or self.ui.lesource_2.text() == ""):
                self.next_to_login(self.picture_position)
            else:
                self.slot_get_image_code()
        else:

            ret = re.match(r"^1\d{10}$", self.ui.lesource1.text())
            if not ret:
                self.ui.btn_login.setEnabled(False)
                self.ui.tips_user_password.setText(_("The number is not correct"))
                self.timer_set()
            else:
                self.ui.btn_login.setEnabled(True)
                self.slot_get_image_code()

        # '''else:
        #     Globals.SERVER_STATE = True
        #     self.ui.tips_user_password.setText(_("request server exception"))
        #     self.ui.picture_widget.hide()
        #     self.slide_widget.picture_slider.move(self.slide_widget.picture_slider.picture_bar_slide.x(),
        #                                           self.slide_widget.picture_slider.picture_bar_slide.y())
        #     self.ui.picture_test_small.move(self.slide_widget.picture_slider.picture_bar_slide.x(),
        #                                     self.ui.picture_test_small.y())
        #
        #     self.ui.btn_login.show()
        #     self.timer_set()'''


    #
    #函数：获取图片验证显示图片
    #
    def slot_show_image(self):

        # self.ui.picture_tip.raise_()
        self.ui.register_new_account.hide()
        self.ui.picture_tip.setFocus(True)

        self.ui.picture_test.setStyleSheet(
            "QLabel{background-image:url('" + UKSC_CACHE_DIR + "/imagecode/originImage.png');}")
        self.ui.picture_test_small.setStyleSheet("QPushButton{border:0px;background-color:transparent;background-image:url('" + UKSC_CACHE_DIR + "/imagecode/image.png');}")

        self.ui.btnPassword.setEnabled(False)
        self.ui.btnShortMessage.setEnabled(False)
        self.ui.picture_widget.raise_()
        self.ui.picture_widget.show()
        self.ui.picture_test_background.show()
        self.ui.picture_test.show()

        self.ui.picture_tip.show()
        self.slide_widget.show()
        self.ui.btn_login.hide()

    # 手机短信登录
    # 函数：获取验证码之前，弹出滑动验证码
    #
    def slot_click_time(self):
        self.slot_login()



    #函数：收集到各种信息，将其返回到服务器来验证正确性
    #
    def next_to_phone_login(self):
        try:
            if not self.ui.lesource1.isHidden():
                ret = re.match(r"^1\d{10}$", self.ui.lesource1.text())
            # if self.ui.lesource1.text().isdigit() and self.ui.lesource1.text() == self.strs:
                if not ret:
                    self.ui.btn_login.setEnabled(False)
                # if not self.ui.lesource1.text() or not self.ui.lesource1.text().isdigit() or not self.ui.lesource1.text() == self.strs:
                    # self.ui.tips_user_password.setText(_("号码不正确"))
                    self.ui.tips_user_password.setText(_("The number is not correct"))
                    self.timer_set()
                elif not self.ui.lesource_1.text():
                    self.ui.btn_login.setEnabled(False)
                    # self.ui.tips_user_password.setText(_("验证码为空"))
                    self.ui.tips_user_password.setText(_("Empty captcha"))
                    self.timer_set()
                elif not self.ui.lesource_1.text().isdigit():
                    self.ui.btn_login.setEnabled(False)
                    self.ui.tips_user_password.setText(_("Error captcha"))
                    self.timer_set()
                else:
                    self.ui.btn_login.setEnabled(True)
                    # caoliang
                    login_list = []
                    login_list.append(self.ui.lesource1.text())
                    login_list.append(self.ui.lesource_1.text())
                    login_list.append(self.login_mode)
                    # login_list.append(self.account_login_values['imgId'])

                    self.ui_login.emit(login_list)
                    # self.ui.btn_login.setText(_("Logging......"))
                    self.ui.btn_login.setEnabled(False)
                    self.ui.lesource1.setEnabled(False)
                    self.ui.lesource_1.setEnabled(False)
                    # self.ui.btn_login.setStyleSheet("QPushButton{background-color:#CCCCCC;border:0px;font-size:14px;border-radius:15px;color:#ffffff}")
        except:
            return False



    # 函数：登录
    # 函数：收集到各种信息，将其返回到服务器来验证正确性
    #
    def next_to_login(self,posS):
        try:
            position = posS + 10000
            if not self.ui.lesource.isHidden():
                if not self.ui.lesource.text():
                    self.ui.tips_user_password.setText(_("please enter user name"))
                    self.timer_set()
                elif not self.ui.lesource_2.text():
                    self.ui.tips_user_password.setText(_("Please enter the user password"))
                    self.timer_set()
                else:
                # res = self.premoter.log_in_appinfo(self.listlogin[0],self.listlogin[1])

                    # caoliang
                    login_list = []
                    login_list.append(self.ui.lesource.text())
                    login_list.append(self.ui.lesource_2.text())
                    login_list.append(self.login_mode)
                    login_list.append(str(position))
                    login_list.append(self.account_login_values['imgId'])
                    # self.listlogin = ["","",""]


                    self.ui_login.emit(login_list)
                    # self.ui.btn_login.setText(_("Logging......"))
                    self.ui.btn_login.setEnabled(False)
                    self.ui.lesource.setEnabled(False)
                    self.ui.lesource_2.setEnabled(False)
                    # self.ui.btn_login.setStyleSheet("QPushButton{background-color:#CCCCCC;border:0px;font-size:14px;border-radius:15px;color:#ffffff}")
            else:
                #手机号登录
                ret = re.match(r"^1\d{10}$", self.ui.lesource1.text())
                if ret:
                    # if self.ui.captcha.isEnabled():
                    self.ui.btn_login.setEnabled(True)
                    phone_num = self.ui.lesource1.text()
                    login_list_imgid = []
                    login_list_imgid.append(self.ui.lesource1.text())
                    login_list_imgid.append(self.account_login_values['imgId'])
                    login_list_imgid.append(str(position))
                    # self.listlogin = ["", "", ""]
                    self.ui_login_phone_number.emit(login_list_imgid)
                    # self.ui_login_phone_number.emit(str(phone_num),self.self.account_login_values['imgId'],self.listlogin[2])
                    if self.phone_code_res == False:

                        self.ui.tips_user_password.setText(_("request server exception"))
                        self.ui.picture_widget.hide()
                        self.slide_widget.picture_slider.move(self.slide_widget.picture_slider.picture_bar_slide.x(),self.slide_widget.picture_slider.picture_bar_slide.y())
                        self.ui.picture_test_small.move(self.slide_widget.picture_slider.picture_bar_slide.x(),self.ui.picture_test_small.y())
                        self.ui.btnShortMessage.setEnabled(False)
                        self.ui.btnPassword.setEnabled(True)
                        self.ui.captcha.setEnabled(True)
                        self.ui.btn_login.show()
                        self.ui.btn_login.setEnabled(True)
                        self.timer_set()
                    else:
                        self.ui.captcha.setEnabled(True)
                        self.ui.btn_login.setEnabled(True)

                        if self.phone_code_res['code'] == 200:

                            self.phone_timer_picture_correct_set()
                        # if self.phone_code_res['code'] != '200':
                        # if self.image_code_res['message'] == '验证失败':
                        elif self.phone_code_res['message'] == '凭证已经过期,需重新登录':
                            self.ui.tips_user_password.setText(_("The certificate has expired. You need to log in again"))
                            self.ui.picture_widget.hide()
                            self.slide_widget.picture_slider.move(self.slide_widget.picture_slider.picture_bar_slide.x(),self.slide_widget.picture_slider.picture_bar_slide.y())
                            self.ui.picture_test_small.move(self.slide_widget.picture_slider.picture_bar_slide.x(),self.ui.picture_test_small.y())

                            self.ui.btn_login.show()
                            self.timer_set()
                        elif self.phone_code_res['message'] == '发送验证码异常，短信服务限流':
                            self.ui.tips_user_password.setText( _("Sending verification code is abnormal, SMS service is limited"))
                            self.ui.picture_widget.hide()
                            self.slide_widget.picture_slider.move(
                                self.slide_widget.picture_slider.picture_bar_slide.x(),
                                self.slide_widget.picture_slider.picture_bar_slide.y())
                            self.ui.picture_test_small.move(self.slide_widget.picture_slider.picture_bar_slide.x(),
                                                            self.ui.picture_test_small.y())

                            self.ui.btn_login.show()
                            self.timer_set()
                        else:
                            if self.phone_code_res['message'] == '滑动验证失败！':

                                self.ui.picture_tip.show()
                                self.slide_widget.picture_slider.move(0, 0)
                                self.ui.picture_test_small.move(0, 20)
                                self.timer_picture_set()
                            else:
                                self.phone_timer_picture_correct_set()
                                if self.phone_code_res['message'] == '凭证已经过期,需重新登录':
                                    self.ui.tips_user_password.setText(_("The certificate has expired. You need to log in again"))
                                    self.timer_set()
                                elif self.phone_code_res['message'] == '发送验证码异常，短信服务限流':
                                    self.ui.tips_user_password.setText( _("Sending verification code is abnormal, SMS service is limited"))
                                    self.timer_set()
                                else:
                                    self.ui.tips_user_password.setText(_(self.phone_code_res['message']))
                                    self.timer_set()
                        # else:
                        #     self.phone_timer_picture_correct_set()

                else:
                    self.ui.btn_login.setEnabled(False)
                    self.ui.tips_user_password.setText(_("The number is not correct"))
                    self.timer_set()
        except:
            return False


    #
    # 函数名:点击登录之后的操作和错误提示
    # Function:click login over
    # 
    def slot_get_ui_login_over(self,login_over_list):

        # self.listlogin = []
        try:
            self.ui.lesource.setEnabled(True)
            self.ui.lesource_2.setEnabled(True)
            self.ui.lesource1.setEnabled(True)
            self.ui.lesource_1.setEnabled(True)
            self.ui.btn_login.setEnabled(True)
            self.ui.captcha.setEnabled(True)

            # self.ui.btn_login.setEnabled(True)
            # self.ui.btn_login.setText(_("登录"))
            self.ui.btn_login.setText(_("Login"))
            self.ui.btn_login.setStyleSheet(
                "QPushButton{background-color:#3790FA;border:0px;font-size:14px;border-radius:15px;color:#ffffff}QPushButton:hover{background-color:#40A9FB;border:0px;border-radius:15px;font-size:14px;color:#ffffff}QPushButton:pressed{background-color:#296CD9;border:0px;border-radius:15px;font-size:14px;color:#ffffff}")

            if login_over_list == False:

                self.ui.tips_user_password.setText(_("request server exception"))
                if self.ui.lesource1.isHidden():

                    self.ui.picture_widget.hide()
                    self.slide_widget.picture_slider.move(self.slide_widget.picture_slider.picture_bar_slide.x(),
                                                          self.slide_widget.picture_slider.picture_bar_slide.y())
                    self.ui.picture_test_small.move(self.slide_widget.picture_slider.picture_bar_slide.x(),
                                                self.ui.picture_test_small.y())

                    self.ui.btn_login.show()
                else:
                    pass
                self.timer_set()
            else:

                code = login_over_list[0]
                tip_text = login_over_list[1]
                if code == 200:

                    Globals.LOGIN_SUCCESS = True
                    self.senior_search.hide()
                    self.search_user_widget.hide()
                    # Globals.LOGIN_ACCOUNT = login_over_list[len(login_over_list) - 1 ]
                    if not self.ui.lesource.isHidden():
                        self.timer_picture_correct_set()
                        self.ui.lesource.hide()
                        self.ui.lesource1.hide()
                        self.ui.lesource1_S_back.hide()
                        self.ui.lesource_1_S_back.hide()
                        self.ui.lesource_2_M_back.hide()
                        self.ui.lesource_1.hide()
                        self.ui.lesource_2.hide()
                        self.senior_search.hide()
                        self.search_user_widget.hide()
                        self.ui.picture_correct.show()

                        Globals.USER = self.ui.lesource.text()
                        Globals.PASSWORD = self.ui.lesource_2.text()
                        if self.ui.checkBox_5.isChecked():
                            Globals.CHEK_SAVE_PASSWORD = "1"
                        else:
                            Globals.CHEK_SAVE_PASSWORD = "0"
                        self.save_usrname_password.emit(Globals.CHEK_SAVE_PASSWORD,self.ui.lesource.text(),self.ui.lesource_2.text())
                    self.ui_login_success.emit()
                    self.listlogin = ["", "", ""]

                else:
                    if tip_text == '滑动验证码失败':

                        self.ui.picture_tip.show()
                        self.slide_widget.picture_slider.move(0,0)
                        self.ui.picture_test_small.move(0,20)
                        self.timer_picture_set()
                    else:
                        self.timer_picture_correct_set()


                        if tip_text == '用户名或密码错误':
                            self.ui.tips_user_password.setText(_("User or password error"))
                            self.timer_set()
                        elif tip_text == '用户名不存在':

                            self.ui.tips_user_password.setText(_("Username does not exist"))
                            self.timer_set()
                        elif tip_text == '手机验证码错误':
                            self.ui.tips_user_password.setText(_("Phone verification code error"))
                            self.timer_set()
                        elif tip_text == '此用户未注册':

                            self.ui.tips_user_password.setText(_("This user is not registered"))
                            self.timer_set()
                        elif tip_text == '滑动验证码已经失效':
                            self.ui.tips_user_password.setText(_("The sliding captcha is invalid"))
                            self.ui.picture_widget.hide()
                            self.timer_set()
                        elif tip_text == '请求处理失败':
                            self.ui.tips_user_password.setText(_("Request processing failed"))
                            self.timer_set()
                        elif tip_text =='手机验证码已经过期':
                            self.ui.tips_user_password.setText(_("The verification code has expired"))
                            self.timer_set()
                        elif tip_text =='系统繁忙，请稍后再试！':
                        # elif self.phone_code_res['message'] == '手机验证码已经过期':
                            self.ui.tips_user_password.setText(_("The system is busy. Please try again later"))
                            self.timer_set()
                        elif tip_text == "短信验证码错误或已过期":
                            self.ui.tips_user_password.setText(_("SMS verification code error or expired"))
                            self.timer_set()
                        elif tip_text == "用户信息错误或不存在":
                            self.ui.tips_user_password.setText(_("User information error or does not exist"))
                            self.timer_set()
                        else:
                            self.ui.tips_user_password.setText(_("Login error"))
                            self.timer_set()
        except:
            return False



        # except:
        #     pass
    #
    # 函数：手机验证码能否发送的信息提示和操作
    #
    def phone_send_SM_tips(self):


        # self.phone_code_res['message'] = '发送验证码异常'
        if self.phone_code_res['message'] == 'success':
            self.timer1.start()
            self.ui.captcha.setEnabled(False)
        elif self.phone_code_res['message'] == '发送验证码异常':
            self.ui.tips_user_password.setText(_("Abnormal sending verification code"))
            self.timer_set()
        elif self.phone_code_res['message'] == '该手机号当日接收短信次数达到上限':
            self.ui.tips_user_password.setText(_("The SMS received by this number reached the upper today"))
            self.timer_set()
        elif self.phone_code_res['message'] == '手机号码其他错误':
            self.ui.tips_user_password.setText(_("Other errors in mobile phone number"))
            self.timer_set()
        else:
            self.ui.tips_user_password.setText(_("Send error"))
            self.timer_set()
        self.ui.lesource.setEnabled(True)
        self.ui.lesource_2.setEnabled(True)
        self.ui.lesource1.setEnabled(True)
        self.ui.lesource_1.setEnabled(True)

    # 函数名:
    # Function:oprate password
    #
    def operate(self):

        self.ui.tips_user_password.hide()
        self.timer.stop()

    #
    # 函数名:设置延时
    # Function:set timer
    #
    def timer_set(self):
        self.ui.btn_login.setEnabled(True)
        self.ui.captcha.setEnabled(True)
        # self.ui.btnPassword.setEnabled(True)
        # self.ui.btnShortMessage.setEnabled(True)
        if self.ui.btn_login.isHidden():
            pass
        else:
            self.ui.register_new_account.show()
        self.ui.tips_user_password.show()
        self.timer.timeout.connect(self.operate)  # 计时结束调用operate()方法
        self.timer.start(3500)  # 设置计时间隔并启动


    # 函数名:图片验证码错误提示
    # Function:oprate password
    #
    def picture_operate(self):

        self.ui.picture_tip.setText(_("Drag the slider to complete the validation"))
        self.ui.picture_tip.setStyleSheet("QLabel{background-color:transparent;font-size:12px;border:none;color:#C0C4CC;}")
        self.timer_picture.stop()


    #
    # 函数名:设置延时
    # Function:set timer
    #
    def timer_picture_set(self):
        self.ui.btn_login.setEnabled(True)
        self.ui.captcha.setEnabled(True)
        # self.ui.btnPassword.setEnabled(True)
        # self.ui.btnShortMessage.setEnabled(True)
        self.ui.picture_tip.show()
        self.ui.picture_tip.setText(_("Verification failed. Please try again"))
        self.ui.picture_tip.setStyleSheet("QLabel{background-color:transparent;font-size:12px;border:none;color:#F56C6C;}")
        self.timer_picture.timeout.connect(self.picture_operate)  # 计时结束调用operate()方法
        self.timer_picture.start(1500)  # 设置计时间隔并启动

    # 函数名:图片验证码正确提示
    # Function:oprate password
    #
    def picture_correct_operate(self):
        self.senior_search.hide()
        self.search_user_widget.hide()
        self.ui.picture_correct.hide()
        self.ui.picture_widget.hide()
        self.ui.picture_widget_cover.hide()
        self.ui.register_new_account.show()
        self.slide_widget.picture_slider.setAttribute(Qt.WA_TransparentForMouseEvents, False)
        self.slide_widget.picture_slider.move(self.slide_widget.picture_slider.picture_bar_slide.x(),self.slide_widget.picture_slider.picture_bar_slide.y())
        self.ui.picture_test_small.move(self.slide_widget.picture_slider.picture_bar_slide.x(),self.ui.picture_test_small.y())

        self.ui.btn_login.show()
        self.ui.btnPassword.setEnabled(True)
        self.ui.btnShortMessage.setEnabled(True)

        self.timer_picture_correct.stop()

    #
    # 函数名:设置延时
    # Function:set timer
    #
    def timer_picture_correct_set(self):
        self.senior_search.hide()
        self.search_user_widget.hide()
        self.ui.btn_login.setEnabled(True)
        self.ui.captcha.setEnabled(True)
        self.ui.register_new_account.hide()
        self.ui.picture_correct.show()
        self.ui.picture_widget_cover.raise_()
        self.ui.picture_widget_cover.show()
        self.ui.picture_correct.raise_()
        self.slide_widget.picture_slider.setAttribute(Qt.WA_TransparentForMouseEvents,True)
        self.timer_picture_correct.start(2000)  # 设置计时间隔并启动
        self.timer_picture_correct.timeout.connect(self.picture_correct_operate)  # 计时结束调用operate()方法

    # 函数名:手机号登录时图片验证码正确提示
    # Function:oprate password
    #
    def phone_picture_correct_operate(self):
        # self.ui.btnPassword.setEnabled(True)
        # self.ui.btnShortMessage.setEnabled(True)
        self.senior_search.hide()
        self.search_user_widget.hide()
        self.ui.picture_correct.hide()
        self.ui.picture_widget.hide()
        self.ui.picture_widget_cover.hide()
        self.slide_widget.picture_slider.setAttribute(Qt.WA_TransparentForMouseEvents, False)
        self.slide_widget.picture_slider.move(self.slide_widget.picture_slider.picture_bar_slide.x(),self.slide_widget.picture_slider.picture_bar_slide.y())
        self.ui.picture_test_small.move(self.slide_widget.picture_slider.picture_bar_slide.x(),self.ui.picture_test_small.y())
        self.ui.register_new_account.show()
        self.ui.btn_login.show()
        self.ui.btnPassword.setEnabled(True)
        self.ui.btnShortMessage.setEnabled(False)
        self.phone_send_SM_tips()
        # self.ui.captcha.setEnabled(False)
        # self.slot_time_Refresh()
        # self.timer1.start()


        self.phone_timer_picture_correct.stop()

    #
    # 函数名:设置延时
    # Function:set timer
    #
    def phone_timer_picture_correct_set(self):
        # self.ui.btnPassword.setEnabled(True)
        # self.ui.btnShortMessage.setEnabled(True)
        self.senior_search.hide()
        self.search_user_widget.hide()
        self.ui.btn_login.setEnabled(True)
        self.ui.captcha.setEnabled(True)
        self.ui.register_new_account.hide()
        self.ui.picture_correct.show()
        self.ui.picture_widget_cover.raise_()
        self.ui.picture_widget_cover.show()
        self.ui.picture_correct.raise_()
        self.slide_widget.picture_slider.setAttribute(Qt.WA_TransparentForMouseEvents, True)
        # self.ui.picture_test.setStyleSheet("QLabel{background-image:url('res/originImage.png');opsity:0.1;}")
        self.phone_timer_picture_correct.start(2000)  # 设置计时间隔并启动
        self.phone_timer_picture_correct.timeout.connect(self.phone_picture_correct_operate)  # 计时结束调用operate()方法
    # 函数名:设置延时
    # Function:set timer
    #
    def slot_ui_login_image_code_move(self, position):
        self.slide_widget.picture_slider.move(position,0)
        self.ui.picture_test_small.move(position, self.ui.picture_test_small.y())

    def slot_get_image_code(self):
        self.ui_login_get_image_code.emit({'form': 1})

    def eventFilter(self, object, event):
        # self.check_foucs_state()
        if object == self.ui.lesource:
            if(event.type() == event.MouseButtonPress ):
                self.pre_num = 0
                self.senior_search.hide()
            if self.ui.lesource.hasFocus():
                self.ui.show_search.show()
                self.ui.Login_user_box.hide()
                self.ui.Login_user_box.setStyleSheet("QWidget{border:1px solid #3790FA}")
                self.ui.Login_user_box.show()
                if (event.type() == event.KeyRelease):
                    if self.ui.lesource.text() == '':
                        if (Qt.Key_A <= event.key() <= Qt.Key_Z or Qt.Key_0 <= event.key() <= Qt.Key_9):
                            if (event.key() == Qt.Key_X):
                                if QApplication.keyboardModifiers() == Qt.ControlModifier:
                                    self.ui.lesource_M_back.show()
                            elif (event.key() == Qt.Key_C):
                                if QApplication.keyboardModifiers() == Qt.ControlModifier:
                                    self.ui.lesource_M_back.show()
                            elif (event.key() == Qt.Key_Z):
                                if QApplication.keyboardModifiers() == Qt.ControlModifier:
                                    self.ui.lesource_M_back.show()
                            else:
                                self.ui.lesource_M_back.hide()
                                self.pre_num += 1
                        elif (Qt.Key_Backspace == event.key()):
                            if self.ui.lesource.text() == '':
                                if (self.pre_num != 0):
                                    self.pre_num -= 1
                                else:
                                    self.ui.lesource_M_back.show()
                                if (self.pre_num == 0):
                                    self.ui.lesource_M_back.show()
                        else:
                            self.ui.lesource_M_back.show()

                    else:
                        self.pre_num = 0
                    if (Qt.Key_Return == event.key() or Qt.Key_Enter == event.key() or Qt.Key_Space == event.key()):
                        if self.ui.lesource.text() == '':
                            self.ui.lesource_M_back.show()
                        else:
                            self.pre_num = 0

                if Globals.THEME_COLOR == 0:
                    self.ui.Login_user_box.setStyleSheet("QWidget{border:1px solid #EBEEF5}")
                else:
                    self.ui.Login_user_box.setStyleSheet("QWidget{border:1px solid #48484C}")

        if object == self.ui.lesource_M_back:
            if(event.type() == event.MouseButtonPress ):
                self.ui.lesource.setFocus(True)
        if object == self.ui.lesource1_S_back:
            if(event.type() == event.MouseButtonPress ):
                self.ui.lesource1.setFocus(True)
                self.pre_num = 0
        if object == self.ui.lesource_2_M_back:
            if(event.type() == event.MouseButtonPress ):
                self.ui.lesource_2.setFocus(True)
                if (event.type() == event.KeyPress):
                    if (event.matches(QKeySequence.SelectAll) or event.matches(QKeySequence.Copy) or event.matches(
                            QKeySequence.Paste)):
                        return
        if(object == self.ui.lesource_2):

            if(QLineEdit.Normal == self.ui.lesource_2.echoMode()):
                if (event.type() == event.ShortcutOverride):
                    if (self.ui.lesource_2.text() == ''):
                        self.ui.lesource_2_M_back.show()
                        self.pre_num = 0
                        self.shortcut = self.shortcut + 1
                if (event.type() == event.KeyRelease):
                    if (self.shortcut == 0):
                        if self.ui.lesource_2.text() == '':
                            if (Qt.Key_A <= event.key() <= Qt.Key_Z or Qt.Key_0 <= event.key() <= Qt.Key_9):
                                if (event.key() == Qt.Key_X):
                                    if QApplication.keyboardModifiers() == Qt.ControlModifier:
                                        self.ui.lesource_2_M_back.show()
                                elif (event.key() == Qt.Key_C):
                                    if QApplication.keyboardModifiers() == Qt.ControlModifier:
                                        self.ui.lesource_2_M_back.show()
                                elif (event.key() == Qt.Key_Z):
                                    if QApplication.keyboardModifiers() == Qt.ControlModifier:
                                        self.ui.lesource_2_M_back.show()
                                else:
                                    self.ui.lesource_2_M_back.hide()
                                    self.pre_num += 1
                            elif (Qt.Key_Backspace == event.key()):
                                if self.ui.lesource_2.text() == '':
                                    if (self.pre_num != 0):
                                        self.pre_num -= 1
                                    else:
                                        self.ui.lesource_2_M_back.show()
                                    if (self.pre_num == 0):
                                        self.ui.lesource_2_M_back.show()
                        else:
                            self.pre_num = 0
                        if (Qt.Key_Return == event.key() or Qt.Key_Enter == event.key() or Qt.Key_Space == event.key()):
                            if self.ui.lesource_2.text() == '':
                                self.ui.lesource_2_M_back.show()
                            else:
                                self.pre_num = 0
                    else:
                        self.shortcut = self.shortcut - 1
            if(event.type() == event.KeyPress):
                if (event.key() == Qt.Key_C):
                    if QApplication.keyboardModifiers() == Qt.ControlModifier:
                        return True
                if (event.key() == Qt.Key_V):
                    if QApplication.keyboardModifiers() == Qt.ControlModifier:
                        return True
                if (event.key() == Qt.Key_X):
                    if QApplication.keyboardModifiers() == Qt.ControlModifier:
                        return True
        if object == self.ui.lesource_1_S_back:
            if(event.type() == event.MouseButtonPress ):
                self.pre_num = 0
                self.shortcut = 0
                self.ui.lesource_1.setFocus(True)
        if object == self.ui.lesource1:
            if(event.type() == event.ShortcutOverride):
                if(self.ui.lesource1.text() == ''):
                    self.ui.lesource1_S_back.show()
                    self.pre_num = 0
                    self.shortcut = self.shortcut + 1
            if (event.type() == event.KeyRelease):
                if(self.shortcut == 0):
                    if self.ui.lesource1.text() == '':
                        if (Qt.Key_A <= event.key() <= Qt.Key_Z or Qt.Key_0 <= event.key() <= Qt.Key_9):
                            if (event.key() == Qt.Key_X):
                                if QApplication.keyboardModifiers() == Qt.ControlModifier:
                                    self.ui.lesource1_S_back.show()
                            elif (event.key() == Qt.Key_C):
                                if QApplication.keyboardModifiers() == Qt.ControlModifier:
                                    self.ui.lesource1_S_back.show()
                            elif (event.key() == Qt.Key_Z):
                                if QApplication.keyboardModifiers() == Qt.ControlModifier:
                                    self.ui.lesource1_S_back.show()
                            else:
                                self.ui.lesource1_S_back.hide()
                                self.pre_num += 1
                        elif (Qt.Key_Backspace == event.key()):
                            if self.ui.lesource1.text() == '':
                                if (self.pre_num != 0):
                                    self.pre_num -= 1
                                else:
                                    self.ui.lesource1_S_back.show()
                                if (self.pre_num == 0):
                                    self.ui.lesource1_S_back.show()
                    else:
                        self.pre_num = 0
                    if (Qt.Key_Return == event.key() or Qt.Key_Enter == event.key() or Qt.Key_Space == event.key()):
                        if self.ui.lesource1.text() == '':
                            self.ui.lesource1_S_back.show()
                        else:
                            self.pre_num = 0
                else:
                    self.shortcut = self.shortcut - 1
            pass

        if(object == self.ui.lesource_1):
            if (event.type() == event.ShortcutOverride):
                if (self.ui.lesource_1.text() == ''):
                    self.ui.lesource_1_S_back.show()
                    self.pre_num = 0
                    self.shortcut = self.shortcut + 1
            if (event.type() == event.KeyRelease):
                if (self.shortcut == 0):
                    if self.ui.lesource_1.text() == '':
                        if (Qt.Key_A <= event.key() <= Qt.Key_Z or Qt.Key_0 <= event.key() <= Qt.Key_9):
                            if (event.key() == Qt.Key_X):
                                if QApplication.keyboardModifiers() == Qt.ControlModifier:
                                    self.ui.lesource_1_S_back.show()
                            elif (event.key() == Qt.Key_C):
                                if QApplication.keyboardModifiers() == Qt.ControlModifier:
                                    self.ui.lesource_1_S_back.show()
                            elif (event.key() == Qt.Key_Z):
                                if QApplication.keyboardModifiers() == Qt.ControlModifier:
                                    self.ui.lesource_1_S_back.show()
                            else:
                                self.ui.lesource_1_S_back.hide()
                                self.pre_num += 1
                        elif (Qt.Key_Backspace == event.key()):
                            if self.ui.lesource_1.text() == '':
                                if (self.pre_num != 0):
                                    self.pre_num -= 1
                                else:
                                    self.ui.lesource_1_S_back.show()
                                if (self.pre_num == 0):
                                    self.ui.lesource_1_S_back.show()
                    else:
                        self.pre_num = 0
                    if (Qt.Key_Return == event.key() or Qt.Key_Enter == event.key() or Qt.Key_Space == event.key()):
                        if self.ui.lesource_1.text() == '':
                            self.ui.lesource_1_S_back.show()
                        else:
                            self.pre_num = 0
                else:
                    self.shortcut = self.shortcut - 1
            pass
        if self.ui.lesource_2.hasFocus():
            self.ui.Login_pwd_box.hide()
            self.ui.Login_pwd_box.setStyleSheet("QWidget{border:1px solid #3790FA}")
            self.ui.Login_pwd_box.show()
        else:
            if Globals.THEME_COLOR == 0:
                self.ui.Login_pwd_box.setStyleSheet("QWidget{border:1px solid #EBEEF5}")
            else:
                self.ui.Login_pwd_box.setStyleSheet("QWidget{border:1px solid #48484C}")

        if self.ui.lesource1.hasFocus():
            self.ui.Login_phone_box.setStyleSheet("QWidget{border:1px solid #3790FA}")
        else:
            if Globals.THEME_COLOR == 0:
                self.ui.Login_phone_box.setStyleSheet("QWidget{border:1px solid #EBEEF5}")
            else:
                self.ui.Login_phone_box.setStyleSheet("QWidget{border:1px solid #48484C}")

        if self.ui.lesource_1.hasFocus():
            self.ui.Login_sm_box.setStyleSheet("QWidget{border:1px solid #3790FA;}")
        else:
            if Globals.THEME_COLOR == 0:
                self.ui.Login_sm_box.setStyleSheet("QWidget{border:1px solid #EBEEF5}")
            else:
                self.ui.Login_sm_box.setStyleSheet("QWidget{border:1px solid #48484C}")
        return False

    def keyPressEvent(self, event):
        if self.ui.btn_login.isEnabled():
            if str(event.key()) == "16777220":
                self.choose_login_way()
        else:
            pass


    # def enterEvent(self, event):

        # self.eventFilter1(self)



    #
    # 函数名:清空用户密码
    # Function:clean user password
    #
    # def clean_user_password(self):
    #     if self.ui.checkBox_5.isChecked():
    #         password_write("1", "0", Globals.USER, Globals.PASSWORD)
    #         pass
    #     else:
    #         password_write("0", "0", Globals.USER, Globals.PASSWORD)
    #         self.ui.lesource_2.setText(None)

# 函数名:主函数
# Function:main
# 
def main():
    import sys
    app = QApplication(sys.argv)
   #QTextCodec.setCodecForTr(QTextCodec.codecForName("UTF-8"))
   #QTextCodec.setCodecForCStrings(QTextCodec.codecForName("UTF-8"))

    globalfont = QFont()
    globalfont.setFamily("PingFangSC-Regular")
    app.setFont(globalfont)
    # a = Login()
    # a.show()

    sys.exit(app.exec_())

if __name__ == "__main__":
    main()

class SlideBar(QWidget):

    def __init__(self,  parent=None):
        QWidget.__init__(self, parent)
        self.resize(320,26)
        # self.setStyleSheet("QWidget{background:transparent;}")

        self.picture_bar = QWidget(self)
        self.picture_bar.resize(320,10)
        self.picture_bar.move(0,7)

        self.picture_slider = Slide(self)
        self.picture_slider.resize(48,26)
        self.picture_slider.move(0,0)
        self.slot_dark_mode()

    def slot_dark_mode(self):
        if Globals.THEME_COLOR == 0:
            self.picture_bar.setStyleSheet("QWidget{border:0px ;background-color:#EBEEED;border-radius:5px;}")
        else:
            self.picture_bar.setStyleSheet("QWidget{border:0px ;background-color:rgba(255,255,255,0.15);border-radius:5px;}")

class Slide(QWidget,Signals):

    def __init__(self,  parent=None):
        QWidget.__init__(self, parent)
        # self.resize(320,48)
        self.slidebar = parent
        self.setStyleSheet("QWidget{background:transparent;}")

        self.picture_bar_slide = QWidget(self)
        self.picture_bar_slide.resize(48,26)
        self.picture_bar_slide.move(0, 0)

        self.picture_bar_slide.setStyleSheet("QWidget{background-image:url('./res/slide.png');}")
        self.picture_bar_slide.setFocusPolicy(Qt.NoFocus)

  #
    #函数名: 鼠标点击事件
    #Function: Mouse click event
    #
    def mousePressEvent(self, event):

        if(event.button() == Qt.LeftButton):
            self.clickx = event.globalPos().x()
            self.clicky = event.globalPos().y()
            self.dragPosition = event.globalPos() - self.pos()

            event.accept()
    #
    #函数名: 窗口拖动事件
    #Function: Window drag event
    #
    def mouseMoveEvent(self, event):

        if(event.buttons() == Qt.LeftButton):

            if self.dragPosition != -1:

                self.move(event.globalPos() - self.dragPosition)
                # self.ui_login_image_code_move.emit(event.globalPos() - self.dragPosition)
                event.accept()

            if self.mapToParent(self.rect().topLeft()).x() <= 0:
                # self.move(0,self.pos().y())
                self.ui_login_image_code_move.emit(0)
            if self.mapToParent(self.rect().bottomRight()).x() >= self.parentWidget().rect().width():
                # self.move(self.parentWidget().rect().width() - self.width(), self.pos().y())
                self.ui_login_image_code_move.emit(self.parentWidget().rect().width() - self.width())
            if self.mapToParent(self.rect().topLeft()).y() <= 0:
                # self.move(self.pos().x(),0)
                self.ui_login_image_code_move.emit(self.pos().x())
            if self.mapToParent(self.rect().bottomRight()).y() >= self.parentWidget().rect().height():
                # self.move(self.pos().x(),self.parentWidget().rect().height() - self.height())
                self.ui_login_image_code_move.emit(self.pos().x())

        else:
            pass




#函数名: 松开鼠标按键事件
    #Function: Mouse button release event
    #
    def mouseReleaseEvent(self, event):
        if(self.pos().x() > 0):
            self.ui_login_mouse_current_position.emit(self.pos().x())
