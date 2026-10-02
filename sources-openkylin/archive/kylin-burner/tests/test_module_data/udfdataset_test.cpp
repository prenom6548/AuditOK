#include "lib/ky-udf-image/include/udfdataset.h"
#include <QtTest/QtTest>

class UdfDatasetTest : public QObject
{
    Q_OBJECT
private slots:
#if 0
    // 测试descriptor_tag结构体
    void testDescriptorTagStruct() {
        struct descriptor_tag tag;
        
        // 验证结构体成员
        tag.tagIdent = 1;
        tag.descVersion = 2;
        tag.tagChecksum = 3;
        tag.reserved = 4;
        tag.tagSerialNum = 5;
        tag.descCRC = 6;
        tag.descCRCLength = 7;
        tag.tagLocation = 8;
        
        QVERIFY(tag.tagIdent == 1);
        QVERIFY(tag.descVersion == 2);
        QVERIFY(tag.tagChecksum == 3);
        QVERIFY(tag.reserved == 4);
        QVERIFY(tag.tagSerialNum == 5);
        QVERIFY(tag.descCRC == 6);
        QVERIFY(tag.descCRCLength == 7);
        QVERIFY(tag.tagLocation == 8);
    }


    // 测试tag_char_sepc结构体
    void testTagCharSpecStruct() {
        struct tag_char_sepc spec;
        
        // 验证结构体成员大小
        QVERIFY(sizeof(spec.charSetType) == 1);
        QVERIFY(sizeof(spec.charSetInfo) == 63);
    }

    // 测试tag_extent_ad结构体
    void testTagExtentAdStruct() {
        struct tag_extent_ad extent;
        
        // 验证结构体成员
        extent.extLength = 1024;
        extent.extLocation = 2048;
        
        QVERIFY(extent.extLength == 1024);
        QVERIFY(extent.extLocation == 2048);
    }
#endif
#if 0
    // 测试time_stamp结构体
    void testTimeStampStruct() {
        struct time_stamp timestamp;
        
        // 验证结构体成员
        timestamp.typeAndTimezone = 1;
        timestamp.year = 2023;
        timestamp.month = 12;
        timestamp.day = 25;
        timestamp.hour = 10;
        timestamp.minute = 30;
        timestamp.second = 45;
        timestamp.hundredsOfMicroseconds = 60;
        timestamp.microseconds = 70;
        
        QVERIFY(timestamp.typeAndTimezone == 1);
        QVERIFY(timestamp.year == 2023);
        QVERIFY(timestamp.month == 12);
        QVERIFY(timestamp.day == 25);
        QVERIFY(timestamp.hour == 10);
        QVERIFY(timestamp.minute == 30);
        QVERIFY(timestamp.second == 45);
        QVERIFY(timestamp.hundredsOfMicroseconds == 60);
        QVERIFY(timestamp.microseconds == 70);
    }

    // 测试tag_imp_id结构体
    void testTagImpIdStruct() {
        struct tag_imp_id impId;
        
        // 验证结构体成员大小
        QVERIFY(sizeof(impId.flags) == 1);
        QVERIFY(sizeof(impId.ident) == 23);
        QVERIFY(sizeof(impId.osClass) == 1);
        QVERIFY(sizeof(impId.osIdentifier) == 1);
        QVERIFY(sizeof(impId.identSuffix) == 6);
    }

    // 测试tag_primary_vol_desc结构体
    void testTagPrimaryVolDescStruct() {
        struct tag_primary_vol_desc pvd;
        
        // 验证结构体成员
        QVERIFY(sizeof(pvd.descTag) == sizeof(struct descriptor_tag));
        QVERIFY(sizeof(pvd.volIdent) == 32);
        QVERIFY(sizeof(pvd.volSetIdent) == 128);
        QVERIFY(sizeof(pvd.descCharSet) == sizeof(struct tag_char_sepc));
        QVERIFY(sizeof(pvd.explanatoryCharSet) == sizeof(struct tag_char_sepc));
        QVERIFY(sizeof(pvd.volAbstract) == sizeof(struct tag_extent_ad));
        QVERIFY(sizeof(pvd.volCopyright) == sizeof(struct tag_extent_ad));
        QVERIFY(sizeof(pvd.appIdent) == sizeof(struct tag_imp_id));
        QVERIFY(sizeof(pvd.recordingDateAndTime) == sizeof(struct time_stamp));
        QVERIFY(sizeof(pvd.impIdent) == sizeof(struct tag_imp_id));
        QVERIFY(sizeof(pvd.impUse) == 64);
        QVERIFY(sizeof(pvd.reserved) == 1558);
    }

