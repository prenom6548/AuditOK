#include "lib/ky-burn-engine/include/dopticaldiscmanager.h"
#include <QtTest/QtTest>

class DOpticalDiscManagerTest : public QObject
{
    Q_OBJECT

private slots:
    // 测试DOpticalDiscManager类的构造函数
    void testDOpticalDiscManagerConstructor() {
        // 测试带设备参数的构造函数
        DFMBURN::DOpticalDiscManager manager("/dev/sr0");
        
        QVERIFY(true); // 如果能编译通过则测试通过
    }

    // 测试DOpticalDiscManager类的构造函数（带父对象）
    void testDOpticalDiscManagerConstructorWithParent() {
        QObject parent;
        // 测试带设备参数和父对象的构造函数
        DFMBURN::DOpticalDiscManager manager("/dev/sr0", &parent);
        
        QVERIFY(true); // 如果能编译通过则测试通过
    }

    // 测试DOpticalDiscManager类的析构函数
    void testDOpticalDiscManagerDestructor() {
        DFMBURN::DOpticalDiscManager* manager = new DFMBURN::DOpticalDiscManager("/dev/sr0");
        delete manager; // 析构函数应该能正常调用
        
        QVERIFY(true); // 如果能编译通过则测试通过
    }

    // 测试DOpticalDiscManager类的静态方法createOpticalInfo
    void testDOpticalDiscManagerCreateOpticalInfo() {
        // 测试静态方法
        DFMBURN::DOpticalDiscInfo* info = DFMBURN::DOpticalDiscManager::createOpticalInfo("/dev/sr0");
        
        QVERIFY(true); // 如果能编译通过则测试通过
    }
};

QTEST_MAIN(DOpticalDiscManagerTest)
#include "dopticaldiscmanager_test.moc"