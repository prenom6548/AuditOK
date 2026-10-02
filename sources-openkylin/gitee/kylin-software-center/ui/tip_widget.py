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

class Tip_Widget(QWidget):
    def __init__(self,text,parent = None):
        QWidget.__init__(self, parent)
        self.text = text
        self.mouse_press = False
        self.init_ui()
        self.paintEvent = self.set_paintEvent

    def init_ui(self):
        self.setFixedSize(442,182)
        self.setWindowFlags(Qt.FramelessWindowHint | Qt.Tool)
        self.setWindowTitle(_("Software Store"))
        self.setAttribute(Qt.WA_TranslucentBackground)
        self.widget = QWidget(self)
        self.widget.resize(424,164)
        self.widget.move(9,9)
        #self.widget.setStyleSheet("QWidget{background: rgba(255, 255, 255, 0.7);border-radius:12px;background-color:#ffffff;}")
        self.title_label = QLabel(self.widget)
        self.title_label.setFixedSize(170,16)
        self.title_label.setText(_("Tip"))
        self.title_label.setStyleSheet("QLabel{border:0px;font-size: 14px;color: #1B1B1B;font-weight: 500;line-height: 14px;}")
        self.title_label.move(16,12)

        self.warning_label = QLabel(self.widget)
        self.warning_label.resize(24,24)
        self.warning_label.setStyleSheet("QLabel{border:0px;background-image:url(./res/warning.png);background-color:transparent;}")
        self.warning_label.move(16,55)
        self.warning_label.hide()

        self.line_label = QLabel(self.widget)
        self.line_label.setFixedSize(422,1)
        self.line_label.setStyleSheet("QLabel{width: 424px;height: 1px;background: #F2F6FD;}")
        self.line_label.move(1,38)

        self.close_btn = QPushButton(self.widget)
        self.close_btn.setFocusPolicy(Qt.NoFocus)
        self.close_btn.resize(30,30)
        self.close_btn.setStyleSheet("QPushButton{border-radius: 4px;border:0px;border-image:url('res/close1.png')}QPushButton:hover{border:0px;border-image:url('res/close2.png')}QPushButton:pressed{border:0px;border-image:url('res/close3.png')}")
        self.close_btn.move(self.widget.width()-self.close_btn.width()-4,4)
        self.close_btn.setToolTip(_("close"))

        self.tips_label = QLabel(self.widget)
        self.tips_label.setWordWrap(True)
        self.tips_label.setAlignment(Qt.AlignLeft)
        self.tips_label.setFixedWidth(self.widget.width()-self.warning_label.width()-self.warning_label.x()-16)
        self.tips_label.setText(self.text)
        # self.tips_label.adjustSize()
        self.tips_label.move(int((self.widget.width()-self.tips_label.width())/2),55)

        self.btn = QPushButton(self.widget)
        self.btn.setFocusPolicy(Qt.NoFocus)
        self.btn.setFixedSize(64,30)
        self.btn.setStyleSheet("QPushButton{background: #3790FA;border-radius: 6px;font-size: 14px;font-weight: 400;color: #FFFFFF;text-align: center;line-height: 12px;}QPushButton:hover{background: #40A9FB;;border-radius: 6px;color: #ffffff;}QPushButton:pressed{background: #296CD9;border-radius: 6px;color: #ffffff;}")
        self.btn.setText(_("OK"))
        self.btn.move(int((self.widget.width()-self.btn.width())/2),110)

        self.cancel_btn = QPushButton(self.widget)
        self.cancel_btn.setFocusPolicy(Qt.NoFocus)
        self.cancel_btn.setFixedSize(64, 30)
        self.cancel_btn.setStyleSheet("QPushButton{font-size: 14px;font-weight: 400;color: #8F9399;line-height: 20px;border-radius: 6px;text-align: center;border: 1px solid #DDDFE7;}QPushButton:hover{background: #F2F6FD;border-radius: 6px;border: 1px solid #DDDFE7;}QPushButton:pressed{background: #F2F6FD;border-radius: 6px;border: 1px solid #DDDFE7;}")
        self.cancel_btn.setText(_("cancel"))
        self.cancel_btn.move(172, 110)
        self.cancel_btn.hide()

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
            self.tips_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #606265;line-height: 24px;background:transparent;}")
            self.line_label.setStyleSheet("QLabel{width: 424px;height: 1px;background: #F2F6FD;}")
            self.close_btn.setStyleSheet("QPushButton{border-radius: 4px;border:0px;border-image:url('res/close1.png')}QPushButton:hover{border:0px;border-image:url('res/close2.png')}QPushButton:pressed{border:0px;border-image:url('res/close3.png')}")
        else:
            if (Globals.RIGHT_ANGLE):
                self.widget.setStyleSheet(".QWidget{background-color:#3D3D41;border: 1px solid rgba(38, 38, 38, 0.15);}")
                self.setStyleSheet("QFrame{background-color:#1F2022;}")
            else:
                self.setStyleSheet("QFrame{background-color:#1F2022;}")
                self.widget.setStyleSheet(".QWidget{border-radius:12px;background-color:#3D3D41;border: 1px solid rgba(38, 38, 38, 0.15);}")
            self.title_label.setStyleSheet("QLabel{font-size: 14px;color: #8F9399;line-height: 14px;font-weight: 500;background:transparent;}")
            self.tips_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 24px;background:transparent;}")
            self.line_label.setStyleSheet("QLabel{width: 424px;height: 1px;background: #48484C;}")
            self.close_btn.setStyleSheet(
                "QPushButton{border-radius: 4px;border:0px;border-image:url('res/common-close-white.png')}QPushButton:hover{border:0px;border-image:url('res/hover-close.png')}QPushButton:pressed{border:0px;border-image:url('res/clicked-close.png')}")
        # QApplication.processEvents()

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

