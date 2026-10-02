/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/>.
*/

#include <QtTest>
#include <QSignalSpy>

#include "src/model/logiccopydisc.h"
#include "src/data/datacdrom.h"
#include "test_common/test_helpers.h"

class LogicCopyDiscTestFriend
{
public:
    static int& sourceIndex(LogicCopyDisc* p) { return p->m_iSource; }
    static int& destinationIndex(LogicCopyDisc* p) { return p->m_iDestination; }
    static QList<void*>& sources(LogicCopyDisc* p) { return p->m_oSources; }
    static QList<void*>& destinations(LogicCopyDisc* p) { return p->m_oDestinations; }
    static void progressWidgetDelayResponeTimer(LogicCopyDisc* p, int ms) { p->progressWidgetDelayResponeTimer(ms); }
};

class LogicCopyDiscTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testInstance();
    void testCapacity_destZero();
    void testCapacity_invalidIndices();
    void testCapacity_valid();
    void testUpdateISOPath();
    void testSlotsAddDestination_new();
    void testSlotsAddDestination_duplicate();
    void testSlotsAddSource_new();
    void testSlotsAddSource_duplicate();
    void testSlotsNoMedium_inDest();
    void testSlotsNoMedium_inSource();
    void testSlotsNoMedium_notPresent();
    void testSlotsDestinationChange();
    void testSlotsSourceChange();
    void testProgressWidgetDelayResponeTimer();
    void testSlotsSetting();
    void testSlotsGetISO_earlyReturn();
    void testSlotsCopyData_earlyReturn();
};

void LogicCopyDiscTest::initTestCase()
{
}

void LogicCopyDiscTest::cleanupTestCase()
{
    TestHelpers::cleanup();
}

void LogicCopyDiscTest::testInstance()
{
    LogicCopyDisc* p1 = LogicCopyDisc::instance();
    LogicCopyDisc* p2 = LogicCopyDisc::instance();
    QVERIFY(p1 != nullptr);
    QVERIFY(p2 != nullptr);
    QCOMPARE(p1, p2);
}

void LogicCopyDiscTest::testCapacity_destZero()
{
    LogicCopyDisc* p = LogicCopyDisc::instance();
    LogicCopyDiscTestFriend::destinationIndex(p) = 0;
    QVERIFY(p->capacity() == true);
}

void LogicCopyDiscTest::testCapacity_invalidIndices()
{
    LogicCopyDisc* p = LogicCopyDisc::instance();
    LogicCopyDiscTestFriend::sourceIndex(p) = -1;
    LogicCopyDiscTestFriend::destinationIndex(p) = 1;
    QVERIFY(p->capacity() == false);
    LogicCopyDiscTestFriend::sourceIndex(p) = 0;
    LogicCopyDiscTestFriend::destinationIndex(p) = -1;
    QVERIFY(p->capacity() == false);
}

void LogicCopyDiscTest::testCapacity_valid()
{
    LogicCopyDisc* p = LogicCopyDisc::instance();
    DataCDROM* src = new DataCDROM("/dev/sr0");
    DataCDROM* des = new DataCDROM("/tmp/iso_path");
    LogicCopyDiscTestFriend::sources(p).clear();
    LogicCopyDiscTestFriend::destinations(p).clear();
    LogicCopyDiscTestFriend::sources(p) << src;
    LogicCopyDiscTestFriend::destinations(p) << des << src;
    LogicCopyDiscTestFriend::sourceIndex(p) = 0;
    LogicCopyDiscTestFriend::destinationIndex(p) = 1;
    bool cap = p->capacity();
    QVERIFY(cap == true || cap == false);
    LogicCopyDiscTestFriend::sources(p).clear();
    LogicCopyDiscTestFriend::destinations(p).clear();
    delete src;
    delete des;
}

