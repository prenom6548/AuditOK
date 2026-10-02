/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#ifndef KMCONFIG_H
#define KMCONFIG_H

//#define REPO_URL "http://ostree.openkylin.top/openkylin-kaiming"
#define REPO_URL "https://archive2.kylinos.cn/KM/OPENKYLIN/repo"

#define KAIMING_CONFIG_PATH "/usr/local/etc/kaiming"
#define KAIMING_SYSTEM_PATH "/opt/kaiming"
#define KAIMING_SYSTEM_EXPORTS_PATH "/var/opt/kaiming"
#define KAIMING_SYSTEM_FONT_PATH "/usr/share/fonts"
#define KAIMING_SYSTEM_FONT_CACHE_PATHS "/var/cache/fontconfig:/usr/lib/fontconfig/cache"

#define KAIMING_PROGRESS_NAME "kaiming"
#define KAIMING_BIN_PATH "/opt/kaiming/bin"
#define KAIMING_PROGRESS_FULL_PATH "/opt/kaiming-tools/bin/kaiming"

#define DBUSPROXY "/usr/bin/xdg-dbus-proxy"
#define KAIMING_TOOLS_PATH "/opt/kaiming-tools"
#define KAIMING_TOOLS_BIN_PATH KAIMING_TOOLS_PATH"/bin"

#endif // KMCONFIG_H