class Click_All_Search(QFrame):

    def __init__(self, mw,parent=None):
        QFrame.__init__(self, parent)
        self.mw = mw
        self.label1 = QLabel(self)
        self.label1.setAlignment(Qt.AlignHCenter)
        self.label1.setAttribute(Qt.WA_TranslucentBackground)
        if (Globals.THEME_COLOR == 0):
            self.label1.setStyleSheet("QLabel{font-size: 20px;font-weight: 400;color: #303133;line-height: 24px;}")
        else:
            self.label1.setStyleSheet("QLabel{font-size: 20px;font-weight: 400;color: #F9F9F9;line-height: 24px;}")
        self.label1.setText(_("Go to"))
        self.label1.adjustSize()
        self.label1.move(0, 0)
        self.all_btn = QPushButton(self)
        self.all_btn.resize(self.mw.detailwidget.no_found_text2.width() + 20, self.mw.detailwidget.no_found_text2.height())
        self.all_btn.setFocusPolicy(Qt.NoFocus)
        self.all_btn.setAttribute(Qt.WA_TranslucentBackground)
        self.all_btn.setStyleSheet(
            "QPushButton{font-size: 20px;border:0px;font-weight: 400;line-height: 24px;background-color:transparent;color:#3790FA;}QPushButton:hover{background:transparent;}QPushButton:pressed{background:transparent;}")
        self.all_btn.move(self.label1.x() + self.label1.width()+3, self.label1.y())
        locale_lan = QLocale.system().name()
        if (locale_lan == "zh_CN"):
            self.all_btn.resize(70, self.label1.height())
        self.all_btn.setText(_("full library"))
        self.all_btn.adjustSize()
        self.all_btn.clicked.connect(self.mw.detailwidget.slot_cilck_all_search_btn)
        self.label2 = QLabel(self)
        self.label2.setAlignment(Qt.AlignHCenter)
        self.label2.setAttribute(Qt.WA_TranslucentBackground)
        if (Globals.THEME_COLOR == 0):
            self.label2.setStyleSheet("QLabel{font-size: 20px;font-weight: 400;color: #303133;line-height: 24px;}")
        else:
            self.label2.setStyleSheet("QLabel{font-size: 20px;font-weight: 400;color: #F9F9F9;line-height: 24px;}")
        self.label2.setText(_("to search!"))
        self.label2.adjustSize()
        self.label2.move(self.all_btn.x() + self.all_btn.width()+3, self.all_btn.y())

    def change_color(self):
        if (Globals.THEME_COLOR == 0):
            self.label1.setStyleSheet("QLabel{font-size: 20px;font-weight: 400;color: #303133;line-height: 24px;}")
            self.label2.setStyleSheet("QLabel{font-size: 20px;font-weight: 400;color: #303133;line-height: 24px;}")
        else:
            self.label1.setStyleSheet("QLabel{font-size: 20px;font-weight: 400;color: #F9F9F9;line-height: 24px;}")
            self.label2.setStyleSheet("QLabel{font-size: 20px;font-weight: 400;color: #F9F9F9;line-height: 24px;}")

