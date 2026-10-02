#!/usr/bin/python3
# DistUpgradeConfigParser.py 
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307

import json
import logging

class Singleton(object):
    def __init__(self, cls):
        self._cls = cls
        self._instance = {}
    def __call__(self):
        if self._cls not in self._instance:
            self._instance[self._cls] = self._cls()
        return self._instance[self._cls]

@Singleton
class JsonConfig():
    NOW_UPDATE_CONFIG = '/usr/share/kylin-update-desktop-config/config/kylin-update-desktop-system.json'

    def __init__(self):
        self._configDate = None
        self.read()

    def read(self):
        try:
            with open(self.NOW_UPDATE_CONFIG,'r') as f:
                logging.info("Reading json configuration file...")

                self._configDate = json.load(f)
        except Exception as exc:
            logging.warning("Reading json configuration file,check json format...")
            logging.warning(exc)
            self._configDate = None
    
    def getWithDefault(self,key1=None,key2=None,key3=None,key4=None,default=None):
        try:
            if self._configDate == None:
                logging.warning("Reading json configuration file,check json format...")
                return default

            if key4 != None:
                return self._configDate[key1][key2][key3][key4]
            elif key3 != None:
                return self._configDate[key1][key2][key3]
            elif key2 != None:
                return self._configDate[key1][key2]
            elif key1 != None:
                return self._configDate[key1]
            else:
                return self._configDate
        except Exception as e:
            logging.warning(str(e))
            return default
    

if __name__ == "__main__":
    # c = UpgradeConfig("/home/x/share/kylin-system-updater/backend/data/")
    # print(c.setValue("SystemStatus", "abnormal_reboot", str(False)),True)
    # print(c.getWithDefault("SystemStatus", "abnormal_reboot", False))
    pass
