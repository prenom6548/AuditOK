
import os
import apt
import subprocess
from models.enums import UBUNTUKYLIN_CACHE_ICON_PATH,UBUNTUKYLIN_CACHE_ICON_PATH,UBUNTUKYLIN_RES_ICON_PATH,KYLIN_SYSTEM_ICON_48_PATH,UK_SYSTEM_ICON_48_PATH,KYLIN_SYSTEM_ICON_128_PATH,UK_SYSTEM_ICON_128_PATH,\
                         UBUNTUKYLIN_CACHE_COVERIMAGE_PATH,UBUNTUKYLIN_RES_COVERIMAGE_PATH,UBUNTUKYLIN_CACHE_NEW_ARRIVALS_PATH,\
                         UBUNTUKYLIN_RES_NEW_ARRIVALS_PATH,UBUNTUKYLIN_RES_AD_PATH,KYLIN_SYSTEM_ICON_64_PATH,KYLIN_SVG_ICON_PATH,UK_SVG_ICON_PATH,UK_SYSTEM_ICON_64_PATH,\
                         SVG_ICON_PATH,ICON_48_PATH,ICON_64_PATH,ICON_128_PATH,KYLIN_SYSTEM_ICON_24_PATH
from models.enums import UKSC_CACHE_DIR
from models.globals import Globals
from gi.repository import Gio

KSC_INSTALL_ICON_PATH = "/usr/share/kylin-software-center/data/icons/"


#
# 函数：获取图标地址返回给调用接口的对象
#
def get_icon_path(app_name):
    if(app_name == "pluma"):
        app_name = "accessories-text-editor"
    if (os.path.isfile(SVG_ICON_PATH + str(app_name) + ".svg")):
        return SVG_ICON_PATH + str(app_name) + ".svg"
    elif (os.path.isfile(ICON_128_PATH + str(app_name) + ".png")):
        return ICON_128_PATH + str(app_name) + ".png"
    elif (os.path.isfile(ICON_128_PATH + str(app_name) + ".jpg")):
        return ICON_128_PATH + str(app_name) + ".jpg"
    elif (os.path.isfile(ICON_64_PATH + str(app_name) + ".png")):
        return ICON_64_PATH + str(app_name) + ".png"
    elif (os.path.isfile(ICON_64_PATH + str(app_name) + ".jpg")):
        return ICON_64_PATH + str(app_name) + ".jpg"
    elif (os.path.isfile(ICON_48_PATH + str(app_name) + ".png")):
        return ICON_48_PATH + str(app_name) + ".png"
    elif (os.path.isfile(ICON_48_PATH + str(app_name) + ".jpg")):
        return ICON_48_PATH + str(app_name) + ".jpg"
    elif (os.path.isfile(KYLIN_SVG_ICON_PATH + str(app_name) + ".svg")):
        return KYLIN_SVG_ICON_PATH + str(app_name) + ".svg"
    elif (os.path.isfile(UK_SVG_ICON_PATH + str(app_name) + ".svg")):
        return UK_SVG_ICON_PATH + str(app_name) + ".svg"
    elif (os.path.isfile(UK_SVG_ICON_PATH + str(app_name) + ".svg")):
        return UK_SVG_ICON_PATH + str(app_name) + ".svg"
    elif (os.path.isfile(KYLIN_SYSTEM_ICON_128_PATH + str(app_name) + ".png")):
        return KYLIN_SYSTEM_ICON_128_PATH + str(app_name) + ".png"
    elif (os.path.isfile(KYLIN_SYSTEM_ICON_128_PATH + str(app_name) + ".jpg")):
        return KYLIN_SYSTEM_ICON_128_PATH + str(app_name) + ".jpg"
    elif (os.path.isfile(UK_SYSTEM_ICON_128_PATH + str(app_name) + ".png")):
        return UK_SYSTEM_ICON_128_PATH + str(app_name) + ".png"
    elif (os.path.isfile(UK_SYSTEM_ICON_128_PATH + str(app_name) + ".jpg")):
        return UK_SYSTEM_ICON_128_PATH + str(app_name) + ".jpg"
    elif (os.path.isfile(KYLIN_SYSTEM_ICON_64_PATH + str(app_name) + ".png")):
        return KYLIN_SYSTEM_ICON_64_PATH + str(app_name) + ".png"
    elif (os.path.isfile(KYLIN_SYSTEM_ICON_64_PATH + str(app_name) + ".jpg")):
        return KYLIN_SYSTEM_ICON_64_PATH + str(app_name) + ".jpg"
    elif (os.path.isfile(UK_SYSTEM_ICON_64_PATH + str(app_name) + ".png")):
        return UK_SYSTEM_ICON_64_PATH + str(app_name) + ".png"
    elif (os.path.isfile(UK_SYSTEM_ICON_64_PATH + str(app_name) + ".jpg")):
        return UK_SYSTEM_ICON_64_PATH + str(app_name) + ".jpg"
    elif (os.path.isfile(KYLIN_SYSTEM_ICON_48_PATH + str(app_name) + ".png")):
        return KYLIN_SYSTEM_ICON_48_PATH + str(app_name) + ".png"
    elif (os.path.isfile(UK_SYSTEM_ICON_48_PATH + str(app_name) + ".png")):
        return UK_SYSTEM_ICON_48_PATH + str(app_name) + ".png"
    elif (os.path.isfile(UBUNTUKYLIN_CACHE_ICON_PATH + str(app_name) + ".png")):
        return UBUNTUKYLIN_CACHE_ICON_PATH + str(app_name) + ".png"
    elif (os.path.isfile(UBUNTUKYLIN_CACHE_ICON_PATH + str(app_name) + ".jpg")):
        return UBUNTUKYLIN_CACHE_ICON_PATH + str(app_name) + ".jpg"
    elif (os.path.isfile(KSC_INSTALL_ICON_PATH + str(app_name) + ".png")):
        return KSC_INSTALL_ICON_PATH + str(app_name) + ".png"
    elif (os.path.isfile(KSC_INSTALL_ICON_PATH + str(app_name) + ".jpg")):
        return KSC_INSTALL_ICON_PATH + str(app_name) + ".jpg"
    else:
        return KSC_INSTALL_ICON_PATH + "default.png"

