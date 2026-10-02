# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'login_ui.ui'
#
# Created by: PyQt5 UI code generator 4.11.4
#
# WARNING! All changes made in this file will be lost!

from PyQt5 import QtCore, QtGui
from PyQt5.QtGui import *
from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
from models.globals import *
import math
try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    def _fromUtf8(s):
        return s

try:
    _encoding = QApplication.UnicodeUTF8
    def _translate(context, text, disambig):
        return QApplication.translate(context, text, disambig, _encoding)
except AttributeError:
    def _translate(context, text, disambig):
        return QApplication.translate(context, text, disambig)

class Cutaway(QPushButton):
    ui_login_image_code_move = pyqtSignal(int)
    ui_login_mouse_current_position = pyqtSignal(int)
    def __init__(self,parent):
        QPushButton.__init__(self, parent)
        self.leave_btn = True
        self.m_parent = parent
        self.setFocusPolicy(Qt.NoFocus)
        pass

    #
    # 函数：实现无标题栏窗口拖动
    #
    def mousePressEvent(self, event):
        if (event.button() == Qt.LeftButton):
            self.mouse_press = True
            if(self.leave_btn == False):
                try:
                    self.drag_pos = event.globalPos() - self.pos()
                except:
                    pass
            event.accept()


    def mouseReleaseEvent(self, event):
        self.mouse_press = False
        if(hasattr(self,"pos")):
            if(hasattr(self.pos,"x") and self.pos.x() > 0):
                self.ui_login_mouse_current_position.emit(self.pos.x())
        self.setWindowOpacity(1)

    def mouseMoveEvent(self, event):
        if (self.mouse_press):
            self.move_pos = event.globalPos()
            self.pos = self.move_pos - self.drag_pos
            # self.move(self.move_pos - self.drag_pos)
            if(self.pos.x() <=0 ):
                pass
            elif(self.pos.x() > self.m_parent.width()-self.width()):
                pass
            else:
                self.ui_login_image_code_move.emit(self.pos.x())
            self.setWindowOpacity(1)
            event.accept()

    def enterEvent(self, event):
        self.leave_btn = False

    def leaveEvent(self, event):
        self.leave_btn = True



