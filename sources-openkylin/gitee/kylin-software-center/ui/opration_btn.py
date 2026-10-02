#!/usr/bin/python3
# -*- coding: utf-8 -*-


from PyQt5.QtGui import *
from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
from models.globals import *
from models.enums import (BUTTON_ENTER, BUTTON_LEAVE, BUTTON_PRESSED, BUTTON_DISABLE)
import gettext
gettext.bindtextdomain("kylin-software-center", "/usr/share/locale")
gettext.textdomain("kylin-software-center")
_ = gettext.gettext

class Opration_Btn(QPushButton):

    def __init__(self):
        self.text_label = QLabel(self)
        self.text_label.setAttribute(Qt.WA_TranslucentBackground)
        self.text_label.move(12,6)