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
from models.enums import setLongTextToElideFormat
import datetime
# from models.enums import CACHE_DIR,BUTTON_PRESSED,BUTTON_ENTER,BUTTON_LEAVE,BUTTON_DISABLE,AppMsg
#


class All_Review_Widget(QWidget,Signals):
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
    i = 1
    reviewlist1=[]
    alist = []
    commend_height = 260

    set_show_review_height = pyqtSignal(int)
    def __init__(self,reviewlist1,app_detail,parent=None):
        QWidget.__init__(self,parent)
        self.dblist = []
        self.m = 0
        # self.app = app
        # self.reviewpage = reviewpage
        self.ratings_average = 0
        # self.messageBox = MessageBox(self)
        self.mainwindow = parent
        self.app_detail = app_detail
        self.mouse_press = False
        # self.setFixedSize(1340,278)

        self.setWindowFlags(Qt.FramelessWindowHint)
        self.setAttribute(Qt.WA_TranslucentBackground)
        #

        self.widget = QWidget(self.mainwindow)
        self.widget.resize(self.app_detail.editorial_comments_widget.width(), self.commend_height)
        self.widget.move(0, 0)
        self.widget.setStyleSheet("QWidget{border-radius:12px;background-color:transparent;}")
        self.review_card_width = int((self.widget.width() - 20) / 2)

        # self.review_widget1 = QWidget(self.widget)
        # self.review_widget1.setFixedWidth(self.widget.width())
        # self.review_widget1.setStyleSheet("QWidget{border:1px ;border-radius:12px;background:transparent;}")
        # self.review_widget1.move(0, 0)


        self.total_review_count = len(reviewlist1)
        self.show_review_count = 0
        self.right_click_count = 0

        self.show_review_x = 0
        self.show_review_y = 0

        self.reviewlist1 = reviewlist1

        self.innit_ui(self.reviewlist1)

    def innit_ui(self,reviewlist1):

        # ratings_average=3
        reviewlist = reviewlist1
        if len(reviewlist) <= 10:
            if len(reviewlist) % 2 == 0:
                self.widget.resize(self.widget.width(), self.commend_height * (len(reviewlist) / 2) + 20 * (len(reviewlist) / 2 ))
                # self.review_widget1.resize(self.review_widget1.width(),self.commend_height * (len(reviewlist)/2) + 20 * (len(reviewlist)/2 ))

            else:
                self.widget.resize(self.widget.width(),self.commend_height * ((len(reviewlist)+1)/2) + 20 * ((len(reviewlist)+1)/2 ))
                # self.review_widget1.resize(self.review_widget1.width(),self.commend_height * ((len(reviewlist)+1)/2) + 20 * ((len(reviewlist)+1)/2 ))
        else:
            self.widget.resize(self.widget.width(), self.commend_height * 5 + 20 * (5 ))
            # self.review_widget1.resize(self.review_widget1.width(),self.commend_height * 5 + 20 * (5 ))


        for k in range(self.total_review_count):
            if k % 2 == 0:
                ratings_average = self.reviewlist1[k].user_rating
                self.review1 = ReviewLabel(ratings_average, self.review_card_width, reviewlist[k],self.app_detail, self.widget)
                self.review1.move(0,self.review1.height() * k/2 + 20 * k/2)
                # self.review1.move(0, 100)
                self.review1.raise_()
                self.review1.show()
            else:
                ratings_average = self.reviewlist1[k].user_rating
                self.review1 = ReviewLabel(ratings_average,self.review_card_width, reviewlist[k], self.app_detail,self.widget)
                self.review1.move(self.review_card_width+20, self.review1.height() * (k-1)/2 + 20 * (k-1)/2)
                self.review1.raise_()
                self.review1.show()


        self.show_review_count = 10
        self.show_review_y = self.review1.height() * 5 + 20 * 5
        self.count = len(reviewlist)

    def re_move(self):
        self.widget.resize(self.app_detail.editorial_comments_widget.width(), self.widget.height())
        self.widget.move(0, 0)
        self.review_card_width = int((self.widget.width() - 20) / 2)
        reviews = self.widget.children()
        # self.widget.setFixedSize(self.widget.width(), self.widget.height())
        # self.review_widget1.move(0,0)
        n = 0
        for i in reviews:
            i.setFixedSize(self.review_card_width, self.commend_height)
            i.ReviewWidget.resize(self.review_card_width, self.commend_height)
            i.re_move()
            if(n % 2 == 1):
                i.move(self.review_card_width  + 20, (int(n/2))*20+int(n/2)*i.height())
            else:
                i.move(0, (int(n/2))*20+int(n/2)*i.height())
            n = n + 1


    #函数：加载更多评论---在app_detail_widget调用，使滑动条到底时自动加载更多评论
    def get_review(self,reviewlist):
        self.m += 1
        self.reviewlist1 = reviewlist
        self.total_review_count = len(self.reviewlist1)
        # 没有更多评论啦~
        if self.show_review_count >= self.total_review_count:
            return
        # 更多评论不够10条
        elif self.show_review_count + 10 > self.total_review_count:
            num = self.total_review_count - self.show_review_count
            # 扩大widget的大小
            if num % 2 == 0:
                self.set_show_review_height.emit(num / 2 * self.commend_height + (num / 2) * 20)
                self.mainwindow.resize(self.mainwindow.width(),self.mainwindow.height() + num / 2 * self.commend_height + (num / 2) * 20)
                self.widget.resize(self.widget.width(), self.commend_height * ((self.show_review_count + num) / 2) + 20 * ((self.show_review_count + num) / 2))
                # self.review_widget1.resize(self.review_widget1.width(), self.commend_height * ((self.show_review_count + num) / 2) + 20 * (
                #             (self.show_review_count + num) / 2))
            else:
                self.set_show_review_height.emit((num + 1) / 2 * self.commend_height + ((num + 1) / 2) * 20)
                self.mainwindow.resize(self.mainwindow.width(),
                                       self.mainwindow.height() + (num + 1) / 2 * self.commend_height + ((num + 1) / 2) * 20)
                self.widget.resize(self.widget.width(), self.commend_height * (((self.show_review_count + num) + 1) / 2) + 20 * (
                            ((self.show_review_count + num) + 1) / 2))
                # self.review_widget1.resize(self.review_widget1.width(), self.commend_height * (((self.show_review_count + num) + 1) / 2) + 20 * (
                #             ((self.show_review_count + num) + 1) / 2))
            # 排列每一个评论的小widget
            for k in range(num):
                if k % 2 == 0:

                    self.ratings_average = self.reviewlist1[self.show_review_count + k].user_rating
                    self.review1 = ReviewLabel(self.ratings_average, self.review_card_width,self.reviewlist1[self.show_review_count + k],
                                               self.app_detail,self.widget)
                    self.review1.move(0, self.show_review_y + self.review1.height() * k / 2 + 20 * k / 2)
                    self.review1.raise_()
                    self.review1.show()
                else:
                    self.ratings_average = self.reviewlist1[self.show_review_count + k].user_rating
                    self.review1 = ReviewLabel(self.ratings_average, self.review_card_width,self.reviewlist1[self.show_review_count + k],
                                               self.app_detail,self.widget)
                    self.review1.move(self.review_card_width + 20, self.show_review_y + self.review1.height() * (k - 1) / 2 + 20 * (k - 1) / 2)
                    self.review1.raise_()
                    self.review1.show()
            self.show_review_count += num
            if num % 2 == 0:
                self.show_review_y += self.review1.height() * num / 2 + 20 * num / 2
            else:
                self.show_review_y += self.review1.height() * (num + 1) / 2 + 20 * (num + 1) / 2
        # 还能加载10条评论
        else:
            num = 10
            self.mainwindow.resize(self.mainwindow.width(), self.mainwindow.height() + 5 * self.commend_height + 5 * 20)
            self.set_show_review_height.emit(5 * self.commend_height + 5 * 20)
            # 扩大widget的大小
            self.widget.resize(self.widget.width(), self.commend_height * ((self.show_review_count + self.i * 10) / 2) + 20 * (
                        (self.show_review_count + self.i * 10) / 2))
            # self.review_widget1.setFixedSize(self.review_widget1.width(), self.commend_height * ((self.show_review_count + self.i * 10) / 2) + 20 * (
            #             (self.show_review_count + self.i * 10) / 2))
            self.i = self.i + 1
            for k in range(num):
                if k % 2 == 0:
                    self.ratings_average = self.reviewlist1[self.show_review_count + k].user_rating
                    self.review1 = ReviewLabel(self.ratings_average, self.review_card_width,self.reviewlist1[self.show_review_count + k],
                                               self.app_detail,self.widget)
                    self.review1.move(0, self.show_review_y + self.review1.height() * k / 2 + 20 * k / 2)
                    self.review1.raise_()
                    self.review1.show()
                else:
                    self.ratings_average = self.reviewlist1[self.show_review_count + k].user_rating
                    self.review1 = ReviewLabel(self.ratings_average, self.review_card_width,self.reviewlist1[self.show_review_count + k],
                                               self.app_detail,self.widget)
                    self.review1.move(self.review_card_width + 20, self.show_review_y + self.review1.height() * (k - 1) / 2 + 20 * (k - 1) / 2)
                    self.review1.raise_()
                    self.review1.show()

            self.show_review_count += num
            if num % 2 == 0:
                self.show_review_y += self.review1.height() * num / 2 + 20 * num / 2
            else:
                self.show_review_y += self.review1.height() * (num + 1) / 2 + 20 * (num + 1) / 2

        Globals.IS_GETTING_REVIEW = False


    #
    # 函数：实现无标题栏窗口拖动
    #
    def mousePressEvent(self, event):
        if (event.button() == Qt.LeftButton):
            self.mouse_press = True
            self.drag_pos = event.globalPos() - self.pos()
            event.accept()

    def mouseReleaseEvent(self, event):
        self.mouse_press = False
        self.setWindowOpacity(1)

    def mouseMoveEvent(self, event):
        if (self.mouse_press):
            self.move_pos = event.globalPos()
            self.move(self.move_pos - self.drag_pos)
            self.setWindowOpacity(1)
            event.accept()

