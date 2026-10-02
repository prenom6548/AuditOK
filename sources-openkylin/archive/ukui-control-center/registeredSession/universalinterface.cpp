/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
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
#include "universalinterface.h"
#include <unistd.h>
#include <QDebug>

static UniversalInterface *s_globalInstance = nullptr;

UniversalInterface* UniversalInterface::self()
{
    if (!s_globalInstance) {
        s_globalInstance = new UniversalInterface();
    }
    return s_globalInstance;
}

UniversalInterface::UniversalInterface()
{
    const QByteArray panelId(UKCC_PANEL_ID);
    if (QGSettings::isSchemaInstalled(panelId)) {
        mUkccPanelGsettings = new QGSettings(panelId, QByteArray());
    }

    const QByteArray StyleId(UKUI_STYLE_ID);
    if (QGSettings::isSchemaInstalled(StyleId)) {
        mUkuiStyleGsettings = new QGSettings(StyleId, QByteArray());
    }

    const QByteArray UkccId(UKCC_ID);
    if (QGSettings::isSchemaInstalled(UkccId)) {
        mUkccGsettings = new QGSettings(UkccId, QByteArray());
    }

    const QByteArray keyboardId(UKUI_KEYBOARD_ID);
    if (QGSettings::isSchemaInstalled(keyboardId)) {
        mKeyboardGsettings = new QGSettings(keyboardId, QByteArray());
    }

    const QByteArray OsdId(UKCC_OSD_ID);
    if (QGSettings::isSchemaInstalled(OsdId)) {
        mUkccOsdGsettings = new QGSettings(OsdId, QByteArray());
    }

    const QByteArray mouseId(UKUI_MOUSE_ID);
    if (QGSettings::isSchemaInstalled(mouseId)) {
        mUkuiMouseGsettings = new QGSettings(mouseId, QByteArray());
    }

    const QByteArray interfaceId(MATE_INTERFACE_ID);
    if (QGSettings::isSchemaInstalled(interfaceId)) {
        mMateInterfaceGsettings = new QGSettings(interfaceId, QByteArray());
    }

    const QByteArray gnomeVinoId(GNOME_VINO_ID);
    if (QGSettings::isSchemaInstalled(gnomeVinoId)) {
        mGnomeVinoGsettings = new QGSettings(gnomeVinoId, QByteArray());
    }

    const QByteArray ukuiVinoId(UKUI_VINO_ID);
    if (QGSettings::isSchemaInstalled(ukuiVinoId)) {
        mUkuiVinoGsettings = new QGSettings(ukuiVinoId, QByteArray());
    }

    const QByteArray backgroundId(MATE_BACKGROUND_ID);
    if (QGSettings::isSchemaInstalled(backgroundId)) {
        mBackgroundGsettings = new QGSettings(backgroundId, QByteArray());
    }

    const QByteArray screensaverId(UKUI_SCREENSAVER_ID);
    if (QGSettings::isSchemaInstalled(screensaverId)) {
        mScreensaverGsettings = new QGSettings(screensaverId, QByteArray());
    }

    const QByteArray screensaverDefaultId(UKUI_SCREENSAVER_DEFAULT_ID);
    if (QGSettings::isSchemaInstalled(screensaverDefaultId)) {
        mScreensaverDefaultGsettings = new QGSettings(screensaverDefaultId, QByteArray());
    }

    const QByteArray calendarId(UKUI_CALENDAR_ID);
    if (QGSettings::isSchemaInstalled(calendarId)) {
        m_calendarGsettings = new QGSettings(calendarId, QByteArray());
    }

    const QByteArray ukccPersonaliseId(UKCC_PERSONALISE_ID);
    if (QGSettings::isSchemaInstalled(ukccPersonaliseId)) {
        m_ukccPersonaliseGsettings = new QGSettings(ukccPersonaliseId, QByteArray());
    }

    unsigned int uid = getuid();
    QString objpath = QString("/org/freedesktop/Accounts/User") + QString::number(uid);
    mAccountUserDbus = new QDBusInterface("org.freedesktop.Accounts",
                                          objpath,
                                          "org.freedesktop.Accounts.User",
                                          QDBusConnection::systemBus());

    mFcitxInputMethodDbus =  new QDBusInterface("org.fcitx.Fcitx",
                                                "/inputmethod",
                                                "org.fcitx.Fcitx.InputMethod",
                                                QDBusConnection::sessionBus());
    mUkccSystemDbus = new QDBusInterface("com.control.center.qt.systemdbus",
                                         "/",
                                         "com.control.center.interface",
                                         QDBusConnection::systemBus());
    mActiveSystemDbus = new QDBusInterface("org.freedesktop.activation",
                           "/org/freedesktop/activation",
                           "org.freedesktop.activation.interface",
                           QDBusConnection::systemBus());
    m_kwinDbusInterface = new QDBusInterface("org.kde.KWin",
                                             "/KWin",
                                             "org.kde.KWin",
                                             QDBusConnection::sessionBus());
    m_wlcomDbusInterface = new QDBusInterface("com.kylin.Wlcom",
                                             "/com/kylin/Wlcom/Renderer",
                                             "com.kylin.Wlcom.Renderer",
                                             QDBusConnection::sessionBus());
    m_wlcomEffectDbusInterface = new QDBusInterface("com.kylin.Wlcom",
                                             "/com/kylin/Wlcom/Effect",
                                             "com.kylin.Wlcom.Effect",
                                             QDBusConnection::sessionBus());
    m_kwinMagnifierInterface = new QDBusInterface("org.ukui.KWin",
                                                  "/Magnifier",
                                                  "org.ukui.Magnifier",
                                                  QDBusConnection::sessionBus());
    m_kwinEffectInterface = new QDBusInterface("org.ukui.KWin",
                                               "/ColorFilter",
                                               "org.ukui.KWin",
                                               QDBusConnection::sessionBus());
}

