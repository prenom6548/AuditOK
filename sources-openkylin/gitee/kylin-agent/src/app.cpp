#include "app.h"
#include <QDebug>
#include <QCoreApplication>
#include <QAbstractButton>
#include <QDir>
#include <QEvent>
#include <QKeyEvent>
#include <QKeySequence>
#include <QLineEdit>
#include <QLocalServer>
#include <QLocalSocket>
#include <QPlainTextEdit>
#include <QSettings>
#include <QTextEdit>
#include <QTimer>
#include <kwindowbuttonbar.h>

namespace {

template<typename Editor>
bool handleStandardTextShortcut(Editor *editor, QKeyEvent *event)
{
    if (!editor || !event) {
        return false;
    }

    if (event->matches(QKeySequence::Undo)) {
        editor->undo();
    } else if (event->matches(QKeySequence::Redo)) {
        editor->redo();
    } else if (event->matches(QKeySequence::Cut)) {
        editor->cut();
    } else if (event->matches(QKeySequence::Copy)) {
        editor->copy();
    } else if (event->matches(QKeySequence::Paste)) {
        editor->paste();
    } else if (event->matches(QKeySequence::SelectAll)) {
        editor->selectAll();
    } else {
        return false;
    }

    event->accept();
    return true;
}

bool handleStandardTextShortcut(QObject *watched, QKeyEvent *event)
{
    QObject *target = QApplication::focusWidget();
    if (!target) {
        target = watched;
    }

    // QTextEdit and QPlainTextEdit can deliver key events through their
    // viewport. Walk up to the actual editor before trying each supported
    // text-control type.
    while (target) {
        if (handleStandardTextShortcut(qobject_cast<QLineEdit *>(target), event) ||
            handleStandardTextShortcut(qobject_cast<QTextEdit *>(target), event) ||
            handleStandardTextShortcut(qobject_cast<QPlainTextEdit *>(target), event)) {
            return true;
        }
        target = target->parent();
    }
    return false;
}

void migrateLegacySettings()
{
    QSettings legacy(QStringLiteral("HermesQt"), QStringLiteral("HermesQt"));
    QSettings current(QStringLiteral("KylinAgent"), QStringLiteral("KylinAgent"));
    for (const QString &key : legacy.allKeys()) {
        if (!current.contains(key)) {
            current.setValue(key, legacy.value(key));
        }
    }
    current.sync();
}

QString startupMessageFromArguments(const QStringList &arguments)
{
    if (arguments.contains(QStringLiteral("--quit"))) {
        return QStringLiteral("quit");
    }
    if (arguments.contains(QStringLiteral("--hide"))) {
        return QStringLiteral("hide");
    }
    if (arguments.contains(QStringLiteral("--show"))) {
        return QStringLiteral("activate");
    }
    if (arguments.contains(QStringLiteral("--show-cron"))) {
        return QStringLiteral("show-cron");
    }
    if (arguments.contains(QStringLiteral("--toggle"))) {
        return QStringLiteral("toggle");
    }
    return QStringLiteral("activate");
}

} // namespace

App::App(int &argc, char **argv)
    : QApplication(argc, argv)
    , m_localServer(nullptr)
    , m_socketName(QDir::home().filePath(QStringLiteral(".cache/kylin-agent.socket")))
    , m_primaryInstance(false)
{
    qDebug() << "KylinAgent starting...";
    installEventFilter(this);
    migrateLegacySettings();

    QDir().mkpath(QDir::home().filePath(QStringLiteral(".cache")));
    if (createLocalServer()) {
        m_primaryInstance = true;
        qDebug() << "KylinAgent primary instance created";
    } else if (tryConnectToPrimary()) {
        qDebug() << "KylinAgent is already running, requesting activation";
        sendActivationMessage(startupMessageFromArguments(arguments()));
    } else {
        qDebug() << "Removing stale KylinAgent socket and retrying";
        cleanupLocalServer();
        if (createLocalServer()) {
            m_primaryInstance = true;
            qDebug() << "KylinAgent primary instance created after cleanup";
        } else {
            qCritical() << "Failed to create KylinAgent local server";
            QTimer::singleShot(0, this, &QCoreApplication::quit);
        }
    }
}

