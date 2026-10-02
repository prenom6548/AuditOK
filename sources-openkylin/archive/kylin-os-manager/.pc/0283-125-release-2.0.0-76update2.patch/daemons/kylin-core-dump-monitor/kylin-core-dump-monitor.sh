#!/bin/bash

CORE_DUMP_FILE_BASE_DIR="/var/lib/systemd/coredump"

function main() {
    /usr/bin/inotifywait -mq --timefmt '%y%m%d%H%M%S' --format '%T %f' -e moved_to ${CORE_DUMP_FILE_BASE_DIR} | while read -r timestamp file; do
        /usr/share/kylin-os-manager/kylin-core-dump-monitor/kylin-core-dump-handler.sh "monitor" "${timestamp}" "${file}"
    done
}

main
