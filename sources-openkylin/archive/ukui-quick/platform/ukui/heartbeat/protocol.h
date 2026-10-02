/*
* Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef UKUI_QUICK_PROTOCOL_H
#define UKUI_QUICK_PROTOCOL_H

#include <sys/types.h>
#include <QString>
namespace UkuiQuick::HeartBeat
{
inline constexpr std::size_t kMaxProcessNameLength = 32;
inline constexpr std::size_t kMaxMessageLength = 1024;

enum class MessageType : uint8_t {
    RegisterRequest = 0x01,
    RegisterResponse = 0x02,
    StatusQuery = 0x03,
    StatusResponse = 0x04,
    Deregister = 0x05, //废弃
    Error = 0xFF
};

enum class ComponentStatus : uint8_t {
    Normal = 0x01,
    Busy = 0x02,
    Error = 0x03,
    Unresponsive = 0xFF
};

enum class ErrorCode : uint8_t {
    None = 0x00,
    InvalidMessage = 0x01,
    Checksum = 0x02,
    UnknownType = 0x03,
    RegisterFailed = 0x04,
    InvalidRequestId = 0x05,
    RetryExhausted = 0x06
};

enum class CriticalLevel : uint8_t {
    Level1 = 0x01,
    Level2 = 0x02,
    Level3 = 0x03,
    Low = 0xFF
};

typedef struct {
    uint8_t msg_type;
    uint16_t msg_len;
    uint8_t checksum;
} __attribute__((packed)) message_header_t;

typedef struct {
    uint8_t is_critical;
    uid_t uid;
    pid_t pid;
    uint8_t name_len;
    char process_name[kMaxProcessNameLength];
} __attribute__((packed)) register_request_t;

typedef struct {
    uint8_t result_cod;
    uint32_t request_id;
} __attribute__((packed)) register_response_t;

typedef struct {
    uint32_t request_id;
} __attribute__((packed)) status_request_t;

typedef struct {
    uint8_t status_cod;
    time_t last_active_time;
} __attribute__((packed)) status_response_t;

typedef struct {
    uint8_t err_cod;
    char err_msg[8];
} __attribute__((packed)) err_response_t;


class MessageHeader
{
public:
    MessageHeader() = default;
    explicit MessageHeader(const message_header_t &raw);
    ~MessageHeader() = default;
    MessageHeader& operator=(const message_header_t &raw);
    message_header_t &raw();

    [[nodiscard]] const message_header_t &raw() const;
    [[nodiscard]] MessageType type() const;
    [[nodiscard]] std::size_t len() const;
    [[nodiscard]] uint8_t checksum() const;
private:
    message_header_t m_raw{};
};

class RegisterRequest
{
public:
    RegisterRequest() = default;
    explicit RegisterRequest(const register_request_t &raw);
    ~RegisterRequest() = default;

    register_request_t &raw();
    [[nodiscard]] const register_request_t &raw() const;
    [[nodiscard]] bool isCritical() const;
    [[nodiscard]] uid_t uid() const;
    [[nodiscard]] pid_t pid() const;
    [[nodiscard]] QString processName() const;
    void setProcessName(const QString &name);
private:
    register_request_t m_raw{};
};

class RegisterResponse
{
public:
    RegisterResponse() = default;
    explicit RegisterResponse(const register_response_t &raw);
    ~RegisterResponse() = default;

    register_response_t &raw();
    [[nodiscard]] const register_response_t &raw() const;
    [[nodiscard]] ErrorCode errorCode() const;
    [[nodiscard]] bool isError() const;
    [[nodiscard]] uint32_t requestId() const;
private:
    register_response_t m_raw{};
};

class StatusRequest
{
public:
    StatusRequest() = default;
    explicit StatusRequest(const status_request_t &raw);
    ~StatusRequest() = default;

    status_request_t &raw();
    [[nodiscard]] const status_request_t &raw() const;
    [[nodiscard]] uint32_t requestId() const;
private:
    status_request_t m_raw{};
};

class StatusResponse
{
public:
    StatusResponse() = default;
    explicit StatusResponse(const status_response_t &raw);
    ~StatusResponse() = default;

    status_response_t &raw();
    [[nodiscard]] const status_response_t &raw() const;
    [[nodiscard]] ComponentStatus statusCode() const;
    [[nodiscard]] time_t lastActiveTime() const;
private:
    status_response_t m_raw{};
};

class ErrorResponse
{
public:
    ErrorResponse() = default;
    explicit ErrorResponse(const err_response_t &raw);
    ~ErrorResponse() = default;

    err_response_t &raw();
    [[nodiscard]] const err_response_t &raw() const;
    [[nodiscard]] ErrorCode errorCode() const;
    [[nodiscard]] QString errorMessage() const;
private:
    err_response_t m_raw{};
};

class Protocol
{
public:
    static ssize_t packMessage(MessageType msgType,
                        const void *payload,
                        std::size_t payloadLen,
                        uint8_t *outBuffer,
                        std::size_t bufferLen);
    static ErrorCode unpackMessage(const uint8_t *buffer,
                      std::size_t bufferLen,
                      MessageHeader& header,
                      void *payload,
                      std::size_t payloadBufferLen);
private:
    static uint8_t calculateChecksum(const void *data, std::size_t len);
};
}




#endif //UKUI_QUICK_PROTOCOL_H
