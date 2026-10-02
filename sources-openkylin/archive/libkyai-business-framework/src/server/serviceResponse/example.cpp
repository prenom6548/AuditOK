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

#ifdef ENABLE_EXAMPLE
#include "example.h"

#include "log.h"

void exampleSyncResponse(const std::string& method, GObject* delegate, GDBusMethodInvocation* invocation,
                         const std::string& output) {
    if (method == "syncMethod") {
        ai_business_example_complete_sync_method(reinterpret_cast<AiBusinessExample*>(delegate), invocation,
                                                 output.c_str());
        PRINT_DEBUG("example service sync response syncMethod.\n");
    } else {
        PRINT_ERROR("example service unknown method: %s\n", method.c_str());
    }
}

void exampleAsyncResponse(const std::string& method, GObject* delegate, GDBusMethodInvocation* invocation,
                          const std::string& output) {
    if (method == "asyncMethod") {
        ai_business_example_complete_async_method(reinterpret_cast<AiBusinessExample*>(delegate), invocation,
                                                  output.c_str());
        PRINT_DEBUG("example service async response asyncMethod.\n");
    } else {
        PRINT_ERROR("example service unknown method: %s\n", method.c_str());
    }
}

void exampleAsyncFlowResponse(const std::string& method, GDBusConnection* connection, GVariant* output) {
    if (method == "asyncFlowMethod") {
        g_dbus_connection_emit_signal(connection, "cn.kylinos.aisdk.business.example", "/com/kylin/AiBusiness/example",
                                      "com.kylinos.aisdk.business.example", "callBack3Continue", output, nullptr);
        PRINT_DEBUG("example service async flow response asyncFlowMethod.\n");
    } else {
        PRINT_ERROR("example service unknown method: %s\n", method.c_str());
    }
}
#endif
