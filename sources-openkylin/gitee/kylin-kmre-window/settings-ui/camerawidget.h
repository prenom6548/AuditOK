/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn/kobe24_lixiang@126.com
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

#ifndef CAMERAWIDGET_H
#define CAMERAWIDGET_H

#include <QWidget>
#include <QMap>

#include "settingscrollcontent.h"

class QComboBox;
class QFileSystemWatcher;
class Preferences;

class CameraWidget : public SettingScrollContent
{
    Q_OBJECT

public:
    explicit CameraWidget(QWidget * parent = 0);
    ~CameraWidget();

public slots:
    void updateCurrentCameraDevice(const QString &deviceName);
    void updateCameraDevices();

private:
    QLabel *m_cameraLabel = nullptr;
    QComboBox *m_cameraComboBox = nullptr;
    QMap<QString,QString> m_cameraMap;
    QFileSystemWatcher *watcher = nullptr;
    Preferences *m_pref = nullptr;
};


#endif // CAMERAWIDGET_H
