#include "sceneruleparser.h"
#include "eventbus.h"
#include "jsonhelper.h"
#include "loghelper.h"
#include "processinfohelper.h"
#include "systemstatehelper.h"

#include <fstream>
#include <iostream>
#include <algorithm>
#include <QDebug>
#include <QUuid>


SceneRuleParser::SceneRuleParser(const Json::Value &sceneRules)
    : m_sceneRules(sceneRules)
{

}


namespace{

template <typename T>
class Comparator {
public:
    virtual ~Comparator() = default;
    virtual bool operator()(const T& lhs, const T& rhs) const = 0;
};

template <typename T>
class EqComparator : public Comparator<T> {
public:
    bool operator()(const T& lhs, const T& rhs) const override {
        return lhs == rhs;
    }
};

template <typename T>
class NeqComparator : public Comparator<T> {
public:
    bool operator()(const T& lhs, const T& rhs) const override {
        return lhs != rhs;
    }
};

template <typename T>
class GtComparator : public Comparator<T> {
public:
    bool operator()(const T& lhs, const T& rhs) const override {
        return lhs > rhs;
    }
};

template <typename T>
class GeqComparator : public Comparator<T> {
public:
    bool operator()(const T& lhs, const T& rhs) const override {
        return lhs >= rhs;
    }
};

template <typename T>
class LtComparator : public Comparator<T> {
public:
    bool operator()(const T& lhs, const T& rhs) const override {
        return lhs < rhs;
    }
};

template <typename T>
class LeqComparator : public Comparator<T> {
public:
    bool operator()(const T& lhs, const T& rhs) const override {
        return lhs <= rhs;
    }
};

template <typename Type>
class OperatorRegistry {
public:
    void registerOperator(
        const std::string& name,
        std::unique_ptr<Comparator<Type>> comparator
    ) {
        m_operators[name] = std::move(comparator);
    }

