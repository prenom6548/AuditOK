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
#ifndef UNIVERSALINTERFACE_H
#define UNIVERSALINTERFACE_H
#include <QByteArray>
#include <QGSettings/QGSettings>
#include <QDBusInterface>

class UniversalInterface
{
public:
    UniversalInterface();
    ~UniversalInterface();

    /**
     * @brief 获取通用接口类的实例
     *
     * @return UniversalInterface
     */
    static UniversalInterface *self();

    /**
     * @brief 获取org.ukui.KWin的dbus
     *
     * @return QDBusInterface
     */
    QDBusInterface* kwinDbusInterface();

    /**
     * @brief 获取com.kylin.Wlcom的dbus
     *
     * @return QDBusInterface
     */
    QDBusInterface* wlcomDbusInterface();

    /**
     * @brief 获取org.ukui.control-center.plugins的gsetting
     *
     * @return QGSettings
     */
    QGSettings *ukccGsettings();

    /**
     * @brief 获取org.ukui.control-center.personalise的gsetting
     *
     * @return QGSettings
     */
    QGSettings *ukccPersonaliseGsettings();

    /**
     * @brief 获取org.kylin.calendar.plugin的gsetting
     *
     * @return QGSettings
     */
    QGSettings *calendarGsettings();

    /**
     * @brief 获取org.ukui.control-center.panel.plugins的gsetting
     *
     * @return QGSettings
     */
    QGSettings *ukccPanelGsettings();

    /**
     * @brief 获取org.ukui.style的gsetting
     *
     * @return QGSettings
     */
    QGSettings *ukuiStyleGsettings();

    /**
     * @brief 获取当前用户的org.freedesktop.Accounts.User接口
     *
     * @return QDBusInterface
     */
    QDBusInterface *accountUserDbus();

    /**
     * @brief 获取org.fcitx.Fcitx.InputMethod接口
     *
     * @return QDBusInterface
     */
    QDBusInterface *fcitxInputMethodDbus();

    /**
     * @brief 获取org.ukui.peripherals-keyboard的gsetting
     *
     * @return QGSettings
     */
    QGSettings* ukuiKeyBoardGsettings();

    /**
     * @brief 获取org.ukui.control-center.osd的gsetting
     *
     * @return QGSettings
     */
    QGSettings* ukccOsdGsettings();

    /**
     * @brief 获取org.ukui.virtualkeyboard的gsetting
     *
     * @return QGSettings
     */
    QGSettings* ukuiVirtualKeyboardGsettings();

    /**
     * @brief 获取org.ukui.peripherals-mouse的gsetting
     *
     * @return QGSettings
     */
    QGSettings* ukuiMouseGsettings();

    /**
     * @brief 获取org.mate.interface的gsetting
     *
     * @return QGSettings
     */
    QGSettings* mateInterfaceGsettings();

    /**
     * @brief 获取org.gnome.Vino的gsetting
     *
     * @return QGSettings
     */
    QGSettings *gnomeVinoGsettings();

    /**
     * @brief 获取org.ukui.control-center的gsetting
     *
     * @return QGSettings
     */
    QGSettings *ukuiVinoGsettings();
    /**
     * @brief 获取com.control.center.qt.systemdbus的dbus
     *
     * @return QDBusInterface
     */
    QDBusInterface *ukccSystemDbus();

    /**
     * @brief 获取org.freedesktop.activation的dbus
     *
     * @return QDBusInterface
     */
    QDBusInterface *activeSystemDbus();

    /**
     * @brief 获取org.mate.background的Gsetting
     *
     * @return QDBusInterface
     */
    QGSettings *mateBackgroundGsettings();

    /**
     * @brief 获取org.ukui.screensaver的Gsetting
     *
     * @return QDBusInterface
     */
    QGSettings *screensaverGsettings();

    /**
     * @brief 获取org.ukui.screensaver-default的Gsetting
     *
     * @return QDBusInterface
     */
    QGSettings *screensaverDefaultGsettings();

