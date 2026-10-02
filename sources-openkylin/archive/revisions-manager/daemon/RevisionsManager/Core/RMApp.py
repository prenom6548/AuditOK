#!/usr/bin/python3

# .appClassification.yaml

import subprocess
import logging
import os
import shutil
import copy
import pathlib
import re
from apt import Cache
import yaml
from RevisionsManager.Core.RMCom import RMCom,RMSUCCESS, RMFAILURE, get_sys_call, get_desktop_key, set_desktop_key
from RevisionsManager.Core.RMConf import RMConf




class RMApp(RMCom):
    PkgInfoIndex_Name = 0
    PkgInfoIndex_CnName = 1
    PkgInfoIndex_DesktopName = 2

    DEB_PATH                = RMCom.CACHE_DIR + "archives/"       # 该目录会在创建deb时重新生成, 存放deb文件
    DESKTOP_BACK_PATH       = RMCom.CACHE_DIR + "applications/"   # 原系统桌面快捷方式备份路径
    DESKTOP_LIST_PATH_CN    = RMCom.CACHE_DIR + "desktopList_CN"  # 原系统桌面快捷方式列表
    DESKTOP_LIST_PATH_EN    = RMCom.CACHE_DIR + "desktopList_EN"  # 原系统桌面快捷方式列表
    RMAPP_DIR               = "/opt/RevisionsManager/"      # 只放revisions-manager deb文件, 和其他记录文件
    RMAPP_NEW_DIR           = "/fs.new" + RMAPP_DIR
    APPCLASSIFICATION_PATH  = "/usr/share/revisions-manager/data/.appClassification.yaml"
    AWOID_PKG_NAME_LIST     = ["ukui", "kylin", "gnome", "mate"]
    REPKG_LIST_FILE         = "repkgList"
    REPKG_LIST_FILE_EX      = "repkgListEx"
    COMPAT_DESKTOP_LIST     = "compat-desktops"
    NOT_COMPAT_LIST         = "not-compat"
    NEWOS_INSTALL_SH        = "handle_compat_depend.sh"
    

    def __init__(self) -> None:
        self.checkedList    = []        # 已经识别过的desktop列表
        self.repkgInfoList  = []        # 需要兼容列表
        self.repkgInfoListEx= []        # 设置后的重打包列表
        self.DesktopList    = []        # 迁移应用的desktop文件列表
        self.appDict        = {}        # 应用迁移列表
        self.compatAwoidList = []       # 不兼容的列表

        # 获取应用迁移配置列表
        if not os.path.exists(self.APPCLASSIFICATION_PATH):
            logging.error("[RMApp]: [%s] not exists", self.APPCLASSIFICATION_PATH)
        else:                   
            with open(self.APPCLASSIFICATION_PATH, 'r') as appfile:
                self.appDict = yaml.load(appfile, Loader=yaml.FullLoader)
            appfile.close()
        
        # 自包含转换配置开关
        self.RMconf = RMConf(self.CONF_PATH)
        self.stateConf = RMConf(self.RM_STATE_PATH)
        # self.actUser = self.stateConf.get_key(self.SEC_HOME_INFO, self.KEY_ACT_USER)

        flag = self.RMconf.get_key(self.RMconf.SEC_SWITCH, self.RMconf.KEY_SW_APP)
        if flag in ["FALSE", "false", "False", "0"]:
            self.appMoveFlag = False
        elif flag in ["TRUE", "true", "True", "1"]:
            self.appMoveFlag = True
        else:
            self.appMoveFlag = False

    def check_install_state(self):
        return True, []

    def _get_desktop_zh_CN_name(self, desktopPath, pkgName):
        pkgCNName = None
        pkgCNName = get_desktop_key(desktopPath, self.KEY_ZH_CN_NAME)
        if pkgCNName is None:
            pkgCNName = get_desktop_key(desktopPath, self.KEY_NAME)
        
        if pkgName == "wps-office":
            pkgCNName = "WPS 办公软件"
        return pkgCNName
    
    def _get_pkgName_from_desktop(self, desktop):
        pkgName = ""
        try:
            # 避免desktop文件是通过脚本移动到'/usr/share/applications/',
            # 该情况dpkg -S /usr/share/applications/desktop 找不到对应软件包
            # 直接使用 dpkg -S desktop 会出现，结果中存在多个包的情况(eg: code.desktop)
            cmd = "dpkg -S applications/" + desktop
            ret = subprocess.run(cmd, shell=True, stdout=subprocess.PIPE)
            if ret.returncode != RMSUCCESS:
                cmd = "dpkg -S " + desktop
                ret = subprocess.run(cmd, shell=True, stdout=subprocess.PIPE)
                if ret.returncode != RMSUCCESS:
                    logging.error("[RMApp]: desktop[%s] find pkg failed [%s].",desktop, ret.stdout.decode())
                    return None

            tmp = ret.stdout.decode().split(":", maxsplit=1)
            if len(tmp) > 0:
                pkgName = tmp[0]
                return pkgName
            else:
                logging.error("[RMApp]: desktop[%s] package[%s] not right.", desktop, tmp)
                return None
        except Exception as e:
            logging.error("[RMApp]: *err[%s] occur.",e)
            return None

    def find_repack_list(self):
        logging.info("[RMApp]: find repack list start")

        self.aptCache = Cache()
        if os.path.exists(self.APPDESKTOP_DIR):
            allFile = os.listdir(self.APPDESKTOP_DIR)
            for line in allFile:
                if not line.endswith(".desktop"):
                    continue

                # 重复 desktop 过滤
                if line in self.checkedList:
                    logging.info("[RMApp]: [%s] has been in List.",line)
                    continue

                # desktop 过滤
                if 'awoid-desktop' in self.appDict.keys() and self.appDict['awoid-desktop'] is not None and line in self.appDict['awoid-desktop']:
                    self.checkedList.append(line)
                    continue

                pkgName = self._get_pkgName_from_desktop(line)
                if pkgName == None:
                    continue

                # 包名过滤
                isawoid = False
                for ele in self.AWOID_PKG_NAME_LIST:
                    if ele in pkgName:
                        self.checkedList.append(line)
                        isawoid = True
                        break
                if isawoid:
                    continue

                if 'compat-awoid-desktop' in self.appDict.keys() and self.appDict['compat-awoid-desktop'] is not None and line in self.appDict['compat-awoid-desktop']:
                    self.checkedList.append(line)
                    self.compatAwoidList.append(pkgName)
                    continue
                if 'compat-awoid-pkg' in self.appDict.keys() and self.appDict['compat-awoid-pkg'] is not None and pkgName in self.appDict['compat-awoid-pkg']:
                    self.checkedList.append(line)
                    self.compatAwoidList.append(pkgName)
                    continue

                if 'awoid-pkg' in self.appDict.keys() and self.appDict['awoid-pkg'] is not None and pkgName in self.appDict['awoid-pkg']:
                    self.checkedList.append(line)
                    continue

                logging.info("[RMApp]: [%s][%s] will repack.",pkgName, line)
                desktopPath = os.path.join(self.APPDESKTOP_DIR, line)
                pkgCNName = self._get_desktop_zh_CN_name(desktopPath, pkgName)
                if pkgCNName is None:
                    pkgCNName = pkgName

                pkgInfo=[]
                pkgInfo.insert(self.PkgInfoIndex_Name, pkgName)
                pkgInfo.insert(self.PkgInfoIndex_CnName, pkgCNName)
                pkgInfo.insert(self.PkgInfoIndex_DesktopName, line)

                self.repkgInfoList.append(pkgInfo)
                self.checkedList.append(line)
        else:
            logging.error("[RMApp]: where are Apps desktop")

        if not os.path.exists(self.RMAPP_DIR):
            os.mkdir(self.RMAPP_DIR, mode=0o755)
        path = os.path.join(self.RMAPP_DIR, self.REPKG_LIST_FILE)
        with open(path, 'w+', encoding="utf-8") as fd:
            for ele in self.repkgInfoList:
                fd.write(ele[self.PkgInfoIndex_Name])
                fd.write("\n")
        fd.close()

        pathEx = os.path.join(self.RMAPP_DIR, self.REPKG_LIST_FILE_EX)

        if len(self.repkgInfoListEx) == 0:
            self.repkgInfoListEx = copy.deepcopy(self.repkgInfoList)
        if not self.appMoveFlag:
            self.repkgInfoListEx = []

        with open(pathEx, 'w+', encoding="utf-8") as fdEx:
            for ele in self.repkgInfoListEx:
                fdEx.write(ele[self.PkgInfoIndex_Name])
                fdEx.write("\n")
        fdEx.close()
        if not os.path.exists(self.CACHE_DIR):
            os.mkdir(self.CACHE_DIR, mode=0o755)
        pathDesktop = os.path.join(self.CACHE_DIR, self.COMPAT_DESKTOP_LIST)
        with open(pathDesktop, 'w+', encoding="utf-8") as fdDt:
            for ele in self.repkgInfoListEx:
                fdDt.write(ele[self.PkgInfoIndex_DesktopName])
                fdDt.write("\n")
        fdDt.close()

        pathAwoid = os.path.join(self.CACHE_DIR, self.NOT_COMPAT_LIST)
        with open(pathAwoid, 'w+', encoding="utf-8") as fdAw:
            for ele in self.compatAwoidList:
                fdAw.write(ele)
                fdAw.write("\n")
        fdAw.close()

        logging.info("[RMApp]: find repack list finished.")

    # 从dpkg-repack命令获取deb全名
    def _get_deb_name_from_ret(self, str):
        if len(str) == 0:
            logging.error("[RMApp]: Length of In str is 0.")
            return None
        
        eleList = str.split("'")
        for ele in eleList:
            if ".deb" in ele:
                start = ele.rfind("/")
                end = len(ele)
                return ele[start+1:end]

        return None

    def create_new_deb(self):
        logging.info("[RMApp]: create new deb start")
        if not os.path.exists(self.RMAPP_NEW_DIR):
            os.makedirs(self.RMAPP_NEW_DIR, mode=0o755)

        curPath = os.getcwd()
        os.chdir(self.RMAPP_DIR)
    
        cmd = "dpkg-repack kylin-revision-manager"
        ret = subprocess.run(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        if ret.returncode != RMSUCCESS or len(ret.stdout) == 0:
            logging.warning("[RMApp]: kylin-revision-manager repack failed[%s].", ret.stderr.decode())
            return RMFAILURE
        debName = self._get_deb_name_from_ret(ret.stdout.decode())
        distPath = os.path.join(self.RMAPP_NEW_DIR, debName)
        shutil.move(debName, distPath)
        logging.info("[RMApp]: move [%s] to [%s]", debName, distPath)
        os.chdir(curPath)

        logging.info("[RMApp]: create new deb end")

    def get_app_list(self):
        pkgNameList = []

        if not "LANGUAGE" in os.environ:
            os.environ["LANGUAGE"] = "zh_CN"

        if os.environ["LANGUAGE"] == "zh_CN":
            for ele in self.repkgInfoList:
                if ele[self.PkgInfoIndex_CnName] is None:
                    pkgNameList.append(ele[self.PkgInfoIndex_Name])
                else:
                    pkgNameList.append(ele[self.PkgInfoIndex_CnName])
        else:
            for ele in self.repkgInfoList:
                pkgNameList.append(ele[self.PkgInfoIndex_Name])

        tmpList = copy.deepcopy(pkgNameList)
        for ele in tmpList:
            if pkgNameList.count(ele) > 1:
                pkgNameList.remove(ele)

        return pkgNameList

    # appList 不迁移的列表
    def set_app_list(self, appList):
        if appList is None or len(appList) == 0:
            logging.error("[RMApp]: App list is none.")
            return RMFAILURE

        self.repkgInfoListEx = copy.deepcopy(self.repkgInfoList)
        for ele_i in appList:
            for ele_j in self.repkgInfoList:
                if ele_i in ele_j:
                    self.repkgInfoListEx.remove(ele_j)

        if not os.path.exists(self.RMAPP_DIR):
            os.mkdir(self.RMAPP_DIR, mode=0o755)
        path = os.path.join(self.RMAPP_DIR, self.REPKG_LIST_FILE_EX)
        with open(path, 'w+', encoding="utf-8") as fd:
            for ele in self.repkgInfoListEx:
                fd.write(ele[self.PkgInfoIndex_Name])
                fd.write("\n")
        fd.close()

        if not os.path.exists(self.CACHE_DIR):
            os.mkdir(self.CACHE_DIR, mode=0o755)
        pathDesktop = os.path.join(self.CACHE_DIR, self.COMPAT_DESKTOP_LIST)
        with open(pathDesktop, 'w+', encoding="utf-8") as fdDt:
            for ele in self.repkgInfoListEx:
                fdDt.write(ele[self.PkgInfoIndex_DesktopName])
                fdDt.write("\n")
        fdDt.close()

    def clear_move_app_list(self):
        self.repkgInfoListEx = []


    # 修改指定路径下的指定desktop列表中icon路径
    def _change_icon_path(self, dir, desktopList):
        if not os.path.exists(dir):
            logging.error("[RMApp]: %s not exists.", dir)
            return

        changeList = [] #desktop列表
        if desktopList == None or len(desktopList) == 0:
            fileList = os.listdir(dir)
            for everyfile in fileList:
                if everyfile.endswith(".desktop"):
                    changeList.append(everyfile)
        else:
            changeList = copy.deepcopy(desktopList)

        # 绝对路径 直接修改为/fs.old-xxx/xxxx
        # 文件名   在fs.old-xxx找到，拷贝到新系统
        for ele in changeList:
            desktopPath = os.path.join(dir,ele)
            iconPath = get_desktop_key(desktopPath, self.KEY_ICON_PATH)
            oldfs = self.stateConf.get_key(self.SEC_OLD_OS_INFO, self.KEY_FS_NAME)

            if iconPath == None:
                logging.debug("[RMApp]: %s no icon.", desktopPath)
                continue

            if iconPath.startswith("/"):
                oldIconPath = oldfs+ iconPath
                set_desktop_key(desktopPath, self.KEY_ICON_PATH, oldIconPath)
            else:
                iconDir = "/usr/share/icons/"
                subIconDirList = os.listdir(iconDir)
                distDir = "/usr/share/icons/hicolor"
                walkPath = oldfs + "/usr/share/icons/"
                pLib = pathlib.Path(walkPath)
                retList = pLib.rglob(iconPath + "*") #fs.old-xxx中icon路径
                parentPathCount = 5

                for everyRet in retList:
                    mark = 0
                    distPath = "" # icon 目标路径
                    if len(everyRet.parts) <= parentPathCount:
                        logging.info("[RMApp]: path len less 5.")
                        continue
                    else:
                        logging.info("[RMApp]: path len is %d.", len(everyRet.parts))

                    try:
                        if everyRet.parts[parentPathCount] in subIconDirList or len(everyRet.parts) == parentPathCount + 1:
                            # 存在对应的文件夹，直接拷贝
                            # 是文件
                            distPath = str(everyRet).replace(oldfs, "")
                            cmd = "cp -rp " + str(everyRet) + " " + distPath
                            os.system(cmd)
                            logging.debug("[RMApp]: icona copy %s to %s.", str(everyRet), distPath)
                            continue
                        else:
                            logging.info("[RMApp]: path is %d.", everyRet.parts[parentPathCount])

                        # 不存在对应文件夹，拷贝到 hicolor
                        for i in range(parentPathCount,len(everyRet.parts)):
                            logging.info("[RMApp]: every part is %s.",everyRet.parts[i])
                            pattern = r'^\d+x\d+$'
                            if re.match(pattern, everyRet.parts[i]):
                                mark = i
                                break
                        if mark < parentPathCount:
                            tmpMark = everyRet.parts[parentPathCount]
                            distPatha = str(everyRet).replace(oldfs, "")
                            distPathb = distPatha.replace(tmpMark, "hicolor")
                            logging.debug("[RMApp]: iconb copy %s to %s.", str(everyRet), distPathb)
                            cmd = "cp -rp " + str(everyRet) + " " + distPath
                            os.system(cmd)
                        else:
                            distPath = distDir
                            for j in range(mark,len(everyRet.parts)):
                                #logging.debug("[RMApp]: tmp [%s]  [%s].", distPath, everyRet.parts[j])
                                distPath = distPath + "/" + everyRet.parts[j]                           
                            logging.debug("[RMApp]: iconc mark[%d] copy %s to %s.", mark, str(everyRet), distPath)
                            cmd = "cp -rp " + str(everyRet) + " " + distPath
                            os.system(cmd)
                    except Exception as e:
                        logging.error("[RMApp]: err[%s].", e)


    # 桌面上desktop 不在迁移列表的清除
    # 桌面上desktop 在迁移列表的 使用/usr/share/applications的更新
    def _update_desktop(self, desktopDir):
        if not os.path.exists(desktopDir):
            logging.error("[RMApp]: %s not exists.", desktopDir)
            return

        compatDesktopPath = self.CACHE_DIR+ self.COMPAT_DESKTOP_LIST
        if not os.path.exists(compatDesktopPath):
            logging.error("[RMApp]: %s not exists.", compatDesktopPath)
            return
        with open(compatDesktopPath, "r") as fd:
            desktopList = fd.readlines()

        fileList = os.listdir(desktopDir)
        if fileList == None or len(fileList) == 0:
            logging.error("[RMApp]: no desktop file.")
            return

        for ele in fileList:
            if not ele.endswith(".desktop"):
                logging.debug("[RMApp]: %s is not desktop.", ele)
                continue
            if not os.path.isfile(desktopDir+ele):
                logging.debug("[RMApp]: %s is not file.", ele)
                continue
            if ele == "RevisionsManager.desktop":
                continue
            try:
                os.remove(desktopDir + ele)
                if ele + "\n" in desktopList:
                    logging.debug("[RMApp]: copy %s start.", ele)
                    shutil.copy2(self.APPDESKTOP_DIR + ele , desktopDir + ele)
                    cmd = "chmod +755 " + desktopDir + ele
                    get_sys_call(cmd)
                    logging.debug("[RMApp]: copy %s end.", ele)
            except Exception as e:
                logging.error("[RMApp]: update desktop failed[%s].",e)

    # 将迁移应用的desktop文件 拷贝到新系统
    # 修改icon路径
    def _move_desktop_to_newOs(self):
        path = self.CACHE_DIR + self.COMPAT_DESKTOP_LIST
        if not os.path.exists(path):
            logging.error("[RMApp]: %s not exists.", path)
            return
        
        with open(path, "r") as fd:
            desktopList = fd.readlines()
        oldfs = self.stateConf.get_key(self.SEC_OLD_OS_INFO, self.KEY_FS_NAME)
        tmpDesktopList = []
        for ele in desktopList:
            try:
                srcPath = oldfs + self.APPDESKTOP_DIR + ele.strip("\n")
                if os.path.islink(srcPath):
                    truePath = os.readlink(srcPath)
                    srcPath = oldfs + truePath
                    logging.debug("[RMApp]: %s is link.", srcPath)
                elif not os.path.exists(srcPath):
                    logging.error("[RMApp]: desktop %s not exists.", srcPath)
                    continue

                tmpDesktopList.append(ele.strip("\n"))
                disPath = self.APPDESKTOP_DIR + ele.strip("\n")
                cmd = "cp -rp " + srcPath + " " + disPath
                os.system(cmd)
                logging.info("[RMApp]: copy %s to %s.", srcPath, disPath)

            except Exception as e:
                logging.error("[RMApp]: move desktop failed[%s].",e)

        self._change_icon_path(self.APPDESKTOP_DIR, tmpDesktopList)


    # 升级成功后, 相关处理
    # 1、安装兼容环境
    # 2、图标迁移到新系统(/usr/share/applications/)
    #       1.icon 迁移到 /usr/share/icons/hicolor
    #       2.修改迁移的桌面图标的icon路径
    # 3、更新桌面快捷方式
    #       1.移除多余快捷方式
    #       2.迁移应用的desktop文件，使用步骤2中的
    def install_third_package(self):
        logging.info("[RMApp]: handle app compat start.")
        handleShell = self.SCRIPTS_DIR + self.NEWOS_INSTALL_SH
        if os.path.exists(handleShell):
            cmd = "bash " + handleShell
            get_sys_call(cmd)

        userName = self.stateConf.get_key(self.SEC_HOME_INFO, self.KEY_ACT_USER)
        if userName == None:
            userName = self.actUser
            if userName == None:
                logging.error("[RMApp]: can't get active user.")
                return
            logging.warning("[RMApp]: use user[%s] not from state conf.", userName)
        
        if not os.path.exists(self.CACHE_DIR):
            os.makedirs(self.CACHE_DIR, mode=0o755)
        oldfs = self.stateConf.get_key(self.SEC_OLD_OS_INFO, self.KEY_FS_NAME)
        srcPath = oldfs + self.CACHE_DIR + self.COMPAT_DESKTOP_LIST
        cmd = "cp " + srcPath + " " + self.CACHE_DIR
        get_sys_call(cmd)

        # 图标迁移到新系统
        self._move_desktop_to_newOs()

        disPath1 = "/home/"+userName+"/桌面/"
        disPath2 = "/home/"+userName+"/Desktop/"
        if os.path.exists(disPath1):
            # 更新快捷方式
            self._update_desktop(disPath1)
        elif os.path.exists(disPath2):
            self._update_desktop(disPath2)

        logging.info("[RMApp]: handle app compat end.")

    def backup_desktop(self):
        userName = self.stateConf.get_key(self.SEC_HOME_INFO, self.KEY_ACT_USER)
        if userName == None:
            userName = self.actUser
            if userName == None:
                logging.error("[RMApp]: can't get active user.")
                return
            logging.warning("[RMApp]: use user[%s] not from state conf.", userName)
        
        if os.path.exists(self.DESKTOP_BACK_PATH):
            cmd = "rm -r " + self.DESKTOP_BACK_PATH
            get_sys_call(cmd)
        os.makedirs(self.DESKTOP_BACK_PATH, mode=0o755)

        disPath1 = "/home/"+userName+"/桌面/"
        disPath2 = "/home/"+userName+"/Desktop/"
        if os.path.exists(disPath1):
            cmd = "cp -p " + disPath1 + "*.desktop" + " " + self.DESKTOP_BACK_PATH
            get_sys_call(cmd)
            fileList = os.listdir(disPath1)
            with open(self.DESKTOP_LIST_PATH_CN, 'w+') as fd:
                for ele in fileList:
                    if ele.endswith(".desktop"):
                        fd.write(ele)
                        fd.write("\n")

        elif os.path.exists(disPath2):
            cmd = "cp -p " + disPath2 + "*.desktop" + " " + self.DESKTOP_BACK_PATH
            get_sys_call(cmd)
            fileList = os.listdir(disPath2)
            with open(self.DESKTOP_LIST_PATH_EN, 'w+') as fd:
                for ele in fileList:
                    if ele.endswith(".desktop"):
                        fd.write(ele)
                        fd.write("\n")

    def resume_desktop(self):
        if not os.path.exists(self.DESKTOP_BACK_PATH):
            logging.debug("[RMApp]: path[%s] not exists.", self.DESKTOP_BACK_PATH)
            return
        
        userName = self.stateConf.get_key(self.SEC_HOME_INFO, self.KEY_ACT_USER)
        if userName == None:
            userName = self.actUser
            if userName == None:
                logging.error("[RMApp]: can't get active user.")
                return
            logging.warning("[RMApp]: use user[%s] not from state conf.", userName)
        
        disPath1 = "/home/"+userName+"/桌面/"
        disPath2 = "/home/"+userName+"/Desktop/"
        if os.path.exists(disPath1):
            cmd = "rm " + disPath1 + "*.desktop"
            get_sys_call(cmd)
            cmd = "cp " + self.DESKTOP_BACK_PATH + "*.desktop" + " " + disPath1
            get_sys_call(cmd)
            cmd = "chown " + userName + ":" + userName + " " + disPath1 + "*.desktop"
            get_sys_call(cmd)
        elif os.path.exists(disPath2):
            cmd = "rm " + disPath2 + "*.desktop"
            get_sys_call(cmd)
            cmd = "cp " + self.DESKTOP_BACK_PATH + "*.desktop" + " " + disPath2
            get_sys_call(cmd)
            cmd = "chown " + userName + ":" + userName + " " + disPath1 + "*.desktop"
            get_sys_call(cmd)

    def enable_app_move(self):
        self.appMoveFlag = True
        self.RMconf.change_key_value(self.RMconf.SEC_SWITCH, self.RMconf.KEY_SW_APP, "True")

    def disable_app_move(self):
        self.appMoveFlag = False
        self.RMconf.change_key_value(self.RMconf.SEC_SWITCH, self.RMconf.KEY_SW_APP, "False")

    def install_revisions_manager(self):
        if not os.path.exists(self.RMAPP_DIR):
            logging.error("[RMApp]: [%s] not exists.", self.RMAPP_DIR)
            return RMFAILURE

        cmd = "dpkg -i " + self.RMAPP_DIR + "kylin-revision-manager*.deb"
        ret = subprocess.run(cmd, shell=True, stdout=subprocess.PIPE)
        logging.info("[RMApp]: install itself [%s].", ret.stdout.decode())
        return ret.returncode

    def get_not_compat_package(self):
        pkgList = []
        oldfs = self.stateConf.get_key(self.SEC_OLD_OS_INFO, self.KEY_FS_NAME)
        path = oldfs + self.CACHE_DIR + self.NOT_COMPAT_LIST
        if os.path.exists(path):
            with open(path, "r") as fd:
                tmpList = fd.readlines()

        for ele in tmpList:
            pkgList.append(ele.strip("\n"))

        return pkgList

    def get_app_move(self):
        return self.RMconf.get_key(self.RMconf.SEC_SWITCH, self.RMconf.KEY_SW_APP)


if __name__ == "__main__":
    rmApp = RMApp()
    # rmApp.find_repack_list()
    rmApp.backup_desktop()
    # rmApp.create_new_deb()
    # ret, list = rmApp.check_install_state()
    exit(0)
