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


PLUGIN_MANAGER_DIR = os.getcwd() + "/"  # 脚本文件所在路径

# 目录结构 FILE PATH
CFG_FILE = "conf.yaml"
CFG_EX_DIR = "conf.d/"
CFG_PATH = PLUGIN_MANAGER_DIR + CFG_FILE
CFG_EX_PATH = PLUGIN_MANAGER_DIR + CFG_EX_DIR
MOD_DIR = "modules/"
MOD_PATH = PLUGIN_MANAGER_DIR + MOD_DIR
MOD_AVAILABLE_DIR = "modules-available/"
MOD_ENABLED_DIR = "modules-enabled/"
MOD_AVAILABLE_PATH = MOD_PATH + MOD_AVAILABLE_DIR
MOD_ENABLED_PATH = MOD_PATH + MOD_ENABLED_DIR
PLUGIN_DIR = "script/"
PLUGIN_PATH = PLUGIN_MANAGER_DIR + PLUGIN_DIR

# 配置 日志路径
MANAGER_LOG_DIR = '/var/log/plugin-manager/'
# 默认插件日志路径
PLUGIN_FOLDER = 'plugin/'
PLUGIN_LOG_DIR = MANAGER_LOG_DIR + PLUGIN_FOLDER

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


#从文件获取name
def get_name_from_yaml(filePath):
    name = None
    if not os.path.exists(filePath):
        logging.error("[PLUGIN]： %s not exists.", filePath)
        return None
    cfg = None
    try:
        with open(filePath) as fd:
            cfg = yaml.load(fd)
        name = cfg.get(PLUGIN_CONF_KEY_NAME)
    except Exception as e:
        logging.error("[PLUGIN]: file [%s] load failed[%s].", filePath, e)
        return None

    return name

#根据name, 从列表中，获取文件路径
def get_yamlPath_from_name(distName):
    if not os.path.exists(MOD_AVAILABLE_PATH):
        logging.error("[PLUGIN]: %s not exists.", MOD_AVAILABLE_PATH)
        return None
    
    fileList = os.listdir(MOD_AVAILABLE_PATH)
    if len(fileList) == 0:
        logging.error("[PLUGIN]: There is no available modules in [%s].", MOD_AVAILABLE_PATH)
        return None

    for file in fileList:
        filePath = MOD_AVAILABLE_PATH + file
        tmpName = get_name_from_yaml(filePath)
        if tmpName == distName:
            return filePath
    logging.error("[PLUGIN]: can't find name[%s].", distName)
    return None

# 判断结尾是.yaml
def check_yaml_legal(fileName:str):
    if not fileName.endswith(".yaml"):
        logging.error("[PLUGIN]: [%s] is not yaml.", fileName)
        return False
    return True

