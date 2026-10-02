#!/bin/bash

lupdate=`which lupdate 2> /dev/null`
if [ -z "${lupdate}" ]; then
	echo "lupdate not fount"
fi

echo "using ${lupdate}"
"$lupdate" `find .. -name \*.cpp -o -name \*.ui` -no-obsolete -ts junk-clean_zh_CN.ts    \
                                                                  junk-clean_zh_HK.ts    \
                                                                  junk-clean_bo_CN.ts    \
                                                                  junk-clean_mn.ts       \
                                                                  junk-clean_kk.ts       \
                                                                  junk-clean_ky.ts       \
                                                                  junk-clean_ug.ts
