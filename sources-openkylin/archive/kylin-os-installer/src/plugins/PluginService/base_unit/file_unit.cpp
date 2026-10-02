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

#include "file_unit.h"
#include <QDir>
#include <QDebug>
#include <QApplication>
#include <QDesktopWidget>
#include <QFile>

namespace KServer {

QString KReadFile(const QString& path)
{
    QString str = "";
    QFile file(path);
    if (!file.exists()) {
        qWarning() << "文件不存在" << path;
    }
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "打开文件失败" << path;
    } else {
        str = QString::fromUtf8(file.readAll());
        file.close();
    }
    return (QString)str;
}

}
