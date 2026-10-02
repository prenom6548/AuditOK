#!/bin/bash

lupdate=`which lupdate 2> /dev/null`
if [ -z "${lupdate}" ]; then
	echo "lupdate not fount"
fi

echo "using ${lupdate}"
"$lupdate" `find ../ -name \*.cpp` -no-obsolete -ts kylin-os-manager-driver-controller_zh_CN.ts     \
                                                    kylin-os-manager-driver-controller_bo_CN.ts     \
                                                    kylin-os-manager-driver-controller_mn.ts        \
                                                    kylin-os-manager-driver-controller_kk.ts        \
                                                    kylin-os-manager-driver-controller_ky.ts        \
                                                    kylin-os-manager-driver-controller_ug.ts        \

