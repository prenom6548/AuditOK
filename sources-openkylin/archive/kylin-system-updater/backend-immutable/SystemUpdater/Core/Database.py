# /*
#  * Copyright (C) 2020, KylinSoft Co., Ltd.
#  *
#  * This program is free software: you can redistribute it and/or modify
#  * it under the terms of the GNU General Public License as published by
#  * the Free Software Foundation, either version 3 of the License, or
#  * (at your option) any later version.
#  *
#  * This program is distributed in the hope that it will be useful,
#  * but WITHOUT ANY WARRANTY; without even the implied warranty of
#  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  * GNU General Public License for more details.
#  *
#  * You should have received a copy of the GNU General Public License
#  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
#  */

#!/usr/bin/python3
# -*- coding: utf-8 -*-
import os
import re
import dbus
import json
import traceback
import shutil
import sqlite3
import logging
import datetime
import threading
from operator import itemgetter
from gettext import gettext as _
from SystemUpdater.Core.errors import *
from SystemUpdater.Core.enums import *
from SystemUpdater.Core.DataAcquisition import UpdateMsgCollector
from SystemUpdater.Core.UpdaterConfigParser import UpgradeConfig
from SystemUpdater.packageManager.UpdaterSDKInterface import UpdaterSDKSysInfo

DB_FILE = os.path.join("/var/cache/kylin-system-updater/kylin-system-updater.db")
# UMDB_FILE = os.path.join("/var/cache/kylin-system-updater/kylin-system-updater.db")
DB_UPDATER = "/var/cache/kylin-update-manager/kylin-update-manager.db"
DB_UPGRADE = "/var/cache/kylin-system-updater/kylin-system-updater.db"
VER_DB = "/usr/share/kylin-system-updater/kylin-system-updater.db"
SYSTEM_VERSION_FILE = "/etc/kylin-version/kylin-system-version.conf"
INSTALLED_LIST = [{"item": "errorcode", "type": "int", "default": "0"}]
DISPALY_LIST = []

