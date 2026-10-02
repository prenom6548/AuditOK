/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
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
#include "touchpad-settings.h"

#include <QDebug>
#include <QTranslator>
#include <QCoreApplication>

#include "settingscommon.h"
#include "touchpadsettingsui.h"
#include "waylandtouchpadsettingsui.h"
#include "touchpaddevicehelper.h"

Touchpad::Touchpad() : m_firstLoad(true)
{
    QTranslator* translator = new QTranslator(this);
    translator->load("/usr/share/touchpad-settings/translations/" + QLocale::system().name());
    qDebug() << "=====" << QLocale::system().name();
    qApp->installTranslator(translator);
}

QString Touchpad::plugini18nName()
{
    //~ contents_path /Touchpad/Touchpad
    return tr("Touchpad");
}

int Touchpad::pluginTypes()
{
    return DEVICES;
}

QWidget *Touchpad::pluginUi()
{
    if (m_firstLoad) {
        m_firstLoad = false;
        if(SettingsCommon::isWaylandEnv()) {
            m_pluginWidget = new WaylandTouchpadUI();
        } else {
            m_pluginWidget = new TouchpadUI(createDefaultInfos());
        }
        m_pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    }

    return m_pluginWidget;
}

const QString Touchpad::name() const
{
    return QStringLiteral("Touchpad");
}

bool Touchpad::isShowOnHomePage() const
{
    return true;
}

QIcon Touchpad::icon() const
{
    return QIcon::fromTheme("input-touchpad-symbolic");
}

bool Touchpad::isEnable() const
{
    bool isFound = TouchpadDeviceHelper::hasTouchPad();
    qDebug() << __FUNCTION__ << __LINE__ << isFound;
    return isFound;
}

QString Touchpad::translationPath() const
{
    return "/usr/share/touchpad-settings/translations/%1.ts";
}

QList<TouchpadGestureInfo> Touchpad::createDefaultInfos()
{
    QString resourcesPath = SettingsCommon::isIntelProject() ? "intelGif/" : QString();

    QList<TouchpadGestureInfo> infos =
    {
        TouchpadGestureInfo(TouchpadGestureType::threeFingerClick,
                            QObject::tr("three fingers click"),
                            QObject::tr("show global search"),
                            QString(":/gif/resources/" + resourcesPath + "三指轻点.gif")),
        TouchpadGestureInfo(TouchpadGestureType::threeFingerSwipeDown,
                            QObject::tr("swipe three fingers down"),
                            QObject::tr("show desktop"),
                            QString(":/gif/resources/" + resourcesPath + "三指向下滑.gif")),
        TouchpadGestureInfo(TouchpadGestureType::threeFingerSwipeUp,
                            QObject::tr("swipe three fingers up"),
                            QObject::tr("open the multitasking view"),
                            QString(":/gif/resources/" + resourcesPath + "三指向上滑.gif")),
        TouchpadGestureInfo(TouchpadGestureType::threeFingersHorizontalSwipe,
                            QObject::tr("swipe three fingers horizontally"),
                            QObject::tr("switch windows"),
                            QString(":/gif/resources/" + resourcesPath + "三指左右滑动.gif")),
        TouchpadGestureInfo(TouchpadGestureType::fourFingerClick,
                            QObject::tr("four fingers click"),
                            QObject::tr("show notification center"),
                            QString(":/gif/resources/" + resourcesPath + "四指轻点.gif"))
    };

    if (!SettingsCommon::isIntelProject()) {
        infos << TouchpadGestureInfo(TouchpadGestureType::fourFingersHorizontalSwipe,
                                     QObject::tr("swipe four fingers horizontally"),
                                     QObject::tr("switch desktop"),
                                     QString(":/gif/resources/" + resourcesPath + "四指左右滑动.gif"));
    }

    return infos;
}
