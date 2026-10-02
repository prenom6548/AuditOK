#!/usr/bin/python3
# -*- coding: utf-8 -*-


from PyQt5.QtGui import *
from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
from models.enums import BUTTON_ENTER,BUTTON_LEAVE,BUTTON_PRESSED,BUTTON_DISABLE
from ui.popular_app_card import *
from ui.systembutton import *
from ui.ad_widget import *
from models.application import *
from models.apkinfo import *
from ui.select_widget import *
from models.enums import setLongTextToElideFormat
import os
import sip
import gettext
gettext.bindtextdomain("kylin-software-center", "/usr/share/locale")
gettext.textdomain("kylin-software-center")
_ = gettext.gettext

class Category_Widget(QFrame):
    clicked_btn = True
    is_first_click = True
    click_type_btn = pyqtSignal(str,bool)
    click_category_ad = pyqtSignal(Application,bool)
    click_category_ad_apk = pyqtSignal(ApkInfo,bool)
    click_none_card = pyqtSignal()
    card_width = 248
    card_height = 124
    card_space = 15
    widget_space = 50
    def __init__(self,w,h,parent = None):
        click_ad_card = pyqtSignal(Application)
        QFrame.__init__(self,parent)
        self.w = w
        self.h = h
        self.m_parent = parent
        self.init_ui()
        self.init_style()

    def init_ui(self):
        self.resize(self.w, self.h)
        self.setWindowFlags(Qt.FramelessWindowHint)
        #self.setAttribute(Qt.WA_TranslucentBackground)

        # self.btn_widget = CategoryBar(Globals.ALL_CATEGORIES,self,False)
        tmp = [val for val in Globals.ALL_CATEGORIES if val in Globals.ORDINARY_CATEGORY]
        # self.btn_widget = CategoryBar(Globals.ORDINARY_CATEGORY,self,False)
        self.btn_widget = CategoryBar(tmp,self,False)
        self.btn_widget.setAttribute(Qt.WA_TranslucentBackground)
        self.btn_widget.resize((self.btn_widget.visiblecategorycount - 1) * ( self.btn_widget.itemwidth + self.btn_widget.categoryspacing) + self.btn_widget.itemwidth, 64)
        self.btn_widget.move(int((self.w - self.btn_widget.width()) / 2), 20)
        self.btn_widget.show()

        self.type_widget = QWidget(self)
        # self.type_widget.setAttribute(Qt.WA_TranslucentBackground)
        self.type_widget.resize(self.w,self.h-self.btn_widget.height()-16)
        self.type_widget.move(0,self.btn_widget.height()+self.btn_widget.y())
        self.type_widget.setStyleSheet("QWidget{background-color:transparent;border-radius:0px;}")
        self.type_widget.hide()

        w1 = int((self.type_widget.width()-self.widget_space-self.widget_space - 20) * (326 / (Globals.INIT_DETAIL_WIDTH -144-144 - 20)))
        w2 = int((self.type_widget.width()-self.widget_space-self.widget_space - 20) * (690 / (Globals.INIT_DETAIL_WIDTH -144-144 - 20)))
        self.category_ad_card = Advertisement_Widget(self.type_widget.width()-self.widget_space-self.widget_space,216,w1,w2,self.m_parent.mw,False,self.type_widget)
        self.category_ad_card.setAttribute(Qt.WA_TranslucentBackground)
        self.category_ad_card_height = 216
        self.category_ad_card.move(int((self.type_widget.width()-self.category_ad_card.width())/2),0)
        self.category_ad_card.show()
        self.category_ad_card.click_ad_card.connect(self.slot_click_ad_card)
        self.category_ad_card.click_ad_card_apk.connect(self.slot_click_ad_card)
        self.category_ad_card.click_none_app_ad_card.connect(self.slot_click_none_card)

        self.preview_ad_card = QLabel(self.type_widget)
        self.preview_ad_card.setAttribute(Qt.WA_TranslucentBackground)
        self.preview_ad_card.resize(self.type_widget.width()-self.widget_space*2,self.category_ad_card.height())
        self.preview_ad_card.move(int((self.type_widget.width() - self.preview_ad_card.width()) / 2), 0)
        photo = "./data/ads/preview_banner.png"
        pix2 = QPixmap(photo)
        fitpixmap_userIcon = pix2.scaled(self.preview_ad_card.width(), self.preview_ad_card.height(), Qt.IgnoreAspectRatio, Qt.SmoothTransformation)
        fitpixmap_userIcon = self.PixmapToRound(fitpixmap_userIcon,self.preview_ad_card.width(), self.preview_ad_card.height())
        self.preview_ad_card.setPixmap(fitpixmap_userIcon)
        self.preview_ad_card.hide()

        self.type_name_label = QLabel(self.type_widget)
        self.type_name_label.setAttribute(Qt.WA_TranslucentBackground)
        self.type_name_label.setFixedHeight(30)
        self.type_name_label.setStyleSheet("QLabel{font-size: 24px;font-weight: 500;color: #303133;line-height: 42px;}")
        self.type_name_label.setText(self.btn_widget.first_btn)
        self.type_name_label.adjustSize()
        self.type_name_label.move(self.widget_space,self.category_ad_card.height()+30)
        self.type_name_label.show()
        # self.type_name_label.hide()

        self.type_show_card_widget = QWidget(self.type_widget)
        self.type_show_card_widget.setAttribute(Qt.WA_TranslucentBackground)
        self.type_show_card_widget.resize(self.type_widget.width() - self.widget_space - self.widget_space,
                                          self.type_widget.height() - self.type_name_label.height())
        self.type_show_card_widget.move(self.widget_space, self.type_name_label.y() + self.type_name_label.height())
        self.type_show_card_widget.show()

        self.type_show_card_widget.children_count = 0

        self.choose_sorting_type_btn = QPushButton(self.type_widget)
        # self.choose_sorting_type_btn.setAttribute(Qt.WA_TranslucentBackground)
        self.choose_sorting_type_btn.setStyleSheet("QPushButton{border-radius: 13px;font-size: 14px;font-weight: 400;color: #3790FA;line-height: 20px;background: transparent;}QPushButton::hover{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 400;}QPushButton:pressed{color: #3790FA;background: transparent;font-weight: 500;}")
        self.choose_sorting_type_btn.setFocusPolicy(Qt.NoFocus)
        self.type_sorting_btn = QPushButton(self.choose_sorting_type_btn)
        self.type_sorting_btn.setAttribute(Qt.WA_TranslucentBackground)
        self.type_sorting_btn.setFocusPolicy(Qt.NoFocus)
        self.type_sorting_btn.move(0,0)
        self.type_sorting_btn.setFixedSize(80, 26)
        self.type_sorting_btn.setStyleSheet("QPushButton{font-size: 14px;font-weight: 400;color: #3790FA;line-height: 20px;background: transparent;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: transparent;font-weight: 500;}")
        self.type_sorting_btn.setText(_("Defualt"))
        self.type_sorting_btn.adjustSize()
        self.type_sorting_btn.show()
        # self.type_sorting_btn.clicked.connect(lambda: self.slot_click_sorting_btn(1))

        self.pull_down_button = QPushButton(self.choose_sorting_type_btn)
        self.pull_down_button.setFocusPolicy(Qt.NoFocus)
        self.pull_down_button.setAttribute(Qt.WA_TranslucentBackground)
        self.pull_down_button.resize(16,16)
        self.pull_down_button.setStyleSheet("QPushButton{border:0px;background-image:url('res/show_search_black.png')}QPushButton:hover{border:0px;background-image:url('res/show_search_clicked.png')}QPushButton:pressed{border:0px;background-image:url('res/show_search_clicked.png')}")
        self.choose_sorting_type_btn.resize(self.type_sorting_btn.width()+self.pull_down_button.width()+5,26)
        self.pull_down_button.move(self.type_sorting_btn.x()+self.type_sorting_btn.width(),int((self.type_sorting_btn.height()-self.pull_down_button.height())/2))
        self.choose_sorting_type_btn.move(self.type_show_card_widget.width() - self.choose_sorting_type_btn.width()+self.type_show_card_widget.x()-10,self.type_name_label.y())
        self.choose_sorting_type_btn.clicked.connect(self.slot_choose_sorting_type)
        self.pull_down_button.clicked.connect(self.slot_choose_sorting_type)
        self.type_sorting_btn.clicked.connect(self.slot_choose_sorting_type)
        self.pull_down_button.show()

        self.rating_sorting_btn = QPushButton(self.type_widget)
        self.rating_sorting_btn.setFocusPolicy(Qt.NoFocus)
        self.rating_sorting_btn.setFixedSize(80, 26)
        self.rating_sorting_btn.setStyleSheet(
            "QPushButton{font-size: 14px;font-weight: 400;color: #303133;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, self.widget_space, 250, 0.12);font-weight: 500;}")
        self.rating_sorting_btn.setText(_("Rattings"))
        self.rating_sorting_btn.move(self.type_show_card_widget.width() - self.rating_sorting_btn.width()+self.type_show_card_widget.x()-10,
                                     self.type_name_label.y())
        self.rating_sorting_btn.show()
        self.rating_sorting_btn.clicked.connect(lambda :self.slot_click_sorting_btn(1))
        self.rating_sorting_btn.hide()

        self.download_sorting_btn = QPushButton(self.type_widget)
        self.download_sorting_btn.setFocusPolicy(Qt.NoFocus)
        self.download_sorting_btn.setFixedSize(80,26)
        self.download_sorting_btn.setStyleSheet(
            "QPushButton{font-size: 14px;font-weight: 400;color: #303133;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, self.widget_space, 250, 0.12);font-weight: 500;}")
        self.download_sorting_btn.setText(_("Top app"))
        self.download_sorting_btn.move(self.rating_sorting_btn.x() -self.download_sorting_btn.width(),self.type_name_label.y())
        self.download_sorting_btn.show()
        self.download_sorting_btn.clicked.connect(lambda :self.slot_click_sorting_btn(0))
        self.download_sorting_btn.hide()

        self.popularity_sorting_btn = QPushButton(self.type_widget)
        self.popularity_sorting_btn.setFocusPolicy(Qt.NoFocus)
        self.popularity_sorting_btn.setFixedSize(80,26)
        self.popularity_sorting_btn.setStyleSheet(
            "QPushButton{font-size: 14px;font-weight: 400;color: #303133;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, self.widget_space, 250, 0.12);font-weight: 500;}")
        self.popularity_sorting_btn.setText(_("Default"))
        self.popularity_sorting_btn.move(self.download_sorting_btn.x() -self.download_sorting_btn.width(),self.type_name_label.y())
        self.popularity_sorting_btn.show()
        self.popularity_sorting_btn.clicked.connect(lambda :self.slot_click_sorting_btn(2))
        self.popularity_sorting_btn.hide()


        self.btn_split_line = QLabel(self.type_widget)
        self.btn_split_line.resize(1,12)
        self.btn_split_line.setStyleSheet("QLabel{border: 1px solid #999999;background-color: #999999;}")
        self.btn_split_line.move(self.download_sorting_btn.x()-self.btn_split_line.width()-6,int((self.popularity_sorting_btn.height()-self.btn_split_line.height())/2)+self.popularity_sorting_btn.y())
        self.btn_split_line.hide()

        self.all_check_btn = QPushButton(self.type_widget)
        self.all_check_btn.setFocusPolicy(Qt.NoFocus)
        self.all_check_btn.setText(_("Select all"))
        self.all_check_btn.setFixedSize(80, 26)
        self.all_check_btn.setStyleSheet(
            "QPushButton{font-size: 14px;font-weight: 400;color: #303133;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, self.widget_space, 250, 0.12);font-weight: 500;}")

        self.one_click_download = QPushButton(self.type_widget)
        self.one_click_download.setFocusPolicy(Qt.NoFocus)
        self.one_click_download.setText(_("all down"))
        self.one_click_download.adjustSize()
        self.one_click_download.setFixedSize(80,26)
        self.one_click_download.setStyleSheet("QPushButton{border:0px;font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;border:0px solid #ff0000;}QPushButton:hover{background: rgba(55, self.widget_space, 250, 0.12);border-radius: 12px;}QPushButton:pressed{background: rgba(55, self.widget_space, 250, 0.12);border-radius: 12px;}")
        self.one_click_download.move(self.choose_sorting_type_btn.x() -self.one_click_download.width(),self.type_name_label.y())
        self.all_check_btn.move(self.one_click_download.x() - self.all_check_btn.width(), self.type_name_label.y())
        self.one_click_download.setEnabled(False)

        self.loadingmove = QMovie("./res/loadgif2.gif")
        self.loadingmove_dark = QMovie("./res/loadgifdark.gif")
        self.loadinglabel = QLabel(self)
        self.loadinglabel.setAttribute(Qt.WA_TranslucentBackground)
        self.loadinglabel.resize(2000,2000)
        self.loadinglabel.move(int((self.width()-self.loadinglabel.width())/2),int((self.height()-self.loadinglabel.height())/2))
        self.loadinglabel.setAlignment(Qt.AlignCenter)
        if(Globals.THEME_COLOR == 0):
            self.loadinglabel.setMovie(self.loadingmove)
            self.loadingmove.start()
        else:
            self.loadinglabel.setMovie(self.loadingmove_dark)
            self.loadingmove_dark.start()
        self.loadinglabel.hide()


        self.scrollArea = QScrollArea(self)
        self.scrollArea.setWidget(self.type_widget)
        self.scrollArea.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self.scrollArea.horizontalScrollBar().setEnabled(False)
        self.scrollArea.setVerticalScrollBarPolicy(Qt.ScrollBarAsNeeded)
        self.scrollArea.setGeometry(0, int(self.btn_widget.height()+self.btn_widget.y()+16), int(self.type_widget.width()-10), int(self.height()-self.btn_widget.height()-10))
        self.type_widget.resize(self.width(),int(self.type_name_label.y()+self.type_name_label.height()+self.type_show_card_widget.height()))
        self.scrollArea.show()
        self.re_move()

    #
    #函数：绘制圆角图片
    #
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

    #
    #函数：初始化界面的样式
    #
    def init_style(self):
        if(Globals.THEME_COLOR == 0):
            self.setStyleSheet("QFrame{border-bottom-left-radius:12px;border-bottom-right-radius:12px; background-color:#FFFFFF;}")
            self.btn_widget.setStyleSheet("QWidget{background-color:#ffffff;border-radius:0px;}")
            # self.type_widget.setStyleSheet("QWidget{background-color:#ffffff;border-radius:0px;}")
            # self.type_show_card_widget.setStyleSheet("QWidget{background-color:#ffffff;border-radius:0px;}")
            self.type_name_label.setStyleSheet("QLabel{font-size: 24px;font-weight: 500;color: #303133;line-height: 42px;}")
            self.type_sorting_btn.setStyleSheet(
                "QPushButton{font-size: 14px;font-weight: 400;color: #3790FA;line-height: 20px;background: transparent;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: transparent;font-weight: 500;}")
            self.pull_down_button.setStyleSheet(
                "QPushButton{border-radius: 4px;border:0px;background-image:url('res/show_search_black.png')}QPushButton:hover{border:0px;background-image:url('res/show_search_clicked.png')}QPushButton:pressed{border:0px;background-image:url('res/show_search_clicked.png')}")
            if(self.one_click_download.isEnabled()):
                self.one_click_download.setStyleSheet("QPushButton{border:0px;font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;border:0px solid #ff0000;}QPushButton:hover{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}QPushButton:pressed{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}")
            else:
                self.one_click_download.setStyleSheet(
                        "QPushButton{font-size:14px;font-weight: 500;color: #8f9399;line-height: 20px;border:0px solid #ff0000;}")
            self.all_check_btn.setStyleSheet(
                "QPushButton{font-size: 14px;border:0px;font-weight: 500;color: #606265;line-height: 20px;}QPushButton:hover{background: rgba(55, 144, 250, 0.12);color: #3790FA;border-radius: 12px;}PushButton:pressed{background: rgba(55, 144, 250, 0.12);color: #3790FA;border-radius: 12px;}")
            self.loadinglabel.setMovie(self.loadingmove)
            self.type_sorting_btn.setStyleSheet(
                "QPushButton{font-size: 14px;font-weight: 400;color: #3790FA;line-height: 20px;background: transparent;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: transparent;font-weight: 500;}")
            self.pull_down_button.setStyleSheet(
                "QPushButton{border-radius: 4px;border:0px;background-image:url('res/show_search_black.png')}QPushButton:hover{border:0px;background-image:url('res/show_search_clicked.png')}QPushButton:pressed{border:0px;background-image:url('res/show_search_clicked.png')}")
            self.loadingmove.start()
            self.scrollArea.verticalScrollBar().setStyleSheet("QScrollBar:vertical{margin:0px 0px 0px 0px;background-color:transparent;border:0px;width:6px;border-radius: 3px;}\
                                QScrollBar::handle:vertical{background-color:#C2C2C2;width:6px;border-radius:3px;} QScrollBar::handle:vertical:hover{background-color:#8F9399;opacity:0.1;width:6px;border-radius:3px;}  QScrollBar::handle:vertical:pressed{background-color:#8F9399;width:6px;border-radius:3px;}\
                                QScrollBar::sub-line:vertical{subcontrol-origin:margin;border:0px solid red;height:13px}\
                                QScrollBar::up-arrow:vertical{subcontrol-origin:margin;background-color:rgba(0,0,0,0);height:13px}\
                                QScrollBar::sub-page:vertical{background-color:transparent;}\
                                QScrollBar::add-page:vertical{background-color:transparent;}\
                                QScrollBar::down-arrow:vertical{background-color:rgba(0,0,0,0);}\
                                QScrollBar::add-line:vertical{subcontrol-origin:margin;border:0px solid green;height:13px}")
            for btn in self.btn_widget.btnGroup.buttons():
                if(self.type_name_label.text() == btn.text_label.text()):
                    btn.setStyleSheet("QPushButton{border-radius: 12px;font-size: 14px;font-weight: 400;line-height: 20px;text-align: center;background-color: #3790FA;color: #ffffff;text-align : center;}")
                else:
                    btn.setStyleSheet("QPushButton{background-color:transparent;border-radius: 12px;font-size: 14px;font-weight: 400;color: #606265;text-align: center;line-height: 20px;text-align : center;}QPushButton:hover{background: rgba(55, 144, 250, 0.12);color: #3790FA;}QPushButton:pressed{background-color: #3790FA;color: #ffffff;}")
                    btn.text_label.setStyleSheet(
                        "QLabel{font-size: 14px;font-weight: 400;color: #606265;line-height: 14px;text-align : center;}")
        else:
            self.setStyleSheet("QFrame{border-bottom-left-radius:12px;border-bottom-right-radius:12px; background-color:#1F2022;}")
            self.btn_widget.setStyleSheet("QWidget{background-color:#1F2022;border-radius:0px;}")
            # self.type_widget.setStyleSheet("QWidget{background-color:#1F2022;border-radius:0px;}")
            # self.type_show_card_widget.setStyleSheet("QWidget{background-color:#1F2022;border-radius:0px;}")
            self.type_name_label.setStyleSheet("QLabel{font-size: 24px;font-weight: 500;color: #F9F9F9;line-height: 42px;}")
            if (self.one_click_download.isEnabled()):
                self.one_click_download.setStyleSheet(
                    "QPushButton{border:0px;font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;border:0px solid #ff0000;}QPushButton:hover{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}QPushButton:pressed{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}")
            else:
                self.one_click_download.setStyleSheet(
                    "QPushButton{font-size:14px;font-weight: 500;color: #8f9399;line-height: 20px;border:0px solid #ff0000;}")
            self.all_check_btn.setStyleSheet("QPushButton{font-size: 14px;border:0px;font-weight: 500;color: #F9F9F9;line-height: 20px;}QPushButton:hover{background: rgba(55, 144, 250, 0.12);color: #3790FA;border-radius: 12px;}PushButton:pressed{background: rgba(55, 144, 250, 0.12);color: #3790FA;border-radius: 12px;}")
            self.loadinglabel.setMovie(self.loadingmove_dark)
            self.loadingmove_dark.start()
            self.scrollArea.verticalScrollBar().setStyleSheet("QScrollBar:vertical{margin:0px 0px 0px 0px;background-color:transparent;border:0px;width:6px;border-radius: 3px;}\
                                QScrollBar::handle:vertical{background-color:#6B6B6B;width:6px;border-radius:3px;} QScrollBar::handle:vertical:hover{background-color:#959595;opacity:0.1;width:6px;border-radius:3px;}  QScrollBar::handle:vertical:pressed{background-color:#959595;width:6px;border-radius:3px;}\
                                QScrollBar::sub-line:vertical{subcontrol-origin:margin;border:0px solid red;height:13px}\
                                QScrollBar::up-arrow:vertical{subcontrol-origin:margin;background-color:rgba(0,0,0,0);height:13px}\
                                QScrollBar::sub-page:vertical{background-color:transparent;}\
                                QScrollBar::add-page:vertical{background-color:transparent;}\
                                QScrollBar::down-arrow:vertical{background-color:rgba(0,0,0,0);}\
                                QScrollBar::add-line:vertical{subcontrol-origin:margin;border:0px solid green;height:13px}")
            for btn in self.btn_widget.btnGroup.buttons():
                if(self.type_name_label.text() == btn.text_label.text()):
                    btn.setStyleSheet("QPushButton{border-radius: 12px;font-size: 14px;font-weight: 400;line-height: 20px;text-align: center;background-color: #3790FA;color: #ffffff;text-align : center;}")
                else:
                    btn.setStyleSheet(
                        "QPushButton{background-color:transparent;border-radius: 12px;font-size: 14px;font-weight: 400;color: #8F9399;text-align: center;line-height: 20px;text-align : center;}QPushButton:hover{background: rgba(55, 144, 250, 0.12);color: #3790FA;}QPushButton:pressed{background-color: #3790FA;color: #ffffff;}")

                    btn.text_label.setStyleSheet(
                            "QLabel{font-size: 14px;font-weight: 400;color: #8F9399;line-height: 14px;text-align : center;}")
        # if (Globals.SORTING_METHOD == 0):
        #     if (Globals.THEME_COLOR == 0):
        #         self.download_sorting_btn.setStyleSheet(
        #             "QPushButton{font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;background: rgba(55, 144, 250, 0.12);border-radius: 13px;}")
        #         self.rating_sorting_btn.setStyleSheet(
        #             "QPushButton{font-size: 14px;font-weight: 400;color: #303133;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
        #         self.popularity_sorting_btn.setStyleSheet(
        #             "QPushButton{font-size: 14px;font-weight: 400;color: #303133;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
        #     else:
        #         self.download_sorting_btn.setStyleSheet(
        #             "QPushButton{font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;background: rgba(55, 144, 250, 0.12);border-radius: 13px;}")
        #         self.rating_sorting_btn.setStyleSheet(
        #             "QPushButton{font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
        #         self.popularity_sorting_btn.setStyleSheet(
        #             "QPushButton{font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
        # elif (Globals.SORTING_METHOD == 1):
        #     if (Globals.THEME_COLOR == 0):
        #         self.rating_sorting_btn.setStyleSheet(
        #             "QPushButton{font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;background: rgba(55, 144, 250, 0.12);border-radius: 13px;}")
        #         self.download_sorting_btn.setStyleSheet(
        #             "QPushButton{font-size: 14px;font-weight: 400;color: #303133;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
        #         self.popularity_sorting_btn.setStyleSheet(
        #             "QPushButton{font-size: 14px;font-weight: 400;color: #303133;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
        #     else:
        #         self.rating_sorting_btn.setStyleSheet(
        #             "QPushButton{font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;background: rgba(55, 144, 250, 0.12);border-radius: 13px;}")
        #         self.download_sorting_btn.setStyleSheet(
        #             "QPushButton{font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
        #         self.popularity_sorting_btn.setStyleSheet(
        #             "QPushButton{font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
        # else:
        #     if (Globals.THEME_COLOR == 0):
        #         self.popularity_sorting_btn.setStyleSheet(
        #             "QPushButton{font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;background: rgba(55, 144, 250, 0.12);border-radius: 13px;}")
        #         self.download_sorting_btn.setStyleSheet(
        #             "QPushButton{font-size: 14px;font-weight: 400;color: #303133;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
        #         self.rating_sorting_btn.setStyleSheet(
        #             "QPushButton{font-size: 14px;font-weight: 400;color: #303133;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
        #     else:
        #         self.popularity_sorting_btn.setStyleSheet(
        #             "QPushButton{font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;background: rgba(55, 144, 250, 0.12);border-radius: 13px;}")
        #         self.download_sorting_btn.setStyleSheet(
        #             "QPushButton{font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
        #         self.rating_sorting_btn.setStyleSheet(
        #             "QPushButton{font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")


    #
    #函数：重新设置界面的控件位置和大小
    #
    def re_move(self):
        self.btn_widget.reload_btn()
        self.btn_widget.resize((self.btn_widget.visiblecategorycount - 1) * (
                self.btn_widget.itemwidth + self.btn_widget.categoryspacing) + self.btn_widget.itemwidth, 64)
        self.btn_widget.move(int((self.width() - self.btn_widget.width()) / 2), 20)
        self.type_widget.resize(self.width(), self.height() - self.btn_widget.height())
        self.category_ad_card.resize(self.type_widget.width()-self.widget_space-self.widget_space, round(self.category_ad_card_height * self.m_parent.proportion))
        self.category_ad_card.re_move(self.m_parent.proportion)
        self.preview_ad_card.resize(self.type_widget.width() - self.widget_space * 2, self.category_ad_card.height())
        photo = "./data/ads/preview_banner.png"
        pix2 = QPixmap(photo)
        fitpixmap_userIcon = pix2.scaled(self.preview_ad_card.width(), self.preview_ad_card.height(),
                                         Qt.IgnoreAspectRatio, Qt.SmoothTransformation)
        fitpixmap_userIcon = self.PixmapToRound(fitpixmap_userIcon, self.preview_ad_card.width(),
                                                self.preview_ad_card.height())
        self.preview_ad_card.setPixmap(fitpixmap_userIcon)
        self.type_name_label.move(self.widget_space, self.category_ad_card.height() + 30)
        self.type_widget.resize(self.type_widget.width(),
                                int(self.type_name_label.y() + self.type_name_label.height() + self.type_show_card_widget.height()))
        self.type_show_card_widget.resize(self.type_widget.width() - self.widget_space - self.widget_space,
                                          self.type_widget.height() - self.type_name_label.height())

        self.choose_sorting_type_btn.move(int(self.type_show_card_widget.width() - self.choose_sorting_type_btn.width()+self.type_show_card_widget.x()-10),int(self.type_name_label.y()))
        self.rating_sorting_btn.move(int(self.type_show_card_widget.width() - self.rating_sorting_btn.width() + self.type_show_card_widget.x() - 10),int(self.type_name_label.y()))
        self.download_sorting_btn.move(int(self.rating_sorting_btn.x() - self.download_sorting_btn.width()),
                                       int(self.type_name_label.y()))
        self.btn_split_line.move(int(self.popularity_sorting_btn.x() - self.btn_split_line.width() - 6),
                                 int((self.popularity_sorting_btn.height() - self.btn_split_line.height()) / 2) +self.popularity_sorting_btn.y())
        self.one_click_download.move(int(self.choose_sorting_type_btn.x() - self.one_click_download.width()),int(self.type_name_label.y()))
        self.all_check_btn.move(int(self.one_click_download.x() - self.all_check_btn.width()), int(self.type_name_label.y()))
        self.loadinglabel.move(int((self.width() - self.loadinglabel.width()) / 2),int((self.height() - self.loadinglabel.height()) / 2))

        self.scrollArea.setGeometry(0, int(self.btn_widget.height()+self.btn_widget.y()+16), int(self.type_widget.width()-10), int(self.height()-self.btn_widget.height()-self.btn_widget.y()-30))
        self.type_show_card_widget.move(self.widget_space, int(self.type_name_label.y() + self.type_name_label.height()))
        self.reload_card(self.type_show_card_widget,self.scrollArea)
        self.type_widget.resize(self.type_widget.width(),
                                int(self.type_name_label.y() + self.type_name_label.height() + self.type_show_card_widget.height()))

    #
    #函数：重新设置卡片
    #
    def reload_card(self,widget,scrollArea):
        cards = widget.children()
        if (len(cards) != 0):
            h_num = int(widget.width()/(self.card_width+self.card_space))
            get_card_width = int((widget.width() - (self.card_space * (h_num - 1)) - 20) / h_num)
            proportion = get_card_width / widget.width()
            get_card_height = self.card_height
            v_num = 0
            num = 0
            for i in range(len(cards)):
                card = cards[i]
                card.resize(get_card_width,get_card_height)
                card.re_move()
                if (num < h_num):
                    if (num == 0):
                        card.move(0, 20 + (20 + card.height()) * v_num)
                        widget.resize(widget.width(), 20 + (20 + card.height()) * (v_num + 1))
                    else:
                        card.move(num * (card.width() + self.card_space) + 5, 20 + (20 + card.height()) * v_num)
                    num = num + 1
                else:
                    num = 0
                    v_num = v_num + 1
                    if (num == 0):
                        card.move(0, 20+ (20 + card.height()) * v_num)
                        widget.resize(widget.width(),  (20 + card.height()) * (v_num + 1))
                    else:
                        card.move(num * (card.width() + self.card_space), 20 + (20 + card.height()) * v_num)
                    num = num + 1

    #
    #函数：处理点击分类界面的类型按钮
    #
    def slot_clicked_btn(self,btn_name):
        self.click_type_btn.emit(btn_name,True)


    def slot_click_sorting_btn(self,sort_type,need_emit_signal = True):
        #sort_type:0(下载排行)，1（评分排行）2（热度排行）
        # if(Globals.SORTING_METHOD == 0):
        #     Globals.SORTING_METHOD = 1
        # else:
        #     Globals.SORTING_METHOD = 0
        if(sort_type == 0):
            Globals.SORTING_METHOD = 0
            if(Globals.THEME_COLOR == 0):
                self.download_sorting_btn.setStyleSheet("QPushButton{font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;background: rgba(55, 144, 250, 0.12);border-radius: 13px;}")
                self.rating_sorting_btn.setStyleSheet("QPushButton{font-size: 14px;font-weight: 400;color: #303133;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
                self.popularity_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 400;color: #303133;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
            else:
                self.download_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;background: rgba(55, 144, 250, 0.12);border-radius: 13px;}")
                self.rating_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
                self.popularity_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
        elif(sort_type == 1):
            Globals.SORTING_METHOD = 1
            if(Globals.THEME_COLOR == 0):
                self.rating_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;background: rgba(55, 144, 250, 0.12);border-radius: 13px;}")
                self.download_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 400;color: #303133;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
                self.popularity_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 400;color: #303133;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
            else:
                self.rating_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;background: rgba(55, 144, 250, 0.12);border-radius: 13px;}")
                self.download_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
                self.popularity_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
        else:
            Globals.SORTING_METHOD = 2
            if (Globals.THEME_COLOR == 0):
                self.popularity_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;background: rgba(55, 144, 250, 0.12);border-radius: 13px;}")
                self.download_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 400;color: #303133;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
                self.rating_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 400;color: #303133;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
            else:
                self.popularity_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;background: rgba(55, 144, 250, 0.12);border-radius: 13px;}")
                self.download_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
                self.rating_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
        # self.check_choose_btn_type()
        if(need_emit_signal == True):
                self.click_type_btn.emit(self.type_name_label.text(),False)

    #
    #函数：处理点击选择排行类型
    #
    def slot_choose_sorting_type(self):
        if(not hasattr(self,"select_widget")):
            item_name_list = [_("Default"),_("Top app"),_("Rattings")]
            self.select_widget = Select_Widget(102,30,3,item_name_list,False,self.type_widget)
            self.select_widget.click_item.connect(self.slot_click_select_list_item)
            item_count = self.select_widget.item_list.count()
            for i in range(item_count):
                item = self.select_widget.item_list.item(i)
                get_item_widget = self.select_widget.item_list.itemWidget(item)
                if(Globals.SORTING_METHOD == 2):
                    if(get_item_widget.item_name_label.text() == _("Default")):
                        get_item_widget.icon_label.show()
                    else:
                        get_item_widget.icon_label.hide()
                elif(Globals.SORTING_METHOD == 1):
                    if (get_item_widget.item_name_label.text() == _("Top app")):
                        get_item_widget.icon_label.show()
                    else:
                        get_item_widget.icon_label.hide()
                else:
                    if (get_item_widget.item_name_label.text() == _("Rattings")):
                        get_item_widget.icon_label.show()
                    else:
                        get_item_widget.icon_label.hide()
        self.select_widget.move(self.x()+self.type_widget.x()+self.choose_sorting_type_btn.x()+self.choose_sorting_type_btn.width()-self.select_widget.width()+6,self.y()+self.type_widget.y()+self.choose_sorting_type_btn.y()+self.choose_sorting_type_btn.height()+self.scrollArea.verticalScrollBar().value()-6)
        self.select_widget.slot_show_dark_mode()
        self.select_widget.show()

    #
    #函数：处理按钮的显示以及select_widget勾选状态
    #
    def check_choose_btn_type(self):
        if(Globals.SORTING_METHOD == 2):
            self.type_sorting_btn.setText(_("Default"))
            self.type_sorting_btn.adjustSize()
        elif(Globals.SORTING_METHOD == 0):
            self.type_sorting_btn.setText(_("Top app"))
            self.type_sorting_btn.adjustSize()
        else:
            self.type_sorting_btn.setText(_("Rattings"))
            self.type_sorting_btn.adjustSize()
        if (hasattr(self, "select_widget")):
            item_count = self.select_widget.item_list.count()
            for i in range(item_count):
                item = self.select_widget.item_list.item(i)
                get_item_widget = self.select_widget.item_list.itemWidget(item)
                if (Globals.SORTING_METHOD == 2):
                    if (get_item_widget.item_name_label.text() == _("Default")):
                        get_item_widget.icon_label.show()
                    else:
                        get_item_widget.icon_label.hide()
                elif (Globals.SORTING_METHOD == 1):
                    if (get_item_widget.item_name_label.text() == _("Top app")):
                        get_item_widget.icon_label.show()
                    else:
                        get_item_widget.icon_label.hide()
                else:
                    if (get_item_widget.item_name_label.text() == _("Rattings")):
                        get_item_widget.icon_label.show()
                    else:
                        get_item_widget.icon_label.hide()

    #
    #函数：处理点击list_item
    #
    def slot_click_select_list_item(self,item):
        get_item_widget = self.select_widget.item_list.itemWidget(item)
        text = get_item_widget.item_name_label.text()
        get_item_widget.icon_label.show()
        self.type_sorting_btn.setText(text)
        self.type_sorting_btn.adjustSize()
        self.select_widget.hide()
        if(text == _("Default")):
            self.slot_click_sorting_btn(2)
        elif(text == _("Top app")):
            self.slot_click_sorting_btn(0)
        else:
            self.slot_click_sorting_btn(1)


    #
    #函数：处理点击广告
    #
    def slot_click_ad_card(self,appmsg,need_send_exposure_rate):
        if(type(appmsg) == Application):
            self.click_category_ad.emit(appmsg,need_send_exposure_rate)
        else:
            self.click_category_ad_apk.emit(appmsg,need_send_exposure_rate)

    #
    #函数：处理点击源中没有的广告
    #
    def slot_click_none_card(self):
        self.click_none_card.emit()

    def mouseReleaseEvent(self, event):
        self.m_parent.mw.hide_tip_widget()
        if (hasattr(self, "select_widget")):
            self.select_widget.hide()

    # 鼠标进入事件

    def enterEvent(self, event):
        self.status = BUTTON_ENTER
        self.scrollArea.verticalScrollBar().show()
        # self.scrollArea.show()

    #
    # 函数：鼠标离开事件
    #

    def leaveEvent(self, event):
        self.status = BUTTON_LEAVE
        self.scrollArea.verticalScrollBar().hide()

    #
    # 函数：鼠标按下事件
    #
    def mousePressEvent(self, event):
        if event.button() == Qt.LeftButton:
            self.status = BUTTON_PRESSED
        else:
            event.ignore()

    #
    # 函数名:设置滚动条起始位置
    # Function:set verticalSscrollBar starting point
    #
    def scrollToTop(self):
        vsb = self.scrollArea.verticalScrollBar()
        vsb.setValue(0)

