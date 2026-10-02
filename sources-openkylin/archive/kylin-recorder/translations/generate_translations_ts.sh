#!/bin/bash

project_path=$(cd `dirname $0`; pwd)
cd $project_path/..

lupdate ./src -ts translations/kylin-recorder_zh_CN.ts
lupdate ./src -ts translations/kylin-recorder_bo_CN.ts
lupdate ./src -ts translations/kylin-recorder_bo.ts
lupdate ./src -ts translations/kylin-recorder_kk.ts
lupdate ./src -ts translations/kylin-recorder_ky.ts
lupdate ./src -ts translations/kylin-recorder_mn.ts
lupdate ./src -ts translations/kylin-recorder_ug.ts
