#!/bin/bash

# 获取当前用户 UID 和脚本信息
current_uid=$(id -u)
current_pid=$$
current_dir=$(pwd)
kaiming_id="${KAIMING_ID:-}"
app_version="${APP_VERSION:-}"
exit_code=0
process_name=$(basename "$0")
process_path="$0"

# xdg-open 特殊情况处理
if [ "$process_name" == "xdg-open" ] && [ "$current_uid" -ne 0 ]; then
    script_name="xdg-open $kaiming_id $app_version \"$1\""
else
    script_name="$process_path"
    for arg in "$@"; do
        script_name+=" \"$arg\""
    done
fi

# 检查 dbus-send 是否存在
if ! command -v dbus-send &>/dev/null; then
    echo "Error: dbus-send not found. Please install D-Bus utilities." >&2
    exit 1
fi

# 创建并验证 FIFO 管道
fifo=$(mktemp -u)
mkfifo "$fifo"
exit_file=$(mktemp)

if [ ! -p "$fifo" ]; then
    echo "Failed to create FIFO at $fifo"
    exit 1
fi

# 启动 dbus-monitor，并记录 PID
if [ "$current_uid" -eq 0 ]; then
    dbus-monitor --system "type='signal',interface='org.kaiming.proxy.system',member='binaryReply'" >"$fifo" &
else
    dbus-monitor --session "type='signal',interface='org.kaiming.proxy.session',member='binaryReply'" >"$fifo" &
fi
monitor_pid=$!

# 准备状态变量
pid=""
in_string=0
string_buffer=""
last_line=""

# 辅助函数：处理双引号
strip_trailing_quote() {
    local s="$1"
    [[ "$s" == *\" ]] && s="${s%\"}"
    echo "$s"
}
strip_surrounding_quotes() {
    local s="$1"
    s="${s#\"}"
    s="${s%\"}"
    echo "$s"
}

# 启动监听循环（后台执行）
(
    while IFS= read -r line; do
        # 匹配 int32 PID
        if [[ "$line" =~ int32[[:space:]]+([0-9-]+) ]]; then
            pid="${BASH_REMATCH[1]}"
            print_mode=0
            printed_header=0
            in_string=0
            string_buffer=""
        fi

        # string 起始行
        if [[ "$line" =~ string[[:space:]]*\"(.*) ]]; then
            str="${BASH_REMATCH[1]}"
            str=$(strip_trailing_quote "$str")
            last_line="$str"
            string_buffer="$str"
            in_string=1

            # 只有pid等于current_pid时，才检查退出条件
            if [ "$pid" == "$current_pid" ]; then
                check_str=$(strip_surrounding_quotes "$string_buffer")
                if [[ "$check_str" =~ ^start\ binary\ done\|exit=([0-9]+)$ ]]; then
                    exit_code="${BASH_REMATCH[1]}"
                    echo "$exit_code" >"$exit_file"
                    kill "$monitor_pid" 2>/dev/null || true
                    exit "$exit_code"
                fi
            fi

            if [[ "$pid" == "$current_pid" && "$str" != "[Invalid UTF-8]" ]]; then
                print_mode=1
                if [ "$printed_header" -eq 0 ]; then
                    printed_header=1
                fi
                echo "$str"
            fi
            continue
        fi

        # string 多行拼接中
        if [ "$in_string" -eq 1 ]; then
            if [[ "$line" =~ ^signal ]]; then
                in_string=0
                clean_last=$(strip_trailing_quote "$last_line")
                string_buffer="${string_buffer%$last_line}$clean_last"

                if [ "$pid" == "$current_pid" ]; then
                    check_str=$(strip_surrounding_quotes "$string_buffer")
                    if [[ "$check_str" =~ ^start\ binary\ done\|exit=([0-9]+)$ ]]; then
                        exit_code="${BASH_REMATCH[1]}"
                        echo "$exit_code" >"$exit_file"
                        kill "$monitor_pid" 2>/dev/null || true
                        exit "$exit_code"
                    fi
                fi
            else
                line_clean=$(strip_trailing_quote "$line")
                last_line="$line_clean"
                string_buffer+=$'\n'"$line_clean"
                if [ "$pid" == "$current_pid" && "$line" != "[Invalid UTF-8]" ]; then
                    if [ "$print_mode" -eq 1 ]; then
                        echo "$line_clean"
                    fi
                fi
            fi
        fi
    done <"$fifo"
) &

# 发送 dbus 请求
if [ "$current_uid" -eq 0 ]; then
    dbus-send --system \
        --reply-timeout=100 \
        --print-reply \
        --dest=org.kaiming.proxy \
        /org/kaiming/proxy \
        org.kaiming.proxy.system.startBinaryAsUser \
        int32:"$current_pid" \
        string:"$script_name" \
        string:"$current_dir" >/dev/null 2>&1
    dbusret="$?"
else
    dbus-send --session \
        --reply-timeout=100 \
        --print-reply \
        --dest=org.kaiming.proxy \
        /org/kaiming/proxy \
        org.kaiming.proxy.session.startBinaryAsUser \
        int32:"$current_pid" \
        string:"$script_name" \
        string:"$current_dir" >/dev/null 2>&1
    dbusret="$?"
fi

# 等待监听进程退出(即收到目标信号)
wait "$monitor_pid"

# 清理 FIFO
rm -f "$fifo"

if [[ -s "$exit_file" ]]; then
    exit_code=$(cat "$exit_file")
fi

# 返回退出码
exit "$exit_code"