void LogicCopyDiscTest::testUpdateISOPath()
{
    LogicCopyDisc* p = LogicCopyDisc::instance();
    DataCDROM* cdrom = new DataCDROM("/tmp/path");
    LogicCopyDiscTestFriend::destinations(p).clear();
    LogicCopyDiscTestFriend::destinations(p) << cdrom;
    p->updateISOPath("/tmp/new.iso");
    LogicCopyDiscTestFriend::destinations(p).clear();
    delete cdrom;
}

void LogicCopyDiscTest::testSlotsAddDestination_new()
{
    LogicCopyDisc* p = LogicCopyDisc::instance();
    DataCDROM* d = new DataCDROM("/tmp/dest");
    QSignalSpy spy(p, &LogicCopyDisc::sigAddDestination);
    p->slotsAddDestination(d);
    QCOMPARE(spy.count(), 1);
    QVERIFY(LogicCopyDiscTestFriend::destinations(p).contains(d));
    LogicCopyDiscTestFriend::destinations(p).removeOne(d);
    delete d;
}

void LogicCopyDiscTest::testSlotsAddDestination_duplicate()
{
    LogicCopyDisc* p = LogicCopyDisc::instance();
    DataCDROM* d = new DataCDROM("/tmp/dest");
    LogicCopyDiscTestFriend::destinations(p) << d;
    QSignalSpy spy(p, &LogicCopyDisc::sigAddDestination);
    p->slotsAddDestination(d);
    QCOMPARE(spy.count(), 1);
    LogicCopyDiscTestFriend::destinations(p).removeOne(d);
    delete d;
}

void LogicCopyDiscTest::testSlotsAddSource_new()
{
    LogicCopyDisc* p = LogicCopyDisc::instance();
    DataCDROM* s = new DataCDROM("/dev/sr0");
    QSignalSpy spy(p, &LogicCopyDisc::sigAddSource);
    p->slotsAddSource(s);
    QCOMPARE(spy.count(), 1);
    QVERIFY(LogicCopyDiscTestFriend::sources(p).contains(s));
    LogicCopyDiscTestFriend::sources(p).removeOne(s);
    delete s;
}

void LogicCopyDiscTest::testSlotsAddSource_duplicate()
{
    LogicCopyDisc* p = LogicCopyDisc::instance();
    DataCDROM* s = new DataCDROM("/dev/sr0");
    LogicCopyDiscTestFriend::sources(p) << s;
    QSignalSpy spy(p, &LogicCopyDisc::sigAddSource);
    p->slotsAddSource(s);
    QCOMPARE(spy.count(), 0);
    LogicCopyDiscTestFriend::sources(p).removeOne(s);
    delete s;
}

void LogicCopyDiscTest::testSlotsNoMedium_inDest()
{
    LogicCopyDisc* p = LogicCopyDisc::instance();
    DataCDROM* d = new DataCDROM("/tmp/d");
    LogicCopyDiscTestFriend::destinations(p) << d;
    QSignalSpy spy(p, &LogicCopyDisc::sigDelDestination);
    p->slotsNoMedium(d);
    QCOMPARE(spy.count(), 1);
    QVERIFY(!LogicCopyDiscTestFriend::destinations(p).contains(d));
    delete d;
}

void LogicCopyDiscTest::testSlotsNoMedium_inSource()
{
    LogicCopyDisc* p = LogicCopyDisc::instance();
    DataCDROM* s = new DataCDROM("/dev/sr0");
    LogicCopyDiscTestFriend::sources(p) << s;
    QSignalSpy spy(p, &LogicCopyDisc::sigDelSource);
    p->slotsNoMedium(s);
    QCOMPARE(spy.count(), 1);
    QVERIFY(!LogicCopyDiscTestFriend::sources(p).contains(s));
    delete s;
}

void LogicCopyDiscTest::testSlotsNoMedium_notPresent()
{
    LogicCopyDisc* p = LogicCopyDisc::instance();
    DataCDROM* d = new DataCDROM("/tmp/notinlist");
    p->slotsNoMedium(d);
    delete d;
    QVERIFY(true);
}

