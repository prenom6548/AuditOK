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
#include <netdb.h>
#include <time.h>
#include <future>
#include <iomanip>
#include <memory>
#include <thread>

#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/md5.h>
#include <openssl/sha.h>

#include "logger.h"

std::string xunfei_nlp_util::base64Encode(const std::string &input) {
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

std::string xunfei_nlp_util::urlEncode(const std::string &input) {
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

std::unordered_map<std::string, std::string> xunfei_nlp_util::parseURL(const std::string &url) {
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

std::string xunfei_nlp_util::hmacSha256Encode(const std::string &data, const std::string &key) {
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

namespace {

#define NTP_PORT 123
#define NTP_TIMESTAMP_DELTA 2208988800ull // 1900-1970 年之间的秒数
#define TIME_SERVER "cn.pool.ntp.org"     // 你可以换成其他国内NTP服务器

bool resolveHostname(const char *hostname, struct sockaddr_in *addr) {
    struct addrinfo hints{};
    struct addrinfo *res{nullptr};
    struct addrinfo *p{nullptr};

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // IPv4 only

    if (getaddrinfo(hostname, nullptr, &hints, &res) != 0) {
        LOGE("DNS resolution failed!");
        return false;
    }
    for (p = res; p != nullptr; p = p->ai_next) {
        if (p->ai_family == AF_INET) {
            memcpy(addr, p->ai_addr, sizeof(struct sockaddr_in));
            freeaddrinfo(res);
            return true;
        }
    }

    freeaddrinfo(res);
    LOGE("No IPv4 address found for {}!", hostname);
    return false;
}

std::string getNtpServerGmtTime() {

    struct NtpPacket {
        uint8_t li_vn_mode; // 8 bits: LI, VN, Mode
        uint8_t stratum;    // 8 bits: Stratum
        uint8_t poll;
        uint8_t precision;
        uint32_t rootDelay;
        uint32_t rootDispersion;
        uint32_t refId;
        uint32_t refTm_s;
        uint32_t refTm_f;
        uint32_t origTm_s;
        uint32_t origTm_f;
        uint32_t rxTm_s;
        uint32_t rxTm_f;
        uint32_t txTm_s;
        uint32_t txTm_f;
    };

    int socketFd;
    struct sockaddr_in serverAddress{};
    NtpPacket packet = {0};

    // 设置 NTP 请求包
    packet.li_vn_mode = 0x1B; // LI=0 (no warning), VN=4 (IPv4), Mode=3 (client)

    // 创建 UDP socket
    socketFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socketFd < 0) {
        LOGE("Socket creation error!");
        return "";
    }
    struct timeval timeout{};
    timeout.tv_sec = 0; // 超时时间为 5 秒
    timeout.tv_usec = 200 * 1000;

    if (setsockopt(socketFd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        LOGE("Failed to set timeout!");
        close(socketFd);
        return "";
    }

    // 解析域名
    if (!resolveHostname(TIME_SERVER, &serverAddress)) {
        close(socketFd);
        return "";
    }

    serverAddress.sin_port = htons(NTP_PORT);

    // 发送 NTP 请求
    if (sendto(socketFd, (const char *)&packet, sizeof(NtpPacket), 0, (const struct sockaddr *)&serverAddress,
               sizeof(serverAddress)) < 0) {
        LOGE("Send failed!");
        close(socketFd);
        return "";
    }

    // 接收响应
    socklen_t len = sizeof(serverAddress);
    ssize_t n = recvfrom(socketFd, &packet, sizeof(NtpPacket), 0, (struct sockaddr *)&serverAddress, &len);
    if (n < 0) {
        LOGE("Receive failed!");
        close(socketFd);
        return "";
    }

    // 转换为 host 字节序
    packet.txTm_s = ntohl(packet.txTm_s);
    packet.txTm_f = ntohl(packet.txTm_f);

    // 将 NTP 时间戳转换为 Unix 时间戳
    time_t txTm = (time_t)(packet.txTm_s - NTP_TIMESTAMP_DELTA);
    struct tm *gmt = gmtime(&txTm); // UTC 时间

    char buffer[80];
    strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", gmt);
    LOGI("GMT time: ", std::string(buffer));
    close(socketFd);
    return buffer;
}

std::string getLocalGmtTime() {
    setlocale(LC_TIME, "C");
    time_t rawTime = time(0);
    tm *gmt = gmtime(&rawTime);
    const char *fmt = "%a, %d %b %Y %H:%M:%S GMT";
    char tstr[30];
    strftime(tstr, sizeof(tstr), fmt, gmt);

    return tstr;
}

} // namespace

std::string xunfei_nlp_util::getGmtTime() {
    std::string gmtTime;
    for (int i = 0; i < 5; ++i) {
        gmtTime = getNtpServerGmtTime();
        if (!gmtTime.empty()) {
            // 获取成功，退出循环
            break;
        }
        // 获取失败，等待并重试
        LOGW("Failed to get GMT time from NTP server, retrying...");
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    if (gmtTime.empty()) {
        return getLocalGmtTime();
    }
    return gmtTime;
}

std::string xunfei_nlp_util::jsonToString(const Json::Value &value) {
    Json::StreamWriterBuilder builder;
    builder["emitUTF8"] = true;
    return Json::writeString(builder, value);
}

Json::Value xunfei_nlp_util::stringToJson(const std::string &string) {
    Json::Value value;
    if (Json::Reader reader; reader.parse(string, value)) {
        return value;
    } else {
        return Json::objectValue;
    }
}
