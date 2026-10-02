#ifndef SCHEDPOLICYMANAGER_H
#define SCHEDPOLICYMANAGER_H

#include <memory>
#include "datacontext.h"
#include "multimediacontroller.h"
#include "appinfo.h"
#include "datacollector.h"
#include "policyruleparser.h"
#include "staticscenedata.h"
#include "resourcemanagerinterface.h"
#include "priorityschedulingmanager.h"

class SchedPolicyManager
{
public:
    SchedPolicyManager(const std::shared_ptr<DataContext> &dataContext);

    void insertStrategy(const std::string &sceneId, const std::string &conditionId);
    void implementStrategy(const StaticSceneData& data);
    void reclaimProcessGroups(const std::vector<std::string> &cachedGroups);
    void handleAppChanged(const AppInfo &appInfo);
    void handlePriorityScheduling(const AppInfo &appInfo, bool isLowPower);
    void handleMultimediaPause(const std::string &serviceName);
    void handleResourceLimitEnabledChanged(bool enabled);
    void handleSoftFreezeModeEnabledChanged(bool enabled);
    void handleResourceSchedulingEnabledChanged(bool enabled);

private:
    std::shared_ptr<DataContext> m_dataContext;
    std::shared_ptr<PrioritySchedulingManager> m_prioritySchedulingManager;
    std::unique_ptr<PolicyRuleParser> m_policyRuleParser;

    ResourceManagerInterface m_resourceManagerInterface;
    MultimediaController m_multimediaController;
    DataCollector m_dataCollector;

    std::unordered_map<std::string, PolicyRule> m_policyRules;

    std::unordered_map<std::string, std::vector<std::string>> m_currStrategy;
    std::unordered_map<std::string, std::vector<std::string>> m_nextStrategy;
};

#endif // SCHEDPOLICYMANAGER_H
