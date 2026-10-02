#!/bin/sh

#
# Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
#
# kaiming is licensed under the GPL v2.0+.
#
# See the LICENSE file for more details.
#

if command -v kaiming > /dev/null; then
      kaiming_share_path=/opt/kaiming/share
      case ":$XDG_DATA_DIRS:" in
          (*":$kaiming_share_path/:"*) kaiming_new_dirs="" ;;
          (*":$kaiming_share_path:"*) kaiming_new_dirs="" ;;
          (*) kaiming_new_dirs="$kaiming_share_path" ;;
      esac

    _cleaned_dir=
    local_dir="${XDG_DATA_HOME:-$HOME/.local/share}/kaiming"
    old_IFS=$IFS
    IFS=:
    for dir in ${XDG_DATA_DIRS:-/usr/local/share:/usr/share}; do
        case "$dir" in
            "$local_dir"|"$local_dir/"|/opt/kaiming/exports/share|/opt/kaiming/exports/share/) continue ;;
            *) _cleaned_dirs=${_cleaned_dirs:+$_cleaned_dirs:}$dir ;;
        esac
    done
    IFS=$old_IFS

    export XDG_DATA_DIRS="${kaiming_new_dirs:+${kaiming_new_dirs}:}$_cleaned_dirs"
    export PATH=$PATH:/opt/kaiming/bin
    #echo "$XDG_DATA_DIRS"
fi
