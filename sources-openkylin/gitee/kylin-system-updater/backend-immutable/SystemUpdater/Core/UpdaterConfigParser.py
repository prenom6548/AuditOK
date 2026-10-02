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
# -*- Mode: Python; indent-tabs-mode: nil; tab-width: 4; coding: utf-8 -*-
# DistUpgradeConfigParser.py 
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
#  USA


from configparser import NoOptionError
from configparser import ConfigParser as SafeConfigParser
import os.path
import os
import logging

class UpgradeConfig(SafeConfigParser):
    def __init__(self, datadir="/var/lib/kylin-system-updater/", name="system-updater.conf",defaults_conf=None):
        SafeConfigParser.__init__(self)
        self.datadir = datadir
        maincfg = os.path.join(datadir, name)
        # defaults are read first
        self.config_files = []
        if defaults_conf:
            self.config_files.append(defaults_conf)
        # our config file
        self.config_files += [maincfg]
        for conf in self.config_files:
            if not os.path.exists(conf):
                logging.warning("Check: Conf(%s) not exits...",conf)
        self.read(self.config_files, encoding='iso-8859-1')
        logging.info("Initialize Upgrade ConfigFile(%s) to success",str(self.config_files))

    def optionxform(self, optionstr):
        return optionstr
        
    def reRead(self):
        self.read(self.config_files, encoding='iso-8859-1')

    def _write(self):
        with open(self.config_files[-1], 'w+') as configfile:
            self.write(configfile)

    def setValue(self, section, option, value=None,is_write = True):
        logging.info("SetValue Section:%s Option:%s Value:%s",section, option, value)
        try:
            self.reRead()
            self.set(section, option,value)
        except Exception as e:
                logging.error("Error: setValue section:%s option:%s value:%s",section, option, value)
                logging.error(str(e))
                return False
        if is_write == True:
            self._write()
        return True

    def getWithDefault(self, section, option, default,re_read=False):
        try:
            if re_read == True:
                self.reRead()

            if type(default) == bool:
                return self.getboolean(section, option)
            elif type(default) == float:
                return self.getfloat(section, option)
            elif type(default) == int:
                return self.getint(section, option)
            return self.get(section, option)
        except Exception as e:
            logging.error(str(e))
            return default

    def getlist(self, section, option):
        try:
            tmp = self.get(section, option)
        except Exception as e:
            logging.error(str(e))
            return []
        items = [x.strip() for x in tmp.split(" ")]
        if '' in items and len(items):
            return []
        return items

    def setListValue(self, section, option, value=None,is_write = True):
        tmp = str(value).replace('[', '').replace(']', '')
        tmp = tmp.replace("'", '').replace(',', '')
        try:
            self.set(section, option,tmp)
        except Exception as e:
            logging.error("setListValue section:%s option:%s",section, option)
            logging.error(str(e))
            return
        if is_write == True:
            self._write()

    def getListFromFile(self, section, option):
        try:
            filename = self.get(section, option)
        except NoOptionError:
            return []
        p = os.path.join(self.datadir, filename)
        if not os.path.exists(p):
            logging.error("getListFromFile: no '%s' found" % p)
        with open(p) as f:
            items = [x.strip() for x in f]
        return [s for s in items if not s.startswith("#") and not s == ""]


if __name__ == "__main__":
    pass
