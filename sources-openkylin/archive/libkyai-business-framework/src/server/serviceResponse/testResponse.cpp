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

#ifdef ENABLE_TEST
#include "testResponse.h"

#include "log.h"

void testSyncResponse(const std::string& method, GObject* delegate, GDBusMethodInvocation* invocation,
                      const std::string& output) {
    if (method == "getSync") {
        ai_business_test_complete_get_sync(reinterpret_cast<AiBusinessTest*>(delegate), invocation, output.c_str());
        PRINT_DEBUG("test service sync response getSync.\n");
    } else {
        PRINT_ERROR("test service unknown method: %s\n", method.c_str());
    }
}

void testAsyncResponse(const std::string& method, GObject* delegate, GDBusMethodInvocation* invocation,
                       const std::string& output) {
    if (method == "getAsync") {
        ai_business_test_complete_get_async(reinterpret_cast<AiBusinessTest*>(delegate), invocation, output.c_str());
        PRINT_DEBUG("test service async response getAsync.\n");
    } else {
        PRINT_ERROR("test service unknown method: %s\n", method.c_str());
    }
}

void testAsyncFlowResponse(const std::string& method, GDBusConnection* connection, GVariant* output) {
    if (method == "getAsyncFlow") {
        g_dbus_connection_emit_signal(connection, "cn.kylinos.aisdk.business.test", "/com/kylin/AiBusiness/test",
                                      "com.kylinos.aisdk.business.test", "callBack3Continue", output, nullptr);
        PRINT_DEBUG("test service async flow response asyncFlowMethod.\n");
    } else {
        PRINT_ERROR("test service unknown method: %s\n", method.c_str());
    }
}
#endif