    const Comparator<Type>* findOperator(const std::string& name) const {
        auto it = m_operators.find(name);
        return (it != m_operators.end()) ? it->second.get() : nullptr;
    }
private:
    std::unordered_map<std::string, std::unique_ptr<Comparator<Type>>> m_operators;
};

template <typename EnumType>
inline OperatorRegistry<EnumType> MakeEnumOperatorRegistry() {
    OperatorRegistry<EnumType> registry;
    registry.registerOperator("eq", std::unique_ptr<Comparator<EnumType>>(
        new EqComparator<EnumType>()
    ));
    registry.registerOperator("neq", std::unique_ptr<Comparator<EnumType>>(
        new NeqComparator<EnumType>()
    ));
    return registry;
}

inline OperatorRegistry<bool> MakeBoolOperatorRegistry() {
    OperatorRegistry<bool> registry;
    registry.registerOperator("eq", std::unique_ptr<Comparator<bool>>(
        new EqComparator<bool>()
    ));
    registry.registerOperator("neq", std::unique_ptr<Comparator<bool>>(
        new NeqComparator<bool>()
    ));
    return registry;
}

inline OperatorRegistry<int> MakeIntOperatorRegistry() {
    OperatorRegistry<int> registry;
    
    registry.registerOperator("eq", std::unique_ptr<Comparator<int>>(
        new EqComparator<int>()
    ));
    registry.registerOperator("neq", std::unique_ptr<Comparator<int>>(
        new NeqComparator<int>()
    ));
    registry.registerOperator("gt", std::unique_ptr<Comparator<int>>(
        new GtComparator<int>()
    ));
    registry.registerOperator("geq", std::unique_ptr<Comparator<int>>(
        new GeqComparator<int>()
    ));
    registry.registerOperator("lt", std::unique_ptr<Comparator<int>>(
        new LtComparator<int>()
    ));
    registry.registerOperator("leq", std::unique_ptr<Comparator<int>>(
        new LeqComparator<int>()
    ));

    return registry;
}


// enum
const std::unordered_map<std::string, sched_policy::PowerMode> power_mode_map = {
    { "balance", sched_policy::PowerMode::Balance },
    { "save", sched_policy::PowerMode::Save },
    { "performance", sched_policy::PowerMode::Performance }
};

const std::unordered_map<std::string, sched_policy::PowerType> power_type_map = {
    { "battery", sched_policy::PowerType::Battery },
    { "ac", sched_policy::PowerType::Ac }
};

const std::unordered_map<std::string, sched_policy::DeviceMode> device_mode_map = {
    { "pc", sched_policy::DeviceMode::PC },
    { "tablet", sched_policy::DeviceMode::Tablet }
};

const std::unordered_map<std::string, sched_policy::StatusMode> status_mode_map = {
    { "available", sched_policy::StatusMode::Available },
    { "invisible", sched_policy::StatusMode::Invisible },
    { "busy", sched_policy::StatusMode::Busy },
    { "idle", sched_policy::StatusMode::Idle }
};

const std::unordered_map<std::string, sched_policy::InputMode> input_mode_map = {
    { "inputidle", sched_policy::InputMode::InputIdle },
    { "inputbusy", sched_policy::InputMode::InputBusy }
};

const std::unordered_map<std::string, sched_policy::ScreenMode> screen_mode_map = {
    { "single_screen", sched_policy::ScreenMode::SINGLE_SCREEN },
    { "extend_screen", sched_policy::ScreenMode::EXTEND_SCREEN },
    { "copy_screen", sched_policy::ScreenMode::COPY_SCREEN },
    { "no_screen", sched_policy::ScreenMode::NO_SCREEN }
};

const std::unordered_map<std::string, sched_policy::DPMSMode> dpms_mode_map = {
    { "dpms_on", sched_policy::DPMSMode::DPMSOn },
    { "dpms_off", sched_policy::DPMSMode::DPMSOff },
    { "dpms_standby", sched_policy::DPMSMode::DPMSStandby },
    { "dpms_suspend", sched_policy::DPMSMode::DPMSSuspend }
};

const std::unordered_map<std::string, sched_policy::AppState> app_state_map = {
    { "focus", sched_policy::AppState::Focus },
    { "foreground", sched_policy::AppState::Foreground },
    { "background", sched_policy::AppState::Background },
    { "cached", sched_policy::AppState::Cached }
};

template <typename EnumType, typename StrToEnumMap>
inline std::pair<bool, bool> check_enum_condition(
    const std::string& keyStr,
    EnumType target,
    const StrToEnumMap& strToEnumMap,
    const std::string& operatorName,
    const OperatorRegistry<EnumType>& registry) 
{
    auto map_it = strToEnumMap.find(keyStr);
    if (map_it == strToEnumMap.end()) {
        return {false, false};
    }
    EnumType value = map_it->second;

    const Comparator<EnumType>* op = registry.findOperator(operatorName);
    if (!op) {
        return {false, false};
    }

    return {true, (*op)(target, value)};
}

std::pair<bool, bool> check_bool_condition(
    const std::string& keyStr,
    bool target,
    const std::string& operatorName,
    const OperatorRegistry<bool>& registry) 
{
    bool value;
    if (keyStr == "true") {
        value = true;
    } else if (keyStr == "false") {
        value = false;
    } else {
        return {false, false};
    }

    const Comparator<bool>* op = registry.findOperator(operatorName);
    if (!op) {
        return {false, false};
    }

    return {true, (*op)(target, value)};
}

std::pair<bool, bool> check_int_condition(
    const std::string& keyStr,
    int target,
    const std::string& operatorName,
    const OperatorRegistry<int>& registry) 
{
    int value = std::stoi(keyStr);

    const Comparator<int>* op = registry.findOperator(operatorName);
    if (!op) {
        return {false, false};
    }

    return {true, (*op)(target, value)};
}

std::unordered_map<std::string, std::function<bool(StaticSceneData &, const std::string&)>> obj_fun_map;

}   // !namespace


