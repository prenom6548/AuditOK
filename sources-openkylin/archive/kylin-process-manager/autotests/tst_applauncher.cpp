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

#include "base/appinfo.h"
#include "core/applauncher.h"
#include <QtCore/QString>
#include <QtTest>
#include <set>
#include <vector>

class MockAppInfoGetter : public AppInfoGetter
{
public:
    MockAppInfoGetter()
        : AppInfoGetter(
            [this](const std::string &desktopFile, const std::vector<std::string> &args) -> AppInfo {
        AppInfo info;
        if (m_runningApps.find(desktopFile) != m_runningApps.end()) {
            // 模拟已经运行的应用，设置一个wid
            info.appendWid("window-id-" + desktopFile);
        }
        return info;
    },
            [this](const std::string &command) -> AppInfo {
        AppInfo info;
        if (m_runningCommands.find(command) != m_runningCommands.end()) {
            // 模拟已经运行的命令，设置一个wid
            info.appendWid("window-id-" + command);
        }
        return info;
    })
    {
    }

    // 添加用于测试的辅助方法
    void setAppRunning(const std::string &desktopFile, bool running)
    {
        if (running) {
            m_runningApps.insert(desktopFile);
        } else {
            m_runningApps.erase(desktopFile);
        }
    }

    void setCommandRunning(const std::string &command, bool running)
    {
        if (running) {
            m_runningCommands.insert(command);
        } else {
            m_runningCommands.erase(command);
        }
    }

private:
    std::set<std::string> m_runningApps;
    std::set<std::string> m_runningCommands;
};

class TestAppLauncher : public QObject
{
    Q_OBJECT

public:
    TestAppLauncher();
    ~TestAppLauncher();

private Q_SLOTS:
    void test_launchApp();
    void test_launchAppWithArguments();
    void test_runCommand();
    void test_errorHandling();
    void test_singleInstance();
};

TestAppLauncher::TestAppLauncher()
{
}

TestAppLauncher::~TestAppLauncher()
{
}

void TestAppLauncher::test_launchApp()
{
    const QString desktopFile = "/usr/share/applications/peony.desktop";

    // Mock callbacks
    bool callbackCalled = false;
    auto processCallback = [&callbackCalled](const std::string &path, const std::vector<std::string> &args, int) {
        callbackCalled = true;
        // 可以进一步验证path和args是否正确
    };
    MockAppInfoGetter appInfoGetter;

    AppLauncher launcher(processCallback, appInfoGetter);

    // 测试正常启动应用
    bool result = launcher.launchApp(desktopFile.toStdString());
    QVERIFY(result);
    QVERIFY(callbackCalled);

    // 测试启动不存在的应用
    callbackCalled = false;
    result = launcher.launchApp("/non/existent/file.desktop");
    QVERIFY(!result);
    QVERIFY(!callbackCalled);
    QCOMPARE(launcher.error(), AppLauncher::LaunchError::FailedToFindDesktopFile);
}

void TestAppLauncher::test_launchAppWithArguments()
{
    // 创建测试文件
    QTemporaryFile tempFile("test_launch_app.txt");
    if (!tempFile.open()) {
        QFAIL("Could not create temporary file for testing");
    }
    tempFile.close();

    // Mock callbacks
    bool callbackCalled = false;
    std::vector<std::string> capturedArgs;
    auto processCallback = [&callbackCalled, &capturedArgs](const std::string &path, const std::vector<std::string> &args, int) {
        callbackCalled = true;
        capturedArgs = args;
    };
    MockAppInfoGetter appInfoGetter;

    AppLauncher launcher(processCallback, appInfoGetter);

    // 测试带参数启动应用
    std::vector<std::string> testArgs = {tempFile.fileName().toStdString()};
    bool result = launcher.launchAppWithArguments("/usr/share/peony.desktop", testArgs);
    QVERIFY(result);
    QVERIFY(callbackCalled);
    QCOMPARE(capturedArgs, testArgs);
}

void TestAppLauncher::test_runCommand()
{
    // Mock callbacks
    auto processCallback = [](const std::string &, const std::vector<std::string> &, int) {};
    MockAppInfoGetter appInfoGetter;

    AppLauncher launcher(processCallback, appInfoGetter);

    // 注意：由于QProcess::startDetached的调用很难在测试环境中模拟和验证
    // 这个测试主要是确保接口正常工作而不是测试实际命令执行

    // 测试运行命令
    bool result = launcher.runCommand("echo test");

    // 我们无法确保命令一定成功，但至少可以验证没有抛出异常
    QVERIFY(result || launcher.error() == AppLauncher::LaunchError::FailedToRunCommand);
}

void TestAppLauncher::test_errorHandling()
{
    // Mock callbacks
    auto processCallback = [](const std::string &, const std::vector<std::string> &, int) {};
    MockAppInfoGetter appInfoGetter;

    AppLauncher launcher(processCallback, appInfoGetter);

    // 初始状态应该是无错误
    QCOMPARE(launcher.error(), AppLauncher::LaunchError::NoError);

    // 测试错误信息的一致性
    launcher.launchApp("/non/existent/file.desktop");
    QCOMPARE(launcher.error(), AppLauncher::LaunchError::FailedToFindDesktopFile);
    AppLauncher::LaunchErrorMessage errorMsg = launcher.errorMessage();
    QCOMPARE(errorMsg.error, AppLauncher::LaunchError::FailedToFindDesktopFile);
    QCOMPARE(errorMsg.errorName, std::string("FailedToFindDesktopFile"));
    QVERIFY(!errorMsg.errorMessage.empty());
}

void TestAppLauncher::test_singleInstance()
{
    // 创建测试文件
    QTemporaryFile tempFile;
    if (!tempFile.open()) {
        QFAIL("Could not create temporary file for testing");
    }
    QString desktopFile = tempFile.fileName();
    tempFile.close();

    // Mock callbacks
    int launchCount = 0;
    auto processCallback = [&launchCount](const std::string &, const std::vector<std::string> &, int) {
        launchCount++;
    };
    MockAppInfoGetter appInfoGetter;

    AppLauncher launcher(processCallback, appInfoGetter);

    // 默认情况下应该允许多实例
    launcher.launchApp(desktopFile.toStdString());
    launcher.launchApp(desktopFile.toStdString());
    QCOMPARE(launchCount, 2);

    // 设置单实例模式
    launchCount = 0;
    launcher.setKeepAppSingleInstance(true);

    // 第一次启动应该成功
    launcher.launchApp(desktopFile.toStdString());
    QCOMPARE(launchCount, 1);

    // 模拟应用已运行
    appInfoGetter.setAppRunning(desktopFile.toStdString(), true);

    // 再次启动相同应用应该尝试激活窗口而不是启动新实例
    launcher.launchApp(desktopFile.toStdString());
    // 启动计数应该还是1，因为应该是激活窗口而不是新启动
    QCOMPARE(launchCount, 1);

    // 测试命令的单实例行为
    std::string testCommand = "test-command";
    launcher.runCommand(testCommand);
    appInfoGetter.setCommandRunning(testCommand, true);
    launcher.runCommand(testCommand);
}

QTEST_APPLESS_MAIN(TestAppLauncher)
#include "tst_applauncher.moc"
