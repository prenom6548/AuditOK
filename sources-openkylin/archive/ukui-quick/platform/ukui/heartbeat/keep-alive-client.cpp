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

#include "keep-alive-client.h"
#include <QSocketNotifier>
#include <QDebug>
#include <QFile>
#include <QProcessEnvironment>
#include <sys/un.h>
#include <sys/socket.h>
#include <cstring>
#include <utility>
#include <fcntl.h>
#include "protocol.h"
using namespace UkuiQuick::HeartBeat;

class KeepAliveClient::Private
{
public:
    struct UnixSocketAddress
    {
        sockaddr_un addr{};
        socklen_t len{0};
        QString origin;
        bool isAbstract{false};
    };

    Private(KeepAliveClient *q, QString appName);
    ~Private() = default;
    bool start();
    void stop();
    [[nodiscard]] bool isStarted() const;
    [[nodiscard]] bool isRegistered() const;
    ssize_t sendToServer(const void *buffer, size_t size) const;
    void sendRegisterRequest() const;
    void sendStatusResponse() const;
    void handleReceivedMessage();
    void handleRegisterResponse(const uint8_t *buffer, size_t size, MessageHeader &header);
    void handleStatusQuery(const uint8_t *buffer, size_t size, MessageHeader &header) const;
    UnixSocketAddress makeUnixAddress(const QString &path, bool abstractDefault);

    KeepAliveClient *q{nullptr};
    int m_socketFd{-1};
    uint32_t m_registryId{0};
    QString m_appName;
    UnixSocketAddress m_serverAddress;
    UnixSocketAddress m_clientAddress;
    QScopedPointer<QSocketNotifier> m_socketNotifier;
};

void KeepAliveClient::Private::sendStatusResponse() const
{
    StatusResponse response;
    response.raw().status_cod =  static_cast<uint8_t>(ComponentStatus::Normal);
    response.raw().last_active_time = std::time(nullptr);

    uint8_t buffer[kMaxMessageLength]{0};
    const auto packedSize = Protocol::packMessage(MessageType::StatusResponse,
                                                         &response.raw(),
                                                         sizeof(response.raw()),
                                                         buffer,
                                                         sizeof(buffer));
    if (packedSize <= 0) {
        return;
    }

    if (sendToServer(buffer, packedSize) < 0) {
        qWarning() << "send status response failed: " << strerror(errno);
        return;
    }

    Q_EMIT q->statusResponseReceived();
}

void KeepAliveClient::Private::handleReceivedMessage()
{
    if (m_socketFd < 0) {
        return;
    }

    sockaddr_un from{};
    socklen_t fromLength = sizeof(from);
    uint8_t buffer[kMaxMessageLength]{0};
    const ssize_t receivedSize = ::recvfrom(m_socketFd,
                                            buffer,
                                            sizeof(buffer),
                                            0,
                                            reinterpret_cast<sockaddr *>(&from),
                                            &fromLength);
    if (receivedSize <= 0) {
        return;
    }

    MessageHeader header;
    const auto result = Protocol::unpackMessage(buffer, static_cast<size_t>(receivedSize), header, nullptr, 0);
    if (result != ErrorCode::None) {
        qWarning() << "unpack message header failed: ";
        return;
    }

    switch (header.type()) {
    case MessageType::RegisterResponse:
        handleRegisterResponse(buffer, static_cast<size_t>(receivedSize), header);
        break;
    case MessageType::StatusQuery:
        handleStatusQuery(buffer, static_cast<size_t>(receivedSize), header);
        break;
    default:
        break;
    }
}

void KeepAliveClient::Private::handleRegisterResponse(const uint8_t* buffer, size_t size, MessageHeader &header)
{
    RegisterResponse response;
    const auto result = Protocol::unpackMessage(buffer, size, header, &response.raw(), sizeof(response.raw()));
    if (result != ErrorCode::None) {
        qWarning() << "unpack register response failed";
        return;
    }

    if (response.isError()) {
        qWarning() << "register response error: " << static_cast<uint8_t>(response.errorCode());
        return;
    }

    if (response.requestId() != 0) {
        m_registryId = response.requestId();
        Q_EMIT q->registrationSucceeded();
        qDebug() << "KeepAliveClient registration succeeded , id : " << m_registryId;
    }
}

void KeepAliveClient::Private::handleStatusQuery(const uint8_t* buffer, size_t size, MessageHeader &header) const
{
    StatusRequest request;
    const auto result = Protocol::unpackMessage(buffer, size, header, &request.raw(), sizeof(request.raw()));
    if (result != ErrorCode::None) {
        qWarning() << "unpack status query failed: ";
        return;
    }
    if (m_registryId == 0 || m_registryId != request.requestId()) {
        qWarning() << "invalid status query id: " << request.requestId() << " ; expected: " << m_registryId;
        return;
    }

    sendStatusResponse();
}

