#!/usr/bin/python3
# -*- coding: utf-8 -*-


from PyQt5.QtGui import *
from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
from models.enums import BUTTON_ENTER,BUTTON_LEAVE,BUTTON_PRESSED,BUTTON_DISABLE,AppMsg,ClickCard
from ui.popular_app_card import *
from models.application import *
import gettext
gettext.bindtextdomain("kylin-software-center", "/usr/share/locale")
gettext.textdomain("kylin-software-center")
_ = gettext.gettext


class Popular_Applications_Widget(QWidget):
    app_num = 0
    card_list = []
    ad_move_left_time = QTimer()
    ad_move_right_time = QTimer()
    final_x_position = 0
    final_x_position2 = 0
    click_card = pyqtSignal(Application,bool,int)
    def __init__(self,w,mw,parent=None):
        QWidget.__init__(self,parent)
        self.m_parent = parent
        self.appmsg = None
        self.w = w
        self.mw = mw
        self.card_width = 288
        self.card_height = 190
        self.num = 3
        self.m_parent = parent
        self.init_ui()
        self.init_connect()
        self.init_style()
        self.add_card_list = []
        self.show_card_list = []

    def init_ui(self):
        self.resize(self.w,240)
        self.setFocusPolicy(Qt.NoFocus)
        self.setWindowFlags(Qt.FramelessWindowHint)
        self.setAttribute(Qt.WA_TranslucentBackground)
        self.locale_lan = QLocale.system().name()
        self.titel_label = QLabel(self)
        self.titel_label.setAttribute(Qt.WA_TranslucentBackground)
        if self.locale_lan == "zh_CN":
            self.titel_label.setFixedSize(150,26)
        else:
            self.titel_label.setFixedSize(160,36)
        self.titel_label.setText(_("Popular apps"))
        self.titel_label.move(5,0)

        self.more_btn = QPushButton(self)
        self.more_btn.setAttribute(Qt.WA_TranslucentBackground)
        self.more_btn.setFocusPolicy(Qt.NoFocus)
        self.more_btn.setStyleSheet(
            "QPushButton{border: 0px;font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;}")
        self.more_btn.setText(_("more"))
        self.more_btn.adjustSize()
        self.more_btn.move(self.width() - self.more_btn.width(),
                           self.titel_label.y() + int((self.titel_label.height() - self.more_btn.height()) / 2))
        self.more_btn.show()

        self.app_list_widget = QWidget(self)
        self.app_list_widget.setAttribute(Qt.WA_TranslucentBackground)
        self.app_list_widget.setFixedHeight(self.height()-self.titel_label.y()-self.titel_label.height())
        self.app_list_widget.move(0,self.titel_label.height())

        self.app_list_widget2 = QWidget(self)
        self.app_list_widget2.setFixedHeight(self.height()-self.titel_label.y()-self.titel_label.height())
        self.app_list_widget2.setAttribute(Qt.WA_TranslucentBackground)
        self.app_list_widget2.move(0,  self.titel_label.height())

        self.left_btn = QPushButton(self)
        self.left_btn.setAttribute(Qt.WA_TranslucentBackground)
        self.left_btn.installEventFilter(self)
        self.left_btn.setFocusPolicy(Qt.NoFocus)
        self.left_btn.resize(60,60)
        if(Globals.THEME_COLOR == 0):
            self.left_btn.setStyleSheet("QPushButton{background-color:transparent;border-radius:30px;border-image: url(./res/ads_left1.png);}QPushButton:hover{border-image: url(./res/ads_left2.png);}QPushButton:pressed{border-image: url(./res/ads_left2.png);}")
        else:
            self.left_btn.setStyleSheet("QPushButton{background-color:transparent;border-radius:30px;border-image: url(./res/ads_left_dark1.png);}QPushButton:hover{border-image: url(./res/ads_left_dark2.png);}QPushButton:pressed{border-image: url(./res/ads_left_dark2.png);}")
        self.left_btn.move(6,int((self.height()-self.left_btn.height())/2)+21)
        self.left_btn.hide()

        self.right_btn = QPushButton(self)
        self.right_btn.setAttribute(Qt.WA_TranslucentBackground)
        self.right_btn.installEventFilter(self)
        self.right_btn.setFocusPolicy(Qt.NoFocus)
        self.right_btn.resize(60, 60)
        self.right_btn.setStyleSheet("QPushButton{background-color:transparent;border-radius:30px;border-image: url(./res/ads_right1.png);}QPushButton:hover{border-image: url(./res/ads_right2.png);}QPushButton:pressed{border-image: url(./res/ads_right2.png);}")
        self.right_btn.move(self.width()-self.right_btn.width()-6,int((self.height()-self.left_btn.height())/2)+21)
        self.right_btn.hide()

    def re_move(self):
        self.left_btn.move(6, int((self.height() - self.left_btn.height()) / 2) + 21)
        self.right_btn.move(self.width() - self.right_btn.width()-6,
                            int((self.height() - self.left_btn.height()) / 2) + 21)
        if (self.app_list_widget.width() < self.width()):
            self.app_list_widget.move(0,self.app_list_widget.y())
            self.app_list_widget2.hide()
        if(int(self.width()/(self.card_width+20)) <3 and int(self.app_list_widget.width()/(self.card_width+20)) > 0):
            self.num = int(self.width()/(self.card_width+20))
        else:
            self.num = 3

    def init_style(self):
        self.titel_label.setStyleSheet("QLabel{font-size: 24px;font-weight: 500;color: #303133;line-height: 33px;}")

    def get_lists(self, applist):
        self.applist = []
        self.applist.extend(applist)

    def init_connect(self):
        self.right_btn.clicked.connect(self.slot_click_right_btn)
        self.left_btn.clicked.connect(self.slot_click_left_btn)
        self.ad_move_left_time.timeout.connect(self.slot_slide_left)
        self.ad_move_right_time.timeout.connect(self.slot_slide_right)

    #
    #函数：添加界面到热门应用
    #
    def add_app_widget(self,app_msg,card_path):
        itemwidget = Popular_Card_Widget(app_msg,self.card_width,self.card_height,card_path, self.app_list_widget)
        itemwidget2 = Popular_Card_Widget(app_msg, self.card_width,self.card_height,card_path,self.app_list_widget2)
        itemwidget.clicked_card.connect(self.slot_click_card)
        itemwidget2.clicked_card.connect(self.slot_click_card)
        self.add_card_list.append(app_msg.app_id)
        #self.card_list.append(app_widget)
        if(self.app_num == 0):
            itemwidget.move(5,15)
            itemwidget2.move(5,15)
            self.app_list_widget.setFixedWidth(itemwidget.width()+10)
            self.app_list_widget2.setFixedWidth(itemwidget.width()+10)
            self.app_list_widget2.move(self.app_list_widget.x()+self.app_list_widget.width(),self.titel_label.height())
        else:
            itemwidget.move(self.app_num*(itemwidget.width()+20)+5,15)
            itemwidget2.move(self.app_num*(itemwidget.width()+20)+5,15)
            self.app_list_widget.setFixedWidth(self.app_num*(itemwidget.width()+20)+itemwidget.width()+10)
            self.app_list_widget2.setFixedWidth(self.app_num*(itemwidget.width()+20)+itemwidget.width()+10)
            self.app_list_widget2.move(self.app_list_widget.x() + self.app_list_widget.width()-10,self.titel_label.height())
        itemwidget.show()
        itemwidget2.show()
        self.app_num = self.app_num +1

    #
    #函数：处理点击卡片
    #
    def slot_click_card(self,app_msg):
        self.mw.save_click_hot_card(app_msg.app_id)
        self.click_card.emit(app_msg,True,ClickCard.HOT)

    #
    # 函数：点击向右的箭头
    #
    def slot_click_right_btn(self):
        x = self.app_list_widget.x()
        x2 = self.app_list_widget2.x()
        if (self.final_x_position == 0 and self.final_x_position2 == 0):
            self.final_x_position = x - (self.card_width + 20)*self.num
            self.final_x_position2 = x2 - (self.card_width + 20)*self.num
            self.ad_move_left_time.start(1)
        else:
            self.final_x_position = self.final_x_position - (self.card_width + 20)*self.num
            self.final_x_position2 = self.final_x_position2 - (self.card_width + 20)*self.num
            self.ad_move_left_time.start(1)
        self.right_btn.setEnabled(False)
        self.left_btn.setEnabled(False)
    #
    # 函数：向左滑动
    #
    def slot_slide_left(self):
        if (self.app_list_widget.x() <= 0 ):
            sub = self.app_list_widget2.x() - self.final_x_position2
            self.app_list_widget2.move(self.app_list_widget.x() + self.app_list_widget.width() + 10, self.titel_label.height())
            self.final_x_position2 = self.app_list_widget2.x() - sub
        if (self.app_list_widget2.x() <= 0):
            sub = self.app_list_widget.x() - self.final_x_position
            self.app_list_widget.move(self.app_list_widget2.x() + self.app_list_widget2.width() + 10, self.titel_label.height())
            self.final_x_position = self.app_list_widget.x() - sub
        x = int(self.app_list_widget.x())
        x2 = int(self.app_list_widget2.x())
        if (x > self.final_x_position):
            self.ad_move_left_time.stop()
            if (x - 40 <= self.final_x_position):
                self.app_list_widget.move(self.final_x_position, self.titel_label.height())
            else:
                self.app_list_widget.move(x - 40, self.titel_label.height())
            if (x2 - 40 <= self.final_x_position2):
                self.app_list_widget2.move(self.final_x_position2, self.titel_label.height())
            else:
                self.app_list_widget2.move(x2 - 40, self.titel_label.height())
            self.app_list_widget.show()
            self.app_list_widget2.show()
            self.ad_move_left_time.start(1)
            self.right_btn.setEnabled(False)
            self.left_btn.setEnabled(False)
        else:
            self.ad_move_left_time.stop()
            self.app_list_widget.move(self.final_x_position, self.titel_label.height())
            self.app_list_widget2.move(self.final_x_position2, self.titel_label.height())
            if(Globals.NEED_SEND_HOT_EXPOSURE == True):
                if (self.app_list_widget.x() < self.width() or (self.app_list_widget.x() + self.app_list_widget.width() < self.width())):
                    for card in self.app_list_widget.children():
                        if (card.x() + self.app_list_widget.x() <= self.width() and card.x() + self.app_list_widget.x() >= 0):
                            if (card.appmsg.app_id not in self.show_card_list):
                                self.show_card_list.append(card.appmsg.app_id)
                                self.mw.save_hot_exposure(card.appmsg.app_id)
                            else:
                                b = [val for val in self.add_card_list if val not in self.show_card_list]
                                if (b == []):
                                    Globals.NEED_SEND_HOT_EXPOSURE = False
                                    self.show_card_list.clear()
                if (self.app_list_widget2.x() < self.width() or (self.app_list_widget2.x() + self.app_list_widget2.width() < self.width())):
                    for card in self.app_list_widget2.children():
                        if (card.x() + self.app_list_widget2.x() <= self.width() and card.x() + self.app_list_widget2.x() >= 0):
                            if (card.appmsg.app_id not in self.show_card_list):
                                self.show_card_list.append(card.appmsg.app_id)
                                self.mw.save_hot_exposure(card.appmsg.app_id)
                            else:
                                b = [val for val in self.add_card_list if val not in self.show_card_list]
                                if (b == []):
                                    Globals.NEED_SEND_HOT_EXPOSURE = False
                                    self.show_card_list.clear()
            else:
                self.show_card_list.clear()
            self.final_x_position = 0
            self.final_x_position2 = 0
            self.app_list_widget.show()
            self.app_list_widget2.show()
            self.right_btn.setEnabled(True)
            self.left_btn.setEnabled(True)

    #
    # 函数：点击向左的箭头
    #
    def slot_click_left_btn(self):
        x = self.app_list_widget.x()
        x2 = self.app_list_widget2.x()
        if (self.final_x_position == 0 and self.final_x_position2 == 0):
            self.final_x_position = x + (self.card_width + 20)*self.num
            self.final_x_position2 = x2 + (self.card_width + 20)*self.num
            self.ad_move_right_time.start(1)
        else:
            self.ad_move_right_time.stop()
            self.final_x_position = self.final_x_position + (self.card_width + 20)*self.num
            self.final_x_position2 = self.final_x_position2 + (self.card_width + 20)*self.num
            self.ad_move_right_time.start(1)
        self.right_btn.setEnabled(False)
        self.left_btn.setEnabled(False)

    #
    # 函数向右滑动
    #
    def slot_slide_right(self):
        if (self.app_list_widget.x() >= 0):
            sub = self.app_list_widget2.x() - self.final_x_position2
            self.app_list_widget2.move(self.app_list_widget.x() - self.app_list_widget2.width() - 20+10, self.titel_label.height())
            self.final_x_position2 = self.app_list_widget2.x() - sub
        if (self.app_list_widget2.x() >= 0):
            sub = self.app_list_widget.x() - self.final_x_position
            self.app_list_widget.move(self.app_list_widget2.x() - self.app_list_widget.width() - 20+10, self.titel_label.height())
            self.final_x_position = self.app_list_widget.x() - sub
        x = int(self.app_list_widget.x())
        x2 = int(self.app_list_widget2.x())
        if (x < self.final_x_position):
            self.ad_move_right_time.stop()
            if (x + 40 >= self.final_x_position):
                self.app_list_widget.move(self.final_x_position, self.titel_label.height())
            else:
                self.app_list_widget.move(x + 40, self.titel_label.height())
            if (x2 + 40 >= self.final_x_position2):
                self.app_list_widget2.move(self.final_x_position2, self.titel_label.height())
            else:
                self.app_list_widget2.move(x2 + 40, self.titel_label.height())
            self.app_list_widget.show()
            self.app_list_widget2.show()
            self.ad_move_right_time.start(1)
            self.right_btn.setEnabled(False)
            self.left_btn.setEnabled(False)
        else:
            self.ad_move_right_time.stop()
            self.app_list_widget.move(self.final_x_position, self.titel_label.height())
            self.app_list_widget2.move(self.final_x_position2, self.titel_label.height())
            if(Globals.NEED_SEND_HOT_EXPOSURE == True):
                if (self.app_list_widget.x() < self.width() or (self.app_list_widget.x() + self.app_list_widget.width() < self.width())):
                    for card in self.app_list_widget.children():
                        if (card.x() + self.app_list_widget.x() <= self.width() and card.x() + self.app_list_widget.x() >= 0):
                            if (card.appmsg.app_id not in self.show_card_list):
                                self.show_card_list.append(card.appmsg.app_id)
                                self.mw.save_hot_exposure(card.appmsg.app_id)
                            else:
                                b = [val for val in self.add_card_list if val not in self.show_card_list]
                                if (b == []):
                                    Globals.NEED_SEND_HOT_EXPOSURE = False
                                    self.show_card_list.clear()
                if (self.app_list_widget2.x() < self.width() or (self.app_list_widget2.x() + self.app_list_widget2.width() < self.width())):
                    for card in self.app_list_widget2.children():
                        if (card.x() + self.app_list_widget2.x() <= self.width() and card.x() + self.app_list_widget2.x() >= 0):
                            if (card.appmsg.app_id not in self.show_card_list):
                                self.show_card_list.append(card.appmsg.app_id)
                                self.mw.save_hot_exposure(card.appmsg.app_id)
                            else:
                                b = [val for val in self.add_card_list if val not in self.show_card_list]
                                if (b == []):
                                    Globals.NEED_SEND_HOT_EXPOSURE = False
                                    self.show_card_list.clear()
            else:
                self.show_card_list.clear()
            self.final_x_position = 0
            self.final_x_position2 = 0
            self.app_list_widget.show()
            self.app_list_widget2.show()
            self.right_btn.setEnabled(True)
            self.left_btn.setEnabled(True)


    # 鼠标进入事件

    def enterEvent(self, event):
        self.status = BUTTON_ENTER
        if(self.app_list_widget.width() > self.width()):
            self.right_btn.show()
            self.left_btn.show()

    #
    # 函数：鼠标离开事件
    #

    def leaveEvent(self, event):
        self.status = BUTTON_LEAVE
        self.right_btn.hide()
        self.left_btn.hide()

    #
    # 函数：鼠标按下事件
    #
    def mousePressEvent(self, event):
        if event.button() == Qt.LeftButton:
            if (hasattr(self.mw, "title_bar")):
                self.mw.title_bar.exit_login_btn.hide()
                self.mw.title_bar.senior_search.hide()
            if (hasattr(self.mw, "search_tip_widget")):
                self.mw.search_tip_widget.hide()
            self.status = BUTTON_PRESSED
        else:
            event.ignore()
