/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <gio/gio.h>
#include <kylin-ai/aiengine.h>

#include <memory>

#include "configserviceglue.h"
#include "modelconfigmanager.h"

class ConfigService : public std::enable_shared_from_this<ConfigService> {
public:
    ConfigService(GDBusConnection &connection, ModelConfigManager &modelConfigManager);

    ~ConfigService();

    void stop();

private:
    void exportSkeleton();
    void unexportSkeleton();

    void connectSignals();

    static bool onHandleGetModelStatus(AiRuntimeConfigService *service, GDBusMethodInvocation *invocation,
                                       gchar *displayName, gpointer userData);

    static bool onHandleGetModelsByCapability(AiRuntimeConfigService *service, GDBusMethodInvocation *invocation,
                                              int capability, gpointer userData);

    static bool onHandleGetModelsByDeployType(AiRuntimeConfigService *service, GDBusMethodInvocation *invocation,
                                              int deployType, gpointer userData);

    static bool onHandleGetCurrentModelName(AiRuntimeConfigService *service, GDBusMethodInvocation *invocation,
                                            gint capability, gint deployType, gpointer userData);

    static bool onHandleGetModelAuthentications(AiRuntimeConfigService *service, GDBusMethodInvocation *invocation,
                                                gchar *displayName, gpointer userData);

    static bool onHandleSetCurrentModelName(AiRuntimeConfigService *service, GDBusMethodInvocation *invocation,
                                            gint capability, gint deployType, gchar *displayName, gpointer userData);

    static bool onHandleGetModelDeployEnabled(AiRuntimeConfigService *service, GDBusMethodInvocation *invocation,
                                              gint deployType, gpointer userData);

    static bool onHandleSetModelAuthentications(AiRuntimeConfigService *service, GDBusMethodInvocation *invocation,
                                                gchar *displayName, gchar *authentications, gpointer userData);

    static bool onHandleSetCustomModelApiUrl(AiRuntimeConfigService *service, GDBusMethodInvocation *invocation,
                                             gchar *displayName, gchar *apiUrl, gpointer userData);

    static bool onHandleSetCustomModelVersion(AiRuntimeConfigService *service, GDBusMethodInvocation *invocation,
                                              gchar *displayName, gchar *modelVersion, gpointer userData);

    static bool onHandleClearModelAuthentications(AiRuntimeConfigService *service, GDBusMethodInvocation *invocation,
                                                  gchar *displayName, gpointer userData);

    static bool onHandleSetModelSchedulingPriority(AiRuntimeConfigService *service, GDBusMethodInvocation *invocation,
                                                   gint deployType, gint priority, gpointer userData);

    static bool onHandleGetModelSchedulingPriority(AiRuntimeConfigService *service, GDBusMethodInvocation *invocation,
                                                   gint deployType, gpointer userData);

    static bool onHandleSetModelDeployEnabled(AiRuntimeConfigService *service, GDBusMethodInvocation *invocation,
                                              gint deployType, gboolean enabled, gpointer userData);

    static bool onHandleAddCustomModel(AiRuntimeConfigService *service, GDBusMethodInvocation *invocation,
                                       gchar *modelName, gchar *modelVersion, gchar *apiKey, gchar *apiUrl,
                                       gpointer userData);

    static bool onHandleDeleteCustomModel(AiRuntimeConfigService *service, GDBusMethodInvocation *invocation,
                                          gchar *displayName, gpointer userData);

    static bool onHandleGetFreeTrialModels(AiRuntimeConfigService *service, GDBusMethodInvocation *invocation,
                                           gint capability, gpointer userData);

    static bool onHandleSetSharedModelName(AiRuntimeConfigService *service, GDBusMethodInvocation *invocation,
                                           gchar *modelName, gpointer userData);
    static bool onHandleStopModelSharing(AiRuntimeConfigService *service, GDBusMethodInvocation *invocation,
                                         gchar *modelName, gpointer userData);
    static bool onHandleGetSharedModelName(AiRuntimeConfigService *service, GDBusMethodInvocation *invocation,
                                           gpointer userData);

    static bool onHandleGetLoadOnBoot(AiRuntimeConfigService *service, GDBusMethodInvocation *invocation,
                                      gchar *model, gpointer userData);
    static bool onHandleSetLoadOnBoot(AiRuntimeConfigService *service, GDBusMethodInvocation *invocation,
                                      gchar *model, gboolean enabled, gpointer userData);

private:
    std::shared_ptr<ConfigService> getSharedObject() { return shared_from_this(); }

private:
    bool isExported_ = false;
    AiRuntimeConfigService *delegate_ = nullptr;

    GDBusConnection &connection_;

    ModelConfigManager &modelManager_;
    std::vector<gulong> handlerIds_;
};
