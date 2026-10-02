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

#ifndef STATUSMANAGER_H
#define STATUSMANAGER_H

#include <QObject>
#include <ukui/event.h>
#include <QMap>

class KeyboardManager;
class KmreManager;
class DeviceModeManager;
class DeviceRotationManager;
class UKUIInputGatherClient;
class ConfigurationManager;

class StatusManager : public QObject {
    Q_OBJECT

public:
    explicit StatusManager(QObject *parent = nullptr);

    bool isOrientationSensorAvailable() const;
    bool isAutoRotateEnabled() const;
    bool setDeviceOrientationPermanent(const QString &orientation);
    QString getDeviceOrientation() const;
    bool isTabletMode() const;
    bool isAutoSwitchEnabled() const;
    bool setAutoSwitchEnabled(bool autoSwitchEnabled);
    bool setTabletMode(bool tabletMode);
    bool isKeyboardAvailable() const;
    void notifyStartupFinished();
    void setTabletModeEnabled(bool enabled);
    bool isTabletModeEnabled() const;
    bool isClassicalMode() const;
    void setClassicalMode(bool classicalMode);

    /**
     * @brief isImmersiveMode
     * @return 0:非沉浸模式，1：沉浸模式
     */
    bool isImmersiveMode() const;
    /**
     * @brief setImmersiveMode
     * @param immersiveMode
     */
    void setImmersiveMode(bool immersiveMode);

    QMap<QString, QVariant> getLiteMode() const;
private:
    void initDeviceModeManager();
    void restoreDeviceStatus();

    void notifySessionStartupFinished();

    void connectSignals();
    void connectKmreManagerSignals();
    void connectDeviceModeManagerSignals();
    void connectKeyboardSignals();
    void connectDeviceRotateManagerSignals();

    bool localizedAutoRotateEnabled() const;
    bool localizedTabletMode() const;
    bool localizedTabletModeEnabled() const;
    bool localizedAutoSwitchEnabled() const;

    QMap<QString, QVariant> localizedLiteMode() const;
    bool localizedClassicalMode() const;
    bool localizedImmersiveMode() const;
    QString localizedDeviceOrientation() const;
    void restoreDeviceMode();
    bool getDeviceModeStatus();
    void restoreAutoSwitchStatus();
    void restoreAutoRotateStatus();
    void restoreDeviceOrientation();

    void initInputGatherProxy();
    void monitorInputGatherTabletSwitchState(Event* e);
    void initKeyboardAvailable();

    void initMachineType();
    bool isMaxTabletDevice() const;
    bool isPhytunumDevice() const;
    QString getDeviceInfo(const QString &property) const;
    QString getSysVendor() const;
    QString getProductName() const;
    QString getProductFamily() const;

    bool isOpenkylinSystem() const;

private:
    bool m_requestToSwitchModeToTablet;
    bool m_isPhytiumDevice = false;
    bool m_isMaxTabletDevice = false;

    KeyboardManager *m_keyboardManager = nullptr;
    KmreManager *m_kmreManager = nullptr;
    DeviceModeManager *m_deviceModeManager = nullptr;
    DeviceRotationManager *m_deviceRotationManager = nullptr;
    UKUIInputGatherClient *m_inputGatherClient = nullptr;
    ConfigurationManager *m_configurationManager = nullptr;

signals:
    void autoRotateEnabledChange(bool autoRotateEnable);
    void keyboardAvailableChanged(bool available);
    void tabletModeChanged(bool tabletMode);
    void autoSwitchEnabledChanged(bool autoSwitchEnable);
    void orientationChanged(const QString &orientation);
    void isTabletModeEnabledChanged(bool enabled);
    void isClassicalModeChanged(bool classicalMode);
    void isImmersiveModeChanged(bool isImmersive);
public slots:
    bool setAutoRotateEnabled(bool autoRotateEnable, bool isPersistence);
    void handleKmreOperationSignal(bool allowModeSwitching);
    void handleSwitchStateChanged(bool tabletMode);
};

#endif // STATUSMANAGER_H