class pluginSelf(pluginState, GObject.GObject):

    def __init__(self):
        GObject.GObject.__init__(self)
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
        self.fifoPath = PLUGIN_PATH + self.fifoName
        # 记录变量
        self.running = False        # 是否在运行
        self.process = 0            # 执行进度
        self.loadState = 0          # 插件配置完成
        # 执行结果
        self.returnCode = -1
        self.stdout = ''
        self.stderr = ''
        logging.info("init finished.")

    @GObject.Signal(flags=GObject.SignalFlags.RUN_FIRST, return_type=None, arg_types=(GObject.TYPE_INT,))
    def processChanged(self, process):
        # logging.info("[PLUGIN]: plugin [%s] process [%d].", self.pluginName, process)
        pass

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
                self.fifoPath = os.path.join(PLUGIN_PATH, self.pluginName, self.fifoName)
                self.logPath = os.path.join(self.logDir, self.pluginName + ".log")
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
                self.execCmd = os.path.join(PLUGIN_MANAGER_DIR,"script",self.pluginName, cfg[key])
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
                if process != '':
                    self.process = int(process.strip("\n"))
            except Exception as e:
                logging.info("[PLUGIN]: get process err[%s].",e)
        else:
            logging.info("[PLUGIN]: fifo[%s] not exists.", self.fifoPath)

        self.emit("processChanged", self.process)
        if self.process >= 100 or self.process < 0:
            logging.info("[PLUGIN]: process [%d] is over.", self.process)
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

        cfg = None
        try:
            with open(filePath) as fd:
                # cfg = yaml.load(fd, Loader=yaml.FullLoader)
                cfg = yaml.load(fd)
            fd.close()
        except Exception as e:
            logging.error("[PLUGIN]: file [%s] load failed[%s].", CFG_PATH, e)
            return self.PLUGINERR_PLUGIN_CONFIG_FAILED

        if cfg == None:
            logging.warning("[PLUGIN]: No config in [%s].", filePath)
            return self.PLUGINERR_PLUGIN_CONFIG_FAILED

        # 从已知的配置项中查找
        for key in PLUGIN_CONF_KEY_LIST:
            self._config_key(cfg, key)
        
        if self.loadState < LOADSTATE.STATESUM:
            logging.error("[PLUGIN]: config uncompleted.")
            return self.PLUGINERR_CONFIG_NOT_COMPLETED

        # TODO: 参数列表形式 重组
        
        
        return self.PLUGIN_SUCCESS

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

    def plg_get_result(self):
        return self.stdout, self.stderr

    # 添加 update cmd 
    # 设置脚本日志路径
    def plg_set_logDir(self, logDir):
        if not os.path.exists(logDir):
            try:
                os.makedirs(logDir, mode=0o755)
            except Exception as e:
                logging.error("[PLUGIN]: create plugin log dir failed.[%s]", e)
                return self.PLUGINERR_LOG_PATH_NOT_EXIT
        self.logDir = logDir
        if self.pluginName != None:
            self.logPath = os.path.join(self.logDir, self.pluginName + ".log")
        else:
            logging.error("[PLUGIN]: plugin name is none.")

    def plg_run(self):
        if not self.enabled or self.running:
            logging.error("[PLUGIN]: [%s] enable[%d] running[%d].", self.pluginName, self.enabled, self.running)
            return self.PLUGINERR_PLUGIN_NOT_ENABLED
        
        self.cmd = "bash " + self.execCmd + " fifoname=" + self.fifoName + " logpath=" + self.logPath + " loglevel=" + str(self.logLevel) + " modename=" + self.pluginName
        if self.cmd == None:
            logging.error("[PLUGIN]: cmd is None.")
            return self.PLUGINERR_CMD_IS_NONE
        
        self.running = True
        tmptimer = threading.Timer(0.5, function=self._update_process)
        tmptimer.start()

        logging.debug("[PLUGIN]: cmd[%s].",self.cmd)
        try:
            ret = subprocess.run(self.cmd, shell = True, stdout = subprocess.PIPE, stderr = subprocess.PIPE)
            self.returnCode = ret.returncode
            self.stdout = ret.stdout.decode()
            self.stderr = ret.stderr.decode()
        except Exception as e:
            logging.error("[PLUGIN]: subprocess run err[%s].", e)

        self.running = False
        logging.debug("[PLUGIN]: [%s] run finished ret[%d].",self.pluginName, ret.returncode)
        return ret.returncode