class CategoryBar(QWidget):

    categorycount = 0
    itemwidth = 52
    categoryspacing = 8
    first_btn = ""
    click_categoy = pyqtSignal(str,bool)

    def __init__(self,category_list,parent=None,is_kydroid = False):
        QWidget.__init__(self, parent)

        # self.caslot_btn_clickedtegorytab = QLabel(self)`
        self.visiblecategorycount = 0
        self.btn_x = 0
        self.btn_y = 0
        self.is_kydroid = is_kydroid
        self.m_parent = parent
        self.category_list = category_list
        self.categoryPanel = QWidget(self)
        self.btnGroup = QButtonGroup(self.categoryPanel)
        self.btnGroup.buttonClicked.connect(self.slot_btn_clicked)
        self.init_categories(self.category_list)

    #
    # 函数名:进入控件
    # Function:enter control
    #
    def enterEvent(self, event):
        pass

    #
    # 函数名:离开控件
    # Function:leave control
    #
    def leaveEvent(self, event):
        pass

    #
    # 函数名:初始化分类
    # Function:init categories
    #
    def init_categories(self,category_list):
        cat_list = category_list
        #安卓界面显示全部分类
        if(self.is_kydroid == True):
            if (self.visiblecategorycount == 0):
                item = {'is_visible': 1, 'id': 10000, 'name': 'all', 'displayname': '全部', 'priority': 5}
                self.add_category(item)
        for item in cat_list:
            category = item["name"]
            if(item["is_visible"] == 1):
                if(item["id"] == 1):
                    if(self.is_kydroid == False):
                        self.visiblecategorycount = self.visiblecategorycount + 1
                        self.add_category(item)
                    else:
                        continue
                else:
                    if(self.is_kydroid):
                        for i in Globals.KYDROID_CATEGORY:
                            if(int(item["id"]) == int(i)):
                                self.visiblecategorycount = self.visiblecategorycount + 1
                                self.add_category(item)
                                break
                    else:
                        self.visiblecategorycount = self.visiblecategorycount + 1
                        self.add_category(item)
        self.setFixedWidth(self.btn_x)

    #
    # 函数名:添加分类
    # Function:add category
    #
    def add_category(self, category):
        btnCategory = CategoryButton(category["name"], category["displayname"], self.categoryPanel)
        # btnCategory.enter_btn.connect(self.slot_btn_clicked)
        # btnCategory.clicked.connect(lambda: self.slot_btn_clicked(btnCategory))
        btnCategory.click_btn.connect(self.slot_btn_clicked)
        self.btnGroup.addButton(btnCategory)
        x = self.btn_x
        if(self.m_parent.width() -self.m_parent.widget_space <self.btn_x + btnCategory.width()):
            btnCategory.move(0, self.btn_y + 20+btnCategory.height())
            self.setFixedWidth(self.btn_x)
            self.btn_x = 0
            self.btn_y = self.btn_y + btnCategory.height() +20
            self.setFixedHeight(self.btn_y + btnCategory.height())
        else:
            btnCategory.move(self.btn_x, self.btn_y)
        self.btn_x = self.btn_x + btnCategory.width() + self.categoryspacing
        self.categorycount = self.categorycount + 1
        if(self.categorycount == 1):
            self.first_btn = btnCategory.text_label.text()
            btnCategory.is_clicked = True
            btnCategory.setStyleSheet("QPushButton{border-radius: 12px;font-size: 14px;font-weight: 400;line-height: 20px;text-align: center;background-color: #3790FA;color: #ffffff;text-align : center;}")
            btnCategory.text_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #ffffff;line-height: 14px;text-align : center;}")
            zh_local = os.getenv("LANG")
            if ("zh_CN" in zh_local):
                self.click_categoy.emit(category["displayname"],True)
            else:
                self.click_categoy.emit(category["name"], True)
                # btnCategory.setText(category["name"])

        if self.visiblecategorycount == self.categorycount:
            return

    #
    # 函数名:控件左侧
    # Function:control left
    #
    def scrollToLeft(self):
        pass

    #
    # 函数名:控件右侧
    # Function:control right
    #
    def scrollToRight(self):
        pass


    #
    # 函数名:分类数量
    # Function:count the categories
    #
    def count(self):
        return self.categorycount

    #
    # 函数名:删除item
    # Function:remove item
    #
    def clear(self):
        categories = self.categoryPanel.children()
        for category in categories:
            sip.delete(category)

    #
    # 函数名:点击控件
    # Function:click control
    #
    def slot_btn_clicked(self, btn,all_reflash = True):
        btns = self.btnGroup.buttons()
        for b in btns:
            if(b != btn):
                b.hide()
                b.is_clicked = False
                if(Globals.THEME_COLOR == 0):
                    b.setStyleSheet("QPushButton{background-color:transparent;border-radius: 12px;font-size: 14px;font-weight: 400;color: #606265;text-align: center;line-height: 20px;text-align : center;}QPushButton:hover{background: rgba(55, 144, 250, 0.12);color: #3790FA;}QPushButton:pressed{background-color: #3790FA;color: #ffffff;}")
                    b.text_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #606265;line-height: 14px;text-align : center;}")
                else:
                    b.setStyleSheet(
                        "QPushButton{background-color:transparent;border-radius: 12px;font-size: 14px;font-weight: 400;color: #8F9399;text-align: center;line-height: 20px;text-align : center;}QPushButton:hover{background: rgba(55, 144, 250, 0.12);color: #3790FA;}QPushButton:pressed{background-color: #3790FA;color: #ffffff;}")
                    b.text_label.setStyleSheet(
                        "QLabel{font-size: 14px;font-weight: 400;color: #8F9399;line-height: 14px;text-align : center;}")
                b.show()
        btn.hide()
        btn.setStyleSheet("QPushButton{border-radius: 12px;font-size: 14px;font-weight: 400;line-height: 20px;background-color: #3790FA;text-align: center;color: #ffffff;text-align : center;}")
        btn.text_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #FFFFFF;line-height: 14px;text-align : center;}")
        btn.show()
        self.m_parent.type_name_label.setText(btn.text_label.text())
        self.m_parent.type_name_label.adjustSize()
        category = str(btn.text_label.text())

        # self.categorytab.move(btn.x(), self.categorytab.y())
        # self.categorytab.show()
        if (self.is_kydroid == False):
            # if(Globals.NOW_CATEGORY != btn.text_label.text()):
            self.click_categoy.emit(category,all_reflash)
        else:
            # if (Globals.NOW_KYDROID_CATEGORY != btn.text_label.text()):
            self.click_categoy.emit(category, all_reflash)
            Globals.NOW_KYDROID_CATEGORY = btn.text_label.text()

    #
    #函数：界面修改大小，修改分类按钮的位置
    #
    def reload_btn(self):
        btns = self.btnGroup.buttons()
        x = 0
        y = 0
        for b in btns:
            if (self.m_parent.width() - self.m_parent.widget_space < x + b.width()):
                b.move(0, y + 20+b.height())
                self.setFixedWidth(x)
                x = 0
                y = y + b.height() + 20
                self.setFixedHeight(y + b.height())
            else:
                b.move(x, y)
            x = x + b.width() + self.categoryspacing
            if(y == 0):
                self.setFixedSize(x ,b.height())
        self.categoryPanel.resize(self.width()-self.categoryspacing,self.height())
        # print(self.height())

    # add by kobe
    #
    # 函数名:重置类别栏
    # Function:reset categorybar
    #
    def reset_categorybar(self):
        # self.categorytab.hide()
        # self.categorytab.setStyleSheet("QLabel{background-image:url('res/categorytab.png');background-position:center;}")
        btns = self.btnGroup.buttons()
        for b in btns:
            if(Globals.THEME_COLOR == 0):
                b.setStyleSheet("QPushButton{border-radius: 12px;background-color:transparent;font-size: 14px;font-weight: 400;color: #606265;text-align: center;line-height: 20px;}QPushButton:hover{background: rgba(55, 144, 250, 0.12);color: #3790FA;}QPushButton:pressed{background-color: #3790FA;color: #ffffff;}")
            else:
                b.setStyleSheet("QPushButton{background-color:transparent;border-radius: 12px;font-size: 14px;font-weight: 400;color: #8F9399;text-align: center;line-height: 20px;text-align : center;}QPushButton:hover{background: rgba(55, 144, 250, 0.12);color: #3790FA;}QPushButton:pressed{background-color: #3790FA;color: #ffffff;}")


