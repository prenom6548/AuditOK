#!/usr/bin/python3
# -*- coding: utf-8 -*-

import os
import sqlite3
import datetime
import threading
from models.enums import UKSC_CACHE_DIR
import gettext
gettext.bindtextdomain("kylin-software-center", "/usr/share/locale")
gettext.textdomain("kylin-software-center")
lock = threading.Lock()
_ = gettext.gettext

DB_FILE = UKSC_CACHE_DIR+"/history_installation.db"

class Record_History_Installation:
    def __init__(self):
        if os.path.isfile(DB_FILE):
            try:
                self.connect = sqlite3.connect(DB_FILE, check_same_thread=False)
                self.cursor = self.connect.cursor()
                select_table = ".table"
                try:
                    lock.acquire(True)
                    sql = "select name from sqlite_master where type = 'table' and name = 'installed'"
                    self.cursor.execute(sql)
                    res = self.cursor.fetchall()
                    if(res == None or res == []):
                        self.cursor.execute('''create table installed(appname text)''')
                        self.connect.commit()
                except Exception as e:
                        pass
                finally:
                    lock.release()
            except:
                pass
        else:
            try:
                self.connect = sqlite3.connect(DB_FILE, check_same_thread=False)
                self.cursor = self.connect.cursor()
                self.cursor.execute('''create table installed(appname text)''')
                self.connect.commit()
                self.cursor.close()
                self.connect.close()
                self.connect = sqlite3.connect(DB_FILE, check_same_thread=False)
                self.cursor = self.connect.cursor()
                os.system("chmod 664 " + DB_FILE)
            except:
                pass

    #
    # 函数:将软件的安装信息写入数据库
    #
    def insert(self, app_name):
        in_db = False
        self.select()
        res = self.cursor.fetchall()
        for i in res:
            if(app_name == i[0]):
                in_db = True
        if(in_db == False):
            try:
                self.cursor.execute("insert into installed values(?)",(app_name,))
            except:
                pass
        self.connect.commit()
        self.cursor.close()
        self.connect.close()

    #
    # 函数:读取数据库的信息
    #
    def select(self):
        try:
            self.cursor.execute("select appname from installed")
        except:
            pass

