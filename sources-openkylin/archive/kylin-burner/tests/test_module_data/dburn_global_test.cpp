#include "lib/ky-burn-engine/include/dburn_global.h"
#include <QtTest/QtTest>

class DBurnGlobalTest : public QObject
{
    Q_OBJECT

private slots:
    // 测试BurnOption枚举
    void testBurnOptionEnum() {
        // 测试各个枚举值
        QVERIFY(DFMBURN::BurnOption::kKeepAppendable == 1);
        QVERIFY(DFMBURN::BurnOption::kVerifyDatas == 2);
        QVERIFY(DFMBURN::BurnOption::kEjectDisc == 4);
        QVERIFY(DFMBURN::BurnOption::kISO9660Only == 8);
        QVERIFY(DFMBURN::BurnOption::kJolietSupport == 16);
        QVERIFY(DFMBURN::BurnOption::kRockRidgeSupport == 32);
        QVERIFY(DFMBURN::BurnOption::kUDF102Supported == 64);
        QVERIFY(DFMBURN::BurnOption::kJolietAndRockRidge == 48); // 16 | 32
    }

    // 测试MediaType枚举
    void testMediaTypeEnum() {
        // 测试各个枚举值
        QVERIFY(DFMBURN::MediaType::kNoMedia == 0);
        QVERIFY(DFMBURN::MediaType::kCD_ROM == 1);
        QVERIFY(DFMBURN::MediaType::kCD_R == 2);
        QVERIFY(DFMBURN::MediaType::kCD_RW == 3);
        QVERIFY(DFMBURN::MediaType::kDVD_ROM == 4);
        QVERIFY(DFMBURN::MediaType::kDVD_R == 5);
        QVERIFY(DFMBURN::MediaType::kDVD_RW == 6);
        QVERIFY(DFMBURN::MediaType::kDVD_PLUS_R == 7);
        QVERIFY(DFMBURN::MediaType::kDVD_PLUS_R_DL == 8);
        QVERIFY(DFMBURN::MediaType::kDVD_RAM == 9);
        QVERIFY(DFMBURN::MediaType::kDVD_PLUS_RW == 10);
        QVERIFY(DFMBURN::MediaType::kBD_ROM == 11);
        QVERIFY(DFMBURN::MediaType::kBD_R == 12);
        QVERIFY(DFMBURN::MediaType::kBD_RE == 13);
    }

    // 测试JobStatus枚举
    void testJobStatusEnum() {
        // 测试各个枚举值
        QVERIFY(DFMBURN::JobStatus::kFailed == -1);
        QVERIFY(DFMBURN::JobStatus::kIdle == 0);
        QVERIFY(DFMBURN::JobStatus::kRunning == 1);
        QVERIFY(DFMBURN::JobStatus::kStalled == 2);
        QVERIFY(DFMBURN::JobStatus::kFinished == 3);
    }
};

QTEST_MAIN(DBurnGlobalTest)
#include "dburn_global_test.moc"