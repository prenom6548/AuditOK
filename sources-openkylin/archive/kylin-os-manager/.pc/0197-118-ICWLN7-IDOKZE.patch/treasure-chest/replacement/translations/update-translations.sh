#!/bin/bash

lupdate=`which lupdate 2> /dev/null`
if [ -z "${lupdate}" ]; then
	echo "lupdate not fount"
fi

echo "using ${lupdate}"
"$lupdate" `find .. -name \*.cpp` -no-obsolete -ts replacement_zh_CN.ts    \
                                                   replacement_bo_CN.ts    \
                                                   replacement_mn.ts       \
                                                   replacement_kk.ts       \
                                                   replacement_ky.ts       \
                                                   replacement_ug.ts       \
                                                   replacement_zh_HK.ts    \
                                                   replacement_de.ts       \
                                                   replacement_es.ts       \
                                                   replacement_fr.ts       \
