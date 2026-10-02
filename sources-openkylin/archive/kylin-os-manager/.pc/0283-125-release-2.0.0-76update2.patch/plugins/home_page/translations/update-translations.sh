#!/bin/bash

lupdate=`which lupdate 2> /dev/null`
if [ -z "${lupdate}" ]; then
	echo "lupdate not fount"
fi

echo "using ${lupdate}"
"$lupdate" `find ../ -name '*.cpp'` -no-obsolete -ts home-page_zh_CN.ts \
                                                     home-page_zh_HK.ts \
													 home-page_bo_CN.ts \
													 home-page_mn.ts
