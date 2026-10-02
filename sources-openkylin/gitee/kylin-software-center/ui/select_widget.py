from PyQt5.QtGui import *
from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
from ui.systembutton import *
from ui.systembutton import *
from models.enums import CONF_PATH
import datetime
import math
import configparser
import gettext
from ui.menubar import menu_list_widget
gettext.bindtextdomain("kylin-software-center", "/usr/share/locale")
gettext.textdomain("kylin-software-center")
_ = gettext.gettext

class Select_Widget(QWidget):
    click_item = pyqtSignal(QListWidgetItem)
    def __init__(self,width,height,item_num,item_name_list,need_icon,parent = None):
        QFrame.__init__(self, parent)
        self.w = width
        self.h = height
        self.item_num = item_num
        self.mouse_press = False
        self.item_name_list = item_name_list
        self.setMouseTracking(True)
        self.need_icon = need_icon
        self.init_ui()
        self.paintEvent = self.set_paintEvent

    def init_ui(self):
        self.resize(self.w+4+20,self.item_num*(self.h+2)+20)
        self.setWindowFlags(Qt.FramelessWindowHint)
        self.setAttribute(Qt.WA_TranslucentBackground)

        self.widget = QFrame(self)
        self.widget.resize(self.w+4,self.item_num*(self.h+2))
        self.widget.move(10,10)

        self.item_list = QListWidget(self.widget)
        self.item_list.setFocusPolicy(Qt.NoFocus)
        self.item_list.resize(self.w,self.item_num*(self.h+2))
        self.item_list.move(2,2)
        self.slot_show_dark_mode()
        for i_name in self.item_name_list:
            item = QListWidgetItem(self.item_list)
            # list_item_widget = menu_list_widget(self.w, self.h, i_name, True, False, self)
            if (self.need_icon == True):
                list_item_widget = menu_list_widget(self.w, self.h, i_name, False, True, self)
                list_item_widget.setFocusPolicy(Qt.NoFocus)
                list_item_widget.clicked_item.connect(self.slot_clicked_item)
                list_item_widget.enter_signal.connect(self.slot_enter_theme)
            else:
                list_item_widget = menu_list_widget(self.w, self.h, i_name, False, False, self)
                list_item_widget.setFocusPolicy(Qt.NoFocus)
                list_item_widget.clicked_item.connect(self.slot_clicked_item)
                # list_item_widget.leave_signal.connect(self.slot_leave_theme)
            self.item_list.addItem(item)
            self.item_list.setItemWidget(item, list_item_widget)
            self.check_item_color()

    def slot_show_dark_mode(self):
        if (Globals.THEME_COLOR == 0):
            if (Globals.RIGHT_ANGLE):
                self.setStyleSheet("QFrame{border:0px solid #ff0000;background-color:#ffffff;}")
                self.widget.setStyleSheet("QFrame{border:0px solid #ff0000;background-color:#ffffff;}")
            else:
                self.setStyleSheet("QFrame{border:0px solid #ff0000;background-color:#ffffff;border-radius: 8px;}")
                self.widget.setStyleSheet(
                    "QFrame{border:0px solid #ff0000;background-color:#ffffff;border-radius: 8px;}")
            self.item_list.setStyleSheet(
                "QListWidget{background-color:transparent;border:0px solid #cfcfcf;}QListWidget::item{height:30px;background-color:transparent;margin-top:0px;border:0px solid #cfcfcf;border-radius: 6px;}QListWidget::item:hover{background:#3790FA;border-radius: 6px;}")
        else:
            if (Globals.RIGHT_ANGLE):
                self.setStyleSheet("QFrame{border:0px solid #ff0000;background-color:#3D3D41;}")
                self.widget.setStyleSheet("QFrame{border:0px solid #ff0000;background-color:#3D3D41;}")
            else:
                self.setStyleSheet("QFrame{border:0px solid #ff0000;background-color:#3D3D41;border-radius: 8px;}")
                self.widget.setStyleSheet(
                    "QFrame{border:0px solid #ff0000;background-color:#3D3D41;border-radius: 8px;}")
            self.item_list.setStyleSheet(
                "QListWidget{background-color:#3D3D41;border:0px solid #cfcfcf;}QListWidget::item{height:30px;background-color:3D3D41;margin-top:0px;border:0px solid #cfcfcf;border-radius: 6px;}QListWidget::item:hover{background:#3790FA;border-radius: 6px;}")
        self.check_item_color()
        # QApplication.processEvents()


    def slot_clicked_item(self, item_name):
        item_count = self.item_list.count()
        for i in range(item_count):
            item = self.item_list.item(i)
            get_item_widget = self.item_list.itemWidget(item)
            if (get_item_widget.item_name_label.text() == item_name):
                self.click_item.emit(item)
            else:
                get_item_widget.icon_label.hide()

        # self.click_item.emit(item_name)

    def slot_enter_theme(self):
        for i in range(self.item_list.count()):
            item = self.item_list.item(i)
            item_widget = self.item_list.itemWidget(item)

    def slot_leave_theme(self):
        # self.hide()
        pass

    def check_item_color(self):
        for i in range(self.item_list.count()):
            item = self.item_list.item(i)
            get_item_widget = self.item_list.itemWidget(item)
            if (Globals.THEME_COLOR == 0):
                get_item_widget.item_name_label.setStyleSheet(
                    "QLabel{font-size: 14px;font-weight: 400;color: #303133;line-height: 14px;}")
                if (get_item_widget.primary_directory == False):
                    get_item_widget.icon_label.setStyleSheet(
                        "QLabel{background-image:url(./res/check2.png);background-color:transparent;}")
            else:
                get_item_widget.item_name_label.setStyleSheet(
                    "QLabel{font-size: 14px;font-weight: 400;color: #F9F9F9;line-height: 14px;}")
                if (get_item_widget.primary_directory == False):
                    get_item_widget.icon_label.setStyleSheet(
                        "QLabel{background-image:url(./res/check_dark.png);background-color:transparent;}")

    #
    # 函数：重绘函数
    #
    def set_paintEvent(self, event):
        painter = QPainter(self)
        m_defaultBackgroundColor = QColor(qRgb(192, 192, 192))
        m_defaultBackgroundColor.setAlpha(50)
        path = QPainterPath()
        path.setFillRule(Qt.WindingFill)
        path.addRoundedRect(10, 10, self.width() - 20, self.height() - 20, 8, 8)

        painter.setRenderHint(QPainter.Antialiasing, True)
        painter.fillPath(path, QBrush(QColor(m_defaultBackgroundColor.red(),
                                             m_defaultBackgroundColor.green(),
                                             m_defaultBackgroundColor.blue())))

        color = QColor(0, 0, 0, 20)
        i = 0
        while i < 4:
            path = QPainterPath()
            path.setFillRule(Qt.WindingFill)
            path.addRoundedRect(10 - i, 10 - i, self.width() - (10 - i) * 2, self.height() - (10 - i) * 2, 8, 8)
            color.setAlpha(int(100 - math.sqrt(i) * 50))
            painter.setPen(color)
            painter.drawPath(path)
            i = i + 1

        painter.setRenderHint(QPainter.Antialiasing)

    #
    # 函数：鼠标进入事件
    #
    def enterEvent(self, event):
        self.status = BUTTON_ENTER
        self.update()

    #
    # 函数：鼠标按下事件
    #
    def mousePressEvent(self, event):
        if (event.button() == Qt.LeftButton):
            self.setAttribute(Qt.WA_NoMouseReplay)
            self.mouse_press = True
            self.status = BUTTON_PRESSED
            self.update()

    #
    # 函数：鼠标释放事件
    #
    def mouseReleaseEvent(self, event):
        if (self.mouse_press and self.rect().contains(event.pos())):
            self.mouse_press = False
            self.status = BUTTON_ENTER
            self.update()

    #
    # 函数：鼠标离开事件
    #
    def leaveEvent(self, event):
        self.status = BUTTON_LEAVE
        self.update()
        self.hide()

