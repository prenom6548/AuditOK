#!/usr/bin/python3
# -*- coding: UTF-8 -*-
# 脚本插件化执行管理
# TODO:
    # 使能/失能      --配置文件--ok
    # 错误码         --脚本/本程序--wait/add
    # 进度同步       --线程获取--add
    # 输出规范       --脚本/本程序(重定向日志文件、输出格式)--ok/ok
    # 元数据类型     --描述信息、翻译(配置文件)--ok
    # 运行等级       --(root/user)--wait

import subprocess
import os
from sre_compile import isstring
import threading
import yaml
import logging
from gi.repository import GObject

class pluginState():
    PLUGIN_SUCCESS                  = 0
    PLUGINERR_PLUGIN_NOT_EXIST      = PLUGIN_SUCCESS - 1
    PLUGINERR_PLUGIN_NOT_COMPLETED  = PLUGINERR_PLUGIN_NOT_EXIST - 1
    PLUGINERR_NO_AVAILABLE_YAML     = PLUGINERR_PLUGIN_NOT_COMPLETED - 1
    PLUGINERR_NOT_LOAD_ALL          = PLUGINERR_NO_AVAILABLE_YAML - 1
    PLUGINERR_PLUGIN_NOT_IN_LIST    = PLUGINERR_NOT_LOAD_ALL - 1
    PLUGINERR_PLUGIN_NOT_ENABLED    = PLUGINERR_PLUGIN_NOT_IN_LIST - 1
    PLUGINERR_PLUGIN_CONFIG_FAILED  = PLUGINERR_PLUGIN_NOT_ENABLED - 1
    PLUGINERR_LOG_PATH_NOT_EXIT     = PLUGINERR_PLUGIN_CONFIG_FAILED - 1
    PLUGINERR_CONFIG_NOT_COMPLETED  = PLUGINERR_LOG_PATH_NOT_EXIT - 1
    PLUGINERR_CMD_IS_NONE           = PLUGINERR_CONFIG_NOT_COMPLETED - 1
    PLUGINERR_LANGUAGE_NOT_SUPPORT  = PLUGINERR_CMD_IS_NONE - 1

    _numToInfo = {
        PLUGIN_SUCCESS:                     'success',
        PLUGINERR_PLUGIN_NOT_EXIST:         'plugin path not exist',
        PLUGINERR_PLUGIN_NOT_COMPLETED:     'plugin folder not completed',
        PLUGINERR_NO_AVAILABLE_YAML:        'there is no available yaml',
        PLUGINERR_NOT_LOAD_ALL:             'not run load_all',
        PLUGINERR_PLUGIN_NOT_IN_LIST:       'plugin not in loaded plugin list',
        PLUGINERR_PLUGIN_NOT_ENABLED:       'plugin not enabled',
        PLUGINERR_PLUGIN_CONFIG_FAILED:     'plugin config failed',
        PLUGINERR_LOG_PATH_NOT_EXIT:        'log path not exists',
        PLUGINERR_CONFIG_NOT_COMPLETED:     'config not completed',
        PLUGINERR_CMD_IS_NONE:              'cmd is none',
        PLUGINERR_LANGUAGE_NOT_SUPPORT:     'not support this language',
    }
    _infoToNum = {
        'success':                          PLUGIN_SUCCESS,
        'plugin path not exist':            PLUGINERR_PLUGIN_NOT_EXIST,
        'plugin folder not completed':      PLUGINERR_PLUGIN_NOT_COMPLETED,
        'there is no available yaml':       PLUGINERR_NO_AVAILABLE_YAML,
        'not run load_all':                 PLUGINERR_NOT_LOAD_ALL,
        'plugin not in loaded plugin list': PLUGINERR_PLUGIN_NOT_IN_LIST,
        'plugin not enabled':               PLUGINERR_PLUGIN_NOT_ENABLED,
        'plugin config failed':             PLUGINERR_PLUGIN_CONFIG_FAILED,
        'log path not exists':              PLUGINERR_LOG_PATH_NOT_EXIT,
        'config not completed':             PLUGINERR_CONFIG_NOT_COMPLETED,
        'cmd is none':                      PLUGINERR_CMD_IS_NONE,
        'not support this language':        PLUGINERR_LANGUAGE_NOT_SUPPORT,
    }


PLUGIN_MANAGER_PATH = "./"  # 可修改

