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

import logging
from subprocess import Popen, PIPE
import subprocess

def get_dist():
    " return the codename of the current runing distro "
    # then check the real one
    try:
        p = Popen(["lsb_release", "-i", "-s"], stdout=PIPE,
                universal_newlines=True)
        res = p.wait()
        if res != 0:
            logging.info("lsb_release returned exitcode: %i\n" % res)
            return "kylin"
        dist = p.stdout.readline().strip()
        p.stdout.close()
        # 将获取的内容全部转化为小写
        dist = dist.lower()
        return dist
    except Exception as e:
        logging.error(e)
        return "kylin"
    
def system_shutdown(reboot_required):
    if reboot_required:
        args = ["/sbin/shutdown","-r","now"]
    else:
        args = ["/sbin/shutdown","-h","now"]

    logging.info("The device will be %s by run system command(%r)...",reboot_required,args)
    p = subprocess.run(args, stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
    if p.returncode != 0:
        logging.info("The device has (%s) Failed...",reboot_required)
        return False