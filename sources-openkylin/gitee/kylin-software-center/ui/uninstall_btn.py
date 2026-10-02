#!/usr/bin/python3
# -*- coding: utf-8 -*-


from PyQt5.QtGui import *
from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
from models.enums import BUTTON_DISABLE,BUTTON_PRESSED,BUTTON_LEAVE,BUTTON_ENTER,AppMsg
from utils import commontools
from models.application import *

class uninstall_btn(QFrame):
    def __init__(self, parent=None):
        QWidget.__init__(self, parent)
        self.m_parent = parent
        self.currentpage = 0
        self.ngif = []
        self.resize(22,22)
        self.setWindowFlags(Qt.FramelessWindowHint)
        self.setAttribute(Qt.WA_TranslucentBackground, True)
        self.setProperty("doNotBlur", True)
        self.setProperty("blurRegion", QRegion(QRect(0, 0, 1, 1)))

        self.switchTimer = QTimer(self)
        self.switchTimer.timeout.connect(self.slot_switch_animation_step)
        self.loadinggif = QLabel(self)
        self.loadinggif.setAttribute(Qt.WA_TranslucentBackground)
        self.loadinggif.resize(22,22)
        self.loadinggif.move(0,0)
        self.load_png()


    #
    # 函数名:加载图片
    # Function: load png
    #
    def load_png(self):
        for i in range(1, 17):
            img = QPixmap("res/uninstall/" + str(i) + ".png")
            self.ngif.append(img)

    #
    # 函数名:更改
    # Function: process change
    #
    def slot_switch_animation_step(self):
        if (self.currentpage == 16):
            self.currentpage = 0

        self.loadinggif.setPixmap(self.ngif[self.currentpage])
        self.loadinggif.setScaledContents(True)
        self.currentpage = self.currentpage + 1

    #
    # 函数名:开始加载
    # Function: start loading
    #
    def start_loading(self):
        self.currentpage = 0
        self.switchTimer.start(60)
        self.show()

    #
    # 函数名:停止加载
    # Function: stop loading
    #
    def stop_loading(self):
        self.switchTimer.stop()
        self.hide()
