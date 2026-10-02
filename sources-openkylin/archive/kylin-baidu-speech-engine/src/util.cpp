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

#include "util.h"

#include <sys/time.h>

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <memory>
#include <thread>

#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>

#include "logger.h"

namespace {
bool isDelimiter(char c) {
    return c == ' ' || c == '\n' || c == '.' || c == ',' || c == ';' || c == ':' || c == '!' || c == '?';
}
} // namespace

Json::Value baidu_speech_util::formatJsonFromString(const std::string &stringJson) {
    Json::Value obj;
    std::string error;
    Json::CharReaderBuilder crbuilder;
    std::unique_ptr<Json::CharReader> reader(crbuilder.newCharReader());
    reader->parse(stringJson.c_str(), stringJson.c_str() + stringJson.size(), &obj, &error);
    return obj;
}

std::string baidu_speech_util::base64Encode(const std::string &input) {
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;
    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);
    BIO_write(bio, input.c_str(), input.length());
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);
    std::string result(bufferPtr->data, bufferPtr->length);

    BIO_free_all(bio);

    return result;
}

std::string baidu_speech_util::base64Decode(const std::string &input) {
    BIO *bio{nullptr};
    BIO *b64{nullptr};
    char *buffer = new char[input.size()];
    std::copy(input.begin(), input.end(), buffer);

    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_new_mem_buf(buffer, static_cast<int>(input.size()));
    bio = BIO_push(b64, bio);

    int decodedLength = BIO_read(bio, buffer, static_cast<int>(input.size()));
    std::string result(buffer, decodedLength);
    BIO_free_all(bio);
    delete[] buffer;

    return result;
}

std::string baidu_speech_util::urlEncode(const std::string &input) {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (auto &&ch : input) {
        if (std::isalnum(ch) || ch == '-' || ch == '_' || ch == '.' || ch == '~') {
            escaped << ch;
            continue;
        }
        escaped << std::uppercase;
        escaped << '%' << std::setw(2) << int((unsigned char)ch);
        escaped << std::nouppercase;
    }

    return escaped.str();
}

std::vector<std::string> baidu_speech_util::segmentString(const std::string &str) {
    std::vector<std::string> chunks;
    std::string currentChunk;
    size_t lastDelimiterPos = std::string::npos;
    int maxChunkSize = 500;

    for (size_t i = 0; i < str.length(); ++i) {
        currentChunk += str[i];

        if (isDelimiter(str[i])) {
            lastDelimiterPos = i;
        }

        if (currentChunk.length() >= maxChunkSize) {
            if (lastDelimiterPos != std::string::npos && lastDelimiterPos < currentChunk.size()) {
                chunks.push_back(currentChunk.substr(0, lastDelimiterPos + 1));
                currentChunk = currentChunk.substr(lastDelimiterPos + 1);
                lastDelimiterPos = std::string::npos;
            } else {
                // TODO 适配中文符号
                //      maxChunkSize设为500，有概率会强行截断一个汉字字符
                //      暂时这么改，后续优化
                // 如果没有找到合适的分隔符，则找到最近的完整字符位置进行截断
                replaceAll(currentChunk, "。", ".");
                replaceAll(currentChunk, "，", ",");
                replaceAll(currentChunk, "；", ";");
                replaceAll(currentChunk, "：", ":");
                replaceAll(currentChunk, "！", "!");
                replaceAll(currentChunk, "？", "?");

                size_t safeBreakPos = currentChunk.find_last_of(" \n.,;:!?");
                if (safeBreakPos == std::string::npos) {
                    safeBreakPos = maxChunkSize; // 强制截断
                }
                chunks.push_back(currentChunk.substr(0, safeBreakPos + 1));
                currentChunk = currentChunk.substr(safeBreakPos + 1);
            }
        }
    }

    if (!currentChunk.empty()) {
        chunks.push_back(currentChunk);
    }

    return chunks;
}

void baidu_speech_util::sleepForSec(const int sec) { std::this_thread::sleep_for(std::chrono::seconds(sec)); }

void baidu_speech_util::sleepForMs(const int ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }

void baidu_speech_util::replaceAll(std::string &str, const std::string &from, const std::string &to) {
    if (from.empty())
        return;
    size_t startPos = 0;
    while ((startPos = str.find(from, startPos)) != std::string::npos) {
        str.replace(startPos, from.length(), to);
        startPos += to.length();
    }
}

int64_t baidu_speech_util::getTimestampMs() {
    struct timeval tv{};
    gettimeofday(&tv, nullptr);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

std::vector<std::string> baidu_speech_util::split(const std::string &str, const std::string &delim) {
    std::vector<std::string> result;
    size_t begin = 0;
    for (size_t end; end = str.find(delim, begin), end != std::string::npos; begin = end + delim.size()) {
        result.emplace_back(str.substr(begin, end - begin));
    }
    result.emplace_back(str.substr(begin));
    return result;
}

void baidu_speech_util::parseAudioBasicContentTypeMessage(const std::string &message, std::string &format, int &rate,
                                                          int &channel) {
    auto segments = baidu_speech_util::split(message, ";");
    for (const auto &segment : segments) {
        auto fields = baidu_speech_util::split(segment, "=");
        if (fields.size() == 2) {
            if (fields[0] == "codec") {
                format = fields[1];
            } else if (fields[0] == "rate") {
                rate = std::stoi(fields[1]);
            } else if (fields[0] == "channel") {
                channel = std::stoi(fields[1]);
            }
        }
    }
}

bool baidu_speech_util::isAllDigits(const std::string &string) {
    if (string.empty()) {
        return false;
    }
    return std::all_of(string.begin(), string.end(),
                       [](const char c) { return std::isdigit(static_cast<unsigned char>(c)); });
}
