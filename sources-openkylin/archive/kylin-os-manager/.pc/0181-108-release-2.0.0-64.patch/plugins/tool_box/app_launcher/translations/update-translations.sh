#!/bin/bash

lupdate=`which lupdate 2> /dev/null`
if [ -z "${lupdate}" ]; then
	echo "lupdate not fount"
fi

echo "using ${lupdate}"
"$lupdate" `find ../ -name \*.cpp` \
           ../../../fault_diagnosis/component/user_experience_dialog.cpp \
           -no-obsolete \
           -ts \
           app_launcher_zh_CN.ts \
           app_launcher_zh_HK.ts
