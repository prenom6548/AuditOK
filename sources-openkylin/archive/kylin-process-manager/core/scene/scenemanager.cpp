#include "scenemanager.h"

#include "eventbus.h"

#include "appinfohelper.h"
#include "loghelper.h"
#include <unistd.h>

SceneManager::SceneManager(const std::shared_ptr<DataContext> &dataContext)
    : m_dataContext(dataContext)
    , m_sceneRuleParser(new SceneRuleParser(dataContext->configManager()->sceneRules()))
    , m_triggerManager(new TriggerManager)
    , m_schedPolicyManager(new SchedPolicyManager(dataContext))
    , m_windowWatcherEnabled(false)
    , m_sceneManagerEnabled(false)
    , m_cgroupCreateEnabled(false)
    , m_notifySender(std::make_shared<SystemNotifiesSender>(dataContext->configManager()))
{
    initSceneDefinition();
    initConnections();
    
    m_dataContext->stateManager()->enableTarget(StateTarget::Init);
}

SceneManager::~SceneManager()
{
    PRINT_NOTICE("~SceneManager\n");
    if (m_conn) {
        QObject::disconnect(m_conn);
        m_conn = {};
    }
}

void SceneManager::sceneRecognition()
{
    std::lock_guard<FairMutex> lock(m_mutex);

    // 截取静态数据
    m_staticSceneData = m_dataContext->clipStaticSceneData();

    StateTarget target = m_dataContext->stateManager()->stateTarget();

    m_dataContext->stateManager()->multiLockMutexUnlock();

    m_triggerManager->check(target, m_staticSceneData);

    for (auto it = m_currScenes.begin(); it != m_currScenes.end(); ) {
        if (it->second.count("") != 0) {
            it->second.erase("");
            ++it;
        } else {
            it = m_currScenes.erase(it);
        }
    }
    
    m_schedPolicyManager->implementStrategy(m_staticSceneData);
}

void SceneManager::initConnections()
{
    m_conn = QObject::connect(m_dataContext->stateManager().get(), &StateManager::stateTargetChanged, this, &SceneManager::sceneRecognition);
}

