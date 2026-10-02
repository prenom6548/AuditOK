#!/bin/bash
test_root () {
    # test root privilege -- rc: 0=root, 1=not root
    [ "$(id -u)" = "0" ]
}

check_root () {
    # show error message and quit when root privilege missing
    if ! test_root; then
        echo "Error: missing root privilege." 1>&2
        exit 1
    fi
}

check_root

if [ -f "/usr/share/ukui-power-manager/tlp/origin_tmp.conf" ]; then
    rm "/usr/share/ukui-power-manager/tlp/origin_tmp.conf"
fi

UPM_TLP_FLAG=ONLY_ECHO_PATH /usr/share/ukui-power-manager/tlp/tlp bat &> /dev/null

sort -u "/usr/share/ukui-power-manager/tlp/origin_tmp.conf" > "/usr/share/ukui-power-manager/tlp/origin.conf"
rm "/usr/share/ukui-power-manager/tlp/origin_tmp.conf"
cat "/usr/share/ukui-power-manager/tlp/origin.conf"
