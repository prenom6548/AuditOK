import os
import math
from PyQt5.QtGui import *
from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
from ui.systembutton import *
import configparser
from models.enums import CONF_PATH
import gettext
gettext.bindtextdomain("kylin-software-center", "/usr/share/locale")
gettext.textdomain("kylin-software-center")
_ = gettext.gettext

class Set_Widget(QWidget):
    def __init__(self,software_store_auto_update,software_auto_update,notice_software_update,parent = None):
        QWidget.__init__(self, parent)
        self.mouse_press = False
        self.software_store_auto_update = software_store_auto_update
        self.software_auto_update = software_auto_update
        self.notice_software_update = notice_software_update
        self.init_ui()
        self.paintEvent = self.set_paintEvent

    def init_ui(self):
        self.setFixedSize(442,450)
        self.setWindowFlags(Qt.FramelessWindowHint | Qt.Tool)
        self.setWindowTitle(_("Software Store"))
        self.setAttribute(Qt.WA_TranslucentBackground)
        self.widget = QWidget(self)
        self.widget.resize(424,432)
        self.widget.move(9,9)
        #self.widget.setStyleSheet("QWidget{background: rgba(255, 255, 255, 0.7);border-radius:12px;background-color:#ffffff;}")
        self.title_label = QLabel(self.widget)
        self.title_label.setFixedSize(170,16)
        self.title_label.setText(_("Set"))
        self.title_label.setStyleSheet("QLabel{border:0px;font-size: 14px;color: #1B1B1B;font-weight: 500;line-height: 14px;}")
        self.title_label.move(16,12)

        self.auto_update_label = QLabel(self.widget)
        self.auto_update_label.setAttribute(Qt.WA_TranslucentBackground)
        self.auto_update_label.setFixedHeight(20)
        self.auto_update_label.setText(_("Auto update:"))
        self.auto_update_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #606265;line-height: 20px;}")
        self.auto_update_label.adjustSize()
        self.auto_update_label.move(16,55)

        self.check_btn1 = QCheckBox(self.widget)
        self.check_btn1.setFocusPolicy(Qt.NoFocus)
        self.check_btn1.resize(16,16)
        self.check_btn1.setStyleSheet(
            "QCheckBox{border-radius: 3px;spacing: 0px;background: transparent;border-radius: 3px;border: 0px solid rgba(0, 0, 0, 0.15);}" \
            "QCheckBox:hover{background: transparent;border-radius: 3px;}" \
            "QCheckBox::indicator{width: 16px;height: 16px;border-radius:3px;}" \
            "QCheckBox::indicator:unchecked{background-color:transparent;border:0px solid #bfbfbf;image: url(./res/set_check.png);}" \
            "QCheckBox::indicator:unchecked:hover{background-color:transparent;border: 0px solid #8F9399;image: url(./res/set_check_hover.png);}" \
            "QCheckBox::indicator:unchecked:pressed{background-color:transparent;border:0px solid #415FC4;image: url(./res/set_check_hover.png);}" \
            "QCheckBox::indicator{width: 16px;height: 16px;}" \
            "QCheckBox::indicator:checked{border-radius: 3px;background-color: transparent;border:0px solid #415FC4;image: url(./res/set_check_press.png);}" \
            "QCheckBox::indicator:checked:hover{border-radius: 3px;background-color:transparent;border:0px solid #415FC4;image: url(./res/set_check_press.png);}" \
            "QCheckBox::indicator:checked:pressed{border-radius: 3px;background-color:transparent;border:0px solid #415FC4;image: url(./res/set_check_press.png);}")
        self.check_btn1.move(self.auto_update_label.x()+self.auto_update_label.width()+2,int((self.auto_update_label.height()-self.check_btn1.height())/2) + self.auto_update_label.y())
        if(self.software_store_auto_update == "True"):
            self.check_btn1.setChecked(True)

        self.auto_update_uksc_label = QLabel(self.widget)
        self.auto_update_uksc_label.setAttribute(Qt.WA_TranslucentBackground)
        self.auto_update_uksc_label.setFixedHeight(20)
        self.auto_update_uksc_label.setText(_("Auto update software store"))
        self.auto_update_uksc_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #606265;line-height: 20px;}")
        self.auto_update_uksc_label.move(self.check_btn1.x()+self.check_btn1.width()+8,int((self.check_btn1.height()-self.auto_update_uksc_label.height())/2)+self.check_btn1.y())
        self.auto_update_uksc_label.adjustSize()

        self.auto_update_uksc_tip_label = QLabel(self.widget)
        self.auto_update_uksc_tip_label.setAttribute(Qt.WA_TranslucentBackground)
        self.auto_update_uksc_tip_label.setFixedHeight(20)
        self.auto_update_uksc_tip_label.setText(_("When the software store has a new version, it will be updated automatically"))
        self.auto_update_uksc_tip_label.setStyleSheet(
            "QLabel{font-size: 14px;font-weight: 400;color: #999999;line-height: 20px;}")
        self.auto_update_uksc_tip_label.move(self.auto_update_uksc_label.x(), self.auto_update_uksc_label.y()+self.auto_update_uksc_label.height()+4)
        self.auto_update_uksc_tip_label.adjustSize()

        self.check_btn2 = QCheckBox(self.widget)
        self.check_btn2.setFocusPolicy(Qt.NoFocus)
        self.check_btn2.resize(16, 16)
        self.check_btn2.setStyleSheet(
            "QCheckBox{border-radius: 3px;spacing: 0px;background: transparent;border-radius: 3px;border: 0px solid rgba(0, 0, 0, 0.15);}" \
            "QCheckBox:hover{background: transparent;border-radius: 3px;}" \
            "QCheckBox::indicator{width: 16px;height: 16px;border-radius:3px;}" \
            "QCheckBox::indicator:unchecked{background-color:transparent;border:0px solid #bfbfbf;image: url(./res/set_check.png);}" \
            "QCheckBox::indicator:unchecked:hover{background-color:transparent;border: 0px solid #8F9399;image: url(./res/set_check_hover.png);}" \
            "QCheckBox::indicator:unchecked:pressed{background-color:transparent;border:0px solid #415FC4;image: url(./res/set_check_hover.png);}" \
            "QCheckBox::indicator{width: 16px;height: 16px;}" \
            "QCheckBox::indicator:checked{border-radius: 3px;background-color: transparent;border:0px solid #415FC4;image: url(./res/set_check_press.png);}" \
            "QCheckBox::indicator:checked:hover{border-radius: 3px;background-color:transparent;border:0px solid #415FC4;image: url(./res/set_check_press.png);}" \
            "QCheckBox::indicator:checked:pressed{border-radius: 3px;background-color:transparent;border:0px solid #415FC4;image: url(./res/set_check_press.png);}")
        self.check_btn2.move(self.check_btn1.x(), self.auto_update_uksc_tip_label.y()+self.auto_update_uksc_tip_label.height()+10)
        self.check_btn2.stateChanged.connect(self.change_check_btn_statue)
        if(self.software_auto_update == "True"):
            self.check_btn2.setChecked(True)

        self.auto_update_software_label = QLabel(self.widget)
        self.auto_update_software_label.setAttribute(Qt.WA_TranslucentBackground)
        self.auto_update_software_label.setFixedHeight(20)
        self.auto_update_software_label.setText(_("Auto update software"))
        self.auto_update_software_label.setStyleSheet(
            "QLabel{font-size: 14px;font-weight: 400;color: #606265;line-height: 20px;}")
        self.auto_update_software_label.move(self.check_btn2.x() + self.check_btn2.width() + 8, int((self.check_btn2.height() - self.auto_update_software_label.height()) / 2) + self.check_btn2.y())
        self.auto_update_software_label.adjustSize()

        self.auto_update_software_tip_label = QLabel(self.widget)
        self.auto_update_software_tip_label.setAttribute(Qt.WA_TranslucentBackground)
        self.auto_update_software_tip_label.setFixedHeight(20)
        self.auto_update_software_tip_label.setText(_("Software installed through the store will be automatically updated"))
        self.auto_update_software_tip_label.setStyleSheet(
            "QLabel{font-size: 14px;font-weight: 400;color: #999999;line-height: 20px;}")
        self.auto_update_software_tip_label.move(self.auto_update_software_label.x(),self.auto_update_software_label.y() + self.auto_update_software_label.height() + 4)
        self.auto_update_software_tip_label.adjustSize()

        if (self.auto_update_software_tip_label.width() > self.auto_update_uksc_tip_label.width()):
            self.widget.setFixedWidth(self.auto_update_software_tip_label.x() + self.auto_update_software_tip_label.width() + 30)
            self.setFixedWidth(self.widget.width() + 18)
        else:
            if (self.auto_update_uksc_tip_label.x() + self.auto_update_uksc_tip_label.width() + 30 > self.widget.width()):
                self.widget.setFixedWidth(self.auto_update_uksc_tip_label.x() + self.auto_update_uksc_tip_label.width() + 30)
                self.setFixedWidth(self.widget.width() + 18)


        self.line_label = QLabel(self.widget)
        self.line_label.setFixedSize(self.widget.width() - 2, 1)
        self.line_label.setStyleSheet("QLabel{width: 424px;height: 1px;background: #F2F6FD;}")
        self.line_label.move(1, 38)

        self.line_label1 = QLabel(self.widget)
        self.line_label1.setFixedSize(self.widget.width()-16*2, 1)
        self.line_label1.setStyleSheet("QLabel{width: 424px;height: 1px;background: #F2F6FD;}")
        self.line_label1.move(16, 167)

        self.notice_label = QLabel(self.widget)
        self.notice_label.setAttribute(Qt.WA_TranslucentBackground)
        self.notice_label.setFixedHeight(20)
        self.notice_label.setText(_("notice:"))
        self.notice_label.setStyleSheet(
            "QLabel{font-size: 14px;font-weight: 400;color: #606265;line-height: 20px;}")
        self.notice_label.adjustSize()
        self.notice_label.move(self.auto_update_label.width()-self.notice_label.width()+self.auto_update_label.x(), self.line_label1.y()+self.line_label1.height()+16)

        self.check_btn3 = QCheckBox(self.widget)
        self.check_btn3.setFocusPolicy(Qt.NoFocus)
        self.check_btn3.resize(16, 16)
        self.check_btn3.setStyleSheet(
            "QCheckBox{border-radius: 3px;spacing: 0px;background: transparent;border-radius: 3px;border: 0px solid rgba(0, 0, 0, 0.15);}" \
            "QCheckBox:hover{background: transparent;border-radius: 3px;}" \
            "QCheckBox::indicator{width: 16px;height: 16px;border-radius:3px;}" \
            "QCheckBox::indicator:unchecked{background-color:transparent;border:0px solid #bfbfbf;image: url(./res/set_check.png);}" \
            "QCheckBox::indicator:unchecked:hover{background-color:transparent;border: 0px solid #8F9399;image: url(./res/set_check_hover.png);}" \
            "QCheckBox::indicator:unchecked:pressed{background-color:transparent;border:0px solid #415FC4;image: url(./res/set_check_hover.png);}" \
            "QCheckBox::indicator{width: 16px;height: 16px;}" \
            "QCheckBox::indicator:checked{border-radius: 3px;background-color: transparent;border:0px solid #415FC4;image: url(./res/set_check_press.png);}" \
            "QCheckBox::indicator:checked:hover{border-radius: 3px;background-color:transparent;border:0px solid #415FC4;image: url(./res/set_check_press.png);}" \
            "QCheckBox::indicator:checked:pressed{border-radius: 3px;background-color:transparent;border:0px solid #415FC4;image: url(./res/set_check_press.png);}")
        self.check_btn3.move(self.notice_label.x() + self.notice_label.width() + 2, int(
            (self.notice_label.height() - self.check_btn3.height()) / 2) + self.notice_label.y())
        if (self.software_auto_update == "True"):
            try:
                cf = configparser.ConfigParser()
                cf.read(CONF_PATH)
                try:
                    cf.set("option-settings", "notice-software-update", "False")
                    with open(CONF_PATH, "w+") as f:
                        cf.write(f)
                except:
                    pass
            except:
                pass
            self.change_check_btn_statue(2)
        else:
            if(self.notice_software_update == "True"):
                self.check_btn3.setChecked(True)


        self.notice_software_update_label = QLabel(self.widget)
        self.notice_software_update_label.setAttribute(Qt.WA_TranslucentBackground)
        self.notice_software_update_label.setFixedHeight(20)
        self.notice_software_update_label.setText(_("Allow software update notifications"))
        self.notice_software_update_label.setStyleSheet(
            "QLabel{font-size: 14px;font-weight: 400;color: #606265;line-height: 20px;}")
        self.notice_software_update_label.move(self.check_btn3.x() + self.check_btn3.width() + 8, int(
            (self.check_btn3.height() - self.notice_software_update_label.height()) / 2) + self.check_btn3.y())
        self.notice_software_update_label.adjustSize()

        self.line_label2 = QLabel(self.widget)
        self.line_label2.setFixedSize(self.widget.width()-16*2, 1)
        self.line_label2.setStyleSheet("QLabel{width: 424px;height: 1px;background: #F2F6FD;}")
        self.line_label2.move(16, 325)
        self.line_label2.hide()

        self.btn = QPushButton(self.widget)
        self.btn.setFocusPolicy(Qt.NoFocus)
        self.btn.setFixedSize(64,30)
        self.btn.setStyleSheet("QPushButton{background: #3790FA;border-radius: 6px;font-size: 14px;font-weight: 400;color: #FFFFFF;text-align: center;line-height: 12px;}QPushButton:hover{background: #40A9FB;;border-radius: 6px;color: #ffffff;}QPushButton:pressed{background: #296CD9;border-radius: 6px;color: #ffffff;}")
        self.btn.setText(_("OK"))
        self.btn.move(self.widget.width()-self.btn.width()-16,self.notice_label.y()+self.notice_label.height()+16)

        self.cancel_btn = QPushButton(self.widget)
        self.cancel_btn.setFocusPolicy(Qt.NoFocus)
        self.cancel_btn.setFixedSize(64, 30)
        self.cancel_btn.setStyleSheet("QPushButton{font-size: 14px;font-weight: 400;color: #8F9399;line-height: 20px;border-radius: 6px;text-align: center;border: 1px solid #DDDFE7;}QPushButton:hover{background: #F2F6FD;border-radius: 6px;border: 1px solid #DDDFE7;}QPushButton:pressed{background: #F2F6FD;border-radius: 6px;border: 1px solid #DDDFE7;}")
        self.cancel_btn.setText(_("cancel"))
        self.cancel_btn.move(self.btn.x()-self.cancel_btn.width()-16, self.btn.y())
        self.cancel_btn.show()

        self.close_btn = QPushButton(self.widget)
        self.close_btn.setFocusPolicy(Qt.NoFocus)
        self.close_btn.resize(30, 30)
        self.close_btn.setStyleSheet(
            "QPushButton{border-radius: 4px;border:0px;border-image:url('res/close1.png')}QPushButton:hover{border:0px;border-image:url('res/close2.png')}QPushButton:pressed{border:0px;border-image:url('res/close3.png')}")
        self.close_btn.move(self.widget.width() - self.close_btn.width() - 4, 4)
        self.close_btn.setToolTip(_("close"))

        self.widget.setFixedSize(self.widget.width(), self.cancel_btn.y() + self.cancel_btn.height() +24)
        self.setFixedSize(self.widget.width() + 18 ,self.widget.height()+18)

        self.slot_show_dark_mode()

    def change_check_btn_statue(self,statue):
        if(hasattr(self,"check_btn3") == False):
            return
        if(statue == 2):
            self.check_btn3.setChecked(False)
            self.check_btn3.setEnabled(False)
            if(Globals.THEME_COLOR == 0):
                self.check_btn3.setStyleSheet("QCheckBox{border-radius: 3px;spacing: 0px;background: #ECECEC;border-radius: 3px;border: 0px solid rgba(0, 0, 0, 0.15);}")
            else:
                self.check_btn3.setStyleSheet("QCheckBox{border-radius: 3px;spacing: 0px;background: #3A3A3A;border-radius: 3px;border: 0px solid rgba(0, 0, 0, 0.15);}")
        else:
            self.check_btn3.setEnabled(True)
            if(Globals.THEME_COLOR == 0):
                self.check_btn3.setStyleSheet(
                    "QCheckBox{border-radius: 3px;spacing: 0px;background: transparent;border-radius: 3px;border: 0px solid rgba(0, 0, 0, 0.15);}" \
                    "QCheckBox:hover{background: transparent;border-radius: 3px;}" \
                    "QCheckBox::indicator{width: 16px;height: 16px;border-radius:3px;}" \
                    "QCheckBox::indicator:unchecked{background-color:transparent;border:0px solid #bfbfbf;image: url(./res/set_check.png);}" \
                    "QCheckBox::indicator:unchecked:hover{background-color:transparent;border: 0px solid #8F9399;image: url(./res/set_check_hover.png);}" \
                    "QCheckBox::indicator:unchecked:pressed{background-color:transparent;border:0px solid #415FC4;image: url(./res/set_check_hover.png);}" \
                    "QCheckBox::indicator{width: 16px;height: 16px;}" \
                    "QCheckBox::indicator:checked{border-radius: 3px;background-color: transparent;border:0px solid #415FC4;image: url(./res/set_check_press.png);}" \
                    "QCheckBox::indicator:checked:hover{border-radius: 3px;background-color:transparent;border:0px solid #415FC4;image: url(./res/set_check_press.png);}" \
                    "QCheckBox::indicator:checked:pressed{border-radius: 3px;background-color:transparent;border:0px solid #415FC4;image: url(./res/set_check_press.png);}")
            else:
                self.check_btn3.setStyleSheet(
                    "QCheckBox{border-radius: 3px;spacing: 0px;background: transparent;border-radius: 3px;border: 0px solid rgba(0, 0, 0, 0.15);}" \
                    "QCheckBox:hover{background: transparent;border-radius: 3px;}" \
                    "QCheckBox::indicator{width: 16px;height: 16px;border-radius:3px;}" \
                    "QCheckBox::indicator:unchecked{background-color:transparent;border:0px solid #bfbfbf;image: url(./res/set_check.png);}" \
                    "QCheckBox::indicator:unchecked:hover{background-color:transparent;border: 0px solid #8F9399;image: url(./res/set_check_hover.png);}" \
                    "QCheckBox::indicator:unchecked:pressed{background-color:transparent;border:0px solid #415FC4;image: url(./res/set_check_hover.png);}" \
                    "QCheckBox::indicator{width: 16px;height: 16px;}" \
                    "QCheckBox::indicator:checked{border-radius: 3px;background-color: transparent;border:0px solid #415FC4;image: url(./res/set_check_press.png);}" \
                    "QCheckBox::indicator:checked:hover{border-radius: 3px;background-color:transparent;border:0px solid #415FC4;image: url(./res/set_check_press.png);}" \
                    "QCheckBox::indicator:checked:pressed{border-radius: 3px;background-color:transparent;border:0px solid #415FC4;image: url(./res/set_check_press.png);}")

    #
    # 函数：处理主题颜色的显示
    #
    def slot_show_dark_mode(self):
        if (Globals.THEME_COLOR == 0):
            if(Globals.RIGHT_ANGLE):
                self.widget.setStyleSheet(".QWidget{background-color:#ffffff;border: 1px solid rgba(38, 38, 38, 0.15);}")
                self.setStyleSheet("QFrame{background-color:#ffffff;}")
            else:
                self.widget.setStyleSheet(".QWidget{border-radius:12px;background-color:#ffffff;border: 1px solid rgba(38, 38, 38, 0.15);}")
                self.setStyleSheet("QFrame{background-color:#ffffff;}")
            self.title_label.setStyleSheet("QLabel{font-size: 14px;color: #1B1B1B;font-weight: 500;background:transparent;}")
            self.line_label.setStyleSheet("QLabel{width: 424px;height: 1px;background: #F2F6FD;}")
            self.line_label1.setStyleSheet("QLabel{width: 424px;height: 1px;background: #F2F6FD;}")
            self.line_label2.setStyleSheet("QLabel{width: 424px;height: 1px;background: #F2F6FD;}")
            self.auto_update_label.setStyleSheet(
                "QLabel{font-size: 14px;font-weight: 400;color: #606265;line-height: 20px;}")
            self.notice_label.setStyleSheet(
                "QLabel{font-size: 14px;font-weight: 400;color: #606265;line-height: 20px;}")
            self.auto_update_uksc_label.setStyleSheet(
                "QLabel{font-size: 14px;font-weight: 400;color: #606265;line-height: 20px;}")
            self.notice_software_update_label.setStyleSheet(
                "QLabel{font-size: 14px;font-weight: 400;color: #606265;line-height: 20px;}")
            self.auto_update_uksc_tip_label.setStyleSheet(
                "QLabel{font-size: 14px;font-weight: 400;color: #999999;line-height: 20px;}")
            self.auto_update_software_label.setStyleSheet(
                "QLabel{font-size: 14px;font-weight: 400;color: #606265;line-height: 20px;}")
            self.auto_update_software_tip_label.setStyleSheet(
                "QLabel{font-size: 14px;font-weight: 400;color: #999999;line-height: 20px;}")
            self.close_btn.setStyleSheet("QPushButton{border-radius: 4px;border:0px;border-image:url('res/close1.png')}QPushButton:hover{border:0px;border-image:url('res/close2.png')}QPushButton:pressed{border:0px;border-image:url('res/close3.png')}")
            self.check_btn1.setStyleSheet(
                "QCheckBox{border-radius: 3px;spacing: 0px;background: transparent;border-radius: 3px;border: 0px solid rgba(0, 0, 0, 0.15);}" \
                "QCheckBox:hover{background: transparent;border-radius: 3px;}" \
                "QCheckBox::indicator{width: 16px;height: 16px;border-radius:3px;}" \
                "QCheckBox::indicator:unchecked{background-color:transparent;border:0px solid #bfbfbf;image: url(./res/set_check.png);}" \
                "QCheckBox::indicator:unchecked:hover{background-color:transparent;border: 0px solid #8F9399;image: url(./res/set_check_hover.png);}" \
                "QCheckBox::indicator:unchecked:pressed{background-color:transparent;border:0px solid #415FC4;image: url(./res/set_check_hover.png);}" \
                "QCheckBox::indicator{width: 16px;height: 16px;}" \
                "QCheckBox::indicator:checked{border-radius: 3px;background-color: transparent;border:0px solid #415FC4;image: url(./res/set_check_press.png);}" \
                "QCheckBox::indicator:checked:hover{border-radius: 3px;background-color:transparent;border:0px solid #415FC4;image: url(./res/set_check_press.png);}" \
                "QCheckBox::indicator:checked:pressed{border-radius: 3px;background-color:transparent;border:0px solid #415FC4;image: url(./res/set_check_press.png);}")
            self.check_btn2.setStyleSheet(
                "QCheckBox{border-radius: 3px;spacing: 0px;background: transparent;border-radius: 3px;border: 0px solid rgba(0, 0, 0, 0.15);}" \
                "QCheckBox:hover{background: transparent;border-radius: 3px;}" \
                "QCheckBox::indicator{width: 16px;height: 16px;border-radius:3px;}" \
                "QCheckBox::indicator:unchecked{background-color:transparent;border:0px solid #bfbfbf;image: url(./res/set_check.png);}" \
                "QCheckBox::indicator:unchecked:hover{background-color:transparent;border: 0px solid #8F9399;image: url(./res/set_check_hover.png);}" \
                "QCheckBox::indicator:unchecked:pressed{background-color:transparent;border:0px solid #415FC4;image: url(./res/set_check_hover.png);}" \
                "QCheckBox::indicator{width: 16px;height: 16px;}" \
                "QCheckBox::indicator:checked{border-radius: 3px;background-color: transparent;border:0px solid #415FC4;image: url(./res/set_check_press.png);}" \
                "QCheckBox::indicator:checked:hover{border-radius: 3px;background-color:transparent;border:0px solid #415FC4;image: url(./res/set_check_press.png);}" \
                "QCheckBox::indicator:checked:pressed{border-radius: 3px;background-color:transparent;border:0px solid #415FC4;image: url(./res/set_check_press.png);}")
            if(self.check_btn3.isEnabled() == True):
                self.check_btn3.setStyleSheet(
                    "QCheckBox{border-radius: 3px;spacing: 0px;background: transparent;border-radius: 3px;border: 0px solid rgba(0, 0, 0, 0.15);}" \
                    "QCheckBox:hover{background: transparent;border-radius: 3px;}" \
                    "QCheckBox::indicator{width: 16px;height: 16px;border-radius:3px;}" \
                    "QCheckBox::indicator:unchecked{background-color:transparent;border:0px solid #bfbfbf;image: url(./res/set_check.png);}" \
                    "QCheckBox::indicator:unchecked:hover{background-color:transparent;border: 0px solid #8F9399;image: url(./res/set_check_hover.png);}" \
                    "QCheckBox::indicator:unchecked:pressed{background-color:transparent;border:0px solid #415FC4;image: url(./res/set_check_hover.png);}" \
                    "QCheckBox::indicator{width: 16px;height: 16px;}" \
                    "QCheckBox::indicator:checked{border-radius: 3px;background-color: transparent;border:0px solid #415FC4;image: url(./res/set_check_press.png);}" \
                    "QCheckBox::indicator:checked:hover{border-radius: 3px;background-color:transparent;border:0px solid #415FC4;image: url(./res/set_check_press.png);}" \
                    "QCheckBox::indicator:checked:pressed{border-radius: 3px;background-color:transparent;border:0px solid #415FC4;image: url(./res/set_check_press.png);}")
            else:
                self.check_btn3.setStyleSheet(
                    "QCheckBox{border-radius: 3px;spacing: 0px;background: #ECECEC;border-radius: 3px;border: 0px solid rgba(0, 0, 0, 0.15);}")
        else:
            if (Globals.RIGHT_ANGLE):
                self.widget.setStyleSheet(".QWidget{background-color:#3D3D41;border: 1px solid rgba(38, 38, 38, 0.15);}")
                self.setStyleSheet("QFrame{background-color:#1F2022;}")
            else:
                self.setStyleSheet("QFrame{background-color:#1F2022;}")
                self.widget.setStyleSheet(".QWidget{border-radius:12px;background-color:#3D3D41;border: 1px solid rgba(38, 38, 38, 0.15);}")
            self.title_label.setStyleSheet("QLabel{font-size: 14px;color: #8F9399;font-weight: 500;background:transparent;}")
            self.line_label.setStyleSheet("QLabel{width: 424px;height: 1px;background: #48484C;}")
            self.line_label1.setStyleSheet("QLabel{width: 424px;height: 1px;background: #48484C;}")
            self.line_label2.setStyleSheet("QLabel{width: 424px;height: 1px;background: #48484C;}")
            self.auto_update_label.setStyleSheet(
                "QLabel{font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 20px;}")
            self.notice_label.setStyleSheet(
                "QLabel{font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 20px;}")
            self.auto_update_uksc_label.setStyleSheet(
                "QLabel{font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 20px;}")
            self.notice_software_update_label.setStyleSheet(
                "QLabel{font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 20px;}")
            self.auto_update_uksc_tip_label.setStyleSheet(
                "QLabel{font-size: 14px;font-weight: 400;color: #999999;line-height: 20px;}")
            self.auto_update_software_label.setStyleSheet(
                "QLabel{font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 20px;}")
            self.auto_update_software_tip_label.setStyleSheet(
                "QLabel{font-size: 14px;font-weight: 400;color: #999999;line-height: 20px;}")
            self.close_btn.setStyleSheet(
                "QPushButton{border-radius: 4px;border:0px;border-image:url('res/common-close-white.png')}QPushButton:hover{border:0px;border-image:url('res/hover-close.png')}QPushButton:pressed{border:0px;border-image:url('res/clicked-close.png')}")
            self.check_btn1.setStyleSheet(
                "QCheckBox{border-radius: 3px;spacing: 0px;background: transparent;border-radius: 3px;border: 0px solid rgba(0, 0, 0, 0.15);}" \
                "QCheckBox:hover{background: transparent;border-radius: 3px;}" \
                "QCheckBox::indicator{width: 16px;height: 16px;border-radius:3px;}" \
                "QCheckBox::indicator:unchecked{background-color:transparent;border:0px solid #bfbfbf;image: url(./res/set_check.png);}" \
                "QCheckBox::indicator:unchecked:hover{background-color:transparent;border: 0px solid #8F9399;image: url(./res/set_check_hover.png);}" \
                "QCheckBox::indicator:unchecked:pressed{background-color:transparent;border:0px solid #415FC4;image: url(./res/set_check_hover.png);}" \
                "QCheckBox::indicator{width: 16px;height: 16px;}" \
                "QCheckBox::indicator:checked{border-radius: 3px;background-color: transparent;border:0px solid #415FC4;image: url(./res/set_check_press.png);}" \
                "QCheckBox::indicator:checked:hover{border-radius: 3px;background-color:transparent;border:0px solid #415FC4;image: url(./res/set_check_press.png);}" \
                "QCheckBox::indicator:checked:pressed{border-radius: 3px;background-color:transparent;border:0px solid #415FC4;image: url(./res/set_check_press.png);}")
            self.check_btn2.setStyleSheet(
                "QCheckBox{border-radius: 3px;spacing: 0px;background: transparent;border-radius: 3px;border: 0px solid rgba(0, 0, 0, 0.15);}" \
                "QCheckBox:hover{background: transparent;border-radius: 3px;}" \
                "QCheckBox::indicator{width: 16px;height: 16px;border-radius:3px;}" \
                "QCheckBox::indicator:unchecked{background-color:transparent;border:0px solid #bfbfbf;image: url(./res/set_check.png);}" \
                "QCheckBox::indicator:unchecked:hover{background-color:transparent;border: 0px solid #8F9399;image: url(./res/set_check_hover.png);}" \
                "QCheckBox::indicator:unchecked:pressed{background-color:transparent;border:0px solid #415FC4;image: url(./res/set_check_hover.png);}" \
                "QCheckBox::indicator{width: 16px;height: 16px;}" \
                "QCheckBox::indicator:checked{border-radius: 3px;background-color: transparent;border:0px solid #415FC4;image: url(./res/set_check_press.png);}" \
                "QCheckBox::indicator:checked:hover{border-radius: 3px;background-color:transparent;border:0px solid #415FC4;image: url(./res/set_check_press.png);}" \
                "QCheckBox::indicator:checked:pressed{border-radius: 3px;background-color:transparent;border:0px solid #415FC4;image: url(./res/set_check_press.png);}")
            if(self.check_btn3.isEnabled() == True):
                self.check_btn3.setStyleSheet(
                    "QCheckBox{border-radius: 3px;spacing: 0px;background: transparent;border-radius: 3px;border: 0px solid rgba(0, 0, 0, 0.15);}" \
                    "QCheckBox:hover{background: transparent;border-radius: 3px;}" \
                    "QCheckBox::indicator{width: 16px;height: 16px;border-radius:3px;}" \
                    "QCheckBox::indicator:unchecked{background-color:transparent;border:0px solid #bfbfbf;image: url(./res/set_check.png);}" \
                    "QCheckBox::indicator:unchecked:hover{background-color:transparent;border: 0px solid #8F9399;image: url(./res/set_check_hover.png);}" \
                    "QCheckBox::indicator:unchecked:pressed{background-color:transparent;border:0px solid #415FC4;image: url(./res/set_check_hover.png);}" \
                    "QCheckBox::indicator{width: 16px;height: 16px;}" \
                    "QCheckBox::indicator:checked{border-radius: 3px;background-color: transparent;border:0px solid #415FC4;image: url(./res/set_check_press.png);}" \
                    "QCheckBox::indicator:checked:hover{border-radius: 3px;background-color:transparent;border:0px solid #415FC4;image: url(./res/set_check_press.png);}" \
                    "QCheckBox::indicator:checked:pressed{border-radius: 3px;background-color:transparent;border:0px solid #415FC4;image: url(./res/set_check_press.png);}")
            else:
                self.check_btn3.setStyleSheet("QCheckBox{border-radius: 3px;spacing: 0px;background: #3A3A3A;border-radius: 3px;border: 0px solid rgba(0, 0, 0, 0.15);}")

        # QApplication.processEvents()

    #
    # 函数：重绘函数
    #
    def set_paintEvent(self, event):
        painter = QPainter(self)
        m_defaultBackgroundColor = QColor(qRgba(0, 0, 0, 16))
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