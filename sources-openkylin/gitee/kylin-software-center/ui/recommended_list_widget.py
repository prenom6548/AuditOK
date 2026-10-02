#!/usr/bin/python3
# -*- coding: utf-8 -*-


from PyQt5.QtGui import *
from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
import gettext
gettext.bindtextdomain("kylin-software-center", "/usr/share/locale")
gettext.textdomain("kylin-software-center")
_ = gettext.gettext

class Recommended_List_Widget(QWidget):
    cardcount = 0
    itemwidth = -1
    itemheight = -1
    space = 0
    card_width = 305
    card_height = 190
    def __init__(self,w,parent=None):
        QWidget.__init__(self, parent)
        self.w = w
        self.init_ui()


    def init_ui(self):
        self.resize(self.w, 450)
        self.setFocusPolicy(Qt.NoFocus)
        self.setWindowFlags(Qt.FramelessWindowHint)
        self.setAttribute(Qt.WA_TranslucentBackground)
        self.locale_lan = QLocale.system().name()
        self.titellabel = QLabel(self)
        self.titellabel.setAttribute(Qt.WA_TranslucentBackground)
        self.titellabel.setStyleSheet("QLabel{font-size: 24px;font-weight: 500;color: #303133;line-height: 33px;}")
        if self.locale_lan =="zh_CN":
            self.titellabel.setFixedHeight(26)
        else:
            self.titellabel.setFixedHeight(36)
        self.titellabel.setText(_("We play a lot. Are you sure you don't want to see it?"))
        # self.titellabel.adjustSize()
        self.titellabel.move(10, 0)

        self.card_widget = QWidget(self)
        self.card_widget.setAttribute(Qt.WA_TranslucentBackground)
        if self.locale_lan =="zh_CN":
            self.card_widget.move(0,self.titellabel.height()+20)
        else:
            self.card_widget.move(0,self.titellabel.height()+10)
        self.card_widget.resize(self.width(),self.height())

    #
    # 函数名:添加卡
    # Function:add card
    #
    def add_card(self, card):
        self.space = 16
        h_num = int((self.card_widget.width()-20) / (305))
        card_width = int(((self.card_widget.width()-20)-(h_num-1)*16)/h_num)
        card_height = int((card_width/305)*190)
        card.resize(card_width,card_height)
        card.re_move()
        # if(self.cardcount >=h_num*2):
        #     return
        v_num = int(self.cardcount/h_num)
        if(int(self.cardcount/h_num) == 0):
            if(self.cardcount%h_num == 0):
                x = 10
                y = 10
            else:
                x = int((card.width() + self.space)*(self.cardcount%h_num))+10
                y = 10
        else:
            if (self.cardcount % h_num == 0):
                x = 10
                y = 10+int(card.height())*(v_num)+20*v_num
            else:
                x = int((card.width() + self.space)*(self.cardcount%h_num))+10
                y = 10+int(card.height())*(v_num)+20*v_num
        card.move(x, y)
        self.cardcount = self.cardcount + 1
        # print("LLLLLLL",v_num)
        if(v_num < 2):
            self.card_widget.resize(self.width(), (card.height() + 20) * (v_num+1) + 20)
            self.resize(self.width(), (card.height() + 20) * (v_num+1) + 33+10)

    def reload_card(self):
        self.space = 16
        self.card_widget.resize(self.width(),self.height()-33)
        cards = self.card_widget.children()
        self.space = 16
        h_num = int((self.card_widget.width() - 20) / (305))
        get_card_width = int(((self.card_widget.width() - 20) - (h_num - 1) * 16) / h_num)
        get_card_height = int((get_card_width / 305) * 190)
        self.cardcount = 0
        for card in cards:
            v_num = int(self.cardcount / h_num)
            card.resize(get_card_width, get_card_height)
            card.re_move()
            if (int(self.cardcount / h_num) == 0):
                if (self.cardcount % h_num == 0):
                    x = 10
                    y = 10
                else:
                    x = int((card.width() + self.space) * (self.cardcount % h_num)) + 10
                    y = 10
            else:
                if (self.cardcount % h_num == 0):
                    x = 10
                    y = 10 + int(card.height()) * (v_num) + 20 * v_num
                else:
                    x = int((card.width() + self.space) * (self.cardcount % h_num)) + 10
                    y = 10 + int(card.height()) * (v_num) + 20 * v_num
            card.move(x, y)
            # if(card.isHidden()):
            #     card.show()
            self.cardcount = self.cardcount + 1
            if(v_num < 2):
                self.card_widget.resize(self.width(),(get_card_height+20)*(v_num+1)+20)
                self.resize(self.width(),(get_card_height+20)*(v_num+1)+33+10)
            
