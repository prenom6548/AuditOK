#
# Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
#
# kaiming is licensed under the GPL v2.0+.
#
# See the LICENSE file for more details.
#

#!/bin/bash

# 检查是否提供了参数
#!/bin/bash

# 检查是否以root运行

if [ $# -ne 1 ]; then
    echo "请提供url作为参数"
    exit 1
fi

new_url=$1

# 配置文件路径
config_file="/opt/kaiming/repo/config"

sudo kaiming remote modify kaiming-repo --repo="$new_url"

# 等待1秒确保文件写入完成
sleep 1

# 检查是否修改成功
if grep -q "^url=${new_url}$" "$config_file" || 
   grep -q "^url=${new_url} #" "$config_file"; then
    echo "✅ 验证成功: 配置文件已更新为 $new_url"
    exit 0
else
    echo "❌ 验证失败: 配置文件未正确更新"
    echo "当前配置内容:"
    grep "^url=" "$config_file"
    exit 1
fi