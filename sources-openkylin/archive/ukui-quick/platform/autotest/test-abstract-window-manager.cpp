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
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 *  Authors: iaom <zhangpengfei@kylinos.cn>
 */

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusError>
#include <QIcon>
#include <QProcess>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QtTest>

#include "abstract-window-manager.h"

using namespace UkuiQuick;

namespace {

const auto HighlightService = "org.kde.KWin.HighlightWindow";
const auto HighlightPath = "/org/kde/KWin/HighlightWindow";

class HighlightReceiver : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.KWin.HighlightWindow")

public slots:
    void highlightWindows(const QStringList &wids)
    {
        Q_EMIT highlighted(wids);
    }

signals:
    void highlighted(const QStringList &wids);
};

class TestWindowManager : public AbstractWindowManager
{
public:
    QStringList windows() override { return {}; }
    QIcon windowIcon(const QString &) override { return {}; }
    QString windowTitle(const QString &) override { return {}; }
    bool skipTaskBar(const QString &) override { return false; }
    QString windowGroup(const QString &) override { return {}; }
    bool isMaximizable(const QString &) override { return false; }
    bool isMaximized(const QString &) override { return false; }
    void maximizeWindow(const QString &) override {}
    bool isMinimizable(const QString &) override { return false; }
    bool isMinimized(const QString &) override { return false; }
    void minimizeWindow(const QString &) override {}
    bool isKeepAbove(const QString &) override { return false; }
    void keepAboveWindow(const QString &) override {}
    bool isOnAllDesktops(const QString &) override { return false; }
    bool isOnCurrentDesktop(const QString &) override { return false; }
    void activateWindow(const QString &) override {}
    QString currentActiveWindow() override { return {}; }
    void closeWindow(const QString &) override {}
    void restoreWindow(const QString &) override {}
    bool isDemandsAttention(const QString &) override { return false; }
    quint32 pid(const QString &) override { return 0; }
    QString appId(const QString &) override { return {}; }
    QRect geometry(const QString &) override { return {}; }
    void setStartupGeometry(const QString &, QQuickItem *) override {}
    void setMinimizedGeometry(const QString &, QQuickItem *) override {}
    void unsetMinimizedGeometry(const QString &, QQuickItem *) override {}
    bool showingDesktop() override { return false; }
    void setShowingDesktop(bool) override {}
    bool isFullscreen(const QString &) override { return false; }
    void moveWindowToDesktop(const QString &, const QString &, const QString &) override {}
    QStringList windowDesktops(const QString &) override { return {}; }
    bool skipSwitcher(const QString &) override { return false; }
};

class RegisteredServiceGuard
{
public:
    RegisteredServiceGuard(QDBusConnection &bus, const QString &service, const QString &path)
        : m_bus(bus)
        , m_service(service)
        , m_path(path)
    {
    }

    ~RegisteredServiceGuard()
    {
        m_bus.unregisterObject(m_path);
        m_bus.unregisterService(m_service);
    }

private:
    QDBusConnection &m_bus;
    QString m_service;
    QString m_path;
};

class PrivateSessionBus
{
public:
    PrivateSessionBus()
        : m_previousAddress(qgetenv("DBUS_SESSION_BUS_ADDRESS"))
    {
        const QString dbusDaemon = QStandardPaths::findExecutable(QStringLiteral("dbus-daemon"));
        if (dbusDaemon.isEmpty()) {
            m_error = QStringLiteral("dbus-daemon is not available");
            return;
        }

        m_daemon.start(dbusDaemon, {QStringLiteral("--session"),
                                    QStringLiteral("--print-address=1"),
                                    QStringLiteral("--nofork")});
        if (!m_daemon.waitForStarted(3000)) {
            m_error = QStringLiteral("failed to start dbus-daemon");
            return;
        }
        if (!m_daemon.waitForReadyRead(3000)) {
            m_error = QStringLiteral("dbus-daemon did not print an address");
            return;
        }

        const QByteArray address = m_daemon.readLine().trimmed();
        if (address.isEmpty()) {
            m_error = QStringLiteral("dbus-daemon printed an empty address");
            return;
        }

        qputenv("DBUS_SESSION_BUS_ADDRESS", address);
        m_ready = true;
    }

    ~PrivateSessionBus()
    {
        if (m_ready) {
            const QString connectionName = QDBusConnection::sessionBus().name();
            QDBusConnection::disconnectFromBus(connectionName);
        }

        if (m_previousAddress.isEmpty()) {
            qunsetenv("DBUS_SESSION_BUS_ADDRESS");
        } else {
            qputenv("DBUS_SESSION_BUS_ADDRESS", m_previousAddress);
        }

        if (m_daemon.state() != QProcess::NotRunning) {
            m_daemon.terminate();
            if (!m_daemon.waitForFinished(3000)) {
                m_daemon.kill();
                m_daemon.waitForFinished(3000);
            }
        }
    }

    bool isReady() const
    {
        return m_ready;
    }

    QString errorString() const
    {
        return m_error;
    }

private:
    bool m_ready = false;
    QByteArray m_previousAddress;
    QString m_error;
    QProcess m_daemon;
};

}

class abstract_window_manager_test : public QObject
{
    Q_OBJECT

private slots:
    void activateWindowViewSendsHighlightWindowsCall();
};

void abstract_window_manager_test::activateWindowViewSendsHighlightWindowsCall()
{
    TestWindowManager manager;
    const QStringList windowIds{QStringLiteral("0x100001"), QStringLiteral("0x100002")};
    PrivateSessionBus privateBus;
    if (!privateBus.isReady()) {
        manager.activateWindowView(windowIds);
        QVERIFY(!privateBus.errorString().isEmpty());
        return;
    }

    QDBusConnection bus = QDBusConnection::sessionBus();
    QVERIFY2(bus.isConnected(), qPrintable(bus.lastError().message()));

    QVERIFY2(bus.registerService(HighlightService), qPrintable(bus.lastError().message()));
    RegisteredServiceGuard guard(bus, HighlightService, HighlightPath);

    HighlightReceiver receiver;
    QVERIFY2(bus.registerObject(HighlightPath, &receiver, QDBusConnection::ExportAllSlots),
             qPrintable(bus.lastError().message()));

    QSignalSpy spy(&receiver, &HighlightReceiver::highlighted);

    manager.activateWindowView(windowIds);

    QTRY_COMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).toStringList(), windowIds);
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    abstract_window_manager_test testCase;
    return QTest::qExec(&testCase, argc, argv);
}

#include "test-abstract-window-manager.moc"
