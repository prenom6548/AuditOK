/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 *
 */
#ifndef UPMEXTERNALINTERFACE_H
#define UPMEXTERNALINTERFACE_H

#include <QObject>
#include <QtDBus/QtDBus>
#include <QGSettings>
#include <QTimer>

#include "common.h"

enum powersave_level
{
    powersave_level_normal = 0,
    powersave_level_low,
    powersave_level_medium,
    powersave_level_high
};

enum power_policy
{
    power_policy_performance = 0,
    power_policy_balance,
    power_policy_powersave,
    power_policy_unknown
};

class UpmExternalInterface : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.powerManager")

public:
    explicit UpmExternalInterface(QObject *parent = nullptr);
    ~UpmExternalInterface();

private:
    int m_powersaveLevel;
    int m_currentPowerPolicy;
    bool m_batteryOnline;
    bool m_percentageLowState;

    QGSettings *m_powerGsettings;

    bool isOnBattery();
    void updatePowersaveLevel();

private slots:
    void dealUPowerDBusMessage(QDBusMessage msg);
    void dealPercentageLowMessage(bool state);

public slots:
    int getPowersaveLevel();

signals:
    void powersaveLevelChanged(int);

};

#endif // UPMEXTERNALINTERFACE_H
