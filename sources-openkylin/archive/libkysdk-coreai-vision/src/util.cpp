/*
 * Copyright 2025 KylinSoft Co., Ltd.
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

#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include <sys/stat.h>
#include <unistd.h>

#include <fstream>
#include <sstream>

namespace util {

std::string base64Encode(const void *data, std::size_t size) {
    if (data == nullptr) {
        return "";
    }

    BIO *bio{nullptr};
    BIO *b64{nullptr};
    BUF_MEM *bufferPtr{nullptr};

    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_write(bio, data, static_cast<int>(size));
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);
    std::string result(bufferPtr->data, bufferPtr->length);
    BIO_free_all(bio);
    return result;
}

std::string base64Encode(const std::vector<uint8_t> &input) { return base64Encode(input.data(), input.size()); }

std::string base64Encode(const std::string &input) { return base64Encode(input.data(), input.size()); }

std::vector<unsigned char> base64Decode(const std::string &encoded) {
    std::vector<unsigned char> decoded;
    BIO *bio, *b64;
    int decodeLen = encoded.size() * 3 / 4;
    decoded.resize(decodeLen);
    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_new_mem_buf(encoded.data(), encoded.size());
    bio = BIO_push(b64, bio);
    int len = BIO_read(bio, decoded.data(), encoded.size());
    if (len > 0)
        decoded.resize(len);
    else
        decoded.clear();
    BIO_free_all(bio);
    return decoded;
}

bool parseJsonString(const std::string &jsonStr, Json::Value &root) {
    Json::CharReaderBuilder builder;
    std::string errs;
    std::istringstream iss(jsonStr);
    return Json::parseFromStream(builder, iss, &root, &errs);
}

bool isDirectory(const std::string &filePath) {
    struct stat pathStat;
    if (stat(filePath.c_str(), &pathStat) == 0) {
        return S_ISDIR(pathStat.st_mode);  // 检查是否为目录
    }
    return false;
}

bool fileExists(const std::string &filePath) {
    if (filePath.empty()) {
        return false;
    }

    // 检查是否为绝对路径
    if (filePath[0] != '/') {
        return false;
    }

    if (isDirectory(filePath)) {
        return false;
    }

    // 检查文件是否存在(文件存在但没有读权限，返回true)
    struct stat fileStat;
    return stat(filePath.c_str(), &fileStat) == 0;
}

bool hasReadPermission(const std::string &filePath) { return access(filePath.c_str(), R_OK) == 0; }
}  // namespace util
