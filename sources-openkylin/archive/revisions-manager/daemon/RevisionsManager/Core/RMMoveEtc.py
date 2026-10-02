#!/usr/bin/python3
# -*- coding: UTF-8 -*-

from genericpath import isfile
import logging
import os
import shutil
import yaml
import subprocess
RMSUCCESS = 0
RMFAILURE = -1

ETCPOLICYPATH = "/usr/share/revisions-manager/data/.etcPolicy.yaml"
ORGFS_CFG_DIR="/var/log/RevisionsManager/"

class RMMoveEtc():
    def __init__(self) -> None:

        self.orgEtcPath = "/etc"
        self.newEtcPath = "/fs.new/etc"

    # 直接拷贝到新系统
    def _move_file_directly(self, etcfile, srcPath, distPath):
        try:
            cmd = "cp -fp " + os.path.join(srcPath,etcfile) + " " + distPath
            ret = subprocess.run(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            logging.info("[moveEtc]:cmd[%s] ret[%s].",cmd, ret.stdout.decode())
        except Exception as e:
            logging.warning("[moveEtc]:copy file[%s] failed [%s].", os.path.join(srcPath,etcfile), e)

    # 直接拷贝到新系统 (目标路径存在，则删除后拷贝)
    def _move_folder_directly(self, etcfolder, srcPath, distPath):
        distFolderPath = os.path.join(distPath, etcfolder)
        srcFolderPath = os.path.join(srcPath, etcfolder)
        try:
            # cmd = "cp -rf " + srcFolderPath + "/* " + distFolderPath
            cmd = "cp -rfp " + srcFolderPath + " " + distPath
            ret = subprocess.run(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            logging.info("[moveEtc]:cmd[%s] ret[%s].",cmd, ret.stdout.decode())
        except Exception as e: 
            logging.warning("[moveEtc]: copy folder[%s] failed [%s].", os.path.join(srcPath,etcfolder),e)


    # 迁移单个用户下的文件
    # file 修改的文件
    # userList 用户列表 
    def _move_single_file(self, file, userList):
        logging.info("[moveEtc]: move single file[%s]", file)
        if len(userList) == 0:
            logging.warning("[moveEtc]: no userlist need moving")
            return RMSUCCESS
        
        orgFilePath = os.path.join(self.orgEtcPath,file)
        if not os.path.exists(orgFilePath):
            logging.error("[moveEtc]: org file[%s] not exists.", orgFilePath)
            return RMFAILURE

        newFilePath = os.path.join(self.newEtcPath,file)
        if not os.path.exists(newFilePath):
            logging.error("[moveEtc]: new file[%s] not exists.", newFilePath)
            return RMFAILURE

        # 原则：
        # org有,new没有，添加到new;
        # org有,new也有，替换原来的new
        try:
            with open(newFilePath, 'r') as newfd_r:
                newList = newfd_r.readlines()

            with open(orgFilePath, 'r') as orgfd:
                orgList = orgfd.readlines()

            with open(newFilePath, 'w+') as newfd_w:

                for user in userList:
                    for rowOrg in orgList:
                        rowAddFlag = False
                        if user not in rowOrg:
                            continue

                        for rowNew in newList:
                            itemOrgList = rowOrg.split(":")
                            itemNewList = rowNew.split(":")
                            if itemOrgList[0] == itemNewList[0]:
                                # TODO:不改变新系统的uid
                                if len(itemOrgList) >= 2 and len(itemNewList) >= 2 and itemOrgList[2] != itemNewList[2]:
                                    rowOrg = rowOrg.replace(itemOrgList[2], itemNewList[2])

                                newList[newList.index(rowNew)] = rowNew.replace(rowNew, rowOrg)
                                rowAddFlag = True
                                logging.info("[moveEtc]: change row[%s] to [%s].", rowNew.strip('\n'), rowOrg.strip('\n'))
                                break
                        if rowAddFlag:
                            continue
                        
                        newList.append(rowOrg)
                        logging.info("[moveEtc]: add row[%s] to file[%s].", rowOrg.strip('\n'), newFilePath)
                newfd_w.writelines(newList)

            newfd_r.close()
            newfd_w.close()
            orgfd.close()
        except Exception as e:
            logging.info("[moveEtc]: error[%s].", e)

        return RMSUCCESS

    # uid_list 在group迁移完成之后调用
    def create_uid_list(self, userList):
        uidPath = "/fs.new/etc/uid_list"
        if os.path.exists(uidPath):
            with open(uidPath, "a") as fd:
                fd.writelines(userList)

            fd.close()

    # 迁移etc下 用户相关文件
    # 在chroot时，通过命令修改
    def _move_etc_users(self, fileList):
        logging.info("[moveEtc]: move users\n")
        if not os.path.exists("/etc/group"):
            logging.error("[moveEtc]: /etc/group not exists.")
            return RMFAILURE

        moveList = []
        uidAddList = []
        fd = open("/etc/group",'r')
        try:
            linelist = fd.readlines()
            for line in linelist:
                blockele = line.split(':')
                if not blockele[2].isdigit:
                    logging.error("[moveEtc]: line[%s] is error.", line)
                    continue
                if int(blockele[2]) >= 1000 and int(blockele[2]) < 65534:
                    moveList.append(blockele[0])
                    uidAdd = blockele[0]+":"+blockele[2]+"\n"
                    uidAddList.append(uidAdd)
                    logging.info("[moveEtc]: will add user[%s] uid[%s]. ", line.strip('\n'), uidAdd.strip('\n'))
        finally:
            fd.close()

        if len(moveList) == 0:
            logging.info("[moveEtc]: moveList is null.")
            return RMSUCCESS

        self.create_uid_list(uidAddList)

        for ele in fileList:
            if ele == "uid_list":
                continue
            if "-" in ele:
                continue
            try:
                self._move_single_file(ele,moveList)
                shutil.copy2(os.path.join(self.newEtcPath,ele), os.path.join(self.newEtcPath,ele+"-"))
            except Exception:
                continue

        return RMSUCCESS

    #
    def handle_signal_file(self ,file):

        pass



    # 迁移 etc
    # 依据策略文件.etcPolicy.yaml
    def move_etc(self):
        if not os.path.exists(ETCPOLICYPATH):
            logging.error("[moveEtc]: File[%s] not exist.", ETCPOLICYPATH)
            return RMFAILURE

        with open(ETCPOLICYPATH, 'r') as etcfile:
            etcPolicyDict = yaml.load(etcfile, Loader=yaml.FullLoader)
        etcfile.close()

        if not os.path.exists(self.newEtcPath):
            logging.error("[moveEtc]: Folder[%s] not exists.", self.newEtcPath)
            return RMFAILURE
       
        orgEtcAllList = os.listdir(self.orgEtcPath)
        orgEtcFileList = []
        orgEtcFileMoveList = []
        orgEtcFileKeepList = []
        orgEtcFileChangeList = []
        orgEtcFileOtherList = []
        orgEtcFileUserList = []
        orgEtcFolderList = []
        orgEtcFolderMoveList = []
        orgEtcFolderKeepList = []
        orgEtcFolderChangeList = []
        orgEtcFolderOtherList = []
        for ele in orgEtcAllList:
            # 文件处理方案
            try:
                if isfile(os.path.join(self.orgEtcPath,ele)):
                    orgEtcFileList.append(ele)
                    # 策略：直接拷贝
                    if ele in etcPolicyDict['movefile']:
                        logging.info("[moveEtc]: movefile[%s].",ele)
                        self._move_file_directly(ele, self.orgEtcPath, self.newEtcPath)
                        orgEtcFileMoveList.append(ele)
                        continue
                    # 策略：不迁移。记录
                    elif ele in etcPolicyDict['keepfile']:
                        orgEtcFileKeepList.append(ele)
                        continue
                    # 策略：user相关。记录
                    elif ele in etcPolicyDict['userfile']:
                        orgEtcFileUserList.append(ele)
                        continue
                    # 策略：修改后迁移
                    elif ele in etcPolicyDict['changefile']:
                        orgEtcFileChangeList.append(ele)
                        #TODO:文件单独处理
                        self.handle_signal_file(ele)
                        continue

                    # 软连接
                    if os.path.islink(os.path.join(self.orgEtcPath, ele)):
                        logging.info("[moveEtc]: [%s] is a link", ele)
                        if not os.path.exists(os.path.join(self.newEtcPath, ele)):
                            self._move_file_directly(ele, self.orgEtcPath, self.newEtcPath)
                    else:
                        # 不在策略列表中的都拷贝
                        logging.info("[moveEtc]: otherfile[%s].",ele)
                        orgEtcFileOtherList.append(ele)
                        self._move_file_directly(ele, self.orgEtcPath, self.newEtcPath)

                # 文件夹处理
                else:
                    orgEtcFolderList.append(ele)

                    # 策略：直接拷贝
                    if ele in etcPolicyDict['movefolder']:
                        self._move_folder_directly(ele, self.orgEtcPath, self.newEtcPath)
                        orgEtcFolderMoveList.append(ele)
                        continue
                    # 策略：不迁移
                    elif ele in etcPolicyDict['keepfolder']:
                        orgEtcFolderKeepList.append(ele)
                        continue
                    # 策略：修改后迁移
                    elif ele in etcPolicyDict['changefolder']:
                        orgEtcFolderChangeList.append(ele)
                        #TODO:文件单独处理
                        continue

                    # 不在策略列表中的都拷贝
                    orgEtcFolderOtherList.append(ele)
                    self._move_folder_directly(ele, self.orgEtcPath, self.newEtcPath)
            except Exception:
                continue

        self._move_etc_users(orgEtcFileUserList)

        orgEtcDict = {}
        orgEtcDict["keepfile"]=orgEtcFileKeepList
        orgEtcDict["movefile"]=orgEtcFileMoveList
        orgEtcDict["changefile"]=orgEtcFileChangeList
        orgEtcDict["otherfile"]=orgEtcFileOtherList
        orgEtcDict["userfile"] = orgEtcFileUserList
        orgEtcDict["keepfolder"]=orgEtcFolderKeepList
        orgEtcDict["movefolder"]=orgEtcFolderMoveList
        orgEtcDict["changefolder"]=orgEtcFolderChangeList
        orgEtcDict["otherfolder"]=orgEtcFolderOtherList

        if not os.path.exists(ORGFS_CFG_DIR):
            os.makedirs(ORGFS_CFG_DIR)

        with open(ORGFS_CFG_DIR+"orgEtcEle"+".yaml", 'w') as f:
            yaml.dump(orgEtcDict, f)
        f.close()


        return RMSUCCESS


if __name__ == "__main__":
    rmclass = RMMoveEtc()
    rmclass.move_etc()

    print("move etc end.")
