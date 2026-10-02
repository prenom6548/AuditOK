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

#include "core/applauncher.h"
#include "core/applaunchmanager.h"
#include <QtCore/QString>
#include <QtTest>
#include <vector>

class MockAppLauncher : public AppLauncher
{
public:
    MockAppLauncher()
        : AppLauncher(
            [](const std::string &, const std::vector<std::string> &, int) {},
            AppInfoGetter(
                [](const std::string &, const std::vector<std::string> &) -> AppInfo {
        return AppInfo();
    },
                [](const std::string &) -> AppInfo {
        return AppInfo();
    }))
    {
    }

    // 重写方法以便跟踪调用
    bool launchApp(const std::string &desktopFile)
    {
        m_launchedApps.push_back(desktopFile);
        return m_mockReturnValue;
    }

    bool launchAppWithArguments(
        const std::string &desktopFile, const std::vector<std::string> &args)
    {
        m_launchedAppsWithArgs.push_back({desktopFile, args});
        return m_mockReturnValue;
    }

    bool runCommand(const std::string &command)
    {
        m_runCommands.push_back(command);
        return m_mockReturnValue;
    }

    // 用于测试的辅助方法
    void setMockReturnValue(bool value)
    {
        m_mockReturnValue = value;
    }

    const std::vector<std::string> &getLaunchedApps() const
    {
        return m_launchedApps;
    }

    const std::vector<std::pair<std::string, std::vector<std::string>>> &getLaunchedAppsWithArgs() const
    {
        return m_launchedAppsWithArgs;
    }

    const std::vector<std::string> &getRunCommands() const
    {
        return m_runCommands;
    }

    void reset()
    {
        m_launchedApps.clear();
        m_launchedAppsWithArgs.clear();
        m_runCommands.clear();
    }

private:
    bool m_mockReturnValue = true;
    std::vector<std::string> m_launchedApps;
    std::vector<std::pair<std::string, std::vector<std::string>>> m_launchedAppsWithArgs;
    std::vector<std::string> m_runCommands;
};

class TestAppLaunchManager : public QObject
{
    Q_OBJECT

public:
    TestAppLaunchManager();
    ~TestAppLaunchManager();

private Q_SLOTS:
    void test_launchAppByDesktopFile();
    void test_launchAppByDesktopFileWithArgs();
    void test_launchAppByCommand();
    void test_launchApp();
    void test_errorHandling();
    void test_singleInstanceMode();

private:
    // 辅助方法
    void setupMockAppLauncher(AppLaunchManager &manager);
};

TestAppLaunchManager::TestAppLaunchManager()
{
}

TestAppLaunchManager::~TestAppLaunchManager()
{
}

void TestAppLaunchManager::setupMockAppLauncher(AppLaunchManager &manager)
{
    // 创建一个MockAppLauncher并将其设置为AppLaunchManager的内部AppLauncher
    auto mockLauncher = new MockAppLauncher();
    manager.setAppLauncher(std::unique_ptr<AppLauncher>(mockLauncher));
}

void TestAppLaunchManager::test_launchAppByDesktopFile()
{
    AppLaunchManager manager;
    auto mockLauncher = new MockAppLauncher();
    manager.setAppLauncher(std::unique_ptr<AppLauncher>(mockLauncher));

    // 测试通过desktop文件启动应用
    std::string testDesktopFile = "/usr/share/applications/test.desktop";
    bool result = manager.launchAppByDesktopFile(testDesktopFile);

    QVERIFY(result);
    QCOMPARE(mockLauncher->getLaunchedApps().size(), 1u);
    QCOMPARE(mockLauncher->getLaunchedApps()[0], testDesktopFile);

    // 测试启动失败的情况
    mockLauncher->reset();
    mockLauncher->setMockReturnValue(false);
    result = manager.launchAppByDesktopFile(testDesktopFile);

    QVERIFY(!result);
    QCOMPARE(mockLauncher->getLaunchedApps().size(), 1u);
}

