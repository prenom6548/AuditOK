#include "utils.h"

#include <kylin-ai/genai/text/chat.h>
#include <kylin-ai/genai/text/chatmessage.h>
#include <kylin-ai/genai/text/config.h>
#include <kylin-ai/genai/text/error.h>
#include <kylin-ai/genai/text/result.h>
#include <openssl/md5.h>
#include <unistd.h>

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace knowledgebase {
std::string stringToUpper(const std::string& str) {
    std::string strUpper = str;
    std::transform(strUpper.begin(), strUpper.end(), strUpper.begin(), ::toupper);
    return strUpper;
}

RetrievalMethod stringToRetrievalMethod(const std::string& str) {
    std::string strUpper = stringToUpper(str);
    if (strUpper == "SEMANTIC") return RetrievalMethod::SEMANTIC;
    if (strUpper == "TEXT") return RetrievalMethod::TEXT;
    if (strUpper == "KEYWORD") return RetrievalMethod::KEYWORD;
    if (strUpper == "HYBRID") return RetrievalMethod::HYBRID;

    return RetrievalMethod::HYBRID;
}

std::string retrievalMethodToString(RetrievalMethod method) {
    switch (method) {
        case RetrievalMethod::SEMANTIC:
            return "SEMANTIC";
        case RetrievalMethod::TEXT:
            return "TEXT";
        case RetrievalMethod::KEYWORD:
            return "KEYWORD";
        case RetrievalMethod::HYBRID:
            return "HYBRID";
        default:
            return "HYBRID";
    }
}

RerankMethod stringToRerankMethod(const std::string& str) {
    std::string strUpper = stringToUpper(str);
    if (strUpper == "RRF") return RerankMethod::RRF;
    if (strUpper == "WEIGHT") return RerankMethod::WEIGHT;

    return RerankMethod::WEIGHT;
}

std::string fileUploadStatusToString(FileUploadStatus status) {
    switch (status) {
        case FileUploadStatus::PENDING:
            return "PENDING";
        case FileUploadStatus::PARSING:
            return "PARSING";
        case FileUploadStatus::EMBEDDING:
            return "EMBEDDING";
        case FileUploadStatus::INDEXING:
            return "INDEXING";
        case FileUploadStatus::COMPLETED:
            return "COMPLETED";
        case FileUploadStatus::ALLCOMPLETED:
            return "ALLCOMPLETED";
        case FileUploadStatus::FAILED:
            return "FAILED";
        default:
            return "FAILED";
    }
}

KBErrorCode genaisdkErrorCodeToKBErrorCode(int errCode) {
    switch (errCode) {
        case AiSdkCommonErrorCode::AISDK_MODEL_NOT_FOUND:
            return KBErrorCode::CHAT_MODEL_NOT_EXIST;
        case AiSdkCommonErrorCode::AISDK_NET_ERROR:
            return KBErrorCode::CHAT_MODEL_NET_ERROR;
        case AiSdkCommonErrorCode::AISDK_AUTHENTICATION_FAILURE:
            return KBErrorCode::CHAT_MODEL_AUTHENTICATION_FAILURE;
        case AiSdkCommonErrorCode::AISDK_RUNTIME_ERROR:
            return KBErrorCode::CHAT_MODEL_RUNTIME_ERROR;
        case AiSdkCommonErrorCode::AISDK_TOO_MANY_REQUESTS:
            return KBErrorCode::CHAT_MODEL_TOO_MANY_REQUESTS;
        case AiSdkCommonErrorCode::AISDK_SERVICE_ERROR:
            return KBErrorCode::CHAT_MODEL_SERVICE_ERROR;
        case AiSdkCommonErrorCode::AISDK_SERVICE_TIMEOUT:
            return KBErrorCode::CHAT_MODEL_SERVICE_TIMEOUT;
        case AiSdkCommonErrorCode::AISDK_BAD_REQUEST:
            return KBErrorCode::CHAT_MODEL_BAD_REQUEST;
        case AiSdkCommonErrorCode::AISDK_MODEL_RUN_FAILED:
            return KBErrorCode::CHAT_MODEL_RUN_FAILED;
        case AiSdkCommonErrorCode::AISDK_MODEL_RUN_TIME_OUT:
            return KBErrorCode::CHAT_MODEL_RUN_TIME_OUT;
        case AiSdkCommonErrorCode::AISDK_INVALID_SESSION:
            return KBErrorCode::CHAT_MODEL_INVALID_SESSION;
        case AiSdkCommonErrorCode::AISDK_SSL_CERTIFICATE_ERROR:
            return KBErrorCode::CHAT_SSL_CERTIFICATE_ERROR;
        case AiSdkCommonErrorCode::AI_COMMON_FREE_ALLOCATED_QUOTA_EXCEEDED:
            return KBErrorCode::CHAT_FREE_QUOTA_EXHAUSTED;
        case AiSdkCommonErrorCode::AI_COMMON_UNKNOWN_ERROR:
            return KBErrorCode::UNKNOWN_ERROR;
        case GenAiTextErrorCode::NLP_INPUT_INVALID:
            return KBErrorCode::INVALID_PARAMETER;
        case GenAiTextErrorCode::NLP_PARAM_ERROR:
            return KBErrorCode::INVALID_PARAMETER;
        default:
            return KBErrorCode::CHAT_MODEL_UNAVAILABLE;
    }
}
std::string getHomeDirectory() {
    // 1. 优先使用 HOME 环境变量
    const char* home_env = std::getenv("HOME");
    if (home_env != nullptr && home_env[0] != '\0') {
        return std::string(home_env);
    }

    // 2. 尝试获取 USER 环境变量
    const char* user_env = std::getenv("USER");
    if (user_env == nullptr || user_env[0] == '\0') {
        // 连 USER 都没有，无法推断，返回当前目录
        return ".";
    }

    std::string username(user_env);

    // 3. 特殊处理 root 用户
    if (username == "root") {
        return "/root";
    }

    // 4. 普通用户：/home/用户名
    return "/home/" + username;
}

// 去除字符串中所有空白字符（空格、换行、制表符等）
std::string removeWhitespace(const std::string& str) {
    std::string result;
    result.reserve(str.size());
    for (char c : str) {
        if (!std::isspace(static_cast<unsigned char>(c)) && c != ' ' && c != '\t' && c != '\n' && c != '\r' &&
            c != '\f' && c != '\v') {
            result += c;
        }
    }
    return result;
}

std::string computeMD5FromFile(const std::string& absolutePath) {
    std::ifstream file(absolutePath, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + absolutePath);
    }

    MD5_CTX md5Context;
    MD5_Init(&md5Context);

    // 使用1MB缓冲区提高大文件处理效率
    std::vector<char> buffer(1024 * 1024);
    while (file.read(buffer.data(), buffer.size()) || file.gcount() > 0) {
        MD5_Update(&md5Context, buffer.data(), file.gcount());
    }

    unsigned char md5Hash[MD5_DIGEST_LENGTH];
    MD5_Final(md5Hash, &md5Context);

    auto bytesToHexString = [](const unsigned char* bytes, size_t length) -> std::string {
        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        for (size_t i = 0; i < length; ++i) {
            ss << std::setw(2) << static_cast<unsigned int>(bytes[i]);
        }
        return ss.str();
    };

    return bytesToHexString(md5Hash, MD5_DIGEST_LENGTH);
}

