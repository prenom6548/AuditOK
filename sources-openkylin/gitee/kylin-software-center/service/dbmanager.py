#!/usr/bin/python3
# -*- coding: utf-8 -*-

import sqlite3
import os
from utils.aescrypt import *
import re
from models.enums import UBUNTUKYLIN_DATA_PATH, UKSC_CACHE_DIR,SYSTEM_VERSION,ARCH

DB_PATH = os.path.join(UBUNTUKYLIN_DATA_PATH, "uksc.db")
from models.globals import Globals
from utils.log_manager import LogManager
import gettext
import base64
gettext.textdomain("kylin-software-center")
_ = gettext.gettext

# DB_PATH = "../data/uksc.db"

QUERY_ALL_IMFORMATION = "select id,app_name,display_name,display_name_cn,categories,summary,description,rating_avg,rating_total,review_total,download_total,icon,cover_image,preview_image,arch,system_name,depends,cpu_model,no_cpu_model,certSubjectOu,softStyle,manufacturers,popularity,website from application order by download_total desc"
QUERY_ALL_IMFORMATION_BY_RATING = "select id,app_name,display_name,display_name_cn,categories,summary,description,rating_avg,rating_total,review_total,download_total,icon,cover_image,preview_image,arch,system_name,depends,cpu_model,no_cpu_model ,certSubjectOu,softStyle,manufacturers,popularity,website from application order by rating_avg desc,download_total desc"
QUERY_CATEGORY = "select * from category where name='%s'"
QUERY_APP = "select display_name, summary,description,rating_avg,rating_total,review_total,rank,download_total \
               from application where app_name='%s'"
QUERY_APPS = "select * from application order by download_total desc"
QUERY_APPS_RATING = "select * from application order by rating_avg,download_total desc"
UPDATE_APP_RNR = "update application set rating_average=%d,rating_total=%d, review_total=%d, \
        download_total=%d where app_name='%s'"
QUERY_CATEGORY_APPS = "select app_name,display_name,first_cat_name,secondary_cat_name,third_cat_name,rating_total,rank from application where first_cat_name='%s' or secondary_cat_name='%s' or third_cat_name='%s' order by rating_total DESC"

QUERY_NAME_CATEGORIES = "select id,app_name,categories,windows_app_name from xp order by priority asc"
QUERY_APP_ACCORD_CATEGORIES = "select app_name,display_name,windows_app_name,display_name_windows,description from xp where categories='%s'"
UPDATE_EXISTS = "update xp set exists_valid='%d' where id='%d'"

import threading
import datetime

lock = threading.Lock()


# from multiprocessing import Process,Lock
# lock = Lock()
class Review(object):
    def __init__(self, pkgname):
        self.package_name = pkgname
        self.id = None
        self.aid = None
        self.distroseries = None
        self.content = None
        self.user_display = None
        self.date = None
        self.language = None
        self.version = None
        self.up_total = None
        self.down_total = None
        self.user_rating=0

