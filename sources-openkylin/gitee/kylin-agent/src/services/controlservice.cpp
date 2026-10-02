#include "services/controlservice.h"

#include <QDir>
#include <QAbstractSocket>
#include <QJsonDocument>
#include <QLocalServer>
#include <QLocalSocket>
#include <QScopedValueRollback>
#include <QPointer>
#include <QTimer>
#include <memory>
#include <utility>

ControlService::ControlService(Handler handler, QObject *parent)
    : QObject(parent), m_server(new QLocalServer(this)), m_handler(std::move(handler))
{
    m_server->setSocketOptions(QLocalServer::UserAccessOption);
    m_server->setMaxPendingConnections(16);
    connect(m_server, &QLocalServer::newConnection, this, &ControlService::acceptConnections);
}

QString ControlService::defaultSocketPath()
{
    return QDir::home().filePath(QStringLiteral(".cache/kylin-agent-control.socket"));
}

bool ControlService::start(const QString &path)
{
    const QString socketPath = path.isEmpty() ? defaultSocketPath() : path;
    if (m_server->isListening()) return true;
    if (m_server->listen(socketPath)) return true;
    if (m_server->serverError() != QAbstractSocket::AddressInUseError) return false;
    QLocalSocket probe;
    probe.connectToServer(socketPath);
    if (probe.waitForConnected(200)) return false;
    // Only remove an endpoint known to be stale, never a live server's socket.
    if (probe.error() != QLocalSocket::ConnectionRefusedError &&
        probe.error() != QLocalSocket::ServerNotFoundError) return false;
    QLocalServer::removeServer(socketPath);
    return m_server->listen(socketPath);
}

QString ControlService::errorString() const { return m_server->errorString(); }

QJsonObject ControlService::error(const QString &code, const QString &message)
{
    return {{"ok", false}, {"error", QJsonObject{{"code", code}, {"message", message}}}};
}

void ControlService::acceptConnections()
{
    while (m_server->hasPendingConnections()) {
        QLocalSocket *socket = m_server->nextPendingConnection();
        socket->setReadBufferSize(1024 * 1024 + 1);
        auto buffer = std::make_shared<QByteArray>();
        auto done = std::make_shared<bool>(false);
        auto reply = [socket = QPointer<QLocalSocket>(socket), done](const QJsonObject &response) {
            if (*done || !socket) return;
            *done = true;
            socket->write(QJsonDocument(response).toJson(QJsonDocument::Compact) + '\n');
            socket->disconnectFromServer();
        };
        auto read = [this, socket, buffer, done, reply]() {
            if (*done) return;
            buffer->append(socket->readAll());
            if (buffer->size() > 1024 * 1024) {
                reply(error("request_too_large", "Request exceeds 1 MiB"));
                return;
            }
            const int newline = buffer->indexOf('\n');
            if (newline < 0) return;
            QJsonParseError parseError;
            const QJsonDocument doc = QJsonDocument::fromJson(buffer->left(newline), &parseError);
            if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
                reply(error("invalid_json", "Expected a JSON object followed by newline"));
                return;
            }
            const QJsonObject request = doc.object();
            QJsonObject response;
            if (m_handling) {
                response = error("busy", "Another control command is in progress");
            } else {
                // Backend sync calls may run nested Qt event loops. Prevent reentry.
                *done = true;
                QScopedValueRollback<bool> handling(m_handling, true);
                response = m_handler(request);
                *done = false;
            }
            if (request.contains("id")) response.insert("id", request.value("id"));
            reply(response);
        };
        connect(socket, &QLocalSocket::readyRead, this, read);
        connect(socket, &QLocalSocket::disconnected, socket, &QObject::deleteLater);
        QTimer::singleShot(10000, socket, [done, reply]() {
            if (!*done) reply(error("timeout", "Request must end with newline within 10 seconds"));
        });
        QTimer::singleShot(0, socket, read);
    }
}
