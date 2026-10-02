#include <QtTest/QtTest>
#include "../../kysdk-appmanager/src/appinfo.h"
#include "../../kysdk-appmanager/src/kappmanager.h"

class TestAppManager : public QObject
{
    Q_OBJECT

private slots:
    void testAppInfo();
    void testGetApplications();
    void testGetApplicationByMime();
    void testGetAutoStartApplications();
    void testAddRemoveAutoStartApplication();
    void testGetDefaultApplicationByMime();
    void testGetDefaultApplicationByType();
    void testSetDefaultApplicationByMime();
    void testSetDefaultApplicationByType();

private:
    kdk::KAppManager appManager;
};

void TestAppManager::testAppInfo()
{
    kdk::AppInfo appInfo("/usr/share/applications/atril.desktop");
    QVERIFY(appInfo.isVaild());
    QCOMPARE(appInfo.name(), QString("文档查看器"));
    QCOMPARE(appInfo.type(), QString("Application"));
    QCOMPARE(appInfo.exec(), QString("atril %U"));
    QCOMPARE(appInfo.icon(), QString("atril"));
    QCOMPARE(appInfo.comment(), QString("View multi-page documents"));
    QCOMPARE(appInfo.desktopName(), QString("atril"));
    QCOMPARE(appInfo.desktopPath(), QString("/usr/share/applications/atril.desktop"));
    QCOMPARE(appInfo.genericName(), QString("文档查看器"));
    QCOMPARE(appInfo.terminal(), false);
    kdk::AppInfo cpInfo(appInfo);
}

void TestAppManager::testGetApplications()
{
    QList<kdk::AppInfo*> apps = appManager.getApplications();
    QVERIFY(apps.size() > 0);
}

void TestAppManager::testGetApplicationByMime()
{
    QList<kdk::AppInfo*> apps = appManager.getApplicationByMime("text/plain");
    QVERIFY(apps.size() > 0);
}

void TestAppManager::testGetAutoStartApplications()
{
    QList<kdk::AppInfo*> apps = appManager.getAutoStartApplications();
    QVERIFY(apps.size() >= 0);
}

void TestAppManager::testAddRemoveAutoStartApplication()
{
    QString desktopPath = "/usr/share/applications/atril.desktop";
    QVERIFY(appManager.addAutoStartApplication(desktopPath));
    QVERIFY(appManager.removeAutoStartApplication("atril.desktop"));
}

void TestAppManager::testGetDefaultApplicationByMime()
{
    kdk::AppInfo* app = appManager.getDefaultApplicationByMime("text/plain");
    QVERIFY(app != nullptr);
}

void TestAppManager::testGetDefaultApplicationByType()
{
    kdk::AppInfo* app = appManager.getDefaultApplicationByType(kdk::KAppManager::TEXT);
    QVERIFY(app != nullptr);
}

void TestAppManager::testSetDefaultApplicationByMime()
{
    QString desktopPath = "/usr/share/applications/atril.desktop";
    QVERIFY(appManager.setDefaultApplicationByMime(desktopPath, "text/plain"));
}

void TestAppManager::testSetDefaultApplicationByType()
{
    QString desktopPath = "/usr/share/applications/atril.desktop";
    QVERIFY(appManager.setDefaultApplicationByType(desktopPath, kdk::KAppManager::BROWSER));
}

QTEST_APPLESS_MAIN(TestAppManager)

#include "appmanagertest.moc"