class Ui_Login_ui(object):
    def setupUi(self, Login_ui):
        Login_ui.setObjectName(_fromUtf8("Login_ui"))
        Login_ui.resize(484, 404)
        self.centralwidget = QFrame(Login_ui)
        self.centralwidget.setObjectName(_fromUtf8("centralwidget"))
        self.centralwidget.setAttribute(QtCore.Qt.WA_TranslucentBackground)
        self.centralwidget.setGeometry(QtCore.QRect(10, 10, 464, 394))
        self.sourceWidget = QWidget(self.centralwidget)
        self.sourceWidget.setGeometry(QtCore.QRect(10, 10, 444, 374))
        self.sourceWidget.setObjectName(_fromUtf8("sourceWidget"))
        if(Globals.RIGHT_ANGLE):
            self.sourceWidget.setStyleSheet(".QWidget#sourceWidget{border:0px solid #FFFFFF;background-color:#ffffff;}")
        else:
            self.sourceWidget.setStyleSheet(".QWidget#sourceWidget{border:0px solid #FFFFFF;background-color:#ffffff;border-radius:4px;}")

        #其余异常提醒，
        self.tips_user_password = QLabel(self.sourceWidget)
        self.tips_user_password.setAttribute(Qt.WA_TranslucentBackground)
        self.tips_user_password.setGeometry(QtCore.QRect(0, 62, 444, 16))
        self.tips_user_password.setObjectName(_fromUtf8("tips_user_password"))
        self.tips_user_password.hide()

        #应用商店登录中心
        self.soft_linedit = QLabel(self.sourceWidget)
        self.soft_linedit.setAttribute(Qt.WA_TranslucentBackground)
        self.soft_linedit.setGeometry(QtCore.QRect(0, 30, 444, 22))
        self.soft_linedit.setObjectName(_fromUtf8("soft_linedit"))
        #图片验证码登录
        self.picture_widget = QWidget(self.sourceWidget)
        # self.picture_widget.setAttribute(Qt.WA_TranslucentBackground)
        self.picture_widget.setGeometry(QtCore.QRect(62, 132, 320, 212))
        self.picture_widget.setObjectName(_fromUtf8("soft_linedit"))

        # 图片验证码登录的遮盖效果
        self.picture_widget_cover = QWidget(self.picture_widget)
        self.picture_widget_cover.setAttribute(Qt.WA_StyledBackground)
        # self.picture_widget_cover.setWindowFlags(QtCore.Qt.FramelessWindowHint)
        # self.picture_widget_cover.setWindowOpacity(0.5)
        self.picture_widget_cover.setGeometry(QtCore.QRect(0, 20, 320, 160))
        self.picture_widget_cover.setObjectName(_fromUtf8("soft_linedit"))

        self.picture_tip = QLabel(self.picture_widget)
        self.picture_tip.setAttribute(Qt.WA_TranslucentBackground)
        self.picture_tip.setGeometry(QtCore.QRect(0, 0, 320, 16))
        self.picture_tip.setObjectName(_fromUtf8("soft_linedit"))

        self.picture_test_background = QLabel(self.picture_widget)
        # self.picture_test.setAttribute(Qt.WA_TranslucentBackground)
        self.picture_test_background.setGeometry(QtCore.QRect(0, 20, 320, 160))
        self.picture_test_background.setObjectName(_fromUtf8("picture_test_background"))

        self.picture_test = QLabel(self.picture_test_background)
        # self.picture_test.setAttribute(Qt.WA_TranslucentBackground)
        self.picture_test.setGeometry(QtCore.QRect(0, 0, 320, 160))
        self.picture_test.setObjectName(_fromUtf8("picture_test"))



        # self.picture_widget_corrrect = QWidget(self.picture_widget)
        # self.picture_widget_corrrect.setAttribute(Qt.WA_TranslucentBackground)
        # self.picture_widget_corrrect.setGeometry(QtCore.QRect(62, 113, 320, 212))
        # self.picture_widget_corrrect.setObjectName(_fromUtf8("soft_linedit"))

        self.picture_test_small = Cutaway(self.picture_widget)
        # self.picture_test.setAttribute(Qt.WA_TranslucentBackground)
        self.picture_test_small.setGeometry(QtCore.QRect(0, 20, 50, 160))
        self.picture_test_small.setObjectName(_fromUtf8("picture_test"))

        # self.picture_back = QLabel(self.picture_widget)
        # # self.picture_test.setAttribute(Qt.WA_TranslucentBackground)
        # self.picture_back.setGeometry(QtCore.QRect(0, 20, 320, 160))
        # self.picture_back.setObjectName(_fromUtf8("picture_test"))

        self.picture_correct = QLabel(self.picture_widget)
        self.picture_correct.setGeometry(QtCore.QRect(125, 65, 70, 70))
        # self.picture_correct.setAttribute(Qt.WA_TranslucentBackground)

        self.picture_bar = QWidget(self.picture_widget)
        self.picture_bar.setGeometry(QtCore.QRect(0,194,320,10))
        self.picture_test.setObjectName(_fromUtf8("picture_bar"))
        #
        # self.picture_bar_slide = QPushButton(self.picture_widget)
        # self.picture_bar_slide.setGeometry(QtCore.QRect(0, 185, 48, 26))
        # self.picture_test.setObjectName(_fromUtf8("picture_bar_slide"))
        # 用户名错误时显示
        self.tips_user=QLabel(self.sourceWidget)
        self.tips_user.setAttribute(Qt.WA_TranslucentBackground)
        self.tips_user.setGeometry(QtCore.QRect(278, 141, 84, 16))
        self.tips_user.setObjectName(_fromUtf8("tips_user"))
        self.tips_user.hide()
        # 密码错误
        self.tips_password = QLabel(self.sourceWidget)
        self.tips_password.setAttribute(Qt.WA_TranslucentBackground)
        self.tips_password.setGeometry(QtCore.QRect(315, 192, 48, 16))
        self.tips_password.setObjectName(_fromUtf8("tips_password"))
        self.tips_password.hide()

        #手机号错误提示
        self.tips_num=QLabel(self.sourceWidget)
        self.tips_num.setAttribute(Qt.WA_TranslucentBackground)
        self.tips_num.setGeometry(QtCore.QRect(315, 141, 48, 16))
        self.tips_num.setObjectName(_fromUtf8("tips_user"))
        self.tips_num.hide()

        # 密码登录
        self.btnPassword = QPushButton(self.sourceWidget)
        self.btnPassword.setAttribute(Qt.WA_TranslucentBackground)
        self.btnPassword.setGeometry(QtCore.QRect(132, 86, 80, 24))
        self.btnPassword.setText(_fromUtf8(""))

        # 短信登录
        self.btnShortMessage = QPushButton(self.sourceWidget)
        self.btnShortMessage.setAttribute(Qt.WA_TranslucentBackground)
        self.btnShortMessage.setGeometry(QtCore.QRect(232, 86, 80, 24))
        self.btnShortMessage.setText(_fromUtf8(""))


        # 登录用户框
        self.Login_user_box=QWidget(self.sourceWidget)
        self.Login_user_box.setGeometry(QtCore.QRect(82, 177, 280, 1))
        self.Login_user_box.setObjectName(_fromUtf8("Login_user_box"))

        # 手机号框
        self.Login_phone_box=QWidget(self.sourceWidget)
        self.Login_phone_box.setGeometry(QtCore.QRect(82, 177, 280, 1))
        self.Login_phone_box.setObjectName(_fromUtf8("Login_phone_box"))

        # 登录密码框
        self.Login_pwd_box = QWidget(self.sourceWidget)
        self.Login_pwd_box.setGeometry(QtCore.QRect(82, 228, 280, 1))
        self.Login_pwd_box.setObjectName(_fromUtf8("Login_pwd_box"))

        # 短信输入框
        self.Login_sm_box = QWidget(self.sourceWidget)
        self.Login_sm_box.setGeometry(QtCore.QRect(82, 228, 280, 1))
        self.Login_sm_box.setObjectName(_fromUtf8("Login_sm_box"))

        # 密码登录界面的用户名框文字
        self.lesource = QLineEdit(self.sourceWidget)
        self.lesource.setFocus(True)
       # self.lesource.setAttribute(Qt.WA_Hover,True)
        self.lesource.setAttribute(Qt.WA_TranslucentBackground)
        self.lesource.setGeometry(QtCore.QRect(109, 150, 200, 20))
        self.lesource.setObjectName(_fromUtf8("lesource"))

        # 密码登录界面的用户框背景文字
        self.lesource_M_back = QLabel(self.sourceWidget)
        self.lesource_M_back.setFocus(True)
        # self.lesource.setAttribute(Qt.WA_Hover,True)
        self.lesource_M_back.setAttribute(Qt.WA_TranslucentBackground)
        self.lesource_M_back.setGeometry(QtCore.QRect(112, 150, 200, 18))
        self.lesource_M_back.setObjectName(_fromUtf8("lesource_M_back"))

        # 显示或者隐藏密码的父窗口，方便处理
        self.show_hide_pwd = QPushButton(self.sourceWidget)
        self.show_hide_pwd.setAttribute(Qt.WA_TranslucentBackground)
        self.show_hide_pwd.setGeometry(QtCore.QRect(340, 203, 16, 16))
        self.show_hide_pwd.setObjectName(_fromUtf8("show or hide pwd"))
        # 显示密码按钮
        self.show_pwd = QPushButton(self.show_hide_pwd)
        self.show_pwd.setAttribute(Qt.WA_TranslucentBackground)
        self.show_pwd.setGeometry(QtCore.QRect(0, 0, 16, 16))
        self.show_pwd.setObjectName(_fromUtf8("show pwd"))

        # 隐藏密码按钮
        self.hide_pwd = QPushButton(self.show_hide_pwd)
        self.hide_pwd.setAttribute(Qt.WA_TranslucentBackground)
        self.hide_pwd.setGeometry(QtCore.QRect(0, 0, 16, 16))
        self.hide_pwd.setObjectName(_fromUtf8("hide pwd"))

        # 用户下拉框
        self.show_search = QPushButton(self.sourceWidget)
        self.show_search.setAttribute(Qt.WA_TranslucentBackground)
        self.show_search.setGeometry(QtCore.QRect(340, 152, 16, 16))
        self.show_search.setObjectName(_fromUtf8("show search"))



        #短信登录界面的电话号码框文字
        self.lesource1 = QLineEdit(self.sourceWidget)
       # self.lesource1.setAttribute(Qt.WA_Hover, True)
        self.lesource1.setAttribute(Qt.WA_TranslucentBackground)
        self.lesource1.setGeometry(QtCore.QRect(109, 150, 264, 20))
        self.lesource1.setObjectName(_fromUtf8("lesource1"))
        self.lesource1.hide()

        self.lesource1_S_back = QLabel(self.sourceWidget)
        # self.lesource1.setAttribute(Qt.WA_Hover, True)
        self.lesource1_S_back.setAttribute(Qt.WA_TranslucentBackground)
        self.lesource1_S_back.setGeometry(QtCore.QRect(112, 150, 200, 18))
        self.lesource1_S_back.setObjectName(_fromUtf8("lesource1_S_back"))
        self.lesource1_S_back.hide()

        #用户框或者手机号框的图标
        self.usr_icon=QWidget(self.sourceWidget)
        self.usr_icon.setGeometry(QtCore.QRect(87, 152, 16, 16))
        self.usr_icon.setObjectName(_fromUtf8("usr_icon"))

        #密码登录界面的密码框背景文字
        self.lesource_2 = QLineEdit(self.sourceWidget)
        #self.lesource_2.setAttribute(Qt.WA_Hover, True)
        self.lesource_2.setAttribute(Qt.WA_TranslucentBackground)
        self.lesource_2.setGeometry(QtCore.QRect(109,201, 237, 20))
        self.lesource_2.setObjectName(_fromUtf8("lesource_2"))

        self.lesource_2_M_back = QLabel(self.sourceWidget)
        # self.lesource_2.setAttribute(Qt.WA_Hover, True)
        self.lesource_2_M_back.setAttribute(Qt.WA_TranslucentBackground)
        self.lesource_2_M_back.setGeometry(QtCore.QRect(112, 201, 200, 18))
        self.lesource_2_M_back.setObjectName(_fromUtf8("lesource_2_M_back"))

        #短信登录界面的验证码框背景文字
        self.lesource_1 = QLineEdit(self.sourceWidget)
        #self.lesource_1.setAttribute(Qt.WA_Hover, True)
        self.lesource_1.setAttribute(Qt.WA_TranslucentBackground)
        self.lesource_1.setGeometry(QtCore.QRect(109,201, 264, 20))
        self.lesource_1.setObjectName(_fromUtf8("lesource_1"))
        self.lesource_1.hide()

        self.lesource_1_S_back = QLabel(self.sourceWidget)
        # self.lesource_1.setAttribute(Qt.WA_Hover, True)
        self.lesource_1_S_back.setAttribute(Qt.WA_TranslucentBackground)
        self.lesource_1_S_back.setGeometry(QtCore.QRect(112, 201, 264, 18))
        self.lesource_1_S_back.setObjectName(_fromUtf8("lesource_1_S_back"))
        self.lesource_1_S_back.hide()

        #密码框或者验证码框的图标
        self.password_icon = QWidget(self.sourceWidget)
        self.password_icon.setGeometry(QtCore.QRect(87, 203, 16, 16))
        self.password_icon.setObjectName(_fromUtf8("password_icon"))
        #验证码分隔线
        self.separator_line = QWidget(self.sourceWidget)
        self.separator_line.setGeometry(QtCore.QRect(294, 204, 1, 14))
        self.separator_line.setObjectName(_fromUtf8("separator_line"))
        self.separator_line.hide()
        #获取验证码
        self.captcha = QPushButton(self.sourceWidget)
        self.captcha.setAttribute(Qt.WA_TranslucentBackground)
        self.captcha.setGeometry(QtCore.QRect(303,203, 64, 16))
        self.captcha.setObjectName(_fromUtf8("captcha"))
        self.captcha.hide()

        #记住密码
        self.rmb_pwd = QPushButton(self.sourceWidget)
        self.rmb_pwd.setAttribute(Qt.WA_TranslucentBackground)
        self.rmb_pwd.setGeometry(QtCore.QRect(109, 240, 65, 20))
        # self.rmb_pwd.setFixedHeight(20)
        # self.rmb_pwd.setFixedSize(58,14)
        self.rmb_pwd.setText(_fromUtf8(""))
        self.rmb_pwd.setObjectName(_fromUtf8("rmb_pwd"))


        #自动登录
        # self.text9 = QLabel(self.groupBox)
        # self.text9.setGeometry(QtCore.QRect(193,226, 70, 14))
        # self.text9.setText(_fromUtf8(""))
        # self.text9.setObjectName(_fromUtf8("text9"))
        #找回密码--改为忘记密码？
        self.forget_pwd=QPushButton(self.sourceWidget)
        self.forget_pwd.setAttribute(Qt.WA_TranslucentBackground)
        self.forget_pwd.setGeometry(QtCore.QRect(297, 240, 79, 20))
        self.forget_pwd.setText(_fromUtf8(""))
        self.forget_pwd.setObjectName(_fromUtf8("forget_pwd"))

        #登录的按钮
        self.btn_login = QPushButton(self.sourceWidget)
        self.btn_login.setAttribute(Qt.WA_TranslucentBackground)
        self.btn_login.setGeometry(QtCore.QRect(82, 292, 280, 32))
        self.btn_login.setText(_fromUtf8(""))
        self.btn_login.setObjectName(_fromUtf8("btn_login"))

        # 注册新账户
        self.register_new_account = QPushButton(self.sourceWidget)
        self.register_new_account.setAttribute(Qt.WA_TranslucentBackground)

        self.register_new_account.setGeometry(QtCore.QRect(208, self.btn_login.height() + self.btn_login.y() + 9, 28, 14))
        self.register_new_account.setFixedHeight(20)
        # self.register_new_account.setFixedHeight(20)
        # self.register_new_account.setFixedsize(80,14)
        self.register_new_account.setText(_fromUtf8(""))
        self.register_new_account.setObjectName(_fromUtf8("register_new_account"))

        # self.btn_login.setFocus(Qt.)
        #self.lesource_6 = QLineEdit(self.groupBox_2)
        #self.lesource_6.setGeometry(QtCore.QRect(100,175,200,25))
        #self.lesource_6.setObjectName(_fromUtf8("lesource_6"))
        #记住密码前面的方框
        self.checkBox_5 = QCheckBox(self.sourceWidget)
        self.checkBox_5.setGeometry(QtCore.QRect(89, 244, 14, 14))
        # 原来的自动登录前面的方框,保留备用
        # self.checkBox_6 = QCheckBox(self.sourceWidget)
        # self.checkBox_6.setGeometry(QtCore.QRect(204, 185, 18, 18))
        # self.checkBox_6.hide()
        # self.checkBox_5.setPalette(palette)

        font = QtGui.QFont()
        font.setFamily(_fromUtf8("Serif"))
        font.setPointSize(10)


        self.btnClose = QPushButton(self.sourceWidget)
        self.btnClose.setGeometry(QtCore.QRect(410, 4, 30, 30))
        self.btnClose.setText(_fromUtf8(""))

        self.sourceWidget.raise_()
        #self.label.raise_()

        self.retranslateUi(Login_ui)
        QtCore.QMetaObject.connectSlotsByName(Login_ui)

    #
    # 函数名:设置窗口标题
    # Function:set window title
    # 
    def retranslateUi(self, Login_ui):
        Login_ui.setWindowTitle(_translate("Login_ui", "Form", None))



if __name__ == "__main__":
    import sys
    app = QApplication(sys.argv)
    Login_ui = QWidget()
    ui = Ui_Login_ui()
    ui.setupUi(Login_ui)
    Login_ui.show()
    sys.exit(app.exec_())