class Sqlite3Server(object):
    def __init__(self, updateManager=None, _no_DataMigration=False):
        self.connect = None
        self.window_main = updateManager
        self.init_sqlit()
        self.collector = UpdateMsgCollector(self.window_main)

        self.window_main.connect("transaction-stage", self._transaction_stage_handle)
        try:
            self.ucconfigs = None
            self.ucconfigs = UpgradeConfig(datadir = "/etc/kylin-version", name = "kylin-system-version.conf")
        except Exception as e:
            logging.warning(e)

        UpdaterSDKSysInfo().sysinfo_interface_get_version_detaile()

        self._system_version_config()
        #判断数据迁移
        if not _no_DataMigration:
            if (not os.path.exists(DB_UPDATER)):
                logging.debug("Can not found database: %s, should ignore.",DB_UPDATER)
                self.__need_DataMigration(mark_FirstMigration = True)
            else:
                self.__need_DataMigration(mark_FirstMigration = False)
        self.current_purge_pkgs = []
        self.init_metadata()
        self.transaction_manager = TransactionManager(self.window_main)

    def _transaction_stage_handle(self,parent,trans):
        self.collector.upgrade_process_msg_trans(trans)

    def init_metadata(self):
        self.deb_metadata = {}
        self.deb_metadata.update({"current_install_debfile":""})
        self.deb_metadata.update({"absolute_path":""})
        self.deb_metadata.update({"debname":""})
        self.deb_metadata.update({"deblist":[]})
        self.deb_metadata.update({"action":''})
        self.deb_metadata.update({"mode":''})
        self.deb_metadata.update({"source":''})
        self.deb_metadata.update({"sender":''})
        self.deb_metadata.update({"caller":''})
        self.deb_metadata.update({"old_version":''})
        self.deb_metadata.update({"new_version":''})
        update_version,os_version = self.get_current_version()
        self.deb_metadata.update({"old_update_version":update_version})
        self.deb_metadata.update({"new_update_version":update_version})
        self.deb_policy_timestamp = 0
        self.deb_policy_keep = False
        self.purge_policy_timestamp = 0
        self.purge_policy_keep = False

    # Initialize the connection database and modify it to connect when using
    def init_sqlit(self):
        try: 
            logging.info(_("Initialize database files ..."))
            if not os.path.isfile(DB_FILE):
                if not os.path.isdir(os.path.dirname(DB_FILE)):
                    os.makedirs(os.path.dirname(DB_FILE))
                shutil.copy("/usr/share/kylin-system-updater/kylin-system-updater.db", os.path.dirname(DB_FILE))
        except Exception as e:
            logging.error("Failed to initialize database files: %s", str(e))

    #connect连接数据库
    def connect_database(self):
        try: 
            self.connect = sqlite3.connect(DB_FILE, check_same_thread=False)
            self.cursor = self.connect.cursor()
        except Exception as e:
            logging.error("Failed to connect database: %s", str(e))
    
    #disconnect连接数据库
    def disconnect_database(self):
        try: 
            if self.connect != None:
                self.connect.close()
            if self.cursor != None:
                del self.cursor
        except Exception as e:
            logging.error("Failed to disconnect database: %s", str(e))

    # 数据库表格中动态增加新的字段用于扩展
    def insert_new_field(self):
        if len(INSTALLED_LIST) == 0 and len(DISPALY_LIST) == 0:
            return
        self.cursor.execute("select sql from sqlite_master where name='installed'")
        installed_sql = self.cursor.fetchone()[0]
        pattern = re.compile(r'\"\w+\"')
        installed_sql_list = pattern.findall(installed_sql)
        for value in INSTALLED_LIST:
            for field in installed_sql_list:
                if value["item"] == str(field).strip("\""):
                    break
                elif field == installed_sql_list[len(installed_sql_list) - 1]:
                    try:
                        if value["default"] != "":
                            sql = 'alter table installed add column "' + value["item"] + '" ' + value["type"] \
                                  + ' default ' + str(value["default"])
                        else:
                            sql = 'alter table installed add column "' + value["item"] + '" ' + value["type"]
                        self.cursor.execute(sql)
                        logging.info(_("installed table insert new field: %s"), value["item"])
                    except:
                        logging.error(_("installed table failed to insert a new field:"), value["item"], exc_info=True)

        self.cursor.execute("select sql from sqlite_master where name='display'")
        display_sql = self.cursor.fetchone()[0]
        pattern = re.compile(r'\"\w+\"')
        display_sql_list = pattern.findall(display_sql)
        for value in DISPALY_LIST:
            for field in display_sql_list:
                if value["item"] == str(field).strip("\""):
                    break
                elif field == display_sql_list[len(display_sql_list) - 1]:
                    try:
                        if value["default"] != "":
                            sql = 'alter table display add column "' + value["item"] + '" ' + value["type"] \
                                  + ' default ' + str(value["default"])
                        else:
                            sql = 'alter table installed add column "' + value["item"] + '" ' + value["type"]
                        self.cursor.execute(sql)
                        logging.info(_("display table insert new field： %s"), value["item"])
                    except:
                        logging.error(_("display table failed to insert a new field： %s"), value["item"], exc_info=True)

    # 写入数据到installed表中
    def insert_into_installed(self, *args, **kwargs):
        self.connect_database()
        self.cursor.execute(
            "insert into installed (appname, version, time, description, icon, statue, keyword, errorcode) values(?,"
            "?,?,?,?,?,?,?)",      (args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7]))
        self.connect.commit()
        logging.info("Database: Insert (%s=%s) To installed Complete ...", args[0], args[1])
        self.disconnect_database()

    # 写入数据到display表中
    def insert_into_display(self, *args, **kwargs):
        display_filed_lists = ["check_time","update_time","auto_check","system_version","auto_backup","download_limit",
                                "download_limit_value","allow_unattended_upgrades_shutdown","update_period",
                                "firstmigration","autoupdate_allow","tid","init_version"]
        if args[0] not in display_filed_lists:
            return "Error"

        def is_valid_string(s):
            pattern = r'^[a-zA-Z0-9=:_\-\s]+$'
            return bool(re.fullmatch(pattern, s))

        if not is_valid_string(args[1]):
            logging.error("Invalid args: %s", args[1])
            return "Error"

        self.connect_database()
        try:
            sql = f"update display set {args[0]} = ? where id = 1"
            self.cursor.execute(sql, (str(args[1]), ))
            self.connect.commit()
        except Exception as e:
            logging.error("Insert error: %s.", str(e))
            self.disconnect_database()
            return False
        logging.info("Database: Insert (%s=%s) To display Complete ...", args[0], args[1])
        self.disconnect_database()
        return True

    # 写入数据到tid_search表中
    def insert_into_tid(self, *args, **kwargs):
        self.connect_database()
        self.cursor.execute(
            "insert into tid_search (key, tid) values(?,?)",
            (args[0], args[1]))
        self.connect.commit()
        logging.info("Database: Insert (%s=%s) To tid_search Complete ...", args[0], args[1])
        self.disconnect_database()
    
    # 搜索tid_search表，获取tid值
    def select_from_tid(self, *args, **kwargs):
        retval = ''
        self.connect_database()
        try:
            sql = "select "+args[0]+" from tid_search where key='"+args[1]+"'"
            self.cursor.execute(sql)
            rets = self.cursor.fetchall()
            if len(rets)!= 0:
                if len(rets[0])!=0:
                    ret_first = rets[0]
                    retval = str(ret_first[0])
        except Exception as e:
            logging.error("Insert error: %s.", str(e))
            self.disconnect_database()
        logging.info("Database: Select tid_search data Complete...")
        self.disconnect_database()
        return retval

    # 读出display表中数据
    def select_from_display(self, *args, **kwargs):
        display_filed_lists = ["check_time","update_time","auto_check","system_version","auto_backup","download_limit",
                               "download_limit_value","allow_unattended_upgrades_shutdown","update_period",
                               "firstmigration","autoupdate_allow","tid","init_version"]
        logging.info(display_filed_lists)
        if args[0] not in display_filed_lists:
            return "Error"

        self.connect_database()
        try:
            sql = "select "+args[0]+" from display"
            self.cursor.execute(sql)
            self.connect.commit()
            retval = str(self.cursor.fetchone()[0])
        except Exception as e:
            logging.error("select error: %s.", str(e))
            self.disconnect_database()
            return "Error"
        logging.info("Database: Search display Complete (%s) ...", args[0])
        self.disconnect_database()
        return retval

    # 写入updateinfos表中
    def insert_into_updateinfo(self, *args, **kwargs):
        self.connect_database()
        try:
            self.cursor.execute(
                "insert into updateinfos (appname, version, description, date, status, keyword, errorcode, appname_cn, status_cn, changelog) values(?,"
                "?,?,?,?,?,?,?,?,?)",
                (args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9]))
            self.connect.commit()
        except Exception as e:
            logging.error("Insert error: %s.", str(e))
            self.disconnect_database()
        logging.info(_("Database: Insert To updateinfos Complete..."))
        self.disconnect_database()
    
    #get display data
    def gen_display_mate(self)->dict:
        mateData = {}
        self.connect_database()
        try:
            sql = "pragma table_info({})".format("display")
            self.cursor.execute(sql)
            keys=[key[1] for key in self.cursor.fetchall()]
            sql = "select * from display where id=1;"
            self.cursor.execute(sql)
            values = self.cursor.fetchone()
            mateData = dict(zip(keys, values))
            #
            # self.connect.row_factory = self.dictFactory
            # values = self.cursor.execute("select * from display where id=1;").fetchall()
        except Exception as e:
            logging.error("select error: %s.", str(e))
        self.disconnect_database()
        return mateData

    def dictFactory(self,cursor,row):
        """将sql查询结果整理成字典形式"""
        d={}
        for index,col in enumerate(cursor.description):
            d[col[0]]=row[index]
        return d

    def __need_DataMigration(self, mark_FirstMigration = False):
        # mateData = self.gen_display_mate()
        # if "firstmigration" in mateData.keys() and mateData["firstmigration"] == "true":
        dm = DataMigration()
        if (mark_FirstMigration):
            dm.MarkFirstMigration()
        else:
            dm.CopyData()

    def _check_upgrade_content(self,cache,content):
        upgradeable_groups = []
        upgradeable_pkgs = []

        for cont in content:
            if cont == '':
                continue 
            if cont in cache:
                upgradeable_pkgs.append(cont)
            else:
                upgradeable_groups.append(cont)

        return upgradeable_pkgs,upgradeable_groups

    # 接收更新列表与信息，生成数据并插入数据库中
    def insert_info(self, success, error_code):
        try:
            output_config_name = '/var/cache/kylin-system-updater/json/kylin-update-desktop-system.json'

            status, status_cn = ('success', '成功')  if success else ('failed', '失败')

            try:
                with open(output_config_name,'r') as f:
                    matedata = json.load(f)
            except Exception as exc:
                matedata = {}
                logging.error(exc)

            def formatted_get(matedata, field, default=""):
                value = default

                if field in matedata.keys():
                    variant = matedata.get(field, default)
                    if isinstance(variant , dict):
                        if "zh_CN" in variant.keys():
                            value = variant.get("zh_CN", default)
                        else:
                            value = next(iter(variant.values()), default)
                    
                    elif isinstance(variant, str):
                        value = variant
                    
                    elif isinstance(variant, list):
                        value = ", ".join(variant)

                return value

            pkgappname      = formatted_get(matedata, "update-name", "")
            pkgversion      = formatted_get(matedata, "version", "")
            pkgdescription  = formatted_get(matedata, "description", "")
            pkgchangelog    = formatted_get(matedata, "changelog", "")

            pkgtimestr      = datetime.datetime.strftime(datetime.datetime.now(), "%Y-%m-%d %H:%M:%S")

            pkgstatus       = status
            pkgkeyword      = "1"
            pkgappnamecn    = "系统更新"
            pkgstatuscn     = status_cn

            errstr = get_error_description_from_enum(error_code)

            if not success and not errstr:
                errstr = _('System upgrade error')

            self.insert_into_updateinfo(pkgappname, pkgversion, pkgdescription, pkgtimestr, \
                                        pkgstatus, pkgkeyword, errstr, pkgappnamecn, pkgstatuscn, pkgchangelog)

            # 系统升级完成 更新版本号
            if status == "success":
                logging.info("Complete system upgrade, refresh system version ...")
                self._refresh_system_version(update_version = pkgversion)
        except Exception as e:
            tbk = traceback.format_exc()
            logging.error("insert_info error: %s.",str(tbk))
            
    def refreshpkglist(self):
        pkgs_install = []
        pkgs_upgrade = []
        pkgs_remove  = []

        for pkg in self.window_main.cache:
                try:
                    if pkg.marked_install:
                        pkgs_install.append(pkg.name)
                    if pkg.marked_upgrade:
                        pkgs_upgrade.append(pkg.name)
                    elif pkg.marked_delete:
                        pkgs_remove.append(pkg.name)
                except KeyError:
                # pkg missing from fresh_cache can't be modified
                    pass
        return pkgs_install,pkgs_upgrade,pkgs_remove

    def _removal_of_marker(self):
        try:
            marker_path = "/var/cache/kylin-update-manager/ignoreOrDelay"
            if os.path.exists(marker_path):
                with open(marker_path, 'r+') as f:
                    line= f.readline()
                    if "2107" in line or "2203" in line:
                        f.seek(0)
                        f.truncate()
        except Exception as e:
            logging.error("Removing the upgrade success mark error: %s.",str(e))

    def _system_version_config(self):
        self.connect_database()
        try:
            sql = "select init_version from display where id=1"
            self.cursor.execute(sql)
            _is_init_verison = self.cursor.fetchone()[0]
            if _is_init_verison == "yes"  or _is_init_verison == "true":
                logging.info("Need to refresh version ...")
                self._initial_system_version()
                sql = "update display set init_version = 'no'"
                self.cursor.execute(sql)
                self.connect.commit()

            update_version, os_version = self.get_current_version()
            if not update_version:
                self.ucconfigs.setValue("SYSTEM","update_version",str(self.get_os_version()),True)
            if not os_version:
                self.ucconfigs.setValue("SYSTEM","os_version",str(self.get_os_version()),True)

        except Exception as e:
            logging.error(str(e))
        self.disconnect_database()
    
    def _refresh_system_version(self, update_version = '', pseudo_version = False):
        try:
            #刷新版本号:update_version、os_version
            os_version = self.get_os_version()

            if not pseudo_version:
                if len(os_version) != 0:
                    self.ucconfigs.setValue("SYSTEM","os_version",str(os_version),True)
                if len(update_version) != 0:
                    self.ucconfigs.setValue("SYSTEM","update_version",str(update_version),True)
                elif len(os_version) != 0 and len(update_version) == 0:
                    self.ucconfigs.setValue("SYSTEM","update_version",str(os_version),True)
            else:
                current_update_version, current_os_version = self.get_current_version()
                if current_os_version != os_version:
                    os_version+='*'
                    self.ucconfigs.setValue("SYSTEM","os_version",str(os_version),True)
                if current_update_version != update_version:
                    update_version+='*'
                    self.ucconfigs.setValue("SYSTEM","update_version",str(update_version),True)
            
            self.deb_metadata.update({"new_update_version":update_version})
        except Exception as e:
            logging.error("Refresh system version error: %s.",str(e))
    
    def _initial_system_version(self):
        try:
            #刷新版本号:update_version、os_version
            os_version = self.get_os_version()

            if len(os_version) != 0:
                self.ucconfigs.setValue("SYSTEM","os_version",str(os_version),True)
                self.ucconfigs.setValue("SYSTEM","update_version",str(os_version),True)

        except Exception as e:
            logging.error("Initial system version error: %s.",str(e))
    
    def get_os_version(self):
        os_version = ""
        version_path = "/etc/os-release"
        if os.path.isfile(version_path):
            with open(version_path, "r") as f:
                lines = f.readlines()
            for line in lines:
                if "KYLIN_RELEASE_ID" in line:
                    os_version = line.split('=')[1]
                    os_version = os_version.strip().strip('"')

        # logging.info('get_os_version: %s .', os_version)
        return os_version

    def get_current_version(self):
        os_version = ''
        update_version = ''
        try:
            if not self.ucconfigs:
                logging.warning("System version file status abnormal ...")
                os.remove(SYSTEM_VERSION_FILE)
                self.ucconfigs = UpgradeConfig(datadir = "/etc/kylin-version", name = "kylin-system-version.conf")

            if not os.path.exists(SYSTEM_VERSION_FILE) or \
                    not self.ucconfigs.has_option("SYSTEM","os_version") or \
                    not self.ucconfigs.has_option("SYSTEM","update_version") or \
                    not self.ucconfigs.has_option("SYSTEM","quality_version"): 
                logging.warning("System version file format abnormal ...")

                default_os_version = self.get_os_version()
                if not self.ucconfigs.has_section("SYSTEM"):
                    self.ucconfigs.add_section("SYSTEM")

                if not self.ucconfigs.has_option("SYSTEM", "os_version"):
                    self.ucconfigs.set("SYSTEM", "os_version", str(default_os_version))

                if not self.ucconfigs.has_option("SYSTEM", "update_version"):
                    self.ucconfigs.set("SYSTEM", "update_version", str(default_os_version))

                if not self.ucconfigs.has_option("SYSTEM", "quality_version"):
                    self.ucconfigs.set("SYSTEM", "quality_version", "")

                with open(SYSTEM_VERSION_FILE, 'w') as configfile:
                    self.ucconfigs.write(configfile)

            os_version = str(self.ucconfigs.get("SYSTEM","os_version")).strip('"\'')
            update_version = str(self.ucconfigs.get("SYSTEM","update_version")).strip('"\'')
        except Exception as e:
            logging.error("get_current_version error: %s", e)

        # logging.info('Current os_version: %s, release_id: %s .', os_version, update_version)
        return str(update_version),str(os_version)
    
    def get_cn_appname(self, name):
        try: 
            SC_DB_FILE = "/usr/share/kylin-software-center/data/uksc.db"
            if os.path.isfile(SC_DB_FILE):
                connect = sqlite3.connect(SC_DB_FILE, check_same_thread=False)
                cursor = connect.cursor()
            else:
                logging.warning("software center database isn't exist .")
                return ""
            sql = "select display_name_cn from application where display_name='"+name+"'"
            cursor.execute(sql)
            connect.commit()
            retval = cursor.fetchone()
            connect.close()
            if retval != None and len(retval) != 0:
                return str(retval[0])
            else:
                return ''
        except Exception as e:
            logging.error(_("Failed to initialize the database: %s"), str(e))
            return ''

    def insert_upgrade_history(self, args, caller):
        caller_list = ['kylin-unattended-upgrade', "d-feet", "root"]
        _in_list = False
        for cl in caller_list:
            if caller in cl:
                _in_list = True
        if _in_list == False:
            logging.warning("Caller \" %s \": Operation without permission...", caller)
            return False
        #    {"appname":GLib.Variant("s", "kylin-system-updater"), "version":GLib.Variant("s", "0.0")}
        #    "description":GLib.Variant("s", "Update Manager for Kylin"), "date":GLib.Variant("s", "2022-07-27 15:23:51")
        #    "status":GLib.Variant("s", "failed"), "keyword":GLib.Variant("s", "1")
        #    "errorcode":GLib.Variant("s", "System upgrade is complete. "), "appname_cn":GLib.Variant("s", "音乐")
        upgrade_info = {}
        try:
            for it in args:
                upgrade_info[str(it)] = str(args[str(it)])
            logging.info("upgrade_info: %s", upgrade_info)

            if "appname" in upgrade_info.keys() and "version" in upgrade_info.keys() \
                                                and "description" in upgrade_info.keys() \
                                                and "date" in upgrade_info.keys() \
                                                and "status" in upgrade_info.keys() \
                                                and "keyword" in upgrade_info.keys() \
                                                and "errorcode" in upgrade_info.keys() \
                                                and "appname_cn" in upgrade_info.keys() \
                                                and "status_cn" in upgrade_info.keys() \
                                                and "changelog" in upgrade_info.keys():
                appname = upgrade_info["appname"]
                if "kylin-unattended-upgrade" == appname:
                    upgrade_info["appname"] = self.get_cn_appname(appname)
                    if upgrade_info["appname"] == "":
                        upgrade_info["appname"] = _("kylin-unattended-upgrade")
                    if appname in self.window_main.cache and upgrade_info["description"] == "":
                        pkg =  self.window_main.cache[appname]
                        if pkg.is_installed:
                            upgrade_info["description"] = pkg.installed.description
                self.insert_into_updateinfo( upgrade_info["appname"],    upgrade_info["version"], \
                                                upgrade_info["description"], \
                                                upgrade_info["date"], \
                                                upgrade_info["status"], \
                                                upgrade_info["keyword"], \
                                                upgrade_info["errorcode"], \
                                                upgrade_info["appname_cn"], \
                                                upgrade_info["status_cn"], \
                                                upgrade_info["changelog"] )
            else:
                logging.warning("Incomplete field.")
                return False
        except Exception as e:
            logging.error(e)
            return False
