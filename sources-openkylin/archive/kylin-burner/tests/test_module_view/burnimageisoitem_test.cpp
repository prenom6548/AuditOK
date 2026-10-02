#include <QtTest>
#include "src/view/burnimageisoitem.h"

class BurnImageIsoItemTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    
    void testBurnImageIsoItemCreation();
    void testBurnImageIsoItemInitialization();
    void testBurnImageIsoItemMethods();
};

void BurnImageIsoItemTest::initTestCase()
{
    // 在所有测试开始前执行
}

void BurnImageIsoItemTest::cleanupTestCase()
{
    // 在所有测试结束后执行
}

void BurnImageIsoItemTest::init()
{
    // 在每个测试函数执行前执行
}

void BurnImageIsoItemTest::cleanup()
{
    // 在每个测试函数执行后执行
}

void BurnImageIsoItemTest::testBurnImageIsoItemCreation()
{
    BurnImageISOItem *item = new BurnImageISOItem("hello");
    QVERIFY(item != nullptr);
    QVERIFY(item->objectName() != "");
    delete item;
}

void BurnImageIsoItemTest::testBurnImageIsoItemInitialization()
{
    BurnImageISOItem *item = new BurnImageISOItem("hello");
    QVERIFY(item != nullptr);
    
    // 检查初始状态
    QVERIFY(item->isEnabled() == true);
    
    delete item;
}

void BurnImageIsoItemTest::testBurnImageIsoItemMethods()
{
    BurnImageISOItem *item = new BurnImageISOItem("hello");
    QVERIFY(item != nullptr);
    
    // 测试基本功能
    item->show();
    QVERIFY(item->isVisible() == true);
    
    item->hide();
    QVERIFY(item->isVisible() == false);
    
    delete item;
}

QTEST_MAIN(BurnImageIsoItemTest)

#include "burnimageisoitem_test.moc"
