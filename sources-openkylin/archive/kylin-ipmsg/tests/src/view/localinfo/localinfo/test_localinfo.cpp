/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 */

#include "test_localinfo.h"
#include <QSignalSpy>
#include <QTimer>
#include <QDir>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QFile>
#include <QTextStream>
#include <QApplication>

TestLocalInfo::TestLocalInfo(QObject *parent)
    : QObject(parent)
    , m_app(nullptr)
    , m_localInfo(nullptr)
{
}

void TestLocalInfo::initTestCase()
{
    // 初始化QApplication
    if (qApp == nullptr) {
        m_app = new QApplication(argc, argv);
    }
    
    // 创建LocalInfo实例
    m_localInfo = LocalInfo::getInstance();
    QVERIFY(m_localInfo != nullptr);
    
    // 确保组件已初始化
    m_localInfo->setWidgetUi();
}

void TestLocalInfo::cleanupTestCase()
{
    // 清理资源
    if (m_app) {
        delete m_app;
        m_app = nullptr;
    }
}

void TestLocalInfo::testConstructor()
{
    // 测试构造函数
    LocalInfo *localInfo = new LocalInfo();
    QVERIFY(localInfo != nullptr);
    delete localInfo;
}

void TestLocalInfo::testGetInstance()
{
    // 测试单例模式
    LocalInfo *instance1 = LocalInfo::getInstance();
    LocalInfo *instance2 = LocalInfo::getInstance();
    QVERIFY(instance1 != nullptr);
    QVERIFY(instance2 != nullptr);
    QCOMPARE(instance1, instance2); // 应该是同一个实例
}

void TestLocalInfo::testSetWidgetUi()
{
    // 测试初始化组件函数
    QVERIFY(m_localInfo != nullptr);
    // 调用函数（不会返回值，主要检查不崩溃）
    m_localInfo->setWidgetUi();
    // 验证组件已创建
    QVERIFY(m_localInfo->findChild<QLabel*>("usericon") != nullptr);
    QVERIFY(m_localInfo->findChild<QLabel*>("username") != nullptr);
    QVERIFY(m_localInfo->findChild<QLabel*>("userip") != nullptr);
}

void TestLocalInfo::testSetWidgetStyle()
{
    // 测试设置组件样式函数
    QVERIFY(m_localInfo != nullptr);
    // 调用函数（不会返回值，主要检查不崩溃）
    m_localInfo->setWidgetStyle();
}

void TestLocalInfo::testShowList()
{
    // 测试显示列表函数
    QVERIFY(m_localInfo != nullptr);
    // 调用函数（不会返回值，主要检查不崩溃）
    m_localInfo->showList();
}

void TestLocalInfo::testChangeTheme()
{
    // 测试主题切换函数
    QVERIFY(m_localInfo != nullptr);
    // 调用函数（不会返回值，主要检查不崩溃）
    m_localInfo->changeTheme();
}

void TestLocalInfo::testDisplayInfo()
{
    // 测试显示信息函数
    QVERIFY(m_localInfo != nullptr);
    
    // 创建信号监视器
    QSignalSpy spy(m_localInfo, SIGNAL(changeLocalName(bool)));
    
    // 测试正常情况
    QString nickname = "TestUser";
    QString ip = "192.168.1.100";
    m_localInfo->displayInfo(nickname, ip);
    
    // 验证信号是否发出（等待一段时间让信号发出）
    QTRY_VERIFY_WITH_TIMEOUT(spy.count() >= 1, 1000);
}

void TestLocalInfo::testGetIniText()
{
    // 测试获取INI文本函数
    QVERIFY(m_localInfo != nullptr);
    // 调用函数（不会返回值，主要检查不崩溃）
    m_localInfo->getIniText();
}

void TestLocalInfo::testCheckLocalName()
{
    // 测试检查本地名称函数
    QVERIFY(m_localInfo != nullptr);
    // 调用函数（不会返回值，主要检查不崩溃）
    m_localInfo->checkLocalName();
}

void TestLocalInfo::testGetLocalAvatar()
{
    // 测试获取本地头像函数
    QVERIFY(m_localInfo != nullptr);
    QString avatar = m_localInfo->getLocalAvatar();
    // 不做具体值验证，因为可能为空，主要检查不崩溃
    Q_UNUSED(avatar);
}

void TestLocalInfo::testChangeFontSize()
{
    // 测试改变字体大小函数
    QVERIFY(m_localInfo != nullptr);
    // 调用函数（不会返回值，主要检查不崩溃）
    m_localInfo->changeFontSize();
}

void TestLocalInfo::testSlotChangePCSize()
{
    // 测试改变PC尺寸槽函数
    QVERIFY(m_localInfo != nullptr);
    // 调用函数（不会返回值，主要检查不崩溃）
    m_localInfo->slotChangePCSize();
}

void TestLocalInfo::testSlotChangeFlatHSize()
{
    // 测试改变水平尺寸槽函数
    QVERIFY(m_localInfo != nullptr);
    // 调用函数（不会返回值，主要检查不崩溃）
    m_localInfo->slotChangeFlatHSize();
}

