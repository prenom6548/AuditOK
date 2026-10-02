#!/usr/bin/python3
# -*- coding: utf-8 -*-


from PyQt5.QtGui import *
from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
from models.enums import BUTTON_ENTER,BUTTON_LEAVE,BUTTON_PRESSED,BUTTON_DISABLE
from ui.popular_app_card import *
from ui.systembutton import *
from ui.category_widget import *
from ui.ad_widget import *
from models.application import *
from models.apkinfo import *
from models.enums import setLongTextToElideFormat
import os
import sip
import gettext
gettext.bindtextdomain("kylin-software-center", "/usr/share/locale")
gettext.textdomain("kylin-software-center")
_ = gettext.gettext

class Drive_Widget(QFrame):
    clicked_btn = True
    is_first_click = True
    click_type_btn = pyqtSignal(str,bool)
    click_category_ad = pyqtSignal(Application)
    click_category_ad_apk = pyqtSignal(ApkInfo)
    click_none_card = pyqtSignal()
    card_width = 248
    card_height = 124
    card_space = 15
    widget_space = 50
    def __init__(self,w,h,parent = None):
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

        self.btn_widget = CategoryBar(Globals.DRIVE_CATEGORIES,self,False)
        self.btn_widget.setAttribute(Qt.WA_TranslucentBackground)
        # self.btn_widget.resize((self.btn_widget.visiblecategorycount - 1) * (
        #         self.btn_widget.itemwidth + self.btn_widget.categoryspacing) + self.btn_widget.itemwidth, 64)
        self.btn_widget.move(int((self.w - self.btn_widget.width()) / 2), 20)
        self.btn_widget.show()

        self.type_widget = QWidget(self)
        # self.type_widget.setAttribute(Qt.WA_TranslucentBackground)
        self.type_widget.resize(self.w,self.h-self.btn_widget.height())
        self.type_widget.move(0,self.btn_widget.height())
        self.type_widget.setStyleSheet("QWidget{background-color:transparent;border-radius:0px;}")
        self.type_widget.show()

        self.type_name_label = QLabel(self.type_widget)
        self.type_name_label.setAttribute(Qt.WA_TranslucentBackground)
        self.type_name_label.setFixedHeight(42)
        self.type_name_label.setStyleSheet("QLabel{font-size: 30px;font-weight: 600;color: #303133;line-height: 42px;}")
        self.type_name_label.setText(self.btn_widget.first_btn)
        self.type_name_label.adjustSize()
        self.type_name_label.move(self.widget_space,0)
        self.type_name_label.show()
        # self.type_name_label.hide()
        self.line_label = QLabel(self.type_widget)
        self.line_label.setFixedSize(self.width()-self.widget_space*2,1)
        self.line_label.setStyleSheet("QLabel{border: 1px solid #E5E7EE;background-color:#E5E7EE;}")
        # self.line_label.move(self.widget_space,0)
        # self.line_label.move(self.widget_space,self.type_name_label.y()+self.type_name_label.height()+10)

        self.type_show_card_widget = QWidget(self.type_widget)
        # self.type_show_card_widget.setAttribute(Qt.WA_TranslucentBackground)
        self.type_show_card_widget.resize(self.type_widget.width() - self.widget_space - self.widget_space,
                                          self.type_widget.height() - self.type_name_label.height())
        self.type_show_card_widget.move(self.widget_space, self.line_label.y() + self.line_label.height()+10)
        self.type_show_card_widget.setStyleSheet("QWidget{background-color:transparent;border-radius:0px;}")
        self.type_show_card_widget.show()
        self.type_show_card_widget.children_count = 0

        self.one_click_download = QPushButton(self.type_widget)
        self.one_click_download.setFocusPolicy(Qt.NoFocus)
        self.one_click_download.setText(_("all down"))
        self.one_click_download.adjustSize()
        self.one_click_download.setFixedSize(80, 26)
        self.one_click_download.setStyleSheet(
            "QPushButton{border:0px;font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;border:0px solid #ff0000;}QPushButton:hover{background: rgba(55, self.widget_space, 250, 0.12);border-radius: 12px;}QPushButton:pressed{background: rgba(55, self.widget_space, 250, 0.12);border-radius: 12px;}")
        self.one_click_download.move(self.type_show_card_widget.width() - self.one_click_download.width() + self.type_show_card_widget.x() - 10,
                                     self.type_name_label.y())
        self.one_click_download.setEnabled(False)

        self.all_check_btn = QPushButton(self.type_widget)
        self.all_check_btn.setFocusPolicy(Qt.NoFocus)
        self.all_check_btn.setText(_("Select all"))
        self.all_check_btn.setFixedSize(80, 26)
        self.all_check_btn.setStyleSheet(
            "QPushButton{font-size: 14px;font-weight: 400;color: #303133;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, self.widget_space, 250, 0.12);font-weight: 500;}")
        self.all_check_btn.move(self.one_click_download.x() - self.all_check_btn.width() + 5, self.type_name_label.y())

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


        self.scrollArea = QScrollArea(self)
        self.scrollArea.setWidget(self.type_widget)
        self.scrollArea.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self.scrollArea.horizontalScrollBar().setEnabled(False)
        self.scrollArea.setVerticalScrollBarPolicy(Qt.ScrollBarAsNeeded)
        self.scrollArea.setGeometry(0, int(self.btn_widget.height()+self.btn_widget.y()+16), self.type_widget.width()-10, self.height()-self.btn_widget.height()-10)
        self.type_widget.resize(self.width(),int(self.type_name_label.y()+self.type_name_label.height()+self.type_show_card_widget.height()))
        self.scrollArea.show()
        self.re_move()

    #
    #函数：初始化界面的样式
    #
    def init_style(self):
        if(Globals.THEME_COLOR == 0):
            self.setStyleSheet("QFrame{border-bottom-left-radius:12px;border-bottom-right-radius:12px; background-color:#ffffff;}")
            self.btn_widget.setStyleSheet("QWidget{background-color:#ffffff;border-radius:0px;}")
            palette1 = QPalette()
            palette1.setColor(self.type_widget.backgroundRole(), QColor(255, 255, 255))  # 设置背景颜色
            self.type_widget.setPalette(palette1)
            self.type_widget.setAutoFillBackground(True)
            # self.type_widget.setStyleSheet("QWidget{background-color:#ffffff;border-radius:0px;}")
            # self.type_show_card_widget.setStyleSheet("QWidget{background-color:#ffffff;border-radius:0px;}")
            self.type_name_label.setStyleSheet("QLabel{font-size: 24px;font-weight: 500;color: #303133;line-height: 42px;}")
            self.line_label.setStyleSheet("QLabel{border: 1px solid #E5E7EE;background-color:#E5E7EE;}")
            self.loadinglabel.setMovie(self.loadingmove)
            self.loadingmove.start()
            if (self.one_click_download.isEnabled()):
                self.one_click_download.setStyleSheet(
                    "QPushButton{border:0px;font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;border:0px solid #ff0000;}QPushButton:hover{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}QPushButton:pressed{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}")
            else:
                self.one_click_download.setStyleSheet(
                    "QPushButton{font-size:14px;font-weight: 500;color: #8f9399;line-height: 20px;border:0px solid #ff0000;}")
            self.all_check_btn.setStyleSheet(
                "QPushButton{font-size: 14px;font-weight: 400;line-height: 20px;border:0px;font-weight: 500;color: #606265;line-height: 20px;}QPushButton:hover{background: rgba(55, 144, 250, 0.12);color: #3790FA;border-radius: 12px;}PushButton:pressed{background: rgba(55, 144, 250, 0.12);color: #3790FA;border-radius: 12px;}")
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
                    btn.setStyleSheet(
                        "QPushButton{background-color:transparent;border-radius: 12px;font-size: 14px;font-weight: 400;color: #606265;text-align: center;line-height: 20px;text-align : center;}QPushButton:hover{background: rgba(55, 144, 250, 0.12);color: #3790FA;}QPushButton:pressed{background-color: #3790FA;color: #ffffff;}")
        else:
            self.setStyleSheet("QFrame{border-bottom-left-radius:12px;border-bottom-right-radius:12px; background-color:#1F2022;}")
            self.btn_widget.setStyleSheet("QWidget{background-color:#1F2022;border-radius:0px;}")
            palette1 = QPalette()
            palette1.setColor(self.type_widget.backgroundRole(), QColor(31, 32, 34))  # 设置背景颜色
            self.type_widget.setPalette(palette1)
            self.type_widget.setAutoFillBackground(True)
            # self.type_widget.setStyleSheet("QWidget{background-color:#1F2022;border-radius:0px;}")
            # self.type_show_card_widget.setStyleSheet("QWidget{background-color:#1F2022;border-radius:0px;}")
            self.type_name_label.setStyleSheet("QLabel{font-size: 24px;font-weight: 500;color: #F9F9F9;line-height: 42px;}")
            self.line_label.setStyleSheet("QLabel{border: 1px solid #393939;background-color:#393939;}")
            self.loadinglabel.setMovie(self.loadingmove_dark)
            self.loadingmove_dark.start()
            if (self.one_click_download.isEnabled()):
                self.one_click_download.setStyleSheet(
                    "QPushButton{border:0px;font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;border:0px solid #ff0000;}QPushButton:hover{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}QPushButton:pressed{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}")
            else:
                self.one_click_download.setStyleSheet(
                    "QPushButton{font-size:14px;font-weight: 500;color: #8f9399;line-height: 20px;border:0px solid #ff0000;}")
            self.all_check_btn.setStyleSheet("QPushButton{font-size: 14px;border:0px;font-weight: 500;color: #F9F9F9;line-height: 20px;}QPushButton:hover{background: rgba(55, 144, 250, 0.12);color: #3790FA;border-radius: 12px;}PushButton:pressed{background: rgba(55, 144, 250, 0.12);color: #3790FA;border-radius: 12px;}")
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

    #
    #函数：重新设置界面的控件位置和大小
    #
    def re_move(self):
        # self.btn_widget.resize((self.btn_widget.visiblecategorycount - 1) * (
        #         self.btn_widget.itemwidth + self.btn_widget.categoryspacing) + self.btn_widget.itemwidth, 64)
        self.btn_widget.reload_btn()
        self.btn_widget.move(int((self.width() - self.btn_widget.width()) / 2), 20)
        self.type_widget.resize(self.width(), self.height() - self.btn_widget.height()-self.btn_widget.y())
        self.type_name_label.move(self.widget_space, 0)
        self.type_widget.resize(self.type_widget.width(),0)
        self.type_show_card_widget.resize(self.type_widget.width() - self.widget_space - self.widget_space,self.type_widget.height() - self.type_name_label.height())
        self.loadinglabel.move(int((self.width() - self.loadinglabel.width()) / 2),int((self.height() - self.loadinglabel.height()) / 2))
        self.line_label.setFixedSize(self.width() - self.widget_space * 2, 1)
        self.line_label.move(self.widget_space, self.type_name_label.height()+10)
        self.type_show_card_widget.move(self.widget_space, self.line_label.y() + self.line_label.height()+10)
        self.one_click_download.move(
            self.type_show_card_widget.width() - self.one_click_download.width() + self.type_show_card_widget.x() - 10,
            self.type_name_label.y())
        self.all_check_btn.move(self.one_click_download.x() - self.all_check_btn.width(), self.type_name_label.y())
        self.scrollArea.setGeometry(0, int(self.btn_widget.height() + self.btn_widget.y()) + 16,
                                    self.type_widget.width() - 10,
                                    int(self.height() - self.btn_widget.height() - self.btn_widget.y() - 30))
        self.type_show_card_widget.move(self.widget_space, int(self.type_name_label.y() + self.type_name_label.height()))
        self.reload_card(self.type_show_card_widget, self.scrollArea)
        self.type_widget.resize(self.type_widget.width(),
                                int(self.type_name_label.y() + self.type_name_label.height() + self.type_show_card_widget.height()))
        # self.scrollArea.setGeometry(0, self.btn_widget.height()+self.btn_widget.y()+16, self.type_widget.width() - 10, self.height() - self.btn_widget.height() - 10)
        # self.reload_card(self.type_show_card_widget,self.scrollArea)
        # self.type_widget.resize(self.type_widget.width(), self.type_show_card_widget.height()+self.line_label.y()+self.line_label.height())
        # self.type_widget.move(0, self.btn_widget.height() + self.btn_widget.y())


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
            self.m_parent.mw.hide_tip_widget()
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
