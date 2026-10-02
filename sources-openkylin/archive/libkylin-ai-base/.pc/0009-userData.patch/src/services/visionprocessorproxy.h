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

#ifndef SERVICES_VISIONPROCESSORPROXY_H
#define SERVICES_VISIONPROCESSORPROXY_H

#include <functional>
#include <string>

#include "services/visionprocessorglue.h"
#include <kylin-ai/ai-base/vision.h>

class VisionProcessorProxy {
public:
    VisionProcessorProxy();
    ~VisionProcessorProxy();

    void setPromptImageCallback(ImageResultCallback callback) {
        promptImageCallback_ = callback;
    }

    void initEngine(const std::string &engineName, const std::string &apiKey = {},
                    const std::string &secretKey = {}, const std::string &appId = {});
    void setPromptImageSize(int width, int height);
    void setPromptImageNumber(int number);
    void setPromptImageStyle(VisionImageStyle style);
    void promptImage(const char* prompt);

private:
    void init();
    void destroy();

    friend void onPromptImageResult(GDBusConnection *connection,
                                    const gchar *sender_name,
                                    const gchar *object_path,
                                    const gchar *interface_name,
                                    const gchar *signal_name,
                                    GVariant *parameters,
                                    gpointer user_data);

    static void printErrorMessage(GError *error, std::string callMethodName);

    static void onInitEngineCallback(AisdkVisionProcessor *delegate,
                                     GAsyncResult *result, gpointer userData);
    static void onPromptImageSizeCallback(AisdkVisionProcessor *delegate,
                                          GAsyncResult *result, gpointer userData);
    static void onPromptImageNumberCallback(AisdkVisionProcessor *delegate,
                                            GAsyncResult *result, gpointer userData);
    static void onPromptImageStyleCallback(AisdkVisionProcessor *delegate,
                                           GAsyncResult *result, gpointer userData);
    static void onPromptImageCallback(AisdkVisionProcessor *delegate,
                                      GAsyncResult *result, gpointer userData);

private:
    AisdkVisionProcessor *delegate_ = nullptr;

    ImageResultCallback promptImageCallback_ = nullptr;

    const char *objectPath_ = "/org/openkylin/aisdk/visionprocessor";
    const char *interfaceName_ = "org.openkylin.aisdk.visionprocessor";
};
#endif
