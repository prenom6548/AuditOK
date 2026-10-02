#!/usr/bin/python3
# -*- coding: utf-8 -*-

import os
import math
import time
from PyQt5.QtGui import *
from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
from models.application import *
from models.apkinfo import *
from utils.commontools import *
from models.enums import CACHE_DIR,BUTTON_PRESSED,BUTTON_ENTER,BUTTON_LEAVE,BUTTON_DISABLE,AppMsg,PageStates,ClickCard

class Advertisement_Widget(QFrame):
    ad_num = 0
    click_ad_card = pyqtSignal(Application,bool,int)
    click_ad_card_apk = pyqtSignal(ApkInfo,bool,int)
    click_none_app_ad_card = pyqtSignal()
    ad_move_left_time = QTimer()
    ad_move_right_time = QTimer()
    final_x_position = 0
    final_x_position2 = 0
    def __init__(self,w,h,w1,w2,mw,is_homepage = False,parent=None):
        QFrame.__init__(self, parent)
        self.dblist = []
        self.applist = []
        self.w = w
        self.h = h
        self.w1 = w1
        self.w2 = w2
        self.re_ad_w = w1
        self.re_ad_w2 = w2
        self.m_parent = parent
        self.is_homepage = is_homepage
        # self.w1 = int((self.width() - 20) * (self.w1 / (self.w - 20)))
        # self.w2 = int((self.width() - 20) * (self.w2 / (self.w - 20)))
        self.appmsg = None
        self.mw = mw
        self.init_ui()


    def init_ui(self):
        if(self.is_homepage):
            self.resize(self.w,self.h+20)
        else:
            self.resize(self.w,self.h)
        self.setWindowFlags(Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint)
        self.setAttribute(Qt.WA_TranslucentBackground)
        self.setFocusPolicy(Qt.NoFocus)
        self.setStyleSheet("QFrame{border:0px solid #ff0000;border-radius: 12px;background-color:transparent;}")

        self.ad_list_widget = QWidget(self)
        self.ad_list_widget.setWindowFlags(Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint)
        self.ad_list_widget.setAttribute(Qt.WA_TranslucentBackground)
        self.ad_list_widget.setStyleSheet("QFrame{border:0px solid #000000;border-radius: 12px;background-color:transparent;}")
        if (self.is_homepage):
            self.ad_list_widget.resize(self.w1+20,self.h+20)
            self.ad_list_widget.move(0, 0)
        else:
            self.ad_list_widget.resize(self.w1, self.h)
            self.ad_list_widget.move(0, 0)
        self.ad_list_widget.show()

        self.ad_list_widget2 = QWidget(self)
        self.ad_list_widget2.setWindowFlags(Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint)
        self.ad_list_widget2.setAttribute(Qt.WA_TranslucentBackground)
        self.ad_list_widget2.setStyleSheet("QFrame{border:0px solid #000000;border-radius: 12px;background-color:transparent;}")
        if (self.is_homepage):
            self.ad_list_widget2.resize(self.w2+20,self.h+20)
            self.ad_list_widget2.move(self.ad_list_widget.width(), 0)
        else:
            self.ad_list_widget2.resize(self.w2, self.h+20)
            self.ad_list_widget2.move(self.ad_list_widget.width() + 20, 0)

    def re_move(self,proportion):
        if (self.is_homepage):
            w1 = int((self.width() -20)*(self.w1/(self.w-20)))
            w2 = int((self.width() - 20) * (self.w2 / (self.w - 20)))
            self.re_ad_w = w1
            self.re_ad_w2 = w2
            self.ad_list_widget.resize(w1+20,self.height())
            self.ad_list_widget2.resize(w2+20,self.height())
            self.ad_list_widget.move(0, 0)
            self.ad_list_widget2.move(self.ad_list_widget.width() , 0)
            ads = self.ad_list_widget.children()
            for i in ads:
                i.resize(w1-20, self.height()-20)
                i.get_ad_background()
                i.move(10,10)
            ads = self.ad_list_widget2.children()
            for i in ads:
                i.resize(w2-20, self.height()-20)
                i.get_ad_background()
                i.move(10,10)
            self.ad_list_widget2.move(self.ad_list_widget.x()+self.ad_list_widget.width(),0)
        else:
            w1 = int((self.width() - 20) * (self.w1 / (self.w - 20)))
            w2 = int((self.width() - 20) * (self.w2 / (self.w - 20)))
            self.re_ad_w = w1
            self.re_ad_w2 = w2
            self.ad_list_widget.resize(w1, self.height())
            self.ad_list_widget2.resize(w2, self.height())
            self.ad_list_widget.move(0, 0)
            self.ad_list_widget2.move(self.ad_list_widget.width() + 20, 0)
            ads = self.ad_list_widget.children()
            for i in ads:
                i.resize(w1, self.height())
                i.get_ad_background()
                i.move(0, 0)
            ads = self.ad_list_widget2.children()
            for i in ads:
                i.resize(w2, self.height())
                i.get_ad_background()
                i.move(0, 0)
            self.ad_list_widget2.move(self.ad_list_widget.x() + self.ad_list_widget.width() + 20, 0)

    def get_lists(self,dblist,applist):
        self.applist = applist
        self.dblist = dblist

    def add_ad_widget(self,image_url,appmsg,ad_count,num):
        if(self.is_homepage):
            if(num == 0):
                itemwidget = Ad_Widget(self.re_ad_w, self.height()-20, image_url, appmsg, False,self.is_homepage,self.ad_list_widget)
                itemwidget.clicked_ad_card.connect(lambda :self.slot_click_ad_card(itemwidget.appmsg))
                itemwidget.move(10, 10)
                itemwidget.resize(self.width() - 60-20, self.height()-20)
                self.ad_list_widget.move(0, 0)
                self.ad_list_widget.show()
                self.ad_list_widget2.show()
                # self.re_move(1)
            else:
                itemwidget = Ad_Widget(self.re_ad_w2, self.height()-20, image_url, appmsg,True, self.is_homepage,self.ad_list_widget2)
                itemwidget.clicked_ad_card.connect(lambda :self.slot_click_ad_card(itemwidget.appmsg))
                itemwidget.move(10, 10)
                itemwidget.resize(self.width() - 60-20, self.height()-20)
                self.ad_list_widget.move(0, 0)
                self.ad_list_widget.show()
                self.ad_list_widget2.show()
                # self.re_move(1)
        else:
            if (num == 0):
                itemwidget = Ad_Widget(self.re_ad_w, self.height(), image_url, appmsg, False,self.is_homepage, self.ad_list_widget)
                itemwidget.clicked_ad_card.connect(lambda: self.slot_click_ad_card(itemwidget.appmsg))
                itemwidget.move(0, 0)
                itemwidget.resize(self.width() - 60, self.height())
                self.ad_list_widget.move(0, 0)
                self.ad_list_widget.show()
                self.ad_list_widget2.show()
                # self.re_move(1)
            else:
                itemwidget = Ad_Widget(self.re_ad_w2, self.height(), image_url, appmsg, True, self.is_homepage,self.ad_list_widget2)
                itemwidget.clicked_ad_card.connect(lambda: self.slot_click_ad_card(itemwidget.appmsg))
                itemwidget.move(0, 0)
                itemwidget.resize(self.width() - 60, self.height())
                self.ad_list_widget.move(0, 0)
                self.ad_list_widget.show()
                self.ad_list_widget2.show()
                # self.re_move(1)

    def change_ad_image(self,image_url,appmsg,ad_count,num):
        if(num == 0):
            for ad_widget in self.ad_list_widget.children():
                ad_widget.image_url = image_url
                ad_widget.appmsg = appmsg
                ad_widget.ad_background()
        else:
            if (num == 0):
                for ad_widget in self.ad_list_widget2.children():
                    ad_widget.image_url = image_url
                    ad_widget.appmsg = appmsg
                    ad_widget.ad_background()

    #
    # 函数：处理点击卡片
    #
    def slot_click_ad_card(self ,app_msg):
        if(app_msg != None):
            if(Globals.NOWPAGE == PageStates.HOMEPAGE):
                self.mw.save_click_ad_card(app_msg.app_id,0)
            elif(Globals.NOWPAGE == PageStates.CAREGORYPAGE):
                for i in Globals.ALL_CATEGORIES:
                    if (i["name"] == self.mw.detailwidget.category_widget.type_name_label.text()):
                        self.mw.save_click_ad_card(app_msg.app_id,i["id"])
                        break
                    elif (i["displayname"] == self.mw.detailwidget.category_widget.type_name_label.text()):
                        self.mw.save_click_ad_card(app_msg.app_id, i["id"])
                        break
            if(type(app_msg) == Application):
                self.click_ad_card.emit(app_msg,True,ClickCard.AD)
            else:
                self.click_ad_card_apk.emit(app_msg,True,ClickCard.AD)
        # else:
            # self.click_none_app_ad_card.emit()


