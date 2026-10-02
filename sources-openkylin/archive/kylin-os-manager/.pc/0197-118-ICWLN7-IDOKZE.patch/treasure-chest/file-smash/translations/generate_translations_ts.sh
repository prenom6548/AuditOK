#!/bin/bash

project_path=$(cd `dirname $0`; pwd)
cd $project_path/..

lupdate ./ -ts translations/kylin-file-crush_zh_CN.ts
lupdate ./ -ts translations/kylin-file-crush_bo_CN.ts
lupdate ./ -ts translations/kylin-file-crush_mn.ts
lupdate ./ -ts translations/kylin-file-crush_kk.ts
lupdate ./ -ts translations/kylin-file-crush_ky.ts
lupdate ./ -ts translations/kylin-file-crush_ug.ts
lupdate ./ -ts translations/kylin-file-crush_zh_HK.ts
lupdate ./ -ts translations/kylin-file-crush_de.ts
lupdate ./ -ts translations/kylin-file-crush_es.ts
lupdate ./ -ts translations/kylin-file-crush_fr.ts
