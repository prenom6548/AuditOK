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

#include "requesthandler.h"

#include <jsoncpp/json/value.h>
#include <kylin-ai/ai-engine/aiengineerror.h>

#include "threadpool/async.h"
#include "utils/logger.h"
#include "utils/util.h"

namespace {

const char *objectPath = "/com/kylin/AiRuntime/Assistant";
const char *assistantResultInterface = "com.kylin.AiRuntime.Assistant";

}  // namespace

void RequestHandler::sendChatResult(GDBusConnection *connection, int sessionId, const std::string &result) {
    const std::string interfaceName = assistantResultInterface + std::to_string((int)sessionId);
    GVariantBuilder builder;
    g_variant_builder_init(&builder, G_VARIANT_TYPE_TUPLE);
    g_variant_builder_add(&builder, "s", result.c_str());
    g_variant_builder_add(&builder, "i", 0);
    GError *error = nullptr;
    g_dbus_connection_emit_signal(connection, assistantResultInterface, objectPath, interfaceName.c_str(), "ChatResult",
                                  g_variant_builder_end(&builder), &error);
}