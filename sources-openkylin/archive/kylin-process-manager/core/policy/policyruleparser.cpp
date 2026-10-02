#include "policyruleparser.h"
#include "appinfohelper.h"
#include "devicestatehelper.h"
#include "jsonhelper.h"
#include "loghelper.h"
#include "misc.h"
#include "processinfohelper.h"
#include <QFile>
#include <QDebug>

static std::shared_ptr<GroupManager> g_group_manager = nullptr;
static std::shared_ptr<ConfigManager> g_configManager = nullptr;
static std::shared_ptr<SystemStateManager> g_systemStateManager = nullptr;

void register_group_manager(std::shared_ptr<GroupManager> groupManager)
{
    g_group_manager = groupManager;
}

void register_config_manager(std::shared_ptr<ConfigManager> configManager)
{
    g_configManager = configManager;
}

void register_systemstate_manager(std::shared_ptr<SystemStateManager> systemStateManager)
{
    g_systemStateManager = systemStateManager;
}

namespace {

struct ProcessInfo {
    std::string name_;
    std::vector<std::string> windows_;
    std::string type_;
};

struct ServiceInfo {
    std::string service_name;
    int power_level;
    int power_mode;
    int recovery_level;
    int resource_limit_cpu;
    int resource_limit_mem;
    int resource_limit_io;
    int privilege;
    int action;
};

static std::unordered_map<std::string, ProcessInfo> g_processInfoList;
static std::unordered_map<std::string, ServiceInfo> g_serviceInfoList;

std::unique_ptr<Handler> parse_freeze_handler(const Json::Value &handlerJson) {
    if (!handlerJson.isMember("freeze_list") || !handlerJson["freeze_list"].isArray()) {
        PRINT_NOTICE("freeze_handler missing or invalid freeze_list\n");
        return nullptr;
    }
    
    auto freezeList = json_helper::parse_string_vector(handlerJson["freeze_list"]);
    return std::unique_ptr<Handler>(new FreezeHandler(freezeList));
}
std::unique_ptr<Handler> parse_memdirty_handler(const Json::Value &handlerJson) {
    if (!handlerJson.isMember("memdirty_list") || !handlerJson["memdirty_list"].isArray()) {
        PRINT_NOTICE("memdirty_handler missing or invalid memdirty_list\n");
        return nullptr;
    }
    
    auto memdirtyList = json_helper::parse_string_vector(handlerJson["memdirty_list"]);
    return std::unique_ptr<Handler>(new MemdirtyHandler(memdirtyList));
}
std::unique_ptr<Handler> parse_kernelsched_handler(const Json::Value &handlerJson) {
    if (!handlerJson.isMember("kernelsched_list") || !handlerJson["kernelsched_list"].isArray()) {
        PRINT_NOTICE("kernelsched_list_handler missing or invalid kernelsched_list\n");
        return nullptr;
    }
    
    auto kernelschedList = json_helper::parse_string_vector(handlerJson["kernelsched_list"]);
    return std::unique_ptr<Handler>(new KernelschedHandler(kernelschedList));
}
std::unique_ptr<Handler> parse_netparams_handler(const Json::Value &handlerJson) {
    if (!handlerJson.isMember("netparams_list") || !handlerJson["netparams_list"].isArray()) {
        PRINT_NOTICE("netparams_handler missing or invalid netparams_list\n");
        return nullptr;
    }
    
    auto netparamsList = json_helper::parse_string_vector(handlerJson["netparams_list"]);
    return std::unique_ptr<Handler>(new NetparamsHandler(netparamsList));
}
std::unique_ptr<Handler> parse_samplingrate_handler(const Json::Value &handlerJson) {
    if (!handlerJson.isMember("samplingrate_list") || !handlerJson["samplingrate_list"].isArray()) {
        PRINT_NOTICE("samplingrate_handler missing or invalid samplingrate_list\n");
        return nullptr;
    }
    
    auto samplingrateList = json_helper::parse_string_vector(handlerJson["samplingrate_list"]);
    return std::unique_ptr<Handler>(new SamplingrateHandler(samplingrateList));
}
std::unique_ptr<Handler> parse_cpulimit_handler(const Json::Value &handlerJson) {
    if (!handlerJson.isMember("cpulimit_list") || !handlerJson["cpulimit_list"].isArray()) {
        PRINT_NOTICE("cpulimit_handler missing or invalid cpulimit_list\n");
        return nullptr;
    }
    
    auto cpulimitList = json_helper::parse_string_vector(handlerJson["cpulimit_list"]);
    return std::unique_ptr<Handler>(new CpulimitHandler(cpulimitList));
}
std::unique_ptr<Handler> parse_brightness_handler(const Json::Value &handlerJson) {
    if (!handlerJson.isMember("brightness_list") || !handlerJson["brightness_list"].isArray()) {
        PRINT_NOTICE("parse_brightness_handler missing or invalid brightness_list\n");
        return nullptr;
    }
    
    auto brightnessList = json_helper::parse_string_vector(handlerJson["brightness_list"]);
    return std::unique_ptr<Handler>(new BrightnessHandler(brightnessList));
}
std::unique_ptr<Handler> parse_PSRState_handler(const Json::Value &handlerJson) {
    return std::unique_ptr<Handler>(new PSRStateHandler());
}
std::unique_ptr<Handler> parse_VRRState_handler(const Json::Value &handlerJson) {
    if (!handlerJson.isMember("power_level_policy") || !handlerJson["power_level_policy"].isObject()) {
        PRINT_NOTICE("parse_VRRState_handler missing or invalid freeze_list\n");
        return nullptr;
    }

    const Json::Value policyJsons = handlerJson["power_level_policy"];

    std::unordered_map<std::string, std::string> powerPolicyMap;
    for(const auto & policyItem : policyJsons.getMemberNames()) {
        powerPolicyMap[policyItem] = policyJsons[policyItem].asString();
        PRINT_NOTICE("parse_VRRState_handler : power: %s, policy: %s\n", policyItem.c_str(), policyJsons[policyItem].asString().c_str());
    }

    return std::unique_ptr<Handler>(new VRRStateHandler(powerPolicyMap));
}

std::unique_ptr<Handler> parse_effects_handler(const Json::Value &handlerJson) {
    if (!handlerJson.isMember("filter_list") || !handlerJson["filter_list"].isArray()) {
        PRINT_NOTICE("parse_effects_handler missing or invalid freeze_list\n");
        return nullptr;
    }
    
    auto filterList = json_helper::parse_string_vector(handlerJson["filter_list"]);
    return std::unique_ptr<Handler>(new EffectsHandler(filterList));
}

std::unique_ptr<Handler> parse_servicefreeze_handler(const Json::Value &handlerJson) {
    if (!handlerJson.isMember("freeze_list") || !handlerJson["freeze_list"].isArray()) {
        PRINT_NOTICE("servicefreeze_handler missing or invalid freeze_list\n");
        return nullptr;
    }
    
    auto freezeList = json_helper::parse_string_vector(handlerJson["freeze_list"]);
    return std::unique_ptr<Handler>(new ServiceFreezeHandler(freezeList));
}

std::unique_ptr<Handler> parse_servicestop_handler(const Json::Value &handlerJson) {
    if (!handlerJson.isMember("stop_list") || !handlerJson["stop_list"].isArray()) {
        PRINT_NOTICE("servicestop_handler missing or invalid freeze_list\n");
        return nullptr;
    }
    
    auto stopList = json_helper::parse_string_vector(handlerJson["stop_list"]);
    return std::unique_ptr<Handler>(new ServiceStopHandler(stopList));
}

const std::unordered_map<std::string, std::function<std::unique_ptr<Handler>(const Json::Value &)>> handler_map = {
    {"freeze_handler", parse_freeze_handler},
    {"memdirty_handler",parse_memdirty_handler},
    {"kernelsched_handler",parse_kernelsched_handler},
    {"netparams_handler",parse_netparams_handler},
    {"samplingrate_handler",parse_samplingrate_handler},
    {"cpulimit_handler",parse_cpulimit_handler},
    {"PSRState_handler", parse_PSRState_handler},
    {"VRRState_handler", parse_VRRState_handler},
    {"effects_handler", parse_effects_handler},
    {"servicefreeze_handler", parse_servicefreeze_handler},
    {"servicestop_handler", parse_servicestop_handler},
    {"brightness_handler", parse_brightness_handler}
};

#define HANDLER_TYPE_KEY "type"
#define HANDLER_NEED_REPEAT_KEY "need_repeat"

inline std::unique_ptr<Handler> parse_handler(const Json::Value &handlerJson) {
    if (!handlerJson.isMember(HANDLER_TYPE_KEY)) {
        PRINT_NOTICE("Handler missing or invalid type\n");
        return nullptr;
    }
    std::string type = handlerJson[HANDLER_TYPE_KEY].asString();
    auto it = handler_map.find(type);
    if (it != handler_map.end()) {
        std::unique_ptr<Handler> handler = it->second(handlerJson);
        if (handlerJson.isMember(HANDLER_NEED_REPEAT_KEY)){
            handler->setNeedRepeat(handlerJson[HANDLER_NEED_REPEAT_KEY].asBool());
        }
        return std::move(handler);
    }
    // Add more handler types as needed
    PRINT_NOTICE("Unknown handler type: %s\n", type.c_str());
    return nullptr;
}

#define PROCESS_INFO_NAME_KEY "name"
#define PROCESS_INFO_WINDOWS_KEY "windows"
#define PROCESS_INFO_TYPE_KEY "type"

inline void parse_process_info(const Json::Value &processInfo)
{
    for (const Json::Value &procInfo : processInfo){
        if (!procInfo.isMember(PROCESS_INFO_NAME_KEY) ||
            !procInfo.isMember(PROCESS_INFO_WINDOWS_KEY) ||
            !procInfo.isMember(PROCESS_INFO_TYPE_KEY)) 
        {
            PRINT_NOTICE("Handler missing or invalid type\n");
            return;
        }
        g_processInfoList[procInfo[PROCESS_INFO_NAME_KEY].asString()] = {
            procInfo[PROCESS_INFO_NAME_KEY].asString(), 
            json_helper::parse_string_vector(procInfo[PROCESS_INFO_WINDOWS_KEY]), 
            procInfo[PROCESS_INFO_TYPE_KEY].asString()
        };
    }
}

#define SERVICE_INFO_NAME_KEY "Service"
#define SERVICE_INFO_POWER_LEVEL_KEY "Power_level"
#define SERVICE_INFO_POWER_MODE_KEY "Power_mode"
#define SERVICE_INFO_RECOVERY_LEVEL_KEY "Recovery_level"
#define SERVICE_INFO_RESOURCE_LIMIT_CPU_KEY "Resource_limit_cpu"
#define SERVICE_INFO_RESOURCE_LIMIT_MEM_KEY "Resource_limit_mem"
#define SERVICE_INFO_RESOURCE_LIMIT_IO_KEY "Resource_limit_io"
#define SERVICE_INFO_PRIVILEGE_KEY "Privilege"
#define SERVICE_INFO_ACTION_KEY "Action"

inline void parse_service_info(const Json::Value &serviceInfo)
{
    for (const Json::Value &serviceItem : serviceInfo){
        if (!serviceItem.isMember(SERVICE_INFO_NAME_KEY) ||
            !serviceItem.isMember(SERVICE_INFO_POWER_LEVEL_KEY) ||
            !serviceItem.isMember(SERVICE_INFO_POWER_MODE_KEY) ||
            !serviceItem.isMember(SERVICE_INFO_RECOVERY_LEVEL_KEY) ||
            !serviceItem.isMember(SERVICE_INFO_RESOURCE_LIMIT_CPU_KEY) ||
            !serviceItem.isMember(SERVICE_INFO_RESOURCE_LIMIT_MEM_KEY) ||
            !serviceItem.isMember(SERVICE_INFO_RESOURCE_LIMIT_IO_KEY) ||
            !serviceItem.isMember(SERVICE_INFO_PRIVILEGE_KEY) ||
            !serviceItem.isMember(SERVICE_INFO_ACTION_KEY)) 

        {
            PRINT_NOTICE("parse_service_info missing member\n");
            return;
        }

        g_serviceInfoList[serviceItem[SERVICE_INFO_NAME_KEY].asString()] = {
            serviceItem[SERVICE_INFO_NAME_KEY].asString(), 
            serviceItem[SERVICE_INFO_POWER_LEVEL_KEY].asInt(),
            serviceItem[SERVICE_INFO_POWER_MODE_KEY].asInt(),
            serviceItem[SERVICE_INFO_RECOVERY_LEVEL_KEY].asInt(),
            serviceItem[SERVICE_INFO_RESOURCE_LIMIT_CPU_KEY].asInt(),
            serviceItem[SERVICE_INFO_RESOURCE_LIMIT_MEM_KEY].asInt(),
            serviceItem[SERVICE_INFO_RESOURCE_LIMIT_IO_KEY].asInt(),
            serviceItem[SERVICE_INFO_PRIVILEGE_KEY].asInt(),
            serviceItem[SERVICE_INFO_ACTION_KEY].asInt()
        };
    }
}



inline bool find_windows_is_active(const std::vector<std::string>& windows, const std::unordered_map<std::string, AppInfo> &allAppInfos)
{
    return std::find_if(allAppInfos.begin(), allAppInfos.end(), [&windows](const std::pair<std::string, AppInfo> &appInfo){
        if (appInfo.second.isSystemTrayIconApp()) return false;
        for (const std::string& window : windows){
            if (appinfo_helper::hasSameDesktopName(appInfo.second.desktopFile(), window)){
                return true;
            }
        }
        return false;
    }) != allAppInfos.end();
}

} // !namespace


