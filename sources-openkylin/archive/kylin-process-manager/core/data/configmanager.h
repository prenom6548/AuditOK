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

#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QGSettings/QGSettings>
#include <QTimer>
#include <functional>
#include <jsoncpp/json/json.h>
#include <list>
#include <memory>
#include <string>

#include "schedpolicy.h"
#include "statemanager.h"

class ConfigManager
{
public:
    using ResourceLimitEnableChangedCallback = std::function<void(bool)>;

    ConfigManager(const std::shared_ptr<StateManager> &stateManager);
    ~ConfigManager();

    int background2CachedStateInterval(sched_policy::DeviceMode mode) const;
    int powerlevelChangedDetectInterval() const
    { 
        return m_powerlevelChangedDetectInterval; 
    }
    bool reousrceLimitEnabled() const
    {
        return m_resourceLimitEnabled;
    }
    bool preResourceLimitEnabled() const 
    { 
        return m_preResourceLimitEnabled; 
    }
    bool scenemanagerEnabled() const 
    { 
        return m_scenemanagerEnabled; 
    }
    bool preScenemanagerEnabled() const 
    { 
        return m_preScenemanagerEnabled; 
    }
    bool resourceSchedulingEnabled() const 
    { 
        return m_resourceSchedulingEnabled; 
    }
    bool preResourceSchedulingEnabled() const 
    { 
        return m_preResourceSchedulingEnabled; 
    }
    bool kernelSupport() const 
    { 
        return m_kernelSupport; 
    }

    void setResourceLimitEnabled(bool enabled);
    bool softFreezeModeEnabled() const
    {
        return m_softFreezeModeEnabled;
    }
    Json::Value policyConfigInfo() const
    {
        return m_policyConfigInfo;
    }
    const Json::Value &sceneRules() const { return m_sceneRules; }
    const Json::Value &policyRules() const { return m_policyRules; }
    const QList<Json::Value> &customPolicyRules() const
    {
        return m_customPolicyRules;
    };
    const QStringList &coreTopProcess() const { return m_coreTopProcess; }
    const QStringList &effectivePowerMode() const { return m_effectivePowerMode; }
    const QStringList &blackListProcess() const { return m_blackListProcess; }
    QStringList defaultAppsInTopGroup() const
    {
        return m_defaultAppsInTopGroup;
    }

    virtual bool addAppToWhitelist(const QString &desktopFile);
    virtual bool removeAppFromWhitelist(const QString &desktopFile);
    virtual const QStringList &appWhitelist() const;

    void setIncreasedBatteryRemainingTime(int increasedTime);

    bool checkDomainController();

private:
    using GSettingsKeyChangedCallback = std::function<void(const std::shared_ptr<QGSettings> &gsettings, const QString &key)>;

    void initGSettings();
    void initRootGSettings();
    void initDefaultGroupAppGSettings();
    void initSoftFreezeModeGsettings();
    std::shared_ptr<QGSettings> doInitGSettings(
        const QByteArray &schemaId, GSettingsKeyChangedCallback keyChangedCallback);
    void loadGeneralConfig();
    void loadPolicyConfig();
    void loadKernelSupport();
    void loadSceneRules();
    void loadPolicyRules();
    void loadResourceSchedConfig();
    void initResourceLimitEnabledChangedTimer();
    bool isGSettingsKeyWritable(const char *schema, const char *key);

private:
    Json::Value m_background2CachedStateInterval;
    int m_powerlevelChangedDetectInterval;
    bool m_resourceLimitEnabled;
    bool m_preResourceLimitEnabled;
    bool m_scenemanagerEnabled;
    bool m_preScenemanagerEnabled;
    bool m_resourceSchedulingEnabled;
    bool m_preResourceSchedulingEnabled;
    bool m_kernelSupport;
    bool m_softFreezeModeEnabled;
    bool m_keyIsLock;
    QStringList m_defaultAppsInTopGroup;
    QStringList m_effectivePowerMode;
    QStringList m_coreTopProcess;
    QStringList m_blackListProcess;
    QStringList m_appWhitelist;
    Json::Value m_policyConfigInfo;
    Json::Value m_sceneRules;
    Json::Value m_policyRules;
    QList<Json::Value> m_customPolicyRules;
    std::shared_ptr<QGSettings> m_rootGSettings;
    std::shared_ptr<QGSettings> m_defaultGroupAppsGSettings;
    std::shared_ptr<QGSettings> m_appWhitelistGSettings;
    std::shared_ptr<QGSettings> m_softFreezeModeGSettings;
    std::shared_ptr<StateManager> m_stateManager;
    std::vector<QMetaObject::Connection> m_conns;

    const int kResourceLimitEnabledChangedTimerInterval = 1000;
    const std::string kGeneralConfigFile = "/etc/kylin-process-manager/kylin-process-manager.json";
    const std::string kPolicyConfigFileV1 = "/etc/kylin-process-manager/task-profiles-v1.json";
    const std::string kPolicyConfigFileV2 = "/etc/kylin-process-manager/task-profiles-v2.json";
    const std::string kSceneRulesFile = "/etc/kylin-process-manager/scene-rules.json";
    const std::string kPolicyRulesFile = "/etc/kylin-process-manager/policy-rules.json";
    const std::string kResourceSchedConfigFile = "/etc/kylin-process-manager/resource-sched-configs.json";
};

#endif // CONFIGMANAGER_H
