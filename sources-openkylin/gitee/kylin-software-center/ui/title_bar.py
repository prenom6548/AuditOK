#!/usr/bin/python3
# -*- coding: utf-8 -*-


from PyQt5.QtGui import *
from PyQt5.QtCore import *
from PyQt5.QtWidgets import QWidget, QHBoxLayout, QLabel, QFrame, QApplication
from ui.systembutton import *
from models.globals import Globals
from models.enums import *
from ui.menubar import *
from utils import commontools
from ui.search_records import *
import gettext
gettext.bindtextdomain("kylin-software-center", "/usr/share/locale")
gettext.textdomain("kylin-software-center")
_ = gettext.gettext

class TitleBar(QFrame):
    first_click_range_menu = True
    change_mouse_statue = pyqtSignal()
    def __init__(self,width,mw,parent=None):
        QFrame.__init__(self, parent)
        self.ui_width = width
        self.m_parent = parent
        self.mw =mw
        self.click_item = False
        self.font = QFont()
        self.font.setPixelSize(14)
        self.m_font = QFontMetrics(self.font)
        self.pre_num = 0
        self.init_ui()

    def init_ui(self):
        self.resize(self.ui_width,85+10)
        self.setWindowFlags(Qt.FramelessWindowHint)
        # self.shadow_1 = QGraphicsBlurEffect(self)
        # self.shadow_1.setBlurRadius(80)
        # self.shadow_1.setBlurHints(QGraphicsBlurEffect.PerformanceHint)
        # self.setGraphicsEffect(self.shadow_1)

        # self.logo_icon_label = QLabel(self)
        # self.logo_icon_label.setFixedSize(24, 24)
        # self.logo_icon_label.setFrameShape(QFrame.NoFrame)
        # self.logo_icon_label.setAutoFillBackground(True)
        # self.logo_icon_label.setScaledContents(True)

        self.logo_icon_label = QPushButton(self)
        self.logo_icon_label.setFixedSize(24, 24)
        # self.logo_icon_label.setFrameShape(QFrame.NoFrame)
        self.logo_icon_label.setAutoFillBackground(True)
        # self.logo_icon_label.setScaledContents(True)
        iconpath = commontools.get_title_icon(Globals.APPNAME)
        self.logo_icon_label.setIcon(QIcon(iconpath))
        self.logo_icon_label.setIconSize(QSize(24, 24))
        self.logo_icon_label.setStyleSheet("QPushButton{background:transparent;border:0px;}")

        # appPicture = new
        # QPushButton(this); // 窗体左上角标题图片
        # appPicture->setIcon(QIcon::fromTheme("kylin-recorder", QIcon(":/png/png/recording_32.png")));
        # appPicture->setFixedSize(25, 25);
        # appPicture->setIconSize(QSize(25, 25)); // 重置图标大小
        # self.label_pixmap = icon.pixmap(8,8)
        # self.label_pixmap = QPixmap(icon)
        # self.logo_icon_label.setPixmap(QIcon.fromTheme("kylin-sotfware-center").pixmap(24,24))
        # for path in QIcon.themeSearchPaths():
        #     print("%s/%s" % (path, QIcon.themeName()))


        # iconpath = commontools.get_big_icon_path(Globals.APPNAME)
        # self.label_pixmap = QPixmap(iconpath)
        # self.logo_icon_label.setPixmap(self.label_pixmap)
        self.logo_icon_label.move(8,8)
        self.logo_icon_label.show()

        self.title_label = QLabel(self)
        self.title_label.resize(204,20)
        self.title_label.setAttribute(Qt.WA_TranslucentBackground)
        self.title_label.setText(_("Software Store"))
        self.title_label.move(self.logo_icon_label.x()+self.logo_icon_label.width()+8,10)
        self.title_label.show()

        self.search_label = QFrame(self)
        #self.search_label.setAttribute(Qt.WA_TranslucentBackground)
        self.search_label.setFixedSize(232,32)
        # self.search_label.setStyleSheet("QFrame{background: #ECECEC;font-size: 14px;font-weight: 500;color: #303133;line-height: 20px;border-radius: 16px;}")
        self.search_label.move(int((self.width()-self.search_label.width())/2),5)
        self.search_label.hide()

        self.all_search_tip_label = QLabel(self)
        self.all_search_tip_label.setAlignment(Qt.AlignLeft)
        self.all_search_tip_label.setStyleSheet("QLabel{font-size: 12px;font-weight: 400;color: #F56C6C;line-height: 17px;}")
        self.all_search_tip_label.setText(_("Full cache search: search applications from software sources, there may be unavailable or quality problems"))
        self.all_search_tip_label.adjustSize()
        self.all_search_tip_label.setFixedHeight(17)
        self.all_search_tip_label.move(self.search_label.x(), self.search_label.y() + self.search_label.height() + 4)
        self.all_search_tip_label.hide()

        self.search_type_label = QPushButton(self.search_label)
        self.search_type_label.setAttribute(Qt.WA_TranslucentBackground)
        self.search_type_label.setFocusPolicy(Qt.NoFocus)
        self.search_type_label.setFixedSize(53,32)
        self.search_type_label.move(0,0)
        self.search_type_label.setStyleSheet("QPushButton{border:0px;background: transparent;font-size: 14px;text-align: center;font-weight: 500;color: #303133;line-height: 20px;border-radius: 16px;}")
        self.search_type_label.clicked.connect(self.slot_click_type_btn)
        self.search_type_label.hide()

        self.search_type = QLabel(self.search_type_label)
        self.search_type.setAttribute(Qt.WA_TranslucentBackground)
        self.search_type.setFixedSize(39,32)
        self.search_type.setAlignment(Qt.AlignCenter)
        self.search_type.setText(_("Chc"))
        # self.search_type.setStyleSheet("QLabel{border:0px;font-size: 12px;font-weight: 400;color: #8F9399;line-height: 12px;}")
        self.search_type.move(3,0)
        self.search_type.hide()

        self.search_down_btn = QLabel(self.search_type_label)
        # self.search_down_btn.setAttribute(Qt.WA_TranslucentBackground)
        self.search_down_btn.setFixedSize(13,12)
        # self.search_down_btn.setStyleSheet("QLabel{border:0px;background-image:url(./res/down.png);background-color:transparent;border:0px;}")
        self.search_down_btn.move(self.search_type_label.width()-self.search_down_btn.width(),int((self.search_type_label.height()-self.search_down_btn.height())/2))


        self.senior_search = Search_records(120,self.mw)
        self.senior_search.move(self.search_label.x(),self.search_label.y() + self.search_label.height())
        item1 = QListWidgetItem()
        list_item1 = line_widget(116,self.senior_search)
        list_item1.appname_label.hide()
        list_item1.appname_label.setText(_("All"))
        list_item1.appname_label.adjustSize()
        list_item1.appname_label.move(list_item1.appname_label.x(),int((list_item1.height()-list_item1.appname_label.height())/2))
        list_item1.appname_label.show()
        self.senior_search.item_list_widget.addItem(item1)
        self.senior_search.item_list_widget.setItemWidget(item1, list_item1)
        item2 = QListWidgetItem()
        list_item2 = line_widget(116,self.senior_search)
        list_item2.appname_label.hide()
        list_item2.appname_label.setText(_("Chc"))
        list_item2.appname_label.adjustSize()
        list_item2.appname_label.move(list_item2.appname_label.x(),
                                      int((list_item2.height() - list_item2.appname_label.height()) / 2))
        list_item2.appname_label.show()
        self.senior_search.item_list_widget.addItem(item2)
        self.senior_search.item_list_widget.setItemWidget(item2, list_item2)
        self.senior_search.item_list_widget.setFixedHeight(2*list_item2.height())
        self.senior_search.item_list_widget.show()
        self.senior_search.widget.setFixedHeight(2*(list_item2.height()+2))
        self.senior_search.setFixedHeight(2*(list_item2.height()+2)+20)
        self.senior_search.show()
        self.senior_search.hide()
        self.senior_search.item_list_widget.itemClicked.connect(self.show_red_search)
        self.locale_lan = QLocale.system().name()


        self.line_label = QLabel(self.search_label)
        #self.line_label.setAttribute(Qt.WA_TranslucentBackground)
        self.line_label.setFixedSize(1,13)
        # self.line_label.setStyleSheet("QLabel{border:1px solid #8F9399;background-color:#8F9399;color:#535353;}")
        self.line_label.move(self.search_type_label.x()+self.search_type_label.width()+4,int((self.search_label.height()-self.line_label.height())/2))
        self.line_label.hide()

        self.search_line_back = QLabel(self.search_label)
        self.search_line_back.setText(_("Please enter appname"))
        # self.search_line.setMaxLength(30)
        # self.search_line_back.setAttribute(Qt.WA_TranslucentBackground)
        self.search_line_back.setEnabled(False)
        self.search_line_back.adjustSize()
        self.search_line_back.setFixedHeight(20)

        self.search_line = QLineEdit(self.search_label)
        self.search_line.setMaxLength(30)
        self.search_line.setAttribute(Qt.WA_TranslucentBackground)
        self.search_line.setFixedSize(180, 20)
        self.search_line.move(60, int((self.search_label.height() - self.search_line.height()) / 2))
        self.search_line.textChanged.connect(self.back_changed)
        self.search_line.textEdited.connect(self.back_changed)
        self.search_line.show()
        self.search_line.installEventFilter(self)
        self.search_line_back.installEventFilter(self)

        self.search_image = QPushButton(self.search_label)
        self.search_image.setFocusPolicy(Qt.NoFocus)
        self.search_image.setAttribute(Qt.WA_TranslucentBackground)
        self.search_image.setFixedSize(16, 16)
        self.search_image.move(12, int((self.search_label.height() - self.search_image.height()) / 2))
        # self.search_image.setStyleSheet(
        #     "QPushButton{border:0px;font-size:12px;color:#a0a0a0;text-align: center;image: url(./res/search.png);background-color: rgba(255, 0, 0, 0);}QPushButton:hover{border:0px;font-size:12px;color:#a0a0a0;image: url(./res/search_2.png);background-color: rgba(255, 0, 0, 0);}QPushButton:pressed{border:0px;font-size:12px;color:#a0a0a0;image: url(./res/search_2.png);background-color: rgba(255, 0, 0, 0);}")
        # self.search_line.setStyleSheet("QLineEdit{border:0px;font-size: 12px;font-weight: 400;color: #8F9399;line-height: 16px;border:0px;background-color: transparent;}")
        self.search_line.move(self.search_image.x()+self.search_image.width(), int((self.search_label.height() - self.search_line.height()) / 2))
        self.search_line.show()
        self.search_line_back.move(self.search_image.x()+self.search_image.width()+5, int((self.search_label.height() - self.search_line_back.height()) / 2))
        self.search_line_back.show()
        self.search_image.show()

        # self.search_line_back.setFixedSize(self.search_image.x()-self.search_line_back.x()-5, 20)

        self.search_cancel_btn = QPushButton(self.search_label)
        self.search_cancel_btn.setFocusPolicy(Qt.NoFocus)
        self.search_cancel_btn.setAttribute(Qt.WA_TranslucentBackground)
        self.search_cancel_btn.setFixedSize(16, 16)
        self.search_cancel_btn.move(self.search_label.width()-self.search_cancel_btn.width()-10, int((self.search_label.height() - self.search_cancel_btn.height()) / 2))
        self.search_cancel_btn.setStyleSheet(
            "QPushButton{border:0px;font-size:12px;color:#a0a0a0;text-align: center;image: url(./res/eliminate.png);background-color: rgba(255, 0, 0, 0);}QPushButton:hover{border:0px;font-size:12px;color:#a0a0a0;image: url(./res/eliminate.png);background-color: rgba(255, 0, 0, 0);}QPushButton:pressed{border:0px;font-size:12px;color:#a0a0a0;image: url(./res/eliminate.png);background-color: rgba(255, 0, 0, 0);}")
        self.search_cancel_btn.hide()
        self.search_cancel_btn.clicked.connect(self.slot_click_search_cancel_btn)

        self.min_button = QPushButton(self)
        self.min_button.resize(30,30)
        self.min_button.setFocusPolicy(Qt.NoFocus)
        self.min_button.move(self.width() - 102, 4)
        self.min_button.setToolTip(_("mini"))
        self.set_button = QPushButton(self)
        self.set_button.resize(30,30)
        self.set_button.setFocusPolicy(Qt.NoFocus)
        self.set_button.setGeometry(self.width() - 136, 4, 30, 30)
        self.set_button.installEventFilter(self)
        self.set_button.setToolTip(_("menu"))
        self.max_button = QPushButton(self)
        self.max_button.resize(30, 30)
        self.max_button.setFocusPolicy(Qt.NoFocus)
        self.max_button.setGeometry(self.width() - 68, 4, 30, 30)
        # self.max_button.installEventFilter(self)
        self.max_button.setToolTip(_("max"))
        self.recovery_button = QPushButton(self)
        self.recovery_button.resize(30, 30)
        self.recovery_button.setFocusPolicy(Qt.NoFocus)
        self.recovery_button.setGeometry(self.width() - 68, 4, 30, 30)
        self.recovery_button.hide()
        self.recovery_button.installEventFilter(self)
        self.recovery_button.setToolTip(_("recovery"))
        self.close_button = QPushButton(self)
        self.close_button.resize(30,30)
        self.close_button.setFocusPolicy(Qt.NoFocus)
        self.close_button.setGeometry(self.width() - 34, 4, 30, 30)
        self.close_button.setToolTip(_("close"))


        self.login_btn = Login_Button(self)
        self.login_btn.resize(120,20)
        self.login_btn.init_ui()
        # self.login_btn.setLongTextToElideFormat(self.login_btn.user_name_label, Globals.LOGIN_ACCOUNT)
        self.login_btn.user_name_label.setText(_("Checking..."))
        self.login_btn.user_name_label.adjustSize()
        self.login_btn.user_name_label.setFixedWidth(self.login_btn.user_name_label.width())
        self.login_btn.user_name_label.resize(self.login_btn.user_name_label.width() + 3, 20)
        self.login_btn.btn.resize(self.login_btn.user_name_label.width() + 4 + self.login_btn.head_portrait_label.width(),20)
        self.login_btn.setAttribute(Qt.WA_TranslucentBackground)
        self.login_btn.resize(self.login_btn.user_name_label.x()+self.login_btn.user_name_label.width(),20)
        self.login_btn.move(self.set_button.x()-10-self.login_btn.width(), 10)
        self.login_btn.btn.setStyleSheet("QPushButton{background:transparent;border:0px;border-radius: 4px;}")
        # self.login_btn.head_portrait_label.setStyleSheet("QLabel{border:0px;border-radius: 10px;}")
        self.login_btn.btn.setEnabled(False)
        self.login_btn.hide()

        self.exit_login_btn = Search_records(120,self.mw)
        # self.exit_login_btn.resize(120,20)
        item1 = QListWidgetItem()
        list_item1 = line_widget(116, self.exit_login_btn)
        list_item1.appname_label.hide()
        list_item1.appname_label.setText(_("User settings"))
        list_item1.appname_label.adjustSize()
        list_item1.appname_label.move(list_item1.appname_label.x(),
                                      int((list_item1.height() - list_item1.appname_label.height()) / 2))

        list_item1.appname_label.show()
        self.exit_login_btn.item_list_widget.addItem(item1)
        self.exit_login_btn.item_list_widget.setItemWidget(item1, list_item1)
        # self.exit_login_btn.resize(120,20)
        # item3 = QListWidgetItem()
        # list_item3 = line_widget(116, self.exit_login_btn)
        # list_item3.appname_label.hide()
        # list_item3.appname_label.setText(_("Scoring and comments"))
        # list_item3.appname_label.adjustSize()
        # list_item3.appname_label.move(list_item3.appname_label.x(),
        #                               int((list_item3.height() - list_item3.appname_label.height()) / 2))
        #
        # list_item3.appname_label.show()
        # self.exit_login_btn.item_list_widget.addItem(item3)
        # self.exit_login_btn.item_list_widget.setItemWidget(item3, list_item3)
        item2 = QListWidgetItem()
        list_item2 = line_widget(116, self.exit_login_btn)
        list_item2.appname_label.hide()
        list_item2.appname_label.setText(_("Exit login"))
        list_item2.appname_label.adjustSize()
        list_item2.appname_label.move(list_item2.appname_label.x(),
                                      int((list_item2.height() - list_item2.appname_label.height()) / 2))
        list_item2.appname_label.show()
        self.exit_login_btn.item_list_widget.addItem(item2)
        self.exit_login_btn.item_list_widget.setItemWidget(item2, list_item2)
        self.exit_login_btn.item_list_widget.setFixedHeight(2 * list_item2.height())
        self.exit_login_btn.item_list_widget.show()
        self.exit_login_btn.widget.setFixedHeight(2 * (list_item2.height() + 2))
        self.exit_login_btn.setFixedHeight(2 * (list_item2.height() + 2) + 20)
        self.exit_login_btn.show()
        self.exit_login_btn.hide()
        self.exit_login_btn.item_list_widget.itemClicked.connect(self.mw.slot_click_account_btn)
        self.exit_login_btn.move(self.login_btn.x(), self.login_btn.y() + self.login_btn.height() + 10)
        self.exit_login_btn.hide()

        self.return_btn = QPushButton(self)
        self.return_btn.setFocusPolicy(Qt.NoFocus)
        self.return_btn.setAttribute(Qt.WA_TranslucentBackground)
        self.return_btn.resize(20,20)
        self.return_btn.move(32,62)
        # self.return_btn.setStyleSheet("QPushButton{border:0px;image: url(./res/return_btn.png);}")
        self.return_btn.hide()


        self.home_page_btn = QPushButton(self)
        self.home_page_btn.setAttribute(Qt.WA_TranslucentBackground)

        self.home_page_btn.setFocusPolicy(Qt.NoFocus)
        # self.home_page_btn.setStyleSheet("QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 400;color: #535353;text-align: center;line-height: 20px;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
        self.home_page_btn.setFixedSize(32,20)
        self.home_page_btn.setText(_("Discover"))
        self.home_page_btn.adjustSize()
        self.home_page_btn.move(76,57)
        self.home_page_btn.hide()
        self.home_page_label = QLabel(self)

        self.home_page_label.setStyleSheet("QLabel{border:0px;background: #3790FA;border-radius: 2px;}")
        self.home_page_label.move(79,81)
        self.home_page_label.hide()
        if self.locale_lan == "zh_CN":
            self.change_btn_width(self.home_page_btn)
            self.home_page_label.resize(self.home_page_btn.width() - 6, 4)
        else:
            self.change_btn_width(self.home_page_btn)
            self.home_page_label.resize(self.home_page_btn.width() - 6, 4)

        self.category_btn = QPushButton(self)
        self.category_btn.setAttribute(Qt.WA_TranslucentBackground)
        self.category_btn.setFocusPolicy(Qt.NoFocus)
        # self.category_btn.setFixedSize(32, 20)
        # self.category_btn.setStyleSheet("QPushButton{border:0px;background-color:transparent;text-align: center;font-size: 14px;text-align: center;font-weight: 400;color: #535353;line-height: 20px;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
        self.category_btn.setText(_("Software"))
        self.category_btn.adjustSize()
        self.category_btn.move(self.home_page_btn.x()+self.home_page_btn.width()+30, 57)
        self.category_btn.hide()
        self.category_label = QLabel(self)
        # self.category_label.resize(self.category_btn.width()-5,4)
        self.category_label.setStyleSheet("QLabel{border:0px;background: #3790FA;border-radius: 2px;}")
        self.category_label.move(self.category_btn.x()+3,81)
        self.category_label.hide()
        if self.locale_lan == "zh_CN":
            # self.category_btn.setFixedSize(32,20)
            self.change_btn_width(self.category_btn)
            self.category_label.resize(self.category_btn.width() - 6, 4)
        else:
            # self.category_btn.setFixedSize(70, 20)
            self.change_btn_width(self.category_btn)
            self.category_label.resize(self.category_btn.width() - 6, 4)

        self.drive_btn = QPushButton(self)
        self.drive_btn.setAttribute(Qt.WA_TranslucentBackground)
        self.drive_btn.setFocusPolicy(Qt.NoFocus)
        # self.category_btn.setFixedSize(32, 20)
        # self.drive_btn.setStyleSheet(
        #     "QPushButton{border:0px;background-color:transparent;text-align: center;font-size: 14px;text-align: center;font-weight: 400;color: #535353;line-height: 20px;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
        self.drive_btn.setText(_("Drive"))
        self.drive_btn.adjustSize()
        self.drive_btn.move(self.category_btn.x() + self.category_btn.width() + 30, 57)
        self.drive_btn.hide()
        self.drive_label = QLabel(self)
        # self.category_label.resize(self.category_btn.width()-5,4)
        self.drive_label.setStyleSheet("QLabel{border:0px;background: #3790FA;border-radius: 2px;}")
        self.drive_label.move(self.drive_btn.x() + 3, 81)
        self.drive_label.hide()
        if self.locale_lan == "zh_CN":
            self.change_btn_width(self.drive_btn)
            self.drive_label.resize(self.drive_btn.width() - 6, 4)
        else:
            self.change_btn_width(self.drive_btn)
            self.drive_label.resize(self.drive_btn.width() - 6, 4)

        self.kydroid_btn = QPushButton(self)
        self.kydroid_btn.setAttribute(Qt.WA_TranslucentBackground)
        self.kydroid_btn.setFocusPolicy(Qt.NoFocus)
        # self.kydroid_btn.setFixedSize(32, 20)
        # self.kydroid_btn.setStyleSheet("QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 400;color: #535353;text-align: center;line-height: 20px;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
        self.kydroid_btn.setText(_("Mobile apps"))
        self.kydroid_btn.adjustSize()
        self.kydroid_btn.move(self.drive_btn.x()+self.drive_btn.width()+30,57)
        self.kydroid_btn.hide()
        self.kydroid_label = QLabel(self)
        # self.kydroid_label.resize(self.kydroid_btn.width()-5,4)
        self.kydroid_label.setStyleSheet("QLabel{border:0px;background: #3790FA;border-radius: 2px;}")
        self.kydroid_label.move(self.kydroid_btn.x()+3,81)
        self.kydroid_label.hide()
        if self.locale_lan == "zh_CN":
            # self.kydroid_btn.setFixedSize(56,20)
            self.change_btn_width(self.kydroid_btn)
            self.kydroid_label.resize(self.home_page_btn.width() - 6, 4)
            self.kydroid_label.move(self.kydroid_btn.x()+int((self.kydroid_btn.width()-self.kydroid_label.width())/2),81)
        else:
            # self.kydroid_btn.setFixedSize(82, 20)
            self.change_btn_width(self.kydroid_btn)
            self.kydroid_label.resize(self.kydroid_btn.width() - 6, 4)


        self.my_btn = QPushButton(self)
        self.my_btn.setAttribute(Qt.WA_TranslucentBackground)
        self.my_btn.setFocusPolicy(Qt.NoFocus)
        # self.my_btn.setFixedSize(32, 20)
        # self.my_btn.setStyleSheet("QPushButton{border:0px;background-color:transparent;font-size: 14px;font-weight: 400;color: #535353;text-align: center;line-height: 20px;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
        self.my_btn.setText(_("My apps"))
        self.my_btn.adjustSize()
        self.my_btn.move(self.kydroid_btn.x()+self.kydroid_btn.width()+30, 57)
        self.my_btn.hide()
        self.my_label = QLabel(self)
        # self.my_label.setFixedSize(self.my_btn.width()-5,4)
        self.my_label.setStyleSheet("QLabel{border:0px;background: #3790FA;border-radius: 2px;}")
        self.my_label.move(self.my_btn.x()+3,81)
        self.my_label.hide()
        self.my_btn_tip = QLabel(self)
        self.my_btn_tip.setFixedSize(6, 6)
        self.my_btn_tip.setStyleSheet("QLabel{background: #F56C6C;border-radius:3px;font-size: 11px;font-weight: 400;color: #FFFFFF;text-align: center;line-height: 16px;}")
        self.my_btn_tip.move(self.my_btn.x() + self.my_btn.width() + 3 , self.my_btn.y()-3)
        self.my_btn_tip.setAlignment(Qt.AlignCenter)
        self.my_btn_tip.hide()
        if self.locale_lan == "zh_CN":
            # self.my_btn.setFixedSize(32,20)
            self.change_btn_width(self.my_btn)
            self.my_label.resize(self.my_btn.width() - 6, 4)
        else:
            # self.my_btn.setFixedSize(55, 20)
            self.change_btn_width(self.my_btn)
            self.my_label.resize(self.my_btn.width() - 6, 4)
        self.refresh_background_color()
        # self.home_page_btn.setEnabled(False)
        # self.my_btn.setEnabled(False)
        # self.drive_btn.setEnabled(False)
        # self.kydroid_btn.setEnabled(False)
        # self.category_btn.setEnabled(False)

    def back_changed(self,text = ""):
        if self.search_line.text() != '':
            self.search_cancel_btn.show()
            self.search_line_back.hide()
        else:
            self.search_cancel_btn.hide()
            self.search_line_back.show()

    def slot_editingFinished(self):
        self.search_line_back.hide()
            
    def re_move(self):
        self.close_button.hide()
        self.min_button.hide()
        # self.max_button.hide()
        self.logo_icon_label.move(8, 8)
        self.title_label.move(self.logo_icon_label.x() + self.logo_icon_label.width() + 8, 10)
        self.search_label.move(int((self.width() - self.search_label.width()) / 2), 5)
        self.senior_search.move(self.search_label.x(), self.search_label.y() + self.search_label.height())
        self.min_button.move(self.width() - 102, 4)
        self.set_button.setGeometry(self.width() - 136, 4, 30, 30)
        self.max_button.setGeometry(self.width() - 68, 4, 30, 30)
        self.recovery_button.setGeometry(self.width() - 68, 4, 30, 30)
        self.close_button.setGeometry(self.width() - 34, 4, 30, 30)
        self.login_btn.move(self.set_button.x() - 10 - self.login_btn.width(), 10)
        self.exit_login_btn.move(self.login_btn.x() , self.login_btn.y()+self.login_btn.height()+10)
        self.all_search_tip_label.move(self.search_label.x(), self.search_label.y() + self.search_label.height() + 4)
        self.return_btn.move(32, 62)
        self.home_page_btn.move(76, 57)
        self.home_page_label.move(79, 81)
        self.category_btn.move(self.home_page_btn.x() + self.home_page_btn.width() + 30, 57)
        self.category_label.move(self.category_btn.x() + 3, 81)
        if(self.drive_btn.isHidden() == False):
            self.drive_btn.move(self.category_btn.x()+self.category_btn.width()+30,57)
            self.drive_label.move(self.drive_btn.x()+3,81)
            self.kydroid_btn.move(self.drive_btn.x() + self.drive_btn.width() + 30, 57)
            self.kydroid_label.move(self.kydroid_btn.x()+int((self.kydroid_btn.width()-self.kydroid_label.width())/2),81)
            if(self.kydroid_btn.isHidden()):
                self.my_btn.move(self.kydroid_btn.x(), 57)
                self.my_label.move(self.my_btn.x() + 3, 81)
            else:
                self.my_btn.move(self.kydroid_btn.x() + self.kydroid_btn.width() + 30, 57)
                self.my_label.move(self.my_btn.x() + 3, 81)
        else:
            if(self.home_page_btn.isHidden() == True):
                self.drive_btn.move(self.category_btn.x() + self.category_btn.width() + 30, 57)
                self.drive_label.move(self.drive_btn.x() + 3, 81)
                self.kydroid_btn.move(self.drive_btn.x() + self.drive_btn.width() + 30, 57)
                self.kydroid_label.move(
                    self.kydroid_btn.x() + int((self.kydroid_btn.width() - self.kydroid_label.width()) / 2), 81)
                if (self.kydroid_btn.isHidden()):
                    self.my_btn.move(self.kydroid_btn.x(), 57)
                    self.my_label.move(self.my_btn.x() + 3, 81)
                else:
                    self.my_btn.move(self.kydroid_btn.x() + self.kydroid_btn.width() + 30, 57)
                    self.my_label.move(self.my_btn.x() + 3, 81)
            else:
                self.kydroid_btn.move(self.category_btn.x() + self.category_btn.width() + 30, 57)
                self.kydroid_label.move(self.kydroid_btn.x()+int((self.kydroid_btn.width()-self.kydroid_label.width())/2),81)
                if (self.kydroid_btn.isHidden()):
                    self.my_btn.move(self.kydroid_btn.x(), 57)
                    self.my_label.move(self.my_btn.x() + 3, 81)
                else:
                    self.my_btn.move(self.kydroid_btn.x() + self.kydroid_btn.width() + 30, 57)
                    self.my_label.move(self.my_btn.x() + 3, 81)
        self.my_btn_tip.move(self.my_btn.x() + self.my_btn.width() + 3, self.my_btn.y() - 3)
        self.close_button.show()
        self.min_button.show()
        # if(self.max_button.isHidden()):
        #     self.recovery_button.show()
        #     self.max_button.hide()
        # else:
        #     self.recovery_button.hide()
        #     self.max_button.show()

    def show_exit_login_widget(self):
        self.login_btn.init_ui()
        self.refresh_background_color()

    def slot_click_type_btn(self):
        self.mw.hide_tip_widget()
        if(self.senior_search.isHidden() == True):
            self.senior_search.item_list_widget.show()
            self.senior_search.show()
        else:
            self.senior_search.item_list_widget.hide()
            self.senior_search.hide()

    def change_btn_width(self,label):
        s = self.m_font.boundingRect(label.text()).size().width()
        label.setFixedSize(s, 22)


    def change_icon(self,status):
        if(status == BUTTON_LEAVE):
            if(Globals.THEME_COLOR == 0):
                self.exit_login_btn.head_portrait_label.setStyleSheet(
                    "QLabel{border:0px;border-radius: 10px;background-image:url(./res/exit_login.png); background-position:center;}")
                self.exit_login_btn.user_name_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #303133;line-height: 20px;}")
            else:
                self.exit_login_btn.user_name_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #F9F9F9;}")
                self.exit_login_btn.head_portrait_label.setStyleSheet("QLabel{border:0px;border-radius: 10px;background-image:url(./res/exit_login_dark.png);background-position:center;}")
        else:
            self.exit_login_btn.head_portrait_label.setStyleSheet(
                "QLabel{border:0px;border-radius: 10px;background-image:url(./res/exit_login1.png);background-position:center;}")
            self.exit_login_btn.user_name_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #F56C6C;line-height: 20px;}")

    #
    #函数：重新设置icon
    #
    def change_icon_show(self):
        iconpath = commontools.get_title_icon(Globals.APPNAME)
        self.logo_icon_label.setIcon(QIcon(iconpath))
        self.logo_icon_label.setIconSize(QSize(24, 24))
        self.logo_icon_label.setStyleSheet("QPushButton{background:transparent;border:0px;}")
        pass
        # iconpath = commontools.get_big_icon_path(Globals.APPNAME)
        # self.label_pixmap = QPixmap(iconpath)
        # self.logo_icon_label.setPixmap(self.label_pixmap)
        # self.logo_icon_label.hide()
        # self.logo_icon_label.show()

    #
    #函数：刷新背景颜色
    #
    def refresh_background_color(self):
        if (Globals.THEME_COLOR == 0):
            self.min_button.setStyleSheet("QPushButton{border-radius: 4px;border:0px;border-image:url('res/mini1.png')}QPushButton:hover{border:0px;border-image:url('res/mini2.png')}QPushButton:pressed{border:0px;border-image:url('res/mini3.png')}")
            self.set_button.setStyleSheet("QPushButton{border-radius: 4px;border:0px;border-image:url('res/set1.png')}QPushButton:hover{border:0px;border-image:url('res/set2.png')}QPushButton:pressed{border:0px;border-image:url('res/set3.png')}")
            self.close_button.setStyleSheet("QPushButton{border-radius: 4px;border:0px;border-image:url('res/close1.png')}QPushButton:hover{border:0px;border-image:url('res/close2.png')}QPushButton:pressed{border:0px;border-image:url('res/close3.png')}")
            self.max_button.setStyleSheet(
                "QPushButton{border-radius: 4px;border:0px;border-image:url('res/max1.png')}QPushButton:hover{border:0px;border-image:url('res/max2.png')}QPushButton:pressed{border:0px;border-image:url('res/max3.png')}")
            self.recovery_button.setStyleSheet(
                "QPushButton{border-radius: 4px;border:0px;border-image:url('res/recovery1.png')}QPushButton:hover{border:0px;border-image:url('res/recovery2.png')}QPushButton:pressed{border:0px;border-image:url('res/recovery3.png')}")
            if(Globals.RIGHT_ANGLE):
                self.setStyleSheet("QFrame{background-color: #ffffff;}")
            else:
                self.setStyleSheet("QFrame{background-color: #ffffff;border-top-left-radius:12px;border-top-right-radius:12px;}")
            self.title_label.setStyleSheet("QLabel{font-size: 14px;color: #303133;font-weight: 500;}")
            self.search_label.setStyleSheet(
                "QFrame{background: #ECECEC;font-size: 14px;font-weight: 500;color: #8F9399;line-height: 20px;border-radius: 16px;}")
            self.line_label.setStyleSheet("QLabel{border:1px solid #8F9399;background-color:#8F9399;color:#535353;}")
            self.search_type.setStyleSheet("QLabel{border:0px;font-size: 12px;font-weight: 400;color: #8F9399;line-height: 12px;}")
            self.search_down_btn.setStyleSheet(
                "QLabel{border:0px;background-image:url(./res/down.png);background-color:transparent;border:0px;}")
            self.search_line_back.setStyleSheet(
                "QLabel{border:0px;font-size: 12px;font-weight: 400;color: #8F9399;line-height: 16px;border:0px;background-color: transparent;}")

            self.search_line.setStyleSheet("QLineEdit{border:0px;font-size: 12px;font-weight: 400;color: #8F9399;line-height: 16px;border:0px;background-color: transparent;}")
            self.search_image.setStyleSheet(
                "QPushButton{border:0px;font-size:12px;color:#a0a0a0;text-align: center;image: url(./res/search.png);background-color: rgba(255, 0, 0, 0);}QPushButton:hover{border:0px;font-size:12px;color:#a0a0a0;image: url(./res/search_2.png);background-color: rgba(255, 0, 0, 0);}QPushButton:pressed{border:0px;font-size:12px;color:#a0a0a0;image: url(./res/search_2.png);background-color: rgba(255, 0, 0, 0);}")
            self.login_btn.user_name_label.setStyleSheet("QLabel{font-size: 14px;border:0px;font-weight: 400;color: #303133;line-height: 20px;border-radius: 0px;}")

            if(Globals.USER_PROFILE == ''):
                user_profile = UBUNTUKYLIN_RES_PATH + "unlogin.png"
                pix2 = QPixmap(user_profile)
                fitpixmap_userIcon = pix2.scaled(20, 20, Qt.IgnoreAspectRatio, Qt.SmoothTransformation)
                fitpixmap_userIcon = self.PixmapToRound(fitpixmap_userIcon, 20,
                                                        20)
                self.login_btn.head_portrait_label.setPixmap(fitpixmap_userIcon)
                # self.login_btn.head_portrait_label.setStyleSheet("QLabel{border:0px;border-radius: 10px;background-image:url(./res/unlogin.png)}")
            self.home_page_btn.setStyleSheet(
                "QPushButton{border:0px;background-color:transparent;text-align: center;font-size: 14px;text-align: center;font-weight: 400;color: #535353;line-height: 20px;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
            self.category_btn.setStyleSheet(
                "QPushButton{border:0px;background-color:transparent;text-align: center;font-size: 14px;text-align: center;font-weight: 400;color: #535353;line-height: 20px;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
            self.kydroid_btn.setStyleSheet(
                "QPushButton{border:0px;background-color:transparent;text-align: center;font-size: 14px;text-align: center;font-weight: 400;color: #535353;line-height: 20px;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
            self.drive_btn.setStyleSheet(
                "QPushButton{border:0px;background-color:transparent;text-align: center;font-size: 14px;text-align: center;font-weight: 400;color: #535353;line-height: 20px;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
            self.my_btn.setStyleSheet(
                "QPushButton{border:0px;background-color:transparent;text-align: center;font-size: 14px;text-align: center;font-weight: 400;color: #535353;line-height: 20px;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
            self.return_btn.setStyleSheet("QPushButton{border:0px;image: url(./res/return_btn.png);}")
        else:
            self.min_button.setStyleSheet("QPushButton{border-radius: 4px;border:0px;border-image:url('res/common-minimize-white.png')}QPushButton:hover{border:0px;border-image:url('res/hover-minimize-white.png')}QPushButton:pressed{border:0px;border-image:url('res/clicked-minimize-white.png')}")
            self.set_button.setStyleSheet("QPushButton{border-radius: 4px;border:0px;border-image:url('res/common-more-white.png')}QPushButton:hover{border:0px;border-image:url('res/hover-more-white.png')}QPushButton:pressed{border:0px;border-image:url('res/clicked-more-white.png')}")
            self.close_button.setStyleSheet("QPushButton{border-radius: 4px;border:0px;border-image:url('res/common-close-white.png')}QPushButton:hover{border:0px;border-image:url('res/hover-close.png')}QPushButton:pressed{border:0px;border-image:url('res/clicked-close.png')}")
            self.max_button.setStyleSheet(
                "QPushButton{border-radius: 4px;border:0px;border-image:url('res/common-maximize-white.png')}QPushButton:hover{border:0px;border-image:url('res/hover-maximize-white.png')}QPushButton:pressed{border:0px;border-image:url('res/clicked-maximize-white.png')}")
            self.recovery_button.setStyleSheet(
                "QPushButton{border-radius: 4px;border:0px;border-image:url('res/common-restore-white.png')}QPushButton:hover{border:0px;border-image:url('res/hover-restore-white.png')}QPushButton:pressed{border:0px;border-image:url('res/clicked-restore-white.png')}")
            if (Globals.RIGHT_ANGLE):
                self.setStyleSheet("QFrame{background-color: #2F3032;}")
            else:
                self.setStyleSheet("QFrame{background: #2F3032;border-top-left-radius:12px;border-top-right-radius:12px;}")
            self.title_label.setStyleSheet("QLabel{font-size: 14px;color: #F9F9F9;line-height: 20px;font-weight: 500;}")
            self.line_label.setStyleSheet("QLabel{border:1px solid rgba(143, 147, 153, 1);background-color:rgba(143, 147, 153, 1);}")
            self.search_type.setStyleSheet("QLabel{border:0px;font-size: 12px;font-weight: 400;color: #F9F9F9;line-height: 12px;}")
            self.search_label.setStyleSheet(
                "QFrame{background: rgba(255, 255, 255, 0.08);font-size: 14px;font-weight: 500;color: #8F9399;line-height: 20px;border-radius: 16px;}")
            self.search_down_btn.setStyleSheet("QLabel{border:0px;background-image:url(./res/down.png);background-color:transparent;}")
            self.search_line_back.setStyleSheet(
                "QLabel{border:0px;font-size: 12px;font-weight: 400;color: #8F9399;line-height: 16px;border:0px;background-color: transparent;}")
            self.search_line.setStyleSheet("QLineEdit{border:0px;font-size: 12px;font-weight: 400;color: #ffffff;line-height: 16px;border:0px;background-color: transparent;}")
            self.search_image.setStyleSheet("QPushButton{border:0px;font-size:12px;color:#a0a0a0;text-align: center;image: url(./res/search_dark.png);background-color: rgba(255, 0, 0, 0);}QPushButton:hover{border:0px;font-size:12px;color:#a0a0a0;image: url(./res/search_2.png);background-color: rgba(255, 0, 0, 0);}QPushButton:pressed{border:0px;font-size:12px;color:#a0a0a0;image: url(./res/search_2.png);background-color: rgba(255, 0, 0, 0);}")
            self.login_btn.user_name_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #F9F9F9;line-height: 20px;}")
            if (Globals.USER_PROFILE == ''):
                user_profile = UBUNTUKYLIN_RES_PATH + "unlogin_dark.png"
                pix2 = QPixmap(user_profile)
                fitpixmap_userIcon = pix2.scaled(20, 20, Qt.IgnoreAspectRatio, Qt.SmoothTransformation)
                fitpixmap_userIcon = self.PixmapToRound(fitpixmap_userIcon, 20, 20)
                self.login_btn.head_portrait_label.setPixmap(fitpixmap_userIcon)
                # self.login_btn.head_portrait_label.setStyleSheet("QLabel{border:0px;border-radius: 10px;background-image:url(./res/unlogin_dark.png);background-position:center;}")
            self.home_page_btn.setStyleSheet("QPushButton{border:0px;background-color:transparent;text-align: center;font-size: 14px;text-align: center;font-weight: 400;color: #C0C4CC;line-height: 20px;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
            self.category_btn.setStyleSheet("QPushButton{border:0px;background-color:transparent;text-align: center;font-size: 14px;text-align: center;font-weight: 400;color: #C0C4CC;line-height: 20px;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
            self.drive_btn.setStyleSheet("QPushButton{border:0px;background-color:transparent;text-align: center;font-size: 14px;text-align: center;font-weight: 400;color: #C0C4CC;line-height: 20px;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
            self.kydroid_btn.setStyleSheet("QPushButton{border:0px;background-color:transparent;text-align: center;font-size: 14px;text-align: center;font-weight: 400;color: #C0C4CC;line-height: 20px;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
            self.my_btn.setStyleSheet("QPushButton{border:0px;background-color:transparent;text-align: center;font-size: 14px;text-align: center;font-weight: 400;color: #C0C4CC;line-height: 20px;}QPushButton:hover{color:#3790FA;font-weight: 400;};QPushButton:pressed{color:#3790FA;font-weight: 500;}")
            self.return_btn.setStyleSheet("QPushButton{border:0px;image: url(./res/return_btn_dark.png);}")
        if(self.my_label.isHidden() == False):
            self.mw.set_title_btn_stylesheet(PageStates.MYPAGE)
        elif(self.kydroid_label.isHidden() == False):
            self.mw.set_title_btn_stylesheet(PageStates.KYDROID)
        elif (self.home_page_label.isHidden() == False):
            self.mw.set_title_btn_stylesheet(PageStates.HOMEPAGE)
        elif (self.category_label.isHidden() == False):
            self.mw.set_title_btn_stylesheet(PageStates.CAREGORYPAGE)
        self.exit_login_btn.slot_show_dark_mode()
        self.senior_search.slot_show_dark_mode()
        # QApplication.processEvents()


    def slot_click_search_cancel_btn(self):
        self.search_line.clear()
        self.search_line_back.show()
        self.search_cancel_btn.hide()

    def show_red_search(self,search_type):
        Globals.ADVANCED_SEARCH = search_type
        self.senior_search.hide()

    def eventFilter(self, widget, event):
        if widget == self.search_line:
            if(event.type() == event.MouseButtonPress):
                self.senior_search.hide()
            elif(event.type() == event.FocusAboutToChange):
                if(self.search_line.text() == ''):
                    self.search_cancel_btn.hide()
                    self.search_line_back.show()
                else:
                    self.search_cancel_btn.show()
                    self.search_line_back.hide()
            elif(event.type() == event.KeyRelease):
                if self.search_line.text() == '':
                    if(Qt.Key_A <= event.key() <= Qt.Key_Z or Qt.Key_0 <= event.key() <= Qt.Key_9):
                        if (event.key() == Qt.Key_X):
                            if QApplication.keyboardModifiers() == Qt.ControlModifier:
                                self.search_cancel_btn.hide()
                                self.search_line_back.show()
                        elif (event.key() == Qt.Key_C):
                            if QApplication.keyboardModifiers() == Qt.ControlModifier:
                                self.search_cancel_btn.hide()
                                self.search_line_back.show()
                        elif (event.key() == Qt.Key_Z):
                            if QApplication.keyboardModifiers() == Qt.ControlModifier:
                                self.search_cancel_btn.hide()
                                self.search_line_back.show()
                        else:
                            self.search_cancel_btn.show()
                            self.search_line_back.hide()
                            self.pre_num +=1
                    elif(Qt.Key_Backspace == event.key()):
                        if self.search_line.text() == '':
                            if(self.pre_num != 0):
                                self.pre_num -=1
                            else:
                                self.search_cancel_btn.hide()
                                self.search_line_back.show()
                            if(self.pre_num == 0):
                                self.search_cancel_btn.hide()
                                self.search_line_back.show()
                        else:
                            self.search_cancel_btn.show()
                            self.search_line_back.hide()
                else:
                    self.search_cancel_btn.show()
                    self.search_line_back.hide()
                    self.pre_num = 0
                if(Qt.Key_Return == event.key() or Qt.Key_Enter == event.key() or Qt.Key_Space == event.key()):
                    if self.search_line.text() == '':
                        self.search_cancel_btn.hide()
                        self.search_line_back.show()
                    else:
                        self.search_cancel_btn.show()
                        self.search_line_back.hide()
                        self.pre_num = 0
        if(hasattr(self,"search_line_back")):
            if widget == self.search_line_back:
                if(event.type() == event.MouseButtonPress):
                    self.search_line.setFocus(True)
        if(hasattr(self,"max_button") and hasattr(self,"recovery_button") and hasattr(self,"set_button")):
            if widget == self.max_button :
                if(event.type() == event.MouseButtonPress):
                    self.mw.slot_click_max_btn()
                    return True
                if(event.type() == event.MouseButtonRelease):
                    return True
            if widget == self.recovery_button and event.type() == event.MouseButtonPress:
                return True
            if (widget == self.recovery_button and event.type() == event.MouseButtonRelease):
                self.mw.slot_click_recovery_btn()
                return True
            if (widget == self.recovery_button and event.type() == event.Enter):
                if(Globals.THEME_COLOR == 0):
                    self.recovery_button.setStyleSheet(
                        "QPushButton{border-radius: 4px;border:0px;border-image:url('res/recovery1.png')}QPushButton:hover{border:0px;border-image:url('res/recovery2.png')}QPushButton:pressed{border:0px;border-image:url('res/recovery3.png')}")
                else:
                    self.recovery_button.setStyleSheet(
                        "QPushButton{border-radius: 4px;border:0px;border-image:url('res/common-restore-white.png')}QPushButton:hover{border:0px;border-image:url('res/hover-restore-white.png')}QPushButton:pressed{border:0px;border-image:url('res/clicked-restore-white.png')}")

            if (widget == self.set_button and event.type() == event.MouseButtonRelease ):
                self.mw.slot_click_set_button()
        return False


    def enterEvent(self, event):
        self.mw.setCursor(Qt.ArrowCursor)


     #将图片切割成圆角
    def PixmapToRound(self,src,x,y):
        pixmapa = QPixmap(src)
        pixmap = QPixmap(x, y)
        pixmap.fill(Qt.transparent)
        painter = QPainter(pixmap)
        painter.setRenderHints(QPainter.Antialiasing | QPainter.SmoothPixmapTransform)
        path = QPainterPath()
        rect = QRectF(0,0,x,y)
        path.addRoundedRect(rect, 10.0, 10.0)
        painter.setClipPath(path)
        painter.drawPixmap(0, 0, x, y, pixmapa)

        return pixmap


class Login_Button(QFrame):
    status_changed = pyqtSignal(int)
    def __init__(self,parent = None):
        QFrame.__init__(self, parent)
        self.setAttribute(Qt.WA_TranslucentBackground)
        self.setFocusPolicy(Qt.NoFocus)
        self.setStyleSheet("QFrame{border:0px;background: #F6F6F6;border-radius: 4px;}")
        self.resize(120,39)

        self.btn = QPushButton(self)
        self.btn.setAttribute(Qt.WA_TranslucentBackground)
        self.btn.setFocusPolicy(Qt.NoFocus)
        self.btn.setStyleSheet("QPushButton{background:#ffffff;border:0px;border-radius: 4px;text-align: center;}QPushButton:hover{background:#F7F7F7;border:0px;}QPushButton:pressed{background:#F7F7F7;border:0px;}")
        self.btn.resize(self.width(),20)
        self.btn.move(0,0)


        self.head_portrait_label = QLabel(self.btn)
        self.head_portrait_label.resize(20, 20)
        # self.head_portrait_label.setAttribute(Qt.WA_TranslucentBackground)

        self.user_name_label = QLabel(self.btn)
        self.user_name_label.setAttribute(Qt.WA_TranslucentBackground)
        self.user_name_label.setAlignment(Qt.AlignVCenter)
        # self.user_name_label.setMaximumWidth(95)
        self.user_name_label.setFixedWidth(65)
        # self.user_name_label.maximumWidth()
        self.user_name_label.setFixedHeight(20)
        # self.user_name_label.resize(95, 20)
        self.user_name_label.show()
        self.init_ui()

    def init_ui(self):
        self.head_portrait_label.setStyleSheet("QLabel{border:0px;border-radius: 10px;}")
        self.head_portrait_label.move(0, int((self.btn.height()-self.head_portrait_label.height())/2))
        self.head_portrait_label.show()
        self.user_name_label.setStyleSheet( "QLabel{border:0px;font-size: 14px;font-weight: 400;color: #303133;line-height: 20px;}")
        self.user_name_label.setAttribute(Qt.WA_TranslucentBackground)
        self.user_name_label.setText(_("login"))
        self.user_name_label.adjustSize()
        self.user_name_label.resize(self.user_name_label.width() +1,20)
        self.btn.resize(self.user_name_label.width() +4 +self.head_portrait_label.width(),20)
        self.user_name_label.move(24,int((self.btn.height()-self.user_name_label.height())/2))


    def setLongTextToElideFormat(self,label, te):
        te = te.replace("\n",",").replace("\r","").replace("\t","")
        metrics = QFontMetrics(label.font())
        elidedText = metrics.elidedText(te, Qt.ElideRight, label.maximumWidth())

        # label.document().adjustSize()
        label.setText(elidedText)

# 鼠标进入事件
    def enterEvent(self, event):
        self.status = BUTTON_ENTER
        self.status_changed.emit(BUTTON_ENTER)


    #
    # 函数：鼠标离开事件
    #
    def leaveEvent(self, event):
        self.status = BUTTON_LEAVE
        self.status_changed.emit(BUTTON_LEAVE)
