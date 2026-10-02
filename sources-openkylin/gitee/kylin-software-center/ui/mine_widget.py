#!/usr/bin/python3
# -*- coding: utf-8 -*-


from PyQt5.QtGui import *
from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
from ui.systembutton import *
from ui.systembutton import *
from ui.historical_installation_widget import *
import gettext
gettext.bindtextdomain("kylin-software-center", "/usr/share/locale")
gettext.textdomain("kylin-software-center")
_ = gettext.gettext

class Mine_Widget(QFrame):
    click_type_btn = pyqtSignal(str)
    card_width = 248
    card_height = 124
    card_space = 15
    def __init__(self,w,h, parent=None):
        QWidget.__init__(self, parent)
        self.w = w
        self.h = h
        self.m_parent = parent
        self.space = 50
        self.init_ui()

    def init_ui(self):
        self.resize(self.w,self.h)
        self.setAttribute(Qt.WA_TranslucentBackground)
        self.setStyleSheet("QFrame{border:0px;background-color:transparent;}")
        self.btn_widget = QWidget(self)
        self.btn_widget.setFixedSize(self.width()-self.space*2,114)
        self.btn_widget.move(int((self.width()-self.btn_widget.width())/2),0)

        self.downloading_btn = Click_Button(_("Onload"),self,self.btn_widget)
        self.downloading_btn.resize(80,24)
        self.downloading_btn.move(int((self.btn_widget.width()-self.downloading_btn.width()*4-12*3)/2),20)
        self.downloading_btn.show()

        self.update_btn = Click_Button(_("Updates"),self,self.btn_widget)
        self.update_btn.resize(80,24)
        self.update_btn.move(self.downloading_btn.x()+self.update_btn.width()+12,20)
        self.update_btn.show()

        self.update_tip_spot = QLabel(self.btn_widget)
        self.update_tip_spot.setFixedSize(6, 6)
        self.update_tip_spot.setStyleSheet("QLabel{background: #F56C6C;border-radius:3px;font-size: 12px;font-weight: 400;color: #FFFFFF;line-height: 16px;}")
        self.update_tip_spot.move(self.update_btn.x()+self.update_btn.width()-10,self.update_btn.y()-3)
        self.update_tip_spot.setAlignment(Qt.AlignCenter)
        self.update_tip_spot.hide()

        self.uninstall_btn = Click_Button(_("Uninstall"),self,self.btn_widget)
        self.uninstall_btn.resize(80, 24)
        self.uninstall_btn.move(self.update_btn.x()+self.update_btn.width()+12,20)
        self.uninstall_btn.show()

        self.installed_btn = Click_Button(_("Installation"),self,self.btn_widget)
        if(Globals.LAN == "zh_CN"):
            self.installed_btn.setFixedSize(self.uninstall_btn.width(), 24)
        else:
            self.installed_btn.setFixedSize(self.installed_btn.width(),24)
        self.installed_btn.move(self.uninstall_btn.x()+self.uninstall_btn.width()+12,20)
        self.installed_btn.show()

        self.downloading_num_label = QLabel(self.btn_widget)
        self.downloading_num_label.resize(18, 18)
        self.downloading_num_label.setMinimumWidth(18)
        self.downloading_num_label.setFixedHeight(18)
        self.downloading_num_label.setStyleSheet(
            "QLabel{background: #F56C6C;border-radius:9px;font-size: 12px;font-weight: 400;color: #FFFFFF;line-height: 16px;}")
        self.downloading_num_label.move(int((
                                                        self.btn_widget.width() - self.downloading_btn.width() * 4 - 12 * 3) / 2) + self.downloading_btn.width() - 10,
                                        13)
        self.downloading_num_label.setAlignment(Qt.AlignCenter)
        self.downloading_num_label.hide()

        self.update_num_label = QLabel(self.btn_widget)
        self.update_num_label.resize(18, 18)
        self.update_num_label.setMinimumWidth(18)
        self.update_num_label.setFixedHeight(18)
        self.update_num_label.setStyleSheet("QLabel{background: #F56C6C;border-radius:9px;font-size: 12px;font-weight: 400;color: #FFFFFF;line-height: 16px;}")
        self.update_num_label.move(self.downloading_btn.x()+self.downloading_btn.width()+12+self.update_btn.width()-10,13)
        self.update_num_label.setAlignment(Qt.AlignCenter)
        self.update_num_label.hide()

        self.type_name_label = QLabel(self.btn_widget)
        self.type_name_label.setAttribute(Qt.WA_TranslucentBackground)
        self.type_name_label.setStyleSheet("QLabel{font-size: 24px;font-weight: 600;color: #303133;line-height: 42px;}")
        self.type_name_label.move(0,self.installed_btn.y()+18)
        self.type_name_label.setText(_("Onload"))
        self.type_name_label.adjustSize()

        self.all_check = QPushButton(self.btn_widget)
        self.all_check.setFocusPolicy(Qt.NoFocus)
        self.all_check.setFixedSize(80,24)
        self.all_check.setStyleSheet("QPushButton{border:0px;font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;border:0px solid #ff0000;}QPushButton:hover{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}QPushButton:pressed{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}")
        self.all_check.setText(_("all puase"))
        self.all_check.adjustSize()
        self.all_check.move(self.btn_widget.width()-self.all_check.width(),self.type_name_label.y() + self.type_name_label.height()+2-self.all_check.height())
        self.all_check.setEnabled(True)
        self.all_check.hide()

        self.download_all_check = QPushButton(self.btn_widget)
        self.download_all_check.setFocusPolicy(Qt.NoFocus)
        self.download_all_check.setFixedSize(80, 24)
        self.download_all_check.setStyleSheet("QPushButton{border:0px;font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;border:0px solid #ff0000;}QPushButton:hover{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}QPushButton:pressed{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}")
        self.download_all_check.setText(_("all puase"))
        self.download_all_check.adjustSize()
        self.download_all_check.move(self.btn_widget.width() - self.download_all_check.width(),
                            self.type_name_label.y() + self.type_name_label.height() + 2 - self.download_all_check.height())
        self.download_all_check.setEnabled(True)
        self.download_all_check.hide()

        self.all_select_btn = QPushButton(self.btn_widget)
        self.all_select_btn.setFocusPolicy(Qt.NoFocus)
        self.all_select_btn.setFixedSize(80, 24)
        self.all_select_btn.setStyleSheet("QPushButton{font-size: 14px;font-weight: 400;line-height: 20px;background: transparent;border-radius: 13px;font-weight: 500;color: #606265;line-height: 20px;}QPushButton:hover{background: rgba(55, 144, 250, 0.12);color: #3790FA;border-radius: 12px;}PushButton:pressed{background: rgba(55, 144, 250, 0.12);color: #3790FA;border-radius: 12px;}")
        self.all_select_btn.setText(_("Select all"))
        self.all_select_btn.adjustSize()
        self.all_select_btn.move(self.all_check.x()-20-self.all_select_btn.width(),self.all_check.y())
        self.all_select_btn.hide()

        self.border = QLabel(self.btn_widget)
        self.border.setFixedSize(self.btn_widget.width(),1)
        self.border.move(0,self.type_name_label.y() + self.type_name_label.height() + 10)
        self.border.show()

        self.app_widget =  QWidget(self)
        self.app_widget.resize(self.width()-self.space*2,self.height()-self.btn_widget.height())
        self.app_widget.move(int((self.width()-self.app_widget.width())/2),int((self.height()-self.btn_widget.height()-self.app_widget.height())/2+self.btn_widget.height()))
        self.app_widget.setStyleSheet("QWidget{border-radius: 0px;}")
        self.app_widget.hide()

        self.scrollArea = QScrollArea(self)
        self.scrollArea.setWidget(self.app_widget)
        self.scrollArea.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self.scrollArea.horizontalScrollBar().setEnabled(False)
        self.scrollArea.setVerticalScrollBarPolicy(Qt.ScrollBarAsNeeded)
        self.scrollArea.setGeometry(int(self.btn_widget.x()+self.border.x()),int((self.height()-self.btn_widget.height()-self.app_widget.height())/2+self.btn_widget.height()),int(self.app_widget.width()+self.space-10),int(self.app_widget.height()-10))
        self.scrollArea.hide()
        self.scrollArea.verticalScrollBar().setStyleSheet("QScrollBar:vertical{margin:0px 0px 0px 0px;background-color:transparent;border:0px;width:6px;border-radius: 3px;}\
                     QScrollBar::handle:vertical{background-color:#C2C2C2;width:6px;border-radius:3px;} QScrollBar::handle:vertical:hover{background-color:#8F9399;opacity:0.1;width:6px;border-radius:3px;}  QScrollBar::handle:vertical:pressed{background-color:#8F9399;width:6px;border-radius:3px;}\
                     QScrollBar::sub-line:vertical{subcontrol-origin:margin;border:0px solid red;height:13px}\
                     QScrollBar::up-arrow:vertical{subcontrol-origin:margin;background-color:rgba(0,0,0,0);height:13px}\
                     QScrollBar::sub-page:vertical{background-color:transparent;}\
                     QScrollBar::add-page:vertical{background-color:transparent;}\
                     QScrollBar::down-arrow:vertical{background-color:rgba(0,0,0,0);}\
                     QScrollBar::add-line:vertical{subcontrol-origin:margin;border:0px solid green;height:13px}")

        self.downloading_widget = QWidget(self)
        self.downloading_widget.setAttribute(Qt.WA_TranslucentBackground)
        self.downloading_widget.resize(self.width()-self.space*2,self.height()-self.btn_widget.height())
        self.downloading_widget.move(int((self.width()-self.downloading_widget.width())/2),int((self.height()-self.btn_widget.height()-self.downloading_widget.height())/2+self.btn_widget.height()))
        self.downloading_widget.setStyleSheet("QWidget{border-radius: 0px;background-color:transparent;border:0px;}")
        self.downloading_widget.hide()

        self.text_prompt_label = QLabel(self)
        self.text_prompt_label.setAttribute(Qt.WA_TranslucentBackground)
        # self.text_prompt_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #BBBDCA;line-height: 24px;}")
        self.text_prompt_label.hide()

        self.no_app_tip_label = QLabel(self)
        self.no_app_tip_label.resize(240,240)
        if Globals.THEME_COLOR == 0:
            self.no_app_tip_label.setStyleSheet("QLabel{border-radius: 12px;background-image:url(./res/no_uninstall.png);}")
        else:
            self.no_app_tip_label.setStyleSheet("QLabel{border-radius: 12px;background-image:url(./res/no_uninstall_dark.png);}")
        self.no_app_tip_label.move(int((self.width()-self.no_app_tip_label.width())/2),int((self.height()-self.no_app_tip_label.height())/2))
        self.no_app_tip_label.hide()

        self.loadingmove = QMovie("./res/loadgif2.gif")
        self.loadingmove_dark = QMovie("./res/loadgifdark.gif")
        self.loadinglabel = QLabel(self)
        self.loadinglabel.resize( 350, 350)
        self.loadinglabel.move(int((self.width()-self.loadinglabel.width())/2),int((self.height()-self.loadinglabel.height())/2))
        self.loadinglabel.setAlignment(Qt.AlignCenter)
        if(Globals.THEME_COLOR == 0):
            self.loadinglabel.setMovie(self.loadingmove)
            self.loadingmove.start()
        else:
            self.loadinglabel.setMovie(self.loadingmove_dark)
            self.loadingmove_dark.start()

        self.loadinglabel.hide()
        #self.loadinglabel.show()

        self.scrollArea1 = QScrollArea(self)
        self.scrollArea1.setWidget(self.downloading_widget)
        self.scrollArea1.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self.scrollArea1.horizontalScrollBar().setEnabled(False)
        self.scrollArea1.setVerticalScrollBarPolicy(Qt.ScrollBarAsNeeded)
        self.scrollArea1.setGeometry(int(self.btn_widget.x()+self.border.x()),int((self.height()-self.btn_widget.height()-self.downloading_widget.height())/2+self.btn_widget.height()),int(self.downloading_widget.width()+self.space-10),int(self.downloading_widget.height()-10))
        self.scrollArea1.show()
        self.scrollArea1.verticalScrollBar().setStyleSheet("QScrollBar:vertical{margin:0px 0px 0px 0px;background-color:transparent;border:0px;width:6px;border-radius: 3px;}\
                     QScrollBar::handle:vertical{background-color:#C2C2C2;width:6px;border-radius:3px;} QScrollBar::handle:vertical:hover{background-color:#8F9399;opacity:0.1;width:6px;border-radius:3px;}  QScrollBar::handle:vertical:pressed{background-color:#8F9399;width:6px;border-radius:3px;}\
                     QScrollBar::sub-line:vertical{subcontrol-origin:margin;border:0px solid red;height:13px}\
                     QScrollBar::up-arrow:vertical{subcontrol-origin:margin;background-color:rgba(0,0,0,0);height:13px}\
                     QScrollBar::sub-page:vertical{background-color:transparent;}\
                     QScrollBar::add-page:vertical{background-color:transparent;}\
                     QScrollBar::down-arrow:vertical{background-color:rgba(0,0,0,0);}\
                     QScrollBar::add-line:vertical{subcontrol-origin:margin;border:0px solid green;height:13px}")

        self.history_widget = Historical_Insatallation_widget(self.width()-self.space*2,self.height()-self.btn_widget.height()-20,self)
        self.history_widget.move(int((self.width()-self.history_widget.width())/2),int((self.height()-self.btn_widget.height()-self.history_widget.height())/2+self.btn_widget.height()))
        self.history_widget.setStyleSheet("QWidget{border-radius: 0px;}")
        self.history_widget.hide()
        self.scrollArea2 = QScrollArea(self)
        self.scrollArea2.setWidget(self.history_widget)
        self.scrollArea2.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self.scrollArea2.horizontalScrollBar().setEnabled(False)
        self.scrollArea2.setVerticalScrollBarPolicy(Qt.ScrollBarAsNeeded)
        self.scrollArea2.setGeometry(int((self.width() - self.history_widget.width()) / 2), int((self.height() - self.btn_widget.height() - self.history_widget.height()) / 2 + self.btn_widget.height()),int(self.history_widget.width() + self.space - 10),self.history_widget.height())
        self.scrollArea2.verticalScrollBar().setStyleSheet("QScrollBar:vertical{margin:0px 0px 0px 0px;background-color:transparent;border:0px;width:6px;border-radius: 3px;}\
                             QScrollBar::handle:vertical{background-color:#C2C2C2;width:6px;border-radius:3px;} QScrollBar::handle:vertical:hover{background-color:#8F9399;opacity:0.1;width:6px;border-radius:3px;}  QScrollBar::handle:vertical:pressed{background-color:#8F9399;width:6px;border-radius:3px;}\
                             QScrollBar::sub-line:vertical{subcontrol-origin:margin;border:0px solid red;height:13px}\
                             QScrollBar::up-arrow:vertical{subcontrol-origin:margin;background-color:rgba(0,0,0,0);height:13px}\
                             QScrollBar::sub-page:vertical{background-color:transparent;}\
                             QScrollBar::add-page:vertical{background-color:transparent;}\
                             QScrollBar::down-arrow:vertical{background-color:rgba(0,0,0,0);}\
                             QScrollBar::add-line:vertical{subcontrol-origin:margin;border:0px solid green;height:13px}")
        self.scrollArea2.verticalScrollBar().valueChanged.connect(self.m_parent.mw.slot_scrollbar_valuechange)
        self.scrollArea2.hide()
        self.set_show_dark_mode()

    #
    #
    #
    def set_show_dark_mode(self):
        if(Globals.THEME_COLOR == 0):
            self.type_name_label.setStyleSheet(
                "QLabel{font-size: 24px;font-weight: 600;color: #303133;line-height: 42px;}")
            self.all_check.setStyleSheet(
                "QPushButton{border:0px;font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;border:0px solid #ff0000;}QPushButton:hover{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}QPushButton:pressed{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}")
            self.download_all_check.setStyleSheet(
                "QPushButton{border:0px;font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;border:0px solid #ff0000;}QPushButton:hover{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}QPushButton:pressed{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}")
            self.all_select_btn.setStyleSheet(
                "QPushButton{font-size: 14px;border:0px;font-weight: 500;color: #606265;line-height: 20px;}QPushButton:hover{background: rgba(55, 144, 250, 0.12);color: #3790FA;border-radius: 12px;}PushButton:pressed{background: rgba(55, 144, 250, 0.12);color: #3790FA;border-radius: 12px;}")
            self.text_prompt_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #BBBDCA;line-height: 24px;}")
            self.border.setStyleSheet("QLabel{background-color:rgba(0,0,0,0.08);}")
            self.scrollArea.verticalScrollBar().setStyleSheet("QScrollBar:vertical{margin:0px 0px 0px 0px;background-color:transparent;border:0px;width:6px;border-radius: 3px;}\
                                 QScrollBar::handle:vertical{background-color:#C2C2C2;width:6px;border-radius:3px;} QScrollBar::handle:vertical:hover{background-color:#8F9399;opacity:0.1;width:6px;border-radius:3px;}  QScrollBar::handle:vertical:pressed{background-color:#8F9399;width:6px;border-radius:3px;}\
                                 QScrollBar::sub-line:vertical{subcontrol-origin:margin;border:0px solid red;height:13px}\
                                 QScrollBar::up-arrow:vertical{subcontrol-origin:margin;background-color:rgba(0,0,0,0);height:13px}\
                                 QScrollBar::sub-page:vertical{background-color:transparent;}\
                                 QScrollBar::add-page:vertical{background-color:transparent;}\
                                 QScrollBar::down-arrow:vertical{background-color:rgba(0,0,0,0);}\
                                 QScrollBar::add-line:vertical{subcontrol-origin:margin;border:0px solid green;height:13px}")
            self.scrollArea1.verticalScrollBar().setStyleSheet("QScrollBar:vertical{margin:0px 0px 0px 0px;background-color:transparent;border:0px;width:6px;border-radius: 3px;}\
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
            self.loadinglabel.setMovie(self.loadingmove)
            self.loadingmove.start()
        else:
            self.type_name_label.setStyleSheet(
                "QLabel{font-size: 24px;font-weight: 600;color: #F9F9F9;line-height: 42px;}")
            self.all_check.setStyleSheet(
                "QPushButton{border:0px;font-size: 14px;font-weight: 500;color: #62666C;line-height: 20px;border:0px solid #ff0000;}QPushButton:hover{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}QPushButton:pressed{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}")
            self.download_all_check.setStyleSheet(
                "QPushButton{border:0px;font-size: 14px;font-weight: 500;color: #3790FA;line-height: 20px;border:0px solid #ff0000;}QPushButton:hover{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}QPushButton:pressed{background: rgba(55, 144, 250, 0.12);border-radius: 12px;}")
            self.all_select_btn.setStyleSheet(
                "QPushButton{font-size: 14px;border:0px;font-weight: 500;color: #F9F9F9;line-height: 20px;}QPushButton:hover{background: rgba(55, 144, 250, 0.12);color: #3790FA;border-radius: 12px;}PushButton:pressed{background: rgba(55, 144, 250, 0.12);color: #3790FA;border-radius: 12px;}")
            self.text_prompt_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 24px;}")
            self.border.setStyleSheet("QLabel{background-color:rgba(255,255,255,0.08);}")
            self.scrollArea.verticalScrollBar().setStyleSheet("QScrollBar:vertical{margin:0px 0px 0px 0px;background-color:transparent;border:0px;width:6px;border-radius: 3px;}\
                                 QScrollBar::handle:vertical{background-color:#6B6B6B;width:6px;border-radius:3px;} QScrollBar::handle:vertical:hover{background-color:#959595;opacity:0.1;width:6px;border-radius:3px;}  QScrollBar::handle:vertical:pressed{background-color:#959595;width:6px;border-radius:3px;}\
                                 QScrollBar::sub-line:vertical{subcontrol-origin:margin;border:0px solid red;height:13px}\
                                 QScrollBar::up-arrow:vertical{subcontrol-origin:margin;background-color:rgba(0,0,0,0);height:13px}\
                                 QScrollBar::sub-page:vertical{background-color:transparent;}\
                                 QScrollBar::add-page:vertical{background-color:transparent;}\
                                 QScrollBar::down-arrow:vertical{background-color:rgba(0,0,0,0);}\
                                 QScrollBar::add-line:vertical{subcontrol-origin:margin;border:0px solid green;height:13px}")
            self.scrollArea1.verticalScrollBar().setStyleSheet("QScrollBar:vertical{margin:0px 0px 0px 0px;background-color:transparent;border:0px;width:6px;border-radius: 3px;}\
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
            self.loadinglabel.setMovie(self.loadingmove_dark)
            self.loadingmove_dark.start()

    #
    #函数：重新设置界面的位置
    def re_move(self):
        self.btn_widget.setFixedSize(self.width() - self.space * 2, 114)
        self.btn_widget.move(int((self.width() - self.btn_widget.width()) / 2), 0)
        self.downloading_btn.move(int((self.btn_widget.width() - self.downloading_btn.width() * 4 - 12 * 3) / 2), 20)
        self.downloading_num_label.move(int((self.btn_widget.width() - self.downloading_btn.width() * 4 - 12 * 3) / 2) + self.downloading_btn.width() - 10,13)
        self.update_btn.move(self.downloading_btn.x() + self.downloading_btn.width() + 12, 20)
        self.update_tip_spot.move(self.update_btn.x() + self.update_btn.width() - 10, self.update_btn.y() - 3)
        self.update_num_label.move(
            self.downloading_btn.x() + self.update_btn.width() + 12 + self.update_btn.width() - 10, 13)
        self.uninstall_btn.move(self.update_btn.x() + self.update_btn.width() + 12, 20)
        self.installed_btn.move(self.uninstall_btn.x() + self.uninstall_btn.width() + 12, 20)
        self.type_name_label.move(0, self.installed_btn.y() + 18)
        self.all_check.move(self.btn_widget.width() - self.all_check.width(),self.type_name_label.y() + self.type_name_label.height() + 2 - self.all_check.height())
        self.download_all_check.move(self.btn_widget.width() - self.download_all_check.width(),
                                     self.type_name_label.y() + self.type_name_label.height() + 2 - self.download_all_check.height())
        self.all_select_btn.move(self.all_check.x() - 20 - self.all_select_btn.width(), self.all_check.y())
        self.border.setFixedSize(self.btn_widget.width(), 1)
        self.border.move(0, self.type_name_label.y() + self.type_name_label.height() + 10)
        self.app_widget.resize(self.width() - self.space * 2, self.height() - self.btn_widget.height())
        self.app_widget.move(int((self.width() - self.app_widget.width()) / 2), int((self.height() - self.btn_widget.height() - self.app_widget.height()) / 2 + self.btn_widget.height()))
        self.scrollArea.setGeometry(self.btn_widget.x() + self.border.x(), int((self.height() - self.btn_widget.height() - self.app_widget.height()) / 2 + self.btn_widget.height()),
                                    self.app_widget.width() + self.space - 10, self.app_widget.height() - 10)
        self.downloading_widget.resize(self.width() - self.space * 2, self.height() - self.btn_widget.height())
        self.downloading_widget.move(int((self.width() - self.downloading_widget.width()) / 2), int((self.height() - self.btn_widget.height() - self.downloading_widget.height()) / 2 + self.btn_widget.height()))
        self.no_app_tip_label.move(int((self.width() - self.no_app_tip_label.width()) / 2),
                                   int((self.height() - self.no_app_tip_label.height()) / 2))
        self.text_prompt_label.move(int((self.width() - self.text_prompt_label.width()) / 2),
                                             self.no_app_tip_label.y() + self.no_app_tip_label.height() + 10)
        self.loadinglabel.move(int((self.width() - self.loadinglabel.width()) / 2),
                               int((self.height() - self.loadinglabel.height()) / 2))
        self.scrollArea1.setGeometry(int(self.btn_widget.x() + self.border.x()), int((self.height() - self.btn_widget.height() - self.downloading_widget.height()) / 2 + self.btn_widget.height()),
                                     self.downloading_widget.width() + self.space - 10,
                                     self.downloading_widget.height() - 10)

        self.history_widget.setFixedSize(self.width() - self.space * 2, self.height() - self.btn_widget.height()-20)
        self.scrollArea2.setGeometry(int(self.btn_widget.x() + self.border.x()), int((self.height() - self.btn_widget.height() - self.app_widget.height()) / 2 + self.btn_widget.height()),
                                    self.history_widget.width() + self.space - 10, self.history_widget.height())
        self.history_widget.re_move()
        # if(self.type_name_label.text() == _("Onload")):
        self.reload_card(self.downloading_widget,self.scrollArea1)
        if(self.type_name_label.text() == _("Installation")):
            self.reload_card(self.history_widget.app_card_widget, self.history_widget)
            if(len(self.history_widget.app_card_widget.children()) == 0):
                pass
            else:
                self.history_widget.setFixedHeight(self.history_widget.label_local.y() + self.history_widget.label_local.height() + self.history_widget.app_card_widget.height() + 30)
        else:
            self.reload_card(self.app_widget, self.scrollArea)


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
                        card.move(num * (card.width() + self.card_space) , 20 + (20 + card.height()) * v_num)
                    num = num + 1
                else:
                    num = 0
                    v_num = v_num + 1
                    if (num == 0):
                        card.move(0, 20 + (20 + card.height()) * v_num)
                        widget.resize(widget.width(),  (20 + card.height()) * (v_num + 1))
                    else:
                        card.move(num * (card.width() + self.card_space), 20 + (20 + card.height()) * v_num)
                    num = num + 1

    #
    # 函数：处理点击分类界面的类型按钮
    #
    def slot_clicked_btn(self, btn_name):
        self.click_type_btn.emit(btn_name)

    #
    # 函数：绘制btn按钮
    # 当返回False时表示没有其他按钮是按下状态，所以这个按钮不能取消按下状态
    #
    def paint_btn(self):
        if (self.downloading_btn.is_clicked == True):
            self.downloading_btn.is_clicked = False
            self.downloading_btn.leave_btn.emit(0)
        elif (self.update_btn.is_clicked == True):
            self.update_btn.is_clicked = False
            self.update_btn.leave_btn.emit(0)
        elif (self.uninstall_btn.is_clicked == True):
            self.uninstall_btn.is_clicked = False
            self.uninstall_btn.leave_btn.emit(0)
        elif (self.installed_btn.is_clicked == True):
            self.installed_btn.is_clicked = False
            self.installed_btn.leave_btn.emit(0)
        else:
            return False
        return True

    #
    # 函数名:设置滚动条起始位置
    # Function:set verticalSscrollBar starting point
    #
    def scrollToTop(self):
        vsb = self.scrollArea.verticalScrollBar()
        vsb.setValue(0)

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
