#include <QtTest>
#include <QApplication>
#include "src/data/toc.h"
#include "src/data/track.h"
#include "src/data/msf.h"
#include <QDebug>

class TocTest : public QObject
{
    Q_OBJECT

private slots:
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
        
        // 测试take方法
        Track takenTrack = toc.take(0);
        QCOMPARE(toc.size(), initialSize);
        QVERIFY(takenTrack.type() == Track::TYPE_DATA);
        
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
};

QTEST_MAIN(TocTest)
#include "toc_test.moc"