#
# 函数：获取图标地址的处理函数
#
def get_small_icon_path(app_name):
    if (os.path.isfile(KYLIN_SYSTEM_ICON_128_PATH + str(app_name) + ".png")):
        return KYLIN_SYSTEM_ICON_128_PATH + str(app_name) + ".png"
    elif (os.path.isfile(KYLIN_SYSTEM_ICON_128_PATH + str(app_name) + ".jpg")):
        return KYLIN_SYSTEM_ICON_128_PATH + str(app_name) + ".jpg"
    elif (os.path.isfile(Globals.UK_SYSTEM_ICON_128_PATH + str(app_name) + ".png")):
        return Globals.UK_SYSTEM_ICON_128_PATH + str(app_name) + ".png"
    elif (os.path.isfile(Globals.UK_SYSTEM_ICON_128_PATH + str(app_name) + ".jpg")):
        return Globals.UK_SYSTEM_ICON_128_PATH + str(app_name) + ".jpg"
    elif (os.path.isfile(KYLIN_SYSTEM_ICON_64_PATH + str(app_name) + ".png")):
        return KYLIN_SYSTEM_ICON_64_PATH + str(app_name) + ".png"
    elif (os.path.isfile(KYLIN_SYSTEM_ICON_64_PATH + str(app_name) + ".jpg")):
        return KYLIN_SYSTEM_ICON_64_PATH + str(app_name) + ".jpg"
    elif (os.path.isfile(Globals.UK_SYSTEM_ICON_64_PATH + str(app_name) + ".png")):
        return Globals.UK_SYSTEM_ICON_64_PATH + str(app_name) + ".png"
    elif (os.path.isfile(Globals.UK_SYSTEM_ICON_64_PATH + str(app_name) + ".jpg")):
        return Globals.UK_SYSTEM_ICON_64_PATH + str(app_name) + ".jpg"
    elif (os.path.isfile(KYLIN_SYSTEM_ICON_48_PATH + str(app_name) + ".png")):
        return KYLIN_SYSTEM_ICON_48_PATH + str(app_name) + ".png"
    elif (os.path.isfile(Globals.UK_SYSTEM_ICON_48_PATH + str(app_name) + ".png")):
        return Globals.UK_SYSTEM_ICON_48_PATH + str(app_name) + ".png"
    elif (os.path.isfile(UBUNTUKYLIN_CACHE_ICON_PATH + str(app_name) + ".png")):
        return UBUNTUKYLIN_CACHE_ICON_PATH + str(app_name) + ".png"
    elif (os.path.isfile(UBUNTUKYLIN_CACHE_ICON_PATH + str(app_name) + ".jpg")):
        return UBUNTUKYLIN_CACHE_ICON_PATH + str(app_name) + ".jpg"
    elif (os.path.isfile(UBUNTUKYLIN_RES_ICON_PATH + str(app_name) + ".png")):
        return UBUNTUKYLIN_RES_ICON_PATH + str(app_name) + ".png"
    elif (os.path.isfile(UBUNTUKYLIN_RES_ICON_PATH + str(app_name) + ".jpg")):
        return UBUNTUKYLIN_RES_ICON_PATH + str(app_name) + ".jpg"
    elif (os.path.isfile(SVG_ICON_PATH + str(app_name) + ".svg")):
        return SVG_ICON_PATH + str(app_name) + ".svg"
    elif (os.path.isfile(ICON_128_PATH + str(app_name) + ".png")):
        return ICON_128_PATH + str(app_name) + ".png"
    elif (os.path.isfile(ICON_128_PATH + str(app_name) + ".jpg")):
        return ICON_128_PATH + str(app_name) + ".jpg"
    elif (os.path.isfile(ICON_64_PATH + str(app_name) + ".png")):
        return ICON_64_PATH + str(app_name) + ".png"
    elif (os.path.isfile(ICON_64_PATH + str(app_name) + ".jpg")):
        return ICON_64_PATH + str(app_name) + ".jpg"
    elif (os.path.isfile(ICON_48_PATH + str(app_name) + ".png")):
        return ICON_48_PATH + str(app_name) + ".png"
    elif (os.path.isfile(ICON_48_PATH + str(app_name) + ".jpg")):
        return ICON_48_PATH + str(app_name) + ".jpg"
    else:
        return UBUNTUKYLIN_RES_ICON_PATH + "default.png"