void FreezeHandler::handle(const StaticSceneData& data)
{
    if (!g_group_manager) {
        PRINT_NOTICE("GroupManager is not registered.\n");
        return;
    }

    std::unordered_map<std::string, std::vector<int>> pids = process_info_helper::pidsByNames(m_freezeList);

    for (const auto &pid : pids){
        std::unordered_map<std::string, ProcessInfo>::iterator it = g_processInfoList.find(pid.first);
        if (it == g_processInfoList.end())  continue;
        if (!it->second.windows_.empty()){
            if (find_windows_is_active(it->second.windows_, data.all_app_infos)){
                continue;
            }
        }
        std::string name = json_helper::remove_chars_copy(pid.first);

        for (int p : pid.second) {
            int index = m_cgroupControlInterface.freezeProcess(p);
            if (index != -1) {
                m_freezeIndex.push_back(index);
            }
        }
    }
}

void FreezeHandler::reverse()
{
    for (int index : m_freezeIndex) {
        m_cgroupControlInterface.releaseLimitByIndex(index);
    }
    m_freezeIndex.clear();
}
#define START true
#define STOP  false
#define FREEZE true
#define THAWD false

void ServiceFreezeHandler::handle(const StaticSceneData &data)
{
    int curr_power_mode_weight = devicestate_helper::getCurrentPowerModeWeight(data.curr_power_mode);
    PRINT_DBG("ServiceFreezeHandler::handle: curr_power_level: %d, curr_power_mode_weight %d\n", data.curr_power_level, curr_power_mode_weight);
    for (const auto &freezeService : m_freezeList) {
        auto serviceItem = g_serviceInfoList.find(freezeService);
        if (serviceItem == g_serviceInfoList.end())
            continue;

        if(serviceItem->second.power_level >= data.curr_power_level || serviceItem->second.power_mode >= curr_power_mode_weight) {
            auto it = m_groupFreezeNames.find(serviceItem->first);
            if (it != m_groupFreezeNames.end()) {
                PRINT_DBG("ServiceFreezeHandler::handle(freeze)----service: %s has been freeze before\n", serviceItem->first.c_str());
                continue;
            }

            if(serviceItem->second.privilege == 1) {
                int pid = m_resourceManager.getSystemdUnitMainpid(serviceItem->first, true);
                if (pid > 0) {
                    int index = m_cgroupControlInterface.freezeProcess(pid);
                    if (index != -1) {
                        m_freezeIndex.push_back(index);
                        m_groupFreezeNames[serviceItem->first] = index;
                        PRINT_DBG("ServiceFreezeHandler::handle:FREEZE SERVICE: %s\n", serviceItem->first.c_str());
                    }
                }
            } else {
                int pid = m_resourceManager.getSystemdUnitMainpid(serviceItem->first, false);
                if (pid > 0) {
                    int index = m_cgroupControlInterface.freezeProcess(pid);
                    if (index != -1) {
                        m_freezeIndex.push_back(index);
                        m_groupFreezeNames[serviceItem->first] = index;
                        PRINT_DBG("ServiceFreezeHandler::handle:FREEZE SERVICE: %s\n", serviceItem->first.c_str());
                    }
                }
            }
        } else {

            auto it = m_groupFreezeNames.find(serviceItem->first);
            if (it == m_groupFreezeNames.end()) {
                PRINT_DBG("ServiceFreezeHandler::handle(thawd)----service: %s has not been freeze before\n", serviceItem->first.c_str());
                continue;
            }

            m_cgroupControlInterface.releaseLimitByIndex(it->second);
            m_groupFreezeNames.erase(it);
        }
    }
}