void SceneManager::initSceneDefinition()
{
    // 场景
    m_sceneRules = std::move(m_sceneRuleParser->parse());
    for (auto &sceneRule: m_sceneRules)
    {
        if (!sceneRule.enable_) {
            continue;
        }
        m_triggerManager->registerSceneTrigger(sceneRule, [this, &sceneRule](StateTarget value, StaticSceneData &data) {
            if (!data.scenemanager_enabled && (data.bench_mark_state == false))
                return;
            m_currScenes[sceneRule.scene_id].insert("");
            PRINT_DBG("=====SceneId : %s\n", sceneRule.scene_id.c_str());
            for (auto &subcondition : sceneRule.subconditions_){
                auto &eeCond = subcondition.second;
                if (!eeCond.entry_condition)
                    continue;
                if (m_currScenes[sceneRule.scene_id].count(subcondition.first) == 0){
                    if (eeCond.entry_condition->check(value, data)){
                        m_currScenes[sceneRule.scene_id].insert(subcondition.first);
                    }
                }
                else{
                    if (eeCond.exit_condition && eeCond.exit_condition->check(value, data)){
                        m_currScenes[sceneRule.scene_id].erase(subcondition.first);
                    }
                }
            }
            for (const std::string &subCond : m_currScenes[sceneRule.scene_id]){
                m_schedPolicyManager->insertStrategy(sceneRule.scene_id, subCond);
                PRINT_DBG("SceneId: %s, SubSceneId : %s\n", sceneRule.scene_id.c_str(), subCond.c_str());
            }
        });
    }

    // 窗口监控开关
    m_triggerManager->registerInstantTrigger<InCondition>(
        20, 
        {}, 
        [this](StateTarget value, StaticSceneData& data){
            if (!data.scenemanager_enabled && !data.resource_limit_enabled && !(data.resource_scheduling_enabled && data.kernel_support)) {
                if (!data.is_init){
                    EventBus::instance()->notifyStopWindowWatcher();
                    m_windowWatcherEnabled = false;
                }
            } else {
                if (!m_windowWatcherEnabled)
                {
                    EventBus::instance()->notifyStartWindowWatcher();
                    m_windowWatcherEnabled = true;
                }
            }
        }, 
        std::initializer_list<StateTarget>{ StateTarget::ResourceLimit, StateTarget::SceneManager, StateTarget::ResourceScheduling});

    // cgroup分组创建
    m_triggerManager->registerInstantTrigger<EqualCondition>(
        21,
        {},
        [this](StateTarget value, StaticSceneData &data) {
            if (!data.resource_limit_enabled) {
                if (!data.is_resLimitInit) {
                    EventBus::instance()->notifyStopWatcher();
                    m_dataContext->groupManager()->removeGroups();
                    m_cgroupCreateEnabled = false;
                }
            } else {
                if (data.curr_device_mode != sched_policy::DeviceMode::Unknown && data.curr_power_mode != sched_policy::PowerMode::Unknown) {
                    m_dataContext->groupManager()->executePolicy(sched_policy::generatePolicyId(data.curr_device_mode, data.curr_power_mode));
                }

                m_dataContext->groupManager()->createGroups();
                EventBus::instance()->notifyStartWatcher();
            }
        },
        StateTarget::ResourceLimit);

    // ResourceLimit开关
    m_triggerManager->registerInstantTrigger<EqualCondition>(
        19, 
        {}, 
        [this](StateTarget value, StaticSceneData& data){
            if (data.resource_limit_enabled){
                for (const auto & pair : data.all_app_infos){
                    handleAppInfo(pair.second);
                }
            }
            m_schedPolicyManager->handleResourceLimitEnabledChanged(data.resource_limit_enabled);
        }, 
        StateTarget::ResourceLimit);

    // SceneManager开关
    m_triggerManager->registerInstantTrigger<InCondition>(
        19,
        {},
        [this](StateTarget value, StaticSceneData &data) {
            if (data.scenemanager_enabled && (data.bench_mark_state == false)) {
                if (!m_sceneManagerEnabled) {
                    EventBus::instance()->notifyStartFreezerWatcher();
                    EventBus::instance()->notifyStartScreenWatcher();
                    m_sceneManagerEnabled = true;
                }
            } else {
                if (!data.is_init){
                    EventBus::instance()->notifyStopFreezerWatcher();
                    EventBus::instance()->notifyStopScreenWatcher();
                    m_sceneManagerEnabled = false;
                }
            }
        },
        std::initializer_list<StateTarget>{StateTarget::SceneManager, StateTarget::BenchmarkStateChanged});

    // ResourceScheduling开关
    m_triggerManager->registerInstantTrigger<EqualCondition>(
        19, 
        {}, 
        [this](StateTarget value, StaticSceneData& data){
            if (data.resource_scheduling_enabled){
                m_schedPolicyManager->handleResourceSchedulingEnabledChanged(true);
                auto it = data.all_app_infos.find(data.focus_app_id);
                if (it != data.all_app_infos.end()){
                    m_schedPolicyManager->handlePriorityScheduling(it->second, data.curr_power_mode == sched_policy::PowerMode::Save);
                }
            }
            else{
                m_schedPolicyManager->handleResourceSchedulingEnabledChanged(false);
            }
        },
        StateTarget::ResourceScheduling);

    // 软冻模式
    m_triggerManager->registerInstantTrigger<EqualCondition>(
        19, 
        {}, 
        [this](StateTarget value, StaticSceneData& data){
            if (data.soft_freeze_mode_enabled){
                m_dataContext->configManager()->setResourceLimitEnabled(true);
            }
            m_schedPolicyManager->handleSoftFreezeModeEnabledChanged(data.soft_freeze_mode_enabled);
            m_dataContext->deviceModeManager()->handleSoftFreezeModeEnabledChanged(data.soft_freeze_mode_enabled);
        },
        StateTarget::SoftFreezeModeEnabled);

    // cgroup 、调度
    m_triggerManager->registerInstantTrigger<EqualCondition>(
        3, 
        {}, 
        [this](StateTarget value, StaticSceneData& data){
            handleProcessInfoChanged();
        }, 
        StateTarget::ProcessInfo);

    // 电源模式变化
    m_triggerManager->registerInstantTrigger<EqualCondition>(
        4, 
        {}, 
        [this](StateTarget value, StaticSceneData& data){
            if (data.curr_device_mode != sched_policy::DeviceMode::Unknown &&
                data.curr_power_mode != sched_policy::PowerMode::Unknown) {
                m_dataContext->groupManager()->executePolicy(sched_policy::generatePolicyId(data.curr_device_mode, data.curr_power_mode));
            }
            if (data.curr_power_mode == sched_policy::PowerMode::Save){
                for (const auto &appInfoPair : m_staticSceneData.all_app_infos)
                {
                    const auto &appInfo = appInfoPair.second;
                    // 多媒体控制
                    handleMultimediaByState(appInfo);
                }
            }
        }, 
        StateTarget::PowerMode);

    // 设备模式变化
    m_triggerManager->registerInstantTrigger<EqualCondition>(
        4, 
        {}, 
        [this](StateTarget value, StaticSceneData& data){
            if (data.curr_device_mode != sched_policy::DeviceMode::Unknown &&
                data.curr_power_mode != sched_policy::PowerMode::Unknown) {
                m_dataContext->groupManager()->executePolicy(sched_policy::generatePolicyId(data.curr_device_mode, data.curr_power_mode));
            }
            m_dataContext->appLaunchManager()->setDeviceMode(data.curr_device_mode);
            m_dataContext->processInfoManager()->setBackgroundToCachedStateInterval(m_dataContext->configManager()->background2CachedStateInterval(data.curr_device_mode));
        }, 
        StateTarget::DeviceMode);

    m_triggerManager->registerInstantTrigger<InCondition>(
        19, 
        {}, 
        [this](StateTarget value, StaticSceneData& data){
            if (data.curr_power_type == sched_policy::PowerType::Battery && data.curr_device_mode == sched_policy::DeviceMode::PC &&
               !data.soft_freeze_mode_enabled && data.resource_limit_enabled) {
                EventBus::instance()->notifyStartBatteryEstimate();
            } else {
                EventBus::instance()->notifyStopBatteryEstimate();
            }
        }, 
        std::initializer_list<StateTarget>{ StateTarget::PowerMode, StateTarget::DeviceMode, StateTarget::ResourceLimit, StateTarget::SoftFreezeModeEnabled});

    // 资源阈值
    m_triggerManager->registerInstantTrigger<EqualCondition>(
        4, 
        {}, 
        [this](StateTarget value, StaticSceneData& data){
            if (!data.resource_limit_enabled) {
                m_notifySender->sendResourceWarningNotify();
            } else {
                if (data.resource_threshold.find("Memory") != data.resource_threshold.end()){
                    std::vector<std::string> cachedGroups; 
                    for (const auto &app : data.all_app_infos) {
                        if (app.second.appState() == sched_policy::AppState::Cached) {
                            cachedGroups.emplace_back(std::move(app.second.groupName()));
                        }
                    }

                    m_schedPolicyManager->reclaimProcessGroups(cachedGroups);
                }
            }
        }, 
        StateTarget::ResourceThreshold);
}