# 目录结构 FILE PATH
CFG_FILE = "conf.yaml"
CFG_EX_DIR = "conf.d/"
CFG_PATH = PLUGIN_MANAGER_PATH + CFG_FILE
CFG_EX_PATH = PLUGIN_MANAGER_PATH + CFG_EX_DIR
MOD_DIR = "modules/"
MOD_PATH = PLUGIN_MANAGER_PATH + MOD_DIR
MOD_AVAILABLE = "modules-available/"
MOD_ENABLED = "modules-enabled/"
MOD_AVAILABLE_PATH = MOD_PATH + MOD_AVAILABLE
MOD_ENABLED_PATH = MOD_PATH + MOD_ENABLED
PLUGIN_DIR = "script/"
PLUGIN_PATH = PLUGIN_MANAGER_PATH + PLUGIN_DIR

# 配置 日志路径
LOG_DIR_ROOT = '/var/log/kylin-system-updater/'
# 默认插件日志路径
PLUGIN_LOG_DIR = '/var/log/kylin-system-updater/plugin/'

# PLUGIN.YAML
PLUGIN_CONF_KEY_NAME = 'name'
PLUGIN_CONF_KEY_DESC = 'desc'
PLUGIN_CONF_KEY_EXEC = 'exec'
PLUGIN_CONF_KEY_LOGLEVEL = 'loglevel'
PLUGIN_CONF_KEY_RUNLEVEL = 'runlevel'
PLUGIN_CONF_KEY_LIST = [PLUGIN_CONF_KEY_NAME,PLUGIN_CONF_KEY_DESC,PLUGIN_CONF_KEY_EXEC,PLUGIN_CONF_KEY_LOGLEVEL,PLUGIN_CONF_KEY_RUNLEVEL]

# CONF.YAML AND CONF.D
MANAGER_CONF_KEY_LOGDIR = "logdir"
MANAGER_CONF_KEY_LIST = [MANAGER_CONF_KEY_LOGDIR, ]


FORMAT = "%(asctime)s [%(levelname)s]: %(message)s"
RUNLEVEL_LIST = ['ROOT', 'USER']
LOGLEVEL_LIST = ['DEBUG', 'INFO', 'NOTIFY', 'WARNING', 'ERROR', 'CRITICAL']

class LOADSTATE():
    PLGNAME = 0x01
    EXECCMD = 0x02
    STATESUM = PLGNAME + EXECCMD


LANG_KEY_ZH_CN = 'zh_CN'
LANG_KEY_EN = 'en'
class LANGLIST():
    LANG_EN = 0
    LANG_ZH_CN = 1