def get_title_icon(app_name):
    if (os.path.isfile(KYLIN_SYSTEM_ICON_24_PATH + str(app_name) + ".png")):
        return KYLIN_SYSTEM_ICON_48_PATH + str(app_name) + ".png"
    elif (os.path.isfile(KYLIN_SYSTEM_ICON_24_PATH + str(app_name) + ".jpg")):
        return KYLIN_SYSTEM_ICON_48_PATH + str(app_name) + ".jpg"
    elif (os.path.isfile(Globals.UK_SYSTEM_ICON_24_PATH + str(app_name) + ".png")):
        return Globals.UK_SYSTEM_ICON_24_PATH + str(app_name) + ".png"
    elif (os.path.isfile(Globals.UK_SYSTEM_ICON_24_PATH + str(app_name) + ".jpg")):
        return Globals.UK_SYSTEM_ICON_24_PATH + str(app_name) + ".jpg"


#
# 函数：获取图标地址的处理函数
#
def get_big_icon_path(app_name):
    if(app_name == "pluma"):
        app_name = "accessories-text-editor"
    if(app_name == Globals.APPNAME):
        if (os.path.isfile(KYLIN_SYSTEM_ICON_128_PATH + str(app_name) + ".png")):
            return KYLIN_SYSTEM_ICON_128_PATH + str(app_name) + ".png"
        elif (os.path.isfile(KYLIN_SYSTEM_ICON_128_PATH + str(app_name) + ".jpg")):
            return KYLIN_SYSTEM_ICON_128_PATH + str(app_name) + ".jpg"
        elif (os.path.isfile(Globals.UK_SYSTEM_ICON_128_PATH + str(app_name) + ".png")):
            return Globals.UK_SYSTEM_ICON_128_PATH + str(app_name) + ".png"
        elif (os.path.isfile(Globals.UK_SYSTEM_ICON_128_PATH + str(app_name) + ".jpg")):
            return Globals.UK_SYSTEM_ICON_128_PATH + str(app_name) + ".jpg"
        elif (os.path.isfile(KYLIN_SYSTEM_ICON_64_PATH + str(app_name) + ".png")):
            return KYLIN_SYSTEM_ICON_64_PATH + str(app_name) + ".png"
        elif (os.path.isfile(KYLIN_SYSTEM_ICON_64_PATH + str(app_name) + ".jpg")):
            return KYLIN_SYSTEM_ICON_64_PATH + str(app_name) + ".jpg"
        elif (os.path.isfile(Globals.UK_SYSTEM_ICON_64_PATH + str(app_name) + ".png")):
            return Globals.UK_SYSTEM_ICON_64_PATH + str(app_name) + ".png"
        elif (os.path.isfile(Globals.UK_SYSTEM_ICON_64_PATH + str(app_name) + ".jpg")):
            return Globals.UK_SYSTEM_ICON_64_PATH + str(app_name) + ".jpg"
        elif (os.path.isfile(KYLIN_SYSTEM_ICON_48_PATH + str(app_name) + ".png")):
            return KYLIN_SYSTEM_ICON_48_PATH + str(app_name) + ".png"
        elif (os.path.isfile(Globals.UK_SYSTEM_ICON_48_PATH + str(app_name) + ".png")):
            return Globals.UK_SYSTEM_ICON_48_PATH + str(app_name) + ".png"
        elif (os.path.isfile(UBUNTUKYLIN_CACHE_ICON_PATH + str(app_name) + ".png")):
            return UBUNTUKYLIN_CACHE_ICON_PATH + str(app_name) + ".png"
        elif (os.path.isfile(UBUNTUKYLIN_CACHE_ICON_PATH + str(app_name) + ".jpg")):
            return UBUNTUKYLIN_CACHE_ICON_PATH + str(app_name) + ".jpg"
        elif (os.path.isfile(UBUNTUKYLIN_RES_ICON_PATH + str(app_name) + ".png")):
            return UBUNTUKYLIN_RES_ICON_PATH + str(app_name) + ".png"
        elif (os.path.isfile(UBUNTUKYLIN_RES_ICON_PATH + str(app_name) + ".jpg")):
            return UBUNTUKYLIN_RES_ICON_PATH + str(app_name) + ".jpg"
        elif (os.path.isfile(SVG_ICON_PATH + str(app_name) + ".svg")):
            return SVG_ICON_PATH + str(app_name) + ".svg"
        elif (os.path.isfile(ICON_128_PATH + str(app_name) + ".png")):
            return ICON_128_PATH + str(app_name) + ".png"
        elif (os.path.isfile(ICON_128_PATH + str(app_name) + ".jpg")):
            return ICON_128_PATH + str(app_name) + ".jpg"
        elif (os.path.isfile(ICON_64_PATH + str(app_name) + ".png")):
            return ICON_64_PATH + str(app_name) + ".png"
        elif (os.path.isfile(ICON_64_PATH + str(app_name) + ".jpg")):
            return ICON_64_PATH + str(app_name) + ".jpg"
        elif (os.path.isfile(ICON_48_PATH + str(app_name) + ".png")):
            return ICON_48_PATH + str(app_name) + ".png"
        elif (os.path.isfile(ICON_48_PATH + str(app_name) + ".jpg")):
            return ICON_48_PATH + str(app_name) + ".jpg"
        else:
            return UBUNTUKYLIN_RES_ICON_PATH + "default.png"
    if (os.path.isfile(KYLIN_SYSTEM_ICON_128_PATH + str(app_name) + ".png")):
        return KYLIN_SYSTEM_ICON_128_PATH + str(app_name) + ".png"
    elif (os.path.isfile(KYLIN_SYSTEM_ICON_128_PATH + str(app_name) + ".jpg")):
        return KYLIN_SYSTEM_ICON_128_PATH + str(app_name) + ".jpg"
    elif (os.path.isfile(Globals.UK_SYSTEM_ICON_128_PATH + str(app_name) + ".png")):
        return Globals.UK_SYSTEM_ICON_128_PATH + str(app_name) + ".png"
    elif (os.path.isfile(Globals.UK_SYSTEM_ICON_128_PATH + str(app_name) + ".jpg")):
        return Globals.UK_SYSTEM_ICON_128_PATH + str(app_name) + ".jpg"
    elif (os.path.isfile(KYLIN_SYSTEM_ICON_64_PATH + str(app_name) + ".png")):
        return KYLIN_SYSTEM_ICON_64_PATH + str(app_name) + ".png"
    elif (os.path.isfile(KYLIN_SYSTEM_ICON_64_PATH + str(app_name) + ".jpg")):
        return KYLIN_SYSTEM_ICON_64_PATH + str(app_name) + ".jpg"
    elif (os.path.isfile(Globals.UK_SYSTEM_ICON_64_PATH + str(app_name) + ".png")):
        return Globals.UK_SYSTEM_ICON_64_PATH + str(app_name) + ".png"
    elif (os.path.isfile(Globals.UK_SYSTEM_ICON_64_PATH + str(app_name) + ".jpg")):
        return Globals.UK_SYSTEM_ICON_64_PATH + str(app_name) + ".jpg"
    elif (os.path.isfile(KYLIN_SYSTEM_ICON_48_PATH + str(app_name) + ".png")):
        return KYLIN_SYSTEM_ICON_48_PATH + str(app_name) + ".png"
    elif (os.path.isfile(Globals.UK_SYSTEM_ICON_48_PATH + str(app_name) + ".png")):
        return Globals.UK_SYSTEM_ICON_48_PATH + str(app_name) + ".png"
    elif (os.path.isfile(UBUNTUKYLIN_CACHE_ICON_PATH + str(app_name) + ".svg")):
        return UBUNTUKYLIN_CACHE_ICON_PATH + str(app_name) + ".svg"
    elif (os.path.isfile(UBUNTUKYLIN_CACHE_ICON_PATH + str(app_name) + ".png")):
        return UBUNTUKYLIN_CACHE_ICON_PATH + str(app_name) + ".png"
    elif (os.path.isfile(UBUNTUKYLIN_CACHE_ICON_PATH + str(app_name) + ".jpg")):
        return UBUNTUKYLIN_CACHE_ICON_PATH + str(app_name) + ".jpg"
    elif (os.path.isfile(UBUNTUKYLIN_RES_ICON_PATH + str(app_name) + ".svg")):
        return UBUNTUKYLIN_RES_ICON_PATH + str(app_name) + ".svg"
    elif (os.path.isfile(UBUNTUKYLIN_RES_ICON_PATH + str(app_name) + ".png")):
        return UBUNTUKYLIN_RES_ICON_PATH + str(app_name) + ".png"
    elif (os.path.isfile(UBUNTUKYLIN_RES_ICON_PATH + str(app_name) + ".jpg")):
        return UBUNTUKYLIN_RES_ICON_PATH + str(app_name) + ".jpg"
    elif (os.path.isfile(SVG_ICON_PATH + str(app_name) + ".svg")):
        return SVG_ICON_PATH + str(app_name) + ".svg"
    elif (os.path.isfile(ICON_128_PATH + str(app_name) + ".png")):
        return ICON_128_PATH + str(app_name) + ".png"
    elif (os.path.isfile(ICON_128_PATH + str(app_name) + ".jpg")):
        return ICON_128_PATH + str(app_name) + ".jpg"
    elif (os.path.isfile(ICON_64_PATH + str(app_name) + ".png")):
        return ICON_64_PATH + str(app_name) + ".png"
    elif (os.path.isfile(ICON_64_PATH + str(app_name) + ".jpg")):
        return ICON_64_PATH + str(app_name) + ".jpg"
    elif (os.path.isfile(ICON_48_PATH + str(app_name) + ".png")):
        return ICON_48_PATH + str(app_name) + ".png"
    elif (os.path.isfile(ICON_48_PATH + str(app_name) + ".jpg")):
        return ICON_48_PATH + str(app_name) + ".jpg"
    elif (os.path.isfile(KYLIN_SVG_ICON_PATH + str(app_name) + ".svg")):
        return KYLIN_SVG_ICON_PATH + str(app_name) + ".svg"
    elif (os.path.isfile(UK_SVG_ICON_PATH + str(app_name) + ".svg")):
        return UK_SVG_ICON_PATH + str(app_name) + ".svg"
    else:
        return UBUNTUKYLIN_RES_ICON_PATH + "default.png"

