#ifndef SENEMANAGER_H
#define SENEMANAGER_H

#include <memory>
#include <string>
#include <vector>

#include <QObject>

#include "datacontext.h"

#include "schedpolicymanager.h"

#include "sceneruleparser.h"

#include "systemnotifiessender.h"


class TriggerManager;

class SceneManager : public QObject 
{
    Q_OBJECT
public:
    SceneManager(const std::shared_ptr<DataContext> &dataContext);
    ~SceneManager();
    
private Q_SLOTS:
    void sceneRecognition();

private:
    void initConnections();
    void initSceneDefinition();

    void handleProcessInfoChanged();
    void handleAppInfo(const AppInfo &appInfo);
    void handleMultimediaByState(const AppInfo &appInfo);
    
private:
    std::shared_ptr<DataContext> m_dataContext;

    StaticSceneData m_staticSceneData;

    std::unique_ptr<SceneRuleParser> m_sceneRuleParser;
    
    std::unique_ptr<TriggerManager> m_triggerManager;

    FairMutex m_mutex;

    std::unique_ptr<SchedPolicyManager> m_schedPolicyManager;

    bool m_windowWatcherEnabled;

    bool m_sceneManagerEnabled;

    bool m_cgroupCreateEnabled;

    std::vector<SceneRule> m_sceneRules;

    std::unordered_map<std::string, std::unordered_set<std::string>> m_currScenes;

    std::shared_ptr<SystemNotifiesSender> m_notifySender;

    QMetaObject::Connection m_conn;
};

class Trigger{
public:
    Trigger(int priority, int sequence, const std::vector<std::string> &mutexGroups)
        :  m_priority(priority), m_sequence(sequence), m_mutexGroups(mutexGroups), m_isTriggering(false) {} 

    virtual ~Trigger() = default;
    
    int getPriority() const { return m_priority; }

    int getSequence() const { return m_sequence; }

    const std::vector<std::string>& getMutexGroups() const { return m_mutexGroups; }

    bool getIsTriggering() const { return m_isTriggering; };

    void setIsTriggering(bool isTriggering) { m_isTriggering = isTriggering; };

    virtual bool triggerIfMatch(StateTarget value, StaticSceneData& data) = 0;
protected:
    int m_priority;         // 优先级 越大越优先
    int m_sequence;         // 注册顺序  越小越优先 优先级一样看注册顺序
    std::vector<std::string> m_mutexGroups;     // 互斥分组
    bool m_isTriggering;
};

class InstantTrigger : public Trigger{
public:
    InstantTrigger(int priority, int sequence, const std::vector<std::string> &mutexGroups, std::unique_ptr<Condition> cond, std::function<void(StateTarget, StaticSceneData&)> cb)
        :Trigger(priority, sequence, mutexGroups), m_pCondition(std::move(cond)), m_callback(std::move(cb)) {}

    bool triggerIfMatch(StateTarget value, StaticSceneData& data) override{
        if (!m_pCondition) {
            return true;
        }
        bool isCheck = m_pCondition->check(value, data);
        if (isCheck) {
            m_callback(value, data);
        }
        return isCheck;
    }
    
private:
    std::unique_ptr<Condition> m_pCondition;
    std::function<void(StateTarget, StaticSceneData&)> m_callback;
    
};

class SustainedTrigger : public Trigger{
public:
    SustainedTrigger(int priority, int sequence, const std::vector<std::string> &mutexGroups, 
        std::unique_ptr<Condition> entryCond, std::unique_ptr<Condition> exitCond, std::function<void(StateTarget, StaticSceneData&)> callback)
            : Trigger(priority, sequence, mutexGroups), m_pEntryCond(std::move(entryCond)), m_pExitCond(std::move(exitCond)), m_callback(std::move(callback)) {}

    bool triggerIfMatch(StateTarget value, StaticSceneData& data) override{
        if (!m_pEntryCond) {
            return false;
        }
        
        if (m_isTriggering){
            if (!m_pExitCond){
                m_callback(StateTarget::All, data);
                return true;
            }   
            bool isCheck = m_pExitCond->check(value, data);
            if (isCheck){
                m_isTriggering = false;
                return false;
            }
            m_callback(StateTarget::All, data);
            return true;
        }
        else{
            bool isCheck = m_pEntryCond->check(value, data);
            if (isCheck) {
                m_callback(StateTarget::All, data);
                m_isTriggering = true;
            }
            return isCheck;
        }
        return false;
    }

private:
    std::unique_ptr<Condition> m_pEntryCond;
    std::unique_ptr<Condition> m_pExitCond;
    std::function<void(StateTarget, StaticSceneData&)> m_callback;

};

class TriggerManager {
public:
    template <typename ConditionType, typename Callback, typename... ConditionArgs>
    void registerInstantTrigger(int priority, const std::vector<std::string> &mutexGroups, Callback&& cb, ConditionArgs&&... condArgs) {
        auto condition = std::unique_ptr<Condition>(
            new ConditionType(std::forward<ConditionArgs>(condArgs)...)
        );
        auto trigger = std::make_shared<InstantTrigger>(
            priority,
            m_nextSequence++,
            mutexGroups,
            std::move(condition),
            std::forward<Callback>(cb)
        );
        m_triggers.push_back(trigger);
    }

    void registerSceneTrigger(SceneRule &sceneRule, std::function<void(StateTarget, StaticSceneData&)> callback) {
        auto trigger = std::make_shared<SustainedTrigger>(
            sceneRule.priority_,
            m_nextSequence++,
            sceneRule.mutex_group,
            std::move(sceneRule.entry_exit_condition.entry_condition),
            std::move(sceneRule.entry_exit_condition.exit_condition),
            std::move(callback)
        );
        m_triggers.push_back(trigger);
    }

    void check(const StateTarget &value, StaticSceneData &sceneData) {
        static std::once_flag flag;
        std::call_once(flag, [this](){
            std::sort(m_triggers.begin(), m_triggers.end(),
            [](const std::shared_ptr<Trigger>& a, const std::shared_ptr<Trigger>& b) {
                if (a->getPriority() != b->getPriority()) {
                    return a->getPriority() > b->getPriority();
                } else {
                    return a->getSequence() < b->getSequence();
                }
            });
        });

        std::unordered_set<std::string> triggeredGroups;

        for (const auto& trig : m_triggers) {
            const std::vector<std::string>& mutexGroups = trig->getMutexGroups();
            bool isBlocked = false;
            for (const auto& group : mutexGroups) {
                if (triggeredGroups.find(group) != triggeredGroups.end()) {
                    isBlocked = true;
                    break;
                }
            }
            if (isBlocked) {
                trig->setIsTriggering(false);
                continue;
            }

            bool isTrigger = trig->triggerIfMatch(value, sceneData);

            if (isTrigger) {
                for (const auto& group : mutexGroups) {
                    triggeredGroups.insert(group);
                }
            }
        }
    }

private:
    std::vector<std::shared_ptr<Trigger>> m_triggers;
    int m_nextSequence {0};
};

#endif // !SENEMANAGER_H