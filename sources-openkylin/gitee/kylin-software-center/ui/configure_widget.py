import os
import math
from PyQt5.QtGui import *
from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
from ui.systembutton import *
from models.enums import CACHE_DIR,BUTTON_PRESSED,BUTTON_ENTER,BUTTON_LEAVE,BUTTON_DISABLE
import gettext
gettext.bindtextdomain("kylin-software-center", "/usr/share/locale")
gettext.textdomain("kylin-software-center")
_ = gettext.gettext

class Configure_Widget(QWidget):
    def __init__(self,parent = None):
        QWidget.__init__(self, parent)
        self.mouse_press = False
        self.mw = parent
        self.init_ui()
        self.paintEvent = self.set_paintEvent

    def init_ui(self):
        self.setFixedSize(442,240)
        self.setWindowFlags(Qt.FramelessWindowHint | Qt.Tool)
        self.setWindowTitle(_("Software Store"))
        self.setAttribute(Qt.WA_TranslucentBackground)
        self.widget = QWidget(self)
        self.widget.resize(424,222)
        self.widget.move(9,9)
        #self.widget.setStyleSheet("QWidget{background: rgba(255, 255, 255, 0.7);border-radius:12px;background-color:#ffffff;}")
        self.title_label = QLabel(self.widget)
        self.title_label.setFixedSize(170,16)
        self.title_label.setText(_("Server address"))
        self.title_label.setStyleSheet("QLabel{border:0px;font-size: 14px;font-weight: 500;color: #1B1B1B;line-height: 14px;}")
        self.title_label.move(16,12)

        self.line_label = QLabel(self.widget)
        self.line_label.setFixedSize(422, 1)
        self.line_label.setStyleSheet("QLabel{width: 424px;height: 1px;background: #F2F6FD;}")
        self.line_label.move(1, 38)

        self.close_btn = QPushButton(self.widget)
        self.close_btn.setFocusPolicy(Qt.NoFocus)
        self.close_btn.resize(30,30)
        self.close_btn.setStyleSheet("QPushButton{border-radius: 4px;border:0px;border-image:url('res/close1.png')}QPushButton:hover{border:0px;border-image:url('res/close2.png')}QPushButton:pressed{border:0px;border-image:url('res/close3.png')}")
        self.close_btn.move(self.widget.width()-self.close_btn.width()-4,4)
        self.close_btn.installEventFilter(self)
        self.close_btn.setToolTip(_("close"))

        self.software_label = QLabel(self.widget)
        self.software_label.setWordWrap(True)
        self.software_label.setAlignment(Qt.AlignLeft)
        self.software_label.setText(_("Software Store")+":")
        self.software_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #606265;line-height: 20px;background:transparent;}")
        self.software_label.adjustSize()
        self.software_label.move(16,self.line_label.y()+self.line_label.height()+22)

        self.service_dress_label = QLineEdit(self.widget)
        self.service_dress_label.installEventFilter(self)
        if(Globals.LAN == "zh_CN"):
            self.service_dress_label.resize(324,32)
        else:
            self.service_dress_label.resize(280, 32)
        # self.service_dress_label.setStyleSheet(
        #     "QLineEdit{font-size: 14px;font-family: PingFangSC-Regular, PingFang SC;font-weight: 400;color: #1B1B1B;line-height: 24px;background:#E6E6E6;border: 1px solid ##E6E6E6;border-radius: 4px;}")
        # self.service_dress_label.setPlaceholderText(_("Please input the server address of the software store"))
        self.service_dress_label.move(self.software_label.x()+self.software_label.width()+8,self.line_label.y()+self.line_label.height()+16)

        self.service_dress_back_label = QLabel(self.widget)
        self.service_dress_back_label.setText(_("Please input the server address of the software store"))
        self.service_dress_back_label.setStyleSheet(
            "QLabel{border:0px;font-size: 14px;font-weight: 400;color: #8F9399;line-height: 16px;border:0px;background-color: transparent;}")
        # self.search_line.setMaxLength(30)
        self.service_dress_back_label.setAttribute(Qt.WA_TranslucentBackground)
        self.service_dress_back_label.setEnabled(False)
        self.service_dress_back_label.setFixedHeight(20)
        self.service_dress_back_label.setMaximumWidth(self.service_dress_label.width()-10)
        self.service_dress_back_label.adjustSize()
        self.service_dress_back_label.move(self.service_dress_label.x() + 8, self.service_dress_label.y() + int(
            (self.service_dress_label.height() - self.service_dress_back_label.height()) / 2))
        self.service_dress_label.raise_()

        self.tip_label = QLabel(self.widget)
        self.tip_label.setText(_("Please enter the correct server address"))
        self.tip_label.setStyleSheet("QLabel{font-size: 12px;font-weight: 400;color: #F56C6C;line-height: 17px;}")
        self.tip_label.adjustSize()
        self.tip_label.move(self.service_dress_label.x(),self.service_dress_label.height()+self.service_dress_label.y()+4)
        self.tip_label.hide()

        self.kylin_id_label = QLabel(self.widget)
        self.kylin_id_label.setWordWrap(True)
        self.kylin_id_label.resize(self.software_label.width(),self.software_label.height())
        self.kylin_id_label.setAlignment(Qt.AlignRight)
        self.kylin_id_label.setText(_("Kylin ID") + ":")
        self.kylin_id_label.setStyleSheet(
            "QLabel{font-size: 14px;font-weight: 400;color: #606265;line-height: 20px;background:transparent;}")
        # self.kylin_id_label.adjustSize()
        self.kylin_id_label.move(16, self.line_label.y() + self.line_label.height() + 79)

        self.kylinid_dress_label = QLineEdit(self.widget)
        self.kylinid_dress_label.installEventFilter(self)
        if (Globals.LAN == "zh_CN"):
            self.kylinid_dress_label.resize(324, 32)
        else:
            self.kylinid_dress_label.resize(280, 32)
        # self.kylinid_dress_label.setStyleSheet("QLineEdit{font-size: 14px;font-family: PingFangSC-Regular, PingFang SC;font-weight: 400;color: #1B1B1B;line-height: 24px;background:#E6E6E6;border: 1px solid ##E6E6E6;border-radius: 4px;}")
        # self.kylinid_dress_label.setPlaceholderText(_("Please input the server address of the Kylin ID"))
        self.kylinid_dress_label.move(self.kylin_id_label.x() + self.kylin_id_label.width() + 8,
                                      self.line_label.y() + self.line_label.height() + 73)

        self.kylinid_dress_back_label = QLabel(self.widget)
        self.kylinid_dress_back_label.setText(_("Please input the server address of the software store"))
        self.kylinid_dress_back_label.setStyleSheet(
            "QLabel{border:0px;font-size: 14px;font-weight: 400;color: #8F9399;line-height: 16px;border:0px;background-color: transparent;}")
        # self.search_line.setMaxLength(30)
        self.kylinid_dress_back_label.setAttribute(Qt.WA_TranslucentBackground)
        self.kylinid_dress_back_label.setEnabled(False)
        self.kylinid_dress_back_label.setMaximumWidth(self.kylinid_dress_label.width() - 10)
        self.kylinid_dress_back_label.setFixedHeight(20)
        self.kylinid_dress_back_label.adjustSize()
        self.kylinid_dress_back_label.move(self.kylinid_dress_label.x() + 8, self.kylinid_dress_label.y() + int((self.kylinid_dress_label.height()-self.kylinid_dress_back_label.height())/2))
        self.kylinid_dress_label.raise_()

        self.tip_label2 = QLabel(self.widget)
        self.tip_label2.setText(_("Please enter the correct server address"))
        self.tip_label2.setStyleSheet("QLabel{font-size: 12px;font-weight: 400;color: #F56C6C;line-height: 17px;}")
        self.tip_label2.adjustSize()
        self.tip_label2.move(self.kylinid_dress_label.x(),
                            self.kylinid_dress_label.height() + self.kylinid_dress_label.y() + 4)
        self.tip_label2.hide()

        self.btn = QPushButton(self.widget)
        self.btn.setFocusPolicy(Qt.NoFocus)
        self.btn.setFixedSize(64,30)
        self.btn.setStyleSheet("QPushButton{background: #3790FA;border-radius: 6px;font-size: 14px;font-weight: 400;color: #FFFFFF;text-align: center;line-height: 12px;}QPushButton:hover{background: #40A9FB;;border-radius: 6px;color: #ffffff;}QPushButton:pressed{background: #296CD9;border-radius: 6px;color: #ffffff;}")
        self.btn.setText(_("OK"))
        self.btn.move(self.widget.width() - self.btn.width() - 16,self.line_label.y() + self.line_label.height() + 129)

        self.cancel_btn = QPushButton(self.widget)
        self.cancel_btn.setFocusPolicy(Qt.NoFocus)
        self.cancel_btn.setFixedSize(64, 30)
        self.cancel_btn.setStyleSheet("QPushButton{font-size: 14px;font-weight: 400;color: #8F9399;line-height: 20px;border-radius: 6px;text-align: center;border: 1px solid #DDDFE7;}QPushButton:hover{background: #F2F6FD;border-radius: 6px;border: 1px solid #DDDFE7;}QPushButton:pressed{background: #F2F6FD;border-radius: 6px;border: 1px solid #DDDFE7;}")
        self.cancel_btn.setText(_("cancel"))
        self.cancel_btn.move(self.btn.x() - self.cancel_btn.width() - 16, self.btn.y())
        self.cancel_btn.show()

        self.slot_show_dark_mode()

    #
    # 函数：处理主题颜色的显示
    #
    def slot_show_dark_mode(self):
        if (Globals.THEME_COLOR == 0):
            if(Globals.RIGHT_ANGLE):
                self.widget.setStyleSheet(".QWidget{background-color:#ffffff;border: 1px solid rgba(38, 38, 38, 0.15);}")
                self.setStyleSheet("QFrame{background-color:#ffffff;}")
            else:
                self.widget.setStyleSheet(".QWidget{border-radius:12px;background-color:#ffffff;border: 1px solid rgba(38, 38, 38, 0.15);}")
                self.setStyleSheet("QFrame{background-color:#ffffff;}")
            self.title_label.setStyleSheet("QLabel{font-size: 14px;color: #1B1B1B;font-weight: 500;background:transparent;}")
            self.software_label.setStyleSheet(
                "QLabel{font-size: 14px;font-weight: 400;color: #606265;line-height: 20px;background:transparent;}")
            self.kylin_id_label.setStyleSheet(
                "QLabel{font-size: 14px;font-weight: 400;color: #606265;line-height: 20px;background:transparent;}")
            self.service_dress_label.setStyleSheet(
                "QLineEdit{font-size: 14px;font-weight: 400;color: #1B1B1B;line-height: 24px;background:#E6E6E6;border: 1px solid #E6E6E6;border-radius: 6px;}QLineEdit:hover{width: 160px;height: 36px;border-radius: 6px;border: 1px solid #3790FA;background:#E6E6E6;}QLineEdit:pressed{width: 160px;height: 36px;border-radius: 6px;border: 2px solid #3790FA;background:transparent;}")
            self.kylinid_dress_label.setStyleSheet(
                "QLineEdit{font-size: 14px;font-weight: 400;color: #1B1B1B;line-height: 24px;background:#E6E6E6;border: 1px solid #E6E6E6;border-radius: 6px;}QLineEdit:hover{width: 160px;height: 36px;border-radius: 6px;border: 1px solid #3790FA;background:#E6E6E6;}QLineEdit:pressed{width: 160px;height: 36px;border-radius: 6px;border: 2px solid #3790FA;background:transparent;}")
            self.line_label.setStyleSheet("QLabel{width: 424px;height: 1px;background: #F2F6FD;}")
            self.service_dress_back_label.setStyleSheet(
                "QLabel{border:0px;font-size: 14px;font-weight: 400;color: #8F9399;line-height: 16px;border:0px;background-color: transparent;}")
            self.kylinid_dress_back_label.setStyleSheet(
                "QLabel{border:0px;font-size: 14px;font-weight: 400;color: #8F9399;line-height: 16px;border:0px;background-color: transparent;}")
            self.close_btn.setStyleSheet("QPushButton{border-radius: 4px;border:0px;border-image:url('res/close1.png')}QPushButton:hover{border:0px;border-image:url('res/close2.png')}QPushButton:pressed{border:0px;border-image:url('res/close3.png')}")
        else:
            if (Globals.RIGHT_ANGLE):
                self.widget.setStyleSheet(".QWidget{background-color:#3D3D41;border: 1px solid rgba(38, 38, 38, 0.15);}")
                self.setStyleSheet("QFrame{background-color:#1F2022;}")
            else:
                self.setStyleSheet("QFrame{background-color:#1F2022;}")
                self.widget.setStyleSheet(".QWidget{border-radius:12px;background-color:#3D3D41;border: 1px solid rgba(38, 38, 38, 0.15);}")
            self.title_label.setStyleSheet("QLabel{font-size: 14px;color: #8F9399;font-weight: 500;line-height: 14px;background:transparent;}")
            self.software_label.setStyleSheet(
                "QLabel{font-size: 14px;font-weight: 400;color: #8F9399;line-height: 20px;background:transparent;}")
            self.kylin_id_label.setStyleSheet(
                "QLabel{font-size: 14px;font-weight: 400;color: #8F9399;line-height: 20px;background:transparent;}")
            self.line_label.setStyleSheet("QLabel{width: 424px;height: 1px;background: #48484C;}")
            self.service_dress_back_label.setStyleSheet(
                "QLabel{border:0px;font-size: 14px;font-weight: 400;color: #8F9399;line-height: 16px;border:0px;background-color: transparent;}")
            self.kylinid_dress_back_label.setStyleSheet(
                "QLabel{border:0px;font-size: 14px;font-weight: 400;color: #8F9399;line-height: 16px;border:0px;background-color: transparent;}")
            self.service_dress_label.setStyleSheet(
                "QLineEdit{font-size: 14px;font-weight: 400;color: #FFFFFF;line-height: 24px;background:#373737;border: 1px solid #373737;border-radius: 6px;}QLineEdit:hover{width: 160px;height: 36px;border-radius: 6px;border: 1px solid #3790FA;background:transparent;}QLineEdit:pressed{width: 160px;height: 36px;border-radius: 6px;border: 2px solid #3790FA;background:transparent;}")
            self.kylinid_dress_label.setStyleSheet(
                "QLineEdit{font-size: 14px;font-weight: 400;color: #FFFFFF;line-height: 24px;background:#373737;border: 1px solid #373737;border-radius: 6px;}QLineEdit:hover{width: 160px;height: 36px;border-radius: 6px;border: 1px solid #3790FA;background:transparent;}QLineEdit:pressed{width: 160px;height: 36px;border-radius: 6px;border: 2px solid #3790FA;background:transparent;}")
            self.close_btn.setStyleSheet(
                "QPushButton{border-radius: 4px;border:0px;border-image:url('res/common-close-white.png')}QPushButton:hover{border:0px;border-image:url('res/hover-close.png')}QPushButton:pressed{border:0px;border-image:url('res/clicked-close.png')}")
        # QApplication.processEvents()

    #
    # 函数：处理点击关闭按钮
    #
    def slot_click_close_btn(self):
        if (Globals.THEME_COLOR == 0):
            self.close_btn.setStyleSheet(
                "QPushButton{border-radius: 4px;border:0px;border-image:url('res/close1.png')}")
        else:
            self.close_btn.setStyleSheet(
                "QPushButton{border-radius: 4px;border:0px;border-image:url('res/res/common-close-white.png.png')}")
        self.hide()
        QApplication.setActiveWindow(self.mw)

    def eventFilter(self, widget, event):
        if(hasattr(self,"service_dress_label")):
            if(widget == self.service_dress_label and event.type() == event.FocusIn):
                if (Globals.THEME_COLOR == 0):
                    self.service_dress_label.setStyleSheet(
                        "QLineEdit{width: 160px;height: 36px;border-radius: 6px;border: 2px solid #3790FA;background:transparent;color: #1B1B1B;}")
                else:
                    self.service_dress_label.setStyleSheet(
                        "QLineEdit{width: 160px;height: 36px;border-radius: 6px;border: 2px solid #3790FA;background:transparent;color:#FFFFFF;}")
                return False
            elif(widget == self.service_dress_label and event.type() == event.FocusOut):
                if (Globals.THEME_COLOR == 0):
                    self.service_dress_label.setStyleSheet("QLineEdit{font-size: 14px;font-weight: 400;color: #1B1B1B;line-height: 24px;background:#E6E6E6;border: 1px solid #E6E6E6;border-radius: 6px;}QLineEdit:hover{width: 160px;height: 36px;border-radius: 6px;border: 1px solid #3790FA;background:#E6E6E6;}QLineEdit:pressed{width: 160px;height: 36px;border-radius: 6px;border: 2px solid #3790FA;background:transparent;}")
                else:
                    self.service_dress_label.setStyleSheet("QLineEdit{font-size: 14px;font-weight: 400;color: #FFFFFF;line-height: 24px;background:#373737;border: 1px solid #373737;border-radius: 6px;}QLineEdit:hover{width: 160px;height: 36px;border-radius: 6px;border: 1px solid #3790FA;background:transparent;}QLineEdit:pressed{width: 160px;height: 36px;border-radius: 6px;border: 2px solid #3790FA;background:transparent;}")

                return False

        if (hasattr(self, "kylinid_dress_label")):
            if (widget == self.kylinid_dress_label and event.type() == event.FocusIn):
                if (Globals.THEME_COLOR == 0):
                    self.kylinid_dress_label.setStyleSheet(
                        "QLineEdit{width: 160px;height: 36px;border-radius: 6px;border: 2px solid #3790FA;background:transparent; color: #1B1B1B;}")
                else:
                    self.kylinid_dress_label.setStyleSheet(
                        "QLineEdit{width: 160px;height: 36px;border-radius: 6px;border: 2px solid #3790FA;background:transparent;color:#FFFFFF;}")
                return False
            elif (widget == self.kylinid_dress_label and event.type() == event.FocusOut):
                if (Globals.THEME_COLOR == 0):
                    self.kylinid_dress_label.setStyleSheet(
                        "QLineEdit{font-size: 14px;font-weight: 400;color: #1B1B1B;line-height: 24px;background:#E6E6E6;border: 1px solid #E6E6E6;border-radius: 6px;}QLineEdit:hover{width: 160px;height: 36px;border-radius: 6px;border: 1px solid #3790FA;background:#E6E6E6;}QLineEdit:pressed{width: 160px;height: 36px;border-radius: 6px;border: 2px solid #3790FA;background:transparent;}")
                else:
                    self.kylinid_dress_label.setStyleSheet(
                        "QLineEdit{font-size: 14px;font-weight: 400;color: #FFFFFF;line-height: 24px;background:#373737;border: 1px solid #373737;border-radius: 6px;}QLineEdit:hover{width: 160px;height: 36px;border-radius: 6px;border: 1px solid #3790FA;background:transparent;}QLineEdit:pressed{width: 160px;height: 36px;border-radius: 6px;border: 2px solid #3790FA;background:transparent;}")
                return False
            pass
        if (hasattr(self, "close_btn")):
            if (widget == self.close_btn and event.type() == event.MouseButtonRelease):
                self.slot_click_close_btn()
                # return True
            elif (widget == self.close_btn and event.type() == event.HoverEnter):
                if (Globals.THEME_COLOR == 0):
                    self.close_btn.setStyleSheet(
                        "QPushButton:hover{border-radius: 4px;border:0px;border-image:url('res/close2.png')}")
                else:
                    self.close_btn.setStyleSheet(
                        "QPushButton{border-radius: 4px;border:0px;border-image:url('res/hover-close.png')}")
            elif (widget == self.close_btn and event.type() == event.HoverLeave):
                if (Globals.THEME_COLOR == 0):
                    self.close_btn.setStyleSheet(
                        "QPushButton{border-radius: 4px;border:0px;border-image:url('res/close1.png')}")
                else:
                    self.close_btn.setStyleSheet(
                        "QPushButton{border-radius: 4px;border:0px;border-image:url('res/common-close-white.png')}")
            elif (widget == self.close_btn and event.type() == event.MouseButtonPress):
                if (Globals.THEME_COLOR == 0):
                    self.close_btn.setStyleSheet(
                        "QPushButton:hover{border-radius: 4px;border:0px;border-image:url('res/close3.png')}")
                else:
                    self.close_btn.setStyleSheet(
                        "QPushButton{border-radius: 4px;border:0px;border-image:url('res/clicked-close.png')}")

        return False

    #
    # 函数：重绘函数
    #
    def set_paintEvent(self, event):
        painter = QPainter(self)
        m_defaultBackgroundColor = QColor(qRgba(0, 0, 0, 16))
        m_defaultBackgroundColor.setAlpha(16)
        path = QPainterPath()
        path.setFillRule(Qt.WindingFill)
        path.addRoundedRect(10, 10, self.width() - 20, self.height() - 20, 12, 12)

        painter.setRenderHint(QPainter.Antialiasing, True)
        painter.fillPath(path, QBrush(QColor(m_defaultBackgroundColor.red(),
                                             m_defaultBackgroundColor.green(),
                                             m_defaultBackgroundColor.blue(),
                                             m_defaultBackgroundColor.alpha())))

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

