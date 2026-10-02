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


if command -v gtk-update-icon-cache >/dev/null 2>&1 && test -d "$1/share/icons/hicolor"; then
   cp /usr/share/icons/hicolor/index.theme "$1/share/icons/hicolor/" >/dev/null 2>&1
   for dir in "$1"/share/icons/*; do
       if test -f "$dir/index.theme"; then
           gtk-update-icon-cache --quiet "$dir" >/dev/null 2>&1
       fi
   done
fi