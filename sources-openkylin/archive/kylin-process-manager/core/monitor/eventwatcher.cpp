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

#include "eventwatcher.h"
#include "energymeter/cpuenergymeter.h"
#include "eventbus.h"
#include "loghelper.h"
#include "systemstatehelper.h"
#include "systemenery/upowersystemenery.h"
#include "waylanddisplay.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>
#include <QObject>
#include <QRegularExpression>
#include <kwindowsystem.h>

namespace
{

const char *free_desktop_service = "org.freedesktop.DBus";
const char *free_desktop_path = "/org/freedesktop/DBus";
const char *free_desktop_interface = "org.freedesktop.DBus";

const char *mpris_service_prefix = "org.mpris.MediaPlayer2.";

const char *status_notifier_watcher_service = "org.kde.StatusNotifierWatcher";
const char *status_notifier_watcher_path = "/StatusNotifierWatcher";
const char *status_notifier_watcher_interface = "org.kde.StatusNotifierWatcher";

const char *optilauncher_service = "com.kylin.optilauncher.interface";
const char *optilauncher_path = "/";
const char *optilauncher_interface = "com.kylin.optilauncher.interface";

int getPidFromStatusNotifierItemServiceName(const QString &serviceName)
{
    QRegularExpression re("\\w+-\\d+-\\d+");
    QRegularExpressionMatch match = re.match(serviceName);
    if (!match.hasMatch()) {
        return 0;
    }

    QStringList nameItems = serviceName.split('-');
    return nameItems.at(1).toInt();
}

}

EventWatcher::EventWatcher(std::shared_ptr<DataContext> dataContext)
    : m_dataContext(dataContext)
    , m_systemEnergy(std::make_shared<UPowerSystemEnery>())
{
    // we should watch daemon signal all the time for watch resource changed.
    initProcessGroupConsumptionMonitor();
    initBatteryRemainingTimeEstimator();
    initDaemonInterfaceConnections();
    initRegisteredWatcherConnections();
    initBenchmarkConnections();
    qRegisterMetaType<uint32_t>("uint32_t");
    std::lock_guard<FairMutex>(m_dataContext->stateManager()->fairMutex(StateTarget::BenchmarkStateChanged));
    m_dataContext->bench_mark_state = static_cast<bool>(system_state_helper::isosinstaller());
    m_dataContext->stateManager()->enableTarget(StateTarget::BenchmarkStateChanged);
}

EventWatcher::~EventWatcher()
{
    PRINT_NOTICE("~EventWatcher\n");
    stopScreenWatcher();
    for (auto &conn : m_defaultConns) {
        QObject::disconnect(conn);
        conn = {};
    }
    m_defaultConns.clear();
    stopBatteryEstimate();
    QDBusConnection::sessionBus().disconnect(
        optilauncher_service, optilauncher_path,
        optilauncher_interface, "BenchmarkStarted",
        this, SLOT(onBenchmarkStarted()));
    QDBusConnection::sessionBus().disconnect(
        optilauncher_service, optilauncher_path,
        optilauncher_interface, "BenchmarkStopped",
        this, SLOT(onBenchmarkStopped()));
    stopWindowWatcher();
    stopFreezerWatcher();
    stopWatcher();
}

void EventWatcher::startWatcher()
{
    m_dataContext->powerManager()->startPowerWatcher();
    m_dataContext->deviceModeManager()->startDeviceModeWatcher();
    initMprisDbusConnections();
    initPowerDetect();
}

void EventWatcher::stopWatcher()
{
    m_dataContext->powerManager()->stopPowerWatcher();
    m_dataContext->deviceModeManager()->stopDeviceModeWatcher();
    QDBusConnection::sessionBus().disconnect(
        free_desktop_service, free_desktop_path, free_desktop_interface,
        "NameOwnerChanged", this, SLOT(onMprisDbusNameAcquired(QString, QString, QString)));
    m_powerDetect.stop();
    QObject::disconnect(m_powerConn);
    m_powerConn = {};
}

void EventWatcher::startFreezerWatcher()
{
    m_dataContext->systemStateManager()->startFreezeWatcher();
    m_dataContext->deviceModeManager()->startDPMSModeWatcher();
}

void EventWatcher::stopFreezerWatcher()
{
    m_dataContext->systemStateManager()->stopFreezeWatcher();
    m_dataContext->deviceModeManager()->stopDPMSModeWatcher();
}

void EventWatcher::startWindowWatcher()
{
    initWindowChangedConnections();
    handleExistedWindows();
    initStatusNotifierWatcherConnections();
}

