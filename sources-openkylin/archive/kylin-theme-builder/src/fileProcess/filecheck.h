/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: LiKe <like1@kylinos.cn>
 *
 */
#ifndef FILECHECK_H
#define FILECHECK_H

#include <QObject>
#include <QDebug>
#include <QSvgRenderer>
#include <QFileInfo>
#include <QImage>
#include <QMessageBox>
#include <QDir>
#include "fileprocess.h"
class FileCheck : public QObject
{
    Q_OBJECT
public:
    explicit FileCheck(QObject *parent = nullptr);
    static bool isLegalIconFile(const QString& filePath);
    static bool isLegalCursorFile(const QString& filePath);
    static bool isLegalWallPaperFile(const QString& filePath, const QString& type);
    static bool isLegalMP4File(const QString& filePath);
    static bool isLegalPlymouthPicFile(const QString& filePath);
    static bool isLegalThemeName(const QString& filename);
    static QStringList isLegalBatchIcon(const QString& filePath,const QMap<QString, QString> *map);
signals:

};

#endif // FILECHECK_H