class pluginClass(pluginState):

    def __init__(self):
        # 必须配置项
        self.pluginName = None
        self.execCmd = None
        # 可选配置项
        self.descList = []  # en / zh
        self.logLevel = LOGLEVEL_LIST.index('DEBUG')
        self.runLevel = RUNLEVEL_LIST.index('ROOT')
        self.enabled = False
        # 合成变量
        self.cmd = None
        self.logDir = PLUGIN_LOG_DIR                            # 插件日志路径
        self.logPath = os.path.join(self.logDir, "default.log") # 插件日志文件
        self.fifoName = "default-fifo"                          # 插件进度文件
        # self.fifoPath = PLUGIN_PATH + self.fifoName
        self.fifoPath = "/var/log/kylin-system-updater"+self.fifoName
        # 记录变量
        self.running = False        # 是否在运行
        self.process = 0            # 执行进度
        self.loadState = 0          # 插件配置完成
        logging.info("init finished.")

    ###################### 内部函数 ######################
    # 1-配置指定字段
    # 2-更新进度 (1/0.5s)
    # 3-
    ######################
    def _config_key(self, cfg, key):
        if cfg == None or key == None or key not in cfg:
            logging.warning("[PLUGIN]: key[%s] not in yaml.", key)

        if key == PLUGIN_CONF_KEY_NAME:
            if isstring(cfg[key]):
                self.pluginName = cfg[key]
                self.fifoName = cfg[key] + "-fifo"
                self.loadState += LOADSTATE.PLGNAME
            else:
                logging.error("[PLUGIN]: name[%s] not string.", cfg[key])

        elif key == PLUGIN_CONF_KEY_DESC:
            langList = cfg[key]
            descDict = {}
            if langList == None or len(langList) == 0:
                return
            for i in range(len(langList)):
                descDict = langList[i]
                if LANG_KEY_EN in descDict:
                    self.descList.insert(LANGLIST.LANG_EN, descDict.pop(LANG_KEY_EN))
                    continue
                elif LANG_KEY_ZH_CN in descDict:
                    self.descList.insert(LANGLIST.LANG_ZH_CN, descDict.pop(LANG_KEY_ZH_CN))
                    continue

        elif key == PLUGIN_CONF_KEY_EXEC:
            if isstring(cfg[key]):
                self.execCmd = cfg[key]
                self.loadState += LOADSTATE.EXECCMD
            else:
                logging.error("[PLUGIN]: execCmd[%s] not string.", cfg[key])

        elif key == PLUGIN_CONF_KEY_LOGLEVEL:
            loglevel = cfg[key].upper()
            if loglevel in LOGLEVEL_LIST:
                self.logLevel = LOGLEVEL_LIST.index(loglevel)

        elif key == PLUGIN_CONF_KEY_RUNLEVEL:
            runlevel = cfg[key].upper() 
            if runlevel in RUNLEVEL_LIST:
                self.runLevel = RUNLEVEL_LIST.index(runlevel)
        else:
            logging.warning("[PLUGIN]: key[%s] not need config.", key)

    def _update_process(self):
        if not self.running:
            logging.info("[PLUGIN]: plugin [%s] is not running.", self.pluginName)
            return

        if os.path.exists(self.fifoPath):
            try:
                fd = open(self.fifoPath, 'r', 1)
                process = fd.readline()
                self.process = int(process.strip("\n"))
            except Exception as e:
                logging.info("[PLUGIN]: get process err[%s].",e)
        else:
            logging.info("[PLUGIN]: fifo[%s] not exists.", self.fifoPath)

        if self.process >= 100 or self.process < 0:
            return

        tmptimer = threading.Timer(0.5, function=self._update_process)
        tmptimer.start()

    ###################### 外部函数 ######################
    # 1-读取配置文件，并配置该插件
    # 2-使能插件
    # 3-失能插件
    # 4-获取插件名称
    # 5-获取进度
    # 6-注册进度跟新回调
    # 7-执行插件
    # 8-获取描述信息
    # 9-设置脚本日志路径

    # TODO:
        # 重配置该插件
    ######################

    # 配置该插件
    def plg_config(self, filePath):
        if not os.path.exists(filePath):
            logging.error("[PLUGIN]: [%s] not exist.", filePath)
            return self.PLUGINERR_PLUGIN_CONFIG_FAILED

    def plg_enable(self):
        self.enabled = True
    def plg_disable(self):
        self.enabled = False

    def plg_get_name(self):
        return self.pluginName

    def plg_get_process(self):
        return self.process
    
    def plg_get_desc(self):
        # 获得语言变量
        #TODO: 例如：中文繁体，如果不存在的话，显示中文简体
        lang=os.getenv("LANG")
        if LANG_KEY_EN in lang:
            if len(self.descList) > LANGLIST.LANG_EN:
                return self.descList[LANGLIST.LANG_EN]
            else:
                logging.error("[PLUGIN]: There is not a desc of the language[%s].", lang)
        elif LANG_KEY_ZH_CN in lang:
            if len(self.descList) > LANGLIST.LANG_ZH_CN:
                return self.descList[LANGLIST.LANG_ZH_CN]
            else:
                logging.error("[PLUGIN]: There is not a desc of the language[%s].", lang)
        else:
            logging.error("[PLUGIN]: There is not a desc of the language[%s].", lang)
            return

    # 添加 update cmd 
    # 设置脚本日志路径
    def plg_set_logDir(self, logPath):
        if not os.path.exists(logPath):
            try:
                os.makedirs(logPath, mode=0o755)
            except Exception as e:
                logging.error("[PLUGIN]: create plugin log dir failed.[%s]", e)
                return self.PLUGINERR_LOG_PATH_NOT_EXIT
        self.logDir = logPath
        if self.pluginName != None:
            self.logPath = os.path.join(self.logDir, self.pluginName + ".log")
            self.cmd = "bash " + self.execCmd + " fifoname=" + self.fifoName + " logpath=" + self.logPath + " loglevel=" + str(self.logLevel) + " modename=" + self.pluginName

    def plg_run(self):
        if not self.enabled:
            logging.error("[PLUGIN]: [%s] not enabled.", self.pluginName)
            return self.PLUGINERR_PLUGIN_NOT_ENABLED

        self.running = True
        tmptimer = threading.Timer(0.5, function=self._update_process)
        tmptimer.start()

        if self.cmd == None:
            logging.error("[PLUGIN]: cmd is None.")
            return self.PLUGINERR_CMD_IS_NONE, self._numToInfo(self.PLUGINERR_CMD_IS_NONE), self._numToInfo(self.PLUGINERR_CMD_IS_NONE)

        logging.debug("[PLUGIN]: cmd[%s].",self.cmd)
        try:
            ret = subprocess.run(self.cmd, shell = True, stdout = subprocess.PIPE, stderr = subprocess.PIPE)
        except Exception as e:
            logging.error("[PLUGIN]: subprocess run err[%s].", e)

        self.running = False
        logging.debug("[PLUGIN]: [%s] run finished ret[%d].",self.pluginName, ret.returncode)
        return ret.returncode, ret.stdout.decode(), ret.stderr.decode()

    def plg_reconfig(self):
        pass



