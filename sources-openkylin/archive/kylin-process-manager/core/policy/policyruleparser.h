#ifndef POLICYRULEPARSER_H
#define POLICYRULEPARSER_H


#include <string>
#include <memory>
#include <unordered_map>
#include <list>
#include <jsoncpp/json/json.h>

#include "cgroupcontrolinterface.h"
#include "configmanager.h"
#include "displayeffectmanager.h"
#include "groupmanager.h"
#include "resourcemanagerinterface.h"
#include "staticscenedata.h"
#include "systemstatemanager.h"

extern void register_group_manager(std::shared_ptr<GroupManager> groupManager);
extern void register_config_manager(std::shared_ptr<ConfigManager> configManager);
extern void register_systemstate_manager(std::shared_ptr<SystemStateManager> systemStateManager);

class Handler {
public:
    virtual ~Handler() = default;
    virtual void handle(const StaticSceneData& data) = 0;
    virtual void reverse() = 0;

    void setNeedRepeat(bool needRepeat){ m_needRepeat = needRepeat; }
    bool needRepeat() { return m_needRepeat; }
protected:
    bool m_needRepeat {false};
};

class FreezeHandler : public Handler {
public:
    explicit FreezeHandler(const std::vector<std::string> &freezeList)
        : m_freezeList(freezeList)
    {
    }
    void handle(const StaticSceneData& data) override;
    void reverse() override;

private:
    std::vector<std::string> m_freezeList;
    std::vector<int> m_freezeIndex;
    CgroupControlInterface m_cgroupControlInterface;
};

class ServiceFreezeHandler : public Handler {
public:
    explicit ServiceFreezeHandler(const std::vector<std::string> &freezeList)
        :m_freezeList(freezeList)
    {
    }
    void handle(const StaticSceneData &data) override;
    void reverse() override;

private:
    std::vector<std::string> m_freezeList;
    ResourceManagerInterface m_resourceManager;
    std::unordered_map<std::string, int> m_groupFreezeNames;
    std::vector<int> m_freezeIndex;
    CgroupControlInterface m_cgroupControlInterface;
};

class ServiceStopHandler : public Handler {
public:
    explicit ServiceStopHandler(const std::vector<std::string> &stopList)
        :m_stopList(stopList)
    {
    }
    void handle(const StaticSceneData &data) override;
    void reverse() override;

private:
    std::vector<std::string> m_stopList;
    ResourceManagerInterface m_resourceManager;

    std::list<std::string> m_groupStopServices;

    // std::unordered_map<std::string, sched_policy::GroupType> m_groupFreezeNames;
    // std::unordered_map<std::string, std::string> m_mapServiceScope;
};

class MemdirtyHandler: public Handler{
public:
    explicit MemdirtyHandler(const std::vector<std::string> &memdirtyList)
        : m_memdirtyList(memdirtyList)
    {
    }
    void handle(const StaticSceneData& data) override;
    void reverse() override;

private:
    std::vector<std::string> m_memdirtyList;
    ResourceManagerInterface m_resourceManager;

};
class KernelschedHandler: public Handler{
public:
    explicit KernelschedHandler(const std::vector<std::string> &kernelschedList)
        : m_kernelschedList(kernelschedList)
    {
    }
    void handle(const StaticSceneData& data) override;
    void reverse() override;

private:
    std::vector<std::string> m_kernelschedList;
    ResourceManagerInterface m_resourceManager;

};
class NetparamsHandler: public Handler{
public:
    explicit NetparamsHandler(const std::vector<std::string> &netparamsList)
        : m_netparamsList(netparamsList)
    {
    }
    void handle(const StaticSceneData& data) override;
    void reverse() override;

private:
    std::vector<std::string> m_netparamsList;
    ResourceManagerInterface m_resourceManager;

};
class SamplingrateHandler: public Handler{
public:
    explicit SamplingrateHandler(const std::vector<std::string> &samplingrateList)
        : m_samplingrateList(samplingrateList)
    {
    }
    void handle(const StaticSceneData& data) override;
    void reverse() override;

private:
    std::vector<std::string> m_samplingrateList;
    ResourceManagerInterface m_resourceManager;

};
class CpulimitHandler: public Handler{
public:
    explicit CpulimitHandler(const std::vector<std::string> &cpulimitList)
        : m_cpulimitList(cpulimitList)
    {
    }
    void handle(const StaticSceneData& data) override;
    void reverse() override;

private:
    std::vector<std::string> m_cpulimitList;
    ResourceManagerInterface m_resourceManager;

};

class BrightnessHandler: public Handler{
public:
    explicit BrightnessHandler(const std::vector<std::string> &brightnessList)
        : m_brightnessList(brightnessList)
    {
    }
    void handle(const StaticSceneData& data) override;
    void reverse() override;

private:
    std::vector<std::string> m_brightnessList;
    int m_oldBrightnessValue;
};

class PSRStateHandler : public Handler {
public:
    explicit PSRStateHandler()
    {
    }
    void handle(const StaticSceneData& data) override;
    void reverse() override;

private:
    ResourceManagerInterface m_resourceManager;
    
};

class VRRStateHandler : public Handler {
public:
    explicit VRRStateHandler(std::unordered_map<std::string, std::string> &policyMap)
        :m_powerLevelPolycies(policyMap)
    {
        powerLevelMap = {
            {1 , "POWER_LEVEL_1"},
            {2 , "POWER_LEVEL_2"},
            {3 , "POWER_LEVEL_2"},
            {4 , "POWER_LEVEL_4"},
        };
    }
    void handle(const StaticSceneData& data) override;
    void reverse() override;

private:
    std::unordered_map<std::string, std::string> m_powerLevelPolycies;
    std::unordered_map<int, std::string> powerLevelMap;
    ResourceManagerInterface m_resourceManager;
    
};

class EffectsHandler : public Handler {
public:
    explicit EffectsHandler(const std::vector<std::string> &filterlist)
        : m_filterList(filterlist)
    {
    }
    void handle(const StaticSceneData& data) override;
    void reverse() override;

private:
    std::vector<std::string> m_filterList;
};

struct PolicyRule {
    std::string scene_id;
    std::vector<std::unique_ptr<Handler>> handlers_;
    std::unordered_map<std::string, std::vector<std::unique_ptr<Handler>>> subhandles_;
};

class PolicyRuleParser {
public:
    PolicyRuleParser(const Json::Value &policyRules, const QList<Json::Value> &customPolicyRules);

    std::unordered_map<std::string, PolicyRule> parse();

private:
    const Json::Value &m_policyRules;
    const QList<Json::Value> &m_customPolicyRules;
};

#endif // !POLICYRULEPARSER_H