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
#ifndef SCREENSAVERINTERFACE_H
#define SCREENSAVERINTERFACE_H

#include "../../universalinterface.h"
#include <ukui-service-interface/serviceobject.h>
#include <QtDBus>

class ScreensaverInterface : public UKUI::ServiceObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.ukcc.session.Screensaver")
    Q_PROPERTY(int preentryTime READ getPreentryTime)
    Q_PROPERTY(QString screensaverType READ getScreensaverType)
    Q_PROPERTY(bool showBreakTimeUkui READ getShowBreakTimeUkui)
    Q_PROPERTY(bool showBreakTimeCustom READ getShowBreakTimeCustom)
    Q_PROPERTY(bool screenLockEnabled READ getScreenLockEnabled)
    Q_PROPERTY(QString customPath READ getcustomPath)
    Q_PROPERTY(bool switchRandom READ getSwitchRandom)
    Q_PROPERTY(int cycleTime READ getCycleTime)
    Q_PROPERTY(QString customText READ getCustomText)
    Q_PROPERTY(bool customTextCentered READ getCustomTextCentered)

public:
    ScreensaverInterface();
    ~ScreensaverInterface();
    int getPreentryTime();
    QString getScreensaverType();
    bool getShowBreakTimeUkui();
    bool getShowBreakTimeCustom();
    bool getScreenLockEnabled();
    QString getcustomPath();
    bool getSwitchRandom();
    int getCycleTime();
    QString getCustomText();
    bool getCustomTextCentered();

private:
    QString mScreensaverType;

public Q_SLOTS:
    void setPreentryTime(int time);
    void setScreensaverType(QString type);
    void setShowBreakTimeUkui(bool b);
    void setShowBreakTimeCustom(bool b);
    void setScreenLockEnabled(bool b);
    void setCustomPath(QString path);
    void setSwitchRandom(bool b);
    void setCycleTime(int time);
    void setCustomText(QString Text);
    void setCustomTextCentered(bool b);

Q_SIGNALS:
    void changed(QString key);
};

#endif // SCREENSAVERINTERFACE_H
