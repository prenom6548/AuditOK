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

#include "global.h"
#include "preferences.h"
#include <sys/syslog.h>
#include <QFile>

using namespace Global;

bool Global::isKylinOSPro()
{
    QFile file("/etc/lsb-release");
        if (file.exists()) {
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                const QString &content = QString::fromUtf8(file.readAll());
                file.close();

                return content.contains("Kylin V10 Professional") || content.contains("Kylin V10 SP1") || content.contains("Kylin V10 SP2");
            }
        }

        return false;
}

bool Global::isOpenKylin()
{
    QFile file("/etc/os-release");
    if (file.exists()) {
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            const QString &content = QString::fromUtf8(file.readAll());
            file.close();

            return content.contains("openKylin");
        }
    }
    return false;
}

