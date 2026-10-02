#!/bin/bash
ukui-screensaver-dialog --lock-startup & >/dev/null 2>&1
#/usr/lib/ukui-screensaver/screensaver-focus-helper & >/dev/null 2>&1

#如果锁屏和桌面进程并行启动，会对锁屏启动时间有较大影响，因此先启动锁屏，1s后再往下执行
sleep 1
