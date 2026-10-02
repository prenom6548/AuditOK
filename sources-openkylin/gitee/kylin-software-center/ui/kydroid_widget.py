#!/usr/bin/python3
# -*- coding: utf-8 -*-


from PyQt5.QtGui import *
from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
from models.globals import *
from models.enums import AppMsg
from ui.category_widget import *
from ui.select_widget import *
import gettext
gettext.bindtextdomain("kylin-software-center", "/usr/share/locale")
gettext.textdomain("kylin-software-center")
_ = gettext.gettext

class Kydroid_Widget(QFrame):
    card_width = 248
    card_height = 124
    card_space = 15
    widget_space = 50
    click_type_btn = pyqtSignal(str,bool)
    def __init__(self,w,h, parent=None):
        QWidget.__init__(self, parent)
        self.w = w
        self.h = h
        self.m_parent = parent
        self.space = 50
        self.init_ui()

    def init_ui(self):
        self.resize(self.w, self.h)
        # self.setAttribute(Qt.WA_TranslucentBackground)
        self.setStyleSheet("QFrame{border:0px solid #FFFFFF;border-bottom-left-radius:12px;border-bottom-right-radius:12px;}")

        self.btn_widget = CategoryBar(Globals.ALL_CATEGORIES,self,True)
        self.btn_widget.setAttribute(Qt.WA_TranslucentBackground)
        # self.btn_widget.resize((self.btn_widget.visiblecategorycount - 1) * ( self.btn_widget.itemwidth + self.btn_widget.categoryspacing) + self.btn_widget.itemwidth, 64)
        self.btn_widget.move(int((self.w - self.btn_widget.width()) / 2), 20)
        self.btn_widget.show()

        self.type_widget = QWidget(self)
        # self.type_widget.setAttribute(Qt.WA_TranslucentBackground)
        self.type_widget.setStyleSheet("QWidget{background-color:transparent;border-radius:0px;}")
        self.type_widget.resize(self.w, self.h - self.btn_widget.height()-self.btn_widget.y())
        self.type_widget.move(0, 65)
        self.type_widget.show()

        self.type_name_label = QLabel(self.type_widget)
        self.type_name_label.setAttribute(Qt.WA_TranslucentBackground)
        self.type_name_label.setFixedHeight(30)
        self.type_name_label.setStyleSheet("QLabel{font-size: 24px;font-weight: 500;color: #303133;line-height: 42px;}")
        self.type_name_label.setText(self.btn_widget.first_btn)
        self.type_name_label.adjustSize()
        self.type_name_label.move(self.space, 0)
        self.type_name_label.hide()

        self.app_widget = QWidget(self.type_widget)
        self.app_widget.setAttribute(Qt.WA_TranslucentBackground)
        self.app_widget.resize(self.w - self.space * 2, self.h)
        self.app_widget.move(self.space, self.type_name_label.y()+self.type_name_label.height()+10)
        self.app_widget.setStyleSheet("QWidget{border-radius: 0px;background:transparent;}")
        self.app_widget.hide()

        self.choose_sorting_type_btn = QPushButton(self.type_widget)
        self.choose_sorting_type_btn.setAttribute(Qt.WA_TranslucentBackground)
        self.choose_sorting_type_btn.setStyleSheet("QPushButton{border-radius: 13px;font-size: 14px;font-weight: 400;color: #3790FA;line-height: 20px;background: transparent;}QPushButton::hover{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
        self.choose_sorting_type_btn.setFocusPolicy(Qt.NoFocus)
        self.type_sorting_btn = QPushButton(self.choose_sorting_type_btn)
        self.type_sorting_btn.setFocusPolicy(Qt.NoFocus)
        self.type_sorting_btn.move(0, 0)
        self.type_sorting_btn.setFixedSize(80, 26)
        self.type_sorting_btn.setStyleSheet(
            "QPushButton{font-size: 14px;font-weight: 400;color: #3790FA;line-height: 20px;background: transparent;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: transparent;font-weight: 500;}")
        self.type_sorting_btn.setText(_("Defualt"))
        self.type_sorting_btn.adjustSize()
        self.type_sorting_btn.show()
        # self.type_sorting_btn.clicked.connect(lambda: self.slot_click_sorting_btn(1))

        self.pull_down_button = QPushButton(self.choose_sorting_type_btn)
        self.pull_down_button.setFocusPolicy(Qt.NoFocus)
        self.pull_down_button.resize(16, 16)
        self.pull_down_button.setStyleSheet(
            "QPushButton{border-radius: 4px;border:0px;background-image:url('res/show_search_black.png')}QPushButton:hover{border:0px;background-image:url('res/show_search_clicked.png')}QPushButton:pressed{border:0px;background-image:url('res/show_search_clicked.png')}")
        self.choose_sorting_type_btn.resize(self.type_sorting_btn.width() + self.pull_down_button.width()+5,
                                            self.type_sorting_btn.height())
        self.pull_down_button.move(self.type_sorting_btn.x() + self.type_sorting_btn.width(),
                                   int((self.type_sorting_btn.height() - self.pull_down_button.height()) / 2))
        self.choose_sorting_type_btn.move(self.app_widget.width() - self.choose_sorting_type_btn.width() + self.app_widget.x() - 10,
            self.type_name_label.y())
        self.choose_sorting_type_btn.clicked.connect(self.slot_choose_sorting_type)
        self.pull_down_button.clicked.connect(self.slot_choose_sorting_type)
        self.type_sorting_btn.clicked.connect(self.slot_choose_sorting_type)

        self.rating_sorting_btn = QPushButton(self.type_widget)
        self.rating_sorting_btn.setFocusPolicy(Qt.NoFocus)
        self.rating_sorting_btn.setFixedSize(80, 26)
        self.rating_sorting_btn.setStyleSheet(
            "QPushButton{font-size: 14px;font-weight: 400;color: #303133;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
        self.rating_sorting_btn.setText(_("Rattings"))
        self.rating_sorting_btn.move( self.app_widget.width() - self.rating_sorting_btn.width() + self.app_widget.x() - 10,
            self.type_name_label.y())
        self.rating_sorting_btn.hide()
        self.rating_sorting_btn.clicked.connect(lambda: self.slot_click_sorting_btn(1))

        self.download_sorting_btn = QPushButton(self.type_widget)
        self.download_sorting_btn.setFocusPolicy(Qt.NoFocus)
        self.download_sorting_btn.setFixedSize(80, 26)
        self.download_sorting_btn.setStyleSheet(
            "QPushButton{font-size: 14px;font-weight: 400;color: #303133;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
        self.download_sorting_btn.setText(_("Top app"))
        self.download_sorting_btn.move(self.rating_sorting_btn.x() - self.download_sorting_btn.width(),
                                       self.type_name_label.y())
        self.download_sorting_btn.hide()
        self.download_sorting_btn.clicked.connect(lambda: self.slot_click_sorting_btn(0))

        self.popularity_sorting_btn = QPushButton(self.type_widget)
        self.popularity_sorting_btn.setFocusPolicy(Qt.NoFocus)
        self.popularity_sorting_btn.setFixedSize(80, 26)
        self.popularity_sorting_btn.setStyleSheet(
            "QPushButton{font-size: 14px;font-weight: 400;color: #303133;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
        self.popularity_sorting_btn.setText(_("Default"))
        self.popularity_sorting_btn.move(self.download_sorting_btn.x() - self.download_sorting_btn.width(),
                                         self.type_name_label.y())
        self.popularity_sorting_btn.hide()
        self.popularity_sorting_btn.clicked.connect(lambda: self.slot_click_sorting_btn(2))

        self.btn_split_line = QLabel(self.type_widget)
        self.btn_split_line.resize(1, 12)
        self.btn_split_line.setStyleSheet("QLabel{border: 1px solid #999999;background-color: #999999;}")
        self.btn_split_line.move(self.popularity_sorting_btn.x() - self.btn_split_line.width() - 6, int(
            (self.popularity_sorting_btn.height() - self.btn_split_line.height()) / 2) + self.popularity_sorting_btn.y())
        self.btn_split_line.hide()

        self.one_click_download = QPushButton(self.type_widget)
        self.one_click_download.setFocusPolicy(Qt.NoFocus)
        self.one_click_download.setText(_("all down"))
        self.one_click_download.adjustSize()
        self.one_click_download.setFixedSize(80, 26)
        self.one_click_download.setStyleSheet(
            "QPushButton{border:0px;font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;border:0px solid #ff0000;}QPushButton:hover{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}QPushButton:pressed{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}")
        self.one_click_download.move(self.choose_sorting_type_btn.x() -self.one_click_download.width()-6,
                                     self.type_name_label.y())
        self.one_click_download.setEnabled(False)

        self.all_check_btn = QPushButton(self.type_widget)
        self.all_check_btn.setFocusPolicy(Qt.NoFocus)
        self.all_check_btn.setText(_("Select all"))
        self.all_check_btn.setFixedSize(80, 26)
        self.all_check_btn.setStyleSheet(
            "QPushButton{font-size: 14px;font-weight: 400;color: #303133;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
        self.all_check_btn.move(self.one_click_download.x() - self.all_check_btn.width(), self.type_name_label.y())

        self.loadingmove = QMovie("./res/loadgif2.gif")
        self.loadingmove_dark = QMovie("./res/loadgifdark.gif")
        self.loadinglabel = QLabel(self)
        self.loadinglabel.setAttribute(Qt.WA_TranslucentBackground)
        self.loadinglabel.resize(2000, 2000)
        self.loadinglabel.move(int((self.width() - self.loadinglabel.width()) / 2),
                               int((self.height() - self.loadinglabel.height()) / 2))
        self.loadinglabel.setAlignment(Qt.AlignCenter)
        if (Globals.THEME_COLOR == 0):
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
        self.scrollArea.setGeometry(0, 65,self.type_widget.width() - 10,  self.height()-self.btn_widget.height()-self.btn_widget.y()-10)
        self.scrollArea.hide()
        self.scrollArea.verticalScrollBar().setStyleSheet("QScrollBar:vertical{margin:0px 0px 0px 0px;background-color:transparent;border:0px;width:6px;border-radius: 3px;}\
                     QScrollBar::handle:vertical{background-color:#C2C2C2;width:6px;border-radius:3px;} QScrollBar::handle:vertical:hover{background-color:#8F9399;opacity:0.1;width:6px;border-radius:3px;}  QScrollBar::handle:vertical:pressed{background-color:#8F9399;width:6px;border-radius:3px;}\
                     QScrollBar::sub-line:vertical{subcontrol-origin:margin;border:0px solid red;height:13px}\
                     QScrollBar::up-arrow:vertical{subcontrol-origin:margin;background-color:rgba(0,0,0,0);height:13px}\
                     QScrollBar::sub-page:vertical{background-color:transparent;}\
                     QScrollBar::add-page:vertical{background-color:transparent;}\
                     QScrollBar::down-arrow:vertical{background-color:rgba(0,0,0,0);}\
                     QScrollBar::add-line:vertical{subcontrol-origin:margin;border:0px solid green;height:13px}")
        self.change_color()
        self.re_move()

    def change_color(self):
        if(Globals.THEME_COLOR == 0):
            self.btn_widget.setStyleSheet("QWidget{background-color:#FFFFFF;border-radius:0px;}")
            palette1 = QPalette()
            palette1.setColor(self.type_widget.backgroundRole(), QColor(255, 255, 255))  # 设置背景颜色
            self.type_widget.setPalette(palette1)
            self.type_widget.setAutoFillBackground(True)
            self.setPalette(palette1)
            self.setAutoFillBackground(True)
            # self.type_widget.setStyleSheet("QWidget{background-color:#FFFFFF;border-radius:0px;}")
            self.type_name_label.setStyleSheet(
                "QLabel{font-size: 24px;font-weight: 500;color: #303133;line-height: 42px;}")
            if (self.one_click_download.isEnabled()):
                self.one_click_download.setStyleSheet(
                    "QPushButton{border:0px;font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;border:0px solid #ff0000;}QPushButton:hover{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}QPushButton:pressed{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}")
            else:
                self.one_click_download.setStyleSheet(
                    "QPushButton{font-size:14px;font-weight: 500;color: #8f9399;line-height: 20px;border:0px solid #ff0000;}")
            self.all_check_btn.setStyleSheet(
                "QPushButton{font-size: 14px;border:0px;font-weight: 500;color: #606265;line-height: 20px;}QPushButton:hover{background: rgba(55, 144, 250, 0.12);color: #3790FA;border-radius: 12px;}PushButton:pressed{background: rgba(55, 144, 250, 0.12);color: #3790FA;border-radius: 12px;}")
            self.scrollArea.verticalScrollBar().setStyleSheet("QScrollBar:vertical{margin:0px 0px 0px 0px;background-color:transparent;border:0px;width:6px;border-radius: 3px;}\
                                 QScrollBar::handle:vertical{background-color:#C2C2C2;width:6px;border-radius:3px;} QScrollBar::handle:vertical:hover{background-color:#8F9399;opacity:0.1;width:6px;border-radius:3px;}  QScrollBar::handle:vertical:pressed{background-color:#8F9399;width:6px;border-radius:3px;}\
                                 QScrollBar::sub-line:vertical{subcontrol-origin:margin;border:0px solid red;height:13px}\
                                 QScrollBar::up-arrow:vertical{subcontrol-origin:margin;background-color:rgba(0,0,0,0);height:13px}\
                                 QScrollBar::sub-page:vertical{background-color:transparent;}\
                                 QScrollBar::add-page:vertical{background-color:transparent;}\
                                 QScrollBar::down-arrow:vertical{background-color:rgba(0,0,0,0);}\
                                 QScrollBar::add-line:vertical{subcontrol-origin:margin;border:0px solid green;height:13px}")
            self.loadinglabel.setMovie(self.loadingmove)
            self.loadingmove.start()
        else:
            self.btn_widget.setStyleSheet("QWidget{background-color:#1F2022;border-radius:0px;}")
            palette1 = QPalette()
            palette1.setColor(self.type_widget.backgroundRole(), QColor(31, 32, 34))  # 设置背景颜色
            self.type_widget.setPalette(palette1)
            self.type_widget.setAutoFillBackground(True)
            self.setPalette(palette1)
            self.setAutoFillBackground(True)
            # self.type_widget.setStyleSheet("QWidget{background-color:#1F2022;border-radius:0px;}")
            self.type_name_label.setStyleSheet(
                "QLabel{font-size: 24px;font-weight: 500;color: #F9F9F9;line-height: 42px;}")
            if (self.one_click_download.isEnabled()):
                self.one_click_download.setStyleSheet(
                    "QPushButton{border:0px;font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;border:0px solid #ff0000;}QPushButton:hover{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}QPushButton:pressed{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}")
            else:
                self.one_click_download.setStyleSheet(
                    "QPushButton{font-size:14px;font-weight: 500;color: #8f9399;line-height: 20px;border:0px solid #ff0000;}")
            self.all_check_btn.setStyleSheet(
                "QPushButton{font-size: 14px;border:0px;font-weight: 500;color: #F9F9F9;line-height: 20px;}QPushButton:hover{background: rgba(55, 144, 250, 0.12);color: #3790FA;border-radius: 12px;}PushButton:pressed{background: rgba(55, 144, 250, 0.12);color: #3790FA;border-radius: 12px;}")
            self.scrollArea.verticalScrollBar().setStyleSheet("QScrollBar:vertical{margin:0px 0px 0px 0px;background-color:transparent;border:0px;width:6px;border-radius: 3px;}\
                                 QScrollBar::handle:vertical{background-color:#6B6B6B;width:6px;border-radius:3px;} QScrollBar::handle:vertical:hover{background-color:#959595;opacity:0.1;width:6px;border-radius:3px;}  QScrollBar::handle:vertical:pressed{background-color:#959595;width:6px;border-radius:3px;}\
                                 QScrollBar::sub-line:vertical{subcontrol-origin:margin;border:0px solid red;height:13px}\
                                 QScrollBar::up-arrow:vertical{subcontrol-origin:margin;background-color:rgba(0,0,0,0);height:13px}\
                                 QScrollBar::sub-page:vertical{background-color:transparent;}\
                                 QScrollBar::add-page:vertical{background-color:transparent;}\
                                 QScrollBar::down-arrow:vertical{background-color:rgba(0,0,0,0);}\
                                 QScrollBar::add-line:vertical{subcontrol-origin:margin;border:0px solid green;height:13px}")
            self.loadinglabel.setMovie(self.loadingmove_dark)
            self.loadingmove_dark.start()


    def re_move(self):
        self.btn_widget.resize((self.btn_widget.visiblecategorycount - 1) * ( self.btn_widget.itemwidth + self.btn_widget.categoryspacing) + self.btn_widget.itemwidth, 64)
        self.btn_widget.move(int((self.width() - self.btn_widget.width()) / 2), 20)
        self.type_widget.resize(self.width(), self.type_widget.height())
        self.type_widget.move(0, 65)
        self.app_widget.resize(self.width() - self.space * 2, self.height())
        self.app_widget.move(self.space, self.type_name_label.height()+self.type_name_label.y()+30)
        self.scrollArea.setGeometry(0, self.type_widget.y(),self.type_widget.width() - 10, self.height()-self.btn_widget.height()-self.btn_widget.y()-10)
        self.reload_card(self.app_widget,self.scrollArea)
        self.type_widget.resize(self.type_widget.width(),
                                self.app_widget.y()  + self.app_widget.height()+30)
        self.choose_sorting_type_btn.move(self.app_widget.width() - self.choose_sorting_type_btn.width() + self.app_widget.x() - 10,
            self.type_name_label.y())
        self.btn_split_line.move(self.popularity_sorting_btn.x() - self.btn_split_line.width() - 6,
                                 int((self.popularity_sorting_btn.height() - self.btn_split_line.height()) / 2) + self.popularity_sorting_btn.y())
        self.one_click_download.move(self.choose_sorting_type_btn.x() - self.one_click_download.width() - 6,
                                     self.type_name_label.y())
        self.all_check_btn.move(self.one_click_download.x() - self.all_check_btn.width(), self.type_name_label.y())
        self.rating_sorting_btn.move(
            self.app_widget.width() - self.rating_sorting_btn.width() + self.app_widget.x() - 10,
            self.type_name_label.y())
        self.download_sorting_btn.move(self.rating_sorting_btn.x() - self.download_sorting_btn.width(),
                                       self.type_name_label.y())
        self.btn_split_line.move(self.popularity_sorting_btn.x() - self.btn_split_line.width() - 6, int(
            (self.popularity_sorting_btn.height() - self.btn_split_line.height()) / 2) + self.popularity_sorting_btn.y())
        self.one_click_download.move(self.choose_sorting_type_btn.x() - self.one_click_download.width() - 6,
                                     self.type_name_label.y())
        self.all_check_btn.move(self.one_click_download.x() - self.all_check_btn.width(), self.type_name_label.y())

    def slot_click_sorting_btn(self,sort_type,need_emit_signal = True):
        # if(Globals.SORTING_METHOD == 0):
        #     Globals.SORTING_METHOD = 1
        # else:
        #     Globals.SORTING_METHOD = 0
        if (sort_type == 0):
            Globals.KYDROID_SORTING_METHOD = 0
            if (Globals.THEME_COLOR == 0):
                self.download_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;background: rgba(55, 144, 250, 0.12);border-radius: 13px;}")
                self.rating_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 400;color: #303133;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
                self.popularity_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 400;color: #303133;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
            else:
                self.download_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;background: rgba(55, 144, 250, 0.12);border-radius: 13px;}")
                self.rating_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
                self.popularity_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
        elif (sort_type == 1):
            Globals.KYDROID_SORTING_METHOD = 1
            if (Globals.THEME_COLOR == 0):
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
            Globals.KYDROID_SORTING_METHOD = 2
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
        if(need_emit_signal == True):
            self.click_type_btn.emit(self.type_name_label.text(),False)

    #
    # 函数：处理点击选择排行类型
    #
    def slot_choose_sorting_type(self):
        if (not hasattr(self, "select_widget")):
            item_name_list = [_("Default"), _("Top app"), _("Rattings")]
            self.select_widget = Select_Widget(102, 30, 3, item_name_list, False, self.type_widget)
            self.select_widget.click_item.connect(self.slot_click_select_list_item)
            item_count = self.select_widget.item_list.count()
            for i in range(item_count):
                item = self.select_widget.item_list.item(i)
                get_item_widget = self.select_widget.item_list.itemWidget(item)
                if (Globals.KYDROID_SORTING_METHOD == 2):
                    if (get_item_widget.item_name_label.text() == _("Default")):
                        get_item_widget.icon_label.show()
                    else:
                        get_item_widget.icon_label.hide()
                elif (Globals.KYDROID_SORTING_METHOD == 1):
                    if (get_item_widget.item_name_label.text() == _("Top app")):
                        get_item_widget.icon_label.show()
                    else:
                        get_item_widget.icon_label.hide()
                else:
                    if (get_item_widget.item_name_label.text() == _("Rattings")):
                        get_item_widget.icon_label.show()
                    else:
                        get_item_widget.icon_label.hide()
        self.select_widget.move(
            self.x() + self.type_widget.x() + self.choose_sorting_type_btn.x() + self.choose_sorting_type_btn.width() - self.select_widget.width() + 6,
            self.y() + self.type_widget.y() + self.choose_sorting_type_btn.y() + self.choose_sorting_type_btn.height() - 6)
        self.select_widget.slot_show_dark_mode()
        self.select_widget.show()

    #
    # 函数：处理按钮的显示以及select_widget勾选状态
    #
    def check_choose_btn_type(self):
        if (Globals.KYDROID_SORTING_METHOD == 2):
            self.type_sorting_btn.setText(_("Default"))
            self.type_sorting_btn.adjustSize()
        elif (Globals.KYDROID_SORTING_METHOD == 1):
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
                if (Globals.KYDROID_SORTING_METHOD == 2):
                    if (get_item_widget.item_name_label.text() == _("Default")):
                        get_item_widget.icon_label.show()
                    else:
                        get_item_widget.icon_label.hide()
                elif (Globals.KYDROID_SORTING_METHOD == 1):
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
    # 函数：处理点击list_item
    #
    def slot_click_select_list_item(self, item):
        get_item_widget = self.select_widget.item_list.itemWidget(item)
        text = get_item_widget.item_name_label.text()
        get_item_widget.icon_label.show()
        self.type_sorting_btn.setText(text)
        self.type_sorting_btn.adjustSize()
        self.select_widget.hide()
        if (text == _("Default")):
            self.slot_click_sorting_btn(2)
        elif (text == _("Top app")):
            self.slot_click_sorting_btn(0)
        else:
            self.slot_click_sorting_btn(1)


    #
    # 函数：重新设置卡片
    #
    def reload_card(self, widget, scrollArea):
        cards = widget.children()
        if (len(cards) != 0):
            h_num = int(widget.width() / (self.card_width + self.card_space))
            get_card_width = int((widget.width() - (self.card_space * (h_num - 1)) - 20) / h_num)
            proportion = get_card_width / widget.width()
            get_card_height = self.card_height
            v_num = 0
            num = 0
            for i in range(len(cards)):
                card = cards[i]
                card.resize(get_card_width, get_card_height)
                card.re_move()
                if (num < h_num):
                    if (num == 0):
                        card.move(5, 5 + (20 + card.height()) * v_num)
                        widget.resize(widget.width(), 5 + (20 + card.height()) * (v_num + 1))
                    else:
                        card.move(num * (card.width() + self.card_space) + 5, 5 + (20 + card.height()) * v_num)
                    num = num + 1
                else:
                    num = 0
                    v_num = v_num + 1
                    if (num == 0):
                        card.move(5, 5 + (20 + card.height()) * v_num)
                        widget.resize(widget.width(), 5 + (20 + card.height()) * (v_num + 1))
                    else:
                        card.move(num * (card.width() + self.card_space) + 5, 5 + (20 + card.height()) * v_num)
                    num = num + 1


    #
    # 函数名:设置滚动条起始位置
    # Function:set verticalSscrollBar starting point
    #
    def scrollToTop(self):
        vsb = self.scrollArea.verticalScrollBar()
        vsb.setValue(0)


    def mouseReleaseEvent(self, event):
        self.m_parent.mw.hide_tip_widget()
        if (hasattr(self, "select_widget")):
            self.select_widget.hide()

    # 鼠标进入事件

    def enterEvent(self, event):
        # self.status = BUTTON_ENTER
        self.scrollArea.verticalScrollBar().show()
        # self.scrollArea.show()

    #
    # 函数：鼠标离开事件
    #

    def leaveEvent(self, event):
        # self.status = BUTTON_LEAVE
        self.scrollArea.verticalScrollBar().hide()

    #
    # 函数：鼠标按下事件
    #
    def mousePressEvent(self, event):
        if event.button() == Qt.LeftButton:
            # self.status = BUTTON_PRESSED
            pass
        else:
            event.ignore()