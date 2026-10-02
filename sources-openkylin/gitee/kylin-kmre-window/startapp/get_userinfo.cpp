/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "get_userinfo.h"
#include "utils.h"

#include <QDebug>

#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>

struct str_info UserInfo::getuserinfo()
{
    QString start_name;
    int start_uid ;

    start_name = Utils::getUserName();
    start_uid = getuid();//user id

    struct str_info info;
    info.name_info = start_name;
    info.uid_info = start_uid;

    return info;
}
