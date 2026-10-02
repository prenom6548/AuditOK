#!/usr/bin/python3
# -*- coding: utf-8 -*-

from PyQt5.QtGui import *
from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
from PyQt5 import QtCore, QtGui
from ui.starwidget import StarWidget
from ui.messagebox import MessageBox
from models.enums import Signals

# from ui.loadingdiv import *
import sip
import math
import gettext

gettext.bindtextdomain("ubuntu-kylin-software-center", "/usr/share/locale")
gettext.textdomain("ubuntu-kylin-software-center")
_ = gettext.gettext
from models.application import *
from models.enums import setLongTextToElideFormat


# from models.enums import CACHE_DIR,BUTTON_PRESSED,BUTTON_ENTER,BUTTON_LEAVE,BUTTON_DISABLE,AppMsg
#


class Screenshot(QPushButton):
    def __init__(self,card_width,card_height,num,app_detail,parent=None):
        super(Screenshot, self).__init__(parent)
        self.card_width = card_width
        self.card_height = card_height
        self.num = num
        self.app_detail = app_detail
        self.resize(card_width, card_height)
        self.hide()
        self.screen_outer_frame = QLabel(self)
        self.screen_outer_frame.resize(self.width()-15*2, self.height()-15*2)
        # self.screen_outer_frame1.move(51, 79)
        self.screen_outer_frame.move(15,15)
        self.screen_outer_frame.setStyleSheet(
            "QLabel{border-radius: 0px;border-image: url(./res/screenshot_2.png) 0 0 0 0 stretch strectch;border-radius: 0px;background-color:transparent;}")
        self.screen_outer_frame.hide()
        self.screen_shot = QLabel(self)
        self.screen_shot.resize(self.screen_outer_frame.width()-12*2, self.screen_outer_frame.height()-12*2)
        self.screen_shot.move(self.screen_outer_frame.x() + 12, self.screen_outer_frame.y() + 12)
        # self.screen_shot1.move(58, 86)
        self.setFocusPolicy(Qt.NoFocus)

        self.screen_shot.hide()
        self.surface_color = QLabel(self)
        self.surface_color.resize(self.width(), 78)
        # self.surface_color1.move(0, 181)
        if (Globals.THEME_COLOR == 0):
            self.surface_color.setStyleSheet(
                "QLabel{border-image: url(./res/surface_color.png) 0 0 0 0 stretch strectch;border-radius: 0px;background-color:transparent;}")
        else:
            self.surface_color.setStyleSheet(
                "QLabel{border-image: url(./res/surface_color_dark.png) 0 0 0 0 stretch strectch;border-radius: 0px;background-color:transparent;}")

        # self.surface_color1.setStyleSheet("QLabel{background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(247,251,251,0), stop:1 #3790FA);border-bottom-left-radius:12px;border-bottom-right-radius:12px;opacity: 0.49;}")
        self.surface_color.move(0, self.height() - self.surface_color.height())
        self.surface_color.hide()

        self.defualt_photo = QLabel(self)
        self.defualt_photo.setFixedSize(30, 30)
        self.defualt_photo.move(int((self.width() - self.defualt_photo.width()) / 2),
                                 int((self.height() - self.defualt_photo.height()) / 2))

        self.defualt_photo.setStyleSheet(
            "QLabel{background-image:url(./res/defualt_screen.png);border-radius: 0px;background-color:transparent;}")
        self.defualt_photo.hide()
        self.change_color()

    def change_color(self):
        if(Globals.THEME_COLOR == 0):
            self.setStyleSheet("QPushButton{background-color: #F4F4F4;border-radius: 20px;}")
            self.surface_color.setStyleSheet(
                "QLabel{border-image: url(./res/surface_color.png) 0 0 0 0 stretch strectch;border-radius: 0px;background-color:transparent;}")
        else:
            self.setStyleSheet(
                "QPushButton{background-color: rgba(61,61,65,0.5);border-radius: 20px;}")
            self.surface_color.setStyleSheet(
                "QLabel{border-image: url(./res/surface_color_dark.png) 0 0 0 0 stretch strectch;border-radius: 0px;background-color:transparent;}")

    def signal_connect(self):
        self.clicked.connect(lambda: self.app_detail.show_screen_detail(self.num))



