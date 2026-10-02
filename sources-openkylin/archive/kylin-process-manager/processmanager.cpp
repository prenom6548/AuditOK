#include "processmanager.h"

#include "eventbus.h"
#include "loghelper.h"

ProcessManager::ProcessManager()
    : m_dataContext(std::make_shared<DataContext>())
    , m_eventWatcher(new EventWatcher(m_dataContext))
    , m_exceptionGroupWatcher(new ExceptionGroupProcessWatcher(m_dataContext->processInfoManager()))
    , m_sceneManager(new SceneManager(m_dataContext))
    , app_whitelist(new AppWhitelist(*m_dataContext->configManager()))
    , app_whitelist_service(new AppWhitelistService(app_whitelist.get()))
    , app_launch_manager(m_dataContext->appLaunchManager())
    , app_launch_manager_service(new AppLaunchManagerService(app_launch_manager.get()))
{
    initWatcher();

    connectAppLauncherSignals();
}

ProcessManager::~ProcessManager()
{
    PRINT_NOTICE("~ProcessManager\n");
    for (auto &conn : m_conns) {
        QObject::disconnect(conn);
        conn = {};
    }
    m_conns.clear();
}

void ProcessManager::ThawProcess(int pid)
{
    PRINT_NOTICE("ThawProcess, pid: %d\n", pid);
    if (canFrozenProcess()) {
        bool ret = m_dataContext->processInfoManager()->forceChangeAppStateByPid(pid, sched_policy::AppState::Focus);
        if (!ret) {
            sendErrorReply(QDBusError::InvalidArgs,
                           QString("Can not find the application by the pid %1.").arg(pid));
        }
    }

    m_cgroupControlInterface.thawedProcess(pid);
}

void ProcessManager::ThawFrozenProcesses()
{
    PRINT_NOTICE("ThawFrozenProcesses\n");
    if (canFrozenProcess()) {
        m_dataContext->processInfoManager()->forceChangCachedStateTo(sched_policy::AppState::Background);
    }

    m_cgroupControlInterface.setFreezeEnable(false);
}

void ProcessManager::initWatcher()
{
    m_conns.emplace_back(QObject::connect(EventBus::instance(), &EventBus::startWatcher, m_exceptionGroupWatcher.get(), &ExceptionGroupProcessWatcher::startWatcher));
    m_conns.emplace_back(QObject::connect(EventBus::instance(), &EventBus::startWatcher, m_eventWatcher.get(), &EventWatcher::startWatcher));
    m_conns.emplace_back(QObject::connect(EventBus::instance(), &EventBus::startFreezerWatcher, m_eventWatcher.get(), &EventWatcher::startFreezerWatcher));
    m_conns.emplace_back(QObject::connect(EventBus::instance(), &EventBus::startWindowWatcher, m_eventWatcher.get(), &EventWatcher::startWindowWatcher));
    m_conns.emplace_back(QObject::connect(EventBus::instance(), &EventBus::startScreenWatcher, m_eventWatcher.get(), &EventWatcher::startScreenWatcher));
    m_conns.emplace_back(QObject::connect(EventBus::instance(), &EventBus::startBatteryEstimate, m_eventWatcher.get(), &EventWatcher::startBatteryEstimate));

    m_conns.emplace_back(QObject::connect(EventBus::instance(), &EventBus::stopWatcher, m_exceptionGroupWatcher.get(), &ExceptionGroupProcessWatcher::stopWatcher));
    m_conns.emplace_back(QObject::connect(EventBus::instance(), &EventBus::stopWatcher, m_eventWatcher.get(), &EventWatcher::stopWatcher));
    m_conns.emplace_back(QObject::connect(EventBus::instance(), &EventBus::stopFreezerWatcher, m_eventWatcher.get(), &EventWatcher::stopFreezerWatcher));
    m_conns.emplace_back(QObject::connect(EventBus::instance(), &EventBus::stopWindowWatcher, m_eventWatcher.get(), &EventWatcher::stopWindowWatcher));
    m_conns.emplace_back(QObject::connect(EventBus::instance(), &EventBus::stopScreenWatcher, m_eventWatcher.get(), &EventWatcher::stopScreenWatcher));
    m_conns.emplace_back(QObject::connect(EventBus::instance(), &EventBus::stopBatteryEstimate, m_eventWatcher.get(), &EventWatcher::stopBatteryEstimate));
}

void ProcessManager::connectAppLauncherSignals()
{
    m_conns.emplace_back(QObject::connect(
        app_launch_manager.get(), &AppLaunchManager::AppLaunched,
        app_launch_manager_service.get(), &AppLaunchManagerService::AppLaunched));
}

bool ProcessManager::canFrozenProcess()
{
    if (!m_dataContext->configManager()->reousrceLimitEnabled()) {
        return false;
    }

    if (m_dataContext->deviceModeManager()->currentDeviceMode() == sched_policy::DeviceMode::PC && !m_dataContext->configManager()->softFreezeModeEnabled()) {
        return false;
    }

    return true;
}
