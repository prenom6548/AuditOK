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

#include "services/visionprocessorproxy.h"

#include <iostream>

#include "services/serverproxy.h"

void onPromptImageResult(GDBusConnection *connection,
                         const gchar *sender_name,
                         const gchar *object_path,
                         const gchar *interface_name,
                         const gchar *signal_name,
                         GVariant *parameters,
                         gpointer user_data) {
    auto *proxy = static_cast<VisionProcessorProxy *>(user_data);

    GVariantIter iter;
    g_variant_iter_init(&iter, parameters);

    GVariant *array_variant = g_variant_iter_next_value(&iter); // 获取字节数组
    GVariant *int_format = g_variant_iter_next_value(&iter); // 获取整数
    GVariant *int_height = g_variant_iter_next_value(&iter);
    GVariant *int_width = g_variant_iter_next_value(&iter);
    GVariant *int_total = g_variant_iter_next_value(&iter);
    GVariant *int_index = g_variant_iter_next_value(&iter);
    GVariant *int_error = g_variant_iter_next_value(&iter);

    int *format = (int *)g_variant_get_data(int_format);
    int *height = (int *)g_variant_get_data(int_height);
    int *width = (int *)g_variant_get_data(int_width);
    int *total = (int *)g_variant_get_data(int_total);
    int *index = (int *)g_variant_get_data(int_index);
    int *error = (int *)g_variant_get_data(int_error);
    std::cout << "onPromptImageResult---format---" << *format
              << "---height---" << *height << "---width---" << *width
              << "---total---" << *total << "---index---" << *index
              << "---error---" << *error << std::endl;

    std::cout << "Type " << g_variant_get_type_string(array_variant) << std::endl;
    std::cout << "size " << g_variant_get_size(array_variant) << std::endl;

    gchar *arr = (gchar*)g_variant_get_data(array_variant);
    std::vector<char> vec { arr, arr + g_variant_get_size(array_variant) - 1 };

    VisionImageData visionImageData {
        (VisionImageFormat)*format, *height, *width, *total,
        *index, (unsigned char*)vec.data(), (int)vec.size(), *error };
    proxy->promptImageCallback_(visionImageData);
}

VisionProcessorProxy::VisionProcessorProxy() { init(); }

VisionProcessorProxy::~VisionProcessorProxy() { destroy(); }

void VisionProcessorProxy::initEngine(
        const std::string &engineName, const std::string &apiKey,
        const std::string &secretKey, const std::string &appId) {
    std::cout << "proxy---initEngine---" << std::endl;
    aisdk_vision_processor_call_init(
        delegate_, engineName.c_str(), appId.c_str(),
        apiKey.c_str(), secretKey.c_str(), nullptr, nullptr, this);
}

void VisionProcessorProxy::setPromptImageSize(int width, int height) {
    aisdk_vision_processor_call_set_prompt_image_size(
       delegate_, width, height, nullptr, nullptr, this);
}

void VisionProcessorProxy::setPromptImageNumber(int number) {
    aisdk_vision_processor_call_set_prompt_image_number(
        delegate_, number, nullptr, nullptr, this);
}

void VisionProcessorProxy::setPromptImageStyle(VisionImageStyle style) {
    aisdk_vision_processor_call_set_prompt_image_style(
        delegate_, style, nullptr, nullptr, this);
}

void VisionProcessorProxy::promptImage(const char* prompt) {
    aisdk_vision_processor_call_prompt_image(
        delegate_, prompt, nullptr, nullptr, this);
}

void VisionProcessorProxy::init() {
    GError *error = nullptr;
    if (!ServerProxy::getInstance().available()) {
        g_printerr("Error creating vision processor proxy: Server proxy "
                   "connection is unavailable.\n");
        return;
    }

    auto *connection = ServerProxy::getInstance().getConnection();
    delegate_ = aisdk_vision_processor_proxy_new_sync(
        connection, G_DBUS_PROXY_FLAGS_NONE, nullptr, objectPath_,
        nullptr, &error);
    if (delegate_ == nullptr) {
        g_printerr("Error creating vision processor proxy %s: %s\n",
                   objectPath_, error->message);
        g_error_free(error);
    }

    g_dbus_connection_signal_subscribe(connection, nullptr,
        interfaceName_, "PromptImageResult", objectPath_, NULL,
        G_DBUS_SIGNAL_FLAGS_NONE, onPromptImageResult, this, nullptr);
}

void VisionProcessorProxy::destroy() {
    if (delegate_ != nullptr) {
        g_object_unref(delegate_);
    }
}
