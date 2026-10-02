/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
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
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#include "newdemand_unit.h"
#include <QFile>
#include <QDebug>
#include <QDir>
const QString failstr = "SI_FAILED";
bool InstallAddFileToEFI()
{
    QString path = "/target/boot/efi/";
    QDir dir(path);
    if(!dir.exists()) {
        qDebug() << "/target/boot/efi is not exist.";
        return false;
    }
    QFile file(path + failstr);
    file.open(QIODevice::ReadWrite | QIODevice::Text);
    qDebug() << "Create /target/boot/efi/SI_FAILED.";
    return true;

}
