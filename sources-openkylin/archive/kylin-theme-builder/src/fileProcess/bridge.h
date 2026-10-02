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
#ifndef BRIDGE_H
#define BRIDGE_H

#include "../module/globalthemewidget.h"
#include "../module/iconthemewidget.h"
#include "../module/cursorthemewidget.h"
#include "configfilemanager.h"
#include <QObject>
#include <QMutex>

class Bridge : public QObject
{
    Q_OBJECT
public:
    static Bridge* getInstance(QObject *parent = nullptr);
    void createFileManager(QString time);
    void createDir();
    void wallpaperPathChanged(QString path);
    void coverPathChanged(QString path);
    void radiusChanged(int radius);
    void accentColorChanged(QColor accentcolor);
    void exteriorChanged(QColor accentcolor);
    void transparencyChanged(int transparency);
    void windowRadiusChanged(int windowradius);
    void appIconsMapChanged(QMap<QString, QString> *appiconsmaps);
    void systemIconsMapChanged(QMap<QString, QString> *systemiconsmaps);
    void placeIconsMapChanged(QMap<QString, QString> *placeiconsmaps);
    void deviceIconsMapChanged(QMap<QString, QString> *deviceiconsmaps);
    void cursorMapChanged(QMap<QString, QString> *cursormap);
    void timeCursorMapChanged(QMap<QString, QString> *timecursormap);
    void plymouthPathChanged(QString path);
    void grubPathChanged(QString path);
    void savePathConfigChanged(QString path);
    void createConfig();
    void createGrub();
    void createPlymouth();
    void createSavePathConfig();
    void creatDesktop();
    void createSetupConf();
    void updateIconCache(QMap<QString, QString> *iconsmaps,QString icontype);
    void ThemeNameChanged();

signals:
    void updateInfo();
    void updateThumbnail(QString thumbnailPath);
private:
    Bridge(QObject *parent = nullptr);
    QString m_time;
    static Bridge* m_instance;
    static QMutex m_mutex;
    ConfigFileManager *m_configfilemanager;

    QMap<QString, QString> *m_appsiconpathmap = nullptr;
    QMap<QString, QString> *m_systemiconpathmap = nullptr;
    QMap<QString, QString> *m_placeiconpathmap = nullptr;
    QMap<QString, QString> *m_deviceiconpathmap = nullptr;
    QMap<QString, QString> *m_cursorpathmap = nullptr;
    QMap<QString, QString> *m_timecursorpathmap = nullptr;

    QString m_wallpaperpath = ":/resource/background/1-openkylin.jpg";
    QString m_coverpath = ":/resource/background/custom-preview.png";
    QString m_plymouthpath = ":/resource/background/openKylin.svg";
    QString m_plymouthdir = ":/resource/plymouth/";
    QString m_grubpath = ":/resource/background/grub-background.png";
//    QDir m_builderConfig;
//    QDir m_buildericons;
//    QDir m_builderappicon;
//    QDir m_buildersystemicon;
//    QDir m_buildercursors;
//    QDir m_buildercursor;
//    QDir m_buildertimecursor;

};

#endif // BRIDGE_H