    // 测试tag_udf_entity_Id结构体
    void testTagUdfEntityIdStruct() {
        struct tag_udf_entity_Id entityId;
        
        // 验证结构体成员大小
        QVERIFY(sizeof(entityId.uFlags) == 1);
        QVERIFY(sizeof(entityId.aID) == 23);
        QVERIFY(sizeof(entityId.uUDFRevision) == 2);
        QVERIFY(sizeof(entityId.uOSClass) == 1);
        QVERIFY(sizeof(entityId.uOSIdentifier) == 1);
        QVERIFY(sizeof(entityId.aReserved) == 4);
    }

    // 测试tag_imp_use_vol_desc结构体
    void testTagImpUseVolDescStruct() {
        struct tag_imp_use_vol_desc impUseVolDesc;
        
        // 验证结构体成员
        QVERIFY(sizeof(impUseVolDesc.descTag) == sizeof(struct descriptor_tag));
        QVERIFY(sizeof(impUseVolDesc.impIdent) == sizeof(struct tag_udf_entity_Id));
        QVERIFY(sizeof(impUseVolDesc.lviCharSet) == sizeof(struct tag_char_sepc));
        QVERIFY(sizeof(impUseVolDesc.logVolIdent) == 128);
        QVERIFY(sizeof(impUseVolDesc.logVolInfo) == 108);
        QVERIFY(sizeof(impUseVolDesc.umplementationID) == sizeof(struct tag_imp_id));
        QVERIFY(sizeof(impUseVolDesc.umplementationUse) == 128);
        QVERIFY(sizeof(impUseVolDesc.reserved) == 1536);
    }

    // 测试tag_reg_id结构体
    void testTagRegIdStruct() {
        struct tag_reg_id regId;
        
        // 验证结构体成员大小
        QVERIFY(sizeof(regId.uFlags) == 1);
        QVERIFY(sizeof(regId.aID) == 23);
        QVERIFY(sizeof(regId.aReserved) == 8);
    }

    // 测试tag_short_ad结构体
    void testTagShortAdStruct() {
        struct tag_short_ad shortAd;
        
        // 验证结构体成员
        shortAd.extLength = 1024;
        shortAd.extPosition = 2048;
        
        QVERIFY(shortAd.extLength == 1024);
        QVERIFY(shortAd.extPosition == 2048);
    }

    // 测试tag_partition_header_desc结构体
    void testTagPartitionHeaderDescStruct() {
        struct tag_partition_header_desc partitionHeaderDesc;
        
        // 验证结构体成员大小
        QVERIFY(sizeof(partitionHeaderDesc.reserve1) == 8);
        QVERIFY(sizeof(partitionHeaderDesc.unallocatedSpaceBitmap) == sizeof(struct tag_short_ad));
        QVERIFY(sizeof(partitionHeaderDesc.reserve2) == 16);
        QVERIFY(sizeof(partitionHeaderDesc.freedSpaceBitmap) == sizeof(struct tag_short_ad));
        QVERIFY(sizeof(partitionHeaderDesc.reserved3) == 88);
    }

