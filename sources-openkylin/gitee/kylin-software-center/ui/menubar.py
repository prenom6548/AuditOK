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
gettext.bindtextdomain("kylin-software-center", "/usr/share/locale")
gettext.textdomain("kylin-software-center")
_ = gettext.gettext

class MenuBar(QWidget):
    click_item = pyqtSignal(QListWidgetItem)
    def __init__(self,width,height,item_num,item_name_list,mw,parent = None):
        QFrame.__init__(self, parent)
        self.w = width
        self.h = height
        self.item_num = item_num
        self.mouse_press = False
        self.item_name_list = item_name_list
        self.setMouseTracking(True)
        self.mw = mw
        self.init_ui()
        self.paintEvent = self.set_paintEvent

    def init_ui(self):
        self.resize(self.w+8+20,self.item_num*(self.h+2)+4+20)
        self.setWindowFlags(Qt.FramelessWindowHint|Qt.Dialog)
        self.setAttribute(Qt.WA_TranslucentBackground)

        self.widget = QFrame(self)
        self.widget.resize(self.w+8,self.item_num*(self.h+2)+4)
        self.widget.move(10,10)

        self.item_list = QListWidget(self.widget)
        self.item_list.setFocusPolicy(Qt.NoFocus)
        self.item_list.resize(self.w,self.item_num*self.h)
        self.item_list.move(4,4)
        self.slot_show_dark_mode()

        self.widget.resize(self.widget.width(),self.item_list.height()+8)
        self.resize(self.width(),self.widget.height()+20)

        for i_name in self.item_name_list:
            item = QListWidgetItem(self.item_list)
            # list_item_widget = menu_list_widget(self.w, self.h, i_name, True, False, self)
            if(self.item_num == 6):
                if(i_name == _("Theme")):
                    list_item_widget = menu_list_widget(self.w,self.h,i_name,True,True,self)
                    list_item_widget.setFocusPolicy(Qt.NoFocus)
                    list_item_widget.clicked_item.connect(self.slot_clicked_item)
                    list_item_widget.enter_signal.connect(self.slot_enter_theme)
                else:
                    list_item_widget = menu_list_widget(self.w, self.h, i_name, True, False,self)
                    list_item_widget.setFocusPolicy(Qt.NoFocus)
                    list_item_widget.clicked_item.connect(self.slot_clicked_item)
                    list_item_widget.leave_signal.connect(self.slot_leave_theme)
                list_item_widget.item_name_label.move(12,int((list_item_widget.height()-list_item_widget.item_name_label.height())/2))
            else:
                cf = configparser.ConfigParser()
                cf.read(CONF_PATH)
                light = cf.get("theme-color", "light")
                dark = cf.get("theme-color", "dark")
                auto = cf.get("theme-color", "auto")
                item_name = "Light mode"
                if (light == "YES"):
                    item_name = _("Light mode")
                if (dark == "YES"):
                    item_name = _("Dark Mode")
                if (auto == "YES"):
                    item_name = _("Automatic")
                if (i_name == item_name):
                    list_item_widget = menu_list_widget(self.w, self.h, i_name, False, True,self)
                    list_item_widget.setFocusPolicy(Qt.NoFocus)
                    list_item_widget.clicked_item.connect(self.slot_clicked_item)
                else:
                    list_item_widget = menu_list_widget(self.w, self.h, i_name, False, False,self)
                    list_item_widget.setFocusPolicy(Qt.NoFocus)
                    list_item_widget.clicked_item.connect(self.slot_clicked_item)

                list_item_widget.item_name_label.move(list_item_widget.icon_label.x() + list_item_widget.icon_label.width() + 11,
                                          int((list_item_widget.height() - list_item_widget.item_name_label.height()) / 2))
            self.item_list.addItem(item)
            self.item_list.setItemWidget(item, list_item_widget)
            self.check_item_color()

    def slot_show_dark_mode(self):
        if(Globals.THEME_COLOR == 0):
            if(Globals.RIGHT_ANGLE):
                self.setStyleSheet("QFrame{border:0px solid #ff0000;background-color:#DD0D00;}")
                self.widget.setStyleSheet("QFrame{border:0px solid #ff0000;background-color:#FFFFFF;}")
            else:
                self.setStyleSheet("QFrame{border:0px solid #ff0000;background-color:#ffffff;border-radius: 8px;}")
                self.widget.setStyleSheet("QFrame{border:0px solid #ff0000;background-color:#ffffff;border-radius: 8px;}")
            self.item_list.setStyleSheet("QListWidget{background-color:transparent;border:0px solid #cfcfcf;}QListWidget::item{height:36;background-color:transparent;margin-top:0px;border:0px solid #cfcfcf;border-radius: 6px;}QListWidget::item:hover{background:#3790FA;border-radius: 6px;}")
        else:
            if (Globals.RIGHT_ANGLE):
                self.setStyleSheet("QFrame{border:0px solid #ff0000;background-color:#3D3D41;}")
                self.widget.setStyleSheet("QFrame{border:0px solid #ff0000;background-color:#3D3D41;}")
            else:
                self.setStyleSheet("QFrame{border:0px solid #ff0000;background-color:#3D3D41;border-radius: 8px;}")
                self.widget.setStyleSheet("QFrame{border:0px solid #ff0000;background-color:#3D3D41;border-radius: 8px;}")
            self.item_list.setStyleSheet("QListWidget{background-color:#3D3D41;border:0px solid #cfcfcf;border-radius: 8px;}QListWidget::item{height:36px;background-color:3D3D41;margin-top:0px;border:0px solid #cfcfcf;border-radius: 6px;}QListWidget::item:hover{background:#296CD9;border-radius: 6px;}")
        self.check_item_color()
        # QApplication.processEvents()

    def slot_clicked_item(self,item_name):
        item_count = self.item_list.count()
        for i in range(item_count):
            item = self.item_list.item(i)
            get_item_widget = self.item_list.itemWidget(item)
            if(get_item_widget.item_name_label.text() == item_name):
                self.click_item.emit(item)
                break

        # self.click_item.emit(item_name)

    def slot_enter_theme(self):
        for i in range(self.item_list.count()):
            item = self.item_list.item(i)
            item_widget = self.item_list.itemWidget(item)
            if(item_widget.item_name_label.text() == _("Theme")):
                self.mw.slot_click_first_level(item)
                break

    def slot_leave_theme(self):
        if (hasattr(self.mw, "second_lavel_menubar")):
            self.mw.second_lavel_menubar.hide()

    def check_item_color(self):
        for i in range(self.item_list.count()):
            item = self.item_list.item(i)
            get_item_widget = self.item_list.itemWidget(item)
            if(Globals.THEME_COLOR == 0):
                get_item_widget.item_name_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #303133;line-height: 14px;}")
                if(get_item_widget.primary_directory == False):
                    get_item_widget.icon_label.setStyleSheet("QLabel{background-image:url(./res/check2.png);background-color:transparent;}")
            else:
                get_item_widget.item_name_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #F9F9F9;line-height: 14px;}")
                if (get_item_widget.primary_directory == False):
                    get_item_widget.icon_label.setStyleSheet("QLabel{background-image:url(./res/check_dark.png);background-color:transparent;}")

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
    def enterEvent(self,event):
        self.status = BUTTON_ENTER
        self.update()

    #
    # 函数：鼠标按下事件
    #
    def mousePressEvent(self,event):
        if(event.button() == Qt.LeftButton):
            self.setAttribute(Qt.WA_NoMouseReplay)
            self.mouse_press = True
            self.status = BUTTON_PRESSED
            self.update()

    #
    # 函数：鼠标释放事件
    #
    def mouseReleaseEvent(self,event):
        if(self.mouse_press and self.rect().contains(event.pos())):
            self.mouse_press = False
            self.status = BUTTON_ENTER
            self.update()

    #
    # 函数：鼠标离开事件
    #
    def leaveEvent(self,event):
        self.status = BUTTON_LEAVE
        self.update()


