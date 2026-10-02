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

#include <gio/gio.h>

#include <string>

#include "server/baseProcessor.h"

namespace scene {
extern GObject* getDelegate(const std::string& serviceName);
extern void serviceSyncResponse(const std::string& serviceName, const std::string& method, GObject* delegate,
                                GDBusMethodInvocation* invocation, const std::string& output);
extern void serviceAsyncResponse(const std::string& serviceName, const std::string& method, GObject* delegate,
                                 GDBusMethodInvocation* invocation, const std::string& output);
extern void serviceAsyncFlowResponse(const std::string& serviceName, const std::string& method,
                                     GDBusConnection* connection, GObject* delegate, GDBusMethodInvocation* invocation,
                                     GVariant* output, bool end);
}  // namespace scene
