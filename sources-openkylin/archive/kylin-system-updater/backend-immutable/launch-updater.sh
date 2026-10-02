#!/bin/bash

# 判断/opt下是否有更新的程序源码，存在时执行，不存在时执行/usr下的
if [ -x /opt/update-module/current/usr/share/kylin-system-updater/kylin-system-updater ]; then
    exec /opt/update-module/current/usr/share/kylin-system-updater/kylin-system-updater "$@"
else
    exec /usr/share/kylin-system-updater/kylin-system-updater "$@"
fi