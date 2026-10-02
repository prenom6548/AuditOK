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

import sys
from urllib.parse import quote
import requests

if (len(sys.argv) != 6) :
    print(sys.argv[0] + ' arch pid vid manufacter model')
    print('example:\n\t' + sys.argv[0] + ' amd64  2109 0817 HP \'Color LaserJet MFP M277c6\'')
    exit()


arch=sys.argv[1]
pid=sys.argv[2]
vid=sys.argv[3]
manufacter=sys.argv[4]
model=sys.argv[5]

if (len(arch)) == 0:
    print('arch is empty!')
    exit()

if (len(arch)) != 0:
    arch = '&arch=' + quote(arch)

if (len(pid)) != 0:
    pid = '&pid=' + quote(pid)

if (len(vid)) != 0:
    vid = '&vid=' + quote(vid)

if (len(manufacter)) != 0:
    manufacter = '&manufacter=' + quote(manufacter)

if (len(model)) != 0:
    model = '&model=' + quote(model)


url = 'https://api.kylinos.cn/api/v1/getprintertype?systemVersion=V10SP1%s%s%s%s%s' % (arch,pid,vid,manufacter,model)


print(url)


response = requests.get(url)
print("Response Content:")
print(response.text)
