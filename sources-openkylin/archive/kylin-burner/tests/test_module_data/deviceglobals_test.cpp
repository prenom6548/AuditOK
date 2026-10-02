#include <QtTest>
#include "src/data/deviceglobals.h"
#include <QDebug>


class DeviceGlobalsTest : public QObject
{
    Q_OBJECT

private slots:
    // 测试初始化
    void initTestCase() {
        // 可以在这里初始化一些测试所需的资源
    }

    // 测试deviceTypeString方法
    void testDeviceTypeString() {
        // 测试各种设备类型
        QString typeStr = Device::deviceTypeString(Device::DEVICE_CD_R);
        QVERIFY(typeStr.contains("CD-R"));
        
        typeStr = Device::deviceTypeString(Device::DEVICE_DVD_ROM);
        QVERIFY(typeStr.contains("DVD-ROM"));
        
        typeStr = Device::deviceTypeString(0); // 无效类型
        QVERIFY(typeStr == "Error");
        
        // 测试复合类型（多个标志的组合）
        int compositeType = Device::DEVICE_CD_R | Device::DEVICE_CD_RW;
        typeStr = Device::deviceTypeString(compositeType);
        QVERIFY(typeStr.contains("CD-R") && typeStr.contains("CD-RW"));
        
        // 测试更多设备类型
        typeStr = Device::deviceTypeString(Device::DEVICE_BD_ROM);
        QVERIFY(typeStr.contains("BD-ROM"));
        
        typeStr = Device::deviceTypeString(Device::DEVICE_DVD_PLUS_R);
        QVERIFY(typeStr.contains("DVD+R"));
        
        // 测试所有可能的设备类型
        typeStr = Device::deviceTypeString(Device::DEVICE_CD_ROM);
        QVERIFY(typeStr.contains("CD-ROM"));
        
        typeStr = Device::deviceTypeString(Device::DEVICE_CD_RW);
        QVERIFY(typeStr.contains("CD-RW"));
        
        typeStr = Device::deviceTypeString(Device::DEVICE_DVD_RAM);
        QVERIFY(typeStr.contains("DVD-RAM"));
        
        typeStr = Device::deviceTypeString(Device::DEVICE_DVD_R);
        QVERIFY(typeStr.contains("DVD-R"));
        
        typeStr = Device::deviceTypeString(Device::DEVICE_DVD_RW);
        QVERIFY(typeStr.contains("DVD-RW"));
        
        typeStr = Device::deviceTypeString(Device::DEVICE_DVD_R_DL);
        QVERIFY(typeStr.contains("DVD-R DL"));
        
        typeStr = Device::deviceTypeString(Device::DEVICE_DVD_PLUS_RW);
        QVERIFY(typeStr.contains("DVD+RW"));
        
        typeStr = Device::deviceTypeString(Device::DEVICE_DVD_PLUS_R_DL);
        QVERIFY(typeStr.contains("DVD+R DL"));
        
        typeStr = Device::deviceTypeString(Device::DEVICE_HD_DVD_ROM);
        QVERIFY(typeStr.contains("HD DVD-ROM"));
        
        typeStr = Device::deviceTypeString(Device::DEVICE_HD_DVD_R);
        QVERIFY(typeStr.contains("HD DVD-R"));
        
        typeStr = Device::deviceTypeString(Device::DEVICE_HD_DVD_RAM);
        QVERIFY(typeStr.contains("HD DVD-RAM"));
        
        typeStr = Device::deviceTypeString(Device::DEVICE_BD_R);
        QVERIFY(typeStr.contains("BD-R"));
        
        typeStr = Device::deviceTypeString(Device::DEVICE_BD_RE);
        QVERIFY(typeStr.contains("BD-RE"));
    }

