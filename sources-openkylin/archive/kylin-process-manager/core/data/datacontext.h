#ifndef DATACONTEXT_H
#define DATACONTEXT_H

#include "configmanager.h"
#include "devicestate.h"
#include "processinfomanager.h"
#include "groupmanager.h"
#include "systemstatemanager.h"
#include "applaunchmanager.h"

#include "statemanager.h"
#include "staticscenedata.h"
#include "policyruleparser.h"


class DataContext
{
public:
    DataContext()
        : m_stateManager(std::make_shared<StateManager>())
        , m_configManager(std::make_shared<ConfigManager>(m_stateManager))
        , m_powerManager(std::make_shared<PowerManager>(m_stateManager))
        , m_deviceModeManager(std::make_shared<DeviceModeManager>(m_configManager->softFreezeModeEnabled(), m_stateManager))
        , m_processInfoManager(std::make_shared<ProcessInfoManager>(m_configManager, m_stateManager))
        , m_appLaunchManager(std::make_shared<AppLaunchManager>(*m_processInfoManager))
        , m_groupManager(std::make_shared<GroupManager>(m_configManager->policyConfigInfo()))
        , m_systemStateManager(std::make_shared<SystemStateManager>(m_stateManager))
        , bench_mark_state(false)
    {
        register_group_manager(m_groupManager);
        register_config_manager(m_configManager);
        register_systemstate_manager(m_systemStateManager);
    }

    ~DataContext() = default;

    std::shared_ptr<ConfigManager> configManager() { return m_configManager; }
    std::shared_ptr<PowerManager> powerManager() { return m_powerManager; }
    std::shared_ptr<DeviceModeManager> deviceModeManager() { return m_deviceModeManager; }
    std::shared_ptr<ProcessInfoManager> processInfoManager() { return m_processInfoManager; }
    std::shared_ptr<AppLaunchManager> appLaunchManager() { return m_appLaunchManager; }
    std::shared_ptr<GroupManager> groupManager() { return m_groupManager; }
    std::shared_ptr<SystemStateManager> systemStateManager() { return m_systemStateManager; }

    std::shared_ptr<StateManager> stateManager() { return m_stateManager; }

    StaticSceneData clipStaticSceneData();

    bool bench_mark_state;

private:
    // 状态变化集
    std::shared_ptr<StateManager> m_stateManager;

    std::shared_ptr<ConfigManager> m_configManager;
    std::shared_ptr<PowerManager> m_powerManager;
    std::shared_ptr<DeviceModeManager> m_deviceModeManager;
    std::shared_ptr<ProcessInfoManager> m_processInfoManager;
    std::shared_ptr<AppLaunchManager> m_appLaunchManager;
    std::shared_ptr<GroupManager> m_groupManager;
    std::shared_ptr<SystemStateManager> m_systemStateManager;

    StaticSceneData m_staticSceneData;
};


#endif // DATACONTEXT_H