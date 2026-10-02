// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <QGuiApplication>
#include <QJsonDocument>
#include <QLibraryInfo>
#include <QSocketNotifier>
#include <QTimer>

#include <fcntl.h>
#include <unistd.h>

#include "MainWindow.h"

#if KYSDK_IS_SUPPORT
#include <ukuiwindowhelper.h>
UkuiWindowHelper *g_windowHelper = nullptr;
#endif

enum class ButtonResult {
    ALLOW = 0,
    DENY,
    CLOSED,
};

QTimer *g_timeout = nullptr;
static QSocketNotifier *g_notifier = nullptr;
static bool g_windowIsActive = false;
MainWindow *g_mainWindow = nullptr;
static QByteArray g_recvBuffer;

void cleanup()
{
    if (g_mainWindow) {
        QObject::disconnect(g_mainWindow, nullptr, nullptr, nullptr);
        g_mainWindow->deleteLater();
        g_mainWindow = nullptr;
        qDebug() << "mainWindow cleaned up";
    }

    if (g_notifier) {
        g_notifier->setEnabled(false);
        int fd = g_notifier->socket();
        if (fd >= 0) {
            ::close(fd);
            qDebug() << QString("Socket fd %1 closed manually.").arg(QString::number(fd));
        }
        delete g_notifier;
        g_notifier = nullptr;
    }

    QTimer::singleShot(10, qApp, &QCoreApplication::quit);
    qDebug() << "wldialog cleaned up";
}

bool writeToSocket(const QJsonObject &response)
{
    QByteArray data = QJsonDocument(response).toJson(QJsonDocument::Compact);
    qDebug() << "Sending response: " + QString::fromUtf8(data);
    int fd = g_notifier->socket();
    ssize_t totalWritten = 0;
    while (totalWritten < data.size()) {
        ssize_t len = ::write(fd, data.constData() + totalWritten, data.size() - totalWritten);
        if (len < 0) {
            if (errno == EINTR) {
                continue;
            }
            qWarning() << "Failed to write to socket_fd";
            return false;
        }
        totalWritten += len;
    }
    if (::write(fd, "\n", 1) < 0) {
        qWarning() << "Failed to write newline to socket_fd";
        return false;
    }
    return true;
}

void sendDialogResponse(int result)
{
    QJsonObject response;
    response["user_action"] = result;
    if (result != static_cast<int>(ButtonResult::CLOSED)) {
        g_mainWindow->results(response);
    }

    writeToSocket(response);
    QTimer::singleShot(50, []() { cleanup(); });
}

void handleDialog(const QJsonObject &req)
{
    qDebug() << "Handling dialog request";
    QJsonObject dialogObj;
    if (req.contains("dialog") && req["dialog"].isObject()) {
        dialogObj = req["dialog"].toObject();
    } else {
        qWarning() << "'dialog' field is not a JSON object";
        return;
    }

    if (g_windowIsActive) {
        qWarning() << "Protocol error: dialog already exists, rejecting duplicate request";
        QJsonObject errorResp;
        errorResp["error"] = "dialog already exists";
        writeToSocket(errorResp);
        return;
    }

    if (g_mainWindow) {
        g_mainWindow->deleteLater();
        g_mainWindow = nullptr;
    }

    g_mainWindow = new MainWindow();

    if (dialogObj.contains("window_title")) {
        g_mainWindow->setTitle(dialogObj["window_title"].toString("Permission Request"));
    }

    if (dialogObj.contains("window_icon")) {
        QIcon icon = QIcon::fromTheme(dialogObj["window_icon"].toString());
        if (!icon.isNull()) {
            g_mainWindow->setIcon(icon);
        }
    }

#if KYSDK_IS_SUPPORT
    if (g_windowHelper) {
        delete g_windowHelper;
        g_windowHelper = nullptr;
    }

    if (dialogObj.contains("title_bar")) {
        UkuiWindowHelper::States state = UkuiWindowHelper::State::Activatable |
                                         UkuiWindowHelper::State::Movable |
                                         UkuiWindowHelper::State::Closeable;
        g_windowHelper = new UkuiWindowHelper(g_mainWindow);
        if (dialogObj["title_bar"].toString() == "no_minimizable") {
            g_windowHelper->setWindowState(UkuiWindowHelper::State::Maximizable |
                                           UkuiWindowHelper::State::Fullscreenable | state);
        } else if (dialogObj["title_bar"].toString() == "no_maximizable") {
            g_windowHelper->setWindowState(UkuiWindowHelper::State::Minimizable | state);
        } else {
            g_windowHelper->setWindowState(state);
        }

        QObject::connect(g_mainWindow, &MainWindow::destroyed, []() {
            if (g_windowHelper) {
                delete g_windowHelper;
                g_windowHelper = nullptr;
            }
        });
    }
#endif

    g_mainWindow->initFromJson(dialogObj);

    QObject::connect(g_mainWindow, &MainWindow::rejectedByAllow,
                     []() { sendDialogResponse(static_cast<int>(ButtonResult::ALLOW)); });
    QObject::connect(g_mainWindow, &MainWindow::rejectedByDeny,
                     []() { sendDialogResponse(static_cast<int>(ButtonResult::DENY)); });
    QObject::connect(g_mainWindow, &MainWindow::rejectedByClose,
                     []() { sendDialogResponse(static_cast<int>(ButtonResult::CLOSED)); });

    g_windowIsActive = true;
}