inline bool condion_target_check(ConditionTarget &condTarget, StaticSceneData &data)
{
    using namespace sched_policy;

    if (!condTarget.is_valid) {
        return true;
    }

    if (condTarget.category_ == "all"){
        return true;
    } else if (condTarget.category_ == "status_mode") {
        auto registry = MakeEnumOperatorRegistry<StatusMode>();
        if (condTarget.object_ == "last_status_mode") {
            auto result = check_enum_condition(condTarget.value_, data.last_status_mode, status_mode_map, condTarget.operator_, registry);
            return result.first && result.second;
        } else if (condTarget.object_ == "curr_status_mode") {
            auto result = check_enum_condition(condTarget.value_, data.curr_status_mode, status_mode_map, condTarget.operator_, registry);
            return result.first && result.second;
        }
    } else if (condTarget.category_ == "input_mode") {
        auto registry = MakeEnumOperatorRegistry<InputMode>();
        if (condTarget.object_ == "last_input_mode") {
            auto result = check_enum_condition(condTarget.value_, data.last_input_mode, input_mode_map, condTarget.operator_, registry);
            return result.first && result.second;
        } else if (condTarget.object_ == "curr_input_mode") {
            auto result = check_enum_condition(condTarget.value_, data.curr_input_mode, input_mode_map, condTarget.operator_, registry);
            return result.first && result.second;
        }
    } else if (condTarget.category_ == "power_mode") {
        auto registry = MakeEnumOperatorRegistry<PowerMode>();
        if (condTarget.object_ == "last_power_mode") {
            auto result = check_enum_condition(condTarget.value_, data.last_power_mode, power_mode_map, condTarget.operator_, registry);
            return result.first && result.second;
        } else if (condTarget.object_ == "curr_power_mode") {
            auto result = check_enum_condition(condTarget.value_, data.curr_power_mode, power_mode_map, condTarget.operator_, registry);
            return result.first && result.second;
        }
    } else if (condTarget.category_ == "power_type") {
        auto registry = MakeEnumOperatorRegistry<PowerType>();
        if (condTarget.object_ == "last_power_type") {
            auto result = check_enum_condition(condTarget.value_, data.last_power_type, power_type_map, condTarget.operator_, registry);
            return result.first && result.second;
        } else if (condTarget.object_ == "curr_power_type") {
            auto result = check_enum_condition(condTarget.value_, data.curr_power_type, power_type_map, condTarget.operator_, registry);
            return result.first && result.second;
        }
    } else if (condTarget.category_ == "power_level") {
        auto registry = MakeIntOperatorRegistry();
        if (condTarget.object_ == "last_power_level") {
            auto result = check_int_condition(condTarget.value_, data.last_power_level, condTarget.operator_, registry);
            return result.first && result.second;
        } else if (condTarget.object_ == "curr_power_level") {
            auto result = check_int_condition(condTarget.value_, data.curr_power_level, condTarget.operator_, registry);
            return result.first && result.second;
        }
    } else if (condTarget.category_ == "power_percentage") {
        auto registry = MakeIntOperatorRegistry();
        if (condTarget.object_ == "last_power_percentage") {
            auto result = check_int_condition(condTarget.value_, data.last_power_percentage, condTarget.operator_, registry);
            return result.first && result.second;
        } else if (condTarget.object_ == "curr_power_percentage") {
            auto result = check_int_condition(condTarget.value_, data.curr_power_percentage, condTarget.operator_, registry);
            return result.first && result.second;
        }
    } else if (condTarget.category_ == "device_mode") {
        auto registry = MakeEnumOperatorRegistry<DeviceMode>();
        if (condTarget.object_ == "last_device_mode") {
            auto result = check_enum_condition(condTarget.value_, data.last_device_mode, device_mode_map, condTarget.operator_, registry);
            return result.first && result.second;
        } else if (condTarget.object_ == "curr_device_mode") {
            auto result = check_enum_condition(condTarget.value_, data.real_device_mode, device_mode_map, condTarget.operator_, registry);
            return result.first && result.second;
        }
    } else if (condTarget.category_ == "screen_mode") {
        auto registry = MakeEnumOperatorRegistry<ScreenMode>();
        if (condTarget.object_ == "last_screen_mode") {
            auto result = check_enum_condition(condTarget.value_, data.last_screen_mode, screen_mode_map, condTarget.operator_, registry);
            return result.first && result.second;
        } else if (condTarget.object_ == "curr_screen_mode") {
            auto result = check_enum_condition(condTarget.value_, data.curr_screen_mode, screen_mode_map, condTarget.operator_, registry);
            return result.first && result.second;
        }
    } else if (condTarget.category_ == "process_info") {
        if (condTarget.object_ == "process_state_changed"){
            return obj_fun_map[condTarget.value_](data, condTarget.operator_);
        } else if (condTarget.object_ == "app_win_changed"){
            return obj_fun_map[condTarget.value_](data, condTarget.operator_);
        } else if (condTarget.object_ == "is_show_desktop"){
            auto registry = MakeBoolOperatorRegistry();
            auto result = check_bool_condition(condTarget.value_, data.is_show_desktop, condTarget.operator_, registry);
            return result.first && result.second;
        }
    } else if (condTarget.category_ == "dpms_mode") {
        auto registry = MakeEnumOperatorRegistry<DPMSMode>();
        if (condTarget.object_ == "last_dpms_mode") {
            auto result = check_enum_condition(condTarget.value_, data.last_dpms_mode, dpms_mode_map,
                                          condTarget.operator_, registry);
            return result.first && result.second;
        } else if (condTarget.object_ == "curr_dpms_mode") {
            auto result = check_enum_condition(condTarget.value_, data.curr_dpms_mode, dpms_mode_map,
                                          condTarget.operator_, registry);
            return result.first && result.second;
        }
    } else if (condTarget.category_ == "prepare_for_sleep") {
        auto registry = MakeBoolOperatorRegistry();
        auto result = check_bool_condition(condTarget.value_, data.is_prepare_for_sleep, condTarget.operator_, registry);
        return result.first && result.second;
    } else if (condTarget.category_ == "prepare_for_shutdown") {
        auto registry = MakeBoolOperatorRegistry();
        auto result = check_bool_condition(condTarget.value_, data.is_prepare_for_shutdown, condTarget.operator_, registry);
        return result.first && result.second;
    } else if (condTarget.category_ == "app_win_changed_spell") {
        bool result = data.app_win_changed_spell.count(condTarget.value_);
        data.app_win_changed_spell.erase(condTarget.value_);
        return result;
    } else if (condTarget.category_ == "app_focus_spell") {
        bool result = data.app_focus_spell.count(condTarget.value_);
        data.app_focus_spell.erase(condTarget.value_);
        return result;
    } else if (condTarget.category_ == "net_state") {
        if (condTarget.object_ == "udp_ports_num") {
            return obj_fun_map[condTarget.value_](data, condTarget.operator_);
        }
    } else if (condTarget.category_ == "cpu_state") {
        if (condTarget.object_ == "cpu_load_avg") {
            int loadAvg = system_state_helper::getLoadAvg();
            auto registry = MakeIntOperatorRegistry();
            auto result = check_int_condition(condTarget.value_, loadAvg, condTarget.operator_, registry);
            return result.first && result.second;
        }
    }

    return false;
}