class DataMigration():
    def __init__(self):
        pass

    #connect连接数据库
    def connect_database(self):
        try: 
            self.connect = sqlite3.connect(DB_FILE, check_same_thread=False)
            self.cursor = self.connect.cursor()
        except Exception as e:
            logging.error("Failed to connect database: %s", str(e))
    
    #disconnect连接数据库
    def disconnect_database(self):
        try: 
            if self.connect != None:
                self.connect.close()
            if self.cursor != None:
                del self.cursor
        except Exception as e:
            logging.error("Failed to disconnect database: %s", str(e))

    def MarkFirstMigration(self):
        if (os.path.exists(DB_UPGRADE)):
            try:
                new_db = sqlite3.connect(DB_UPGRADE, check_same_thread=False)
                new_db_cursor = new_db.cursor()

                if (self._has_first_migration(new_db, new_db_cursor)): # 存在 firstmigration
                    if (self._is_first_migration(new_db, new_db_cursor)): 
                        sql_commnd = "UPDATE display SET firstmigration='false';"
                        new_db_cursor.execute(sql_commnd)
                        new_db.commit()
                else:# 不存在firstmigration
                    # 新增  firstmigration 字段
                    sql_commnd = "alter table display add column firstmigration text;"
                    new_db_cursor.execute(sql_commnd)
                    sql_commnd = "UPDATE display SET firstmigration='false';"
                    new_db_cursor.execute(sql_commnd)
                    new_db.commit()
            except Exception as e:
                logging.error("Failed to disconnect database: %s", str(e))
        else :
            logging.info("Not found kylin-system-updater.db, ensure that \'kylin-system-updater\' is successfully installed ... ")

    def CopyData(self):
        # 判断数据库是否存在
        if (os.path.exists(VER_DB) and os.path.exists(DB_UPGRADE)):
            try:
                new_db = sqlite3.connect(DB_UPGRADE, check_same_thread=False)
                new_db_cursor = new_db.cursor()
                ver_db = sqlite3.connect(VER_DB, check_same_thread=False)
                ver_db_cursor = ver_db.cursor()

                if (self._has_first_migration(new_db, new_db_cursor)): # 存在 firstmigration
                    if (self._is_first_migration(new_db, new_db_cursor)): 
                        # 数据迁移
                        if (self.dateMigration(new_db=new_db, new_db_cursor=new_db_cursor)):
                            sql_commnd = "UPDATE display SET firstmigration='false';"
                            new_db_cursor.execute(sql_commnd)
                            new_db.commit()
                    else:
                        logging.info("No data migration is required ...")
                else:# 不存在firstmigration
                    # 新增  firstmigration 字段
                    sql_commnd = "alter table display add column firstmigration text;"
                    new_db_cursor.execute(sql_commnd)
                    sql_commnd = "UPDATE display SET firstmigration='yes';"
                    new_db_cursor.execute(sql_commnd)
                    #数据迁移
                    if (self.dateMigration(new_db=new_db, new_db_cursor=new_db_cursor)):
                        sql_commnd = "UPDATE display SET firstmigration='false';"
                        new_db_cursor.execute(sql_commnd)
                        new_db.commit()
            except Exception as e:
                logging.error("Failed to disconnect database: %s", str(e))
        else :
            logging.info("Not found kylin-system-updater.db, ensure that \'kylin-system-updater\' is successfully installed ... ")
    
    def _has_first_migration(self, new_db, new_db_cursor)->bool:
        try:
            sql_commnd = "select * from sqlite_master where type='table' and name='display';"
            new_db_cursor.execute(sql_commnd)
            retval = new_db_cursor.fetchone()
            for rv in retval:
                if "firstmigration" in str(rv):
                    return True
        except Exception as e:
            logging.error("Failed to get field firstMigration: %s", str(e))
        return False

    def _is_first_migration(self, new_db, new_db_cursor):
        try:
            sql_commnd = "select firstmigration from display;"
            new_db_cursor.execute(sql_commnd)
            retval = new_db_cursor.fetchone()
            if "true" in retval or "yes" in retval:
                return True
        except Exception as e:
            logging.error("Failed to select firstMigration: %s", str(e))
        return False

    def _is_display_exist_fields(self, field, db, db_cursor):
        try:
            sql_commnd = "select * from sqlite_master where type='table' and name='display';"
            db_cursor.execute(sql_commnd)
            retval = db_cursor.fetchone()
            for rv in retval:
                if field in str(rv):
                    return True
        except Exception as e:
            logging.error("Failed to select %s: %s", field, str(e))
        return False

    def _add_display_fields(self, fields_default):
        try:
            if "=" not in fields_default:
                print("format: field=value")
                return False
            field, value = fields_default.split('=')
            print(_("Loading Sqlite3Server..."))
            db = sqlite3.connect(DB_UPGRADE, check_same_thread=False)
            db_cursor = db.cursor()
            if self._is_display_exist_fields(field, db, db_cursor):
                print("field %s is exist."%field)
                return False
            # 字段不存在,新增字段
            sql_commnd = "alter table display add column "+field+" TEXT;"
            db_cursor.execute(sql_commnd)
            sql_commnd = "UPDATE display SET "+field+"='"+value+"'"
            db_cursor.execute(sql_commnd)
            db.commit()
        except Exception as e:
            print(e)
            return False
        print("Succeeded in adding field: '%s' "%field)
        return True

    def dateMigration(self,new_db,new_db_cursor,old_db=None,old_db_cursor=None)->bool:
        sql_commnd = ""
        update_record_dict = {}
        tmp_update_record_dict = []
        old_cfg_dict = {}
        new_cfg_dict = {}
        
        try:
            # step 1: 数据库初始化
            if old_db==None and old_db_cursor==None:
                old_db = sqlite3.connect(DB_UPDATER, check_same_thread=False)
                old_db_cursor = old_db.cursor()
        
            # step 2: 获取更新数据
            sql_commnd = "SELECT * FROM installed"
            old_db_cursor.execute(sql_commnd)
            update_record = old_db_cursor.fetchall()

            for ur in update_record:
                id,appname,version,time,description,icon,statue,keyword,errorcode = ur
                if keyword == '2': #2：本条数据是deb安装记录,不迁移
                    continue
                if errorcode in range(200):
                    errorcode = 'null'
                update_record_dict.clear()
                update_record_dict.update({"appname":appname})
                update_record_dict.update({"version":version})
                update_record_dict.update({"time":time})
                update_record_dict.update({"description":description})
                update_record_dict.update({"icon":icon})
                if statue == 'Success':
                    statue = 'success'
                elif statue == 'Fail':
                    statue = 'failed'
                update_record_dict.update({"statue":statue})
                update_record_dict.update({"keyword":'1'})
                update_record_dict.update({"errorcode":errorcode})
                tmp_update_record_dict.append(update_record_dict.copy())

            # 按时间排序
            tmp_update_record_dict = sorted(tmp_update_record_dict, key=itemgetter('time'))
            logging.info("Generate update records complete.")
            
            # step 3: insert新生成的记录
            sql_commnd = "create table IF NOT EXISTS tmp('id' INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,\
            'appname'	TEXT,\
            'version'	TEXT,\
            'description'	TEXT,\
            'date'	TEXT,\
            'status'	TEXT,\
            'keyword'	TEXT,\
            'errorcode' TEXT,\
            'appname_cn' TEXT,\
            'status_cn' TEXT,\
            'changelog' TEXT) "
            new_db_cursor.execute(sql_commnd)
            # 更新数据
            for urd in tmp_update_record_dict:
                new_db_cursor.execute(
                "insert into tmp (appname, version, description, date, status, keyword, errorcode) values(?,"
                "?,?,?,?,?,?)",
                (urd['appname'], urd['version'], urd['description'], urd['time'], urd['statue'], urd['keyword'], urd['errorcode']))
                new_db.commit()

            # 备份updateinfos
            new_db_cursor.execute(
                "insert into tmp (appname, version, description, date, status, keyword, errorcode, appname_cn, status_cn, changelog) "
                "SELECT appname, version, description, date, status, keyword, errorcode, appname_cn, status_cn, changelog FROM updateinfos")
            new_db.commit()
            
            # 删除updateinfos
            sql_commnd = "drop table updateinfos"
            new_db_cursor.execute(sql_commnd)
            new_db.commit()

            # 修改表名
            sql_commnd = "alter table tmp rename to updateinfos"
            new_db_cursor.execute(sql_commnd)
            new_db.commit()
            logging.info("Insert update records complete.")
            
            # step 4: 更新配置转移
            sql_commnd = "SELECT * FROM display where id=1"
            old_db_cursor.execute(sql_commnd)
            old_cfg = old_db_cursor.fetchone()
            if not old_cfg:
                logging.warning("Table 'display' is empty .")
            else:
                for od in old_db_cursor.description:
                    old_cfg_dict.update({str(od[0]):old_cfg[old_db_cursor.description.index(od)]})
                if old_db != None:
                    old_db.close()
                if old_db_cursor != None:
                    del old_db_cursor
                new_db_cursor.execute(sql_commnd)
                new_cfg = new_db_cursor.fetchone()
                for od in new_db_cursor.description:
                    new_cfg_dict.update({str(od[0]):new_cfg[new_db_cursor.description.index(od)]})

                sql_commnd = "UPDATE display set check_time='"+old_cfg_dict['check_time']+"' Where id=1"
                new_db_cursor.execute(sql_commnd)
                new_db.commit()
                sql_commnd = "UPDATE display set update_time='"+old_cfg_dict['update_time']+"' Where id=1"
                new_db_cursor.execute(sql_commnd)
                new_db.commit()
                sql_commnd = "UPDATE display set auto_check='"+old_cfg_dict['auto_check']+"' Where id=1"
                new_db_cursor.execute(sql_commnd)
                new_db.commit()
                sql_commnd = "UPDATE display set system_version='"+old_cfg_dict['system_version']+"' Where id=1"
                new_db_cursor.execute(sql_commnd)
                new_db.commit()
                if old_cfg_dict['auto_backup'] != None:
                    sql_commnd = "UPDATE display set auto_backup='"+old_cfg_dict['auto_backup']+"' Where id=1"
                    new_db_cursor.execute(sql_commnd)
                    new_db.commit()
                if 'download_limit' in old_cfg_dict.keys() and old_cfg_dict['download_limit'] != None:
                    sql_commnd = "UPDATE display set download_limit='"+old_cfg_dict['download_limit']+"' Where id=1"
                    new_db_cursor.execute(sql_commnd)
                    new_db.commit()
                if 'download_limit_value' in old_cfg_dict.keys() and old_cfg_dict['download_limit_value'] != None:
                    sql_commnd = "UPDATE display set download_limit_value='"+old_cfg_dict['download_limit_value']+"' Where id=1"
                    new_db_cursor.execute(sql_commnd)
                    new_db.commit()

        except Exception as e:
            logging.warning("Update table 'display' error: %s .",e)
        logging.info("The data migration is complete.")
        return True

