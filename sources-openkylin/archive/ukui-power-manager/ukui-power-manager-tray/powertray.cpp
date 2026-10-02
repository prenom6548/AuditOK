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

#include <QDebug>

#include "powertray.h"
#include "powerwindow.h"
#include "batteryiconhelper.h"
#include "kwindowsystem.h"
#include "dmi_chassis_type.h"

PowerTray::PowerTray(QObject *parent) : QObject(parent)
{
    if (!UpmUpowerDBus::self()->hasBattery() && !UpmUpowerDBus::self()->getAcOnlineState()) {
        qDebug() << "No battery and no AC adapter, Program exit";
        exit(0);
    }
    
    if (isDesktop()) {
        qDebug() << "This is a desktop, Program exit";
        exit(0);
    }

    initPowerTray();
    QDBusConnection::systemBus().connect("org.freedesktop.login1",
                                         "/org/freedesktop/login1",
                                         "org.freedesktop.login1.Manager",
                                         "PrepareForSleep",
                                         this,
                                         SLOT(dealWakeUpSignal(bool)));
}

PowerTray::~PowerTray()
{
    if (nullptr != m_TrayMenu) {
        delete m_TrayMenu;
        m_TrayMenu = nullptr;
    }

    if (nullptr != m_powerWindow) {
        delete m_powerWindow;
        m_powerWindow = nullptr;
    }
}

void PowerTray::initPowerTray()
{
    QVariant value;
    if (true == UpmGsettings::self()->getGsettingsConfig(GSETTINGS_KEY_DISPLAY_LEFT_TIME, value)) {
        m_showBatteryLeftTime = value.toInt();
    }
    connect(UpmGsettings::self(), &UpmGsettings::displayLeftTime,
            this, [=](QVariant value) {
        m_showBatteryLeftTime = value.toInt();
        updateBatteryInfo();
        setTrayIconToolTipForBatteryInfoChanged(m_batteryPercentage);
        setPowerWindowProperties();
        m_powerWindow->setPowerStateLabelForBatteryInfoChanged();
    });
    // 用户调整低电量阈值后，立即刷新托盘与电源窗口图标颜色
    connect(UpmGsettings::self(), &UpmGsettings::percentageLowChanged,
            this, [=](QVariant value) {
        qDebug() << "[BatteryIconColor] percentageLowChanged ->" << value.toInt();
        updateBatteryInfo();
        m_trayIconName = UpmUpowerDBus::self()->getBatteryIconName();
        setTrayIcon(m_trayIconName);
        setPowerWindowProperties();
    });

    if (true == UpmGsettings::self()->getGsettingsConfig(GSETTINGS_KEY_POWER_POLICY_BATTERY, value)) {
        m_oldPolicy = value.toInt();
    }

    m_acChangeTimer = new QTimer(this);
    m_isPowerSupplyChange = false;
    connect(m_acChangeTimer, &QTimer::timeout, this, [=](){
        m_isPowerSupplyChange = false;
        m_acChangeTimer->stop();
    });

    updateBatteryInfo();
    m_upowerInterface = new QDBusInterface(UPOWER_SERVICE,
                                 UPOWER_DISPLAY_PATH,
                                 UPOWER_DISPLAY_INTERFACE,
                                 QDBusConnection::sessionBus());
    if (!m_upowerInterface->isValid()) {
        qWarning() << "Failed to connect to UPower D-Bus interface.";
    }
    m_trayIcon = new QSystemTrayIcon(this);
    setTrayIconToolTipForAcOnlineChanged(m_acOnlineState, m_batteryPercentage);
    m_trayIconName = UpmUpowerDBus::self()->getBatteryIconName();
    setTrayIcon(m_trayIconName);

    initTrayMenuUI();
    initPowerWindowUI();

    connect(m_trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(onActivatedIcon(QSystemTrayIcon::ActivationReason)));

    connect(UpmUpowerDBus::self(), &UpmUpowerDBus::acOnlineStateChanged,
            this, &PowerTray::dealAcOnlineStateChanged);
    connect(UpmUpowerDBus::self(), &UpmUpowerDBus::batteryInfoChanged,
            this, &PowerTray::dealBatteryInfoChanged);

    connect(UpmGsettings::self(), &UpmGsettings::powerPolicyBatteryChanged,
            this, &PowerTray::dealPowerPolicyBatteryChanged);

    connect(UpmUpowerDBus::self(), &UpmUpowerDBus::deviceChanged,
            this, &PowerTray::dealDeviceChanged);
}

