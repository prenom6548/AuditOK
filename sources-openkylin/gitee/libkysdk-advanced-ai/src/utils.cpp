#include "utils.h"

#include <memory>

#include "types.h"

void kb_free_string(char *str) {
    if (str == nullptr) {
        return;
    }
    delete[] str;
}

void kb_free_string_array(char **str_array, int count) {
    if (str_array == nullptr || count <= 0) {
        return;
    }
    for (int i = 0; i < count; i++) {
        kb_free_string(str_array[i]);
    }
    delete[] str_array;
}

namespace util {
std::string buildJsonString(const std::map<std::string, std::string> &fields) {
    Json::Value root;
    for (const auto &[key, value] : fields) {
        root[key] = value;
    }
    return Json::FastWriter().write(root);
}

std::string buildJsonArray(
    const std::vector<std::map<std::string, std::string>> &items) {
    Json::Value array(Json::arrayValue);

    for (const auto &item : items) {
        Json::Value obj;
        for (const auto &kv : item) {
            obj[kv.first] = kv.second;
        }
        array.append(obj);
    }

    return Json::FastWriter().write(array);
}

std::string buildJsonArray(const std::vector<std::string> &items) {
    Json::Value array(Json::arrayValue);
    for (const auto &str : items) {
        array.append(str);
    }
    return Json::FastWriter().write(array);
}
Json::Value stringToJson(const std::string &string) {
    Json::Value value;
    if (Json::Reader reader; reader.parse(string, value)) {
        return value;
    }
    return Json::objectValue;
}

std::string buildNestedJsonString(
    const std::map<std::string, std::string> &fields,
    const std::set<std::string> &forceStringFields) {
    Json::Value root;

    Json::CharReaderBuilder readerBuilder;
    std::unique_ptr<Json::CharReader> reader(readerBuilder.newCharReader());
    JSONCPP_STRING errs;

    for (const auto &pair : fields) {
        // 如果字段在强制字符串集合中
        if (forceStringFields.find(pair.first) != forceStringFields.end()) {
            root[pair.first] = pair.second;
        } else {
            // 尝试解析值作为JSON
            Json::Value jsonValue;
            if (reader->parse(pair.second.c_str(),
                              pair.second.c_str() + pair.second.length(),
                              &jsonValue, &errs)) {
                // 如果解析成功，使用解析后的JSON值
                root[pair.first] = jsonValue;
            } else {
                // 如果解析失败，将其作为普通字符串处理
                root[pair.first] = pair.second;
            }
        }
    }

    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    return Json::writeString(builder, root);
}

}  // namespace util