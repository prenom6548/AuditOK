#include "schedpolicymanager.h"
#include "processinfohelper.h"
#include "appinfohelper.h"
#include "loghelper.h"

#include <QDebug>

#define SUBCONDITION_SEPARATOR "#"       // 拼接子场景的分隔符

SchedPolicyManager::SchedPolicyManager(const std::shared_ptr<DataContext> &dataContext)
    : m_dataContext(dataContext)
    , m_prioritySchedulingManager(std::make_shared<PrioritySchedulingManager>())
    , m_policyRuleParser(new PolicyRuleParser(dataContext->configManager()->policyRules(), dataContext->configManager()->customPolicyRules()))
    , m_policyRules(m_policyRuleParser->parse())
{
}

void SchedPolicyManager::insertStrategy(const std::string &sceneId, const std::string &conditionId)
{
    m_nextStrategy[sceneId].push_back(conditionId);
}

void SchedPolicyManager::implementStrategy(const StaticSceneData& data)
{
    std::unordered_set<std::string> all_keys;
    for (const auto& pair : m_currStrategy) {
        all_keys.insert(pair.first);
    }
    for (const auto& pair : m_nextStrategy) {
        all_keys.insert(pair.first);
    }

    bool sceneIsChanged = false;
    for (const auto& key : all_keys) {
        const auto &policyRule = m_policyRules.find(key);
        if (policyRule == m_policyRules.end())  
        {
            PRINT_NOTICE("scene id : %s policy rule is not find\n", key.c_str());
            continue;
        }
        auto it1 = m_currStrategy.find(key);
        auto it2 = m_nextStrategy.find(key);

        // 场景没有变化，但子条件可能有变化
        if (it1 != m_currStrategy.end() && it2 != m_nextStrategy.end()) {
            auto& subConds1 = it1->second;
            auto& subConds2 = it2->second;

            std::sort(subConds1.begin(), subConds1.end());
            std::sort(subConds2.begin(), subConds2.end());

            std::vector<std::string> only1, only2, repeat;

            std::set_difference(
                subConds1.begin(), subConds1.end(),
                subConds2.begin(), subConds2.end(),
                std::back_inserter(only1)
            );

            std::set_difference(
                subConds2.begin(), subConds2.end(),
                subConds1.begin(), subConds1.end(),
                std::back_inserter(only2)
            );

            std::set_intersection(
                subConds1.begin(), subConds1.end(),
                subConds2.begin(), subConds2.end(),
                std::back_inserter(repeat)
            );

            // 当前场景独有的子场景消失，需要退出子场景
           for (const auto &subConditionId : only1){
               PRINT_NOTICE("subCondition : %s exit\n", subConditionId.c_str());
               auto it = policyRule->second.subhandles_.find(subConditionId);
               if (it != policyRule->second.subhandles_.end()) {
                   PRINT_NOTICE("subCondition : %s exit\n", subConditionId.c_str());
                   for (auto &handler : it->second) {
                       handler->reverse();
                   }
               }
            }

            // 触发场景独有的子场景出现，需要进入子场景
            for (const auto &subConditionId : only2){
                PRINT_NOTICE("subCondition : %s enter\n", subConditionId.c_str());
                auto it = policyRule->second.subhandles_.find(subConditionId);
                if (it != policyRule->second.subhandles_.end()){
                    PRINT_NOTICE("subCondition : %s enter\n", subConditionId.c_str());
                    for (auto &handler : it->second){
                        handler->handle(data);
                    }
                }
            }

            if (!only1.empty() || !only2.empty()){
                sceneIsChanged = true;
            }

            // 已经触发过的场景下，需要重复触发执行，再次触发一次
            for (auto &handler : policyRule->second.handlers_){
                if (handler->needRepeat()){
                    PRINT_DBG("handler repeat\n");
                    handler->handle(data);
                }
            }
            for (const auto &subConditionId : repeat){
                auto it = policyRule->second.subhandles_.find(subConditionId);
                if (it != policyRule->second.subhandles_.end()){
                    for (auto &handler : it->second){
                        if (handler->needRepeat()){
                            PRINT_DBG("handler repeat\n");
                            handler->handle(data);
                        }
                    }
                }
            }
        }
        // 当前这个场景已经退出
        else if (it1 != m_currStrategy.end()) {
            sceneIsChanged = true;
            PRINT_NOTICE("sceneId : %s exit\n", key.c_str());
            for (const auto &subConditionId : it1->second){
                PRINT_NOTICE("subCondition : %s exit\n", subConditionId.c_str());
                auto it = policyRule->second.subhandles_.find(subConditionId);
                if (it != policyRule->second.subhandles_.end()){
                    PRINT_NOTICE("subCondition : %s exit\n", subConditionId.c_str());
                    for (auto &handler : it->second){
                        handler->reverse();
                    }
                }
            }
            for (auto &handler : policyRule->second.handlers_){
                handler->reverse();
            }
        } 
        // 之前没有这个场景 新进入场景
        else {
            sceneIsChanged = true;
            PRINT_NOTICE("scene : %s enter\n", key.c_str());
            for (auto &handler : policyRule->second.handlers_){
                handler->handle(data);
            }
            for (const auto &subConditionId : it2->second){
                PRINT_NOTICE("subCondition : %s enter\n", subConditionId.c_str());
                auto it = policyRule->second.subhandles_.find(subConditionId);
                if (it != policyRule->second.subhandles_.end()){
                    PRINT_NOTICE("subCondition : %s enter\n", subConditionId.c_str());
                    for (auto &handler : it->second){
                        handler->handle(data);
                    }
                }
            }
        }
    }
    if (sceneIsChanged){
        if (!m_nextStrategy.empty()){
            std::vector<std::string> sceneIds;
            bool isHaveLongIdle = false;
            for (const auto& s : m_nextStrategy){
                std::string sceneStr = s.first;
                for (const auto& ss : s.second){
                    if (ss == "long_idle"){
                        isHaveLongIdle = true;
                    } else {
                        if (!ss.empty()){
                            sceneStr += SUBCONDITION_SEPARATOR + ss;
                        }
                    }
                }
                sceneIds.push_back(sceneStr);
                if (isHaveLongIdle){
                    sceneIds.push_back("long_" + sceneStr);
                }
            }
            m_resourceManagerInterface.setCurrentScene(sceneIds);
        }
    }
    m_currStrategy.swap(m_nextStrategy);
    m_nextStrategy.clear();
}