    // 测试writingModeString方法
    void testWritingModeString() {
        // 测试各种写入模式
        QString modeStr = Device::writingModeString(Device::WRITINGMODE_SAO);
        QVERIFY(modeStr.contains("SAO"));
        
        modeStr = Device::writingModeString(Device::WRITINGMODE_TAO);
        QVERIFY(modeStr.contains("TAO"));
        
        modeStr = Device::writingModeString(0); // 无效模式
        QVERIFY(modeStr == "no writing mode");
        
        // 测试复合模式（多个标志的组合）
        int compositeMode = Device::WRITINGMODE_SAO | Device::WRITINGMODE_TAO;
        modeStr = Device::writingModeString(compositeMode);
        QVERIFY(modeStr.contains("SAO") && modeStr.contains("TAO"));
        
        // 测试更多写入模式
        modeStr = Device::writingModeString(Device::WRITINGMODE_RAW);
        QVERIFY(modeStr.contains("RAW"));
        
        modeStr = Device::writingModeString(Device::WRITINGMODE_INCR_SEQ);
        QVERIFY(modeStr.contains("Incremental Sequential"));
        
        modeStr = Device::writingModeString(Device::WRITINGMODE_RRM);
        QVERIFY(modeStr.contains("Random Recording"));
        
        // 测试所有可能的写入模式
        modeStr = Device::writingModeString(Device::WRITINGMODE_SAO_R96P);
        QVERIFY(modeStr.contains("SAO/R96P"));
        
        modeStr = Device::writingModeString(Device::WRITINGMODE_SAO_R96R);
        QVERIFY(modeStr.contains("SAO/R96R"));
        
        modeStr = Device::writingModeString(Device::WRITINGMODE_RAW_R16);
        QVERIFY(modeStr.contains("RAW/R16"));
        
        modeStr = Device::writingModeString(Device::WRITINGMODE_RAW_R96P);
        QVERIFY(modeStr.contains("RAW/R96P"));
        
        modeStr = Device::writingModeString(Device::WRITINGMODE_RAW_R96R);
        QVERIFY(modeStr.contains("RAW/R96R"));
        
        modeStr = Device::writingModeString(Device::WRITINGMODE_RES_OVWR);
        QVERIFY(modeStr.contains("Restricted Overwrite"));
        
        modeStr = Device::writingModeString(Device::WRITINGMODE_LAYER_JUMP);
        QVERIFY(modeStr.contains("Layer Jump"));
        
        modeStr = Device::writingModeString(Device::WRITINGMODE_SRM);
        QVERIFY(modeStr.contains("Sequential Recording"));
        
        modeStr = Device::writingModeString(Device::WRITINGMODE_SRM_POW);
        QVERIFY(modeStr.contains("Sequential Recording + POW"));
    }

