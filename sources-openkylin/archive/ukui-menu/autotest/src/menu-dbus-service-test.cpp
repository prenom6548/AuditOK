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
 *
 */

#include <QtTest>
#include <QDir>
#include <QSignalSpy>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusContext>
#include <QDBusError>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDBusServiceWatcher>

namespace {

bool g_serviceRegistered = false;
bool g_registerObjectResult = true;
bool g_registerServiceResult = true;
uint g_servicePid = 0;
QString g_lastSentSignalPath;
QString g_lastSentSignalName;
QString g_lastConnectedService;
QString g_lastDisconnectedService;
QString g_lastWatcherService;
bool g_watcherDestroyed = false;

void resetState()
{
    g_serviceRegistered = false;
    g_registerObjectResult = true;
    g_registerServiceResult = true;
    g_servicePid = 0;
    g_lastSentSignalPath.clear();
    g_lastSentSignalName.clear();
    g_lastConnectedService.clear();
    g_lastDisconnectedService.clear();
    g_lastWatcherService.clear();
    g_watcherDestroyed = false;
}

} // namespace

template<typename T>
class TestFakeDBusReply
{
public:
    TestFakeDBusReply() = default;
    explicit TestFakeDBusReply(const T &value)
        : m_value(value)
        , m_valid(true)
    {
    }

    bool isValid() const { return m_valid; }
    T value() const { return m_value; }
    operator T() const { return m_value; }

private:
    T m_value {};
    bool m_valid {false};
};

class TestFakeDBusConnectionInterface
{
public:
    enum RegisterServiceReply {
        ServiceRegistered,
        ServiceNotRegistered
    };
    enum {
        ReplaceExistingService = 0,
        DontAllowReplacement = 1
    };

    bool isServiceRegistered(const QString &) const
    {
        return g_serviceRegistered;
    }

    TestFakeDBusReply<RegisterServiceReply> registerService(const QString &, int, int) const
    {
        return TestFakeDBusReply<RegisterServiceReply>(g_registerServiceResult ? ServiceRegistered : ServiceNotRegistered);
    }

    TestFakeDBusReply<uint> servicePid(const QString &) const
    {
        return TestFakeDBusReply<uint>(g_servicePid);
    }
};

class TestFakeDBusConnection
{
public:
    static TestFakeDBusConnection sessionBus()
    {
        return {};
    }

    TestFakeDBusConnectionInterface *interface() const
    {
        return &s_interface;
    }

    bool registerObject(const QString &, QObject *) const
    {
        return g_registerObjectResult;
    }

    bool connect(const QString &service, const QString &, const QString &, const QString &, QObject *, const char *) const
    {
        g_lastConnectedService = service;
        return true;
    }

    bool disconnect(const QString &service, const QString &, const QString &, const QString &, QObject *, const char *) const
    {
        g_lastDisconnectedService = service;
        return true;
    }

    bool send(const QDBusMessage &message) const
    {
        g_lastSentSignalPath = message.path();
        g_lastSentSignalName = message.member();
        return true;
    }

    QDBusError lastError() const
    {
        return QDBusError(QDBusError::Failed, QStringLiteral("fake-dbus-error"));
    }

private:
    static TestFakeDBusConnectionInterface s_interface;
};

TestFakeDBusConnectionInterface TestFakeDBusConnection::s_interface;

class TestMenuAdaptor
{
public:
    explicit TestMenuAdaptor(QObject *)
    {
    }
};

class TestFakeServiceWatcher : public QObject
{
    Q_OBJECT
public:
    enum WatchMode {
        WatchForOwnerChange
    };

    TestFakeServiceWatcher(const QString &service, const TestFakeDBusConnection &, WatchMode, QObject *parent = nullptr)
        : QObject(parent)
    {
        g_lastWatcherService = service;
    }

    ~TestFakeServiceWatcher() override
    {
        g_watcherDestroyed = true;
    }

Q_SIGNALS:
    void serviceOwnerChanged(const QString &service, const QString &oldOwner, const QString &newOwner);
};

