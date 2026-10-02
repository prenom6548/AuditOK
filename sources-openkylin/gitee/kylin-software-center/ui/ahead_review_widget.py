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
from models.enums import UBUNTUKYLIN_DATA_PATH
# from models.enums import CACHE_DIR,BUTTON_PRESSED,BUTTON_ENTER,BUTTON_LEAVE,BUTTON_DISABLE,AppMsg
#


class Ahead_Review_Widget(QWidget,Signals):
    ad_move_left_time = QTimer()
    ad_move_right_time = QTimer()
    final_x_position = 0
    x = 0
    maxpage = ''
    mainwindow = ''
    debfile = ''
    sshotcount = 0
    bigsshot = ''
    reviewpage = ''
    currentreviewready = ''
    debfile = None
    Row_er = 0
    Expand_height = 0
    is_clean_star = 0
    add_revieheight = 0
    Line_height = 0
    text_lidit = ""
    dest_str = ""
    SET_ISNTALL = False
    count = 0
    commend_height = 260
    # i = 0
    reviewlist1=[]
    alist = []
    def __init__(self,reviewlist1,app_detail,parent=None):
        QWidget.__init__(self,parent)
        # self.dblist = []
        # applist1 = []
        # self.app = app
        # self.reviewpage = reviewpage
        self.app_detail = app_detail
        self.ratings_average = 0
        self.messageBox = MessageBox(self)
        self.mainwindow = parent
        self.mainwindow.installEventFilter(self)
        self.mouse_press = False
        # self.setFixedSize(1340,278)

        self.setWindowFlags(Qt.FramelessWindowHint)
        # self.setAttribute(Qt.WA_TranslucentBackground)
        #
        self.left_btn = QPushButton(self.mainwindow)
        self.left_btn.setFocusPolicy(Qt.NoFocus)
        self.left_btn.resize(31, 31)
        if(Globals.THEME_COLOR == 0):
            self.left_btn.setStyleSheet(
                "QPushButton{background-color:transparent;border-radius:20px;image: url(./res/left_false_btn.png);}")
        else:
            self.left_btn.setStyleSheet(
                "QPushButton{background-color:transparent;border-radius:20px;image: url(./res/left_false_dark.png);}")
        self.left_btn.move(10, int((self.commend_height - self.left_btn.height()) / 2))
        self.left_btn.hide()
        self.left_btn.setEnabled(False)



        self.widget = QWidget(self.mainwindow)
        self.widget.resize(self.app_detail.editorial_comments_widget.width(), self.commend_height)
        self.widget.move(self.left_btn.width()+20,0)
        self.widget.setStyleSheet(".QWidget{border-radius:12px;background-color:transparent;}")
        self.widget.installEventFilter(self)
        self.review_card_width = int((self.widget.width() - 20) / 2)

        self.right_btn = QPushButton(self.mainwindow)
        self.right_btn.setFocusPolicy(Qt.NoFocus)
        self.right_btn.resize(31, 31)
        if (Globals.THEME_COLOR == 0):
            self.right_btn.setStyleSheet("QPushButton{background-color:transparent;border-radius:20px;image: url(./res/right_true_btn.png);}QPushButton:hover{background-color:transparent;border-radius:20px;image: url(./res/right_true_clicked_btn.png);}QPushButton:pressed{background-color:transparent;border-radius:20px;image: url(./res/right_true_clicked_btn.png);}")
        else:
            self.right_btn.setStyleSheet("QPushButton{background-color:transparent;border-radius:20px;image: url(./res/right_true_btn_dark.png);}QPushButton:hover{background-color:transparent;border-radius:20px;image: url(./res/right_true_click_dark.png);}QPushButton:pressed{background-color:transparent;border-radius:20px;image: url(./res/right_true_click_dark.png);}")
        self.right_btn.move(self.widget.width() + self.widget.x() + 10,
                            int((self.commend_height - self.right_btn.height()) / 2))
        self.right_btn.hide()

        # 评论的大widget，对applist其中每一条评论都实例化一个小widget

        self.review_widget1 = QWidget(self.widget)
        self.review_widget1.setStyleSheet(".QWidget{border:1px ;border-radius:12px;background:transparent;}")
        self.review_widget1.move(0, 0)
        self.review_widget1.installEventFilter(self)

        self.right_btn.clicked.connect(self.slot_click_right_btn)
        self.left_btn.clicked.connect(self.slot_click_left_btn)
        self.ad_move_left_time.timeout.connect(self.slot_slide_left)
        self.ad_move_right_time.timeout.connect(self.slot_slide_right)


        self.total_review_count = len(reviewlist1)
        self.show_review_count = 0
        self.right_click_count = 0

        self.show_review_x = 0
        self.show_review_y = 0
        self.reviewlist1 = reviewlist1

        # for i in range(self.total_review_count):
        #     self.applist1.append(i)
        #     self.innit_ui(self.applist1)

        self.innit_ui(self.reviewlist1)


    def innit_ui(self,reviewlist):
        self.review_widget1.resize(self.review_card_width * (len(reviewlist)) + 20 * (len(reviewlist) -1),self.commend_height)
        for k in range(self.total_review_count):
            ratings_average = self.reviewlist1[k].user_rating
            self.review1 = ReviewLabel(ratings_average, self.review_card_width, reviewlist[k], self.app_detail,self.review_widget1)
            self.review1.move(self.review1.width() * k + 20 * k, 0)

        self.show_review_count = 10
        self.show_review_x = self.review1.width() * 10 + 20 * 10

        self.count = len(reviewlist)

    def re_move(self):
        self.left_btn.move(10, int((self.commend_height - self.left_btn.height()) / 2))
        self.widget.resize(self.app_detail.editorial_comments_widget.width(), self.commend_height)
        self.widget.move(self.left_btn.width() + 20, 0)
        self.review_card_width = int((self.widget.width() - 20) / 2)
        self.right_btn.move(self.widget.width() + self.widget.x() + 10,
                            int((self.commend_height - self.right_btn.height()) / 2))
        self.review_widget1.move(0, 0)
        self.review_widget1.setFixedSize(self.review_card_width * self.count + 20 * (self.count - 1), self.commend_height)
        reviews = self.review_widget1.children()
        n = 0
        for i in reviews:
            i.setFixedSize(self.review_card_width,self.commend_height)
            i.ReviewWidget.resize(self.review_card_width,self.commend_height)
            i.re_move()
            i.move(self.review_card_width * n + 20 * n, 0)
            n = n+1
        if(self.count <= 2 ):
            self.right_btn.setEnabled(False)
            self.left_btn.setEnabled(False)
            self.check_btn_styleSheet()
        else:
            self.right_btn.setEnabled(True)
            self.left_btn.setEnabled(False)
            self.check_btn_styleSheet()

    def check_btn_styleSheet(self):
        if(Globals.THEME_COLOR == 0):
            if(self.left_btn.isEnabled()):
                self.left_btn.setStyleSheet("QPushButton{background-color:transparent;border-radius:20px;image: url(./res/left_true_btn.png);}QPushButton:hover{background-color:transparent;border-radius:20px;image: url(./res/left_true_clicked_btn.png);}QPushButton:pressed{background-color:transparent;border-radius:20px;image: url(./res/left_true_clicked_btn.png);}")
            else:
                self.left_btn.setStyleSheet("QPushButton{background-color:transparent;border-radius:20px;image: url(./res/left_false_btn.png);}")
            if(self.right_btn.isEnabled()):
                self.right_btn.setStyleSheet("QPushButton{background-color:transparent;border-radius:20px;image: url(./res/right_true_btn.png);}QPushButton:hover{background-color:transparent;border-radius:20px;image: url(./res/right_true_clicked_btn.png);}QPushButton:pressed{background-color:transparent;border-radius:20px;image: url(./res/right_true_clicked_btn.png);}")
            else:
                self.right_btn.setStyleSheet("QPushButton{background-color:transparent;border-radius:20px;image: url(./res/right_false_btn.png);}")
        else:
            if (self.left_btn.isEnabled()):
                self.left_btn.setStyleSheet("QPushButton{background-color:transparent;border-radius:20px;image: url(./res/left_true_btn_dark.png);}QPushButton:hover{background-color:transparent;border-radius:20px;image: url(./res/left_true_click_dark.png);}QPushButton:pressed{background-color:transparent;border-radius:20px;image: url(./res/left_true_click_dark.png);}")
            else:
                self.left_btn.setStyleSheet("QPushButton{background-color:transparent;border-radius:20px;image: url(./res/left_false_dark.png);}")
            if (self.right_btn.isEnabled()):
                self.right_btn.setStyleSheet("QPushButton{background-color:transparent;border-radius:20px;image: url(./res/right_true_btn_dark.png);}QPushButton:hover{background-color:transparent;border-radius:20px;image: url(./res/right_true_click_dark.png);}QPushButton:pressed{background-color:transparent;border-radius:20px;image: url(./res/right_true_click_dark.png);}")
            else:
                self.right_btn.setStyleSheet("QPushButton{background-color:transparent;border-radius:20px;image: url(./res/right_false_dark.png);}")


    def get_review(self):
        # 没有更多评论啦~
        if self.show_review_count >= self.total_review_count:
            return
        # 更多评论不够10条
        elif self.show_review_count + 10 >= self.total_review_count:
            num = self.total_review_count - self.show_review_count
            for k in range(num):
                self.review1 = ReviewLabel(self.ratings_average,self.review_card_width, self.reviewlist1[self.show_review_count + k],self.app_detail, self.review_widget1)
                self.review1.move(self.show_review_x + self.review1.width() * k + 20 * k, 0)
                self.review1.raise_()
                self.review1.show()

            self.show_review_count += num
            self.show_review_x += self.review1.width() * num + 20 * num
        # 还能加载10条评论
        else:
            num = 10
            for k in range(num):
                self.review1 = ReviewLabel(self.ratings_average,self.review_card_width, self.reviewlist1[self.show_review_count + k], self.app_detail,self.review_widget1)
                self.review1.move(self.show_review_x + self.review1.width() * k + 20 * k, 0)
                self.review1.raise_()
                self.review1.show()

            self.show_review_count += num
            self.show_review_x += self.review1.width() * num + 20 * num

    

    # 函数：点击右边按钮
    def slot_click_right_btn(self):
        # self.right_btn.setEnabled(False)
        self.right_click_count += 1
        if(len(self.reviewlist1) >2):
            if ((self.right_click_count + 1) % 5) == 0:
                self.right_btn.clicked.connect(self.get_review)

            self.left_btn.setEnabled(True)
            self.check_btn_styleSheet()
            x = self.review_widget1.x()
            if (self.final_x_position == 0):
                self.final_x_position =-(self.review_card_width*2+2*20)
                Globals.POS = self.final_x_position
                self.ad_move_left_time.start(1)
            else:
                self.final_x_position = self.review_widget1.x() - (self.review_card_width*2+20*2)
                Globals.POS = self.final_x_position
                self.ad_move_left_time.start(1)

    #
    # #
    # 函数：向左滑动:1.左边滑动。2.滑到终点按钮置灰。3.滑到一般预加载
    def slot_slide_left(self):
        self.ad_move_left_time.stop()
        self.final_x_position = Globals.POS
        self.right_btn.setEnabled(False)
        self.left_btn.setEnabled(False)

        x = self.review_widget1.x()
        if(x-67 > self.final_x_position):
            self.review_widget1.move(x - 67, 0)
        else:
            self.review_widget1.move(self.final_x_position, 0)
        self.ad_move_left_time.start(1)
        x = self.review_widget1.x()
        if (x == self.final_x_position) :
            self.ad_move_left_time.stop()
            self.right_btn.setEnabled(True)
            self.left_btn.setEnabled(True)
            self.check_btn_styleSheet()


            # self.final_x_position = 0
        if self.count % 2 == 0:
            if (x == -(self.review_widget1.width() - (self.review_card_width*2+20))):
                self.right_btn.setEnabled(False)
                self.check_btn_styleSheet()

        else:
            if (x == -(self.review_widget1.width() - self.review_card_width)):
                self.right_btn.setEnabled(False)
                self.check_btn_styleSheet()

        # if x == - self.review_widget1.width()/2 :
        #     self.ma
        #     self.m_parent.mw.at.appmgr.get_application_reviews(self.app_msg.pkgname, page=self.reviewpage)
        #     self.get_review()


    #
    # 函数：点击向左的箭头
    #
    def slot_click_left_btn(self):
        self.right_btn.setEnabled(True)
        self.check_btn_styleSheet()

        x = self.review_widget1.x()

        if (self.final_x_position == 0):
            self.final_x_position = (self.review_card_width*2+20*2)
            Globals.POS = self.final_x_position
            self.ad_move_right_time.start(0.1)
        else:
            self.final_x_position = self.review_widget1.x() + (self.review_card_width*2+20*2)
            Globals.POS = self.final_x_position
            self.ad_move_right_time.start(0.1)

    #
    # 函数向右滑动

    def slot_slide_right(self):
        self.ad_move_right_time.stop()
        self.final_x_position = Globals.POS
        self.left_btn.setEnabled(False)
        self.right_btn.setEnabled(False)

        x = self.review_widget1.x()
        if(x+67 < self.final_x_position):
            self.review_widget1.move(x + 67, 0)
        else:
            self.review_widget1.move(self.final_x_position, 0)
        # self.review_widget1.move(x + 67, 0)
        self.ad_move_right_time.start(1)
        x = self.review_widget1.x()
        if (x ==self.final_x_position):
            self.ad_move_right_time.stop()
            self.left_btn.setEnabled(True)

            self.right_btn.setEnabled(True)
            self.check_btn_styleSheet()

            # self.final_x_position = 0
        if (x == 0):
            self.left_btn.setEnabled(False)
            self.check_btn_styleSheet()


    #
    # 函数：鼠标按下事件
    #
    def mousePressEvent(self, event):
        if event.button() == Qt.LeftButton:
            pass
        else:
            event.ignore()

    def enterEvent(self, event):
        if (len(self.reviewlist1) > 2):
            self.left_btn.show()
            self.right_btn.show()

    def eventFilter(self, obj, event):
        if (event.type() == event.Enter):
            if (obj == self.review_widget1 or obj == self.widget or obj == self.mainwindow):
                if (len(self.reviewlist1) > 2):
                    self.left_btn.show()
                    self.right_btn.show()
        if (event.type() == event.Leave):
            if (obj == self.mainwindow):
                self.left_btn.hide()
                self.right_btn.hide()
        return False