#
#函数：获取封面图地址处理函数
#
def get_coverimage_path(app_name):
    if(os.path.isfile(UBUNTUKYLIN_CACHE_COVERIMAGE_PATH + str(app_name) + ".png")):
        return UBUNTUKYLIN_CACHE_COVERIMAGE_PATH + str(app_name) + ".png"
    elif (os.path.isfile(UBUNTUKYLIN_CACHE_COVERIMAGE_PATH + str(app_name) + ".jpg")):
        return UBUNTUKYLIN_CACHE_COVERIMAGE_PATH + str(app_name) + ".jpg"
    elif(os.path.isfile(UBUNTUKYLIN_RES_COVERIMAGE_PATH + str(app_name) + ".png")):
        return UBUNTUKYLIN_RES_COVERIMAGE_PATH + str(app_name) + ".png"
    elif (os.path.isfile(UBUNTUKYLIN_RES_COVERIMAGE_PATH + str(app_name) + ".jpg")):
        return UBUNTUKYLIN_RES_COVERIMAGE_PATH + str(app_name) + ".jpg"
    else:
        return UBUNTUKYLIN_RES_COVERIMAGE_PATH + "default.png"

#
#函数：获取封面图地址处理函数
#
def get_coverimage(icon_path):
    if(os.path.isfile(UBUNTUKYLIN_CACHE_COVERIMAGE_PATH + icon_path)):
        return UBUNTUKYLIN_CACHE_COVERIMAGE_PATH + icon_path
    elif (os.path.isfile(UBUNTUKYLIN_RES_COVERIMAGE_PATH + icon_path)):
        return UBUNTUKYLIN_RES_COVERIMAGE_PATH + icon_path
    else:
        return UBUNTUKYLIN_RES_COVERIMAGE_PATH + "default.png"

