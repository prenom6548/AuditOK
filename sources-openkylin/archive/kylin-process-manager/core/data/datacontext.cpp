#include "datacontext.h"
#include "loghelper.h"

StaticSceneData DataContext::clipStaticSceneData()
{
    m_staticSceneData.app_win_changed_spell.clear();
    m_staticSceneData.app_focus_spell.clear();
    m_staticSceneData.is_init = m_stateManager->isHaveStateTarget(StateTarget::Init);
    m_staticSceneData.bench_mark_state = bench_mark_state;

    // ConfigManager
    if (m_stateManager->isHaveStateTarget(StateTarget::ResourceLimit))
    {
        m_staticSceneData.resource_limit_enabled = m_configManager->reousrceLimitEnabled();
        m_staticSceneData.pre_resource_limit_enabled = m_configManager->preResourceLimitEnabled();
    }

    bool isGroupInit = m_groupManager->isInit();
    m_staticSceneData.is_resLimitInit = false;
    if (!isGroupInit) {
        m_groupManager->initUserProps();
        isGroupInit = m_groupManager->isInit();
        if (!isGroupInit) {
            m_stateManager->disableTarget(StateTarget::ResourceLimit);
        } else {
            m_stateManager->enableTarget(StateTarget::ResourceLimit, false);
            m_staticSceneData.is_resLimitInit = true;
        }
    }

    if (m_stateManager->isHaveStateTarget(StateTarget::ResourceScheduling))
    {
        m_staticSceneData.resource_scheduling_enabled = m_configManager->resourceSchedulingEnabled();
        m_staticSceneData.pre_resource_scheduling_enabled = m_configManager->preResourceSchedulingEnabled();
        m_staticSceneData.kernel_support = m_configManager->kernelSupport();
    }

    if (m_stateManager->isHaveStateTarget(StateTarget::SceneManager))
    {
        m_staticSceneData.scenemanager_enabled = m_configManager->scenemanagerEnabled();
        m_staticSceneData.pre_scenemanager_enabled = m_configManager->preScenemanagerEnabled();
    }

    if (m_stateManager->isHaveStateTarget(StateTarget::SoftFreezeModeEnabled))
    {
        m_staticSceneData.soft_freeze_mode_enabled = m_configManager->softFreezeModeEnabled();
    }

    if (m_stateManager->isHaveStateTarget(StateTarget::AppWhiteList))
    {
        m_staticSceneData.app_whitelist = m_configManager->appWhitelist();
    }

    if (m_stateManager->isHaveStateTarget(StateTarget::TopAppList))
    {
        m_staticSceneData.default_apps_in_top_group = m_configManager->defaultAppsInTopGroup();
    }

    // SystemStatusManager
    if (m_stateManager->isHaveStateTarget(StateTarget::ScreenMode))
    {
        m_staticSceneData.curr_screen_mode = m_systemStateManager->currScreenMode();
        m_staticSceneData.last_screen_mode = m_systemStateManager->lastScreenMode();
    }
    
    if (m_stateManager->isHaveStateTarget(StateTarget::StatusMode))
    {
        m_staticSceneData.last_status_mode = m_systemStateManager->lastStatusMode();
        m_staticSceneData.curr_status_mode = m_systemStateManager->currStatusMode();
        if (m_staticSceneData.last_status_mode == sched_policy::StatusMode::Idle &&
            m_staticSceneData.curr_status_mode != sched_policy::StatusMode::Idle){
                m_staticSceneData.last_dpms_mode = m_staticSceneData.curr_dpms_mode;
                m_staticSceneData.curr_dpms_mode = sched_policy::DPMSMode::DPMSOn;
        }
    }

    if (m_stateManager->isHaveStateTarget(StateTarget::InputMode)) {
        m_staticSceneData.last_input_mode = m_systemStateManager->lastInputMode();
        m_staticSceneData.curr_input_mode = m_systemStateManager->currInputMode();
    }

    if (m_stateManager->isHaveStateTarget(StateTarget::ResourceThreshold))
    {
        m_staticSceneData.resource_threshold = std::move(m_systemStateManager->resourceThreshold());
    }

    // PowerManager
    if (m_stateManager->isHaveStateTarget(StateTarget::PowerMode))
    {
        m_staticSceneData.curr_power_mode = m_powerManager->currentPowerMode();
        m_staticSceneData.last_power_mode = m_powerManager->lastPowerMode();
    }

    if (m_stateManager->isHaveStateTarget(StateTarget::PowerType))
    {
        m_staticSceneData.curr_power_type = m_powerManager->currentPowerType();
        m_staticSceneData.last_power_type = m_powerManager->lastPowerType();
    }

    if (m_stateManager->isHaveStateTarget(StateTarget::PowerLevelChanged))
    {
        m_staticSceneData.last_power_percentage = m_staticSceneData.curr_power_percentage,
        m_staticSceneData.curr_power_percentage = PowerManager::getPowerPercentage(); 
        m_staticSceneData.last_power_level = m_staticSceneData.curr_power_level;
        m_staticSceneData.curr_power_level = PowerManager::getPowerLevel(m_staticSceneData.curr_power_percentage);
    }

    // DeviceModeManager
    if (m_stateManager->isHaveStateTarget(StateTarget::DeviceMode))
    {
        m_staticSceneData.curr_device_mode = m_deviceModeManager->currentDeviceMode();
        m_staticSceneData.last_device_mode = m_deviceModeManager->lastDeviceMode();
        m_staticSceneData.real_device_mode = m_deviceModeManager->realDeviceMode();
    }

    if (m_stateManager->isHaveStateTarget(StateTarget::DPMSMode))
    {
        m_staticSceneData.curr_dpms_mode = m_deviceModeManager->currDPMSMode();
        m_staticSceneData.last_dpms_mode = m_deviceModeManager->lastDPMSMode();
    }

    m_staticSceneData.is_prepare_for_sleep = m_stateManager->isHaveStateTarget(StateTarget::PrepareForSleep);
    m_staticSceneData.is_prepare_for_shutdown = m_stateManager->isHaveStateTarget(StateTarget::PrepareForShutdown);

    // ProcessInfoManager
    if (m_stateManager->isHaveStateTarget(StateTarget::ProcessInfo))
    {
        if (isGroupInit) {
            m_staticSceneData.app_change_type = std::move(m_processInfoManager->app_change_type);
        }
        m_staticSceneData.all_app_infos = m_processInfoManager->all_app_infos;
        m_staticSceneData.focus_app_id = m_processInfoManager->focus_app_id;
        m_staticSceneData.is_show_desktop = std::count_if(m_staticSceneData.all_app_infos.begin(), m_staticSceneData.all_app_infos.end(), 
        [](std::unordered_map<std::string, AppInfo>::value_type & appInfo){
            return appInfo.second.name() != "peony-desktop" && appInfo.second.name() != "ukui-panel" && appInfo.second.name() != "ukui-sidebar" && !appInfo.second.isAllWinsMinimize();
        }) == 0;
    }

    if (m_stateManager->isHaveStateTarget(StateTarget::AppWinChangedSpell))
    {
        m_staticSceneData.app_win_changed_spell = std::move(m_processInfoManager->app_win_changed_spell);
    }

    if (m_stateManager->isHaveStateTarget(StateTarget::AppFocusSpell))
    {
        m_staticSceneData.app_focus_spell = std::move(m_processInfoManager->app_focus_spell);
    }


    return m_staticSceneData;
}