#!/bin/bash

lupdate=`which lupdate 2> /dev/null`
if [ -z "${lupdate}" ]; then
	echo "lupdate not fount"
fi

echo "using ${lupdate}"
"$lupdate" `find ../ -name \*.cpp` -no-obsolete -ts komt-network-route-track_zh_CN.ts \
                                                    komt-network-route-track_zh_HK.ts \
                                                    komt-network-route-track_bo_CN.ts \
                                                    komt-network-route-track_mn.ts \
                                                    komt-network-route-track_kk.ts \
                                                    komt-network-route-track_ky.ts \
                                                    komt-network-route-track_ug.ts \