class Database:

    def __init__(self):
        self.updatecount = 0
        srcFile = os.path.join(UBUNTUKYLIN_DATA_PATH, "uksc.db")
        destFile = os.path.join(UKSC_CACHE_DIR, "uksc.db")
        newFile = os.path.join(UKSC_CACHE_DIR, "uksc_new.db")

        # no cache file, copy
        if not os.path.exists(destFile):
            if not os.path.exists(srcFile):
                Globals.CONNECT_DB = False
                if (Globals.DEBUG_SWITCH):
                    Globals.LOG_ERROR.logger.error(_("error with db file"))
                return
            os.system("cp " + srcFile + " " + destFile)
        try:
            self.connect = sqlite3.connect(destFile, timeout=30.0, check_same_thread=False)
            self.cursor = self.connect.cursor()
        except Exception as e:
            Globals.CONNECT_DB = False
            Globals.LOG_ERROR.logger.error(_("Database link failed:  ") + str(e))
        try:
            self.connect1 = sqlite3.connect(srcFile, timeout=30.0, check_same_thread=False)
            self.cursor1 = self.connect1.cursor()
        except Exception as e:
            Globals.CONNECT_DB = False
            Globals.LOG_ERROR.logger.error(_("Database link failed:  ") + str(e))
        res = None
        sql = "select value from dict where key='dbversion' or key = 'appinfo_updatetime'"
        try:
            self.cursor.execute(sql)
            res = self.cursor.fetchall()
        except Exception as e:
            Globals.CONNECT_DB = False
            Globals.LOG_ERROR.logger.error(_("Database link failed:  ") + str(e))
            if (str(e) == "database is locked"):
                self.reconnect_database()
                return
        install_db_version = ""
        cache_info_update_time = ""
        if (res != None and res != []):
            install_db_version = res[0][0]
            cache_info_update_time = res[1][0].split(".")[0]
        res = None
        try:
            self.cursor1.execute(sql)
            res = self.cursor1.fetchall()
        except Exception as e:
            Globals.CONNECT_DB = False
            Globals.LOG_ERROR.logger.error(_("Database link failed:  ") + str(e))
            if (str(e) == "database is locked"):
                self.reconnect_database()
                return
        db_version = ""
        install_info_update_time = ""
        if (res != None and db_version != []):
            db_version = res[0][0]
            install_info_update_time = res[1][0].split(".")[0]
        if (db_version != install_db_version):
            self.change_cache_db()
        else:
            if (install_info_update_time != "" and cache_info_update_time != ""):
                format_pattern = "%Y-%m-%d %H:%M:%S"
                difference = (datetime.datetime.strptime(str(install_info_update_time),
                                                         format_pattern) - datetime.datetime.strptime(
                    str(cache_info_update_time), format_pattern))
                if (difference.days <= 0):
                    pass
                else:
                    self.change_cache_db()

    def change_cache_db(self):
        srcFile = os.path.join(UBUNTUKYLIN_DATA_PATH, "uksc.db")
        destFile = os.path.join(UKSC_CACHE_DIR, "uksc.db")
        newFile = os.path.join(UKSC_CACHE_DIR, "uksc_new.db")
        self.cursor.close()
        self.connect.close()
        try:
            self.cursor1.close()
            self.connect1.close()
        except:
            pass
        if os.path.exists(destFile):
            if os.path.exists(srcFile):
                os.system("cp " + srcFile + " " + destFile)
                try:
                    self.connect = sqlite3.connect(destFile, timeout=10.0, check_same_thread=False)
                    self.cursor = self.connect.cursor()
                except Exception as e:
                    Globals.CONNECT_DB = False
                    Globals.LOG_ERROR.logger.error(_("Database link failed:  ") + str(e))
                    if (str(e) == "database is locked"):
                        self.reconnect_database()
                        return
            else:
                Globals.CONNECT_DB = False
        else:
            Globals.CONNECT_DB = False


    def reconnect_database(self):
        srcFile = os.path.join(UBUNTUKYLIN_DATA_PATH, "uksc.db")
        destFile = os.path.join(UKSC_CACHE_DIR, "uksc.db")
        newFile = os.path.join(UKSC_CACHE_DIR, "uksc_new.db")
        try:
            if(os.path.exists(newFile)):
                os.remove(newFile)
            os.system("echo \".dump\" | sqlite3 "+destFile+" | sqlite3 "+newFile)
            os.system("mv "+newFile +" " + destFile)
            os.system("chmod 666 " + destFile)
        except:
            pass
        self.cursor.close()
        self.connect.close()
        try:
            self.cursor1.close()
            self.connect1.close()
        except:
            pass
        if not os.path.exists(destFile):
            if not os.path.exists(srcFile):
                Globals.CONNECT_DB = False
                if (Globals.DEBUG_SWITCH):
                    Globals.LOG_ERROR.logger.error(_("error with db file"))
                return
            os.system("cp "+srcFile +" " + destFile)
        try:
            self.connect = sqlite3.connect(destFile, timeout=10.0, check_same_thread=False)
            self.cursor = self.connect.cursor()
        except Exception as e:
            Globals.CONNECT_DB = False
            Globals.LOG_ERROR.logger.error(_("Database link failed:  ")+str(e))
        try:
            self.connect1 = sqlite3.connect(srcFile, timeout=10.0, check_same_thread=False)
            self.cursor1 = self.connect1.cursor()
        except Exception as e:
            Globals.CONNECT_DB = False
            Globals.LOG_ERROR.logger.error(_("Database link failed:  ")+str(e))
        res = None
        sql = "select value from dict where key='dbversion'"
        try:
            self.cursor.execute(sql)
            res = self.cursor.fetchall()
        except Exception as e:
            Globals.CONNECT_DB = False
            Globals.LOG_ERROR.logger.error(_("Database link failed:  ") + str(e))
        install_db_version = ""
        if(res != None):
            install_db_version = res[0][0]
        res = None
        try:
            lock.acquire(True)
            self.cursor1.execute(sql)
            res = self.cursor1.fetchall()
        except Exception as e:
            Globals.CONNECT_DB = False
            Globals.LOG_ERROR.logger.error(_("Database link failed:  ") + str(e))
        finally:
            lock.release()
        db_version = "source_version"
        if(res != None):
            db_version = res[0][0]
        if(db_version != install_db_version):
            self.cursor.close()
            self.connect.close()
            try:
                self.cursor1.close()
                self.connect1.close()
            except:
                pass
            if os.path.exists(destFile):
                if os.path.exists(srcFile):
                    os.system("cp "+srcFile +" " + destFile)
                    try:
                        self.connect = sqlite3.connect(destFile, timeout=30.0, check_same_thread=False)
                        self.cursor = self.connect.cursor()
                    except Exception as e:
                        Globals.CONNECT_DB = False
                        Globals.LOG_ERROR.logger.error(_("Database link failed:  ") + str(e))
                else:
                    Globals.CONNECT_DB = False
            else:
                Globals.CONNECT_DB = False




    #
    # 函数：获取数据库分类
    #
    def query_categories(self):
        try:
            self.cursor.execute("select * from category order by priority asc")
            res = self.cursor.fetchall()
        except:
            self.connect.close()
            srcFile = os.path.join(UBUNTUKYLIN_DATA_PATH, "uksc.db")
            destFile = os.path.join(UKSC_CACHE_DIR, "uksc.db")
            os.system("cp " + srcFile + " " + destFile)
            self.connect = sqlite3.connect(destFile, timeout=30.0, check_same_thread=False)
            self.cursor = self.connect.cursor()
            try:
                self.cursor.execute("select * from category order by priority asc")
                res = self.cursor.fetchall()
            except:
                return []
        return res

    #
    # 函数：获取包信息
    #
    def query_package_list(self):
        get_sourse_list = []
        sourse_list = os.popen("cat /etc/apt/sources.list").readlines()
        for i in sourse_list:
            if(i.startswith("#")):
                continue
            if (len(i.split(" ")) > 2):
                for j in range(len(i.split(" "))):
                    if (i.split(" ")[j].startswith("https")):
                        # get_sourse_list.append(j.replace("https", "", 1))
                        source = i.split(" ")[j] + " " + i.split(" ")[j+1]
                        get_sourse_list.append(source)
                    elif (i.split(" ")[j].startswith("http")):
                        # get_sourse_list.append(j.replace("http", "", 1))
                        source = i.split(" ")[j] + " " + i.split(" ")[j + 1]
                        get_sourse_list.append(source)
        source_correspondence_list = self.source_list()
        get_source_correspondence_list = []
        for j in get_sourse_list:
            in_list = False
            for i in source_correspondence_list:
                if(j == i[0]):
                    if(i[0] not in get_source_correspondence_list and Globals.ARCH.lower() in i[1].lower()):
                        # if (j.startswith("https")):
                        #     get_list.append(j.replace("https", "", 1),i[1])
                        # elif (j.startswith("http")):
                        #     i[0] = j.replace("http", "", 1)
                        get_source_correspondence_list.append(i[0])
                        get_source_correspondence_list.append(i[1])
                        in_list = True
                elif(i[1] in j):
                    if (i[1] not in get_source_correspondence_list):
                        get_source_correspondence_list.append(i[0])
                        get_source_correspondence_list.append(i[1])
                        in_list = True
            if(in_list == False):
                get_source_correspondence_list.append(j)
        like_sqlite = ""
        for i in get_source_correspondence_list:
            if(like_sqlite == "" and i != ""):
                if(len(i.split(" ")) >= 2):
                    if(i.split(" ")[1] != "default"):
                        like_sqlite = "(downloadLink like '%"+i.split(" ")[0]+"%' and downloadLink like '%" +i.split(" ")[1]+"%') "
                    else:
                        like_sqlite = "(downloadLink like '%" + i.split(" ")[0] + "%' ) "
                else:
                    like_sqlite = "(downloadLink like '%" + i + "%' ) "
            else:
                if (len(i.split(" ")) >= 2):
                    if (i.split(" ")[1] != "default"):
                        like_sqlite = like_sqlite + "or (downloadLink like '%" + i.split(" ")[0] + "%' and downloadLink like '%" + i.split(" ")[1] + "%') "
                    else:
                        like_sqlite = like_sqlite + "or (downloadLink like '%" + i.split(" ")[0] + "%' ) "
                else:
                    like_sqlite = like_sqlite + "or (downloadLink like '%" + i + "%' ) "
                # like_sqlite = like_sqlite + "or downloadLink like '%" + get_source_correspondence_list[i][1] + "%' "
        res = []
        try:
            lock.acquire(True)
            sql = "select * from package_list where (cpuArch=? or cpuArch=?) and (" + like_sqlite + ")"
            self.cursor.execute(sql,(Globals.ARCH.lower(),"all",))
            res = self.cursor.fetchall()
        except:
            pass
        finally:
            lock.release()
        get_packagedict_py_id = {}
        for i in res:
            if(str(i[0]) not in get_packagedict_py_id.keys()):
                get_packagedict_py_id[str(i[0])] = []
                get_packagedict_py_id[str(i[0])].append(i)
            else:
                get_packagedict_py_id[str(i[0])].append(i)
        return get_packagedict_py_id,get_source_correspondence_list

    #
    # 函数：获取仓库源对应关系函数
    #
    def source_list(self):
        sql = "select * from source_list"
        res = None
        try:
            lock.acquire(True)
            self.cursor.execute(sql)
            res = self.cursor.fetchall()
        except Exception as e:
            pass
        finally:
            lock.release()
        return res


    #
    #函数：根据下载数量获取驱动信息
    #
    def get_drive_info_by_download(self):
        res = []
        res_list = []
        sql = "select b.id,b.app_name,b.display_name,b.display_name_cn,b.categories,b.summary,b.description,b.rating_avg,b.rating_total,b.review_total,b.download_total,b.icon,b.cover_image,b.preview_image,b.arch,b.system_name,b.depends,b.cpu_model,b.no_cpu_model,b.certSubjectOu,b.softStyle,a.manufacturers,a.package_version,a.package_size,b.arch,a.system_version,popularity,website from drive_info a left join application b on a.id=b.id order by b.download_total desc"
        try:
            self.cursor.execute(sql)
            res = self.cursor.fetchall()
        except:
            pass
        if(res != [] and res != None):
            for i in res:
                if(i[0] == None):
                    pass
                else:
                    res_list.append(i)
        return res_list

    #
    # 函数：根据应用评分获取驱动信息
    #
    def get_drive_info_by_rating(self):
        res = []
        res_list = []
        sql = "select b.id,b.app_name,b.display_name,b.display_name_cn,b.categories,b.summary,b.description,b.rating_avg,b.rating_total,b.review_total,b.download_total,b.icon,b.cover_image,b.preview_image,b.arch,b.system_name,b.depends,b.cpu_model,b.no_cpu_model,b.certSubjectOu,b.softStyle,a.manufacturers,a.package_version,a.package_size,b.arch,a.system_version,popularity,website from drive_info a left join application b on a.id=b.id order by b.rating_avg desc,b.download_total desc"
        try:
            self.cursor.execute(sql)
            res = self.cursor.fetchall()
        except:
            pass
        if (res != [] and res != None):
            for i in res:
                if (i[0] == None):
                    pass
                else:
                    res_list.append(i)
        return res_list

    #
    #函数：保存广告的曝光次数
    #
    def save_db_exposure_rate(self):
        res = []
        sql = "select * from ad_exposure_rate"
        try:
            lock.acquire(True)
            self.cursor.execute(sql)
            res = self.cursor.fetchall()
        except Exception as e:
            if (str(e) == "database is locked"):
                self.reconnect_database()
                try:
                    lock.acquire(True)
                    self.cursor.execute(sql)
                    res = self.cursor.fetchall()
                except Exception as e:
                    pass
        finally:
            lock.release()
        if(res == None or res == []):
            for i in list(Globals.AD_EXPOSURE_RATE.keys()):
                sql = "insert into ad_exposure_rate values(?,?,?,?,?)"
                try:
                    lock.acquire(True)
                    self.cursor.execute(sql,(str(Globals.AD_EXPOSURE_RATE[i]["appid"]), str(Globals.AD_EXPOSURE_RATE[i]["category_id"]), Globals.AD_EXPOSURE_RATE[i]["show_count"], Globals.AD_EXPOSURE_RATE[i]["click_count"], Globals.AD_EXPOSURE_RATE[i]["down_count"]))
                    self.connect.commit()
                except Exception as e:
                    if (str(e) == "database is locked"):
                        self.reconnect_database()
                        try:
                            self.cursor.execute(sql, (str(Globals.AD_EXPOSURE_RATE[i]["appid"]), str(Globals.AD_EXPOSURE_RATE[i]["category_id"]),Globals.AD_EXPOSURE_RATE[i]["show_count"], Globals.AD_EXPOSURE_RATE[i]["click_count"],
                            Globals.AD_EXPOSURE_RATE[i]["down_count"]))
                            self.connect.commit()
                        except:
                            pass
                    pass
                finally:
                    lock.release()
        else:
            for ad in list(Globals.AD_EXPOSURE_RATE.keys()):
                in_res = False
                for i in res:
                    if (str(i[0]) + "_" + str(i[1]) == ad):
                        in_res = True
                        sql = "update ad_exposure_rate set show_exposure_rate=?,clicked_exposure_rate=?,down_exposure_rate=? where appid=? and category_id=?"
                        try:
                            lock.acquire(True)
                            self.cursor.execute(sql,(Globals.AD_EXPOSURE_RATE[ad]["show_count"]+i[2],
                                                 Globals.AD_EXPOSURE_RATE[ad]["click_count"]+i[3],
                                                 Globals.AD_EXPOSURE_RATE[ad]["down_count"]+i[4],str(Globals.AD_EXPOSURE_RATE[str(i[0])+"_"+str(i[1])]["appid"]),str(Globals.AD_EXPOSURE_RATE[str(i[0])+"_"+str(i[1])]["category_id"])))
                            self.connect.commit()
                        except Exception as e:
                            if (str(e) == "database is locked"):
                                self.reconnect_database()
                                try:
                                    self.cursor.execute(sql, (Globals.AD_EXPOSURE_RATE[ad]["show_count"] + i[2],
                                                              Globals.AD_EXPOSURE_RATE[ad]["click_count"] + i[3],
                                                              Globals.AD_EXPOSURE_RATE[ad]["down_count"] + i[4], str(
                                        Globals.AD_EXPOSURE_RATE[str(i[0]) + "_" + str(i[1])]["appid"]), str(
                                        Globals.AD_EXPOSURE_RATE[str(i[0]) + "_" + str(i[1])]["category_id"])))
                                    self.connect.commit()
                                except:
                                    pass
                        finally:
                            lock.release()
                        break
                if(in_res == False):
                    sql = "insert into ad_exposure_rate values(?,?,?,?,?)"
                    try:
                        lock.acquire(True)
                        self.cursor.execute(sql, (
                        str(Globals.AD_EXPOSURE_RATE[ad]["appid"]), str(Globals.AD_EXPOSURE_RATE[ad]["category_id"]),
                        Globals.AD_EXPOSURE_RATE[ad]["show_count"], Globals.AD_EXPOSURE_RATE[ad]["click_count"],
                        Globals.AD_EXPOSURE_RATE[ad]["down_count"]))
                        self.connect.commit()
                    except Exception as e:
                        if (str(e) == "database is locked"):
                            self.reconnect_database()
                            try:
                                self.cursor.execute(sql, (
                                    str(Globals.AD_EXPOSURE_RATE[ad]["appid"]),
                                    str(Globals.AD_EXPOSURE_RATE[ad]["category_id"]),
                                    Globals.AD_EXPOSURE_RATE[ad]["show_count"],
                                    Globals.AD_EXPOSURE_RATE[ad]["click_count"],
                                    Globals.AD_EXPOSURE_RATE[ad]["down_count"]))
                                self.connect.commit()
                            except:
                                pass
                    finally:
                        lock.release()
                Globals.AD_EXPOSURE_RATE[ad]["show_count"] = 0

    #
    # 函数：保存新品上架的曝光次数
    #
    def save_new_arrivals_exposure_rate(self):
        res = []
        sql = "select * from new_arrivals_exposure_rate"
        try:
            lock.acquire(True)
            self.cursor.execute(sql)
            res = self.cursor.fetchall()
        except Exception as e:
            if (str(e) == "database is locked"):
                self.reconnect_database()
            try:
                self.cursor.execute(sql)
                res = self.cursor.fetchall()
            except:
                pass
        finally:
            lock.release()
        if (res == None or res == []):
            for i in list(Globals.NEW_ARRIVALS_EXPOSURE_RATE.keys()):
                sql = "insert into new_arrivals_exposure_rate values(?,?,?,?)"
                try:
                    lock.acquire(True)
                    self.cursor.execute(sql, (
                    str(Globals.NEW_ARRIVALS_EXPOSURE_RATE[i]["appid"]),
                    Globals.NEW_ARRIVALS_EXPOSURE_RATE[i]["show_count"], Globals.NEW_ARRIVALS_EXPOSURE_RATE[i]["click_count"],
                    Globals.NEW_ARRIVALS_EXPOSURE_RATE[i]["down_count"]))
                    self.connect.commit()
                except Exception as e:
                    if (str(e) == "database is locked"):
                        self.reconnect_database()
                    try:
                        self.cursor.execute(sql, (
                            str(Globals.NEW_ARRIVALS_EXPOSURE_RATE[i]["appid"]),
                            Globals.NEW_ARRIVALS_EXPOSURE_RATE[i]["show_count"],
                            Globals.NEW_ARRIVALS_EXPOSURE_RATE[i]["click_count"],
                            Globals.NEW_ARRIVALS_EXPOSURE_RATE[i]["down_count"]))
                        self.connect.commit()
                    except:
                        pass
                finally:
                    lock.release()
        else:
            for ad in list(Globals.NEW_ARRIVALS_EXPOSURE_RATE.keys()):
                in_res = False
                for i in res:
                    if (str(i[0]) == ad):
                        in_res = True
                        sql = "update new_arrivals_exposure_rate set show_exposure_rate=?,clicked_exposure_rate=?,down_exposure_rate=? where appid=?"
                        try:
                            lock.acquire(True)
                            self.cursor.execute(sql, (Globals.NEW_ARRIVALS_EXPOSURE_RATE[ad]["show_count"] + i[1],
                                                      Globals.NEW_ARRIVALS_EXPOSURE_RATE[ad]["click_count"] + i[2],
                                                      Globals.NEW_ARRIVALS_EXPOSURE_RATE[ad]["down_count"] + i[3], str(
                                Globals.NEW_ARRIVALS_EXPOSURE_RATE[ad]["appid"])))
                            self.connect.commit()
                        except Exception as e:
                            if (str(e) == "database is locked"):
                                self.reconnect_database()
                            try:
                                self.cursor.execute(sql, (Globals.NEW_ARRIVALS_EXPOSURE_RATE[ad]["show_count"] + i[1],
                                                          Globals.NEW_ARRIVALS_EXPOSURE_RATE[ad]["click_count"] + i[2],
                                                          Globals.NEW_ARRIVALS_EXPOSURE_RATE[ad]["down_count"] + i[3],
                                                          str(
                                                              Globals.NEW_ARRIVALS_EXPOSURE_RATE[ad]["appid"])))
                                self.connect.commit()
                            except:
                                pass
                        finally:
                            lock.release()
                        break
                if (in_res == False):
                    sql = "insert into new_arrivals_exposure_rate values(?,?,?,?)"
                    try:
                        lock.acquire(True)
                        self.cursor.execute(sql, (
                            str(Globals.NEW_ARRIVALS_EXPOSURE_RATE[ad]["appid"]),
                            Globals.NEW_ARRIVALS_EXPOSURE_RATE[ad]["show_count"], Globals.NEW_ARRIVALS_EXPOSURE_RATE[ad]["click_count"],
                            Globals.NEW_ARRIVALS_EXPOSURE_RATE[ad]["down_count"]))
                        self.connect.commit()
                    except Exception as e:
                        if (str(e) == "database is locked"):
                            self.reconnect_database()
                            try:
                                self.cursor.execute(sql, (
                                    str(Globals.NEW_ARRIVALS_EXPOSURE_RATE[ad]["appid"]),
                                    Globals.NEW_ARRIVALS_EXPOSURE_RATE[ad]["show_count"],
                                    Globals.NEW_ARRIVALS_EXPOSURE_RATE[ad]["click_count"],
                                    Globals.NEW_ARRIVALS_EXPOSURE_RATE[ad]["down_count"]))
                                self.connect.commit()
                            except:
                                pass
                    finally:
                        lock.release()
        Globals.NEW_ARRIVALS_EXPOSURE_RATE.clear()

    #
    #函数：保存热门应用曝光数据
    #
    def save_hot_exposure_rate(self):
        res = []
        sql = "select * from hot_exposure_rate"
        try:
            lock.acquire(True)
            self.cursor.execute(sql)
            res = self.cursor.fetchall()
        except Exception as e:
            if (str(e) == "database is locked"):
                self.reconnect_database()
                try:
                    self.cursor.execute(sql)
                    res = self.cursor.fetchall()
                except:
                    pass
        finally:
            lock.release()
        if (res == None or res == []):
            for i in list(Globals.HOT_EXPOSURE_RATE.keys()):
                sql = "insert into hot_exposure_rate values(?,?,?,?)"
                try:
                    lock.acquire(True)
                    self.cursor.execute(sql, (
                        str(Globals.HOT_EXPOSURE_RATE[i]["appid"]),
                        Globals.HOT_EXPOSURE_RATE[i]["show_count"],
                        Globals.HOT_EXPOSURE_RATE[i]["click_count"],
                        Globals.HOT_EXPOSURE_RATE[i]["down_count"]))
                    self.connect.commit()
                except Exception as e:
                    if (str(e) == "database is locked"):
                        self.reconnect_database()
                        try:
                            self.cursor.execute(sql, (
                                str(Globals.HOT_EXPOSURE_RATE[i]["appid"]),
                                Globals.HOT_EXPOSURE_RATE[i]["show_count"],
                                Globals.HOT_EXPOSURE_RATE[i]["click_count"],
                                Globals.HOT_EXPOSURE_RATE[i]["down_count"]))
                            self.connect.commit()
                        except:
                            pass
                finally:
                    lock.release()
        else:
            for ad in list(Globals.HOT_EXPOSURE_RATE.keys()):
                in_res = False
                for i in res:
                    if (str(i[0]) == ad):
                        in_res = True
                        sql = "update hot_exposure_rate set show_exposure_rate=?,clicked_exposure_rate=?,down_exposure_rate=? where appid=?"
                        try:
                            lock.acquire(True)
                            self.cursor.execute(sql, (Globals.HOT_EXPOSURE_RATE[ad]["show_count"] + i[1],
                                                      Globals.HOT_EXPOSURE_RATE[ad]["click_count"] + i[2],
                                                      Globals.HOT_EXPOSURE_RATE[ad]["down_count"] + i[3], str(
                                Globals.HOT_EXPOSURE_RATE[ad]["appid"])))
                            self.connect.commit()
                        except Exception as e:
                            if (str(e) == "database is locked"):
                                self.reconnect_database()
                            try:
                                self.cursor.execute(sql, (Globals.HOT_EXPOSURE_RATE[ad]["show_count"] + i[1],
                                                          Globals.HOT_EXPOSURE_RATE[ad]["click_count"] + i[2],
                                                          Globals.HOT_EXPOSURE_RATE[ad]["down_count"] + i[3], str(
                                    Globals.HOT_EXPOSURE_RATE[ad]["appid"])))
                                self.connect.commit()
                            except:
                                pass
                        finally:
                            lock.release()
                        break
                if (in_res == False):
                    sql = "insert into hot_exposure_rate values(?,?,?,?)"
                    try:
                        lock.acquire(True)
                        self.cursor.execute(sql, (
                            str(Globals.HOT_EXPOSURE_RATE[ad]["appid"]),
                            Globals.HOT_EXPOSURE_RATE[ad]["show_count"],
                            Globals.HOT_EXPOSURE_RATE[ad]["click_count"],
                            Globals.HOT_EXPOSURE_RATE[ad]["down_count"]))
                        self.connect.commit()
                    except Exception as e:
                        if (str(e) == "database is locked"):
                            self.reconnect_database()
                        try:
                            self.cursor.execute(sql, (str(Globals.HOT_EXPOSURE_RATE[ad]["appid"]),Globals.HOT_EXPOSURE_RATE[ad]["show_count"],Globals.HOT_EXPOSURE_RATE[ad]["click_count"],Globals.HOT_EXPOSURE_RATE[ad]["down_count"]))
                            self.connect.commit()
                        except:
                            pass
                    finally:
                        lock.release()
        Globals.HOT_EXPOSURE_RATE.clear()

    #
    #函数：获取广告
    #
    def query_ad_apps(self):
        res = []
        sql = "select name,path,type from advertisement"
        try:
            lock.acquire(True)
            self.cursor.execute(sql)
            res = self.cursor.fetchall()
        finally:
            lock.release()
        return res

    #
    # 函数：获取新品上架
    #
    def query_new_arrivals_apps(self):
        applist = []
        res = []
        sql = "select app_id,app_name,image_path from newAppList"
        try:
            lock.acquire(True)
            self.cursor.execute(sql)
            res = self.cursor.fetchall()
        finally:
            lock.release()
        return res

    #
    #函数：获取安卓应用推荐
    #
    def query_android_recommend_apps(self):
        applist = []
        res = []
        sql = "select appid,name,image_path from recommend_android_apps"
        try:
            lock.acquire(True)
            self.cursor.execute(sql)
            res = self.cursor.fetchall()
        finally:
            lock.release()
        return res

    #
    # 函数：获取热门应用列表
    #
    def query_hot_apps(self):
        applist = []
        res = []
        sql = "select app_id,name,image_path from hot_app_list"
        try:
            lock.acquire(True)
            self.cursor.execute(sql)
            res = self.cursor.fetchall()
        finally:
            lock.release()
        return res


    #
    #函数：获取搜索快捷列表信息
    #
    def query_search_association(self):
        applist = []
        res = []
        sql = "select * from search_association"
        try:
            lock.acquire(True)
            self.cursor.execute(sql)
            res = self.cursor.fetchall()
        finally:
            lock.release()
        return res

    #
    #函数：获取win替换
    #
    def query_win_replace_apps(self):
        app_list = []
        win_list = []
        res = []
        sql = "select * from xp order by priority asc"
        try:
            lock.acquire(True)
            self.cursor.execute(sql)
            res = self.cursor.fetchall()
        finally:
            lock.release()
        return res

    #
    #函数：获取下载排行榜
    #
    def query_download_ranking_apps(self):
        # print("start", datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
        app_list = []
        res = []
        sql = "select id,app_name,display_name,display_name_cn,categories,summary,description,rating_avg,rating_total,review_total,download_total,icon,cover_image,preview_image,arch,system_name,depends,cpu_model,no_cpu_model,certSubjectOu,softStyle,manufacturers,popularity,website from application order by download_total DESC"
        try:
            lock.acquire(True)
            self.cursor.execute(sql)
            res = self.cursor.fetchall()
        finally:
            lock.release()

        # print("end", datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
        sql = "select count from display_number where type=?"
        count_res = []
        try:
            lock.acquire(True)
            self.cursor.execute(sql,("download_rank_count",))
            count_res = self.cursor.fetchall()
        except Exception as e:
            pass
        finally:
            lock.release()
        if(count_res != [] and count_res != None):
            if(count_res[0][0] != None):
                Globals.DISPLAY_DOWNLOAD_RANK_NUMBER = count_res[0][0]
        #        print "query_application:",pkgname,len(res),res
        # print("end", datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
        if len(res) == 0:
            return []
        else:
            return res

    #
    #函数：通过软件名获取软件信息
    #
    def query_app_by_appname(self,appname):
        app_list = []
        sql = "select * from application where app_name like '%" + (str(appname)) + "%' or display_name like '%" + (str(appname)) + "%' or display_name_cn like '%" + (str(appname)) + "%'"
        try:
            lock.acquire(True)
            self.cursor.execute(sql)
            res = self.cursor.fetchall()
        finally:
            lock.release()
        return res

    #
    #函数：从数据库获取安卓应用列表
    #
    def get_kydroid_app_list_from_db(self):
        try:
            lock.acquire(True)
            self.cursor.execute("select * from kydroid_app_list")
            # self.cursor.execute("select appname from cannotremove where system_version=?", ("V10",))
            res = self.cursor.fetchall()
        except:
            res = None
        finally:
            lock.release()
        return res

    #
    #函数：获取不能卸载软件
    #
    def get_cannot_remove_list(self):
        try:
            lock.acquire(True)
            self.cursor.execute("select appname from cannotremove where status=? and (system_version=? or system_version=?)",(1,Globals.SYSTEM_VERSION.upper(),Globals.SYSTEM_VERSION.lower(),))
            # self.cursor.execute("select appname from cannotremove where system_version=?", ("V10",))
            res = self.cursor.fetchall()
        except:
            res = None
        finally:
            lock.release()
        return res

    #
    # 函数：获取特殊desktop列表
    #
    def get_desktop_list(self):
        try:
            lock.acquire(True)
            self.cursor.execute("select pkgname,desktop from desktop where (system_version=? or system_version=?) and arch=?",(Globals.SYSTEM_VERSION.upper(),Globals.SYSTEM_VERSION.lower(),Globals.ARCH,))
            # self.cursor.execute("select pkgname,desktop from desktop where system_version=? and arch=?",("V10 SP1", Globals.ARCH,))
            res = self.cursor.fetchall()
        except:
            res = None
        finally:
            lock.release()
        return res

    #
    #函数：从本地获取安卓源地址
    #
    def get_kydroid_source_list(self,system_version,kydroid_version,arch):
        try:
            lock.acquire(True)
            self.cursor.execute("select kydroid_source from kydroid_list where system_version=? and kydroid_version=? and arch=?",(system_version,kydroid_version,arch,))
            # self.cursor.execute("select kydroid_source from kydroid_list where system_version=? and kydroid_version=? and arch=?",("V10 SP1","kydroid3","arm64",))
            res = self.cursor.fetchall()
        except:
            res = None
        finally:
            lock.release()
        return res


    #
    # 函数：获取分类
    #
    def get_categories(self):
        try:
            lock.acquire(True)
            self.cursor.execute("select * from category order by priority asc")
            res = self.cursor.fetchall()
        except:
            res = []
        finally:
            lock.release()
        #        print "query_categories:",len(res),res
        return res

    #
    #函数：获取驱动分类
    #
    def get_drive_categories(self):
        try:
            lock.acquire(True)
            self.cursor.execute("select visible,id,name,display_name,priority from driving_classification order by priority asc")
            res = self.cursor.fetchall()
        except:
            res = []
        finally:
            lock.release()
        #        print "query_categories:",len(res),res
        return res

    #
    #函数：通过分类id获取分类
    #
    def get_categories_by_id(self,id):
        try:
            lock.acquire(True)
            self.cursor.execute("select name,display_name from category where id='%d'" % (int(id)))
            res = self.cursor.fetchall()
        finally:
            lock.release()
        #        print "query_categories:",len(res),res
        return res

    #
    #函数：获取application列表中所有软件的所有信息
    #
    def query_all_applications(self):
        try:
            lock.acquire(True)
            self.cursor.execute(QUERY_ALL_IMFORMATION)
            res = self.cursor.fetchall()
        finally:
            lock.release()
        if len(res) == 0:
            return []
        else:
            return res

    #
    # 函数：获取application列表中所有软件的所有信息
    #
    def query_get_recommend_applications(self, category_id):
        try:
            lock.acquire(True)
            sql = "select id,app_name,display_name,display_name_cn,categories,summary,description,rating_avg,rating_total,review_total,download_total,icon,cover_image,preview_image,arch,system_name,depends,cpu_model,no_cpu_model,certSubjectOu,softStyle,manufacturers,popularity,website from application where categories like \"%"+str(category_id)+"%\" order by download_total desc"
            self.cursor.execute(sql)
            res = self.cursor.fetchall()
        finally:
            lock.release()
        if len(res) == 0:
            return []
        else:
            return res

    #
    #函数：获取application列表中所有软件的所有信息
    #
    def query_one_applications(self,appname):
        try:
            lock.acquire(True)
            sql = "select id,app_name,display_name,display_name_cn,categories,summary,description,rating_avg,rating_total,review_total,download_total,icon,cover_image,preview_image,arch,system_name,depends,cpu_model,no_cpu_model,certSubjectOu,softStyle,manufacturers,popularity,website from application where app_name=?"
            self.cursor.execute(sql,(appname,))
            res = self.cursor.fetchall()
        finally:
            lock.release()
        if len(res) == 0:
            return []
        else:
            return res

    #
    # 函数：获取application表所有软件名和中文名
    #
    # return as (display_name, app_name)
    def query_applications(self):
        try:
            lock.acquire(True)
            self.cursor.execute(QUERY_APPS)
            res = self.cursor.fetchall()
        finally:
            lock.release()
        if len(res) == 0:
            return []
        else:
            return res

    #
    #函数：根据评分和下载进行排序
    #
    def query_applications_by_ratings(self):
        try:
            lock.acquire(True)
            self.cursor.execute(QUERY_ALL_IMFORMATION_BY_RATING)
            res = self.cursor.fetchall()
        finally:
            lock.release()
        if len(res) == 0:
            return []
        else:
            return res

    #
    #函数：通过软件包名获取数据库中软件信息
    #
    def query_appmassage_by_appname(self,appname):
        app_list = []
        sql = "select * from application where app_name='%s'"
        try:
            lock.acquire(True)
            self.cursor.execute(sql % appname)
            res = self.cursor.fetchall()
        finally:
            lock.release()
        return res


    #
    # 函数：获取单个软件application表内容
    #
    #return as (display_name, summary, description, rating_average,rating_total,review_total,download_total)
    def query_application(self,pkgname):
        try:
            lock.acquire(True)
            self.cursor.execute(QUERY_APP % (pkgname))
            res = self.cursor.fetchall()
        finally:
            lock.release()