namespace {
const std::unordered_map<std::string, StateTarget> category_map = {
    {"all", StateTarget::All},
    {"power_mode", StateTarget::PowerMode},
    {"power_type", StateTarget::PowerType},
    {"device_mode", StateTarget::DeviceMode},
    {"resource_limit", StateTarget::ResourceLimit},
    {"scene_manager", StateTarget::SceneManager},
    {"resource_scheduling", StateTarget::ResourceScheduling},
    {"app_white_list", StateTarget::AppWhiteList},
    {"top_app_list", StateTarget::TopAppList},
    {"status_mode", StateTarget::StatusMode},
    {"screen_mode", StateTarget::ScreenMode},
    {"process_info", StateTarget::ProcessInfo},
    {"dpms_mode", StateTarget::DPMSMode},
    {"prepare_for_sleep", StateTarget::PrepareForSleep},
    {"prepare_for_shutdown", StateTarget::PrepareForShutdown},
    {"app_focus_spell", StateTarget::AppFocusSpell},
    {"app_win_changed_spell", StateTarget::AppWinChangedSpell},
    {"power_level_changed", StateTarget::PowerLevelChanged}
};

inline StateTarget mapCategoryToTarget(const std::string& category) 
{
    auto it = category_map.find(category);
    if (it != category_map.end()) {
        return it->second;
    }
    PRINT_DBG("Unknown category: %s\n", category.c_str());
    return StateTarget::All;
}

#define SCENES_CONFIG_KEY "scenes_config"
#define SCENES_KEY "scenes"
#define SCENE_ID_KEY "scene_id"
#define SCENE_NAME_KEY "scene_name"
#define DESCRIPTION_KEY "description"
#define ENABLE_KEY "enable"
#define PRIORITY_KEY "priority"
#define MUTEX_GROUP_KEY "mutex_group"
#define DEPENDENCIES_KEY "dependencices"
#define ALLOWED_COEXIST_KEY "allowed_coexist"
#define ENTRY_EXIT_CONDITION_KEY "entry_exit_condition"
#define ENTRY_CONDITION_KEY "entry_condition"
#define EXIT_CONDITION_KEY "exit_condition"
#define CONDITION_KEY "condition"
#define SUBCONDITIONS_KEY "subconditions"
#define CONDITION_ID_KEY "condition_id"
#define CONDITION_TYPE_KEY "type"
#define CONDITION_TARGET_KEY "target"
#define CATEGORY_KEY "category"
#define OBJECT_KEY "object"
#define VALUE_KEY "value"
#define OPERATOR_KEY "operator"

extern std::unique_ptr<Condition> mapJsonToCondion(const Json::Value &cond);

inline RegistrProcessInfo parse_registr_process_info(const Json::Value &value){
    RegistrProcessInfo rpi;
    if (value.isMember("name")){
        rpi.name_ = value["name"].asString();
    } else {
        rpi.name_ = "-";
    }

    if (value.isMember("cmdline")){
        rpi.cmdline_ = value["cmdline"].asString();
    } else {
        rpi.cmdline_ = "-";
    }
    return rpi;
}

inline RegistrWinInfo parse_registr_win_info(const Json::Value &value){
    RegistrWinInfo rwi;
    if (value.isMember("title")){
        rwi.title_ = value["title"].asString();
    } else{
        rwi.title_ = "any";
    }

    if (value.isMember("state")){
        rwi.state_ = json_helper::parse_string_vector(value["state"]);
    }
    return rwi;
}

inline RegistrAppInfo parse_registr_app_info(const Json::Value &value){
    RegistrAppInfo rai;
    if (!value.isMember("process") || !value.isMember("window")) return rai;
    rai.process_ = parse_registr_process_info(value["process"]);
    rai.window_ = parse_registr_win_info(value["window"]);
    return rai;
}

inline std::string parse_value_object(const std::string &object, const Json::Value &value){
    std::string uuid = QUuid::createUuid().toString(QUuid::Id128).toStdString();
    if (object == "app_focus_spell"){
        if (value.isMember("process") && value.isMember("spell")){
            EventBus::instance()->notifyRegisterAppFocusSpellWatcher(uuid, parse_registr_process_info(value["process"]), value["spell"].asInt());
        }
    } else if (object == "app_win_changed_spell"){
        if (value.isMember("appinfo") && value.isMember("spell")){
            EventBus::instance()->notifyRegisterAppWinChangedSpellWatcher(uuid, parse_registr_app_info(value["appinfo"]), value["spell"].asInt());
        }
    } else if (object == "udp_ports_num"){
        if (value.isMember("process") && value.isMember("num")){
            std::string process = value["process"].asString();
            int num = value["num"].asInt();
            obj_fun_map[uuid] = [process, num](StaticSceneData &data, const std::string& oper){
                std::vector<int> pids = process_info_helper::pidsByName(process, false);
                if (!pids.empty()){
                    int realNum = process_info_helper::udpSocketCount(pids.front());
                    OperatorRegistry<int> registry = MakeIntOperatorRegistry();
                    const Comparator<int>* op = registry.findOperator(oper);
                    if (!op) {
                        return false;
                    }
                    return (*op)(num, realNum);
                }
                return false;
            };
        }
    } else if (object == "app_win_changed"){
        if (value.isMember("appinfo")){
            RegistrAppInfo ai = parse_registr_app_info(value["appinfo"]);
            obj_fun_map[uuid] = [ai](StaticSceneData &data, const std::string& oper){
                auto it = std::find_if(data.all_app_infos.begin(), data.all_app_infos.end(), [&ai](const std::pair<std::string, AppInfo> &appInfo){
                    if (ai.process_.name_ != "-"){
                        return appInfo.second.name().find(ai.process_.name_) != std::string::npos;
                    }
                    if (ai.process_.cmdline_ != "-"){
                        return appInfo.second.cmdline().find(ai.process_.cmdline_) != std::string::npos;
                    }
                    return false;
                });
                if (it != data.all_app_infos.end()){
                    bool result = false;
                    if (ai.window_.title_ == "any"){
                        for (const auto &winInfo : it->second.winInfos()){
                            result |= std::all_of(ai.window_.state_.begin(), ai.window_.state_.end(), [&winInfo](const std::string &s){
                                if (s == "keepabove"){
                                    return winInfo.second.is_keepabove;
                                } else if (s == "maximized"){
                                    return winInfo.second.is_maximized && !winInfo.second.is_fullscreen;
                                } else if (s == "fullscreen"){
                                    return winInfo.second.is_fullscreen;
                                } else if (s == "minimized"){
                                    return winInfo.second.is_minimized;
                                } else if (s == "active"){
                                    return winInfo.second.is_active;
                                }
                                return false;
                            });                            
                        }
                    }
                    OperatorRegistry<bool> registry = MakeBoolOperatorRegistry();
                    const Comparator<bool>* op = registry.findOperator(oper);
                    if (!op) {
                        return false;
                    }
                    return (*op)(true, result);
                } else{
                    return false;
                } 
                return false;
            };
        }
    } else if (object == "process_state_changed"){
        if (value.isMember("process") && value.isMember("state")){
            RegistrProcessInfo ai = parse_registr_process_info(value["process"]);
            auto it = app_state_map.find(value["state"].asString());
            sched_policy::AppState appState = sched_policy::AppState::Foreground;
            if (it != app_state_map.end()){
                appState = it->second;
            }
            obj_fun_map[uuid] = [ai, appState](StaticSceneData &data, const std::string& oper){
                auto it = std::find_if(data.all_app_infos.begin(), data.all_app_infos.end(), [&data, &ai, &appState, &oper](const std::pair<std::string, AppInfo> &appInfo){
                    if (ai.name_ != "-"){
                        return appInfo.second.name().find(ai.name_) != std::string::npos;
                    }
                    if (ai.cmdline_ != "-"){
                        return appInfo.second.cmdline().find(ai.cmdline_) != std::string::npos;
                    }
                    return false;
                });
                if (it != data.all_app_infos.end()){
                    OperatorRegistry<sched_policy::AppState> registry = MakeEnumOperatorRegistry<sched_policy::AppState>();
                    const Comparator<sched_policy::AppState>* op = registry.findOperator(oper);
                    if (!op) {
                        return false;
                    }
                    return (*op)(it->second.appState(), appState); 
                }
                else
                {
                    OperatorRegistry<bool> registry = MakeBoolOperatorRegistry();
                    const Comparator<bool>* op = registry.findOperator(oper);
                    if (!op) {
                        return false;
                    }
                    return (*op)(true, false);
                }
            };
        }
    }

    PRINT_DBG("Condition target value object is invalid\n");
    return uuid;
}

inline std::unique_ptr<Condition> parse_equal_condition(const Json::Value &condTarget){
    if (!condTarget.isMember(CATEGORY_KEY) || 
        !condTarget.isMember(OBJECT_KEY) ||
        !condTarget.isMember(VALUE_KEY) || 
        !condTarget.isMember(OPERATOR_KEY)) 
    {
        PRINT_NOTICE("Condition target does not have required keys\n");
        return nullptr;
    }
    std::string category = condTarget[CATEGORY_KEY].asString();
    std::string object = condTarget[OBJECT_KEY].asString();
    std::string value;
    if (condTarget[VALUE_KEY].isObject()){
        value = parse_value_object(object, condTarget[VALUE_KEY]);
    } else {
        value = condTarget[VALUE_KEY].asString();
    }
    std::string oper = condTarget[OPERATOR_KEY].asString();
    StateTarget stateTarget = mapCategoryToTarget(category);
    return std::unique_ptr<Condition>(new TargetEqualCondition(stateTarget, { true, category, object, value, oper }));
}

inline std::unique_ptr<Condition> parse_in_condition(const Json::Value &condTarget){
    std::unordered_map<StateTarget, ConditionTarget> values;
    for (const Json::Value &ct : condTarget)
    {
        if (!ct.isMember(CATEGORY_KEY) || 
            !ct.isMember(OBJECT_KEY) || 
            !ct.isMember(VALUE_KEY) || 
            !ct.isMember(OPERATOR_KEY)) 
        {
            PRINT_NOTICE("Condition target does not have required keys\n");
            continue;
        }
        std::string category = ct[CATEGORY_KEY].asString();
        std::string object = condTarget[OBJECT_KEY].asString();
        std::string value;
        if (condTarget[VALUE_KEY].isObject()){
            value = parse_value_object(object, condTarget[VALUE_KEY]);
        } else {
            value = condTarget[VALUE_KEY].asString();
        }
        std::string oper = ct[OPERATOR_KEY].asString();
        StateTarget stateTarget = mapCategoryToTarget(category);
        values[stateTarget] = { true, category, object, value, oper };
    }
    return std::unique_ptr<Condition>(new TargetInCondition(values));
}

inline std::unique_ptr<Condition> parse_not_condition(const Json::Value &condTarget){
    if (!condTarget.isMember(CONDITION_KEY)) {
        PRINT_NOTICE("Condition target does not have a condition key\n");
        return nullptr;
    }
    const Json::Value &cond = condTarget[CONDITION_KEY];
    std::unique_ptr<Condition> conPtr = mapJsonToCondion(cond);

    return std::unique_ptr<Condition>(new NotCondition(std::move(conPtr)));
}

inline std::unique_ptr<Condition> parse_and_condition(const Json::Value &condTarget){
    std::vector<std::unique_ptr<Condition>> values;
    for (const Json::Value &ct : condTarget)
    {
        if (!ct.isMember(CONDITION_KEY)) {
            PRINT_NOTICE("Condition target does not have a condition key\n");
            continue;
        }
        const Json::Value &cond = ct[CONDITION_KEY];
        std::unique_ptr<Condition> conPtr = mapJsonToCondion(cond);
        values.push_back(std::move(conPtr));
    }
    return std::unique_ptr<Condition>(new AndCondition(std::move(values)));
}

inline std::unique_ptr<Condition> parse_or_condition(const Json::Value &condTarget){
    std::vector<std::unique_ptr<Condition>> values;
    for (const Json::Value &ct : condTarget)
    {
        if (!ct.isMember(CONDITION_KEY)) {
            PRINT_NOTICE("Condition target does not have a condition key\n");
            continue;
        }
        const Json::Value &cond = ct[CONDITION_KEY];
        std::unique_ptr<Condition> conPtr = mapJsonToCondion(cond);
        values.push_back(std::move(conPtr));
    }
    return std::unique_ptr<Condition>(new OrCondition(std::move(values)));
}

const std::unordered_map<std::string, std::function<std::unique_ptr<Condition>(const Json::Value &)>> cond_map = {
    {"equal_condition", parse_equal_condition},
    {"in_condition", parse_in_condition},
    {"not_condition", parse_not_condition},
    {"and_condition", parse_and_condition},
    {"or_condition", parse_or_condition}
};

inline std::unique_ptr<Condition> mapJsonToCondion(const Json::Value &cond)
{
    if (!cond.isMember(CONDITION_TYPE_KEY))    return nullptr;
    std::string condType = cond[CONDITION_TYPE_KEY].asString();
    auto it = cond_map.find(condType);
    if (it != cond_map.end()){
        if (!cond.isMember(CONDITION_TARGET_KEY)) {
            return nullptr;
        }
        return it->second(cond[CONDITION_TARGET_KEY]);
    }
    PRINT_NOTICE("Unknown condtype: %s\n", condType.c_str());
    return nullptr;
}

inline EntryExitCondition mapJsonToEntryExitCondition(const Json::Value &cond)
{
    EntryExitCondition ret;
    if (!cond.isMember(ENTRY_CONDITION_KEY)) return ret;

    const Json::Value &entryCond = cond[ENTRY_CONDITION_KEY];
    if (entryCond.isMember(CONDITION_KEY))
    {
        ret.entry_condition = std::move(mapJsonToCondion(entryCond[CONDITION_KEY]));
    }
    
    if (!cond.isMember(EXIT_CONDITION_KEY)) return ret;
    const Json::Value &exitCond = cond[EXIT_CONDITION_KEY];
    if (exitCond.isMember(CONDITION_KEY))
    {
        ret.exit_condition = std::move(mapJsonToCondion(exitCond[CONDITION_KEY]));
    }
    return ret;
}

inline std::unordered_map<std::string, EntryExitCondition> mapJsonToSubCondition(const Json::Value &subConditions)
{
    std::unordered_map<std::string, EntryExitCondition> ret;
    if (!subConditions.isArray())   return ret;
    for (const Json::Value &subCondition : subConditions){
        if (!subCondition.isMember(CONDITION_ID_KEY) || !subCondition.isMember(ENTRY_EXIT_CONDITION_KEY)){
            PRINT_NOTICE("Unknown subcondition define\n");
            continue;
        }
        if (subCondition.isMember(ENABLE_KEY) && !subCondition[ENABLE_KEY].asBool()){
            continue;
        }
        std::string condId = subCondition[CONDITION_ID_KEY].asString();
        const Json::Value &cond = subCondition[ENTRY_EXIT_CONDITION_KEY];

        ret.insert({condId, std::move(mapJsonToEntryExitCondition(cond))});
    }
    return ret;
}

} // namespace

