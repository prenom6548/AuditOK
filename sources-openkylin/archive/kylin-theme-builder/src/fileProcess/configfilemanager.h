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
#ifndef CONFIGFILEMANAGER_H
#define CONFIGFILEMANAGER_H

#include "../fileProcess/fileprocess.h"

#include <QObject>
#include <QColor>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>
#include <QDebug>
#include <QFile>
#include <QCoreApplication>
#include <QResource>

class ConfigFileManager : public QObject
{
    Q_OBJECT
public:
    explicit ConfigFileManager(QObject *parent = nullptr);

    bool createJson(QDir cachedir);
    bool createConf(QDir cachedir);
    bool createXml(QDir cachedir);
    bool createFocalXml(QDir cachedir);
    bool createSetupConf(QDir cachedir);
    bool createSavePathConfig();
    bool createDesktop(QDir cachedir);
    bool modifyRadiusJson(int radius,QString jsonFilePath);

    bool modifyAccentColorConf(QColor accentcolor,QString confFilePath);
    bool modifyQtAccentColorConf(QString accentcolor,QString confFilePath);
    bool modifyWallPaperConf(QString wallpaperpath,QString confFilePath);
    bool modifyWidgetStyleConf(QString WidgetStyle,QString confFilePath);
    bool modifyTransparencyConf(int transparency,QString confFilePath);
    bool modifywindowRadiusConf(int windowradius,QString confFilePath);
    bool modifyFocalXml(QString focalXmlPath);
    bool modifyDesktop(QString desktop);
    bool modifySavePathConf(QString savePath);

    bool copyFileContent(const QString& sourceFilePath, const QString& destinationFilePath);
    bool copyIcontoCacheDir(QMap<QString, QString> *map,QDir cachedir);

    void copyPictoCacheDir(const QString& destinationFilePath,QDir cachedir,QString type);
    void copyPicstoCacheDir(const QString& destinationFilePath,QDir cachedir);
    void copyMp4toCacheDir(const QString& destinationFilePath,QDir cachedir);
    void copyAllPictoCacheDir(const QDir sourcedir,QDir cachedir);
signals:
    void updateInfo();
    void updateThumbnail(QString thumbnailPath);
private:

};

#endif // CONFIGFILEMANAGER_H
