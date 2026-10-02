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
#include <QDBusInterface>
#include <QDBusReply>
#include <QDateTime>
#include <QProcess>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QtTest>

#include <memory>

#include "dbus-connector.h"

using namespace UkuiQuick;

namespace {

constexpr auto TestInterface = "org.ukui.quick.TestConnector";
constexpr auto TestPath = "/org/ukui/quick/TestConnector";

class TestService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.quick.TestConnector")

public slots:
    QString ping() const
    {
        return QStringLiteral("pong");
    }
};

QString uniqueServiceName(const QString &suffix)
{
    return QStringLiteral("org.ukui.quick.TestConnector.pid%1.case%2")
        .arg(QCoreApplication::applicationPid())
        .arg(suffix);
}

class PrivateSessionBus
{
public:
    explicit PrivateSessionBus(const QString &name)
        : m_name(name)
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

        const QString address = QString::fromUtf8(m_daemon.readLine().trimmed());
        if (address.isEmpty()) {
            m_error = QStringLiteral("dbus-daemon printed an empty address");
            return;
        }

        m_connection = std::make_unique<QDBusConnection>(QDBusConnection::connectToBus(address, m_name));
        if (!m_connection->isConnected()) {
            m_error = QStringLiteral("failed to connect to private session bus");
        }
    }

    ~PrivateSessionBus()
    {
        if (m_connection && m_connection->isConnected()) {
            QDBusConnection::disconnectFromBus(m_name);
        }

        if (m_daemon.state() != QProcess::NotRunning) {
            m_daemon.terminate();
            if (!m_daemon.waitForFinished(3000)) {
                m_daemon.kill();
                m_daemon.waitForFinished(3000);
            }
        }
    }

    bool isConnected() const
    {
        return m_connection && m_connection->isConnected();
    }

    QDBusConnection connection() const
    {
        return *m_connection;
    }

    QString errorString() const
    {
        return m_error;
    }

private:
    QString m_name;
    QString m_error;
    QProcess m_daemon;
    std::unique_ptr<QDBusConnection> m_connection;
};

} // namespace

class dbus_connector_test : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void generatedServiceNamesAreValidBusNames();
    void readyIsEmittedForRegisteredService();
    void missingServiceCanBeStoppedWithoutReady();
};

void dbus_connector_test::initTestCase()
{
    qRegisterMetaType<QDBusInterface *>("QDBusInterface*");
    qRegisterMetaType<QDBusInterface *>("QDBusInterface *");
}

void dbus_connector_test::generatedServiceNamesAreValidBusNames()
{
    const auto serviceNames = {
        uniqueServiceName(QStringLiteral("missing")),
        uniqueServiceName(QStringLiteral("123456")),
    };

    for (const auto &serviceName : serviceNames) {
        for (const auto &element : serviceName.split(QLatin1Char('.'))) {
            QVERIFY2(!element.isEmpty(), qPrintable(serviceName));
            QVERIFY2(!element.at(0).isDigit(), qPrintable(serviceName));
        }
    }
}

void dbus_connector_test::readyIsEmittedForRegisteredService()
{
    PrivateSessionBus privateBus(QStringLiteral("codex-ready-bus"));
    if (!privateBus.isConnected()) {
        const QByteArray reason = privateBus.errorString().toUtf8();
        QSKIP(reason.constData());
    }

    auto bus = privateBus.connection();
    const QString serviceName = uniqueServiceName(QString::number(QDateTime::currentMSecsSinceEpoch()));
    TestService service;
    QVERIFY2(bus.registerService(serviceName), qPrintable(bus.lastError().message()));
    QVERIFY2(bus.registerObject(QString::fromLatin1(TestPath), &service, QDBusConnection::ExportAllSlots),
             qPrintable(bus.lastError().message()));

    DbusConnector connector(serviceName, QString::fromLatin1(TestPath), QString::fromLatin1(TestInterface), bus);
    QSignalSpy readySpy(&connector, &DbusConnector::ready);

    connector.start();

    QVERIFY(readySpy.wait(3000));
    QVERIFY(connector.wait(3000));
    QCOMPARE(readySpy.count(), 1);

    auto *interface = readySpy.takeFirst().at(0).value<QDBusInterface *>();
    QVERIFY(interface != nullptr);
    QCOMPARE(interface->service(), serviceName);
    QCOMPARE(interface->path(), QString::fromLatin1(TestPath));
    QCOMPARE(interface->interface(), QString::fromLatin1(TestInterface));

    QDBusReply<QString> reply = interface->call(QStringLiteral("ping"));
    QVERIFY(reply.isValid());
    QCOMPARE(reply.value(), QStringLiteral("pong"));

    delete interface;
    bus.unregisterObject(QString::fromLatin1(TestPath));
    bus.unregisterService(serviceName);
}

void dbus_connector_test::missingServiceCanBeStoppedWithoutReady()
{
    PrivateSessionBus privateBus(QStringLiteral("codex-missing-bus"));
    auto bus = privateBus.isConnected() ? privateBus.connection() : QDBusConnection::sessionBus();
    const QString serviceName = uniqueServiceName(QStringLiteral("missing"));
    DbusConnector connector(serviceName, QString::fromLatin1(TestPath), QString::fromLatin1(TestInterface), bus);
    QSignalSpy readySpy(&connector, &DbusConnector::ready);

    connector.start();
    QTest::qWait(100);
    QCOMPARE(readySpy.count(), 0);

    connector.quit();
    QVERIFY(connector.wait(3000));
}

QTEST_GUILESS_MAIN(dbus_connector_test)

#include "test-dbus-connector.moc"
