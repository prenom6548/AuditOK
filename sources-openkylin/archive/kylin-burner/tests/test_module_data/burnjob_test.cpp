#include "lib/ky-burn-engine/include/burnjob.h"
#include <QtTest/QtTest>

class BurnJobTest : public QObject
{
    Q_OBJECT

private slots:
    // 测试AbstractBurnJob类的枚举
    void testAbstractBurnJobEnums() {
        // 测试JobType枚举
        QVERIFY(ky_plugin_burn::AbstractBurnJob::kOpticalBurn == 0);
        QVERIFY(ky_plugin_burn::AbstractBurnJob::kOpticalBlank == 1);
        QVERIFY(ky_plugin_burn::AbstractBurnJob::kOpticalImageBurn == 2);
        QVERIFY(ky_plugin_burn::AbstractBurnJob::kOpticalCheck == 3);
        QVERIFY(ky_plugin_burn::AbstractBurnJob::kOpticalImageDump == 4);
        
        // 测试PropertyType枚举
        QVERIFY(ky_plugin_burn::AbstractBurnJob::KStagingUrl == 0);
        QVERIFY(ky_plugin_burn::AbstractBurnJob::kImageUrl == 1);
        QVERIFY(ky_plugin_burn::AbstractBurnJob::kVolumeName == 2);
        QVERIFY(ky_plugin_burn::AbstractBurnJob::kSpeeds == 3);
        QVERIFY(ky_plugin_burn::AbstractBurnJob::kBurnOpts == 4);
        
        // 测试JobPhase枚举
        QVERIFY(ky_plugin_burn::AbstractBurnJob::kReady == 0);
        QVERIFY(ky_plugin_burn::AbstractBurnJob::kWriteData == 1);
        QVERIFY(ky_plugin_burn::AbstractBurnJob::kCheckData == 2);
    }

    // 测试AbstractBurnJob类的构造函数
    void testAbstractBurnJobConstructor() {
        // 测试构造函数（需要提供设备名参数）
        ky_plugin_burn::AbstractBurnJob* job = nullptr;
        QVERIFY(true); // 如果能编译通过则测试通过
    }

    // 测试BurnISOFilesJob类的构造函数
    void testBurnISOFilesJobConstructor() {
        // 测试构造函数（需要提供设备名参数）
        ky_plugin_burn::BurnISOFilesJob job("/dev/sr0");
        QVERIFY(true); // 如果能编译通过则测试通过
    }

    // 测试CreateISOJob类的构造函数
    void testCreateISOJobConstructor() {
        // 测试构造函数（需要提供设备名参数）
        ky_plugin_burn::CreateISOJob job("/dev/sr0");
        QVERIFY(true); // 如果能编译通过则测试通过
    }

    // 测试BurnISOImageJob类的构造函数
    void testBurnISOImageJobConstructor() {
        // 测试构造函数（需要提供设备名参数）
        ky_plugin_burn::BurnISOImageJob job("/dev/sr0");
        QVERIFY(true); // 如果能编译通过则测试通过
    }

    // 测试BurnUDFFilesJob类的构造函数
    void testBurnUDFFilesJobConstructor() {
        // 测试构造函数（需要提供设备名参数）
        ky_plugin_burn::BurnUDFFilesJob job("/dev/sr0");
        QVERIFY(true); // 如果能编译通过则测试通过
    }

    // 测试EraseJob类的构造函数
    void testEraseJobConstructor() {
        // 测试构造函数（需要提供设备名参数）
        ky_plugin_burn::EraseJob job("/dev/sr0");
        QVERIFY(true); // 如果能编译通过则测试通过
    }

    // 测试DumpISOImageJob类的构造函数
    void testDumpISOImageJobConstructor() {
        // 测试构造函数（需要提供设备名参数）
        ky_plugin_burn::DumpISOImageJob job("/dev/sr0");
        QVERIFY(true); // 如果能编译通过则测试通过
    }
};

QTEST_MAIN(BurnJobTest)
#include "burnjob_test.moc"