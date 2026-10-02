/*
 * Copyright 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "upmcommonfile.h"

#include <QDebug>
#include <QFileInfo>
#include <QTextStream>

UpmCommonFile::UpmCommonFile(QObject *parent) : QObject(parent) {}

bool UpmCommonFile::read(const QString &filePath, const QString &fileName, QString &content)
{
    QFile file(filePath + fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "read file error :" << filePath + fileName;
        return false;
    }

    content = file.readLine();
    file.close();

    return true;
}

bool UpmCommonFile::write(const QString &filePath, const QString &fileName, const QString &content)
{
    QFile file(filePath + fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "write file error :" << filePath + fileName;
        return false;
    }

    QTextStream in(&file);
    in << content << endl;
    file.close();

    return true;
}

bool UpmCommonFile::exists(const QString &filePath, const QString &fileName)
{
    QFile file(filePath + fileName);
    return file.exists();
}
