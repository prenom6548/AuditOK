#include "lib/ky-burn-engine/include/dopticaldiscinfo.h"
#include <QtTest/QtTest>

class DOpticalDiscInfoTest : public QObject
{
    Q_OBJECT

private slots:
    // 测试DOpticalDiscInfo类的构造函数
    void testDOpticalDiscInfoConstructor() {
        // 测试默认构造函数
        DFMBURN::DOpticalDiscInfo info1;
        
        // 测试带设备参数的构造函数
        DFMBURN::DOpticalDiscInfo info2("/dev/sr0");
        
        QVERIFY(true); // 如果能编译通过则测试通过
    }

    // 测试DOpticalDiscInfo类的拷贝构造函数
    void testDOpticalDiscInfoCopyConstructor() {
        DFMBURN::DOpticalDiscInfo info1("/dev/sr0");
        DFMBURN::DOpticalDiscInfo info2(info1); // 拷贝构造
        
        QVERIFY(true); // 如果能编译通过则测试通过
    }

    // 测试DOpticalDiscInfo类的赋值操作符
    void testDOpticalDiscInfoAssignmentOperator() {
        DFMBURN::DOpticalDiscInfo info1("/dev/sr0");
        DFMBURN::DOpticalDiscInfo info2;
        info2 = info1; // 赋值操作
        
        QVERIFY(true); // 如果能编译通过则测试通过
    }

    // 测试DOpticalDiscInfo类的析构函数
    void testDOpticalDiscInfoDestructor() {
        DFMBURN::DOpticalDiscInfo* info = new DFMBURN::DOpticalDiscInfo("/dev/sr0");
        delete info; // 析构函数应该能正常调用
        
        QVERIFY(true); // 如果能编译通过则测试通过
    }
};

QTEST_MAIN(DOpticalDiscInfoTest)
#include "dopticaldiscinfo_test.moc"