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

#include <QCoreApplication>
#include <QObject>
#include <QSignalSpy>
#include <QSocketNotifier>
#include <QTest>

#include <cerrno>
#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define private public
#include "../ukui/heartbeat/keep-alive-client.h"
#undef private
#include "../ukui/heartbeat/protocol.h"
#include "../ukui/heartbeat/keep-alive-client.cpp"

using namespace UkuiQuick::HeartBeat;

namespace {

constexpr uint32_t kRequestId = 1001;

socklen_t makeAddress(const QString &path, sockaddr_un &address)
{
    address.sun_family = AF_UNIX;

    QString cleaned = path;
    bool isAbstract = false;
    if (cleaned.startsWith(QLatin1Char('@'))) {
        cleaned = cleaned.mid(1);
        isAbstract = true;
    }

    const QByteArray encoded = cleaned.toUtf8();
    if (isAbstract) {
        address.sun_path[0] = '\0';
        std::strncpy(address.sun_path + 1, encoded.constData(), sizeof(address.sun_path) - 2);
        return static_cast<socklen_t>(offsetof(sockaddr_un, sun_path) + 1 + encoded.size());
    }

    ::unlink(encoded.constData());
    std::strncpy(address.sun_path, encoded.constData(), sizeof(address.sun_path) - 1);
    return static_cast<socklen_t>(offsetof(sockaddr_un, sun_path) + encoded.size() + 1);
}

class FakeHeartbeatServer : public QObject
{
    Q_OBJECT

public:
    explicit FakeHeartbeatServer(const QString &serverPath, QObject *parent = nullptr)
        : QObject(parent)
        , m_serverPath(serverPath)
    {
        m_socketFd = ::socket(AF_UNIX, SOCK_DGRAM, 0);
        if (m_socketFd < 0) {
            m_lastError = errno;
            return;
        }

        sockaddr_un address{};
        const auto addressLength = makeAddress(serverPath, address);
        if (::bind(m_socketFd, reinterpret_cast<const sockaddr *>(&address), addressLength) < 0) {
            m_lastError = errno;
            ::close(m_socketFd);
            m_socketFd = -1;
            return;
        }

        m_notifier = new QSocketNotifier(m_socketFd, QSocketNotifier::Read, this);
        connect(m_notifier, &QSocketNotifier::activated, this, &FakeHeartbeatServer::handleReadyRead);
    }

    ~FakeHeartbeatServer() override
    {
        if (m_socketFd >= 0) {
            ::close(m_socketFd);
        }
        if (m_serverPath.startsWith(QLatin1Char('/'))) {
            ::unlink(m_serverPath.toUtf8().constData());
        }
    }

    [[nodiscard]] bool isReady() const
    {
        return m_socketFd >= 0;
    }

    [[nodiscard]] int lastError() const
    {
        return m_lastError;
    }

    [[nodiscard]] QString lastErrorString() const
    {
        return QString::fromLocal8Bit(std::strerror(m_lastError));
    }

    [[nodiscard]] int registerRequestCount() const
    {
        return m_registerRequestCount;
    }

    [[nodiscard]] int statusResponseCount() const
    {
        return m_statusResponseCount;
    }

    [[nodiscard]] QString registeredProcessName() const
    {
        return m_registeredProcessName;
    }

    [[nodiscard]] uid_t registeredUid() const
    {
        return m_registeredUid;
    }

    [[nodiscard]] pid_t registeredPid() const
    {
        return m_registeredPid;
    }

    [[nodiscard]] ComponentStatus lastComponentStatus() const
    {
        return m_lastComponentStatus;
    }

    [[nodiscard]] time_t lastActiveTime() const
    {
        return m_lastActiveTime;
    }

    bool sendRegisterResponse(uint32_t requestId = kRequestId, ErrorCode errorCode = ErrorCode::None)
    {
        if (!m_hasClientAddress) {
            return false;
        }

        RegisterResponse response;
        response.raw().result_cod = static_cast<uint8_t>(errorCode);
        response.raw().request_id = requestId;
        return sendMessage(MessageType::RegisterResponse, &response.raw(), sizeof(response.raw()));
    }

