#!/bin/sh
set -e 
/sbin/plymouthd --mode=boot --pid-file=/run/plymouth/pid --attach-to-session
/bin/plymouth show-splash

/bin/plymouth message --text="正在启动, 请稍后 ..."
sleep 1

/bin/plymouth change-mode --system-upgrade    
/bin/plymouth message --text="正在检测系统升级"
sleep 1

/bin/plymouth system-update --progress=10
/bin/plymouth message --text="获取升级项"
sleep 1

/bin/plymouth system-update --progress=20
/bin/plymouth message --text="正在安装更新, 请勿关闭计算机 ..."
sleep 1

/bin/plymouth system-update --progress=30
sleep 2

/bin/plymouth system-update --progress=90
/bin/plymouth message --text="安装完成，即将重启"
sleep 1

/bin/plymouth system-update --progress=100
/bin/plymouth message --text="升级完成"
sleep 1

/bin/plymouth quit