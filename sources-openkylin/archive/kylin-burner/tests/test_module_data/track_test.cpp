#include <QtTest>
#include "../src/data/track.h"
#include "../tests/test_common/test_helpers.h"

class TrackTest : public QObject
{
    Q_OBJECT

private slots:
    // 测试初始化
    void initTestCase() {
        // 可以在这里初始化一些测试所需的资源
    }

    // 测试Track构造函数
    void testTrackConstructor() {
        // 创建Track对象
        Track track;
        QVERIFY(true); // 如果能成功构造，则测试通过
    }

    void testMethod(){
        Track* track = new Track();
        //Track* track1 = new Track(track);

        Msf firstSector(0, 2, 0);  // LBA 150
        Msf lastSector(0, 4, 0);   // LBA 300
        Track track3(firstSector, lastSector, Track::TYPE_DATA, Track::MODE1);

        Track track1;
        Track track2;

        // 测试赋值操作
        track2 = track1;


        QVERIFY(true);

    }

    // 测试type()和setType()方法
    void testTypeMethods()
    {
        Track track;

        // 测试默认值
        QCOMPARE(track.type(), Track::TYPE_UNKNOWN);

        // 测试设置类型
        track.setType(Track::TYPE_AUDIO);
        QCOMPARE(track.type(), Track::TYPE_AUDIO);

        track.setType(Track::TYPE_DATA);
        QCOMPARE(track.type(), Track::TYPE_DATA);

        track.setType(Track::TYPE_UNKNOWN);
        QCOMPARE(track.type(), Track::TYPE_UNKNOWN);
    }

    // 测试mode()和setMode()方法
    void testModeMethods()
    {
        Track track;

        // 测试默认值
        QCOMPARE(track.mode(), Track::UNKNOWN);

        // 测试设置模式
        track.setMode(Track::MODE1);
        QCOMPARE(track.mode(), Track::MODE1);

        track.setMode(Track::MODE2);
        QCOMPARE(track.mode(), Track::MODE2);

        track.setMode(Track::XA_FORM1);
        QCOMPARE(track.mode(), Track::XA_FORM1);

        track.setMode(Track::XA_FORM2);
        QCOMPARE(track.mode(), Track::XA_FORM2);

        track.setMode(Track::DVD);
        QCOMPARE(track.mode(), Track::DVD);

        track.setMode(Track::UNKNOWN);
        QCOMPARE(track.mode(), Track::UNKNOWN);
    }

    // 测试copyPermitted()和setCopyPermitted()方法
    void testCopyPermittedMethods()
    {
        Track track;

        // 测试默认值（应该为true）
        QCOMPARE(track.copyPermitted(), true);

        // 测试设置为false
        track.setCopyPermitted(false);
        QCOMPARE(track.copyPermitted(), false);

        // 测试设置为true
        track.setCopyPermitted(true);
        QCOMPARE(track.copyPermitted(), true);
    }

    // 测试preEmphasis()和setPreEmphasis()方法
    void testPreEmphasisMethods()
    {
        Track track;

        // 测试默认值（应该为false）
        QCOMPARE(track.preEmphasis(), false);

        // 测试设置为true
        track.setPreEmphasis(true);
        QCOMPARE(track.preEmphasis(), true);

        // 测试设置为false
        track.setPreEmphasis(false);
        QCOMPARE(track.preEmphasis(), false);
    }

    // 测试recordedIncremental()和recordedUninterrupted()方法
    void testRecordedMethods()
    {
        Track track;

        // 默认情况下preEmphasis为false，所以recordedIncremental也应该为false
        QCOMPARE(track.recordedIncremental(), false);
        QCOMPARE(track.recordedUninterrupted(), true);

        // 设置preEmphasis为true
        track.setPreEmphasis(true);
        QCOMPARE(track.recordedIncremental(), true);
        QCOMPARE(track.recordedUninterrupted(), false);

        // 设置preEmphasis为false
        track.setPreEmphasis(false);
        QCOMPARE(track.recordedIncremental(), false);
        QCOMPARE(track.recordedUninterrupted(), true);
    }

    // 测试isrc()和setIsrc()方法
    void testIsrcMethods()
    {
        Track track;

        // 测试默认值
        QCOMPARE(track.isrc(), QByteArray());

        // 测试设置ISRC
        QByteArray testIsrc = "TEST12345678";
        track.setIsrc(testIsrc);
        QCOMPARE(track.isrc(), testIsrc);

        // 测试设置为空
        track.setIsrc(QByteArray());
        QCOMPARE(track.isrc(), QByteArray());
    }

    // 测试firstSector()和setFirstSector()方法
    void testFirstSectorMethods()
    {
        Track track;

        // 测试默认值
        Msf defaultMsf;
        QCOMPARE(track.firstSector(), defaultMsf);

        // 测试设置firstSector
        Msf newFirstSector(1, 0, 0);
        track.setFirstSector(newFirstSector);
        QCOMPARE(track.firstSector(), newFirstSector);
    }

