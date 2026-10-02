#!/usr/bin/python3
# -*- coding: utf-8 -*-


from PyQt5.QtGui import *
from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
from models.globals import *
from models.enums import (BUTTON_ENTER, BUTTON_LEAVE, BUTTON_PRESSED, BUTTON_DISABLE)
import gettext
gettext.bindtextdomain("kylin-software-center", "/usr/share/locale")
gettext.textdomain("kylin-software-center")
_ = gettext.gettext

class SystemButton(QPushButton):

    leave_btn = pyqtSignal(int)
    enter_btn = pyqtSignal(int)
    click_btn = pyqtSignal(int)

    def __init__(self,is_setingbtn,is_minibtn,is_closebtn, parent = None):
        super(SystemButton,self).__init__(parent)
        #self.setStyleSheet("QPushButton{border-radius:4px;background-color:#3d6be5;}")
        self.mouse_press = False
        self.isclosebtn = is_closebtn
        self.issetingbtn=is_setingbtn
        self.isminibtn=is_minibtn
        self.setFocusPolicy(Qt.NoFocus)
        self.status = BUTTON_LEAVE
        self.setFixedSize(30, 30)
        self.click_btn.connect(self.paintbuttonevent)
        self.enter_btn.connect(self.paintbuttonevent)
        self.leave_btn.connect(self.paintbuttonevent)
        self.leave_btn.emit(0)


    #
    # 函数：鼠标进入事件
    #
    def enterEvent(self,event):
        self.enter_btn.emit(1)
        self.status = BUTTON_ENTER
        self.update()

    #
    # 函数：鼠标按下事件
    #
    def mousePressEvent(self,event):
        if(event.button() == Qt.LeftButton):
            self.mouse_press = True
            self.status = BUTTON_PRESSED
            self.update()

    #
    # 函数：鼠标释放事件
    #
    def mouseReleaseEvent(self,event):
        if(self.mouse_press and self.rect().contains(event.pos())):
            self.mouse_press = False
            self.click_btn.emit(2)
            self.status = BUTTON_ENTER
            self.update()
            self.clicked.emit(True)

    #
    # 函数：鼠标离开事件
    #
    def leaveEvent(self,event):
        self.leave_btn.emit(0)
        self.status = BUTTON_LEAVE
        self.update()

    #
    # 函数：绘图函数
    #
    def paintbuttonevent(self,mouse_state=0):
        self.painter = QPainter()
        if mouse_state==0:
            if self.isclosebtn:
                self.setStyleSheet("QPushButton{background-color:transparent;border-radius:4px;image: url(./res/close1.png);}")
            elif self.isminibtn:
                self.setStyleSheet("QPushButton{background-color:transparent;border-radius:4px;image: url(./res/mini1.png);}")
            else:
                self.setStyleSheet("QPushButton{background-color:transparent;border-radius:4px;image: url(./res/set1.png);}")
        elif mouse_state==1:
            if self.isclosebtn:
                self.setStyleSheet("QPushButton{background-color:#f86457;border-radius:4px;image: url(./res/close2.png);}")
            elif self.isminibtn:
                self.setStyleSheet("QPushButton{background-color:rgba(0, 0, 0, 0.04);border-radius:4px;image: url(./res/mini2.png);}")
            else:
                self.setStyleSheet("QPushButton{background-color:rgba(0, 0, 0, 0.04);border-radius:4px;image: url(./res/set2.png);}")
        elif mouse_state==2:
            if self.isclosebtn:
                self.setStyleSheet("QPushButton{background-color:#e44c50;border-radius:4px;image: url(./res/close3.png);}")
            elif self.isminibtn:
                self.setStyleSheet("QPushButton{background-color:rgba(48, 49, 51, 0.08);border-radius:4px;image: url(./res/mini3.png);}")
            else:
                self.setStyleSheet("QPushButton{background-color:rgba(48, 49, 51, 0.08);border-radius:4px;image: url(./res/set3.png);}")


