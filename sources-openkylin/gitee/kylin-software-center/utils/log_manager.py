#!/usr/bin/python3
# -*- coding: utf-8 -*-
__author__ = "huangsheng"

import os
import sys
# reload(sys)
# sys.setdefaultencoding('utf8')
# from models.enums import ERR_LOG,DEBUG_LOG,INFO_LOG,UKSC_CACHE_DIR_LOG

import logging
from logging import handlers
from xdg import BaseDirectory as xdg

UKSC_CACHE_DIR = os.path.join(xdg.xdg_cache_home, "uksc")
if not os.path.exists(UKSC_CACHE_DIR):
        try:
            os.makedirs(UKSC_CACHE_DIR)
        except OSError as e:
            pass
UKSC_CACHE_DIR_LOG = os.path.join(UKSC_CACHE_DIR,"log")
if not os.path.exists(UKSC_CACHE_DIR_LOG):
        try:
            os.makedirs(UKSC_CACHE_DIR_LOG)
        except OSError as e:
            pass
ERR_LOG = os.path.join(UKSC_CACHE_DIR_LOG,"kylin-software-center-error.log")
INFO_LOG = os.path.join(UKSC_CACHE_DIR_LOG,"kylin-software-center-info.log")
DEBUG_LOG = os.path.join(UKSC_CACHE_DIR_LOG,"kylin-software-center-debug.log")

class LogManager(object):
    level_relations = {
        'debug': logging.DEBUG,
        'info': logging.INFO,
        'warning': logging.WARNING,
        'error': logging.ERROR,
        'crit': logging.CRITICAL
    }  # 日志级别关系映射

    def __init__(self, filename, level='info', when="W0", backCount=1,
                 fmt='%(asctime)s - %(pathname)s[line:%(lineno)d] - %(levelname)s: %(message)s'):
        error_log_file = ERR_LOG.split("/")[len(ERR_LOG.split("/"))-1]
        info_log_file = INFO_LOG.split("/")[len(INFO_LOG.split("/")) - 1]
        debug_log_file = DEBUG_LOG.split("/")[len(DEBUG_LOG.split("/")) - 1]
        if(os.path.exists(UKSC_CACHE_DIR_LOG)):
            for file in os.listdir(UKSC_CACHE_DIR_LOG):
                if(file != info_log_file and file != error_log_file and file != debug_log_file):
                    os.remove(os.path.join(UKSC_CACHE_DIR_LOG,file))
        self.logger = logging.getLogger(filename)
        format_str = logging.Formatter(fmt)  # 设置日志格式
        self.logger.setLevel(self.level_relations.get(level))  # 设置日志级别
        # sh = logging.StreamHandler()  # 往屏幕上输出
        # sh.setFormatter(format_str)  # 设置屏幕上显示的格式
        th = handlers.TimedRotatingFileHandler(filename=filename, when=when, backupCount=backCount,
                                               encoding='utf-8')  # 往文件里写入#指定间隔时间自动生成文件的处理器
        # 实例化TimedRotatingFileHandler
        # interval是时间间隔，backupCount是备份文件的个数，如果超过这个个数，就会自动删除，when是间隔的时间单位，单位有以下几种：
        # S 秒
        # M 分
        # H 小时、
        # D 天、
        # W 每星期（interval==0时代表星期一）
        # midnight 每天凌晨
        th.setFormatter(format_str)  # 设置文件里写入的格式
        # self.logger.addHandler(sh)  # 把对象加到logger里
        self.logger.addHandler(th)

# class LogManager():
#     LOG = None
#
#     # @classmethod
#     # def INSTALL(self, msg):
#     #     try:
#     #         syslog.openlog("kylin-update-manager-install",syslog.LOG_PID,syslog.LOG_USER)
#     #         syslog.syslog(syslog.LOG_INFO, msg)
#     #         syslog.closelog()
#     #         return True
#     #     except Exception as e:
#     #         # print("syslog failed: ", e)
#     #         return False
#
#     @classmethod
#     def INFO(self, msg):
#         try:
#             syslog.openlog("kylin-software-center-info",syslog.LOG_PID,syslog.LOG_USER)
#             syslog.syslog(syslog.LOG_INFO, msg)
#             syslog.closelog()
#             return True
#         except Exception as e:
#             # print("syslog failed: ", e)
#             return False
#
#     # @classmethod
#     # def WARN(self, msg):
#     #     try:
#     #         syslog.openlog("kylin-update-manager-kum",syslog.LOG_PID,syslog.LOG_USER)
#     #         syslog.syslog(syslog.LOG_WARNING, msg)
#     #         syslog.closelog()
#     #         return True
#     #     except Exception as e:
#     #         # print("syslog failed: ", e)
#     #         return False
#
#     @classmethod
#     def ERR(self, msg):
#         try:
#             syslog.openlog("kylin-software-center-erro",syslog.LOG_PID,syslog.LOG_USER)
#             syslog.syslog(syslog.LOG_ERR, msg)
#             syslog.closelog()
#             return True
#         except Exception as e:
#             # print("syslog failed: ", e)
#             return False
#
#     # @classmethod
#     # def CRI(self, msg):
#     #     try:
#     #         syslog.openlog("kylin-update-manager-kum",syslog.LOG_PID,syslog.LOG_USER)
#     #         syslog.syslog(syslog.LOG_CRIT, msg)
#     #         syslog.closelog()
#     #         return True
#     #     except Exception as e:
#     #         # print("syslog failed: ", e)
#     #         return False
#
#
# #
# # 函数：初始化日志系统
# #
# # def init_log():
#
#     # cache_dir = os.environ.get('XDG_CACHE_HOME','').strip()
#     # if not cache_dir:
#     #     cache_dir = os.path.expanduser("~/.cache")
#     # log_file = os.path.join(cache_dir, "kum.log")
#     # LogManager.LOG = logging.getLogger("KUM")
#     # LogManager.LOG.propagate = False
#     # LogManager.LOG.setLevel(logging.DEBUG)
#     # log_handler = logging.handlers.RotatingFileHandler(log_file, maxBytes=1024*1024, backupCount=5)
#     # log_formatter = logging.Formatter("[%(asctime)s - %(levelname)s - %(message)s")
#     # log_handler.setFormatter(log_formatter)
#     # LogManager.LOG.addHandler(log_handler)
#
