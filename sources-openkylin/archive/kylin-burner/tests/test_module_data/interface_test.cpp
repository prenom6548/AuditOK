#include "lib/ky-udf-image/include/interface.h"
#include <QtTest/QtTest>

class InterfaceTest : public QObject
{
    Q_OBJECT

private slots:
    // 测试init_udf102_image函数声明
    void testInitUdf102ImageFunction() {
        // 只测试函数声明，不实际调用
        int (*func_ptr)(std::string, std::string, std::string) = init_udf102_image;
        
        QVERIFY(true); // 如果能编译通过则测试通过
    }

    // 测试build_append_udf102_image函数声明
    void testBuildAppendUdf102ImageFunction() {
        // 只测试函数声明，不实际调用
        int (*func_ptr)(std::string, int, std::string, std::string, std::string) = build_append_udf102_image;
        
        QVERIFY(true); // 如果能编译通过则测试通过
    }
};

QTEST_MAIN(InterfaceTest)
#include "interface_test.moc"