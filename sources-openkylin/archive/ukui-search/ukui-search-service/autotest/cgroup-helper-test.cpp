#include "cgroup-helper-test.h"

#include <QDBusConnection>
#include <QDBusContext>
#include <QDir>
#include <QTemporaryDir>
#include <QTest>

#define private public
#include "cgroup-helper.h"
#undef private

void uniquePtrDeleter(QDBusInterface *ptr);

namespace {

class FakeProcessManagerWhiteListService : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.kylin.ProcessManager.AppWhitelist")

public:
    QStringList appList;
    QStringList addedApps;
    bool failAppList = false;
    bool failAddApp = false;
    bool addAppResult = true;

public Q_SLOTS:
    QStringList AppList()
    {
        if (failAppList) {
            sendErrorReply(QDBusError::Failed, QStringLiteral("forced AppList failure"));
            return {};
        }
        return appList;
    }

    bool AddApp(const QString &desktopPath)
    {
        if (failAddApp) {
            sendErrorReply(QDBusError::Failed, QStringLiteral("forced AddApp failure"));
            return false;
        }
        addedApps.append(desktopPath);
        appList.append(desktopPath);
        return addAppResult;
    }
};

class ScopedDbusRegistration
{
public:
    ScopedDbusRegistration(const QString &service,
                           const QString &path,
                           QObject *object,
                           const QDBusConnection &connection = QDBusConnection::sessionBus())
        : m_service(service),
          m_path(path),
          m_connection(connection)
    {
        if (!m_connection.isConnected()) {
            return;
        }
        m_serviceRegistered = m_connection.registerService(service);
        m_objectRegistered = m_connection.registerObject(path, object, QDBusConnection::ExportAllSlots);
    }

    ~ScopedDbusRegistration()
    {
        if (m_objectRegistered) {
            m_connection.unregisterObject(m_path);
        }
        if (m_serviceRegistered) {
            m_connection.unregisterService(m_service);
        }
    }

    bool isRegistered() const
    {
        return m_serviceRegistered && m_objectRegistered;
    }

private:
    QString m_service;
    QString m_path;
    QDBusConnection m_connection{QString()};
    bool m_serviceRegistered = false;
    bool m_objectRegistered = false;
};

}

void CgroupHelperTest::testSetServiceCpuLimitFailsWithoutProcessManager()
{
    QTest::ignoreMessage(QtDebugMsg, "get dbusinterface failed");
    QVERIFY(!CgroupHelper::createProcessGroup(nullptr));
    QTest::ignoreMessage(QtDebugMsg, "get dbusinterface failed");
    QVERIFY(!CgroupHelper::moveProcessToGroup(nullptr));
    QTest::ignoreMessage(QtDebugMsg, "get dbusinterface failed");
    QVERIFY(!CgroupHelper::setProcessGroupCPULimit(nullptr));
    QTest::ignoreMessage(QtDebugMsg, "get dbusinterface failed");
    QVERIFY(!CgroupHelper::setProcessGroupCPULimitV1(nullptr));
    QTest::ignoreMessage(QtDebugMsg, "get dbusinterface failed");
    QVERIFY(!CgroupHelper::setProcessGroupCPULimitV2(nullptr));
    QVERIFY(!CgroupHelper::getPath().isNull());
}

void CgroupHelperTest::testUserPathV2Resolution()
{
    QTemporaryDir root;
    QVERIFY(root.isValid());

    const QString userSlice = root.path() + QStringLiteral("/user.slice");
    const QString userIdSlice = userSlice + QStringLiteral("/user-1000.slice");
    const QString userService = userIdSlice + QStringLiteral("/user@1000.service");
    QVERIFY(QDir().mkpath(userService));

    QCOMPARE(CgroupHelper::getUserPathV2(root.path() + QStringLiteral("/")), userService + QStringLiteral("/"));
    QCOMPARE(CgroupHelper::getUserPathV2(root.path() + QStringLiteral("/plain/")), root.path() + QStringLiteral("/plain/"));
}

void CgroupHelperTest::testUserPathV2IgnoresNonUserEntriesAndRecurses()
{
    QTemporaryDir root;
    QVERIFY(root.isValid());

    QVERIFY(QDir().mkpath(root.path() + QStringLiteral("/system.slice")));
    QVERIFY(QDir().mkpath(root.path() + QStringLiteral("/user.slice/user-1000.slice/user@1000.service/session.slice")));

    const QString resolved = CgroupHelper::getUserPathV2(root.path() + QStringLiteral("/"));
    QCOMPARE(resolved, root.path() + QStringLiteral("/user.slice/user-1000.slice/user@1000.service/"));

    QTemporaryDir noUserRoot;
    QVERIFY(noUserRoot.isValid());
    QVERIFY(QDir().mkpath(noUserRoot.path() + QStringLiteral("/plain.slice")));
    QCOMPARE(CgroupHelper::getUserPathV2(noUserRoot.path() + QStringLiteral("/")), noUserRoot.path() + QStringLiteral("/"));
}

