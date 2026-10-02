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

#ifndef _DATAMANAGEMENTSESSION_H
#define _DATAMANAGEMENTSESSION_H

#include <kylin-ai/business/data-management/common.h>

#include <memory>

#include "datamanagementserviceproxy.h"

#define STATUS_SUCCESS InitializeStatus(true, 0, "")
#define STATUS_FAILURE(errorCode, errorMsg) InitializeStatus(false, errorCode, errorMsg)
#define DATA_MANAGEMENT_STATUS(errorCode, errorMsg) InitializeStatus(errorCode == 0, errorCode, errorMsg)

extern DataManagementStatus InitializeStatus(bool success, int errorCode, const char* errorMsg);

namespace datamanagement {

class _DataManagementSession {
public:
    _DataManagementSession() : proxy_(std::make_shared<DataManagementServiceProxy>()) {};

    DataManagementStatus checkFileFormat(const std::string& filePath, std::string& formatResult);
    DataManagementStatus similaritySearch(const std::string& searchConditions, std::string& searchResult);
    DataManagementStatus addTextFiles(const std::string& fileinfos);
    DataManagementStatus addImageFiles(const std::string& fileinfos);
    DataManagementStatus deleteFiles(const std::string& fileinfos);
    DataManagementStatus updateFilesName(const std::string& fileinfos);
    DataManagementStatus updateContentData(const std::string& fileinfos);
    DataManagementStatus getAllFileinfos(std::string& result);
    DataManagementStatus getFeatureStatus(int& status);
    DataManagementStatus getTagsOfFiles(const std::string& files, std::string& result);
    DataManagementStatus getAllTags(std::string& result);
    DataManagementStatus searchSimilarTags(const std::string& tags, std::string& result);
    DataManagementStatus searchFilesAboutTags(const std::string& tags, std::string& result);
    DataManagementStatus isFileRelevantToTags(const std::string& fileInfos, bool& isRelevant);
    DataManagementStatus isFileRelevantToDescription(const std::string& fileInfos, bool& isRelevant);
    DataManagementStatus extractSearchConditions(const std::string& description, std::string& conditions);
    DataManagementStatus getSummarysOfFiles(const std::string& files, std::string& summary);
    bool connect();

private:
    std::shared_ptr<DataManagementServiceProxy> proxy_;
};

}  // namespace datamanagement

#endif  // _DATAMANAGEMENTSESSION_H
