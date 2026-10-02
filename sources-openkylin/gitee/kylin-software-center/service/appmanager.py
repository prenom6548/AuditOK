#!/usr/bin/python3
# -*- coding: utf-8 -*-

import locale
import pwd
from urllib.error import *
from PyQt5.QtCore import *
from PyQt5.QtGui import *
from service.dbmanager import *
from kydroid.kydroid_service import *
from kydroid import confparse
from kydroid import downloadmanager
from kydroid.uninstallmanager import *
import multiprocessing
from utils.record_history_installation import *
from models.application import *
from models.apkinfo import *
from models.category import *
import configparser
from models import globals
from models.enums import Signals,UBUNTUKYLIN_RES_PATH,get_app_info,category_dict,UBUNTUKYLIN_CACHE_NEW_ARRIVALS_PATH,ARCH,SYSTEM_VERSION,GET_KYDROID_VERSION,GPU,CONF_PATH,PageStates
from utils.log_manager import LogManager
import gettext
gettext.textdomain("kylin-software-center")
_ = gettext.gettext

# Globals.LOG_INFO = LogManager(INFO_LOG, level='info')
# Globals.LOG_ERROR = LogManager(ERR_LOG, level='error')

from concurrent.futures import ThreadPoolExecutor
pool = ThreadPoolExecutor(max_workers=2)

class AppManager(QObject,Signals):
    apk_process = pyqtSignal(str, str, str, int, str)
    kydroid_envrun_over = pyqtSignal(bool,bool)
    download_apk_source_over = pyqtSignal(bool)
    download_apk_source_error = pyqtSignal(bool)
    get_apk_list_over = pyqtSignal(bool)
    network_error = pyqtSignal(str,str)
    get_search_tip = pyqtSignal(list,bool)
    kydroid_service = None
    kydroid = KydroidService()
    apk_list = []
    ratting_apk_list = []
    popularity_apklist = []
    kydroid_check = kydroid.check_has_kydroid()
    submit_rating_over = pyqtSignal(float,str)
    get_popularity_sort_signal = pyqtSignal()
    get_rating_list_signal = pyqtSignal(str)
    def __init__(self,backend,ca):
        #super(AppManager, self).__init__()
        QObject.__init__(self)

        # # 函数参数通过json文件读取
        # try:
        #     with open('./data/clientAPItest/values.json', 'r', encoding='utf8') as fp:
        #         json_values = json.load(fp)
        # except:
        #     if(Globals.DEBUG_SWITCH):
        #     #print("参数文件values.json读取失败")
        #         Globals.LOG_ERROR.logger.error(_("参数文件values.json读取失败"))

        self.apkenvrunfrist = False
        self.apt_cache = ca
        self.backend = backend
        self.cat_list = {}
        self.apk_list = []
        self.ratting_apk_list = []
        # self.open_cache()
        res = os.popen("LC_ALL=en_US apt-cache policy kylin-software-center |grep Installed|awk '{print $2}'").readline()
        Globals.UKSC_VERSION = res
        self.db = Database()
        if(Globals.CONNECT_DB == False):
            return
        # self.squeue = multiprocessing.Queue()
