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

#include "baiduresponseprocessor.h"

#include <jsoncpp/json/json.h>

#include "imageloader.h"

static bool isAllSubTaskSucceeded(const Json::Value &subTaskResultList) {
    if (!subTaskResultList.isArray() || subTaskResultList.empty()) {
        return false;
    }

    for (const auto &subTaskResult : subTaskResultList) {
        if (!subTaskResult.isMember("sub_task_status") || subTaskResult["sub_task_status"] != "SUCCESS") {
            return false;
        }
    }

    return true;
}

static ApproveConclusion getApproveConclusion(const std::string &conclusion) {
    if (conclusion == "pass") {
        return ApproveConclusion::PASS;
    }

    if (conclusion == "review") {
        return ApproveConclusion::REVIEW;
    }

    if (conclusion == "block") {
        return ApproveConclusion::BLOCK;
    }

    return ApproveConclusion::UNKNOWN;
}

static bool isTaskBlocked(const Json::Value &subTaskResultList) {
    if (!subTaskResultList.isArray()) {
        return false;
    }

    if (subTaskResultList.size() == 0) {
        return false;
    }

    if (!subTaskResultList[0].isMember("sub_task_error_code")) {
        return false;
    }

    // 文本黄反拦截
    return subTaskResultList[0]["sub_task_error_code"].asInt() == 501;
}

ProcessResult BaiduResponseProcessor::process(const std::string &json, ImageDataList &imageDataList, bool &isBlocked,
                                              std::string &errorMessage) {
    Json::Reader reader;
    Json::Value response;
    isBlocked = false;
    if (!reader.parse(json, response)) {
        return ProcessResult::FAILED;
    }

    if (!response.isMember("data")) {
        fprintf(stderr, "Can not get the 'data' key from result: %s\n", json.c_str());
        errorMessage = json;
        return ProcessResult::FAILED;
    }

    Json::Value data = response["data"];
    if (!data.isMember("task_status")) {
        fprintf(stderr, "Can not get the 'task_status' key from result: %s\n", json.c_str());
        errorMessage = json;
        return ProcessResult::FAILED;
    }

    Json::Value taskStatus = data["task_status"];
    if (taskStatus.asString() == "FAILED") {
        if (data.isMember("sub_task_result_list")) {
            Json::Value subTaskResultList = data["sub_task_result_list"];
            isBlocked = isTaskBlocked(subTaskResultList);
            fprintf(stderr, "Task blocked: %s\n", isBlocked ? "true" : "false");
        }

        fprintf(stderr, "Generate task failed: %s\n", json.c_str());
        errorMessage = json;
        return ProcessResult::FAILED;
    }

    if (taskStatus.asString() != "SUCCESS") {
        return ProcessResult::CONTINUED;
    }

    if (!data.isMember("sub_task_result_list")) {
        return ProcessResult::FAILED;
    }

    if (!data["sub_task_result_list"].isArray()) {
        return ProcessResult::FAILED;
    }

    Json::Value subTaskResultList = data["sub_task_result_list"];
    if (!isAllSubTaskSucceeded(subTaskResultList)) {
        return ProcessResult::CONTINUED;
    }

    imageDataList.clear();

    for (Json::ArrayIndex resultIndex = 0; resultIndex < subTaskResultList.size(); resultIndex++) {
        Json::Value subTaskResult = subTaskResultList[resultIndex];
        if (!subTaskResult.isMember("final_image_list")) {
            continue;
        }

        Json::Value finalImageList = subTaskResult["final_image_list"];
        for (Json::ArrayIndex imageIndex = 0; imageIndex < finalImageList.size(); imageIndex++) {
            imageDataList.push_back(ImageData());
            imageDataList.back().index_ = resultIndex;

            Json::Value finalImage = finalImageList[imageIndex];
            if (!finalImage.isMember("img_approve_conclusion")) {
                continue;
            }
            auto approveConclusion = getApproveConclusion(finalImage["img_approve_conclusion"].asString());
            if (approveConclusion == ApproveConclusion::UNKNOWN) {
                continue;
            }

            if (approveConclusion == ApproveConclusion::BLOCK) {
                imageDataList.back().conclusion_ = ApproveConclusion::BLOCK;
                continue;
            }

            if (approveConclusion == ApproveConclusion::PASS) {
                if (!finalImage.isMember("img_url")) {
                    continue;
                }

                auto imageUrl = finalImage["img_url"].asString();
                if (imageUrl.empty()) {
                    continue;
                }
                auto imageData = vision::image_loader::downloadImage(imageUrl, errorMessage);
                imageDataList.back().data_ = imageData;
                imageDataList.back().conclusion_ = ApproveConclusion::PASS;
            }
        }
    }

    return ProcessResult::SUCCEEDED;
}