    bool sendStatusQuery(uint32_t requestId = kRequestId)
    {
        if (!m_hasClientAddress) {
            return false;
        }

        StatusRequest request;
        request.raw().request_id = requestId;
        return sendMessage(MessageType::StatusQuery, &request.raw(), sizeof(request.raw()));
    }

Q_SIGNALS:
    void registerRequestReceived();
    void statusResponseReceived();

private:
    bool sendMessage(MessageType type, const void *payload, std::size_t payloadSize)
    {
        uint8_t buffer[kMaxMessageLength]{0};
        const ssize_t packedSize = Protocol::packMessage(type, payload, payloadSize, buffer, sizeof(buffer));
        if (packedSize <= 0) {
            return false;
        }

        return ::sendto(
                   m_socketFd,
                   buffer,
                   static_cast<size_t>(packedSize),
                   0,
                   reinterpret_cast<const sockaddr *>(&m_clientAddress),
                   m_clientAddressLength) >= 0;
    }

    void handleReadyRead()
    {
        sockaddr_un from{};
        socklen_t fromLength = sizeof(from);
        uint8_t buffer[kMaxMessageLength]{0};
        const ssize_t receivedSize = ::recvfrom(
            m_socketFd,
            buffer,
            sizeof(buffer),
            0,
            reinterpret_cast<sockaddr *>(&from),
            &fromLength);
        if (receivedSize <= 0) {
            return;
        }

        MessageHeader header;
        const auto result = Protocol::unpackMessage(
            buffer,
            static_cast<size_t>(receivedSize),
            header,
            nullptr,
            0);
        if (result != ErrorCode::None) {
            return;
        }

        switch (header.type()) {
        case MessageType::RegisterRequest:
            handleRegisterRequest(buffer, static_cast<size_t>(receivedSize), header, from, fromLength);
            break;
        case MessageType::StatusResponse:
            handleStatusResponse(buffer, static_cast<size_t>(receivedSize), header);
            break;
        default:
            break;
        }
    }

    void handleRegisterRequest(
        const uint8_t *buffer,
        size_t size,
        MessageHeader &header,
        const sockaddr_un &from,
        socklen_t fromLength)
    {
        RegisterRequest request;
        if (Protocol::unpackMessage(buffer, size, header, &request.raw(), sizeof(request.raw())) != ErrorCode::None) {
            return;
        }

        m_clientAddress = from;
        m_clientAddressLength = fromLength;
        m_hasClientAddress = true;
        m_registeredProcessName = request.processName();
        m_registeredUid = request.uid();
        m_registeredPid = request.pid();
        ++m_registerRequestCount;
        Q_EMIT registerRequestReceived();
    }

    void handleStatusResponse(const uint8_t *buffer, size_t size, MessageHeader &header)
    {
        StatusResponse response;
        if (Protocol::unpackMessage(buffer, size, header, &response.raw(), sizeof(response.raw())) != ErrorCode::None) {
            return;
        }

        m_lastComponentStatus = response.statusCode();
        m_lastActiveTime = response.lastActiveTime();
        ++m_statusResponseCount;
        Q_EMIT statusResponseReceived();
    }

    int m_socketFd{-1};
    int m_lastError{0};
    QSocketNotifier *m_notifier{nullptr};
    QString m_serverPath;
    sockaddr_un m_clientAddress{};
    socklen_t m_clientAddressLength{0};
    bool m_hasClientAddress{false};
    int m_registerRequestCount{0};
    int m_statusResponseCount{0};
    QString m_registeredProcessName;
    uid_t m_registeredUid{0};
    pid_t m_registeredPid{0};
    ComponentStatus m_lastComponentStatus{ComponentStatus::Unresponsive};
    time_t m_lastActiveTime{0};
};

QString uniqueSocketPath(const QString &name)
{
    return QStringLiteral("@%1-%2").arg(name, QString::number(QCoreApplication::applicationPid()));
}

void useServerPath(KeepAliveClient &client, const QString &serverPath)
{
    client.d->m_serverAddress = client.d->makeUnixAddress(serverPath, false);
}

}

class KeepAliveClientTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testStartSendsRegisterRequestToConfiguredServerPath();
    void testRegistrationResponseMarksClientRegistered();
    void testRegisterErrorDoesNotMarkClientRegistered();
    void testStatusQuerySendsStatusResponseForMatchingRequestId();
    void testStatusQueryWithWrongRequestIdIsIgnored();
    void testStopResetsStartedAndRegisteredState();
};

void KeepAliveClientTest::testStartSendsRegisterRequestToConfiguredServerPath()
{
    const QString serverPath = uniqueSocketPath(QStringLiteral("ukui-heartbeat-register"));
    FakeHeartbeatServer server(serverPath);
    if (!server.isReady() && server.lastError() == EPERM) {
        QSKIP("AF_UNIX datagram socket bind is blocked in this environment");
    }
    QVERIFY2(server.isReady(), "fake heartbeat server failed to bind");

    KeepAliveClient client(QStringLiteral("heartbeat-autotest"));
    useServerPath(client, serverPath);
    QSignalSpy serverRegisterSpy(&server, &FakeHeartbeatServer::registerRequestReceived);

    QVERIFY(client.start());
    QVERIFY(client.isStarted());

    QTRY_COMPARE_WITH_TIMEOUT(serverRegisterSpy.count(), 1, 3000);
    QCOMPARE(server.registerRequestCount(), 1);
    QCOMPARE(server.registeredProcessName(), QStringLiteral("heartbeat-autotest"));
    QCOMPARE(server.registeredUid(), getuid());
    QCOMPARE(server.registeredPid(), getpid());
}

void KeepAliveClientTest::testRegistrationResponseMarksClientRegistered()
{
    const QString serverPath = uniqueSocketPath(QStringLiteral("ukui-heartbeat-success"));
    FakeHeartbeatServer server(serverPath);
    if (!server.isReady() && server.lastError() == EPERM) {
        QSKIP("AF_UNIX datagram socket bind is blocked in this environment");
    }
    QVERIFY2(server.isReady(), "fake heartbeat server failed to bind");

    KeepAliveClient client(QStringLiteral("heartbeat-register-success"));
    useServerPath(client, serverPath);
    QSignalSpy serverRegisterSpy(&server, &FakeHeartbeatServer::registerRequestReceived);
    QSignalSpy registeredSpy(&client, &KeepAliveClient::registrationSucceeded);

    QVERIFY(client.start());
    QTRY_COMPARE_WITH_TIMEOUT(serverRegisterSpy.count(), 1, 3000);
    QVERIFY(server.sendRegisterResponse());

    QTRY_COMPARE_WITH_TIMEOUT(registeredSpy.count(), 1, 3000);
    QVERIFY(client.isRegistered());
}

void KeepAliveClientTest::testRegisterErrorDoesNotMarkClientRegistered()
{
    const QString serverPath = uniqueSocketPath(QStringLiteral("ukui-heartbeat-register-error"));
    FakeHeartbeatServer server(serverPath);
    if (!server.isReady() && server.lastError() == EPERM) {
        QSKIP("AF_UNIX datagram socket bind is blocked in this environment");
    }
    QVERIFY2(server.isReady(), "fake heartbeat server failed to bind");

    KeepAliveClient client(QStringLiteral("heartbeat-register-error"));
    useServerPath(client, serverPath);
    QSignalSpy serverRegisterSpy(&server, &FakeHeartbeatServer::registerRequestReceived);
    QSignalSpy registeredSpy(&client, &KeepAliveClient::registrationSucceeded);

    QVERIFY(client.start());
    QTRY_COMPARE_WITH_TIMEOUT(serverRegisterSpy.count(), 1, 3000);
    QVERIFY(server.sendRegisterResponse(0, ErrorCode::RegisterFailed));

    QTest::qWait(100);
    QCOMPARE(registeredSpy.count(), 0);
    QVERIFY(!client.isRegistered());
}

