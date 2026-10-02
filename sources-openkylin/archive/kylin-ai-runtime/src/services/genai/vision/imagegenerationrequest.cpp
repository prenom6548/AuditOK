#include "imagegenerationrequest.h"

#include <jsoncpp/json/json.h>

#include "utils/util.h"

namespace genai {

bool ImageGenerationConfig::fromString(const std::string& config) {
    std::optional<Json::Value> optional = util::stringToJson2(config);
    if (!optional.has_value()) {
        width = 0;
        height = 0;
        number = 0;
        style = std::string{};
        modelName = std::string{};
        return false;
    }
    const Json::Value& root = optional.value();
    if (root.isMember("width") && root["width"].isInt()) {
        width = root["width"].asInt();
    } else {
        width = 0;
    }
    if (root.isMember("height") && root["height"].isInt()) {
        height = root["height"].asInt();
    } else {
        height = 0;
    }
    if (root.isMember("number") && root["number"].isInt()) {
        number = root["number"].asInt();
    } else {
        number = 0;
    }
    if (root.isMember("style") && root["style"].isString()) {
        style = root["style"].asString();
    } else {
        style = std::string{};
    }
    if (root.isMember("modelName") && root["modelName"].isString()) {
        modelName = root["modelName"].asString();
    } else {
        modelName = std::string{};
    }
    return true;
}

bool ImageGenerationRequest::fromString(const std::string& request) {
    std::optional<Json::Value> optional = util::stringToJson2(request);
    if (!optional.has_value()) {
        width = 0;
        height = 0;
        number = 0;
        style = std::string{};
        modelName = std::string{};
        prompt = std::string{};
        return false;
    }
    const Json::Value& root = optional.value();
    if (root.isMember("width") && root["width"].isInt()) {
        width = root["width"].asInt();
    } else {
        width = 0;
    }
    if (root.isMember("height") && root["height"].isInt()) {
        height = root["height"].asInt();
    } else {
        height = 0;
    }
    if (root.isMember("number") && root["number"].isInt()) {
        number = root["number"].asInt();
    } else {
        number = 0;
    }
    if (root.isMember("style") && root["style"].isString()) {
        style = root["style"].asString();
    } else {
        style = std::string{};
    }
    if (root.isMember("modelName") && root["modelName"].isString()) {
        modelName = root["modelName"].asString();
    } else {
        modelName = std::string{};
    }
    if (root.isMember("prompt") && root["prompt"].isString()) {
        prompt = root["prompt"].asString();
    } else {
        prompt = std::string{};
    }
    return true;
}

std::string ImageGenerationRequest::toString() const {
    Json::Value root;
    root["prompt"] = prompt;
    root["width"] = width;
    root["height"] = height;
    root["number"] = number;
    root["style"] = style;
    root["modelName"] = modelName;
    return util::jsonToString(root);
}

}  // namespace genai