#define QDBusConnection TestFakeDBusConnection
#define QDBusConnectionInterface TestFakeDBusConnectionInterface
#define QDBusServiceWatcher TestFakeServiceWatcher
#define QDBusReply TestFakeDBusReply
#define MenuAdaptor TestMenuAdaptor
#define MENUADAPTOR_H
#define private public
#include "../../src/menu-dbus-service.cpp"
#undef private
#undef MENUADAPTOR_H
#undef MenuAdaptor
#undef QDBusReply
#undef QDBusServiceWatcher
#undef QDBusConnectionInterface
#undef QDBusConnection

using namespace UkuiMenu;

class menuDbusService_test : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void test_displayFromPid_invalidPid_returnsEmpty();
    void test_active_emitsExpectedSignals();
    void test_activeMenu_onlyMatchesSameDisplay();
    void test_visible_roundTripAndNoopUpdate();
    void test_constructor_registersServiceAndDoesNotWatchOnSuccess();
    void test_registerService_failureFallsBackToWatcher();
    void test_registerService_serviceReplyFailureFallsBackToWatcher();
    void test_ownerChanged_emptyOwner_reRegistersAndClearsWatcher();
    void test_ownerChanged_emptyOwner_registerFailureKeepsWatcher();
    void test_connectAndDisconnectActiveRequest();
    void test_sendPropertiesChangedSignal_andOwnerChanged_nonEmptyOwner();
};

void menuDbusService_test::init()
{
    resetState();
}

void menuDbusService_test::test_displayFromPid_invalidPid_returnsEmpty()
{
    QCOMPARE(MenuDbusService::displayFromPid(0), QString());
    QCOMPARE(MenuDbusService::displayFromPid(UINT_MAX), QString());
}

void menuDbusService_test::test_active_emitsExpectedSignals()
{
    MenuDbusService service(QStringLiteral(":0"));
    QSignalSpy activeSpy(&service, &MenuDbusService::menuActive);
    QSignalSpy requestSpy(&service, &MenuDbusService::activeRequest);

    service.active(QString());
    QCOMPARE(activeSpy.count(), 1);
    QCOMPARE(requestSpy.count(), 0);

    service.active(QStringLiteral(":0"));
    QCOMPARE(activeSpy.count(), 2);
    QCOMPARE(requestSpy.count(), 0);

    service.active(QStringLiteral(":1"));
    QCOMPARE(activeSpy.count(), 2);
    QCOMPARE(requestSpy.count(), 1);
    QCOMPARE(requestSpy.takeFirst().at(0).toString(), QStringLiteral(":1"));
}

void menuDbusService_test::test_activeMenu_onlyMatchesSameDisplay()
{
    MenuDbusService service(QStringLiteral(":0"));
    QSignalSpy activeSpy(&service, &MenuDbusService::menuActive);

    service.activeMenu(QStringLiteral(":1"));
    QCOMPARE(activeSpy.count(), 0);

    service.activeMenu(QStringLiteral(":0"));
    QCOMPARE(activeSpy.count(), 1);
}

void menuDbusService_test::test_visible_roundTripAndNoopUpdate()
{
    MenuDbusService service(QStringLiteral(":0"));

    QCOMPARE(service.visible(), false);
    service.setVisible(false);
    QCOMPARE(service.visible(), false);

    service.sendPropertiesChangedSignal(QStringLiteral("Visible"), true);
    QCOMPARE(g_lastSentSignalPath, QStringLiteral("/org/ukui/menu"));
    QCOMPARE(g_lastSentSignalName, QStringLiteral("PropertiesChanged"));

    service.setVisible(true);
    QCOMPARE(service.visible(), true);

    service.setVisible(true);
    QCOMPARE(service.visible(), true);
}

