#!/bin/bash

CALLER=$1
TIME_STAMP=$2
CORE_DUMP_FILE_NAME=$3
PID=$(echo "${CORE_DUMP_FILE_NAME}" | awk -F "." '{print $(NF-2)}')

CORE_DUMP_FILE_BASE_DIR="/var/lib/systemd/coredump"
PKG_LIST_FILE_NAME="package_list.info"
CORE_DUMP_INFO_FILE_NAME="kylin_coredump_${PID}.info"
CORE_DUMP_EXTRA_INFO_NAME="kylin_coredump_extra_${PID}.info"
CORE_DUMP_ARICHIVE_FILE_NAME="kylin_coredump_${PID}.tar.gz"

KYLIN_CORE_DUMP_TMP_DIR="/tmp/kylin-core-dump-monitor"
CORE_DUMP_INFO_BASE_DIR="${KYLIN_CORE_DUMP_TMP_DIR}/core_dump_${PID}_${TIME_STAMP}"
CORE_DUMP_INFO_FILE_PATH="${CORE_DUMP_INFO_BASE_DIR}/${CORE_DUMP_INFO_FILE_NAME}"
CORE_DUMP_EXTRA_INFO_FILE_PATH="${CORE_DUMP_INFO_BASE_DIR}/${CORE_DUMP_EXTRA_INFO_NAME}"
CORE_DUMP_ARICHIVE_FILE_PATH="${CORE_DUMP_INFO_BASE_DIR}/${CORE_DUMP_ARICHIVE_FILE_NAME}"
PACKAGE_LIST_FILE_PATH="${CORE_DUMP_INFO_BASE_DIR}/${PKG_LIST_FILE_NAME}"

KOM_SESSION_SERVICE="/usr/share/kylin-os-manager/kylin-os-manager-session-service/kylin-os-manager-session-service"

# 判断是否第一方服务
function is_first_party_service() {
    local service=$1
    if grep -w "${service}" "/usr/share/kylin-os-manager/kylin-core-dump-monitor/first-party-service-list.conf" >/dev/null 2>&1; then
        return 0
    fi
    return 1
}

# 判断是否第一方图形程序
function is_first_party_ui_app() {
    local bin=$1
    if grep -w "${bin}" "/usr/share/kylin-os-manager/kylin-core-dump-monitor/first-party-ui-app-list.conf" >/dev/null 2>&1; then
        return 0
    fi
    return 1
}

# 判断是否需要上传
# 当前core的进程距离上一次core的时间差超过3600秒则上传
function is_need_upload() {
    local bin=$1
    local log
    log=$(coredumpctl list | grep "${bin}" | tail -n 2)

    local line_num
    line_num=$(echo "${log}" | wc -l)
    if [ "${line_num}" -lt 2 ]; then
        return 0
    fi

    local cur_line
    cur_line=$(echo "${log}" | tail -n 1)
    local pre_line
    pre_line=$(echo "${log}" | head -n 1)

    local cur_timestamp
    cur_timestamp=$(echo "${cur_line}" | awk -F' ' '{print $1, $2, $3, $4}' | date -d "$(cut -d' ' -f2-4)" +%s)
    local pre_timestamp
    pre_timestamp=$(echo "${pre_line}" | awk -F' ' '{print $1, $2, $3, $4}' | date -d "$(cut -d' ' -f2-4)" +%s)

    # 计算时间差
    local time_diff
    time_diff=$((cur_timestamp - pre_timestamp))
    if [ "${time_diff}" -gt 3600 ]; then
        return 0
    fi

    return 1
}

# 设置一个定时任务，系统重启后失效
function clean_after_upload() {
    echo "remove file | path=${CORE_DUMP_ARICHIVE_FILE_PATH}"
    rm -f "${CORE_DUMP_ARICHIVE_FILE_PATH}"
    echo "remove dir | dir=${CORE_DUMP_INFO_BASE_DIR}"
    rmdir "${CORE_DUMP_INFO_BASE_DIR}"
    echo "clean_after_upload"
}

