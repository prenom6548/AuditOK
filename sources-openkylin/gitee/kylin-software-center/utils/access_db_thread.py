#!/usr/bin/python3
# -*- coding: utf-8 -*-

import sys
# reload(sys)
# sys.setdefaultencoding('utf8')
from PyQt5.QtCore import QThread
from PyQt5.QtWidgets import QWidget
from PyQt5.QtCore import pyqtSignal
import locale
from service.appmanager import *
import apt
import time
import json
import dbus
import configparser
from models.enums import CONF_PATH,ARCH,GET_KYDROID_VERSION,SYSTEM_VERSION,PKG_NAME,CPU,GPU,KYDROID_DOWNLOAD_PATH,KYDROID_INSTALLED_FILENAME,UBUNTUKYLIN_USER_TOKEN_PATH
from models.globals import Globals
from backend.run_app.run_dbus_service import Run_Dbus_Service
import threading
import datetime
from utils.log_manager import LogManager
from utils.machine import *
from gi.repository import GObject
from dbus.mainloop.glib import DBusGMainLoop
mainloop = DBusGMainLoop(set_as_default=True)
import gettext
gettext.bindtextdomain("kylin-software-center", "/usr/share/locale")
gettext.textdomain("kylin-software-center")
_ = gettext.gettext

# 配置文件组名
CONF_GROUP = "update-data"

class CheckconfThread(QThread):
    def __init__(self, mw,parent=None):
        QThread.__init__(self)
        self.work_list = []
        self.mw = mw

    def init_system_dbus(self):
        dbus_service = "com.kylin.software.properties"
        dbus_path = "/com/kylin/software/properties"
        dbus_interface = "com.kylin.software.properties.interface"
        funcname = "updateSourcePackages"
        checkupdate = "sourceIsNeedUpdate"
        try:
            bus = dbus.SystemBus(mainloop)
        except Exception as e:
            Globals.LOG_ERROR.logger.error("could not initiate com.kylin.software.properties dbus")
            Globals.LOG_ERROR.logger.error("com.kylin.software.properties: dbus exception: %s" % str(e))
            return False

        try:
            obj = bus.get_object(dbus_service, dbus_path)
            source_iface = dbus.Interface(obj, dbus_interface)
            if source_iface is None:
                return False
            source_iface.Enroll(0, 0, -1, checkupdate, timeout=30,
                                reply_handler=self.handle_check_need_update,
                                error_handler=self.handle_timeout_error)
            # fun0 = None
            # func0 = getattr(source_iface,checkupdate)
            # if(func0 is None):
            #     return False
            # need_update = False
            # try:
            #     need_update = func0()
            # except Exception as e:
            #     Globals.LOG_ERROR.logger.error("check need update source list exception:%s" % str(e))
            #     need_update = False
            # if(need_update):
            #     func = getattr(source_iface, funcname)
            #     if func is None:
            #         return False
            #     dbus_res = -1
            #     try:
            #         dbus_res = func("kylin-software-center")
            #     except Exception as e:
            #         Globals.LOG_ERROR.logger.error("update source list exception:%s" % str(e))
            #         dbus_res = -1
            #     # update conf file
            #     if dbus_res == 100:
            #         self.update_remoter_daemon_conf("update_source_list")
            #         try:
            #             self.mw.detailwidget.ca.open()
            #         except:
            #             pass

        except Exception as e:
            Globals.LOG_ERROR.logger.error("com.kylin.software.properties: dbus exception:%s" % str(e))
            return False
            # print("remoter daemon: update source list success")


    #
    #函数：处理返回需要update的情况
    #
    def handle_check_need_update(self,need_update):
        # 更新软件源缓存
        dbus_service = "com.kylin.software.properties"
        dbus_path = "/com/kylin/software/properties"
        dbus_interface = "com.kylin.software.properties.interface"
        checkupdate = "sourceIsNeedUpdate"
        dbus_res = False
        if (need_update):

            try:
                bus = dbus.SystemBus()
            except Exception as e:
                Globals.LOG_ERROR.logger.error("remoter daemon: dbus exception: %s" % str(e))
                return False

            try:
                obj = bus.get_object(dbus_service, dbus_path)
                source_iface = dbus.Interface(obj, dbus_interface)
                source_iface.Enroll(0, 0, -1, checkupdate, timeout=120,
                                    reply_handler=self.handle_update_source_list,
                                    error_handler=self.handle_timeout_error1)
            except Exception as e:
                Globals.LOG_ERROR.logger.error("remoter daemon: dbus exception: %s" % str(e))

        else:
            return False

    #
    #函数：进行源更新
    #
    def handle_update_source_list(self,ret):
        if(type(ret) == int):
            if(ret == 100):
                try:
                    self.apt_cache = apt.Cache()
                    self.apt_cache.open()
                except:
                    self.apt_cache = None
                    return False

    def handle_timeout_error(self,error):
        Globals.LOG_INFO.logger.error("remoter daemon: check_need_update error",str(error))
        self.handle_update_source_list(100)

    def handle_timeout_error1(self,error):
        Globals.LOG_INFO.logger.error("remoter daemon: update_source_list error",str(error))



    # 函数: 数据更新配置文件
    # 参数: 更新数据项
    # 返回:
    def update_remoter_daemon_conf(self, action):
        Globals.LOG_INFO.logger.info("remoter daemon: update remoter daemon conf")

        try:
            try:
                remoter_daemon_conf = configparser.ConfigParser()
                remoter_daemon_conf.read(CONF_PATH)
                remoter_daemon_conf.get(CONF_GROUP, action)
            except Exception as e:
                if os.path.exists(CONF_PATH):
                    os.remove(CONF_PATH)
                self.init_uksc_cache_conf()

            if not os.path.exists(CONF_PATH):
                return

            remoter_daemon_conf = configparser.ConfigParser()
            remoter_daemon_conf.read(CONF_PATH)
            remoter_daemon_conf.set(CONF_GROUP, action, "True")

            with open(CONF_PATH, "w+") as f:
                remoter_daemon_conf.write(f)

        except Exception as e:
            self.ERROR_CODE = "daemon_conf_error"
            if Globals.DEBUG_SWITCH:
                Globals.LOG_ERROR.logger.error("remoter daemon: update remoter daemon conf exception: %s" % str(e))

    def run(self):
        # try:
        #     self.init_system_dbus()
        # except:
        #     pass
        while(1):
            if (not os.path.exists(CONF_PATH)):
                os.system("cp /usr/share/kylin-software-center/kylin-software-center.conf " + UKSC_CACHE_DIR)
            try:
                cf = configparser.ConfigParser()
                cf.read(CONF_PATH)
                newer_list = cf.get("update-data", "update_app_list")
                update_source = cf.get("update-data", "update_source_list")
                if(newer_list == "True"):
                    cf.set("update-data", "update_app_list", "False")
                    with open(CONF_PATH, "w+") as f:
                        cf.write(f)
                    Globals.INIT_DATA_OVER = True
                if(update_source == "True"):
                    cf.set("update-data", "update_source_list", "False")
                    with open(CONF_PATH, "w+") as f:
                        cf.write(f)
                    Globals.INIT_DATA_OVER = True
                # Globals.INIT_DATA_OVER = True
                time.sleep(270)
                # time.sleep(10)
            except:
                os.system("cp /usr/share/kylin-software-center/kylin-software-center.conf " + UKSC_CACHE_DIR)


