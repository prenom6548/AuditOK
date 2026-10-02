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

import cups
import subprocess
import re
import sys
import signal


def run_command_with_timeout(command, timeout):
    try:
        # Start the command
        process = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        # Wait for the process to complete or raise a TimeoutExpired exception
        stdout, stderr = process.communicate(timeout=timeout)
        
        # Check if the command completed successfully
        if process.returncode == 0:
            return stdout.decode(), None
        else:
            return None, stderr.decode()
    except subprocess.TimeoutExpired:
        # If timeout occurs, kill the process and raise TimeoutExpired exception
        process.kill()
        return None, "Command timed out!"

def extract_numbers(string):
    pattern = r'\d+'
    return ''.join(re.findall(pattern, string))

def show_help():
    print ("使用方法: 运行此脚本后，会监控系统中ipp打印机返回的页数信息，并进行显示,例如：\n"
           "发现可用ipp打印机\n"
           "HP-LaserJet-M407 ipp://10.41.120.170:631/ipp/print\n"
           "\n"
           "printer_name | jobid | page\n"
           "HP-LaserJet-M407 | 12 | 0\n"
           "HP-LaserJet-M407 | 12 | 1\n"
           "HP-LaserJet-M407 | 12 | 2")

def signal_handler(sig, frame):
    sys.exit(0)

signal.signal(signal.SIGINT, signal_handler)  # 捕获键盘中断信号
signal.signal(signal.SIGTERM, signal_handler)  # 捕获终止信号


if len(sys.argv)>1 and sys.argv[1] == '--help':
    show_help ()
    sys.exit (0)


conn = cups.Connection()
printers = conn.getPrinters()

printer_list = {}


for printer in printers:
    printer_info = printers[printer]
    
    uri = printer_info['device-uri']

    if ('http://' in uri or 
        'https://' in uri or
        'ipp://' in uri or
        'ipps://' in uri
       ) :
        printer_list[printer] = printer_info



if not printer_list:
    print("未发现可用ipp打印机")
    sys.exit()
else:
    print("发现可用ipp打印机")
for printer_name, printer_info in printer_list.items():
    print(printer_name, printer_info['device-uri'])

print("\nprinter_name | jobid | page")
while True:
    for printer_name, printer_info in printer_list.items():
        command = 'ipptool -t -v %s /usr/share/cups/ipptool/get-jobs.test' % printer_info['device-uri']
        timeout = 0.5
        stdout, stderr = run_command_with_timeout(command, timeout)

        if stdout is None:
            # print("Error:", stderr)
            continue
        list = stdout.split('\n')
        job_id = -1
        job_impressions_completed = -1
        for str in list:
            str = str.rstrip()
            if 'job-id (integer) = ' in str:
                job_id = extract_numbers(str)
            if 'job-impressions-completed (integer)' in str:
                job_impressions_completed = extract_numbers(str)
        
        if job_id != -1 and job_impressions_completed != -1:
            if job_id in printer_info and job_impressions_completed == printer_info[job_id]:
                continue
            else:
                printer_info[job_id] = job_impressions_completed
                print('%s | %s | %s' % (printer_name, job_id, job_impressions_completed))
    pass




