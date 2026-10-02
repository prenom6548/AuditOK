#!/bin/bash

lupdate=`which lupdate 2> /dev/null`
if [ -z "${lupdate}" ]; then
	echo "lupdate not fount"
fi

echo "using ${lupdate}"
"$lupdate" `find ../ -name \*.cpp` -no-obsolete -ts kylin-os-manager-rubbish-clear_zh_CN.ts     \
                                                    kylin-os-manager-rubbish-clear_bo_CN.ts     \
                                                    kylin-os-manager-rubbish-clear_mn.ts        \
                                                    kylin-os-manager-rubbish-clear_kk.ts        \
                                                    kylin-os-manager-rubbish-clear_ky.ts        \
                                                    kylin-os-manager-rubbish-clear_ug.ts        \