void TestAppLaunchManager::test_launchAppByDesktopFileWithArgs()
{
    AppLaunchManager manager;
    auto mockLauncher = new MockAppLauncher();
    manager.setAppLauncher(std::unique_ptr<AppLauncher>(mockLauncher));

    // 测试带参数启动应用
    std::string testDesktopFile = "/usr/share/applications/test.desktop";
    std::vector<std::string> testArgs = {"--arg1", "--arg2=value"};
    bool result = manager.launchAppByDesktopFile(testDesktopFile, testArgs);

    QVERIFY(result);
    QCOMPARE(mockLauncher->getLaunchedAppsWithArgs().size(), 1u);
    QCOMPARE(mockLauncher->getLaunchedAppsWithArgs()[0].first, testDesktopFile);
    QCOMPARE(mockLauncher->getLaunchedAppsWithArgs()[0].second, testArgs);

    // 测试启动失败的情况
    mockLauncher->reset();
    mockLauncher->setMockReturnValue(false);
    result = manager.launchAppByDesktopFile(testDesktopFile, testArgs);

    QVERIFY(!result);
    QCOMPARE(mockLauncher->getLaunchedAppsWithArgs().size(), 1u);
}

void TestAppLaunchManager::test_launchAppByCommand()
{
    AppLaunchManager manager;
    auto mockLauncher = new MockAppLauncher();
    manager.setAppLauncher(std::unique_ptr<AppLauncher>(mockLauncher));

    // 测试通过命令启动应用
    std::string testCommand = "test-command --arg";
    bool result = manager.launchAppByCommand(testCommand);

    QVERIFY(result);
    QCOMPARE(mockLauncher->getRunCommands().size(), 1u);
    QCOMPARE(mockLauncher->getRunCommands()[0], testCommand);

    // 测试启动失败的情况
    mockLauncher->reset();
    mockLauncher->setMockReturnValue(false);
    result = manager.launchAppByCommand(testCommand);

    QVERIFY(!result);
    QCOMPARE(mockLauncher->getRunCommands().size(), 1u);
}

void TestAppLaunchManager::test_launchApp()
{
    AppLaunchManager manager;
    auto mockLauncher = new MockAppLauncher();
    manager.setAppLauncher(std::unique_ptr<AppLauncher>(mockLauncher));

    // 测试通用启动接口 - desktop文件路径
    std::string testDesktopFile = "/usr/share/applications/test.desktop";
    bool result = manager.launchApp(testDesktopFile);

    QVERIFY(result);
    QCOMPARE(mockLauncher->getLaunchedApps().size(), 1u);
    QCOMPARE(mockLauncher->getLaunchedApps()[0], testDesktopFile);

    // 测试通用启动接口 - 命令
    mockLauncher->reset();
    std::string testCommand = "test-command";
    result = manager.launchApp(testCommand);

    QVERIFY(result);
    QCOMPARE(mockLauncher->getRunCommands().size(), 1u);
    QCOMPARE(mockLauncher->getRunCommands()[0], testCommand);

    // 测试启动失败的情况
    mockLauncher->reset();
    mockLauncher->setMockReturnValue(false);
    result = manager.launchApp(testDesktopFile);

    QVERIFY(!result);
}

void TestAppLaunchManager::test_errorHandling()
{
    AppLaunchManager manager;
    auto mockLauncher = new MockAppLauncher();
    manager.setAppLauncher(std::unique_ptr<AppLauncher>(mockLauncher));

    // 设置启动失败
    mockLauncher->setMockReturnValue(false);

    // 启动应用
    bool result = manager.launchAppByDesktopFile("invalid.desktop");

    // 验证错误处理
    QVERIFY(!result);
    // 检查错误信息 (如果AppLaunchManager有错误报告机制)
    // QCOMPARE(manager.getLastError(), expectedError);
}

void TestAppLaunchManager::test_singleInstanceMode()
{
    AppLaunchManager manager;
    auto mockLauncher = new MockAppLauncher();
    manager.setAppLauncher(std::unique_ptr<AppLauncher>(mockLauncher));

    // 测试单实例模式设置
    manager.setSingleInstanceMode(true);
    // 验证AppLauncher的设置是否被正确传递
    // 需要添加相应的访问方法来检查

    // 测试启动应用在单实例模式下的行为
    // 根据AppLaunchManager的实际实现添加相应的测试
}

QTEST_APPLESS_MAIN(TestAppLaunchManager)
#include "tst_applaunchmanager.moc"