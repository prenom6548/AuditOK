#!/bin/python3

#  Copyright (C) 2020, KylinSoft Co., Ltd.
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <https://www.gnu.org/licenses/>.

import os
import cups

# print(os.environ.get('USER'))

jobStrList = os.popen('lpstat -W all -u %s | awk \'{print $1}\'' % os.environ.get('USER'))

jobs = []
for jobStr in jobStrList:
    jobStr = jobStr.rstrip()
    jobStr = jobStr[jobStr.rindex('-') + 1 : len(jobStr)]
    # print(jobStr)
    jobs.append(int(jobStr))

c = cups.Connection()
for job in jobs:
    try:
        c.cancelJob(job, False)
    except cups.IPPError as e:
        (e, s) = e.args
        # print(e,s)
for job in jobs:
    try:
        c.cancelJob(job, True)
    except cups.IPPError as e:
        (e, s) = e.args
        # print(e,s)
for job in jobs:
    try:
        c.cancelJob(job, True)
    except cups.IPPError as e:
        (e, s) = e.args
        # print(e,s)