    /**
     * @brief 获取com.kylin.Wlcom的dbus
     *
     * @return QDBusInterface
     */
    QDBusInterface *wlcomDbus();

    /**
     * @brief 获取com.kylin.Wlcom effect的dbus
     *
     * @return QDBusInterface
     */
    QDBusInterface *wlcomEffectDbusInterface();

    /**
     * @brief 获取com.kylin.KWin Magnifier的dbus
     *
     * @return QDBusInterface
     */
    QDBusInterface *kwinMagnifierInterface();

    /**
     * @brief 获取com.kylin.KWin effect的dbus
     *
     * @return QDBusInterface
     */
    QDBusInterface *kwinEffectInterface();

private:
    const QByteArray UKCC_ID           = QByteArray("org.ukui.control-center");
    const QByteArray UKCC_PANEL_ID     = QByteArray("org.ukui.control-center.panel.plugins");
    const QByteArray UKUI_STYLE_ID     = QByteArray("org.ukui.style");
    const QByteArray UKUI_KEYBOARD_ID  = QByteArray("org.ukui.peripherals-keyboard");
    const QByteArray UKCC_OSD_ID       = QByteArray("org.ukui.control-center.osd");
    const QByteArray UKUI_VIRKEYBOARD_ID = QByteArray("org.ukui.virtualkeyboard");
    const QByteArray UKUI_MOUSE_ID     = QByteArray("org.ukui.peripherals-mouse");
    const QByteArray MATE_INTERFACE_ID = QByteArray("org.mate.interface");
    const QByteArray GNOME_VINO_ID     = QByteArray("org.gnome.Vino");
    const QByteArray UKUI_VINO_ID      = QByteArray("org.ukui.control-center");
    const QByteArray MATE_BACKGROUND_ID  = QByteArray("org.mate.background");
    const QByteArray UKUI_SCREENSAVER_ID = QByteArray("org.ukui.screensaver");
    const QByteArray UKUI_SCREENSAVER_DEFAULT_ID = QByteArray("org.ukui.screensaver-default");
    const QByteArray UKUI_CALENDAR_ID = QByteArray("org.kylin.calendar.plugin");
    const QByteArray UKCC_PERSONALISE_ID = QByteArray("org.ukui.control-center.personalise");

private:
    QGSettings         *mBackgroundGsettings         = nullptr;
    QGSettings         *mUkccGsettings               = nullptr;
    QGSettings         *mUkccPanelGsettings          = nullptr;
    QGSettings         *mUkuiStyleGsettings          = nullptr;
    QDBusInterface     *mAccountUserDbus             = nullptr;
    QDBusInterface     *mFcitxInputMethodDbus        = nullptr;
    QGSettings         *mKeyboardGsettings           = nullptr;
    QGSettings         *mUkccOsdGsettings            = nullptr;
    QGSettings         *mUkuiVirtualKeyboardGsettings= nullptr;
    QGSettings         *mUkuiMouseGsettings          = nullptr;
    QGSettings         *mMateInterfaceGsettings      = nullptr;
    QGSettings         *mGnomeVinoGsettings          = nullptr;
    QGSettings         *mUkuiVinoGsettings           = nullptr;
    QDBusInterface     *mUkccSystemDbus              = nullptr;
    QGSettings         *mScreensaverGsettings        = nullptr;
    QGSettings         *mScreensaverDefaultGsettings = nullptr;
    QDBusInterface     *mActiveSystemDbus            = nullptr;
    QGSettings         *m_calendarGsettings          = nullptr;
    QGSettings         *m_ukccPersonaliseGsettings   = nullptr;
    QDBusInterface     *m_kwinDbusInterface          = nullptr;
    QDBusInterface     *m_wlcomDbusInterface         = nullptr;
    QDBusInterface     *m_wlcomEffectDbusInterface   = nullptr;
    QDBusInterface     *m_kwinMagnifierInterface     = nullptr;
    QDBusInterface     *m_kwinEffectInterface        = nullptr;
};

#endif // UNIVERSALINTERFACE_H
