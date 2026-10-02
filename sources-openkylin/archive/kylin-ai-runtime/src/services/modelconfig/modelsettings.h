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

#include <mutex>
#include <string>

class Settings {
public:
    explicit Settings(const std::string &schema);
    ~Settings();

    bool setJsonValue(const std::string &key, const std::string &jsonValue);
    bool setIntValue(const std::string &key, int value);
    bool setBoolValue(const std::string &key, bool value);
    const std::string getJsonValue(const std::string &key) const;
    int getIntValue(const std::string &key) const;
    bool getBoolValue(const std::string &key) const;

private:
    GSettings *settings_{nullptr};
    mutable std::mutex mutex_;
};