KeepAliveClient::Private::UnixSocketAddress KeepAliveClient::Private::makeUnixAddress(
    const QString& path, bool abstractDefault)
{
    UnixSocketAddress address;
    address.addr.sun_family = AF_UNIX;
    address.origin = path;

    QString cleaned = path;
    bool isAbstract = abstractDefault;
    if (path.startsWith(QLatin1Char('@'))) {
        cleaned = path.mid(1);
        isAbstract = true;
    } else if (path.startsWith(QLatin1Char('/'))) {
        isAbstract = false;
    }
    address.isAbstract = isAbstract;

    const QByteArray encoded = cleaned.toUtf8();
    if (isAbstract) {
        address.addr.sun_path[0] = '\0';
        std::strncpy(address.addr.sun_path + 1, encoded.constData(), sizeof(address.addr.sun_path) - 2);
        address.len = static_cast<socklen_t>(offsetof(sockaddr_un, sun_path) + 1 + std::strlen(encoded.constData()));
    } else {
        ::unlink(encoded.constData());
        std::strncpy(address.addr.sun_path, encoded.constData(), sizeof(address.addr.sun_path) - 1);
        address.len = static_cast<socklen_t>(offsetof(sockaddr_un, sun_path) + std::strlen(address.addr.sun_path) + 1);
    }

    return address;
}

KeepAliveClient::Private::Private(KeepAliveClient *q, QString appName)
    : q(q), m_appName(std::move(appName))
{

    auto defaultClientPath = [this]()
    {
        QString clientName;
        if (!m_appName.isEmpty()) {
            clientName = m_appName.trimmed();
        }

        if (clientName.isEmpty()) {
            QFile procFile("/proc/self/comm");
            if (procFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                clientName = QString::fromUtf8(procFile.readLine()).trimmed();
            }
        }

        return QStringLiteral("/tmp/@%1_%2.sock").arg(clientName, QString::number(getpid()));
    };

    const QString serverPath = QStringLiteral("@ukui-session-%1.sock").arg(getuid());
    m_serverAddress = makeUnixAddress(serverPath, false);
    m_clientAddress = makeUnixAddress(defaultClientPath(), false);
}

bool KeepAliveClient::Private::start()
{
    if (m_socketFd >= 0) {
        return true;
    }

    m_socketFd = ::socket(AF_UNIX, SOCK_DGRAM, 0);
    if (m_socketFd < 0) {
        qWarning() << "client socket create failed";
        return false;
    }

    auto setNonBlocking = [](const int fd) {
        const int flags = fcntl(fd, F_GETFL, 0);
        if (flags < 0) {
            return false;
        }
        return fcntl(fd, F_SETFL, flags | O_NONBLOCK) == 0;
    };

    if (!setNonBlocking(m_socketFd)) {
        qWarning() << "client socket set non blocking failed";
        stop();
        return false;
    }

    if (::bind(m_socketFd, reinterpret_cast<const sockaddr *>(&m_clientAddress.addr), m_clientAddress.len) < 0) {
        qWarning() << "client socket bind failed";
        stop();
        return false;
    }

    m_socketNotifier.reset(new QSocketNotifier(m_socketFd, QSocketNotifier::Read));
    connect(m_socketNotifier.data(), &QSocketNotifier::activated, q, [this]() {
        handleReceivedMessage();
    });

    sendRegisterRequest();
    return true;
}

void KeepAliveClient::Private::stop()
{
    m_registryId = 0;
    if (m_socketNotifier) {
        if (m_socketNotifier->isEnabled()) {
            m_socketNotifier->setEnabled(false);
        }
        m_socketNotifier.reset(nullptr);
    }

    if (m_socketFd >= 0) {
        ::close(m_socketFd);
        m_socketFd = -1;
    }
}

bool KeepAliveClient::Private::isStarted() const
{
    return m_socketFd >= 0;
}

bool KeepAliveClient::Private::isRegistered() const
{
    return m_registryId != 0;
}

ssize_t KeepAliveClient::Private::sendToServer(const void* buffer, size_t size) const
{
    if (m_socketFd < 0) {
        return -1;
    }
    return ::sendto(m_socketFd, buffer, size, 0,
        reinterpret_cast<const sockaddr *>(&m_serverAddress.addr), m_serverAddress.len);
}

void KeepAliveClient::Private::sendRegisterRequest() const
{
    if (m_socketFd < 0) {
        qWarning() << "send register skipped: socket is not started";
        return;
    }

    if (m_registryId != 0) {
        qDebug() << "register id already set .";
        return;
    }

    RegisterRequest request;
    request.raw().uid = getuid();
    request.raw().pid = getpid();
    request.setProcessName(m_appName);

    uint8_t buffer[kMaxMessageLength]{0};
    const ssize_t packedSize = Protocol::packMessage(MessageType::RegisterRequest,
                                                 &request.raw(),
                                                 sizeof(request.raw()),
                                                 buffer,
                                                 sizeof(buffer));
    if (packedSize < 0) {
        qWarning() << "pack register request message failed .";
        return;
    }

    if (sendToServer(buffer, packedSize) < 0) {
        qWarning() << "send register request failed: " << strerror(errno);
    }
}

KeepAliveClient::KeepAliveClient(const QString &appName, QObject* parent)
    : QObject(parent)
    , d(new Private(this, appName))
{
}

KeepAliveClient::~KeepAliveClient()
{
    d->stop();
}

bool KeepAliveClient::start()
{
    return d->start();
}

void KeepAliveClient::stop()
{
    d->stop();
}

bool KeepAliveClient::isStarted() const
{
    return d->isStarted();
}

bool KeepAliveClient::isRegistered() const
{
    return d->isRegistered();
}

void KeepAliveClient::sendStatusResponse() const
{
    d->sendStatusResponse();
}
