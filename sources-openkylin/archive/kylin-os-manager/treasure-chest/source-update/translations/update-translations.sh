#!/bin/bash

lupdate=`which lupdate 2> /dev/null`
if [ -z "${lupdate}" ]; then
	echo "lupdate not fount"
fi

echo "using ${lupdate}"
"$lupdate" `find .. -name \*.cpp` -no-obsolete -ts kylin-source-update-tool_zh_CN.ts    \
                                                   kylin-source-update-tool_zh_HK.ts    \
                                                   kylin-source-update-tool_bo_CN.ts    \
                                                   kylin-source-update-tool_mn.ts    \
                                                   kylin-source-update-tool_kk.ts    \
                                                   kylin-source-update-tool_ky.ts    \
                                                   kylin-source-update-tool_ug.ts    \