class Review(object):
    def __init__(self, msg , appname):
        self.package_name = appname
        self.id = None
        self.aid = msg["commentId"]
        self.user_id = msg["userId"]
        self.distroseries = None
        if ("account" in msg.keys()):
            self.account = msg["account"]
        else:
            self.account = None
        self.comment_id = msg["commentId"]
        if("likeCount" in msg.keys()):
            self.like_count = msg["likeCount"]
            self.unlike_count = msg["unlikeCount"]
            self.property = msg["property"]
        else:
            self.like_count = None
            self.unlike_count = None
            self.property = None
        self.content = msg["content"]
        if(msg["userId"] == 0):
            self.user_display = msg["oldUserName"]
        else:
            self.user_display = msg["userName"]
        try:
            time1 = msg["createTime"].split(".")[0]
            time_list = time1.split("T")
            time = time_list[0] +" "+ time_list[1]
        except:
            time = msg["createTime"]
        # self.date = msg["createTime"]
        self.date = time
        self.language = "zh_CN"
        self.version = ""
        if(msg["avatar"] != ''):
            self.user_profile = msg["avatar"]
        else:
            self.user_profile = ''
        self.up_total = 0
        self.down_total = 0
        self.user_rating=0


class SendUserInfo(object):
    def __init__(self, msg):
        self.send_user_info = {"account": "", "avatar": "", "dataSharing": "", "email": "", "hasPhone": "", "identity": "",
                          "name": "", "redirectUrl": "", "region": "", "status": 0, "subscribe": "", "telephone": "", "uid": ""}
        self.send_user_info["account"] = msg["kylinID"]
        self.send_user_info["avatar"] = msg["avatar"]
        self.send_user_info["dataSharing"] = msg["data_sharing"]
        self.send_user_info["email"] = msg["kylinID"]
        if("hasPhone" not in msg.keys()):
            self.send_user_info["hasPhone"] = None
        else:
            self.send_user_info["hasPhone"] = msg["hasPhone"]
        self.send_user_info["identity"] = msg["identity"]
        self.send_user_info["name"] = msg["userName"]
        self.send_user_info["redirectUrl"] = msg["redirect_url"]
        self.send_user_info["region"] = msg["region"]
        self.send_user_info["status"] = msg["status"]
        self.send_user_info["subscribe"] = msg["subscribe"]
        self.send_user_info["telephone"] = msg["phone"]
        self.send_user_info["uid"] = msg["uid"]



