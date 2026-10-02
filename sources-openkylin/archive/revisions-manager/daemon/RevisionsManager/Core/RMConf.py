#!/usr/bin/python3
#TODO: more cfg file
#TODO: wrapping about section/key 

from configparser import ConfigParser
import logging
import os


class RMConf():
    def __init__(self,path) -> None:
        self.SEC_PERIOD='period'
        self.KEY_BACKPERIOD='backPeriod'
        self.SEC_LOG='logging'
        self.KEY_LOGLEVEL='logLevel'
        self.SEC_SWITCH='switch'
        self.KEY_SW_ETC='moveEtc'
        self.KEY_SW_APP='moveApp'
        self.KEY_SW_APPCON='convertApp'
        self.KEY_SW_SEC='moveSecutiry'

        self.path = path
        self.conf = ConfigParser()
        if os.path.exists(path):
            self.conf.read(path, encoding="utf-8")
        else:
            # 创建文件
            try:
                Fd = os.open(path, os.O_CREAT|os.O_RDWR)
                os.close(Fd)
            except Exception as e:
                logging.error("[Conf]: create file failed[%s].", e)

    #覆盖原key value
    def change_key_value(self, sec, key, value=None):
        self.conf.read(self.path, encoding="utf-8")
        if not self.conf.has_section(sec):
            self.conf.add_section(sec)
        self.conf.set(sec, key, value)
        self.conf.write(open(self.path, "w+", encoding="utf-8"))
        logging.info("[Conf]: change sec[%s] key[%s] value[%s]", sec, key, value)
    
    def get_key(self, sec, key):
        self.conf.read(self.path, encoding="utf-8")
        if not self.conf.has_section(sec):
            logging.info("[Conf]: There is not sec[%s].", sec)
            return None
        if not self.conf.has_option(sec, key):
            logging.info("[Conf]: There is not key[%s] in sec[%s].", key, sec)
            return None

        value = self.conf.get(sec, key)
        logging.info("[Conf]: Get sec[%s] key[%s] value[%s]", sec, key, value)
        return value

    def has_key(self, sec, key):
        return self.conf.has_option(sec,key)

    def clear_sec(self, sec):
        try:
            if self.conf.has_section(sec):
                self.conf.remove_section(sec)
                self.conf.write(open(self.path,'w+'))
        except Exception as Error:
            logging.error("[Conf]: clear sec[%s] err[%s]", sec, Error)


if __name__ == "__main__":
    path = "/etc/RevisionsManager/RevisionsManager.conf"
    rmConf = RMConf(path)
    pass
