/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * Licensed under the GNU General Public License v3 or later.
 */

#include "lifecycle/powereventmonitor.h"

#include <gio/gunixfdlist.h>

#include "lifecycle/enginestopcontroller.h"
#include "utils/logger.h"

namespace lifecycle {

namespace {
constexpr const char kLogin1Service[] = "org.freedesktop.login1";
constexpr const char kLogin1Path[] = "/org/freedesktop/login1";
constexpr const char kLogin1Interface[] = "org.freedesktop.login1.Manager";
constexpr const char kPrepareForSleepSignal[] = "PrepareForSleep";
constexpr const char kInhibitMethod[] = "Inhibit";
constexpr const char kInhibitWhat[] = "sleep";
constexpr const char kInhibitWho[] = "kylin-ai-runtime";
constexpr const char kInhibitWhy[] = "Unload models before system sleep";
constexpr const char kInhibitMode[] = "delay";
constexpr gint kInhibitTimeoutMs = 5000;
}  // namespace

PowerEventMonitor::PowerEventMonitor(ModelLifecycleManager &manager) : manager_(manager) {}

PowerEventMonitor::~PowerEventMonitor() { stop(); }

void PowerEventMonitor::start() {
    if (running_) {
        return;
    }
    running_ = true;

    GError *error = nullptr;
    connection_ = g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, &error);
    if (!connection_) {
        LOGE("PowerEventMonitor failed to connect system bus: {}", error ? error->message : "unknown error");
        if (error) {
            g_error_free(error);
        }
        running_ = false;
        return;
    }

    subscriptionId_ = g_dbus_connection_signal_subscribe(
        connection_, kLogin1Service, kLogin1Interface, kPrepareForSleepSignal, kLogin1Path, nullptr,
        G_DBUS_SIGNAL_FLAGS_NONE, &PowerEventMonitor::onPrepareForSleep, this, nullptr);
    if (subscriptionId_ == 0) {
        LOGE("PowerEventMonitor failed to subscribe PrepareForSleep signal.");
        g_object_unref(connection_);
        connection_ = nullptr;
        running_ = false;
        return;
    }

    if (!acquireInhibit()) {
        LOGW("PowerEventMonitor failed to acquire initial inhibit, but will continue.");
    }

    LOGI("PowerEventMonitor watching PrepareForSleep signal.");
}

void PowerEventMonitor::stop() {
    if (!running_) {
        return;
    }
    running_ = false;
    releaseInhibit();

    // 确保资源被正确释放
    if (subscriptionId_ != 0 && connection_ != nullptr) {
        g_dbus_connection_signal_unsubscribe(connection_, subscriptionId_);
        subscriptionId_ = 0;
    }
    if (connection_ != nullptr) {
        g_object_unref(connection_);
        connection_ = nullptr;
    }
}

void PowerEventMonitor::onPrepareForSleep(GDBusConnection *, const gchar *, const gchar *, const gchar *, const gchar *,
                                          GVariant *parameters, gpointer userData) {
    if (!userData) {
        LOGE("PowerEventMonitor onPrepareForSleep: userData is null");
        return;
    }

    bool isSleep = false;
    if (parameters != nullptr) {
        if (g_variant_is_of_type(parameters, G_VARIANT_TYPE("(b)"))) {
            g_variant_get(parameters, "(b)", &isSleep);
        } else if (g_variant_is_of_type(parameters, G_VARIANT_TYPE_BOOLEAN)) {
            isSleep = g_variant_get_boolean(parameters);
        }
    }
    auto *monitor = static_cast<PowerEventMonitor *>(userData);
    monitor->handlePrepareForSleep(isSleep);
}

void PowerEventMonitor::handlePrepareForSleep(bool isSleep) {
    if (isSleep) {
        EngineStopController::getInstance().stopAll("sleep");
        auto unloaded = manager_.forceUnloadForSleep("sleep");
        {
            std::lock_guard<std::mutex> lock(unloadedMutex_);
            unloadedOnSleep_ = std::move(unloaded);
        }
        LOGI("PowerEventMonitor unloaded {} models before sleep.", unloadedOnSleep_.size());
        releaseInhibit();
    } else {
        if (!acquireInhibit()) {
            LOGW("PowerEventMonitor failed to re-acquire inhibit after wake.");
        }
        std::vector<ModelDescriptor> toPrewarm;
        {
            std::lock_guard<std::mutex> lock(unloadedMutex_);
            toPrewarm = unloadedOnSleep_;
            unloadedOnSleep_.clear();
        }
        if (!toPrewarm.empty()) {
            LOGI("PowerEventMonitor force loading {} models after wake.", toPrewarm.size());
            std::thread([this, toPrewarm = std::move(toPrewarm)]() {
                try {
                    manager_.forceLoadModels(toPrewarm);
                } catch (const std::exception &e) {
                    LOGE("PowerEventMonitor error loading models after wake: {}", e.what());
                }
            }).detach();
        } else {
            LOGI("PowerEventMonitor wake event received, nothing to reload.");
        }
    }
}

bool PowerEventMonitor::acquireInhibit() {
    std::lock_guard<std::mutex> lock(inhibitMutex_);
    if (inhibitFd_ >= 0 || !running_) {
        return true;
    }
    if (connection_ == nullptr) {
        LOGW("PowerEventMonitor cannot acquire inhibit: no DBus connection.");
        return false;
    }

    GError *error = nullptr;
    GUnixFDList *out_fd_list = nullptr;
    GVariant *result = g_dbus_connection_call_with_unix_fd_list_sync(
        connection_, kLogin1Service, kLogin1Path, kLogin1Interface, kInhibitMethod,
        g_variant_new("(ssss)", kInhibitWhat, kInhibitWho, kInhibitWhy, kInhibitMode), G_VARIANT_TYPE("(h)"),
        G_DBUS_CALL_FLAGS_NONE, kInhibitTimeoutMs, nullptr, &out_fd_list, nullptr, &error);

    if (!result) {
        LOGE("PowerEventMonitor failed to call Inhibit method: {}", error ? error->message : "unknown error");
        if (error) {
            g_error_free(error);
        }
        if (out_fd_list) {
            g_object_unref(out_fd_list);
        }
        return false;
    }

    gint32 handle = -1;
    g_variant_get(result, "(h)", &handle);
    gint fd = g_unix_fd_list_get(out_fd_list, handle, &error);

    if (fd < 0) {
        LOGE("PowerEventMonitor received invalid inhibit fd: {}", fd);
        g_variant_unref(result);
        g_object_unref(out_fd_list);
        if (error) {
            g_error_free(error);
        }
        return false;
    }

    if (!running_) {
        close(fd);
        g_variant_unref(result);
        g_object_unref(out_fd_list);
        return false;
    }

    inhibitFd_ = fd;
    LOGI("PowerEventMonitor acquired sleep inhibitor fd {}.", inhibitFd_);
    g_variant_unref(result);
    g_object_unref(out_fd_list);
    return true;
}

void PowerEventMonitor::releaseInhibit() {
    std::lock_guard<std::mutex> lock(inhibitMutex_);
    if (inhibitFd_ <= 0) {
        return;
    }
    // 记录原始fd，避免在close后被其他线程修改
    int fd_to_close = inhibitFd_;
    inhibitFd_ = -1;

    if (close(fd_to_close) == 0) {
        LOGI("PowerEventMonitor released sleep inhibitor fd {}.", fd_to_close);
    } else {
        LOGW("PowerEventMonitor failed to close inhibit fd {}: {}", fd_to_close, strerror(errno));
    }
}

}  // namespace lifecycle
