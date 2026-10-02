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
#include "uptttestconfigpage.h"

#include <QFileInfo>
#include <QTextStream>
#include <QDebug>

UpttTestConfigPage::UpttTestConfigPage(QWidget *parent) : QWidget(parent)
{
    m_energyFull = UpmUpowerDBus::self()->getEnergyFull();
    m_energyFullDesign = UpmUpowerDBus::self()->getEnergyFullDesign();
    m_batteryHealth = m_energyFull / m_energyFullDesign;
    qDebug() << "battery health:" << m_batteryHealth;
}

UpttTestConfigPage::~UpttTestConfigPage()
{
    if (!m_batteryDateVector.isEmpty()) {
        clearBatteryDate();
    }
    
    if (nullptr != m_energyRateTestTimer) {
        delete m_energyRateTestTimer;
    }
}

void UpttTestConfigPage::initUpttTestConfigPage()
{
    initTestConfigPageUI();

    connect(m_runTestButton, &UpttRunTestButton::runEnduranceTestSignal,
            this, &UpttTestConfigPage::runEnduranceTest);
    connect(m_runTestButton, &UpttRunTestButton::runEnergyRateTestSignal,
            this, &UpttTestConfigPage::runEnergyRateTest);
    connect(m_runTestButton, &UpttRunTestButton::stopTestSignal,
            this, &UpttTestConfigPage::stopTest);
    connect(m_runTestButton, &UpttRunTestButton::acOnlineTestNotRunSignal,
            this, &UpttTestConfigPage::acOnlineTestNotRun);

    connect(UpmUpowerDBus::self(), &UpmUpowerDBus::batteryInfoChanged,
            this, &UpttTestConfigPage::recordBatteryDate);
}

void UpttTestConfigPage::initTestConfigPageUI()
{
    setWindowIcon(QIcon("ukui-power-manager"));
    setFixedSize(800, 600);

    setWindowTitle("续航测试简陋版");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    m_upttCpuPolicyConfig = new UpttCpuPolicyConfig(this);

    m_upttBrightnessConfig = new UpttBrightnessConfig(this);

    m_balanceModeReduceFrequency = new QCheckBox("平衡模式降低CPU频率");
    m_balanceModeReduceFrequency->setChecked(false);

    m_autoSwitchCpuCore = new QCheckBox("自动开关CPU核心");
    m_autoSwitchCpuCore->setChecked(false);

    m_idleReserveCpu0 = new QCheckBox("系统空闲时仅保留cpu0");
    m_idleReserveCpu0->setChecked(false);

    m_idleReduceBrightness = new QCheckBox("系统空闲时降低屏幕亮度");
    m_idleReduceBrightness->setChecked(true);

    m_idleTurnoffDisplay   = new QCheckBox("系统空闲时不关闭显示器");
    m_idleTurnoffDisplay->setChecked(true);

    m_idleSystemSuspend    = new QCheckBox("系统空闲时不进入睡眠");
    m_idleSystemSuspend->setChecked(true);

    m_disableScreenSaver   = new QCheckBox("系统空闲时不开启屏幕保护");
    m_disableScreenSaver->setChecked(true);

    m_idlePowerPolicyAuto  = new QCheckBox("系统空闲时切换节能模式");
    m_idlePowerPolicyAuto->setChecked(false);

    m_criticalPercentageDoNothing = new QCheckBox("低电量无操作");
    m_criticalPercentageDoNothing->setChecked(true);

    m_runTestButton = new UpttRunTestButton(this);

    m_hintMessage = new QLabel(this);

    mainLayout->addWidget(m_upttCpuPolicyConfig);
    mainLayout->addWidget(m_upttBrightnessConfig);
    mainLayout->addWidget(m_balanceModeReduceFrequency);
    mainLayout->addWidget(m_autoSwitchCpuCore);
    mainLayout->addWidget(m_idleReserveCpu0);
    mainLayout->addWidget(m_idleReduceBrightness);
    mainLayout->addWidget(m_idleTurnoffDisplay);
    mainLayout->addWidget(m_idleSystemSuspend);
    mainLayout->addWidget(m_disableScreenSaver);
    mainLayout->addWidget(m_idlePowerPolicyAuto);
    mainLayout->addWidget(m_criticalPercentageDoNothing);
    mainLayout->addWidget(m_runTestButton);
    mainLayout->addWidget(m_hintMessage);
    mainLayout->addStretch();
}