#        print "query_application:",pkgname,len(res),res
        if len(res)==0:
            return []
        else:
            return res[0]

    # 函数：获取软件的评论页数
    #
    def get_pagecount_by_pkgname(self, package_name):
        try:
            lock.acquire(True)
            self.cursor.execute("select review_total from application where app_name=?", (package_name,))
            res = self.cursor.fetchall()
        finally:
            lock.release()
        for item in res:
            review_total = item[0]
            if(review_total == None):
                review_total = 0
            return review_total / 10 + 1

    #
    # 函数：获取软件某页的所有评论
    #
    def get_review_by_pkgname(self, package_name, page):
        return


    #
    #函数：更新数据库
    #
    def update_app_msg(self,id,reslist):
        # newestImage = reslist["newestImage"]
        appId = reslist["appId"]
        averageScore = reslist["averageScore"]
        commentCount = reslist["commentCount"]
        scoreCount = reslist["scoreCount"]
        installCount = reslist["installCount"]
        previewImage = reslist["previewImage"]
        sql = "update application set rating_avg=?,rating_total=?,review_total=?,download_total=?,preview_image=? where id=?"
        try:
            lock.acquire(True)
            self.cursor.execute(sql, (averageScore,scoreCount,commentCount,installCount,previewImage, id))
            self.connect.commit()
        except Exception as e:
            Globals.LOG_ERROR.logger.error("update app msg: %s" % str(e))
        finally:
            lock.release()


    #-------------kydroid APK ----------------
    #
    # 函数：获取安卓兼容软件列表
    #
    def query_apk_applications(self):
        res = []
        rating_res = []
        try:
            lock.acquire(True)
            sql = "select id,app_name,display_name,display_name_cn,categories,summary,description,rating_avg,rating_total,review_total,download_total,icon,cover_image,preview_image,arch,system_name,depends,cpu_model,no_cpu_model,certSubjectOu,softStyle,popularity,website from application where softStyle=3 order by download_total desc"
            self.cursor.execute(sql)
            res = self.cursor.fetchall()
        finally:
            lock.release()