function main() {
    # core dump 文件是否存在
    if [ ! -f "${CORE_DUMP_FILE_BASE_DIR}/${CORE_DUMP_FILE_NAME}" ]; then
        return 1
    fi

    # 上传程序是否存在
    if [ ! -f "${KOM_SESSION_SERVICE}" ]; then
        return 1
    fi

    mkdir -p "${CORE_DUMP_INFO_BASE_DIR}"

    # 等待systemd写入异常程序信息
    while true; do
        if coredumpctl info "${PID}" >"${CORE_DUMP_INFO_FILE_PATH}"; then
            break
        fi
        sleep 5
    done

    local cmd
    cmd=$(grep <"${CORE_DUMP_INFO_FILE_PATH}" "Command Line: " | grep -oP '(?<=Command Line: ).*')
    local exec
    exec=$(grep <"${CORE_DUMP_INFO_FILE_PATH}" "Executable: " | grep -oP '(?<=Executable: ).*')
    local unit
    unit=$(grep <"${CORE_DUMP_INFO_FILE_PATH}" "^[[:space:]]*Unit" | grep -oP '\s*Unit:\s*\K.*')

    # 判断是否需要上传
    if [ "${CALLER}" == "monitor" ]; then
        if ! is_need_upload "${exec}"; then
            echo "no upload | 距离上一次程序异常时间间隔太短 | caller=${CALLER} | exec=${exec} | path=${CORE_DUMP_ARICHIVE_FILE_PATH}"
            return 1
        fi
    fi

    echo "EXTRA INFO" >"${CORE_DUMP_EXTRA_INFO_FILE_PATH}"
    {
        echo ""
        echo "uname:"
        uname -a
        echo ""
        echo "kylin-build:"
        cat /etc/kylin-build
    } >>"${CORE_DUMP_EXTRA_INFO_FILE_PATH}"

    dpkg -l >"${PACKAGE_LIST_FILE_PATH}"

    tar -zcvf "${CORE_DUMP_ARICHIVE_FILE_PATH}" \
        -C "${CORE_DUMP_INFO_BASE_DIR}" "${CORE_DUMP_INFO_FILE_NAME}" "${CORE_DUMP_EXTRA_INFO_NAME}" "${PKG_LIST_FILE_NAME}" \
        -C "${CORE_DUMP_FILE_BASE_DIR}" "${CORE_DUMP_FILE_NAME}" >/dev/null

    rm -f "${CORE_DUMP_INFO_FILE_PATH}"
    rm -f "${CORE_DUMP_EXTRA_INFO_FILE_PATH}"
    rm -f "${PACKAGE_LIST_FILE_PATH}"

    local exe
    exe=$(basename "${exec}")

    # 判断是否第一方服务
    if is_first_party_service "${unit}"; then
        echo "Implicit upload | first party system service |caller=${CALLER} | exec=${exec} | path=${CORE_DUMP_ARICHIVE_FILE_PATH}"
        dbus-send --system --type=signal \
            /com/KylinOsManager/ServiceSupport \
            com.KylinOsManager.ServiceSupport.RunGlobalFeedback \
            int32:3 \
            string:"" \
            string:"${exe}" \
            string:"${CORE_DUMP_ARICHIVE_FILE_PATH}" \
            string:""

        return 0
    fi

    # 判断是否第一方图形程序
    if is_first_party_ui_app "${exec}"; then
        if [ "${CALLER}" == "trailblazer" ]; then
            echo "Implicit upload | first party ui app |caller=${CALLER} | exec=${exec} | path=${CORE_DUMP_ARICHIVE_FILE_PATH}"
            dbus-send --system --type=signal \
                /com/KylinOsManager/ServiceSupport \
                com.KylinOsManager.ServiceSupport.RunGlobalFeedback \
                int32:3 \
                string:"" \
                string:"${exe}" \
                string:"${CORE_DUMP_ARICHIVE_FILE_PATH}" \
                string:""

            return 0
        fi

        local restart
        restart=$(echo -n "${cmd}" | od -An -tx1 | tr -d ' \n')
        echo "Explicit upload | first party ui app |caller=${CALLER} | exec=${exec} | path=${CORE_DUMP_ARICHIVE_FILE_PATH}"
        set -x
        dbus-send --system --type=signal \
            /com/KylinOsManager/ServiceSupport \
            com.KylinOsManager.ServiceSupport.RunGlobalFeedback \
            int32:2 \
            string:"" \
            string:"${exe}" \
            string:"${CORE_DUMP_ARICHIVE_FILE_PATH}" \
            string:"${restart}"

        return 0
    fi

    echo "无需上传 | 非第一方程序 |caller=${CALLER} | exec=${exec} | path=${CORE_DUMP_ARICHIVE_FILE_PATH}"
    clean_after_upload
    return 0
}

main
