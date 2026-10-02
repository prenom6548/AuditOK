#!/bin/bash

lupdate=`which lupdate 2> /dev/null`
if [ -z "${lupdate}" ]; then
	echo "lupdate not fount"
fi

echo "using ${lupdate}"
"$lupdate" `find .. -name \*.cpp` -no-obsolete -ts kylin-os-manager_zh_CN.ts    \
                                                   kylin-os-manager_zh_HK.ts    \
                                                   kylin-os-manager_bo_CN.ts    \
                                                   kylin-os-manager_mn.ts       \
                                                   kylin-os-manager_kk.ts       \
                                                   kylin-os-manager_ky.ts       \
                                                   kylin-os-manager_ug.ts       \
