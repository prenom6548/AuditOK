/*
 * Copyright 2026 KylinSoft Co., Ltd.
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

#include "systemapi.h"

class AppStoreSystemApi : public CommandSystemApi {
public:
    explicit AppStoreSystemApi(const std::vector<ApiParam> &params);

    bool execute() override;

private:
    std::string appName_;
    std::string actionType_;
    void buildCommand();
};

class AlarmSystemApi : public CommandSystemApi {
public:
    explicit AlarmSystemApi(const std::vector<ApiParam> &params);

    bool execute() override;

private:
    void parseSlots(const std::vector<ApiParam> &params);
    void parseTime(const std::string &dateStr);
    int chineseNumberToInt(const std::string &chineseNum);
    std::string escapeJsonString(const std::string &str);
    std::string escapeShellString(const std::string &str);
    void buildCommand();

    int hour_;
    int minute_;
    std::string eventStr_;
    std::string remindLater_;
    std::string repeat_;
    std::string operationStr_;
};