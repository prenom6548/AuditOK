#!/usr/bin/python3
# -*- coding: utf-8 -*-

from PyQt5.QtWidgets import *
from PyQt5.QtCore import *
from PyQt5.QtGui import *

class Update_Source_Dialog(QMessageBox):

    def __init__(self, parent=None):
        QMessageBox.__init__(self, parent)
        #self.setWindowTitle("软件源更新提示")
        self.setWindowTitle(_("Software source update tips"))
        #self.checkbox = QCheckBox(self.tr("不再提醒"),self)
        self.checkbox = QCheckBox(self.tr(_("Never remind")), self)
        self.checkbox.isCheckable()

        self.checkbox.setGeometry(QRect(35, 120, 90, 20))
        #self.button_update = self.addButton(self.tr("更新"), QMessageBox.ActionRole)
        self.button_update = self.addButton(self.tr(_("Update")), QMessageBox.ActionRole)
        #self.button_notupdate = self.addButton(self.tr("不更新"), QMessageBox.ActionRole)
        self.button_notupdate = self.addButton(self.tr(_("Don't update")), QMessageBox.ActionRole)
        #self.button_exit = self.addButton(self.tr("退出"), QMessageBox.ActionRole)
        self.button_exit = self.addButton(self.tr(_("Quit")), QMessageBox.ActionRole)
        self.button_exit.hide()
        self.setEscapeButton(self.button_exit)