class CategoryButton(QPushButton):

    category_name = ''
    display_name = ''
    enter_btn = pyqtSignal(QPushButton)
    click_btn = pyqtSignal(QPushButton)

    def __init__(self, categoryname, displayname, parent=None):
        QPushButton.__init__(self, parent)
        self.is_clicked = False

        self.setFixedSize(52, 24)
        self.setCheckable(True)
        self.setFocusPolicy(Qt.NoFocus)

        self.text_label = QLabel(self)
        self.text_label.setAttribute(Qt.WA_TranslucentBackground)
        self.text_label.setAlignment(Qt.AlignCenter)
        self.text_label.move(12,6)

        self.category_name = categoryname
        self.display_name = displayname
        # if displayname=="特色":
        #     self.setText(_("Feature"))
        # elif displayname=="办公":
        #     self.setText(_("Office"))
        # elif displayname=="开发":
        #     self.setText(_("Develp"))
        # elif displayname=="图像":
        #     self.setText(_("Image"))
        # elif displayname=="影音":
        #     self.setText(_("Audio"))
        # elif displayname=="网络":
        #     self.setText(_("Net"))
        # elif displayname=="游戏":
        #     self.setText(_("Game"))
        # elif displayname=="教育":
        #     self.setText(_("Edu"))
        # elif displayname == "社交":
        #     self.setText(_("Social"))
        # elif displayname == "系统":
        #     self.setText(_("System"))
        # elif displayname == "其它":
        #     self.setText(_("Other"))
        zh_local = os.getenv("LANG")
        if("zh_CN" in zh_local):
            self.text_label.setText(displayname)
        else:
            self.text_label.setText(categoryname)
        if(Globals.THEME_COLOR == 0):
            self.setStyleSheet("QPushButton{background-color:transparent;border-radius: 12px;font-size: 14px;font-weight: 400;color: #606265;line-height: 20px;text-align : center;}QPushButton:hover{background: rgba(55, 144, 250, 0.12);color: #3790FA;}QPushButton:pressed{background-color: #3790FA;color: #ffffff;}")
            self.text_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #606265;line-height: 14px;text-align : center;}")
        else:
            self.setStyleSheet("QPushButton{background-color:transparent;border-radius: 12px;font-size: 14px;font-weight: 400;color: #8F9399;text-align: center;line-height: 20px;text-align : center;}QPushButton:hover{background: rgba(55, 144, 250, 0.12);color: #3790FA;}QPushButton:pressed{background-color: #3790FA;color: #ffffff;}")
            self.text_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #8F9399;line-height: 14px;text-align : center;}")
        self.text_label.adjustSize()
        self.text_label.move(12,int((self.height()-self.text_label.height())/2))
        self.setFixedWidth(self.text_label.width()+24)

    #
    # 函数：鼠标进入事件
    #
    def enterEvent(self, event):
        #self.enter_btn.emit(self)
        if(self.is_clicked == False):
            self.text_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #3790FA;line-height: 14px;text-align : center;}")
        else:
            self.text_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #FFFFFF;line-height: 14px;text-align : center;}")
        self.update()

    def mousePressEvent(self,event):
        if event.button() == Qt.LeftButton:
            self.text_label.setStyleSheet(
                "QLabel{font-size: 14px;font-weight: 400;color: #ffffff;line-height: 14px;text-align : center;}")
            self.setStyleSheet("QPushButton{border-radius: 12px;font-size: 14px;font-weight: 400;line-height: 20px;background-color: #3790FA;text-align: center;color: #ffffff;text-align : center;}")
            self.is_clicked = True
            self.click_btn.emit(self)
        else:
            event.ignore()

    def leaveEvent(self, event):
        if(self.is_clicked == False):
            if (Globals.THEME_COLOR == 0):
                self.text_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #606265;line-height: 14px;text-align : center;}")
            else:
                self.text_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #8F9399;line-height: 14px;text-align : center;}")
        else:
            self.text_label.setStyleSheet(
                "QLabel{font-size: 14px;font-weight: 400;color: #FFFFFF;line-height: 14px;text-align : center;}")
            self.setStyleSheet("QPushButton{border-radius: 12px;font-size: 14px;font-weight: 400;line-height: 20px;background-color: #3790FA;text-align: center;color: #ffffff;text-align : center;}")