void ServiceFreezeHandler::reverse() {
    PRINT_DBG("ServiceFreezeHandler::reverse\n");

    for (const auto &freezeService : m_freezeList) {

        auto it = m_groupFreezeNames.find(freezeService);
        if (it == m_groupFreezeNames.end()) {
            continue;
        }
        m_cgroupControlInterface.releaseLimitByIndex(it->second);
    }

    m_groupFreezeNames.clear();
    std::unordered_map<std::string, int>().swap(m_groupFreezeNames);
}

void ServiceStopHandler::handle(const StaticSceneData &data)
{
    int curr_power_mode_weight = devicestate_helper::getCurrentPowerModeWeight(data.curr_power_mode);
    PRINT_DBG("ServiceStopHandler::handle: curr_power_level: %d, curr_power_mode_weight %d\n", data.curr_power_level, curr_power_mode_weight);
    for(const auto& stopService : m_stopList) {
        auto serviceItem = g_serviceInfoList.find(stopService);
        if (serviceItem == g_serviceInfoList.end())
            continue;

        if(serviceItem->second.power_level >= data.curr_power_level || serviceItem->second.power_mode >= curr_power_mode_weight) {
            auto it = std::find(m_groupStopServices.begin(), m_groupStopServices.end(), serviceItem->first);
            if(it != m_groupStopServices.end()) {
                PRINT_DBG("ServiceStopHandler::handle(stop)----service %s has been stopped before\n", serviceItem->first.c_str());
                continue;
            }

            if(serviceItem->second.privilege == 1) {
                m_resourceManager.setSystemdUnitRun(serviceItem->first, STOP, true);
                m_groupStopServices.push_back(serviceItem->first);
                PRINT_DBG("ServiceStopHandler::handle----STOP root SERVICE: %s\n", serviceItem->first.c_str());
            } else {
                m_resourceManager.setSystemdUnitRun(serviceItem->first, STOP, false); 
                m_groupStopServices.push_back(serviceItem->first);
                PRINT_DBG("ServiceStopHandler::handle----STOP user SERVICE: %s\n", serviceItem->first.c_str());
            }

        } else {
            auto it = std::find(m_groupStopServices.begin(), m_groupStopServices.end(), serviceItem->first);
            if(it == m_groupStopServices.end()) {
                PRINT_DBG("ServiceStopHandler::handle(start)----service %s has not been stopped before\n", serviceItem->first.c_str());
                continue;
            }

            if(serviceItem->second.privilege == 1) {
                m_resourceManager.setSystemdUnitRun(serviceItem->first, START, true);
                PRINT_DBG("ServiceStopHandler::handle Start root Service %s\n", serviceItem->first.c_str());
                m_groupStopServices.erase(it);
            } else {
                m_resourceManager.setSystemdUnitRun(serviceItem->first, START, false); 
                m_groupStopServices.erase(it);
                PRINT_DBG("ServiceStopHandler::handle Start user Service %s\n", serviceItem->first.c_str());
            }
        }
    }

}

