#!/bin/bash

lupdate=`which lupdate 2> /dev/null`
if [ -z "${lupdate}" ]; then
	echo "lupdate not fount"
fi

echo "using ${lupdate}"
"$lupdate" `find ../ -name \*.cpp` -no-obsolete -ts komt-system-cleanup_zh_CN.ts \
                                                    komt-system-cleanup_zh_HK.ts    \
                                                    komt-system-cleanup_bo_CN.ts    \
                                                    komt-system-cleanup_mn.ts       \
                                                    komt-system-cleanup_kk.ts       \
                                                    komt-system-cleanup_ky.ts       \
                                                    komt-system-cleanup_ug.ts       \