void handleControl(const QJsonObject &req)
{
    Q_UNUSED(req);
    cleanup();
}

void handleUpdate(const QJsonObject &req)
{
    qDebug() << "Handling update request";
    if (!g_mainWindow) {
        return;
    }

    QJsonObject updateReq;
    if (req.contains("update") && req["update"].isObject()) {
        updateReq = req["update"].toObject();
    } else {
        qWarning() << "'update' field is not a JSON object";
        return;
    }

    g_mainWindow->updateFromJson(updateReq);
}

void handleRecvData()
{
    g_notifier->setEnabled(false);
    int fd = g_notifier->socket();

    bool fdIsErr = false;
    QByteArray buffer(4096, Qt::Uninitialized);
    while (true) {
        ssize_t count = ::read(fd, buffer.data(), buffer.size());

        if (count > 0) {
            g_recvBuffer.append(buffer.data(), count);
        } else if (count == -1 && errno == EWOULDBLOCK) {
            break;
        } else {
            qDebug() << "fd closed / abnormal error";
            fdIsErr = true;
            g_recvBuffer.clear();
            break;
        }
    }
    g_notifier->setEnabled(true);

    if (fdIsErr) {
        cleanup();
        return;
    }

    while (true) {
        int newlinePos = g_recvBuffer.indexOf('\n');
        if (newlinePos < 0) {
            qDebug() << "No more data available at the moment. Waiting for the next notification.";
            break;
        }

        QByteArray line = g_recvBuffer.left(newlinePos).trimmed();
        g_recvBuffer.remove(0, newlinePos + 1);

        if (line.isEmpty()) {
            continue;
        }

        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(line, &err);
        if (!doc.isObject()) {
            qWarning() << QString("Invalid JSON: %1").arg(err.errorString());
            continue;
        }

        QJsonObject req = doc.object();
        if (req.contains("dialog") && req.contains("control")) {
            qWarning() << "Unsupported request: 'dialog' and 'control' are mutually exclusive";
            g_recvBuffer.clear();
            return;
        }
        if (req.contains("dialog")) {
            g_timeout->stop();
            handleDialog(req);
        } else if (req.contains("control")) {
            handleControl(req);
        } else if (req.contains("update")) {
            handleUpdate(req);
        } else {
            qWarning() << "Protocol Error: Missing 'dialog' or 'control' key";
        }
        qDebug() << "Parsed request:" << req;
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        qWarning() << "No socket fd provided in argv!";
        return EXIT_FAILURE;
    }

    qputenv("QT_QPA_PLATFORM", "wayland");
    qputenv("QT_QPA_PLATFORMTHEME", "ukui");

    int socket_fd = atoi(argv[argc - 1]);
    if (socket_fd <= 0) {
        qWarning() << "Invalid socket fd: " + QString::number(socket_fd);
        return EXIT_FAILURE;
    }
    qDebug() << "Received communication fd: " << QString::number(socket_fd);

    int flags = fcntl(socket_fd, F_GETFL);
    if (flags != -1) {
        fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);
    }

    QGuiApplication app(argc, argv);

    g_notifier = new QSocketNotifier(socket_fd, QSocketNotifier::Read);
    QObject::connect(g_notifier, &QSocketNotifier::activated, []() { handleRecvData(); });

    g_timeout = new QTimer(&app);
    g_timeout->setSingleShot(true);
    g_timeout->start(20000);

    QObject::connect(g_timeout, &QTimer::timeout, [&]() {
        qWarning() << "Socket read timed out after 20 seconds.";
        cleanup();
    });

    return app.exec();
}
