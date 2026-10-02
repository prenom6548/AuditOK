#!/usr/bin/python3
# -*- coding: utf-8 -*-


from PyQt5.QtGui import *
from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
from models.globals import *
from models.apkinfo import *
from models.application import *
import gettext
gettext.bindtextdomain("kylin-software-center", "/usr/share/locale")
gettext.textdomain("kylin-software-center")
_ = gettext.gettext

class show_ratings_widget(QWidget):
    change_background_image = pyqtSignal()
    submit_my_rating_apk = pyqtSignal(int,ApkInfo)
    submit_my_rating_app = pyqtSignal(int,Application)
    def __init__(self,app_detail,parent):
        QWidget.__init__(self, parent)
        self.m_parent = parent
        self.app_detail = app_detail
        self.MY_SCORE = 0
        self.app_name = ""
        self.pre_app_name = ""
        self.star_list = []
        self.is_click = False
        self.init_ui()
        self.submit_my_rating_apk.connect(self.app_detail.m_parent.mw.slot_submit_my_rating)
        self.submit_my_rating_app.connect(self.app_detail.m_parent.mw.slot_submit_my_rating)
        self.timer = QTimer(self)
        self.timer.timeout.connect(self.slot_timer_timeout)

    def init_ui(self):
        self.count_of_participation_rating = QLabel(self)
        self.count_of_participation_rating.resize(96, 20)
        self.count_of_participation_rating.move(0,0)
        self.count_of_participation_rating.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #606265;line-height: 20px;}")
        self.label1 = start_label(self)
        self.label1.move(0, self.count_of_participation_rating.y()+self.count_of_participation_rating.height()+15)
        self.label2 = start_label(self)
        self.label2.move(7+self.label1.x()+self.label1.width(), self.label1.y())
        self.label3 = start_label(self)
        self.label3.move(7+self.label2.x()+self.label2.width(), self.label1.y())
        self.label4 = start_label(self)
        self.label4.move(7+self.label3.x()+self.label3.width(), self.label1.y())
        self.label5 = start_label(self)
        self.label5.move(7+self.label4.x()+self.label4.width(), self.label1.y())
        self.process1 = QProgressBar(self)
        self.process1.setMinimum(0)
        self.process1.setMaximum(100)
        self.process1.setTextVisible(False)
        self.process1.resize(240,8)
        self.process1.move(self.label5.x()+self.label5.width()+22,self.label1.y() + int((self.label1.height()-self.process1.height())/2))
        self.label6 = start_label(self)
        self.label6.move(self.label2.x(), self.label2.height()+self.label2.y()+5)
        self.label7 = start_label(self)
        self.label7.move(self.label3.x(), self.label2.height()+self.label2.y()+5)
        self.label8 = start_label(self)
        self.label8.move(self.label4.x(), self.label2.height()+self.label2.y()+5)
        self.label9 = start_label(self)
        self.label9.move(self.label5.x(), self.label2.height()+self.label2.y()+5)
        self.process2 = QProgressBar(self)
        self.process2.setMinimum(0)
        self.process2.setMaximum(100)
        self.process2.setTextVisible(False)
        self.process2.resize(240,8)
        self.process2.move(self.label9.x()+self.label9.width()+22,self.label9.y() + int((self.label9.height()-self.process2.height())/2))
        self.label10 = start_label(self)
        self.label10.move(self.label3.x(), self.label6.height()+self.label6.y()+5)
        self.label11 = start_label(self)
        self.label11.move(self.label4.x(), self.label6.height()+self.label6.y()+5)
        self.label12 = start_label(self)
        self.label12.move(self.label5.x(), self.label6.height()+self.label6.y()+5)
        self.process3 = QProgressBar(self)
        self.process3.setMinimum(0)
        self.process3.setMaximum(100)
        self.process3.setTextVisible(False)
        self.process3.resize(240,8)
        self.process3.move(self.label12.x()+self.label12.width()+22,self.label10.y() + int((self.label10.height()-self.process3.height())/2))
        self.label13 = start_label(self)
        self.label13.move(self.label4.x(), self.label10.height()+self.label10.y()+5)
        self.label14 = start_label(self)
        self.label14.move(self.label5.x(), self.label10.height()+self.label10.y()+5)
        self.process4 = QProgressBar(self)
        self.process4.setMinimum(0)
        self.process4.setMaximum(100)
        self.process4.setTextVisible(False)
        self.process4.resize(240,8)
        self.process4.move(self.label14.x()+self.label14.width()+22,self.label14.y() + int((self.label14.height()-self.process4.height())/2))
        self.label15 = start_label(self)
        self.label15.move(self.label5.x(), self.label13.height()+self.label13.y()+5)
        self.process5 = QProgressBar(self)
        self.process5.setMinimum(0)
        self.process5.setMaximum(100)
        self.process5.setTextVisible(False)
        self.process5.resize(240,8)
        self.process5.move(self.label15.x()+self.label15.width()+22,self.label15.y() + int((self.label15.height()-self.process5.height())/2))

        self.my_ratings = QLabel(self)
        self.my_ratings.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #606265;line-height: 20px;}")
        self.my_ratings.setText(_("My ratings"))
        self.my_ratings.adjustSize()
        self.my_ratings.move(self.process1.x()+self.process1.width(),self.count_of_participation_rating.y())

        self.set_progressbar_color()
        self.clear_progress()

        self.star1 = start_btn(1,"big",self)
        self.star1.move(self.my_ratings.x(),self.my_ratings.y()+self.my_ratings.height()+28)
        self.star_list.append(self.star1)
        self.star2 = start_btn(2,"big",self)
        self.star2.move(self.star1.x()+self.star1.width()+9,self.my_ratings.y()+self.my_ratings.height()+28)
        self.star_list.append(self.star2)
        self.star3 = start_btn(3,"big",self)
        self.star3.move(self.star2.x()+self.star2.width()+9,self.my_ratings.y()+self.my_ratings.height()+28)
        self.star_list.append(self.star3)
        self.star4 = start_btn(4,"big",self)
        self.star4.move(self.star3.x()+self.star3.width()+9,self.my_ratings.y()+self.my_ratings.height()+28)
        self.star_list.append(self.star4)
        self.star5 = start_btn(5,"big",self)
        self.star5.move(self.star4.x()+self.star4.width()+9,self.my_ratings.y()+self.my_ratings.height()+28)
        self.star_list.append(self.star5)

        self.edit_btn = QPushButton(self)
        self.edit_btn.setFocusPolicy(Qt.NoFocus)
        self.edit_btn.setAttribute(Qt.WA_TranslucentBackground)
        self.edit_btn.setStyleSheet("QPushButton{background-color:transparent;border:0px;}QPushButton:hover{background-color:transparent;border:0px;}QPushButton:pressed{background-color:transparent;border:0px;}")

        self.edit_pen = QLabel(self.edit_btn)
        self.edit_pen.setFocusPolicy(Qt.NoFocus)
        self.edit_pen.setFixedSize(20,20)
        self.edit_pen.setStyleSheet("QLabel{background-image:url(./res/edit.png);}")
        self.edit_pen.move(0,0)

        self.edit_label = QLabel(self.edit_btn)
        self.edit_label.resize(80,26)
        self.edit_label.setFocusPolicy(Qt.NoFocus)
        self.edit_label.setText(_("Write comments"))
        self.edit_label.setStyleSheet("QLabel{font-size: 18px;font-weight: 500;color: #3790FA;line-height: 26px;}")
        self.edit_label.adjustSize()

        self.edit_btn.resize(self.edit_pen.width()+self.edit_label.width(),self.edit_label.height())
        self.edit_label.move(self.edit_btn.width()-self.edit_label.width(),0)
        self.edit_pen.move(0, int((self.edit_label.height()-self.edit_pen.height())/2))
        self.edit_btn.move(self.star5.x()+self.star5.width()-self.edit_btn.width(),int((self.my_ratings.height()-self.edit_btn.height())/2))

        self.btn_all_review = QPushButton(self)
        self.btn_all_review.setText(_("View reviews"))
        self.btn_all_review.setFocusPolicy(Qt.NoFocus)
        self.btn_all_review.setStyleSheet(
            "QPushButton{background: transparent;border:none;font-size: 18px;font-weight: 400;text-align: left;color: #3790FA;line-height: 12px;}")
        # self.btn_all_review.resize(120,40)
        if Globals.LAN == 'zh_CN':
            self.btn_all_review.setFixedSize(len(self.btn_all_review.text()) * 18, 20)
        else:
            self.btn_all_review.setFixedSize(len(self.btn_all_review.text()) * 9, 20)
        self.btn_all_review.adjustSize()
        self.btn_all_review.move(self.star5.x()+self.star5.width()-self.btn_all_review.width(),self.star1.y()+self.star1.height()+30)


        self.setFixedHeight(self.label15.y()+self.label15.height()+10)

    def resize_progress(self,size):
        if(self.process1.width() - size < 130):
            self.process1.resize(130,self.process1.height())
            self.process2.resize(130,self.process1.height())
            self.process3.resize(130,self.process1.height())
            self.process4.resize(130,self.process1.height())
            self.process5.resize(130,self.process1.height())
            self.my_ratings.move(self.my_ratings.x()-(size - 100),
                                 self.count_of_participation_rating.y())
            self.star1.move(self.my_ratings.x(), self.my_ratings.y() + self.my_ratings.height() + 28)
            self.star2.move(self.star1.x() + self.star1.width() + 9,
                            self.my_ratings.y() + self.my_ratings.height() + 28)
            self.star3.move(self.star2.x() + self.star2.width() + 9,
                            self.my_ratings.y() + self.my_ratings.height() + 28)
            self.star4.move(self.star3.x() + self.star3.width() + 9,
                            self.my_ratings.y() + self.my_ratings.height() + 28)
            self.star5.move(self.star4.x() + self.star4.width() + 9,
                            self.my_ratings.y() + self.my_ratings.height() + 28)
        else:
            self.process1.resize(self.process1.width() - size, self.process1.height())
            self.process2.resize(self.process2.width() - size, self.process1.height())
            self.process3.resize(self.process3.width() - size, self.process1.height())
            self.process4.resize(self.process4.width() - size, self.process1.height())
            self.process5.resize(self.process5.width() - size, self.process1.height())

    def reload_progress(self):
        self.process1.resize(240, self.process1.height())
        self.process2.resize(240, self.process1.height())
        self.process3.resize(240, self.process1.height())
        self.process4.resize(240, self.process1.height())
        self.process5.resize(240, self.process1.height())

    def re_move(self,need_move_space,proportion):
        self.setFixedWidth(self.width()+need_move_space)
        self.my_ratings.move(self.process1.x() + self.process1.width() + need_move_space,
                             self.count_of_participation_rating.y())
        self.star1.move(self.my_ratings.x(), self.my_ratings.y() + self.my_ratings.height() + 28)
        self.star2.move(self.star1.x() + self.star1.width() + 9, self.my_ratings.y() + self.my_ratings.height() + 28)
        self.star3.move(self.star2.x() + self.star2.width() + 9, self.my_ratings.y() + self.my_ratings.height() + 28)
        self.star4.move(self.star3.x() + self.star3.width() + 9, self.my_ratings.y() + self.my_ratings.height() + 28)
        self.star5.move(self.star4.x() + self.star4.width() + 9, self.my_ratings.y() + self.my_ratings.height() + 28)
        self.edit_btn.move(self.star5.x() + self.star5.width() - self.edit_btn.width(), 0)
        self.btn_all_review.move(self.star5.x() + self.star5.width() - self.btn_all_review.width(),
                                 self.star1.y() + self.star1.height() + 30)
        # self.resize()

    def clear_progress(self):
        self.process1.hide()
        self.process1.setValue(0)
        self.process1.show()
        self.process2.hide()
        self.process2.setValue(0)
        self.process2.show()
        self.process3.hide()
        self.process3.setValue(0)
        self.process3.show()
        self.process4.hide()
        self.process4.setValue(0)
        self.process4.show()
        self.process5.hide()
        self.process5.setValue(0)
        self.process5.show()

    def slot_enter_star(self,num):
        n = 0
        if(num == -1):
            if (Globals.LOGIN_SUCCESS):
                num = self.MY_SCORE
            else:
                num = 0
        for star in self.star_list:
            if(n < num):
                self.star_list[n].setStyleSheet("QPushButton{background-image: url(./res/my_ratings_star.png);border:0px;}")
            else:
                self.star_list[n].setStyleSheet("QPushButton{background-image: url(./res/my_rating_star2.png);border:0px;}")
            n = n + 1

    def slot_click_star(self,num,need_send = True):
        pre_my_score = self.MY_SCORE
        if (Globals.LOGIN_SUCCESS):
            self.MY_SCORE = num
        else:
            self.MY_SCORE = 0
        #(^-^)
        if(Globals.LOGIN_SUCCESS):
            if(self.app_detail.app_msg.from_ukscdb == True):
                n = 0
                for star in self.star_list:
                    self.star_list[n].mouse_press = True
                    if (n < num):
                        self.star_list[n].setStyleSheet("QPushButton{background-image: url(./res/my_ratings_star.png);border:0px;}")
                    else:
                        self.star_list[n].setStyleSheet("QPushButton{background-image: url(./res/my_rating_star2.png);border:0px;}")
                    n = n + 1
                #发送评分到服务器 num为多少就是几分
                if(need_send == True):
                    if (self.timer.isActive()):
                        if(self.pre_app_name == self.app_name):
                            self.slot_click_star(pre_my_score, False)
                            self.app_detail.m_parent.mw.show_tip_widget(_("The operation is too frequent. Please try again later!"),True)
                            self.app_detail.m_parent.mw.tip_widget.btn.clicked.connect(self.app_detail.m_parent.mw.slot_close_tipwidget)
                            self.change_star_state()
                        else:
                            if (self.app_detail.btn.text() == _("Open") or self.app_detail.btn.text() == _("installed") or self.app_detail.btn.text() == _("update")):
                                if (type(self.app_detail.app_msg) == ApkInfo):
                                    self.submit_my_rating_apk.emit(num, self.app_detail.app_msg)
                                else:
                                    self.submit_my_rating_app.emit(num, self.app_detail.app_msg)
                                self.timer.start(10000)
                                self.pre_app_name = self.app_name
                            else:
                                # self.refresh_star()
                                self.slot_click_star(self.MY_SCORE, False)
                                self.app_detail.m_parent.mw.show_tip_widget(_("Please install the software before scoring!"), True)
                                self.app_detail.m_parent.mw.tip_widget.btn.clicked.connect(
                                    self.app_detail.m_parent.mw.slot_close_tipwidget)
                                self.change_star_state()
                    else:
                        if(self.app_detail.btn.text() == _("Open") or self.app_detail.btn.text() == _("installed") or self.app_detail.btn.text() == _("update")):
                            if(type(self.app_detail.app_msg) == ApkInfo):
                                self.submit_my_rating_apk.emit(num,self.app_detail.app_msg)
                            else:
                                self.submit_my_rating_app.emit(num, self.app_detail.app_msg)
                            self.timer.start(10000)
                            self.pre_app_name = self.app_name
                        else:
                            # self.refresh_star()
                            self.slot_click_star(pre_my_score, False)
                            # self.slot_click_star(self.MY_SCORE,False)
                            self.app_detail.m_parent.mw.show_tip_widget(_("Please install the software before scoring!"), True)
                            self.app_detail.m_parent.mw.tip_widget.btn.clicked.connect(self.app_detail.m_parent.mw.slot_close_tipwidget)
                            self.change_star_state()
            else:
                if(need_send == True):
                    n = 0
                    for star in self.star_list:
                        self.star_list[n].setStyleSheet("QPushButton{background-image: url(./res/my_rating_star2.png);border:0px;}")
                        n = n + 1
                    self.app_detail.m_parent.mw.show_tip_widget(_("Non database software, does not support scoring!"),True)
                    self.app_detail.m_parent.mw.tip_widget.btn.clicked.connect(self.app_detail.m_parent.mw.slot_close_tipwidget)
                    self.change_star_state()
                else:
                    self.refresh_star()
                    self.change_star_state()
        else:
            self.refresh_star()
            self.app_detail.m_parent.mw.show_tip_widget(_("Please log in before scoring!"))
            self.app_detail.m_parent.mw.tip_widget.btn.clicked.connect(self.app_detail.m_parent.mw.slot_close_tipwidget)
            self.change_star_state()

    #
    #函数：刷新星星
    #
    def refresh_star(self):
        n = 0
        for star in self.star_list:
            self.star_list[n].setStyleSheet("QPushButton{background-image: url(./res/my_rating_star2.png);border:0px;}")
            n = n + 1

    def change_star_state(self):
        n = 0
        for star in self.star_list:
            self.star_list[n].mouse_press = False
            n = n + 1

    def set_progressbar_color(self):
        self.change_background_image.emit()
        re = os.popen("lscpu |grep -i 3A3000").readlines()
        if(re):
            if (Globals.THEME_COLOR == 0):
                self.process1.setStyleSheet(
                    "QProgressBar{background-color:#C0C4CC;border:0px solid #aaaaaa;border-radius:4px;color:#666666;text-align:center;}")
                self.process2.setStyleSheet(
                    "QProgressBar{background-color:#C0C4CC;border:0px solid #aaaaaa;border-radius:4px;color:#666666;text-align:center;}")
                self.process3.setStyleSheet(
                    "QProgressBar{background-color:#C0C4CC;border:0px solid #aaaaaa;border-radius:4px;color:#666666;text-align:center;}")
                self.process4.setStyleSheet(
                    "QProgressBar{background-color:#C0C4CC;border:0px solid #aaaaaa;border-radius:4px;color:#666666;text-align:center;}")
                self.process5.setStyleSheet(
                    "QProgressBar{background-color:#C0C4CC;border:0px solid #aaaaaa;border-radius:4px;color:#666666;text-align:center;}")
                self.my_ratings.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #606265;line-height: 20px;}")
                self.count_of_participation_rating.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #606265;line-height: 20px;}")
            else:
                self.process1.setStyleSheet(
                    "QProgressBar{background-color:#3D3D41;border:0px solid #aaaaaa;border-radius:4px;color:#666666;text-align:center;}")
                self.process2.setStyleSheet(
                    "QProgressBar{background-color:#3D3D41;border:0px solid #aaaaaa;border-radius:4px;color:#666666;text-align:center;}")
                self.process3.setStyleSheet(
                    "QProgressBar{background-color:#3D3D41;border:0px solid #aaaaaa;border-radius:4px;color:#666666;text-align:center;}")
                self.process4.setStyleSheet(
                    "QProgressBar{background-color:#3D3D41;border:0px solid #aaaaaa;border-radius:4px;color:#666666;text-align:center;}")
                self.process5.setStyleSheet(
                    "QProgressBar{background-color:#3D3D41;border:0px solid #aaaaaa;border-radius:4px;color:#666666;text-align:center;}")
                self.my_ratings.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #F9F9F9;line-height: 20px;}")
                self.count_of_participation_rating.setStyleSheet(
                    "QLabel{font-size: 14px;font-weight: 400;color: #8F9399;line-height: 20px;}")
        else:
            if (Globals.THEME_COLOR == 0):
                self.process1.setStyleSheet(
                    "QProgressBar{background-color:#C0C4CC;border:0px solid #aaaaaa;border-radius:4px;color:#666666;text-align:center;}"
                    "QProgressBar:chunk{background-color:#606265;border-radius:4px;}")
                self.process2.setStyleSheet(
                    "QProgressBar{background-color:#C0C4CC;border:0px solid #aaaaaa;border-radius:4px;color:#666666;text-align:center;}"
                    "QProgressBar:chunk{background-color:#606265;border-radius:4px;}")
                self.process3.setStyleSheet(
                    "QProgressBar{background-color:#C0C4CC;border:0px solid #aaaaaa;border-radius:4px;color:#666666;text-align:center;}"
                    "QProgressBar:chunk{background-color:#606265;border-radius:4px;}")
                self.process4.setStyleSheet(
                    "QProgressBar{background-color:#C0C4CC;border:0px solid #aaaaaa;border-radius:4px;color:#666666;text-align:center;}"
                    "QProgressBar:chunk{background-color:#606265;border-radius:4px;}")
                self.process5.setStyleSheet(
                    "QProgressBar{background-color:#C0C4CC;border:0px solid #aaaaaa;border-radius:4px;color:#666666;text-align:center;}"
                    "QProgressBar:chunk{background-color:#606265;border-radius:4px;}")
                self.my_ratings.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #606265;line-height: 20px;}")
                self.count_of_participation_rating.setStyleSheet(
                    "QLabel{font-size: 14px;font-weight: 400;color: #606265;line-height: 20px;}")
            else:
                self.process1.setStyleSheet(
                    "QProgressBar{background-color:#3D3D41;border:0px solid #aaaaaa;border-radius:4px;color:#666666;text-align:center;}"
                    "QProgressBar:chunk{background-color:#8F9399;border-radius:4px;}")
                self.process2.setStyleSheet(
                    "QProgressBar{background-color:#3D3D41;border:0px solid #aaaaaa;border-radius:4px;color:#666666;text-align:center;}"
                    "QProgressBar:chunk{background-color:#8F9399;border-radius:4px;}")
                self.process3.setStyleSheet(
                    "QProgressBar{background-color:#3D3D41;border:0px solid #aaaaaa;border-radius:4px;color:#666666;text-align:center;}"
                    "QProgressBar:chunk{background-color:#8F9399;border-radius:4px;}")
                self.process4.setStyleSheet(
                    "QProgressBar{background-color:#3D3D41;border:0px solid #aaaaaa;border-radius:4px;color:#666666;text-align:center;}"
                    "QProgressBar:chunk{background-color:#8F9399;border-radius:4px;}")
                self.process5.setStyleSheet(
                    "QProgressBar{background-color:#3D3D41;border:0px solid #aaaaaa;border-radius:4px;color:#666666;text-align:center;}"
                    "QProgressBar:chunk{background-color:#8F9399;border-radius:4px;}")
                self.my_ratings.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #F9F9F9;line-height: 20px;}")
                self.count_of_participation_rating.setStyleSheet(
                    "QLabel{font-size: 14px;font-weight: 400;color: #8F9399;line-height: 20px;}")

    #
    #函数：处理计时器超时
    #
    def slot_timer_timeout(self):
        self.timer.stop()
        self.pre_app_name = ""


