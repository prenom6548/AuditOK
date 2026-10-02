#!/bin/bash

lupdate=`which lupdate 2> /dev/null`
if [ -z "${lupdate}" ]; then
	echo "lupdate not fount"
fi

echo "using ${lupdate}"
"$lupdate" `find ../ -type d -name "printerbackend" -prune \
            -o \( -type f \( -name '*.cpp' -o -name '*.h' \) -print \)` \
            -no-obsolete -ts kylin-os-manager-session-service_zh_CN.ts \
                             kylin-os-manager-session-service_bo_CN.ts \
                             kylin-os-manager-session-service_mn.ts \
                             kylin-os-manager-session-service_kk.ts \
                             kylin-os-manager-session-service_ky.ts \
                             kylin-os-manager-session-service_ug.ts \
                             kylin-os-manager-session-service_zh_HK.ts \