class pluginManagerClass(pluginState):
    def __init__(self):
        # 变量初始化
        self.plgClassList = []      # 插件句柄
        self.loaded = False

        self.managerLogDir = LOG_DIR_ROOT       # 管理器日志路径
        # 日志配置初始化,试用updater的logger
        # if not os.path.exists(self.managerLogDir):
        #     os.mkdir(self.managerLogDir, mode=0o755)
        # logfile = os.path.join(self.managerLogDir, 'PluginManager.log.' + str(self.classNum))
        # logging.basicConfig(format=FORMAT, level='DEBUG', datefmt='%m-%d,%H:%M:%S', filename=logfile, filemode='a')
        # self.pluginLogDir = PLUGIN_LOG_DIR      # 插件日志路径

    # 将单个插件句柄添加到全局记录, 并使能
    def _add_single_plugin(self, filePath, enable):
        if not os.path.exists(filePath):
            logging.debug("[PLUGIN]: [%s] not exist.", filePath)
            return

        singlePlgClass = pluginClass()
        singlePlgClass.plg_config(filePath)
        self.plgClassList.append(singlePlgClass)
        if enable:
            singlePlgClass.plg_enable()
        singlePlgClass.plg_set_logDir(self.pluginLogDir)

    def _remove_single_plugin(self, pluginClass):
        if pluginClass in self.plgClassList:
            logging.debug("[PLUGIN]: remove [%s].", pluginClass.plg_get_name())
            pluginClass.remove(pluginClass)
            pluginClass.plg_disable()


    # 加载所有插件,读取所有配置
    # 第一个执行
    # 返回插件句柄列表
    # TODO:加载指定插件, 读取指定配置
    def reload_plugin(self, pluginName):
        pass

    # 通过句柄获取插件名称
    def get_plugin_name(self, pluginClass):
        if not self.loaded:
            logging.error("[PLUGIN]: please run load_all first.")
            return self.PLUGINERR_NOT_LOAD_ALL
        if pluginClass not in self.plgClassList:
            logging.error("[PLUGIN]: there is no this plugin in pluginList.")
            return self.PLUGINERR_PLUGIN_NOT_IN_LIST
        
        return pluginClass.plg_get_name()

    # 运行指定插件
    # pluginName, pluginClass 都指定时，以名称为准
    def run_plugin(self, pluginName = None):
        self.running = True
        
        if pluginName == None or not os.path.isfile(pluginName):
            logging.error("[PLUGIN]: [%s] Cann't found.",pluginName)
            return True
        cmd = "bash " + pluginName
        try:
            ret = subprocess.run(cmd, shell = True, stdout = subprocess.PIPE, stderr = subprocess.PIPE)
            logging.info("[PLUGIN]: script[%s].",pluginName)
        except Exception as e:
            logging.error("[PLUGIN]: subprocess run err[%s].", e)
            return True

        self.running = False

        if ret.returncode != 0:
            logging.error("[PLUGIN]: code:%d, out:%s, err:%s",ret.returncode, ret.stdout.decode(), ret.stderr.decode())
        logging.debug("[PLUGIN]: run finished returncode[%d], out[%s], err[%s]",ret.returncode, ret.stdout.decode(), ret.stderr.decode())
        return (ret.returncode==0)

    def connect_signal(self, plgclass, signal, handler):
        if plgclass not in self.plgClassList:
            logging.error("[PLUGIN]: there is no this plugin in pluginList.")
            return self.PLUGINERR_PLUGIN_NOT_IN_LIST
        return plgclass.connect(signal, handler)
    

def plugin_process_handler(obj, process):
    logging.info("[PLUGIN]: ******* process [%d].", process)

# if __name__ == "__main__":

#     pMClass = pluginManagerClass()
#     plgList = pMClass.load_all("./")

#     for everyPlg in iter(plgList):
#         name = pMClass.get_plugin_name(everyPlg)
#         print("name:", name)
#         desc = pMClass.get_desc(everyPlg)
#         print("desc:", desc)
#         pMClass.connect_signal(everyPlg, "processChanged", plugin_process_handler)

#         ret = pMClass.run_plugin(name)

#     exit(0)

