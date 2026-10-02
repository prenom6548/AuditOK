#!/usr/bin/python3

import logging
import os


#定义日志的格式 
FORMAT = "%(asctime)s [%(levelname)s]: %(message)s"
LOGDIR = "/var/log/RevisionsManager/"


def get_fileSize(filePath):
    fsize = os.path.getsize(filePath)
    fsize_M = fsize / float(1024 * 1024)
    return round(fsize_M, 2)

def get_logfile():
    numlist = []
    if not os.path.exists(LOGDIR):
        os.makedirs(LOGDIR)
    #优先获取当前未写满100M的日志编号文件
    for i in os.listdir(LOGDIR):
        if "RevisionsManager.log." in os.path.basename(LOGDIR + i):
            numlist.append((LOGDIR + i).split(".")[2])
            if get_fileSize(LOGDIR + i) < 10:
                return LOGDIR + i

    #获取1-5未使用的最小数字的标号作为日志文件
    for i in range(1, 6):
        if str(i) not in numlist:
            return(os.path.join(LOGDIR, ("RevisionsManager.log.%s") % i))
    try:
        #编号1-5日志文件均写满时，删除第一个，往前移动日志编号，获取最后一个编号作为日志文件
        if len(numlist) != 0:
            os.remove(os.path.join(LOGDIR, "RevisionsManager.log.1"))
            for i in range(2, 6):
                os.rename(os.path.join(LOGDIR, ("RevisionsManager.log.%s") % i),
                        os.path.join(LOGDIR, ("RevisionsManager.log.%s") % (i - 1)))
            return os.path.join(LOGDIR, "RevisionsManager.log.5")
        #默认情况下未生成任何日志时使用编号1的日志文件
        else:
            return os.path.join(LOGDIR, "RevisionsManager.log.1")
    except:
        return os.path.join(LOGDIR, "RevisionsManager.log.1")

class RMLog():
    def __init__(self) -> None:

        loglevel = logging.DEBUG
        logFileName = get_logfile()
        logging.basicConfig(format=FORMAT, level=loglevel, datefmt='%m-%d,%H:%M:%S', filename=logFileName, filemode='a')
        logging.info("RMLog init file[%s]", logFileName)

