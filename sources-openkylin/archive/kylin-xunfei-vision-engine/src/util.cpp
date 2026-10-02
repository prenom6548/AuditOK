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
#include <time.h>
#include <iomanip>
#include <memory>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/md5.h>
#include <openssl/sha.h>

Json::Value xunfei_vision_util::formatJsonFromString(const std::string &stringJson) {
    Json::Value obj;
    std::string error;
    Json::CharReaderBuilder crbuilder;
    std::unique_ptr<Json::CharReader> reader(crbuilder.newCharReader());
    reader->parse(stringJson.c_str(), stringJson.c_str() + stringJson.size(), &obj, &error);
    return obj;
}

std::string xunfei_vision_util::base64Encode(const std::string &input) {
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

std::string xunfei_vision_util::base64Decode(const std::string &input) {
    // Base64 字符集
    const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                     "abcdefghijklmnopqrstuvwxyz"
                                     "0123456789+/";

    std::vector<unsigned char> decoded_data;

    // 解码 Base64 编码的字符串
    size_t in_len = input.size();
    size_t i = 0;
    unsigned char char_array_4[4], char_array_3[3];
    int j = 0;
    while (in_len-- && (input[i] != '=') && (isalnum(input[i]) || (input[i] == '+') || (input[i] == '/'))) {
        char_array_4[j++] = static_cast<unsigned char>(input[i]);
        ++i;
        if (j == 4) {
            for (j = 0; j < 4; ++j)
                char_array_4[j] = base64_chars.find(char_array_4[j]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0x0F) << 4) + ((char_array_4[2] & 0x3C) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x03) << 6) + char_array_4[3];

            for (j = 0; j < 3; ++j)
                decoded_data.push_back(char_array_3[j]);
            j = 0;
        }
    }

    if (j) {
        for (int k = j; k < 4; ++k)
            char_array_4[k] = 0;

        for (int k = 0; k < 4; ++k)
            char_array_4[k] = base64_chars.find(char_array_4[k]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0x0F) << 4) + ((char_array_4[2] & 0x3C) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x03) << 6) + char_array_4[3];

        for (int k = 0; k < j - 1; ++k)
            decoded_data.push_back(char_array_3[k]);
    }

    // 转换解码后的数据为字符串
    std::string decoded_string(decoded_data.begin(), decoded_data.end());
    return decoded_string;
}

std::string xunfei_vision_util::urlEncode(const std::string &input) {
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

std::unordered_map<std::string, std::string> xunfei_vision_util::parseURL(const std::string &url) {
    std::string protocol, host, port, path, query, fragment;

    // 解析协议部分
    size_t protocolEnd = url.find("://");
    if (protocolEnd != std::string::npos) {
        protocol = url.substr(0, protocolEnd);
    }

    // 解析主机和端口部分
    size_t hostStart = protocolEnd != std::string::npos ? protocolEnd + 3 : 0;
    size_t hostEnd = url.find_first_of(":/", hostStart);
    if (hostEnd != std::string::npos) {
        size_t portStart = url[hostEnd] == ':' ? hostEnd + 1 : std::string::npos;
        size_t portEnd = url.find("/", hostEnd);
        host = url.substr(hostStart, hostEnd - hostStart);
        port = (portStart != std::string::npos && portEnd != std::string::npos)
                   ? url.substr(portStart, portEnd - portStart)
                   : "";
    }

    // 解析路径、查询字符串和片段部分
    size_t pathStart = url.find("/", hostEnd);
    size_t queryStart = url.find("?", pathStart);
    size_t fragmentStart = url.find("#", queryStart);
    path = url.substr(pathStart, queryStart - pathStart);
    query = queryStart != std::string::npos ? url.substr(queryStart + 1, fragmentStart - queryStart - 1) : "";
    fragment = fragmentStart != std::string::npos ? url.substr(fragmentStart + 1) : "";

    std::unordered_map<std::string, std::string> urlParts = {{"Protocol", protocol}, {"Host", host},
                                                             {"Port", port},         {"Path", path},
                                                             {"Query", query},       {"Fragment", fragment}};

    return urlParts;
}

std::string xunfei_vision_util::hmacSha256Encode(const std::string &data, const std::string &key) {
    unsigned char digest[SHA256_DIGEST_LENGTH];
    HMAC_CTX *ctx = HMAC_CTX_new();

    HMAC_Init_ex(ctx, key.c_str(), key.length(), EVP_sha256(), NULL);
    HMAC_Update(ctx, reinterpret_cast<const unsigned char *>(data.c_str()), data.length());
    HMAC_Final(ctx, digest, NULL);

    HMAC_CTX_free(ctx);

    BIO *bio, *b64;
    BUF_MEM *bptr;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);
    BIO_write(bio, digest, SHA256_DIGEST_LENGTH);
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bptr);

    std::string result(bptr->data, bptr->length - 1);
    BIO_free_all(bio);

    return result;
}

std::string xunfei_vision_util::getGmtTime() {
    setlocale(LC_TIME, "C");
    time_t rawTime = time(0);
    tm *gmt = gmtime(&rawTime);
    const char *fmt = "%a, %d %b %Y %H:%M:%S GMT";
    char tstr[30];
    strftime(tstr, sizeof(tstr), fmt, gmt);
    return tstr;
}