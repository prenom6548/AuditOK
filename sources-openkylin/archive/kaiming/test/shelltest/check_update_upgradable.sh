#
# Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
#
# kaiming is licensed under the GPL v2.0+.
#
# See the LICENSE file for more details.
#

#!/bin/bash

#!/bin/bash

# 1. 获取所有可升级的包，并解析为 id, version, arch
kaiming update --upgradable | awk 'NR>1 {
    id=$1;
    version=$2;
    arch=$3;
    module=$4;
    print id, version, arch, module;
}'  | while read -r id version arch module; do
    # 2. 用 jq 检查该包和版本是否存在
    echo "检查包: $id ($version, $arch, $module)"
    jq -r --arg arch "$arch" --arg module "$module" --arg name "$id" --arg ver "$version" '
    .stable[$arch] |
    paths as $path |
    getpath($path) as $value |
    if ($value | objects | has($module)) and ($value[$module] | has($ver)) and ($path[-1] == $name) then
        "✅ \($name) 版本 \($ver) 存在"
    else
        empty
    end
' /opt/kaiming/summary.json
done
