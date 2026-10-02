#include "lib/ky-udf-image/include/ecma167.h"
#include <QtTest/QtTest>

class Ecma167Test : public QObject
{
    Q_OBJECT

private slots:
    // 测试tag_identifier枚举
    void testTagIdentifierEnum() {
        QVERIFY(ECMA_PrimaryVolumeDescriptor == 1);
        QVERIFY(ECMA_AnchorVolumeDescriptorPointer == 2);
        QVERIFY(ECMA_VolumeDescriptorPointer == 3);
        QVERIFY(ECMA_ImpUseVolumeDescriptor == 4);
        QVERIFY(ECMA_PartitionDescriptor == 5);
        QVERIFY(ECMA_LogicalVolumeDescriptor == 6);
        QVERIFY(ECMA_UnAllocatedSpaceDesc == 7);
        QVERIFY(ECMA_TerminatingDescriptor == 8);
        QVERIFY(ECMA_LogicVolIntegrityDesc == 9);
        QVERIFY(ECMA_FileSetDescriptor == 256);
        QVERIFY(ECMA_FileIdentifierDescriptor == 257);
        QVERIFY(ECMA_AllocationExtentDescriptor == 258);
        QVERIFY(ECMA_FileEntry == 261);
        QVERIFY(ECMA_ExtendedFileEntry == 266);
    }

    // 测试数值获取函数
    void testNumericFunctions() {
        uint8_t data[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
        
        QVERIFY(_get_u8(data) == 0x01);
        QVERIFY(_get_u16(data) == 0x0201);
        QVERIFY(_get_u32(data) == 0x04030201);
        QVERIFY(_get_u64(data) == 0x0807060504030201ULL);
    }

    // 测试entity_id结构体
    void testEntityIdStruct() {
        struct entity_id eid;
        
        // 验证结构体成员
        QVERIFY(sizeof(eid.identifier) == 23);
        QVERIFY(sizeof(eid.identifier_suffix) == 8);
    }

    // 测试extent_ad结构体
    void testExtentAdStruct() {
        struct extent_ad ad;
        
        // 验证结构体成员
        ad.lba = 100;
        ad.length = 2048;
        
        QVERIFY(ad.lba == 100);
        QVERIFY(ad.length == 2048);
    }

    // 测试long_ad结构体
    void testLongAdStruct() {
        struct long_ad ad;
        
        // 验证结构体成员
        ad.lba = 100;
        ad.length = 2048;
        ad.partition = 1;
        ad.extent_type = 0;
        
        QVERIFY(ad.lba == 100);
        QVERIFY(ad.length == 2048);
        QVERIFY(ad.partition == 1);
        QVERIFY(ad.extent_type == 0);
    }

    // 测试file_identifier结构体
    void testFileIdentifierStruct() {
        struct file_identifier fi;
        
        // 验证结构体成员
        fi.characteristic = 0;
        fi.filename_len = 0;
        
        QVERIFY(sizeof(fi.icb) == sizeof(struct long_ad));
        QVERIFY(sizeof(fi.filename) == 256);
    }

    // 测试file_entry结构体
    void testFileEntryStruct() {
        struct file_entry fe;
        
        // 验证结构体成员
        fe.length = 0;
        fe.file_type = 0;
        fe.content_inline = 0;
        fe.ad_type = 0;
        
        QVERIFY(sizeof(fe.u.ads.ad) > 0);
    }

    // 测试primary_volume_descriptor结构体
    void testPrimaryVolumeDescriptorStruct() {
        struct primary_volume_descriptor pvd;
        
        // 验证结构体成员大小
        QVERIFY(sizeof(pvd.volume_identifier) == 31);
        QVERIFY(sizeof(pvd.volume_identifier_length) == 1);
        QVERIFY(sizeof(pvd.volume_set_identifier) == 128);
    }

    // 测试anchor_volume_descriptor结构体
    void testAnchorVolumeDescriptorStruct() {
        struct anchor_volume_descriptor avd;
        
        // 验证结构体成员
        QVERIFY(sizeof(avd.mvds) == sizeof(struct extent_ad));
        QVERIFY(sizeof(avd.rvds) == sizeof(struct extent_ad));
    }

    // 测试partition_descriptor结构体
    void testPartitionDescriptorStruct() {
        struct partition_descriptor pd;
        
        // 验证结构体成员
        pd.number = 1;
        pd.start_block = 100;
        pd.num_blocks = 1000;
        
        QVERIFY(pd.number == 1);
        QVERIFY(pd.start_block == 100);
        QVERIFY(pd.num_blocks == 1000);
    }

    // 测试logical_volume_descriptor结构体
    void testLogicalVolumeDescriptorStruct() {
        struct logical_volume_descriptor lvd;
        
        // 验证结构体成员
        lvd.block_size = 2048;
        lvd.num_partition_maps = 1;
        lvd.partition_map_lable_length = 0;
        
        QVERIFY(lvd.block_size == 2048);
        QVERIFY(sizeof(lvd.domain_id) == sizeof(struct entity_id));
        QVERIFY(sizeof(lvd.contents_use) == 16);
        QVERIFY(sizeof(lvd.partition_map_table) == 2048);
    }

    // 测试file_set_descriptor结构体
    void testFileSetDescriptorStruct() {
        struct file_set_descriptor fsd;
        
        // 验证结构体成员
        QVERIFY(sizeof(fsd.root_icb) == sizeof(struct long_ad));
    }
};

QTEST_MAIN(Ecma167Test)
#include "ecma167_test.moc"