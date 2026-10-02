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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <QtTest>

#include <cstring>

#include "../ukui/heartbeat/protocol.h"

using namespace UkuiQuick::HeartBeat;

namespace {

uint8_t xorChecksum(const uint8_t *buffer, std::size_t len)
{
    uint8_t checksum = 0;
    for (std::size_t index = 0; index < len; ++index) {
        checksum ^= buffer[index];
    }
    return checksum;
}

}

class HeartbeatProtocolTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testRegisterRequestProcessNameIsCapped();
    void testRegisterResponseAccessors();
    void testStatusAndErrorAccessors();
    void testPackAndUnpackRoundTrip();
    void testPackRejectsInvalidArguments();
    void testUnpackRejectsMalformedMessages();
};

void HeartbeatProtocolTest::testRegisterRequestProcessNameIsCapped()
{
    RegisterRequest request;
    request.raw().uid = 123;
    request.raw().pid = 456;
    request.raw().is_critical = 1;
    request.setProcessName(QStringLiteral("abcdefghijklmnopqrstuvwxyz0123456789"));

    QVERIFY(request.isCritical());
    QCOMPARE(request.uid(), static_cast<uid_t>(123));
    QCOMPARE(request.pid(), static_cast<pid_t>(456));
    QCOMPARE(request.raw().name_len, static_cast<uint8_t>(kMaxProcessNameLength));
    QCOMPARE(request.processName(), QStringLiteral("abcdefghijklmnopqrstuvwxyz012345"));

    request.raw().name_len = static_cast<uint8_t>(kMaxProcessNameLength + 10);
    request.raw().process_name[5] = '\0';
    QCOMPARE(request.processName(), QStringLiteral("abcde"));
}

void HeartbeatProtocolTest::testRegisterResponseAccessors()
{
    RegisterResponse success;
    success.raw().result_cod = static_cast<uint8_t>(ErrorCode::None);
    success.raw().request_id = 42;

    QVERIFY(!success.isError());
    QCOMPARE(success.errorCode(), ErrorCode::None);
    QCOMPARE(success.requestId(), static_cast<uint32_t>(42));

    RegisterResponse failure;
    failure.raw().result_cod = static_cast<uint8_t>(ErrorCode::RegisterFailed);

    QVERIFY(failure.isError());
    QCOMPARE(failure.errorCode(), ErrorCode::RegisterFailed);
}

void HeartbeatProtocolTest::testStatusAndErrorAccessors()
{
    StatusRequest statusRequest;
    statusRequest.raw().request_id = 1001;
    QCOMPARE(statusRequest.requestId(), static_cast<uint32_t>(1001));

    StatusResponse statusResponse;
    statusResponse.raw().status_cod = static_cast<uint8_t>(ComponentStatus::Busy);
    statusResponse.raw().last_active_time = 123456;
    QCOMPARE(statusResponse.statusCode(), ComponentStatus::Busy);
    QCOMPARE(statusResponse.lastActiveTime(), static_cast<time_t>(123456));

    ErrorResponse errorResponse;
    errorResponse.raw().err_cod = static_cast<uint8_t>(ErrorCode::InvalidRequestId);
    std::memcpy(errorResponse.raw().err_msg, "bad-id", 6);
    QCOMPARE(errorResponse.errorCode(), ErrorCode::InvalidRequestId);
    QCOMPARE(errorResponse.errorMessage(), QStringLiteral("bad-id"));
}

void HeartbeatProtocolTest::testPackAndUnpackRoundTrip()
{
    RegisterRequest request;
    request.raw().uid = 1000;
    request.raw().pid = 2000;
    request.setProcessName(QStringLiteral("heartbeat-test"));

    uint8_t buffer[kMaxMessageLength]{0};
    const ssize_t packedSize = Protocol::packMessage(
        MessageType::RegisterRequest,
        &request.raw(),
        sizeof(request.raw()),
        buffer,
        sizeof(buffer));

    QCOMPARE(packedSize, static_cast<ssize_t>(sizeof(message_header_t) + sizeof(register_request_t)));
    QCOMPARE(xorChecksum(buffer, static_cast<std::size_t>(packedSize)), static_cast<uint8_t>(0));

    MessageHeader header;
    RegisterRequest unpacked;
    QCOMPARE(
        Protocol::unpackMessage(
            buffer,
            static_cast<std::size_t>(packedSize),
            header,
            &unpacked.raw(),
            sizeof(unpacked.raw())),
        ErrorCode::None);
    QCOMPARE(header.type(), MessageType::RegisterRequest);
    QCOMPARE(header.len(), sizeof(register_request_t));
    QCOMPARE(unpacked.uid(), request.uid());
    QCOMPARE(unpacked.pid(), request.pid());
    QCOMPARE(unpacked.processName(), QStringLiteral("heartbeat-test"));
}