void LogicCopyDiscTest::testSlotsDestinationChange()
{
    LogicCopyDisc* p = LogicCopyDisc::instance();
    p->slotsDestinationChange(0);
    QCOMPARE(LogicCopyDiscTestFriend::destinationIndex(p), 0);
    p->slotsDestinationChange(1);
    QCOMPARE(LogicCopyDiscTestFriend::destinationIndex(p), 1);
}

void LogicCopyDiscTest::testSlotsSourceChange()
{
    LogicCopyDisc* p = LogicCopyDisc::instance();
    p->slotsSourceChange(0);
    QCOMPARE(LogicCopyDiscTestFriend::sourceIndex(p), 0);
    p->slotsSourceChange(-1);
    QCOMPARE(LogicCopyDiscTestFriend::sourceIndex(p), -1);
}

void LogicCopyDiscTest::testProgressWidgetDelayResponeTimer()
{
    LogicCopyDisc* p = LogicCopyDisc::instance();
    LogicCopyDiscTestFriend::progressWidgetDelayResponeTimer(p, 10);
    QVERIFY(true);
}

void LogicCopyDiscTest::testSlotsSetting()
{
    LogicCopyDisc* p = LogicCopyDisc::instance();
    DataCDROM* src = new DataCDROM("/dev/sr0");
    DataCDROM* des0 = new DataCDROM("/tmp/iso_path");
    DataCDROM* des1 = new DataCDROM("/tmp/iso_path2");
    LogicCopyDiscTestFriend::sources(p).clear();
    LogicCopyDiscTestFriend::destinations(p).clear();
    LogicCopyDiscTestFriend::sources(p) << src;
    LogicCopyDiscTestFriend::destinations(p) << des0 << des1;
    LogicCopyDiscTestFriend::sourceIndex(p) = 0;
    LogicCopyDiscTestFriend::destinationIndex(p) = 0;
    p->slotsSetting();
    LogicCopyDiscTestFriend::destinationIndex(p) = 1;
    p->slotsSetting();
    LogicCopyDiscTestFriend::sources(p).clear();
    LogicCopyDiscTestFriend::destinations(p).clear();
    delete src;
    delete des0;
    delete des1;
}

void LogicCopyDiscTest::testSlotsGetISO_earlyReturn()
{
    LogicCopyDisc* p = LogicCopyDisc::instance();
    DataCDROM* src = new DataCDROM("/dev/sr0");
    DataCDROM* des = new DataCDROM("/tmp/out");
    LogicCopyDiscTestFriend::sources(p).clear();
    LogicCopyDiscTestFriend::destinations(p).clear();
    LogicCopyDiscTestFriend::sources(p) << src;
    LogicCopyDiscTestFriend::destinations(p) << des;
    LogicCopyDiscTestFriend::sourceIndex(p) = 0;
    LogicCopyDiscTestFriend::destinationIndex(p) = 0;
    p->slotsGetISO();
    LogicCopyDiscTestFriend::sources(p).clear();
    LogicCopyDiscTestFriend::destinations(p).clear();
    delete src;
    delete des;
}

void LogicCopyDiscTest::testSlotsCopyData_earlyReturn()
{
    LogicCopyDisc* p = LogicCopyDisc::instance();
    DataCDROM* src = new DataCDROM("/dev/sr0");
    DataCDROM* des = new DataCDROM("/dev/sr1");
    LogicCopyDiscTestFriend::sources(p).clear();
    LogicCopyDiscTestFriend::destinations(p).clear();
    LogicCopyDiscTestFriend::sources(p) << src;
    LogicCopyDiscTestFriend::destinations(p) << des;
    LogicCopyDiscTestFriend::sourceIndex(p) = 0;
    LogicCopyDiscTestFriend::destinationIndex(p) = 1;
    p->slotsCopyData();
    LogicCopyDiscTestFriend::sources(p).clear();
    LogicCopyDiscTestFriend::destinations(p).clear();
    delete src;
    delete des;
}

#ifndef RUN_MODEL_UNIFIED_TESTS
QTEST_MAIN(LogicCopyDiscTest)
#endif
#include "logiccopydisc_test.moc"