void PowerTray::initTrayMenuUI()
{
    qDebug() << "init tray menu ui";
    m_TrayMenu = new QMenu();
    m_TrayMenu->setAttribute(Qt::WA_TranslucentBackground);
    m_trayIcon->setContextMenu(m_TrayMenu);
    QAction *setPreference = new QAction(m_TrayMenu);
    QIcon icon = QIcon::fromTheme("system-settings-symbolic");
    setPreference->setIcon(icon);
    setPreference->setText(tr("SetPower"));
    connect(setPreference, &QAction::triggered,[=]{QProcess::startDetached(QString("ukui-control-center -m Power"));});
    m_TrayMenu->addAction(setPreference);
}

void PowerTray::initPowerWindowUI()
{
    qDebug() << "init power window ui";
    m_powerWindow = new powerwindow();
    m_powerWindow->hide();

    setPowerWindowProperties();
    m_powerWindow->setPowerStateLabelForAcOnlineChanged();
}

void PowerTray::setPowerWindowProperties()
{
    m_powerWindow->setAcOnlineState(m_acOnlineState);
    m_powerWindow->setBatteryState(m_batteryState);
    m_powerWindow->setBatteryPercentage(m_batteryPercentage);
    m_powerWindow->setIconName(m_trayIconName);
    m_powerWindow->setShowPowerLeftTime(m_showBatteryLeftTime);
    m_powerWindow->setBatteryLeftTime(m_timeToFull, m_timeToEmpty);
}

void PowerTray::setTrayIconToolTipForAcOnlineChanged(bool acOnlineState, int percentage)
{
    //添加无电池情况的处理
    if (!UpmUpowerDBus::self()->hasBattery()) {
        if (m_acOnlineState) {
            applyTooltipIfChanged(tr("No battery detected"));
        }
        return;
    }

    QString tip;
    if (true == acOnlineState) {
        if (100 == percentage) {
            tip = tr("Fully Charged (100%)");
        } else {
            tip = tr("%1% available (plugged in)").arg(percentage);
        }
    } else {
        tip = tr("%1% remaining").arg(percentage);
    }
    applyTooltipIfChanged(tip);
}

void PowerTray::setTrayIconToolTipForBatteryInfoChanged(int percentage)
{
    //添加无电池情况的处理
    if (!UpmUpowerDBus::self()->hasBattery()) {
        if (m_acOnlineState) {
            applyTooltipIfChanged(tr("No battery detected"));
        }
        return;
    }

    qint64 batteryLeftTime = 0;
    int hour = 0;
    QString tip;

    switch (UpmUpowerDBus::self()->getBatteryState()) {
        case battery_state_unknown:
        case battery_state_fully:
            if (100 == percentage) {
                tip = tr("Fully Charged (100%)");
            } else {
                tip = tr("%1% remaining").arg(percentage);
            }
            break;

        case battery_state_charging:
            batteryLeftTime = m_timeToFull;
            if (0 == m_showBatteryLeftTime || 0 == batteryLeftTime || percentage > 95 ) {
                if (100 == percentage) {
                    tip = tr("Fully Charged (100%)");
                } else {
                    tip = tr("%1% available (plugged in)").arg(percentage);
                }
            } else {
                hour = batteryLeftTime / 3600;
                if (0 == hour) {
                    tip = tr("%1 min to fully charge (%2%)")
                                           .arg(((batteryLeftTime) % 3600) / 60)
                                           .arg(percentage);
                } else if (hour > 10) {
                    tip = tr("%1% available (plugged in)").arg(percentage);
                } else {
                    tip = tr("%1 hr %2 min to fully charge (%3%)")
                                           .arg(hour)
                                           .arg(((batteryLeftTime) % 3600) / 60)
                                           .arg(percentage);
                }
            }
            break;

        case battery_state_discharging:
            batteryLeftTime = m_timeToEmpty;
            if (0 == m_showBatteryLeftTime || 0 == batteryLeftTime) {
                tip = tr("%1% remaining").arg(percentage);
            } else {
                hour = batteryLeftTime / 3600;
                if (0 == hour) {
                    tip = tr("%1 min (%2%) remaining")
                                           .arg(((batteryLeftTime) % 3600) / 60)
                                           .arg(percentage);
                } else if (hour > 20) {
                    tip = tr("%1% remaining").arg(percentage);
                } else {
                    tip = tr("%1 hr %2 min (%3%) remaining")
                                           .arg(hour)
                                           .arg(((batteryLeftTime) % 3600) / 60)
                                           .arg(percentage);
                }
            }
            break;

        default:
            tip = tr("%1% remaining").arg(percentage);
            break;
    }
    applyTooltipIfChanged(tip);
}

