/*
 * Copyright 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "configmanager.h"
#include "daemondbusinterface.h"
#include "loghelper.h"
#include "misc.h"
#include <QCoreApplication>
#include <QDebug>
#include <QVariant>
#include <dirent.h>
#include <fstream>
#include <gio/gio.h>
#include <iostream>
#include <sstream>
#include <unistd.h>

namespace
{

const char *resource_limit_key = "resourceLimitEnabled";
const char *resource_limit_compatibility_key = "resourceLimitEnable";
const char *scene_manager_key = "sceneManagerEnable";
const char *resource_scheduling_key = "resourceSchedulingEnable";
const char *top_app_group_key = "topApp";
const char *whitelist_key = "whitelist";

const char *kylin_sched_path = "/proc/kylin_sched";

const char *root_id = "org.ukui.process-manager";
const char *default_apps_id = "org.ukui.process-manager.default-group-apps";

const char *soft_freeze_mode_id = "org.ukui.process-manager.soft-freeze-mode";
const char *soft_freeze_mode_enabled_key = "enabled";
const char *soft_freeze_mode_increased_battery_remaining_time = "increasedBatteryRemainingTime";

const char *customization_path = "/etc/kylin-process-manager/policy_rules.d/";
}

ConfigManager::ConfigManager(const std::shared_ptr<StateManager> &stateManager)
    : m_stateManager(stateManager)
    , m_background2CachedStateInterval(1800)
    , m_powerlevelChangedDetectInterval(1200)
    , m_resourceLimitEnabled(false)
    , m_preResourceLimitEnabled(false)
    , m_keyIsLock(false)
{
    initGSettings();
    loadGeneralConfig();
    loadResourceSchedConfig();
    loadPolicyConfig();
    loadKernelSupport();
    loadSceneRules();
    loadPolicyRules();
}

ConfigManager::~ConfigManager()
{
    PRINT_NOTICE("~ConfigManager\n");
    for (auto &conn : m_conns) {
        QObject::disconnect(conn);
        conn = {};
    }
    m_conns.clear();
}

int ConfigManager::background2CachedStateInterval(sched_policy::DeviceMode mode) const
{
    std::string modeStr = sched_policy::deviceModeToString(mode);
    return m_background2CachedStateInterval[modeStr].asInt();
}

void ConfigManager::setResourceLimitEnabled(bool enabled)
{
    if (m_rootGSettings->keys().contains(resource_limit_key)) {
        m_rootGSettings->trySet(resource_limit_key, enabled);
    }
}

bool ConfigManager::addAppToWhitelist(const QString &desktopFile)
{
    auto whiteListDesktopFiles = appWhitelist();
    if (whiteListDesktopFiles.contains(desktopFile)) {
        return true;
    }

    whiteListDesktopFiles.append(desktopFile);
    if (m_rootGSettings->keys().contains(whitelist_key)) {
        return m_rootGSettings->trySet(whitelist_key, whiteListDesktopFiles);
    }
    return false;
}

bool ConfigManager::removeAppFromWhitelist(const QString &desktopFile)
{
    auto whiteListDesktopFiles = appWhitelist();
    if (!whiteListDesktopFiles.contains(desktopFile)) {
        return true;
    }

    whiteListDesktopFiles.removeOne(desktopFile);
    if (m_rootGSettings->keys().contains(whitelist_key)) {
        return m_rootGSettings->trySet(whitelist_key, whiteListDesktopFiles);
    }
    return false;
}

const QStringList &ConfigManager::appWhitelist() const
{
    return m_appWhitelist;
}

void ConfigManager::setIncreasedBatteryRemainingTime(int increasedTime)
{
    if (!m_softFreezeModeGSettings) {
        PRINT_NOTICE("Soft freeze mode GSettings is not initialized.");
        return;
    }
    m_softFreezeModeGSettings->set(soft_freeze_mode_increased_battery_remaining_time, increasedTime);
}

bool ConfigManager::checkDomainController()
{
    return !isGSettingsKeyWritable(root_id, "resource-limit-enable");
}

void ConfigManager::initGSettings()
{
    initRootGSettings();
    initDefaultGroupAppGSettings();
    initSoftFreezeModeGsettings();
}

void ConfigManager::initRootGSettings()
{
    static DaemonDbusInterface interface;
    m_rootGSettings = doInitGSettings(
        root_id, [this](const std::shared_ptr<QGSettings> &gsettings, const QString &key) {
            auto keys = m_rootGSettings->keys();
            if (key == resource_limit_key) {
                std::lock_guard<FairMutex>(m_stateManager->fairMutex(StateTarget::ResourceLimit));
                m_preResourceLimitEnabled = m_resourceLimitEnabled;
                m_resourceLimitEnabled = m_rootGSettings->get(resource_limit_key).toBool();
                m_stateManager->enableTarget(StateTarget::ResourceLimit);
                if (!checkDomainController() && keys.contains(resource_limit_compatibility_key)) {
                    m_rootGSettings->trySet(resource_limit_compatibility_key, m_resourceLimitEnabled);
                }
            } else if (key == resource_limit_compatibility_key) {
                if (checkDomainController()) {
                    if (!m_keyIsLock) {
                        interface.lockGSettingsKey("/org/ukui/process-manager/resource-limit-enabled", true);
                        m_keyIsLock = true;
                    }
                    if (keys.contains(resource_limit_key)) {
                        std::lock_guard<FairMutex>(m_stateManager->fairMutex(StateTarget::ResourceLimit));
                        m_preResourceLimitEnabled = m_resourceLimitEnabled;
                        m_resourceLimitEnabled = m_rootGSettings->get(resource_limit_key).toBool();
                        m_stateManager->enableTarget(StateTarget::ResourceLimit);
                    }
                } else {
                    if (m_keyIsLock) {
                        interface.lockGSettingsKey("/org/ukui/process-manager/resource-limit-enabled", false);
                        m_keyIsLock = false;
                    }
                    bool rlck = m_rootGSettings->get(resource_limit_compatibility_key).toBool();
                    if (keys.contains(resource_limit_key) && m_resourceLimitEnabled != rlck) {
                        m_rootGSettings->trySet(resource_limit_key, rlck);
                    }
                }
            } else if (key == whitelist_key) {
                std::lock_guard<FairMutex>(m_stateManager->fairMutex(StateTarget::AppWhiteList));
                m_appWhitelist = m_rootGSettings->get(whitelist_key).toStringList();
                m_stateManager->enableTarget(StateTarget::AppWhiteList);
            } else if (key == scene_manager_key) {
                std::lock_guard<FairMutex>(m_stateManager->fairMutex(StateTarget::SceneManager));
                m_preScenemanagerEnabled = m_scenemanagerEnabled;
                m_scenemanagerEnabled = m_rootGSettings->get(scene_manager_key).toBool();
                m_stateManager->enableTarget(StateTarget::SceneManager);
            } else if (key == resource_scheduling_key) {
                std::lock_guard<FairMutex>(m_stateManager->fairMutex(StateTarget::ResourceScheduling));
                m_preResourceSchedulingEnabled = m_resourceSchedulingEnabled;
                m_resourceSchedulingEnabled = m_rootGSettings->get(resource_scheduling_key).toBool();
                m_stateManager->enableTarget(StateTarget::ResourceScheduling);
            }
        });

    if (m_rootGSettings) {
        static auto keys = m_rootGSettings->keys();
        if (keys.contains(resource_limit_key)) {
            std::lock_guard<FairMutex>(m_stateManager->fairMutex(StateTarget::ResourceLimit));
            m_resourceLimitEnabled = m_rootGSettings->get(resource_limit_key).toBool();
            m_preResourceLimitEnabled = m_resourceLimitEnabled;
            m_stateManager->enableTarget(StateTarget::ResourceLimit);
        }

        if (keys.contains(whitelist_key)) {
            std::lock_guard<FairMutex>(m_stateManager->fairMutex(StateTarget::AppWhiteList));
            m_appWhitelist = m_rootGSettings->get(whitelist_key).toStringList();
            m_stateManager->enableTarget(StateTarget::AppWhiteList);
        }

        if (keys.contains(scene_manager_key)) {
            std::lock_guard<FairMutex>(m_stateManager->fairMutex(StateTarget::SceneManager));
            m_scenemanagerEnabled = m_rootGSettings->get(scene_manager_key).toBool();
            m_preScenemanagerEnabled = m_scenemanagerEnabled;
            m_stateManager->enableTarget(StateTarget::SceneManager);
        }

        if (keys.contains(resource_scheduling_key)) {
            std::lock_guard<FairMutex>(m_stateManager->fairMutex(StateTarget::ResourceScheduling));
            m_resourceSchedulingEnabled = m_rootGSettings->get(resource_scheduling_key).toBool();
            m_preResourceSchedulingEnabled = m_resourceSchedulingEnabled;
            m_stateManager->enableTarget(StateTarget::ResourceScheduling);
        }
    }
}

void ConfigManager::initDefaultGroupAppGSettings()
{
    m_defaultGroupAppsGSettings = doInitGSettings(
        default_apps_id, [this](const std::shared_ptr<QGSettings> &gsettings, const QString &key) {
        if (key == top_app_group_key) {
            std::lock_guard<FairMutex>(m_stateManager->fairMutex(StateTarget::TopAppList));
            m_defaultAppsInTopGroup = gsettings->get(top_app_group_key).toStringList();
            m_stateManager->enableTarget(StateTarget::TopAppList);
        }
    });

    if (m_defaultGroupAppsGSettings) {
        std::lock_guard<FairMutex>(m_stateManager->fairMutex(StateTarget::TopAppList));
        m_defaultAppsInTopGroup = m_defaultGroupAppsGSettings->get(top_app_group_key).toStringList();
        m_stateManager->enableTarget(StateTarget::TopAppList);
    }
}

void ConfigManager::initSoftFreezeModeGsettings()
{
    m_softFreezeModeGSettings = doInitGSettings(
        soft_freeze_mode_id, [this](const std::shared_ptr<QGSettings> &gsettings, const QString &key) {
        if (key == soft_freeze_mode_enabled_key) {
            std::lock_guard<FairMutex>(m_stateManager->fairMutex(StateTarget::SoftFreezeModeEnabled));
            m_softFreezeModeEnabled = gsettings->get(soft_freeze_mode_enabled_key).toBool();
            m_stateManager->enableTarget(StateTarget::SoftFreezeModeEnabled);
            PRINT_DBG("softFreezeModeEnabled %s\n", m_softFreezeModeEnabled ? "TRUE" : "FALSE");
        }
    });

    if (m_softFreezeModeGSettings) {
        std::lock_guard<FairMutex>(m_stateManager->fairMutex(StateTarget::SoftFreezeModeEnabled));
        m_softFreezeModeEnabled = m_softFreezeModeGSettings->get(soft_freeze_mode_enabled_key).toBool();
        m_stateManager->enableTarget(StateTarget::SoftFreezeModeEnabled);
    }
}

std::shared_ptr<QGSettings> ConfigManager::doInitGSettings(const QByteArray &schemaId,
                                                           GSettingsKeyChangedCallback keyChangedCallback)
{
    if (!QGSettings::isSchemaInstalled(schemaId)) {
        PRINT_NOTICE("schemaId %s is not installed\n", schemaId.toStdString().c_str());
        return nullptr;
    }

    std::shared_ptr<QGSettings> gsettings = std::make_shared<QGSettings>(schemaId);
    m_conns.emplace_back(QObject::connect(gsettings.get(), &QGSettings::changed, [keyChangedCallback, gsettings](const QString &key) {
        keyChangedCallback(gsettings, key);
    }));

    return gsettings;
}

void ConfigManager::loadGeneralConfig()
{
    std::ifstream file(kGeneralConfigFile);
    if (!file.is_open()) {
        PRINT_NOTICE("Failed to open general config file: %s.\n", kGeneralConfigFile.c_str());
        return;
    }

    Json::Value root;
    file >> root;

    if (root.isMember("Background2CachedSwitchInterval")) {
        m_background2CachedStateInterval = root["Background2CachedSwitchInterval"];
    }
    if (root.isMember("PowerLevelChangedDetectInterval") && root["PowerLevelChangedDetectInterval"].isInt()) {
        m_powerlevelChangedDetectInterval = root["PowerLevelChangedDetectInterval"].asInt();
    }
}

void ConfigManager::loadPolicyConfig()
{
    const auto configFile = misc::version::cgroupVersion() == 2 ? kPolicyConfigFileV2 : kPolicyConfigFileV1;
    std::ifstream file(configFile);
    if (!file.is_open()) {
        PRINT_NOTICE("Failed to open policy config file: %s.\n", configFile.c_str());
        return;
    }
    m_policyConfigInfo.clear();
    file >> m_policyConfigInfo;
}

void ConfigManager::loadKernelSupport()
{
    m_kernelSupport = access(kylin_sched_path, F_OK) == 0;
}

void ConfigManager::loadSceneRules()
{
    std::ifstream file(kSceneRulesFile);
    if (!file.is_open()) {
        PRINT_NOTICE("Failed to open scene rules config file: %s\n", kSceneRulesFile.c_str());
        return;
    }

    m_sceneRules.clear();
    file >> m_sceneRules;
}

void ConfigManager::loadPolicyRules()
{
    std::ifstream file(kPolicyRulesFile);
    if (file.is_open()) {
        m_policyRules.clear();
        file >> m_policyRules;
    }

    std::vector<std::string> files;
    DIR *dir = opendir(customization_path);
    m_customPolicyRules.clear();
    if (dir) {
        struct dirent *entry;
        while ((entry = readdir(dir)) != nullptr) {
            std::string filename = entry->d_name;
            if (filename.substr(filename.find_last_of(".") + 1) == "json") {
                files.push_back(filename);
            }
        }
        closedir(dir);

        std::sort(files.begin(), files.end());

        for (std::string f : files) {
            std::string path = customization_path + f;
            std::ifstream cusFile(path);
            if (cusFile.is_open()) {
                Json::Value tmpPolicyRules;
                cusFile >> tmpPolicyRules;
                m_customPolicyRules.append(tmpPolicyRules);
            }
        }
    }
}

void ConfigManager::loadResourceSchedConfig()
{
    std::ifstream file(kResourceSchedConfigFile);
    if (!file.is_open()) {
        PRINT_NOTICE("Failed to open resource sched config file: %s.", kResourceSchedConfigFile.c_str());
        return;
    }

    Json::Value root;
    file >> root;

    if (!root.isMember("EffectivePowerMode") || !root.isMember("CoreTopProcess") || !root.isMember("BlackListProcess")) {
        qWarning() << "Resource sched config file format is incorrect.";
        return;
    }

    Json::Value &effectivePowerMode = root["EffectivePowerMode"];
    for (Json::Value::ArrayIndex idx = 0; idx < effectivePowerMode.size(); ++idx)
    {
        m_effectivePowerMode.push_back(QString::fromStdString(effectivePowerMode[idx].asString()));
    }
    Json::Value &coreTopProcess = root["CoreTopProcess"];
    for (Json::Value::ArrayIndex idx = 0; idx < coreTopProcess.size(); ++idx)
    {
        m_coreTopProcess.push_back(QString::fromStdString(coreTopProcess[idx].asString())); 
    }
    Json::Value &blackListProcess = root["BlackListProcess"];
    for (Json::Value::ArrayIndex idx = 0; idx < blackListProcess.size(); ++idx)
    {
        m_blackListProcess.push_back(QString::fromStdString(blackListProcess[idx].asString())); 
    }
}

bool ConfigManager::isGSettingsKeyWritable(const char *schema, const char *key)
{
    PRINT_DBG("isGSettingsKeyWritable schema : %s, key : %s\n", schema, key);
    GSettings *gs = g_settings_new(schema);
    if (!gs) {
        qWarning() << "Failed to create GSettings for schema" << schema;
        return false;
    }

    gboolean writable = g_settings_is_writable(gs, key);
    PRINT_DBG("isGSettingsKeyWritable schema : %s, key : %s, writable : %d\n", schema, key, static_cast<int>(writable));
    g_object_unref(gs);

    return writable;
}