void KeepAliveClientTest::testStatusQuerySendsStatusResponseForMatchingRequestId()
{
    const QString serverPath = uniqueSocketPath(QStringLiteral("ukui-heartbeat-status"));
    FakeHeartbeatServer server(serverPath);
    if (!server.isReady() && server.lastError() == EPERM) {
        QSKIP("AF_UNIX datagram socket bind is blocked in this environment");
    }
    QVERIFY2(server.isReady(), "fake heartbeat server failed to bind");

    KeepAliveClient client(QStringLiteral("heartbeat-status"));
    useServerPath(client, serverPath);
    QSignalSpy serverRegisterSpy(&server, &FakeHeartbeatServer::registerRequestReceived);
    QSignalSpy serverStatusSpy(&server, &FakeHeartbeatServer::statusResponseReceived);
    QSignalSpy clientStatusSpy(&client, &KeepAliveClient::statusResponseReceived);

    QVERIFY(client.start());
    QTRY_COMPARE_WITH_TIMEOUT(serverRegisterSpy.count(), 1, 3000);
    QVERIFY(server.sendRegisterResponse());
    QTRY_VERIFY_WITH_TIMEOUT(client.isRegistered(), 3000);

    QVERIFY(server.sendStatusQuery());

    QTRY_COMPARE_WITH_TIMEOUT(serverStatusSpy.count(), 1, 3000);
    QCOMPARE(server.statusResponseCount(), 1);
    QCOMPARE(server.lastComponentStatus(), ComponentStatus::Normal);
    QVERIFY(server.lastActiveTime() > 0);
    QCOMPARE(clientStatusSpy.count(), 1);
}

void KeepAliveClientTest::testStatusQueryWithWrongRequestIdIsIgnored()
{
    const QString serverPath = uniqueSocketPath(QStringLiteral("ukui-heartbeat-wrong-id"));
    FakeHeartbeatServer server(serverPath);
    if (!server.isReady() && server.lastError() == EPERM) {
        QSKIP("AF_UNIX datagram socket bind is blocked in this environment");
    }
    QVERIFY2(server.isReady(), "fake heartbeat server failed to bind");

    KeepAliveClient client(QStringLiteral("heartbeat-wrong-id"));
    useServerPath(client, serverPath);
    QSignalSpy serverRegisterSpy(&server, &FakeHeartbeatServer::registerRequestReceived);
    QSignalSpy serverStatusSpy(&server, &FakeHeartbeatServer::statusResponseReceived);
    QSignalSpy clientStatusSpy(&client, &KeepAliveClient::statusResponseReceived);

    QVERIFY(client.start());
    QTRY_COMPARE_WITH_TIMEOUT(serverRegisterSpy.count(), 1, 3000);
    QVERIFY(server.sendRegisterResponse());
    QTRY_VERIFY_WITH_TIMEOUT(client.isRegistered(), 3000);

    QVERIFY(server.sendStatusQuery(kRequestId + 1));

    QTest::qWait(100);
    QCOMPARE(serverStatusSpy.count(), 0);
    QCOMPARE(clientStatusSpy.count(), 0);
}

void KeepAliveClientTest::testStopResetsStartedAndRegisteredState()
{
    const QString serverPath = uniqueSocketPath(QStringLiteral("ukui-heartbeat-stop"));
    FakeHeartbeatServer server(serverPath);
    if (!server.isReady() && server.lastError() == EPERM) {
        QSKIP("AF_UNIX datagram socket bind is blocked in this environment");
    }
    QVERIFY2(server.isReady(), "fake heartbeat server failed to bind");

    KeepAliveClient client(QStringLiteral("heartbeat-stop"));
    useServerPath(client, serverPath);
    QSignalSpy serverRegisterSpy(&server, &FakeHeartbeatServer::registerRequestReceived);

    QVERIFY(client.start());
    QTRY_COMPARE_WITH_TIMEOUT(serverRegisterSpy.count(), 1, 3000);
    QVERIFY(server.sendRegisterResponse());
    QTRY_VERIFY_WITH_TIMEOUT(client.isRegistered(), 3000);

    client.stop();

    QVERIFY(!client.isStarted());
    QVERIFY(!client.isRegistered());
}

QTEST_MAIN(KeepAliveClientTest)

#include "test-heartbeat-client.moc"