App::~App()
{
    if (m_localServer) {
        m_localServer->close();
    }
    if (m_primaryInstance) {
        cleanupLocalServer();
    }
    qDebug() << "KylinAgent shutting down...";
}

bool App::isPrimaryInstance() const
{
    return m_primaryInstance;
}

bool App::sendActivationMessage(const QString &message)
{
    QLocalSocket socket;
    socket.connectToServer(m_socketName);
    if (!socket.waitForConnected(500)) {
        return false;
    }

    const QByteArray payload = message.trimmed().isEmpty()
                                   ? QByteArrayLiteral("activate")
                                   : message.trimmed().toUtf8();
    socket.write(payload);
    socket.flush();
    socket.waitForBytesWritten(500);
    socket.disconnectFromServer();
    if (socket.state() != QLocalSocket::UnconnectedState) {
        socket.waitForDisconnected(500);
    }
    return true;
}

bool App::event(QEvent *event)
{
    return QApplication::event(event);
}

bool App::eventFilter(QObject *watched, QEvent *event)
{
    if (event && event->type() == QEvent::KeyPress &&
        handleStandardTextShortcut(watched, static_cast<QKeyEvent *>(event))) {
        return true;
    }

    if (event && (event->type() == QEvent::Polish || event->type() == QEvent::Show)) {
        if (QAbstractButton *button = qobject_cast<QAbstractButton *>(watched)) {
            if (button->toolTip().trimmed().isEmpty()) {
                QString toolTip = button->text();
                toolTip.remove(QLatin1Char('&'));
                toolTip = toolTip.trimmed();

                if (kdk::KWindowButtonBar *bar =
                        qobject_cast<kdk::KWindowButtonBar *>(button->parentWidget())) {
                    if (button == bar->minimumButton()) {
                        toolTip = tr("最小化");
                    } else if (button == bar->maximumButton()) {
                        toolTip = tr("最大化或还原");
                    } else if (button == bar->closeButton()) {
                        toolTip = tr("关闭");
                    } else if (button == bar->menuButton()) {
                        toolTip = tr("菜单");
                    }
                }

                if (!toolTip.isEmpty()) {
                    button->setToolTip(toolTip);
                }
            }
        }
    }
    return QApplication::eventFilter(watched, event);
}

void App::handleQuit()
{
    qDebug() << "Quit requested";
    quit();
}

bool App::createLocalServer()
{
    if (m_localServer) {
        m_localServer->deleteLater();
        m_localServer = nullptr;
    }

    m_localServer = new QLocalServer(this);
    connect(m_localServer, &QLocalServer::newConnection, this, &App::handleNewConnection);
    return m_localServer->listen(m_socketName);
}

bool App::tryConnectToPrimary()
{
    QLocalSocket socket;
    socket.connectToServer(m_socketName);
    const bool connected = socket.waitForConnected(500);
    socket.close();
    return connected;
}

void App::cleanupLocalServer()
{
    QLocalServer::removeServer(m_socketName);
}

void App::handleNewConnection()
{
    while (m_localServer && m_localServer->hasPendingConnections()) {
        QLocalSocket *socket = m_localServer->nextPendingConnection();
        if (!socket) {
            continue;
        }

        const auto processMessage = [this, socket]() {
            const QString message = QString::fromUtf8(socket->readAll()).trimmed();
            if (!message.isEmpty()) {
                qDebug() << "KylinAgent local command received:" << message;
                emit messageReceived(message);
            }
        };

        connect(socket, &QLocalSocket::readyRead, this, processMessage);
        connect(socket, &QLocalSocket::disconnected, this, [socket, processMessage]() {
            processMessage();
            socket->deleteLater();
        });
        QTimer::singleShot(0, socket, processMessage);
    }
}
