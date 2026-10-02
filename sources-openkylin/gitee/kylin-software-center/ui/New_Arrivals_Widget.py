#!/usr/bin/python3
# -*- coding: utf-8 -*-

import os
from PyQt5.QtGui import *
from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
from models.enums import CACHE_DIR,BUTTON_PRESSED,BUTTON_ENTER,BUTTON_LEAVE,AppMsg,PageStates,ClickCard
from models.application import *
from utils.commontools import *
import threading
import gettext
gettext.bindtextdomain("kylin-software-center", "/usr/share/locale")
gettext.textdomain("kylin-software-center")
_ = gettext.gettext


class New_Arrivals_Widget(QWidget):
    click_ad_card = pyqtSignal(Application,bool,int)
    start_move_time = QTimer()
    ad_move_left_time = QTimer()
    final_x_position = 0
    final_x_position2 = 0
    def __init__(self,w,mw,parent=None):
        QWidget.__init__(self, parent)
        self.appmsg = None
        self.mw = mw
        self.num = 0
        self.w = w
        self.card_w = 496
        self.card_h = 170
        self.add_card_app_id = []
        self.show_id_list = [] #保存首页发送过的软件id，为了处理一个界面没有进行界面切换时，每个应用仅保存一次
        self.init_ui()
        self.init_connect()
        # self.move_card = Move_Thread(self)

    def init_ui(self):
        self.resize(self.w,222+10)
        self.setFocusPolicy(Qt.NoFocus)
        self.setWindowFlags(Qt.FramelessWindowHint)
        self.setAttribute(Qt.WA_TranslucentBackground)

        self.titellabel = QLabel(self)
        self.titellabel.setFixedSize(600,26)
        self.titellabel.setAttribute(Qt.WA_TranslucentBackground)
        self.titellabel.setStyleSheet("QLabel{width: 94px;height: 33px;font-size: 24px;font-weight: 500;color: #303133;line-height: 33px;}")
        self.titellabel.setText(_("New Arrivals"))
        # self.titellabel.adjustSize()
        self.titellabel.move(10,0)

        self.more_btn = QPushButton(self)
        self.more_btn.setAttribute(Qt.WA_TranslucentBackground)
        self.more_btn.setFocusPolicy(Qt.NoFocus)
        self.more_btn.setStyleSheet("QPushButton{border: 0px;font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;}")
        self.more_btn.setText(_("more"))
        self.more_btn.adjustSize()
        self.more_btn.move(self.width()-self.more_btn.width(),self.titellabel.y()+int((self.titellabel.height()-self.more_btn.height())/2))
        self.more_btn.show()

        self.card_list_widget = QWidget(self)
        self.card_list_widget.setFixedSize(self.width(), self.card_h+20)
        self.card_list_widget.setWindowFlags(Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint)
        #self.card_list_widget.setAttribute(Qt.WA_TranslucentBackground)
        self.card_list_widget.setStyleSheet("QWidget{border:0px solid #000000;border-radius: 12px;background-color:transparent;}")
        self.card_list_widget.move(0, self.titellabel.height()+10)
        self.card_list_widget.show()

        self.card_list_widget2 = QWidget(self)
        self.card_list_widget2.setFixedSize(self.width(), self.card_h+20)
        self.card_list_widget2.setWindowFlags(Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint)
        #self.card_list_widget2.setAttribute(Qt.WA_TranslucentBackground)
        self.card_list_widget2.setStyleSheet("QWidget{border:0px solid #000000;border-radius: 12px;background-color:transparent;}")
        self.card_list_widget2.move(self.card_list_widget.width() , self.titellabel.height()+10)
        self.card_list_widget2.hide()

    def init_connect(self):
        # self.right_btn.clicked.connect(self.slot_click_right_btn)
        # self.left_btn.clicked.connect(self.slot_click_left_btn)
        self.ad_move_left_time.timeout.connect(self.slot_slide_left)
        self.start_move_time.timeout.connect(self.slot_click_right_btn)

    def get_lists(self, dblist, applist):
        self.dblist = []
        self.applist = []
        self.applist.extend(applist)
        self.dblist.extend(dblist)

    def add_ad_widget(self,appname, appmsg, ad_count,path):
        self.card_list_widget2.hide()
        if(appmsg != None):
            self.add_card_app_id.append(appmsg.app_id)
        itemwidget = New_APP_Card_Widget(self.card_w,self.card_h, appmsg, appname,path,self,self.card_list_widget)
        itemwidget2 = New_APP_Card_Widget(self.card_w,self.card_h, appmsg, appname,path,self,self.card_list_widget2)
        itemwidget.clicked_ad_card.connect(lambda: self.slot_click_ad_card(itemwidget.appmsg))
        itemwidget2.clicked_ad_card.connect(lambda: self.slot_click_ad_card(itemwidget2.appmsg))
        if (ad_count == 1):
            itemwidget.move(10, 10)
            itemwidget2.move(10, 10)
            itemwidget.resize(self.card_w, self.card_h)
            itemwidget2.resize(self.card_w, self.card_h)
            self.card_list_widget.move(0, self.titellabel.height()+10)
            self.card_list_widget.show()
            self.card_list_widget2.hide()
        else:
            if (ad_count == 0):
                itemwidget.move(10, 10)
                itemwidget2.move(10, 10)
                self.card_list_widget.setFixedWidth(itemwidget.width()+20)
                self.card_list_widget2.setFixedWidth(itemwidget.width()+20)
                self.card_list_widget2.move(self.card_list_widget.x() + self.card_list_widget.width(), self.titellabel.height()+10)
                self.card_list_widget.show()
            else:
                itemwidget.move((self.num) * (itemwidget.width() + 20) + 10, 10)
                itemwidget2.move(self.num * (itemwidget.width() + 20) + 10, 10)
                self.card_list_widget.setFixedWidth(self.num * (itemwidget.width() + 20) + itemwidget.width()+20)
                self.card_list_widget2.setFixedWidth(self.num * (itemwidget.width() + 20) + itemwidget.width()+20)
                self.card_list_widget2.move(self.card_list_widget.x() + self.card_list_widget.width(), self.titellabel.height()+10)
                self.card_list_widget.show()
        self.final_x_position = self.card_list_widget.x()
        self.final_x_position2 = self.card_list_widget2.x()
        itemwidget.show()
        itemwidget2.show()
        self.num = self.num + 1

    #
    #函数：添加卡片结束,显示界面
    #
    def add_card_over(self):
        if(Globals.NOWPAGE == PageStates.HOMEPAGE):
            if (self.card_list_widget.x() < self.width() or (self.card_list_widget.x() + self.card_list_widget.width() < self.width())):
                for card in self.card_list_widget.children():
                    if (card.x() + self.card_list_widget.x() <= self.width() and card.x() + self.card_list_widget.x() >= 0):
                        self.mw.save_new_arrivals_exposure(card.appmsg.app_id)
            if (self.card_list_widget2.x() < self.width() or (
                    self.card_list_widget2.x() + self.card_list_widget2.width() < self.width())):
                for card in self.card_list_widget2.children():
                    if (card.x() + self.card_list_widget2.x() <= self.width() and card.x() + self.card_list_widget2.x() >= 0):
                        self.mw.save_new_arrivals_exposure(card.appmsg.app_id)
        if (self.card_list_widget.width() > self.width()):
            self.card_list_widget.move(0, self.titellabel.height()+10)
            self.card_list_widget2.move(self.card_list_widget.x() + self.card_list_widget.width() , self.titellabel.height() + 10)
            self.start_move_time.start(4000)

    #
    # 函数：处理点击卡片
    #
    def slot_click_ad_card(self, app_msg):
        if(app_msg != None):
            self.mw.save_click_new_arrivals_card(app_msg.app_id)
            self.click_ad_card.emit(app_msg,True,ClickCard.NEWARRIVALS)
        else:
            self.mw.show_tip_widget(_("Native architecture does not support, please wait!"))
            self.mw.tip_widget.btn.clicked.connect(self.mw.slot_close_tipwidget)

    #
    # 函数：点击向右的箭头
    #
    def slot_click_right_btn(self):
        self.start_move_time.stop()
        x = self.card_list_widget.x()
        x2 = self.card_list_widget2.x()
        if (self.final_x_position == 0 and self.final_x_position2 == 0):
            self.final_x_position = x - ((self.card_w +20)*2 )
            self.final_x_position2 = x2 - ((self.card_w + 20)*2 )
            self.ad_move_left_time.start(1)
        else:
            self.final_x_position = self.final_x_position - ((self.card_w + 20)*2 )
            self.final_x_position2 = self.final_x_position2 - ((self.card_w + 20)*2 )
            self.ad_move_left_time.start(1)

    #
    # 函数：向左滑动
    #
    def slot_slide_left(self):
        if ((self.card_list_widget.x() + self.card_list_widget.width() == (self.card_w) * 2 + 40) or (self.card_list_widget.x() + self.card_list_widget.width() <= self.card_w * 2 + 40)):
            sub = self.card_list_widget2.x() - self.final_x_position2
            self.card_list_widget2.move(self.card_list_widget.x() + self.card_list_widget.width() , self.titellabel.height() + 10)
            self.final_x_position2 = self.card_list_widget2.x() - sub
        if (self.card_list_widget2.x() + self.card_list_widget.width() == (self.card_w) * 2 + 40) or (self.card_list_widget2.x() + self.card_list_widget2.width() <= self.card_w * 2 + 40):
            sub = self.card_list_widget.x() - self.final_x_position
            self.card_list_widget.move(self.card_list_widget2.x() + self.card_list_widget2.width() , self.titellabel.height() + 10)
            self.final_x_position = self.card_list_widget.x() - sub
        x = self.card_list_widget.x()
        x2 = self.card_list_widget2.x()
        if (x > self.final_x_position):
            self.ad_move_left_time.stop()
            self.card_list_widget.hide()
            self.card_list_widget2.hide()
            if (x - 50 <= self.final_x_position):
                self.card_list_widget.move(self.final_x_position, self.titellabel.height() + 10)
            else:
                self.card_list_widget.move(x - 50, self.titellabel.height() + 10)
            if (x2 - 50 <= self.final_x_position2):
                self.card_list_widget2.move(self.final_x_position2, self.titellabel.height()+10)
            else:
                self.card_list_widget2.move(x2 - 50, self.titellabel.height() + 10)
            self.card_list_widget.show()
            self.card_list_widget2.show()
            if(Globals.NOWPAGE == PageStates.HOMEPAGE):
                self.ad_move_left_time.start(1)
        else:
            self.ad_move_left_time.stop()
            if(Globals.NOWPAGE == PageStates.HOMEPAGE):
                if(Globals.NEED_SEND_EXPOSURE == True):
                    if(self.card_list_widget.x() < self.width() or (self.card_list_widget.x()+self.card_list_widget.width() < self.width())):
                        for card in self.card_list_widget.children():
                            if(card.x()+self.card_list_widget.x() <= self.width() and card.x()+self.card_list_widget.x() >= 0):
                                if(card.appmsg.app_id not in self.show_id_list):
                                    self.show_id_list.append(card.appmsg.app_id)
                                    self.mw.save_new_arrivals_exposure(card.appmsg.app_id)
                                else:
                                    b =  [val for val in self.add_card_app_id if val not in self.show_id_list]
                                    if(b == []):
                                        Globals.NEED_SEND_EXPOSURE = False
                                        self.show_id_list.clear()
                                    pass
                    if(self.card_list_widget2.x() < self.width() or (self.card_list_widget2.x()+self.card_list_widget2.width() < self.width())):
                        for card in self.card_list_widget2.children():
                            if(card.x()+self.card_list_widget2.x() <= self.width() and card.x()+self.card_list_widget2.x() >= 0):
                                if (card.appmsg.app_id not in self.show_id_list):
                                    self.show_id_list.append(card.appmsg.app_id)
                                    self.mw.save_new_arrivals_exposure(card.appmsg.app_id)
                                else:
                                    b =  [val for val in self.add_card_app_id if val not in self.show_id_list]
                                    if (b == []):
                                        Globals.NEED_SEND_EXPOSURE = False
                                        self.show_id_list.clear()
                                    pass
                else:
                    self.show_id_list.clear()
            else:
                self.show_id_list.clear()
            # self.final_x_position = 0
            # self.final_x_position2 = 0
            self.card_list_widget.show()
            self.card_list_widget2.show()
            self.start_move_time.start(4000)