void EventWatcher::stopWindowWatcher()
{
    for (auto &conn : m_conns) {
        disconnect(conn);
        conn = {};
    }
    m_conns.clear();
    QDBusConnection::sessionBus().disconnect(
        status_notifier_watcher_service, status_notifier_watcher_path, status_notifier_watcher_interface,
        "StatusNotifierItemRegistered", this, SLOT(onStatusNotifierItemRegistered(QString)));
}

void EventWatcher::startScreenWatcher()
{
    m_dataContext->systemStateManager()->startScreenWatcher();
}

void EventWatcher::stopScreenWatcher()
{
    m_dataContext->systemStateManager()->stopScreenWatcher();
}

void EventWatcher::startBatteryEstimate()
{
    m_battertRemainingTimeEstimator->startEstimate();
}

void EventWatcher::stopBatteryEstimate()
{
    m_battertRemainingTimeEstimator->stopEstimate();
}

void EventWatcher::onStatusNotifierItemRegistered(QString itemName)
{
    int pid = getPidFromStatusNotifierItemServiceName(itemName);
    // qDebug() << "Status notifier item registered: " << pid;
    PRINT_DBG("Status notifier item registered: %d\n", pid);
    m_dataContext->processInfoManager()->handleStatusNotifierItemRegistered(pid);
}

void EventWatcher::onMprisDbusNameAcquired(QString name, QString newOwner, QString oldOwner)
{
    Q_UNUSED(newOwner)
    Q_UNUSED(oldOwner)

    if (!name.contains(mpris_service_prefix)) {
        return;
    }
    QDBusInterface mediaInterface(
        free_desktop_service, free_desktop_path,
        free_desktop_service, QDBusConnection::sessionBus());

    QDBusPendingReply<quint32> pidReply =
        mediaInterface.call(QStringLiteral("GetConnectionUnixProcessID"), name);
    quint32 pid = pidReply.value();
    if (pid > 0) {
        m_dataContext->processInfoManager()->handleMprisDbusAdded((int)pid, name.toStdString());
    }
}

void EventWatcher::onBenchmarkStarted()
{
    std::lock_guard<FairMutex>(m_dataContext->stateManager()->fairMutex(StateTarget::BenchmarkStateChanged));
    m_dataContext->bench_mark_state = true;
    m_dataContext->stateManager()->enableTarget(StateTarget::BenchmarkStateChanged);
}

void EventWatcher::onBenchmarkStopped()
{
    std::lock_guard<FairMutex>(m_dataContext->stateManager()->fairMutex(StateTarget::BenchmarkStateChanged));
    m_dataContext->bench_mark_state = false;
    m_dataContext->stateManager()->enableTarget(StateTarget::BenchmarkStateChanged);
}

void EventWatcher::initBenchmarkConnections()
{
    QDBusConnection::sessionBus().connect(
        optilauncher_service, optilauncher_path,
        optilauncher_interface, "BenchmarkStarted",
        this, SLOT(onBenchmarkStarted()));
    QDBusConnection::sessionBus().connect(
        optilauncher_service, optilauncher_path,
        optilauncher_interface, "BenchmarkStopped",
        this, SLOT(onBenchmarkStopped()));
}

void EventWatcher::initWindowChangedConnections()
{
    m_conns.emplace_back(connect(waylandDisplay(), &WaylandDisplay::windowAdded, this, [this](std::string uuid) {
        m_dataContext->processInfoManager()->handleWindowAdded(uuid);
    }));

    m_conns.emplace_back(connect(waylandDisplay(), &WaylandDisplay::windowRemoved, this, [this](std::string uuid) {
        m_dataContext->processInfoManager()->handleWindowRemoved(uuid);
    }));

    m_conns.emplace_back(connect(waylandDisplay(), &WaylandDisplay::windowStateChanged, this, [this](std::string uuid, uint32_t state) {
        handleWinInfo(uuid, state);
    }));
}

