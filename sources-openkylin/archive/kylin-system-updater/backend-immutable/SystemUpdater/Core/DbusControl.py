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

import os
import configparser
import logging
import dbus
from gettext import gettext as _

from SystemUpdater.configs import settings
from SystemUpdater.Core.errors import *
from SystemUpdater.Core.enums import *

class DbusControl():
    def __init__(self,dbus):
        self.dbus = dbus
        self.debug = settings.get("debug")
        self.prohibit_env = ["LD_PRELOAD","LD_LIBRARY_PATH","LD_AUDIT"]
        self.config = configparser.ConfigParser()
        self.whitelist_values = []
        try:
            self.config.read("/etc/dbus-1/conf/com.kylin.UpgradeStrategies.limit")
            whitelist = self.config['whitelist']
            for key in whitelist:
                self.whitelist_values.append(whitelist[key])
        except Exception as e:
            logging.warning("Skip dbus control and dsc:%s",str(e))

    def _get_environment_from_pid(self, pid):
        try:
            with open(f'/proc/{pid}/environ', 'r') as f:
                environ = f.read()
            # 环境变量用 '\0' 分隔，替换为换行符便于读取
            env_vars = environ.split('\0')
            env_dict = {var.split('=')[0]: var.split('=')[1] for var in env_vars if '=' in var}
            return env_dict
        except Exception as e:
            logging.error("Error reading environment variables: %s",str(e))
            return None

    def _get_process_path_from_pid(self, pid):
        try:
            # 读取 /proc/[pid]/exe 符号链接获取进程路径
            process_path = os.readlink(f'/proc/{pid}/exe')
            if process_path.startswith('/usr/bin/python'):
                cmd_file = '/proc/%u/cmdline' % pid
                with open(cmd_file, 'r') as cf:
                    cmd_line = cf.readline()
                    cmds = cmd_line.split('\0')
                    if len(cmds) >= 2:
                        process_path = cmds[1]
            return process_path
        except Exception as e:
            return str(e)

    def _get_pid_from_dbus_name(self,dbus_name, bus=None):
        """Return a deferred that gets the id of process owning the given
        system D-Bus name.
        """
        bus_obj = self.dbus.get_object("org.freedesktop.DBus",
                                "/org/freedesktop/DBus/Bus")
        pid = bus_obj.GetConnectionUnixProcessID(dbus_name,
                                        dbus_interface="org.freedesktop.DBus")
        # proc = psutil.Process(int(pid))

        return int(pid)

    def check_sender(self,sender,skip_check=False):
        pid = self._get_pid_from_dbus_name(sender)

        process_path = self._get_process_path_from_pid(pid)

        if self.debug or skip_check:
            return process_path

        # /opt下面的进程则直接允许调用
        if "/opt/update-module" in process_path:
            return process_path

        # 检查运行白名单
        if self.whitelist_values and process_path not in self.whitelist_values:
            logging.error("dbus method control,operation not permitted. sender:%s",process_path)
            raise dbus.exceptions.DBusException("dbus method control,operation not permitted.")

        # # 检查环境变量
        # env_vars = self._get_environment_from_pid(pid)

        # if env_vars:
        #     for env in self.prohibit_env:
        #         if env in env_vars:
        #             logging.error("dbus method control,env forbidden. sender:%s env:%s",process_path,env)
        #             raise dbus.exceptions.DBusException("dbus method control,env forbidden.")
        return process_path