void UpttTestConfigPage::setTestConfig()
{
    UpmGsettings::self()->setGsettingsConfig(GSETTINGS_KEY_AUTO_BRIGHTENSS, false);

    int cpuPolicy = m_upttCpuPolicyConfig->getPowerPolicyBattery();

    QDBusInterface dBusInterface(UPM_DBUS_SERVICE,
                                 UPM_DBUS_PATH,
                                 UPM_DBUS_INTERFACE,
                                 QDBusConnection::systemBus());
    dBusInterface.call("EnableReduceCpuFrequency", m_balanceModeReduceFrequency->isChecked());

    dBusInterface.call("EnableAutoSwitchCpuCore", m_autoSwitchCpuCore->isChecked(), cpuPolicy);

    dBusInterface.call("EnableSystemIdleReserveCpu0", m_idleReserveCpu0->isChecked(), cpuPolicy);

    if (true == m_idleReduceBrightness->isChecked()) {
        UpmGsettings::self()->setGsettingsConfig(GSETTINGS_KEY_IDLE_BRIGHTNESS, 70);
    } else {
        UpmGsettings::self()->setGsettingsConfig(GSETTINGS_KEY_IDLE_BRIGHTNESS, 100);
    }

    if (true == m_idleTurnoffDisplay->isChecked()) {
        UpmGsettings::self()->setGsettingsConfig(GSETTINGS_KEY_SLEEP_DISPLAY_BATTERY, -1);
    } else {
        UpmGsettings::self()->resetGsettingsConfig(GSETTINGS_KEY_SLEEP_DISPLAY_BATTERY);
    }

    if (true == m_idleSystemSuspend->isChecked()) {
        UpmGsettings::self()->setGsettingsConfig(GSETTINGS_KEY_SLEEP_COMPUTER_BATTERY, -1);
    } else {
        UpmGsettings::self()->resetGsettingsConfig(GSETTINGS_KEY_SLEEP_COMPUTER_BATTERY);
    }

    if (true == m_disableScreenSaver->isChecked()) {
        UpmGsettings::self()->setGsettingsConfig(GSETTINGS_KEY_IDLE_DELAY, -1);
    } else {
        UpmGsettings::self()->resetGsettingsConfig(GSETTINGS_KEY_IDLE_DELAY);
    }

    if (true == m_idlePowerPolicyAuto->isChecked()) {
        UpmGsettings::self()->setGsettingsConfig(GSETTINGS_KEY_IDLE_POWER_POLICY_AUTO, true);
    } else {
        UpmGsettings::self()->setGsettingsConfig(GSETTINGS_KEY_IDLE_POWER_POLICY_AUTO, false);
    }

    if (true == m_criticalPercentageDoNothing->isChecked()) {
        UpmGsettings::self()->setGsettingsConfig(GSETTINGS_KEY_ACTION_CRITICAL_BATTERY, "nothing");
        m_minBatteryPercentage = 2;
    } else {
        UpmGsettings::self()->resetGsettingsConfig(GSETTINGS_KEY_ACTION_CRITICAL_BATTERY);
        QVariant value;
        if (true == UpmGsettings::self()->getGsettingsConfig(GSETTINGS_KEY_PERCENTAGE_ACTION, value)) {
            m_minBatteryPercentage = value.toInt();
        }
    }

    UpmGsettings::self()->setGsettingsConfig(GSETTINGS_KEY_POWER_POLICY_BATTERY, cpuPolicy);

    UpmGsettings::self()->setGsettingsConfig(GSETTINGS_KEY_BRIGHTNESS_AC,
                                             m_upttBrightnessConfig->getBrightnessPercentage());

    QDBusReply<quint64> reply = dBusInterface.call("GetBrightness");
    m_startBrightness = reply.value();
}

void UpttTestConfigPage::runEnduranceTest()
{
    clearBatteryDate();

    setTestConfig();

    m_hasNewReport = false;
    m_runTestDateTime = QDateTime::currentDateTime();
    m_energyNow = UpmUpowerDBus::self()->getEnergyNow();
    m_testIsRunning = true;
    m_testType = endurance_test;
    showMinimized();
}