class New_APP_Card_Widget(QFrame):
    appmsg = []
    image_url = ""
    clicked_ad_card = pyqtSignal()

    def __init__(self,card_w,card_h, appmsg,appname,path,new_arrivals_widget, parent=None):
        QFrame.__init__(self, parent)
        self.m_parent = parent
        self.new_arrivals_widget = new_arrivals_widget
        self.appmsg = appmsg
        self.appname = appname
        self.card_w = card_w
        self.card_h = card_h
        self.init_ui()
        self.get_ad_background(path)

    def init_ui(self):
        self.resize(self.card_w, self.card_h)
        self.setWindowFlags(Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint)
        self.setAttribute(Qt.WA_TranslucentBackground)
        self.setFocusPolicy(Qt.NoFocus)
        self.setStyleSheet("QFrame{border:0px solid  #ffff00;border-radius: 12px;background-color:transparent}")

        self.icon_label = QLabel(self)
        self.icon_label.resize(self.card_w, self.card_h)
        self.icon_label.move(0, 0)

        self.hover_widget = QWidget(self)
        self.hover_widget.setFixedSize(self.width(), self.card_h)
        self.hover_widget.setWindowFlags(Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint)
        # self.card_list_widget.setAttribute(Qt.WA_TranslucentBackground)
        self.hover_widget.setStyleSheet("QWidget{background: rgba(0, 0, 0, 0.1);border-radius: 12px;}")
        self.hover_widget.move(0, 0)
        self.hover_widget.hide()

    def get_ad_background(self,path):
        imagepath = get_new_arrivals_image(path)
        if (Globals.DEVICE == 1):
            self.icon_label.setStyleSheet("QLabel{border-radius: 12px;}")
            pix2 = QPixmap(imagepath)
            fitpixmap_userIcon = pix2.scaled(self.icon_label.width(), self.icon_label.height(), Qt.IgnoreAspectRatio, Qt.SmoothTransformation)
            fitpixmap_userIcon = self.PixmapToRound(fitpixmap_userIcon, self.width(), self.height())
            self.icon_label.setPixmap(fitpixmap_userIcon)
        else:
            self.icon_label.setStyleSheet("QLabel{border-image:url('" + imagepath + "');border-radius: 12px;}")
        self.icon_label.show()
        self.show()

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
        if((self.m_parent == self.new_arrivals_widget.card_list_widget and self.m_parent.x() == self.new_arrivals_widget.final_x_position) or (self.m_parent == self.new_arrivals_widget.card_list_widget2 and self.m_parent.x() == self.new_arrivals_widget.final_x_position2)):
            self.setGeometry(self.x(), self.y(), self.width(), self.height())
            self.shadow = QGraphicsDropShadowEffect(self)
            self.shadow.setOffset(0, 0)
            self.shadow.setColor(QColor(qRgba(149, 149, 149, 0)))
            self.shadow.setBlurRadius(12)
            self.setGraphicsEffect(self.shadow)
        self.hover_widget.show()

    #
    # 函数：鼠标离开事件
    #

    def leaveEvent(self, event):
        self.status = BUTTON_LEAVE
        if ((self.m_parent == self.new_arrivals_widget.card_list_widget and self.m_parent.x() == self.new_arrivals_widget.final_x_position) or (self.m_parent == self.new_arrivals_widget.card_list_widget2 and self.m_parent.x() == self.new_arrivals_widget.final_x_position2)):
            self.resize(self.width(), self.height())
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
            self.clicked_ad_card.emit()
        else:
            event.ignore()
