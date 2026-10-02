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

#ifndef DATAMANAGEMENTSERVICEPROXY_H
#define DATAMANAGEMENTSERVICEPROXY_H

#include <functional>
#include <string>

#include "datamanagementserviceglue.h"

using AiBusinessDataManagementCallSyncFunc =
    std::function<gboolean(AiBusinessDataManagement* proxy, const gchar* arg_input, GDBusCallFlags call_flags,
                           gint timeout_msec, gchar** out_output, GCancellable* cancellable, GError** error)>;

class DataManagementServiceProxy {
public:
    DataManagementServiceProxy() = default;
    ~DataManagementServiceProxy();

    std::string similaritySearch(const std::string& input);
    std::string addTextFiles(const std::string& input);
    std::string addImageFiles(const std::string& input);
    std::string deleteFiles(const std::string& input);
    std::string updateFilesName(const std::string& input);
    std::string updateContentData(const std::string& input);
    std::string getAllFileinfos();
    std::string getFeatureStatus();
    std::string getTagsOfFiles(const std::string& input);
    std::string getAllTags();
    std::string searchSimilarTags(const std::string& input);
    std::string searchFilesAboutTags(const std::string& input);
    std::string isFileRelevantToTags(const std::string& input);
    std::string isFileRelevantToDescription(const std::string& input);
    std::string extractSearchConditions(const std::string& input);
    std::string getSummarysOfFiles(const std::string& input);
    bool connect();

private:
    void destroy();

    std::string syncFuncCall(const std::string& input, gint timeout_msec,
                             const AiBusinessDataManagementCallSyncFunc& func);

private:
    const std::string objectPath_ = "/com/kylin/AiBusiness/DataManagement";
    GDBusConnection* connection_ = nullptr;
    AiBusinessDataManagement* delegate_ = nullptr;
};

#endif  // DATAMANAGEMENTSERVICEPROXY_H
