#!/bin/bash

lupdate=`which lupdate 2> /dev/null`
if [ -z "${lupdate}" ]; then
	echo "lupdate not fount"
fi

echo "using ${lupdate}"
"$lupdate" `find .. -name \*.cpp` -no-obsolete -ts komt-packet-capture_zh_CN.ts    \
                                                   komt-packet-capture_zh_HK.ts    \
                                                   komt-packet-capture_bo_CN.ts    \
                                                   komt-packet-capture_mn.ts    \
                                                   komt-packet-capture_kk.ts    \
                                                   komt-packet-capture_ky.ts    \
                                                   komt-packet-capture_ug.ts    \