void PowerTray::setTrayIcon(QString strIconName)
{
    if (!strIconName.isNull()) {
        QIcon icon = buildBatteryLevelIcon(strIconName, m_batteryPercentage,
                                           QStringLiteral("(tray)"));
        m_trayIcon->setIcon(icon);
        m_trayIcon->show();
        QDBusReply<bool> reply = m_upowerInterface->call("SetShowIcon",true);
    } else {
        m_trayIcon->hide();
        QDBusReply<bool> reply = m_upowerInterface->call("SetShowIcon",false);
    }
}

void PowerTray::onActivatedIcon(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == 3) {
        bool isVisible = m_powerWindow->isVisible();
        qDebug() << "power window is visible:" << isVisible;
        if (true == isVisible) {
            m_powerWindow->hide();
        } else {
            m_powerWindow->showPowerWindow();
        }
    } else {
        qDebug() << "on Activated Icon reason:" << reason;
    }
}

void PowerTray::acOnlineStateChangedAction()
{
    updateBatteryInfo();

    //deal tray icon
    setTrayIconToolTipForAcOnlineChanged(m_acOnlineState, m_batteryPercentage);
    m_trayIconName = UpmUpowerDBus::self()->getBatteryIconName();
    setTrayIcon(m_trayIconName);

    //deal power window
    setPowerWindowProperties();
    m_powerWindow->setPowerStateLabelForAcOnlineChanged();
}

void PowerTray::dealAcOnlineStateChanged(int index, bool value)
{
    qDebug() << "power tray ac online state changed";
    m_isPowerSupplyChange = true;
    m_acChangeTimer->start(120 * 1000);

    acOnlineStateChangedAction();
    if (false == m_acOnlineState) {
        qInfo() << "unplug power";
    }
}

void PowerTray::dealBatteryInfoChanged(int index, QStringList batteryInfoChangedList)
{
    qDebug() << "power tray battery info changed";
    if (true == m_isPowerSupplyChange) {
        qDebug() << "ac online changed, not deal battery info changed";
        acOnlineStateChangedAction();
        return ;
    }

    updateBatteryInfo();
    if (false == m_acOnlineState && m_batteryPercentage < 2) {
        qInfo() << "battery remaining 1%";
    }

    //deal tray icon
    setTrayIconToolTipForBatteryInfoChanged(m_batteryPercentage);

    m_trayIconName = UpmUpowerDBus::self()->getBatteryIconName();
    setTrayIcon(m_trayIconName);

    //deal power window
    setPowerWindowProperties();
    m_powerWindow->setPowerStateLabelForBatteryInfoChanged();
}

