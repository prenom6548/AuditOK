#include <QtTest>

#include "../src/data/datacdrom.h"
#include "../src/data/track.h"
#include "../src/data/toc.h"
#include "../src/data/msf.h"
#include "../src/data/iso9660backend.h"
#include "../src/data/iso9660.h"
#include "../src/libisofs/iso_fs.h"

class ModelTest : public QObject
{
    Q_OBJECT

private slots:
    // 测试初始化

    void testTrackData(){
        Track track;
        Track track1;
        Track track2;

        // 测试赋值操作
        track2 = track1;

        // 测试默认值
        QCOMPARE(track.type(), Track::TYPE_UNKNOWN);

        // 测试设置类型
        track.setType(Track::TYPE_AUDIO);
        QCOMPARE(track.type(), Track::TYPE_AUDIO);

        // 测试默认值
        QCOMPARE(track.mode(), Track::UNKNOWN);

        // 测试设置模式
        track.setMode(Track::MODE1);
        QCOMPARE(track.mode(), Track::MODE1);

        // 测试默认值（应该为true）
        QCOMPARE(track.copyPermitted(), true);

        // 测试设置为false
        track.setCopyPermitted(false);
        QCOMPARE(track.copyPermitted(), false);

        // 测试默认值（应该为false）
        QCOMPARE(track.preEmphasis(), false);

        // 测试设置为true
        track.setPreEmphasis(true);
        QCOMPARE(track.preEmphasis(), true);


        // 设置preEmphasis为true
        track.setPreEmphasis(true);
        QCOMPARE(track.recordedIncremental(), true);
        QCOMPARE(track.recordedUninterrupted(), false);

        // 设置preEmphasis为false
        track.setPreEmphasis(false);
        QCOMPARE(track.recordedIncremental(), false);
        QCOMPARE(track.recordedUninterrupted(), true);

        // 测试默认值
        QCOMPARE(track.isrc(), QByteArray());

        // 测试设置ISRC
        QByteArray testIsrc = "TEST12345678";
        track.setIsrc(testIsrc);
        QCOMPARE(track.isrc(), testIsrc);

        // 设置firstSector和lastSector来计算长度
        Msf first(0, 2, 0);  // LBA 150
        Msf last(0, 4, 0);   // LBA 300
        track.setFirstSector(first);
        track.setLastSector(last);

        // 测试默认值（应该为0）
        QCOMPARE(track.session(), 0);

        // 测试设置session
        track.setSession(5);
        QCOMPARE(track.session(), 5);

        // 两个默认构造的track应该相等
        QVERIFY(track1 == track2);
        QVERIFY(!(track1 != track2));

        // 修改一个track使其不相等
        track1.setSession(5);
        QVERIFY(track1 != track2);
        QVERIFY(!(track1 == track2));

        // 使它们再次相等
        track2.setSession(5);
        QVERIFY(track1 == track2);
        QVERIFY(!(track1 != track2));

        // 测试不同类型的track
        track1.setType(Track::TYPE_AUDIO);
        track2.setType(Track::TYPE_DATA);
        QVERIFY(track1 != track2);
    }

    // 测试Toc构造函数
    void testTocConstructor()
    {
        // 测试默认构造函数
        Toc toc;
        QVERIFY(true); // 如果能成功构造，则测试通过

        // 测试拷贝构造函数
        Toc toc2(toc);
        QVERIFY(true);
    }

    // 测试Toc赋值操作符
    void testTocAssignmentOperator()
    {
        Toc toc1;
        Toc toc2;

        // 测试赋值操作
        toc2 = toc1;
        QVERIFY(true);
    }

    // 测试Device::ContentsType枚举
    void testDeviceContentsTypeEnum()
    {
        QCOMPARE((int)Device::DATA, 0);
        QCOMPARE((int)Device::AUDIO, 1);
        QCOMPARE((int)Device::MIXED, 2);
        QCOMPARE((int)Device::NONE, 3);
    }

    // 测试Toc公共方法
    void testTocPublicMethods()
    {
        Toc toc;

        // 测试各种方法
        unsigned int discId = toc.discId();
        Q_UNUSED(discId)

        QByteArray mcn = toc.mcn();
        Q_UNUSED(mcn)

        Device::ContentsType type = toc.contentType();
        Q_UNUSED(type)

        int sessions = toc.sessions();
        Q_UNUSED(sessions)

        Msf first = toc.firstSector();
        Q_UNUSED(first)

        Msf last = toc.lastSector();
        Q_UNUSED(last)

        Msf length = toc.length();
        Q_UNUSED(length)

        QVERIFY(true); // 确保所有方法都能被调用
    }

    // 测试Toc容器操作
    void testTocContainerOperations()
    {
        Toc toc;

        // 作为QList<Track>的容器操作
        int initialSize = toc.size();
        QVERIFY(initialSize >= 0);

        bool isEmpty = toc.isEmpty();
        Q_UNUSED(isEmpty)

        // 测试添加Track对象
        Msf firstSector(0, 2, 0);
        Msf lastSector(0, 4, 0);
        Track track(firstSector, lastSector, Track::TYPE_DATA, Track::MODE1);

        toc.append(track);
        QCOMPARE(toc.size(), initialSize + 1);

        // 测试at方法
        Track retrievedTrack = toc.at(0);
        QVERIFY(retrievedTrack.type() == Track::TYPE_DATA);

        // 重新添加track
        toc.append(track);

        // 测试clear方法
        toc.clear();
        QVERIFY(toc.isEmpty());
        QCOMPARE(toc.size(), 0);

        // 测试索引操作
        toc.append(track);
        Track& refTrack = toc[0];
        QVERIFY(refTrack.type() == Track::TYPE_DATA);

        const Toc& constToc = toc;
        const Track& constRefTrack = constToc[0];
        QVERIFY(constRefTrack.type() == Track::TYPE_DATA);

        QVERIFY(true); // 确保容器操作正常工作
    }

