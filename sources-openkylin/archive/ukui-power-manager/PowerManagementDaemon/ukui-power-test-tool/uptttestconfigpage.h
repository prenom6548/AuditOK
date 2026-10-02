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
#ifndef UPTTTESTCONFIGPAGE_H
#define UPTTTESTCONFIGPAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QDateTime>
#include <QFile>
#include <QDir>
#include <QStringList>

#include "upttcpupolicyconfig.h"
#include "upttbrightnessconfig.h"
#include "upttruntestbutton.h"
#include "upttbatterydate.h"

#include "upower-dbus/upm_upowerdbus.h"
#include "uptt-gsettings/upm_gsettings.h"

#define UPM_DBUS_SERVICE          "org.ukui.powermanagement"
#define UPM_DBUS_PATH             "/"
#define UPM_DBUS_INTERFACE        "org.ukui.powermanagement.interface"

class UpttTestConfigPage : public QWidget
{
    Q_OBJECT
public:
    explicit UpttTestConfigPage(QWidget *parent = nullptr);
    ~UpttTestConfigPage();

    void initUpttTestConfigPage();
private:
    UpttCpuPolicyConfig  *m_upttCpuPolicyConfig;
    UpttBrightnessConfig *m_upttBrightnessConfig;
    QCheckBox *m_balanceModeReduceFrequency;
    QCheckBox *m_autoSwitchCpuCore;
    QCheckBox *m_idleReserveCpu0;
    QCheckBox *m_idleReduceBrightness;
    QCheckBox *m_idleTurnoffDisplay;
    QCheckBox *m_idleSystemSuspend;
    QCheckBox *m_disableScreenSaver;
    QCheckBox *m_idlePowerPolicyAuto;
    QCheckBox *m_criticalPercentageDoNothing;
    UpttRunTestButton *m_runTestButton;
    QLabel *m_hintMessage;
    int m_cpuCoreNum = 0;

    enum test_type {
        unknown_test,
        endurance_test,
        energy_rate_test
    };

    bool m_testIsRunning = false;
    int m_testType = unknown_test;
    bool m_refreshBatteryInfo = false;

    QDateTime m_runTestDateTime;
    double m_energyNow;
    double m_energyFull;
    double m_energyFullDesign;
    double m_batteryHealth;
    quint64 m_startBrightness;
    quint64 m_stopBrightness;

    QVector<UpttBatteryDate *> m_batteryDateVector;
    int m_minBatteryPercentage;

    QTimer *m_energyRateTestTimer = nullptr;

    QString m_testReportDir;

    bool m_hasNewReport = false;

    void initTestConfigPageUI();
    void setTestConfig();

    void createReportPath();
    void createEnduranceTestReport();
    void createEnergyRateTestReport();

    void clearBatteryDate();
    QString secondsToTimeFormat(int seconds);

private slots:
    void runEnduranceTest();
    void runEnergyRateTest();
    void stopTest();
    void acOnlineTestNotRun();

    void recordBatteryDate(int , QStringList);
    void dealEnergyRateTestTimer();
};

#endif // UPTTTESTCONFIGPAGE_H
