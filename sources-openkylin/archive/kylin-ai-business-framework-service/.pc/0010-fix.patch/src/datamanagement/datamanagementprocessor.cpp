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

// #include "datamanagement/datamanagementtaskmanager.h"
#include "datamanagement/datamanagementprocessor.h"
#include "datamanagement/datamanagementservice.h"

#include <iostream>

const std::string DataManagementProcessor::objectPath_ =
    "/org/openkylin/aisdk/datamanagementprocessor";

DataManagementProcessor::DataManagementProcessor(GDBusConnection &connection)
    : connection_(connection)
{
    exportSkeleton();
}

DataManagementProcessor::~DataManagementProcessor()
{
    unexportSkeleton();
}

void DataManagementProcessor::exportSkeleton()
{
    delegate_ = aisdk_data_management_processor_skeleton_new();
    connectSignal();

    GError *error = nullptr;
    isExported_ = g_dbus_interface_skeleton_export(
        G_DBUS_INTERFACE_SKELETON(delegate_), &connection_, objectPath_.c_str(), &error);
    if (!isExported_) {
        g_printerr("Error creating server at address %s: %s\n",
                   objectPath_.c_str(), error->message);
        g_error_free(error);
    }
}

void DataManagementProcessor::unexportSkeleton()
{
    if (delegate_ == nullptr) {
        return;
    }

    if (isExported_) {
        g_dbus_interface_skeleton_unexport_from_connection(
            G_DBUS_INTERFACE_SKELETON(delegate_), &connection_);
    }

    g_object_unref(delegate_);
}

void DataManagementProcessor::connectSignal()
{
    g_signal_connect(delegate_, "handle-similarity-search",
                     G_CALLBACK(DataManagementProcessor::handleSimilaritySearch),
                     this);
    g_signal_connect(delegate_, "handle-add-text-files",
                     G_CALLBACK(DataManagementProcessor::handleAddTextFiles),
                     this);
    g_signal_connect(delegate_, "handle-add-image-files",
                     G_CALLBACK(DataManagementProcessor::handleAddImageFiles),
                     this);
    g_signal_connect(delegate_, "handle-delete-files",
                     G_CALLBACK(DataManagementProcessor::handleDeleteFiles),
                     this);
    g_signal_connect(delegate_, "handle-update-files-name",
                     G_CALLBACK(DataManagementProcessor::handleUpdateFilesName),
                     this);
    g_signal_connect(delegate_, "handle-update-files-content-data",
                     G_CALLBACK(DataManagementProcessor::handleUpdateFilesContent),
                     this);
    g_signal_connect(delegate_, "handle-get-all-fileinfos",
                     G_CALLBACK(DataManagementProcessor::handleGetAllFileinfos),
                     this);
}

bool DataManagementProcessor::handleSimilaritySearch(AisdkDataManagementProcessor *delegate,
                                                     GDBusMethodInvocation *invocation,
                                                     gchar *searchConditions, int /*uid*/,
                                                     gpointer /*userData*/)
{
    auto callback = [delegate, invocation](const std::string& searchResult, int errorCode)
    {
        std::cout << "Complete similarity_search request" << std::endl;
        aisdk_data_management_processor_complete_similarity_search(
            delegate, invocation, searchResult.c_str(), errorCode);
    };

    DataManagementService::getInstance()->similaritySearch(searchConditions, callback);

    return true;
}

bool DataManagementProcessor::handleAddTextFiles(AisdkDataManagementProcessor *delegate,
                                                 GDBusMethodInvocation *invocation,
                                                 gchar *fileinfos, int /*uid*/, gpointer /*userData*/)
{
    auto callback = [delegate, invocation](int errorCode)
    {
        std::cout << "Complete add_text_files request" << std::endl;
        aisdk_data_management_processor_complete_add_text_files(
            delegate, invocation, errorCode);
    };

    DataManagementService::getInstance()->addTextFiles(fileinfos, callback);

    return true;
}

bool DataManagementProcessor::handleAddImageFiles(AisdkDataManagementProcessor *delegate,
                                                  GDBusMethodInvocation *invocation,
                                                  gchar *fileinfos, int /*uid*/, gpointer /*userData*/)
{
    auto callback = [delegate, invocation](int errorCode)
    {
        std::cout << "Complete add_image_files request" << std::endl;
        aisdk_data_management_processor_complete_add_image_files(
            delegate, invocation, errorCode);
    };

    DataManagementService::getInstance()->addImageFiles(fileinfos, callback);

    return true;
}

bool DataManagementProcessor::handleDeleteFiles(AisdkDataManagementProcessor *delegate,
                                                GDBusMethodInvocation *invocation,
                                                gchar *fileinfos, int /*uid*/, gpointer /*userData*/)
{
    auto callback = [delegate, invocation](int errorCode)
    {
        std::cout << "Complete delete_files request" << std::endl;
        aisdk_data_management_processor_complete_delete_files(
            delegate, invocation, errorCode);
    };
    DataManagementService::getInstance()->deleteFiles(fileinfos, callback);

    return true;
}

bool DataManagementProcessor::handleUpdateFilesName(AisdkDataManagementProcessor *delegate,
                                                    GDBusMethodInvocation *invocation,
                                                    gchar *fileinfos, int /*uid*/, gpointer /*userData*/)
{
    auto callback = [delegate, invocation](int errorCode)
    {
        std::cout << "Complete update_files_name request" << std::endl;
        aisdk_data_management_processor_complete_update_files_name(
            delegate, invocation, errorCode);
    };

    DataManagementService::getInstance()->updateFilesName(fileinfos, callback);

    return true;
}

bool DataManagementProcessor::handleUpdateFilesContent(AisdkDataManagementProcessor *delegate,
                                                       GDBusMethodInvocation *invocation,
                                                       gchar *fileinfos, int /*uid*/, gpointer /*userData*/)
{
    auto callback = [delegate, invocation](int errorCode)
    {
        std::cout << "Complete update_files_content request" << std::endl;
        aisdk_data_management_processor_complete_update_files_content_data(
            delegate, invocation, errorCode);
    };

    DataManagementService::getInstance()->updateFilesContent(fileinfos, callback);

    return true;
}

bool DataManagementProcessor::handleGetAllFileinfos(AisdkDataManagementProcessor *delegate,
                                                    GDBusMethodInvocation *invocation, int /*uid*/,
                                                    gpointer /*userData*/)
{

    auto callback = [delegate, invocation](const std::string& fileinfos, int errorCode)
    {
        aisdk_data_management_processor_complete_get_all_fileinfos(
            delegate, invocation, fileinfos.c_str(), errorCode);
    };

    DataManagementService::getInstance()->getAllFileInfos(callback);

    return true;
}