class Shot_Detail(QFrame, Signals):
    ad_move_left_time = QTimer()
    ad_move_right_time = QTimer()
    final_x_position = 0
    clicked_order = {}
    shot_pic = []
    pic_order = 0
    max_pic_num = 20

    def __init__(self, shot_phone,w,y,app_detail, parent = None):
        QWidget.__init__(self,parent)
        self.resize(w,y)
        self.m_parent = parent
        self.app_detail = app_detail
        self.setStyleSheet("QFrame{background-color:rgba(0, 0, 0, 0.7);border-bottom-left-radius:6px;border-bottom-right-radius:6px; }")
        self.left_btn = QPushButton(self)
        self.left_btn.setFocusPolicy(Qt.NoFocus)
        self.left_btn.resize(50, 70)
        # self.i = 0
        self.left_btn.move(100, int((self.height() - self.left_btn.height()) / 2))
        # self.left_btn.hide()
        self.left_btn.setEnabled(False)

        self.right_btn = QPushButton(self)
        self.right_btn.setFocusPolicy(Qt.NoFocus)
        self.right_btn.resize(50, 70)
        self.right_btn.move(self.width()-self.right_btn.width()-100,int((self.height() - self.right_btn.height()) / 2))

        self.shot_pic = shot_phone
        screen_count = len(self.shot_pic)
        # self.app_detail.screen_shot1.show()
        self.photo_widget = QWidget(self)
        self.photo_widget.setStyleSheet("QWidget{border:1px ;border-radius:12px;background:transparent;}")
        self.photo_widget.move(self.left_btn.x()+self.left_btn.width()+20, 76)
        self.photo_widget.resize(self.width()-2*(self.left_btn.x()+self.left_btn.width()+20), self.height()-2*76)

        self.photo_widget_slide = QWidget(self.photo_widget)
        self.photo_widget_slide.setStyleSheet("QWidget{border:1px ;border-radius:12px;background:transparent;}")

        # self.photo_widget_slide.resize(1000, 680)
        self.close_btn = QPushButton(self)

        self.close_btn.setFocusPolicy(Qt.NoFocus)
        self.close_btn.setStyleSheet("QPushButton{background-color:transparent;border-radius:15px;image: url(./res/close_shot.png);}")

        self.close_btn.setFixedSize(40, 40)
        self.close_btn.move(self.width()-self.close_btn.width()-30,30)
        self.close_btn.clicked.connect(self.close_widget)

        self.save_pic = []
        for i in range(self.max_pic_num):
            self.photo = Photo("",self.photo_widget.width(),self.photo_widget.height(),self.photo_widget_slide)
            self.photo.move(self.photo_widget.width()*i, 0)
            self.save_pic.append(self.photo)

        # 更新截图
        self.shot_pic_change(self.shot_pic)

        # # 点击截图
        # self.shot_pic_click(picture_order)

        self.right_btn.clicked.connect(self.slot_click_right_btn)
        self.left_btn.clicked.connect(self.slot_click_left_btn)
        self.ad_move_left_time.timeout.connect(self.slot_slide_left)
        self.ad_move_right_time.timeout.connect(self.slot_slide_right)

    def re_move(self,shot_screen_list):
        self.left_btn.move(100, int((self.height() - self.left_btn.height()) / 2))
        self.right_btn.move(self.width()-self.right_btn.width()-100,int((self.height() - self.right_btn.height()) / 2))
        self.close_btn.move(self.width() - self.close_btn.width() - 30, 30)
        self.photo_widget.resize(self.width() - 2 * (self.left_btn.x() + self.left_btn.width() + 20),self.height() - 2 * 76)
        self.photo_widget.move(self.left_btn.x() + self.left_btn.width() + 20, 76)
        photos = self.photo_widget_slide.children()
        self.photo_widget_slide.resize(self.photo_widget.width() * len(photos), self.photo_widget.height())
        # self.photo_widget_slide.move(0, 0)
        num = 0
        for photo in photos:
            photo.resize(self.photo_widget.width(),self.photo_widget.height())
            photo.re_move()
            photo.move(self.photo_widget.width()*num,int((self.photo_widget_slide.height()-photo.height())/2))
            if(len(self.save_pic) >= num):
                photo.resize_image()
                # photo.shot_photo_change(shot_screen_list[num])
                num = num +1
        self.shot_pic_click(self.pic_order)
        # if(self.pic_order == 1):
        #     self.left_btn.setEnabled(False)
        #     self.left_btn.setStyleSheet(
        #         "QPushButton{background-color:transparent;border-radius:20px;image: url(./res/Bleft_false.png);}")
        #     self.right_btn.setEnabled(True)
        #     self.right_btn.setStyleSheet(
        #         "QPushButton{background-color:transparent;border-radius:20px;image: url(./res/Bright_default.png);}QPushButton:hover{background-color:transparent;border-radius:20px;image: url(./res/Bright_clicked.png);}QPushButton:pressed{background-color:transparent;border-radius:20px;image: url(./res/Bright_clicked.png);}")
        # elif

    def close_widget(self):
        try:
            # self.i += 1
            # print(self.i)
            self.left_btn.setEnabled(True)
            self.right_btn.setEnabled(True)
            self.hide()
            self.m_parent.hide()
        except Exception:
            pass

    # 函数：点击右边按钮
    def slot_click_right_btn(self):

        if (len(self.shot_pic) > 1):

            self.pic_order = self.pic_order + 1
            self.left_btn.setEnabled(True)
            self.left_btn.setStyleSheet(
                "QPushButton{background-color:transparent;border-radius:20px;image: url(./res/Bleft_default.png);}QPushButton:hover{background-color:transparent;border-radius:20px;image: url(./res/Bleft_clicked.png);}QPushButton:pressed{background-color:transparent;border-radius:20px;image: url(./res/Bleft_clicked.png);}")

            x = self.photo_widget_slide.x()
            if (self.final_x_position == 0):

                self.final_x_position =self.photo_widget_slide.x()-self.photo_widget.width()
                Globals.POS = self.final_x_position
                self.ad_move_left_time.start(1)
            else:

                self.final_x_position = self.photo_widget_slide.x() - self.photo_widget.width()
                Globals.POS = self.final_x_position
                self.ad_move_left_time.start(1)

    #
    # #
    # 函数：向左滑动:1.左边滑动。2.滑到终点按钮置灰。
    def slot_slide_left(self):
        self.final_x_position = Globals.POS
        self.right_btn.setEnabled(False)
        self.right_btn.setStyleSheet(
            "QPushButton{background-color:transparent;border-radius:20px;image: url(./res/Bright_false.png);}")
        self.left_btn.setEnabled(False)
        self.left_btn.setStyleSheet(
            "QPushButton{background-color:transparent;border-radius:20px;image: url(./res/Bleft_false.png);}")

        x = self.photo_widget_slide.x()
        if(x-100 < self.final_x_position):
            self.photo_widget_slide.move(self.final_x_position, 0)
        else:
            self.photo_widget_slide.move(x - 100, 0)
        self.ad_move_left_time.start(1)
        x = self.photo_widget_slide.x()
        if (x % self.final_x_position ==0) :
            self.ad_move_left_time.stop()
            self.right_btn.setEnabled(True)
            self.right_btn.setStyleSheet(
                "QPushButton{background-color:transparent;border-radius:20px;image: url(./res/Bright_default.png);}QPushButton:hover{background-color:transparent;border-radius:20px;image: url(./res/Bright_clicked.png);}QPushButton:pressed{background-color:transparent;border-radius:20px;image: url(./res/Bright_clicked.png);}")

            self.left_btn.setEnabled(True)
            self.left_btn.setStyleSheet(
                "QPushButton{background-color:transparent;border-radius:20px;image: url(./res/Bleft_default.png);}QPushButton:hover{background-color:transparent;border-radius:20px;image: url(./res/Bleft_clicked.png);}QPushButton:pressed{background-color:transparent;border-radius:20px;image: url(./res/Bleft_clicked.png);}")

            # self.final_x_position = 0
            # if(len(self.shot_pic)>4):
            #     if (x == -self.photo_widget.width()*(4-1)):
            #
            #         self.right_btn.setEnabled(False)
            #         self.right_btn.setStyleSheet("QPushButton{background-color:transparent;border-radius:20px;image: url(./res/Bright_false.png);}")
            # else:
            if (x == -self.photo_widget.width() * (len(self.shot_pic) - 1)):
                self.right_btn.setEnabled(False)
                self.right_btn.setStyleSheet("QPushButton{background-color:transparent;border-radius:20px;image: url(./res/Bright_false.png);}")

    #
    # 函数：点击向左的箭头
    #
    def slot_click_left_btn(self):

        self.pic_order = self.pic_order -1
        self.right_btn.setEnabled(True)
        self.right_btn.setStyleSheet(
            "QPushButton{background-color:transparent;border-radius:20px;image: url(./res/Bright_default.png);}QPushButton:hover{background-color:transparent;border-radius:20px;image: url(./res/Bright_clicked.png);}QPushButton:pressed{background-color:transparent;border-radius:20px;image: url(./res/Bright_clicked.png);}")

        x = self.photo_widget_slide.x()

        if (self.final_x_position == 0):
            self.final_x_position =  self.photo_widget_slide.x()+ self.photo_widget.width()
            Globals.POS = self.final_x_position
            self.ad_move_right_time.start(0.1)
        else:
            self.final_x_position = self.photo_widget_slide.x() + self.photo_widget.width()
            Globals.POS = self.final_x_position
            self.ad_move_right_time.start(0.1)

    #
    # 函数向右滑动

    def slot_slide_right(self):
        self.ad_move_left_time.stop()
        self.final_x_position = Globals.POS
        self.right_btn.setEnabled(False)
        self.right_btn.setStyleSheet(
            "QPushButton{background-color:transparent;border-radius:20px;image: url(./res/Bright_false.png);}")
        self.left_btn.setEnabled(False)
        self.left_btn.setStyleSheet(
            "QPushButton{background-color:transparent;border-radius:20px;image: url(./res/Bleft_false.png);}")
        x = self.photo_widget_slide.x()
        if(x+100 > self.final_x_position):
            self.photo_widget_slide.move(self.final_x_position, 0)
        else:
            self.photo_widget_slide.move(x + 100, 0)
        self.ad_move_right_time.start(1)
        x = self.photo_widget_slide.x()
        if (x == self.final_x_position):
            self.ad_move_right_time.stop()
            self.left_btn.setEnabled(True)
            self.left_btn.setStyleSheet(
                "QPushButton{background-color:transparent;border-radius:20px;image: url(./res/Bleft_default.png);}QPushButton:hover{background-color:transparent;border-radius:20px;image: url(./res/Bleft_clicked.png);}QPushButton:pressed{background-color:transparent;border-radius:20px;image: url(./res/Bleft_clicked.png);}")

            self.right_btn.setEnabled(True)
            self.right_btn.setStyleSheet(
                "QPushButton{background-color:transparent;border-radius:20px;image: url(./res/Bright_default.png);}QPushButton:hover{background-color:transparent;border-radius:20px;image: url(./res/Bright_clicked.png);}QPushButton:pressed{background-color:transparent;border-radius:20px;image: url(./res/Bright_clicked.png);}")

            # self.final_x_position = 0
        if (x == 0):
            self.left_btn.setEnabled(False)
            self.left_btn.setStyleSheet(
                "QPushButton{background-color:transparent;border-radius:20px;image: url(./res/Bleft_false.png);}")

    # 截图改变
    def shot_pic_change(self, shot_photo):
        self.shot_pic = shot_photo
        # if(range(len(self.shot_pic) >4)):
        #     range_num = 4
        # else:
        range_num = len(self.shot_pic)
        for i in range(range_num):
            self.save_pic[i].shot_photo_change(self.shot_pic[i])
            self.save_pic[i].show()

        for i in range(len(self.shot_pic), self.max_pic_num):
            self.save_pic[i].hide()

        self.photo_widget_slide.resize(self.photo_widget.width() * len(self.shot_pic), self.photo_widget.height())

    # 截图点击
    def shot_pic_click(self, pic_order):
        self.pic_order = pic_order
        self.app_detail.m_parent.mw.hide_tip_widget()
        if pic_order == 1:
            self.photo_widget_slide.move(0, 0)
            self.left_btn.setEnabled(False)
            self.left_btn.setStyleSheet(
                "QPushButton{background-color:transparent;border-radius:20px;image: url(./res/Bleft_false.png);}")
            if len(self.shot_pic) >1:
                self.right_btn.setStyleSheet(
                    "QPushButton{background-color:transparent;border-radius:20px;image: url(./res/Bright_default.png);}QPushButton:hover{background-color:transparent;border-radius:20px;image: url(./res/Bright_clicked.png);}QPushButton:pressed{background-color:transparent;border-radius:20px;image: url(./res/Bright_clicked.png);}")
            else:
                self.right_btn.setStyleSheet(
                    "QPushButton{background-color:transparent;border-radius:20px;image: url(./res/Bright_false.png);}")

        elif pic_order == len(self.shot_pic):
            self.photo_widget_slide.move(-self.photo_widget.width() * (pic_order - 1), 0)
            self.left_btn.setEnabled(True)
            self.right_btn.setEnabled(False)
            self.left_btn.setStyleSheet(
                "QPushButton{background-color:transparent;border-radius:20px;image: url(./res/Bleft_default.png);}QPushButton:hover{background-color:transparent;border-radius:20px;image: url(./res/Bleft_clicked.png);}QPushButton:pressed{background-color:transparent;border-radius:20px;image: url(./res/Bleft_clicked.png);}")
            self.right_btn.setStyleSheet(
                "QPushButton{background-color:transparent;border-radius:20px;image: url(./res/Bright_false.png);}")

        else:
            self.photo_widget_slide.move(-self.photo_widget.width() * (pic_order - 1), 0)
            self.left_btn.setStyleSheet(
                "QPushButton{background-color:transparent;border-radius:20px;image: url(./res/Bleft_default.png);}QPushButton:hover{background-color:transparent;border-radius:20px;image: url(./res/Bleft_clicked.png);}QPushButton:pressed{background-color:transparent;border-radius:20px;image: url(./res/Bleft_clicked.png);}")
            if len(self.shot_pic) > 2:
                self.right_btn.setStyleSheet(
                    "QPushButton{background-color:transparent;border-radius:20px;image: url(./res/Bright_default.png);}QPushButton:hover{background-color:transparent;border-radius:20px;image: url(./res/Bright_clicked.png);}QPushButton:pressed{background-color:transparent;border-radius:20px;image: url(./res/Bright_clicked.png);}")
            else:
                self.right_btn.setStyleSheet(
                    "QPushButton{background-color:transparent;border-radius:20px;image: url(./res/Bright_false.png);}")
            self.left_btn.setEnabled(True)
        self.photo_widget_slide.show()
        self.show()


