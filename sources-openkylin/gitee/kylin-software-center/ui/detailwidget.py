#!/usr/bin/python3
# -*- coding: utf-8 -*-

from PyQt5.QtGui import *
from PyQt5.QtCore import *
from concurrent.futures import ThreadPoolExecutor
from PyQt5.QtWidgets import QWidget, QHBoxLayout, QLabel, QFrame, QApplication,QScrollArea
from ui.New_Arrivals_Widget import *
from ui.ad_widget import *
from ui.card_widget import *
from ui.Popular_applications_widget import *
from ui.download_ranking_widget import *
from ui.recommended_list_widget import *
from ui.app_detail_widget import *
from ui.mine_widget import *
from ui.kydroid_widget import *
from ui.category_widget import *
from ui.drive_widget import *
from ui.tip_widget import *
from models.enums import DOWNLOADING,PUASE,OTHER,INSTALLING,DB_ITEM,PageStates,ClickCard
import apt
import gettext
gettext.bindtextdomain("kylin-software-center", "/usr/share/locale")
gettext.textdomain("kylin-software-center")
_ = gettext.gettext

pool = ThreadPoolExecutor(max_workers=1)

class DetailWidget(QFrame):
    first_click_card = True
    first_show_mywidget = True
    first_show_category = True
    tip_error = pyqtSignal(str)
    get_list_over = pyqtSignal(str,list,list)
    card_width = 248
    card_height = 124
    card_space = 15
    scrollArea_change = pyqtSignal()
    init_cache_signal = pyqtSignal()
    init_cache_over = pyqtSignal()
    # change_category_clolor = pyqtSignal()
    def __init__(self,mw,w,h,parent = None):
        QWidget.__init__(self, parent)
        self.m_parent = parent
        self.mw = mw
        self.w = w
        self.h = h
        self.space = 50
        self.proportion = Globals.REMEMBER_WIDTH / Globals.INIT_WIDTH
        # print("init_detail",datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
        self.init_ui()
        # print("init_detail_over",datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
        self.setWindowOpacity(1)
        self.show()
        self.init_connect()
        self.ca = None
        # pool.submit(self.init_cache)
        # try:
        #     self.ca = apt.Cache()
        #     self.ca.open()
        # except:
        #     self.ca = None

    def init_cache(self):
        try:
            self.ca = apt.Cache()
            # self.ca.open()
        except:
            self.ca = None
        self.init_cache_over.emit()


    def init_ui(self):
        self.resize(self.w,self.h)
        self.setWindowFlags(Qt.FramelessWindowHint)
        # if(Globals.THEME_COLOR == 0):
        #     if(Globals.RIGHT_ANGLE):
        #         self.setStyleSheet(".QFrame{background: rgba(255, 255, 255, 1); background-color:#ffffff;border:0px;}")
        #     else:
        #         self.setStyleSheet(".QFrame{background: rgba(255, 255, 255, 1);border-bottom-left-radius:6px;border-bottom-right-radius:6px; background-color:#ffffff;border:0px;}")
        # else:
        #     if (Globals.RIGHT_ANGLE):
        #         self.setStyleSheet("QFrame{border:0px;background-color:#1F2022;}")
        #     else:
        #         self.setStyleSheet("QFrame{border:0px;border-bottom-left-radius:6px;border-bottom-right-radius:6px; background-color:#1F2022;}")
        # self.setStyleSheet( "QFrame{border:0px;border-bottom-left-radius:6px;border-bottom-right-radius:6px; background-color:transparent;}")

        self.loadingmove = QMovie("./res/loadgif2.gif")
        self.loadingmove_dark = QMovie("./res/loadgifdark.gif")
        self.loadinglabel = QLabel(self)
        self.loadinglabel.setAttribute(Qt.WA_TranslucentBackground)
        self.loadinglabel.resize(32,32)
        self.loadinglabel.move(int((self.width()-self.loadinglabel.width())/2),int((self.height()-self.loadinglabel.height())/2))
        self.loadinglabel.setAlignment(Qt.AlignCenter)
        if(Globals.THEME_COLOR == 0):
            self.loadinglabel.setMovie(self.loadingmove)
            self.loadingmove.start()
        else:
            self.loadinglabel.setMovie(self.loadingmove_dark)
            self.loadingmove_dark.start()
        self.loadinglabel.show()

        self.loading_kydroid_evn = QLabel(self)
        self.loading_kydroid_evn.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #606265;line-height: 24px;}")
        self.loading_kydroid_evn.setText(_("The environment is initialized, the interface is temporarily locked, please wait..."))
        self.loading_kydroid_evn.adjustSize()
        self.loading_kydroid_evn.move(self.loadinglabel.x()+int((self.loadinglabel.width()-self.loading_kydroid_evn.width())/2),self.loadinglabel.y()+self.loadinglabel.height()+20)
        self.loading_kydroid_evn.hide()

        # 设置一个滚动区域，当超过时自动出现滚动条
        self.scrollArea = QScrollArea(self)
        self.show_homepage_widget = QWidget(self)
        self.show_homepage_widget.setAttribute(Qt.WA_TranslucentBackground)
        self.show_homepage_widget.setStyleSheet(".QWidget{border-bottom-left-radius:6px;border-bottom-right-radius:6px; background-color:#ffffff;}")
        self.show_homepage_widget.setMaximumSize(self.width(),2000)
        self.show_homepage_widget.setWindowFlags(Qt.FramelessWindowHint)
        self.show_homepage_widget.setAutoFillBackground(True)
        palette1 = QPalette()
        palette1.setColor(self.backgroundRole(), QColor(0, 222, 123))
        self.show_homepage_widget.setPalette(palette1)
        self.scrollArea.setWidget(self.show_homepage_widget)
        self.scrollArea.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self.scrollArea.horizontalScrollBar().setEnabled(False)
        self.scrollArea.setVerticalScrollBarPolicy(Qt.ScrollBarAsNeeded)

        w1 = int((self.width()-60 - 20) * (396 / (Globals.INIT_DETAIL_WIDTH - 60 - 20)))
        w2 = int((self.width()-60 - 20) * (864 / (Globals.INIT_DETAIL_WIDTH - 60 - 20)))
        #self.width()-60+20:+20是为了增加hover阴影
        self.ad_widget = Advertisement_Widget(self.width()-60+20,int(263 * self.proportion),w1,w2,self.mw,True,self.show_homepage_widget)
        self.ad_widget_height = 263
        self.ad_widget.setAttribute(Qt.WA_TranslucentBackground)
        self.ad_widget.move(20,0)
        self.ad_widget.show()
        self.download_ranking_widget = Download_Ranking_Widget(self.show_homepage_widget)
        self.download_ranking_widget.more_btn.clicked.connect(self.mw.slot_show_more_downrankapp)
        self.download_ranking_widget.move(self.width()-30-self.download_ranking_widget.width(),self.ad_widget.height()+self.ad_widget.y()+36)
        self.download_ranking_widget.show()
        #+10：加10px的阴影
        self.new_arrivals_widget = New_Arrivals_Widget(self.width()-60-self.download_ranking_widget.width()-30+10,self.mw,self.show_homepage_widget)
        self.new_arrivals_widget.more_btn.clicked.connect(self.mw.slot_show_more_newapp)
        self.new_arrivals_widget.move(20,self.ad_widget.height()+self.ad_widget.y()+36)
        self.new_arrivals_widget.show()
        self.hot_app_widget = Popular_Applications_Widget(self.new_arrivals_widget.width(),self.mw,self.show_homepage_widget)
        self.hot_app_widget.more_btn.clicked.connect(self.mw.slot_show_more_popularapp)
        self.hot_app_widget.move(25,self.new_arrivals_widget.y()+self.new_arrivals_widget.height()+36)
        self.hot_app_widget.show()
        self.recommended_list_widget = Recommended_List_Widget(self.width()-60,self.show_homepage_widget)
        self.recommended_list_widget.move(20, self.hot_app_widget.y()+self.hot_app_widget.height()+36)
        self.recommended_list_widget.show()
        self.recommended_android_list_widget = Recommended_List_Widget(self.width() - 60, self.show_homepage_widget)
        self.recommended_android_list_widget.titellabel.setText(_("Mobile applications that everyone is using"))
        self.recommended_android_list_widget.adjustSize()
        self.recommended_android_list_widget.move(20, self.recommended_list_widget.y() + self.recommended_list_widget.height() + 36)
        self.recommended_android_list_widget.hide()

        self.mywidget = Mine_Widget(self.width(),self.height(),self)
        self.mywidget.click_type_btn.connect(self.mw.slot_clicked_my_widget_btn)
        self.mywidget.all_select_btn.clicked.connect(self.mw.slot_click_all_check_btn)
        self.mywidget.all_check.clicked.connect(self.mw.slot_check_all_operation)
        self.mywidget.download_all_check.clicked.connect(self.mw.slot_download_check_all_operation)
        self.mywidget.hide()
        self.first_show_mywidget = False

        # self.kydroid_widget = Kydroid_Widget(self.width(),self.height()-20,self)
        # self.kydroid_widget.scrollArea.verticalScrollBar().valueChanged.connect(self.mw.slot_scrollbar_valuechange)
        # self.kydroid_widget.move(0,10)
        # self.kydroid_widget.hide()

        # self.scrollArea.verticalScrollBar().setStyleSheet("QScrollBar:vertical{margin:0px 0px 0px 0px;background-color:transparent;border:0px;width:6px;border-radius: 3px;}\
        #              QScrollBar::handle:vertical{background-color:#C2C2C2;width:6px;border-radius:3px;} QScrollBar::handle:vertical:hover{background-color:#8F9399;opacity:0.1;width:6px;border-radius:3px;}  QScrollBar::handle:vertical:pressed{background-color:#8F9399;width:6px;border-radius:3px;}\
        #              QScrollBar::sub-line:vertical{subcontrol-origin:margin;border:0px solid red;height:13px}\
        #              QScrollBar::up-arrow:vertical{subcontrol-origin:margin;background-color:rgba(0,0,0,0);height:13px}\
        #              QScrollBar::sub-page:vertical{background-color:transparent;}\
        #              QScrollBar::add-page:vertical{background-color:transparent;}\
        #              QScrollBar::down-arrow:vertical{background-color:rgba(0,0,0,0);}\
        #              QScrollBar::add-line:vertical{subcontrol-origin:margin;border:0px solid green;height:13px}")

        # # 设置一个滚动区域，当超过时自动出现滚动条
        # self.show_homepage_widget = QWidget()
        # self.scrollArea = Scroll_Widget(self.show_homepage_widget)
        #
        # self.win_replace_widget = Win_Replace_Widget(self)
        # self.win_replace_widget.move(30,0)
        # self.win_replace_widget.show()
        # self.show_homepage_widget.show()\

        self.scrollArea.setGeometry(0, 10, self.width()-5, self.height()-10)
        self.show_homepage_widget.setGeometry(0, 0, self.width(), int(self.recommended_android_list_widget.y()+self.recommended_android_list_widget.height()+30))
        self.show_homepage_widget.hide()
        self.scrollArea.hide()

        self.show_homepage_widget2 = QWidget(self)
        # self.show_homepage_widget2.setMaximumWidth(self.width())
        self.show_homepage_widget2.setWindowFlags(Qt.FramelessWindowHint)
        self.show_homepage_widget2.setAutoFillBackground(True)
        self.show_homepage_widget2.setPalette(palette1)
        self.show_homepage_widget2.setGeometry(0, 0, self.width(),self.height())
        self.scrollArea2 = QScrollArea(self.show_homepage_widget2)

        self.text_prompt_label = QLabel(self)
        self.text_prompt_label.setAttribute(Qt.WA_TranslucentBackground)
        self.text_prompt_label.setAlignment(Qt.AlignHCenter)
        self.text_prompt_label.hide()

        self.no_found_tip = QFrame(self)
        self.no_found_tip.resize(300,100)
        self.no_found_text1 = QLabel(self.no_found_tip)
        self.no_found_text1.setAlignment(Qt.AlignHCenter)
        self.no_found_text1.setAttribute(Qt.WA_TranslucentBackground)
        self.no_found_text1.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #BBBDCA;line-height: 24px;}")
        self.no_found_text1.setText(_("Sorry, we didn't find the app you want!"))
        self.no_found_text1.adjustSize()
        self.no_found_text1.move(0,0)
        self.no_found_text2 = QLabel(self.no_found_tip)
        self.no_found_text2.setAlignment(Qt.AlignHCenter)
        self.no_found_text2.setAttribute(Qt.WA_TranslucentBackground)
        self.no_found_text2.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #BBBDCA;line-height: 24px;}")
        self.no_found_text2.setText(_("You can choose"))
        self.no_found_text2.adjustSize()
        self.no_found_text2.move(0, self.no_found_text1.height())
        self.all_search_btn = QPushButton(self.no_found_tip)
        self.all_search_btn.resize(self.no_found_text2.width()+20,self.no_found_text2.height())
        self.all_search_btn.setFocusPolicy(Qt.NoFocus)
        self.all_search_btn.setAttribute(Qt.WA_TranslucentBackground)
        self.all_search_btn.setStyleSheet("QPushButton{font-size: 14px;font-weight: 400;border:0px;line-height: 24px;text-align: center;background-color:transparent;color:#3790FA;}")
        self.all_search_btn.move(self.no_found_text2.x()+self.no_found_text2.width(),self.no_found_text2.y())
        self.locale_lan = QLocale.system().name()
        if (self.locale_lan == "zh_CN"):
            self.all_search_btn.resize(50,self.no_found_text2.height())
        self.all_search_btn.setText(_("full library"))
        self.all_search_btn.adjustSize()
        self.all_search_btn.clicked.connect(self.slot_cilck_all_search_btn)
        self.no_found_text3 = QLabel(self.no_found_tip)
        self.no_found_text3.setAlignment(Qt.AlignHCenter)
        self.no_found_text3.setAttribute(Qt.WA_TranslucentBackground)
        self.no_found_text3.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #BBBDCA;line-height: 24px;}")
        self.no_found_text3.setText(_("search to try"))
        self.no_found_text3.adjustSize()
        self.no_found_text3.move(self.all_search_btn.x()+self.all_search_btn.width(),self.all_search_btn.y())
        self.no_found_text1.move(int((self.no_found_tip.width()-self.no_found_text1.width())/2),0)
        self.no_found_text2.move(int((self.no_found_tip.width()-self.no_found_text2.width()-self.all_search_btn.width()-self.no_found_text3.width())/2),self.no_found_text1.height())
        self.all_search_btn.move(self.no_found_text2.x() + self.no_found_text2.width()+3, self.no_found_text2.y()+int(self.no_found_text2.height()-self.all_search_btn.height()))
        self.no_found_text3.move(self.all_search_btn.x() + self.all_search_btn.width()+3, self.all_search_btn.y())
        self.no_found_tip.hide()

        self.no_app_tip_label = QLabel(self)
        self.no_app_tip_label.resize(240, 240)
        if(Globals.THEME_COLOR == 0):
            self.no_app_tip_label.setStyleSheet("QLabel{border-radius: 12px;background-image:url(./res/no_search.png);}")
        else:
            self.no_app_tip_label.setStyleSheet("QLabel{border-radius: 12px;background-image:url(./res/no_search_dark.png);}")
        # self.no_app_tip_label.move(int((self.width() - self.no_app_tip_label.width()) / 2),
        #                            int((self.height() - self.no_app_tip_label.height()) / 2))
        self.no_app_tip_label.move(self.mywidget.no_app_tip_label.x()+self.mywidget.x()-15,int((self.height() - self.no_app_tip_label.height()) / 2))
        self.no_app_tip_label.hide()

        self.search_title_label = QLabel(self.show_homepage_widget2)
        # self.search_title_label.setAttribute(Qt.WA_TranslucentBackground)
        self.search_title_label.setFixedHeight(42)
        # self.search_title_label.setStyleSheet("QLabel{font-size: 24px;font-weight: 600;background:red;color: #303133;line-height: 42px;}")
        self.search_title_label.setText(_("search result"))
        self.search_title_label.move(self.space,25)
        self.search_title_label.show()

        self.all_search_tip_label = QLabel(self.show_homepage_widget2)
        self.all_search_tip_label.setAlignment(Qt.AlignLeft)
        self.all_search_tip_label.setStyleSheet("QLabel{font-size: 12px;font-weight: 400;color: #8F9399;line-height: 17px;}")
        self.all_search_tip_label.setText(_("search applications from software sources, there may be unavailable or quality problems"))
        self.all_search_tip_label.adjustSize()
        self.all_search_tip_label.setFixedHeight(17)
        self.all_search_tip_label.move(self.search_title_label.x()+self.search_title_label.width()+10, self.search_title_label.y() + self.search_title_label.height() - self.all_search_tip_label.height()-5)
        self.all_search_tip_label.hide()

        self.search_widget_line = QLabel(self.show_homepage_widget2)
        self.search_widget_line.resize(self.show_homepage_widget2.width()-self.space*2,1)
        # self.search_widget_line.setStyleSheet("QLabel{border: 1px solid #E5E7EE;}")
        self.search_widget_line.move(self.space,self.search_title_label.y()+self.search_title_label.height()+10)
        self.search_widget_line.show()

        self.search_app_widget = QWidget(self.show_homepage_widget2)
        self.search_app_widget.setAttribute(Qt.WA_TranslucentBackground)
        self.search_app_widget.setStyleSheet("QWidget{border:0px;background-color:transparent;}")
        self.search_app_widget.show()

        #用于判断搜索界面是否显示卸载按钮
        self.search_is_remove = False
        #用于判断是否需要进行全库搜索的按钮
        self.need_search_full_library = True


        self.scrollArea2.setWidget(self.search_app_widget)
        self.scrollArea2.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self.scrollArea2.horizontalScrollBar().setEnabled(False)
        self.scrollArea2.setVerticalScrollBarPolicy(Qt.ScrollBarAsNeeded)
        # self.scrollArea2.verticalScrollBar().setStyleSheet("QScrollBar:vertical{margin:0px 0px 0px 0px;background-color:transparent;border:0px;width:6px;border-radius: 3px;}\
        #              QScrollBar::handle:vertical{background-color:#C2C2C2;width:6px;border-radius:3px;} QScrollBar::handle:vertical:hover{background-color:#8F9399;opacity:0.1;width:6px;border-radius:3px;}  QScrollBar::handle:vertical:pressed{background-color:#8F9399;width:6px;border-radius:3px;}\
        #              QScrollBar::sub-line:vertical{subcontrol-origin:margin;border:0px solid red;height:13px}\
        #              QScrollBar::up-arrow:vertical{subcontrol-origin:margin;background-color:rgba(0,0,0,0);height:13px}\
        #              QScrollBar::sub-page:vertical{background-color:transparent;}\
        #              QScrollBar::add-page:vertical{background-color:transparent;}\
        #              QScrollBar::down-arrow:vertical{background-color:rgba(0,0,0,0);}\
        #              QScrollBar::add-line:vertical{subcontrol-origin:margin;border:0px solid green;height:13px}")

        self.search_app_widget.setGeometry(self.space, int(self.search_widget_line.y() + self.search_widget_line.height() + 20),
                                           self.width() - self.space * 2,
                                           int(self.show_homepage_widget2.height() - self.search_widget_line.y() - self.search_widget_line.height()))
        self.scrollArea2.setGeometry(self.space, int(self.search_app_widget.y()), int(self.width()-self.space-10), int(self.height()-self.search_widget_line.y() - self.search_widget_line.height()- 30))
        self.scrollArea2.hide()
        self.show_homepage_widget2.hide()
        self.init_more_widget()

    def init_more_widget(self):
        self.show_homepage_widget3 = QWidget(self)
        self.show_homepage_widget3.setStyleSheet(".QWidget{border-radius: 4px;}")
        # self.show_homepage_widget2.setMaximumWidth(self.width())
        self.show_homepage_widget3.setWindowFlags(Qt.FramelessWindowHint)
        self.show_homepage_widget3.setAutoFillBackground(True)
        palette1 = QPalette()
        palette1.setColor(self.backgroundRole(), QColor(0, 222, 123))
        self.show_homepage_widget3.setPalette(palette1)
        self.show_homepage_widget3.setGeometry(self.space, 0, self.width() - self.space,
                                               self.show_homepage_widget.height())

        self.more_widget_title_label = QLabel(self.show_homepage_widget3)
        # self.search_title_label.setAttribute(Qt.WA_TranslucentBackground)
        self.more_widget_title_label.setFixedHeight(42)
        self.more_widget_title_label.setStyleSheet("QLabel{font-size: 24px;font-weight: 600;background:transparent;color: #303133;line-height: 42px;}")
        self.more_widget_title_label.setText(_("search result"))
        self.more_widget_title_label.move(0, 25)
        self.more_widget_title_label.show()

        self.more_widget_line = QLabel(self.show_homepage_widget3)
        self.more_widget_line.resize(self.show_homepage_widget3.width() - self.space, 2)
        self.more_widget_line.setStyleSheet("QLabel{border: 1px solid #E5E7EE;}")
        self.more_widget_line.move(0, self.search_title_label.y() + self.search_title_label.height() + 10)
        self.more_widget_line.show()

        self.show_more_app_widget = QWidget(self.show_homepage_widget3)
        self.show_more_app_widget.setAttribute(Qt.WA_TranslucentBackground)
        self.show_more_app_widget.setStyleSheet("QWidget{border:0px;background-color:transparent;}")
        self.show_more_app_widget.show()

        self.scrollArea3 = QScrollArea(self.show_homepage_widget3)
        self.scrollArea3.setWidget(self.show_more_app_widget)
        self.scrollArea3.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self.scrollArea3.horizontalScrollBar().setEnabled(False)
        self.scrollArea3.setVerticalScrollBarPolicy(Qt.ScrollBarAsNeeded)
        self.scrollArea3.verticalScrollBar().setStyleSheet("QScrollBar:vertical{margin:0px 0px 0px 0px;background-color:transparent;border:0px;width:6px;border-radius: 3px;}\
                             QScrollBar::handle:vertical{background-color:#C2C2C2;width:6px;border-radius:3px;} QScrollBar::handle:vertical:hover{background-color:#8F9399;opacity:0.1;width:6px;border-radius:3px;}  QScrollBar::handle:vertical:pressed{background-color:#8F9399;width:6px;border-radius:3px;}\
                             QScrollBar::sub-line:vertical{subcontrol-origin:margin;border:0px solid red;height:13px}\
                             QScrollBar::up-arrow:vertical{subcontrol-origin:margin;background-color:rgba(0,0,0,0);height:13px}\
                             QScrollBar::sub-page:vertical{background-color:transparent;}\
                             QScrollBar::add-page:vertical{background-color:transparent;}\
                             QScrollBar::down-arrow:vertical{background-color:rgba(0,0,0,0);}\
                             QScrollBar::add-line:vertical{subcontrol-origin:margin;border:0px solid green;height:13px}")

        self.show_more_app_widget.setGeometry(0, int(self.more_widget_line.y() + self.more_widget_line.height() + 20),
                                           self.width() - self.space * 2,
                                           int(self.show_homepage_widget3.height() - self.more_widget_line.y() - self.more_widget_line.height()))
        self.scrollArea3.setGeometry(0, int(self.show_more_app_widget.y()), int(self.width() - self.space - 10),
                                     int(self.height() - self.more_widget_line.y() - self.more_widget_line.height() - 30))
        self.scrollArea3.hide()
        self.show_homepage_widget3.hide()

    def re_move(self):
        self.show_homepage_widget.setMaximumSize(self.width(), 9000)
        self.scrollArea.setGeometry(0, 10, self.width() - 5, self.height() - 10)
        # self.show_homepage_widget.setGeometry(10, 0, self.width(),
        #                                       self.recommended_list_widget.y() + self.recommended_list_widget.height() + 30)
        self.ad_widget.resize(self.width()-60+20,round(self.ad_widget_height * self.proportion)+20)
        self.ad_widget.re_move(self.proportion)
        self.new_arrivals_widget.resize(self.width()-60-self.download_ranking_widget.width()-30+10,222)
        if (self.new_arrivals_widget.card_list_widget.width() < self.new_arrivals_widget.width()):
            self.new_arrivals_widget.card_list_widget2.hide()
            self.new_arrivals_widget.card_list_widget.move(0,self.new_arrivals_widget.titellabel.height()+10)
            self.new_arrivals_widget.start_move_time.stop()
        else:
            self.new_arrivals_widget.start_move_time.start(4000)
        # else:
        #     self.new_arrivals_widget.start_move_time.stop()
        #     self.new_arrivals_widget.start_move_time.start(4000)
        #     self.new_arrivals_widget.card_list_widget2.move(self.new_arrivals_widget.card_list_widget.x() + self.new_arrivals_widget.card_list_widget.width()+20, self.new_arrivals_widget.titellabel.height()+20)
        #     self.new_arrivals_widget.card_list_widget2.show()
        # self.new_arrivals_widget.card_list_widget.show()
        self.hot_app_widget.resize(self.new_arrivals_widget.width(),240)
        self.hot_app_widget.re_move()
        self.recommended_list_widget.resize(self.width()-60, 450)
        self.recommended_list_widget.reload_card()
        self.recommended_android_list_widget.resize(self.width()-60, 450)
        self.recommended_android_list_widget.reload_card()
        self.mywidget.resize(self.width(),self.height())
        self.mywidget.re_move()
        # ky_value = self.kydroid_widget.scrollArea.verticalScrollBar().value()
        # print("**&*&*",ky_value)
        if (hasattr(self, "kydroid_widget")):
            self.kydroid_widget.resize(self.width(),self.height()-20)
            self.kydroid_widget.re_move()
            # self.kydroid_widget.scrollArea.verticalScrollBar().setValue(ky_value)
            # print("OPOPOP",self.kydroid_widget.scrollArea.verticalScrollBar().value(),ky_value)
        self.loadinglabel.move(int((self.width() - self.loadinglabel.width()) / 2),
                               int((self.height() - self.loadinglabel.height()) / 2))
        self.loading_kydroid_evn.move(
            self.loadinglabel.x() + int((self.loadinglabel.width() - self.loading_kydroid_evn.width()) / 2),
            self.loadinglabel.y() + self.loadinglabel.height() + 20)
        self.download_ranking_widget.move(self.width() - 30 - self.download_ranking_widget.width(),
                                          self.ad_widget.height() + self.ad_widget.y() + 36)
        self.download_ranking_widget.more_btn.move(self.download_ranking_widget.width() - self.download_ranking_widget.more_btn.width(),
                           self.download_ranking_widget.titel_label.y() + int((self.download_ranking_widget.titel_label.height() - self.download_ranking_widget.more_btn.height()) / 2))
        self.new_arrivals_widget.move(20, self.ad_widget.height() + self.ad_widget.y() + 36)
        self.new_arrivals_widget.more_btn.move(self.new_arrivals_widget.width()-self.new_arrivals_widget.more_btn.width(),self.new_arrivals_widget.titellabel.y()+int((self.new_arrivals_widget.titellabel.height()-self.new_arrivals_widget.more_btn.height())/2))
        self.hot_app_widget.move(25, self.new_arrivals_widget.y() + self.new_arrivals_widget.height() + 36)
        self.hot_app_widget.more_btn.move(self.hot_app_widget.width() - self.hot_app_widget.more_btn.width(),
                           self.hot_app_widget.titel_label.y() + int((self.hot_app_widget.titel_label.height() - self.hot_app_widget.more_btn.height()) / 2))
        self.recommended_list_widget.move(20, self.hot_app_widget.y() + self.hot_app_widget.height() + 36)
        self.recommended_android_list_widget.move(20, self.recommended_list_widget.y() + self.recommended_list_widget.height() + 36)
        self.no_app_tip_label.move(self.mywidget.no_app_tip_label.x() + self.mywidget.x() - 15,
                                   int((self.height() - self.no_app_tip_label.height()) / 2))
        self.text_prompt_label.move(int((self.width() - self.text_prompt_label.width()) / 2),
            self.no_app_tip_label.y() + self.no_app_tip_label.height() + 10)
        self.show_homepage_widget2.move(0, 0)
        self.show_homepage_widget2.resize(self.width(), self.height())
        self.search_title_label.move(self.space, 25)
        self.all_search_tip_label.move(self.search_title_label.x() + self.search_title_label.width() + 10,
                                       self.search_title_label.y() + self.search_title_label.height() - self.all_search_tip_label.height() - 5)
        self.search_widget_line.resize(self.show_homepage_widget2.width() - self.space*2, 1)
        self.search_widget_line.move(self.space, self.search_title_label.y() + self.search_title_label.height() + 10)

        self.search_app_widget.resize(self.width() - self.space * 2,self.show_homepage_widget2.height() - self.search_widget_line.y() - self.search_widget_line.height())
        self.search_app_widget.move(0,self.search_widget_line.y()+self.search_widget_line.height()+20)
        self.scrollArea2.setGeometry(self.space, int(self.search_app_widget.y()), self.width() - self.space - 10,
                                     int(self.height() - self.search_widget_line.y() - self.search_widget_line.height() - 30))

        self.show_homepage_widget3.move(self.space, 0)
        self.show_homepage_widget3.resize(self.width() - self.space, self.show_homepage_widget.height())
        self.more_widget_title_label.move(0, 25)
        self.more_widget_line.resize(self.show_homepage_widget3.width() - self.space, 2)
        self.more_widget_line.move(0, int(self.more_widget_title_label.y() + self.more_widget_title_label.height() + 10))

        self.show_more_app_widget.resize(self.width() - self.space * 2,
                                      int(self.show_homepage_widget3.height() - self.more_widget_line.y() - self.more_widget_line.height()))
        self.show_more_app_widget.move(0, int(self.more_widget_line.y() + self.more_widget_line.height() + 20))
        self.scrollArea3.setGeometry(0, int(self.show_more_app_widget.y()), self.width() - self.space - 10,
                                     int(self.height() - self.more_widget_line.y() - self.more_widget_line.height() - 30))


        if(hasattr(self,"category_widget")):
            self.category_widget.resize(self.width(),self.height()-20)
            self.category_widget.re_move()
        if(hasattr(self,"app_detail_widget")):
            # app_detail_value = self.app_detail_widget.scrollArea.verticalScrollBar().value()
            self.app_detail_widget.resize(self.width(),self.height())
            self.app_detail_widget.re_move()
            # self.app_detail_widget.scrollArea.verticalScrollBar().setValue(app_detail_value)
        if(hasattr(self,"search_app_widget")):
            self.search_app_widget.resize(self.width()-self.space*2,int(self.show_homepage_widget2.height() - self.search_widget_line.y() - self.search_widget_line.height()))
            self.reload_card(self.search_app_widget,self.scrollArea2)
        if(hasattr(self,"show_more_app_widget")):
            self.show_more_app_widget.resize(self.width() - 50*2,int(self.show_homepage_widget3.height() - self.more_widget_line.y() - self.more_widget_line.height()))
            self.reload_card(self.show_more_app_widget, self.scrollArea3)
        if(hasattr(self,"drive_widget")):
            self.drive_widget.setFixedSize(self.width(),self.height()-20)
            self.drive_widget.re_move()
        if(self.recommended_android_list_widget.isHidden() == True):
            self.show_homepage_widget.setGeometry(0, 10, self.width(),
                                                  int(self.recommended_list_widget.y() + self.recommended_list_widget.height() + 30))
        else:
            self.show_homepage_widget.resize(self.width(),int(self.recommended_android_list_widget.y() + self.recommended_android_list_widget.height() + 30))

        self.scrollArea_change.emit()


    #
    #处理点击全库搜索
    #
    def slot_cilck_all_search_btn(self):
        Globals.ADVANCED_SEARCH = False
        self.mw.slot_textChanged("",Globals.ADVANCED_SEARCH)

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
                if (type(card) != Click_All_Search):
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
                else:
                    card.move(0,widget.height()+10)
                    widget.resize(widget.width(),widget.height()+card.height())

    def init_connect(self):
        self.hot_app_widget.click_card.connect(self.slot_clicked_card)
        self.ad_widget.click_ad_card.connect(self.slot_clicked_card)
        self.ad_widget.click_none_app_ad_card.connect(self.slot_clicked_none_card)
        self.download_ranking_widget.download_ranking_listWidget.itemClicked.connect(self.slot_click_listitem)
        self.new_arrivals_widget.click_ad_card.connect(self.slot_clicked_card)
        self.scrollArea2.verticalScrollBar().valueChanged.connect(self.mw.slot_scrollbar_valuechange)
        self.mywidget.scrollArea.verticalScrollBar().valueChanged.connect(self.mw.slot_scrollbar_valuechange)
        self.scrollArea3.verticalScrollBar().valueChanged.connect(self.mw.slot_scrollbar_valuechange)
        self.scrollArea_change.connect(self.mw.resize_window_show)
        self.init_cache_signal.connect(self.init_cache)
        self.init_cache_over.connect(self.mw.slot_init_cache_over)
        #self.mywidget.scrollArea1.verticalScrollBar().valueChanged.connect(self.mw.slot_scrollbar_valuechange)

    #
    # 函数：刷新背景颜色
    #
    def refresh_background_color(self):
        # print("******", datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
        if (Globals.THEME_COLOR == 0):
            # print("++++++++", datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
            # palette1 = QPalette()
            # palette1.setColor(self.backgroundRole(), QColor(255, 255, 255))  # 设置背景颜色
            # self.setPalette(palette1)
            # palette1 = QPalette()
            # palette1.setColor(self.backgroundRole(), QColor(255, 255, 255))  # 设置背景颜色
            # self.setPalette(palette1)
            # self.setAutoFillBackground(True)
            # self.show_homepage_widget.setPalette(p)
            if (Globals.RIGHT_ANGLE):
                self.setStyleSheet("QFrame{border:0px;background-color:#ffffff;}")
            else:
                self.setStyleSheet("QFrame{border:0px;border-bottom-left-radius:12px;border-bottom-right-radius:12px; background-color:#ffffff;}")
            self.show_homepage_widget2.setStyleSheet(".QWidget{border-radius: 12px;background-color:#ffffff;}")
            # self.show_homepage_widget2.setStyleSheet(".QWidget{border-radius: 12px;background-color:red;}")
            # self.mywidget.type_name_label.setStyleSheet("QLabel{font-size: 30px;font-weight: 550;color: #303133;line-height: 42px;}")
            self.download_ranking_widget.titel_label.setStyleSheet("QLabel{font-size: 24px;font-weight: 500;color: #303133;line-height: 33px;}")
            self.new_arrivals_widget.titellabel.setStyleSheet("QLabel{width: 94px;height: 33px;font-size: 24px;font-weight: 500;color: #303133;line-height: 33px;}")
            self.hot_app_widget.titel_label.setStyleSheet("QLabel{font-size: 24px;font-weight: 500;color: #303133;line-height: 33px;}")
            self.hot_app_widget.left_btn.setStyleSheet("QPushButton{background-color:transparent;border-radius:30px;border-image: url(./res/ads_left1.png);}QPushButton:hover{border-image: url(./res/ads_left2.png);}QPushButton:pressed{border-image: url(./res/ads_left2.png);}")
            self.hot_app_widget.right_btn.setStyleSheet("QPushButton{background-color:transparent;border-radius:30px;border-image: url(./res/ads_right1.png);}QPushButton:hover{border-image: url(./res/ads_right2.png);}QPushButton:pressed{border-image: url(./res/ads_right2.png);}")
            self.recommended_list_widget.titellabel.setStyleSheet("QLabel{width: 306px;height: 33px;font-size: 24px;font-weight: 500;color: #303133;line-height: 33px;}")
            self.recommended_android_list_widget.titellabel.setStyleSheet("QLabel{width: 306px;height: 33px;font-size: 24px;font-weight: 500;color: #303133;line-height: 33px;}")
            # print("~~~~~~~", datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
            self.search_title_label.setStyleSheet("QLabel{font-size: 24px;font-weight: 600;color: #303133;line-height: 42px;}")
            self.search_widget_line.setStyleSheet("QLabel{background-color:rgba(0,0,0,0.08)}")
            self.text_prompt_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #BBBDCA;line-height: 24px;}")
            self.more_widget_title_label.setStyleSheet("QLabel{font-size: 24px;font-weight: 600;color: #303133;line-height: 42px;}")
            self.more_widget_line.setStyleSheet("QLabel{border: 1px solid #E5E7EE;}")
            self.loadinglabel.setMovie(self.loadingmove)
            self.loadingmove.start()
            # print("^^^^^^^", datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
            self.scrollArea.verticalScrollBar().setStyleSheet("QScrollBar:vertical{margin:0px 0px 0px 0px;background-color:transparent;border:0px;width:6px;border-radius: 3px;}\
                                 QScrollBar::handle:vertical{background-color:#C2C2C2;width:6px;border-radius:3px;} QScrollBar::handle:vertical:hover{background-color:#8F9399;opacity:0.1;width:6px;border-radius:3px;}  QScrollBar::handle:vertical:pressed{background-color:#8F9399;width:6px;border-radius:3px;}\
                                 QScrollBar::sub-line:vertical{subcontrol-origin:margin;border:0px solid red;height:13px}\
                                 QScrollBar::up-arrow:vertical{subcontrol-origin:margin;background-color:rgba(0,0,0,0);height:13px}\
                                 QScrollBar::sub-page:vertical{background-color:transparent;}\
                                 QScrollBar::add-page:vertical{background-color:transparent;}\
                                 QScrollBar::down-arrow:vertical{background-color:rgba(0,0,0,0);}\
                                 QScrollBar::add-line:vertical{subcontrol-origin:margin;border:0px solid green;height:13px}")
            self.scrollArea2.verticalScrollBar().setStyleSheet("QScrollBar:vertical{margin:0px 0px 0px 0px;background-color:transparent;border:0px;width:6px;border-radius: 3px;}\
                                             QScrollBar::handle:vertical{background-color:#C2C2C2;width:6px;border-radius:3px;} QScrollBar::handle:vertical:hover{background-color:#8F9399;opacity:0.1;width:6px;border-radius:3px;}  QScrollBar::handle:vertical:pressed{background-color:#8F9399;width:6px;border-radius:3px;}\
                                             QScrollBar::sub-line:vertical{subcontrol-origin:margin;border:0px solid red;height:13px}\
                                             QScrollBar::up-arrow:vertical{subcontrol-origin:margin;background-color:rgba(0,0,0,0);height:13px}\
                                             QScrollBar::sub-page:vertical{background-color:transparent;}\
                                             QScrollBar::add-page:vertical{background-color:transparent;}\
                                             QScrollBar::down-arrow:vertical{background-color:rgba(0,0,0,0);}\
                                             QScrollBar::add-line:vertical{subcontrol-origin:margin;border:0px solid green;height:13px}")
            # print("-------", datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
            if(hasattr(self,"app_detail_widget")):
                self.app_detail_widget.slot_set_dark_mode()
            if(hasattr(self,"category_widget")):
                # print("11111", datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
                # self.change_category_clolor.emit()
                self.category_widget.init_style()
                # print("22222", datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
            if (hasattr(self, "drive_widget")):
                self.drive_widget.init_style()
            self.mywidget.all_select_btn.setStyleSheet(
                "QPushButton{font-size: 14px;border:0px;font-weight: 500;color: #606265;line-height: 20px;}QPushButton:hover{background: rgba(55, 144, 250, 0.12);color: #3790FA;border-radius: 12px;}PushButton:pressed{background: rgba(55, 144, 250, 0.12);color: #3790FA;border-radius: 12px;}")

        else:
            # palette1 = QPalette()
            # palette1.setColor(self.backgroundRole(), QColor(31, 32, 34))  # 设置背景颜色
            # self.setPalette(palette1)
            # self.setAutoFillBackground(True)
            if (Globals.RIGHT_ANGLE):
                self.setStyleSheet("QFrame{border:0px;background-color:#1F2022;}")
            else:
                self.setStyleSheet("QFrame{border:0px;border-bottom-left-radius:12px;border-bottom-right-radius:12px; background-color:#1F2022;}")
            # print("{}{}{}{}",datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
            self.show_homepage_widget2.setStyleSheet(".QWidget{border-radius: 12px;background-color:#1F2022;}")
            # self.mywidget.type_name_label.setStyleSheet("QLabel{font-size: 30px;font-weight: 550;color: #F9F9F9;line-height: 42px;}")
            self.download_ranking_widget.titel_label.setStyleSheet("QLabel{font-size: 24px;font-weight: 500;color: #F9F9F9;line-height: 33px;}")
            self.new_arrivals_widget.titellabel.setStyleSheet("QLabel{width: 94px;height: 33px;font-size: 24px;font-weight: 500;color: #F9F9F9;line-height: 33px;}")
            self.hot_app_widget.titel_label.setStyleSheet("QLabel{font-size: 24px;font-weight: 500;color: #F9F9F9;line-height: 33px;}")
            self.hot_app_widget.left_btn.setStyleSheet(
                "QPushButton{background-color:transparent;border-radius:30px;border-image: url(./res/ads_left_dark1.png);}QPushButton:hover{border-image: url(./res/ads_left_dark2.png);}QPushButton:pressed{border-image: url(./res/ads_left_dark2.png);}")
            self.hot_app_widget.right_btn.setStyleSheet(
                "QPushButton{background-color:transparent;border-radius:30px;border-image: url(./res/ads_right_dark1.png);}QPushButton:hover{border-image: url(./res/ads_right_dark2.png);}QPushButton:pressed{border-image: url(./res/ads_right_dark2.png);}")
            self.recommended_list_widget.titellabel.setStyleSheet("QLabel{width: 306px;height: 33px;font-size: 24px;font-weight: 500;color: #F9F9F9;line-height: 33px;}")
            self.recommended_android_list_widget.titellabel.setStyleSheet("QLabel{width: 306px;height: 33px;font-size: 24px;font-weight: 500;color: #F9F9F9;line-height: 33px;}")
            self.search_title_label.setStyleSheet("QLabel{font-size: 24px;font-weight: 600;color: #F9F9F9;line-height: 42px;}")
            self.search_widget_line.setStyleSheet("QLabel{background-color:rgba(255,255,255,0.08)}")
            self.text_prompt_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 24px;}")
            self.loadinglabel.setMovie(self.loadingmove_dark)
            self.loadingmove_dark.start()
            self.more_widget_title_label.setStyleSheet("QLabel{font-size: 24px;font-weight: 600;color: #F9F9F9;line-height: 42px;}")
            self.more_widget_line.setStyleSheet("QLabel{border: 1px solid #393939;}")
            self.scrollArea.verticalScrollBar().setStyleSheet("QScrollBar:vertical{margin:0px 0px 0px 0px;background-color:transparent;border:0px;width:6px;border-radius: 3px;}\
                                 QScrollBar::handle:vertical{background-color:#6B6B6B;width:6px;border-radius:3px;} QScrollBar::handle:vertical:hover{background-color:#959595;opacity:0.1;width:6px;border-radius:3px;}  QScrollBar::handle:vertical:pressed{background-color:#959595;width:6px;border-radius:3px;}\
                                 QScrollBar::sub-line:vertical{subcontrol-origin:margin;border:0px solid red;height:13px}\
                                 QScrollBar::up-arrow:vertical{subcontrol-origin:margin;background-color:rgba(0,0,0,0);height:13px}\
                                 QScrollBar::sub-page:vertical{background-color:transparent;}\
                                 QScrollBar::add-page:vertical{background-color:transparent;}\
                                 QScrollBar::down-arrow:vertical{background-color:rgba(0,0,0,0);}\
                                 QScrollBar::add-line:vertical{subcontrol-origin:margin;border:0px solid green;height:13px}")
            self.scrollArea2.verticalScrollBar().setStyleSheet("QScrollBar:vertical{margin:0px 0px 0px 0px;background-color:transparent;border:0px;width:6px;border-radius: 3px;}\
                                                         QScrollBar::handle:vertical{background-color:#6B6B6B;width:6px;border-radius:3px;} QScrollBar::handle:vertical:hover{background-color:#959595;opacity:0.1;width:6px;border-radius:3px;}  QScrollBar::handle:vertical:pressed{background-color:#959595;width:6px;border-radius:3px;}\
                                                         QScrollBar::sub-line:vertical{subcontrol-origin:margin;border:0px solid red;height:13px}\
                                                         QScrollBar::up-arrow:vertical{subcontrol-origin:margin;background-color:rgba(0,0,0,0);height:13px}\
                                                         QScrollBar::sub-page:vertical{background-color:transparent;}\
                                                         QScrollBar::add-page:vertical{background-color:transparent;}\
                                                         QScrollBar::down-arrow:vertical{background-color:rgba(0,0,0,0);}\
                                                         QScrollBar::add-line:vertical{subcontrol-origin:margin;border:0px solid green;height:13px}")
            if (hasattr(self, "app_detail_widget")):
                self.app_detail_widget.slot_set_dark_mode()
            if(hasattr(self,"category_widget")):
                # print("44444", datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
                self.category_widget.init_style()
                # print("33333", datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
            if (hasattr(self, "drive_widget")):
                self.drive_widget.init_style()
            # self.mywidget.all_select_btn.setStyleSheet("QPushButton{border:0px;font-weight: 500;color: #F9F9F9;line-height: 20px;}QPushButton:hover{background: rgba(55, 144, 250, 0.12);color: #3790FA;border-radius: 12px;}PushButton:pressed{background: rgba(55, 144, 250, 0.12);color: #3790FA;border-radius: 12px;}")
        if (hasattr(self, "kydroid_widget")):
            self.kydroid_widget.change_color()
        self.mywidget.set_show_dark_mode()
        self.mywidget.downloading_btn.paintbuttonevent()
        self.mywidget.update_btn.paintbuttonevent()
        self.mywidget.uninstall_btn.paintbuttonevent()
        self.mywidget.installed_btn.paintbuttonevent()

    #
    #函数：通过软件名在全局变量中获取软件信息
    #
    def get_app_msg_py_appname(self,app_msg):
        for i in Globals.APPLICATION_LIST:
            if (i["name"] == app_msg.pkgname):
                return i["appmsg"]
        return None

    #
    #函数：处理点击卡片界面
    #
    def slot_clicked_card(self,app_msg,need_send_exposure_rate=False,click_card_type=ClickCard.INSIDECARD):
        self.mw.title_bar.exit_login_btn.hide()
        self.mw.title_bar.senior_search.hide()
        if (hasattr(self.mw, "title_bar")):
            self.mw.title_bar.search_line.setFocus(False)
            self.mw.title_bar.search_line.clearFocus()
        if (hasattr(self.mw, "search_tip_widget")):
            self.mw.search_tip_widget.hide()
        if (hasattr(self.mw, "menubar")):
            self.mw.menubar.hide()
        if (hasattr(self.mw, "second_lavel_menubar")):
            self.mw.second_lavel_menubar.hide()
        self.mw.title_bar.search_line.clear()
        self.text_prompt_label.hide()
        self.no_app_tip_label.hide()
        if (app_msg != None):
            # try:
            get_app_msg = self.get_app_msg_py_appname(app_msg)
            if(get_app_msg == None):
                get_app_msg = app_msg
            if(self.first_click_card == True):
                self.app_detail_widget = App_Detail_Widget(self.width(),self.height(),get_app_msg, self.ca,need_send_exposure_rate,click_card_type,self)
                self.app_detail_widget.check_all_check_statue.connect(self.mw.slot_all_puase_btn_state)
                self.app_detail_widget.puase_apk_download.connect(self.mw.slot_puase_download_pkg)
                self.app_detail_widget.puase_download.connect(self.mw.slot_puase_download_pkg)
                self.mw.change_kydroid_card_btn_state.connect(self.app_detail_widget.change_kydroid_btn_statue)

                self.mw.at.appmgr.submit_rating_over.connect(self.mw.detailwidget.app_detail_widget.slot_submit_my_rating_over)
                self.mw.download_count_change.connect(self.app_detail_widget.slot_change_download_count)
                self.mw.change_card_state.connect(self.app_detail_widget.change_btn_state)
                self.mw.send_download_progress.connect(self.app_detail_widget.slot_get_download_progress)
                self.mw.signal_reload_btn.connect(self.app_detail_widget.reload_btn)
                self.mw.signal_uninstall.connect(self.app_detail_widget.slot_uninstall)
                page = {"pagetype": PageStates.APPDETAIL, "app_msg": get_app_msg,"now_page":Globals.NOWPAGE}
                if (len(Globals.RETURN_PATH_LIST) >= 10):
                    Globals.RETURN_PATH_LIST.pop(0)
                if (len(Globals.RETURN_PATH_LIST) >= 1 and Globals.RETURN_PATH_LIST[len(Globals.RETURN_PATH_LIST) - 1] != page):
                    Globals.RETURN_PATH_LIST.append(page)
                self.first_click_card = False
            else:
                page = {"pagetype": PageStates.APPDETAIL, "app_msg": get_app_msg,"now_page":Globals.NOWPAGE}
                if (len(Globals.RETURN_PATH_LIST) >= 10):
                    Globals.RETURN_PATH_LIST.pop(0)
                if (len(Globals.RETURN_PATH_LIST) >= 1 and Globals.RETURN_PATH_LIST[len(Globals.RETURN_PATH_LIST) - 1] != page):
                    Globals.RETURN_PATH_LIST.append(page)
                #self.app_detail_widget.app_statue = app_statue
                self.app_detail_widget.need_send_exposure_rate = need_send_exposure_rate
                self.app_detail_widget.click_card_type = click_card_type
                self.app_detail_widget.app_msg = get_app_msg
                self.app_detail_widget.pre_progress = get_app_msg.percent
                self.app_detail_widget.ratings_widget.MY_SCORE = 0
                self.app_detail_widget.app_msg_change = True
                self.app_detail_widget.set_stylesheet()
                self.app_detail_widget.down_progress.init_down = True
                self.app_detail_widget.down_progress.update_progress(get_app_msg.percent)
                self.app_detail_widget.down_progress.update()
            self.app_detail_widget.change_btn_state(get_app_msg.pkgname,get_app_msg.status)
            try:
                self.app_detail_widget.btn.clicked.disconnect()
            except:
                pass
            try:
                self.app_detail_widget.kmre_btn.clicked.disconnect()
            except:
                pass
            self.app_detail_widget.btn.clicked.connect(lambda :self.mw.slot_click_action_btn(self.app_detail_widget,self.app_detail_widget.btn.text(),self.app_detail_widget.app_msg))
            self.app_detail_widget.kmre_btn.clicked.connect(lambda :self.mw.slot_click_action_btn(self.app_detail_widget,self.app_detail_widget.kmre_btn.text(),self.app_detail_widget.app_msg))
            self.app_detail_widget.scrollArea.verticalScrollBar().setValue(0)
            self.app_detail_widget.show()
            self.app_detail_widget.raise_()
            if (self.app_detail_widget in self.mw.path_of_particle):
                self.mw.path_of_particle.remove(self.app_detail_widget)
                self.mw.path_of_particle.append(self.app_detail_widget)
            else:
                self.mw.path_of_particle.append(self.app_detail_widget)
                #self.show_homepage_widget.hide()
        # except:
        #     self.tip_error.emit(_("Software source is incomplete or does not contain the software"))
        #     self.mw.tip_widget.btn.clicked.connect(self.mw.slot_close_tipwidget)
        # else:
        #     self.mw.show_tip_widget(_("Native architecture does not support, please wait!"))
        #     self.mw.tip_widget.btn.clicked.connect(self.mw.slot_close_tipwidget)


    #
    #函数：处理显示我的界面
    #
    def show_my_widget(self):
        self.mw.title_bar.exit_login_btn.hide()
        self.mw.title_bar.senior_search.hide()
        if (hasattr(self.mw, "title_bar")):
            self.mw.title_bar.search_line.setFocus(False)
            self.mw.title_bar.search_line.clearFocus()
        if (hasattr(self.mw, "search_tip_widget")):
            self.mw.search_tip_widget.hide()
        if (hasattr(self.mw, "menubar")):
            self.mw.menubar.hide()
        if (hasattr(self.mw, "second_lavel_menubar")):
            self.mw.second_lavel_menubar.hide()
        self.mw.title_bar.search_line.clear()
        self.text_prompt_label.hide()
        self.no_app_tip_label.hide()
        self.mywidget.type_name_label.show()
        self.mywidget.loadinglabel.hide()
        if (self.mywidget.type_name_label.text() == _("Onload")):
            self.mywidget.all_check.hide()
            self.mw.title_bar.search_line_back.setText(_("Please enter onload appname"))
            self.mw.title_bar.search_line_back.adjustSize()
            Globals.NOWPAGE = PageStates.DOWNLOADINGPAGE
            Globals.SEARCH_NOWPAGE = PageStates.DOWNLOADINGPAGE
            if(len(self.mywidget.downloading_widget.children()) == 0 ):
                self.mywidget.download_all_check.setEnabled(False)
                self.mywidget.download_all_check.setStyleSheet("QPushButton{font-size:14px;font-weight: 500;color: #8f9399;line-height: 20px;border:0px solid #ff0000;}")
                if(Globals.THEME_COLOR == 0):
                    self.mywidget.no_app_tip_label.setStyleSheet("QLabel{border-radius: 12px;background-image:url(./res/no_download.png);}")
                else:
                    self.mywidget.no_app_tip_label.setStyleSheet("QLabel{border-radius: 12px;background-image:url(./res/no_download_dark.png);}")
                self.mywidget.text_prompt_label.setText(_("No software downloading!"))
                self.mywidget.text_prompt_label.adjustSize()
                self.mywidget.no_app_tip_label.move(int((self.mywidget.width() - self.mywidget.no_app_tip_label.width()) / 2),
                    int((self.mywidget.height() - self.mywidget.no_app_tip_label.height() - self.mywidget.text_prompt_label.height() + 10) / 2))
                self.mywidget.text_prompt_label.move(int((self.mywidget.width() - self.mywidget.text_prompt_label.width()) / 2),
                    self.mywidget.no_app_tip_label.y() + self.mywidget.no_app_tip_label.height() + 10)
                self.mywidget.text_prompt_label.show()
                self.mywidget.download_all_check.hide()
                self.mywidget.no_app_tip_label.show()
                self.mywidget.scrollArea1.hide()
            else:
                self.mywidget.download_all_check.setEnabled(True)
                self.mywidget.download_all_check.setStyleSheet("QPushButton{font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;border:0px solid #ff0000;}QPushButton:hover{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}QPushButton:pressed{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}")
                self.mywidget.text_prompt_label.hide()
                self.mywidget.no_app_tip_label.hide()
                self.mywidget.download_all_check.show()
                self.mywidget.downloading_widget.show()
                self.mywidget.border.show()
                self.mywidget.scrollArea1.show()
                self.mywidget.app_widget.hide()
                self.mywidget.scrollArea.hide()
        else:
            self.mywidget.all_check.show()
            self.mywidget.download_all_check.hide()
            # self.mywidget.no_app_tip_label.hide()
            # self.mywidget.text_prompt_label.hide()
            self.mw.app_list_msg.clear()
            if (self.mywidget.type_name_label.text() == _("Uninstall")):
                self.mw.title_bar.search_line_back.setText(_("Please enter uninstall appname"))
                self.mw.title_bar.search_line_back.adjustSize()
                Globals.NOWPAGE = PageStates.MYPAGE
                Globals.SEARCH_NOWPAGE = PageStates.MYPAGE
                self.mw.is_remove = True
                self.mw.slot_clicked_my_widget_btn(_("Uninstall"))
            elif (self.mywidget.type_name_label.text() == _("Updates")):
                self.mw.title_bar.search_line_back.setText(_("Please enter update appname"))
                self.mw.title_bar.search_line_back.adjustSize()
                Globals.NOWPAGE = PageStates.MYPAGE
                Globals.SEARCH_NOWPAGE = PageStates.MYPAGE
                self.mw.slot_clicked_my_widget_btn(_("Updates"))
            elif (self.mywidget.type_name_label.text() == _("Installation")):
                self.mw.title_bar.search_line_back.setText(_("Please enter installation appname"))
                # self.mw.title_bar.search_line_back.adjustSize()
                Globals.NOWPAGE = PageStates.HISTORY
                Globals.SEARCH_NOWPAGE = PageStates.HISTORY
                # self.mw.slot_clicked_my_widget_btn(_("History"))
                self.mw.app_list_msg.extend(Globals.FRIST_LEVEL_BTN_SCROLLAREA["MY"])
        self.mywidget.show()
        if(hasattr(self,"category_widget")):
            if (self.category_widget in self.mw.path_of_particle):
                self.mw.path_of_particle.remove(self.category_widget)
            self.category_widget.hide()
        if(self.show_homepage_widget2 in self.mw.path_of_particle):
            self.mw.path_of_particle.remove(self.show_homepage_widget2)
            self.show_homepage_widget2.hide()
            self.scrollArea2.hide()
        if (hasattr(self,"app_detail_widget")):
            if(self.app_detail_widget in self.mw.path_of_particle):
                self.mw.path_of_particle.remove(self.app_detail_widget)
            self.app_detail_widget.hide()
        if (hasattr(self, "kydroid_widget")):
            if (self.kydroid_widget in self.mw.path_of_particle):
                self.mw.path_of_particle.remove(self.kydroid_widget)
            self.kydroid_widget.hide()
        if(self.mywidget in self.mw.path_of_particle):
            self.mw.path_of_particle.remove(self.mywidget)
            self.mw.path_of_particle.append(self.mywidget)
        else:
            self.mw.path_of_particle.append(self.mywidget)


    def slot_check_action_btn_state(self,appname):
        if (Globals.NOWPAGE == PageStates.HISTORY):
            cards = self.mywidget.history_widget.app_card_widget.children()
        else:
            cards = self.mywidget.app_widget.children()
        all_check = True
        has_check = False
        for card in cards:
            if(card.app.pkgname == appname):
                card.check_btn.hide()
                card.check_btn.setChecked(False)
                card.check_btn.setEnabled(False)
            if (card.check_btn.isChecked()):
                has_check = True
        self.mywidget.all_check.setEnabled(True)
        self.mywidget.all_select_btn.setEnabled(True)
        if(has_check == False):
            self.mywidget.all_check.setEnabled(False)
            self.mywidget.all_check.setStyleSheet("QPushButton{font-size:14px;font-weight: 500;color: #8f9399;line-height: 20px;border:0px solid #ff0000;}")
        else:
            self.mywidget.all_check.setEnabled(True)
            if (self.mywidget.type_name_label.text() == _("Uninstall")):
                self.mywidget.all_check.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 500;color: #F56C6C;line-height: 20px;border:0px solid #ff0000;}QPushButton:hover{background: rgba(245, 108, 108, 0.12);border-radius: 12px;}QPushButton:pressed{background: rgba(245, 108, 108, 0.12);border-radius: 12px;}")
            else:
                self.mywidget.all_check.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;border:0px solid #ff0000;}QPushButton:hover{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}QPushButton:pressed{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}")

    #
    #函数：处理显示分类界面
    #
    def show_category_widget(self):
        self.mw.title_bar.exit_login_btn.hide()
        self.mw.title_bar.senior_search.hide()
        if (hasattr(self.mw, "title_bar")):
            self.mw.title_bar.search_line.setFocus(False)
            self.mw.title_bar.search_line.clearFocus()
        if (hasattr(self.mw, "search_tip_widget")):
            self.mw.search_tip_widget.hide()
        if (hasattr(self.mw, "menubar")):
            self.mw.menubar.hide()
        if (hasattr(self.mw, "second_lavel_menubar")):
            self.mw.second_lavel_menubar.hide()
        self.mw.is_remove = False
        self.mw.title_bar.search_line.clear()
        self.text_prompt_label.hide()
        self.no_app_tip_label.hide()
        if (not hasattr(self,"category_widget")):
            self.category_widget = Category_Widget(self.width(),self.height(),self)
            self.category_widget.btn_widget.click_categoy.connect(self.slot_click_type_btn)
            self.category_widget.click_type_btn.connect(self.slot_click_type_btn)
            self.category_widget.click_category_ad.connect(self.slot_clicked_card)
            self.category_widget.click_category_ad_apk.connect(self.slot_clicked_card)
            self.category_widget.click_none_card.connect(self.slot_clicked_none_card)
            self.category_widget.one_click_download.clicked.connect(self.mw.slot_category_one_click_download)
            self.category_widget.all_check_btn.clicked.connect(lambda :self.mw.slot_category_all_select(self.category_widget))
            self.category_widget.scrollArea.verticalScrollBar().valueChanged.connect(self.mw.slot_scrollbar_valuechange)
            # self.change_category_clolor.connect(self.category_widget.init_style)
            self.category_widget.show()
            self.first_show_category = False
            Globals.NOW_CATEGORY = self.category_widget.btn_widget.first_btn
            self.slot_click_type_btn(self.category_widget.btn_widget.first_btn)
        else:
            Globals.NOWPAGE = PageStates.CAREGORYPAGE
            Globals.SEARCH_NOWPAGE = PageStates.CAREGORYPAGE
            # self.mw.app_list_msg = Globals.FRIST_LEVEL_BTN_SCROLLAREA["CATEGORY"]
            # self.category_widget.show()
            self.slot_click_type_btn(self.category_widget.type_name_label.text(),True)

        if (hasattr(self.category_widget, "select_widget")):
            self.category_widget.select_widget.hide()
        if (self.mywidget in self.mw.path_of_particle):
            self.mw.path_of_particle.remove(self.mywidget)
            self.mywidget.hide()
        if(hasattr(self,"app_detail_widget")):
            if (self.app_detail_widget in self.mw.path_of_particle):
                self.mw.path_of_particle.remove(self.app_detail_widget)
            self.app_detail_widget.hide()
        if(hasattr(self,"kydroid_widget")):
            if (self.kydroid_widget in self.mw.path_of_particle):
                self.mw.path_of_particle.remove(self.kydroid_widget)
            self.kydroid_widget.hide()
        if (self.show_homepage_widget2 in self.mw.path_of_particle):
            self.mw.path_of_particle.remove(self.show_homepage_widget2)
            self.show_homepage_widget2.hide()
            self.scrollArea2.hide()
        if(self.category_widget in self.mw.path_of_particle):
            self.mw.path_of_particle.remove(self.category_widget)
            self.mw.path_of_particle.append(self.category_widget)
        else:
            self.mw.path_of_particle.append(self.category_widget)


    #
    #函数：点击分类界面的类型按钮
    #
    def slot_click_type_btn(self,btn_name,need_all_flesh = True):
        self.category_widget.show()
        Globals.NOW_CATEGORY = btn_name
        page = {"pagetype": PageStates.CAREGORYPAGE, "Type": self.category_widget.type_name_label.text(),"sort_way": Globals.SORTING_METHOD}

        if (len(Globals.RETURN_PATH_LIST) >= 10):
            Globals.RETURN_PATH_LIST.pop(0)
        if (len(Globals.RETURN_PATH_LIST) >= 1 and Globals.RETURN_PATH_LIST[len(Globals.RETURN_PATH_LIST) - 1] != page):
            Globals.RETURN_PATH_LIST.append(page)
        # if(btn_name in Globals.CATEGORIES_AD_LIST.keys()):
        #     self.category_widget.loadinglabel.hide()
        # else:
        #      self.category_widget.loadinglabel.show()
        self.category_widget.show()
        if(need_all_flesh == True):
            self.category_widget.check_choose_btn_type()
            self.category_widget.scrollArea.hide()
            self.category_widget.type_widget.hide()
        else:
            self.category_widget.type_show_card_widget.hide()
        self.mw.title_bar.exit_login_btn.hide()
        self.mw.title_bar.senior_search.hide()
        for i in Globals.ALL_CATEGORIES:
            if(i["displayname"] == btn_name or i["name"] == btn_name):
                if(str(i["id"]) == "100"):
                    self.category_widget.category_ad_card.hide()
                    self.category_widget.preview_ad_card.show()
                    self.category_widget.all_check_btn.hide()
                    self.category_widget.choose_sorting_type_btn.hide()
                    self.category_widget.one_click_download.hide()
                else:
                    self.category_widget.category_ad_card.show()
                    self.category_widget.preview_ad_card.hide()
                    self.category_widget.all_check_btn.show()
                    self.category_widget.choose_sorting_type_btn.show()
                    self.category_widget.one_click_download.show()
                break
        if (Globals.SORTING_METHOD == 0):
            if(Globals.THEME_COLOR == 0):
                self.category_widget.download_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;background: rgba(55, 144, 250, 0.12);border-radius: 13px;}")
                self.category_widget.rating_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 400;color: #303133;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
                self.category_widget.popularity_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 400;color: #303133;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
            else:
                self.category_widget.download_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;background: rgba(55, 144, 250, 0.12);border-radius: 13px;}")
                self.category_widget.rating_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
                self.category_widget.popularity_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
        elif(Globals.SORTING_METHOD == 1):
            if(Globals.THEME_COLOR == 0):
                self.category_widget.rating_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;background: rgba(55, 144, 250, 0.12);border-radius: 13px;}")
                self.category_widget.download_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 400;color: #303133;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
                self.category_widget.popularity_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 400;color: #303133;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
            else:
                self.category_widget.rating_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;background: rgba(55, 144, 250, 0.12);border-radius: 13px;}")
                self.category_widget.download_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
                self.category_widget.popularity_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
        else:
            if (Globals.THEME_COLOR == 0):
                self.category_widget.popularity_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;background: rgba(55, 144, 250, 0.12);border-radius: 13px;}")
                self.category_widget.download_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 400;color: #303133;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
                self.category_widget.rating_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 400;color: #303133;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
            else:
                self.category_widget.popularity_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;background: rgba(55, 144, 250, 0.12);border-radius: 13px;}")
                self.category_widget.download_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
                self.category_widget.rating_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
        if (hasattr(self.mw, "title_bar")):
            self.mw.title_bar.search_line.setFocus(False)
            self.mw.title_bar.search_line.clearFocus()
        if (hasattr(self.mw, "search_tip_widget")):
            self.mw.search_tip_widget.hide()
        if (hasattr(self.mw, "menubar")):
            self.mw.menubar.hide()
        if (hasattr(self.mw, "second_lavel_menubar")):
            self.mw.second_lavel_menubar.hide()
        if (hasattr(self.category_widget, "select_widget")):
            self.category_widget.select_widget.hide()
        self.mw.title_bar.search_line.clear()
        self.text_prompt_label.hide()
        self.no_app_tip_label.hide()
        applist = []
        win_list = []
        item = {"action":"","need_all_flesh":"","type":"switch_classification"}
        item["action"] = btn_name
        item["need_all_flesh"] = need_all_flesh
        self.mw.at.work_list.append(item)


    #
    #函数：处理显示驱动界面
    #
    def show_drive_widget(self):
        self.mw.title_bar.exit_login_btn.hide()
        self.mw.title_bar.senior_search.hide()
        if (hasattr(self.mw, "title_bar")):
            self.mw.title_bar.search_line.setFocus(False)
            self.mw.title_bar.search_line.clearFocus()
        if (hasattr(self.mw, "search_tip_widget")):
            self.mw.search_tip_widget.hide()
        if (hasattr(self.mw, "menubar")):
            self.mw.menubar.hide()
        if (hasattr(self.mw, "second_lavel_menubar")):
            self.mw.second_lavel_menubar.hide()
        self.mw.is_remove = False
        self.mw.title_bar.search_line.clear()
        self.text_prompt_label.hide()
        self.no_app_tip_label.hide()
        if (not hasattr(self, "drive_widget")):
            self.drive_widget = Drive_Widget(self.width(), self.height(), self)
            self.drive_widget.btn_widget.click_categoy.connect(self.slot_drive_type_click)
            self.drive_widget.click_type_btn.connect(self.slot_drive_type_click)
            self.drive_widget.scrollArea.verticalScrollBar().valueChanged.connect(self.mw.slot_scrollbar_valuechange)
            self.drive_widget.one_click_download.clicked.connect(self.mw.slot_category_one_click_download)
            self.drive_widget.all_check_btn.clicked.connect(lambda: self.mw.slot_category_all_select(self.drive_widget))
            self.drive_widget.show()
            self.slot_drive_type_click(self.drive_widget.btn_widget.first_btn)
        else:
            Globals.NOWPAGE = PageStates.DRIVE
            Globals.SEARCH_NOWPAGE = PageStates.DRIVE
            # self.mw.app_list_msg = Globals.FRIST_LEVEL_BTN_SCROLLAREA["CATEGORY"]
            # self.category_widget.show()
            self.slot_drive_type_click(self.drive_widget.type_name_label.text())

        if (self.mywidget in self.mw.path_of_particle):
            self.mw.path_of_particle.remove(self.mywidget)
            self.mywidget.hide()
        if (hasattr(self, "app_detail_widget")):
            if (self.app_detail_widget in self.mw.path_of_particle):
                self.mw.path_of_particle.remove(self.app_detail_widget)
            self.app_detail_widget.hide()
        if (hasattr(self, "kydroid_widget")):
            if (self.kydroid_widget in self.mw.path_of_particle):
                self.mw.path_of_particle.remove(self.kydroid_widget)
            self.kydroid_widget.hide()
        if (hasattr(self, "category_widget")):
            if (self.category_widget in self.mw.path_of_particle):
                self.mw.path_of_particle.remove(self.category_widget)
            self.category_widget.hide()
        if (self.show_homepage_widget2 in self.mw.path_of_particle):
            self.mw.path_of_particle.remove(self.show_homepage_widget2)
            self.show_homepage_widget2.hide()
            self.scrollArea2.hide()

    #
    # 函数：点击驱动界面的类型按钮
    #
    def slot_drive_type_click(self, btn_name):
        self.drive_widget.show()
        page = {"pagetype": PageStates.DRIVE, "Type": self.drive_widget.type_name_label.text()}
        if (len(Globals.RETURN_PATH_LIST) >= 10):
            Globals.RETURN_PATH_LIST.pop(0)
        if (len(Globals.RETURN_PATH_LIST) >= 1 and Globals.RETURN_PATH_LIST[len(Globals.RETURN_PATH_LIST) - 1] != page):
            Globals.RETURN_PATH_LIST.append(page)
        if(btn_name in Globals.DRIVE_CATEGORIES_APP_LIST.keys()):
            self.drive_widget.loadinglabel.hide()
        else:
            self.drive_widget.loadinglabel.show()
        self.drive_widget.show()
        self.drive_widget.scrollArea.hide()
        self.drive_widget.type_widget.hide()
        self.mw.title_bar.exit_login_btn.hide()
        self.mw.title_bar.senior_search.hide()
        if (hasattr(self.mw, "title_bar")):
            self.mw.title_bar.search_line.setFocus(False)
            self.mw.title_bar.search_line.clearFocus()
        if (hasattr(self.mw, "search_tip_widget")):
            self.mw.search_tip_widget.hide()
        if (hasattr(self.mw, "menubar")):
            self.mw.menubar.hide()
        if (hasattr(self.mw, "second_lavel_menubar")):
            self.mw.second_lavel_menubar.hide()
        self.mw.title_bar.search_line.clear()
        self.text_prompt_label.hide()
        self.no_app_tip_label.hide()
        applist = []
        win_list = []
        item = {}
        item["action"] = "get_drive"
        item["btn_name"] = btn_name
        item["need_all_flesh"] = True
        self.mw.at.work_list.append(item)

    #
    # 函数：点击安卓界面的类型按钮
    #
    def slot_click_kydroid_type_btn(self, btn_name, need_all_flesh=True):
        self.kydroid_widget.all_check_btn.setText(_("Select all"))
        self.kydroid_widget.one_click_download.setText(_("all down"))
        self.kydroid_widget.one_click_download.setStyleSheet("QPushButton{font-size:14px;font-weight: 500;color: #8f9399;line-height: 20px;border:0px solid #ff0000;}")
        self.kydroid_widget.one_click_download.setEnabled(False)
        self.loadinglabel.hide()
        self.loading_kydroid_evn.hide()
        self.kydroid_widget.show()
        page = {"pagetype": PageStates.KYDROID, "Type": self.kydroid_widget.type_name_label.text(),
                "sort_way": Globals.KYDROID_SORTING_METHOD}
        if (len(Globals.RETURN_PATH_LIST) >= 10):
            Globals.RETURN_PATH_LIST.pop(0)
        if (len(Globals.RETURN_PATH_LIST) >= 1 and Globals.RETURN_PATH_LIST[len(Globals.RETURN_PATH_LIST) - 1] != page):
            Globals.RETURN_PATH_LIST.append(page)
        # self.kydroid_widget.loadinglabel.show()
        self.kydroid_widget.show()
        if (need_all_flesh == True):
            self.kydroid_widget.check_choose_btn_type()
            self.kydroid_widget.scrollArea.hide()
            self.kydroid_widget.type_widget.hide()
        else:
            self.kydroid_widget.app_widget.hide()
        self.mw.title_bar.exit_login_btn.hide()
        self.mw.title_bar.senior_search.hide()
        if (Globals.KYDROID_SORTING_METHOD == 0):
            if (Globals.THEME_COLOR == 0):
                self.kydroid_widget.download_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;background: rgba(55, 144, 250, 0.12);border-radius: 13px;}")
                self.kydroid_widget.rating_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 400;color: #303133;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
                self.kydroid_widget.popularity_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 400;color: #303133;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
            else:
                self.kydroid_widget.download_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;background: rgba(55, 144, 250, 0.12);border-radius: 13px;}")
                self.kydroid_widget.rating_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
                self.kydroid_widget.popularity_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
        elif (Globals.KYDROID_SORTING_METHOD == 1):
            if (Globals.THEME_COLOR == 0):
                self.kydroid_widget.rating_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;background: rgba(55, 144, 250, 0.12);border-radius: 13px;}")
                self.kydroid_widget.download_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 400;color: #303133;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
                self.kydroid_widget.popularity_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 400;color: #303133;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
            else:
                self.kydroid_widget.rating_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;background: rgba(55, 144, 250, 0.12);border-radius: 13px;}")
                self.kydroid_widget.download_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
                self.kydroid_widget.popularity_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
        else:
            if (Globals.THEME_COLOR == 0):
                self.kydroid_widget.popularity_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;background: rgba(55, 144, 250, 0.12);border-radius: 13px;}")
                self.kydroid_widget.download_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 400;color: #303133;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
                self.kydroid_widget.rating_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 400;color: #303133;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
            else:
                self.kydroid_widget.popularity_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;background: rgba(55, 144, 250, 0.12);border-radius: 13px;}")
                self.kydroid_widget.download_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
                self.kydroid_widget.rating_sorting_btn.setStyleSheet(
                    "QPushButton{font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 20px;background: transparent;border-radius: 13px;}QPushButton::hover{color: #3790FA;background: transparent;font-weight: 400;}QPushButton:pressed{color: #3790FA;background: rgba(55, 144, 250, 0.12);font-weight: 500;}")
        if (hasattr(self.mw, "title_bar")):
            self.mw.title_bar.search_line.setFocus(False)
            self.mw.title_bar.search_line.clearFocus()
        if (hasattr(self.mw, "search_tip_widget")):
            self.mw.search_tip_widget.hide()
        if (hasattr(self.mw, "menubar")):
            self.mw.menubar.hide()
        if (hasattr(self.mw, "second_lavel_menubar")):
            self.mw.second_lavel_menubar.hide()
        if (hasattr(self.kydroid_widget, "select_widget")):
            self.kydroid_widget.select_widget.hide()
        self.mw.title_bar.search_line.clear()
        self.text_prompt_label.hide()
        self.no_app_tip_label.hide()
        # applist = []
        # win_list = []
        item = {"action": "", "need_all_flash": ""}
        item["action"] = "get_kydroid_category_applist"
        item["category"] = btn_name
        item["need_all_flesh"] = need_all_flesh
        self.mw.at.work_list.append(item)

    #
    #函数：点击下载界面的listitem
    #
    def slot_click_listitem(self,item):
        self.mw.title_bar.exit_login_btn.hide()
        self.mw.title_bar.senior_search.hide()
        if (hasattr(self.mw, "title_bar")):
            self.mw.title_bar.search_line.setFocus(False)
            self.mw.title_bar.search_line.clearFocus()
        if (hasattr(self.mw, "search_tip_widget")):
            self.mw.search_tip_widget.hide()
        if (hasattr(self.mw, "menubar")):
            self.mw.menubar.hide()
        if (hasattr(self.mw, "second_lavel_menubar")):
            self.mw.second_lavel_menubar.hide()
        self.mw.title_bar.search_line.clear()
        self.text_prompt_label.hide()
        self.no_app_tip_label.hide()
        get_item_widget = self.download_ranking_widget.download_ranking_listWidget.itemWidget(item)
        app_msg = get_item_widget.appmsg
        try:
            get_app_msg = self.get_app_msg_py_appname(app_msg)
            if(get_app_msg == None):
                get_app_msg = app_msg
            if (self.first_click_card == True):
                self.app_detail_widget = App_Detail_Widget(self.width(),self.height(),get_app_msg, self.ca,False,ClickCard.INSIDECARD,self)
                self.mw.at.appmgr.submit_rating_over.connect(self.mw.detailwidget.app_detail_widget.slot_submit_my_rating_over)
                self.app_detail_widget.check_all_check_statue.connect(self.mw.slot_all_puase_btn_state)
                self.app_detail_widget.puase_apk_download.connect(self.mw.slot_puase_download_pkg)
                self.app_detail_widget.puase_download.connect(self.mw.slot_puase_download_pkg)
                self.mw.change_kydroid_card_btn_state.connect(self.app_detail_widget.change_kydroid_btn_statue)
                self.mw.change_card_state.connect(self.app_detail_widget.change_btn_state)
                self.mw.send_download_progress.connect(self.app_detail_widget.slot_get_download_progress)
                self.mw.signal_reload_btn.connect(self.app_detail_widget.reload_btn)
                self.mw.signal_uninstall.connect(self.app_detail_widget.slot_uninstall)
                page = {"pagetype": PageStates.APPDETAIL,"app_msg": get_app_msg,"now_page":Globals.NOWPAGE}
                if (len(Globals.RETURN_PATH_LIST) >= 10):
                    Globals.RETURN_PATH_LIST.pop(0)
                if (len(Globals.RETURN_PATH_LIST) >= 1 and Globals.RETURN_PATH_LIST[len(Globals.RETURN_PATH_LIST) - 1] != page):
                    Globals.RETURN_PATH_LIST.append(page)
                self.first_click_card = False
            else:
                page = {"pagetype": PageStates.APPDETAIL, "app_msg": get_app_msg,"now_page":Globals.NOWPAGE}
                if (len(Globals.RETURN_PATH_LIST) >= 10):
                    Globals.RETURN_PATH_LIST.pop(0)
                if (len(Globals.RETURN_PATH_LIST) >= 1 and Globals.RETURN_PATH_LIST[len(Globals.RETURN_PATH_LIST) - 1] != page):
                    Globals.RETURN_PATH_LIST.append(page)
                self.app_detail_widget.app_msg = get_app_msg
                self.app_detail_widget.pre_progress = get_app_msg.percent
                self.app_detail_widget.ratings_widget.MY_SCORE = 0
                self.app_detail_widget.app_msg_change = True
                self.app_detail_widget.down_progress.init_down = True
                self.app_detail_widget.down_progress.GenUpdate(get_app_msg.percent)
                self.app_detail_widget.down_progress.update()
                self.app_detail_widget.set_stylesheet()
            self.app_detail_widget.change_btn_state(get_app_msg.pkgname, get_app_msg.status)
            try:
                self.app_detail_widget.btn.clicked.disconnect()
            except:
                pass
            self.app_detail_widget.btn.clicked.connect(lambda: self.mw.slot_click_action_btn(self.app_detail_widget, self.app_detail_widget.btn.text(),self.app_detail_widget.app_msg))
            self.app_detail_widget.scrollArea.verticalScrollBar().setValue(0)
            self.app_detail_widget.show()
            self.app_detail_widget.raise_()
            if (self.app_detail_widget in self.mw.path_of_particle):
                self.mw.path_of_particle.remove(self.app_detail_widget)
                self.mw.path_of_particle.append(self.app_detail_widget)
            else:
                self.mw.path_of_particle.append(self.app_detail_widget)
        except:
            self.tip_error.emit(_("Software source is incomplete or does not contain the software"))
            self.mw.tip_widget.btn.clicked.connect(self.mw.slot_close_tipwidget)


    #
    #函数：点击空的广告
    #
    def slot_clicked_none_card(self):
        self.mw.show_tip_widget(_("Native architecture does not support, please wait!"))
        self.mw.tip_widget.btn.clicked.connect(self.mw.slot_close_tipwidget)

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
            if (hasattr(self.mw, "title_bar")):
                self.mw.title_bar.search_line.setFocus(False)
                self.mw.title_bar.search_line.clearFocus()
                self.mw.title_bar.exit_login_btn.hide()
                self.mw.title_bar.senior_search.hide()
            if (hasattr(self.mw, "search_tip_widget")):
                self.mw.search_tip_widget.hide()
            if (hasattr(self.mw, "menubar")):
                if (hasattr(self.mw, "second_lavel_menubar")):
                    self.mw.menubar.hide()
                    self.mw.second_lavel_menubar.hide()
                else:
                    self.mw.menubar.hide()
            self.status = BUTTON_PRESSED
        else:
            event.ignore()


