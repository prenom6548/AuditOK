#!/bin/bash

# KMVersion 验证函数（修复版：正确输出错误信息）
validate_version() {
    local versionString="$1"
    local strict="${2:-false}"  # 默认为非严格模式
    local error_msg=""

    # 检查是否为空
    if [[ -z "$versionString" ]]; then
        error_msg="Version string is empty"
        echo "Error: $error_msg" >&2
        return 1
    fi

    # 去掉可选的 v 前缀
    local processedString="$versionString"
    if [[ "${versionString:0:1}" =~ [vV] ]]; then
        processedString="${versionString:1}"
    fi

    # 查找 - 和 + 位置
    local dashPos="${processedString%%-*}"
    dashPos=$(( ${#dashPos} ))
    [[ "$dashPos" -eq "${#processedString}" ]] && dashPos=-1

    local plusPos="${processedString%%+*}"
    plusPos=$(( ${#plusPos} ))
    [[ "$plusPos" -eq "${#processedString}" ]] && plusPos=-1

    # 分割核心部分、预发布和构建元数据
    if [[ "$plusPos" -ne -1 && ( "$dashPos" -eq -1 || "$plusPos" -lt "$dashPos" ) ]]; then
        corePart="${processedString:0:$plusPos}"
        buildMeta="${processedString:$((plusPos+1))}"
    elif [[ "$dashPos" -ne -1 ]]; then
        corePart="${processedString:0:$dashPos}"
        if [[ "$plusPos" -ne -1 && "$plusPos" -gt "$dashPos" ]]; then
            preRelease="${processedString:$((dashPos+1)):$((plusPos-dashPos-1))}"
            buildMeta="${processedString:$((plusPos+1))}"
        else
            preRelease="${processedString:$((dashPos+1))}"
        fi
    elif [[ "$plusPos" -ne -1 ]]; then
        corePart="${processedString:0:$plusPos}"
        buildMeta="${processedString:$((plusPos+1))}"
    else
        corePart="$processedString"
    fi

    # 解析核心部分
    IFS='.' read -ra nums <<< "$corePart"
    
    # 严格模式检查
    if [[ "$strict" == "true" ]]; then
        if [[ "${#nums[@]}" -ne 3 && "${#nums[@]}" -ne 4 ]]; then
            error_msg="Invalid version: must have 3 or 4 numeric components in strict mode"
            echo "Error: $error_msg" >&2
            return 1
        fi
    fi

    # 数字解析函数
    validate_number_part() {
        local s="$1"
        if [[ -z "$s" ]]; then
            return 0
        fi
        
        # 检查是否全是数字
        if [[ ! "$s" =~ ^[0-9]+$ ]]; then
            error_msg="Invalid numeric part: $s"
            return 1
        fi
        
        # 检查是否有前导0（除非是0本身）
        if [[ "$s" =~ ^0[0-9] ]]; then
            error_msg="Numeric part cannot have leading zeros: $s"
            return 1
        fi
        
        return 0
    }

    # 验证每个数字部分
    for num in "${nums[@]}"; do
        validate_number_part "$num" || {
            echo "Error: $error_msg" >&2
            return 1
        }
    done

    # 验证预发布标签（如果有）
    if [[ -n "$preRelease" ]]; then
        # 预发布标签只能包含 ASCII 字母数字和连字符
        if [[ ! "$preRelease" =~ ^[0-9A-Za-z-]+$ ]]; then
            error_msg="Invalid pre-release identifier: $preRelease"
            echo "Error: $error_msg" >&2
            return 1
        fi
    fi

    # 验证构建元数据（如果有）
    if [[ -n "$buildMeta" ]]; then
        # 构建元数据只能包含 ASCII 字母数字和连字符
        if [[ ! "$buildMeta" =~ ^[0-9A-Za-z-]+$ ]]; then
            error_msg="Invalid build metadata: $buildMeta"
            echo "Error: $error_msg" >&2
            return 1
        fi
    fi

    return 0  # 验证通过
}

if [ $# -eq 0 ]; then
    echo "请输入版本号作为参数"
    exit 1
fi

versionString="$1"
strict="${2:-false}"

# 调用验证函数
if validate_version "$versionString" "$strict"; then
    echo "Version is valid: $versionString"
    exit 0
else
    exit 1
fi
