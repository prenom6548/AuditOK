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

#include <QDBusConnection>
#include <QDBusError>
#include <QDBusPendingCallWatcher>
#include <QDesktopServices>
#include <QFile>
#include <QGuiApplication>
#include <QProcess>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QtTest>

#include <memory>

#include "ukui/app-launcher.h"

using namespace UkuiQuick;

namespace {

constexpr auto LauncherService = "com.kylin.ProcessManager";
constexpr auto LauncherPath = "/com/kylin/ProcessManager/AppLauncher";
constexpr auto QtDefaultSessionBus = "qt_default_session_bus";

class ProcessManagerReceiver : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.kylin.ProcessManager.AppLauncher")

public slots:
    void LaunchApp(const QString &desktopFile)
    {
        Q_EMIT launchAppCalled(desktopFile);
    }

    void LaunchAppWithArguments(const QString &desktopFile, const QStringList &args)
    {
        Q_EMIT launchAppWithArgumentsCalled(desktopFile, args);
    }

    void RunCommand(const QString &cmd)
    {
        Q_EMIT runCommandCalled(cmd);
    }

signals:
    void launchAppCalled(const QString &desktopFile);
    void launchAppWithArgumentsCalled(const QString &desktopFile, const QStringList &args);
    void runCommandCalled(const QString &cmd);
};

class UrlHandlerReceiver : public QObject
{
    Q_OBJECT

public slots:
    void handle(const QUrl &url)
    {
        Q_EMIT handled(url);
    }

signals:
    void handled(const QUrl &url);
};

QString writeExecutableScript(QTemporaryDir &dir, const QString &name, const QByteArray &contents)
{
    const QString path = dir.filePath(name);
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return {};
    }

    if (file.write(contents) != contents.size()) {
        return {};
    }
    file.close();

    if (!file.setPermissions(QFileDevice::ReadOwner
                             | QFileDevice::WriteOwner
                             | QFileDevice::ExeOwner
                             | QFileDevice::ReadGroup
                             | QFileDevice::ExeGroup
                             | QFileDevice::ReadOther
                             | QFileDevice::ExeOther)) {
        return {};
    }

    return path;
}

QString writeDesktopFile(QTemporaryDir &dir, const QString &name, const QString &execLine)
{
    const QString path = dir.filePath(name + QStringLiteral(".desktop"));
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return {};
    }

    const QByteArray contents =
        "[Desktop Entry]\n"
        "Type=Application\n"
        "Name=Codex Test\n"
        "Terminal=false\n"
        "Exec=" + execLine.toUtf8() + "\n";
    if (file.write(contents) != contents.size()) {
        return {};
    }
    file.close();
    return path;
}

void waitForNoPendingCallWatchers(QObject *object)
{
    QTRY_COMPARE(object->findChildren<QDBusPendingCallWatcher *>().size(), 0);
}

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
        QDBusConnection::disconnectFromBus(QString::fromLatin1(QtDefaultSessionBus));
        m_connection = std::make_unique<QDBusConnection>(
            QDBusConnection::connectToBus(QString::fromUtf8(address), QString::fromLatin1(QtDefaultSessionBus)));
        m_ready = m_connection && m_connection->isConnected();
        if (!m_ready) {
            m_error = QStringLiteral("failed to connect to private session bus");
        }
    }

    ~PrivateSessionBus()
    {
        QDBusConnection::disconnectFromBus(QString::fromLatin1(QtDefaultSessionBus));

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
    std::unique_ptr<QDBusConnection> m_connection;
};

} // namespace

class app_launcher_test : public QObject
{
    Q_OBJECT

private slots:
    void cleanupTestCase();
    void singletonReturnsSharedInstance();
    void dbusMethodsAreForwardedToProcessManager();
    void launchAppFallsBackWhenProcessManagerCallFails();
    void launchAppWithArgumentsFallsBackWhenProcessManagerCallFails();
    void runCommandFallsBackToDetachedProcessWhenProcessManagerCallFails();
    void emptyInputsAreIgnoredAndOpenUriUsesDesktopHandler();
    void invalidUriStillAttemptsDesktopOpen();
    void emptyParentWindowArgumentDoesNotAffectOpenUri();
};

void app_launcher_test::cleanupTestCase()
{
    delete AppLauncher::instance();
}

void app_launcher_test::singletonReturnsSharedInstance()
{
    auto *launcher = AppLauncher::instance();
    QVERIFY(launcher != nullptr);
    QCOMPARE(AppLauncher::instance(), launcher);
}

