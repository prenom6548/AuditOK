#!/bin/bash

lupdate=`which lupdate 2> /dev/null`
if [ -z "${lupdate}" ]; then
	echo "lupdate not fount"
fi

echo "using ${lupdate}"
"$lupdate" `find .. -name \*.cpp` -no-obsolete -ts win-data-migration_zh_CN.ts	\
							win-data-migration_bo_CN.ts	\
							win-data-migration_mn.ts	\
							win-data-migration_kk.ts		\
							win-data-migration_ky.ts		\
							win-data-migration_ug.ts		\
