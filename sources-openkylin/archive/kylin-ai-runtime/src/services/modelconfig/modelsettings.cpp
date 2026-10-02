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

#include "modelsettings.h"

#include <jsoncpp/json/json.h>

#include "utils/logger.h"

Settings::Settings(const std::string &schema) {
    if (!g_settings_schema_source_lookup(g_settings_schema_source_get_default(), schema.c_str(), true)) {
        LOGE("Schema {} not found.", schema.c_str());
        return;
    }
    settings_ = g_settings_new(schema.c_str());
}

Settings::~Settings() { g_object_unref(settings_); }

bool Settings::setJsonValue(const std::string &key, const std::string &jsonValue) {
    std::lock_guard<std::mutex> lock(mutex_);
    GVariant *value = g_variant_new("s", jsonValue.c_str());
    g_settings_set_value(settings_, key.c_str(), value);
    g_settings_sync();
    return true;
}

bool Settings::setIntValue(const std::string &key, int value) {
    std::lock_guard<std::mutex> lock(mutex_);
    GVariant *v = g_variant_new_int32(value);
    g_settings_set_value(settings_, key.c_str(), v);
    g_settings_sync();
    return true;
}

bool Settings::setBoolValue(const std::string &key, bool value) {
    std::lock_guard<std::mutex> lock(mutex_);
    GVariant *v = g_variant_new_boolean(value);
    g_settings_set_value(settings_, key.c_str(), v);
    g_settings_sync();
    return true;
}

const std::string Settings::getJsonValue(const std::string &key) const {
    std::lock_guard<std::mutex> lock(mutex_);
    gchar *value = g_settings_get_string(settings_, key.c_str());
    if (!value) {
        return "";
    }

    std::string json(value);
    g_free(value);

    return json;
}

int Settings::getIntValue(const std::string &key) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return g_settings_get_int(settings_, key.c_str());
}

bool Settings::getBoolValue(const std::string &key) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return g_settings_get_boolean(settings_, key.c_str());
}