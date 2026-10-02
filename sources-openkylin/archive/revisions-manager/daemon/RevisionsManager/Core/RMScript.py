#!/usr/bin/python3
# 脚本执行管理
# TODO:
    # 使能 失能     -- 配置文件
    # 错误码        -- 没必要 无路如何后续都会执行，错误也不会处理
    # 进度同步      -- 新家线程获取
    # 输出规范      --(重定向日志文件、输出格式)
    # 元数据类型    -- 描述 暂用脚本名称
    # 运行等级      -- 暂不支持(root/user)


import logging
import os
import sys
import threading

from RevisionsManager.Core.RMCom import RMFAILURE, RMSUCCESS, RMCom, get_sys_call
from RevisionsManager.Core.RMConf import RMConf


SEC_ABS="abstract"
KEY_COUNT="count"
SEC_PRO="program"
KEY_NAME="name"
KEY_PARAM="param"


class RMScriptSingle():
    def __init__(self, name):
        if name is None:
            logging.error("[Script]: in name is None.")
            return
        self.fileName = name                # 脚本名称
        self.discription = ""               # 说明
        self.paramList = []                 # 参数列表
        self.runUser = ""                   # 执行用户(root / user)
        self.cmd = "/bin/bash " + name      # 执行命令
        self.result = ""                    # 结果
        self.singleProcess = 0              # 进度
        self.fifoName = name + "-fifo"      # fifo 文件名称

    # 根据用户、参数，组建命令
    def conf_script(self, discription="Script", paramList=[], runUser="root", logPath=None):
        # TODO : runUser 暂不支持
        # TODO : discription 暂不支持
        if len(paramList) != 0:
            self.cmd = "/bin/bash " + self.fileName
            for ele in paramList:
                self.cmd = self.cmd + " " + ele.strip("\n")
            
        if logPath is not None:
            self.cmd = self.cmd + " >> " + logPath + " 2>&1"

        logging.info("[Script]: script[%s] cmd is [%s].", self.fileName, self.cmd)

    # 获取脚本中的进度
    def update_script_process(self):
        if os.path.exists(self.fifoName):
            try:
                fd = open(self.fifoName, 'r', 1)
                process = fd.readline()
                self.singleProcess = int(process.strip("\n"))
                return self.singleProcess
            except Exception as e:
                logging.info("[Script]: get process err[%s].",e)
        else:
            logging.info("[Script]: script[%s] not exists.", self.fifoName)
            return self.singleProcess

    def run_single(self):
        ret = get_sys_call(self.cmd)
        self.singleProcess = 100
        self.result = ret
        logging.info("[Script]: cmd[%s] ret[%s].", self.cmd, ret)
    
    def get_result(self):
        return self.result