    // 测试mediaTypeString方法
    void testMediaTypeString() {
        // 测试各种媒体类型
        QString mediaStr = Device::mediaTypeString(Device::MEDIA_CD_R);
        QVERIFY(mediaStr.contains("CD-R"));
        
        mediaStr = Device::mediaTypeString(Device::MEDIA_DVD_ROM);
        QVERIFY(mediaStr.contains("DVD-ROM"));
        
        // 测试简化版本
        mediaStr = Device::mediaTypeString(Device::MEDIA_DVD_R, true);
        QVERIFY(mediaStr.contains("DVD-R"));
        
        mediaStr = Device::mediaTypeString(Device::MEDIA_UNKNOWN);
        QVERIFY(mediaStr == "unknown medium type");
        
        mediaStr = Device::mediaTypeString(0); // 无效类型
        QVERIFY(mediaStr == "Error");
        
        // 测试simple=false的情况（详细格式）
        mediaStr = Device::mediaTypeString(Device::MEDIA_DVD_R_SEQ, false);
        QVERIFY(mediaStr.contains("DVD-R Sequential"));
        
        mediaStr = Device::mediaTypeString(Device::MEDIA_DVD_R_DL_SEQ, false);
        QVERIFY(mediaStr.contains("DVD-R Dual Layer Sequential"));
        
        mediaStr = Device::mediaTypeString(Device::MEDIA_DVD_R_DL_JUMP, false);
        QVERIFY(mediaStr.contains("DVD-R Dual Layer Jump"));
        
        // 测试simple=true的情况（简化格式）
        mediaStr = Device::mediaTypeString(Device::MEDIA_DVD_R_SEQ, true);
        QVERIFY(mediaStr.contains("DVD-R") && !mediaStr.contains("Sequential"));
        
        mediaStr = Device::mediaTypeString(Device::MEDIA_DVD_R_DL_SEQ, true);
        QVERIFY(mediaStr.contains("DVD-R Dual Layer"));
        
        // 测试复合媒体类型
        int compositeMedia = Device::MEDIA_CD_R | Device::MEDIA_CD_RW;
        mediaStr = Device::mediaTypeString(compositeMedia);
        QVERIFY(mediaStr.contains("CD-R") && mediaStr.contains("CD-RW"));
        
        // 测试BD媒体类型
        mediaStr = Device::mediaTypeString(Device::MEDIA_BD_R_SRM, false);
        QVERIFY(mediaStr.contains("BD-R Sequential (SRM)"));
        
        mediaStr = Device::mediaTypeString(Device::MEDIA_BD_R_SRM, true);
        mediaStr.contains("BD-R");
        
        // 测试所有可能的媒体类型
        mediaStr = Device::mediaTypeString(Device::MEDIA_NONE);
        QVERIFY(mediaStr.contains("No media"));
        
        mediaStr = Device::mediaTypeString(Device::MEDIA_CD_ROM);
        QVERIFY(mediaStr.contains("CD-ROM"));
        
        mediaStr = Device::mediaTypeString(Device::MEDIA_CD_RW);
        QVERIFY(mediaStr.contains("CD-RW"));
        
        mediaStr = Device::mediaTypeString(Device::MEDIA_DVD_RAM);
        QVERIFY(mediaStr.contains("DVD-RAM"));
        
        mediaStr = Device::mediaTypeString(Device::MEDIA_DVD_RW);
        QVERIFY(mediaStr.contains("DVD-RW"));
        
        mediaStr = Device::mediaTypeString(Device::MEDIA_DVD_PLUS_RW);
        QVERIFY(mediaStr.contains("DVD+RW"));
        
        mediaStr = Device::mediaTypeString(Device::MEDIA_DVD_PLUS_R);
        QVERIFY(mediaStr.contains("DVD+R"));
        
        mediaStr = Device::mediaTypeString(Device::MEDIA_DVD_PLUS_R_DL);
        QVERIFY(mediaStr.contains("DVD+R Dual Layer"));
        
        mediaStr = Device::mediaTypeString(Device::MEDIA_DVD_PLUS_RW_DL);
        QVERIFY(mediaStr.contains("DVD+RW Dual Layer"));
        
        mediaStr = Device::mediaTypeString(Device::MEDIA_HD_DVD_ROM);
        QVERIFY(mediaStr.contains("HD DVD-ROM"));
        
        mediaStr = Device::mediaTypeString(Device::MEDIA_HD_DVD_R);
        QVERIFY(mediaStr.contains("HD DVD-R"));
        
        mediaStr = Device::mediaTypeString(Device::MEDIA_HD_DVD_RAM);
        QVERIFY(mediaStr.contains("HD DVD-RAM"));
        
        mediaStr = Device::mediaTypeString(Device::MEDIA_BD_ROM);
        QVERIFY(mediaStr.contains("BD-ROM"));
        
        mediaStr = Device::mediaTypeString(Device::MEDIA_BD_RE);
        QVERIFY(mediaStr.contains("BD-RE"));
        
        mediaStr = Device::mediaTypeString(Device::MEDIA_BD_R_RRM, false);
        
        mediaStr = Device::mediaTypeString(Device::MEDIA_BD_R_RRM, true);

        
        mediaStr = Device::mediaTypeString(Device::MEDIA_BD_R_SRM_POW, false);
        QVERIFY(mediaStr.contains("BD-R Sequential Pseudo Overwrite (SRM+POW)"));
        
        mediaStr = Device::mediaTypeString(Device::MEDIA_BD_R_SRM_POW, true);

        
        // 测试DVD-RW的详细格式
        mediaStr = Device::mediaTypeString(Device::MEDIA_DVD_RW_OVWR, false);
        QVERIFY(mediaStr.contains("DVD-RW Restricted Overwrite"));
        
        mediaStr = Device::mediaTypeString(Device::MEDIA_DVD_RW_SEQ, false);
        QVERIFY(mediaStr.contains("DVD-RW Sequential"));
    }

    // 测试mediaStateString方法
    void testMediaStateString() {
        // 测试各种媒体状态
        QString stateStr = Device::mediaStateString(Device::STATE_EMPTY);
        QVERIFY(stateStr.contains("empty medium"));
        
        stateStr = Device::mediaStateString(Device::STATE_COMPLETE);
        QVERIFY(stateStr.contains("complete medium"));
        
        stateStr = Device::mediaStateString(Device::STATE_UNKNOWN);
        QVERIFY(stateStr == "unknown medium state");
        
        stateStr = Device::mediaStateString(0); // 无效状态
        QVERIFY(stateStr == "Error");
        
        // 测试所有可能的媒体状态
        stateStr = Device::mediaStateString(Device::STATE_NO_MEDIA);
        QVERIFY(stateStr.contains("no medium"));
        
        stateStr = Device::mediaStateString(Device::STATE_INCOMPLETE);
        QVERIFY(stateStr.contains("incomplete medium"));
        
        // 测试复合状态（多个标志的组合）
        int compositeState = Device::STATE_COMPLETE | Device::STATE_INCOMPLETE;
        stateStr = Device::mediaStateString(compositeState);
        QVERIFY(stateStr.contains("complete medium") && stateStr.contains("incomplete medium"));
    }

