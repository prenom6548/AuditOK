#!/usr/bin/python3


from enum import IntEnum
from sre_compile import isstring
import logging
import os
from configparser import DuplicateOptionError, ConfigParser
from RevisionsManager.Core.RMOsInfo import RMOsInfo

RMSUCCESS = 0
RMFAILURE = -1

SEC_DESKTOP = 'Desktop Entry'

def get_sys_call(cmd):
    if not isstring(cmd):
        logging.error("[Com]: cmd is not string")

    try:
        with os.popen(cmd) as fd:
            ret = fd.read()
        fd.close()
        logging.info("[Com]: sys call (%s) return[%s]", cmd, ret.strip("\n"))
    except Exception as e:
        logging.error("[Com]: [%s] failed as [%s].", cmd, e)

    return ret

def str_to_list(str, symbol):
    if not isstring(str):
        logging.warning("[Com]: str is not string")
        return None
    if symbol not in ["\n", "\t", "\'", "\"", " "]:
        logging.warning("[Com]: symbol[%s] is not included.", symbol)
        return None

    strlist = []
    strwhile = str
    strindex = strwhile.find(symbol)
    while strindex!= -1:
        strtmp = strwhile[:strindex]
        strwhile = strwhile[strindex+1:]
        strlist.append(strtmp)
        strindex = strwhile.find(symbol)

    return strlist

def get_desktop_key(filePath, key):
    value = None
    conf = ConfigParser()
    if not os.path.exists(filePath):
        logging.error("[Com]: %s not exist", filePath)
        return value
    try:
        conf.read(filePath, encoding="utf-8")
    except DuplicateOptionError:
        try:
            with open(filePath, 'r') as fd:
                infoList = fd.readlines()
            for ele in infoList:
                retList = ele.strip("\n").split("=")
                if len(retList) == 2 and retList[0] == key:
                    value = retList[1]
                    return value
        except Exception as e:
            logging.error("[Com]: err[%s]", e)
            return None
    except Exception as e:
        logging.error("[Com]: read %s failed err[%s]", filePath, e)
        return None

    if not conf.has_option(SEC_DESKTOP, key):
        logging.info("[Com]: There is not key[%s].", key)
        return None

    try:
        value = conf.get(SEC_DESKTOP, key)
        logging.info("[Com]: Get key[%s] value[%s]", key, value)
    except Exception as e:
        logging.error("[Com]: err[%s]", e)

    return value

def set_desktop_key(filePath, key, value):
    try:
        infoList = []
        with open(filePath, 'r') as fd_r:
            infoList = fd_r.readlines()
        for ele in infoList:
            retList = ele.strip("\n").split("=")
            if len(retList) == 2 and retList[0] == key:
                rowNew = key + "=" + value + "\n"
                infoList[infoList.index(ele)] = ele.replace(ele, rowNew)
                logging.info("[Com]: change [%s]", rowNew)
                break
        with open(filePath, 'w+') as fd_w:
            fd_w.writelines(infoList)
    except Exception as e:
        logging.error("[Com]: set key[%s] err[%s]", key, e)


def update_grub():
    logging.info("[Com]: update grub.")
    if os.path.exists("/usr/sbin/update-grub2"):
        os.system("update-grub2")
    elif os.path.exists("/usr/sbin/update-grub"):
        os.system("update-grub")
    else:
        logging.error("Base tools not exists [update-grub]")
        return RMFAILURE
    return RMSUCCESS