void SceneManager::handleProcessInfoChanged()
{
    for (const auto &appChangeType : m_staticSceneData.app_change_type){
        if (appChangeType.second.at(sched_policy::AppChangeType::AppStateChanged))
        {
            auto it = m_staticSceneData.all_app_infos.find(appChangeType.first);
            if (it == m_staticSceneData.all_app_infos.end())    continue;
            const auto &appInfo = it->second;
            // 焦点进程做优先调度
            if (m_staticSceneData.resource_scheduling_enabled){
                if (appInfo.appState() == sched_policy::AppState::Focus){
                    m_schedPolicyManager->handlePriorityScheduling(appInfo, m_staticSceneData.curr_power_mode == sched_policy::PowerMode::Save);
                }
            }

            // 分级冻结
            if (m_staticSceneData.resource_limit_enabled){
                handleAppInfo(appInfo);
            }
        }
    }
}

void SceneManager::handleAppInfo(const AppInfo &appInfo)
{
    // 白名单
    for (const auto &appDesktopFile : m_dataContext->configManager()->appWhitelist()) {
        if (appinfo_helper::isSameDesktopFile(appDesktopFile.toStdString(), appInfo.desktopFile())) {
            continue;
        }
    }

    // 多媒体控制
    handleMultimediaByState(appInfo);

    m_schedPolicyManager->handleAppChanged(appInfo);
}

void SceneManager::handleMultimediaByState(const AppInfo &appInfo)
{
    if (!appInfo.mprisDbusService().empty() && (appInfo.appState() == sched_policy::AppState::Background || appInfo.appState() == sched_policy::AppState::Cached) && (m_staticSceneData.curr_power_mode == sched_policy::PowerMode::Save || m_staticSceneData.curr_device_mode == sched_policy::DeviceMode::Tablet)) {
        m_schedPolicyManager->handleMultimediaPause(appInfo.mprisDbusService());
    }
}