/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef DEVICEMODEMANAGER_H
#define DEVICEMODEMANAGER_H

#include <functional>
#include <QObject>
#include <QVariant>
#include <QMap>

class QDBusPendingCallWatcher;
class DeviceModeManager : public QObject {
    Q_OBJECT

public:
    using ShowNotificationCallback = std::function<void()>;
    using ShouldShowNotificationCallback = std::function<bool()>;
    using CheckFirstDeviceModeSwitchCallback = std::function<bool()>;

public:
    explicit DeviceModeManager(QObject *parent = nullptr);
    bool isAutoSwitchEnabled() const;
    bool isTabletMode() const;
    bool isTabletModeEnabled() const;
    void setShowNotificationCallback(ShowNotificationCallback func);
    void setShouldShowNotificationCallback(ShouldShowNotificationCallback func);
    void setCheckFirstDeviceModeSwitchFunction(CheckFirstDeviceModeSwitchCallback func);
    /*Lite Mode*/
    /**
     * @brief isLiteMode
     * @return 0:非 lite模式，1：lite模式
     */
    QMap<QString, QVariant> getLiteMode() const;
    /*Desktop Mode*/
    /**
     * @brief isClassicMode
     * @return 0：三岛模式，1：经典模式，
     */
    bool isClassicalMode() const;
    /**
     * @brief isImmersiveMode
     * @return 0:非沉浸模式，1：沉浸模式。
     */
    bool isImmersiveMode() const;
    int getDeviceAllMode();
    void setLiteMode(QMap<QString, QVariant> liteMode);
private:
    bool m_tabletMode = false;
    bool m_autoSwitchEnabled = true;
    bool m_isTabletModeEnabled = false;
    bool m_isClassicalMode = true;
    bool m_isImmersiveMode = false;
    QMap<QString, QVariant> m_liteModeMap;

    ShowNotificationCallback m_showNotification;
    ShouldShowNotificationCallback m_shouldShowNotification;
    CheckFirstDeviceModeSwitchCallback m_checkFirstDeviceModeSwitch;

private:
    void showNotification() const;
    bool shouldShowNotification() const;
    bool shouldOpenUserGuide() const;
    void openUserGuide();

signals:
    void tabletModeChanged(bool tabletMode);
    void isTabletModeEnabledChanged(bool enabled);
    void autoSwitchEnabledChanged(bool autoSwitchEnabled);
    void isClassicalModeChanged(bool classicalMode);
    void isImmersiveModeChanged(bool ImmersiveMode);

public slots:
    bool setAutoSwitchEnabled(bool autoSwitchEnabled);
    void requestToSwitchMode(bool tabletMode);

    void setTabletMode(bool tabletMode);
    void setTabletModeEnabled(bool enabled);

    void setClassicalMode(bool classicalMode);
    void setImmersiveMode(bool immersiveMode);
private slots:
    void callUserGuideFinishedSlot(QDBusPendingCallWatcher *watcher);
};

#endif // DEVICEMODEMANAGER_H