UniversalInterface::~UniversalInterface()
{
    if (s_globalInstance) {
        delete s_globalInstance;
        s_globalInstance = nullptr;
    }
    if (mUkccPanelGsettings) {
        delete mUkccPanelGsettings;
        mUkccPanelGsettings = nullptr;
    }
    if (mUkuiStyleGsettings) {
        delete mUkuiStyleGsettings;
        mUkuiStyleGsettings = nullptr;
    }
    if (mUkccGsettings) {
        delete mUkccGsettings;
        mUkccGsettings = nullptr;
    }
    if (mKeyboardGsettings) {
        delete mKeyboardGsettings;
        mKeyboardGsettings = nullptr;
    }
    if (mUkccOsdGsettings) {
        delete mUkccOsdGsettings;
        mUkccOsdGsettings = nullptr;
    }
    if (mUkuiMouseGsettings) {
        delete mUkuiMouseGsettings;
        mUkuiMouseGsettings = nullptr;
    }
    if (mMateInterfaceGsettings) {
        delete mMateInterfaceGsettings;
        mMateInterfaceGsettings = nullptr;
    }
    if (mGnomeVinoGsettings) {
        delete mGnomeVinoGsettings;
        mGnomeVinoGsettings = nullptr;
    }
    if (mUkuiVinoGsettings) {
        delete mUkuiVinoGsettings;
        mUkuiVinoGsettings = nullptr;
    }
    if (mBackgroundGsettings) {
        delete mBackgroundGsettings;
        mBackgroundGsettings = nullptr;
    }
    if (mScreensaverGsettings) {
        delete mScreensaverGsettings;
        mScreensaverGsettings = nullptr;
    }
    if (mScreensaverDefaultGsettings) {
        delete mScreensaverDefaultGsettings;
        mScreensaverDefaultGsettings = nullptr;
    }
    if (m_calendarGsettings) {
        delete m_calendarGsettings;
        m_calendarGsettings = nullptr;
    }
    if (m_ukccPersonaliseGsettings) {
        delete m_ukccPersonaliseGsettings;
        m_ukccPersonaliseGsettings = nullptr;
    }
    if (mAccountUserDbus) {
        delete mAccountUserDbus;
        mAccountUserDbus = nullptr;
    }
    if (mFcitxInputMethodDbus) {
        delete mFcitxInputMethodDbus;
        mFcitxInputMethodDbus = nullptr;
    }
    if (mActiveSystemDbus) {
        delete mActiveSystemDbus;
        mActiveSystemDbus = nullptr;
    }
    if (m_kwinDbusInterface) {
        delete m_kwinDbusInterface;
        m_kwinDbusInterface = nullptr;
    }
    if (m_wlcomDbusInterface) {
        delete m_wlcomDbusInterface;
        m_wlcomDbusInterface = nullptr;
    }
    if (m_wlcomEffectDbusInterface) {
        delete m_wlcomEffectDbusInterface;
        m_wlcomEffectDbusInterface = nullptr;
    }
    if (m_kwinMagnifierInterface) {
        delete m_kwinMagnifierInterface;
        m_kwinMagnifierInterface = nullptr;
    }
    if (m_kwinEffectInterface) {
        delete m_kwinEffectInterface;
        m_kwinEffectInterface = nullptr;
    }
}