class RMCom():
    VMINIT = 0
    VMUPACT2 = 1
    VMUP = 2
    VMUPSUCCESS = 3
    VMUPFAILED = 4
    VMBACKACT2 = 5
    VMBACK = 6 
    VMBACKSUCCESS = 7
    VMBACKFAILED = 8

    intToStr = {
        VMINIT : "none",
        VMUPACT2 : "upact2",
        VMUP : "upover",
        VMUPSUCCESS : "upsuccess",
        VMUPFAILED : "upfailed",
        VMBACKACT2 : "backact2",
        VMBACK : "backover",
        VMBACKSUCCESS : "backsuccess",
        VMBACKFAILED : "backfailed",
    }

    strToInt = {
        "none" : VMINIT,
        "upact2" : VMUPACT2,
        "upover" : VMUP,
        "upsuccess" : VMUPSUCCESS,
        "upfailed" : VMUPFAILED,
        "backact2" : VMBACKACT2,
        "backover" : VMBACK,
        "backsuccess" : VMBACKSUCCESS,
        "backfailed" : VMBACKFAILED,
    }

    SPACE_G = 1024*1024*1024

    # state sec/key
    SEC_HW_INFO     = 'hw_info'
    KEY_ARCH        = 'arch'
    SEC_HOME_INFO   = 'home_info'
    KEY_USERS       = 'userlist'
    KEY_ACT_USER    = 'active_user'
    KEY_KERNEL      = 'kerversion'
    KEY_FS_NAME     = 'oldfsname'
    SEC_TIME        = 'time'
    KEY_UPTIEM      = 'uptime'
    SEC_STATE       = 'state'
    KEY_VERSIONMANAGER = 'versionmanager'
    SEC_RUNNING_OS_INFO    = 'running_os'
    SEC_OLD_OS_INFO = 'old_os'
    SEC_NEW_OS_INFO = 'new_os'

    ARCHLIST                = [['arm','arm64','aarch64'],['amd','x86','x86_64','i386'],['armhf'],['loongarch64'],['mips64el','mips64'],['sw64']]

    CACHE_DIR               = "/var/cache/RevisionsManager/"
    ETC_DIR                 = "/etc/RevisionsManager/"
    RM_STATE_FIEL           = "RevisionsManager.state"
    RM_STATE_PATH           = ETC_DIR + RM_STATE_FIEL
    CONF_FILE               = "RevisionsManager.conf"
    CONF_PATH               = ETC_DIR + CONF_FILE
    MD5_FILE                = "RevisionsManager.md5sum"
    MD5_PATH                = ETC_DIR + MD5_FILE

    NEWFS_DIR               = "/fs.new/"
    GRUB_FILE_PATH          = "/etc/grub.d/10_linux"

    APPDESKTOP_DIR          = "/usr/share/applications/"
    LOG_DIR                 = "/var/log/RevisionsManager/"
    SCRIPTS_DIR             = "/usr/share/revisions-manager/RevisionsManager/scripts/"

    SCRIPT_POSTISNTALL      = "postinstall"
    # 安全相关 kysec pam
    SCRIPT_SECURITY         = "security"
    CMD_AUDIT               = "open_audit.sh"

    KEY_ZH_CN_NAME          = 'Name[zh_CN]'
    KEY_NAME                = 'Name'
    KEY_ICON_PATH           = 'Icon'

    def _final_fs_name(self):
        postfix = ''
        osInfoHD   = RMOsInfo()
        osName = osInfoHD.get_pretty_name()
        if "ubuntu" in osName or "Ubuntu" in osName:
            tmpList = osName.split()
            if len(tmpList) >= 2:
                postfix ='-u' + tmpList[1].split(".")[0]
            else:
                postfix = '-u'
        elif "openKylin" in osName:
            tmpList = osName.split()
            if len(tmpList) >= 2:
                postfix ='-ok' + tmpList[1].split(".")[0]
            else:
                postfix = '-ok'
        elif "Kylin V10 SP1" in osName:
            postfix = '-v10sp1'
        elif "Kylin V10" in osName:
            postfix = '-v10'
        elif "Kylin V4" in osName:
            postfix = '-v4'

        self.ORGFS_DIR  = "/fs.old" + postfix
        logging.info("[RMCOM]: get pretty name[%s] oldfs[%s].", osName, self.ORGFS_DIR)
    
    def __init__(self):
        self.ORGFS_DIR  = "/fs.old"
        self._final_fs_name()


if __name__ == "__main__":

    comClass = RMCom()

    path = "/usr/share/applications/cn.lanxin.desktop"
    value = get_desktop_key(path, "Icon")
    print(value)
    set_desktop_key(path, "Icon", "aaaa")

    pass