void CgroupHelperTest::testUserPathV2AcceptsServiceAndSliceNameVariants()
{
    QTemporaryDir serviceRoot;
    QVERIFY(serviceRoot.isValid());
    QVERIFY(QDir().mkpath(serviceRoot.path() + QStringLiteral("/user@1000.service/session.slice")));
    QCOMPARE(CgroupHelper::getUserPathV2(serviceRoot.path() + QStringLiteral("/")),
             serviceRoot.path() + QStringLiteral("/user@1000.service/"));

    QTemporaryDir userDotRoot;
    QVERIFY(userDotRoot.isValid());
    QVERIFY(QDir().mkpath(userDotRoot.path() + QStringLiteral("/user.slice/user-1000.slice")));
    QCOMPARE(CgroupHelper::getUserPathV2(userDotRoot.path() + QStringLiteral("/")),
             userDotRoot.path() + QStringLiteral("/user.slice/user-1000.slice/"));

    QTemporaryDir ignoredRoot;
    QVERIFY(ignoredRoot.isValid());
    QVERIFY(QDir().mkpath(ignoredRoot.path() + QStringLiteral("/user-folder")));
    QVERIFY(QDir().mkpath(ignoredRoot.path() + QStringLiteral("/user-1000.scope")));
    QCOMPARE(CgroupHelper::getUserPathV2(ignoredRoot.path() + QStringLiteral("/")),
             ignoredRoot.path() + QStringLiteral("/"));
}

void CgroupHelperTest::testInvalidDbusInterfaceFailures()
{
    QDBusInterface invalidInterface(QStringLiteral("com.invalid.Service"),
                                    QStringLiteral("/com/invalid/Object"),
                                    QStringLiteral("com.invalid.Interface"),
                                    QDBusConnection::sessionBus());

    QVERIFY(!CgroupHelper::createProcessGroup(&invalidInterface));
    QVERIFY(!CgroupHelper::moveProcessToGroup(&invalidInterface));
    QVERIFY(!CgroupHelper::setProcessGroupCPULimitV1(&invalidInterface));
    QVERIFY(!CgroupHelper::setProcessGroupCPULimitV2(&invalidInterface));
    QVERIFY(!CgroupHelper::setProcessGroupCPULimit(&invalidInterface));
}

void CgroupHelperTest::testCgroupVersionAndPathAreStable()
{
    const int version = CgroupHelper::getCgroupVerson();
    QVERIFY(version == -1 || version == 1 || version == 2);

    const QString path = CgroupHelper::getPath();
    if (version == 1) {
        QCOMPARE(path, QStringLiteral("/ukui-search-service"));
    } else if (version == 2) {
        QVERIFY(path.startsWith(QStringLiteral("/")));
        QVERIFY(path.endsWith(QStringLiteral("session.slice/ukui-search-service")));
    } else {
        QVERIFY(path.isEmpty());
    }
}

void CgroupHelperTest::testUniquePtrDeleterAcceptsNullAndObject()
{
    uniquePtrDeleter(nullptr);
    uniquePtrDeleter(new QDBusInterface(QStringLiteral("com.invalid.Service"),
                                        QStringLiteral("/com/invalid/Object"),
                                        QStringLiteral("com.invalid.Interface"),
                                        QDBusConnection::sessionBus()));
}

void CgroupHelperTest::testAddProcessManagerWhiteListHandlesExistingAndNewEntries()
{
    if (!QDBusConnection::sessionBus().isConnected()) {
        QSKIP("session D-Bus is unavailable in this environment");
    }

    FakeProcessManagerWhiteListService service;
    ScopedDbusRegistration registration(QStringLiteral("com.kylin.ProcessManager"),
                                        QStringLiteral("/com/kylin/ProcessManager/AppWhitelist"),
                                        &service);
    QVERIFY(registration.isRegistered());

    service.appList = QStringList{QStringLiteral("/etc/xdg/autostart/ukui-search-service.desktop")};
    QVERIFY(CgroupHelper::addProcessManagerWhiteList());
    QVERIFY(service.addedApps.isEmpty());

    service.appList.clear();
    service.addedApps.clear();
    service.addAppResult = true;
    QVERIFY(CgroupHelper::addProcessManagerWhiteList());
    QCOMPARE(service.addedApps, QStringList{QStringLiteral("/etc/xdg/autostart/ukui-search-service.desktop")});
    QCOMPARE(service.appList, QStringList{QStringLiteral("/etc/xdg/autostart/ukui-search-service.desktop")});
}

void CgroupHelperTest::testServiceCpuLimitAndWhitelistFailSafelyWithoutDbus()
{
    if (!QDBusConnection::sessionBus().isConnected()) {
        QSKIP("session D-Bus is unavailable in this environment");
    }

    FakeProcessManagerWhiteListService service;
    ScopedDbusRegistration registration(QStringLiteral("com.kylin.ProcessManager"),
                                        QStringLiteral("/com/kylin/ProcessManager/AppWhitelist"),
                                        &service);
    QVERIFY(registration.isRegistered());

    service.failAppList = true;
    QVERIFY(!CgroupHelper::addProcessManagerWhiteList());

    service.failAppList = false;
    service.failAddApp = true;
    QVERIFY(!CgroupHelper::addProcessManagerWhiteList());

    service.failAppList = true;
    QVERIFY(!CgroupHelper::setServiceCpuLimit());
}

QTEST_MAIN(CgroupHelperTest)
#include "cgroup-helper-test.moc"
