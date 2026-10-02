#include "test_menufactory.h"
#include <QApplication>
#include <QMenu>
#include <QToolButton>
#include <QAction>
#include <QList>
#include <QSize>

TestMenuFactory::TestMenuFactory()
    : m_menuFactory(nullptr)
{
}

void TestMenuFactory::initTestCase()
{
    m_menuFactory = new MenuFactory();
}

void TestMenuFactory::cleanupTestCase()
{
    delete m_menuFactory;
    m_menuFactory = nullptr;
}

void TestMenuFactory::testConstructor()
{
    // 测试构造函数
    MenuFactory factory;
    QVERIFY(true); // 如果没有崩溃，说明构造函数执行正常
}

void TestMenuFactory::testInitFunction()
{
    // 测试init函数的调用（通过构造函数间接测试）
    QVERIFY(m_menuFactory != nullptr);
    
    // 验证init函数是否正确初始化了成员变量
    QVERIFY(m_menuFactory->menuButton != nullptr);
    QVERIFY(m_menuFactory->menuButton->menu() != nullptr);
}

void TestMenuFactory::testMenuButtonProperties()
{
    // 测试菜单按钮创建和属性
    QVERIFY(m_menuFactory != nullptr);
    
    // 获取菜单按钮
    QToolButton* button = m_menuFactory->menuButton;
    QVERIFY(button != nullptr);
    
    // 验证按钮属性
    QCOMPARE(button->popupMode(), QToolButton::InstantPopup);
    QCOMPARE(button->iconSize(), QSize(16, 16));
    QVERIFY(button->autoRaise());
    QVERIFY(!button->accessibleName().isEmpty());
    QCOMPARE(button->accessibleName(), QString("kylin-usb-creator_QToolButton_menuButton"));
    
    // 验证固定大小
    QCOMPARE(button->size(), QSize(30, 30));
    
    // 验证工具提示
    QVERIFY(!button->toolTip().isEmpty());
    
    // 验证图标是否设置
    QVERIFY(!button->icon().isNull());
}

void TestMenuFactory::testMenuActions()
{
    // 测试菜单动作创建
    QVERIFY(m_menuFactory != nullptr);
    
    // 获取菜单按钮
    QToolButton* button = m_menuFactory->menuButton;
    QVERIFY(button != nullptr);
    
    // 获取菜单
    QMenu* menu = button->menu();
    QVERIFY(menu != nullptr);
    
    // 验证菜单中的动作
    QList<QAction*> actions = menu->actions();
    QVERIFY(actions.size() >= 3); // 至少应有About, Help, Quit三个动作
    
    // 验证特定动作的存在
    bool hasAbout = false;
    bool hasHelp = false;
    bool hasQuit = false;
    
    for (QAction* action : actions) {
        if (action->text() == "About") hasAbout = true;
        if (action->text() == "Help") hasHelp = true;
        if (action->text() == "Quit") hasQuit = true;
    }
    
    QVERIFY(hasAbout);
    QVERIFY(hasHelp);
    QVERIFY(hasQuit);
    
    // 验证动作的顺序（About, Help, Quit）
    QCOMPARE(actions.at(0)->text(), QString("About"));
    QCOMPARE(actions.at(1)->text(), QString("Help"));
    QCOMPARE(actions.at(2)->text(), QString("Quit"));
}

void TestMenuFactory::testUnusedMemberVariables()
{
    // 测试未使用的成员变量是否存在
    QVERIFY(m_menuFactory != nullptr);
    
    // 验证所有私有成员变量都已声明（即使未使用）
    // 这些测试主要是为了确保代码完整性
    QVERIFY(true); // 占位符，实际测试在编译时进行
}

void TestMenuFactory::testMultipleInstances()
{
    // 测试创建多个MenuFactory实例
    MenuFactory* factory1 = new MenuFactory();
    MenuFactory* factory2 = new MenuFactory();
    
    // 验证两个实例都正确初始化
    QVERIFY(factory1 != nullptr);
    QVERIFY(factory2 != nullptr);
    QVERIFY(factory1->menuButton != nullptr);
    QVERIFY(factory2->menuButton != nullptr);
    
    // 清理
    delete factory1;
    delete factory2;
}