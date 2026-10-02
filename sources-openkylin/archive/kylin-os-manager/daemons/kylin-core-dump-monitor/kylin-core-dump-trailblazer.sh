#!/bin/bash

function main() {
    local up_time_stamp
    up_time_stamp=$(uptime -s)

    local core_dump_list_file
    core_dump_list_file=$(mktemp)
    if ! coredumpctl -S "${up_time_stamp}" >"${core_dump_list_file}"; then
        return 0
    fi

    local core_dump_list
    core_dump_list=$(grep -v -E "none|error|missing" <"${core_dump_list_file}" | tail -n +2)

    local line_num
    line_num=$(echo "${core_dump_list}" | wc -l)
    if [ "${line_num}" -lt 1 ]; then
        return 0
    fi

    while IFS= read -r line; do
        local pid
        pid=$(echo "${line}" | awk -F' ' '{print $5}')
        local timestamp
        timestamp=$(echo "${line}" | awk -F' ' '{print $1, $2, $3, $4}' | date -d "$(cut -d' ' -f2-4)" +%y%m%d%H%M%S)
        local file_path
        file_path=$(find /var/lib/systemd/coredump/core.*.*.*."${pid}".*.lz4 | head -1)
        local file_name
        file_name=$(basename "${file_path}")

        /usr/share/kylin-os-manager/kylin-core-dump-monitor/kylin-core-dump-handler.sh "trailblazer" "${timestamp}" "${file_name}"
    done <<<"${core_dump_list}"

    return 0
}

main
