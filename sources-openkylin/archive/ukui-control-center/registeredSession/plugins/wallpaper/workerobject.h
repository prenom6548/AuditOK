/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef WORKEROBJECT_H
#define WORKEROBJECT_H

#include <QObject>

#include "xmlhandle.h"
#include <QStandardPaths>
#include <QtXml>
#include <QThread>

#define LOCAL_USR_PATH QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
#define LOCAL_APP_PATH QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)\
                       %QStringLiteral("/ukui-control-center/")
#define LOCAL_WALLPAPER_PATH QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)\
                                %QStringLiteral("/ukui-control-center/wallpaperData/")
#define LOCAL_WALLPAPER_HASH_PATH QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)\
                                  %QStringLiteral("/ukui-control-center/wallpaperData/HASH")



class WorkerObject : public QThread
{
    Q_OBJECT

public:
    explicit WorkerObject(QObject *parent = nullptr);
    ~WorkerObject();

public:
    void run();

private:
    XmlHandle * xmlHandleObj;

    QMap<QString, QMap<QString, QString> > wallpaperinfosMap;
    QStringList filenameList;

Q_SIGNALS:
    void wallpaperReaded(QStringList previewFiles, QStringList sourceFiles);
};

#endif // WORKEROBJECT_H