void EventWatcher::initRegisteredWatcherConnections()
{
    m_defaultConns.emplace_back(connect(EventBus::instance(), &EventBus::registerAppWinChangedSpellWatcher,
                                        [this](const std::string &uuid, const RegistrAppInfo &process, int spell) {
                                            m_dataContext->processInfoManager()->registerAppWinChangedSpellWatcher(uuid, process, spell);
                                        }));

    m_defaultConns.emplace_back(connect(EventBus::instance(), &EventBus::unregisterAppWinChangedSpellWatcher,
                                        [this](const std::string &uuid) {
                                            m_dataContext->processInfoManager()->unregisterAppWinChangedSpellWatcher(uuid);
                                        }));

    m_defaultConns.emplace_back(connect(EventBus::instance(), &EventBus::registerAppFocusSpellWatcher,
                                        [this](const std::string &uuid, const RegistrProcessInfo &process, int spell) {
                                            m_dataContext->processInfoManager()->registerAppFocusSpellWatcher(uuid, process, spell);
                                        }));

    m_defaultConns.emplace_back(connect(EventBus::instance(), &EventBus::unregisterAppFocusSpellWatcher,
                                        [this](const std::string &uuid) {
                                            m_dataContext->processInfoManager()->unregisterAppFocusSpellWatcher(uuid);
                                        }));

    m_defaultConns.emplace_back(connect(EventBus::instance(), &EventBus::addProcessGroup, [this](const std::string &id, const std::vector<int> &pids) {
        m_consumptionMonitor->addProcessGroup(id, pids, [this](const std::string &appId) {
            AppInfo latestAppInfo = m_dataContext->processInfoManager()->getAppInfo(appId);
            return latestAppInfo.pids();
        });
    }));
}

void EventWatcher::initPowerDetect()
{
    m_powerDetect.setInterval(m_dataContext->configManager()->powerlevelChangedDetectInterval() * 1000);
    connect(&m_powerDetect, &QTimer::timeout, [this](){
        m_dataContext->stateManager()->enableTarget(StateTarget::PowerLevelChanged);
    });
    m_powerDetect.start();
    m_dataContext->stateManager()->enableTarget(StateTarget::PowerLevelChanged);
}

void EventWatcher::initMprisDbusConnections()
{
    QDBusConnection::sessionBus().connect(
        free_desktop_service, free_desktop_path, free_desktop_interface,
        "NameOwnerChanged", this, SLOT(onMprisDbusNameAcquired(QString, QString, QString)));
}

void EventWatcher::initStatusNotifierWatcherConnections()
{
    QDBusConnection::sessionBus().connect(
        status_notifier_watcher_service, status_notifier_watcher_path, status_notifier_watcher_interface,
        "StatusNotifierItemRegistered", this, SLOT(onStatusNotifierItemRegistered(QString)));
}

void EventWatcher::initDaemonInterfaceConnections()
{
    m_defaultConns.emplace_back(connect(&m_daemonInterface, &DaemonDbusInterface::ResourceThresholdWarning,
                                        m_dataContext->systemStateManager().get(), &SystemStateManager::resourceWarning));
}

void EventWatcher::initProcessGroupConsumptionMonitor()
{
    std::shared_ptr<EnergyMeter> cpuEnergyMeter(new CpuEnergyMeter());
    m_consumptionMonitor.reset(
        new ProcessGroupConsumptionMonitor(m_systemEnergy, cpuEnergyMeter, 60, 10));
}

void EventWatcher::initBatteryRemainingTimeEstimator()
{
    m_battertRemainingTimeEstimator.reset(
        new BatteryRemainingTimeEstimator(
            m_systemEnergy, m_consumptionMonitor, 60 * 10,
            [this]() {
        auto appInfos = m_dataContext->processInfoManager()->getAppInfosWithState(sched_policy::AppState::Cached);
        std::vector<std::string> appIds;
        for (const auto &appInfo : appInfos) {
            appIds.emplace_back(appInfo.desktopFilePid());
        }
        return appIds;
    }));

    m_battertRemainingTimeEstimator->setIncreasedRemainTimeChangedCallback([this](int time) {
        // qDebug() << "Remain time: " << time;
        PRINT_DBG("Remain time: %d\n", time);
        m_dataContext->configManager()->setIncreasedBatteryRemainingTime(time);
    });
}

void EventWatcher::handleExistedWindows()
{
    const auto windows = waylandDisplay()->getWindows();
    for (auto &window : windows) {
        handleWinInfo(window.first, window.second->state);
    }
}

void EventWatcher::handleWinInfo(const std::string &uuid, uint32_t state)
{
    if (state & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_ACTIVE) {
        m_dataContext->processInfoManager()->handleWindowActive(uuid);
    }
    m_dataContext->processInfoManager()->setWindowMinimized(uuid, (state & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_MINIMIZED));
    // m_dataContext->processInfoManager()->setWindowKeepAbove(uuid, (state & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_KEEP_ABOVE));
    m_dataContext->processInfoManager()->setWindowMaximized(uuid, (state & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_MAXIMIZED));
    m_dataContext->processInfoManager()->setWindowFullscreen(uuid, (state & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_FULLSCREEN));
}