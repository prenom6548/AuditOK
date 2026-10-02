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
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "services/textprocessorproxy.h"
#include "services/serverproxy.h"
#include <iostream>

namespace {

const char *textProcessorObjectPath = "/org/openkylin/aisdk/textprocessor";
const char *textProcessorInterfaceName = "org.openkylin.aisdk.textprocessor";

}

void onChatResult(GDBusConnection *connection,
                  const gchar      *sender_name,
                  const gchar      *object_path,
                  const gchar      *interface_name,
                  const gchar      *signal_name,
                  GVariant         *parameters,
                  gpointer          user_data) {

    auto *proxy = static_cast<TextProcessorProxy *>(user_data);
    const gchar *cstr = nullptr;
    g_variant_get(parameters, "(s)", &cstr);
    std::string result(cstr);
    proxy->chatCallback(result, user_data);
}

TextProcessorProxy::TextProcessorProxy() { init(); }

TextProcessorProxy::~TextProcessorProxy() { destroy(); }

void TextProcessorProxy::initEngine(const std::string &engineName,
    const std::string &apiKey, const std::string &secretKey, const std::string &appId)
{
    aisdk_text_processor_call_init(delegate_, engineName.c_str(), appId.c_str(),
                                   apiKey.c_str(), secretKey.c_str(), nullptr, nullptr,
                                   this);
}

void TextProcessorProxy::chat(const std::string &question) {
    aisdk_text_processor_call_chat(
        delegate_, question.c_str(), nullptr, nullptr, this);
}

void TextProcessorProxy::stopChat()
{
    aisdk_text_processor_call_stop_chat(delegate_, nullptr, nullptr, nullptr);
}

void TextProcessorProxy::setActor(int actor)
{
    aisdk_text_processor_call_set_actor(
        delegate_, actor, nullptr, nullptr, nullptr);
}

void TextProcessorProxy::setContext(int size)
{
    aisdk_text_processor_call_set_context(delegate_, size, nullptr, nullptr, nullptr);
}

void TextProcessorProxy::clearContext()
{
    aisdk_text_processor_call_clear_context(delegate_, nullptr, nullptr, nullptr);
}

void TextProcessorProxy::init() {
    const auto &unixPath = ServerProxy::getUnixPath();
    GError *error = nullptr;
    if (!ServerProxy::getInstance().available()) {
        g_printerr("Error creating text processor proxy: Server proxy "
                   "connection is unavailable.\n");
        return;
    }

    auto *connection = ServerProxy::getInstance().getConnection();
    delegate_ = aisdk_text_processor_proxy_new_sync(
        connection, G_DBUS_PROXY_FLAGS_NONE, nullptr, textProcessorObjectPath,
        nullptr, &error);
    if (delegate_ == nullptr) {
        g_printerr("Error creating text processor proxy %s: %s\n",
                   textProcessorObjectPath, error->message);
        g_error_free(error);
    }

    g_dbus_connection_signal_subscribe(connection, nullptr,
        textProcessorInterfaceName, "ChatResult", textProcessorObjectPath, NULL,
        G_DBUS_SIGNAL_FLAGS_NONE, onChatResult, this, nullptr);
}

void TextProcessorProxy::destroy() {
    if (delegate_ != nullptr) {
        g_object_unref(delegate_);
    }
}
