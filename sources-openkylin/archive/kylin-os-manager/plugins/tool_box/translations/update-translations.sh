#!/bin/bash

lupdate=`which lupdate 2> /dev/null`
if [ -z "${lupdate}" ]; then
	echo "lupdate not fount"
fi

echo "using ${lupdate}"
"$lupdate" `find ../ -name \*.cpp` -no-obsolete -ts tool-box_zh_CN.ts      \
                                                    tool-box_zh_HK.ts      \
                                                    tool-box_bo_CN.ts      \
                                                    tool-box_mn.ts         \
                                                    tool-box_kk.ts         \
                                                    tool-box_ky.ts         \
                                                    tool-box_ug.ts         \