class RMScriptDir(RMCom):
    def __init__(self, orderName, scriptDir):
        if not os.path.exists(scriptDir):
            logging.error("[Script]: dir[%s] not exists.", scriptDir)
            return None
        if orderName is None:
            orderName = "scriptList"

        self.scriptDir = scriptDir              # 脚本目录
        self.orderName = orderName              # 脚本执行列表文件
        self.orderPath = os.path.join(scriptDir,orderName)
        if not os.path.exists(self.orderPath):
            logging.error("[Script]: orderFile[%s] not exists.", self.orderPath)
            return None

        if not os.path.exists(self.LOG_DIR):
            os.makedirs(self.LOG_DIR,mode=0o755)
        self.logPath = os.path.join(self.LOG_DIR, orderName+".log")     # 日志文件

        self.dirProcess = 0                     # DIR 进度
        self.runningName = ""                   # 运行的程序名称
        self.runningState = RMSUCCESS           # 运行的程序状态
        self.runningClass = None                # 运行的程序句柄

        self.execCount = 0
        self.execDict = {}


        logging.info("[Script]: init finished.")

    def get_exec_info(self):
        conf = RMConf(self.orderPath)
        execCount = conf.get_key(SEC_ABS, KEY_COUNT)
        if execCount is not None:
            self.execCount = int(execCount)
            for i in range(self.execCount):
                tmpDict = {}
                execName = conf.get_key(SEC_PRO+str(i), KEY_NAME)
                tmpParam = conf.get_key(SEC_PRO+str(i), KEY_PARAM)

                tmpDict[KEY_NAME] = execName
                if tmpParam is not None:
                    execParamList = tmpParam.split()
                    tmpDict[KEY_PARAM] = execParamList

                self.execDict[SEC_PRO+str(i)] = tmpDict
        
        logging.info("[Script]: get exec info finished.")

    def get_processState(self):
        logging.info("[Script]: get processState info [%s %s].", self.runningName, self.dirProcess)
        return self.runningName, self.dirProcess

    def _update_process(self, execClass, index):
        singleprocess = execClass.update_script_process()
        
        self.runningName = execClass.fileName
        self.dirProcess = int((singleprocess / 100 + index) / self.execCount * 100)

        print("[Script]: index:", index, "name:", self.runningName, "process:", self.dirProcess, "singlepro:", singleprocess)

        if singleprocess >= 100 or singleprocess < 0:
            return

        tmptimer = threading.Timer(1, function=self._update_process, args=[execClass, index])
        tmptimer.start()


    def run_dir(self):
        self.get_exec_info()
        if self.execCount == 0:
            logging.info("[Script]: no exec program.")
            return RMSUCCESS

        orgDir = os.getcwd()
        os.chdir(self.scriptDir)

        for i in range(self.execCount):
            if SEC_PRO+str(i) not in self.execDict.keys():
                logging.info("[Script]: no key[%s] in execDit.", SEC_PRO+str(i))
                continue
            
            singleDict = self.execDict[SEC_PRO+str(i)]
            if KEY_NAME not in singleDict.keys():
                logging.info("[Script]: no key[%s] in singleDict.", KEY_NAME)
                continue
            name = singleDict[KEY_NAME]
            if name is None:
                logging.info("[Script]: bad info exec name is None.")
                continue
            
            paramList = []
            if KEY_PARAM in singleDict.keys():
                paramList = singleDict[KEY_PARAM]

            singleClass = RMScriptSingle(name)
            self.runningClass = singleClass
            singleClass.conf_script(paramList=paramList, logPath=self.logPath)

            atimer = threading.Timer(0.1, function=self._update_process, args=[singleClass, i])
            atimer.start()
            singleClass.run_single()
        
        self.dirProcess = 100
        self.runningState = RMSUCCESS

        os.chdir(orgDir)


def test_update_Process():
    global SCLASS
    global RUNCOUNT

    ret = 0
    ret = SCLASS.update_script_process()
    print("[test]:", ret)

    if ret >= 100 or ret < 0:
        return
    timertmp = threading.Timer(1, function=test_update_Process)
    timertmp.start()


RUNCOUNT=0
SCLASS=None

if __name__ == "__main__":
    path = "/home/yangwenxuan/tmp/test"
    name = "count"
    # scriptClass = RMScriptDir(name,path)

    ##### test single shell #####
    # os.chdir(path)
    # filelist = os.listdir("./")
    # atimer = threading.Timer(1, function=test_update_Process)
    # SCLASS = RMScriptSingle("a.sh")
    # SCLASS.conf_script(logPath="outlog")
    # atimer.start()
    # SCLASS.run_single()

    ##### test RMScriptDir #####
    scriptClass = RMScriptDir(name, path)
    ret = scriptClass.get_exec_info()
    scriptClass.run_dir()

    sys.exit(0)



# a.sh

#!/bin/bash
# date
# echo "xxx start"
# fifoName="a.sh-fifo"
# if [ ! -e ${fifoName} ]; then
#     mkfifo -m 666 ${fifoName}
# fi
# exec 6<>${fifoName}
# i=0
# while :
# do
#     echo $i
#     echo $i >&6
#     sleep 1
#     let i++
#     if [ $i -gt 50 ];then
#         exec 6<&-
#         rm -f ${fifoName}
#         exit 0
#     fi
# done
