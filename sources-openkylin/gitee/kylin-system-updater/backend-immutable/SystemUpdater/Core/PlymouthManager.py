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

import os
import time
from gettext import gettext as _
import logging
import subprocess

class PlymouthManager():
    def __init__(self):
        # type: (str) -> None
        self.has_started = False

    def message_to(self,message):
        subprocess.call(["/bin/plymouth", "message", "--text", message])

    def progress_to(self,progress):
        tmp = ("--progress=%d"%progress)
        run_cmd = ["/bin/plymouth","system-update",tmp]
        subprocess.call(run_cmd)

    def check_state(self):
        try:
            args = ["/bin/plymouth","--ping"]
            p = subprocess.run(args,timeout=2,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
            if p.returncode == 0:
                logging.info("Check: Plymouthd state start to success...")
            else:
                logging.info("Check: Plymouthd Failed to Boot and Restart boot up again...")
                self._start_plymouth_splash()
        except Exception as exc:
            logging.error(exc) 

    def show(self):
        try:
            if not self.has_started:
                if os.path.exists("/sbin/plymouthd"):
                    if not self._check_device_type():
                        args = ["/sbin/plymouthd", "--mode=shutdown","--attach-to-session","--kernel-command-line=splash plymouth.use-legacy-input"]
                    else:
                        os.environ["DISPLAY"] = ":0"
                        args = ["/sbin/plymouthd", "--mode=shutdown","--attach-to-session","--kernel-command-line=splash plymouth.use-legacy-input"]
                    logging.info("Running plymouth:%r",args)
                    subprocess.run(args,timeout=3)
                if os.path.exists("/bin/plymouth"):
                    subprocess.Popen(["/bin/plymouth", "show-splash","--wait"])
            # 标识Plymouth界面已经启动 不需要再次开启
            self.has_started = True
        except Exception as exc:
            logging.error(exc)

    def quit_retain(self):
        if os.path.exists("/bin/plymouth"):
            subprocess.run(["/bin/plymouth","quit","--retain-splash"])

    def _check_device_type(self):
        try:
            with open("/proc/cpuinfo", 'r') as f:
                for line in f.readlines():
                    if "Hardware" in line and ":" in line:
                        cpuinfos = line.split(':')
                        if "PANGU M900" in cpuinfos[1]:
                            logging.info("Get cpuinfo: PANGU M900")
                            return True
        except Exception as exc:
            logging.error(exc)
        return False
    
    def show_countdown(self,success,reboot_required):
        try:
            state = "reboot" if reboot_required else "shutdown"
            success_msg = "The system update is successful and your computer will %s "%state + "in %s s"
            failed_msg = "System update is abnormal and your computer will %s "%state + "in %s s"
            if success:
                msg = _(success_msg)
            else:
                msg = _(failed_msg)

            for i in range(5, 0, -1):
                msges = msg%str(i)
                self.message_to(msges)
                time.sleep(1)
                logging.info(msges)
        except Exception as exc:
            logging.error(exc)   


    def clean_all(self):
        self.message_to("")
        self.progress_to(0)