class start_label(QLabel):
    def __init__(self,parent):
        QLabel.__init__(self, parent)
        self.m_parent = parent
        self.resize(19,19)
        self.init_ui()
        self.m_parent.change_background_image.connect(self.init_ui)

    def init_ui(self):
        if(Globals.THEME_COLOR == 0):
            self.setStyleSheet("QLabel{background-image: url(./res/S.png);background-color:transparent;}")
        else:
            self.setStyleSheet("QLabel{background-image: url(./res/s_dark.png);background-color:transparent;}")


class start_btn(QPushButton):
    enter_btn = pyqtSignal(int)
    click_btn = pyqtSignal(int)
    def __init__(self,num,size,parent):
        QPushButton.__init__(self,parent)
        self.m_parent = parent
        self.mouse_press = False
        self.num = num
        self.size = size
        self.setFocusPolicy(Qt.NoFocus)
        if(self.size == "big"):
            self.resize(42,42)
            self.setStyleSheet("QPushButton{background-image: url(./res/my_rating_star2.png);border:0px;}QPushButton:hover{background-image: url(./res/my_ratings_star.png);}QPushButton:pressed{background-image: url(./res/my_ratings_star2.png);}")
        else:
            self.resize(30, 30)
            self.setStyleSheet(
                "QPushButton{background-image: url(./res/medium_dark_star.png);border:0px;}QPushButton:hover{background-image: url(./res/medium_bright_star.png);}QPushButton:pressed{background-image: url(./res/medium_bright_star.png);}")
        self.enter_btn.connect(self.m_parent.slot_enter_star)
        self.click_btn.connect(self.m_parent.slot_click_star)

    #
    # 函数：鼠标进入事件
    #
    def enterEvent(self,event):
        if (self.mouse_press == False):
            self.enter_btn.emit(self.num)
        self.update()

    #
    # 函数：鼠标按下事件
    #
    def mousePressEvent(self,event):
        if(event.button() == Qt.LeftButton):
            if(self.mouse_press == False):
                self.mouse_press = True
                if((self.m_parent.app_detail.btn.text() == _("Open") or self.m_parent.app_detail.btn.text() == _("installed") or self.m_parent.app_detail.btn.text() == _("update"))
                        and not (self.m_parent.timer.isActive() and self.m_parent.pre_app_name == self.m_parent.app_name)):
                    self.click_btn.emit(self.num)
                else:
                    self.click_btn.emit(self.m_parent.MY_SCORE)
            self.update()

    #
    # 函数：鼠标释放事件
    #
    def mouseReleaseEvent(self,event):
        if(self.mouse_press and self.rect().contains(event.pos())):
            self.mouse_press = True
            self.update()

    #
    # 函数：鼠标离开事件
    #
    def leaveEvent(self,event):
        if(self.mouse_press == False):
            if(self.size == "big"):
                self.setStyleSheet("QPushButton{background-image: url(./res/my_rating_star2.png);border:0px;}")
            else:
                self.setStyleSheet("QPushButton{background-image: url(./res/medium_dark_star.png);border:0px;}")
            self.enter_btn.emit(-1)
        self.update()