    // 测试tag_partition_desc结构体
    void testTagPartitionDescStruct() {
        struct tag_partition_desc partitionDesc;
        
        // 验证结构体成员
        QVERIFY(sizeof(partitionDesc.descTag) == sizeof(struct descriptor_tag));
        QVERIFY(sizeof(partitionDesc.volDescSeqNum) == 4);
        QVERIFY(sizeof(partitionDesc.partitionFlags) == 2);
        QVERIFY(sizeof(partitionDesc.partitionNumber) == 2);
        QVERIFY(sizeof(partitionDesc.partitionContents) == sizeof(struct tag_reg_id));
        QVERIFY(sizeof(partitionDesc.partitionContentsUse) == sizeof(struct tag_partition_header_desc));
        QVERIFY(sizeof(partitionDesc.accessType) == 4);
        QVERIFY(sizeof(partitionDesc.partitionStartingLocation) == 4);
        QVERIFY(sizeof(partitionDesc.partitionLength) == 4);
        QVERIFY(sizeof(partitionDesc.impIdent) == sizeof(struct tag_imp_id));
        QVERIFY(sizeof(partitionDesc.impUse) == 128);
        QVERIFY(sizeof(partitionDesc.reserved) == 1692);
    }

    // 测试tag_lb_addr结构体
    void testTagLbAddrStruct() {
        struct tag_lb_addr lbAddr;
        
        // 验证结构体成员
        lbAddr.logicalBlockNum = 100;
        lbAddr.partitionReferenceNum = 1;
        
        QVERIFY(lbAddr.logicalBlockNum == 100);
        QVERIFY(lbAddr.partitionReferenceNum == 1);
    }

    // 测试tag_long_ad结构体
    void testTagLongAdStruct() {
        struct tag_long_ad longAd;
        
        // 验证结构体成员
        longAd.extLength = 2048;
        QVERIFY(sizeof(longAd.extLocation) == sizeof(struct tag_lb_addr));
        QVERIFY(sizeof(longAd.impUse) == 6);
    }

    // 测试tag_domain_id结构体
    void testTagDomainIdStruct() {
        struct tag_domain_id domainId;
        
        // 验证结构体成员大小
        QVERIFY(sizeof(domainId.uFlags) == 1);
        QVERIFY(sizeof(domainId.aID) == 23);
        QVERIFY(sizeof(domainId.uUDFRevision) == 2);
        QVERIFY(sizeof(domainId.domainFlag) == 1);
        QVERIFY(sizeof(domainId.aReserved) == 5);
    }

    // 测试tag_generic_partition_map结构体
    void testTagGenericPartitionMapStruct() {
        struct tag_generic_partition_map genericPartitionMap;
        
        // 验证结构体成员
        genericPartitionMap.partitionMapType = 1;
        genericPartitionMap.partitionMapLength = 2;
        genericPartitionMap.volSequNo = 3;
        genericPartitionMap.partitionNo = 4;
        
        QVERIFY(genericPartitionMap.partitionMapType == 1);
        QVERIFY(genericPartitionMap.partitionMapLength == 2);
        QVERIFY(genericPartitionMap.volSequNo == 3);
        QVERIFY(genericPartitionMap.partitionNo == 4);
    }

    // 测试tag_logical_vol_desc结构体
    void testTagLogicalVolDescStruct() {
        struct tag_logical_vol_desc logicalVolDesc;
        
        // 验证结构体成员
        QVERIFY(sizeof(logicalVolDesc.descTag) == sizeof(struct descriptor_tag));
        QVERIFY(sizeof(logicalVolDesc.descCharSet) == sizeof(struct tag_char_sepc));
        QVERIFY(sizeof(logicalVolDesc.logicalVolIdent) == 128);
        QVERIFY(sizeof(logicalVolDesc.logicalBlockSize) == 4);
        QVERIFY(sizeof(logicalVolDesc.domainIdent) == sizeof(struct tag_domain_id));
        QVERIFY(sizeof(logicalVolDesc.fileSetDescSequExt) == sizeof(struct tag_long_ad));
        QVERIFY(sizeof(logicalVolDesc.mapTableLength) == 4);
        QVERIFY(sizeof(logicalVolDesc.numPartitionMaps) == 4);
        QVERIFY(sizeof(logicalVolDesc.impIdent) == sizeof(struct tag_imp_id));
        QVERIFY(sizeof(logicalVolDesc.impUse) == 128);
        QVERIFY(sizeof(logicalVolDesc.integritySeqExt) == sizeof(struct tag_extent_ad));
        QVERIFY(sizeof(logicalVolDesc.p_partitionMaps) == sizeof(struct tag_generic_partition_map));
    }

