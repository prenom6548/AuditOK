#!/bin/bash

lupdate=`which lupdate 2> /dev/null`
if [ -z "${lupdate}" ]; then
	echo "lupdate not fount"
fi

echo "using ${lupdate}"
"$lupdate" `find ../ -name \*.cpp` `find ../ -name \*.h` -no-obsolete -ts kylin-os-manager-device-controller_zh_CN.ts     \
                                                    kylin-os-manager-device-controller_bo_CN.ts     \
                                                    kylin-os-manager-device-controller_mn.ts        \
                                                    kylin-os-manager-device-controller_kk.ts        \
                                                    kylin-os-manager-device-controller_ky.ts        \
                                                    kylin-os-manager-device-controller_ug.ts        \
                                                    kylin-os-manager-device-controller_zh_HK.ts     \
