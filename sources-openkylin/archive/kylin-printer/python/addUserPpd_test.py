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
import requests
import json


if (len(sys.argv) != 9) :
    print(sys.argv[0] + ' arch pid vid manufacter model ppdName ppdRank packageLevel')
    # print('example:\n\t' + sys.argv[0] + ' amd64  2109 0817 HP \'Color LaserJet MFP M277c6\' kylin-printer-test 07 00FDDD')
    print('example:\n\t' + sys.argv[0] + ' arm64  2109 0817 biaotop ZY-U98PD aaa.ppd 10 None')
    exit()

# 定义请求的URL
host = '172.20.107.254:8888'
# host = '10.42.44.77:8081'
url = 'http://' + host + '/api/v1/addUserPdd'

# 定义要发送的数据
payload = json.dumps({
    "arch":sys.argv[1],
    "pid":sys.argv[2],
    "vid":sys.argv[3],
    "manufacter":sys.argv[4],
    "model":sys.argv[5],
    "ppdName":sys.argv[6],
    "ppdRank":int(sys.argv[7]),
    "packageLevel":sys.argv[8],
    "systemVersion":"V10SP1"
})

jsondata=json.dumps({"model": "ZY-U98PD", "ppdRank": 27, "systemVersion": "V10SP1", "manufacter": "biaotop", "arch": "arm64","ppdName": "2"})

headers = {'Content-Type': 'application/json'}

# print(json_data)
# 发送POST请求
response = requests.request("POST", url, headers=headers, data=jsondata)
# 打印响应内容
print(response.text)


# curl -X POST -d "arch=amd64&manufacter=HP&SystemVersion=V10SP1&model=Color%20LaserJet%20MFP%20M277c6&vid=0817&pid=2109&ppdName=aaa.ppd&ppdRank=100&packageLevel=3" http://172.20.107.254:8888/api/addUserPpd
# curl -X POST -H "Content-Type: application/json" -d '{"arch": "arm64", "pid": "2109", "vid": "0817", "manufacter": "biaotop", "model": "ZY-U98PD", "ppdName": "1", "ppdRank": 10, "packageLevel": "None", "systemVersion": "V10SP1"}' http://10.42.44.77:8081/api/v1/addUserPpd

# curl -X POST -H "Content-Type: application/json" -d '{ "model": "ZY-U98PD", "ppdRank": 27, "systemVersion": "V10SP1", "manufacter": "biaotop", "arch": "arm64","ppdName": "2"}' http://10.42.44.77:8081/api/v1/addUserPpd

# { "model": "ZY-U98PD", "ppdRank": 27, "systemVersion": "V10SP1", "manufacter": "biaotop", "arch": "arm64","ppdName": "2"}