#        print "query_application:",pkgname,len(res),res

        sql_rating = "select id,app_name,display_name,display_name_cn,categories,summary,description,rating_avg,rating_total,review_total,download_total,icon,cover_image,preview_image,arch,system_name,depends,cpu_model,no_cpu_model ,certSubjectOu,softStyle,popularity,website from application where softStyle=3 order by rating_avg desc,download_total desc"
        try:
            lock.acquire(True)
            self.cursor.execute(sql_rating)
            rating_res = self.cursor.fetchall()
        finally:
            lock.release()
        if len(res)==0:
            return [],[]
        else:
            return res,rating_res


    #
    #保存用户名、密码已经是否勾选保存密码
    #
    def save_password_at_mysql(self):
        SETPASSWORD=False
        try:
            lock.acquire(True)
            self.cursor.execute("select username,save_password from password")
            res = self.cursor.fetchall()
        finally:
            lock.release()
        if res == []:
            self.insert_username_to_mysql()
            return
        else:
            asswd = "kylin123"
            iv = os.urandom(32)
            aescryptor = Aescrypt("kylin123", AES.MODE_ECB, iv)  # ECB模式
            for i in res:
                de = base64.b64decode(i[0])
                name = aescryptor.aesdecrypt(de)
                get_name = name.decode()
                if get_name == Globals.USER:

                    username = get_name

                    SETPASSWORD=True
                    # en_text = aescryptor.aesencrypt(Globals.PASSWORD.encode())
                    # base64EncodedStr = base64.b64encode(en_text)
                    # pass_word = bytes.decode(base64EncodedStr)
                    # # pass_word = base64.b64encode(Globals.PASSWORD.encode())
                    pass_word = aescryptor.aesencrypt(Globals.PASSWORD)
                    base64_pass_word = base64.b64encode(pass_word).decode()
                    Globals.PASSWORD = base64_pass_word

                    save_password = aescryptor.aesencrypt(Globals.CHEK_SAVE_PASSWORD)
                    base64_save_password = base64.b64encode(save_password).decode()
                    de = base64.b64decode(i[1])
                    pwd = aescryptor.aesdecrypt(de)
                    pwd = pwd.decode()
                    if(pwd != "1"):
                        if Globals.CHEK_SAVE_PASSWORD == "1":
                            try:
                                lock.acquire(True)
                                # self.cursor.execute("DELETE FROM password WHERE username = '%s'" % i[0])
                                self.cursor.execute("update password set save_password=?,user_password=? where username=?",(base64_save_password,base64_pass_word,i[0]))
                                self.connect.commit()

                            finally:
                                lock.release()
                            # self.insert_username_to_mysql()
                        else:
                            # password = ""
                            # pass_word = base64.b64encode(password.encode())
                            try:

                                lock.acquire(True)
                                # self.cursor.execute("DELETE FROM password WHERE username = '%s'" % i[0])
                                self.cursor.execute("update password set save_password=?,user_password=? where username=?",(base64_save_password, "",i[0]))
                                self.connect.commit()

                            finally:
                                lock.release()
                            # self.insert_username_to_mysql()
                    else:
                        if( Globals.CHEK_SAVE_PASSWORD == "1"):
                            try:
                                lock.acquire(True)
                                # self.cursor.execute("DELETE FROM password WHERE username = '%s'" % i[0])
                                self.cursor.execute("update password set save_password=?,user_password=? where username=?",(base64_save_password,base64_pass_word,i[0]))
                                self.connect.commit()

                            finally:
                                lock.release()
                            # self.insert_username_to_mysql()
                        else:
                            Globals.PASSWORD = ""
                            try:
                                lock.acquire(True)
                                # self.cursor.execute("DELETE FROM password WHERE username = '%s'" % i[0])
                                self.cursor.execute("update password set save_password=?,user_password=? where username=?",(base64_save_password, "",i[0]))
                                self.connect.commit()

                            finally:
                                lock.release()
                            # self.insert_username_to_mysql()
                    return
        if SETPASSWORD == False:
            self.insert_username_to_mysql()
        Globals.USER = ""
        Globals.PASSWORD = ""

    #
    #插入用户名密码
    #
    def insert_username_to_mysql(self):
        passwd = "kylin123"
        iv = os.urandom(32)
        aescryptor = Aescrypt(passwd, AES.MODE_ECB, iv)  # ECB模式
        user_name = aescryptor.aesencrypt(Globals.USER)
        base64_user_name = base64.b64encode(user_name).decode()
        Globals.USER = base64_user_name

        pass_word = aescryptor.aesencrypt(Globals.PASSWORD)
        base64_pass_word = base64.b64encode(pass_word).decode()
        Globals.PASSWORD = base64_pass_word

        save_password = aescryptor.aesencrypt(Globals.CHEK_SAVE_PASSWORD)
        base64_save_password = base64.b64encode(save_password).decode()

        # user_name = base64.b64encode(Globals.USER.encode())
        # pass_word = base64.b64encode(Globals.PASSWORD.encode())
        # save_password = base64.b64encode(Globals.CHEK_SAVE_PASSWORD.encode())
        #
        # user_name = bytes.decode(user_name)
        # pass_word = bytes.decode(pass_word)
        # save_password = bytes.decode(save_password)
        # destname=base64.b64decode(user_name).decode()#解密
        if Globals.CHEK_SAVE_PASSWORD == "1":
            try:

                lock.acquire(True)
                self.cursor.execute("insert into password values(?,?,?,?)", ("0", base64_save_password, base64_user_name,base64_pass_word ))
                self.connect.commit()
            finally:
                lock.release()
        else:
            password=""
            pass_word = aescryptor.aesencrypt(password.encode())
            base64_pass_word = base64.b64encode(pass_word)
            # pass_word = base64.b64encode(password.encode())
            #
            # pass_word = bytes.decode(pass_word)
            try:
                lock.acquire(True)
                self.cursor.execute("insert into password values(?,?,?,?)", ("0", base64_save_password, base64_user_name,base64_pass_word ))
                self.connect.commit()
            except:
                pass
            finally:
                lock.release()

    #
    #数据库查询用户密码
    #
    def select_username_password(self):
        res = []
        try:
            lock.acquire(True)
            self.cursor.execute("select save_password,username,user_password from password")
            res = self.cursor.fetchall()
        finally:
            lock.release()
        if res == []:
            return []
        else:
            res_list = []
            for i in res:
                if(i[1] == ""):
                    pass
                else:
                    res_list.append(i)
            return res_list

    #密码解析使用destname=base64.b64decode(user_name).decode()#解密

