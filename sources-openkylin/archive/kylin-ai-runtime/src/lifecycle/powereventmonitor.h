/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * Licensed under the GNU General Public License v3 or later.
 */

#pragma once

#include <gio/gio.h>

#include "lifecycle/modellifecyclemanager.h"

namespace lifecycle {

class PowerEventMonitor {
public:
    explicit PowerEventMonitor(ModelLifecycleManager &manager);
    ~PowerEventMonitor();

    void start();
    void stop();

private:
    static void onPrepareForSleep(GDBusConnection *connection, const gchar *senderName, const gchar *objectPath,
                                  const gchar *interfaceName, const gchar *signalName, GVariant *parameters,
                                  gpointer userData);
    void handlePrepareForSleep(bool isSleep);
    bool acquireInhibit();
    void releaseInhibit();

private:
    ModelLifecycleManager &manager_;
    bool running_{false};
    GDBusConnection *connection_{nullptr};
    gulong subscriptionId_{0};
    std::mutex unloadedMutex_;
    std::vector<ModelDescriptor> unloadedOnSleep_;
    std::mutex inhibitMutex_;
    int inhibitFd_{-1};
};

}  // namespace lifecycle