class ReviewLabel(QWidget):

    commend_height = 260
    agree_with = False
    disagree = False

    def __init__(self, ratings_average,width, reviewlist,app_detail, parent=None):
        QWidget.__init__(self,parent)
        # strstr = ""
        # k = 0
        self.m_parent = parent
        self.app_detail = app_detail
        self.reviewlist = reviewlist
        self.comment_line_num = 6
        self.ReviewWidget = QWidget(self)
        self.ReviewWidget.resize(width,self.commend_height)
        self.ReviewWidget.move(0,0)
        # self.ReviewWidget.setStyleSheet("QWidget{border:none;border-radius:12px;background-color:#F4F4F4}")
        Globals.ADD_WIDGET.append(self.ReviewWidget)
        self.userHead = QLabel(self.ReviewWidget)
        self.userHead.setGeometry(QtCore.QRect(16, 16, 40, 40))
        self.userHead.setAttribute(Qt.WA_TranslucentBackground)
        self.userName = QLabel(self.ReviewWidget)
        self.userName.setGeometry(QtCore.QRect(66, 16, 166, 22))
        self.userName.setAttribute(Qt.WA_TranslucentBackground)
        # self.userName.setAttribute(Qt.WA_TranslucentBackground)
        self.me_label = QLabel(self.ReviewWidget)
        self.me_label.resize(20, 18)
        # self.me_label.setAttribute(Qt.WA_TranslucentBackground)
        self.me_label.move(self.userName.x() + self.userName.width() + 8,
                           int((self.userName.height() - self.me_label.height()) / 2))
        if (Globals.LAN == "zh_CN"):
            self.me_label.setStyleSheet("QLabel{background-image:url('res/myself_zh.png')}")
        else:
            self.me_label.setStyleSheet("QLabel{background-image:url('res/myself_en.png')}")
        self.comment = QLabel(self.ReviewWidget)
        self.comment.setAttribute(Qt.WA_TranslucentBackground)
        self.comment.setContextMenuPolicy(Qt.NoContextMenu)
        self.createDate = QLabel(self.ReviewWidget)
        self.createDate.setAttribute(Qt.WA_TranslucentBackground)
        self.createDate.move(self.width() - 16 - len(self.createDate.text() * 7), 16)
        #self.createDate.setGeometry(QtCore.QRect(345, 16, 155, 20))

        self.star = StarWidget('small', ratings_average, self.ReviewWidget)
        self.star.move(67, 43)
        self.star.setFixedSize(98,14)
        self.star.setStyleSheet("QWidget{background-color:transparent;}")
        self.setFixedSize(width,self.commend_height)
        self.star.hide()

        self.agree_with_btn = QPushButton(self.ReviewWidget)
        self.agree_with_btn.setFocusPolicy(Qt.NoFocus)
        # self.agree_with_btn.setAttribute(Qt.WA_TranslucentBackground)
        self.agree_with_btn.resize(20,20)
        self.agree_with_btn.setStyleSheet("QPushButton{border: 0px;font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;background-image:url('res/default_agree.png');}QPushButton:hover{background-image: url(./res/hover_agree.png);}QPushButton:pressed{background-image: url(./res/press_agree.png);}")
        self.agree_with_btn.move(16,self.ReviewWidget.height()-self.agree_with_btn.height()-16)
        self.agree_with_btn.clicked.connect(lambda: self.app_detail.slot_click_agree_with_btn(self.reviewlist,self))

        self.agree_with_count_label = QLabel(self.ReviewWidget)
        self.agree_with_count_label.setAttribute(Qt.WA_TranslucentBackground)
        self.agree_with_count_label.resize(20, 20)
        self.agree_with_count_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #8F9399;line-height: 20px;}")
        self.agree_with_count_label.move(self.agree_with_btn.x()+self.agree_with_btn.width()+4,self.agree_with_btn.y()+int((self.agree_with_btn.height()-self.agree_with_count_label.height())/2))

        self.disagree_btn = QPushButton(self.ReviewWidget)
        self.disagree_btn.setFocusPolicy(Qt.NoFocus)
        # self.agree_with_btn.setAttribute(Qt.WA_TranslucentBackground)
        self.disagree_btn.resize(20, 20)
        self.disagree_btn.setStyleSheet(
            "QPushButton{border: 0px;font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;background-image:url('res/default_disagree.png');}QPushButton:hover{background-image: url(./res/hover_disagree.png);}QPushButton:pressed{background-image: url(./res/press_disagree.png);}")
        self.disagree_btn.move(79, self.ReviewWidget.height() - self.agree_with_btn.height() - 16)
        self.disagree_btn.clicked.connect(lambda: self.app_detail.slot_click_disagree_btn(self.reviewlist,self))

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
        self.delete_btn.move(self.ReviewWidget.width() - 16 - self.delete_btn.width(),
                             self.ReviewWidget.height() - self.delete_btn.height() - 16)
        self.delete_btn.clicked.connect(lambda: self.app_detail.slot_delete_comment(self.reviewlist,self.app_detail.app_msg.pkgname))


        self.comment.setAlignment(Qt.AlignVCenter)
        self.comment.setWordWrap(True)
        self.comment.setAlignment(Qt.AlignTop)
        self.comment.setTextInteractionFlags(Qt.TextSelectableByMouse)
        self.userName.setStyleSheet("QLabel{font-size: 16px;font-weight: 500;color: #303133;line-height: 22px;}")
        Globals.ADD_WIDGET_USERNAME.append(self.userName)
        self.comment.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #606265;line-height: 21px;}")
        Globals.ADD_WIDGET_COMMENT.append(self.comment)
        if(reviewlist.user_profile != ''):
            if (reviewlist.user_display == Globals.LOGIN_ACCOUNT and Globals.LOGIN_SUCCESS):
                user_profile = UBUNTUKYLIN_DATA_PATH + "user_profile/" + Globals.USER_PROFILE
            else:
                user_profile = UBUNTUKYLIN_DATA_PATH + "user_profile/" + reviewlist.user_profile
            pix2 = QPixmap(user_profile)
            fitpixmap_userIcon = pix2.scaled(40, 40, Qt.IgnoreAspectRatio, Qt.SmoothTransformation)
            fitpixmap_userIcon = self.PixmapToRound(fitpixmap_userIcon, 40,40)
            self.userHead.setPixmap(fitpixmap_userIcon)
        else:
            self.userHead.setStyleSheet("QLabel{background-image:url('res/userhead.png')}")
        if(reviewlist.like_count != None):
            self.comment.setGeometry(QtCore.QRect(16, 66, int(self.ReviewWidget.width()-16*2), int(self.delete_btn.y()-self.comment.y()-10)))
            self.agree_with_count_label.setText(str(reviewlist.like_count))
            self.agree_with_count_label.adjustSize()
            self.disagree_count_label.setText(str(reviewlist.unlike_count))
            self.disagree_count_label.adjustSize()
            self.comment_line_num = 6
        else:
            self.comment.setGeometry(QtCore.QRect(16, 66, self.ReviewWidget.width() - 16*2, 150))
            self.agree_with_count_label.hide()
            self.agree_with_btn.hide()
            self.disagree_count_label.hide()
            self.disagree_btn.hide()
            self.comment_line_num = 7


        # SHA_TZ = datetime.timezone(
        #     datetime.timedelta(hours=8),
        #     name='Asia/Shanghai',
        # )
        # # 协调世界时
        # utc_now = datetime.datetime.utcnow().replace(tzinfo=datetime.timezone.utc)
        # beijing_now = utc_now.astimezone(SHA_TZ).strftime("%Y-%m-%d %H:%M:%S")
        # format_pattern = "%Y-%m-%d %H:%M:%S"
        # difference = (datetime.datetime.strptime(str(beijing_now), format_pattern) - datetime.datetime.strptime(
        #     reviewlist.date, format_pattern))
        # print(difference)

        self.createDate.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #8F9399;line-height: 20px;}")
        # Globals.ADD_WIDGET_DATE.append(self.createDate)

        self.userName.setText(reviewlist.user_display)
        self.userName.adjustSize()
        self.me_label.move(self.userName.x() + self.userName.width() + 8,
                           self.userName.y()+int((self.userName.height() - self.me_label.height()) / 2))
        # if(str(reviewlist.user_id) == str(Globals.USER_ID)):
        if(str(reviewlist.account) == str(Globals.ACCOUNT)):
            self.me_label.show()
            self.delete_btn.show()
        else:
            self.me_label.hide()
            self.delete_btn.hide()
        self.createDate.setText(reviewlist.date)
        self.comment.setText(reviewlist.content)
        self.setLongTextToElideFormat(self.comment, reviewlist.content,self.comment_line_num)
        self.createDate.move(width-16-len(self.createDate.text()*7),16)
        self.createDate.setFixedHeight(20)
        if(reviewlist.property != None and str(reviewlist.property) == "1" ):
            self.agree_with = True
            self.click_agree_over()
        elif(reviewlist.property != None and str(reviewlist.property) == "2" ):
            self.disagree = True
            self.click_disagree_over()

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
        self.comment.setGeometry(QtCore.QRect(16, 66, self.ReviewWidget.width() - 16*2, self.comment.height()))
        print("big", self.comment.height())
        self.createDate.move(self.width() - 16 - len(self.createDate.text() * 7), 16)
        self.createDate.setFixedHeight(20)
        self.setLongTextToElideFormat(self.comment, self.reviewlist.content,self.comment_line_num)
        self.delete_btn.move(self.ReviewWidget.width() - 16 - self.delete_btn.width(),
                             self.ReviewWidget.height() - self.delete_btn.height() - 16)

    def show_dark_mode(self):
        if (Globals.THEME_COLOR == 0):
            self.setStyleSheet(".QWidget{border:none;border-radius:12px;background-color:#F4F4F4}")
            self.ReviewWidget.setStyleSheet(".QWidget{border:none;border-radius:12px;background-color:#F4F4F4}")
            self.userName.setStyleSheet("QLabel{font-size: 16px;font-weight: 500;color: #303133;line-height: 22px;}")
            self.comment.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #606265;line-height: 21px;}")
        else:
            self.setStyleSheet(".QWidget{border:none;border-radius:12px;background-color:#393939}")
            self.ReviewWidget.setStyleSheet(".QWidget{border:none;border-radius:12px;background-color:#393939}")
            self.userName.setStyleSheet("QLabel{font-size: 16px;font-weight: 500;color: #F9F9F9;line-height: 22px;}")
            self.comment.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 21px;}")


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


    #将图片切割成圆角
    def PixmapToRound(self,src,x,y):
        pixmapa = QPixmap(src)
        pixmap = QPixmap(x, y)
        pixmap.fill(Qt.transparent)
        painter = QPainter(pixmap)
        painter.setRenderHint(QPainter.Antialiasing, True)
        # painter.setRenderHints(QPainter.Antialiasing | QPainter.SmoothPixmapTransform)
        path = QPainterPath()
        rect = QRectF(0,0,x,y)
        path.addRoundedRect(rect, 20.0, 20.0)
        painter.setClipPath(path)
        painter.drawPixmap(0, 0, x, y, pixmapa)

        return pixmap
