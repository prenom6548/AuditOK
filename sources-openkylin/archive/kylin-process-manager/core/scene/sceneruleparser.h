#ifndef SCENERULEPARSER_H
#define SCENERULEPARSER_H

#include <memory>
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <jsoncpp/json/json.h>

#include "statetarget.h"
#include "staticscenedata.h"

struct ConditionTarget{
    bool is_valid;
    std::string category_;
    std::string object_;
    std::string value_;
    std::string operator_;
};

extern bool condion_target_check(ConditionTarget &condTarget, StaticSceneData &data);

class Condition {
public:
    virtual ~Condition() = default;
    virtual bool check(StateTarget value, StaticSceneData& data) = 0;
};

// 等于目标值
class EqualCondition : public Condition {
public:
    explicit EqualCondition(StateTarget state) : m_state(state) {}
    bool check(StateTarget value, StaticSceneData& data) override {
        return static_cast<bool>(m_state & value);
    }

private:
    StateTarget m_state;
};

class TargetEqualCondition : public Condition {
    public:
        explicit TargetEqualCondition(StateTarget state, ConditionTarget cond) : m_state(state), m_conditionTarget(cond) {}
        bool check(StateTarget value, StaticSceneData& data) override {
            return static_cast<bool>(m_state & value) && condion_target_check(m_conditionTarget, data);
        }
    
    private:
        StateTarget m_state;
        ConditionTarget m_conditionTarget;
};

// 在集合中
class InCondition : public Condition 
{
public:
    explicit InCondition(const std::initializer_list<StateTarget> &values)
        : m_values(values) 
    {

    }
    bool check(StateTarget value, StaticSceneData& data) override {
        bool isChecked = false;
        for (const auto &v : m_values) {
            if (static_cast<bool>(v & value)) {
                isChecked = true;
                break;
            }
        }
        return isChecked;
    }

private:
    std::unordered_set<StateTarget> m_values;
};

class TargetInCondition : public Condition 
{
public:
    explicit TargetInCondition(const std::unordered_map<StateTarget, ConditionTarget> &values) : m_values(values) {}
    bool check(StateTarget value, StaticSceneData& data) override {
        bool isChecked = false;
        for (auto &pair : m_values) {
            if (static_cast<bool>(pair.first & value) 
                && condion_target_check(pair.second, data)) {
                isChecked = true;
                break;
            }
        }
        return isChecked;
    }

private:
    std::unordered_map<StateTarget, ConditionTarget> m_values;
};

// 不满足某个子条件
class NotCondition : public Condition 
{
public:
    explicit NotCondition(std::unique_ptr<Condition> cond)
        : m_pCondition(std::move(cond)) {}
    bool check(StateTarget value, StaticSceneData& data)override {
        return !m_pCondition->check(value, data);
    }

private:
    std::unique_ptr<Condition> m_pCondition;
};

// 所有子条件都满足
class AndCondition : public Condition 
{
public:
    explicit AndCondition(std::vector<std::unique_ptr<Condition>> &&subConds) 
        : m_pConditions(std::move(subConds))
    {
    }

    bool check(StateTarget value, StaticSceneData& data) override {
        return std::all_of(m_pConditions.begin(), m_pConditions.end(),
            [&value, &data](const std::unique_ptr<Condition>& cond) { return cond->check(value, data); });
    }

private:
    std::vector<std::unique_ptr<Condition>> m_pConditions;
};

// 至少一个子条件满足
class OrCondition : public Condition 
{
public:
    explicit OrCondition(std::vector<std::unique_ptr<Condition>> &&subConds) 
        : m_pConditions(std::move(subConds))
    {
    }

    bool check(StateTarget value, StaticSceneData& data) override {
        return std::any_of(m_pConditions.begin(), m_pConditions.end(),
            [&value, &data](const std::unique_ptr<Condition>& cond) { return cond->check(value, data); });
    }

private:
    std::vector<std::unique_ptr<Condition>> m_pConditions;
};

struct EntryExitCondition{
    std::unique_ptr<Condition> entry_condition;
    std::unique_ptr<Condition> exit_condition;
};

struct SceneRule {
    std::string scene_id;
    bool enable_;
    int priority_;
    std::vector<std::string> mutex_group;
    std::vector<std::string> dependencices_;
    std::vector<std::string> allowed_coexist;
    EntryExitCondition entry_exit_condition;
    std::unordered_map<std::string, EntryExitCondition> subconditions_;
};

class SceneRuleParser {
public:
    SceneRuleParser(const Json::Value &sceneRules);

    std::vector<SceneRule> parse();
private:
    const Json::Value &m_sceneRules;
};

#endif // SCENERULEPARSER_H