class PluginManager(pluginState):
    def __init__(self, plgPath = None):
        global PLUGIN_MANAGER_DIR
        if plgPath != None:
            PLUGIN_MANAGER_DIR = plgPath
        self.plgList = []        # 插件句柄
        self.availableList = []  # available文件列表
        self.loaded = False
        self.cfg = None
        self.managerLogDir = MANAGER_LOG_DIR
        self.pluginLogDir = PLUGIN_LOG_DIR

        if not os.path.exists(CFG_PATH):
            logging.debug("[PLUGIN]: [%s] not exist.", CFG_PATH)
        else:
            self._configure_from_yaml(CFG_PATH)

        # conf.d/
        # 1-判断文件名称合规性
        # 2-读取并配置
        if os.path.exists(CFG_EX_PATH):
            cfgList = os.listdir(CFG_EX_PATH)
            if len(cfgList) != 0:
                for file in cfgList:
                    if not check_yaml_legal(file):
                        logging.debug("[PLUGIN]: [%s] not a legal yaml conf file.", file)
                        continue
                    self._configure_from_yaml(os.path.join(CFG_EX_PATH, file))
            else:
                logging.debug("[PLUGIN]: [%s] no extra conf.", CFG_EX_PATH)


    def _config_manager_key(self, cfg, key):
        if cfg == None or key == None or key not in cfg:
            logging.warning("[PLUGIN]: key[%s] not in yaml.", key)

        # logging.info("[PLUGIN]: config key[%s] value[%s].", key, cfg[key])
        #TODO: 全局配置
        if key == MANAGER_CONF_KEY_LOGDIR:
            self.managerLogDir = cfg[key]
            self.pluginLogDir = cfg[key] + PLUGIN_FOLDER
            if not os.path.exists(self.managerLogDir):
                os.mkdir(self.managerLogDir, mode=0o755)
            logfile = os.path.join(self.managerLogDir, 'PluginManager.log')
            logging.basicConfig(format=FORMAT, level='DEBUG', datefmt='%m-%d,%H:%M:%S', filename=logfile, filemode='a')

    def _configure_from_yaml(self, filePath):
        if not os.path.exists(filePath):
            logging.debug("[PLUGIN]: [%s] not exist.", filePath)
            return

        cfg = None
        try:
            with open(filePath) as fd:
                # cfg = yaml.load(fd, Loader=yaml.FullLoader)
                cfg = yaml.load(fd)
            fd.close()
        except Exception as e:
            logging.error("[PLUGIN]: file [%s] load failed[%s].", CFG_PATH, e)
            return
        
        if cfg == None:
            logging.warning("[PLUGIN]: No config in [%s].", filePath)
            return

        for key in MANAGER_CONF_KEY_LIST:
            self._config_manager_key(cfg, key)    

    # 检测必要文件夹存在
    def _check_folder_exist(self):
        if not os.path.exists(MOD_AVAILABLE_PATH):
            logging.error("[PLUGIN]: %s not exists.", MOD_AVAILABLE_PATH)
            return self.PLUGINERR_NO_AVAILABLE_YAML
        if not os.path.exists(MOD_ENABLED_PATH):
            logging.error("[PLUGIN]: %s not exists.", MOD_ENABLED_PATH)
            return self.PLUGINERR_PLUGIN_NOT_ENABLED
        if not os.path.exists(PLUGIN_PATH):
            logging.error("[PLUGIN]: %s not exists.", PLUGIN_PATH)
            return self.PLUGINERR_PLUGIN_NOT_EXIST
        return self.PLUGIN_SUCCESS

    def load_All(self):
        ret = self._check_folder_exist()
        if ret != self.PLUGIN_SUCCESS:
            return

        fileList = os.listdir(MOD_AVAILABLE_PATH)
        if len(fileList) == 0:
            logging.error("[PLUGIN]: There is no available modules in [%s].", MOD_AVAILABLE_PATH)
            return
        
        enFileList = os.listdir(MOD_ENABLED_PATH)

        tmpAvailableDict = {}
        for file in fileList:
            if not check_yaml_legal(file):
                logging.debug("[PLUGIN]: [%s] not a legal yaml conf file.", file)
                continue
            filePath = MOD_AVAILABLE_PATH + file
            plgSelf = pluginSelf()
            if plgSelf == None:
                continue
            ret = plgSelf.plg_config(filePath)
            if ret != self.PLUGIN_SUCCESS:
                continue

            if file in enFileList:
                plgSelf.plg_enable()
            plgSelf.plg_set_logDir(self.pluginLogDir)
            self.plgList.append(plgSelf)
            tmpAvailableDict["path"] = filePath
            tmpAvailableDict[PLUGIN_CONF_KEY_NAME] = get_name_from_yaml(filePath)
            tmpAvailableDict["handle"] = plgSelf
            self.availableList.append(tmpAvailableDict)
        self.loaded = True
        return self.plgList

    def get_plugin_handle(self, name):
        for plgClass in self.plgList:
            if name == plgClass.plg_get_name():
                return plgClass
        logging.error("[PLUGIN]: [%s] not in plugin list.", name)
        return None

    def get_plugin_name(self, plgClass):
        if plgClass not in self.plgList:
            logging.error("[PLUGIN]: there is no this plugin in pluginList.")
            return self.PLUGINERR_PLUGIN_NOT_IN_LIST
        return plgClass.plg_get_name()
    
    def get_desc(self, plgClass):
        if plgClass not in self.plgList:
            logging.error("[PLUGIN]: there is no this plugin in pluginList.")
            return self.PLUGINERR_PLUGIN_NOT_IN_LIST
        return plgClass.plg_get_desc()

    def connect_signal(self, plgClass, signal, handler):
        if plgClass not in self.plgList:
            logging.error("[PLUGIN]: there is no this plugin in pluginList.")
            return self.PLUGINERR_PLUGIN_NOT_IN_LIST
        return plgClass.connect(signal, handler)
    
    def run_plugin(self, plgClass):
        if not self.loaded:
            logging.error("[PLUGIN]: please run load_all first.")
            return self.PLUGINERR_NOT_LOAD_ALL
        if plgClass not in self.plgList:
            logging.error("[PLUGIN]: there is no this plugin in pluginList.")
            return self.PLUGINERR_PLUGIN_NOT_IN_LIST

        return plgClass.plg_run()


def plugin_process_handler(plgClass, process):
    logging.info("[PLUGIN]: ***[%s]**** process [%d].",plgClass.plg_get_name() , process)


if __name__ == "__main__":

    pMClass = PluginManager()
    plgList = pMClass.load_All()
    for everyPlg in iter(plgList):
        name = pMClass.get_plugin_name(everyPlg)
        print("name:", name)
        desc = pMClass.get_desc(everyPlg)
        print("desc:", desc)
        pMClass.connect_signal(everyPlg, "processChanged", plugin_process_handler)

        ret = pMClass.run_plugin(everyPlg)

    exit(0)

