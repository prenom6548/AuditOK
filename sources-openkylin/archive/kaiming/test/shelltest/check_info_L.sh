#
# Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
#
# kaiming is licensed under the GPL v2.0+.
#
# See the LICENSE file for more details.
#

#!/bin/bash

# 检查是否提供了参数
if [ $# -eq 0 ]; then
    echo "请提供应用名称作为参数"
    exit 1
fi
# 获取应用名称
app_name="$1"
architecture=$(arch)

# 获取 kaiming 输出的第一行
kaiming_first_line=$(kaiming info -L $app_name | head -n 1)

if [ -z "$kaiming_first_line" ]; then
    echo "未找到$app_name"
    exit 1
fi

found_dir=$(find /opt/kaiming/layers/stable/${architecture}/app/${app_name}/binary/  -name "deploy" 2>/dev/null)
# 获取 deploy 文件的第一行
echo "$found_dir"
deploy_first_line=$(head -n 1 $found_dir)


# 比较并输出结果
if [ "$kaiming_first_line" = "$deploy_first_line" ]; then

    echo "info执行成功"
else
    echo "info执行失败"
    exit 1
fi