#define SCENE_ID_EXAMPLE "example"

std::vector<SceneRule> SceneRuleParser::parse() 
{
    std::vector<SceneRule> rules;
    if (!m_sceneRules.isMember(SCENES_CONFIG_KEY))    return rules;
    const Json::Value &scenesConfig = m_sceneRules[SCENES_CONFIG_KEY];
    if (!scenesConfig.isMember(SCENES_KEY))    return rules;
    const Json::Value &scenes = scenesConfig[SCENES_KEY];
    
    for (const Json::Value& scene : scenes) {
        if (!scene.isMember(SCENE_ID_KEY)){
            PRINT_NOTICE("Unknown scene define\n");
            continue;
        }
        SceneRule rule;
        rule.scene_id = scene[SCENE_ID_KEY].asString();
        if (rule.scene_id == SCENE_ID_EXAMPLE) {
            continue;
        }
        
        if (scene.isMember(ENABLE_KEY)){
            rule.enable_ = scene[ENABLE_KEY].asBool();
        } else {
            rule.enable_ = true;
        }

        if (scene.isMember(PRIORITY_KEY)){
            rule.priority_ = scene[PRIORITY_KEY].asInt();
        } else {
            rule.priority_ = 0;
        }

        if (scene.isMember(MUTEX_GROUP_KEY)){
            rule.mutex_group = std::move(json_helper::parse_string_vector(scene[MUTEX_GROUP_KEY]));
        }
        
        if (scene.isMember(DEPENDENCIES_KEY)){
            rule.dependencices_ = std::move(json_helper::parse_string_vector(scene[DEPENDENCIES_KEY]));
        }

        if (scene.isMember(ALLOWED_COEXIST_KEY)){
            rule.allowed_coexist= std::move(json_helper::parse_string_vector(scene[ALLOWED_COEXIST_KEY]));
        }

        if (scene.isMember(ENTRY_EXIT_CONDITION_KEY))
        {
            const Json::Value &entryExitCondJson = scene[ENTRY_EXIT_CONDITION_KEY];
            rule.entry_exit_condition = std::move(mapJsonToEntryExitCondition(entryExitCondJson));
        }

        if (scene.isMember(SUBCONDITIONS_KEY)){
            const Json::Value subConditions = scene[SUBCONDITIONS_KEY];

            rule.subconditions_ = std::move(mapJsonToSubCondition(subConditions));
        }
        
        rules.emplace_back(std::move(rule));
    }
    
    return rules;
}