class menu_list_widget(QWidget):
    enter_signal = pyqtSignal()
    leave_signal = pyqtSignal()
    clicked_item = pyqtSignal(str)
    def __init__(self,w,h,item_name,primary_directory,need_icon,parent = None):
        QWidget.__init__(self, parent)
        self.primary_directory = primary_directory
        self.resize(w,h)
        self.item_name_label = QLabel(self)
        self.item_name_label.setAttribute(Qt.WA_TranslucentBackground)
        self.item_name_label.setText(item_name)
        self.item_name_label.adjustSize()
        self.item_name_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #303133;line-height: 14px;}")
        self.item_name_label.move(26,int((self.height()-self.item_name_label.height())/2))

        self.icon_label = QLabel(self)
        self.icon_label.resize(12,13)
        if(need_icon == True and primary_directory == True):
            self.icon_label.move(self.width() - self.icon_label.width() - 8,int((self.height() - self.icon_label.height()) / 2))
            self.icon_label.setStyleSheet("QLabel{background-image:url(./res/right_btn.png);background-color:transparent;}")
        elif(need_icon == True and primary_directory == False):
            self.icon_label.move(8,int((self.height() - self.icon_label.height()) / 2))
            self.icon_label.setStyleSheet("QLabel{background-image:url(./res/check2.png);background-color:transparent;}")
        else:
            self.icon_label.move(8, int((self.height() - self.icon_label.height()) / 2))
            self.icon_label.setStyleSheet("QLabel{background-image:url(./res/check2.png);background-color:transparent;}")
            self.icon_label.hide()

    def enterEvent(self,event):
        self.item_name_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #FFFFFF;}")
        if (self.item_name_label.text() == _("Theme")):
            self.enter_signal.emit()
        else:
            self.leave_signal.emit()

    def leaveEvent(self, event):
        if(Globals.THEME_COLOR == 0):
            self.item_name_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #303133;line-height: 14px;}")
        else:
            self.item_name_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #FFFFFF;line-height: 14px;}")

    def mousePressEvent(self, event):
        # if (event.button() == Qt.LeftButton):
        pass


    def mouseReleaseEvent(self, event):
        self.clicked_item.emit(self.item_name_label.text())