def listtojsonstr(lists):
    import json
    jsonfile = json.dumps(lists)
    return jsonfile

class TransactionManager:
    TRANSTYPE_INSTALL = 30
    TRANSTYPE_PURGE   = 10

    def __init__(self, updateDaemon):
        self.updateDaemon = updateDaemon
        self.name_timers = {}
        self.bus = dbus.SystemBus()
    
    def signal_handle_install(self,name,ret):
        logging.info("receive installed:%s, %d",name,ret)
        self.stop_timer(name)
        self.updateDaemon.dbus_send.InstalldebFinished (True if ret==0 else False,'kare install success:'+name,'')
    
    def signal_handler_purge(self,name,ret):
        logging.info("receive purged:%s, %d",name,ret)
        self.stop_timer(name)
        self.updateDaemon.dbus_send.PurgePackagesFinished (True if ret==0 else False,'kare remove success:'+name,'')
    
    def signal_installed(self):
        self.bus.add_signal_receiver (
            self.signal_handle_install,
            signal_name     = "installFinished",
            dbus_interface  = "org.kylin.kare",
            bus_name        = "org.kylin.kare",
            path            = "/org/kylin/kare"
        )

    def signal_purged(self):
        self.bus.add_signal_receiver (
            self.signal_handler_purge,
            signal_name     = "removeFinished",
            dbus_interface  = "org.kylin.kare",
            bus_name        = "org.kylin.kare",
            path            = "/org/kylin/kare"
        )

    def start_transaction(self, name, transTimeout = TRANSTYPE_INSTALL):
        if transTimeout == self.TRANSTYPE_INSTALL:
            self.signal_installed()
        if transTimeout == self.TRANSTYPE_PURGE:
            self.signal_purged()

        logging.info("Transaction running:'%s', timeout: %s min", name, transTimeout)
        timer = threading.Timer(60 * transTimeout, function=self.timeout_callback, args=(name,))
        timer.start()
        self.name_timers[name] = timer

        logging.info(self.name_timers)

    def timeout_callback(self, name):
        logging.info("Transaction '%s' timed out.",name)
        self.exit_transaction(name)
        del self.name_timers[name]

        if not self.name_timers:
            self.disconnect_signal()

    def exit_transaction(self, name):
        logging.info("Exiting transaction '%s'.",name)

    def stop_timer(self, name):
        if name in self.name_timers.keys():
            self.name_timers[name].cancel()
            del self.name_timers[name]
            logging.info("Timer for transaction '%s' stopped.",name)
        else:
            logging.info("No active timer found for transaction '%s'.",name)
        
        if not self.name_timers:
            self.disconnect_signal()

    def stop_all_timers(self):
        for name, timer in self.name_timers.items():
            timer.cancel()
            logging.info("timer cancel '%s'.",name)

        self.name_timers.clear()

        self.disconnect_signal()

    def disconnect_signal(self):
        # return #发现某些时候收不到返回信号,暂时不断开
        logging.info("disconnect from removeFinished")
        self.bus.remove_signal_receiver (
            self.signal_handler_purge,
            signal_name     = "removeFinished",
            dbus_interface  = "org.kylin.kare",
            bus_name        = "org.kylin.kare",
            path            = "/org/kylin/kare"
        )

        logging.info("disconnect from installFinished")
        self.bus.remove_signal_receiver (
            self.signal_handle_install,
            signal_name     = "installFinished",
            dbus_interface  = "org.kylin.kare",
            bus_name        = "org.kylin.kare",
            path            = "/org/kylin/kare"
        )