    // 测试tag_unallocated_space_desc结构体
    void testTagUnallocatedSpaceDescStruct() {
        struct tag_unallocated_space_desc unallocatedSpaceDesc;
        
        // 验证结构体成员
        QVERIFY(sizeof(unallocatedSpaceDesc.descTag) == sizeof(struct descriptor_tag));
        QVERIFY(sizeof(unallocatedSpaceDesc.volDescSeqNum) == 4);
        QVERIFY(sizeof(unallocatedSpaceDesc.numAllocDescs) == 4);
        QVERIFY(sizeof(unallocatedSpaceDesc.p_reserved) == (2048-24));
    }

    // 测试tag_terminating_desc结构体
    void testTagTerminatingDescStruct() {
        struct tag_terminating_desc terminatingDesc;
        
        // 验证结构体成员
        QVERIFY(sizeof(terminatingDesc.descTag) == sizeof(struct descriptor_tag));
        QVERIFY(sizeof(terminatingDesc.reserved) == 2032);
    }

    // 测试tag_logic_vol_integrity_desc结构体
    void testTagLogicVolIntegrityDescStruct() {
        struct tag_logic_vol_integrity_desc logicVolIntegrityDesc;
        
        // 验证结构体成员
        QVERIFY(sizeof(logicVolIntegrityDesc.descTag) == sizeof(struct descriptor_tag));
        QVERIFY(sizeof(logicVolIntegrityDesc.recordingDateAndTime) == sizeof(struct time_stamp));
        QVERIFY(sizeof(logicVolIntegrityDesc.integrityType) == 4);
        QVERIFY(sizeof(logicVolIntegrityDesc.nextIntegrityExt) == sizeof(struct tag_extent_ad));
        QVERIFY(sizeof(logicVolIntegrityDesc.uniqueId) == 8);
        QVERIFY(sizeof(logicVolIntegrityDesc.reserved) == 24);
        QVERIFY(sizeof(logicVolIntegrityDesc.numOfPartitions) == 4);
        QVERIFY(sizeof(logicVolIntegrityDesc.lengthOfImpUse) == 4);
        QVERIFY(sizeof(logicVolIntegrityDesc.freeSpaceTable) == 4);
        QVERIFY(sizeof(logicVolIntegrityDesc.sizeTable) == 4);
        QVERIFY(sizeof(logicVolIntegrityDesc.p_impUse) == 46);
        QVERIFY(sizeof(logicVolIntegrityDesc.p_reserved) == (2048-88-46));
    }

    // 测试tag_anchor_vol_desc_ptr结构体
    void testTagAnchorVolDescPtrStruct() {
        struct tag_anchor_vol_desc_ptr anchorVolDescPtr;
        
        // 验证结构体成员
        QVERIFY(sizeof(anchorVolDescPtr.descTag) == sizeof(struct descriptor_tag));
        QVERIFY(sizeof(anchorVolDescPtr.mainVolDescSequExt) == sizeof(struct tag_extent_ad));
        QVERIFY(sizeof(anchorVolDescPtr.ResVolDescSequExt) == sizeof(struct tag_extent_ad));
        QVERIFY(sizeof(anchorVolDescPtr.reserved) == 2016);
    }

