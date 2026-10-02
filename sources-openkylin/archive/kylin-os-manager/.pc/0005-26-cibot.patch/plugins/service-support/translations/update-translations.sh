#!/bin/bash

lupdate=`which lupdate 2> /dev/null`
if [ -z "${lupdate}" ]; then
	echo "lupdate not fount"
fi

echo "using ${lupdate}"
"$lupdate" `find ../ -name \*.cpp` -no-obsolete -ts kom-service-support_zh_CN.ts		\
									kom-service-support_bo_CN.ts	\
									kom-service-support_mn.ts	\
									kom-service-support_kk.ts	\
									kom-service-support_ky.ts	\
									kom-service-support_ug.ts	\
