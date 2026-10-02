#include "services/datamanagementprocessorproxy.h"

#include "services/serverproxy.h"

#include <iostream>

DataManagementProcessorProxy::DataManagementProcessorProxy(uid_t uid) : uid_(uid)
{
    init();
}

DataManagementProcessorProxy::~DataManagementProcessorProxy()
{
    destroy();
}

void DataManagementProcessorProxy::init()
{
    GError *error = nullptr;
    if (!ServerProxy::getInstance().available()) {
        g_printerr("Error creating speech processor proxy: Server proxy "
                   "connection is unavailable.\n");
        return;
    }

    auto *connection = ServerProxy::getInstance().getConnection();
    delegate_ = aisdk_data_management_processor_proxy_new_sync(
        connection, G_DBUS_PROXY_FLAGS_NONE, nullptr, objectPath_.c_str(),
        nullptr, &error);
    if (delegate_ == nullptr) {
        g_printerr("Error creating speech processor proxy %s: %s\n",
                   objectPath_.c_str(), error->message);
        g_error_free(error);
    }
}

void DataManagementProcessorProxy::destroy()
{
    if (delegate_ != nullptr) {
        g_object_unref(delegate_);
    }
}

int DataManagementProcessorProxy::similaritySearch(const std::string& searchConditions, std::string& searchResult)
{
    char* output = nullptr;
    int errorCode = -1;
    aisdk_data_management_processor_call_similarity_search_sync(
        delegate_, searchConditions.c_str(), uid_,
        G_DBUS_CALL_FLAGS_NONE, 600000,
        &output, &errorCode, nullptr, nullptr);

    if (output != nullptr)
        searchResult = output;

    return errorCode;
}

int DataManagementProcessorProxy::addTextFiles(const std::string &fileinfos)
{
    int errorCode = -1;
    aisdk_data_management_processor_call_add_text_files_sync(
        delegate_, fileinfos.c_str(), uid_,
        G_DBUS_CALL_FLAGS_NONE, 3600000,
        &errorCode, nullptr, nullptr);

    return errorCode;
}

int DataManagementProcessorProxy::addImageFiles(const std::string &fileinfos)
{
    int errorCode = -1;
    aisdk_data_management_processor_call_add_image_files_sync(
        delegate_, fileinfos.c_str(), uid_,
        G_DBUS_CALL_FLAGS_NONE, 3600000,
        &errorCode, nullptr, nullptr);

    return errorCode;
}

int DataManagementProcessorProxy::deleteFiles(const std::string &fileinfos)
{
    int errorCode = -1;
    aisdk_data_management_processor_call_delete_files_sync(
        delegate_, fileinfos.c_str(), uid_,
        G_DBUS_CALL_FLAGS_NONE, 600000,
        &errorCode, nullptr, nullptr);

    return errorCode;
}

int DataManagementProcessorProxy::updateFilesName(const std::string &fileinfos)
{
    int errorCode = -1;
    aisdk_data_management_processor_call_update_files_name_sync(
        delegate_, fileinfos.c_str(), uid_,
        G_DBUS_CALL_FLAGS_NONE, 600000,
        &errorCode, nullptr, nullptr);

    return errorCode;
}

int DataManagementProcessorProxy::updateContentData(const std::string &fileinfos)
{
    int errorCode = -1;
    aisdk_data_management_processor_call_update_files_content_data_sync(
        delegate_, fileinfos.c_str(), uid_,
        G_DBUS_CALL_FLAGS_NONE, 600000,
        &errorCode, nullptr, nullptr);

    return errorCode;
}

int DataManagementProcessorProxy::getAllFileinfos(std::string &result)
{
    char* output = nullptr;
    int errorCode = -1;
    aisdk_data_management_processor_call_get_all_fileinfos_sync(
        delegate_, uid_,
        G_DBUS_CALL_FLAGS_NONE, 600000,
        &output, &errorCode, nullptr, nullptr);

    if (output != nullptr)
        result = output;

    return errorCode;
}
