/*
 * Copyright 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef EVENTWATCHER_H
#define EVENTWATCHER_H

#include <memory>
#include <functional>

#include <QObject>

#include "schedpolicy.h"
#include "datacontext.h"
#include "daemondbusinterface.h"
#include "batteryremainingtimeestimator/batteryremainingtimeestimator.h"
#include "consumptionmonitor/processgroupconsumptionmonitor.h"


class EventWatcher : public QObject
{
    Q_OBJECT

public:
    EventWatcher(std::shared_ptr<DataContext> dataContext);
    ~EventWatcher();

public Q_SLOTS:
    void startWatcher();
    void stopWatcher();

    void startFreezerWatcher();
    void stopFreezerWatcher();

    void startWindowWatcher();
    void stopWindowWatcher();
    
    void startScreenWatcher();
    void stopScreenWatcher();

    void startBatteryEstimate();
    void stopBatteryEstimate();

private Q_SLOTS:
    void onMprisDbusNameAcquired(QString name, QString newOwner, QString oldOwner);
    void onStatusNotifierItemRegistered(QString itemName);
    void onBenchmarkStarted();
    void onBenchmarkStopped();

private:
    void initBenchmarkConnections();
    void initWindowChangedConnections();
    void initMprisDbusConnections();
    void initStatusNotifierWatcherConnections();
    void initDaemonInterfaceConnections();
    void initProcessGroupConsumptionMonitor();
    void initBatteryRemainingTimeEstimator();
    void initRegisteredWatcherConnections();
    void initPowerDetect();

    void handleExistedWindows();
    void handleWinInfo(const std::string &uuid, uint32_t state);
    void handleBenchmarkStateChanged(bool state);

private:
    std::shared_ptr<DataContext> m_dataContext;

    std::shared_ptr<ProcessGroupConsumptionMonitor> m_consumptionMonitor;
    std::shared_ptr<SystemEnergy> m_systemEnergy;
    std::unique_ptr<BatteryRemainingTimeEstimator> m_battertRemainingTimeEstimator;
    DaemonDbusInterface m_daemonInterface;

    QTimer m_powerDetect;

    std::vector<QMetaObject::Connection> m_conns;
    std::vector<QMetaObject::Connection> m_defaultConns;
    QMetaObject::Connection m_powerConn;
};

#endif // EVENTWATCHER_H