void UpttTestConfigPage::runEnergyRateTest()
{
    clearBatteryDate();
    if (nullptr == m_energyRateTestTimer) {
        m_energyRateTestTimer = new QTimer();
        connect(m_energyRateTestTimer, &QTimer::timeout, this, &UpttTestConfigPage::dealEnergyRateTestTimer);
    }
    setTestConfig();

    m_testType = energy_rate_test;
    m_hasNewReport = false;
    m_testIsRunning = true;
    m_refreshBatteryInfo = true;
    UpmUpowerDBus::self()->refreshBatteryInfo();
    m_energyRateTestTimer->start(360000);
    showMinimized();
}

void UpttTestConfigPage::stopTest()
{
    if (true == m_hasNewReport) {
        m_hintMessage->setText("测试结束,有新的测试报告");
    } else {
        m_hintMessage->setText("测试结束");
    }
    activateWindow();
}

void UpttTestConfigPage::acOnlineTestNotRun()
{
    m_hintMessage->setText("电源未拔出，1分钟内拔出电源，将开始测试，否则结束测试");
}

void UpttTestConfigPage::recordBatteryDate(int index, QStringList batteryInfo)
{
    if (false == m_testIsRunning) {
        return ;
    }

    int percentage = UpmUpowerDBus::self()->getBatteryPercentage();
    if (endurance_test == m_testType) {
        if (percentage < 6) {
            if (percentage != m_batteryDateVector.last()->getBatteryPercentage()) {
                UpttBatteryDate *batteryDate = new UpttBatteryDate(percentage,
                                                                   UpmUpowerDBus::self()->getEnergyNow(),
                                                                   m_batteryDateVector.last()->getRecordDateTime(),
                                                                   m_batteryDateVector.last()->getBatteryEnergy());
                m_batteryDateVector << batteryDate;
            }
            if (percentage <= m_minBatteryPercentage) {
                m_testIsRunning = false;
                createEnduranceTestReport();
            }
        } else {
            if (0 == percentage % 10 && 100 != percentage) {
                if (0 == m_batteryDateVector.size()) {
                    UpttBatteryDate *batteryDate = new UpttBatteryDate(percentage,
                                                                       UpmUpowerDBus::self()->getEnergyNow(),
                                                                       m_runTestDateTime,
                                                                       m_energyNow);
                    m_batteryDateVector << batteryDate;
                    if (true == m_idleReduceBrightness->isChecked()) {
                        QDBusInterface dBusInterface(UPM_DBUS_SERVICE,
                                                     UPM_DBUS_PATH,
                                                     UPM_DBUS_INTERFACE,
                                                     QDBusConnection::systemBus());
                        QDBusReply<quint64> reply = dBusInterface.call("GetBrightness");
                        m_stopBrightness = reply.value();
                    }
                } else {
                    if (percentage != m_batteryDateVector.last()->getBatteryPercentage()) {
                        UpttBatteryDate *batteryDate = new UpttBatteryDate(percentage,
                                                                           UpmUpowerDBus::self()->getEnergyNow(),
                                                                           m_batteryDateVector.last()->getRecordDateTime(),
                                                                           m_batteryDateVector.last()->getBatteryEnergy());
                        m_batteryDateVector << batteryDate;
                    }
                }
            }
        }
    } else if (energy_rate_test == m_testType) {
        if (true == m_refreshBatteryInfo) {
            if (0 == m_batteryDateVector.size()) {
                UpttBatteryDate *batteryDate = new UpttBatteryDate(
                            percentage, UpmUpowerDBus::self()->getEnergyNow(), QDateTime::currentDateTime(), 0);
                m_batteryDateVector << batteryDate;
            } else {
                UpttBatteryDate *batteryDate = new UpttBatteryDate(percentage,
                                                                   UpmUpowerDBus::self()->getEnergyNow(),
                                                                   m_batteryDateVector.last()->getRecordDateTime(),
                                                                   m_batteryDateVector.last()->getBatteryEnergy());
                m_batteryDateVector << batteryDate;
                createEnergyRateTestReport();
                m_energyRateTestTimer->stop();
            }
            m_refreshBatteryInfo = false;
        }
    } else {
        ;
    }
}

