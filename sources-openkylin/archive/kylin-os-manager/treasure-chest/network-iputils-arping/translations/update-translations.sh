#!/bin/bash

lupdate=`which lupdate 2> /dev/null`
if [ -z "${lupdate}" ]; then
	echo "lupdate not fount"
fi

echo "using ${lupdate}"
"$lupdate" `find ../ -name \*.cpp` -no-obsolete -ts komt-network-iputils-arping_zh_CN.ts \
                                                    komt-network-iputils-arping_zh_HK.ts \
                                                    komt-network-iputils-arping_bo_CN.ts \
                                                    komt-network-iputils-arping_mn.ts \
                                                    komt-network-iputils-arping_kk.ts \
                                                    komt-network-iputils-arping_ky.ts \
                                                    komt-network-iputils-arping_ug.ts \