void HeartbeatProtocolTest::testPackRejectsInvalidArguments()
{
    uint8_t buffer[kMaxMessageLength]{0};
    RegisterResponse response;

    QCOMPARE(
        Protocol::packMessage(
            MessageType::RegisterResponse,
            &response.raw(),
            sizeof(response.raw()),
            nullptr,
            sizeof(buffer)),
        static_cast<ssize_t>(-1));
    QCOMPARE(
        Protocol::packMessage(
            MessageType::RegisterResponse,
            &response.raw(),
            sizeof(response.raw()),
            buffer,
            sizeof(message_header_t)),
        static_cast<ssize_t>(-1));
    QCOMPARE(
        Protocol::packMessage(
            MessageType::RegisterResponse,
            &response.raw(),
            0,
            buffer,
            sizeof(buffer)),
        static_cast<ssize_t>(-1));
}

void HeartbeatProtocolTest::testUnpackRejectsMalformedMessages()
{
    MessageHeader header;
    RegisterResponse response;

    QCOMPARE(
        Protocol::unpackMessage(
            nullptr,
            0,
            header,
            &response.raw(),
            sizeof(response.raw())),
        ErrorCode::InvalidMessage);

    uint8_t shortBuffer[sizeof(message_header_t) - 1]{0};
    QCOMPARE(
        Protocol::unpackMessage(
            shortBuffer,
            sizeof(shortBuffer),
            header,
            &response.raw(),
            sizeof(response.raw())),
        ErrorCode::InvalidMessage);

    RegisterResponse validResponse;
    validResponse.raw().result_cod = static_cast<uint8_t>(ErrorCode::None);
    validResponse.raw().request_id = 7;

    uint8_t buffer[kMaxMessageLength]{0};
    const ssize_t packedSize = Protocol::packMessage(
        MessageType::RegisterResponse,
        &validResponse.raw(),
        sizeof(validResponse.raw()),
        buffer,
        sizeof(buffer));
    QVERIFY(packedSize > 0);

    auto *rawHeader = reinterpret_cast<message_header_t *>(buffer);
    rawHeader->msg_len = static_cast<uint16_t>(sizeof(register_response_t) + 1);
    QCOMPARE(
        Protocol::unpackMessage(
            buffer,
            static_cast<std::size_t>(packedSize),
            header,
            &response.raw(),
            sizeof(response.raw())),
        ErrorCode::InvalidMessage);

    const ssize_t repackedSize = Protocol::packMessage(
        MessageType::RegisterResponse,
        &validResponse.raw(),
        sizeof(validResponse.raw()),
        buffer,
        sizeof(buffer));
    QVERIFY(repackedSize > 0);
    buffer[sizeof(message_header_t)] ^= 0x01;
    QCOMPARE(
        Protocol::unpackMessage(
            buffer,
            static_cast<std::size_t>(repackedSize),
            header,
            &response.raw(),
            sizeof(response.raw())),
        ErrorCode::Checksum);

    const ssize_t finalPackedSize = Protocol::packMessage(
        MessageType::RegisterResponse,
        &validResponse.raw(),
        sizeof(validResponse.raw()),
        buffer,
        sizeof(buffer));
    QVERIFY(finalPackedSize > 0);
    uint8_t tooSmallPayload[sizeof(register_response_t) - 1]{0};
    QCOMPARE(
        Protocol::unpackMessage(
            buffer,
            static_cast<std::size_t>(finalPackedSize),
            header,
            tooSmallPayload,
            sizeof(tooSmallPayload)),
        ErrorCode::InvalidMessage);
}

QTEST_MAIN(HeartbeatProtocolTest)

#include "test-heartbeat-protocol.moc"
