#!/usr/bin/python
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
from models.globals import Globals
import base64

#
# 函数：缓存读取
#
def password_read():
    try:
        file_object = open('backend/service/password.txt', 'r')
        # line = file_object.readlines()
        # line = line.strip()
        # print "aaaaaaaaaaaaaa",line

        line = file_object.readlines()
        if line != " ":

            Globals.SET_REM = base64.b64decode(line[0].strip()).decode()
            Globals.AUTO_LOGIN = base64.b64decode(line[1].strip()).decode()
            Globals.USER = base64.b64decode(line[2].strip()).decode()
            Globals.PASSWORD = base64.b64decode(line[3].strip()).decode()

        file_object.close()

    except:
        pass


#
# 函数：缓存写入
#
def password_write(set_rem_pass, auto_login, sequence, password):
    try:
        set_rem_pass = base64.b64encode(set_rem_pass.encode())
        auto_login = base64.b64encode(auto_login.encode())
        sequence = base64.b64encode(sequence.encode())
        password = base64.b64encode(password.encode())

        set_rem_pass = bytes.decode(set_rem_pass)
        auto_login = bytes.decode(auto_login)
        sequence = bytes.decode(sequence)
        password = bytes.decode(password)

        file_object = open('backend/service/password.txt', 'w')
        file_object.writelines(set_rem_pass + '\n')
        file_object.writelines(auto_login + '\n')
        file_object.writelines(sequence + '\n')
        file_object.writelines(password + '\n')
        file_object.close()
    except:
        pass