void menuDbusService_test::test_constructor_registersServiceAndDoesNotWatchOnSuccess()
{
    g_serviceRegistered = false;
    g_registerObjectResult = true;
    g_registerServiceResult = true;

    MenuDbusService service(QStringLiteral(":0"));
    QCOMPARE(g_lastConnectedService, QString());
    QCOMPARE(g_lastWatcherService, QString());
    QVERIFY(service.visible() == false);
}

void menuDbusService_test::test_registerService_failureFallsBackToWatcher()
{
    g_serviceRegistered = false;
    g_registerObjectResult = false;
    g_registerServiceResult = false;

    MenuDbusService service(QStringLiteral(":0"));
    QCOMPARE(g_lastConnectedService, QStringLiteral("org.ukui.menu"));
    QVERIFY(g_lastWatcherService == QStringLiteral("org.ukui.menu"));
    QVERIFY(!g_watcherDestroyed);
}

void menuDbusService_test::test_registerService_serviceReplyFailureFallsBackToWatcher()
{
    g_serviceRegistered = false;
    g_registerObjectResult = true;
    g_registerServiceResult = false;

    MenuDbusService service(QStringLiteral(":0"));
    QCOMPARE(g_lastConnectedService, QStringLiteral("org.ukui.menu"));
    QCOMPARE(g_lastWatcherService, QStringLiteral("org.ukui.menu"));
    QVERIFY(!g_watcherDestroyed);
}

void menuDbusService_test::test_ownerChanged_emptyOwner_reRegistersAndClearsWatcher()
{
    g_serviceRegistered = true;
    g_registerObjectResult = true;
    g_registerServiceResult = true;

    MenuDbusService service(QStringLiteral(":0"));
    QCOMPARE(g_lastWatcherService, QStringLiteral("org.ukui.menu"));

    service.onServiceOwnerChanged(QStringLiteral("org.ukui.menu"),
                                  QStringLiteral(":1.1"),
                                  QString());

    QCOMPARE(g_lastDisconnectedService, QStringLiteral("org.ukui.menu"));
    QVERIFY(g_watcherDestroyed);
}

void menuDbusService_test::test_ownerChanged_emptyOwner_registerFailureKeepsWatcher()
{
    g_serviceRegistered = true;
    g_registerObjectResult = true;
    g_registerServiceResult = true;

    MenuDbusService service(QStringLiteral(":0"));
    QCOMPARE(g_lastWatcherService, QStringLiteral("org.ukui.menu"));

    g_registerObjectResult = false;
    service.onServiceOwnerChanged(QStringLiteral("org.ukui.menu"),
                                  QStringLiteral(":1.1"),
                                  QString());

    QVERIFY(g_lastDisconnectedService.isEmpty());
    QVERIFY(!g_watcherDestroyed);
    QVERIFY(service.m_watcher != nullptr);
}

void menuDbusService_test::test_connectAndDisconnectActiveRequest()
{
    MenuDbusService service(QStringLiteral(":0"));

    service.connectToActiveRequest();
    QCOMPARE(g_lastConnectedService, QStringLiteral("org.ukui.menu"));

    service.disConnectActiveRequest();
    QCOMPARE(g_lastDisconnectedService, QStringLiteral("org.ukui.menu"));
}

void menuDbusService_test::test_sendPropertiesChangedSignal_andOwnerChanged_nonEmptyOwner()
{
    g_serviceRegistered = true;
    g_registerObjectResult = true;
    g_registerServiceResult = true;
    g_servicePid = 4242;

    MenuDbusService service(QStringLiteral(":0"));
    service.sendPropertiesChangedSignal(QStringLiteral("Visible"), false);
    QCOMPARE(g_lastSentSignalPath, QStringLiteral("/org/ukui/menu"));
    QCOMPARE(g_lastSentSignalName, QStringLiteral("PropertiesChanged"));

    service.onServiceOwnerChanged(QStringLiteral("org.ukui.menu"),
                                  QStringLiteral(":1.1"),
                                  QStringLiteral(":1.2"));

    QVERIFY(g_lastDisconnectedService.isEmpty());
}

QTEST_MAIN(menuDbusService_test)

#include "menu-dbus-service-test.moc"