void UpttTestConfigPage::dealEnergyRateTestTimer()
{
    m_refreshBatteryInfo = true;
    UpmUpowerDBus::self()->refreshBatteryInfo();
}

void UpttTestConfigPage::createReportPath()
{
    if (endurance_test == m_testType) {
        m_testReportDir = QDir::homePath() + QString("/power-test/endurance_test/");
    } else if (energy_rate_test == m_testType) {
        m_testReportDir = QDir::homePath() + QString("/power-test/energy_rate_test/");
    } else {
        return ;
    }

    QDir testReportDir(m_testReportDir);
    if (false == testReportDir.exists()) {
        testReportDir.mkpath(m_testReportDir);
    }
}

void UpttTestConfigPage::createEnduranceTestReport()
{
    createReportPath();

    QString reportFileName = m_testReportDir
                           + QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss")
                           + QString(".report");

    QFile reportFile(reportFileName);

    if (!reportFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return ;
    }

    QString reportLine("---------------<测试配置>----------------\n");
    reportFile.write(reportLine.toUtf8());
    QDBusInterface dBusInterface(UPM_DBUS_SERVICE,
                                 UPM_DBUS_PATH,
                                 UPM_DBUS_INTERFACE,
                                 QDBusConnection::systemBus());
    QDBusReply<int> replyCpuCoreNum = dBusInterface.call("GetCpuCoreNum");

    for (int i = 0; i < replyCpuCoreNum.value(); ++i) {
        QDBusReply<QString> replyPolicy = dBusInterface.call("GetCpuCorePolicy", i);
        reportLine = QString("<cpu-%1-模式： %2>\n").arg(i).arg(replyPolicy.value().simplified());
        reportFile.write(reportLine.toUtf8());
    }

    QStringList policyList = {"performance", "balance", "powersave"};
    reportLine = QString("<工作模式： %1>\n").arg(policyList[m_upttCpuPolicyConfig->getPowerPolicyBattery()]);
    reportFile.write(reportLine.toUtf8());
    reportLine = QString("<屏幕亮度百分比： %1%>\n").arg(m_upttBrightnessConfig->getBrightnessPercentage());
    reportFile.write(reportLine.toUtf8());
    reportLine = QString("<平衡模式降低CPU频率： %1>\n").arg(m_balanceModeReduceFrequency->isChecked());
    reportFile.write(reportLine.toUtf8());
    reportLine = QString("<自动开关CPU核心： %1>\n").arg(m_autoSwitchCpuCore->isChecked());
    reportFile.write(reportLine.toUtf8());
    reportLine = QString("<系统空闲时仅保留cpu0： %1>\n").arg(m_idleReserveCpu0->isChecked());
    reportFile.write(reportLine.toUtf8());
    reportLine = QString("<系统空闲时降低屏幕亮度： %1>\n").arg(m_idleReduceBrightness->isChecked());
    reportFile.write(reportLine.toUtf8());
    if (true == m_idleReduceBrightness->isChecked()) {
        reportLine = QString("<开始亮度： %1>\n").arg(m_startBrightness);
        reportFile.write(reportLine.toUtf8());
        reportLine = QString("<结束亮度： %1>\n").arg(m_stopBrightness);
        reportFile.write(reportLine.toUtf8());
    }
    reportLine = QString("<系统空闲时不关闭显示器： %1>\n").arg(m_idleTurnoffDisplay->isChecked());
    reportFile.write(reportLine.toUtf8());
    reportLine = QString("<系统空闲时不进入睡眠： %1>\n").arg(m_idleSystemSuspend->isChecked());
    reportFile.write(reportLine.toUtf8());
    reportLine = QString("<系统空闲时不开启屏幕保护： %1>\n").arg(m_disableScreenSaver->isChecked());
    reportFile.write(reportLine.toUtf8());
    reportLine = QString("<系统空闲时切换节能模式： %1>\n").arg(m_idlePowerPolicyAuto->isChecked());
    reportFile.write(reportLine.toUtf8());
    reportLine = QString("<低电量无操作： %1>\n").arg(m_criticalPercentageDoNothing->isChecked());
    reportFile.write(reportLine.toUtf8());

    reportLine = QString("\n---------------<电池信息>---------------\n");
    reportFile.write(reportLine.toUtf8());

    reportLine = QString("<开始时间： %1>\n").arg(m_runTestDateTime.toString("hh:mm:ss"));
    reportFile.write(reportLine.toUtf8());
    reportLine = QString("<当前电量： %1wh>\n").arg(m_energyNow);
    reportFile.write(reportLine.toUtf8());
    reportLine = QString("<满电电量： %1wh>\n").arg(m_energyFull);
    reportFile.write(reportLine.toUtf8());
    reportLine = QString("<设计电量： %1wh>\n").arg(m_energyFullDesign);
    reportFile.write(reportLine.toUtf8());
    reportLine = QString("<电池健康： %1>\n").arg(m_batteryHealth);
    reportFile.write(reportLine.toUtf8());

    reportLine = QString("\n---------------<测试数据>---------------\n");
    reportFile.write(reportLine.toUtf8());

    int secondsOfEndurance = 0;
    for (int i = 0; i < m_batteryDateVector.size(); ++i) {
        reportLine = QString("<当前时间： %1>    <百分比： %2%>    <间隔时间： %3s>    <当前电量： %4wh>    <功率： w>\n")
                .arg(m_batteryDateVector[i]->getRecordDateTime().toString("hh:mm:ss"))
                .arg(m_batteryDateVector[i]->getBatteryPercentage())
                .arg(m_batteryDateVector[i]->getElapsedTime())
                .arg(m_batteryDateVector[i]->getBatteryEnergy())
                .arg(m_batteryDateVector[i]->getBatteryEnergyRate());
        reportFile.write(reportLine.toUtf8());
        secondsOfEndurance += m_batteryDateVector[i]->getElapsedTime();
    }
    reportLine = QString("<续航时间： %1 ： %2s>\n")
            .arg(secondsToTimeFormat(secondsOfEndurance))
            .arg(secondsOfEndurance);
    reportFile.write(reportLine.toUtf8());

    reportFile.close();
    m_hasNewReport = true;
    m_runTestButton->stopTest();
    QDBusInterface dbusInterface("org.freedesktop.login1",
                                 "/org/freedesktop/login1",
                                 "org.freedesktop.login1.Manager",
                                 QDBusConnection::systemBus());
    dbusInterface.call("PowerOff", true);
}