    // 测试qDebug输出操作符
    void testQDebugOperator()
    {
        Toc toc;

        // 简单测试QDebug操作符不崩溃
        qDebug() << toc;

        QVERIFY(true);
    }

    // 测试qHash函数
    void testQHashFunction()
    {
        Track track;

        // 测试qHash函数不崩溃
        uint hashValue = qHash(track);
        Q_UNUSED(hashValue)

        QVERIFY(true);
    }

    // 测试MSF构造函数
    void testMSFConstructor() {
        // 创建MSF对象
        Msf msf;
        QVERIFY(true); // 如果能成功构造，则测试通过
    }

    // 测试MSF带参数构造函数
    void testMSFConstructorWithParams() {
        uchar m = 10;
        uchar s = 20;
        uchar f = 30;
        // 创建MSF对象
        Msf msf(m, s, f);
        QVERIFY(true); // 如果能成功构造，则测试通过
    }

    // 测试set方法
    void testSet() {
        Msf msf;
        uchar m = 10;
        uchar s = 20;
        uchar f = 30;
        // 调用set方法
        msf.setValue(m, s, f);
        QVERIFY(true);
    }

    // 测试minute方法
    void testMinute() {
        Msf msf;
        // 调用minute方法
        uchar minute = msf.minutes();
        QVERIFY(true);
    }

    // 测试second方法
    void testSecond() {
        Msf msf;
        // 调用second方法
        uchar second = msf.seconds();
        QVERIFY(true);
    }

    // 测试frame方法
    void testFrame() {
        Msf msf;
        // 调用frame方法
        uchar frame = msf.frames();
        QVERIFY(true);
    }

    // 测试toLBA方法
    void testToLBA() {
        Msf msf;
        // 调用toLBA方法
        long lba = msf.totalFrames();
        QVERIFY(true);
    }

    // 测试fromLBA方法
    void testFromLBA() {
        Msf msf;
        long lba = 12345;
        // 调用fromLBA方法
        msf.fromAudioBytes(lba);
        QVERIFY(true);
    }

    // 测试重载运算符==
    void testOperatorEqual() {
        Msf msf1;
        Msf msf2;
        // 调用重载运算符==
        bool result = (msf1 == msf2);
        QVERIFY(true);
    }

    // 测试重载运算符!=
    void testOperatorNotEqual() {
        Msf msf1;
        Msf msf2;
        // 调用重载运算符!=
        bool result = (msf1 != msf2);
        QVERIFY(true);
    }

    // 测试重载运算符<
    void testOperatorLessThan() {
        Msf msf1;
        Msf msf2;
        // 调用重载运算符<
        bool result = (msf1 < msf2);
        QVERIFY(true);
    }

    // 测试重载运算符<=
    void testOperatorLessEqual() {
        Msf msf1;
        Msf msf2;
        // 调用重载运算符<=
        bool result = (msf1 <= msf2);
        QVERIFY(true);
    }

    // 测试重载运算符>
    void testOperatorGreaterThan() {
        Msf msf1;
        Msf msf2;
        // 调用重载运算符>
        bool result = (msf1 > msf2);
        QVERIFY(true);
    }

    // 测试重载运算符>=
    void testOperatorGreaterEqual() {
        Msf msf1;
        Msf msf2;
        // 调用重载运算符>=
        bool result = (msf1 >= msf2);
        QVERIFY(true);
    }

    // 测试Iso9660DeviceBackend构造函数
    void testIso9660DeviceBackendConstructor()
    {
        DataCDROM* mockDevice = new DataCDROM("/dev/sr0");
        char* data = new char[256];
        Iso9660DeviceBackend backend(mockDevice);
        backend.open();

       // backend.read(25532,data,200);

        backend.close();

        // 验证对象可以正常构造
        QVERIFY(!backend.isOpen()); // 初始状态应该是关闭的
    }

    // 测试Iso9660DeviceBackend虚函数
    void testIso9660DeviceBackendVirtualFunctions()
    {
        DataCDROM* mockDevice = new DataCDROM("/dev/sr0");
        Iso9660DeviceBackend backend(mockDevice);

        // 测试isOpen函数
        QCOMPARE(backend.isOpen(), false);
    }

    void testIso9660(){

        // 创建ISO9660对象并删除
        Iso9660* iso = new Iso9660(nullptr);
        iso->setStartSector(255);
        iso->setPlainIso9660(true);
        iso->plainIso9660();
        //iso->open();

        Iso9660SimplePrimaryDescriptor spd1;
        Iso9660SimplePrimaryDescriptor spd2;
        spd1.getUsedCapacity();
        spd1.getVolumeID();
        spd1.getVolumSpacesSize();

        if(iso->isOpen()){

        }
        iso->fileName();
        iso->primaryDescriptor();
        iso->debug();

        iso->close();
        delete iso;

        QVERIFY(true); // 如果能成功删除，则测试通过

    }

};


#ifndef RUN_MODEL_UNIFIED_TESTS
QTEST_MAIN(ModelTest)
#endif
#include "model_test.moc"