void TestLocalInfo::testSlotChangeFlatVSize()
{
    // 测试改变垂直尺寸槽函数
    QVERIFY(m_localInfo != nullptr);
    // 调用函数（不会返回值，主要检查不崩溃）
    m_localInfo->slotChangeFlatVSize();
}

void TestLocalInfo::testPaintEvent()
{
    // 测试绘制事件函数
    QVERIFY(m_localInfo != nullptr);
    // 调用函数（不会返回值，主要检查不崩溃）
    // 注意：paintEvent通常由Qt框架调用，我们在这里只是确保它不会崩溃
    QPaintEvent event(m_localInfo->rect());
    m_localInfo->paintEvent(&event);
}

void TestLocalInfo::testEvent()
{
    // 测试事件处理函数
    QVERIFY(m_localInfo != nullptr);
    // 调用函数（不会返回值，主要检查不崩溃）
    // 注意：event通常由Qt框架调用，我们在这里只是确保它不会崩溃
    QEvent event(QEvent::WindowActivate);
    m_localInfo->event(&event);
}

void TestLocalInfo::testEventFilter()
{
    // 测试事件过滤器函数
    QVERIFY(m_localInfo != nullptr);
    // 调用函数（不会返回值，主要检查不崩溃）
    QEvent event(QEvent::None);
    m_localInfo->eventFilter(m_localInfo, &event);
}

void TestLocalInfo::testPrivateSlots()
{
    QVERIFY(m_localInfo != nullptr);
    
    // 测试 slotUpdateName
    m_localInfo->slotUpdateName();
    QVERIFY(m_localInfo->m_updateNameWid != nullptr);
    
    // 测试 slotChangeFont
    m_localInfo->slotChangeFont();
    
    // 测试 setBackgroundColor
    m_localInfo->setBackgroundColor();
    
    // 测试 displayInfo 的不同分支
    m_localInfo->displayInfo("AVeryLongNicknameThatWillBeElided", "127.0.0.1");
    m_localInfo->displayInfo("", "127.0.0.1");

    // 测试 changeFontSize
    m_localInfo->changeFontSize();

    // 测试 slotChangePCSize, slotChangeFlatHSize, slotChangeFlatVSize
    m_localInfo->slotChangePCSize();
    m_localInfo->slotChangeFlatHSize();
    m_localInfo->slotChangeFlatVSize();
}

void TestLocalInfo::testEventFilters()
{
    QVERIFY(m_localInfo != nullptr);
    
    // 测试 eventFilter
    QEvent event(QEvent::MouseButtonPress);
    m_localInfo->eventFilter(m_localInfo->m_updateNameBtn, &event);
    m_localInfo->eventFilter(m_localInfo->m_openFilePathBtn, &event);
    m_localInfo->eventFilter(m_localInfo->m_searchLineEdit, &event);
}

void TestLocalInfo::testCheckLocalNameTimer()
{
    QVERIFY(m_localInfo != nullptr);
    
    // 模拟昵称位空的情况
    // IniSettings::getInstance()->setLocalNickname("");
    m_localInfo->checkLocalName();
    
    // 等待定时器触发
    QTest::qWait(200);
}

void TestLocalInfo::testCoverageAdditions()
{
    QVERIFY(m_localInfo != nullptr);

    // 1. 覆盖 141-146 (CurrentMode::HMode 和 CurrentMode::VMode)
    // 通过调用 slotChangeFlatHSize 和 slotChangeFlatVSize 来触发
    m_localInfo->slotChangeFlatHSize();
    QVERIFY(m_localInfo->m_nameWidth > 0);
    
    m_localInfo->slotChangeFlatVSize();
    QVERIFY(m_localInfo->m_nameWidth > 0);

    // 2. 覆盖 277-278 (setBackgroundColor 中的透明度设置)
    // 模拟系统透明度变化
    // 注意：setBackgroundColor 内部会检查 GsettingMonitor 的透明度设置
    m_localInfo->setBackgroundColor();

    // 3. 覆盖 306-323 (slotOpenFilePath)
    // 为了避免真正打开文件夹阻塞测试，我们可以通过 QTimer::singleShot 来处理可能弹出的对话框（如果有的话）
    // 这里的 QDesktopServices::openUrl 在单测环境下通常不会阻塞，但为了安全可以加保护
    m_localInfo->slotOpenFilePath();

    // 4. 覆盖 401-402 (displayInfo 中的长昵称省略)
    // 提供一个非常长的昵称，确保触发 elidedText
    QString longNickname = "ThisIsAnExtremelyLongNicknameThatShouldDefinitelyBeElidedByTheFontMetricsToEnsureCoverageOfTheRelevantLinesInLocalInfoCpp";
    m_localInfo->displayInfo(longNickname, "127.0.0.1");
    
    // 验证 tooltip 是否被设置
    QVERIFY(!m_localInfo->m_nicknameLab->toolTip().isEmpty());
}