void UpttTestConfigPage::createEnergyRateTestReport()
{
    createReportPath();

    QString reportFileName = m_testReportDir
                           + QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss")
                           + QString(".report");

    QFile reportFile(reportFileName);

    if (!reportFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return ;
    }

    QString reportLine("---------------<测试配置>----------------\n");
    reportFile.write(reportLine.toUtf8());
    QDBusInterface dBusInterface(UPM_DBUS_SERVICE,
                                 UPM_DBUS_PATH,
                                 UPM_DBUS_INTERFACE,
                                 QDBusConnection::systemBus());
    QDBusReply<int> replyCpuCoreNum = dBusInterface.call("GetCpuCoreNum");

    for (int i = 0; i < replyCpuCoreNum.value(); ++i) {
        QDBusReply<QString> replyPolicy = dBusInterface.call("GetCpuCorePolicy", i);
        reportLine = QString("<cpu-%1-模式： %2>\n").arg(i).arg(replyPolicy.value().simplified());
        reportFile.write(reportLine.toUtf8());
    }
    QStringList policyList = {"performance", "balance", "powersave"};
    reportLine = QString("<工作模式： %1>\n").arg(policyList[m_upttCpuPolicyConfig->getPowerPolicyBattery()]);
    reportFile.write(reportLine.toUtf8());
    reportLine = QString("<屏幕亮度百分比： %1%>\n").arg(m_upttBrightnessConfig->getBrightnessPercentage());
    reportFile.write(reportLine.toUtf8());
    reportLine = QString("<平衡模式降低CPU频率： %1>\n").arg(m_balanceModeReduceFrequency->isChecked());
    reportFile.write(reportLine.toUtf8());
    reportLine = QString("<自动开关CPU核心： %1>\n").arg(m_autoSwitchCpuCore->isChecked());
    reportFile.write(reportLine.toUtf8());
    reportLine = QString("<系统空闲时仅保留cpu0： %1>\n").arg(m_idleReserveCpu0->isChecked());
    reportFile.write(reportLine.toUtf8());
    reportLine = QString("<系统空闲时降低屏幕亮度： %1>\n").arg(m_idleReduceBrightness->isChecked());
    reportFile.write(reportLine.toUtf8());
    if (true == m_idleReduceBrightness->isChecked()) {
        QDBusInterface dBusInterface(UPM_DBUS_SERVICE,
                                     UPM_DBUS_PATH,
                                     UPM_DBUS_INTERFACE,
                                     QDBusConnection::systemBus());
        QDBusReply<quint64> reply = dBusInterface.call("GetBrightness");
        m_stopBrightness = reply.value();
        reportLine = QString("<开始亮度： %1>\n").arg(m_startBrightness);
        reportFile.write(reportLine.toUtf8());
        reportLine = QString("<结束亮度： %1>\n").arg(m_stopBrightness);
        reportFile.write(reportLine.toUtf8());
    }
    reportLine = QString("<系统空闲时不关闭显示器： %1>\n").arg(m_idleTurnoffDisplay->isChecked());
    reportFile.write(reportLine.toUtf8());
    reportLine = QString("<系统空闲时不进入睡眠： %1>\n").arg(m_idleSystemSuspend->isChecked());
    reportFile.write(reportLine.toUtf8());
    reportLine = QString("<系统空闲时不开启屏幕保护： %1>\n").arg(m_disableScreenSaver->isChecked());
    reportFile.write(reportLine.toUtf8());
    reportLine = QString("<系统空闲时切换节能模式： %1>\n").arg(m_idlePowerPolicyAuto->isChecked());
    reportFile.write(reportLine.toUtf8());
    reportLine = QString("<低电量无操作： %1>\n").arg(m_criticalPercentageDoNothing->isChecked());
    reportFile.write(reportLine.toUtf8());

    reportLine = QString("\n---------------<测试数据>---------------\n");
    reportFile.write(reportLine.toUtf8());

    reportLine = QString("<开始时间： %1>    <开始时电池百分比： %2%>    <开始时电池电量： %3wh>\n")
            .arg(m_batteryDateVector[0]->getRecordDateTime().toString("hh:mm:ss"))
            .arg(m_batteryDateVector[0]->getBatteryPercentage())
            .arg(m_batteryDateVector[0]->getBatteryEnergy());
    reportFile.write(reportLine.toUtf8());
    reportLine = QString("<结束时间： %1>    <结束时电池百分比： %2%>    <结束时电池电量： %3wh>\n")
            .arg(m_batteryDateVector[1]->getRecordDateTime().toString("hh:mm:ss"))
            .arg(m_batteryDateVector[1]->getBatteryPercentage())
            .arg(m_batteryDateVector[1]->getBatteryEnergy());
    reportFile.write(reportLine.toUtf8());
    reportLine = QString("<放电功率： %1w>\n").arg(m_batteryDateVector[1]->getBatteryEnergyRate());
    reportFile.write(reportLine.toUtf8());

    reportFile.close();
    m_hasNewReport = true;
    m_runTestButton->stopTest();
}

void UpttTestConfigPage::clearBatteryDate()
{
    for (int i = 0; i < m_batteryDateVector.size(); ++i) {
        delete m_batteryDateVector.at(i);
    }
    m_batteryDateVector.clear();
}

QString UpttTestConfigPage::secondsToTimeFormat(int seconds)
{
    int hours = seconds / 3600; // 计算小时数
    int remainingSeconds = seconds % 3600; // 计算剩余秒数
    int minutes = remainingSeconds / 60; // 从剩余秒数中计算分钟数
    int secondsInMinutes = remainingSeconds % 60; // 剩余的秒数（不包括用于计算分钟的秒数）

    // 格式化输出，例如 "HH:MM:SS" 的格式
    return QString("%1:%2:%3").arg(hours).arg(minutes).arg(secondsInMinutes);
}