void app_launcher_test::dbusMethodsAreForwardedToProcessManager()
{
    PrivateSessionBus privateBus;
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!privateBus.isReady() && !bus.isConnected()) {
        const QByteArray reason = privateBus.errorString().toUtf8();
        QSKIP(reason.constData());
    }
    if (!bus.isConnected()) {
        QSKIP("session bus is not connected");
    }

    if (!bus.registerService(QString::fromLatin1(LauncherService))) {
        const QByteArray reason = bus.lastError().message().toUtf8();
        QSKIP(reason.constData());
    }
    RegisteredServiceGuard guard(bus,
                                 QString::fromLatin1(LauncherService),
                                 QString::fromLatin1(LauncherPath));

    ProcessManagerReceiver receiver;
    QVERIFY2(bus.registerObject(QString::fromLatin1(LauncherPath), &receiver, QDBusConnection::ExportAllSlots),
             qPrintable(bus.lastError().message()));

    QSignalSpy launchSpy(&receiver, &ProcessManagerReceiver::launchAppCalled);
    QSignalSpy launchArgsSpy(&receiver, &ProcessManagerReceiver::launchAppWithArgumentsCalled);
    QSignalSpy runCommandSpy(&receiver, &ProcessManagerReceiver::runCommandCalled);

    auto *launcher = AppLauncher::instance();
    const QString desktopFile = QStringLiteral("/tmp/codex-launch.desktop");
    const QStringList args{QStringLiteral("--flag"), QStringLiteral("value")};
    const QString command = QStringLiteral("printf codex");

    launcher->launchApp(desktopFile);
    QTRY_COMPARE(launchSpy.count(), 1);
    QCOMPARE(launchSpy.takeFirst().at(0).toString(), desktopFile);

    launcher->launchAppWithArguments(desktopFile, args);
    QTRY_COMPARE(launchArgsSpy.count(), 1);
    const auto launchArgs = launchArgsSpy.takeFirst();
    QCOMPARE(launchArgs.at(0).toString(), desktopFile);
    QCOMPARE(launchArgs.at(1).toStringList(), args);

    launcher->runCommand(command);
    QTRY_COMPARE(runCommandSpy.count(), 1);
    QCOMPARE(runCommandSpy.takeFirst().at(0).toString(), command);
    waitForNoPendingCallWatchers(launcher);
}

void app_launcher_test::emptyInputsAreIgnoredAndOpenUriUsesDesktopHandler()
{
    UrlHandlerReceiver receiver;
    QSignalSpy handledSpy(&receiver, &UrlHandlerReceiver::handled);
    const QString scheme = QStringLiteral("codex-open");
    const QString uri = QStringLiteral("codex-open:test-target");

    QDesktopServices::setUrlHandler(scheme, &receiver, "handle");

    auto *launcher = AppLauncher::instance();
    launcher->launchApp(QString());
    launcher->launchAppWithArguments(QString(), {QStringLiteral("--ignored")});
    launcher->runCommand(QString());
    launcher->openUri(QString());
    QCOMPARE(handledSpy.count(), 0);

    launcher->openUri(uri);
    QTRY_COMPARE(handledSpy.count(), 1);
    QCOMPARE(handledSpy.takeFirst().at(0).toUrl(), QUrl(uri));

    QDesktopServices::unsetUrlHandler(scheme);
}

void app_launcher_test::launchAppFallsBackWhenProcessManagerCallFails()
{
    PrivateSessionBus privateBus;
    if (!privateBus.isReady()) {
        const QByteArray reason = privateBus.errorString().toUtf8();
        QSKIP(reason.constData());
    }

    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString markerPath = dir.filePath(QStringLiteral("launch.marker"));
    const QByteArray scriptContents =
        "#!/bin/sh\n"
        "printf fallback-launch > \"" + markerPath.toUtf8() + "\"\n";
    const QString scriptPath = writeExecutableScript(dir, QStringLiteral("launch.sh"), scriptContents);
    QVERIFY(!scriptPath.isEmpty());

    const QString desktopFile = writeDesktopFile(dir, QStringLiteral("launch"), scriptPath);
    QVERIFY(!desktopFile.isEmpty());
    auto *launcher = AppLauncher::instance();

    launcher->launchApp(desktopFile);
    QTRY_VERIFY(QFile::exists(markerPath));

    QFile markerFile(markerPath);
    QVERIFY(markerFile.open(QIODevice::ReadOnly));
    QCOMPARE(QString::fromUtf8(markerFile.readAll()).trimmed(), QStringLiteral("fallback-launch"));
    waitForNoPendingCallWatchers(launcher);
}

