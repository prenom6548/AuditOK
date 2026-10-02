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
#include "screensaverinterface.h"

ScreensaverInterface::ScreensaverInterface():UKUI::ServiceObject(nullptr)
{
    mScreensaverType.clear();
    QString mode = UniversalInterface::self()->screensaverGsettings()->get("mode").toString();
    if (mode == "default-ukui") {
        mScreensaverType = "ukui";
    } else if (mode == "default-ukui-custom") {
        mScreensaverType = "custom";
    } else {
        mScreensaverType = "other";
    }
    connect(UniversalInterface::self()->screensaverGsettings(), &QGSettings::changed, this, [=](QString key){
        if (key == "mode") {
            QString mode = UniversalInterface::self()->screensaverGsettings()->get("mode").toString();
            if (mode == "default-ukui") {
                mScreensaverType = "ukui";
            } else if (mode == "default-ukui-custom") {
                mScreensaverType = "custom";
            } else {
                mScreensaverType = "other";
            }
        }
    });
}

ScreensaverInterface::~ScreensaverInterface()
{

}

int ScreensaverInterface::getPreentryTime()
{
    return UniversalInterface::self()->screensaverGsettings()->get("idle-delay").toInt();
}

void ScreensaverInterface::setPreentryTime(int time)
{
    UniversalInterface::self()->screensaverGsettings()->set("idle-delay", time);
    Q_EMIT changed("preentryTime");
}

QString ScreensaverInterface::getScreensaverType()
{
    return mScreensaverType;
}

void ScreensaverInterface::setScreensaverType(QString type)
{
    if (type == "ukui") {
        UniversalInterface::self()->screensaverGsettings()->set("mode", "default-ukui");
    } else if (type == "custom") {
        UniversalInterface::self()->screensaverGsettings()->set("mode", "default-ukui-custom");
    }
    Q_EMIT changed("screensaverType");
}

bool ScreensaverInterface::getShowBreakTimeUkui()
{
    return UniversalInterface::self()->screensaverDefaultGsettings()->get("show-ukui-rest-time").toBool();
}

void ScreensaverInterface::setShowBreakTimeUkui(bool b)
{
    UniversalInterface::self()->screensaverDefaultGsettings()->set("show-ukui-rest-time", b);
    Q_EMIT changed("showBreakTimeUkui");
}

bool ScreensaverInterface::getShowBreakTimeCustom()
{
    return UniversalInterface::self()->screensaverDefaultGsettings()->get("show-custom-rest-time").toBool();
}

void ScreensaverInterface::setShowBreakTimeCustom(bool b)
{
    UniversalInterface::self()->screensaverDefaultGsettings()->set("show-custom-rest-time", b);
    Q_EMIT changed("showBreakTimeCustom");
}

bool ScreensaverInterface::getScreenLockEnabled()
{
    return UniversalInterface::self()->screensaverGsettings()->get("lock-enabled").toBool();
}

void ScreensaverInterface::setScreenLockEnabled(bool b)
{
    UniversalInterface::self()->screensaverGsettings()->set("lock-enabled", b);
    Q_EMIT changed("screenLockEnabled");
}

QString ScreensaverInterface::getcustomPath()
{
    return UniversalInterface::self()->screensaverDefaultGsettings()->get("background-path").toString();
}

void ScreensaverInterface::setCustomPath(QString path)
{
    UniversalInterface::self()->screensaverDefaultGsettings()->set("background-path", path);
    Q_EMIT changed("customPath");
}

bool ScreensaverInterface::getSwitchRandom()
{
    return UniversalInterface::self()->screensaverDefaultGsettings()->get("automatic-switching-enabled").toBool();
}

void ScreensaverInterface::setSwitchRandom(bool b)
{
    UniversalInterface::self()->screensaverDefaultGsettings()->set("automatic-switching-enabled", b);
    Q_EMIT changed("switchRandom");
}

int ScreensaverInterface::getCycleTime()
{
    return UniversalInterface::self()->screensaverDefaultGsettings()->get("cycle-time").toInt();
}

void ScreensaverInterface::setCycleTime(int time)
{
    UniversalInterface::self()->screensaverDefaultGsettings()->set("cycle-time", time);
    Q_EMIT changed("cycleTime");
}

QString ScreensaverInterface::getCustomText()
{
    return UniversalInterface::self()->screensaverDefaultGsettings()->get("mytext").toString();
}

void ScreensaverInterface::setCustomText(QString text)
{
    UniversalInterface::self()->screensaverDefaultGsettings()->set("mytext", text);
    Q_EMIT changed("customText");
}

bool ScreensaverInterface::getCustomTextCentered()
{
    return UniversalInterface::self()->screensaverDefaultGsettings()->get("text-is-center").toBool();
}

void ScreensaverInterface::setCustomTextCentered(bool b)
{
    UniversalInterface::self()->screensaverDefaultGsettings()->set("text-is-center", b);
    Q_EMIT changed("customTextCentered");
}