class Ask_Service_Thread(QThread):

    work_list = []
    read_token_over = pyqtSignal(dict)
    return_download_count = pyqtSignal(int, str,str)
    # zzz
    return_app_sign_state = pyqtSignal(str, bool)
    # zzz
    return_submit_review_state = pyqtSignal(dict)
    return_submit_review_state_fail = pyqtSignal(bool)
    #
    get_my_rating_over = pyqtSignal(int, int, str)
    return_rating = pyqtSignal(int, list, float, str)
    return_get_shot_screen_over = pyqtSignal(str,str)
    get_shot_screen_list = pyqtSignal(dict)
    submit_my_review_over = pyqtSignal(str)
    exit_login_finish = pyqtSignal()
    get_image_over = pyqtSignal(dict)
    get_image_over_fail = pyqtSignal(bool)
    get_search_appmsg_tip = pyqtSignal(list, bool)
    login_failure_signal = pyqtSignal(str)
    get_search_applist_over = pyqtSignal(list)
    login_finish_over = pyqtSignal(str)
    send_previe_num = pyqtSignal(str,str)
    get_previe_num_fail = pyqtSignal(str)
    send_dislike_finish = pyqtSignal(dict,QWidget,int)
    send_like_finish = pyqtSignal(dict, QWidget,int)
    User_operation = pyqtSignal(str)
    def __init__(self, mw,appmgr,parent=None):
        QThread.__init__(self)
        self.mutex = threading.RLock()
        self.work_list = []
        self.mw = mw
        self.appmgr = appmgr
        self.kylinid_iface = None
        self.run_iface = None
        self.init_run_bus()
        self.init_kylinid_dbus()
        self.read_token_over.connect(self.mw.slot_read_token_over)
        self.get_image_over.connect(self.mw.slot_get_image_over)
        self.get_image_over_fail.connect(self.mw.slot_get_image_over)
        self.return_download_count.connect(self.mw.slot_return_download_count)

        # zzz
        self.return_app_sign_state.connect(self.mw.slot_return_app_sign_state)
        # zzz
        self.return_submit_review_state.connect(self.mw.slot_return_submit_review)
        self.return_submit_review_state_fail.connect(self.mw.slot_return_submit_review)
        self.appmgr.network_error.connect(self.mw.slot_network_error)
        # self.appmgr.network_error.connect(lambda: self.mw.slot_network_error("./res/no_network.png", _("There is something wrong with the network. Please try again later.")))
        self.get_my_rating_over.connect(self.mw.slot_get_my_reating_over)
        self.return_rating.connect(self.mw.slot_return_rating)
        self.return_get_shot_screen_over.connect(self.mw.slot_get_screen_shot_over)
        self.appmgr.app_reviews_ready.connect(self.mw.slot_app_reviews_ready)
        self.submit_my_review_over.connect(self.mw.slot_submit_my_review_over)
        self.exit_login_finish.connect(self.mw.slot_exit_login_finish)
        self.login_failure_signal.connect(self.mw.slot_login_failure)
        self.get_shot_screen_list.connect(self.mw.slot_get_shot_screen_list)
        self.login_finish_over.connect(self.mw.slot_login_finish)
        self.get_previe_num_fail.connect(self.mw.slot_get_previe_num_fail)
        self.send_previe_num.connect(self.mw.slot_send_previe_num)
        self.send_dislike_finish.connect(self.mw.slot_send_dislike_finish)
        self.send_like_finish.connect(self.mw.slot_send_like_finish)
        self.User_operation.connect(self.mw.slot_User_operation)

    def init_kylinid_dbus(self):
        dbus_service = "org.kylinID.service"
        dbus_path = "/org/kylinID/path"
        dbus_interface = "org.kylinID.interface"
        dbus_res = False

        try:
            # mainloop = DBusGMainLoop(set_as_default=True)
            bus = dbus.SessionBus(mainloop)
        except Exception as e:
            return False

        try:
            obj = bus.get_object(dbus_service, dbus_path)
            self.kylinid_iface = dbus.Interface(obj, dbus_interface)
            # if self.source_iface is None:
            #     return False
        except Exception as e:
            pass
        if(self.kylinid_iface != None):
            GObject.timeout_add(2000, self.slotTimeout)

            pass

    def init_run_bus(self):
        self.run_iface = Run_Dbus_Service()


    def slotTimeout(self):
        if(self.kylinid_iface != None):
            self.kylinid_iface.connect_to_signal("userInfo", self.login_finish, dbus_interface="org.kylinID.interface")
            self.kylinid_iface.connect_to_signal("finishedLogout", self.loginout_finish, dbus_interface="org.kylinID.interface")


    def login_finish(self,res):
        res_list = json.loads(res)
        if(str(res_list["code"]) == "200"):
            if(res_list["data"] != None and res_list["data"] != [] and res_list["data"] != {}):
                if(res_list["data"]["entity"] != None and res_list["data"]["entity"] != [] and res_list["data"]["entity"] != {}):
                    try:
                        senduserinfo = SendUserInfo(res_list["data"]["entity"])
                        send_user_info = senduserinfo.send_user_info
                        send_res = self.appmgr.premoter.sendUserInfo(send_user_info)
                        if(send_res != False and send_res["code"] == 200):
                            self.appmgr.premoter.user_token = res_list['data']['entity']['token']
                            self.login_finish_over.emit(res)
                    except:
                        pass
        pass

    def loginout_finish(self,res):
        self.exit_login_finish.emit()


    #
    #函数：修改所有软件列表中的对应软件的信息
    #
    def update_app_detail(self,reslist):
        appId = reslist["appId"]
        averageScore = reslist["averageScore"]
        commentCount = reslist["commentCount"]
        scoreCount = reslist["scoreCount"]
        installCount = reslist["installCount"]
        previewImage = reslist["previewImage"]
        num = 0
        for i in Globals.APPLICATION_LIST:
            if str(i["appmsg"].app_id) == str(appId):
                Globals.APPLICATION_LIST[num]["appmsg"].down_preview_image_path = previewImage
                Globals.APPLICATION_LIST[num]["appmsg"].downloadcount = installCount
                Globals.APPLICATION_LIST[num]["appmsg"].ratings_average = averageScore
                Globals.APPLICATION_LIST[num]["appmsg"].ratings_total = scoreCount
                Globals.APPLICATION_LIST[num]["appmsg"].review_total = commentCount
            num = num + 1
        try:
            self.appmgr.db.update_app_msg(appId,reslist)
        except:
            pass

    def change_rating_list(self, avg_score, appname,softstyle):
        app_msg = {}
        check_list = []
        if(softstyle == "2"):
            check_list.extend(Globals.GET_ALL_DRIVE_APP_LIST)
        else:
            check_list.extend(Globals.GET_RATING_APP_LIST)
        # elif(softstyle == "3"):
        #     check_list.extend(Globals.KYDROID_APP_LIST_BY_RATING)
        num = 0
        for i in check_list:
            if (i["name"] == appname):
                app_msg = i
                # check_list.remove(i)
                check_list[num]["rating_avg"] = avg_score
                break
            num = num + 1

        sort_list = sorted(check_list, key=lambda app: (app["rating_avg"], app["download_total"]), reverse=True)

        if (softstyle == "2"):
            Globals.GET_RATING_DRIVE_APP_LIST.clear()
            Globals.GET_RATING_DRIVE_APP_LIST.extend(sort_list)
        else:
            Globals.GET_RATING_APP_LIST.clear()
            Globals.GET_RATING_APP_LIST.extend(sort_list)
        # elif (softstyle == "3"):
        #     Globals.KYDROID_APP_LIST_BY_RATING.clear()
        #     Globals.KYDROID_APP_LIST_BY_RATING.extend(check_list)
        # # Globals.GET_RATING_APP_LIST

    def change_kydroid_rating_list(self, avg_score, appname, softstyle):
        app_msg = {}
        check_list = []
        check_list.extend(self.appmgr.ratting_apk_list)
        num = 0
        for i in check_list:
            if (i.pkgname == appname):
                # app_msg = i
                # check_list.remove(i)
                check_list[num].ratings_average = avg_score
                break
            num = num + 1
        sort_list = sorted(check_list, key=lambda app: (app.ratings_average, app.downloadcount), reverse=True)

        self.appmgr.ratting_apk_list.clear()
        self.appmgr.ratting_apk_list.extend(sort_list)
                
                
    def change_download_list(self, download_total, appname,softstyle):
        app_msg = {}
        check_list = []
        if (softstyle == "2"):
            check_list.extend(Globals.GET_ALL_DRIVE_APP_LIST)
        else:
            check_list.extend(Globals.GET_ALL_APP_LIST)
        num = 0
        for i in check_list:
            if (i["name"] == appname):
                # app_msg = i
                # check_list.remove(i)
                check_list[num]["download_total"] = download_total
                break
            num = num + 1
        #先已评分进行降序排序，若评分一样按照下载数量进行降序排序
        sort_list = sorted(check_list, key=lambda app:(app["download_total"],app["rating_avg"]), reverse=True)

        if (softstyle == "2"):
            Globals.GET_ALL_DRIVE_APP_LIST.clear()
            Globals.GET_ALL_DRIVE_APP_LIST.extend(sort_list)
        else:
            Globals.GET_ALL_APP_LIST.clear()
            Globals.GET_ALL_APP_LIST.extend(sort_list)
            # if (app_msg not in Globals.ALL):
            #     Globals.APPLICATION_LIST.append(app_msg)
        # Globals.GET_RATING_APP_LIST
        
    #
    #
    #
    def change_kydroid_download_list(self, download_total, appname,softstyle):
        app_msg = {}
        check_list = []
        check_list.extend(self.appmgr.apk_list)
        num = 0
        for i in check_list:
            if (i.pkgname == appname):
                # app_msg = i
                # check_list.remove(i)
                check_list[num].ratings_average = download_total
                break
            num = num + 1
        sort_list = sorted(check_list, key=lambda app: (app.downloadcount, app.ratings_average), reverse=True)

        self.appmgr.apk_list.clear()
        self.appmgr.apk_list.extend(sort_list)

    #
    #函数：保存安卓列表
    #
    def get_kydroid_source_list(self,data):
        for msg in data:
            if(msg["packageName"] not in Globals.KYDROID_SOURCE_LIST.keys()):
                Globals.KYDROID_SOURCE_LIST[msg["packageName"]] = msg
        pass

    #函数：获取安卓应用列表
    def get_kydroid_app_list(self):
        res = self.appmgr.get_kydroid_app_list()
        if(res == False):
            # get_kydroid_source_list_value = {"cpu": "Kunpeng920", "systemVersion": "V10SP1", "androidVersion": "kydroid3",
            #                                  "gpu": "NVIDIA"}
            if (Globals.SYSTEM_VERSION.lower() == "Ubuntu_Kylin".lower()):
                systemversion = "V10 SP1"
            else:
                systemversion = Globals.SYSTEM_VERSION
            if(Globals.CPU != ""):
                # get_kydroid_source_list_value = {"cpu": "Kunpeng920", "systemVersion": "V10SP1", "androidVersion": "kydroid3",
                #                                  "gpu": "NVIDIA"}
                if(Globals.CPU.lower() == "HUAWEI_Kirin990".lower()):
                    get_kydroid_source_list_value = {"cpu": "kirin990","systemVersion": systemversion.replace(" ", ""),"androidVersion": GET_KYDROID_VERSION, "gpu": GPU}
                else:
                    get_kydroid_source_list_value = {"cpu": Globals.CPU.replace("-", "").replace(" ", ""), "systemVersion": systemversion.replace(" ",""),"androidVersion": GET_KYDROID_VERSION, "gpu": GPU}
                res = self.appmgr.premoter.get_kydroid_source_list(get_kydroid_source_list_value)
                if (res != False):
                    if (res["code"] == 200 and res["data"] != []):
                        self.get_kydroid_source_list(res["data"])
                    else:
                        Globals.KYDROID_SOURCE_LIST = {}


    def run(self):
        review_list = []
        if(self.mw.kydroid_service.hasKydroid == True):
            self.appmgr.get_kydroid_app_list()
        item = {}
        token_login_value = {}
        local_login_token = self.appmgr.get_local_token()
        if not local_login_token:
            Globals.RE_LOGIN = False
            self.read_token_over.emit({})
            pass
        else:
            ad_list = self.appmgr.verify_token(token_login_value)
            if (ad_list != False):
                self.read_token_over.emit(ad_list)
            else:
                Globals.RE_LOGIN = False
                self.read_token_over.emit({})
        self.appmgr.submit_pingback_main()
        self.check_conf_thread = CheckconfThread(self.mw)
        self.check_conf_thread.start()
        while(1):
            res = None
            if len(self.work_list) == 0:
                time.sleep(0.2)
                continue
            self.mutex.acquire()
            item_dict = self.work_list.pop(0)
            item = item_dict["action"]
            self.mutex.release()
            if (item == "submit_pingback_app"):
                res = self.appmgr.submit_pingback_app(item_dict["appname"], item_dict["id"])
                if (res["code"] == 1001):
                    self.login_failure_signal.emit()
            elif (item == "get_app_sign"):
                # pass
                # app_state = True
                app_state = False
                self.return_app_sign_state.emit(item_dict["appname"], app_state)
            elif (item == "get_download_count"):
                count, app_name,update_time = self.appmgr.get_download_count_from_server(item_dict["appname"], item_dict["id"])
                if(item_dict["softstyle"] == "3"):
                    self.change_kydroid_download_list(count, app_name,item_dict["softstyle"])
                else:
                    self.change_download_list(count, app_name,item_dict["softstyle"])
                self.return_download_count.emit(count, app_name,update_time)
            elif (item == "apk_page_create_emit"):
                self.appmgr.apk_page_create_emit(item_dict["need_show_widget"])
            time.sleep(0.2)


