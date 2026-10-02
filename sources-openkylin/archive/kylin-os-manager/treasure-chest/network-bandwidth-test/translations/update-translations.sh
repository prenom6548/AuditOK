#!/bin/bash

lupdate=`which lupdate 2> /dev/null`
if [ -z "${lupdate}" ]; then
	echo "lupdate not fount"
fi

echo "using ${lupdate}"
"$lupdate" `find ../ -name \*.cpp` -no-obsolete -ts komt-network-bandwidth-test_zh_CN.ts \
                                                    komt-network-bandwidth-test_zh_HK.ts \
                                                    komt-network-bandwidth-test_mn.ts \
                                                    komt-network-bandwidth-test_bo_CN.ts \
                                                    komt-network-bandwidth-test_kk.ts \
                                                    komt-network-bandwidth-test_ky.ts \
                                                    komt-network-bandwidth-test_ug.ts \
