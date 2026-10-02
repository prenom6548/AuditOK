#ifndef STATICSCENEDATA_H
#define STATICSCENEDATA_H

#include <QStringList>
#include <unordered_map>
#include <unordered_set>

#include "schedpolicy.h"
#include "appinfo.h"

struct StaticSceneData
{
    bool is_init;
    bool is_resLimitInit;

    // ConfigerManager
    bool resource_limit_enabled; 
    bool pre_resource_limit_enabled;
    bool scenemanager_enabled;
    bool pre_scenemanager_enabled;
    bool resource_scheduling_enabled;
    bool pre_resource_scheduling_enabled;
    bool kernel_support;
    bool soft_freeze_mode_enabled;
    QStringList default_apps_in_top_group;
    QStringList app_whitelist;

    // PowerManager
    sched_policy::PowerType last_power_type;
    sched_policy::PowerType curr_power_type;
    sched_policy::PowerMode last_power_mode;
    sched_policy::PowerMode curr_power_mode;
    int last_power_level;
    int curr_power_level;
    int last_power_percentage;
    int curr_power_percentage;

    // DeviceModeManager
    sched_policy::DeviceMode last_device_mode;
    sched_policy::DeviceMode curr_device_mode;
    sched_policy::DeviceMode real_device_mode;
    sched_policy::DPMSMode last_dpms_mode;
    sched_policy::DPMSMode curr_dpms_mode;

    // SystemStatusManager
    sched_policy::ScreenMode last_screen_mode;
    sched_policy::ScreenMode curr_screen_mode;

    sched_policy::StatusMode last_status_mode;
    sched_policy::StatusMode curr_status_mode;

    sched_policy::InputMode last_input_mode;
    sched_policy::InputMode curr_input_mode;

    // ProcessInfoManger
    std::unordered_map<std::string, AppChangeHash> app_change_type;
    std::unordered_map<std::string, AppInfo> all_app_infos;
    std::unordered_set<std::string> app_win_changed_spell;
    std::unordered_set<std::string> app_focus_spell;
    std::string focus_app_id;

    std::unordered_map<std::string, int> resource_threshold;

    bool is_prepare_for_sleep;

    bool is_prepare_for_shutdown;

    bool is_show_desktop;

    bool bench_mark_state;
};

#endif // !STATICSCENEDATA_H