class Photo(QWidget):

    def __init__(self, shot_photo,w,h, parent=None):
        QWidget.__init__(self,parent)
        self.Photo = QLabel(self)
        self.Photo.move(0,0)
        self.Photo.resize(w,h)
        self.photo = QLabel(self.Photo)
        self.photo.resize(w,h)
        self.photo.setStyleSheet("QLabel{border-radius: 8px;border-image: url('" + shot_photo+ "') 0 0 0 0 stretch strectch;background:transparent;}")
        self.photo_path = ""
        self.photo_image = None

    def re_move(self):
        self.Photo.resize(self.width(), self.height())

    def resize_image(self):
        if(self.photo_image != None):
            if self.photo_image.width() > self.Photo.width() or self.photo_image.height() > self.Photo.height():

                if self.photo_image.width()/self.Photo.width()  > self.photo_image.height()/self.Photo.height():
                    self.photo.resize(self.Photo.width(), int(self.photo_image.height()*(self.Photo.width()/self.photo_image.width())))
                else:
                    self.photo.resize(int(self.photo_image.width()*self.Photo.height()/self.photo_image.height()), self.Photo.height())
            else:
                self.photo.resize(self.photo_image.width(),self.photo_image.height())
            self.photo.move(int((self.Photo.width()-self.photo.width())/2),int((self.Photo.height()-self.photo.height())/2))

            if(not hasattr(self,"pix")):
                self.pix = QPixmap(self.photo_path)
                self.photo.setPixmap(self.pix)
                self.photo.setScaledContents(True)
            # self.photo.setStyleSheet("QLabel{border-radius: 8px;border-image: url('" + self.photo_path + "') 0 0 0 0 stretch strectch;background:transparent;}")


    def shot_photo_change(self,shot_photo):

        self.photo_path = shot_photo
        shot_photo_image  = QImage(shot_photo)
        self.photo_image = shot_photo_image
        if shot_photo_image.width() > self.Photo.width() or shot_photo_image.height() > self.Photo.height():

            if shot_photo_image.width()/self.Photo.width()  > shot_photo_image.height()/self.Photo.height():
                self.photo.resize(self.Photo.width(), int(shot_photo_image.height()*(self.Photo.width()/shot_photo_image.width())))
            else:
                self.photo.resize(int(shot_photo_image.width()*self.Photo.height()/shot_photo_image.height()), self.Photo.height())
        else:
            if("svg" in shot_photo.split(".")):
                if(self.Photo.width()/shot_photo_image.width() < self.Photo.height()/shot_photo_image.height()):
                    self.photo.resize(int(shot_photo_image.width()*self.Photo.width()/shot_photo_image.width()),int(shot_photo_image.height()*self.Photo.width()/shot_photo_image.width()))
                else:
                    self.photo.resize(int(shot_photo_image.width() * self.Photo.height() / shot_photo_image.height()),
                                      int(shot_photo_image.height() * self.Photo.height() / shot_photo_image.height()))
            else:
                self.photo.resize(shot_photo_image.width(),shot_photo_image.height())
        self.photo.move(int((self.Photo.width()-self.photo.width())/2),int((self.Photo.height()-self.photo.height())/2))

        self.pix = QPixmap(self.photo_path)
        self.photo.setPixmap(self.pix)
        self.photo.setScaledContents(True)
        # self.photo.setStyleSheet("QLabel{border-radius: 8px;border-image: url('" + shot_photo + "') 0 0 0 0 stretch strectch;background:transparent;}")