class Click_Button(QPushButton):
    leave_btn = pyqtSignal(int)
    enter_btn = pyqtSignal(int)
    click_btn = pyqtSignal(int)
    clicked_this_btn = pyqtSignal(str)
    is_clicked = False

    def __init__(self, text,paint_parent,parent=None):
        super(Click_Button,self).__init__(parent)
        # self.setStyleSheet("QPushButton{border-radius:4px;background-color:#3d6be5;}")
        self.m_parent = parent
        self.mouse_press = False
        self.setFocusPolicy(Qt.NoFocus)
        self.status = BUTTON_LEAVE
        self.setText(text)
        self.adjustSize()
        self.setFixedHeight(24)
        self.click_btn.connect(self.paintbuttonevent)
        self.enter_btn.connect(self.paintbuttonevent)
        self.leave_btn.connect(self.paintbuttonevent)
        self.clicked_this_btn.connect(paint_parent.slot_clicked_btn)
        self.paint_parent = paint_parent
        if(text == _("ubuntukylin")):
            self.is_clicked = True
            self.setStyleSheet("QPushButton{font-size: 14px;font-weight: 400;color: #ffffff;background-color:#12C188;text-align: center;border-radius:12px;}")
            self.clicked_this_btn.emit(self.text())
        elif(text == _("Onload")):
            self.is_clicked = True
            self.setStyleSheet("QPushButton{border-radius: 12px;font-size: 14px;font-weight: 400;line-height: 20px;text-align: center;background-color: #3790FA;color: #ffffff;}")
            self.clicked_this_btn.emit(self.text())
        else:
            self.setStyleSheet("QPushButton{background-color:transparent;border-radius: 12px;font-size: 14px;font-weight: 400;text-align: center;color: #606265;line-height: 20px;}QPushButton:hover{background: rgba(55, 144, 250, 0.12);color: #3790FA;}QPushButton:pressed{background-color: #3790FA;color: #ffffff;}")

    #
    # 函数：鼠标进入事件
    #
    def enterEvent(self, event):
        self.enter_btn.emit(1)
        self.status = BUTTON_ENTER
        self.update()

    #
    # 函数：鼠标按下事件
    #
    def mousePressEvent(self, event):
        if (event.button() == Qt.LeftButton):
            self.mouse_press = True
            self.status = BUTTON_PRESSED
            self.update()

    #
    # 函数：鼠标释放事件
    #
    def mouseReleaseEvent(self, event):
        if (self.mouse_press and self.rect().contains(event.pos())):
            self.mouse_press = False
            self.show_which_btn()
            self.status = BUTTON_ENTER
            self.update()

    #
    # 函数：鼠标离开事件
    #
    def leaveEvent(self, event):
        self.leave_btn.emit(0)
        self.status = BUTTON_LEAVE
        self.update()

    #
    #函数：判断该按钮是否显示点击状态
    #
    def show_which_btn(self):
        self.click_btn.emit(2)
        state = self.is_clicked
        self.is_clicked = False
        clicked = self.paint_parent.paint_btn()
        if (clicked == False):
            self.is_clicked = True
        else:
            self.is_clicked = state
            if (self.is_clicked == False):
                self.is_clicked = True
            else:
                self.is_clicked = False
        if (self.is_clicked == True):
            try:
                self.paint_parent.type_name_label.setText(self.text())
                self.paint_parent.type_name_label.adjustSize()
            except:
                pass
            self.clicked_this_btn.emit(self.text())


    #
    # 函数：绘图函数
    #
    def paintbuttonevent(self, mouse_state=0):
        self.painter = QPainter()
        if(Globals.THEME_COLOR == 0):
            if mouse_state == 0:
                if(self.is_clicked == False):
                    self.setStyleSheet("QPushButton{background-color:transparent;border-radius: 12px;font-size: 14px;text-align: center;font-weight: 400;color: #606265;line-height: 20px;}")
                else:
                    self.setStyleSheet("QPushButton{border-radius: 12px;font-size: 14px;font-weight: 400;text-align: center;line-height: 20px;background-color: #3790FA;color: #ffffff;}")
            elif mouse_state == 1:
                if (self.is_clicked == False):
                    self.setStyleSheet("QPushButton{border-radius: 12px;font-size: 14px;font-weight: 400;text-align: center;line-height: 20px;background: rgba(55, 144, 250, 0.12);color: #3790FA;}")
                else:
                    self.setStyleSheet("QPushButton{border-radius: 12px;font-size: 14px;font-weight: 400;text-align: center;line-height: 20px;background-color: #3790FA;color: #ffffff;}")
            elif mouse_state == 2:
                self.setStyleSheet("QPushButton{border-radius: 12px;font-size: 14px;font-weight: 400;text-align: center;line-height: 20px;background-color: #3790FA;color: #ffffff;}")
        else:
            if mouse_state == 0:
                if (self.is_clicked == False):
                    self.setStyleSheet(
                        "QPushButton{background-color:transparent;border-radius: 12px;font-size: 14px;text-align: center;font-weight: 400;color: #8F9399;line-height: 20px;}")
                else:
                    self.setStyleSheet(
                        "QPushButton{border-radius: 12px;font-size: 14px;font-weight: 400;text-align: center;line-height: 20px;background-color: #3790FA;color: #ffffff;}")
            elif mouse_state == 1:
                if (self.is_clicked == False):
                    self.setStyleSheet(
                        "QPushButton{border-radius: 12px;font-size: 14px;font-weight: 400;text-align: center;line-height: 20px;background: rgba(55, 144, 250, 0.12);color: #3790FA;}")
                else:
                    self.setStyleSheet(
                        "QPushButton{border-radius: 12px;font-size: 14px;font-weight: 400;text-align: center;line-height: 20px;background-color: #3790FA;color: #ffffff;}")
            elif mouse_state == 2:
                self.setStyleSheet(
                    "QPushButton{border-radius: 12px;font-size: 14px;font-weight: 400;text-align: center;line-height: 20px;background-color: #3790FA;color: #ffffff;}")


if __name__ == '__main__':
   import sys
   app = QApplication(sys.argv)
   btn = SystemButton()
   btn.loadPixmap("../res/close_button.png")
   btn.show()
   sys.exit(app.exec_())