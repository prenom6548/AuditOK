#!/bin/bash

lupdate=`which lupdate 2> /dev/null`
if [ -z "${lupdate}" ]; then
	echo "lupdate not fount"
fi

echo "using ${lupdate}"
"$lupdate" `find .. -name \*.cpp -o -name \*.ui` -no-obsolete -ts fault-diagnosis_zh_CN.ts    \
