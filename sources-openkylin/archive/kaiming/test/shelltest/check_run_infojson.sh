#
# Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
#
# kaiming is licensed under the GPL v2.0+.
#
# See the LICENSE file for more details.
#

#!/bin/bash
#info.json设置的环境变量可以覆盖默认环境变量的问题
uid=$(id -u)

if [ $# -ne 1 ]; then
     echo "请提供应用名称作为参数"
    exit 0
fi
target=$1

function is_app_running() {
    local target="$1"
    kaiming ps | tail -n +2 | awk '{print $1}' | grep -q "^$target$"
}

kaiming run "$target" > /dev/null 2>&1 &

sleep 20
if is_app_running "$target"; then
    echo "应用'$target'成功运行" > /dev/null 2>&1
else
    echo "应用'$target'未运行"
    exit 1
fi

base_dir="/opt/kaiming/layers/stable/x86_64/app/$target"
echo $base_dir
info_json=$(find "$base_dir" -type f -name "info.json" | head -n 1)

if [ -z "$info_json" ]; then
    echo "未找到 info.json 文件"
    kaiming kill $target
    exit 1
fi

echo "找到 info.json: $info_json"
# 提取 info.json 中的 env 字段内容
function extract_env_from_json() {
    local json_file="$1"
    awk '/"env": *{/{flag=1;next} /}/{if(flag)exit} flag' "$json_file" | sed 's/[{},]//g;s/^[ \t]*//;s/[ \t]*$//' | grep -v '^$'
}

# 将 env 内容转换为 key=value 形式（用于比较）
function normalize_env_content() {
    echo "$1" | \
        sed 's/[{},]//g;s/^ *//;s/ *$//' | \
        grep -E '"[^"]+": *"[^"]+"' | \
        awk -F'"' '{gsub(/ /, "", $2); print $2 "=" substr($4, 1, length($4))}'
}
# 获取容器内的环境变量
function get_container_env() {
    local target="$1"
    kaiming enter "$target" printenv 2>/dev/null
}

function compare_envs() {
    local json_env="$1"
    local container_env="$2"

    local missing_vars=()
    local mismatched_vars=()

    # 将 json_env 按行处理为 key=value 格式
    while IFS= read -r line; do
        # 提取 key 和 value
        key=$(echo "$line" | cut -d= -f1)
        expected_value=$(echo "$line" | cut -d= -f2-)
        # 在容器 env 中查找是否存在该 key
        container_value=$(echo "$container_env" | grep "^$key=" | cut -d= -f2- | tr -d '[:space:]')

        if [ -z "$container_value" ]; then
            missing_vars+=("$key")
        elif [ "$container_value" != "$expected_value" ]; then
            mismatched_vars+=("$key (期望: $expected_value, 实际: $container_value)")
        fi
    done <<< "$json_env"

    # 判断结果
    if [ ${#missing_vars[@]} -eq 0 ] && [ ${#mismatched_vars[@]} -eq 0 ]; then
        echo "info.json 中的 env 在容器中全部存在且值一致"
        return 0
    else
        if [ ${#missing_vars[@]} -gt 0 ]; then
            echo "缺少以下环境变量：${missing_vars[*]}"
        fi

        if [ ${#mismatched_vars[@]} -gt 0 ]; then
            echo "以下环境变量值不一致：${mismatched_vars[*]}"
        fi

        return 1
    fi
}
# 主流程开始
json_env_raw=$(extract_env_from_json "$info_json")
if [ -z "$json_env_raw" ]; then
    echo "info.json 中未定义 env,跳过环境变量检查"
    kaiming kill "$target"
    exit 0
fi
json_env_parsed=$(normalize_env_content "$json_env_raw")


container_env=$(get_container_env "$target")

compare_envs "$json_env_parsed" "$container_env"
result=$?

if [ $result -ne 0 ]; then
    echo "环境变量不一致，测试失败"
    kaiming kill "$target"
    exit 1
fi

echo "info与容器环境变量一致，测试通过"
kaiming kill "$target"
exit 0