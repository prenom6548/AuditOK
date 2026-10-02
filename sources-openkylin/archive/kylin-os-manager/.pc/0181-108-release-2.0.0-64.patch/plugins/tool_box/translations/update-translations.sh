#!/bin/bash

lupdate=`which lupdate 2> /dev/null`
if [ -z "${lupdate}" ]; then
	echo "lupdate not fount"
fi

echo "using ${lupdate}"
"$lupdate" `find ../ -name \*.cpp` -no-obsolete -ts kylin-os-manager-tool-box_zh_CN.ts      \
                                                    kylin-os-manager-tool-box_bo_CN.ts      \
                                                    kylin-os-manager-tool-box_mn.ts         \
                                                    kylin-os-manager-tool-box_kk.ts         \
                                                    kylin-os-manager-tool-box_ky.ts         \
                                                    kylin-os-manager-tool-box_ug.ts         \
                                                    kylin-os-manager-tool-box_zh_HK.ts      \
                                                    kylin-os-manager-tool-box_de.ts         \
                                                    kylin-os-manager-tool-box_es.ts         \
                                                    kylin-os-manager-tool-box_fr.ts         \
