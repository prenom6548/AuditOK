#!/usr/bin/python3
# -*- coding: utf-8 -*-
from math import log10
import os
import re
import json
import yaml
import shutil
import sqlite3
import logging
import datetime
from operator import itemgetter
from gettext import gettext as _
from sys import exec_prefix
from SystemUpdater.Core.DataAcquisition import PHPSeverSend
from SystemUpdater.Core.UpdaterConfigParser import UpgradeConfig
from SystemUpdater.Core.utils import get_config_patch

import apt_pkg
from ..backend import InstallBackend

DB_FILE = os.path.join("/var/cache/kylin-system-updater/kylin-system-updater.db")
# UMDB_FILE = os.path.join("/var/cache/kylin-system-updater/kylin-system-updater.db")
DB_UPDATER = "/var/cache/kylin-update-manager/kylin-update-manager.db"
DB_UPGRADE = "/var/cache/kylin-system-updater/kylin-system-updater.db"
VER_DB = "/usr/share/kylin-system-updater/kylin-system-updater.db"
SYSTEM_VERSION_FILE = "/etc/kylin-version/kylin-system-version.conf"
INSTALLED_LIST = [{"item": "errorcode", "type": "int", "default": "0"}]
DISPALY_LIST = []

class Sqlite3Server(object):
    def __init__(self, updateManager, _no_DataMigration=False):
        self.connect = None
        self.window_main = updateManager
        self.config_path = get_config_patch()
        self.init_sqlit()

        # uncoverable配置文件
        self._ensure_system_version_config()
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
        self.connect_database()
        try:
            sql = "update display set " + args[0] + "='" + args[1] + "' where id = 1"
            self.cursor.execute(sql)
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

    # 接收更新列表与信息，生成数据并插入数据库中
    def insert_info(self, mode, pkg_list=[], pkg_group=[], adjust_pkg=[], success = False, error_string = '', error_desc = ''):
        errstr = error_string + " " + error_desc
        status = " "
        status_cn = " "
        appname_cn = ""
        errorcode = "10000"
        UpdateInfos  = {}
        InstallInfos = {}
        time = datetime.datetime.now()
        timestr = datetime.datetime.strftime(time, "%Y-%m-%d %H:%M:%S")
        pkg_list = pkg_list.copy()
        pkg_group = pkg_group.copy()
        adjust_pkg = adjust_pkg.copy()
        # 更新列表空，无更新
        if not pkg_list and not pkg_group and mode != InstallBackend.MODE_INSTALL_SYSTEM:
            logging.info("There is no update.")
            return True

        if success:
            status = 'success'
            status_cn = '成功'
            errorcode = "10000000"
        else:
            status = 'failed'
            status_cn = '失败'
            errorcode = "10000100"
        changeLog = ""

        try:
            # 判断更新方式
            if mode == InstallBackend.MODE_INSTALL_PARTIAL: # 部分更新
                pkg_adj = ""
                # 判断更新包为单包或更新组
                if pkg_group:
                # 更新组
                    pkgname = pkg_group.pop(0)
                    pkgversion,pkgdescription,appname_cn = self.GetGroupmsg(pkgname)
                    #更新信息update-infos
                    UpdateInfos.update({"appname":str(pkgname)})
                    UpdateInfos.update({"source":"Kylin System Updater"})
                    UpdateInfos.update({"status":status})
                    UpdateInfos.update({"errorCode":str(errorcode)})
                    UpdateInfos.update({"error_string":str(error_string)})
                    UpdateInfos.update({"error_desc":str(error_desc)})
                    self.window_main.collector.Upgrade_Process_Msg("finish-update", UpdateInfos.copy())
                    #安装信息install-infos
                    InstallInfos.update({"appname":str(pkgname)})
                    if pkgname in self.window_main.main_meta.versoin_pkgs['groups_upgrade'].keys():
                        InstallInfos.update({"old_version":str(self.window_main.main_meta.versoin_pkgs['groups_upgrade'][pkgname])})
                    else:
                        InstallInfos.update({"old_version":'UnKnown'})
                    InstallInfos.update({"new_version":str(pkgversion)})
                    InstallInfos.update({"status":status})
                    InstallInfos.update({"errorCode":str(errorcode)})
                    InstallInfos.update({"error_string":str(error_string)})
                    InstallInfos.update({"error_desc":str(error_desc)})
                    self.window_main.collector.Upgrade_Process_Msg("finish-install", InstallInfos.copy())
                    # 系统升级完成 ..判断版本号
                    if status == "success" and "kylin-update-desktop-system" in pkgname:
                        # 更新版本号
                        if "=" in str(pkgversion):
                            pkgversion = str(pkgversion).split('=')[-1]
                        logging.info("Complete system upgrade, refresh system version ...")
                        self._refresh_system_version(pkgversion)
                        #移除step-two标记
                        self._removal_of_marker()

                    #FIXME: 临时方案 PHP
                    PHPSeverSend(_appname=pkgname, _appversion=pkgversion, _statue=status, _errorcode=str(errorcode), _errorstring=errstr)
                    file_path = os.path.join(get_config_patch(), str(pkgname) + ".yaml")
                    with open(file_path, "r") as stream:
                        try:
                            data_yaml = yaml.safe_load(stream)
                            changeLog = data_yaml['changelog']
                        except yaml.YAMLError as exc:
                            logging.error(exc)
                elif pkg_list:
                    changeLog = " "
                    # 单包更新 # 获取单包数据插入数据库
                    pkgname = pkg_list.pop(0)
                    for adj in adjust_pkg:
                        if pkgname in adj:
                        # 该部分升级的单包为调整版本，与候选版本不一致
                            pkg_adj = adj
                            break
                    if pkg_adj: # 有调整的情况
                        try:
                            pkg =  self.window_main.cache[pkg_adj.split("=")[0]]
                            for ver in pkg.versions:
                                if ver.version == pkg_adj.split("=")[1]:
                                    pkg_inst_ver = ver
                                    break
                            pkgname = pkg_adj.split("=")[0]
                            pkgversion = str(pkg_inst_ver.source_version)
                            pkgdescription = str(pkg_inst_ver.description)
                        except Exception as e:
                            logging.error(_("%s could not be detected in the source because the source was changed or for other reasons."), \
                                str(pkgname))
                            logging.error(str(e))
                    else: # 没有调整的情况
                        try:
                            pkg =  self.window_main.cache[pkgname]
                            pkgversion = str(pkg.candidate.version)
                            pkgdescription = str(pkg.candidate.raw_description)
                        except Exception as e:
                            logging.error(str(e))

                    #更新信息update-infos
                    UpdateInfos.update({"appname":str(pkgname)})
                    UpdateInfos.update({"source":"Kylin System Updater"})
                    UpdateInfos.update({"status":status})
                    UpdateInfos.update({"errorCode":str(errorcode)})
                    UpdateInfos.update({"error_string":str(error_string)})
                    UpdateInfos.update({"error_desc":str(error_desc)})
                    self.window_main.collector.Upgrade_Process_Msg("finish-update", UpdateInfos.copy())
                    #安装信息install-infos
                    InstallInfos.update({"appname":str(pkgname)})
                    if pkgname in self.window_main.main_meta.versoin_pkgs['single_upgrade'].keys():
                        InstallInfos.update({"old_version":str(self.window_main.main_meta.versoin_pkgs['single_upgrade'][pkgname])})
                    else:
                        InstallInfos.update({"old_version":'UnKnown'})
                    InstallInfos.update({"new_version":str(pkgversion)})
                    InstallInfos.update({"status":status})
                    InstallInfos.update({"errorCode":str(errorcode)})
                    InstallInfos.update({"error_string":str(error_string)})
                    InstallInfos.update({"error_desc":str(error_desc)})
                    self.window_main.collector.Upgrade_Process_Msg("finish-install", InstallInfos.copy())
                    # 软件商店获取中文名
                    appname_cn = self.get_cn_appname(str(pkgname))
                    #FIXME: 临时方案 PHP
                    PHPSeverSend(_appname=pkgname, _appversion=pkgversion, _statue=status, _errorcode=str(errorcode), _errorstring=errstr)
                try:
                    self.insert_into_updateinfo(pkgname, pkgversion, pkgdescription, timestr, status, "1", errstr, appname_cn, status_cn, changeLog)
                    # FIXME: 发送插入数据库成功的信号local_upgrade_list
                    self.window_main.dbusController.UpdateSqlitSingle(pkgname, timestr)
                # 数据库文件被删除或者新增字段导致需要重新初始化数据库再写入
                except Exception as e:
                    self.init_sqlit()
                    self.insert_into_updateinfo(pkgname, pkgversion, pkgdescription, timestr, status, "1", errstr, appname_cn, status_cn, changeLog)
                    # FIXME: 这里也需要, 发送插入数据库成功的信号 
                    self.window_main.dbusController.UpdateSqlitSingle(pkgname, timestr)
            elif mode == InstallBackend.MODE_INSTALL_ALL: # 系统全部升级
                # # insert signal deb first
                for i in pkg_list:
                    changeLog = ""
                    try:
                        pkg =  self.window_main.cache[i]
                    except Exception as e:
                        logging.error(_("%s could not be detected in the source because the source was changed or for other reasons."), \
                                str(i))
                        continue
                    if not pkg:
                        continue 
                    pkgversion = str(pkg.candidate.version)
                    pkgdescription = str(pkg.candidate.raw_description)

                    #更新信息update-infos
                    UpdateInfos.update({"appname":str(pkg.name)})
                    UpdateInfos.update({"source":"Kylin System Updater"})
                    UpdateInfos.update({"status":status})
                    UpdateInfos.update({"errorCode":str(errorcode)})
                    UpdateInfos.update({"error_string":str(error_string)})
                    UpdateInfos.update({"error_desc":str(error_desc)})
                    self.window_main.collector.Upgrade_Process_Msg("finish-update", UpdateInfos.copy())
                    #安装信息install-infos
                    InstallInfos.update({"appname":str(pkg.name)})
                    if pkg.name in self.window_main.main_meta.versoin_pkgs['groups_upgrade'].keys():
                        InstallInfos.update({"old_version":str(self.window_main.main_meta.versoin_pkgs['groups_upgrade'][pkg.name])})
                    else:
                        InstallInfos.update({"old_version":'UnKnown'})
                    InstallInfos.update({"new_version":str(pkgversion)})
                    InstallInfos.update({"status":status})
                    InstallInfos.update({"errorCode":str(errorcode)})
                    InstallInfos.update({"error_string":str(error_string)})
                    InstallInfos.update({"error_desc":str(error_desc)})
                    self.window_main.collector.Upgrade_Process_Msg("finish-install", InstallInfos.copy())

                    try:
                        # 软件商店获取中文名
                        appname_cn = self.get_cn_appname(str(i))
                        self.insert_into_updateinfo(str(i), pkgversion, pkgdescription, timestr, status, "1", errstr, appname_cn, status_cn, changeLog)
                        self.window_main.dbusController.UpdateSqlitSingle(str(i), timestr)
                    # 数据库文件被删除或者新增字段导致需要重新初始化数据库再写入
                    except Exception as e:
                        self.init_sqlit()
                        self.insert_into_updateinfo(str(i), pkgversion, pkgdescription, timestr, status, "1", errstr, appname_cn, status_cn, changeLog)
                        self.window_main.dbusController.UpdateSqlitSingle(str(i), timestr)
                    #FIXME: 临时方案 PHP
                    PHPSeverSend(_appname=pkg.name, _appversion=pkgversion, _statue=status, _errorcode=str(errorcode), _errorstring=errstr)
                # insert group deb next
                for i in pkg_group:
                    # FIXME: 获取组信息
                    pkgversion,pkgdescription,appname_cn = self.GetGroupmsg(i)
                    #更新信息update-infos
                    UpdateInfos.update({"appname":str(i)})
                    UpdateInfos.update({"source":"Kylin System Updater"})
                    UpdateInfos.update({"status":status})
                    UpdateInfos.update({"errorCode":str(errorcode)})
                    UpdateInfos.update({"error_string":str(error_string)})
                    UpdateInfos.update({"error_desc":str(error_desc)})
                    self.window_main.collector.Upgrade_Process_Msg("finish-update", UpdateInfos.copy())
                    #安装信息install-infos
                    InstallInfos.update({"appname":str(i)})
                    if i in self.window_main.main_meta.versoin_pkgs['groups_upgrade'].keys():
                        InstallInfos.update({"old_version":str(self.window_main.main_meta.versoin_pkgs['groups_upgrade'][i])})
                    else:
                        InstallInfos.update({"old_version":'UnKnown'})
                    InstallInfos.update({"new_version":str(pkgversion)})
                    InstallInfos.update({"status":status})
                    InstallInfos.update({"errorCode":str(errorcode)})
                    InstallInfos.update({"error_string":str(error_string)})
                    InstallInfos.update({"error_desc":str(error_desc)})
                    json_file = json.dumps(InstallInfos.copy())
                    try:
                        self.window_main.collector.UpdateMsg("InstallInfos", json_file)
                    except:
                        pass

                    #FIXME: 临时方案 PHP
                    PHPSeverSend(_appname=i, _appversion=pkgversion, _statue=status, _errorcode=str(errorcode), _errorstring=errstr)
                    file_path = os.path.join(get_config_patch(), str(i) + ".yaml")
                    with open(file_path, "r") as stream:
                        try:
                            data_yaml = yaml.safe_load(stream)
                            changeLog = data_yaml['changelog']
                        except yaml.YAMLError as exc:
                            logging.error(exc)
                    try:
                        self.insert_into_updateinfo(str(i), pkgversion, pkgdescription, timestr, status, "1", errstr, appname_cn, status_cn, changeLog)
                        self.window_main.dbusController.UpdateSqlitSingle(str(i), timestr)
                    # 数据库文件被删除或者新增字段导致需要重新初始化数据库再写入
                    except Exception as e:
                        self.init_sqlit()
                        self.insert_into_updateinfo(str(i), pkgversion, pkgdescription, timestr, status, "1", errstr, appname_cn, status_cn, changeLog)
                        self.window_main.dbusController.UpdateSqlitSingle(str(i), timestr)
                
                    # 系统升级完成 ..判断版本号
                    if status == "success" and "kylin-update-desktop-system" in pkg_group:
                        # 更新版本号
                        if "=" in str(pkgversion):
                            pkgversion = str(pkgversion).split('=')[-1]
                        logging.info("Complete system upgrade, refresh system version ...")
                        self._refresh_system_version(str(pkgversion))
                        #移除step-two标记
                        self._removal_of_marker()

            elif mode == InstallBackend.MODE_INSTALL_SYSTEM: # 全盘升级
                self.insert_into_updateinfo(_("Upgrade System"), "", "This is a complete system upgrade, equivalent to the implementation of apt dist-upgrade", timestr, status, "1", errstr, str("全盘升级"), status_cn, " ")
                self.window_main.dbusController.UpdateSqlitSingle("Upgrade System", timestr)
                # 全盘更新完成 ..判断版本号
                if status == "success":
                    # 更新版本号
                    self._refresh_system_version(pseudo_version=True)
                    #移除step-two标记
                    self._removal_of_marker()
            else:
                logging.warning("Cache is None.")
        except Exception as e:
            logging.error("record update error: %s.",str(e))
        self.window_main.collector.Msg_Clean()

    # 获取group信息    
    def GetGroupmsg(self, appname):
        jsonfile = appname+".json"
        files = os.listdir(self.config_path) #获取文件夹中所有文件
        if jsonfile in files: # 存在
            # 读取组JSON文件
            with open(self.config_path+jsonfile, "r") as f:
                try :
                    data = json.load(f)
                except json.JSONDecodeError as e:
                    logging.error(str(e))
            try:
                version = data['version']
                if "=" in version:
                    version = version.split("=")[1].strip()
                tmpdescription = data['description']
                appname_cn = data['name']['zh_CN']
            except Exception as e:
                logging.error(str(e))
            if "zh_CN" in tmpdescription and "en_US" in tmpdescription:
                description = tmpdescription["zh_CN"] + ": " + tmpdescription["en_US"]
            return (version,description,appname_cn)
        else: #  不存在
            return (None, None, None)

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

    # #查找数据库
    # def find_msg_from_datebase(self, table, field, action = 'check', cid = 0):
    #     # 查询数据
    #     try:
    #         sql = "select "+field+" from "+table
    #         self.cursor.execute(sql)
    #         update_count = self.cursor.fetchone()[0]
    #         logging.info("%d history updates detected.", update_count)
    #     except Exception as e:
    #         logging.error("Check update error: %s", str(e))
    
    def _system_version_config(self):
        self.connect_database()
        try:
            sql = "select init_version from display where id=1"
            self.cursor.execute(sql)
            _is_init_verison = self.cursor.fetchone()[0]
            if _is_init_verison == "yes"  or _is_init_verison == "true":
                update_version, os_version = self.get_default_version()
                logging.info("Need to refresh version ...")
                self._refresh_system_version(update_version, os_version)
                sql = "update display set init_version = 'no'"
                self.cursor.execute(sql)
                self.connect.commit()
        except Exception as e:
            logging.error(str(e))
        self.disconnect_database()
    
    def _ensure_system_version_config(self):
        try:
            config_file = SYSTEM_VERSION_FILE
            config_dir = os.path.dirname(config_file)
            if config_dir and not os.path.isdir(config_dir):
                os.makedirs(config_dir)
            if not os.path.exists(config_file):
                with open(config_file, 'a'):
                    pass

            changed = False
            if not getattr(self, 'ucconfigs', None):
                self.ucconfigs = UpgradeConfig(
                    datadir=config_dir, name=os.path.basename(config_file),
                    allow_no_section=True)
            else:
                self.ucconfigs.reRead()

            if not self.ucconfigs.has_section("SYSTEM"):
                return True
            if not self.ucconfigs.has_option("SYSTEM", "os_version"):
                self.ucconfigs.set("SYSTEM", "os_version", "3.0")
                changed = True
            if not self.ucconfigs.has_option("SYSTEM", "update_version"):
                self.ucconfigs.set("SYSTEM", "update_version", "")
                changed = True
            if changed:
                with open(config_file, 'w+') as configfile:
                    self.ucconfigs.write(configfile)
            return True
        except Exception as e:
            logging.error("Ensure system version config error: %s.", str(e))
            return False

    def _refresh_system_version(self, update_version='', os_version = '', pseudo_version = False):
        try:
            if "=" in update_version:
                update_version = str(update_version).split('=')[-1]
            if "=" in os_version:
                os_version = str(os_version).split('=')[-1]

            #刷新系统版本号:os_version
            if update_version == '':
                update_version, os_version = self.get_default_version()
            if os_version == '' and os.path.isfile("/etc/os-release"):
                with open("/etc/os-release", "r+") as f:
                    lines = f.readlines()
                for line in lines:
                    if "KYLIN_RELEASE_ID" in line:
                        os_version = line.split('=')[1]
                        os_version = os_version.strip().strip('"')
            if not pseudo_version:
                if len(os_version) != 0:
                    self.ucconfigs.setValue("SYSTEM","os_version",str(os_version),True)
                if len(update_version) != 0:
                    self.ucconfigs.setValue("SYSTEM","update_version",str(update_version),True)
            else:
                current_update_version, current_os_version = self.get_current_version()
                if current_os_version != os_version:
                    os_version+='*'
                    self.ucconfigs.setValue("SYSTEM","os_version",str(os_version),True)
                if current_update_version != update_version:
                    update_version+='*'
                    self.ucconfigs.setValue("SYSTEM","update_version",str(update_version),True)
        except Exception as e:
            logging.error("Refresh system version error: %s.",str(e))
    
    def get_default_version(self):
        update_version = ""
        os_version = ""
        INPUT_CONFIG_PATH = self.config_path + 'kylin-update-desktop-system.json'
        if os.path.isfile(INPUT_CONFIG_PATH): # 存在
            # 读取JSON文件
            with open(INPUT_CONFIG_PATH, "r") as f:
                try :
                    data = json.load(f)
                except json.JSONDecodeError as e:
                    logging.error(str(e))
            try:
                update_version = data['version']
                if "=" in update_version:
                    update_version = update_version.split('=')[-1].strip()
            except Exception as e:
                logging.error("get_default_version error: %s .",str(e))
        version_path = "/etc/os-release"
        if os.path.isfile(version_path):
            with open(version_path, "r+") as f:
                lines = f.readlines()
            for line in lines:
                if "KYLIN_RELEASE_ID" in line:
                    os_version = line.split('=')[1]
                    os_version = os_version.strip().strip('"')
        if update_version == "" and os_version != "":
            update_version = os_version
        elif update_version != "" and os_version == "":
            os_version = update_version
        return str(update_version),str(os_version)
    
    def get_current_version(self):
        os_version = ''
        update_version = ''
        try:
            if not os.path.exists(SYSTEM_VERSION_FILE):
                logging.warning("System version file doesn't exist.")
                return os_version,update_version
            os_version = str(self.window_main.sqlite3_server.ucconfigs.get(
                "SYSTEM", "os_version")).strip('"\'')
            update_version = str(self.window_main.sqlite3_server.ucconfigs.get(
                "SYSTEM", "update_version")).strip('"\'')
        except Exception as e:
            logging.error(str(e))
            return update_version,os_version
        logging.info('Current os_version: %s, release_id: %s .', os_version, update_version)
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
            if self.connect != None:
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
        try:
            if old_db==None and old_db_cursor==None:
                old_db = sqlite3.connect(DB_UPDATER, check_same_thread=False)
                old_db_cursor = old_db.cursor()
        except Exception as e:
            logging.error("Failed to dateMigration: %s", str(e))
            return False
        
        sql_commnd = ""
        update_record_dict = {}
        tmp_update_record_dict = []
        # step 2: 获取更新数据
        try:
            sql_commnd = "SELECT * FROM installed"
            old_db_cursor.execute(sql_commnd)
            update_record = old_db_cursor.fetchall()
            # sql_commnd = "SELECT * FROM updateinfos"
            # new_db_cursor.execute(sql_commnd)
            # new_update_record = new_db_cursor.fetchall()

            for ur in update_record:
                id,appname,version,time,description,icon,statue,keyword,errorcode = ur
                if errorcode in range(200):
                    errorcode = 'null'
                update_record_dict.clear()
                update_record_dict.update({"appname":appname})
                update_record_dict.update({"version":version})
                update_record_dict.update({"time":time})
                update_record_dict.update({"description":description})
                update_record_dict.update({"icon":icon})
                update_record_dict.update({"statue":statue})
                update_record_dict.update({"keyword":'1'})
                update_record_dict.update({"errorcode":errorcode})
                tmp_update_record_dict.append(update_record_dict.copy())
            # 按时间排序
            tmp_update_record_dict = sorted(tmp_update_record_dict, key=itemgetter('time'))
        except Exception as e:
            logging.error("Failed to generate update records: %s", str(e))
            return False
        logging.info("Generate update records complete.")
        # step 3: insert新生成的记录
        try:
            # 创建临时表
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
            
            # 删除updateinfos
            sql_commnd = "drop table updateinfos"
            new_db_cursor.execute(sql_commnd)
            new_db.commit()
            # 修改表名
            sql_commnd = "alter table tmp rename to updateinfos"
            new_db_cursor.execute(sql_commnd)
            new_db.commit()
        except Exception as e:
            logging.error("Failed to transfer data: %s", str(e))
            return False
        
        old_cfg_dict = {}
        new_cfg_dict = {}
        # step 4: 更新配置转移
        try:
            sql_commnd = "SELECT * FROM display where id=1"
            old_db_cursor.execute(sql_commnd)
            old_cfg = old_db_cursor.fetchone()
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
            print(e)
            print("更新配置文件错误")
            return 
        logging.info("The data migration is complete.")
        return True

def listtojsonstr(lists):
    import json
    jsonfile = json.dumps(lists)
    return jsonfile
