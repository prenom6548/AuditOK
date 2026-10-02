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

#include "protocol.h"
#include <QByteArray>
#include <algorithm>
#include <cstring>

using namespace UkuiQuick::HeartBeat;

uint8_t Protocol::calculateChecksum(const void* data, std::size_t len)
{
    const auto *bytes = static_cast<const uint8_t *>(data);
    uint8_t checksum = 0;
    for (std::size_t index = 0; index < len; ++index) {
        checksum ^= bytes[index];
    }
    return checksum;
}

MessageHeader::MessageHeader(const message_header_t& raw)
    : m_raw(raw)
{

}

MessageHeader& MessageHeader::operator=(const message_header_t& raw)
{
    m_raw = raw;
    return *this;
}

message_header_t& MessageHeader::raw()
{
    return m_raw;
}

const message_header_t& MessageHeader::raw() const
{
    return m_raw;
}

MessageType MessageHeader::type() const
{
    return static_cast<MessageType>(m_raw.msg_type);
}

std::size_t MessageHeader::len() const
{
    return m_raw.msg_len;
}

uint8_t MessageHeader::checksum() const
{
    return m_raw.checksum;
}

RegisterRequest::RegisterRequest(const register_request_t& raw)
    : m_raw(raw)
{

}

register_request_t& RegisterRequest::raw()
{
    return m_raw;
}

const register_request_t& RegisterRequest::raw() const
{
    return m_raw;
}

bool RegisterRequest::isCritical() const
{
    return m_raw.is_critical != 0;
}

uid_t RegisterRequest::uid() const
{
    return m_raw.uid;
}

pid_t RegisterRequest::pid() const
{
    return m_raw.pid;
}

QString RegisterRequest::processName() const
{
    const std::size_t cappedLen = std::min<std::size_t>(m_raw.name_len, kMaxProcessNameLength);
    const auto *begin = m_raw.process_name;
    const auto *end = std::find(begin, begin + cappedLen, '\0');
    return QString::fromUtf8(begin, static_cast<int>(end - begin));
}

void RegisterRequest::setProcessName(const QString& name)
{
    const QByteArray utf8Name = name.toUtf8();
    const std::size_t copyLen = std::min<std::size_t>(utf8Name.size(), kMaxProcessNameLength);
    std::memset(m_raw.process_name, 0, sizeof(m_raw.process_name));
    std::memcpy(m_raw.process_name, utf8Name.constData(), copyLen);
    m_raw.name_len = static_cast<uint8_t>(copyLen);
}

RegisterResponse::RegisterResponse(const register_response_t& raw)
    : m_raw(raw)
{

}

register_response_t& RegisterResponse::raw()
{
    return m_raw;
}

const register_response_t& RegisterResponse::raw() const
{
    return m_raw;
}

ErrorCode RegisterResponse::errorCode() const
{
    return static_cast<ErrorCode>(m_raw.result_cod);
}

bool RegisterResponse::isError() const
{
    return m_raw.result_cod != 0;
}

uint32_t RegisterResponse::requestId() const
{
    return m_raw.request_id;
}

StatusRequest::StatusRequest(const status_request_t& raw)
    : m_raw(raw)
{

}

status_request_t& StatusRequest::raw()
{
    return m_raw;
}

const status_request_t& StatusRequest::raw() const
{
    return m_raw;
}

uint32_t StatusRequest::requestId() const
{
    return m_raw.request_id;
}

StatusResponse::StatusResponse(const status_response_t& raw)
    : m_raw(raw)
{

}

status_response_t& StatusResponse::raw()
{
    return m_raw;
}

const status_response_t& StatusResponse::raw() const
{
    return m_raw;
}

ComponentStatus StatusResponse::statusCode() const
{
    return static_cast<ComponentStatus>(m_raw.status_cod);
}

time_t StatusResponse::lastActiveTime() const
{
    return m_raw.last_active_time;
}

ErrorResponse::ErrorResponse(const err_response_t& raw)
    : m_raw(raw)
{

}

err_response_t& ErrorResponse::raw()
{
    return m_raw;
}

const err_response_t& ErrorResponse::raw() const
{
    return m_raw;
}

ErrorCode ErrorResponse::errorCode() const
{
    return static_cast<ErrorCode>(m_raw.err_cod);
}

QString ErrorResponse::errorMessage() const
{
    constexpr std::size_t kErrMsgLen = sizeof(m_raw.err_msg);
    const auto *begin = m_raw.err_msg;
    const auto *end = std::find(begin, begin + kErrMsgLen, '\0');
    return QString::fromUtf8(begin, static_cast<int>(end - begin));
}

ssize_t Protocol::packMessage(MessageType msgType, const void* payload, std::size_t payloadLen, uint8_t* outBuffer,
                              std::size_t bufferLen)
{
    if (!outBuffer || (payload && payloadLen == 0) || bufferLen < sizeof(message_header_t) + payloadLen) {
        return -1;
    }

    auto *header = reinterpret_cast<message_header_t *>(outBuffer);
    header->msg_type = static_cast<uint8_t>(msgType);
    header->msg_len = static_cast<uint16_t>(payloadLen);

    uint8_t *payloadPtr = outBuffer + sizeof(message_header_t);
    if (payload && payloadLen > 0) {
        std::memcpy(payloadPtr, payload, payloadLen);
    }

    header->checksum = 0;
    header->checksum = calculateChecksum(outBuffer, sizeof(message_header_t) + payloadLen);
    return static_cast<ssize_t>(sizeof(message_header_t) + payloadLen);
}

ErrorCode Protocol::unpackMessage(const uint8_t* buffer, std::size_t bufferLen, MessageHeader& header, void* payload,
    std::size_t payloadBufferLen)
{
    if (!buffer || bufferLen < sizeof(message_header_t)) {
        return ErrorCode::InvalidMessage;
    }

    message_header_t rawHeader{};
    std::memcpy(&rawHeader, buffer, sizeof(message_header_t));

    const std::size_t payloadLen = rawHeader.msg_len;
    const std::size_t totalMessageLen = sizeof(message_header_t) + payloadLen;

    if (payloadLen > bufferLen - sizeof(message_header_t) || totalMessageLen != bufferLen) {
        return ErrorCode::InvalidMessage;
    }

    if (calculateChecksum(buffer, bufferLen) != 0) {
        return ErrorCode::Checksum;
    }

    if (payload && payloadLen > 0) {
        if (payloadBufferLen < payloadLen) {
            return ErrorCode::InvalidMessage;
        }
        std::memcpy(payload, buffer + sizeof(message_header_t), payloadLen);
    }

    header = rawHeader;
    return ErrorCode::None;
}
