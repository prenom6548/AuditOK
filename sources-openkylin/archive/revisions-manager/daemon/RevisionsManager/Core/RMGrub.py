#!/usr/bin/python3

import os
import logging

from RevisionsManager.Core.RMCom import RMCom, RMSUCCESS, RMFAILURE,get_sys_call,str_to_list

class GrubMenuClass():
    def __init__(self):
        self.menuName = None
        self.menuStartLine = 0
        self.insmodList = []
        self.linuxList = []
        self.initrdList = []
        self.paramList = []
        self.isboot = False

    def analysis_menu(self, blockList, startLine):
        self.menuStartLine = startLine
        self.menuBlockList = blockList

        for line in blockList:
            partList = line.split(" ")
            if "menuentry" in line:
                menuTitle = line.split("'")
                self.menuName = menuTitle[1]
                continue

            if "insmod" in line and len(partList) == 2:
                modname = partList[1]
                self.insmodList.append(modname)
                continue

            if "linux" in line and "root=UUID=" in line:
                self.linuxList = partList
                continue
            
            if "initrd.img" in line:
                self.initrdList = partList
                continue

            if line.split() == ['boot']:
                self.isboot = True
                continue

            if "echo" not in line and "{" not in line and "}" not in line:
                self.paramList.append(line)
                continue



class GrubsubMenuClass(GrubMenuClass):
    def __init__(self):
        self.subMenuName = None
        self.subMenuStartLine = 0
        self.isSubMenu = False
        self.menuList = []

    def analysis_subMenu(self, blockList, startLine):
        self.subMenuStartLine = startLine
        self.subMenuBlockList = blockList

        if "submenu" in blockList[0]:
            self.isSubMenu = True
            subMenuTitle = blockList[0].split("'")
            self.subMenuName = subMenuTitle[1]

        menuStartFlag = 0
        menuStart = 0
        for i in range(len(blockList)):

            if "menuentry" in blockList[i] and "submenu" not in blockList[i]:
                if menuStartFlag == 0:
                    menuStart = i + self.subMenuStartLine
                menuStartFlag += 1
                continue
                
            if "}" in blockList[i] and menuStartFlag > 0:
                menuStartFlag -= 1
                if menuStartFlag == 0:
                    menuClass = GrubMenuClass()
                    menuClass.analysis_menu(blockList[(menuStart-self.subMenuStartLine):i+1], menuStart)
                    self.menuList.append(menuClass)
                continue

class GrubFileClass(GrubsubMenuClass):
    def __init__(self) -> None:

        self.fileName = None
        self.fileStartLine = 0
        self.fileEndLine = 0
        self.firstSubMenuStart = 0
        self.isParam = True
        self.subMenuList = []
        self.otherList = []

    # blockList 包括 BIGIN -- END
    def analysis_file(self, blockList, startLine):
        self.fileStartLine = startLine
        self.fileEndLine = startLine + len(blockList) - 1
        self.firstSubMenuStart = startLine
        subStartFlag = 0
        subStart = 0

        titleList = blockList[0].split(" ")
        self.fileName = titleList[2]

        del blockList[0]

        for i in range(len(blockList)):
            if "menuentry" in blockList[i] or "submenu" in blockList[i]:
                self.isParam = False
                # 记录第一个menu的开始位置
                if self.firstSubMenuStart == startLine:
                    self.firstSubMenuStart = i + startLine

            if self.isParam:
                self.otherList.append(blockList[i])
                continue

            if "{" in blockList[i]:
                if subStartFlag == 0:
                    subStart = i + startLine
                subStartFlag += 1
                continue

            if "}" in blockList[i] and subStartFlag > 0:
                subStartFlag -= 1
                if subStartFlag == 0:
                    subClass = GrubsubMenuClass()
                    subClass.analysis_subMenu(blockList[(subStart-startLine) : (i+1)], subStart)
                    self.subMenuList.append(subClass)
                continue
        
        logging.info("[Grub]: analysis file[%s] finished.", self.fileName)



