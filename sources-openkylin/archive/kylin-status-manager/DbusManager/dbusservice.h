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

#ifndef _DBUSSERVICE_H
#define _DBUSSERVICE_H

#include <QDBusConnection>
#include <QObject>
#include <QDBusContext>
class StatusManager;
class InterfaceAdaptor;
class DBusService : public QObject, public QDBusContext{
    Q_OBJECT

public:
    explicit DBusService(StatusManager *manager = nullptr);
    ~DBusService() override;

public slots:
    bool isAutoRotateEnabled() const;
    QString getDeviceOrientation();
    bool isTabletMode() const;
    bool getCurrentTabletMode() const;
    // 这个 Builtin 不能去掉，虚拟键盘已经在使用这个接口了。
    bool isBuiltinKeyboardAvailable() const;
    bool isAutoSwitchEnabled() const;
    bool isSupportedAutoRotation() const;
    bool setAutoRotateEnabled(bool autoRotateEnable, const QString &who, const QString &why);
    bool setAutoSwitchEnabled(bool autoSwitchEnable, const QString &who, const QString &why);
    bool setDeviceOrientation(const QString &orientation, const QString &who, const QString &why);
    bool setTabletMode(bool tabletMode, const QString &who, const QString &why);
    bool isTabletModeEnabled() const;
    void setTabletModeEnabled(bool enabled);

    bool get_auto_rotation() const;
    QString get_current_rotation();
    bool get_current_tabletmode() const;
    int get_inputmethod_mode() const;
    bool get_modemonitor() const;
    bool is_supported_autorotation() const;
    bool set_auto_rotation(bool autoRotation, const QString &who, const QString &why);
    bool set_modemonitor(bool modeMonitor, const QString &who, const QString &why);
    bool set_rotation(const QString &rotation, const QString &who, const QString &why);
    bool set_tabletmode(bool tabletMode, const QString &who, const QString &why);

    bool isClassicalMode();
    void setClassicalMode(bool classicalMode, const QString &why);

    /**
     * @brief isImmersiveMode 沉浸模式仅在PC下生效，避免额外的变化切换到平板时不对外发送信号，影响桌面组件的变化导致平板切换回沉浸PC需要过度经典的问题。
     * @return
     */
    bool isImmersiveMode();
    /**
     * @brief setImmersiveMode
     * @param immersiveMode
     * @param why
     */
    void setImmersiveMode(bool immersiveMode, const QString &why);

    QString OSAnimation();
    QString OSFunction();
signals:
    void autoRotateEnabledChange(bool autoRotateEnable);
    void keyboardAvailableChanged(bool available);
    void tabletModeChanged(bool tabletMode);
    void autoSwitchEnabledChanged(bool autoSwitchEnable);
    void orientationChanged(const QString &orientation);
    void isTabletModeEnabledChanged(bool enabled);
    void isClassicalModeChanged(bool enabled);
    void isImmersiveModeChanged(bool enabled);
private:
    InterfaceAdaptor *m_adaptor = nullptr;
    StatusManager *m_statusManager = nullptr;
    QString m_serviceName;

    void startService();
    void stopService();
    void connectSignals();
    void connectSignal(const char* fromSignal,
                       const char* toOldSignal,
                       const char* toNewSignal);
    void connectStatusManagerSignal();
};

#endif