bool pathReadable(const std::string& path) {
    if (access(path.c_str(), R_OK) == 0) {
        return true;
    } else {
        if (errno == EACCES) {
            std::cerr << "无读权限: " << path << "\n";
        }
        return false;
    }
}

// 判断路径是否可写
bool pathWritable(const std::string& path) {
    if (access(path.c_str(), W_OK) == 0) {
        return true;
    } else {
        if (errno == EACCES) {
            std::cerr << "无写权限: " << path << "\n";
        }
        return false;
    }
}

std::vector<std::string> parseStringToVector(const std::string& input, const char delimiter) {
    std::vector<std::string> result;
    size_t start = 0;
    size_t end = input.find(delimiter);

    while (end != std::string::npos) {
        result.push_back(input.substr(start, end - start));
        start = end + 1;
        end = input.find(delimiter, start);
    }

    // 添加最后一个元素
    result.push_back(input.substr(start));

    return result;
}

std::vector<int> parseStringToIntVector(const std::string& input, const char delimiter) {
    std::vector<int> result;
    std::vector<std::string> stringResult = parseStringToVector(input, delimiter);
    for (auto& str : stringResult) {
        try {
            result.push_back(std::stoi(str));
        } catch (const std::invalid_argument& e) {
            std::cerr << "Invalid argument: " << e.what() << std::endl;
        }
    }

    return result;
}

std::string readFileToString(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("无法打开文件: " + filePath);
    }

    // 获取文件大小
    auto fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // 预分配内存并读取内容
    std::string content(fileSize, '\0');
    file.read(&content[0], fileSize);

    return content;
}

bool createDirectoryIfNotExists(const std::string& path) {
    if (!std::filesystem::exists(path)) {
        if (!std::filesystem::create_directories(path)) {
            std::cerr << "Create DB path failed: " << path << std::endl;
            return false;
        }
        return true;
    }
    return true;
}

}  // namespace knowledgebase