#!/bin/bash

lupdate=`which lupdate 2> /dev/null`
if [ -z "${lupdate}" ]; then
	echo "lupdate not fount"
fi

echo "using ${lupdate}"
"$lupdate" `find ../ -name \*.cpp` -no-obsolete -ts kom-service-support-mini_zh_CN.ts    \
                                                    kom-service-support-mini_zh_HK.ts    \
                                                    kom-service-support-mini_bo_CN.ts    \
                                                    kom-service-support-mini_mn.ts       \
                                                    kom-service-support-mini_kk.ts       \
                                                    kom-service-support-mini_ky.ts       \
                                                    kom-service-support-mini_ug.ts       \