void ServiceStopHandler::reverse()
{
    PRINT_DBG("ServiceStopHandler::reverse\n");

    for(const auto& stopService : m_stopList) {
        auto it = std::find(m_groupStopServices.begin(), m_groupStopServices.end(), stopService);
        if(it == m_groupStopServices.end()) {
            continue;
        }
        m_resourceManager.setSystemdUnitRun(stopService, START, (g_serviceInfoList[stopService].privilege == 1) ? true : false);
        PRINT_DBG("ServiceStopHandler::reverse() Start Service %s\n", stopService.c_str());
    }

    m_groupStopServices.clear();
    std::list<std::string>().swap(m_groupStopServices);

}

void MemdirtyHandler::handle(const StaticSceneData& data)
{
    for(const auto& str: m_memdirtyList)
    {
        PRINT_DBG("MemdirtyHandler::handle -- %s\n", str.c_str());
    }
    m_resourceManager.SetDirty(1);
}
void MemdirtyHandler::reverse()
{
    m_resourceManager.SetDirty(0);
}

void KernelschedHandler::handle(const StaticSceneData& data)
{
    for(const auto& str: m_kernelschedList)
    {
        PRINT_DBG("KernelschedHandler::handle -- %s\n", str.c_str());
    }
    m_resourceManager.SetKernelSched(1);
}
void KernelschedHandler::reverse()
{
    m_resourceManager.SetKernelSched(0);
}

