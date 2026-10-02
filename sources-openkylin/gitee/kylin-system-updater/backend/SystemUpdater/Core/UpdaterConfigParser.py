#!/usr/bin/python3
# -*- Mode: Python; indent-tabs-mode: nil; tab-width: 4; coding: utf-8 -*-
# DistUpgradeConfigParser.py 
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
#  USA


from configparser import MissingSectionHeaderError
from configparser import NoOptionError
from configparser import ConfigParser as SafeConfigParser
import os.path
import logging
import re

class UpgradeConfig(SafeConfigParser):
    def __init__(self, datadir="/var/lib/kylin-system-updater/", name="system-updater.conf",defaults_dir=None, allow_no_section=False):
        SafeConfigParser.__init__(self)
        self.datadir = datadir
        self.allow_no_section = allow_no_section
        self._no_section = False
        self._no_section_lines = []
        maincfg = os.path.join(datadir, name)
        # defaults are read first
        self.config_files = []
        if defaults_dir:
            self.config_files.append(os.path.join(datadir, defaults_dir))
        # our config file
        self.config_files += [maincfg]
        self._read_config()
        logging.info("Initialize Upgrade ConfigFile(%s) to success",str(self.config_files))

    def optionxform(self, optionstr):
        return optionstr
        
    def reRead(self):
        self._read_config()

    def _read_config(self):
        try:
            self.read(self.config_files, encoding='iso-8859-1')
            self._no_section = False
        except MissingSectionHeaderError:
            if not self.allow_no_section:
                raise
            self.clear()
            config_file = self.config_files[-1]
            with open(config_file, 'r', encoding='iso-8859-1') as configfile:
                self._no_section_lines = configfile.readlines()
            self.read_string('[SYSTEM]\n' + ''.join(self._no_section_lines))
            self._no_section = True

    def _render_no_section(self):
        values = self._sections.get('SYSTEM', {})
        found_options = set()
        lines = []
        option_pattern = re.compile(r'^(\s*)([^#;\s][^=]*?)(\s*)=(.*?)(\r?\n)?$')
        for line in self._no_section_lines:
            match = option_pattern.match(line)
            option = match.group(2).strip() if match else None
            if option not in values:
                lines.append(line)
                continue
            newline = match.group(5) or '\n'
            lines.append('{}{} = {}{}'.format(
                match.group(1), option, values[option], newline))
            found_options.add(option)

        for option, value in values.items():
            if option not in found_options:
                lines.append('{} = {}\n'.format(option, value))
        return lines

    def write(self, fp, space_around_delimiters=True):
        if self._no_section:
            lines = self._render_no_section()
            fp.writelines(lines)
            self._no_section_lines = lines
            return
        SafeConfigParser.write(self, fp, space_around_delimiters)

    def setValue(self, section, option, value=None,is_write = True):
        if option != 'upgradelist':
            logging.info("SetValue Section:%s Option:%s Value:%s",section, option, value)
        try:
            self.reRead()

            self.set(section, option,value)
        except Exception as e:
                logging.error("Error: setValue section:%s option:%s value:%s",section, option, value)
                logging.error(str(e))
                return False
        if is_write == True:
            with open(self.config_files[-1], 'w+') as configfile:
                self.write(configfile)
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
            with open(self.config_files[-1], 'w+') as configfile:
                self.write(configfile)

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
    # c = UpgradeConfig("/home/x/share/kylin-system-updater/backend/data/")
    # print(c.setValue("SystemStatus", "abnormal_reboot", str(False)),True)
    # print(c.getWithDefault("SystemStatus", "abnormal_reboot", False))
    pass