class ReviewLabel(QWidget):

    commend_height=260
    agree_with = False
    disagree = False

    def __init__(self, ratings_average, width,reviewlist, app_detail ,parent=None):
        QWidget.__init__(self,parent)
        # strstr = ""
        # k = 0
        self.m_parent = parent
        self.app_detail = app_detail
        self.ratings_average = ratings_average
        self.comment_line_num = 6
        self.w = width
        self.reviewlist = reviewlist
        self.init_ui()

    def init_ui(self):
        self.reviewlist = self.reviewlist
        self.ReviewWidget = QWidget(self)
        self.ReviewWidget.resize(self.w,self.commend_height)
        self.ReviewWidget.move(0,0)
        # self.ReviewWidget.setStyleSheet("QWidget{border:none;border-radius:12px;background-color:#F4F4F4}")
        Globals.ADD_WIDGET.append(self.ReviewWidget)
        self.userHead = QLabel(self.ReviewWidget)
        self.userHead.setGeometry(QtCore.QRect(16, 16, 40, 40))
        self.userHead.setAttribute(Qt.WA_TranslucentBackground)
        self.userName = QLabel(self.ReviewWidget)
        self.userName.setGeometry(QtCore.QRect(66, 16, 166, 22))
        self.userName.setAttribute(Qt.WA_TranslucentBackground)
        self.me_label = QLabel(self.ReviewWidget)
        self.me_label.resize(20,18)
        # self.me_label.setAttribute(Qt.WA_TranslucentBackground)
        self.me_label.move(self.userName.x()+self.userName.width()+8,self.userName.y()+int((self.userName.height()-self.me_label.height())/2))
        if(Globals.LAN == "zh_CN"):
            self.me_label.setStyleSheet("QLabel{background-image:url('res/myself_zh.png')}")
        else:
            self.me_label.setStyleSheet("QLabel{background-image:url('res/myself_en.png')}")
        self.comment = QLabel(self.ReviewWidget)
        self.comment.move(16,66)
        self.comment.setAttribute(Qt.WA_TranslucentBackground)
        # self.comment.setTextInteractionFlags(Qt.TextSelectableByMouse)
        self.comment.setContextMenuPolicy(Qt.NoContextMenu)
        # setTextInteractionFlags(Qt::TextSelectableByMouse)
        # self.comment.setFixedSize(484,126)
        # self.comment.show()
        self.createDate = QLabel(self.ReviewWidget)
        self.createDate.setAttribute(Qt.WA_TranslucentBackground)
        # self.createDate.setGeometry(QtCore.QRect(345, 16, 155, 20))
        # self.createDate.move(516-16-self.createDate.width(),16)
        # self.createDate.setFixedHeight(20)

        self.agree_with_btn = QPushButton(self.ReviewWidget)
        self.agree_with_btn.setFocusPolicy(Qt.NoFocus)
        # self.agree_with_btn.setAttribute(Qt.WA_TranslucentBackground)
        self.agree_with_btn.resize(20, 20)
        self.agree_with_btn.setStyleSheet(
            "QPushButton{border: 0px;font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;background-image:url('res/default_agree.png');}QPushButton:hover{background-image: url(./res/hover_agree.png);}QPushButton:pressed{background-image: url(./res/press_agree.png);}")
        self.agree_with_btn.move(16, self.ReviewWidget.height() - self.agree_with_btn.height() - 16)
        self.agree_with_btn.clicked.connect(lambda :self.app_detail.slot_click_agree_with_btn(self.reviewlist,self))

        self.agree_with_count_label = QLabel(self.ReviewWidget)
        self.agree_with_count_label.setAttribute(Qt.WA_TranslucentBackground)
        self.agree_with_count_label.resize(20, 20)
        self.agree_with_count_label.setStyleSheet(
            "QLabel{font-size: 14px;font-weight: 400;color: #8F9399;line-height: 20px;}")
        self.agree_with_count_label.move(self.agree_with_btn.x() + self.agree_with_btn.width() + 4,
                                         self.agree_with_btn.y() + int(
                                             (self.agree_with_btn.height() - self.agree_with_count_label.height()) / 2))

        self.disagree_btn = QPushButton(self.ReviewWidget)
        self.disagree_btn.setFocusPolicy(Qt.NoFocus)
        # self.agree_with_btn.setAttribute(Qt.WA_TranslucentBackground)
        self.disagree_btn.resize(20, 20)
        self.disagree_btn.setStyleSheet(
            "QPushButton{border: 0px;font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;background-image:url('res/default_disagree.png');}QPushButton:hover{background-image: url(./res/hover_disagree.png);}QPushButton:pressed{background-image: url(./res/press_disagree.png);}")
        self.disagree_btn.move(79, self.ReviewWidget.height() - self.agree_with_btn.height() - 16)
        self.disagree_btn.clicked.connect(lambda :self.app_detail.slot_click_disagree_btn(self.reviewlist,self))

        self.disagree_count_label = QLabel(self.ReviewWidget)
        self.disagree_count_label.setAttribute(Qt.WA_TranslucentBackground)
        self.disagree_count_label.resize(20, 20)
        self.disagree_count_label.setStyleSheet(
            "QLabel{font-size: 14px;font-weight: 400;color: #8F9399;line-height: 20px;}")
        self.disagree_count_label.move(self.disagree_btn.x() + self.disagree_btn.width() + 4,
                                       self.disagree_btn.y() + int(
                                           (self.disagree_btn.height() - self.disagree_count_label.height()) / 2))

        self.delete_btn = QPushButton(self.ReviewWidget)
        self.delete_btn.setFocusPolicy(Qt.NoFocus)
        self.delete_btn.setAttribute(Qt.WA_TranslucentBackground)
        self.delete_btn.setStyleSheet("QPushButton{border: 0px;font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;}")
        self.delete_btn.setText(_("Delete"))
        self.delete_btn.adjustSize()
        self.delete_btn.move(self.ReviewWidget.width()-16-self.delete_btn.width(),self.ReviewWidget.height()-self.delete_btn.height()-16)
        self.delete_btn.clicked.connect(lambda :self.app_detail.slot_delete_comment(self.reviewlist,self.app_detail.app_msg.pkgname))


        self.star = StarWidget('small', self.ratings_average, self.ReviewWidget)
        self.star.move(67, 43)
        self.star.setFixedSize(98,14)
        self.star.setStyleSheet("QWidget{background-color:transparent;}")
        self.setFixedSize(self.w,self.commend_height)
        self.star.hide()

        # self.comment.setAlignment(Qt.AlignVCenter)
        # self.comment.setWordWrap(True)
        self.comment.setAlignment(Qt.AlignLeft | Qt.AlignTop)
        self.comment.setWordWrap(True)
        # self.comment.setAlignment(Qt.AlignTop)
        self.comment.setTextInteractionFlags(Qt.TextSelectableByMouse)
        self.userName.setStyleSheet("QLabel{font-size: 16px;font-weight: 500;color: #303133;line-height: 22px;}")
        Globals.ADD_WIDGET_USERNAME.append(self.userName)
        self.comment.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #606265;line-height: 21px;}")
        Globals.ADD_WIDGET_COMMENT.append(self.comment)
        if (self.reviewlist.user_profile != ''):
            if (self.reviewlist.user_display == Globals.LOGIN_ACCOUNT):
                user_profile = UBUNTUKYLIN_DATA_PATH + "user_profile/" + Globals.USER_PROFILE
            else:
                user_profile = UBUNTUKYLIN_DATA_PATH + "user_profile/" + self.reviewlist.user_profile
            pix2 = QPixmap(user_profile)
            fitpixmap_userIcon = pix2.scaled(40, 40, Qt.IgnoreAspectRatio, Qt.SmoothTransformation)
            fitpixmap_userIcon = self.PixmapToRound(fitpixmap_userIcon, 40,
                                                    40)
            self.userHead.setPixmap(fitpixmap_userIcon)
        else:
            self.userHead.setStyleSheet("QLabel{background-image:url('res/userhead.png')}")
        self.createDate.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #8F9399;line-height: 20px;}")
        # Globals.ADD_WIDGET_DATE.append(self.createDate)


        self.userName.setText(self.reviewlist.user_display)
        self.userName.adjustSize()
        self.me_label.move(self.userName.x() + self.userName.width() + 8,
                           self.userName.y() + int((self.userName.height() - self.me_label.height()) / 2))

        if (str(self.reviewlist.account) == str(Globals.ACCOUNT)):
            self.me_label.show()
            self.delete_btn.show()
        else:
            self.me_label.hide()
            self.delete_btn.hide()
        self.createDate.setText(self.reviewlist.date)
        # print("评论的长度",len(self.createDate.text()))
        self.createDate.move(self.w-16-len(self.createDate.text()*7),16)
        self.createDate.setFixedHeight(20)

        if (self.reviewlist.like_count != None):
            self.comment.setGeometry(QtCore.QRect(16, 66, int(self.ReviewWidget.width()-16*2), int(self.delete_btn.y()-self.comment.y())))
            self.agree_with_count_label.setText(str(self.reviewlist.like_count))
            self.agree_with_count_label.adjustSize()
            self.disagree_count_label.setText(str(self.reviewlist.unlike_count))
            self.disagree_count_label.adjustSize()
            self.comment_line_num = 6
        else:
            self.comment.setGeometry(QtCore.QRect(16, 66, self.ReviewWidget.width() - 16 * 2, 150))
            self.agree_with_count_label.hide()
            self.agree_with_btn.hide()
            self.disagree_count_label.hide()
            self.disagree_btn.hide()
            self.comment_line_num = 7
        if (self.reviewlist.property != None and str(self.reviewlist.property) == "1"):
            self.agree_with = True
            self.click_agree_over()
        elif (self.reviewlist.property != None and str(self.reviewlist.property) == "2"):
            self.disagree = True
            self.click_disagree_over()

        self.comment.setText(self.reviewlist.content)
        self.setLongTextToElideFormat(self.comment, self.reviewlist.content,self.comment_line_num)
        self.show_dark_mode()

    def check_account(self):
        if (str(self.reviewlist.account) == str(Globals.ACCOUNT)):
            self.me_label.show()
            self.delete_btn.show()
        else:
            self.me_label.hide()
            self.delete_btn.hide()

    def click_disagree_over(self):
        self.agree_with_btn.setStyleSheet(
            "QPushButton{border: 0px;font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;background-image:url('res/default_agree.png');}QPushButton:hover{background-image: url(./res/hover_agree.png);}QPushButton:pressed{background-image: url(./res/press_agree.png);}")
        self.disagree_btn.setStyleSheet(
            "QPushButton{border: 0px;font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;background-image: url(./res/press_disagree.png);}")

    def click_agree_over(self):
        self.agree_with_btn.setStyleSheet(
            "QPushButton{border: 0px;font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;background-image: url(./res/press_agree.png);}")
        self.disagree_btn.setStyleSheet(
            "QPushButton{border: 0px;font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;background-image:url('res/default_disagree.png');}QPushButton:hover{background-image: url(./res/hover_disagree.png);}QPushButton:pressed{background-image: url(./res/press_disagree.png);}")

    def init_btn_statue(self):
        self.agree_with_btn.setStyleSheet(
            "QPushButton{border: 0px;font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;background-image:url('res/default_agree.png');}QPushButton:hover{background-image: url(./res/hover_agree.png);}QPushButton:pressed{background-image: url(./res/press_agree.png);}")
        self.disagree_btn.setStyleSheet(
            "QPushButton{border: 0px;font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;background-image:url('res/default_disagree.png');}QPushButton:hover{background-image: url(./res/hover_disagree.png);}QPushButton:pressed{background-image: url(./res/press_disagree.png);}")


    def re_move(self):
        self.comment.setGeometry(QtCore.QRect(16, 66, self.ReviewWidget.width() - 16 * 2, self.comment.height()))
        print("small",self.comment.height())
        self.createDate.move(self.width() - 16 - len(self.createDate.text() * 7), 16)
        self.createDate.setFixedHeight(20)
        self.setLongTextToElideFormat(self.comment, self.reviewlist.content,self.comment_line_num)
        self.delete_btn.move(self.ReviewWidget.width() - 16 - self.delete_btn.width(),
                             self.ReviewWidget.height() - self.delete_btn.height() - 16)

    def setLongTextToElideFormat(self, label, te,line_num=7):

        te = te.replace("\n", ",").replace("\r", "").replace("\t", "").replace(";","；").replace(",","，")
        metrics = QFontMetrics(label.font())
        elidedText = metrics.elidedText(te, Qt.ElideRight, label.width()*line_num)


        label.setText(elidedText)
        if ("…" in elidedText):

            count = 0
            set_text = ""
            for i in te:
                if(count < 35):
                    set_text = set_text + i
                elif(count == 35):
                    set_text = set_text + i + "\n"
                    count = 0
                count = count +1
            QToolTip.setFont(QFont("SansSerif", 10))
            self.setToolTip(set_text)
        else:
            self.splitText(label, te, label.width())


    #label:控件，text：文字，nlabelsize：控件宽
    def splitText(self,label,text,nlabelsize):
        font = QFont()
        font.setPixelSize(14)
        m_font = QFontMetrics(font)
        nTextSize = m_font.width(text)
        text = label.text()
        label.setText("")
        split_num = 1
        if (label.width() % 2 != 0):
            width = label.width() - 1
        else:
            width = label.width()
        for i in text:
            pre_text = label.text()
            label.setText(label.text() + i)
            s = m_font.boundingRect(label.text()).size().width()
            if (s <= width * split_num+split_num-1):
                pass
            else:
                label.setText(pre_text + "\n" + i)
                split_num = split_num + 1



    def show_dark_mode(self):
        if (Globals.THEME_COLOR == 0):
            self.setStyleSheet(".QWidget{border:none;border-radius:12px;background-color:#F4F4F4}")
            self.ReviewWidget.setStyleSheet(".QWidget{border:none;border-radius:12px;background-color:#F4F4F4}")
            self.userName.setStyleSheet("QLabel{font-size: 16px;font-weight: 500;color: #303133;line-height: 22px;}")
            self.comment.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #606265;line-height: 21px;}")
        else:
            self.setStyleSheet(".QWidget{border:none;border-radius:12px;background-color:#393939}")
            self.ReviewWidget.setStyleSheet(".QWidget{border:none;border-radius:12px;background-color:#393939}")
            self.userName.setStyleSheet(
                "QLabel{font-size: 16px;font-weight: 500;color: #F9F9F9;line-height: 22px;}")
            self.comment.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 21px;}")

    # 将图片切割成圆角
    def PixmapToRound(self, src, x, y):
        pixmapa = QPixmap(src)
        pixmap = QPixmap(x, y)
        pixmap.fill(Qt.transparent)
        painter = QPainter(pixmap)
        painter.setRenderHint(QPainter.Antialiasing, True)
        # painter.setRenderHints(QPainter.Antialiasing | QPainter.SmoothPixmapTransform)
        path = QPainterPath()
        rect = QRectF(0, 0, x, y)
        path.addRoundedRect(rect, 20.0, 20.0)
        painter.setClipPath(path)
        painter.drawPixmap(0, 0, x, y, pixmapa)

        return pixmap