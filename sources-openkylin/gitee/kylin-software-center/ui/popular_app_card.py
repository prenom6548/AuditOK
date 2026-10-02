#!/usr/bin/python3
# -*- coding: utf-8 -*-


from PyQt5.QtGui import *
from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
from models.enums import BUTTON_DISABLE,BUTTON_PRESSED,BUTTON_LEAVE,BUTTON_ENTER,AppMsg
from utils import commontools
from models.application import *

class Popular_Card_Widget(QFrame):
    clicked_card = pyqtSignal(Application)
    def __init__(self, app_msg, card_width ,card_height,card_path,parent=None):
        QWidget.__init__(self, parent)
        self.m_parent = parent
        self.card_width = card_width
        self.card_height = card_height
        self.appmsg = app_msg
        self.card_path = card_path
        self.init_ui()

    def init_ui(self):
        self.resize(self.card_width, self.card_height)
        self.setFocusPolicy(Qt.NoFocus)
        self.setWindowFlags(Qt.FramelessWindowHint)
        self.setAttribute(Qt.WA_TranslucentBackground)

        self.card_label = QLabel(self)
        self.card_label.resize(288,190)
        self.card_label.move(0, 0)
        cover_image_name = self.card_path.split("/")[len(self.card_path.split("/"))-1]
        iconpath = commontools.get_coverimage(cover_image_name)

        if (Globals.DEVICE == 1):
            self.card_label.setStyleSheet("QLabel{border-radius: 12px;}")
            pix2 = QPixmap(iconpath)
            fitpixmap_userIcon = pix2.scaled(self.card_label.width(), self.card_label.height(), Qt.IgnoreAspectRatio,
                                             Qt.SmoothTransformation)
            fitpixmap_userIcon = self.PixmapToRound(fitpixmap_userIcon, self.width(), self.height())
            self.card_label.setPixmap(fitpixmap_userIcon)
        else:
            self.card_label.setStyleSheet("QLabel{border-image:url('" + iconpath + "');border-radius: 12px;}")

        self.hover_widget = QWidget(self)
        self.hover_widget.resize(self.width(), self.height())
        self.hover_widget.setWindowFlags(Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint)
        # self.card_list_widget.setAttribute(Qt.WA_TranslucentBackground)
        self.hover_widget.setStyleSheet("QWidget{background: rgba(0, 0, 0, 0.1);border-radius: 12px;}")
        self.hover_widget.move(0, 0)
        self.hover_widget.hide()


    def PixmapToRound(self,src,x,y):
        pixmapa = QPixmap(src)
        pixmap = QPixmap(x, y)
        pixmap.fill(Qt.transparent)
        painter = QPainter(pixmap)
        painter.setRenderHint(QPainter.Antialiasing, True)
        # painter.setRenderHints(QPainter.Antialiasing | QPainter.SmoothPixmapTransform)
        path = QPainterPath()
        rect = QRectF(0,0,x,y)
        path.addRoundedRect(rect, 12.0, 12.0)
        painter.setClipPath(path)
        painter.drawPixmap(0, 0, x, y, pixmapa)

        return pixmap


    # 鼠标进入事件
    def enterEvent(self, event):
        self.status = BUTTON_ENTER
        self.setGeometry(self.x(), self.y(), self.width() , self.height())
        self.shadow = QGraphicsDropShadowEffect(self)
        self.shadow.setOffset(0, 0)
        self.shadow.setColor(QColor(qRgba(149,149,149,0)))
        self.shadow.setBlurRadius(12)
        self.setGraphicsEffect(self.shadow)
        self.hover_widget.show()

    #
    # 函数：鼠标离开事件
    #
    def leaveEvent(self, event):
        self.status = BUTTON_LEAVE
        self.resize(self.width(),self.height())
        self.shadow = QGraphicsDropShadowEffect(self)
        self.shadow.setBlurRadius(0)
        self.shadow.setOffset(0, 0)
        self.setGraphicsEffect(self.shadow)
        self.hover_widget.hide()

    #
    # 函数：鼠标按下事件
    #
    def mousePressEvent(self, event):
        if event.button() == Qt.LeftButton:
            self.status = BUTTON_PRESSED
            self.clicked_card.emit(self.appmsg)
        else:
            event.ignore()