class Ad_Widget(QFrame):
    big_width = 1263
    small_width = 578
    get_height = 384
    appmsg = []
    image_url = ""
    clicked_ad_card = pyqtSignal()
    def __init__(self,w,h,image_url,appmsg,is_big,is_homepage,parent=None):
        QFrame.__init__(self, parent)
        self.appmsg = appmsg
        self.w = w
        self.h = h
        self.is_homepage = is_homepage
        self.is_big = is_big
        self.image_url = image_url
        self.init_ui()
        self.m_parent = parent
        # print("start change imag",time.strftime("%Y-%m-%dT%H:%M:%SZ", time.localtime()))
        self.ad_background()
        # print("end change imag", time.strftime("%Y-%m-%dT%H:%M:%SZ", time.localtime()))
        self.show()

    def init_ui(self):
        self.resize(self.w,self.h)
        self.setWindowFlags(Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint)
        self.setAttribute(Qt.WA_TranslucentBackground)
        self.setFocusPolicy(Qt.NoFocus)
        self.setStyleSheet("QFrame{border:0px solid  #ffffff;border-radius: 12px;background-color:#FFFFFF;}")

        self.icon_label = QLabel(self)
        # self.icon_label.setAttribute(Qt.WA_TranslucentBackground)
        self.icon_label.resize(self.w,self.h)
        self.icon_label.move(0,0)

        self.hover_widget = QWidget(self)
        self.hover_widget.resize(self.w,self.h)
        self.hover_widget.setWindowFlags(Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint)
        # self.card_list_widget.setAttribute(Qt.WA_TranslucentBackground)
        self.hover_widget.setStyleSheet("QWidget{background: rgba(0, 0, 0, 0.1);border-radius: 12px;}")
        self.hover_widget.move(0, 0)
        self.hover_widget.hide()


    def ad_background(self):
        self.icon_label.resize(self.width(), self.height())
        self.icon_label.move(0, 0)
        self.hover_widget.setFixedSize(self.width(), self.height())
        self.hover_widget.move(0, 0)
        #imagepath = os.path.join(CACHE_DIR,self.image_url)
        if(self.appmsg != None):
            if(self.image_url!= None):
                if(os.path.isfile(self.image_url) == False):
                    file_path_list = self.image_url.split("/")
                    file_name = file_path_list[len(file_path_list)-1]
                    self.image_url = get_ads_image(file_name)
            # self.icon_label.setStyleSheet(
            #     "QLabel{border-radius: 12px;border-image: url('" + self.image_url + "') 0 0 0 0 stretch strectch;}")
            # self.icon_label.setStyleSheet(
            #     "QLabel{border-radius: 12px;border-image: url('" + self.image_url + "');}")
            if(self.image_url != None):
                self.icon_label.setStyleSheet("QLabel{border-radius: 12px;}")
                if(Globals.DEVICE == 1):
                    self.pix2 = QPixmap(self.image_url)
                    if(self.is_big):
                        #IgnoreAspectRatio:忽略横纵比    SmoothTransformation：平滑变换
                        self.fitpixmap_userIcon = self.pix2.scaled(self.width()+2, self.height()+2, Qt.IgnoreAspectRatio, Qt.SmoothTransformation)
                        self.fitpixmap_userIcon = self.PixmapToRound(self.fitpixmap_userIcon, self.width(), self.height())
                    else:
                        self.fitpixmap_userIcon = self.pix2.scaled(self.width()+2, self.height()+2,
                                                                   Qt.IgnoreAspectRatio, Qt.SmoothTransformation)
                        self.fitpixmap_userIcon = self.PixmapToRound(self.fitpixmap_userIcon, self.width(), self.height())
                    self.icon_label.setPixmap(self.fitpixmap_userIcon)
                else:
                    self.icon_label.setStyleSheet("QLabel{border-image:url('"+self.image_url+"');border-radius: 12px;}")
            else:
                # self.setStyleSheet("QFrame{border:0px solid  #ffffff;border-radius: 12px;background-color:#F4F4F4;}")
                self.icon_label.setStyleSheet("QLabel{background-image:url(./res/ad_default.png);border-radius: 12px;}")
            # # self.icon_label.show()
            # # self.show()
        else:
            self.image_url = None
            self.icon_label.setStyleSheet("QLabel{background-image:url(./res/ad_default.png);background-repeat:no-repeat;background-position:center;background-color:#F4F4F4;border-radius: 12px;}")
        self.icon_label.setScaledContents(True)


    def get_ad_background(self):
        self.icon_label.resize(self.width(), self.height())
        self.icon_label.move(0, 0)
        self.hover_widget.setFixedSize(self.width(), self.height())
        self.hover_widget.move(0, 0)
        # imagepath = os.path.join(CACHE_DIR,self.image_url)
        if(self.image_url != None):
            if (os.path.isfile(self.image_url) == False):
                file_path_list = self.image_url.split("/")
                file_name = file_path_list[len(file_path_list) - 1]
                self.image_url = get_ads_image(file_name)
            # self.icon_label.setStyleSheet(
            #     "QLabel{border-radius: 12px;border-image: url('" + self.image_url + "') 0 0 0 0 stretch strectch;}")
            if (Globals.DEVICE == 1):
                self.icon_label.setStyleSheet("QLabel{border-radius: 12px;}")
                self.pix2 = QPixmap(self.image_url)
                self.fitpixmap_userIcon = self.pix2.scaled(self.icon_label.width(), self.icon_label.height(),
                                                           Qt.IgnoreAspectRatio, Qt.SmoothTransformation)
                self.fitpixmap_userIcon = self.PixmapToRound(self.fitpixmap_userIcon, self.width(), self.height())
                self.icon_label.setPixmap(self.fitpixmap_userIcon)
                self.icon_label.setScaledContents(True)
            else:
                self.icon_label.setStyleSheet("QLabel{border-image:url('" + self.image_url + "');border-radius: 12px;}")
            # palette = QPalette()
            # pix = QPixmap(self.image_url)
            # pix = pix.scaled(self.width(), self.height())
            # palette.setBrush(QPalette.Background, QBrush(pix))
            # self.icon_label.setPalette(palette)


    def PixmapToRound(self,src,x,y):
        pixmapa = QPixmap(src)
        pixmap = QPixmap(x, y)
        # pixmap.fill(Qt.transparent)
        pixmap.fill(Qt.transparent)
        painter = QPainter(pixmap)
        painter.setRenderHint(QPainter.Antialiasing, True)
        #QPainter.Antialiasing:抗锯齿
        #QPainter.SmoothPixmapTransform：在对图片进行缩放时启用线性插值算法而不是最邻近算法
        # painter.setRenderHints(QPainter.Antialiasing | QPainter.SmoothPixmapTransform)
        path = QPainterPath()
        rect = QRectF(0,0,x,y)
        path.addRoundedRect(rect, 12.0, 12.0)
        painter.setClipPath(path)
        painter.drawPixmap(0, 0, x, y, pixmapa)

        return pixmap

    def paintEvent(self, QPaintEvent):
        paint = QPainter(self)
        paint.setRenderHint(QPainter.Antialiasing)

# 鼠标进入事件

    def enterEvent(self, event):
        self.status = BUTTON_ENTER
        if(self.appmsg != None):
            self.hover_widget.show()
            if(self.is_homepage):
                self.setGeometry(int(self.x()), int(self.y()), self.width(), self.height())
                self.shadow = QGraphicsDropShadowEffect(self)
                self.shadow.setOffset(0, 0)
                self.shadow.setColor(QColor(qRgba(149, 149, 149, 0)))
                self.shadow.setBlurRadius(12)
                self.setGraphicsEffect(self.shadow)

    #
    # 函数：鼠标离开事件
    #

    def leaveEvent(self, event):
        self.status = BUTTON_LEAVE
        if (self.is_homepage):
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
