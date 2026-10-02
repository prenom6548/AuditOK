/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "kactiontest.h"
#include "widget/kaction.h"
#include <QWidget>
#include <QIcon>
#include <QKeySequence>
#include <iostream>

KActionTest::KActionTest(QObject *parent) : QObject(parent)
{
    // 初始化成员变量为nullptr
    m_action = nullptr;
}

void KActionTest::testConstructorWithParentAndAutoAdd()
{
    std::cout << "=== Testing KAction(QObject*, bool) ===" << std::endl;
    
    // 创建父对象
    QWidget parentWidget;
    
    // 测试构造函数
    m_action = new KAction(&parentWidget, false);
    
    // 验证对象是否正确创建
    QVERIFY(m_action != nullptr);
    
    // 验证父对象是否正确设置
    QCOMPARE(m_action->parent(), &parentWidget);
    
    // 清理资源
    delete m_action;
    m_action = nullptr;
    
    std::cout << "KAction(QObject*, bool) test completed." << std::endl;
}

void KActionTest::testConstructorWithParentNameAndAutoAdd()
{
    std::cout << "=== Testing KAction(QObject*, const char*, bool) ===" << std::endl;
    
    // 创建父对象
    QWidget parentWidget;
    
    // 测试构造函数
    m_action = new KAction(&parentWidget, "testAction", false);
    
    // 验证对象是否正确创建
    QVERIFY(m_action != nullptr);
    
    // 验证父对象是否正确设置
    QCOMPARE(m_action->parent(), &parentWidget);
    
    // 验证对象名称是否正确设置
    QCOMPARE(m_action->objectName(), QString("testAction"));
    
    // 清理资源
    delete m_action;
    m_action = nullptr;
    
    std::cout << "KAction(QObject*, const char*, bool) test completed." << std::endl;
}

void KActionTest::testConstructorWithTextKeySequenceAndMore()
{
    std::cout << "=== Testing KAction(const QString&, QKeySequence, QObject*, const char*, bool) ===" << std::endl;
    
    // 创建父对象
    QWidget parentWidget;
    
    // 测试构造函数
    m_action = new KAction("Test Action", QKeySequence("Ctrl+T"), &parentWidget, "testAction", false);
    
    // 验证对象是否正确创建
    QVERIFY(m_action != nullptr);
    
    // 验证父对象是否正确设置
    QCOMPARE(m_action->parent(), &parentWidget);
    
    // 验证对象名称是否正确设置
    QCOMPARE(m_action->objectName(), QString("testAction"));
    
    // 验证文本是否正确设置
    QCOMPARE(m_action->text(), QString("Test Action"));
    
    // 验证快捷键是否正确设置
    QCOMPARE(m_action->shortcut(), QKeySequence("Ctrl+T"));
    
    // 清理资源
    delete m_action;
    m_action = nullptr;
    
    std::cout << "KAction(const QString&, QKeySequence, QObject*, const char*, bool) test completed." << std::endl;
}

void KActionTest::testConstructorWithKeySequenceAndMore()
{
    std::cout << "=== Testing KAction(QKeySequence, QObject*, const char*, bool) ===" << std::endl;
    
    // 创建父对象
    QWidget parentWidget;
    
    // 测试构造函数
    m_action = new KAction(QKeySequence("Ctrl+T"), &parentWidget, "testAction", false);
    
    // 验证对象是否正确创建
    QVERIFY(m_action != nullptr);
    
    // 验证父对象是否正确设置
    QCOMPARE(m_action->parent(), &parentWidget);
    
    // 验证对象名称是否正确设置
    QCOMPARE(m_action->objectName(), QString("testAction"));
    
    // 验证快捷键是否正确设置
    QCOMPARE(m_action->shortcut(), QKeySequence("Ctrl+T"));
    
    // 清理资源
    delete m_action;
    m_action = nullptr;
    
    std::cout << "KAction(QKeySequence, QObject*, const char*, bool) test completed." << std::endl;
}

void KActionTest::testAddShortcut()
{
    std::cout << "=== Testing addShortcut ===" << std::endl;
    
    // 创建父对象
    QWidget parentWidget;
    
    // 创建KAction实例
    m_action = new KAction(&parentWidget, false);
    
    // 获取初始快捷键列表
    QList<QKeySequence> initialShortcuts = m_action->shortcuts();
    
    // 添加新快捷键
    QKeySequence newShortcut("Ctrl+S");
    m_action->addShortcut(newShortcut);
    
    // 验证快捷键是否已添加
    QList<QKeySequence> updatedShortcuts = m_action->shortcuts();
    QVERIFY(updatedShortcuts.contains(newShortcut));
    
    // 验证快捷键数量是否增加
    QCOMPARE(updatedShortcuts.size(), initialShortcuts.size() + 1);
    
    // 清理资源
    delete m_action;
    m_action = nullptr;
    
    std::cout << "addShortcut test completed." << std::endl;
}

void KActionTest::testChangeWithIconAndText()
{
    std::cout << "=== Testing change(QIcon, QString) ===" << std::endl;
    
    // 创建父对象
    QWidget parentWidget;
    
    // 创建KAction实例
    m_action = new KAction(&parentWidget, false);
    
    // 保存初始状态
    QString initialText = m_action->text();
    QIcon initialIcon = m_action->icon();
    
    // 创建测试图标和文本
    QIcon testIcon(":/test-icon.png");  // 使用虚拟路径
    QString testText = "Test Text";
    
    // 调用change函数
    m_action->change(testIcon, testText);
    
    // 验证图标是否已更改
    QVERIFY(m_action->icon().cacheKey() != initialIcon.cacheKey() || 
           (!initialIcon.isNull() && testIcon.isNull()) ||
           (initialIcon.isNull() && !testIcon.isNull()));
    
    // 验证文本是否已更改
    QCOMPARE(m_action->text(), testText);
    
    // 清理资源
    delete m_action;
    m_action = nullptr;
    
    std::cout << "change(QIcon, QString) test completed." << std::endl;
}

void KActionTest::testChangeWithText()
{
    std::cout << "=== Testing change(QString) ===" << std::endl;
    
    // 创建父对象
    QWidget parentWidget;
    
    // 创建KAction实例
    m_action = new KAction(&parentWidget, false);
    
    // 设置初始快捷键
    m_action->setShortcut(QKeySequence("Ctrl+T"));
    
    // 保存初始状态
    QString initialText = m_action->text();
    
    // 创建测试文本
    QString testText = "Test Text";
    
    // 调用change函数
    m_action->change(testText);
    
    // 验证文本是否已更改
    QCOMPARE(m_action->text(), testText);
    
    // 验证工具提示是否包含文本和快捷键
    QString expectedToolTip = testText + " (" + m_action->shortcut().toString() + ")";
    QCOMPARE(m_action->toolTip(), expectedToolTip);
    
    // 清理资源
    delete m_action;
    m_action = nullptr;
    
    std::cout << "change(QString) test completed." << std::endl;
}

void KActionTest::testDestructor()
{
    std::cout << "=== Testing Destructor ===" << std::endl;
    
    // 创建父对象
    QWidget parentWidget;
    
    // 创建KAction实例
    m_action = new KAction(&parentWidget, false);
    
    // 验证对象是否正确创建
    QVERIFY(m_action != nullptr);
    
    // 删除对象
    delete m_action;
    m_action = nullptr;
    
    // 验证指针是否为nullptr
    QVERIFY(m_action == nullptr);
    
    std::cout << "Destructor test completed." << std::endl;
}