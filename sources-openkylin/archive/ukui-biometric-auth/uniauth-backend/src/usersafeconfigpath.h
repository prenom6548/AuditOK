/*
 * Copyright (C) 2026 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#ifndef USERSAFECONFIGPATH_H
#define USERSAFECONFIGPATH_H

class QString;

/** 基于 passwd 家目录；配置文件若已存在须为普通文件、不得为软链接；父目录若已存在须为目录且不得为软链接。 */
QString safeUserBiometricConfPath(const QString &userName);
QString safeUserLegacy990BiometricConfPath(const QString &userName);

#endif // USERSAFECONFIGPATH_H