#
#函数：获取新品上架图
#
def get_new_arrivals_image_path(app_name):
    if (os.path.isfile(UBUNTUKYLIN_CACHE_NEW_ARRIVALS_PATH + str(app_name) + ".png")):
        return UBUNTUKYLIN_CACHE_NEW_ARRIVALS_PATH + str(app_name) + ".png"
    elif (os.path.isfile(UBUNTUKYLIN_CACHE_NEW_ARRIVALS_PATH + str(app_name) + ".jpg")):
        return UBUNTUKYLIN_CACHE_NEW_ARRIVALS_PATH + str(app_name) + ".jpg"
    elif (os.path.isfile(UBUNTUKYLIN_RES_NEW_ARRIVALS_PATH + str(app_name) + ".png")):
        return UBUNTUKYLIN_RES_NEW_ARRIVALS_PATH + str(app_name) + ".png"
    elif (os.path.isfile(UBUNTUKYLIN_RES_NEW_ARRIVALS_PATH + str(app_name) + ".jpg")):
        return UBUNTUKYLIN_RES_NEW_ARRIVALS_PATH + str(app_name) + ".jpg"
    else:
        return UBUNTUKYLIN_RES_COVERIMAGE_PATH + "default.png"

#
#函数：获取新品上架图
#
def get_new_arrivals_image(path):
    if (os.path.isfile(UBUNTUKYLIN_CACHE_NEW_ARRIVALS_PATH + path)):
        return UBUNTUKYLIN_CACHE_NEW_ARRIVALS_PATH + path
    elif (os.path.isfile(UBUNTUKYLIN_RES_NEW_ARRIVALS_PATH + path)):
        return UBUNTUKYLIN_RES_NEW_ARRIVALS_PATH + path
    else:
        return UBUNTUKYLIN_RES_COVERIMAGE_PATH + "default.png"