QGSettings *UniversalInterface::ukccGsettings()
{
    return mUkccGsettings;
}

QGSettings *UniversalInterface::ukccPersonaliseGsettings()
{
    return m_ukccPersonaliseGsettings;
}

QGSettings *UniversalInterface::calendarGsettings()
{
    return m_calendarGsettings;
}

QGSettings* UniversalInterface::ukccPanelGsettings()
{
    return mUkccPanelGsettings;
}

QGSettings* UniversalInterface::ukuiStyleGsettings()
{
    return mUkuiStyleGsettings;
}

QDBusInterface* UniversalInterface::accountUserDbus()
{
    return mAccountUserDbus;
}

QDBusInterface* UniversalInterface::fcitxInputMethodDbus()
{
    return mFcitxInputMethodDbus;
}


QGSettings* UniversalInterface::ukuiKeyBoardGsettings()
{
    return mKeyboardGsettings;
}

QGSettings* UniversalInterface::ukccOsdGsettings()
{
    return mUkccOsdGsettings;
}

QGSettings* UniversalInterface::ukuiVirtualKeyboardGsettings()
{
    if (!mUkuiVirtualKeyboardGsettings) {
        const QByteArray id(UKUI_VIRKEYBOARD_ID);
        if (QGSettings::isSchemaInstalled(id)) {
            mUkuiVirtualKeyboardGsettings = new QGSettings(id, QByteArray());
        }
    }
    return mUkuiVirtualKeyboardGsettings;
}

QGSettings* UniversalInterface::ukuiMouseGsettings()
{
    return mUkuiMouseGsettings;
}

QGSettings* UniversalInterface::mateInterfaceGsettings()
{
    return mMateInterfaceGsettings;
}

QGSettings *UniversalInterface::gnomeVinoGsettings()
{
    return mGnomeVinoGsettings;
}

QGSettings *UniversalInterface::ukuiVinoGsettings()
{
    return mUkuiVinoGsettings;
}

QDBusInterface * UniversalInterface::activeSystemDbus()
{
    return mActiveSystemDbus;
}

QDBusInterface * UniversalInterface::ukccSystemDbus()
{
    return mUkccSystemDbus;
}

QGSettings *UniversalInterface::mateBackgroundGsettings()
{
    return mBackgroundGsettings;
}

QGSettings *UniversalInterface::screensaverGsettings()
{
    return mScreensaverGsettings;
}

QGSettings *UniversalInterface::screensaverDefaultGsettings()
{
    return mScreensaverDefaultGsettings;
}

QDBusInterface* UniversalInterface::kwinDbusInterface() {
    if (!m_kwinDbusInterface) {
        m_kwinDbusInterface = new QDBusInterface("org.kde.KWin",
                                                 "/KWin",
                                                 "org.kde.KWin",
                                                 QDBusConnection::sessionBus());
        qDebug() << Q_FUNC_INFO << "kwin interface is empty, new QDBusInterface valid:" << m_kwinDbusInterface->isValid();
    } else {
        if (!m_kwinDbusInterface->isValid()) {
            m_kwinDbusInterface = new QDBusInterface("org.kde.KWin",
                                                     "/KWin",
                                                     "org.kde.KWin",
                                                     QDBusConnection::sessionBus());
            qDebug() << Q_FUNC_INFO << "kwin interface is not empty but invalid, new QDBusInterface valid:" << m_kwinDbusInterface->isValid();
        }
    }
    return m_kwinDbusInterface;
}

QDBusInterface* UniversalInterface::wlcomDbus()
{
    return m_wlcomDbusInterface;
}

QDBusInterface* UniversalInterface::wlcomEffectDbusInterface() {
    return m_wlcomEffectDbusInterface;
}

QDBusInterface* UniversalInterface::kwinMagnifierInterface() {
    return m_kwinMagnifierInterface;
}

QDBusInterface* UniversalInterface::kwinEffectInterface() {
    return m_kwinEffectInterface;
}
