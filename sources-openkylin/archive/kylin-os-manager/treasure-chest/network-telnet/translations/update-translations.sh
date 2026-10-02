#!/bin/bash

lupdate=`which lupdate 2> /dev/null`
if [ -z "${lupdate}" ]; then
	echo "lupdate not fount"
fi

echo "using ${lupdate}"
"$lupdate" `find ../ -name \*.cpp` -no-obsolete -ts \
    komt-network-telnet_bo_CN.ts \
    komt-network-telnet_kk.ts \
    komt-network-telnet_ky.ts \
    komt-network-telnet_mn.ts \
    komt-network-telnet_ug.ts \
    komt-network-telnet_zh_CN.ts \
    komt-network-telnet_zh_HK.ts
