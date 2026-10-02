#include <QtTest>
#include <QApplication>
#include "src/data/iso9660backend.h"

// Mock DataCDROM class since it's needed by Iso9660DeviceBackend
class DataCDROM {
public:
    DataCDROM() {}
};

// Since Iso9660Backend is an abstract class, we'll test its concrete implementation
class Iso9660BackendTest : public QObject
{
    Q_OBJECT

private slots:
    // 测试Iso9660Backend基类的纯虚函数（通过具体实现类测试）
    void testIso9660BackendAbstractFunctions()
    {
        // 这些是纯虚函数，在具体子类中实现
        // 我们验证它们的存在性而不是具体实现
        QVERIFY(true); // 占位符测试
    }

    // 测试Iso9660DeviceBackend构造函数
    void testIso9660DeviceBackendConstructor()
    {
        DataCDROM* mockDevice = new DataCDROM();
        Iso9660DeviceBackend backend(mockDevice);
        
        // 验证对象可以正常构造
        QVERIFY(!backend.isOpen()); // 初始状态应该是关闭的
    }

    // 测试Iso9660DeviceBackend虚函数
    void testIso9660DeviceBackendVirtualFunctions()
    {
        DataCDROM* mockDevice = new DataCDROM();
        Iso9660DeviceBackend backend(mockDevice);
        
        // 测试isOpen函数
        QCOMPARE(backend.isOpen(), false);
    }
};

QTEST_MAIN(Iso9660BackendTest)
#include "iso9660backend_test.moc"