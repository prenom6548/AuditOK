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
#ifndef UPMCOMMONFILE_H
#define UPMCOMMONFILE_H

#include <QObject>

class UpmCommonFile : public QObject
{
    Q_OBJECT
public:
    explicit UpmCommonFile(QObject *parent = nullptr);

    static bool read(const QString &filePath, const QString &fileName, QString &content);
    static bool write(const QString &filePath, const QString &fileName, const QString &content);
    static bool exists(const QString &filePath, const QString &fileName);
signals:

};

#endif // UPMCOMMONFILE_H