void NetparamsHandler::handle(const StaticSceneData& data)
{
    for(const auto& str: m_netparamsList)
    {
        PRINT_DBG("NetparamsHandler::handle -- %s\n", str.c_str());
    }
    m_resourceManager.SetNetParams(1);
}

void NetparamsHandler::reverse()
{
    PRINT_DBG("NetparamsHandler:: -- reverse\n");
    m_resourceManager.SetNetParams(0);
}

void SamplingrateHandler::handle(const StaticSceneData& data)
{
    for(const auto& str: m_samplingrateList)
    {
        PRINT_DBG("SamplingrateHandler::handle -- %s\n", str.c_str());
    }
    m_resourceManager.SetSamplingRate(1);
}
void SamplingrateHandler::reverse()
{
    m_resourceManager.SetSamplingRate(0);
}

void CpulimitHandler::handle(const StaticSceneData& data)
{
    for(const auto& str: m_cpulimitList)
    {
        PRINT_DBG("CpulimitHandler::handle -- %s\n", str.c_str());
    }
    m_resourceManager.SetCpuLimit(1);
}
void CpulimitHandler::reverse()
{
    m_resourceManager.SetCpuLimit(0);
}
void BrightnessHandler::handle(const StaticSceneData& data)
{
    const QByteArray schemaId1 = "org.ukui.process-manager";
    if (QGSettings::isSchemaInstalled(schemaId1)) {
        QGSettings *settings = new QGSettings(schemaId1, nullptr);
        bool value = settings->get("lowpower-brightness-enable").toBool();
        if (value == false)
        {
            m_oldBrightnessValue = 0;
            return;
        }
    }
    else
    {
        m_oldBrightnessValue = 0;
        return;
    }

    const QByteArray schemaId = "org.ukui.power-manager";
    if (QGSettings::isSchemaInstalled(schemaId)) {
        QGSettings *settings = new QGSettings(schemaId, nullptr);
        QString value = settings->get("brightness-ac").toString();
        m_oldBrightnessValue = value.toInt();
        PRINT_DBG("BrightnessHandler::brightness-ac -- %s\n", value.toLocal8Bit().constData());
        int newBrightnessValue = int(m_oldBrightnessValue * 0.7);
        settings->set("brightness-ac", newBrightnessValue);
    }
}
void BrightnessHandler::reverse()
{
    if(m_oldBrightnessValue == 0)
    {
        return ;
    }
    const QByteArray schemaId = "org.ukui.power-manager";
    if (QGSettings::isSchemaInstalled(schemaId)) {
        QGSettings *settings = new QGSettings(schemaId, nullptr);
        settings->set("brightness-ac", m_oldBrightnessValue);
    }
}
#define PROCESS_INFO_KEY "process_info"
#define SERVICE_INFO_KEY "service_info"
#define POLICIES_CONFIG_KEY "policys_config"
#define POLICIES_KEY "policys"
#define SCENE_ID_KEY "scene_id"
#define HANDLERS_KEY "handlers"
#define HANDLER_ENABLE_KEY "enable"
#define SUBHANDLES_KEY "subhandles"
#define CONDITION_ID_KEY "condition_id"
#define SCENE_ID_EXAMPLE "example"
#define DEVICE_INFO_KEY "device_info"
#define HARDWARE_INFO_KEY "dmi"
#define CPU_INFO_KEY "cpu"