    // 测试lastSector()和setLastSector()方法
    void testLastSectorMethods()
    {
        Track track;

        // 测试默认值
        Msf defaultMsf;
        QCOMPARE(track.lastSector(), defaultMsf);

        // 测试设置lastSector
        Msf newLastSector(5, 0, 0);
        track.setLastSector(newLastSector);
        QCOMPARE(track.lastSector(), newLastSector);
    }

    // 测试nextWritableAddress()和setNextWritableAddress()方法
    void testNextWritableAddressMethods()
    {
        Track track;

        // 测试默认值
        Msf defaultMsf;
        QCOMPARE(track.nextWritableAddress(), defaultMsf);

        // 测试设置nextWritableAddress
        Msf newAddress(10, 0, 0);
        track.setNextWritableAddress(newAddress);
        QCOMPARE(track.nextWritableAddress(), newAddress);
    }

    // 测试freeBlocks()和setFreeBlocks()方法
    void testFreeBlocksMethods()
    {
        Track track;

        // 测试默认值
        Msf defaultMsf;
        QCOMPARE(track.freeBlocks(), defaultMsf);

        // 测试设置freeBlocks
        Msf newFreeBlocks(20, 0, 0);
        track.setFreeBlocks(newFreeBlocks);
        QCOMPARE(track.freeBlocks(), newFreeBlocks);
    }

    // 测试length()方法
    void testLengthMethod()
    {
        Track track;

        // 设置firstSector和lastSector来计算长度
        Msf first(0, 2, 0);  // LBA 150
        Msf last(0, 4, 0);   // LBA 300
        track.setFirstSector(first);
        track.setLastSector(last);

        // 长度应该是 last - first + 1
        Msf expectedLength = last - first + 1;
        QCOMPARE(track.length(), expectedLength);
    }

    // 测试realAudioLength()方法
    void testRealAudioLengthMethod()
    {
        Track track;

        // 设置firstSector和lastSector
        Msf first(0, 2, 0);
        Msf last(0, 10, 0);
        track.setFirstSector(first);
        track.setLastSector(last);

        // 当index0为0时，realAudioLength应该等于length
        QCOMPARE(track.realAudioLength(), track.length());

        // 设置index0为非零值
        Msf index0Value(0, 5, 0);
        track.setIndex0(index0Value);
        QCOMPARE(track.realAudioLength(), index0Value);
    }

    // 测试session()和setSession()方法
    void testSessionMethods()
    {
        Track track;

        // 测试默认值（应该为0）
        QCOMPARE(track.session(), 0);

        // 测试设置session
        track.setSession(5);
        QCOMPARE(track.session(), 5);

        track.setSession(0);
        QCOMPARE(track.session(), 0);

        track.setSession(-1);
        QCOMPARE(track.session(), -1);
    }

    // 测试index0()和setIndex0()方法
    void testIndex0Methods()
    {
        Track track;

        // 测试默认值
        Msf defaultMsf;
        QCOMPARE(track.index0(), defaultMsf);

        // 测试设置index0（有效值）
        Msf index0Value(0, 3, 0);
        track.setIndex0(index0Value);
        QCOMPARE(track.index0(), index0Value);

        // 测试设置index0（边界值）
        Msf first(0, 2, 0);
        Msf last(0, 10, 0);
        track.setFirstSector(first);
        track.setLastSector(last);

        // 计算相对偏移量
        Msf relOffset = last - first;  // 这应该是允许的最大偏移
        track.setIndex0(relOffset);
        QCOMPARE(track.index0(), relOffset);

        // 测试超过限制的值（应该不设置）
        Msf tooLarge(0, 15, 0);
        track.setIndex0(tooLarge);
        // 由于tooLarge > (last-first)，index0不应该改变
        QCOMPARE(track.index0(), relOffset);
    }

    // 测试indices()和setIndices()方法以及indexCount()
    void testIndicesMethods()
    {
        Track track;

        // 测试默认值
        QVERIFY(track.indices().isEmpty());
        QCOMPARE(track.indexCount(), 0);

        // 测试设置indices
        QList<Msf> indicesList;
        indicesList << Msf(0, 1, 0) << Msf(0, 2, 0) << Msf(0, 3, 0);
        track.setIndices(indicesList);

        QCOMPARE(track.indices(), indicesList);
        QCOMPARE(track.indexCount(), indicesList.count() - 1);  // indexCount返回count-1
    }

    // 测试operator==和operator!=
    void testEqualityOperators()
    {
        Track track1, track2;

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



};

QTEST_MAIN(TrackTest)
#include "track_test.moc"
