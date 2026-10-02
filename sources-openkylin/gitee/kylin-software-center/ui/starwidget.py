#!/usr/bin/python3
# -*- coding: utf-8 -*-

### BEGIN LICENSE

# Copyright (C) 2013 National University of Defense Technology(NUDT) & Kylin Ltd

# Author:
#     Shine Huang<shenghuang@ubuntukylin.com>
# Maintainer:
#     Shine Huang<shenghuang@ubuntukylin.com>

# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 3, as published
# by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranties of
# MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
# PURPOSE.  See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program.  If not, see <http://www.gnu.org/licenses/>.

from PyQt5.QtWidgets import *
from PyQt5.QtGui import *
from PyQt5.QtCore import *
from ui.uksw import Ui_StarWidget
from models.globals import *


class StarWidget(QWidget):
    # small or big
    size = ''

    def __init__(self, size, grade, parent=None):
        QWidget.__init__(self,parent)
        self.ui_init()
        self.size = size
        self.grade = grade

        if(self.size == 'small'):
            self.ui.star1.setStyleSheet("QLabel{background-image:url('res/star-small-2.png')}")
            self.ui.star2.setStyleSheet("QLabel{background-image:url('res/star-small-2.png')}")
            self.ui.star3.setStyleSheet("QLabel{background-image:url('res/star-small-2.png')}")
            self.ui.star4.setStyleSheet("QLabel{background-image:url('res/star-small-2.png')}")
            self.ui.star5.setStyleSheet("QLabel{background-image:url('res/star-small-2.png')}")
        elif(self.size == 'medium'):
            self.resize(128, 26)
            self.ui.star1.setGeometry(1, 0, 20, 20)
            self.ui.star2.setGeometry(27, 0, 20, 20)
            self.ui.star3.setGeometry(54, 0, 20, 20)
            self.ui.star4.setGeometry(81, 0, 20, 20)
            self.ui.star5.setGeometry(108, 0, 20, 20)
            self.ui.star1.setStyleSheet("QLabel{background-image:url('res/s2.png')}")
            self.ui.star2.setStyleSheet("QLabel{background-image:url('res/s2.png')}")
            self.ui.star3.setStyleSheet("QLabel{background-image:url('res/s2.png')}")
            self.ui.star4.setStyleSheet("QLabel{background-image:url('res/s2.png')}")
            self.ui.star5.setStyleSheet("QLabel{background-image:url('res/s2.png')}")
        elif(self.size == "big"):
            self.resize(167, 30)
            self.ui.star1.setGeometry(0, 0, 30, 30)
            self.ui.star2.setGeometry(34, 0, 30, 30)
            self.ui.star3.setGeometry(68, 0, 30, 30)
            self.ui.star4.setGeometry(102, 0, 30, 30)
            self.ui.star5.setGeometry(136, 0, 30, 30)
            self.ui.star1.setStyleSheet("QLabel{background-image:url('res/medium_dark_star.png')}")
            self.ui.star2.setStyleSheet("QLabel{background-image:url('res/medium_dark_star.png')}")
            self.ui.star3.setStyleSheet("QLabel{background-image:url('res/medium_dark_star.png')}")
            self.ui.star4.setStyleSheet("QLabel{background-image:url('res/medium_dark_star.png')}")
            self.ui.star5.setStyleSheet("QLabel{background-image:url('res/medium_dark_star.png')}")

        self.changeGrade(grade)
   
    #
    #函数名:初始化界面 
    #Function: init innterface
    #
    def ui_init(self):
        self.ui = Ui_StarWidget()
        self.ui.setupUi(self)
        self.show()

    #
    #函数名:更改等级 
    #Function: change grade
    #
    def changeGrade(self, grade):
        self.grade = grade
        if(self.size == 'small'):
            self.ui.star1.setStyleSheet("QLabel{background-image:url('res/star-small-1-2.png')}")
            self.ui.star2.setStyleSheet("QLabel{background-image:url('res/star-small-1-2.png')}")
            self.ui.star3.setStyleSheet("QLabel{background-image:url('res/star-small-1-2.png')}")
            self.ui.star4.setStyleSheet("QLabel{background-image:url('res/star-small-1-2.png')}")
            self.ui.star5.setStyleSheet("QLabel{background-image:url('res/star-small-1-2.png')}")
            if(grade > 0):
                # add by kobe
                if(grade < 1):
                    self.ui.star1.setStyleSheet("QLabel{background-image:url('res/star-small-1-1.png')}")
                else:
                    self.ui.star1.setStyleSheet("QLabel{background-image:url('res/star-small-1-1.png')}")
            if(grade > 1):
                if(grade < 2):
                    self.ui.star2.setStyleSheet("QLabel{background-image:url('res/star-small-1-1.png')}")
                else:
                    self.ui.star2.setStyleSheet("QLabel{background-image:url('res/star-small-1-1.png')}")
            if(grade > 2):
                if(grade < 3):
                    self.ui.star3.setStyleSheet("QLabel{background-image:url('res/star-small-1-1.png')}")
                else:
                    self.ui.star3.setStyleSheet("QLabel{background-image:url('res/star-small-1-1.png')}")
            if(grade > 3):
                if(grade < 4):
                    self.ui.star4.setStyleSheet("QLabel{background-image:url('res/star-small-1-1.png')}")
                else:
                    self.ui.star4.setStyleSheet("QLabel{background-image:url('res/star-small-1-1.png')}")
            if(grade > 4):
                if(grade < 5):
                    self.ui.star5.setStyleSheet("QLabel{background-image:url('res/star-small-1-1.png')}")
                else:
                    self.ui.star5.setStyleSheet("QLabel{background-image:url('res/star-small-1-1.png')}")
        if(self.size == 'medium'):
            self.ui.star1.setStyleSheet("QLabel{background-image:url('res/s2.png')}")
            self.ui.star2.setStyleSheet("QLabel{background-image:url('res/s2.png')}")
            self.ui.star3.setStyleSheet("QLabel{background-image:url('res/s2.png')}")
            self.ui.star4.setStyleSheet("QLabel{background-image:url('res/s2.png')}")
            self.ui.star5.setStyleSheet("QLabel{background-image:url('res/s2.png')}")
            if (grade > 0):
                if (grade < 1):
                    self.ui.star1.setStyleSheet("QLabel{background-image:url('res/s2.png')}")
                else:
                    if (Globals.THEME_COLOR == 0):
                        self.ui.star1.setStyleSheet("QLabel{background-image:url('res/s.png')}")
                    else:
                        self.ui.star1.setStyleSheet("QLabel{background-image:url('res/s_dark.png')}")
            if (grade > 1):
                if (grade < 2):
                    self.ui.star2.setStyleSheet("QLabel{background-image:url('res/s2.png')}")
                else:
                    if (Globals.THEME_COLOR == 0):
                        self.ui.star2.setStyleSheet("QLabel{background-image:url('res/s.png')}")
                    else:
                        self.ui.star2.setStyleSheet("QLabel{background-image:url('res/s_dark.png')}")
            if (grade > 2):
                if (grade < 3):
                    self.ui.star3.setStyleSheet("QLabel{background-image:url('res/s2.png')}")
                else:
                    if (Globals.THEME_COLOR == 0):
                        self.ui.star3.setStyleSheet("QLabel{background-image:url('res/s.png')}")
                    else:
                        self.ui.star3.setStyleSheet("QLabel{background-image:url('res/s_dark.png')}")
            if (grade > 3):
                if (grade < 4):
                    self.ui.star4.setStyleSheet("QLabel{background-image:url('res/s2.png')}")
                else:
                    if (Globals.THEME_COLOR == 0):
                        self.ui.star4.setStyleSheet("QLabel{background-image:url('res/s.png')}")
                    else:
                        self.ui.star4.setStyleSheet("QLabel{background-image:url('res/s_dark.png')}")
            if (grade > 4):
                if (grade < 5):
                    self.ui.star5.setStyleSheet("QLabel{background-image:url('res/s2.png')}")
                else:
                    if (Globals.THEME_COLOR == 0):
                        self.ui.star5.setStyleSheet("QLabel{background-image:url('res/s.png')}")
                    else:
                        self.ui.star5.setStyleSheet("QLabel{background-image:url('res/s_dark.png')}")
        if(self.size == 'big'):
            self.ui.star1.setStyleSheet("QLabel{background-image:url('res/medium_dark_star.png')}")
            self.ui.star2.setStyleSheet("QLabel{background-image:url('res/medium_dark_star.png')}")
            self.ui.star3.setStyleSheet("QLabel{background-image:url('res/medium_dark_star.png')}")
            self.ui.star4.setStyleSheet("QLabel{background-image:url('res/medium_dark_star.png')}")
            self.ui.star5.setStyleSheet("QLabel{background-image:url('res/medium_dark_star.png')}")
            if(grade > 0):
                if(grade < 1):
                    self.ui.star1.setStyleSheet("QLabel{background-image:url('res/medium_dark_star.png')}")
                else:
                    if(Globals.THEME_COLOR == 0):
                        self.ui.star1.setStyleSheet("QLabel{background-image:url('res/medium_bright_star.png')}")
                    else:
                        self.ui.star1.setStyleSheet("QLabel{background-image:url('res/medium_bright_star.png')}")
            if(grade > 1):
                if(grade < 2):
                    self.ui.star2.setStyleSheet("QLabel{background-image:url('res/medium_dark_star.png')}")
                else:
                    if(Globals.THEME_COLOR == 0):
                        self.ui.star2.setStyleSheet("QLabel{background-image:url('res/medium_bright_star.png')}")
                    else:
                        self.ui.star2.setStyleSheet("QLabel{background-image:url('res/medium_bright_star.png')}")
            if(grade > 2 ):
                if(grade < 3):
                    self.ui.star3.setStyleSheet("QLabel{background-image:url('res/medium_dark_star.png')}")
                else:
                    if(Globals.THEME_COLOR == 0):
                        self.ui.star3.setStyleSheet("QLabel{background-image:url('res/medium_bright_star.png')}")
                    else:
                        self.ui.star3.setStyleSheet("QLabel{background-image:url('res/medium_bright_star.png')}")
            if(grade > 3 ):
                if(grade < 4):
                    self.ui.star4.setStyleSheet("QLabel{background-image:url('res/medium_dark_star.png')}")
                else:
                    if(Globals.THEME_COLOR == 0):
                        self.ui.star4.setStyleSheet("QLabel{background-image:url('res/medium_bright_star.png')}")
                    else:
                        self.ui.star4.setStyleSheet("QLabel{background-image:url('res/medium_bright_star.png')}")
            if(grade > 4):
                if(grade < 5):
                    self.ui.star5.setStyleSheet("QLabel{background-image:url('res/medium_dark_star.png')}")
                else:
                    if(Globals.THEME_COLOR == 0):
                        self.ui.star5.setStyleSheet("QLabel{background-image:url('res/medium_bright_star.png')}")
                    else:
                        self.ui.star5.setStyleSheet("QLabel{background-image:url('res/medium_bright_star.png')}")