    // 测试debugBitfield方法
    void testDebugBitfield() {
        // 准备测试数据
        unsigned char testData[] = {0xFF, 0x00, 0xAA, 0x55};
        long len = sizeof(testData);
        
        // 调用debugBitfield方法（不会崩溃即可）
        Device::debugBitfield(testData, len);
        QVERIFY(true);
    }

    // 测试from2Byte方法
    void testFrom2Byte() {
        // 测试正常转换
        unsigned char data[] = {0x12, 0x34};
        quint16 result = Device::from2Byte(data);
        QVERIFY(result == 0x1234);
        
        // 测试NULL输入
        quint16 nullResult = Device::from2Byte(nullptr);
        QVERIFY(nullResult == 0);
    }

    // 测试from4Byte方法
    void testFrom4Byte() {
        // 测试正常转换
        unsigned char data[] = {0x12, 0x34, 0x56, 0x78};
        quint32 result = Device::from4Byte(data);
        QVERIFY(result == 0x12345678U);
        
        // 测试NULL输入
        quint32 nullResult = Device::from4Byte(nullptr);
        QVERIFY(nullResult == 0);
    }

    // 测试fromBcd方法
    void testFromBcd() {
        // 测试BCD转换
        char bcdValue = 0x25; // 表示十进制25
        char result = Device::fromBcd(bcdValue);
        QVERIFY(result == 25);
        
        bcdValue = 0x00; // 表示十进制0
        result = Device::fromBcd(bcdValue);
        QVERIFY(result == 0);
    }

    // 测试toBcd方法
    void testToBcd() {
        // 测试十进制转BCD
        char decimal = 25;
        char result = Device::toBcd(decimal);
        QVERIFY(result == 0x25);
        
        decimal = 0;
        result = Device::toBcd(decimal);
        QVERIFY(result == 0x00);
    }

    // 测试isValidBcd方法
    void testIsValidBcd() {
        // 测试有效的BCD值
        QVERIFY(Device::isValidBcd(0x00) == true);
        QVERIFY(Device::isValidBcd(0x12) == true);
        QVERIFY(Device::isValidBcd(0x99) == true);
        
        // 测试无效的BCD值
        QVERIFY(Device::isValidBcd(0xAB) == false);
        QVERIFY(Device::isValidBcd(0xFF) == false);
    }

    // 测试QDebug操作符
    void testQDebugOperators() {
        // 测试各种QDebug输出操作符
        QDebug debug = qDebug();
        
        // 测试MediaType输出
        debug << Device::MEDIA_CD_R;
        
        // 测试MediaTypes输出
        Device::MediaTypes types = Device::MEDIA_CD_R;
        debug << types;

        
        // 测试WritingMode输出
        debug << Device::WRITINGMODE_SAO;
        
        // 测试WritingModes输出
        Device::WritingModes modes = Device::WRITINGMODE_SAO;
        debug << modes;
        
        QVERIFY(true); // 只要没有崩溃就通过
    }

    // 测试devicetypes.h中的inline函数
    void testInlineFunctions() {
        // 测试isDvdMedia函数
        QVERIFY(Device::isDvdMedia(Device::MEDIA_DVD_ROM) == true);
        QVERIFY(Device::isDvdMedia(Device::MEDIA_CD_ROM) == false);
        
        // 测试isCdMedia函数
        QVERIFY(Device::isCdMedia(Device::MEDIA_CD_ROM) == true);
        QVERIFY(Device::isCdMedia(Device::MEDIA_DVD_ROM) == false);
        
        // 测试isBdMedia函数
        QVERIFY(Device::isBdMedia(Device::MEDIA_BD_ROM) == true);
        QVERIFY(Device::isBdMedia(Device::MEDIA_CD_ROM) == false);
        
        // 测试isRewritableMedia函数
        QVERIFY(Device::isRewritableMedia(Device::MEDIA_CD_RW) == true);
        QVERIFY(Device::isRewritableMedia(Device::MEDIA_CD_ROM) == false);
    }

    // 清理测试环境
    void cleanupTestCase() {
        // 这里可以添加清理代码
    }
};

QTEST_MAIN(DeviceGlobalsTest)
#include "deviceglobals_test.moc"
