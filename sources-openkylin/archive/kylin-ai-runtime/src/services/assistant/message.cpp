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

#include "message.h"

#include <jsoncpp/json/value.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>

#include <cstdio>
#include <fstream>
#include <sstream>

#include "assistant/error.h"
#include "utils/logger.h"
#include "utils/util.h"

namespace assistant::message {

int informationSourceStartId = 100;

#ifdef RUN_IN_V101_2503
const std::map<std::string, session::InformationSource> informationSources = {
    {"accessories-text-editor", {"文件编辑器", "用户手册->文本编辑器", informationSourceStartId}},
    {"atril", {"文件查看器", "用户手册->文档查看器", ++informationSourceStartId}},
    {"biometric-manager", {"生物特征管理工具", "用户手册->生物特征管理工具", ++informationSourceStartId}},
    {"box-manager", {"文件保护箱", "用户手册->文件保护箱", ++informationSourceStartId}},
    {"engrampa", {"归档管理器", "用户手册->归档管理器", ++informationSourceStartId}},
    {"filezilla", {"FTP客户端", "用户手册->FTP客户端", ++informationSourceStartId}},
    {"gparted", {"分区编辑器", "用户手册->分区编辑器", ++informationSourceStartId}},
    {"hot-key", {"快捷键", "用户手册->快捷键", ++informationSourceStartId}},
    {"kolourpaint", {"画图", "用户手册->画图", ++informationSourceStartId}},
    {"ksc-defender", {"安全中心", "用户手册->安全中心", ++informationSourceStartId}},
    {"kylin-activation", {"系统激活", "用户手册->系统激活", ++informationSourceStartId}},
    {"kylin-aiassistant", {"AI助手", "用户手册->AI助手", ++informationSourceStartId}},
    {"kylin-alarm-clock", {"闹钟", "用户手册->闹钟", ++informationSourceStartId}},
    {"kylin-burner", {"刻 录", "用户手册->刻 录", ++informationSourceStartId}},
    {"kylin-calculator", {"计算器", "用户手册->计算器", ++informationSourceStartId}},
    {"kylin-camera", {"摄像头", "用户手册->摄像头", ++informationSourceStartId}},
    {"kylin-connectivity", {"多端协同", "用户手册->多端协同", ++informationSourceStartId}},
    {"kylin-file-crush", {"文件粉碎机", "用户手册->文件粉碎机", ++informationSourceStartId}},
    {"kylin-font-viewer", {"字体管理器", "用户手册->字体管理器", ++informationSourceStartId}},
    {"kylin-installer", {"安装器", "用户手册->安装器", ++informationSourceStartId}},
    {"kylin-ipmsg", {"传书", "用户手册->传书", ++informationSourceStartId}},
    {"kylin-log-viewer", {"日志查看器", "用户手册->日志查看器", ++informationSourceStartId}},
    {"kylin-music", {"音乐", "用户手册->音乐", ++informationSourceStartId}},
    {"kylin-notebook", {"便签贴", "用户手册->便签贴", ++informationSourceStartId}},
    {"kylin-os-installer", {"系统安装", "用户手册->系统安装", ++informationSourceStartId}},
    {"kylin-os-manager", {"麒麟管家", "用户手册->麒麟管家", ++informationSourceStartId}},
    {"kylin-photo-viewer", {"看图", "用户手册->看图", ++informationSourceStartId}},
    {"kylin-printer", {"打印机", "用户手册->打印机", ++informationSourceStartId}},
    {"kylin-recorder", {"录 音", "用户手册->录 音", ++informationSourceStartId}},
    {"kylin-scanner", {"扫 描", "用户手册->扫 描", ++informationSourceStartId}},
    {"kylin-screenshot", {"截 图", "用户手册->截 图", ++informationSourceStartId}},
    {"kylin-software-center", {"软件商店", "用户手册->软件商店", ++informationSourceStartId}},
    {"kylin-usb-creator", {"U盘启动器", "用户手册->U盘启动器", ++informationSourceStartId}},
    {"kylin-user-desktop", {"桌面环境", "用户手册->桌面环境", ++informationSourceStartId}},
    {"kylin-video", {"影音", "用户手册->影音", ++informationSourceStartId}},
    {"kylin-weather", {"天气", "用户手册->天气", ++informationSourceStartId}},
    {"onboard", {"屏幕键盘", "用户手册->屏幕键盘", ++informationSourceStartId}},
    {"org.remmina.Remmina", {"远程桌面客户端", "用户手册->远程桌面客户端", ++informationSourceStartId}},
    {"kylin-wacom", {"触控板", "用户手册->触控板", ++informationSourceStartId}},
    {"system-file-manager", {"文件管理器", "用户手册->文件管理器", ++informationSourceStartId}},
    {"time-shutdown", {"定时关机", "用户手册->定时关机", ++informationSourceStartId}},
    {"transmission", {"BT下载工具", "用户手册->BT下载工具", ++informationSourceStartId}},
    {"ukui-control-center", {"设置", "用户手册->设置", ++informationSourceStartId}},
    {"ukui-system-monitor", {"系统监视器", "用户手册->系统监视器", ++informationSourceStartId}},
    {"utilities-terminal", {"终 端", "用户手册->终 端", ++informationSourceStartId}},
    {"yhkylin-backup-tools", {"备份还原", "用户手册->备份还原", ++informationSourceStartId}},
};
#else
const std::map<std::string, session::InformationSource> informationSources = {
    {"accessories-text-editor", {"文件编辑器", "用户手册->文本编辑器", informationSourceStartId}},
    {"baobab", {"磁盘使用情况分析器", "用户手册->磁盘使用情况分析器", ++informationSourceStartId}},
    {"biometric-manager", {"生物特征管理工具", "用户手册->生物特征管理工具", ++informationSourceStartId}},
    {"box-manager", {"文件保护箱", "用户手册->文件保护箱", ++informationSourceStartId}},
    {"d-feet", {"D-Feet", "用户手册->D-Feet", ++informationSourceStartId}},
    {"engrampa", {"归档管理器", "用户手册->归档管理器", ++informationSourceStartId}},
    {"gparted", {"分区编辑器", "用户手册->分区编辑器", ++informationSourceStartId}},
    {"hot-key", {"快捷键", "用户手册->快捷键", ++informationSourceStartId}},
    {"kolourpaint", {"画图", "用户手册->画图", ++informationSourceStartId}},
    {"ksc-defender", {"安全中心", "用户手册->安全中心", ++informationSourceStartId}},
    {"kylin-activation", {"系统激活", "用户手册->系统激活", ++informationSourceStartId}},
    {"kylin-aiassistant", {"AI助手", "用户手册->AI助手", ++informationSourceStartId}},
    {"kylin-ai-memorymap", {"记忆地图", "用户手册->记忆地图", ++informationSourceStartId}},
    {"kylin-file-crush", {"文件粉碎机", "用户手册->文件粉碎机", ++informationSourceStartId}},
    {"kylin-installer", {"安装器", "用户手册->安装器", ++informationSourceStartId}},
    {"kylin-log-viewer", {"日志查看器", "用户手册->日志查看器", ++informationSourceStartId}},
    {"kylin-note", {"麒麟速记", "用户手册->麒麟速记", ++informationSourceStartId}},
    {"kylin-os",
     {"银河麒麟最终用户使用许可协议和隐私政策声明", "用户手册->银河麒麟最终用户使用许可协议和隐私政策声明",
      ++informationSourceStartId}},
    {"kylin-os-installer", {"安装器", "用户手册->安装器", ++informationSourceStartId}},
    {"kylin-os-manager", {"麒麟管家", "用户手册->麒麟管家", ++informationSourceStartId}},
    {"kylin-photo-viewer", {"看图", "用户手册->看图", ++informationSourceStartId}},
    {"kylin-printer", {"打印机", "用户手册->打印机", ++informationSourceStartId}},
    {"kylin-screencap", {"录 屏", "用户手册->录 屏", ++informationSourceStartId}},
    {"kylin-screenshot", {"截 图", "用户手册->截 图", ++informationSourceStartId}},
    {"kylin-software-center", {"软件商店", "用户手册->软件商店", ++informationSourceStartId}},
    {"kylin-telnet", {"网络连通性检测工具", "用户手册->网络连通性检测工具", ++informationSourceStartId}},
    {"kylin-user-desktop", {"桌面环境", "用户手册->桌面环境", ++informationSourceStartId}},
    {"kylin-weather", {"天气", "用户手册->天气", ++informationSourceStartId}},
    {"system-file-manager", {"文件管理器", "用户手册->文件管理器", ++informationSourceStartId}},
    {"time-shutdown", {"定时关机", "用户手册->定时关机", ++informationSourceStartId}},
    {"ukui-control-center", {"设置", "用户手册->设置", ++informationSourceStartId}},
    {"ukui-system-monitor", {"系统监视器", "用户手册->系统监视器", ++informationSourceStartId}},
    {"utilities-terminal", {"终 端", "用户手册->终 端", ++informationSourceStartId}},
    {"yhkylin-backup-tools", {"备份还原", "用户手册->备份还原", ++informationSourceStartId}},
};
#endif

const std::string userGuidePathPrefix = "/usr/share/kylin-user-guide/data/guide/";

Json::Value createJsonInformationSources(const std::set<session::InformationSource> &sources) {
    Json::Value sourcesJson;
    for (const auto &source : sources) {
        Json::Value sourceJson;
        sourceJson["text"] = source.url;
        sourceJson["tool_id"] = source.id;
        sourcesJson.append(sourceJson);
    }
    return sourcesJson;
}

Json::Value createToolInformationSources(const std::set<DialogueCallback::KBChatInfo> &sources) {
    Json::Value sourcesJson;
    for (const auto &source : sources) {
        Json::Value sourceJson;
        sourceJson["path"] = source.toolCmd;
        sourceJson["index"] = source.toolCmdId;
        sourcesJson.append(sourceJson);
    }
    return sourcesJson;
}

Json::Value createImageDataMessageItem(const ImageData &imageData, bool isGenerating) {
    Json::Value fileDataItem;
    fileDataItem["type"] = "file_data";
    if (imageData.success) {
        fileDataItem["status"] = isGenerating ? "generating" : "generated";
        if (!isGenerating && imageData.imageBase64Data.empty()) {
            fileDataItem["status"] = "canceled";
        }
    } else {
        fileDataItem["status"] = "failed";
    }
    std::string fileData;
    if (!imageData.imageBase64Data.empty()) {
        fileData = "image/" + imageData.imageFormat + ";base64," + imageData.imageBase64Data;
    }
    fileDataItem["file_data"] = fileData;
    return fileDataItem;
}

bool isSourceUrlToolCall(const Json::Value &jsonMessage) {
    return jsonMessage.isMember("file_type") && jsonMessage["file_type"].asString() == "source_url";
}

bool isStreamChatResponse(const Json::Value &json) {
    if (json.isMember("result") && json.isMember("is_end")) {
        return true;
    }
    return false;
}

int toolCallIdFromMessage(const Json::Value &json) {
    if (json.isMember("content") && json["content"].isArray() && json["content"].size() > 0) {
        for (unsigned int i = 0; i < json["content"].size(); i++) {
            if (json["content"][i].isMember("type") && json["content"][i]["type"].asString() == "tool_id" &&
                json["content"][i].isMember("tool_id") && json["content"][i]["tool_id"].isInt()) {
                return json["content"][i]["tool_id"].asInt();
            }
        }
    }
    return -1;
}

std::string createErrorMessageResult(int errorCode, const std::string &errorMessage) {
    Json::Value result;
    result["error_code"] = errorCode;
    result["error_message"] = errorMessage;
    return util::jsonToString(result);
}

std::string createImageDataMessage(const ImageData &imageData, bool isGenerating, const std::string &feedbackUrl) {
    Json::Value message;
    message["message_type"] = "image_generation";
    message["content"].append(createImageDataMessageItem(imageData, isGenerating));
    message["model"] = imageData.modelName;
    message["error_code"] = imageData.errorCode;
    message["error_message"] = imageData.errorMessage;
    if (!feedbackUrl.empty()) {
        message["feedback_url"] = feedbackUrl;
    }
    return util::jsonToString(message);
}

std::string createImageDataMessageWithErrorCode(const std::string &modelName, const std::string &messageType,
                                                int errorCode, const std::string &errorMessage) {
    Json::Value item;
    item["type"] = "file_data";
    item["status"] = "failed";
    Json::Value message;
    message["message_type"] = messageType;
    message["content"].append(item);
    message["model"] = modelName;
    message["error_code"] = errorCode;
    message["error_message"] = errorMessage;
    return util::jsonToString(message);
}

std::string createTextMessageFromStreamChatResult(const Json::Value &jsonResult, const std::string &modelName,
                                                  const std::string &feedbackUrl) {
    if (!isStreamChatResponse(jsonResult)) {
        return {};
    }

    const std::string chatResult = jsonResult["result"].asString();
    const int sentenceId = jsonResult["sentence_id"].asInt();
    const bool isEnd = jsonResult["is_end"].asBool();

    Json::Value message;
    Json::Value textMessage;
    message["message_type"] = "chat";
    // 模型名称为空时，UI显示"AI助手"
    message["model"] = modelName;
    textMessage["type"] = "text";
    Json::Value textResultJson;
    textResultJson["sentence_id"] = sentenceId;
    textResultJson["result"] = chatResult;
    if (isEnd && chatResult == "<|stopChat_8167431|>") {
        textResultJson["result"] = "";
    }
    textResultJson["is_end"] = isEnd;
    if (jsonResult.isMember("reasoning_content")) {
        textResultJson["reasoning_content"] = jsonResult["reasoning_content"].asString();
    }
    textMessage["text"] = textResultJson;
    message["content"].append(textMessage);

    if (jsonResult.isMember("information_sources")) {
        message["information_sources"] = jsonResult["information_sources"];
    }
    if (!feedbackUrl.empty()) {
        message["feedback_url"] = feedbackUrl;
    }
    if (jsonResult.isMember("search_info")) {
        message["search_info"] = jsonResult["search_info"];
    }

    return util::jsonToString(message);
}

// /usr/share/kylin-user-guide/data/guide/xxx/zh_CN/index.md
session::InformationSource getInformationSourceFromUserGuidePath(const std::string &path) {
    if (userGuidePathPrefix.length() >= path.length() ||
        path.compare(0, userGuidePathPrefix.length(), userGuidePathPrefix) != 0) {
        return {};
    }

    std::string tempPath = path;
    tempPath.erase(0, userGuidePathPrefix.length());
    size_t pos = tempPath.find_first_of("/");
    if (pos == std::string::npos) {
        return {};
    }

    const std::string appName = tempPath.substr(0, pos);
    LOGI("appName: {}", appName);
    if (informationSources.find(appName) == informationSources.end()) {
        return {};
    }

    return informationSources.at(appName);
}

std::string getAppNameFromSourceId(int id) {
    for (const auto &[appName, source] : informationSources) {
        if (source.id == id) {
            return appName;
        }
    }
    return {};
}

std::string createChatEndMessageResult() {
    Json::Value text;
    text["is_end"] = true;
    text["result"] = "";
    text["sentence_id"] = 1;

    Json::Value textMessage;
    textMessage["type"] = "text";
    textMessage["text"] = text;

    Json::Value message;
    message["message_type"] = "chat";
    // 模型名称为空时，UI显示"AI助手"
    message["model"] = "";
    message["error_code"] = 0;
    message["error_message"] = "";
    message["content"].append(textMessage);

    return util::jsonToString(message);
}

std::string createChatMessage(const std::string &result) {
    Json::Value text;
    text["is_end"] = true;
    text["result"] = result;
    text["sentence_id"] = 1;

    Json::Value textMessage;
    textMessage["type"] = "text";
    textMessage["text"] = text;

    Json::Value message;
    message["message_type"] = "chat";
    // 模型名称为空时，UI显示"AI助手"
    message["model"] = "";
    message["error_code"] = 0;
    message["error_message"] = "";
    message["content"].append(textMessage);

    return util::jsonToString(message);
}

}  // namespace assistant::message
