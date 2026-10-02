#!/bin/sh

#
# Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
#
# kaiming is licensed under the GPL v2.0+.
#
# See the LICENSE file for more details.
#

etc_path="/etc"

. $etc_path/profile.d/kaiming.sh

if command -v update-desktop-database >/dev/null && test -d "$1/share/applications"; then
    update-desktop-database -q "$1/share/applications"
fi