def get_ads_image(file_name):
    if (os.path.isfile(UBUNTUKYLIN_RES_AD_PATH + file_name)):
        return UBUNTUKYLIN_RES_AD_PATH + file_name
    else:
        return None

#
# 函数名: 通过默认浏览器打开链接
# Function: Open Url with default browser
#
def webbrowser_open_url(url):
    if(url == None):
        url = "https://www.kylinos.cn/"
    browser_name = "qaxbrowser-safe-stable"
    apt_ca = apt.Cache()
    try:
        try:
            appList = Gio.AppInfo.get_default_for_type("x-scheme-handler/https", False)
            name = Gio.AppInfo.get_name(appList)
            executable = Gio.AppInfo.get_executable(appList)
            os.system(executable + " " + url + " & > /dev/null")
            return
        except:
            pass
        try:
            config_home = UKSC_CACHE_DIR.rsplit("/", 2)[0] + "/.config/"
            mimeapps_path = config_home + "mimeapps.list"
            with open(mimeapps_path, "r", encoding="utf-8") as mine_file:
                for line in mine_file.readlines():
                    if line.find("x-scheme-handler/https") != -1:
                        browser_name = (line.rsplit("=", 1)[1]).rsplit(".", 1)[0]
                        if Globals.DEBUG_SWITCH:
                            print("broswer name: %s" % browser_name)
                        break
            if(browser_name == "browser360-cn"):
                browser_name = "browser360-cn-stable"
            elif(browser_name == "qaxbrowser-safe"):
                browser_name = "qaxbrowser-safe-stable"
            try:
                pkg = apt_ca[browser_name]
                if (pkg.is_installed == True):
                    os.system(browser_name + " " + url + " & > /dev/null")
                    return
            except:
                pass
        except:
            pass
        try:
            browser_name = "qaxbrowser-safe-stable"
            pkg = apt_ca[browser_name]
            if(pkg.is_installed == True):
                os.system(browser_name + " " + url + " & > /dev/null")
            else:
                browser_name = "firefox"
                try:
                    pkg = apt_ca[browser_name]
                    if (pkg.is_installed == True):
                        os.system(browser_name + " " + url + " & > /dev/null")
                    else:
                        try:
                            browser_name = "firefox-esr"
                            pkg = apt_ca[browser_name]
                            if (pkg.is_installed == True):
                                os.system(browser_name + " " + url + " & > /dev/null")
                        except:
                            browser_name = "lbrowser"
                            try:
                                pkg = apt_ca[browser_name]
                                if (pkg.is_installed == True):
                                    os.system(browser_name + " " + url + " & > /dev/null")
                            except:
                                pass
                except:
                    try:
                        browser_name = "firefox-esr"
                        pkg = apt_ca[browser_name]
                        os.system(browser_name + " " + url + " & > /dev/null")
                    except:
                        browser_name = "lbrowser"
                        try:
                            pkg = apt_ca[browser_name]
                            if (pkg.is_installed == True):
                                os.system(browser_name + " " + url + " & > /dev/null")
                        except:
                            pass
        except:
            browser_name = "firefox"
            try:
                pkg = apt_ca[browser_name]
                if(pkg.is_installed == True):
                    os.system(browser_name + " " + url + " & > /dev/null")
                else:
                    try:
                        browser_name = "firefox-esr"
                        pkg = apt_ca[browser_name]
                        if(pkg.is_installed):
                            os.system(browser_name + " " + url + " & > /dev/null")
                    except:
                        browser_name = "lbrowser"
                        try:
                            pkg = apt_ca[browser_name]
                            if (pkg.is_installed == True):
                                os.system(browser_name + " " + url + " & > /dev/null")
                        except:
                            pass
            except:
                try:
                    browser_name = "firefox-esr"
                    pkg = apt_ca[browser_name]
                    if(pkg.is_installed):
                        os.system(browser_name + " " + url + " & > /dev/null")
                    else:
                        browser_name = "lbrowser"
                        try:
                            pkg = apt_ca[browser_name]
                            if (pkg.is_installed == True):
                                os.system(browser_name + " " + url + " & > /dev/null")
                        except:
                            pass
                except:
                    browser_name = "lbrowser"
                    try:
                        pkg = apt_ca[browser_name]
                        if (pkg.is_installed == True):
                            os.system(browser_name + " " + url + " & > /dev/null")
                    except:
                        pass
    except Exception as e:
        pass


