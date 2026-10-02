#
# Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
#
# kaiming is licensed under the GPL v2.0+.
#
# See the LICENSE file for more details.
#

#!/bin/bash

if [ $# -ne 2 ]; then
    echo "请提供应用类型和名称作为参数"
    exit 1
fi
# 获取应用名称
app_id="$2"
app_kind="$1"
architecture=$(arch)
# 1. 获取 kaiming search 的所有 ID（跳过表头，提取第一列）
kaiming_ids=$(kaiming search --kind=$app_kind $app_id | awk 'NR>1 {print $1}')

# 2. 获取 summary.json 的所有 ID（app/base/runtime/depend）
json_ids=$(jq -r --arg arch "$architecture" --arg type "$app_kind" '.stable[$arch][$type] | keys[]' /opt/kaiming/summary.json | grep "$app_id")
# 3. 逐行检查每个 kaiming_id 是否存在于 json_ids 中
echo "匹配的包 ID："
while IFS= read -r id; do

  if grep -q -F "$id" <<< "$json_ids"; then
    echo "$id"
  else
    echo "[未找到] $id" >&2  # 输出到标准错误（可选）
    exit 1
  fi
done <<< "$kaiming_ids"