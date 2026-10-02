#!/usr/bin/python3
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

import dbus
import psutil
import signal
import logging

STRATEGIES_DBUS_PATH = '/com/kylin/UpgradeStrategies'
STRATEGIES_DBUS_SERVICE = 'com.kylin.UpgradeStrategies'
STRATEGIES_DBUS_INTERFACE = 'com.kylin.UpgradeStrategies.interface'

process_names = [
    "kylin-upgrade-strategies",
    "kconf2 monitor kylin-update-frontend."]

def _quit_strategies_service():
    try:
        bus = dbus.SystemBus()
        the_other_guy = bus.get_object(STRATEGIES_DBUS_SERVICE,
                                        STRATEGIES_DBUS_PATH)
        the_other_guy.Quit(dbus_interface=STRATEGIES_DBUS_INTERFACE,
                            timeout=2)
    except Exception as e:
        pass

def kill_process_tree(pid, sig=signal.SIGTERM):
    try:
        parent = psutil.Process(pid)
    except psutil.NoSuchProcess:
        return

    children = parent.children(recursive=True)
    for child in children:
        try:
            child.send_signal(sig)
        except psutil.NoSuchProcess:
            pass

    try:
        parent.send_signal(sig)
    except psutil.NoSuchProcess:
        pass

def _quit_force_strategies_service():
    try:
        for proc in psutil.process_iter(['pid', 'name', 'cmdline']):
            if isinstance(proc.info['cmdline'], list):
                cmdline_str = ' '.join(proc.info['cmdline'])

            if any(name in cmdline_str for name in process_names):
                print(f"正在终止进程 {cmdline_str} - {proc}")
                kill_process_tree(proc.info['pid'])
    except (psutil.NoSuchProcess, psutil.AccessDenied, psutil.ZombieProcess):
        pass
    except Exception as e:
        logging.error("_quit_force_strategies_service err:%s", e)

if __name__ == "__main__":
    _quit_strategies_service()
    _quit_force_strategies_service()