def get_desktop_dict(system_version):
    if system_version == "V10":
        dingtalk = "DingTalkLinux"
    else:
        dingtalk = "nwjs-dingtalk"
    # moshengren add
    PKG_NAME = {
        "wps-office": "wps-office-wps",
        "uget": "uget-gtk",
        "eclipse-platform": "eclipse",
        "software-center": "ubuntu-software-center",
        "mathwar": "MathWar",
        "gnome-disk-utility": "gnome-disks",
        "kino": "Kino",
        "monajat-applet": "monajat",
        "system-config-printer-applet": "system-config-printer",
        "xterm": "debian-uxterm",
        "virtualbox-qt": "virtualbox",
        "lovewallpaper": "love-wallpaper",
        "steam-launcher": "steam",
        "obs-studio": "obs",
        "google-chrome-stable": "google-chrome",
        "youker-assistant": "kylin-assistant",
        "crossover:i386": "/opt/cxoffice/bin/crossover",  # crossover:i386
        "gnome-screenshot": "org.gnome.Screenshot",
        # "gnome-mines":  "gnomine",
        "qtcreator": "org.qt-project.qtcreator",
        "suwellreaderpro": "SuwellReader",
        "qaxbrowser-safe-stable": "qaxbrowser-safe",
        "foxitofficesuite": "FoxitofficeSuite",
        "linkdoodsetup": "Linkdood",
        "yozo-office": "yozo-writer",
        "zwcad-linuxpreinst": "ZWCAD-LinuxPreInst",
        "browser360-cn-stable": "browser360-cn",
        "360safe": "start360safe",
        "eleanscan": "Elean",
        "jingyunsd": "JingyunSd",
        "ukui-biometric-manager": "biometric-manager",
        "linuxqq": "qq",
        "tenvideo-universal": "TencentVideo",
        "teamviewer": "com.teamviewer.TeamViewer",
        "retext": "me.mitya57.ReText",
        "youku-app": "YouKu",
        "zjcakeymanager": "keymanager",
        "foundercebreader": "CEBReader",
        "ofdreader": "OFDReader",
        "founderdoceditor": "DocEditor",
        "qaxbrowser-pioneer-stable": "qaxbrowser-pioneer",
        "dianjureader": "DianjuReader",
        "cn.lanxin": "lanxin",
        "parole": "org.xfce.Parole",
        "isignaturechina": "isignature",
        "ffado-mixer-qt4": "ffado.org-ffadomixer",
        "umbrello": "org.kde.umbrello",
        "kig": "org.kde.kig",
        "krdc": "org.kde.krdc",
        "kexi": "org.kde.kexi-3.1",
        "krusader": "org.kde.krusader",
        "kbruch": "org.kde.kbruch",
        "calligrawords": "org.kde.calligrawords",
        "zescrow-client": "zEscrow",
        "okular": "org.kde.okular",
        "digikam": "org.kde.digikam",
        "acetoneiso": "AcetoneISO",
        "k3b": "org.kde.k3b",
        "showfoto": "org.kde.showfoto",
        "skanlite": "org.kde.skanlite",
        "kcachegrind": "org.kde.kcachegrind",
        "baobab": "org.gnome.baobab",
        "aisleriot": "sol",
        "sofa-apps": "sofa",
        "kubrick": "org.kde.kubrick",
        "kwrite": "org.kde.kwrite",
        "gupnp-tools": "gupnp-av-cp",
        "kturtle": "org.kde.kturtle",
        "vino": "vino-server",
        "yakuake": "org.kde.yakuake",
        "kompare": "org.kde.kompare",
        # "ktouch":"rg.kde.ktouch.desktop",
        "freeciv-client-gtk": "freeciv-gtk2",
        "owncloud-client": "owncloud",
        "qtsmbstatus-light": "qtsmbstatusl",
        "supertux": "supertux2",
        "kylin-log-viewer": "logview",
        "iagno": "org.gnome.Reversi",
        "kile": "org.kde.kile",
        "meld": "org.gnome.meld",
        "bpython": "org.bpython-interpreter.bpython",
        "spyder": "spyder3",
        "kdiff3": "org.kde.kdiff3",
        "poedit": "net.poedit.Poedit",
        "ghex": "org.gnome.GHex",
        "kdevelop": "org.kde.kdevelop",
        "d-feet": "org.gnome.dfeet",
        "gwenview": "org.kde.gwenview",
        "gthumb": "org.gnome.gThumb",
        "quodlibet": "io.github.quodlibet.QuodLibet",
        "lingot": "org.nongnu.lingot",
        "gnome-subtitles": "org.gnome.GnomeSubtitles",
        "devede": "devede_ng.py",
        "sound-juicer": "org.gnome.SoundJuicer",
        "goobox": "org.gnome.Goobox",
        "rosegarden": "com.rosegardenmusic.rosegarden",
        "songwrite": "songwrite3",
        "qbittorrent": "org.qbittorrent.qBittorrent",
        "remmina": "org.remmina.Remmina",
        "midori": "org.midori_browser.Midori",
        "geary": "org.gnome.Geary",
        "ktouch": "org.kde.ktouch",
        "wordnet-gui": "wordnet",
        "wxmaxima": "io.github.wxmaxima_developers.wxMaxima",
        "file-roller": "org.gnome.FileRoller",
        "rsibreak": "org.kde.rsibreak",
        "catfish": "org.xfce.Catfish",
        "sweeper": "org.kde.sweeper",
        "blueman": "blueman-manager",
        "nautilus": "org.gnome.Nautilus",
        "bleachbit": "org.bleachbit.BleachBit",
        "meritous": "net.asceai.meritous",
        "lightsoff": "org.gnome.LightsOff",
        "gnome-mines": "org.gnome.Mines",
        "gnome-chess": "org.gnome.Chess",
        "gnome-tetravex": "org.gnome.Tetravex",
        "kylin-burner": "org.kde.kylin-burner",
        "recoll": "recoll-searchgui",
        "dingtalk": dingtalk

    }

    Globals.PKG_NAME = PKG_NAME