#define HARDWARE_INFO_FILE QStringLiteral("/sys/class/dmi/id/modalias")
#define CPU_INFO_FILE QStringLiteral("/proc/cpuinfo")

PolicyRuleParser::PolicyRuleParser(const Json::Value &policyRules, const QList<Json::Value> &customPolicyRules)
    : m_policyRules(policyRules)
    , m_customPolicyRules(customPolicyRules)
{

}

inline std::vector<std::unique_ptr<Handler>> parse_handlers(const Json::Value &handlers)
{
    std::vector<std::unique_ptr<Handler>> ret;
    const Json::Value::Members &members = handlers.getMemberNames();
    for (const auto &member : members) {
        const Json::Value &memberHandlers = handlers[member];
        for (const auto &handler : memberHandlers) {
            if (handler.isMember(HANDLER_ENABLE_KEY) && !handler[HANDLER_ENABLE_KEY].asBool())
                continue;
            std::unique_ptr<Handler> handlerPtr = parse_handler(handler);
            if (handlerPtr) {
                ret.push_back(std::move(handlerPtr));
            }
        }
    }
    return ret;
}

inline std::unordered_map<std::string, PolicyRule> parse_policy_rules(const Json::Value &policyRules)
{
    std::unordered_map<std::string, PolicyRule> rules;
    if (!policyRules.isMember(POLICIES_CONFIG_KEY))
        return rules;
    const Json::Value &policysConfig = policyRules[POLICIES_CONFIG_KEY];
    if (!policysConfig.isMember(POLICIES_KEY))    return rules;
    const Json::Value &policys = policysConfig[POLICIES_KEY];
    
    for (const Json::Value& policy : policys) {
        if (!policy.isMember(SCENE_ID_KEY)){
            PRINT_NOTICE("Unknown scene define\n");
            continue;
        }
        PolicyRule rule;
        std::string sceneId = policy[SCENE_ID_KEY].asString();
        size_t pos = sceneId.find(".");
        std::string subCondition = "";
        if (pos != std::string::npos) {
            rule.scene_id = sceneId.substr(0, pos) + "*";
            subCondition = sceneId.substr(pos + 1);
        } else {
            rule.scene_id = policy[SCENE_ID_KEY].asString();
        }

        if (rule.scene_id.find(SCENE_ID_EXAMPLE) != std::string::npos)
            continue;

        if (policy.isMember(HANDLERS_KEY)){
            const Json::Value &handlers = policy[HANDLERS_KEY];
            std::vector<std::unique_ptr<Handler>> tmpHandlers = parse_handlers(handlers);
            if (subCondition.empty()) {
                rule.handlers_ = std::move(tmpHandlers);
            } else {
                rule.subhandles_[subCondition] = std::move(tmpHandlers);
            }
        }

        if (policy.isMember(SUBHANDLES_KEY)){
            const Json::Value &subhandles = policy[SUBHANDLES_KEY];
            for (const auto &subhandle : subhandles){
                if (!subhandle.isMember(CONDITION_ID_KEY)) {
                    PRINT_NOTICE("Unknown subhandle define\n");
                    continue;
                }
                const Json::Value &condId = subhandle[CONDITION_ID_KEY];
                if (subhandle.isMember(HANDLERS_KEY)) {
                    const Json::Value &handlers = subhandle[HANDLERS_KEY];
                    std::vector<std::unique_ptr<Handler>> subhandlers = parse_handlers(handlers);
                    rule.subhandles_.insert({condId.asString(), std::move(subhandlers)});
                }
            }
        }
        
        rules[rule.scene_id] = std::move(rule);
    }
    
    return rules;
}

