# /*
#  * Copyright (C) 2020, KylinSoft Co., Ltd.
#  *
#  * This program is free software: you can redistribute it and/or modify
#  * it under the terms of the GNU General Public License as published by
#  * the Free Software Foundation, either version 3 of the License, or
#  * (at your option) any later version.
#  *
#  * This program is distributed in the hope that it will be useful,
#  * but WITHOUT ANY WARRANTY; without even the implied warranty of
#  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  * GNU General Public License for more details.
#  *
#  * You should have received a copy of the GNU General Public License
#  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
#  */

#!/usr/bin/python3
# -*- coding: utf-8 -*-
#参考文档： https://cuiqingcai.com/6080.html

import os

path = '/var/log/kylin-system-updater/'

numlist = []

def get_FileSize(filePath):
    fsize = os.path.getsize(filePath)
    fsize = fsize / float(1024 * 1024)
    return round(fsize, 2)

#日志回滚
def get_logfile():
    if not os.path.exists(path):
        os.makedirs(path)
    return os.path.join(path, "kylin-system-updater.log.1")
    #优先获取当前未写满100M的日志编号文件
    for i in os.listdir(path):
        if "kylin-system-updater.log." in os.path.basename(path + i):
            numlist.append((path + i).split(".")[2])
            if get_FileSize(path + i) < 100:
                return path + i
    #获取1-5未使用的最小数字的标号作为日志文件
    for i in range(1, 6):
        if str(i) not in numlist:
            return(os.path.join(path, ("kylin-system-updater.log.%s") % i))
    try:
        #编号1-5日志文件均写满时，删除第一个，往前移动日志编号，获取最后一个编号作为日志文件
        if len(numlist) != 0:
            os.remove(os.path.join(path, "kylin-system-updater.log.1"))
            for i in range(2, 6):
                os.rename(os.path.join(path, ("kylin-system-updater.log.%s") % i),
                          os.path.join(path, ("kylin-system-updater.log.%s") % (i - 1)))
            return os.path.join(path, "kylin-system-updater.log.5")
        #默认情况下未生成任何日志时使用编号1的日志文件
        else:
            return os.path.join(path, "kylin-system-updater.log.1")
    except:
        return os.path.join(path, "kylin-system-updater.log.1")

def upgrade_strategies_logfile():
    if not os.path.exists(path):
        os.makedirs(path)
    #优先获取当前未写满100M的日志编号文件
    for i in os.listdir(path):
        if "upgrade-strategies-daemon.log." in os.path.basename(path + i):
            numlist.append((path + i).split(".")[2])
            if get_FileSize(path + i) < 10:
                return path + i
    #获取1-5未使用的最小数字的标号作为日志文件
    for i in range(1, 6):
        if str(i) not in numlist:
            return(os.path.join(path, ("upgrade-strategies-daemon.log.%s") % i))
    try:
        #编号1-5日志文件均写满时，删除第一个，往前移动日志编号，获取最后一个编号作为日志文件
        if len(numlist) != 0:
            os.remove(os.path.join(path, "upgrade-strategies-daemon.log.1"))
            for i in range(2, 6):
                os.rename(os.path.join(path, ("upgrade-strategies-daemon.log.%s") % i),
                          os.path.join(path, ("upgrade-strategies-daemon.log.%s") % (i - 1)))
            return os.path.join(path, "upgrade-strategies-daemon.log.5")
        #默认情况下未生成任何日志时使用编号1的日志文件
        else:
            return os.path.join(path, "upgrade-strategies-daemon.log.1")
    except:
        return os.path.join(path, "upgrade-strategies-daemon.log.1")
