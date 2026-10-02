#!/bin/python3

import os
import subprocess
import sys

workspaceDir = os.path.abspath(os.path.dirname(__file__) + '/../')
sourceDir = os.path.abspath(workspaceDir + '/../../../../work/kylin-printer/')
print(workspaceDir, sourceDir)

def copy_file(source, destination):
    try:
        result = subprocess.run(['cp', source, destination], check=True, text=True, capture_output=True)
        print(f"文件已复制: {source} -> {destination}")
    except subprocess.CalledProcessError as e:
        print(f"错误: {e.stderr}")

def move_file(source, destination):
    try:
        result = subprocess.run(['mv', source, destination], check=True, text=True, capture_output=True)
        print(f"文件已移动: {source} -> {destination}")
    except subprocess.CalledProcessError as e:
        print(f"错误: {e.stderr}")

list=[
    "cmake/printer.cmake",
    "cmake/common.cmake",
    "ui/new_pop_window",
    "ui/new_pop_window/pop_window_manager.h",
    "ui/new_pop_window/pop_window_manager.cpp",
    "ui/gsettings_monitor.cpp",
    "ui/gsettings_monitor.h",
    "device_manager/udev_devices.cpp",
    "device_manager/device_information.cpp",
    "device_manager/device_information.h",
    "device_manager/device_monitor.h",
    "device_manager/device_monitor.cpp",
    "device_manager/udev_devices.h",
    "backend/launch_printer.cpp",
    "backend/device_map.cpp",
    "backend/device_map.h",
    "backend/launch_printer.h",
    "util/information_collector.h",
    "util/system_notification.cpp",
    "util/information_collector.cpp",
    "util/system_notification.h",
    "printer_manager/find_and_match_ppds.h",
    "printer_manager/lpinfo_browser.cpp",
    "printer_manager/cupsconnection4ppds.cpp",
    "printer_manager/cupsconnection4ppds.h",
    "printer_manager/lpinfo_browser.h",
    "printer_manager/ukui_apt.h",
    "printer_manager/package_helper.h",
    "printer_manager/ukui_printer.cpp",
    "printer_manager/findppdsthread.h",
    "printer_manager/findppdsthread.cpp",
    "printer_manager/ukui_printer.h",
    "printer_manager/find_and_match_ppds.cpp",
    "printer_manager/matchppdsthread.cpp",
    "printer_manager/matchppdsthread.h",
    "printer_manager/ukui_apt.cpp",
    "printer_manager/package_helper.cpp",
    "common/uri.h",
    "common/base_info.cpp",
    "common/common.h",
    "common/custom_qstring_converter.cpp",
    "common/global_variable.h",
    "common/common.cpp",
    "common/uri.cpp",
    "common/singleton.h",
    "common/custom_qstring_converter.h",
    "common/base_info.h",
    "translations/kylin-printer_bo_CN.ts",
    "translations/kylin-printer_en.ts",
    "translations/kylin-printer_kk.ts",
    "translations/kylin-printer_ky.ts",
    "translations/kylin-printer_mn.ts",
    "translations/kylin-printer_ru.ts",
    "translations/kylin-printer_ug.ts",
    "translations/kylin-printer_zh_CN.ts",
    "translations/kylin-printer_zh_HK.ts",
]

for path in list:
    sourcePath = sourceDir + "/" + path
    destPath   = workspaceDir + "/" + path
    copy_file(sourcePath, destPath)