class AccessDbThread(QThread):
    work_list = []
    init_all_data = False
    click_kydroid = {}
    app_search_list = []
    access_over = pyqtSignal(str,list,list)
    get_list_over = pyqtSignal(str,list,list,bool)
    get_drive_list_over = pyqtSignal(str)
    get_search_applist = pyqtSignal(list,bool,bool)
    check_kydroid_envrun = pyqtSignal(bool,bool)
    reflesh_cache_finish = pyqtSignal(str)
    init_timeout = pyqtSignal()
    return_download_count = pyqtSignal(int,str)
    #zzz
    return_app_sign_state = pyqtSignal(str,bool)
    #zzz
    return_submit_review_state = pyqtSignal(dict)
    return_submit_review_state_fail = pyqtSignal(bool)
    #
    get_my_rating_over = pyqtSignal(int,int,str)
    return_rating = pyqtSignal(int,list,float,str)
    return_get_shot_screen_over = pyqtSignal(str)
    submit_my_review_over = pyqtSignal(str)
    exit_login_finish = pyqtSignal()
    get_image_over = pyqtSignal(dict)
    get_image_over_fail = pyqtSignal(bool)
    get_search_appmsg_tip = pyqtSignal(list,bool)
    login_failure_signal = pyqtSignal(str)
    get_search_applist_over = pyqtSignal(list)
    init_appmger_over = pyqtSignal()
    connect_db_false = pyqtSignal()
    get_kydroid_category_list = pyqtSignal(list)
    slot_start_download_apk = pyqtSignal(ApkInfo)
    init_new_arrivals_signal = pyqtSignal()
    init_download_rank = pyqtSignal()
    init_hot_signal = pyqtSignal()
    init_recommend = pyqtSignal(bool)
    init_android_recommend = pyqtSignal()
    check_uksc_update = pyqtSignal()
    get_all_imfomation = pyqtSignal()
    def __init__(self, mw, backend,ca,parent=None):
        QThread.__init__(self)

        locale.setlocale(locale.LC_ALL, "zh_CN.UTF-8")
        self.kus = parent
        self.backend = backend
        self.ca = ca
        self.mutex = threading.RLock()
        self.mw = mw
        self.init = True
        self.is_first_show_kydroid_installed = True
        self.is_first_show_kydroid_update = True
        self.net_work_error = False
        #self.init_home_page = {"screenshot":False,"new app":False,"new ads":False,"new icon":False}
        self.init_home_page = {"new app": False}
        #获取分类中软件的信息
        #self.appmgr.init_models()

    #
    #函数获取初始化数据s
    #
    def get_init_date(self):
        pass
        # self.appmgr.get_newer_application_info()
        #self.appmgr.get_newer_application_screenshots()
        # self.appmgr.get_newer_application_ads()
        # self.appmgr.get_download_ranking()
        # self.appmgr.get_newest_app()
        #self.appmgr.get_newer_application_icon()
        #获取所有新品上架列表
        #获取所有封面图列表
        #获取所有广告图列表

    #
    #函数：显示第一个界面
    #
    def check_firstwidget_show(self):
        # print("获取数据库", time.strftime("%Y-%m-%dT%H:%M:%SZ", time.localtime()))
        self.appmgr.get_desktop_list()
        self.appmgr.get_all_categories_from_db()
        # self.appmgr.get_all_app_information_from_db()
        # self.appmgr.get_search_association()
        self.appmgr.get_ad_from_db()
        self.appmgr.get_drive_categories_from_db()
        # self.appmgr.get_cannot_remove_app_list()
        # print("获取数据库数据结束", time.strftime("%Y-%m-%dT%H:%M:%SZ", time.localtime()))

    def slot_check_kydroid_enrun_over(self,ret,need_show_widget):
        self.check_kydroid_envrun.emit(ret,need_show_widget)

    def init_connect(self):
        self.appmgr.network_error.connect(lambda :self.mw.slot_network_error("./res/no_network.png",_("There is something wrong with the network. Please try again later.")))
        self.appmgr.get_apk_list_over.connect(self.mw.slot_show_kydroid_widget)
        self.appmgr.kydroid_envrun_over.connect(self.slot_check_kydroid_enrun_over)
        self.appmgr.hot_ready.connect(self.mw.slot_hot_apps_ready)
        self.appmgr.recommend_ready.connect(self.mw.slot_recommend_apps_ready)
        self.appmgr.download_ranking_ready.connect(self.mw.slot_download_ranking_ready)
        self.appmgr.get_ad_ready.connect(self.mw.slot_get_ad_ready)
        self.appmgr.get_new_arrivals_ready.connect(self.mw.slot_new_arrivals_ready)
        self.appmgr.get_android_recommend_ready.connect(self.mw.slot_android_recommend_ready)
        self.appmgr.get_win_replace_ready.connect(self.mw.slot_win_replace_ready)
        self.appmgr.apk_process.connect(self.mw.slot_apk_progress)
        self.appmgr.get_search_tip.connect(self.get_search_tip_list)
        self.reflesh_cache_finish.connect(self.mw.slot_reflesh_cache_finish)
        #self.init_time.timeout.connect(self.slot_time_out)
        self.init_timeout.connect(self.mw.slot_network_error)
        self.get_image_over.connect(self.mw.slot_get_image_over)
        self.get_image_over_fail.connect(self.mw.slot_get_image_over)
        self.get_search_applist_over.connect(self.mw.slot_get_search_applist_over)
        self.get_search_appmsg_tip.connect(self.slot_get_search_appmsg)
        self.init_appmger_over.connect(self.mw.slot_init_appmger_over)
        self.connect_db_false.connect(self.mw.slot_connect_db_false)
        self.slot_start_download_apk.connect(self.appmgr.slot_start_download_apk)
        self.get_kydroid_category_list.connect(self.mw.slot_show_kydroid_category_widget)
        self.init_new_arrivals_signal.connect(self.appmgr.get_new_arrivals_from_db)
        self.init_hot_signal.connect(self.appmgr.get_hot_apps_from_db)
        self.init_download_rank.connect(self.appmgr.get_download_ranking_from_db)
        self.init_recommend.connect(self.appmgr.get_recommend_apps)
        self.init_android_recommend.connect(self.slot_init_android_recommend)
        self.check_uksc_update.connect(self.mw.check_uksc_update)
        self.get_all_imfomation.connect(self.slot_get_all_infomation)

    def get_appmsg(self,applist):
        re_applist = []
        if (applist != []):
            for i in applist:
                app = {"id":0,"name":i.name,"displayname":i.name,"displayname_cn":"","categories":"","summary":"","description":i.candidate.description,"rating_avg":0,"rating_total":0,"review_total":0,"download_total":0,"icon":None,"preview_image":None,"cover_image":None,"from_db":False,"arch":None,"system_name":Globals.SYSTEM_VERSION,"depends":None,"cpu_model":"","no_cpu_model":"","initialsName":'',"pinyinName":"","anotherName":"","certSubjectOu":"","softStyle":1,"manufacturers":None,"popularity":None,"website":None}
                re_applist.append(app)
        return re_applist

    def get_appname(self,applist):
        re_applist = []
        if (applist != []):
            for i in applist:
                re = {"show_name":i.name,"english_name":i.name,"categories":""}
                re_applist.append(re)
        return re_applist

    #name
    #函数：初始化超时
    #
    def slot_time_out(self):
        if(self.init == True):
            self.net_work_error = True
            #self.init_time.stop()
            while True:
                try:
                    res = self.appmgr.squeue.get(block=False)
                except:
                    break
            self.init_timeout.emit()

    def check_init_homepage_over(self):
        if not self.appmgr.squeue_put.empty():
            item = self.appmgr.squeue_put.get_nowait()
            action = item["action"]
            if (action == "get_newer_application_info"):
                if(item["re"] != False and item["num"] != 0):
                    self.init_home_page ["new app"] = True
            if(self.init_home_page["new app"] == True):
                self.init_home_page = {"new app": False}
                return True
            else:
                return False

    #
    #函数：修改所有软件列表中的对应软件的信息
    #
    def update_app_detail(self,reslist):
        newestImage = reslist["newestImage"]
        englishName = reslist["packageName"]
        icon = reslist["icon"]
        appId = reslist["appId"]
        slogan = reslist["slogan"]
        averageScore = reslist["averageScore"]
        categoryId = reslist["categoryId"]
        intro = reslist["intro"]
        commentCount = reslist["commentCount"]
        scoreCount = reslist["scoreCount"]
        detailIcon = reslist["detailIcon"]
        appName = reslist["appName"]
        installCount = reslist["installCount"]
        packageName = reslist["packageName"]
        coverImage = reslist["coverImage"]
        previewImage = reslist["previewImage"]
        arch = reslist["cpuArchitecture"]
        system_name = reslist["systemName"]
        num = 0
        for i in Globals.APPLICATION_LIST:
            if i["name"] == englishName:
                Globals.APPLICATION_LIST[num]["appmsg"].icon_path = icon
                Globals.APPLICATION_LIST[num]["appmsg"].down_cover_image_path = coverImage
                Globals.APPLICATION_LIST[num]["appmsg"].down_preview_image_path = previewImage
                Globals.APPLICATION_LIST[num]["appmsg"].downloadcount = installCount
                Globals.APPLICATION_LIST[num]["appmsg"].ratings_average = averageScore
                Globals.APPLICATION_LIST[num]["appmsg"].ratings_total = scoreCount
                Globals.APPLICATION_LIST[num]["appmsg"].review_total = commentCount
                Globals.APPLICATION_LIST[num]["appmsg"].arch = arch
                Globals.APPLICATION_LIST[num]["appmsg"].system_name = system_name
            num = num + 1
        try:
            self.appmgr.db.update_app_msg(appId,reslist)
        except:
            pass

    #
    #函数：获取搜索提示
    #
    def get_search_tip_list(self,tip_list,search_over = False):
        app_list = self.get_appmsg(tip_list)
        if(len(self.app_search_list) < 6):
            for i in app_list:
                if(i["name"] not in self.app_search_list):
                    if(Globals.LAN == "zh_CN"):
                        self.app_search_list.append(i["displayname"])
                    else:
                        self.app_search_list.append(i["name"])
                if(len(self.app_search_list) >= 6):
                    self.get_search_applist_over.emit(self.app_search_list)
                    return
        if(search_over == True):
            self.get_search_applist_over.emit(self.app_search_list)

    #
    #函数：处理获取搜索软件的字典信息
    #
    def slot_get_search_appmsg(self,applist,search_over = False):
        if (len(self.app_search_list) < 6):
            for i in applist:
                apk = None
                if(type(i) != ApkInfo and "100" in i["categories"].split(",")):
                    apk = 1
                else:
                    try:
                        apk = self.appmgr.apt_cache[i["english_name"]]
                    except:
                        in_apk_list = False
                        for app in self.appmgr.apk_list:
                            if(app.pkgname == i["english_name"]):
                                in_apk_list = True
                                self.app_search_list.append(i["show_name"])
                                if (len(self.app_search_list) >= 6):
                                    self.get_search_applist_over.emit(self.app_search_list)
                                    return
                                break
                        if(in_apk_list):
                            continue
                        else:
                            if("17" in i["categories"].split(",")):
                                self.app_search_list.append(i["show_name"])
                                if (len(self.app_search_list) >= 6):
                                    self.get_search_applist_over.emit(self.app_search_list)
                                    return
                            else:
                                apk = None
                if (apk != None):
                    if (i not in self.app_search_list):
                        self.app_search_list.append(i["show_name"])
                    if (len(self.app_search_list) >= 6):
                        self.get_search_applist_over.emit(self.app_search_list)
                        return
        if (search_over == True):
            self.get_search_applist_over.emit(self.app_search_list)

    def slot_init_android_recommend(self):
        # self.appmgr.start_download_kydroid_sl(False)
        self.appmgr.get_android_recommend_from_db()

    def slot_get_all_infomation(self):
        # print("init_ui_over", datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
        self.appmgr.get_all_app_information_from_db()
        # print("init_ui_over", datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
        self.check_uksc_update.emit()
        # print("init_ui_over", datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
        self.appmgr.get_drive_list_by_category()
        # print("init_ui_over", datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
        self.init_all_data = True
        if(Globals.NOWPAGE == PageStates.DRIVE):
            self.mw.slot_click_drive_btn()
        elif(Globals.NOWPAGE == PageStates.CAREGORYPAGE):
            self.mw.slot_click_category_btn()
        elif(Globals.NOWPAGE == PageStates.KYDROID):
            # self.appmgr.start_download_kydroid_sl(False)
            self.mw.slot_click_kydroid_btn()
        # if(self.click_kydroid != {} and Globals.NOWPAGE == True):
        #     self.mutex.acquire()
        #     self.work_list.append(self.click_kydroid)
        #     self.mutex.release()
    #
    # 函数：线程执行函数
    #
    def run(self):
        self.appmgr = AppManager(self.backend,self.ca)
        self.init_connect()
        # Globals.CONNECT_DB = False
        if(Globals.CONNECT_DB == False):
            self.connect_db_false.emit()
            # self.mw.slot_connect_db_false()
            return
        self.appmgr.kydroid_service = self.mw.kydroid_service
        self.init_appmger_over.emit()
        # self.get_init_date()
        #init_time = threading.Timer(60,self.slot_time_out).start()
        review_list = []
        applist = []
        apklist = []
        get_list = []
        search_list = []
        while (True):
            winlist = []
            ad_list = []
            category_app_list = []
            category_ad_list = []
            res = None
            review_list = []
            if (self.init == True and Globals.CONNECT_DB):
                self.init = False
                self.check_firstwidget_show()
                self.mw.init_sessionbus()
                # saerch_item = {"action": "get_search_association_list"}
                # self.mutex.acquire()
                # self.work_list.append(saerch_item)
                # self.mutex.release()
                # set_item = {"action":"","appname":""}
                # set_item["action"] = "get_update_num"
                # self.mutex.acquire()
                # self.work_list.append(set_item)
                # self.mutex.release()
                # # self.get_init_date()
                # set_item = {"action": ""}
                # set_item["action"] = "get_cannot_remove_list"
                # self.mutex.acquire()
                # self.work_list.append(set_item)
                # self.mutex.release()

            # init_over = self.check_init_homepage_over()
            # if(self.init == False and init_over):
            #     while True:
            #         try:
            #             res = self.appmgr.squeue_put.get(block=False)
            #         except:
            #             break
            #     Globals.INIT_DATA_OVER = True
            if len(self.work_list) == 0:
                time.sleep(0.2)
                continue
            self.mutex.acquire()
            item_dict = self.work_list.pop(0)
            item = item_dict["action"]
            self.mutex.release()
            if(item == "init_firstwidget"):
                if (self.init == True):
                    self.init = False
                    self.check_firstwidget_show()
                    self.mw.init_sessionbus()
                    # saerch_item = {"action": "get_search_association_list"}
                    # self.mutex.acquire()
                    # self.work_list.append(saerch_item)
                    # self.mutex.release()
                    # set_item = {"action": "", "appname": ""}
                    # set_item["action"] = "get_update_num"
                    # self.mutex.acquire()
                    # self.work_list.append(set_item)
                    # self.mutex.release()
                    # set_item = {"action": "", "appname": ""}
                    # set_item["action"] = "get_cannot_remove_list"
                    # self.mutex.acquire()
                    # self.work_list.append(set_item)
                    # self.mutex.release()
            elif(item == "get_cannot_remove_list"):
                self.appmgr.get_cannot_remove_app_list()
            elif(item == "init_all_app_list"):
                self.get_all_imfomation.emit()
            elif (item == "get_update_num" or item == "get_update_list"):
                apklist.clear()
                applist.clear()
                applist, apklist = self.appmgr.get_need_update_app(True)
                self.access_over.emit(item,applist,apklist)
            elif (item == "get_history_list"):
                apklist.clear()
                applist.clear()
                applist, apklist = self.appmgr.get_history_install_app(item_dict["type"])
                self.access_over.emit(item,applist,apklist)
            elif( item == "get_uninstall_list"):
                get_list.clear()
                apklist.clear()
                applist.clear()
                applist, apklist = self.appmgr.get_need_update_app()
                for i in applist:
                    if(i["name"] in Globals.CAN_NOT_REMOVE_SOFT):
                        continue
                    else:
                        get_list.append(i)
                self.access_over.emit(item, get_list, apklist)
            elif( item == "get_new_data"):
                # self.appmgr.submit_pingback_main()
                self.get_init_date()
            elif(item == "get_search_association_list"):
                self.appmgr.get_search_association()
            elif( item == "reload_home_page"):
                Globals.GET_HOME_PGE_READY = {"AD":False,"NEW":False,"HOT":False,"RANK":False,"RECOMMEND":False,"ANDROID_RECOMMEND":False}
                Globals.GET_ALL_APP_LIST = []
                Globals.GET_ALL_DRIVE_APP_LIST = []
                Globals.GET_ALL_KYDROID_APP_LIST = []
                Globals.GET_RATING_DRIVE_APP_LIST = []
                Globals.GET_RATING_APP_LIST = []
                Globals.KYDROID_APP_LIST_BY_RATING = []
                self.mw.detailwidget.ca.open()
                self.check_firstwidget_show()
                saerch_item = {"action": "get_search_association_list"}
                self.mutex.acquire()
                self.work_list.append(saerch_item)
                self.mutex.release()
                set_item = {"action": "", "appname": ""}
                set_item["action"] = "get_update_num"
                self.mutex.acquire()
                self.work_list.append(set_item)
                self.mutex.release()
            elif(item == _("replace Win")):
                if(Globals.WIN_REPLACE_LIST["Win"]==[] and Globals.WIN_REPLACE_LIST["Linux"]==[]):
                    get_list= self.appmgr.db.query_win_replace_apps()
                    for i in get_list:
                        if (i[1] != "yuliu"):
                            pkg = self.appmgr.get_one_app_info_by_appname(i[1])
                            if (pkg != None):
                                winlist.append(i)
                                applist.append(pkg)
                    Globals.WIN_REPLACE_LIST["Win"] = winlist
                    Globals.WIN_REPLACE_LIST["Linux"] = applist
                else:
                    winlist = Globals.WIN_REPLACE_LIST["Win"]
                    applist = Globals.WIN_REPLACE_LIST["Linux"]
                self.get_list_over.emit(item,winlist,applist)
                pass
            elif(item == "search_app"):
                search_list.clear()
                apklist.clear()
                applist.clear()
                get_list.clear()
                appname = self.mw.title_bar.search_line.text().strip()
                applist,apklist = self.appmgr.get_app_info_by_appname(appname,item_dict["search_app_list"],item_dict["search_apk_list"])
                if(Globals.ADVANCED_SEARCH == False):
                    get_list = self.appmgr.get_cache_apt_pkg(appname,False,applist,item_dict["is_remove"])
                    get_list = self.get_appmsg(get_list)
                if(Globals.ADVANCED_SEARCH == False):
                    search_list.extend(applist)
                    search_list.extend(get_list)
                else:
                    search_list.extend(applist)
                if(apklist != []):
                    search_list.extend(apklist)
                self.get_search_applist.emit(search_list,item_dict["is_remove"],item_dict["need_all_search"])
            elif(item == "search_app_tips"):
                self.app_search_list = []
                search_list.clear()
                apklist.clear()
                applist.clear()
                get_list.clear()
                appname = self.mw.title_bar.search_line.text().strip()
                if(appname != ""):
                    applist, apklist = self.appmgr.get_tip_app_info_by_appname(appname,item_dict["search_app_list"],item_dict["search_apk_list"])
                    search_list.extend(applist)
                    search_list.extend(apklist)
                    if (search_list != []):
                        self.get_search_appmsg_tip.emit(search_list, False)
                    if (Globals.ADVANCED_SEARCH == False and len(search_list)<6):
                        get_list = self.appmgr.get_cache_apt_pkg(appname,True,applist,item_dict["is_remove"])
                        get_list = self.get_appname(get_list)
                    if (len(self.app_search_list) < 6):
                        self.get_search_appmsg_tip.emit(get_list, True)
            elif(item == "reflesh cache"):
                self.mw.detailwidget.ca.close()
                self.mw.detailwidget.ca.open()
                # self.appmgr.apt_cache.close()
                # self.appmgr.apt_cache.open()
                # if(item_dict["apt_action"].lower() == "install"):
                #     try:
                #         pkg = self.mw.detailwidget.ca[item_dict["appname"]]
                #     except:
                #         pkg = None
                #     if(pkg != None and pkg.is_installed == True and pkg.is_upgradable == False):
                #         self.reflesh_cache_finish.emit(item_dict["appname"])
                #     else:
                #         if(item["percent"] == -11 or ):
                #             self.reflesh_cache_finish.emit(item_dict["appname"])
                # else:
                # 刷新完cache进行按钮状态修改
                self.reflesh_cache_finish.emit(item_dict["appname"])
            elif(item == "check_kydroid"):
                self.appmgr.kydroid_check()
            elif (item == "check_kydroid_envrun"):
                res = self.appmgr.check_kydroid_envrun()
                self.check_kydroid_envrun.emit(res)
            elif(item == "download_kydroid_sl"):
                self.appmgr.start_download_kydroid_sl()
            elif(item == "cycle_check_kydroid_envrun"):
                self.appmgr.cycle_check_kydroid_envrun(item_dict["need_show_kyroid_widget"])
            elif(item == "apk_page_create_emit"):
                self.appmgr.apk_page_create_emit()
            elif(item == "remove_apk"):
                self.appmgr.uninstall_app(item_dict["appname"])
            elif(item == "install_apk"):
                self.appmgr.start_download_apk(item_dict["app"])
            #zzz
            elif(item == "get_app_sign"):
                # pass
                # app_state = True
                app_state = False
                self.return_app_sign_state.emit("dd",app_state)
            elif(item == "get_installed_kydroid_app"):
                Globals.INSTALLED_KYDROID_APK = []
                if(os.path.exists(KYDROID_INSTALLED_FILENAME)):
                    try:
                        with open(KYDROID_INSTALLED_FILENAME, 'r', encoding='utf8')as fp:
                            json_data = json.load(fp)
                            for i in json_data["data"]:
                                if(i not in Globals.INSTALLED_KYDROID_APK):
                                    Globals.INSTALLED_KYDROID_APK.append(i)
                    except:
                        pass
                # Globals.INSTALLED_KYDROID_APK = [{"name":"com.tencent.mm","version":"8.0.8","size":191919428}]
            elif(item == "get_kydroid_category_applist"):
                apklist.clear()
                apklist = self.appmgr.get_android_category(item_dict["category"])
                self.get_kydroid_category_list.emit(apklist)
                Globals.KYDROID_SORTING_METHOD = 2
            elif(item == "save_db_exposure_rate"):
                self.appmgr.db.save_db_exposure_rate()
            elif (item == "save_new_arrivals_exposure_rate"):
                self.appmgr.db.save_new_arrivals_exposure_rate()
            elif (item == "save_hot_exposure_rate"):
                self.appmgr.db.save_hot_exposure_rate()
            elif(item == "get_drive"):
                applist = []
                if(Globals.DRIVE_CATEGORIES_APP_LIST[item_dict["btn_name"]] == []):
                    self.appmgr.get_drive_list_by_category(True)
                if(Globals.DRIVE_CATEGORIES_APP_LIST_BY_RATING[item_dict["btn_name"]] == []):
                    self.appmgr.get_drive_list_by_category(False)
                self.get_drive_list_over.emit(item_dict["btn_name"])
            elif(item == "init_new_arrivals"):
                self.init_new_arrivals_signal.emit()
                #self.appmgr.get_new_arrivals_from_db()
            elif (item == "init_download_rank"):
                self.init_download_rank.emit()
                # self.appmgr.get_download_ranking_from_db()
            elif (item == "init_hot_app"):
                self.init_hot_signal.emit()
                # self.appmgr.get_hot_apps_from_db()
            elif(item == "init_recommend"):
                self.init_recommend.emit(False)
                # self.appmgr.get_recommend_apps(False)
            elif(item == "init_android_recommend"):
                self.init_android_recommend.emit()
            else:
                if (Globals.NOW_CATEGORY == item):
                    #获取分类界面的banner保存到category_ad_list
                    if(Globals.SORTING_METHOD == 0):
                        if ((item in Globals.ALL_CATEGORY_APP_LIST.keys() and Globals.ALL_CATEGORY_APP_LIST[item] == []) or (item not in Globals.ALL_CATEGORY_APP_LIST.keys())):
                            category_app_list,ad_list = self.appmgr.get_app_info_by_categories(item)
                            if(category_app_list == None or ad_list == None):
                                pass
                            else:
                                try:
                                    Globals.ALL_CATEGORY_APP_LIST[item].clear()
                                    Globals.CATEGORIES_AD_LIST[item].clear()
                                except:
                                    Globals.ALL_CATEGORY_APP_LIST[item] = []
                                    Globals.CATEGORIES_AD_LIST[item] = []
                                Globals.ALL_CATEGORY_APP_LIST[item].extend(category_app_list)
                                Globals.CATEGORIES_AD_LIST[item].extend(ad_list)
                                Globals.SORTING_METHOD = 2
                        else:
                            sort_list = sorted(Globals.ALL_CATEGORY_APP_LIST[item], key=lambda app:(app["download_total"],app["rating_avg"]), reverse=True)
                            Globals.ALL_CATEGORY_APP_LIST[item].clear()
                            Globals.ALL_CATEGORY_APP_LIST[item].extend(sort_list)
                            category_app_list.extend(sort_list)
                            category_app_list.extend(Globals.ALL_CATEGORY_APP_LIST[item])
                            ad_list.extend(Globals.CATEGORIES_AD_LIST[item])
                    elif(Globals.SORTING_METHOD == 1):
                        if ((item in Globals.ALL_CATEGORY_APP_LIST_BY_RATING.keys() and Globals.ALL_CATEGORY_APP_LIST_BY_RATING[item] == []) or (item not in Globals.ALL_CATEGORY_APP_LIST_BY_RATING.keys())):
                            category_app_list, ad_list = self.appmgr.get_app_info_by_categories(item)
                            if (category_app_list == None or ad_list == None):
                                pass
                            else:
                                try:
                                    Globals.ALL_CATEGORY_APP_LIST_BY_RATING[item].clear()
                                    Globals.CATEGORIES_AD_LIST[item].clear()
                                except:
                                    Globals.ALL_CATEGORY_APP_LIST_BY_RATING[item] = []
                                    Globals.CATEGORIES_AD_LIST[item] = []
                                Globals.ALL_CATEGORY_APP_LIST_BY_RATING[item].extend(category_app_list)
                                Globals.CATEGORIES_AD_LIST[item].extend(ad_list)
                        else:
                            sort_list = sorted(Globals.ALL_CATEGORY_APP_LIST_BY_RATING[item], key=lambda app:(app["rating_avg"],app["download_total"]), reverse=True)
                            Globals.ALL_CATEGORY_APP_LIST_BY_RATING[item].clear()
                            Globals.ALL_CATEGORY_APP_LIST_BY_RATING[item].extend(sort_list)
                            category_app_list.extend(Globals.ALL_CATEGORY_APP_LIST_BY_RATING[item])
                            ad_list.extend(Globals.CATEGORIES_AD_LIST[item])
                        Globals.SORTING_METHOD = 2
                    else:
                        if ((item in Globals.ALL_CATEGORY_APP_LIST_BY_POPULARITY.keys() and Globals.ALL_CATEGORY_APP_LIST_BY_POPULARITY[item] == []) or (item not in Globals.ALL_CATEGORY_APP_LIST_BY_POPULARITY.keys())):

                            category_app_list, ad_list = self.appmgr.get_app_info_by_categories(item)
                            if (category_app_list == None or ad_list == None):
                                pass
                            else:
                                try:
                                    Globals.ALL_CATEGORY_APP_LIST_BY_POPULARITY[item].clear()
                                    Globals.CATEGORIES_AD_LIST[item].clear()
                                except:
                                    Globals.ALL_CATEGORY_APP_LIST_BY_POPULARITY[item] = []
                                    Globals.CATEGORIES_AD_LIST[item] = []
                                Globals.ALL_CATEGORY_APP_LIST_BY_POPULARITY[item].extend(category_app_list)
                                Globals.CATEGORIES_AD_LIST[item].extend(ad_list)
                        else:
                            category_app_list.extend(Globals.ALL_CATEGORY_APP_LIST_BY_POPULARITY[item])
                            ad_list.extend(Globals.CATEGORIES_AD_LIST[item])
                        Globals.SORTING_METHOD = 2
                    if(category_app_list != None or ad_list != None):
                        if(Globals.NOW_CATEGORY == item):
                            self.get_list_over.emit(item, ad_list, category_app_list,item_dict["need_all_flesh"])
                else:
                    if len(self.work_list) == 0:
                        pass
                    else:
                        re_list = self.work_list
                        for i in re_list:
                            if("type" in i.keys() and i["type"] == "switch_classification"):
                                if(Globals.NOW_CATEGORY != i["action"]):
                                    self.work_list.remove(i)

            winlist.clear()
            # applist.clear()
            # apklist.clear()
            # get_list.clear()
            # ad_list.clear()
            res = None
            # review_list.clear()
            # search_list.clear()
            category_ad_list.clear()
            time.sleep(0.2)