void app_launcher_test::launchAppWithArgumentsFallsBackWhenProcessManagerCallFails()
{
    PrivateSessionBus privateBus;
    if (!privateBus.isReady()) {
        const QByteArray reason = privateBus.errorString().toUtf8();
        QSKIP(reason.constData());
    }

    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString markerPath = dir.filePath(QStringLiteral("launch-args.marker"));
    const QByteArray scriptContents =
        "#!/bin/sh\n"
        "printf fallback-launch-args > \"" + markerPath.toUtf8() + "\"\n";
    const QString scriptPath = writeExecutableScript(dir, QStringLiteral("launch-with-args.sh"), scriptContents);
    QVERIFY(!scriptPath.isEmpty());

    const QString desktopFile = writeDesktopFile(dir, QStringLiteral("launch-with-args"), scriptPath);
    QVERIFY(!desktopFile.isEmpty());
    auto *launcher = AppLauncher::instance();

    launcher->launchAppWithArguments(desktopFile, {QStringLiteral("--flag"), QStringLiteral("value")});
    QTRY_VERIFY(QFile::exists(markerPath));

    QFile markerFile(markerPath);
    QVERIFY(markerFile.open(QIODevice::ReadOnly));
    QCOMPARE(QString::fromUtf8(markerFile.readAll()).trimmed(), QStringLiteral("fallback-launch-args"));
    waitForNoPendingCallWatchers(launcher);
}

void app_launcher_test::runCommandFallsBackToDetachedProcessWhenProcessManagerCallFails()
{
    PrivateSessionBus privateBus;
    if (!privateBus.isReady()) {
        const QByteArray reason = privateBus.errorString().toUtf8();
        QSKIP(reason.constData());
    }

    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString markerPath = dir.filePath(QStringLiteral("run-command.marker"));
    const QByteArray scriptContents =
        "#!/bin/sh\n"
        "printf fallback-run > \"" + markerPath.toUtf8() + "\"\n";
    const QString scriptPath = writeExecutableScript(dir, QStringLiteral("run-command.sh"), scriptContents);
    QVERIFY(!scriptPath.isEmpty());

    auto *launcher = AppLauncher::instance();

    launcher->runCommand(scriptPath);
    QTRY_VERIFY(QFile::exists(markerPath));

    QFile markerFile(markerPath);
    QVERIFY(markerFile.open(QIODevice::ReadOnly));
    QCOMPARE(QString::fromUtf8(markerFile.readAll()).trimmed(), QStringLiteral("fallback-run"));
    waitForNoPendingCallWatchers(launcher);
}

void app_launcher_test::invalidUriStillAttemptsDesktopOpen()
{
    UrlHandlerReceiver receiver;
    QDesktopServices::setUrlHandler(QStringLiteral("file"), &receiver, "handle");
    QSignalSpy handledSpy(&receiver, &UrlHandlerReceiver::handled);

    auto *launcher = AppLauncher::instance();
    const QString uri = QStringLiteral("file:///tmp/codex-missing-target");
    launcher->openUri(uri);

    QTRY_COMPARE(handledSpy.count(), 1);
    QCOMPARE(handledSpy.takeFirst().at(0).toUrl(), QUrl(uri));
    QDesktopServices::unsetUrlHandler(QStringLiteral("file"));
}

void app_launcher_test::emptyParentWindowArgumentDoesNotAffectOpenUri()
{
    UrlHandlerReceiver receiver;
    QDesktopServices::setUrlHandler(QStringLiteral("https"), &receiver, "handle");
    QSignalSpy handledSpy(&receiver, &UrlHandlerReceiver::handled);

    auto *launcher = AppLauncher::instance();
    const QString uri = QStringLiteral("https://example.invalid");
    launcher->openUri(uri, QString());

    QTRY_COMPARE(handledSpy.count(), 1);
    QCOMPARE(handledSpy.takeFirst().at(0).toUrl(), QUrl(uri));
    QDesktopServices::unsetUrlHandler(QStringLiteral("https"));
}

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QT_QPA_PLATFORMTHEME", "generic");

    QGuiApplication app(argc, argv);
    app_launcher_test testCase;
    return QTest::qExec(&testCase, argc, argv);
}

#include "test-app-launcher.moc"