#zzzzzzzzzzzzzzzzzzzzzzzzzzz
        self.language = 'zh_CN'  # 'any' for all
        self.distroseries = 'any'  #'any' for all
        self.worklist = []
        self.mutex = threading.RLock()
        # self.worker_thread = ThreadWorkerDaemon(self)
        # self.cancel_name_list = []
        # self.worker_thread.setDaemon(True)
        # self.worker_thread.start()
        # self.silent_process.daemon = True
        # self.squeue_put = multiprocessing.Queue()
        # self.silent_process = SilentProcess(self.db.cursor,self.db.connect,self.squeue,self.squeue_put)
        # self.silent_process.start()
        # self.silent_process.daemon = True
        self.backend.kydroid_dbus_ifaces()
        self.list = self.db.query_categories()
        for item in self.list:
            # c = UnicodeToAscii(item[2])
            c = item[2]
            zhcnc = item[3]
            index = item[4]
            visible = (item[0] == 1)
            icon = UBUNTUKYLIN_RES_PATH + str(c) + ".png"
            # if (c == 'recommend'):
            #     cat = Category(c, zhcnc, index, visible, icon, self.get_category_apps_from_db(c))
            # else:
            #
            #     cat = Category(c, zhcnc, index, visible, icon, {})
            # self.cat_list[c] = cat
        self.check_kydroid_envrun()

        self.get_popularity_sort_signal.connect(self.get_popularity_sort)
        self.get_rating_list_signal.connect(self.get_rating_list)
    #--------------------------------0.1----------------------------------

    def open_cache(self):
        locale.setlocale(locale.LC_ALL, "zh_CN.UTF-8")
        if not self.apt_cache:
            self.apt_cache = apt.Cache()
        self.apt_cache.open()
        self.pkgcount = len(self.apt_cache)

    def refresh_apt_cache(self):
        self.apt_cache.close()
        self.apt_cache.open()

    # check and download kydroid apk sourcelist
    def start_download_kydroid_sl(self,need_show_widget = True):
        Globals.isOnline = True
        if (Globals.DEBUG_SWITCH):
            #print("start_download_kydroid_sl")
            Globals.LOG_INFO.logger.info(_("start_download_kydroid_sl"))
        # try:
        #     urllib.request.urlopen(Globals.FIND_PASSWORD, timeout=2)
        # except HTTPError as e:
        #     if e.code != 401:
        #         Globals.isOnline = False
        # except Exception as e:
        #     Globals.isOnline = False
        if Globals.isOnline == False:
            if (Globals.DEBUG_SWITCH):
                #print('bad apk source   ')
                Globals.LOG_INFO.logger.info(_('bad apk source   '))
            return False
            # self.download_apk_source_over.emit(False)
        else:
            if(Globals.KYDROID_SOURCE_LIST == {}):
                Globals.isOnline = False
                return False
            else:
                get_apk_list = []
                get_apk_list,self.ratting_apk_list,self.popularity_apklist = confparse.getApks(Globals.KYDROID_SOURCE_LIST,self.apk_list,self)
                self.apk_list.clear()
                self.apk_list.extend(get_apk_list)

            # self.dbapk_list = self.db.query_apk_applications()
            if (Globals.APK_EVNRUN):
                installed_list = self.kydroid_service.get_installed_applist()
                if (installed_list != -1):
                    for app in installed_list:
                        self.merge_apk_list(app)

            for apk in self.apk_list:
                apk.kydroid_service = self.kydroid_service
                search_list = []
                if(Globals.SORTING_METHOD == 0):
                    search_list = Globals.GET_ALL_APP_LIST
                else:
                    search_list = Globals.GET_RATING_APP_LIST
                for dbapk in search_list:
                    if (apk.pkgname == dbapk["name"]):
                        apk.app_id = dbapk["id"]
                        apk.summary_init = apk.orig_summary = dbapk["summary"]
                        apk.description_init = apk.orig_description = dbapk["description"]
                        apk.ratings_average = dbapk["rating_avg"]
                        apk.ratings_total = dbapk["rating_total"]
                        apk.review_total = dbapk["review_total"]
                        apk.downloadrank = dbapk["download_total"]
                        apk.downloadcount = dbapk["download_total"]
                        apk.icon_path = dbapk["icon"]
                        apk.down_cover_image_path = dbapk["cover_image"]
                        apk.down_preview_image_path = dbapk["preview_image"]
                        apk.system_name = dbapk["system_name"]
                        apk.arch = dbapk["arch"]
                        apk.cpu_model = dbapk["cpu_model"]
                        apk.no_cpu_model = dbapk["no_cpu_model"]
                        apk.depends = dbapk["depends"]
                        apk.from_ukscdb = True
                        category = ""
                        category_cn = ""
                        category_list = dbapk["categories"].split(",")
                        try:
                            num = 0
                            for i in category_list:
                                if (not i.isdigit()):
                                    continue
                                if (int(i) != 1 and int(i) != 2 and int(i) != 15 and int(i) != 18 and int(i) != 16 and int(i) != 17):
                                    get_category, get_category_cn = self.get_category_name_by_id(int(i))
                                    if (num == 0):
                                        if(i not in Globals.KYDROID_CATEGORY):
                                            Globals.KYDROID_CATEGORY.append(i)
                                        category = get_category
                                        category_cn = get_category_cn
                                    else:
                                        if (i not in Globals.KYDROID_CATEGORY):
                                            Globals.KYDROID_CATEGORY.append(i)
                                        category = category + "、" + get_category
                                        category_cn = category_cn + "、" + get_category_cn
                                    # break
                                elif (int(i) == 17):
                                    if (num == 0):
                                        category = "kydroid"
                                        category_cn = "安卓软件"
                                    else:
                                        category = category + "、kydroid"
                                        category_cn = category_cn + "、安卓软件"
                                else:
                                    category = category
                                    category_cn = category_cn
                                num = num + 1
                                if (num > 3):
                                    break
                        except:
                            category = category_cn = ""
                        apk.category_name = category
                        apk.category_name_cn = category_cn

            # self.download_apk_source_over.emit(True)
            # self.get_recommend_apps(False)
            if self.apkenvrunfrist:
                # self.download_apk_source_over.emit(True)
                self.apkenvrunfrist = False
            # if(need_show_widget):
            Globals.INIT_APK_LIST_OVER = True
            self.get_apk_list_over.emit(need_show_widget)
            return True


    # get apk by pkgname
    def get_apk_by_name(self, pkgname):
        for oneapp in self.apk_list:
            if oneapp.pkgname == pkgname:
                return oneapp
        return None

    #
    #检查包状态
    #
    def check_package_statue(self,get_package_dict,app_dict,get_sourse_list):
        sourse_list = {}
        need_show = True
        if (str(app_dict["id"]) in get_package_dict.keys()):
            for package in get_package_dict[str(app_dict["id"])]:
                try:
                    app_version = self.apt_cache[app_dict["name"]].candidate.version
                except:
                    continue
                for source in get_sourse_list:
                    # if (source.startswith("https")):
                    #     pkg_source = package[3].replace("https", "", 1)
                    # else:
                    pkg_source = package[3]
                    if (len(source.split(" ")) >= 2 and source.split(" ")[1] != "default"):
                        if (pkg_source.startswith(source.split(" ")[0]) and source.split(" ")[1] in
                                pkg_source.split(app_dict["name"])[0]):
                            if(package[6] == app_version):
                                if (source not in sourse_list.keys()):
                                    if (str(package[4]) == "1" and str(package[5]) == "0"):
                                        sourse_list[source] = "1"  # 表示上架且未删除状态
                                    else:
                                        sourse_list[source] = "0"  # 表示下架状态或仓库源中删除
                                else:
                                    if (sourse_list[source] == "1"):
                                        pass
                                    else:
                                        if (str(package[4]) == "1" and str(package[5]) == "0"):
                                            sourse_list[source] = "0"  # 表示上架且未删除状态
                                        else:
                                            sourse_list[source] = "1"  # 表示下架状态或仓库源中删除
                    else:
                        if (pkg_source.startswith(source.split(" ")[0])):
                            if (package[6] == app_version):
                                if (source not in sourse_list.keys()):
                                    if (str(package[4]) == "1" and str(package[5]) == "0"):
                                        sourse_list[source] = "1"  # 表示上架且未删除状态
                                    else:
                                        sourse_list[source] = "0"  # 表示下架状态或仓库源中删除
                                else:
                                    if (sourse_list[source] == "1"):
                                        pass
                                    else:
                                        if (str(package[4]) == "1" and str(package[5]) == "0"):
                                            sourse_list[source] = "1"  # 表示上架且未删除状态
                                        else:
                                            sourse_list[source] = "0"  # 表示下架状态或仓库源中删除
            need_show = True
            for statue in sourse_list:
                if (str(sourse_list[statue]) == "0"):
                    need_show = False
                    break
        return need_show

    #
    #函数：获取所有的应用信息
    #
    def get_all_app_information_from_db(self):
        get_list = []
        rating_list = []
        Globals.GET_ALL_APP_LIST = []
        Globals.GET_RATING_APP_LIST = []
        get_package_dict = {}
        get_sourse_list = []
        # get_package_dict,get_sourse_list = self.db.query_package_list()
        # print("init_ui_over", datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
        get_list = self.db.query_all_applications()
        # Globals.SYSTEM_VERSION ="V10 SP1"
        # Globals.CPU = "kunpeng920"
        # Globals.CPU = "FT1500A"
        # Globals.ARCH = "arm64"
        check_cpu = Globals.CPU
        if (os.path.exists("/sys/class/hwmon")):
            for path in os.listdir("/sys/class/hwmon"):
                if (os.path.exists("/sys/class/hwmon/" + path + "/name")):
                    read_name = os.popen("cat /sys/class/hwmon/" + path + "/name").readline()
                    if ("huawei_qs" in read_name.strip("\n")):
                        check_cpu = "huawei_qs"
        for i in get_list:
            show_app = True
            hide_app = False
            app_dict = self.set_all_app_list(i)
            need_show = self.check_package_statue(get_package_dict,app_dict,get_sourse_list)
            if(need_show == False):
                # print(app_dict["id"])
                continue
            if("100" in app_dict["categories"]):
                for category in Globals.ALL_CATEGORIES:
                    for app_category in app_dict["categories"].split(","):
                        if(str(category["id"]) == app_category):
                            if(category not in Globals.ORDINARY_CATEGORY):
                                Globals.ORDINARY_CATEGORY.append(category)
                Globals.GET_ALL_APP_LIST.append(app_dict)
                continue
            if(app_dict["name"] not in Globals.BLACKLIST):
                if(app_dict["name"] != Globals.APPNAME and app_dict["name"] != Globals.KUM):
                    if(app_dict["name"] == Globals.KYDROID_PACKAGE_NAME):
                        kernel_version = os.popen("uname -r").readline().strip("\n")
                        if(len(kernel_version.split("-")) >= 2 and len(kernel_version.split("-")[1]) > 2):
                            kernel_time = kernel_version.split("-")[1].split(".")[0]
                            if(len(kernel_time) == 8):
                                if(int(kernel_time) < 20200710):
                                    continue
                    if(app_dict["cpu_model"] != None and app_dict["cpu_model"] != ""):
                        arch_in = False
                        # num = 0
                        for cpu in app_dict["cpu_model"].split(";"):
                            if(Globals.ARCH.lower() == cpu.split(":")[0].lower() or cpu.split(":")[0].lower() == "all"):
                                arch_in = True
                                # if (num != 0):
                                #     app_dict["cpu_model"] = app_dict["cpu_model"] + "," + cpu.split(":")[1].lower()
                                # else:
                                app_dict["cpu_model"] = ""
                                for i in cpu.split(":")[1].lower().split(","):
                                    if(i == "huawei_qs"):
                                        if(Globals.CPU.lower() == "kunpeng920"):
                                            if (app_dict["cpu_model"] == ""):
                                                app_dict["cpu_model"] = "kunpeng920"
                                            else:
                                                app_dict["cpu_model"] = app_dict["cpu_model"] + "," + "kunpeng920"
                                        else:
                                            if (app_dict["cpu_model"] == ""):
                                                app_dict["cpu_model"] = i
                                            else:
                                                app_dict["cpu_model"] = app_dict["cpu_model"] + "," + i
                                    else:
                                        if(app_dict["cpu_model"] == ""):
                                            app_dict["cpu_model"] = i
                                        else:
                                            app_dict["cpu_model"] = app_dict["cpu_model"] + "," + i
                                # if(cpu.split(":")[1].lower() == "huawei_qs".lower()):
                                #     if(Globals.CPU.lower() == "kunpeng920"):
                                #         app_dict["cpu_model"] = "kunpeng920"
                                #     else:
                                #         app_dict["cpu_model"] = cpu.split(":")[1].lower()
                                # else:
                                #     app_dict["cpu_model"] = cpu.split(":")[1].lower()
                                # num = num + 1
                        if (arch_in == False):
                            show_app = False
                            app_dict["cpu_model"] = None
                    if (app_dict["no_cpu_model"] != None and app_dict["no_cpu_model"] != ""):
                        arch_in = False
                        # num = 0
                        for cpu in app_dict["no_cpu_model"].split(";"):
                            if (Globals.ARCH.lower() == cpu.split(":")[0].lower() or cpu.split(":")[0].lower() == "all"):
                                arch_in = True
                                app_dict["no_cpu_model"] = ""
                                for i in cpu.split(":")[1].lower().split(","):
                                    if (i == "huawei_qs"):
                                        if (Globals.CPU.lower() == "kunpeng920"):
                                            if (app_dict["no_cpu_model"] == ""):
                                                app_dict["no_cpu_model"] = "kunpeng920"
                                            else:
                                                app_dict["no_cpu_model"] = app_dict["no_cpu_model"] + "," + "kunpeng920"
                                        else:
                                            if (app_dict["no_cpu_model"] == ""):
                                                app_dict["no_cpu_model"] = i
                                            else:
                                                app_dict["no_cpu_model"] = app_dict["no_cpu_model"] + "," + i
                                    else:
                                        if (app_dict["no_cpu_model"] == ""):
                                            app_dict["no_cpu_model"] = i
                                        else:
                                            app_dict["no_cpu_model"] = app_dict["no_cpu_model"] + "," + i
                                # if (cpu.split(":")[1].lower() == "huawei_qs".lower()):
                                #     if (Globals.CPU.lower() == "kunpeng920"):
                                #         app_dict["no_cpu_model"] = "kunpeng920"
                                #     else:
                                #         app_dict["no_cpu_model"] = cpu.split(":")[1].lower()
                                # else:
                                #     app_dict["no_cpu_model"] = cpu.split(":")[1].lower()
                                    # num = num + 1

                        if(arch_in == True):
                            hide_app = True
                        else:
                            app_dict["no_cpu_model"] = None
                    # if((app_dict["cpu_model"] == None or Globals.CPU.lower() in app_dict["cpu_model"].lower().split(",")) and (app_dict["no_cpu_model"] == None or Globals.CPU.lower() not in app_dict["no_cpu_model"].lower().split(",")) and (app_dict["arch"] == None or Globals.ARCH.lower() in app_dict["arch"].lower().split(",")) and (app_dict["system_name"] == None or Globals.SYSTEM_VERSION.lower().replace(" ","").replace("-","") in app_dict["system_name"].lower().replace(" ","").replace("-","").split(","))):
                    if((show_app == True and (app_dict["cpu_model"] == None or check_cpu.lower() in app_dict["cpu_model"].lower().split(","))) and (hide_app == False or (hide_app == True and (app_dict["no_cpu_model"] == None or check_cpu.lower() not in app_dict["no_cpu_model"].lower().split(","))))):
                        if(str(app_dict["softStyle"]) == "2"):
                            Globals.GET_ALL_DRIVE_APP_LIST.append(app_dict)
                        elif(str(app_dict["softStyle"]) == "3"):
                            Globals.KYDROID_APP_LIST.append(app_dict)
                        else:
                            ###########
                            # if(app_dict["name"] == "kmre"):
                            #     pass
                            # else:
                            for category in Globals.ALL_CATEGORIES:
                                for app_category in app_dict["categories"].split(","):
                                    if (str(category["id"]) == app_category):
                                        if (category not in Globals.ORDINARY_CATEGORY):
                                            Globals.ORDINARY_CATEGORY.append(category)
                            if(app_dict not in Globals.GET_ALL_APP_LIST):
                                Globals.GET_ALL_APP_LIST.append(app_dict)
                else:
                    if(app_dict["name"] == Globals.APPNAME):
                        Globals.UKSC_APPMSG = app_dict
        # self.get_rating_list_signal.emit(check_cpu)
        # self.get_popularity_sort_signal.emit()
        # print("init_ui_over", datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
        # pool.submit(self.get_popularity_sort)
        self.get_popularity_sort()
        # print("init_ui_over", datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))
        pool.submit(self.get_rating_list,check_cpu,get_package_dict,get_sourse_list)
        # print("init_ui_over", datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"))


    def get_rating_list(self,check_cpu,get_package_dict,get_sourse_list):
        rating_list = self.db.query_applications_by_ratings()
        for i in rating_list:
            show_app = True
            hide_app = False
            app_dict = self.set_all_app_list(i)
            need_show = self.check_package_statue(get_package_dict, app_dict, get_sourse_list)
            if (need_show == False):
                continue
            if ("100" in app_dict["categories"]):
                Globals.GET_RATING_APP_LIST.append(app_dict)
                continue
            if (app_dict["name"] not in Globals.BLACKLIST):
                if (app_dict["name"] != Globals.APPNAME and app_dict["name"] != Globals.KUM ):
                    if (app_dict["name"] == Globals.KYDROID_PACKAGE_NAME):
                        kernel_version = os.popen("uname -r").readline().strip("\n")
                        if(len(kernel_version.split("-")) >= 2 and  len(kernel_version.split("-")[1]) > 2):
                            kernel_time = kernel_version.split("-")[1].split(".")[0]
                            if (len(kernel_time) == 8):
                                if (int(kernel_time) < 20200710):
                                    continue
                    if (app_dict["cpu_model"] != None and app_dict["cpu_model"] != ""):
                        arch_in = False
                        # num = 0
                        for cpu in app_dict["cpu_model"].split(";"):
                            if (Globals.ARCH.lower() == cpu.split(":")[0].lower() or cpu.split(":")[0].lower() == "all"):
                                arch_in = True
                                # if (num != 0):
                                #     app_dict["cpu_model"] = app_dict["cpu_model"] + "," + cpu.split(":")[1].lower()
                                # else:
                                app_dict["cpu_model"] = ""
                                for i in cpu.split(":")[1].lower().split(","):
                                    if (i == "huawei_qs"):
                                        if (Globals.CPU.lower() == "kunpeng920"):
                                            if (app_dict["cpu_model"] == ""):
                                                app_dict["cpu_model"] = "kunpeng920"
                                            else:
                                                app_dict["cpu_model"] = app_dict[
                                                                            "cpu_model"] + "," + "kunpeng920"
                                        else:
                                            if (app_dict["cpu_model"] == ""):
                                                app_dict["cpu_model"] = i
                                            else:
                                                app_dict["cpu_model"] = app_dict["cpu_model"] + "," + i
                                    else:
                                        if (app_dict["cpu_model"] == ""):
                                            app_dict["cpu_model"] = i
                                        else:
                                            app_dict["cpu_model"] = app_dict["cpu_model"] + "," + i
                                # if (num != 0):
                                #     app_dict["cpu_model"] = app_dict["cpu_model"] + "," + cpu.split(":")[1].lower()
                                # else:
                                # app_dict["no_cpu_model"] = cpu.split(":")[1].lower()
                                # num = num + 1
                        if (arch_in == False):
                            show_app = False
                            app_dict["cpu_model"] = None

                    if (app_dict["no_cpu_model"] != None and app_dict["no_cpu_model"] != ""):
                        arch_in = False
                        # num = 0
                        for cpu in app_dict["no_cpu_model"].split(";"):
                            if (Globals.ARCH.lower() == cpu.split(":")[0].lower() or cpu.split(":")[0].lower() == "all"):
                                arch_in = True
                                app_dict["no_cpu_model"] = ""
                                for i in cpu.split(":")[1].lower().split(","):
                                    if (i == "huawei_qs"):
                                        if (Globals.CPU.lower() == "kunpeng920"):
                                            if (app_dict["no_cpu_model"] == ""):
                                                app_dict["no_cpu_model"] = "kunpeng920"
                                            else:
                                                app_dict["no_cpu_model"] = app_dict["no_cpu_model"] + "," + "kunpeng920"
                                        else:
                                            if (app_dict["no_cpu_model"] == ""):
                                                app_dict["no_cpu_model"] = i
                                            else:
                                                app_dict["no_cpu_model"] = app_dict["no_cpu_model"] + "," + i
                                    else:
                                        if (app_dict["no_cpu_model"] == ""):
                                            app_dict["no_cpu_model"] = i
                                        else:
                                            app_dict["no_cpu_model"] = app_dict["no_cpu_model"] + "," + i
                        if (arch_in == True):
                            hide_app = True
                        else:
                            app_dict["no_cpu_model"] = None
                    # if((app_dict["cpu_model"] == None or Globals.CPU.lower() in app_dict["cpu_model"].lower().split(",")) and (app_dict["no_cpu_model"] == None or Globals.CPU.lower() not in app_dict["no_cpu_model"].lower().split(",")) and (app_dict["arch"] == None or Globals.ARCH.lower() in app_dict["arch"].lower().split(",")) and (app_dict["system_name"] == None or Globals.SYSTEM_VERSION.lower().replace(" ","").replace("-","") in app_dict["system_name"].lower().replace(" ","").replace("-","").split(","))):
                    # if((show_app == True and (app_dict["cpu_model"] == None or check_cpu.lower() in app_dict["cpu_model"].lower().split(","))) and (hide_app == False and (app_dict["no_cpu_model"] == None or check_cpu.lower() not in app_dict["no_cpu_model"].lower().split(",")))):
                    if((show_app == True and (app_dict["cpu_model"] == None or check_cpu.lower() in app_dict["cpu_model"].lower().split(","))) and (hide_app == False or (hide_app == True and (app_dict["no_cpu_model"] == None or check_cpu.lower() not in app_dict["no_cpu_model"].lower().split(","))))):
                        if (str(app_dict["softStyle"]) == "2"):
                            Globals.GET_RATING_DRIVE_APP_LIST.append(app_dict)
                        elif (str(app_dict["softStyle"]) == "3"):
                            Globals.KYDROID_APP_LIST_BY_RATING.append(app_dict)
                        else :
                            Globals.GET_RATING_APP_LIST.append(app_dict)

    #
    #函数：处理热度排行
    #
    def get_popularity_sort(self):
        sort_list = sorted(Globals.GET_ALL_APP_LIST, key=lambda app:app["popularity"], reverse=True)
        Globals.GET_POPULARITY_APP_LIST.clear()
        Globals.GET_POPULARITY_APP_LIST.extend(sort_list)

        sort_list = sorted(Globals.KYDROID_APP_LIST, key=lambda app: app["popularity"], reverse=True)
        Globals.KYDROID_APP_LIST_BY_POPULARITY.clear()
        Globals.KYDROID_APP_LIST_BY_POPULARITY.extend(sort_list)

        sort_list = sorted(Globals.GET_ALL_DRIVE_APP_LIST, key=lambda app: app["popularity"], reverse=True)
        Globals.GET_POPULARITY_DRIVE_APP_LIST.clear()
        Globals.GET_POPULARITY_DRIVE_APP_LIST.extend(sort_list)


    #
    #函数：获取所有的安卓应用
    #
    def get_all_kydroid_app_information_from_db(self):
        res = []
        rating_res = []
        res,rating_res = self.db.query_apk_applications()
        for i in res:
            app_dict = self.set_all_app_list(i)
            Globals.KYDROID_APP_LIST.append(app_dict)
        for i in rating_res:
            app_dict = self.set_all_app_list(i)
            Globals.KYDROID_APP_LIST_BY_RATING.append(app_dict)


    #
    # 函数：将数据库中获取的数据转换成Appmsg类型
    #
    def get_appmsg(self, app):
        if (app == None):
            return None
        if (type(app) == ApkInfo):
            return self.get_apkmsg(app)
        try:
            category, category_cn = self.get_category_name_by_id(int(app["categories"].split(",")[0]))
        except:
            category = category_cn = ""
        in_list = False
        for i in Globals.APPLICATION_LIST:
            if (app["name"] == i["name"]):
                app_msg = i["appmsg"]
                in_list = True
        if (in_list == False):
            app_msg = Application(app["id"], app["name"], app["displayname_cn"], category, category_cn,
                                  self.apt_cache)
            Globals.APPLICATION_LIST.append({"name": app_msg.pkgname, "appmsg": app_msg})
        if (app_msg.package == None):
            return None
        app_msg.from_ukscdb = app["from_db"]
        app_msg.orig_name = app["name"]  # zx2015.01.26
        app_msg.orig_summary = app["summary"]
        app_msg.orig_description = app["description"]
        app_msg.displayname = app["displayname_cn"]
        app_msg.summary = app["summary"]
        app_msg.description = app["description"]
        rating_average = app["rating_avg"]
        rating_total = app["rating_total"]
        review_total = app["review_total"]
        app_msg.downloadcount = app["download_total"]
        app_msg.icon_path = app["icon"]
        app_msg.down_preview_image_path = app["preview_image"]
        app_msg.down_cover_image_path = app["cover_image"]
        app_msg.depends = app["depends"]
        app_msg.certSubjectOu = app["certSubjectOu"]
        app_msg.softStyle = app["softStyle"]
        app_msg.popularity = app["popularity"]
        app_msg.website = app["website"]
        if rating_average is not None:
            app_msg.ratings_average = float(rating_average)
        if rating_total is not None:
            app_msg.ratings_total = int(rating_total)
        if review_total is not None:
            app_msg.review_total = int(review_total)
        return app_msg

    #
    # 函数：获取大小
    #
    def get_human_size(self, size):
        size_text = ""
        sizek = size / 1024
        if (sizek <= 1):
            size_text = "1 KB"
        elif (sizek > 1 and sizek < 1024):
            size_text = str('%.0f' % (sizek)) + " KB"
        elif (sizek >= 1024 and sizek < 1024 * 1024):
            if (len(str('%.2f' % (sizek / 1024.0)) + " MB") > 7):
                size_text = str('%.0f' % (sizek / 1024.0)) + " MB"
            else:
                size_text = str('%.2f' % (sizek / 1024.0)) + " MB"
        else:
            if (len(str('%.2f' % (sizek / (1024.0 * 1024.0))) + " GB") > 7):
                size_text = str('%.0f' % (sizek / (1024.0 * 1024.0))) + "GB"
            else:
                size_text = str('%.2f' % (sizek / (1024.0 * 1024.0))) + " GB"
        return size_text

    #
    # 函数：获取apk的软件信息
    #
    def get_apkmsg(self, apk):
        apk.category_name = "kydroid"
        apk.category_name_cn = "安卓软件"
        in_list = False
        for i in Globals.APPLICATION_LIST:
            if (apk.pkgname == i["name"]):
                app_msg = i["appmsg"]
                in_list = True
                if (type(app_msg) != ApkInfo):
                    Globals.APPLICATION_LIST.remove(i)
                    in_list = False
        if (in_list == False):
            Globals.APPLICATION_LIST.append({"name": apk.pkgname, "appmsg": apk})
        return apk

    #
    #函数：向全局变量的所有列表中添加
    #
    def set_all_app_list(self,app):
        app_dict = {"id":0,"name":"","displayname":"","displayname_cn":"","categories":"","summary":"","description":"","rating_avg":0,"rating_total":0,"review_total":0,"download_total":0,"icon":"","cover_image":"","preview_image":"","arch":"","system_name":"","depends":"","cpu_model":"","no_cpu_model":"","from_db":True,"initialsName":'',"pinyinName":"","anotherName":"","certSubjectOu":"","softStyle":1,"popularity":None,"website":None,"manufacturers":None}
        app_dict["id"] = app[0]
        app_dict["name"] = app[1]
        app_dict["displayname"] = app[2]
        app_dict["displayname_cn"] = app[3]
        app_dict["categories"] = app[4]
        app_dict["summary"] = app[5]
        app_dict["description"] = app[6]
        app_dict["rating_avg"] = app[7]
        app_dict["rating_total"] = app[8]
        app_dict["review_total"] = app[9]
        app_dict["download_total"] = app[10]
        app_dict["icon"] = app[11]
        app_dict["cover_image"] = app[12]
        app_dict["preview_image"] = app[13]
        app_dict["arch"] = app[14]
        app_dict["system_name"] = app[15]
        app_dict["depends"] = app[16]
        app_dict["cpu_model"] = app[17]
        app_dict["no_cpu_model"] = app[18]
        app_dict["certSubjectOu"] = app[19]
        app_dict["softStyle"] = app[20]
        app_dict["manufacturers"] = app[21]
        if(app[22] != None and app[22] != ""):
            app_dict["popularity"] = int(float(app[22]))
        else:
            app_dict["popularity"] = 0
        app_dict["website"] = app[23]
        app_dict["from_db"] = True
        # print(app_dict["cpu_model"],app_dict["no_cpu_model"])
        return app_dict

    #
    # 函数：获取安卓软件列表
    #
    def get_kydroid_app_list(self):
        return


    #函数：获取不可卸载软件
    def get_cannot_remove_app_list(self):
        res = self.db.get_cannot_remove_list()
        if(res != None):
            Globals.CAN_NOT_REMOVE_SOFT.clear()
            for i in res:
                Globals.CAN_NOT_REMOVE_SOFT.append(i[0])

    # 函数：获取特殊desktop列表
    def get_desktop_list(self):
        res = self.db.get_desktop_list()
        if(res != None):
            Globals.PKG_NAME.clear()
            for i in res:
                Globals.PKG_NAME[i[0]] = i[1]

    def get_kydroid_source(self,system_version,kydroid_version,arch):
        res = self.db.get_kydroid_source_list(system_version,kydroid_version,arch)
        if(res == None or res == []):
            Globals.KYDROID_SOURCE_SERVER = "http://archive2.kylinos.cn/pulp/isos/kylin/Library/custom/kylin-desktop/kmre/"
        else:
            Globals.KYDROID_SOURCE_SERVER = res[0][0]

    #
    #函数:获取分类列表信息
    #
    def get_all_categories_from_db(self):
        Globals.ALL_CATEGORIES = []
        categories_list = []
        categories_list = self.db.get_categories()
        for i in categories_list:
            res = self.set_categories_list(i)
            zh_local = os.getenv("LANG")
            if ("zh_CN" in zh_local):
                Globals.ALL_CATEGORY_APP_LIST[res["displayname"]] = []
                Globals.ALL_CATEGORY_APP_LIST_BY_RATING[res["displayname"]] = []
            else:
                Globals.ALL_CATEGORY_APP_LIST[res["name"]] = []
                Globals.ALL_CATEGORY_APP_LIST_BY_RATING[res["name"]] = []
            Globals.ALL_CATEGORIES.append(res)

    #
    #函数：从数据库获取驱动分类
    #
    def get_drive_categories_from_db(self):
        Globals.DRIVE_CATEGORIES = []
        categories_list = []
        categories_list = self.db.get_drive_categories()
        for i in categories_list:
            res = self.set_categories_list(i)
            zh_local = os.getenv("LANG")
            if ("zh_CN" in zh_local):
                Globals.DRIVE_CATEGORIES_APP_LIST[res["displayname"]] = []
                Globals.DRIVE_CATEGORIES_APP_LIST_BY_RATING[res["displayname"]] = []
            else:
                Globals.DRIVE_CATEGORIES_APP_LIST[res["name"]] = []
                Globals.DRIVE_CATEGORIES_APP_LIST_BY_RATING[res["name"]] = []
            Globals.DRIVE_CATEGORIES.append(res)
            # self.get_drive_list_by_category()


    #
    #函数：向全局变量中添加数据
    #
    def set_categories_list(self,item):
        category = {"is_visible":0,"id":0,"name":"","displayname":"","priority":0}
        category["is_visible"] = item[0]
        category["id"] = item[1]
        category["name"] = item[2]
        category["displayname"] = item[3]
        category["priority"] = item[4]
        return category

    #
    #函数：通过软件名获取信息
    #
    def get_app_info_by_appname(self,appname,app_list = None,apklist = None):
        search_list = []
        search_apk_list = []
        #apklist = []表示不需要查询安卓应用
        #apklist = None 表示在所有的安卓应用中查询
        # if(apklist == None):
        #     if(self.apk_list != []):
        #         apklist = self.apk_list
        all_list = []
        if(app_list == None):
            # if (Globals.SORTING_METHOD == 0):
            all_list = Globals.GET_ALL_APP_LIST
            # else:
            #     all_list = Globals.GET_RATING_APP_LIST
        else:
            all_list.extend(app_list)
        num = 0
        for i in all_list:
            # try:
            if(type(i) != ApkInfo):
                if((i["name"] != None and appname.lower() in i["name"].lower()) or (i["displayname"] != None and  appname.lower() in i["displayname"].lower()) or (i["displayname_cn"] != None and appname.lower() in i["displayname_cn"].lower()) or (i["initialsName"] != None and appname.lower().replace(" ","") in i["initialsName"].lower().replace(" ","")) or (i["pinyinName"] != None and appname.lower().replace(" ","") in i["pinyinName"].lower().replace(" ","")) or (i["anotherName"] != None and appname.lower().replace(" ","") in i["anotherName"].lower().replace(" ",""))):
                    get_search_app = None
                    if("100" in i["categories"].split(",")):
                        get_search_app = 1
                    else:
                        try:
                            get_search_app = self.apt_cache[i["name"]]
                        except:
                            get_search_app = None
                    if (get_search_app != None):
                        if((i["name"] != None and i["name"].lower().startswith(appname.lower()) )or (i["displayname"] != None and i["displayname"].lower().startswith(appname.lower())) or (i["displayname_cn"] != None and i["displayname_cn"].lower().startswith(appname.lower())) or (i["initialsName"] != None and i["initialsName"].lower().replace(" ","").startswith(appname.lower())) or (i["pinyinName"] != None and  i["pinyinName"].lower().replace(" ","").startswith(appname.lower())) or (i["anotherName"] !=None and i["anotherName"].lower().replace(" ","").startswith(appname.lower()))):
                            search_list.insert(num,i)
                            num = num +1
                        else:

                            search_list.append(i)
            else:
                if ((appname.lower() in i.pkgname.lower()) or (appname in i.displayname_cn.lower()) or appname.lower().replace(" ","") in i.initialsName.lower().replace(
                        " ", "") or appname.lower().replace(" ", "") in i.pinyinName.lower().replace(" ","") or appname.lower().replace(
                    " ", "") in i.anotherName.lower().replace(" ", "")):
                    if (i.pkgname.lower().startswith(appname.lower()) or i.displayname_cn.lower().startswith(
                            appname.lower()) or i.initialsName.lower().replace(" ", "").startswith(
                        appname.lower()) or i.pinyinName.lower().replace(" ", "").startswith(
                        appname.lower()) or i.anotherName.lower().replace(" ", "").startswith(appname.lower())):
                        in_list = False
                        for search_pkg in search_apk_list:
                            if (search_pkg.pkgname == i.pkgname):
                                in_list = True
                                break
                        if (in_list == False):
                            search_apk_list.insert(num, i)
                            num = num + 1
                    else:
                        in_list = False
                        for search_pkg in search_apk_list:
                            if (search_pkg.pkgname == i.pkgname):
                                in_list = True
                                break
                        if (in_list == False):
                            search_apk_list.append(i)
            # except:
            #     continue
        if(apklist != []):
            for apk in Globals.KYDROID_APP_LIST:
                if(apklist != None):
                    for i in apklist:
                        if(type(i) == ApkInfo and i.pkgname == apk["name"]):
                            if ((appname.lower() in i.pkgname.lower()) or (appname.lower() in i.displayname_cn.lower()) or appname.lower().replace(" ","") in i.initialsName.lower().replace(
                                    " ", "") or appname.lower().replace(" ", "") in i.pinyinName.lower().replace(" ","") or appname.lower().replace(
                                    " ", "") in i.anotherName.lower().replace(" ", "")):
                                if (i.pkgname.lower().startswith(appname.lower()) or i.displayname_cn.lower().startswith(
                                        appname.lower()) or i.initialsName.lower().replace(" ", "").startswith(
                                        appname.lower()) or i.pinyinName.lower().replace(" ", "").startswith(
                                        appname.lower()) or i.anotherName.lower().replace(" ", "").startswith(appname.lower())):
                                    in_list = False
                                    for search_pkg in search_apk_list:
                                        if (search_pkg.pkgname == i.pkgname):
                                            in_list = True
                                            break
                                    if (in_list == False):
                                        search_apk_list.insert(num, i)
                                        num = num + 1
                                else:
                                    in_list = False
                                    for search_pkg in search_apk_list:
                                        if (search_pkg.pkgname == i.pkgname):
                                            in_list = True
                                            break
                                    if (in_list == False):
                                        search_apk_list.append(i)
                else:
                    if ((appname.lower() in apk["name"].lower()) or (appname.lower() in apk["displayname"].lower()) or (
                            appname.lower() in apk["displayname_cn"].lower()) or (
                            appname.lower().replace(" ", "") in apk["initialsName"].lower().replace(" ", "")) or (
                            appname.lower().replace(" ", "") in apk["pinyinName"].lower().replace(" ", "")) or (
                            appname.lower().replace(" ", "") in apk["anotherName"].lower().replace(" ", ""))):
                        if (apk["name"] in Globals.KYDROID_SOURCE_LIST.keys()):
                            apk["displayname_cn"] = Globals.KYDROID_SOURCE_LIST[apk["name"]]["name"]
                            apk["version"] = Globals.KYDROID_SOURCE_LIST[apk["name"]]["version"]
                            apk["size"] = Globals.KYDROID_SOURCE_LIST[apk["name"]]["size"]
                            apk["file_path"] = Globals.KYDROID_SOURCE_LIST[apk["name"]]["file"]
                            get_apk_info = ApkInfo(apk["name"], apk["displayname_cn"], apk["version"], apk["size"],apk["file_path"], apk["description"])
                            self.set_apk_information(apk, get_apk_info)
                            if (apk["initialsName"] != None):
                                get_apk_info.initialsName = apk["initialsName"]
                            if (apk["pinyinName"] != None):
                                get_apk_info.pinyinName = apk["pinyinName"]
                            if (apk["anotherName"] != None):
                                get_apk_info.anotherName = apk["anotherName"]
                            get_apk_info.certSubjectOu = apk["certSubjectOu"]
                            get_apk_info.softStyle = apk["softStyle"]
                            get_apk_info.popularity = apk["popularity"]
                            get_apk_info.manufacturers = apk["manufacturers"]
                            get_apk_info.website = apk["website"]
                            in_list = False
                            for pkg in search_apk_list:
                                if (pkg.pkgname == get_apk_info.pkgname):
                                    in_list = True
                                    break
                            if (in_list == False):
                                in_appmgr_apk = False
                                for get_apk in self.apk_list:
                                    if (get_apk.pkgname == get_apk_info.pkgname):
                                        in_appmgr_apk = True
                                        search_apk_list.insert(num, get_apk)
                                        break
                                if(in_appmgr_apk == False):
                                    self.apk_list.append(get_apk_info)
                                    search_apk_list.insert(num, get_apk_info)
                                num = num + 1
        return search_list,search_apk_list

    def get_tip_app_info_by_appname(self,appname,all_list = None,apklist = None):
        search_list = []
        search_apk_list = []
        if(all_list == None):
            # if (Globals.SORTING_METHOD == 0):
            all_list = Globals.GET_ALL_APP_LIST
            # else:
            #     all_list = Globals.GET_RATING_APP_LIST
        # if(apklist == None):
        #     if(self.apk_list != []):
        #         apklist = self.apk_list
        num = 0
        for app in all_list:
            if(type(app) == ApkInfo):
                in_list = False
                for apk in Globals.KYDROID_APP_LIST:
                    if(app.pkgname == apk["name"] and app.pkgname in Globals.KYDROID_SOURCE_LIST.keys()):
                        i = apk
                        in_list = True
                        break
                if(in_list == False):
                    continue
            else:
                i = app
            try:
                if(i["name"] != None and appname.lower() in i["name"].lower()):
                    show = True
                    if(show):
                        if(Globals.LAN == "zh_CN"):
                            re = {"show_name": i["displayname_cn"], "english_name": i["name"],"categories":i["categories"]}
                        else:
                            re = {"show_name":i["name"],"english_name":i["name"],"categories":i["categories"]}
                        if(i["name"].lower().startswith(appname.lower())):
                            search_list.insert(num,re)
                            num = num + 1
                        else:
                            search_list.append(re)
                elif(i["displayname"] != None and appname.lower() in i["displayname"].lower()):
                    show = True
                    if (show):
                        if (Globals.LAN == "zh_CN"):
                            re = {"show_name": i["displayname_cn"], "english_name": i["name"],"categories":i["categories"]}
                        else:
                            re = {"show_name": i["name"], "english_name": i["name"],"categories":i["categories"]}
                        if (i["displayname"].lower().startswith(appname.lower())):
                            search_list.insert(num, re)
                            num = num + 1
                        else:
                            search_list.append(re)
                elif(i["displayname_cn"] != None and appname.lower() in i["displayname_cn"].lower()):
                    show = True
                    if (show):
                        if (Globals.LAN == "zh_CN"):
                            re = {"show_name": i["displayname_cn"], "english_name": i["name"],"categories":i["categories"]}
                        else:
                            re = {"show_name": i["name"], "english_name": i["name"],"categories":i["categories"]}
                        if (i["displayname_cn"].lower().startswith(appname.lower())):
                            search_list.insert(num, re)
                            num = num + 1
                        else:
                            search_list.append(re)
                elif((i["initialsName"] != None and appname.lower().replace(" ","") in i["initialsName"].lower().replace(" ","")) or (i["pinyinName"] != None and appname.lower().replace(" ","") in i["pinyinName"].lower().replace(" ","")) or (i["anotherName"] != None and appname.lower().replace(" ","") in i["anotherName"].lower().replace(" ",""))):
                    show = True
                    if (show):
                        if (Globals.LAN == "zh_CN"):
                            re = {"show_name": i["displayname_cn"], "english_name": i["name"],"categories":i["categories"]}
                        else:
                            re = {"show_name": i["name"], "english_name": i["name"],"categories":i["categories"]}
                        if ((i["initialsName"] != None and i["initialsName"].lower().replace(" ","").startswith(appname.lower())) or (i["pinyinName"] != None and i["pinyinName"].lower().replace(" ","").startswith(appname.lower()))or (i["anotherName"] != None and i["anotherName"].lower().replace(" ","").startswith(appname.lower()))):
                            search_list.insert(num, re)
                            num = num + 1
                        else:
                            search_list.append(re)
            except Exception as e:
                Globals.LOG_ERROR.logger.error("get tip app info exception: %s" % str(e))

        if(apklist == None):
            for i in Globals.KYDROID_APP_LIST:
                if (i["name"] in Globals.KYDROID_SOURCE_LIST.keys()):
                    try:
                        if (i["name"].lower().startswith(appname.lower())):
                            if(Globals.LAN == "zh_CN"):
                                re = {"show_name": i["displayname_cn"], "english_name":  i["name"],"categories":i["categories"]}
                            else:
                                re = {"show_name": i["name"], "english_name":  i["name"],"categories":i["categories"]}
                            search_list.insert(num,re)
                            num = num  + 1
                        elif (i["displayname_cn"].lower().startswith(appname.lower())):
                            if(Globals.LAN == "zh_CN"):
                                re = {"show_name": i["displayname_cn"], "english_name": i["name"],"categories":i["categories"]}
                            else:
                                re = {"show_name": i["name"], "english_name": i["name"],"categories":i["categories"]}
                            search_list.insert(num, re)
                            num = num + 1
                        elif (i["pinyinName"].lower().startswith(appname.lower())):
                            if(Globals.LAN == "zh_CN"):
                                re = {"show_name": i["displayname_cn"], "english_name": i["name"],"categories":i["categories"]}
                            else:
                                re = {"show_name": i["name"], "english_name": i["name"],"categories":i["categories"]}
                            search_list.insert(num, re)
                            num = num + 1
                        elif (i["initialsName"].lower().startswith(appname.lower())):
                            re = {"show_name": i["displayname_cn"], "english_name": i["name"],"categories":i["categories"]}
                            search_list.insert(num, re)
                            num = num + 1
                        elif (i["anotherName"].lower().startswith(appname.lower())):
                            if (Globals.LAN == "zh_CN"):
                                re = {"show_name": i["displayname_cn"], "english_name": i["name"],"categories":i["categories"]}
                            else:
                                re = {"show_name": i["name"], "english_name": i["name"],"categories":i["categories"]}
                            search_list.insert(num, re)
                            num = num + 1
                        elif(appname.lower() in i["name"].lower() or appname.lower() in i["displayname_cn"].lower() or appname.lower() in i["initialsName"].lower() or appname.lower() in i["pinyinName"].lower() or appname.lower() in i["anotherName"].lower() ):
                            if (Globals.LAN == "zh_CN"):
                                re = {"show_name": i["displayname_cn"], "english_name": i["name"],"categories":i["categories"]}
                            else:
                                re = {"show_name": i["name"], "english_name": i["name"],"categories":i["categories"]}
                            search_list.append(re)
                    except Exception as e:
                        Globals.LOG_ERROR.logger.error("get tip app info exception: %s" % str(e))
                continue
        else:
            num = 0  # 不是以关键字开始的数量
            for i in apklist:
                try:
                    if(type(i) == ApkInfo):
                        if (i.pkgname in Globals.KYDROID_SOURCE_LIST.keys() or apklist == []):
                            # if(appname.lower() in i.pkgname.lower()):
                            if(i.pkgname.lower().startswith(appname.lower())):
                                if (Globals.LAN == "zh_CN"):
                                    re = {"show_name": i.displayname_cn, "english_name": i.pkgname,"categories":"17"}
                                else:
                                    re = {"show_name": i.pkgname, "english_name": i.pkgname,"categories":"17"}
                                if(re not in search_list):
                                    search_list.insert(len(search_list)-num,re)
                            # elif(appname.lower() in i.displayname_cn.lower()):
                            elif(i.displayname_cn.lower().startswith(appname.lower())):
                                re = {"show_name": i.displayname_cn, "english_name": i.pkgname,"categories":"17"}
                                if (re not in search_list):
                                    search_list.insert(len(search_list)-num,re)
                            elif (i.pinyinName.lower().startswith(appname.lower())):
                                re = {"show_name": i.displayname_cn, "english_name": i.pkgname,"categories":"17"}
                                if (re not in search_list):
                                    search_list.insert(len(search_list) - num, re)
                            elif (i.anotherName.lower().startswith(appname.lower())):
                                re = {"show_name": i.displayname_cn, "english_name": i.pkgname,"categories":"17"}
                                if (re not in search_list):
                                    search_list.insert(len(search_list) - num, re)
                            elif (i.initialsName.lower().startswith(appname.lower())):
                                re = {"show_name": i.displayname_cn, "english_name": i.pkgname,"categories":"17"}
                                if (re not in search_list):
                                    search_list.insert(len(search_list) - num, re)
                            elif(appname.lower() in i.displayname_cn.lower() or appname.lower() in i.pkgname.lower() or appname.lower() in i.pinyinName.lower() or appname.lower() in i.anotherName.lower() or appname.lower() in i.initialsName.lower()):
                                if (Globals.LAN == "zh_CN"):
                                    re = {"show_name": i.displayname_cn, "english_name": i.pkgname,"categories":"17"}
                                else:
                                    re = {"show_name": i.pkgname, "english_name": i.pkgname,"categories":"17"}
                                if(re not in search_list):
                                    search_list.append(re)
                                    num = num + 1
                except Exception as e:
                    Globals.LOG_ERROR.logger.error("get tip app info exception: %s" % str(e))
        return search_list,search_apk_list

    def get_one_app_info_by_appname(self,appname,need_check_cache = True):
        try:
            pkg = self.apt_cache[appname]
        except:
            pkg = None
        all_list = []
        # if (Globals.SORTING_METHOD == 0):
        all_list = Globals.GET_ALL_APP_LIST
        # else:
        #     all_list = Globals.GET_RATING_APP_LIST
        for i in all_list:
            if(pkg != None and (appname == i["name"] or appname == i["displayname"] or appname == i["displayname_cn"])):
                return i
        for i in Globals.KYDROID_APP_LIST:
            if (appname == i["name"]):
                return i
        for i in Globals.GET_ALL_DRIVE_APP_LIST:
            if (appname == i["name"]):
                return i


    def get_history_app_info_by_appname(self, appname):
        # try:
        #     pkg = self.apt_cache[appname]
        # except:
        #     pkg = None
        all_list = []
        # if (Globals.SORTING_METHOD == 0):
        all_list = Globals.GET_ALL_APP_LIST
        # else:
        #     all_list = Globals.GET_RATING_APP_LIST
        for i in all_list:
            # if (pkg != None and ((appname == i["name"]) or (appname == i["displayname"]) or (appname == i["displayname_cn"]))):
            if (((appname == i["name"]) or (appname == i["displayname"]) or (appname == i["displayname_cn"]))):
                try:
                    pkg = self.apt_cache[appname]
                except:
                    pkg = None
                if(pkg != None):
                    return i
        for j in self.apk_list:
            if (appname == j.pkgname and Globals.NEED_SHOW_KMRE == False):
                return j
            # elif ("17" in i["categories"].split(",") and ((appname == i["name"]) or (appname == i["displayname"]) or (
        if(Globals.NEED_SHOW_KMRE == False):
            for i in Globals.KYDROID_APP_LIST:
                if (i["softStyle"] == 3 and ((appname == i["name"]) or (appname == i["displayname"]) or ( appname == i["displayname_cn"]))):
                    #如果在Globals.KYDROID_SOURCE_LIST这个字典中则显示，否则不显示,版本、大小、下载路径、中文名、英文名，都需要保存
                    in_apk_list = False
                    for get_apk in self.apk_list:
                        if(i["name"] == get_apk.pkgname):
                            in_apk_list = True
                            return get_apk
                    if(in_apk_list == False):
                        if(i["name"] in Globals.KYDROID_SOURCE_LIST.keys()):
                            i["displayname_cn"] = Globals.KYDROID_SOURCE_LIST[i["name"]]["name"]
                            i["version"] = Globals.KYDROID_SOURCE_LIST[i["name"]]["version"]
                            i["size"] = Globals.KYDROID_SOURCE_LIST[i["name"]]["size"]
                            i["file_path"] = Globals.KYDROID_SOURCE_LIST[i["name"]]["file"]
                            return i
        for i in Globals.GET_ALL_DRIVE_APP_LIST:
            # if (pkg != None and ((appname == i["name"]) or (appname == i["displayname"]) or (appname == i["displayname_cn"]))):
            if (((appname == i["name"]) or (appname == i["displayname"]) or (appname == i["displayname_cn"]))):
                try:
                    pkg = self.apt_cache[appname]
                except:
                    pkg = None
                if(pkg != None):
                    return i

    def check_cup_model(self,appdict):
        check_cpu = Globals.CPU
        show_app = True
        hide_app = False
        app_dict = self.set_all_app_list(appdict)
        if ("100" in app_dict["categories"]):
            for category in Globals.ALL_CATEGORIES:
                for app_category in app_dict["categories"].split(","):
                    if (str(category["id"]) == app_category):
                        # if (category not in Globals.ORDINARY_CATEGORY):
                        #     Globals.ORDINARY_CATEGORY.append(category)
                        pass
            Globals.GET_ALL_APP_LIST.append(app_dict)
            return None
        if (app_dict["name"] not in Globals.BLACKLIST):
            if (app_dict["name"] != Globals.APPNAME and app_dict["name"] != Globals.KUM):
                if (app_dict["name"] == Globals.KYDROID_PACKAGE_NAME):
                    kernel_version = os.popen("uname -r").readline().strip("\n")
                    if (len(kernel_version.split("-")) >= 2 and len(kernel_version.split("-")[1]) > 2):
                        kernel_time = kernel_version.split("-")[1].split(".")[0]
                        if (len(kernel_time) == 8):
                            if (int(kernel_time) < 20200710):
                                return None
                if (app_dict["cpu_model"] != None and app_dict["cpu_model"] != ""):
                    arch_in = False
                    # num = 0
                    for cpu in app_dict["cpu_model"].split(";"):
                        if (Globals.ARCH.lower() == cpu.split(":")[0].lower() or cpu.split(":")[0].lower() == "all"):
                            arch_in = True
                            # if (num != 0):
                            #     app_dict["cpu_model"] = app_dict["cpu_model"] + "," + cpu.split(":")[1].lower()
                            # else:
                            app_dict["cpu_model"] = ""
                            for i in cpu.split(":")[1].lower().split(","):
                                if (i == "huawei_qs"):
                                    if (Globals.CPU.lower() == "kunpeng920"):
                                        if (app_dict["cpu_model"] == ""):
                                            app_dict["cpu_model"] = "kunpeng920"
                                        else:
                                            app_dict["cpu_model"] = app_dict["cpu_model"] + "," + "kunpeng920"
                                    else:
                                        if (app_dict["cpu_model"] == ""):
                                            app_dict["cpu_model"] = i
                                        else:
                                            app_dict["cpu_model"] = app_dict["cpu_model"] + "," + i
                                else:
                                    if (app_dict["cpu_model"] == ""):
                                        app_dict["cpu_model"] = i
                                    else:
                                        app_dict["cpu_model"] = app_dict["cpu_model"] + "," + i
                            # if(cpu.split(":")[1].lower() == "huawei_qs".lower()):
                            #     if(Globals.CPU.lower() == "kunpeng920"):
                            #         app_dict["cpu_model"] = "kunpeng920"
                            #     else:
                            #         app_dict["cpu_model"] = cpu.split(":")[1].lower()
                            # else:
                            #     app_dict["cpu_model"] = cpu.split(":")[1].lower()
                            # num = num + 1
                    if (arch_in == False):
                        show_app = False
                        app_dict["cpu_model"] = None
                if (app_dict["no_cpu_model"] != None and app_dict["no_cpu_model"] != ""):
                    arch_in = False
                    # num = 0
                    for cpu in app_dict["no_cpu_model"].split(";"):
                        if (Globals.ARCH.lower() == cpu.split(":")[0].lower() or cpu.split(":")[0].lower() == "all"):
                            arch_in = True
                            app_dict["no_cpu_model"] = ""
                            for i in cpu.split(":")[1].lower().split(","):
                                if (i == "huawei_qs"):
                                    if (Globals.CPU.lower() == "kunpeng920"):
                                        if (app_dict["no_cpu_model"] == ""):
                                            app_dict["no_cpu_model"] = "kunpeng920"
                                        else:
                                            app_dict["no_cpu_model"] = app_dict["no_cpu_model"] + "," + "kunpeng920"
                                    else:
                                        if (app_dict["no_cpu_model"] == ""):
                                            app_dict["no_cpu_model"] = i
                                        else:
                                            app_dict["no_cpu_model"] = app_dict["no_cpu_model"] + "," + i
                                else:
                                    if (app_dict["no_cpu_model"] == ""):
                                        app_dict["no_cpu_model"] = i
                                    else:
                                        app_dict["no_cpu_model"] = app_dict["no_cpu_model"] + "," + i
                            # if (cpu.split(":")[1].lower() == "huawei_qs".lower()):
                            #     if (Globals.CPU.lower() == "kunpeng920"):
                            #         app_dict["no_cpu_model"] = "kunpeng920"
                            #     else:
                            #         app_dict["no_cpu_model"] = cpu.split(":")[1].lower()
                            # else:
                            #     app_dict["no_cpu_model"] = cpu.split(":")[1].lower()
                            # num = num + 1

                    if (arch_in == True):
                        hide_app = True
                    else:
                        app_dict["no_cpu_model"] = None
                # if((app_dict["cpu_model"] == None or Globals.CPU.lower() in app_dict["cpu_model"].lower().split(",")) and (app_dict["no_cpu_model"] == None or Globals.CPU.lower() not in app_dict["no_cpu_model"].lower().split(",")) and (app_dict["arch"] == None or Globals.ARCH.lower() in app_dict["arch"].lower().split(",")) and (app_dict["system_name"] == None or Globals.SYSTEM_VERSION.lower().replace(" ","").replace("-","") in app_dict["system_name"].lower().replace(" ","").replace("-","").split(","))):
                if ((show_app == True and (app_dict["cpu_model"] == None or check_cpu.lower() in app_dict["cpu_model"].lower().split(","))) and (hide_app == False or (hide_app == True and (app_dict["no_cpu_model"] == None or check_cpu.lower() not in app_dict[
                    "no_cpu_model"].lower().split(","))))):
                    # if (str(app_dict["softStyle"]) == "2" and app_dict not in Globals.GET_ALL_DRIVE_APP_LIST):
                    #     Globals.GET_ALL_DRIVE_APP_LIST.append(app_dict)
                    # elif (str(app_dict["softStyle"]) == "3"  and app_dict not in Globals.KYDROID_APP_LIST):
                    #     Globals.KYDROID_APP_LIST.append(app_dict)
                    # else:
                    #     ###########
                    #     # if(app_dict["name"] == "kmre"):
                    #     #     pass
                    #     # else:
                    #     for category in Globals.ALL_CATEGORIES:
                    #         for app_category in app_dict["categories"].split(","):
                    #             if (str(category["id"]) == app_category):
                    #                 if (category not in Globals.ORDINARY_CATEGORY):
                    #                     Globals.ORDINARY_CATEGORY.append(category)
                    #     if(app_dict not in Globals.GET_ALL_APP_LIST):
                    #         Globals.GET_ALL_APP_LIST.append(app_dict)
                    return app_dict
            else:
                if (app_dict["name"] == Globals.APPNAME):
                    Globals.UKSC_APPMSG = app_dict


    def get_ad_app_info(self,appname):
        # for i in all_list:
        #     for j in self.apk_list:
        #         if (appname == j.pkgname):
        #             return j
        #     if ((appname == i["name"]) or (appname == i["displayname"]) or (appname == i["displayname_cn"])):
        #         return i

        pkg_info = self.db.query_one_applications(appname)
        if(pkg_info == []):
            return None
        else:
            app_dict = self.check_cup_model(pkg_info[0])
            return app_dict

    #
    #函数：根据id获取软件分类名
    #
    def get_category_id_by_name(self,category_name):
        for i in Globals.ALL_CATEGORIES:
            if(category_name == i["name"] or category_name == i["displayname"]):
                return i["id"]

    #
    #函数：通过软件名获取分类id
    #
    def get_category_name_by_id(self,id):
        for i in Globals.ALL_CATEGORIES:
            if (id == i["id"]):
                return i["name"],i["displayname"]


    #
    #函数：通过分类获取软件信息
    #
    def get_app_info_by_categories(self,category_name):
        sorting_mathod = Globals.SORTING_METHOD
        app_list = []
        ad_list = []
        category_id = self.get_category_id_by_name(category_name)
        if(str(category_id) != "100"):
            for app in Globals.AD_LIST:
                pkg = None
                try:
                    pkg = self.apt_cache[app[0]]
                except:
                    continue
                if(str(category_id) == str(app[2])):
                    ad_list.append(app)
        all_list = []
        if(category_name == "recommend"):
            all_list = Globals.GET_ALL_APP_LIST
        else:
            if (Globals.SORTING_METHOD == 0):
                all_list = Globals.GET_ALL_APP_LIST
            elif(Globals.SORTING_METHOD == 1):
                all_list = Globals.GET_RATING_APP_LIST
            else:
                all_list = Globals.GET_POPULARITY_APP_LIST
        for i in all_list:
            # page = {"pagetype": PageStates.CAREGORYPAGE, "Type": self.category_widget.type_name_label.text(),
            #         "sort_way": Globals.SORTING_METHOD}
            # if (len(Globals.RETURN_PATH_LIST) >= 10):
            #     Globals.RETURN_PATH_LIST.pop(0)
            # Globals.RETURN_PATH_LIST.append(page)
            if(category_name == "recommend" or (Globals.NOWPAGE == PageStates.CAREGORYPAGE and category_name == Globals.NOW_CATEGORY and Globals.SORTING_METHOD == sorting_mathod)):
                app_category_list = []
                try:
                    app_category_list = i["categories"].split(",")
                    if(str(category_id) in app_category_list):
                        if(str(category_id) != "100"):
                            try:
                                self.apt_cache[i["name"]]
                                app_list.append(i)
                            except:
                                pass
                        else:
                            app_list.append(i)

                except:
                    pass
            else:
                return None,None

        return app_list, ad_list

    #
    # 函数：通过我们常玩
    #
    def get_recommend_by_categories(self, category_name):
        sorting_mathod = Globals.SORTING_METHOD
        app_list = []
        ad_list = []
        category_id = self.get_category_id_by_name(category_name)
        if (str(category_id) != "100"):
            for app in Globals.AD_LIST:
                pkg = None
                try:
                    pkg = self.apt_cache[app[0]]
                except:
                    continue
                if (str(category_id) == str(app[2])):
                    ad_list.append(app)
        pkg_info = self.db.query_get_recommend_applications(category_id)
        if (pkg_info == []):
            return None,None
        else:
            for i in pkg_info:
                app_dict = self.check_cup_model(i)
                # page = {"pagetype": PageStates.CAREGORYPAGE, "Type": self.category_widget.type_name_label.text(),
                #         "sort_way": Globals.SORTING_METHOD}
                # if (len(Globals.RETURN_PATH_LIST) >= 10):
                #     Globals.RETURN_PATH_LIST.pop(0)
                # Globals.RETURN_PATH_LIST.append(page)
                # if (category_name == "recommend" or (Globals.NOWPAGE == PageStates.CAREGORYPAGE and category_name == Globals.NOW_CATEGORY and Globals.SORTING_METHOD == sorting_mathod)):
                #     app_category_list = []
                try:
                    app_category_list = app_dict["categories"].split(",")
                    if (str(category_id) in app_category_list):
                        if (str(category_id) != "100"):
                            try:
                                self.apt_cache[app_dict["name"]]
                                app_list.append(app_dict)
                            except:
                                pass
                        else:
                            app_list.append(app_dict)

                except:
                    pass

        return app_list, ad_list

    #
    #函数：获取安卓分类
    #
    def get_android_category(self,category):
        apk_list = []
        search_list = []
        if(Globals.KYDROID_SORTING_METHOD == 0):
            search_list.extend(self.apk_list)
        elif(Globals.KYDROID_SORTING_METHOD == 1):
            search_list.extend(self.ratting_apk_list)
        else:
            search_list.extend(self.popularity_apklist)
        if(category == "全部" or category == "all"):
            return search_list
        for apk in search_list:
            if(category in apk.category_name.split("、")):
                inlist = False
                for i in apk_list:
                    if(apk.pkgname == i.pkgname):
                        inlist = True
                if(inlist == False):
                    apk_list.append(apk)
            elif(category in apk.category_name_cn.split("、")):
                inlist = False
                for i in apk_list:
                    if (apk.pkgname == i.pkgname):
                        inlist = True
                if (inlist == False):
                    apk_list.append(apk)
        return apk_list

    #
    #函数：根据分类获取驱动列表
    #
    def get_drive_list_by_category(self,download_rank = True):
        applist = []
        if(download_rank == True):
            # applist = self.db.get_drive_info_by_download()
            applist = Globals.GET_ALL_DRIVE_APP_LIST
        else:
            applist = Globals.GET_RATING_DRIVE_APP_LIST
        category_list = []
        for dict in applist:
            # dict = self.get_drive_info_dict(i)
            for i in dict["categories"].split(","):
                in_list = True
                for category_dict in Globals.DRIVE_CATEGORIES:
                    if(str(i) == str(category_dict["id"])):
                        if(download_rank == True):
                            for category_name in Globals.DRIVE_CATEGORIES_APP_LIST:
                                if(category_name == category_dict["name"] or category_name == category_dict["displayname"]):
                                    try:
                                        self.apt_cache[dict["name"]]
                                        in_list = False
                                        for app_dict in Globals.DRIVE_CATEGORIES_APP_LIST[category_name]:
                                            if(dict["name"] == app_dict["name"]):
                                                in_list = True
                                                break
                                        if(in_list == False):
                                            Globals.DRIVE_CATEGORIES_APP_LIST[category_name].append(dict)
                                        if (category_dict not in category_list):
                                            category_list.append(category_dict)
                                    except:
                                        pass
                                    break
                            break
                        else:
                            for category_name in Globals.DRIVE_CATEGORIES_APP_LIST_BY_RATING:
                                if (category_name == category_dict["name"] or category_name == category_dict[
                                    "displayname"]):
                                    try:
                                        self.apt_cache[dict["name"]]
                                        in_list = False
                                        for app_dict in Globals.DRIVE_CATEGORIES_APP_LIST[category_name]:
                                            if (dict["name"] == app_dict["name"]):
                                                in_list = True
                                                break
                                        if(in_list == False):
                                            Globals.DRIVE_CATEGORIES_APP_LIST_BY_RATING[category_name].append(dict)

                                        if (category_dict not in category_list):
                                            category_list.append(category_dict)
                                    except:
                                        pass
                                    break
                            break
        end_category = []
        for i in Globals.DRIVE_CATEGORIES:
            if(i in category_list):
                end_category.append(i)
        Globals.DRIVE_CATEGORIES.clear()
        Globals.DRIVE_CATEGORIES.extend(end_category)


    #
    #函数：获取驱动包字段
    #
    def get_drive_info_dict(self,app):
        app_dict = {"id": 0, "name": "", "displayname": "", "displayname_cn": "", "categories": "", "summary": "",
                    "description": "", "rating_avg": 0, "rating_total": 0, "review_total": 0, "download_total": 0,
                    "icon": "", "cover_image": "", "preview_image": "", "arch": "", "system_name": "", "depends": "",
                    "cpu_model": "", "no_cpu_model": "", "from_db": True, "initialsName": '', "pinyinName": "",
                    "anotherName": "", "certSubjectOu": "", "softStyle": 2,"popularity":None,"website":None,"manufacturers":"","package_version":"","package_size":"","arch":"","system_version":""}
        app_dict["id"] = app[0]
        app_dict["name"] = app[1]
        app_dict["displayname"] = app[2]
        app_dict["displayname_cn"] = app[3]
        app_dict["categories"] = app[4]
        app_dict["summary"] = app[5]
        app_dict["description"] = app[6]
        app_dict["rating_avg"] = app[7]
        app_dict["rating_total"] = app[8]
        app_dict["review_total"] = app[9]
        app_dict["download_total"] = app[10]
        app_dict["icon"] = app[11]
        app_dict["cover_image"] = app[12]
        app_dict["preview_image"] = app[13]
        app_dict["arch"] = app[14]
        app_dict["system_name"] = app[15]
        app_dict["depends"] = app[16]
        app_dict["cpu_model"] = app[17]
        app_dict["no_cpu_model"] = app[18]
        app_dict["certSubjectOu"] = app[19]
        app_dict["softStyle"] = app[20]
        app_dict["manufacturers"] = app[21]
        app_dict["package_version"] = app[22]
        app_dict["package_size"] = app[23]
        app_dict["arch"] = app[24]
        app_dict["system_version"] = app[25]
        # app_dict["popularity"] = app[26]
        if (app[26] != None and app[26] != ""):
            app_dict["popularity"] = int(float(app[26]))
        else:
            app_dict["popularity"] = 0
        app_dict["website"] = app[27]
        app_dict["from_db"] = True
        # print(app_dict["cpu_model"],app_dict["no_cpu_model"])
        return app_dict


    #
    #函数：获取软件更新
    #
    def get_need_update_app(self,is_upgrade = False):
        applist = []
        itemlist = []
        apklist = []
        upgrade_list = []
        apk_upgrade_list = []
        # itemlist = self.db.query_applications()
        all_list = []
        if (Globals.SORTING_METHOD == 0):
            all_list = Globals.GET_ALL_APP_LIST
        else:
            all_list = Globals.GET_RATING_APP_LIST
        for item in all_list:
            pkgname = item["name"]
            package = self.get_package_by_name(pkgname)
            if package is None:
                continue
            if package.is_installed:
                if (package.name != Globals.APPNAME and item not in applist):
                    applist.append(item)
            if package.is_upgradable and item not in upgrade_list:
                upgrade_list.append(item)
        for i in Globals.GET_ALL_DRIVE_APP_LIST:
            pkgname = i["name"]
            package = self.get_package_by_name(pkgname)
            if package is None:
                continue

            if package.is_installed:
                if (package.name != Globals.APPNAME and i not in applist):
                    applist.append(i)
            if package.is_upgradable and i not in upgrade_list:
                upgrade_list.append(i)
        #*********处理已安装安卓*************
        # if(self.apk_list == []):
        if(self.kydroid.hasKydroid == True and Globals.NEED_SHOW_KMRE == False):
            for installed_apk in Globals.INSTALLED_KYDROID_APK:
                in_apk_list = False
                for apk in self.apk_list:
                    if(installed_apk["appname"] == apk.pkgname):
                        in_apk_list = True
                        if(apk not in apklist):
                            apklist.append(apk)
                        break
                if(in_apk_list == False):
                    if(installed_apk["appname"] in Globals.KYDROID_SOURCE_LIST.keys()):
                        get_apk = ApkInfo(installed_apk["appname"],installed_apk["appname"],installed_apk["version"],installed_apk["size"],"","")
                        for i in Globals.KYDROID_APP_LIST:
                            if(get_apk.pkgname == i["name"]):
                                if (i["initialsName"] != None):
                                    get_apk.initialsName = i["initialsName"]
                                if (i["pinyinName"] != None):
                                    get_apk.pinyinName = i["pinyinName"]
                                if (i["anotherName"] != None):
                                    get_apk.anotherName = i["anotherName"]
                                self.set_apk_information(i,get_apk)
                                if(get_apk not in apklist):
                                    apklist.append(get_apk)
                                break
                else:
                    for apk in self.apk_list:
                        if (type(apk) == ApkInfo and installed_apk["appname"] == apk.pkgname):
                            for i in Globals.KYDROID_APP_LIST:
                                if (apk.pkgname == i["name"]):
                                    if (i["initialsName"] != None):
                                        apk.initialsName = i["initialsName"]
                                    if (i["pinyinName"] != None):
                                        apk.pinyinName = i["pinyinName"]
                                    if (i["anotherName"] != None):
                                        apk.anotherName = i["anotherName"]
                                    self.set_apk_information(i, apk)
                                    break
                            if apk.is_installed and apk not in apklist:
                                apklist.append(apk)
                            if apk.is_upgradable and apk not in apk_upgrade_list:
                                apk_upgrade_list.append(apk)
                            break
            # if(len(Globals.INSTALLED_KYDROID_APK) == 0):
            for apk in self.apk_list:
                if (type(apk) == ApkInfo):
                    for i in Globals.KYDROID_APP_LIST:
                        if (apk.pkgname == i["name"]):
                            if (i["initialsName"] != None):
                                apk.initialsName = i["initialsName"]
                            if (i["pinyinName"] != None):
                                apk.pinyinName = i["pinyinName"]
                            if (i["anotherName"] != None):
                                apk.anotherName = i["anotherName"]
                            self.set_apk_information(i, apk)
                            break
                    if apk.is_installed and apk not in apklist:
                        apklist.append(apk)
                    if apk.is_upgradable and apk not in apk_upgrade_list:
                        apk_upgrade_list.append(apk)
        if (is_upgrade):
            return upgrade_list, apk_upgrade_list
        else:
            return applist, apklist

    #
    #函数：获取历史安装
    #
    def get_history_install_app(self,history_type):
        other_list = []
        local_list = []
        if(Globals.LOGIN_SUCCESS and history_type != "local"):
            #获取登录后的安装记录，调用服务器接口（返回应用ID、应用名称、分类ID、宣传语、应用简介、平均评分、下载次数、图标地址、封面图地址、预览图地址）
            res = self.slot_get_record_from_server()
            if(res != None):
                while(len(res) != 0):
                    i = res.pop(len(res)-1)
                    # app_msg = self.get_one_app_info_by_appname(i["packageName"])
                    app_msg = self.get_history_app_info_by_appname(i["packageName"])
                    if(app_msg != None):
                        if(type(app_msg) == dict):
                            # if ("17" in app_msg["categories"].split(",") and self.apk_list == []):
                            if (app_msg["softStyle"] == 3 and self.apk_list == []):
                                apk = ApkInfo(app_msg["name"],app_msg["displayname_cn"],app_msg["version"],app_msg["size"],app_msg["file_path"],app_msg["description"])
                                self.set_apk_information(app_msg,apk)
                                if (app_msg["initialsName"] != None):
                                    apk.initialsName = app_msg["initialsName"]
                                if (app_msg["pinyinName"] != None):
                                    apk.pinyinName = app_msg["pinyinName"]
                                if (app_msg["anotherName"] != None):
                                    apk.anotherName = app_msg["anotherName"]
                                other_list.append(apk)
                            elif (app_msg["softStyle"] == 3 and self.apk_list != []):
                            # elif ("17" in app_msg["categories"].split(",") and self.apk_list != []):
                                in_apk_list = False
                                for get_apk in self.apk_list:
                                    if (app_msg["name"] == get_apk.pkgname):
                                        in_apk_list = True
                                        other_list.append(apk)
                                        break
                                if (in_apk_list == False):
                                    apk = ApkInfo(app_msg["name"], app_msg["displayname_cn"], app_msg["version"],
                                                  app_msg["size"], app_msg["file_path"], app_msg["description"])
                                    self.set_apk_information(app_msg, apk)
                                    if (app_msg["initialsName"] != None):
                                        apk.initialsName = app_msg["initialsName"]
                                    if (app_msg["pinyinName"] != None):
                                        apk.pinyinName = app_msg["pinyinName"]
                                    if (app_msg["anotherName"] != None):
                                        apk.anotherName = app_msg["anotherName"]
                                    self.apk_list.append(apk)
                                    other_list.append(apk)
                            else:
                                other_list.append(app_msg)
                        else:
                            if (type(app_msg) == ApkInfo and self.apk_list == []):
                                pass
                            else:
                                other_list.append(app_msg)
            pass
        installed_db = Record_History_Installation()
        installed_db.select()
        res = installed_db.cursor.fetchall()
        if(res != None and res != []):
            if(res[0] != None):
                while(len(res) != 0):
                    i = res.pop(len(res)-1)
                    # app_msg = self.get_one_app_info_by_appname(i[0])
                    app_msg = self.get_history_app_info_by_appname(i[0])
                    if(app_msg not in local_list and app_msg != None):
                        if (type(app_msg) == dict):
                            # if("17" in app_msg["categories"].split(",") and self.apk_list == []):
                            if(app_msg["softStyle"] == 3 and self.apk_list == []):
                                apk = ApkInfo(app_msg["name"],app_msg["displayname_cn"],app_msg["version"],app_msg["size"],app_msg["file_path"],app_msg["description"])
                                self.set_apk_information(app_msg,apk)
                                if (app_msg["initialsName"] != None):
                                    apk.initialsName = app_msg["initialsName"]
                                if (app_msg["pinyinName"] != None):
                                    apk.pinyinName = app_msg["pinyinName"]
                                if (app_msg["anotherName"] != None):
                                    apk.anotherName = app_msg["anotherName"]
                                self.apk_list.append(apk)
                                local_list.append(apk)
                            # elif ("17" in app_msg["categories"].split(",") and self.apk_list != []):
                            elif (app_msg["softStyle"] == 3 and self.apk_list != []):
                                in_apk_list = False
                                for get_apk in self.apk_list:
                                    if(app_msg["name"] == get_apk.pkgname):
                                        in_apk_list = True
                                        local_list.append(apk)
                                        break
                                if(in_apk_list == False):
                                    apk = ApkInfo(app_msg["name"], app_msg["displayname_cn"], app_msg["version"],
                                                  app_msg["size"], app_msg["file_path"], app_msg["description"])
                                    self.set_apk_information(app_msg, apk)
                                    if (app_msg["initialsName"] != None):
                                        apk.initialsName = app_msg["initialsName"]
                                    if (app_msg["pinyinName"] != None):
                                        apk.pinyinName = app_msg["pinyinName"]
                                    if (app_msg["anotherName"] != None):
                                        apk.anotherName = app_msg["anotherName"]
                                    self.apk_list.append(apk)
                                    local_list.append(apk)
                            else:
                                local_list.append(app_msg)
                        else:
                            if (type(app_msg) == ApkInfo and self.apk_list == []):
                                pass
                            else:
                                local_list.append(app_msg)
        installed_db.connect.commit()
        installed_db.cursor.close()
        installed_db.connect.close()
        return local_list,other_list

    #
    #
    #
    def set_apk_information(self,app_dict,apk):
        apk.displayname = app_dict["displayname_cn"]
        apk.app_id = app_dict["id"]
        apk.summary = app_dict["summary"]
        apk.description = app_dict["description"]
        apk.ratings_average = app_dict["rating_avg"]
        apk.ratings_total = app_dict["rating_total"]
        apk.review_total = app_dict["review_total"]
        apk.downloadcount = app_dict["download_total"]
        apk.category_name = app_dict["categories"]
        apk.icon_path = app_dict["icon"]
        apk.down_cover_image_path = app_dict["cover_image"]
        apk.down_preview_image_path = app_dict["preview_image"]
        apk.arch = app_dict["arch"]
        apk.system_name = app_dict["system_name"]
        apk.depends = app_dict["depends"]
        apk.certSubjectOu = app_dict["certSubjectOu"]
        apk.softStyle = app_dict["softStyle"]
        apk.popularity = app_dict["popularity"]
        apk.manufacturers = app_dict["manufacturers"]
        apk.website = app_dict["website"]
        apk.from_ukscdb = True
        category_list = apk.category_name.split(",")
        category = category_cn = ""
        try:
            num = 0
            for i in category_list:
                # if(not i.isdigit()):
                #     continue
                if (int(i) != 1 and int(i) != 2 and int(i) != 15 and int(i) != 18 and int(i) != 16 and int(i) != 17):
                    get_category, get_category_cn = self.get_category_name_by_id(int(i))
                    if (num == 0):
                        if (i not in Globals.KYDROID_CATEGORY):
                            Globals.KYDROID_CATEGORY.append(i)
                        category = get_category
                        category_cn = get_category_cn
                        num = num + 1
                    else:
                        if (i not in Globals.KYDROID_CATEGORY):
                            Globals.KYDROID_CATEGORY.append(i)
                        category = category + "、" + get_category
                        category_cn = category_cn + "、" + get_category_cn
                        num = num + 1
                    # break
                elif (int(i) == 17):
                    pass
                    # if (num == 0):
                    #     category = "kydroid"
                    #     category_cn = "安卓软件"
                    # else:
                    #     category = category + "、kydroid"
                    #     category_cn = category_cn + "、安卓软件"
                else:
                    pass
                    # num = num + 1
                if (num > 3):
                    break
        except:
            category = category_cn = ""
        apk.category_name = category
        apk.category_name_cn = category_cn
        for i in Globals.INSTALLED_KYDROID_APK:
            if(apk.pkgname == i["appname"]):
                apk.is_installed = True

    #
    #函数：处理从服务器获取安装记录的返回结果
    #
    def slot_get_record_from_server(self):
        #与Globals.ALL_ALL_LIST中的信息对比
        return None

    #
    # 函数：获取软件数量
    #
    def get_application_count(self, cat_name=""):
        sum_inst = 0
        sum_up = 0
        # sum_all = len(self.apt_cache)
        sum_all = len(self.apt_cache)
        sum_apk = len(self.apk_list)

        if len(cat_name) > 0:
            cat = self.cat_list[cat_name]
            (sum_inst, sum_up, sum_all) = cat.get_application_count()
        else:
            applist = self.db.query_applications()
            for item in applist:
                pkgname = item[1]
                package = self.get_package_by_name(pkgname)
                if package is None:
                    continue

                if package.is_installed:
                    sum_inst = sum_inst + 1
                if package.is_upgradable:
                    sum_up = sum_up + 1
        for apk in self.apk_list:
            if apk.is_installed:
                sum_inst = sum_inst + 1
            if apk.is_upgradable:
                sum_up = sum_up + 1
        return (sum_inst, sum_up, sum_all, sum_apk)

    def get_package_by_name(self,pkgname):
        if not pkgname:
            return None
        if self.apt_cache is None:
            return None
        package = None
        try:
            package = self.apt_cache[pkgname]
        except:
            package = None
        if package is not None:
            try:
                if package.candidate is None:
                    package = None
            except:
                package = None
        return package

    def get_cache_apt_pkg(self,appname,is_get_tips,applist,is_remove = False):
        appname_list = []
        for name in applist:
            try:
                appname_list.append(name["english_name"])
            except:
                try:
                    appname_list.append(name["name"])
                except:
                    appname_list.append(name)

        if ("$" in appname or "^" in appname):
            return []
        else:
            # cmd = "apt-cache show '*"+appname+"*' |grep Package: |awk '{print $2}'"
            cmd = "apt-cache search "+appname+" |awk  {' print $1 '} |grep "+appname
            lines = os.popen(cmd).readlines()
            pkg_list = []
            tip_list = []
            find_num = 0
            for i in lines:
                app_name = i.strip("\n")
                if(app_name.startswith(appname)):
                    lines.remove(i)
                    lines.insert(find_num,app_name)
                    find_num = find_num + 1
            num = 0
            for i in lines:
                app = self.get_package_by_name(i.strip("\n"))
                if(app != None):
                    if(app.name not in Globals.BLACKLIST):
                        if(app.name not in appname_list):
                            if(app.name != Globals.APPNAME and app.name != Globals.OLDNAME and app.name != Globals.KUM):
                                if(is_remove):
                                    if(app.is_installed):
                                        pkg_list.append(app)
                                else:
                                    pkg_list.append(app)
                                if(num <= 6):
                                    if (is_remove):
                                        if (app.is_installed):
                                            tip_list.append(app)
                                            num = num + 1
                                    else:
                                        tip_list.append(app)
                                        num = num + 1
                                elif(num == 7):
                                    self.get_search_tip.emit(tip_list,False)
                                    if(is_get_tips == True):
                                        return tip_list
                                    num = num +1
            if(len(tip_list) < 6):
                self.get_search_tip.emit(tip_list,False)
            return pkg_list


    #
    #函数：检测是否显示kmre推送
    #
    def detect_whether_to_display_kmre(self):
        need_show_kmre = False
        try:
            kmre = self.apt_cache[Globals.KYDROID_PACKAGE_NAME]
        except:
            kmre = None
        # if(self.kydroid.hasKydroid == True):
        #     need_show_kmre = False
        # else:
        # num = 0
        # for i in Globals.GET_ALL_APP_LIST:
        #     if (i["name"] == Globals.KYDROID_PACKAGE_NAME):
        #         break
        #     num = num + 1
        # if (num > len(Globals.GET_ALL_APP_LIST) - 1):
        #     return False
        if (kmre != None):
            if (kmre.is_installed == False):
                need_show_kmre = True
            else:
                conf = configparser.ConfigParser()
                conf.read(CONF_PATH)
                startup_time = os.popen(
                    "date -d \"$(awk -F. '{print $1}' /proc/uptime) second ago\" +\"%Y-%m-%d %H:%M:%S\"").readline().strip(
                    "\n")
                install_kmre = ""
                try:
                    install_kmre = conf.get("essential-information", "install_kmre")
                except:
                    pass
                if (install_kmre != ""):
                    format_pattern = "%Y-%m-%d %H:%M:%S"
                    difference = (datetime.datetime.strptime(startup_time, format_pattern) - datetime.datetime.strptime(
                        install_kmre, format_pattern))
                    if (difference.days < 0):
                        need_show_kmre = True
        else:
            if(self.kydroid.hasKydroid):
                need_show_kmre = False
        return need_show_kmre

    #
    #函数：获取广告
    #
    def get_ad_from_db(self):
        dblist = []
        applist = []
        dblist = self.db.query_ad_apps()
        ret_dblist = []
        need_show_kmre = self.detect_whether_to_display_kmre()
        Globals.NEED_SHOW_KMRE = need_show_kmre
        if(need_show_kmre == True):
            in_list = False
            for i in dblist:
                if(i[0] == Globals.KYDROID_PACKAGE_NAME):
                    in_list = True
            if(in_list == False):
                photo_path = UBUNTUKYLIN_DATA_PATH + "ads/kmre-burner_big.png"
                kmre_ad = (Globals.KYDROID_PACKAGE_NAME, photo_path, '0')
                dblist.append(kmre_ad)
        ret_dblist.extend(dblist)
        for i in ret_dblist:
            if(str(i[2]) == '0'):
                if(need_show_kmre == True):
                    path_list = i[1].split("/")
                    if ("_big" in path_list[len(path_list) - 1]):
                        if (i[0] != Globals.KYDROID_PACKAGE_NAME):
                            # dblist.remove(i)
                            app = self.get_ad_app_info(i[0])
                            applist.append(app)
                            Globals.AD_LIST.append(i)
                            continue
                        else:
                            app = self.get_ad_app_info(i[0])
                            # if(app != None):
                            dblist.remove(i)
                            dblist.insert(0,i)
                            applist.insert(0,app)
                            Globals.AD_LIST.insert(0,i)
                            continue
                app = self.get_ad_app_info(i[0])
                # if(app != None):
                applist.append(app)
                Globals.AD_LIST.append(i)
                # else:
                #     dblist
            else:
                app = self.get_ad_app_info(i[0])
                # if(app != None):
                applist.append(app)
                Globals.AD_LIST.append(i)
        self.get_ad_ready.emit(dblist,applist)

    #
    #函数：获取新品上架
    #
    def get_new_arrivals_from_db(self):
        dblist = []
        re_dblist = []
        applist = []
        dblist = self.db.query_new_arrivals_apps()
        re_dblist.extend(dblist)
        for i in dblist:
            pkg_info = self.db.query_one_applications(i[1])
            if (pkg_info == []):
                re_dblist.remove(i)
            else:
                app_dict = self.check_cup_model(pkg_info[0])
                applist.append(app_dict)
            # app = self.get_one_app_info_by_appname(i[1],False)
            # if(app != None):
            #     applist.append(app)
            # else:
            #     re_dblist.remove(i)
        self.get_new_arrivals_ready.emit(re_dblist,applist)

    #
    #函数：获取安卓应用推荐
    #
    def get_android_recommend_from_db(self):
        dblist = []
        re_dblist = []
        applist = []
        try:
            kmre = self.apt_cache[Globals.KYDROID_PACKAGE_NAME]
        except:
            kmre = None
        need_show_kmre = True
        if(kmre != None):
            if (kmre.is_installed == False):
                need_show_kmre = False
            else:
                conf = configparser.ConfigParser()
                conf.read(CONF_PATH)
                startup_time = os.popen(
                    "date -d \"$(awk -F. '{print $1}' /proc/uptime) second ago\" +\"%Y-%m-%d %H:%M:%S\"").readline().strip(
                    "\n")
                install_kmre = ""
                try:
                    install_kmre = conf.get("essential-information", "install_kmre")
                except:
                    pass
                if (install_kmre != ""):
                    format_pattern = "%Y-%m-%d %H:%M:%S"
                    difference = (datetime.datetime.strptime(startup_time, format_pattern) - datetime.datetime.strptime(
                        install_kmre, format_pattern))
                    if (difference.days < 0):
                        need_show_kmre = False
        if (self.kydroid.hasKydroid == True and need_show_kmre == True):
            dblist = self.db.query_android_recommend_apps()
            re_dblist.extend(dblist)
            for i in dblist:
                pkg_info = self.db.query_one_applications(i[1])
                if (pkg_info == []):
                    re_dblist.remove(i)
                else:
                    app_dict = self.check_cup_model(pkg_info[0])
                    applist.append(app_dict)
                # app = self.get_one_app_info_by_appname(i[1], False)
                # if (app != None and app not in applist):
                #     applist.append(app)
                # else:
                #     re_dblist.remove(i)
        # self.get_new_arrivals_ready.emit(re_dblist, applist)
        self.get_android_recommend_ready.emit(re_dblist,applist)

    #
    #函数：获取搜索快捷表
    #
    def get_search_association(self):
        db_list = []
        db_list = self.db.query_search_association()
        for i  in db_list:
            in_list = False
            for app_msg in Globals.GET_ALL_APP_LIST:
                if(app_msg["id"] == i[0]):
                    in_list = True
                    if (i[6] != 1):
                        app_msg["initialsName"] = ""
                        app_msg["pinyinName"] = ""
                        app_msg["anotherName"] = ""
                        break
                    if (i[4] != None):
                        app_msg["initialsName"] = i[4]
                    else:
                        app_msg["initialsName"] = ""
                    if(i[3] != None):
                        app_msg["pinyinName"] = i[3]
                    else:
                        app_msg["pinyinName"] = ""
                    if(i[5] != None):
                        app_msg["anotherName"] = i[5]
                    else:
                        app_msg["anotherName"] = ""
                    break
            if(in_list == True):
                for app_msg in Globals.GET_RATING_APP_LIST:
                    if(app_msg["id"] == i[0]):
                        in_list = True
                        if (i[6] != 1):
                            app_msg["initialsName"] = ""
                            app_msg["pinyinName"] = ""
                            app_msg["anotherName"] = ""
                            break
                        if (i[4] != None):
                            app_msg["initialsName"] = i[4]
                        else:
                            app_msg["initialsName"] = ""
                        if(i[3] != None):
                            app_msg["pinyinName"] = i[3]
                        else:
                            app_msg["pinyinName"] = ""
                        if(i[5] != None):
                            app_msg["anotherName"] = i[5]
                        else:
                            app_msg["anotherName"] = ""
                        break
                continue
            in_list = False
            for app_msg in Globals.KYDROID_APP_LIST:
                if (app_msg["id"] == i[0]):
                    in_list = True
                    if (i[6] != 1):
                        app_msg["initialsName"] = ""
                        app_msg["pinyinName"] = ""
                        app_msg["anotherName"] = ""
                        break
                    if (i[4] != None):
                        app_msg["initialsName"] = i[4]
                    else:
                        app_msg["initialsName"] = ""
                    if (i[3] != None):
                        app_msg["pinyinName"] = i[3]
                    else:
                        app_msg["pinyinName"] = ""
                    if (i[5] != None):
                        app_msg["anotherName"] = i[5]
                    else:
                        app_msg["anotherName"] = ""
                    break
            if(in_list == True):
                for app_msg in Globals.KYDROID_APP_LIST_BY_RATING:
                    if (app_msg["id"] == i[0]):
                        in_list = True
                        if (i[6] != 1):
                            app_msg["initialsName"] = ""
                            app_msg["pinyinName"] = ""
                            app_msg["anotherName"] = ""
                            break
                        if (i[4] != None):
                            app_msg["initialsName"] = i[4]
                        else:
                            app_msg["initialsName"] = ""
                        if (i[3] != None):
                            app_msg["pinyinName"] = i[3]
                        else:
                            app_msg["pinyinName"] = ""
                        if (i[5] != None):
                            app_msg["anotherName"] = i[5]
                        else:
                            app_msg["anotherName"] = ""
                        break
                continue
            for app_msg in Globals.GET_ALL_APP_LIST:
                if (app_msg["id"] == i[0]):
                    in_list = True
                    if (i[6] != 1):
                        app_msg["initialsName"] = ""
                        app_msg["pinyinName"] = ""
                        app_msg["anotherName"] = ""
                        break
                    if (i[4] != None):
                        app_msg["initialsName"] = i[4]
                    else:
                        app_msg["initialsName"] = ""
                    if (i[3] != None):
                        app_msg["pinyinName"] = i[3]
                    else:
                        app_msg["pinyinName"] = ""
                    if (i[5] != None):
                        app_msg["anotherName"] = i[5]
                    else:
                        app_msg["anotherName"] = ""
                    break
            if (in_list == True):
                for app_msg in Globals.GET_RATING_APP_LIST:
                    if (app_msg["id"] == i[0]):
                        in_list = True
                        if (i[6] != 1):
                            app_msg["initialsName"] = ""
                            app_msg["pinyinName"] = ""
                            app_msg["anotherName"] = ""
                            break
                        if (i[4] != None):
                            app_msg["initialsName"] = i[4]
                        else:
                            app_msg["initialsName"] = ""
                        if (i[3] != None):
                            app_msg["pinyinName"] = i[3]
                        else:
                            app_msg["pinyinName"] = ""
                        if (i[5] != None):
                            app_msg["anotherName"] = i[5]
                        else:
                            app_msg["anotherName"] = ""
                        break
                continue
            for app_msg in Globals.GET_ALL_DRIVE_APP_LIST:
                if (app_msg["id"] == i[0]):
                    in_list = True
                    if (i[6] != 1):
                        app_msg["initialsName"] = ""
                        app_msg["pinyinName"] = ""
                        app_msg["anotherName"] = ""
                        break
                    if (i[4] != None):
                        app_msg["initialsName"] = i[4]
                    else:
                        app_msg["initialsName"] = ""
                    if (i[3] != None):
                        app_msg["pinyinName"] = i[3]
                    else:
                        app_msg["pinyinName"] = ""
                    if (i[5] != None):
                        app_msg["anotherName"] = i[5]
                    else:
                        app_msg["anotherName"] = ""
                    break
            if (in_list == True):
                for app_msg in Globals.GET_RATING_DRIVE_APP_LIST:
                    if (app_msg["id"] == i[0]):
                        in_list = True
                        if (i[6] != 1):
                            app_msg["initialsName"] = ""
                            app_msg["pinyinName"] = ""
                            app_msg["anotherName"] = ""
                            break
                        if (i[4] != None):
                            app_msg["initialsName"] = i[4]
                        else:
                            app_msg["initialsName"] = ""
                        if (i[3] != None):
                            app_msg["pinyinName"] = i[3]
                        else:
                            app_msg["pinyinName"] = ""
                        if (i[5] != None):
                            app_msg["anotherName"] = i[5]
                        else:
                            app_msg["anotherName"] = ""
                        break
                continue


    #
    #函数：获取win替换
    #
    def get_win_replace_from_db(self):
        app_list = []
        win_list = []
        get_list = []
        get_list = self.db.query_win_replace_apps()
        for i in get_list:
            if (i[1] != "yuliu"):
                pkg = self.get_one_app_info_by_appname(i[1])
                if (pkg != None):
                    win_list.append(i)
                    app_list.append(pkg)
        self.get_win_replace_ready.emit(win_list,app_list)

    #
    # 函数：获取下载排行
    #
    def get_download_ranking_from_db(self):
        lists = []
        applist = []
        lists = self.db.query_download_ranking_apps()
        # self.db.query_download_ranking_apps()
        for i in lists:
            try:
                pkg = self.apt_cache[i[1]]
            except:
                pkg = None
            if(pkg == None):
                continue
            pkg_info = self.db.query_one_applications(i[1])
            if (pkg_info == []):
                pass
            else:
                app_dict = self.check_cup_model(pkg_info[0])
                if (app_dict != None and app_dict not in applist):
                    applist.append(app_dict)
            # in_list = False
            # for app in Globals.GET_ALL_APP_LIST:
            #     if(app["name"] == i[1]):
            #         in_list = True
            #         break
            # if(in_list == True):
            #     app = self.set_all_app_list(i)
            #     applist.append(app)
            # else:
            #     continue
            if(len(applist) == Globals.DISPLAY_DOWNLOAD_RANK_NUMBER):
                break
        self.download_ranking_ready.emit(applist)

    #
    #函数：获取热门应用
    #
    def get_hot_apps_from_db(self):
        applist = []
        path_list = []
        dblist = self.db.query_hot_apps()
        for i in dblist:
            pkg_info = self.db.query_one_applications(i[1])
            if (pkg_info == []):
                pass
            else:
                app_dict = self.check_cup_model(pkg_info[0])
                applist.append(app_dict)
                path_list.append(i[2])
            # app = self.get_one_app_info_by_appname(i[1], False)
            # if (app != None):
            #     applist.append(app)
            #     path_list.append(i[2])
        self.hot_ready.emit(applist,path_list)


    # get recommend apps
    def get_recommend_apps(self, bysignal=False, first=True):
        # recommends = self.db.get_recommend_apps()
        ad_list = []
        recommends,ad_list = self.get_recommend_by_categories("recommend")
        applist = []
        # list = self.db.query_category_apps("recommend")
        in_hot = False
        if(recommends != None or ad_list != None):
            for rec in recommends:
                in_hot = False
                for app in Globals.HOT_APP_LIST:
                    if(app == rec["name"]):
                        in_hot = True
                        break
                if(in_hot == False):
                    if (self.kydroid_check != False):
                        apk = self.get_apk_by_name(rec["name"])
                        if (apk is not None):
                            # apk.recommendrank = rec[1]
                            pkg_info = self.db.query_one_applications(i[1])
                            if (pkg_info == []):
                                pass
                            else:
                                app_dict = self.check_cup_model(pkg_info[0])
                                if (app_dict != None and app_dict not in applist):
                                    applist.append(app_dict)
                            # res = self.get_one_app_info_by_appname(apk.pkgname)
                            # if(res != None and res not in applist):
                            #     applist.append(res)
            for rec in recommends:
                in_hot = False
                for app in Globals.HOT_APP_LIST:
                    if (app == rec["name"]):
                        in_hot = True
                        break
                if (in_hot == False):
                    # app = self.get_application_by_name(rec[0])
                    if (rec is not None and rec not in applist):
                        # app.recommendrank = rec[1]
                        applist.append(rec)
        # applist = []
        # for i in Globals.RECOMMEND_APP_LIST:
        #     for app in Globals.GET_ALL_APP_LIST:
        #         if (i == app["name"]):
        #             applist.append(app)
        if Globals.UPDATE_HOM == 0:
            self.recommend_ready.emit(applist, bysignal, first)

    #--------------------------------kydroid------------------------------
    # 检测安卓环境是否启动
    def check_kydroid_envrun(self):
        try:
            # kydroid_dri3_desktop = len(os.popen('ps aux | grep "kydroid-app-window" | grep -v grep').readlines())
            # kydroid_appstream = len(os.popen('ps aux | grep "kydroid-appstream" | grep -v grep').readlines())
            if(os.path.exists("/usr/share/kmre/kmre.conf")):
                res = self.backend.get_kydroid_evnrun(pwd.getpwuid(os.getuid())[0], os.getuid(),
                                                      'sys.kmre.boot_completed')
            else:
                res = self.backend.get_kydroid_evnrun(pwd.getpwuid(os.getuid())[0], os.getuid(),
                                                      'sys.kydroid.boot_completed')

            # if kydroid_dri3_desktop and kydroid_appstream:
            if res:
                Globals.APK_EVNRUN = 1
                return True
            else:
                Globals.APK_EVNRUN = 0
                return False
        except:
            if (Globals.DEBUG_SWITCH):
                #print("Check kydroid process ERROR!!!")
                Globals.LOG_ERROR.logger.error(_("Check kydroid process ERROR!!!"))
            return False


    # 循环检测安卓环境是否启动
    def cycle_check_kydroid_envrun(self,need_show_widget):
        flag = False
        sum = 0
        ret = 1
        while not flag:
            sum += 1
            flag = self.check_kydroid_envrun()
            time.sleep(1)
            if sum >  60*3:
                if (Globals.DEBUG_SWITCH):
                    #print("移动应用环境启动超时")
                    Globals.LOG_ERROR.logger.error(_("移动应用环境启动超时"))
                ret = 0
                break
        self.apkenvrunfrist = True
        Globals.IS_INIT_KYDROID_ENVIRONMENT = False
        self.kydroid_envrun_over.emit(ret,need_show_widget)

    #
    # 函数：保存安卓列表
    #
    def get_kydroid_source_list(self, data):
        try:
            for msg in data:
                if(msg["allStatus"] != "2" and msg["allDeleteStatus"] != "1" and msg["archStatus"] != "1"):
                    Globals.KYDROID_SOURCE_LIST[msg["packageName"]] = msg
        except:
            pass

    #循环获取安卓列表
    def apk_page_create_emit(self,need_show_widget):
        for i in range(10):
            if (not Globals.apkpagefirst) and Globals.isOnline:
                break
            if(Globals.KYDROID_SOURCE_LIST == {}):
                self.get_kydroid_app_list()
            res_data = self.start_download_kydroid_sl(need_show_widget)
            if Globals.isOnline:
                Globals.apkpagefirst = False
            else:
                pass
            # res = False
            if(res_data == False and i == 9):
                Globals.isOnline = False
                Globals.IS_INIT_KYDROID_ENVIRONMENT = False
                try:
                    if(res_data != False and (res_data["code"] == 200 or res_data["code"] == 204)):
                        self.network_error.emit("./res/no_search.png", _("There is no Android Software for this architecture."))
                    else:
                        self.network_error.emit("./res/no_network.png", _("There is something wrong with the network. Please try again later."))
                except:
                    self.network_error.emit("./res/no_network.png",_("There is something wrong with the network. Please try again later."))
        Globals.IS_INIT_KYDROID_ENVIRONMENT = False
            # self.download_apk_source_error.emit(True)
        # self.download_apk_source_over.emit(True)
        # self.get_apk_list_over.emit(need_show_widget)

    #
    # 函数：合并源中包列表和本地列表
    #
    def merge_apk_list(self, app_dict):
        for apk in self.apk_list:
            if apk.name == app_dict['package_name']:
                apk.is_installed = True
                apk.installed_version = app_dict['version_name']
                if apt.apt_pkg.version_compare(apk.candidate_version, apk.installed_version) == 1:
                    apk.is_upgradable = True
                return True
        for i in Globals.KYDROID_SOURCE_LIST.keys():
            if(i == app_dict['package_name']):
                apkinfo = ApkInfo(app_dict['package_name'], app_dict['app_name'], '', '0', '/', '/')
                apkinfo.installed_version = app_dict['version_name']
                apkinfo.from_ukscdb = False
                apkinfo.is_installed = True
                self.apk_list.append(apkinfo)
                break
        return False

    def start_download_apk(self, apkInfo):
        try:
            dm = downloadmanager.DownloadManager(self, apkInfo)
            if (Globals.DEBUG_SWITCH):
                #print("apkinfo : ", apkInfo.__dict__)
                Globals.LOG_INFO.logger.info(_("apkinfo : "+str(apkInfo.__dict__)))
            dm.run()
            return True
        except:
            return False

    def slot_start_download_apk(self,apkinfo):
        self.start_download_apk(apkinfo)

    #
    # 函数：卸载安卓兼容应用接口
    #
    def uninstall_app(self, appname):
        try:
            um = UninstallManager(self, appname)
            um.start()
            return True
        except:
            return False

    #--------------------------------0.3----------------------------------
    #
    # 函数：发送启动记录
    #
    def submit_pingback_main(self):
        return
        # kwargs = {}
        # #
        # item = SilentWorkerItem("submit_pingback_main", kwargs)
        # self.squeue.put_nowait(item)


    #
    # 函数：获取新应用信息
    #
    def get_newer_application_info(self):
        kwargs = {}

        item = SilentWorkerItem("get_newer_application_info", kwargs)
        self.squeue.put_nowait(item)


    #
    # 函数：获取新的广告
    #
    def get_newer_application_ads(self):
        kwargs = {}

        item = SilentWorkerItem("get_newer_application_ads", kwargs)
        self.squeue.put_nowait(item)

    #
    #函数：获取新品上架
    #
    def get_newest_app(self):
        kwargs = {}

        item = SilentWorkerItem("get_newest_app", kwargs)
        self.squeue.put_nowait(item)

    #
    #函数：获取下载排行
    #
    def get_download_ranking(self):
        kwargs = {}

        item = SilentWorkerItem("get_download_ranking", kwargs)
        self.squeue.put_nowait(item)

    #
    # 函数：获取所有分类
    #
    def get_all_categories(self):
        kwargs = {}
        item = SilentWorkerItem("get_all_categories", kwargs)
        self.squeue.put_nowait(item)


    #
    # 函数：获取所有排名和推荐
    #
    def get_all_rank_and_recommend(self):
        kwargs = {}

        item = SilentWorkerItem("get_all_rank_and_recommend", kwargs)
        self.squeue.put_nowait(item)

    #
    #函数：实时获取下载次数
    #
    def get_download_count_from_server(self,appname,id):
        return 0,appname,""

    # #
    # # 函数：发送启动记录
    # #
    # def submit_pingback_main(self):
    #     pass
    #     kwargs = {}
    #     item = SilentWorkerItem("submit_pingback_main", kwargs)
    #     self.squeue.put_nowait(item)

    #
    # 函数：登陆接口
    #
    def ui_login(self, login_list):
        return

    #
    #函数：发送手机号，获得验证码
    #
    def get_captcha(self,phone_num):
        return

    # 函数：拿到本地token
    # 参数：无
    # 返回：本地token值
    def get_local_token(self):

        return False

    # 函数：从服务器获取本地token返回值
    # 参数：空
    # 返回值：user_list:
    def verify_token(self,res_list):
        return ""


    #
    # 函数：发送安装记录
    #
    def submit_pingback_app(self, app_name,app_id, isrcm=False):
#        pass
        #(^-^)更新下载次数(更新完成后还需要重新加载app_detail界面的下载次数)
        #self.db.update_app_downloadtotal(app_name)
        kwargs = {"app_name": app_name,
                "id": app_id,
                "isrcm": isrcm,
                "user": Globals.USER,
                }
        item = SilentWorkerItem("submit_pingback_app", kwargs)
        self.squeue.put_nowait(item)

    #
    # 函数：提交评分
    #
    def submit_rating(self, app_name, rating):
        return
        # self.submit_rating_over.emit(res)


    def download_other_images(self):
        kwargs = {}

        item = SilentWorkerItem("download_images", kwargs)
        self.squeue.put_nowait(item)
