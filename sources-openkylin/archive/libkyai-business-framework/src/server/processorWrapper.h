/*
 * Copyright (C) 2024 KylinSoft Co., Ltd.
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

#pragma once

#include "baseProcessor.h"

namespace scene {
/**
 * processor的包装类
 */
class ProcessorWrapper : public BaseProcessor {
public:
    ProcessorWrapper(const std::string& serviceName, GDBusConnection* connection);
    void syncResponse(const std::string& method, GDBusMethodInvocation* invocation, const std::string& output) override;

    static void asyncResponse(const std::shared_ptr<handlerTask>& task, const std::string& output);
    static void asyncFlowResponse(const std::shared_ptr<handlerTask>& task, const std::string& output);
};
}  // namespace scene
