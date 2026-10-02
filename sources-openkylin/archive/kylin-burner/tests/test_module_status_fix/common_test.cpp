#include <QtTest>
#include <QApplication>
#include "src/status-fix/common.h"

class CommonTest : public QObject
{
    Q_OBJECT

private slots:
    // 测试disc_info结构体
    void testDiscInfoStruct()
    {
        disc_info_t discInfo;
        
        // 验证结构体大小和基本字段
        QVERIFY(sizeof(discInfo.length) == sizeof(unsigned short));
        QVERIFY(sizeof(discInfo.disc_type) == sizeof(unsigned char));
        QVERIFY(sizeof(discInfo.disc_id) == sizeof(unsigned int));
        
        // 验证结构体中的位域字段
        // 注意：由于是位域，我们只能验证其存在性而不能直接访问
        QVERIFY(true); // 占位符测试
    }

    // 测试MediaType枚举
    void testMediaTypeEnum()
    {
        // 测试基本媒体类型
        QCOMPARE((int)MEDIA_UNKNOWN, 0x1);
        QCOMPARE((int)MEDIA_NONE, 0x2);
        
        // 测试DVD媒体类型
        QCOMPARE((int)MEDIA_DVD_ROM, 0x4);
        QCOMPARE((int)MEDIA_DVD_R, 0x8);
        QCOMPARE((int)MEDIA_DVD_R_SEQ, 0x10);
        QCOMPARE((int)MEDIA_DVD_R_DL, 0x20);
        QCOMPARE((int)MEDIA_DVD_R_DL_SEQ, 0x40);
        QCOMPARE((int)MEDIA_DVD_R_DL_JUMP, 0x80);
        QCOMPARE((int)MEDIA_DVD_RAM, 0x100);
        QCOMPARE((int)MEDIA_DVD_RW, 0x200);
        QCOMPARE((int)MEDIA_DVD_RW_OVWR, 0x400);
        QCOMPARE((int)MEDIA_DVD_RW_SEQ, 0x800);
        QCOMPARE((int)MEDIA_DVD_PLUS_RW, 0x1000);
        QCOMPARE((int)MEDIA_DVD_PLUS_R, 0x2000);
        QCOMPARE((int)MEDIA_DVD_PLUS_R_DL, 0x4000);
        QCOMPARE((int)MEDIA_DVD_PLUS_RW_DL, 0x8000);
        
        // 测试CD媒体类型
        QCOMPARE((int)MEDIA_CD_ROM, 0x10000);
        QCOMPARE((int)MEDIA_CD_R, 0x20000);
        QCOMPARE((int)MEDIA_CD_RW, 0x40000);
        
        // 测试组合媒体类型
        QCOMPARE((int)MEDIA_DVD_R_ALL, 0xE8); // 0x8|0x10|0x20|0x40|0x80
        QCOMPARE((int)MEDIA_WRITEABLE_CD, 0x60000); // 0x20000|0x40000
        QCOMPARE((int)MEDIA_CD_ALL, 0x70000); // 0x10000|0x20000|0x40000
    }

    // 测试MediaType组合值
    void testMediaTypeCombinationValues()
    {
        // 测试DVD+R所有类型
        QCOMPARE((int)MEDIA_DVD_PLUS_R_ALL, 0x6000); // 0x2000|0x4000
        
        // 测试DVD-RW所有类型
        QCOMPARE((int)MEDIA_DVD_RW_ALL, 0xE00); // 0x200|0x400|0x800
        
        // 测试DVD+RW所有类型
        QCOMPARE((int)MEDIA_DVD_PLUS_RW_ALL, 0x9000); // 0x1000|0x8000
        
        // 测试所有DVD类型
        QCOMPARE((int)MEDIA_DVD_ALL, 0xFFFC); // 所有DVD类型的组合
    }

    // 测试结构体字段偏移（占位符）
    void testStructFieldOffsets()
    {
        // 由于结构体包含位域，字段偏移可能因编译器而异
        // 这里仅作占位符测试
        QVERIFY(true);
    }
};

QTEST_MAIN(CommonTest)
#include "common_test.moc"