void SchedPolicyManager::reclaimProcessGroups(const std::vector<std::string> &cachedGroups)
{
    m_resourceManagerInterface.reclaimProcessGroups(cachedGroups);
}

void SchedPolicyManager::handleAppChanged(const AppInfo &appInfo)
{
    auto groupType = sched_policy::getGroupTypeByAppInfo(appInfo.appType(), appInfo.appState());
    std::string groupStr = sched_policy::groupTypeToString(groupType);
    if (appInfo.pids().empty() || groupStr.empty()) {
        return;
    }
    m_resourceManagerInterface.limitProcessByGroup(groupStr, appInfo.pids().front());
}

void SchedPolicyManager::handlePriorityScheduling(const AppInfo &appInfo, bool isLowPower)
{
    static std::vector<int> lastPriorityPids;
    for (int pid : lastPriorityPids){
        m_prioritySchedulingManager->eraseTopTask(pid);
    }
    lastPriorityPids.clear();
    const QStringList &blackListProcess = m_dataContext->configManager()->blackListProcess();
    bool isBlack = false;
    for (const QString &proc : blackListProcess){
        if (appInfo.name().find(proc.toStdString()) != std::string::npos){
            isBlack = true;
            break;
        }
    }

    if (!appInfo.pids().empty() && !isBlack){
        lastPriorityPids.push_back(appInfo.pids().front());
        if (isLowPower) {
            m_prioritySchedulingManager->insertTopTask(appInfo.pids().front(), 1, 3, 140, 1);
        } else {
            m_prioritySchedulingManager->insertTopTask(appInfo.pids().front(), 1, 3, 10, 1);
        }
    }

    static std::unordered_map<std::string, std::vector<int>> coreProcessMap;
    static std::once_flag flag;
    std::call_once(flag, [&](){
        const QStringList &coreTopProcess = m_dataContext->configManager()->coreTopProcess();
        for (const QString &ctp : coreTopProcess){
            coreProcessMap[ctp.toStdString()].emplace_back();
        }
    });

    for (auto &pair : coreProcessMap){
        bool needObtain = pair.second.empty();
        for (const int &pid : pair.second){
            if (!process_info_helper::processExists(pid)){
                needObtain = true;
                break;
            }
        }
        if (needObtain){
            for (int pid : pair.second){
                m_prioritySchedulingManager->eraseTopTask(pid);
            }
            pair.second = process_info_helper::pidsByCmdline(pair.first, false);
            for (int pid : pair.second){
                m_prioritySchedulingManager->insertTopTask(pid, 1, 5, 5, 1);
            }
        }
    }
}

void SchedPolicyManager::handleMultimediaPause(const std::string &serviceName)
{
    m_multimediaController.pause(serviceName);
}

void SchedPolicyManager::handleResourceLimitEnabledChanged(bool enabled)
{
    m_dataCollector.collectResouceLimitedEnebaledChanged(enabled);
}

void SchedPolicyManager::handleSoftFreezeModeEnabledChanged(bool enabled)
{
    m_dataCollector.collectSoftFreezeModeEnabledChanged(enabled);
}

void SchedPolicyManager::handleResourceSchedulingEnabledChanged(bool enabled)
{
    if (enabled){
        m_prioritySchedulingManager->enable();
    } else {
        m_prioritySchedulingManager->disable();
    }
}