bool read_all(const QString &filePath, QString &content)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    content = file.readAll();
    file.close();

    return true;
}

bool check_info(const QString &original, const std::string &verified)
{
    std::vector<std::string> splitStrs = misc::string::split(verified, ": ");
    bool ret = true;
    for (const std::string &str : splitStrs) {
        ret &= original.contains(QString::fromStdString(str));
    }
    return ret;
}

std::unordered_map<std::string, PolicyRule> PolicyRuleParser::parse()
{
    if (m_policyRules.isMember(PROCESS_INFO_KEY)) {
        parse_process_info(m_policyRules[PROCESS_INFO_KEY]);
    }

    if (m_policyRules.isMember(SERVICE_INFO_KEY)) {
        parse_service_info(m_policyRules[SERVICE_INFO_KEY]);
    }

    std::unordered_map<std::string, PolicyRule> rules = parse_policy_rules(m_policyRules);

    QString hardwareInfo, cpuInfo;
    if (false == read_all(HARDWARE_INFO_FILE, hardwareInfo)) {
        // qDebug() << "get hardware info failed";
        PRINT_NOTICE("get hardware info failed\n");
    }

    if (false == read_all(CPU_INFO_FILE, cpuInfo)) {
        // qDebug() << "get cpu info failed";
        PRINT_NOTICE("get cpu info failed\n");
    }

    for (auto &policyRules : m_customPolicyRules) {
        // 匹配设备信息
        if (policyRules.isMember(DEVICE_INFO_KEY)) {
            const Json::Value &devInfo = policyRules[DEVICE_INFO_KEY];
            bool isMatch = devInfo.isMember(HARDWARE_INFO_KEY) || devInfo.isMember(CPU_INFO_KEY);
            if (devInfo.isMember(HARDWARE_INFO_KEY)) {
                std::string info = devInfo[HARDWARE_INFO_KEY].asString();
                isMatch &= check_info(hardwareInfo, info);
            }
            if (devInfo.isMember(CPU_INFO_KEY)) {
                std::string info = devInfo[CPU_INFO_KEY].asString();
                isMatch &= check_info(cpuInfo, info);
            }
            if (!isMatch)
                continue;
        } else {
            continue;
        }
        std::unordered_map<std::string, PolicyRule> customRules = parse_policy_rules(policyRules);

        for (auto &rule : customRules) {
            if (rule.first.back() == '*') {
                std::string sceneId = rule.first.substr(0, rule.first.size() - 1);
                for (auto &handler : rule.second.subhandles_) {
                    rules[sceneId].subhandles_[handler.first] = std::move(handler.second);
                }
            } else {
                rules[rule.first] = std::move(rule.second);
            }
        }
    }
    return rules;
}

void PSRStateHandler::handle(const StaticSceneData &data)
{
    PRINT_DBG("PSRStateHandler::handle\n");
    m_resourceManager.openPSR();
}

void PSRStateHandler::reverse()
{
    PRINT_DBG("PSRStateHandler::reverse\n");
    m_resourceManager.closePSR();
}

void VRRStateHandler::handle(const StaticSceneData &data)
{
    PRINT_DBG("VRRStateHandler::handle\n");
    m_resourceManager.setVRRRefreshRate(m_powerLevelPolycies[powerLevelMap[data.curr_power_level]]);
}

void VRRStateHandler::reverse()
{
    PRINT_DBG("VRRStateHandler::reverse\n");
    m_resourceManager.reverseVRRRefreshRate();
}

void EffectsHandler::handle(const StaticSceneData &data)
{
    PRINT_DBG("EffectsHandler::handle\n");
    DisplayEffectManager::getInstance()->suspendEffectEffects(m_filterList);
}

void EffectsHandler::reverse()
{
    PRINT_DBG("EffectsHandler::reverse\n");
    DisplayEffectManager::getInstance()->resumeEffectEffects();
}