class RMGrub(GrubFileClass, RMCom):

    def __init__(self) -> None:
        self.grubCfgPath = "/boot/grub/grub.cfg"
        self.fileList = []
        self.rootUUID = None
        self.bootUUID = None
        cmd = "archdetect"
        arch = get_sys_call(cmd).strip("\n")
        self.arch = arch
        if arch == "arm64/efi":
            self.grubCfgPath = "/boot/efi/boot/grub/grub.cfg"
        elif arch == "loongarch64/generic":
            self.grubCfgPath = "/boot/efi/boot/EFI/grub.cfg"
        elif arch == "mips64el/generic" or arch == "mips64el/loongson-3":
            self.grubCfgPath = "/boot/EFI/BOOT/grub.cfg"
        else:
            self.grubCfgPath = "/boot/grub/grub.cfg"

        if not os.path.exists(self.grubCfgPath):
            logging.error("[Grub]: arch[%s] grub[%s] not exist.", arch, self.grubCfgPath)


    def analysis_cfg(self, grubCfgPath):
        with open(grubCfgPath, 'r') as fd_r:
            grubfdr = fd_r.readlines()
        fd_r.close()

        fileStartFlag = 0
        for line in grubfdr:
            if "### BEGIN" in line:
                fileStart = grubfdr.index(line)
                fileStartFlag += 1
                continue

            if "### END" in line and fileStartFlag > 0:
                fileStartFlag -= 1
                fileClass = GrubFileClass()
                fileClass.analysis_file(grubfdr[fileStart:grubfdr.index(line)+1], fileStart)
                self.fileList.append(fileClass)
        logging.info("[Grub]: analysis_cfg finished.")

    # TODO:未考虑多系统情况
    def get_UUID(self):
        mountinfo = get_sys_call("mount | grep /dev/")
        mountList = str_to_list(mountinfo, "\n")
        rootDev = None
        rootUuid = None
        bootDev = None
        bootUuid = None
        for line in mountList:
            if "on / " in line:
                rootDev = line.split(" ")[0]
                continue
            if "on /boot " in line:
                bootDev = line.split(" ")[0]
                continue
        
        if rootDev is None:
            logging.error("[Grub]: Can not find root dev")
            return RMFAILURE
        else:
            cmdRet = get_sys_call("blkid -s UUID " + rootDev)
            rootUuid = cmdRet.split("UUID=\"")[1]
            logging.info("[Grub]: root UUID is [%s].",rootUuid.strip("\"\n"))
            self.rootUUID = rootUuid.strip("\"\n")

        if bootDev is None:
            logging.error("[Grub]: Can not find boot dev")
            bootDev = rootDev
            self.bootUUID = self.rootUUID

        else:
            cmdRet = get_sys_call("blkid -s UUID " + bootDev)
            bootUuid = cmdRet.split("UUID=\"")[1]
            logging.info("[Grub]: boot UUID is [%s].",bootUuid.strip("\"\n"))
            self.bootUUID = bootUuid.strip("\"\n")

        logging.info("[Grub]: get boot[%s] root[%s].", self.bootUUID, self.rootUUID)
        return RMSUCCESS


    # TODO: 目前只取第一个menu中的
    def _get_menu_insmod(self, fileName):
        for file in self.fileList:
            if fileName != file.fileName:
                continue
            subMenuList = file.subMenuList
            if len(subMenuList) == 0:
                logging.error("[grub]:there is no submenu")
                return None

            menuList = subMenuList[0].menuList
            if len(menuList) == 0:
                logging.error("[Grub]: there is no menuList")
                return None
            return menuList[0].insmodList
        logging.error("[Grub]: Can't get insmod list")
        return None

    # 找到包含指定str和版本的文件
    def _find_specified_file(self, str, kerVersion, bootPath):
        fileList = os.listdir(bootPath)
        for file in fileList:
            if str in file and kerVersion in file:
                if self.bootUUID == self.rootUUID:
                    return bootPath + file
                else:
                    return "/" + file
        logging.info("[Grub]: Can't find file about [%s] and [%s].", str, kerVersion)
        return None

    def find_image(self, kerVersion, bootPath = "/boot/"):
        if "arm" in self.arch and "efi" in self.arch:
            imageFile = self._find_specified_file("vmlinuz", kerVersion, bootPath)
            if imageFile is None:
                imageFile = self._find_specified_file("Image", kerVersion, bootPath)
        elif "arm" in self.arch:
            imageFile = self._find_specified_file("uImage", kerVersion, bootPath)
        elif "amd" in self.arch:
            imageFile = self._find_specified_file("vmlinuz", kerVersion, bootPath)
        else:
            imageFile = self._find_specified_file("vmlinuz", kerVersion, bootPath)
        
        logging.info("[Grub]: Find image[%s].", imageFile)
        return imageFile

    def find_initrd(self, kerVersion, bootPath = "/boot/"):
        initrdFile = self._find_specified_file("initrd.img", kerVersion, bootPath)
        logging.info("[Grub]: Find initrd[%s].", initrdFile)
        return initrdFile


    # 创建升级的GRUB
    # 在get_UUID之后调用
    def create_upgrade_submenu(self, kerVersion):
        logging.info("[Grub]: create_upgrade_submenu start.")
        upgradeMenuList = []
        try:
            classStr = " --class kylin --class gnu-linux --class gnu --class os $menuentry_id_option"
            menuTitle = "menuentry 'Upgrade to New OS " + kerVersion + "'" + classStr + " 'gnulinux-upgrade-" + self.rootUUID + "' {\n"
            upgradeMenuList.append(menuTitle)
            insmodList =self._get_menu_insmod(self.GRUB_FILE_PATH)
            if insmodList != None and len(insmodList) != 0:
                for i in range(len(insmodList)):
                    insmodline = "\tinsmod " + insmodList[i]
                    upgradeMenuList.append(insmodline)

            paramstr = "\tsearch --no-floppy --fs-uuid --set=root " + self.bootUUID + "\n"
            upgradeMenuList.append(paramstr)

            # echo	'Loading Linux xxxx ...'
            echostr = "\techo\t'Loading Linux " + kerVersion + " ...'\n"
            upgradeMenuList.append(echostr)

            resumePath = "/etc/initramfs-tools/conf.d/resume"
            if os.path.exists(resumePath):
                tmpCmd = "cat " + resumePath
                resumDev = get_sys_call(tmpCmd)
            else:
                tmpCmd = "cat /etc/fstab |grep -v \"^#\"|grep swap|head -n 1| awk \'{print $1}\'"
                resumDev = get_sys_call(tmpCmd)
            resumDev = resumDev.strip("\n")
            linuxParam = " ro upgrade debug splash loglevel=7 security=kysec kysec_status=4 resume=" + resumDev

            imagefile = self.find_image(kerVersion)
            if imagefile is None:
                logging.error("[Grub]: Can't find image")
                return None
            linuxstr = "\tlinux\t" + imagefile + " root=UUID=" + self.rootUUID + linuxParam + "\n"
            upgradeMenuList.append(linuxstr)

            initrdfile = self.find_initrd(kerVersion)
            if initrdfile is None:
                logging.error("[Grub]: Can't find image")
                return None
            intirdstr = "\tinitrd\t" + initrdfile + "\n"
            upgradeMenuList.append(intirdstr)

            upgradeMenuList.append("}\n")
        except Exception as e:
            logging.error("[Grub]: err[%s].",e)

        logging.info("[Grub]: create_upgrade_submenu finished.")
        return upgradeMenuList

    def create_force_org_submenu(self, kerVersion):
        logging.info("[Grub]: create_force_org_submenu start.")
        fallbackMenuList = []

        classStr = " --class kylin --class gnu-linux --class gnu --class os $menuentry_id_option"
        menuTitle = "menuentry 'Force back to old OS " + kerVersion + "'" + classStr + " 'gnulinux-fallback-" + self.rootUUID + "' {\n"
        fallbackMenuList.append(menuTitle)
        insmodList =self._get_menu_insmod(self.GRUB_FILE_PATH)
        if insmodList != None and len(insmodList) != 0:
            for i in range(len(insmodList)):
                insmodline = "\tinsmod " + insmodList[i]
                fallbackMenuList.append(insmodline)
        paramList = []
        isboot = False
        paramList, isboot = self._get_param(self.GRUB_FILE_PATH)

        for line in paramList:
            fallbackMenuList.append(line)

        echostr = "\techo\t'Loading Linux " + kerVersion + " ...'\n"
        fallbackMenuList.append(echostr)

        resumePath = "/etc/initramfs-tools/conf.d/resume"
        if os.path.exists(resumePath):
            tmpCmd = "cat " + resumePath
            resumDev = get_sys_call(tmpCmd)
        else:
            tmpCmd = "cat /etc/fstab |grep -v \"^#\"|grep swap|head -n 1| awk \'{print $1}\'"
            resumDev = get_sys_call(tmpCmd)
        resumDev = resumDev.strip("\n")
        linuxParam = " forceback debug splash loglevel=0 rootdelay=10 KEYBOARDTYPE=pc KEYTABLE=us security= resume=" + resumDev

        imagefile = self.find_image(kerVersion)
        if imagefile is None:
            logging.error("[Grub]: Can't find image")
            return None
        linuxstr = "\tlinux\t" + imagefile + " root=UUID=" + self.rootUUID + linuxParam + "\n"
        fallbackMenuList.append(linuxstr)

        initrdfile = self.find_initrd(kerVersion)
        if initrdfile is None:
            logging.error("[Grub]: Can't find image")
            return None
        intirdstr = "\tinitrd\t" + initrdfile + "\n"
        fallbackMenuList.append(intirdstr)

        if isboot:
            fallbackMenuList.append("        boot\n")

        fallbackMenuList.append("}\n")

        logging.info("[Grub]: create_force_org_submenu finished.")
        return fallbackMenuList



    # 获取选项中的参数条目
    def _get_param(self, fileName):
        for file in self.fileList:
            if fileName != file.fileName:
                continue
            subMenuList = file.subMenuList
            if len(subMenuList) == 0:
                logging.error("[grub]:there is no submenu")
                return None

            menuList = subMenuList[0].menuList
            if len(menuList) == 0:
                logging.error("[Grub]: there is no menuList")
                return None
            return menuList[0].paramList, menuList[0].isboot
        logging.error("[Grub]: Can't get param list")
        return None


    # 创建还原的GRUB
    # 在get_UUID之后调用
    def create_fallback_submenu(self, kerVersion):
        logging.info("[Grub]: create_fallback_submenu start.")
        fallbackMenuList = []

        classStr = " --class kylin --class gnu-linux --class gnu --class os $menuentry_id_option"
        menuTitle = "menuentry 'Fallback to old OS " + kerVersion + "'" + classStr + " 'gnulinux-fallback-" + self.rootUUID + "' {\n"
        fallbackMenuList.append(menuTitle)
        insmodList =self._get_menu_insmod(self.GRUB_FILE_PATH)
        if insmodList != None and len(insmodList) != 0:
            for i in range(len(insmodList)):
                insmodline = "\tinsmod " + insmodList[i]
                fallbackMenuList.append(insmodline)
        paramList = []
        isboot = False
        paramList, isboot = self._get_param(self.GRUB_FILE_PATH)

        for line in paramList:
            fallbackMenuList.append(line)

        echostr = "\techo\t'Loading Linux " + kerVersion + " ...'\n"
        fallbackMenuList.append(echostr)

        resumePath = "/etc/initramfs-tools/conf.d/resume"
        if os.path.exists(resumePath):
            tmpCmd = "cat " + resumePath
            resumDev = get_sys_call(tmpCmd)
        else:
            tmpCmd = "cat /etc/fstab |grep -v \"^#\"|grep swap|head -n 1| awk \'{print $1}\'"
            resumDev = get_sys_call(tmpCmd)
        resumDev = resumDev.strip("\n")
        linuxParam = " back2old debug splash loglevel=0 rootdelay=10 KEYBOARDTYPE=pc KEYTABLE=us security= resume=" + resumDev

        imagefile = self.find_image(kerVersion)
        if imagefile is None:
            logging.error("[Grub]: Can't find image")
            return None
        linuxstr = "\tlinux\t" + imagefile + " root=UUID=" + self.rootUUID + linuxParam + "\n"
        fallbackMenuList.append(linuxstr)

        initrdfile = self.find_initrd(kerVersion)
        if initrdfile is None:
            logging.error("[Grub]: Can't find image")
            return None
        intirdstr = "\tinitrd\t" + initrdfile + "\n"
        fallbackMenuList.append(intirdstr)

        if isboot:
            fallbackMenuList.append("        boot\n")

        fallbackMenuList.append("}\n")

        logging.info("[Grub]: create_fallback_submenu finished.")
        return fallbackMenuList

    def add_subMenu(self, fileName, subMenuIn):
        if subMenuIn is None:
            logging.error("[Grub]: IN param subMenuIn is None.")
            return RMFAILURE
        logging.info("[Grub]: add_subMenu start.")
        subMenuList = []
        newSubMenuList = []
        beginIndex = 0
        endIndex = 0
        for file in self.fileList:
            if fileName == file.fileName:
               subMenuList = file.subMenuList
               beginIndex = file.fileStartLine
               endIndex = file.fileEndLine
               firstSubMenuBeginIndex = file.firstSubMenuStart
               otherList = file.otherList
               break
        if len(subMenuList) == 0:
            logging.error("[Grub]: file[%s] has not subMenu.", fileName)
            return RMFAILURE

        if firstSubMenuBeginIndex != beginIndex:
            newSubMenuList[-1:-1] = iter(otherList)

        for line in subMenuIn:
            newSubMenuList.append(line)
        
        subTitle = "submenu \'Original options\' $menuentry_id_option 'gnulinux-original-" + self.rootUUID +"' {\n"
        newSubMenuList.append(subTitle)
        newSubMenuList.append("}\n")
        for subMenutmp in subMenuList:
            newSubMenuList[-1:-1] = iter(subMenutmp.subMenuBlockList)

        with open(self.grubCfgPath, 'r') as fd_r:
            grubfdr = fd_r.readlines()
        fd_r.close()

        del grubfdr[beginIndex+1:endIndex]
        grubfdr[beginIndex+1:beginIndex+1] = iter(newSubMenuList)

        with open(self.grubCfgPath, 'w') as fd_w:
            try:
                fd_w.writelines(iter(grubfdr))
            except Exception as e:
                logging.error("[Grub]: write grub failed as [%s]",e)

        fd_w.close()
        logging.info("[Grub]: add_subMenu success.")
        return RMSUCCESS

    def remove_hidden(self):
        cmd = "sed -i \'/timeout_style=hidden/d\' " + self.grubCfgPath
        get_sys_call(cmd)

        cmd = "sed -i \'s/timeout=0/timeout=5/g\' " + self.grubCfgPath
        get_sys_call(cmd)


# test
if __name__ == "__main__":
    grubClass = RMGrub()

    grubClass.grubCfgPath = "/home/kylin/scp/grub.cfg"
    grubClass.analysis_cfg(grubClass.grubCfgPath)

    # mountinfo = get_sys_call("mount")
    # mountList = str_to_list(mountinfo,"\n")

    grubClass.get_UUID()
    # ver = "5.4.18-35-generic"
    # menulist = grubClass.create_upgrade_submenu(ver)
    # grubClass.add_subMenu(RMCom.GRUB_FILE_PATH, menulist)

    if "arm" in grubClass.arch:
        orgver = "4.4.131-20210817.kylin.desktop-generic"
    elif "amd" in grubClass.arch:
        orgver = "4.4.131-20210817.kylin.x86-generic"
    menulist = grubClass.create_fallback_submenu(orgver)
    grubClass.add_subMenu(RMCom.GRUB_FILE_PATH, menulist)

    exit(0)
