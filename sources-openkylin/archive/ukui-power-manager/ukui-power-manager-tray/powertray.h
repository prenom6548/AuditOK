/*
 * Copyright 2021 KylinSoft Co., Ltd.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef POWERTRAY_H
#define POWERTRAY_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QTimer>
#include <QDBusInterface>
#include "upm-gsettings/upm_gsettings.h"
#include "upower-dbus/upm_upowerdbus.h"

#include "powerwindow.h"

class PowerTray : public QObject
{
    Q_OBJECT
public:
    explicit PowerTray(QObject *parent = 0);
    ~PowerTray();

private:
    void initPowerTray();
    void setTrayIconToolTipForAcOnlineChanged(bool acOnlineState, int percentage);
    void setTrayIconToolTipForBatteryInfoChanged(int percentage);
    void setTrayIcon(QString strIconName);
    void initTrayMenuUI();
    void initPowerWindowUI();
    void setPowerWindowProperties();
    void updateBatteryInfo();
    void acOnlineStateChangedAction();
    bool isDesktop();
    void applyTooltipIfChanged(const QString& tip);
private:
    QTimer          *m_acChangeTimer;
    bool            m_isPowerSupplyChange;

    int             m_showBatteryLeftTime = 1;
    QMenu           *m_TrayMenu = nullptr;

    powerwindow     *m_powerWindow = nullptr;
    QSystemTrayIcon *m_trayIcon;

    bool            m_acOnlineState;
    int             m_batteryState;
    double          m_batteryPercentage;
    QString         m_trayIconName;
    QString m_lastTooltip;

    int     m_oldPolicy;
    int     m_correctionTime = 0;
    qint64  m_correctionTimeToEmpty;
    bool    m_policyChanged = false;

    qint64  m_timeToFull;
    qint64  m_timeToEmpty;
    QDBusInterface *m_upowerInterface;
private Q_SLOTS:
    void onActivatedIcon(QSystemTrayIcon::ActivationReason reason);

    void dealAcOnlineStateChanged(int, bool);
    void dealBatteryInfoChanged(int, QStringList);
    void dealPowerPolicyBatteryChanged(QVariant);
    void dealWakeUpSignal(bool);
    void dealDeviceChanged();
};

#endif // POWERTRAY_H
