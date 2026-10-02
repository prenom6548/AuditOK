#!/bin/bash

lupdate=`which lupdate 2> /dev/null`
if [ -z "${lupdate}" ]; then
	echo "lupdate not fount"
fi

echo "using ${lupdate}"
"$lupdate" `find ../ -name \*.cpp` `find ../ -name \*.h` -no-obsolete -ts driver-acquirer-widget_zh_CN.ts     \
                                                    driver-acquirer-widget_bo_CN.ts     \
                                                    driver-acquirer-widget_mn.ts        \
                                                    driver-acquirer-widget_kk.ts        \
                                                    driver-acquirer-widget_ky.ts        \
                                                    driver-acquirer-widget_ug.ts        \
                                                    driver-acquirer-widget_zh_HK.ts     \
