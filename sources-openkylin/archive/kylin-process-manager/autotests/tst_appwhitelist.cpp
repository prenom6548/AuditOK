/*
 * Copyright 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "core/appwhitelist.h"
#include "core/configmanager.h"
#include <QtCore/QString>
#include <QtTest>
#include <memory>
#include <qdebug.h>

// 创建一个MockConfigManager类用于测试
class MockConfigManager : public ConfigManager
{
public:
    MockConfigManager()
        : ConfigManager()
    {
    }

    bool addAppToWhitelist(const QString &desktopFile) override
    {
        // 如果应用已在白名单中，返回false
        if (m_whitelist.contains(desktopFile)) {
            return false;
        }

        qDebug() << "Added app to whitelist: " << desktopFile;

        m_whitelist.append(desktopFile);
        return true;
    }

    bool removeAppFromWhitelist(const QString &desktopFile) override
    {
        return m_whitelist.removeOne(desktopFile);
    }

    const QStringList &appWhitelist() const override
    {
        return m_whitelist;
    }

    // 用于测试的辅助方法
    void clearWhitelist()
    {
        m_whitelist.clear();
    }

    void setWhitelist(const QStringList &whitelist)
    {
        m_whitelist = whitelist;
    }

private:
    QStringList m_whitelist;
};

class TestAppWhitelist : public QObject
{
    Q_OBJECT

public:
    TestAppWhitelist();
    ~TestAppWhitelist();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void test_AddApp();
    void test_RemoveApp();
    void test_AppList();
    void test_AddDuplicateApp();
    void test_RemoveNonExistentApp();
    void test_EmptyWhitelist();

private:
    std::unique_ptr<MockConfigManager> m_mockConfigManager;
    std::unique_ptr<AppWhitelist> m_appWhitelist;
};

TestAppWhitelist::TestAppWhitelist()
{
}

TestAppWhitelist::~TestAppWhitelist()
{
}

void TestAppWhitelist::initTestCase()
{
    m_mockConfigManager.reset(new MockConfigManager());
    m_appWhitelist.reset(new AppWhitelist(*m_mockConfigManager));
}

void TestAppWhitelist::cleanupTestCase()
{
    m_appWhitelist.reset();
    m_mockConfigManager.reset();
}

void TestAppWhitelist::test_AddApp()
{
    m_mockConfigManager->clearWhitelist();

    // 测试添加应用到空白名单
    QString testApp1 = "/usr/share/applications/test1.desktop";
    bool result = m_appWhitelist->AddApp(testApp1);
    qDebug() << "Resetl " << result;

    QVERIFY(result);
    QCOMPARE(m_mockConfigManager->appWhitelist().size(), 1);
    QVERIFY(m_mockConfigManager->appWhitelist().contains(testApp1));

    // 测试添加第二个应用
    QString testApp2 = "/usr/share/applications/test2.desktop";
    result = m_appWhitelist->AddApp(testApp2);

    QVERIFY(result);
    QCOMPARE(m_mockConfigManager->appWhitelist().size(), 2);
    QVERIFY(m_mockConfigManager->appWhitelist().contains(testApp2));
}

void TestAppWhitelist::test_RemoveApp()
{
    // 准备测试数据
    m_mockConfigManager->clearWhitelist();
    QStringList initialWhitelist = {
        "/usr/share/applications/test1.desktop",
        "/usr/share/applications/test2.desktop",
        "/usr/share/applications/test3.desktop"};
    m_mockConfigManager->setWhitelist(initialWhitelist);

    // 测试移除存在的应用
    QString appToRemove = "/usr/share/applications/test2.desktop";
    bool result = m_appWhitelist->RemoveApp(appToRemove);

    QVERIFY(result);
    QCOMPARE(m_mockConfigManager->appWhitelist().size(), 2);
    QVERIFY(!m_mockConfigManager->appWhitelist().contains(appToRemove));
}

void TestAppWhitelist::test_AppList()
{
    // 准备测试数据
    m_mockConfigManager->clearWhitelist();
    QStringList expectedWhitelist = {
        "/usr/share/applications/app1.desktop",
        "/usr/share/applications/app2.desktop",
        "/usr/share/applications/app3.desktop"};
    m_mockConfigManager->setWhitelist(expectedWhitelist);

    // 测试获取白名单
    QStringList actualWhitelist = m_appWhitelist->AppList();

    QCOMPARE(actualWhitelist.size(), expectedWhitelist.size());
    for (const auto &app : expectedWhitelist) {
        QVERIFY(actualWhitelist.contains(app));
    }
}

void TestAppWhitelist::test_AddDuplicateApp()
{
    // 准备测试数据
    m_mockConfigManager->clearWhitelist();
    QString testApp = "/usr/share/applications/duplicate.desktop";
    m_appWhitelist->AddApp(testApp);

    // 测试添加重复应用
    bool result = m_appWhitelist->AddApp(testApp);

    QVERIFY(!result); // 应返回false表示添加失败
    QCOMPARE(m_mockConfigManager->appWhitelist().size(), 1); // 白名单大小不变
}

void TestAppWhitelist::test_RemoveNonExistentApp()
{
    // 准备测试数据
    m_mockConfigManager->clearWhitelist();
    m_mockConfigManager->setWhitelist({"/usr/share/applications/existing.desktop"});

    // 测试移除不存在的应用
    QString nonExistentApp = "/usr/share/applications/nonexistent.desktop";
    bool result = m_appWhitelist->RemoveApp(nonExistentApp);

    QVERIFY(!result); // 应返回false表示移除失败
    QCOMPARE(m_mockConfigManager->appWhitelist().size(), 1); // 白名单大小不变
}

void TestAppWhitelist::test_EmptyWhitelist()
{
    // 准备测试数据
    m_mockConfigManager->clearWhitelist();

    // 测试空白名单
    QStringList emptyList = m_appWhitelist->AppList();

    QVERIFY(emptyList.isEmpty());

    // 测试从空白名单中移除应用
    bool result = m_appWhitelist->RemoveApp("/usr/share/applications/anyapp.desktop");
    QVERIFY(!result); // 应返回false表示移除失败
}

QTEST_APPLESS_MAIN(TestAppWhitelist)
#include "tst_appwhitelist.moc"