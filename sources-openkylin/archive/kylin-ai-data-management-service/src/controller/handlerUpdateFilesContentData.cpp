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

#include <scene.h>

#include <thread>

#include "aiIndex.h"
#include "fileInfoService.h"
#include "imageService.h"
#include "tagService.h"

namespace {

bool isSupportedImageFormat(const std::string& format) {
    const std::vector<std::string> supportFormat = {"png", "jpg", "jpeg", "jpe", "bmp", "dib"};
    return std::find(supportFormat.begin(), supportFormat.end(), format) != supportFormat.end();
}

bool isSupportedTextFormat(const std::string& format) {
    const std::vector<std::string> supportFormat = {"txt",  "pdf",  "docx", "pptx", "xlsx", "xls",
                                                    "xlsd", "html", "xml",  "md",   "json"};
    return std::find(supportFormat.begin(), supportFormat.end(), format) != supportFormat.end();
}

void parseFilePaths(const nlohmann::json& values, std::unordered_set<std::string>& textFilePaths,
                    std::unordered_set<std::string>& imgFilePaths) {
    for (const auto& item : values) {
        if (!item.is_object() || !item.contains("filepath") || !item["filepath"].is_string() ||
            !item.contains("fileformat") || !item["fileformat"].is_string()) {
            PRINT_ERROR("handleUpdateFilesContentData request json is error.\n");
            continue;
        }
        auto filePath = item["filepath"].get<std::string>();

        if (isSupportedTextFormat(item["fileformat"].get<std::string>())) {
            textFilePaths.insert(filePath);
        } else if (isSupportedImageFormat(item["fileformat"].get<std::string>())) {
            imgFilePaths.insert(filePath);
        } else {
            PRINT_ERROR("handleUpdateFilesContentData unknown type: %s.\n",
                        item["fileformat"].get<std::string>().c_str());
            continue;
        }
    }
}

}  // namespace

Response handleUpdateFilesContentData(const Request& request) {
    PRINT_DEBUG("handleUpdateFilesContentData request: %s\n", request.toString().c_str());

    // 解析参数
    nlohmann::json values = request.getValues();
    if (!values.is_array() || values.empty()) {
        PRINT_ERROR("handleUpdateFilesContentData request is not array or is empty.\n");
        return Response(PARSE_PARAM_FAILED);
    }

    std::unordered_set<std::string> textFilePaths;  // 需要添加的文本文件
    std::unordered_set<std::string> imgFilePaths;   // 需要添加的图片
    parseFilePaths(values, textFilePaths, imgFilePaths);

    if (!textFilePaths.empty()) {
        std::unordered_set<std::string> newFilePaths;
        std::vector<FileInfoDO> updateFileInfos;

        FileInfoService().classifyFile(textFilePaths, newFilePaths, updateFileInfos);
        if (!newFilePaths.empty()) {
            FileInfoService().insertTask(newFilePaths);
        }

        if (!updateFileInfos.empty()) {
            std::unordered_set<int64_t> updateIds = FileInfoService().updateTask(updateFileInfos);

            if (!updateIds.empty()) {
                // 需要更新的文件 先删掉，等待索引重建
                FileInfoService::deleteTextFileVectorInfo(updateIds);
                TagService().deleteFileTags(updateIds);

                // 删完之后 唤醒向量化任务
                DataManagement::AIIndex::getInstance().notifyTaskByType(
                    {DataManagement::TaskType::TASK_TYPE_TEXT_EMBEDDING});
            }
        }
    }

    if (!imgFilePaths.empty()) {
        ImageService().embedding(imgFilePaths);
    }

    return Response::success();
}

REGISTER_SERVICE_HANDLER("updateFilesContentData", handleUpdateFilesContentData)
