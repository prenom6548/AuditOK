#!/usr/bin/python3

import logging
import os

OSRELEASE_PATH = "/etc/os-release"
RMSUCCESS = 0
RMFAILURE = -1


class RMOsInfo():
    def __init__(self):
        self.osName = None
        self.prettyName = None
        self.proCodeName = None
        self.versionId = None

        self._load_os_release()

    def _load_os_release(self):
        if not os.path.exists(OSRELEASE_PATH):
            logging.error("[OSINFO]: os-release not exists.")
            return RMFAILURE
        infoList = []
        with open(OSRELEASE_PATH, "r") as fd:
            infoList = fd.readlines()

        if len(infoList) == 0:
            logging.error("[OSINFO]: no info in os-release.")
            return RMFAILURE
        
        for ele in infoList:
            tmpList = ele.strip("\n").split("=")
            if len(tmpList) != 2:
                logging.debug("[OSINFO]: info[%s] is not standard.", ele.strip("\n"))
                continue
            value = tmpList[1].strip("\"")
            if tmpList[0] == "NAME":
                self.osName = value
            elif tmpList[0] == "PRETTY_NAME":
                self.prettyName = value
            elif tmpList[0] == "PROJECT_CODENAME":
                self.proCodeName = value
            elif tmpList[0] == "VERSION_ID":
                self.versionId = value
        
        return RMSUCCESS

    def get_pretty_name(self):
        return self.prettyName
    def get_os_name(self):
        return self.osName
    def get_version_id(self):
        return self.versionId


if __name__ == "__main__":

    osInfo = RMOsInfo()
    osInfo.get_os_name()

