#!/bin/bash

lupdate=`which lupdate 2> /dev/null`
if [ -z "${lupdate}" ]; then
	echo "lupdate not fount"
fi

echo "using ${lupdate}"
"$lupdate" `find ../ -type d -name "service-support-mini" -prune \
            -o \( -type f \( -name '*.cpp' -o -name '*.h' \) -print \)` \
            -no-obsolete -ts kom-service-support_ar.ts \
                             kom-service-support_bo_CN.ts \
                             kom-service-support_de.ts \
                             kom-service-support_es.ts \
                             kom-service-support_fr.ts \
                             kom-service-support_kk.ts \
                             kom-service-support_ky.ts \
                             kom-service-support_mn.ts \
                             kom-service-support_ug.ts \
                             kom-service-support_vi.ts \
                             kom-service-support_zh_CN.ts \
                             kom-service-support_zh_HK.ts \