    // 测试tag_file_set_desc结构体
    void testTagFileSetDescStruct() {
        struct tag_file_set_desc fileSetDesc;
        
        // 验证结构体成员
        QVERIFY(sizeof(fileSetDesc.descTag) == sizeof(struct descriptor_tag));
        QVERIFY(sizeof(fileSetDesc.recordingDateAndTime) == sizeof(struct time_stamp));
        QVERIFY(sizeof(fileSetDesc.interchangeLvl) == 2);
        QVERIFY(sizeof(fileSetDesc.maxInterchangeLvl) == 2);
        QVERIFY(sizeof(fileSetDesc.charSetList) == 4);
        QVERIFY(sizeof(fileSetDesc.maxCharSetList) == 4);
        QVERIFY(sizeof(fileSetDesc.fileSetNum) == 4);
        QVERIFY(sizeof(fileSetDesc.fileSetDescNum) == 4);
        QVERIFY(sizeof(fileSetDesc.logicalVolIdentCharSet) == sizeof(struct tag_char_sepc));
        QVERIFY(sizeof(fileSetDesc.logicalVolIdent) == 128);
        QVERIFY(sizeof(fileSetDesc.fileSetCharSet) == sizeof(struct tag_char_sepc));
        QVERIFY(sizeof(fileSetDesc.fileSetIdent) == 32);
        QVERIFY(sizeof(fileSetDesc.p_reserved1) == 64);
        QVERIFY(sizeof(fileSetDesc.rootDirectoryICB) == sizeof(struct tag_long_ad));
        QVERIFY(sizeof(fileSetDesc.domainIdent) == sizeof(struct tag_domain_id));
        QVERIFY(sizeof(fileSetDesc.nextExt) == sizeof(struct tag_long_ad));
        QVERIFY(sizeof(fileSetDesc.p_reserved2) == 1584);
    }

    // 测试tag_icb_tag结构体
    void testTagIcbTagStruct() {
        struct tag_icb_tag icbTag;
        
        // 验证结构体成员
        icbTag.priorRecordedNumDirectEntries = 10;
        icbTag.strategyType = 1;
        icbTag.strategyParameter = 2;
        icbTag.numEntries = 3;
        icbTag.reserved = 4;
        icbTag.fileType = 5;
        icbTag.flags = 6;
        
        QVERIFY(icbTag.priorRecordedNumDirectEntries == 10);
        QVERIFY(icbTag.strategyType == 1);
        QVERIFY(icbTag.strategyParameter == 2);
        QVERIFY(icbTag.numEntries == 3);
        QVERIFY(icbTag.reserved == 4);
        QVERIFY(icbTag.fileType == 5);
        QVERIFY(icbTag.flags == 6);
        QVERIFY(sizeof(icbTag.p_reserved) == 6);
    }

    // 测试tag_file_entry结构体
    void testTagFileEntryStruct() {
        struct tag_file_entry fileEntry;
        
        // 验证结构体成员
        QVERIFY(sizeof(fileEntry.descTag) == sizeof(struct descriptor_tag));
        QVERIFY(sizeof(fileEntry.icbTag) == sizeof(struct tag_icb_tag));
        QVERIFY(sizeof(fileEntry.uid) == 4);
        QVERIFY(sizeof(fileEntry.gid) == 4);
        QVERIFY(sizeof(fileEntry.permissions) == 4);
        QVERIFY(sizeof(fileEntry.fileLinkCount) == 2);
        QVERIFY(sizeof(fileEntry.p_reserved_1) == 6);
        QVERIFY(sizeof(fileEntry.informationLength) == 8);
        QVERIFY(sizeof(fileEntry.logicalBlocksRecorded) == 8);
        QVERIFY(sizeof(fileEntry.accessTime) == sizeof(struct time_stamp));
        QVERIFY(sizeof(fileEntry.modificationTime) == sizeof(struct time_stamp));
        QVERIFY(sizeof(fileEntry.attrmodifyTime) == sizeof(struct time_stamp));
        QVERIFY(sizeof(fileEntry.checkpoint) == 4);
        QVERIFY(sizeof(fileEntry.p_reserved_2) == 16);
        QVERIFY(sizeof(fileEntry.impIdent) == sizeof(struct tag_imp_id));
        QVERIFY(sizeof(fileEntry.uniqueID) == 8);
        QVERIFY(sizeof(fileEntry.LengthofExtendedAttributes) == 4);
        QVERIFY(sizeof(fileEntry.LengthofAllocationDescriptors) == 4);
        QVERIFY(sizeof(fileEntry.Allocationdescriptors) == sizeof(struct tag_short_ad) * 10);
        QVERIFY(sizeof(fileEntry.EncryptArea) == 128);
        QVERIFY(sizeof(fileEntry.p_reserved) == (2048-176-8-128-72));
    }

