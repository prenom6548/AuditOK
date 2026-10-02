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

if (len(sys.argv) != 9) :
    print(sys.argv[0] + ' arch pid vid manufacter model appName usb_class sn')
    print('example:\n\t' + sys.argv[0] + ' amd64  2109 0817 HP \'Color LaserJet MFP M277c6\' kylin-printer-test 07 00FDDD')
    print('example:\n\t' + sys.argv[0] + ' arm64  2109 0817 biaotop ZY-U98PD kylin-printer-test 07 00FDDD')
    print('example:\n\t' + sys.argv[0] + ' amd64  2109 0817 HP \'Color LaserJet 150\' kylin-printer-test 07 00FDDD')
    exit()

host = '172.20.184.183:8888'
# host = '10.42.44.77:8081'

# url = 'http://'+host +'/api/v1/getprintertype'
url = 'https://api.kylinos.cn/api/v1/getprintertype'


arch=sys.argv[1]
pid=sys.argv[2]
vid=sys.argv[3]
manufacter=sys.argv[4]
model=sys.argv[5]
appName=sys.argv[6]
usb_class=sys.argv[7]
sn=sys.argv[8]



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

if (len(appName)) != 0:
    appName = '&appName=' + quote(appName)

if (len(usb_class)) != 0:
    usb_class = '&usb_class=' + quote(usb_class)

if (len(sn)) != 0:
    sn = '&sn=' + quote(sn)

url = url +'?systemVersion=V10SP1%s%s%s%s%s%s%s%s' % (arch,pid,vid,manufacter,model,appName,usb_class,sn)


print(url)


response = requests.get(url)
print("Response Content:")
print(response.text)