void PowerTray::dealPowerPolicyBatteryChanged(QVariant value)
{
    if (true == m_isPowerSupplyChange) {
        return;
    }

    if (false == UpmUpowerDBus::self()->getAcOnlineState()) {
        m_policyChanged = true;

        QVector<double> timeCoefficientVector;
        timeCoefficientVector << 0.03 << 0.04 << 0.05 << 0.06;
        double timeCoefficient = 0.07;
        int timeCoefficientIndex = m_batteryPercentage / 10;

        if (timeCoefficientIndex < timeCoefficientVector.size()) {
            timeCoefficient = timeCoefficientVector[timeCoefficientIndex];
        }

        int newPolicy = value.toInt();
        if (1 == m_oldPolicy - newPolicy) {
            m_timeToEmpty -= (m_timeToEmpty * timeCoefficient);
        } else if (2 == m_oldPolicy - newPolicy) {
            m_timeToEmpty -= (m_timeToEmpty * timeCoefficient * 2);
        } else if (-1 == m_oldPolicy - newPolicy) {
            m_timeToEmpty += (m_timeToEmpty * timeCoefficient);
        } else if (-2 == m_oldPolicy - newPolicy) {
            m_timeToEmpty += (m_timeToEmpty * timeCoefficient * 2);
        } else {
            ;
        }
        m_oldPolicy = newPolicy;

        //deal tray icon
        setTrayIconToolTipForBatteryInfoChanged(m_batteryPercentage);

        QString trayIconName = UpmUpowerDBus::self()->getBatteryIconName();
        if (m_trayIconName != trayIconName) {
            m_trayIconName = trayIconName;
            setTrayIcon(m_trayIconName);
        }

        //deal power window
        setPowerWindowProperties();
        m_powerWindow->setPowerStateLabelForBatteryInfoChanged();
        m_policyChanged = false;
    }
}

void PowerTray::dealWakeUpSignal(bool isSleepState)
{
    if (true == isSleepState) {
        qDebug() << "The computer is going to sleep ";
    } else {
        qDebug() << "The computer wakes up ";
        //机器刚从休眠或睡眠中唤醒时，此时不显示时间
        m_isPowerSupplyChange = true;
        m_acChangeTimer->start(60 * 1000);
        acOnlineStateChangedAction();
    }
}

void PowerTray::dealDeviceChanged()
{
    qDebug() << "Device changed, refresh battery info";
    updateBatteryInfo();

    //deal tray icon
    setTrayIconToolTipForBatteryInfoChanged(m_batteryPercentage);
    m_trayIconName = UpmUpowerDBus::self()->getBatteryIconName();
    setTrayIcon(m_trayIconName);

    //deal power window
    setPowerWindowProperties();
    m_powerWindow->setPowerStateLabelForBatteryInfoChanged();
}


void PowerTray::updateBatteryInfo()
{
    m_acOnlineState = UpmUpowerDBus::self()->getAcOnlineState();
    m_batteryState = UpmUpowerDBus::self()->getBatteryState();
    m_batteryPercentage = UpmUpowerDBus::self()->getBatteryPercentage();
    m_timeToFull = UpmUpowerDBus::self()->getTimeToFull();
    if (false == m_policyChanged) {
        m_timeToEmpty = UpmUpowerDBus::self()->getTimeToEmpty();
    }
}

bool PowerTray::isDesktop()
{
    QFile file("/sys/class/dmi/id/chassis_type");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open chassis_type file";
        return false;
    }

    QString chassisTypeStr = file.readAll().trimmed();
    file.close();

    bool ok = false;
    int chassisTypeInt = chassisTypeStr.toInt(&ok);
    if (!ok) {
        qWarning() << "Invalid chassis_type format:" << chassisTypeStr;
        return false;
    }

    if (static_cast<DmiChassisType>(chassisTypeInt) == DmiChassisType::Desktop) {
        return true; // 台式机
    }
    return false; // 非台式机
}

void PowerTray::applyTooltipIfChanged(const QString& tip)
{
    if (tip == m_lastTooltip) {
        return; // 无变化，直接返回
    }
    m_lastTooltip = tip;
    if (m_trayIcon) m_trayIcon->setToolTip(tip);
    if (m_upowerInterface && m_upowerInterface->isValid()) {
        m_upowerInterface->call("SetBatteryTooltip", tip);
    }
}