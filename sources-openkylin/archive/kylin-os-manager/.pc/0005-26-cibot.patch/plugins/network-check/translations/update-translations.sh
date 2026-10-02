#!/bin/bash

lupdate=`which lupdate 2> /dev/null`
if [ -z "${lupdate}" ]; then
	echo "lupdate not fount"
fi

echo "using ${lupdate}"
"$lupdate" `find ../ -name \*.cpp -o -name \*.ui` -no-obsolete -ts kylin-netcheck-tools_zh_CN.ts    \
                                                                   kylin-netcheck-tools_bo_CN.ts    \
                                                                   kylin-netcheck-tools_mn.ts       \
                                                                   kylin-netcheck-tools_kk.ts       \
                                                                   kylin-netcheck-tools_ky.ts       \
                                                                   kylin-netcheck-tools_ug.ts       \