    // 测试tag_icb_param结构体
    void testTagIcbParamStruct() {
        struct tag_icb_param icbParam;
        
        // 验证结构体成员
        icbParam.tagLocation = 100;
        icbParam.fileType = 1;
        icbParam.fileLinkCount = 2;
        icbParam.informationLength = 1024;
        icbParam.logicalBlocksRecorded = 3;
        icbParam.uniqueID = 4;
        
        QVERIFY(icbParam.tagLocation == 100);
        QVERIFY(icbParam.fileType == 1);
        QVERIFY(icbParam.fileLinkCount == 2);
        QVERIFY(icbParam.informationLength == 1024);
        QVERIFY(icbParam.logicalBlocksRecorded == 3);
        QVERIFY(icbParam.uniqueID == 4);
        QVERIFY(sizeof(icbParam.Allocationdescriptors) == sizeof(struct tag_short_ad));
    }

    // 测试tag_file_indent_desc结构体
    void testTagFileIndentDescStruct() {
        struct tag_file_indent_desc fileIndentDesc;
        
        // 验证结构体成员
        QVERIFY(sizeof(fileIndentDesc.descTag) == sizeof(struct descriptor_tag));
        QVERIFY(sizeof(fileIndentDesc.fileVersionNum) == 2);
        QVERIFY(sizeof(fileIndentDesc.fileCharacteristics) == 1);
        QVERIFY(sizeof(fileIndentDesc.lengthFileIdent) == 1);
        QVERIFY(sizeof(fileIndentDesc.icb) == sizeof(struct tag_long_ad));
        QVERIFY(sizeof(fileIndentDesc.lengthOfImpUse) == 2);
    }

    // 测试tag_file_id_info结构体
    void testTagFileIdInfoStruct() {
        struct tag_file_id_info fileIdInfo;
        
        // 验证结构体成员
        fileIdInfo.tagLocation = 100;
        fileIdInfo.descCRCLength = 200;
        fileIdInfo.lengthFileIdent = 10;
        fileIdInfo.fileCharacteristics = 1;
        
        QVERIFY(fileIdInfo.tagLocation == 100);
        QVERIFY(fileIdInfo.descCRCLength == 200);
        QVERIFY(fileIdInfo.lengthFileIdent == 10);
        QVERIFY(sizeof(fileIdInfo.icb) == sizeof(struct tag_long_ad));
        QVERIFY(sizeof(fileIdInfo.fileIdent) == 256);
        QVERIFY(sizeof(fileIdInfo.PadLength) == 1);
        QVERIFY(fileIdInfo.fileCharacteristics == 1);
    }

    // 测试udf_disc结构体
    void testUdfDiscStruct() {
        struct udf_disc disc;
        
        // 验证结构体成员
        QVERIFY(sizeof(disc.pvd) == sizeof(struct tag_primary_vol_desc));
        QVERIFY(sizeof(disc.lvd) == sizeof(struct tag_logical_vol_desc));
        QVERIFY(sizeof(disc.pd) == sizeof(struct tag_partition_desc));
        QVERIFY(sizeof(disc.imp) == sizeof(struct tag_imp_use_vol_desc));
        QVERIFY(sizeof(disc.usd) == sizeof(struct tag_unallocated_space_desc));
        QVERIFY(sizeof(disc.fsd) == sizeof(struct tag_file_set_desc));
        QVERIFY(sizeof(disc.terdes) == sizeof(struct tag_terminating_desc));
    }
#endif
};

QTEST_MAIN(UdfDatasetTest)
#include "udfdataset_test.moc"
