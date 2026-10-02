import os,sys
from PyQt5.QtGui import *
from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
from models.globals import *

class Circular_Progress_Bar(QFrame):
    ad_num = 0
    need_update = False
    ad_move_left_time = QTimer()
    ad_move_right_time = QTimer()
    final_x_position = 0
    final_x_position2 = 0
    def __init__(self,w,h,app_name,parent=None):
        QFrame.__init__(self ,parent)
        self.dblist = []
        self.applist = []
        self.pre_persent = 0
        self.appname= app_name
        self.w = w
        self.h = h
        self.init_down = True
        self.init_ui()


    def init_ui(self):
        self.resize(self.w,self.h)
        self.setWindowFlags(Qt.FramelessWindowHint)
        self.setAttribute(Qt.WA_TranslucentBackground)
        self.setFocusPolicy(Qt.NoFocus)
        # self.setStyleSheet("QFrame{border:0px solid #ff0000;border-radius: 11px;background-color:red;}")
        self.btn = QPushButton(self)
        self.btn.setFocusPolicy(Qt.NoFocus)
        self.btn.setFixedSize(self.w-4,self.w-4)
        self.btn.move(int((self.w-self.btn.width())/2),int((self.w-self.btn.width())/2))
        if(self.h == 36):
            self.btn.setStyleSheet("QPushButton{border:0px solid #ff0000;border-radius: 16px;background-color:red;image: url(./res/puase2.png);background-position:center;background-repeat:no-repeat;}")
        else:
            self.btn.setStyleSheet("QPushButton{border:0px solid #ff0000;border-radius: 9px;background-color:transparent;image: url(./res/puase1.png);background-position:center;background-repeat:no-repeat;}")
        self.btn.setAutoFillBackground(True)
        #self.btn.setScaledContents(True)
        #label_pixmap = QPixmap("./res/puase3.png")
        #self.btn.setPixmap(label_pixmap)
        self.btn.show()
        self.cont_btn = QPushButton(self)
        self.cont_btn.setAttribute(Qt.WA_TranslucentBackground)
        self.cont_btn.setFocusPolicy(Qt.NoFocus)
        self.cont_btn.setFixedSize(self.w - 4, self.w - 4)
        self.cont_btn.move(int((self.w - self.btn.width()) / 2), int((self.w - self.btn.width()) / 2))
        if (self.h == 36):
            self.cont_btn.setStyleSheet(
                "QPushButton{border:0px solid #ff0000;border-radius: 16px;background-color:transparent;image: url(./res/continue2.png);background-position:center;background-repeat:no-repeat;}")
        else:
            self.cont_btn.setStyleSheet(
                "QPushButton{border:0px solid #ff0000;border-radius: 9px;background-color:transparent;image: url(./res/continue1.png);background-position:center;background-repeat:no-repeat;}")
        self.cont_btn.setAutoFillBackground(True)
        self.cont_btn.hide()
        self.show()
        self.persent = 0


    def puase_download(self,p):
        self.persent = self.persent
        self.btn.hide()
        self.cont_btn.show()
        # self.persent = p
        self.update()

    #定义更新
    def GenUpdate(self, p):
        self.cont_btn.hide()
        self.btn.show()
        if(p == 0 and self.init_down == True):
            self.persent = p
            self.update()
            self.init_down = False
        if(p - self.persent >= 1):
            self.need_update = True
            self.persent = p
            self.update()
            self.init_down = False
        else:
            self.need_update = False
            self.init_down = False
        self.show()

    def update_progress(self,p):
        self.persent = p
        self.update()
        self.show()

    def reflash_color(self):
        self.update()

    def re_setstyle(self):
        if (self.h == 36):
            self.btn.setStyleSheet(
                "QPushButton{border:1px solid #ff0000;border-radius: 16px;background-color:transparent;image: url(./res/puase2.png);background-position:center;background-repeat:no-repeat;}")
            self.cont_btn.setStyleSheet(
                "QPushButton{border:0px solid #ff0000;border-radius: 16px;background-color:transparent;image: url(./res/continue2.png);background-position:center;background-repeat:no-repeat;}")
        else:
            self.btn.setStyleSheet(
                "QPushButton{border:0px solid #ff0000;border-radius: 9px;background-color:transparent;image: url(./res/puase1.png);background-position:center;background-repeat:no-repeat;}")
            self.cont_btn.setStyleSheet(
                "QPushButton{border:0px solid #ff0000;border-radius: 9px;background-color:transparent;image: url(./res/continue1.png);background-position:center;background-repeat:no-repeat;}")



    def paintEvent(self, event):
        #360°分成100等分
        rotateAngle = 360 * self.persent / 100
        painter1 = QPainter(self)
        # 启用反锯齿，如果本行注释，那么圆的外线有锯齿，不光滑。
        painter1.setRenderHints(QPainter.Antialiasing)
        # 角度渐变(QConicalGradient)
        gradient1 = QConicalGradient(50, 50, 91)
        # 进度条的画笔颜色
        if(Globals.THEME_COLOR == 0):
            gradient1.setColorAt(1, QColor(225, 225, 225))
        else:
            gradient1.setColorAt(1, QColor(69, 69, 69))
        self.pen1 = QPen()
        self.pen1.setBrush(gradient1)  # 设置画刷渐变效果
        self.pen1.setWidth(2)
        self.pen1.setCapStyle(Qt.RoundCap)
        painter1.setPen(self.pen1)
        # 250和250是圆点的坐标
        painter1.drawArc(QRectF(1, 1, self.width() - 2, self.width() - 2), int((90 - 0) * (self.width() - 4)),
                        -int(360 * (self.width() - 4)))  # 画圆环
        # 绘制准备工作，启用反锯齿
        painter = QPainter(self)
        #启用反锯齿，如果本行注释，那么圆的外线有锯齿，不光滑。
        painter.setRenderHints(QPainter.Antialiasing)
        #角度渐变(QConicalGradient)
        gradient = QConicalGradient(50, 50, 91)
        #进度条的画笔颜色
        gradient.setColorAt(1, QColor(55,144,250))
        self.pen = QPen()
        self.pen.setBrush(gradient)  # 设置画刷渐变效果
        self.pen.setWidth(2)
        self.pen.setCapStyle(Qt.RoundCap)
        painter.setPen(self.pen)
        #250和250是圆点的坐标
        painter.drawArc(QRectF(1, 1, self.width()-2, self.width()-2), int((90 - 0) * 16), -int(rotateAngle * 16))  # 画圆环
