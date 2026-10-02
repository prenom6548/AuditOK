import os
import math
from PyQt5.QtGui import *
from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
from ui.systembutton import *
from ui.show_ratings import *
from models.enums import CACHE_DIR,BUTTON_PRESSED,BUTTON_ENTER,BUTTON_LEAVE,BUTTON_DISABLE
from utils import commontools
import gettext
gettext.bindtextdomain("kylin-software-center", "/usr/share/locale")
gettext.textdomain("kylin-software-center")
_ = gettext.gettext

class Comment_Pop_Up(QWidget):
    change_background_image = pyqtSignal()
    submit_my_rating_apk = pyqtSignal(int, ApkInfo)
    submit_my_rating_app = pyqtSignal(int, Application)
    def __init__(self,app_msg,my_rating,app_detail,parent = None):
        QWidget.__init__(self, parent)
        self.mouse_press = False
        self.appmsg = app_msg
        self.app_detail = app_detail
        self.MY_SCORE = my_rating
        self.star_list = []
        self.app_name = ""
        self.pre_app_name = ""
        self.init_ui()
        self.paintEvent = self.set_paintEvent
        self.submit_my_rating_apk.connect(self.app_detail.m_parent.mw.slot_submit_my_rating)
        self.submit_my_rating_app.connect(self.app_detail.m_parent.mw.slot_submit_my_rating)
        self.timer = QTimer(self)
        self.timer.timeout.connect(self.slot_timer_timeout)

    def init_ui(self):
        self.setFixedSize(442,317)
        self.setWindowFlags(Qt.FramelessWindowHint | Qt.Tool)
        self.setWindowTitle(_("Comment"))
        self.setAttribute(Qt.WA_TranslucentBackground)
        self.widget = QWidget(self)
        self.widget.resize(424,299)
        self.widget.move(9,9)

        self.title_label = QLabel(self.widget)
        self.title_label.setText(_("Comment"))
        if(Globals.THEME_COLOR == 0):
            self.title_label.setStyleSheet("QLabel{font-size: 14px;color: #1B1B1B;font-weight: 500;line-height: 22px}")
        else:
            self.title_label.setStyleSheet("QLabel{font-size: 14px;color: #8F9399;font-weight: 500;line-height: 14px;background:transparent;}")
        self.title_label.move(16,8)

        self.close_btn = QPushButton(self.widget)
        self.close_btn.setFocusPolicy(Qt.NoFocus)
        self.close_btn.resize(30, 30)
        self.close_btn.setStyleSheet(
            "QPushButton{border-radius: 4px;border:0px;border-image:url('res/close1.png')}QPushButton:hover{border:0px;border-image:url('res/close2.png')}QPushButton:pressed{border:0px;border-image:url('res/close3.png')}")
        self.close_btn.move(self.widget.width() - self.close_btn.width() - 4, 4)
        self.close_btn.setToolTip(_("close"))

        self.line_label = QLabel(self.widget)
        self.line_label.setFixedSize(422, 1)
        self.line_label.setStyleSheet("QLabel{width: 424px;height: 1px;background: #F2F6FD;}")
        self.line_label.move(1, 38)

        self.icon_label = QLabel(self.widget)
        self.icon_label.resize(60,60)
        self.icon_label.move(16,55)
        iconpath = commontools.get_big_icon_path(self.appmsg.pkgname)
        # self.icon_label.setStyleSheet("QLabel{border-radius: 12px;border-image: url('"+iconpath+"') 0 0 0 0 stretch strectch;background-color:transparent;}")
        self.icon_label.setStyleSheet("QLabel{border-radius: 12px;background-color:transparent;}")
        self.pix2 = QPixmap(iconpath)
        self.fitpixmap_userIcon = self.pix2.scaled(60, 60, Qt.IgnoreAspectRatio,
                                                   Qt.SmoothTransformation)
        self.fitpixmap_userIcon = self.PixmapToRound(self.fitpixmap_userIcon, 60, 60)
        self.icon_label.setPixmap(self.fitpixmap_userIcon)
        self.icon_label.setScaledContents(True)

        self.star1 = start_btn(1, "small",self)
        self.star1.move(self.icon_label.x() + self.icon_label.width()+18, self.icon_label.y()+self.widget.y())
        self.star_list.append(self.star1)
        self.star2 = start_btn(2, "small",self)
        self.star2.move(self.star1.x() + self.star1.width() + 10, self.star1.y())
        self.star_list.append(self.star2)
        self.star3 = start_btn(3, "small",self)
        self.star3.move(self.star2.x() + self.star2.width() + 10, self.star1.y())
        self.star_list.append(self.star3)
        self.star4 = start_btn(4, "small",self)
        self.star4.move(self.star3.x() + self.star3.width() + 10, self.star1.y())
        self.star_list.append(self.star4)
        self.star5 = start_btn(5, "small",self)
        self.star5.move(self.star4.x() + self.star4.width() + 10, self.star1.y())
        self.star_list.append(self.star5)

        self.editorial_comments_widget = QFrame(self.widget)
        self.editorial_comments_widget.resize(316, 100)
        self.editorial_comments_widget.move(self.icon_label.x()+self.icon_label.width()+16,self.star1.y()+self.star1.height()+19)
        if (Globals.THEME_COLOR == 0):
            self.editorial_comments_widget.setStyleSheet(
                "QFrame{border-radius: 4px;border: 1px solid #C0C4CC;}QFrame:hover{border-radius: 4px;border: 1px solid #C0C4CC;}")
        else:
            self.editorial_comments_widget.setStyleSheet(
                "QFrame{border-radius: 4px;border: 1px solid #3D3D41;}QFrame:hover{border-radius: 4px;border: 1px solid #3D3D41;}")

        self.submit_btn = QPushButton(self.widget)
        self.submit_btn.resize(64, 30)
        self.submit_btn.setStyleSheet(
            "QPushButton{background: #3790FA;border-radius: 6px;font-size: 14px;font-weight: 400;color: #FFFFFF;text-align: center;line-height: 12px;}QPushButton:hover{background: #40A9FB;;border-radius: 6px;color: #ffffff;}QPushButton:pressed{background: #296CD9;border-radius: 6px;color: #ffffff;}")
        self.submit_btn.setText(_("submit"))
        self.submit_btn.move(self.widget.width() - self.submit_btn.width() - 16,
                             self.editorial_comments_widget.y() + self.editorial_comments_widget.height() + 36)
        self.submit_btn.clicked.connect(self.app_detail.submit_my_review)

        self.comment_edit_label = QTextEdit(self.editorial_comments_widget)
        self.comment_edit_label.resize(314,74)
        self.comment_edit_label.setStyleSheet("QTextEdit{border:0px;font-size: 14px;font-weight: 400;color: #606265;line-height: 20px;}")

        # self.comment_edit_label.verticalScrollBar().setStyleSheet("QScrollBar:vertical{margin:0px 0px 0px 0px;background-color:transparent;border:0px;width:6px;border-radius: 3px;}\
        #                      QScrollBar::handle:vertical{background-color:#C2C2C2;width:6px;border-radius:3px;border:0px;}QScrollBar::handle:vertical:hover{background-color:#8F9399;opacity:0.1;width:6px;border-radius:3px;border:0px;}QScrollBar::handle:vertical:pressed{border:0px;background-color:#8F9399;width:6px;border-radius:3px;}\
        #                      QScrollBar::sub-line:vertical{subcontrol-origin:margin;border:0px solid red;height:13px}\
        #                      QScrollBar::up-arrow:vertical{subcontrol-origin:margin;background-color:rgba(0,0,0,0);height:13px;border:0px}\
        #                      QScrollBar::sub-page:vertical{background-color:transparent;border:0px}\
        #                      QScrollBar::add-page:vertical{background-color:transparent;border:0px}\
        #                      QScrollBar::down-arrow:vertical{background-color:rgba(0,0,0,0);border:0px}\
        #                      QScrollBar::add-line:vertical{subcontrol-origin:margin;border:0px solid green;height:13px}")
        self.comment_edit_label.textChanged.connect(self.slot_comment_edit_label_changed)
        self.comment_edit_label.setText("")
        self.comment_edit_label.move(1,1)

        self.record_comment_length_label = QLabel(self.editorial_comments_widget)
        self.record_comment_length_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 22px;border: 0px;}")
        self.record_comment_length_label.setText("0/200")
        self.record_comment_length_label.adjustSize()
        self.record_comment_length_label.move(self.editorial_comments_widget.width()-self.record_comment_length_label.width()-8,self.editorial_comments_widget.height()-self.record_comment_length_label.height()-4)

        self.tip_label = QLabel(self.widget)
        self.tip_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #F72213;line-height: 24px;}")
        self.tip_label.setText(_("Please rate before commenting"))
        self.tip_label.adjustSize()
        self.tip_label.move(self.editorial_comments_widget.x(),self.editorial_comments_widget.y()+self.editorial_comments_widget.height()+4)
        self.tip_label.hide()

        self.time_tip = QLabel(self.editorial_comments_widget)
        self.time_tip.setStyleSheet(
            "QLabel{background-color:transparent;font-size:14px;font-weight: 400;color: #F56C6C;line-height: 20px;border:none;}")
        self.time_tip.resize(349, 20)

        self.time_tip.setFocusPolicy(Qt.NoFocus)
        self.time_tip.hide()


        self.cancel_btn = QPushButton(self.widget)
        self.cancel_btn.resize(64,30)
        self.cancel_btn.setStyleSheet("QPushButton{font-size: 14px;font-weight: 400;color: #8F9399;line-height: 20px;border-radius: 6px;text-align: center;border: 1px solid #DDDFE7;}QPushButton:hover{background: #F2F6FD;border-radius: 6px;border: 1px solid #DDDFE7;}QPushButton:pressed{background: #F2F6FD;border-radius: 6px;border: 1px solid #DDDFE7;}")
        self.cancel_btn.setText(_("cancel"))
        self.cancel_btn.move(self.submit_btn.x()-self.cancel_btn.width()-16,self.submit_btn.y())
        if(Globals.LOGIN_SUCCESS):
            self.slot_click_star(self.MY_SCORE,False)
        self.change_star_state()
        self.slot_show_dark_mode()

        self.show()


    def slot_comment_edit_label_changed(self):
        maxLength = 200
        strText = (self.comment_edit_label.toPlainText())
        length = self.comment_edit_label.document().characterCount()
        if(strText == ""):
            if(hasattr(self,"tip_label")):
                self.tip_label.hide()
            self.submit_btn.setEnabled(False)
            self.submit_btn.setStyleSheet("QPushButton{background: #E9E9E9;border-radius: 6px;font-size: 14px;font-weight: 400;color: #B3B3B3;line-height: 20px;}")
            if(hasattr(self,"record_comment_length_label")):
                self.record_comment_length_label.setText("0/200")
                self.record_comment_length_label.setStyleSheet(
                    "QLabel{font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 22px;border: 0px;}")
                self.record_comment_length_label.adjustSize()
                self.record_comment_length_label.move(
                    self.editorial_comments_widget.width() - self.record_comment_length_label.width() - 8,
                    self.editorial_comments_widget.height() - self.record_comment_length_label.height() - 4)
        else:
            self.submit_btn.setEnabled(True)
            self.submit_btn.setStyleSheet(
                "QPushButton{background: #3790FA;border-radius: 6px;font-size: 14px;font-weight: 400;color: #FFFFFF;text-align: center;line-height: 12px;}QPushButton:hover{background: #40A9FB;;border-radius: 6px;color: #ffffff;}QPushButton:pressed{background: #296CD9;border-radius: 6px;color: #ffffff;}")
            if length-1 > maxLength:

                strText = strText[:-1]
                set_text = strText[0:200]
                self.comment_edit_label.clear()
                self.comment_edit_label.setText(set_text)
                tc = self.comment_edit_label.textCursor()
                tc.movePosition(QTextCursor.End)
                self.comment_edit_label.setTextCursor(tc)
                self.record_comment_length_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #F72213;line-height: 24px;border: 0px;}")
                # self.editorial_comments_widget.setStyleSheet(
                #     "QFrame{border-radius: 12px;border: 2px solid #F56C6C;}QFrame:hover{border-radius: 12px;border: 2px solid #F56C6C;}")
            else:
                self.record_comment_length_label.setText(str(length-1)+"/200")
                self.record_comment_length_label.setStyleSheet(
                    "QLabel{font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 22px;border: 0px;}")
                self.record_comment_length_label.adjustSize()
                self.record_comment_length_label.move(
                    self.editorial_comments_widget.width() - self.record_comment_length_label.width() - 8,
                    self.editorial_comments_widget.height() - self.record_comment_length_label.height() - 4)

                if (hasattr(self, "tip_label")):
                    self.tip_label.hide()


    def slot_enter_star(self,num):
        n = 0
        if(num == -1):
            if (Globals.LOGIN_SUCCESS):
                num = self.MY_SCORE
            else:
                num = 0
        for star in self.star_list:
            if(n < num):
                self.star_list[n].setStyleSheet("QPushButton{background-image: url(./res/medium_bright_star.png);border:0px;}")
            else:
                self.star_list[n].setStyleSheet("QPushButton{background-image: url(./res/medium_dark_star.png);border:0px;}")
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
                        self.star_list[n].setStyleSheet("QPushButton{background-image: url(./res/medium_bright_star.png);border:0px;}")
                    else:
                        self.star_list[n].setStyleSheet("QPushButton{background-image: url(./res/medium_dark_star.png);border:0px;}")
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
                        self.star_list[n].setStyleSheet("QPushButton{background-image: url(./res/medium_dark_star.png);border:0px;}")
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
            self.star_list[n].setStyleSheet("QPushButton{background-image: url(./res/medium_dark_star.png);border:0px;}")
            n = n + 1

    def change_star_state(self):
        n = 0
        for star in self.star_list:
            self.star_list[n].mouse_press = False
            n = n + 1

    #
    # 函数：处理计时器超时
    #
    def slot_timer_timeout(self):
        self.timer.stop()
        self.pre_app_name = ""

    def PixmapToRound(self,src,x,y):
        pixmapa = QPixmap(src)
        pixmap = QPixmap(x, y)
        pixmap.fill(Qt.transparent)
        painter = QPainter(pixmap)
        painter.setRenderHint(QPainter.Antialiasing, True)
        # painter.setRenderHints(QPainter.Antialiasing | QPainter.SmoothPixmapTransform)
        path = QPainterPath()
        rect = QRectF(0,0,x,y)
        path.addRoundedRect(rect, 12.0, 12.0)
        painter.setClipPath(path)
        painter.drawPixmap(0, 0, x, y, pixmapa)

        return pixmap

    def slot_show_dark_mode(self):
        if (Globals.THEME_COLOR == 0):
            if(Globals.RIGHT_ANGLE):
                self.widget.setStyleSheet(".QWidget{background-color:#ffffff;border: 1px solid rgba(38, 38, 38, 0.15);}")
                self.setStyleSheet("QFrame{background-color:#ffffff;}")
            else:
                self.widget.setStyleSheet(".QWidget{border-radius:12px;background-color:#ffffff;border: 1px solid rgba(38, 38, 38, 0.15);}")
                self.setStyleSheet("QFrame{background-color:#ffffff;}")
            self.title_label.setStyleSheet("QLabel{font-size: 14px;color: #1B1B1B;font-weight: 500;background:transparent;}")
            self.line_label.setStyleSheet("QLabel{width: 424px;height: 1px;background: #F2F6FD;}")
            self.close_btn.setStyleSheet("QPushButton{border-radius: 4px;border:0px;border-image:url('res/close1.png')}QPushButton:hover{border:0px;border-image:url('res/close2.png')}QPushButton:pressed{border:0px;border-image:url('res/close3.png')}")
            self.editorial_comments_widget.setStyleSheet(
                "QFrame{border-radius: 4px;border: 1px solid #C0C4CC;}QFrame:hover{border-radius: 4px;border: 1px solid #C0C4CC;}")
        else:
            if (Globals.RIGHT_ANGLE):
                self.widget.setStyleSheet(".QWidget{background-color:#3D3D41;border: 1px solid rgba(38, 38, 38, 0.15);}")
                self.setStyleSheet("QFrame{background-color:#1F2022;}")
            else:
                self.setStyleSheet("QFrame{background-color:#1F2022;}")
                self.widget.setStyleSheet(".QWidget{border-radius:12px;background-color:#3D3D41;border: 1px solid rgba(38, 38, 38, 0.15);}")
            self.title_label.setStyleSheet("QLabel{font-size: 14px;color: #8F9399;line-height: 14px;font-weight: 500;background:transparent;}")
            self.line_label.setStyleSheet("QLabel{width: 424px;height: 1px;background: #48484C;}")
            self.close_btn.setStyleSheet(
                "QPushButton{border-radius: 4px;border:0px;border-image:url('res/common-close-white.png')}QPushButton:hover{border:0px;border-image:url('res/hover-close.png')}QPushButton:pressed{border:0px;border-image:url('res/clicked-close.png')}")
            self.editorial_comments_widget.setStyleSheet(
                "QFrame{border-radius: 4px;border: 1px solid #3D3D41;}QFrame:hover{border-radius: 4px;border: 1px solid #3D3D41;}")

    #
    # 函数：重绘函数
    #
    def set_paintEvent(self, event):
        painter = QPainter(self)
        m_defaultBackgroundColor = QColor(qRgba(0, 0, 0, 16))
        m_defaultBackgroundColor.setAlpha(50)
        path = QPainterPath()
        path.setFillRule(Qt.WindingFill)
        path.addRoundedRect(10, 10, self.width() - 20, self.height() - 20, 12, 12)

        painter.setRenderHint(QPainter.Antialiasing, True)
        painter.fillPath(path, QBrush(QColor(m_defaultBackgroundColor.red(),
                                             m_defaultBackgroundColor.green(),
                                             m_defaultBackgroundColor.blue())))

        color = QColor(0, 0, 0, 16)
        i = 0
        while i < 4:
            path = QPainterPath()
            path.setFillRule(Qt.WindingFill)
            path.addRoundedRect(10 - i, 10 - i, self.width() - (10 - i) * 2, self.height() - (10 - i) * 2, 12, 12)
            color.setAlpha(int(100 - math.sqrt(i) * 50))
            painter.setPen(color)
            painter.drawPath(path)
            i = i + 1

        painter.setRenderHint(QPainter.Antialiasing)

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

    # def mouseMoveEvent(self, event):
    #     if (self.mouse_press):
    #         self.move_pos = event.globalPos()
    #         self.move(self.move_pos - self.drag_pos)
    #         self.setWindowOpacity(1)
    #         event.accept()
