/*
 * Copyright 2025 KylinSoft Co., Ltd.
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

#include "ctrl.h"

#include <gio/gio.h>

#include "utils/logger.h"
#include "utils/util.h"

Ctrl::Ctrl() = default;

bool Ctrl::controlMprisPlayerDbus(const std::string &funcName) {
    GError *error{nullptr};
    GDBusProxy *proxy =
        g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION, G_DBUS_PROXY_FLAGS_NONE, nullptr, SESSION_DBUS_NAME_STR_FIELD,
                                      SESSION_DBUS_PLAYER_PATH, SESSION_DBUS_PLAYER_INTERFACE, nullptr, &error);

    if (error != nullptr) {
        LOGE("g_dbus_proxy_new_for_bus_sync error: {}", error->message);
        g_error_free(error);
        g_object_unref(proxy);
        return false;
    }
    if (util::stringStartsWith(funcName, "seek")) {
        GVariantBuilder builderArray;
        g_variant_builder_init(&builderArray, G_VARIANT_TYPE_ARRAY);
        g_variant_builder_add(&builderArray, "v", g_variant_new("x", gint64(10000000)));
        GVariant *array = g_variant_builder_end(&builderArray);

        GVariantBuilder builder;
        g_variant_builder_init(&builder, G_VARIANT_TYPE_TUPLE);
        g_variant_builder_add(&builder, "s", "seek");
        g_variant_builder_add_value(&builder, array);
        GVariant *param = g_variant_builder_end(&builder);

        g_dbus_proxy_call_sync(proxy, "operation", param, G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error);

        if (error != nullptr) {
            LOGE("controlMprisPlayerDbus {}", error->message);
            g_error_free(error);
            g_object_unref(proxy);
            return false;
        }
    } else {
        GVariant *param = g_variant_new("(sav)", funcName.c_str(), nullptr);
        g_dbus_proxy_call_sync(proxy, "operation", param, G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error);
        if (error != nullptr) {
            LOGE("controlMprisPlayerDbus {}", error->message);
            g_error_free(error);
            g_object_unref(proxy);
            return false;
        }
        g_variant_unref(param);
    }
    g_object_unref(proxy);
    return true;
}

void Ctrl::setBrightness(bool isUp) {
    // TODO 设置未成功
    GError *error{nullptr};
    GDBusProxy *proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION, G_DBUS_PROXY_FLAGS_NONE, nullptr,
                                                      SESSION_DBUS_NAME_BRIGHTNESS, SESSION_DBUS_BRIGHTNESS_PATH,
                                                      SESSION_DBUS_BRIGHTNESS_INTERFACE, nullptr, &error);
    if (error != nullptr) {
        LOGE("g_dbus_proxy_new_for_bus_sync error: {}", error->message);
        g_error_free(error);
        g_object_unref(proxy);
        return;
    }

    GVariant *result =
        g_dbus_proxy_call_sync(proxy, "getPrimaryBrightness", nullptr, G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error);
    if (error != nullptr) {
        LOGE("Set brightness, get primary brightness {}", error->message);
        g_error_free(error);
        g_object_unref(proxy);
        return;
    }
    uint32_t value{};
    if (result != nullptr) {
        g_variant_get(result, "(u)", &value);
        g_variant_unref(result);
    }

    LOGI("Current brightness: {}", value);

    if (isUp) {
        value += 10;
        if (value >= 100) {
            value = 100;
        }
    } else {
        if (value >= 10) {
            value -= 10;
        } else {
            value = 0;
        }
    }

    LOGI("New brightness: {}", value);

    GVariant *param = g_variant_new("(u)", value);
    g_dbus_proxy_call_sync(proxy, "setPrimaryBrightness", param, G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error);
    if (error != nullptr) {
        LOGE("Set brightness, get primary brightness {}", error->message);
        g_error_free(error);
        g_object_unref(proxy);
        return;
    }
    g_variant_unref(param);
    g_object_unref(proxy);
}

void Ctrl::setDefaultAdapterSwitchStatus(bool power) {
    LOGD("setDefaultAdapterSwitchStatus");

    GError *error{nullptr};
    GDBusProxy *proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION, G_DBUS_PROXY_FLAGS_NONE, nullptr,
                                                      SYSTEM_DBUS_BLE_SERVICE.c_str(), SYSTEM_DBUS_BLE_PATH.c_str(),
                                                      SYSTEM_DBUS_BLE_INTERFACE.c_str(), nullptr, &error);

    if (error != nullptr) {
        LOGE("g_dbus_proxy_new_for_bus_sync error: {}", error->message);
        g_error_free(error);
        g_object_unref(proxy);
        return;
    }

    GVariant *param = g_variant_new("(b)", power);

    g_dbus_proxy_call_sync(proxy, "setDefaultAdapterPower", param, G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error);
    if (error != nullptr) {
        LOGE("Set brightness, get primary brightness {}", error->message);
        g_error_free(error);
        g_object_unref(proxy);
        return;
    }
    g_variant_unref(param);
    g_object_unref(proxy);
}
