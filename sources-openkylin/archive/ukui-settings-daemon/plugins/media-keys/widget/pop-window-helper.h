/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef POPWINDOWHELPER_H
#define POPWINDOWHELPER_H

#include <QObject>
#include <QtDBus/QtDBus>
#include <QDBusInterface>
#include "privacy-osd.h"

#define FRAMEWORK_DBUS_ID           "org.ukui.Framework"
#define FRAMEWORK_DBUS_PATH         "/org/ukui/Framework/UI/OsdServer"
#define FRAMEWORK_DBUS_INTERFACE    "org.ukui.Framework.UI.OsdServer"

#define UKUI_SOUND "org.ukui.sound"
#define VOLUME_INCREASED "volume-increase"
#define VOLUME_INCREASED_VALUE "volume-increase-value"
#define MAX_VOLUME_NORMAL 100
#define MAX_VOLUM_INCREASED 125

class PopWindowHelper : public QObject
{
    Q_OBJECT
public:
    static PopWindowHelper* self();
    explicit PopWindowHelper(QObject *parent = nullptr);
    ~PopWindowHelper();
    int getMaxVolume();

    void initWindow();
    void freeWindow();

    void showVolumeBarByDbus(int currunt, int max, bool mute);
    void showBrightnessBarByDbus(int currunt, int max);
    void showCustomOsdByDbus(const QString& icon);

private Q_SLOTS:
    void volumeIncreased(const QString&);

private:
    void initMaxVolume();

private:
    PrivacyOSD* m_privacyOsd = nullptr;
    QGSettings *m_soundSettings = nullptr;

    int m_maxVolume = MAX_VOLUME_NORMAL;
};

#endif // POPWINDOWHELPER_H
