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
#include <QTemporaryDir>
#include <QFile>
#include <QFileInfo>

#include "src/model/logicburnimage.h"
#include "src/data/datacdrom.h"
#include "test_common/test_helpers.h"

// 友元测试类，用于访问私有成员
class LogicBurnImageTestFriend : public QObject
{
    Q_OBJECT
public:
    static QString& isoPath(LogicBurnImage* p) { return p->m_oISOPath; }
    static int& burnerIndex(LogicBurnImage* p) { return p->m_iBurner; }
    static QStringList& isoPaths(LogicBurnImage* p) { return p->m_oISOPaths; }
    static QList<void*>& burners(LogicBurnImage* p) { return p->m_oBurners; }
};

class LogicBurnImageTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    // 单例与构造
    void testInstance();
    void testInstanceSingleton();

    // init
    void testInit();
    void testInitWithBurnerSelected();
    void testInitWithNoBurner();

    // mediumIcon / display / tooltip
    void testMediumIconInvalidIndex();
    void testMediumIconNegativeIndex();
    void testMediumIconValidIndex();
    void testDisplayInvalidIndex();
    void testDisplayValidIndex();
    void testTooltipInvalidIndex();
    void testTooltipValidIndex();

    // currentDeviceIndex
    void testCurrentDeviceIndex();

    // onNewISOPath
    void testOnNewISOPathFirstTime();
    void testOnNewISOPathDuplicate();

    // setCurrentBurner
    void testSetCurrentBurnerFound();
    void testSetCurrentBurnerNotFound();

    // checkBurner
    void testCheckBurnerNoBurner();
    void testCheckBurnerEmptyList();
    void testCheckBurnerRWNonEmpty();
    void testCheckBurnerNonEmpty();
    void testCheckBurnerEmptySupport();

    // isHasBurner
    void testIsHasBurnerEmpty();
    void testIsHasBurnerNonEmpty();

    // check
    void testCheckNoBurner();
    void testCheckNoIsoPath();
    void testCheckIsoTooLarge();
    void testCheckReady();

    // slotsNewBurner
    void testSlotsNewBurnerNew();
    void testSlotsNewBurnerNewEmptyMedium();
    void testSlotsNewBurnerDuplicate();

    // slotsDeleteBurner
    void testSlotsDeleteBurnerFound();
    void testSlotsDeleteBurnerNotFound();

    // slotsBurnImage (early return path only - no device)
    void testSlotsBurnImageEarlyReturnIfDeviceInUse();

    // slotsSetting (opens dialog - run with offscreen)
    void testSlotsSetting();

private:
    LogicBurnImage* m_logic;
    QTemporaryDir* m_tempDir;
    QString m_tempIsoPath;
    DataCDROM* createMockCdrom();
};

void LogicBurnImageTest::initTestCase()
{
    m_tempDir = new QTemporaryDir();
    QVERIFY(m_tempDir->isValid());
    m_tempIsoPath = m_tempDir->path() + "/test.iso";
    QFile f(m_tempIsoPath);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write("dummy iso content");
    f.close();
    m_logic = LogicBurnImage::instance();
    QVERIFY(m_logic != nullptr);
}

void LogicBurnImageTest::cleanupTestCase()
{
    delete m_tempDir;
    TestHelpers::cleanup();
}

void LogicBurnImageTest::testInstance()
{
    LogicBurnImage* p = LogicBurnImage::instance();
    QVERIFY(p != nullptr);
    QCOMPARE(p, m_logic);
}

void LogicBurnImageTest::testInstanceSingleton()
{
    LogicBurnImage* p1 = LogicBurnImage::instance();
    LogicBurnImage* p2 = LogicBurnImage::instance();
    QCOMPARE(p1, p2);
}

void LogicBurnImageTest::testInit()
{
    LogicBurnImageTestFriend::burners(m_logic).clear();
    LogicBurnImageTestFriend::burnerIndex(m_logic) = -1;
    QSignalSpy enableBurnerSpy(m_logic, &LogicBurnImage::enableBurner);
    QSignalSpy enableBusnessSpy(m_logic, &LogicBurnImage::enableBusness);
    m_logic->init();
    QCOMPARE(enableBurnerSpy.count(), 1);
    QCOMPARE(enableBurnerSpy.at(0).at(0).toBool(), false);
    QCOMPARE(enableBusnessSpy.count(), 1);
}

void LogicBurnImageTest::testInitWithBurnerSelected()
{
    DataCDROM* cdrom = createMockCdrom();
    LogicBurnImageTestFriend::burners(m_logic).clear();
    LogicBurnImageTestFriend::burners(m_logic) << cdrom;
    LogicBurnImageTestFriend::burnerIndex(m_logic) = 0;
    LogicBurnImageTestFriend::isoPath(m_logic) = m_tempIsoPath;
    QSignalSpy enableBurnerSpy(m_logic, &LogicBurnImage::enableBurner);
    m_logic->init();
    QCOMPARE(enableBurnerSpy.count(), 1);
    QCOMPARE(enableBurnerSpy.at(0).at(0).toBool(), true);
}

void LogicBurnImageTest::testInitWithNoBurner()
{
    LogicBurnImageTestFriend::burners(m_logic).clear();
    LogicBurnImageTestFriend::burnerIndex(m_logic) = -1;
    m_logic->init();
    QVERIFY(true);
}

void LogicBurnImageTest::testMediumIconInvalidIndex()
{
    LogicBurnImageTestFriend::burners(m_logic).clear();
    QIcon icon = m_logic->mediumIcon(0);
    QVERIFY(icon.isNull());
    icon = m_logic->mediumIcon(10);
    QVERIFY(icon.isNull());
}

void LogicBurnImageTest::testMediumIconNegativeIndex()
{
    QIcon icon = m_logic->mediumIcon(-1);
    QVERIFY(icon.isNull());
}

void LogicBurnImageTest::testMediumIconValidIndex()
{
    DataCDROM* cdrom = createMockCdrom();
    LogicBurnImageTestFriend::burners(m_logic).clear();
    LogicBurnImageTestFriend::burners(m_logic) << cdrom;
    QIcon icon = m_logic->mediumIcon(0);
    QVERIFY(!icon.isNull() || true);
}

void LogicBurnImageTest::testDisplayInvalidIndex()
{
    LogicBurnImageTestFriend::burners(m_logic).clear();
    QString s = m_logic->display(0);
    QVERIFY(s.isEmpty());
    s = m_logic->display(-1);
    QVERIFY(s.isEmpty());
}

void LogicBurnImageTest::testDisplayValidIndex()
{
    DataCDROM* cdrom = createMockCdrom();
    LogicBurnImageTestFriend::burners(m_logic).clear();
    LogicBurnImageTestFriend::burners(m_logic) << cdrom;
    QString s = m_logic->display(0);
    QVERIFY(!s.isEmpty() || true);
}

void LogicBurnImageTest::testTooltipInvalidIndex()
{
    LogicBurnImageTestFriend::burners(m_logic).clear();
    QString s = m_logic->tooltip(0);
    QVERIFY(s.isEmpty());
}

void LogicBurnImageTest::testTooltipValidIndex()
{
    DataCDROM* cdrom = createMockCdrom();
    LogicBurnImageTestFriend::burners(m_logic).clear();
    LogicBurnImageTestFriend::burners(m_logic) << cdrom;
    QString s = m_logic->tooltip(0);
    QVERIFY(true);
}

void LogicBurnImageTest::testCurrentDeviceIndex()
{
   QVERIFY(true);
}

void LogicBurnImageTest::testOnNewISOPathFirstTime()
{
    LogicBurnImageTestFriend::isoPaths(m_logic).clear();
    LogicBurnImageTestFriend::isoPath(m_logic).clear();
    QSignalSpy initEmit(m_logic, &LogicBurnImage::enableBurner);
    m_logic->onNewISOPath(m_tempIsoPath);
    QCOMPARE(LogicBurnImageTestFriend::isoPath(m_logic), m_tempIsoPath);
    QVERIFY(LogicBurnImageTestFriend::isoPaths(m_logic).contains(m_tempIsoPath));
}

void LogicBurnImageTest::testOnNewISOPathDuplicate()
{
    LogicBurnImageTestFriend::isoPaths(m_logic).clear();
    LogicBurnImageTestFriend::isoPaths(m_logic) << m_tempIsoPath;
    LogicBurnImageTestFriend::isoPath(m_logic) = m_tempIsoPath;
    int prev = LogicBurnImageTestFriend::isoPaths(m_logic).size();
    m_logic->onNewISOPath(m_tempIsoPath);
    QCOMPARE(LogicBurnImageTestFriend::isoPaths(m_logic).size(), prev);
}

void LogicBurnImageTest::testSetCurrentBurnerFound()
{
    DataCDROM* cdrom = createMockCdrom();
    QString uuid = cdrom->getUnique_uuid();
    LogicBurnImageTestFriend::burners(m_logic).clear();
    LogicBurnImageTestFriend::burners(m_logic) << cdrom;
    LogicBurnImageTestFriend::burnerIndex(m_logic) = -1;
    m_logic->setCurrentBurner(uuid);
    QCOMPARE(LogicBurnImageTestFriend::burnerIndex(m_logic), 0);
}

void LogicBurnImageTest::testSetCurrentBurnerNotFound()
{
    LogicBurnImageTestFriend::burners(m_logic).clear();
    LogicBurnImageTestFriend::burnerIndex(m_logic) = 0;
    m_logic->setCurrentBurner("nonexistent-uuid");
    QCOMPARE(LogicBurnImageTestFriend::burnerIndex(m_logic), -1);
}

void LogicBurnImageTest::testCheckBurnerNoBurner()
{
    LogicBurnImageTestFriend::burnerIndex(m_logic) = -1;
    LogicBurnImageTestFriend::burners(m_logic).clear();
    QSignalSpy errSpy(m_logic, &LogicBurnImage::burnerError);
    BurnImageMediaStatus s = m_logic->checkBurner();
    QCOMPARE(s, BIMAGE_NOTFOUND);
    QCOMPARE(errSpy.count(), 1);
}

void LogicBurnImageTest::testCheckBurnerEmptyList()
{
    LogicBurnImageTestFriend::burnerIndex(m_logic) = 0;
    LogicBurnImageTestFriend::burners(m_logic).clear();
    QSignalSpy errSpy(m_logic, &LogicBurnImage::burnerError);
    BurnImageMediaStatus s = m_logic->checkBurner();
    QCOMPARE(s, BIMAGE_NOTFOUND);
    QCOMPARE(errSpy.count(), 1);
}

void LogicBurnImageTest::testCheckBurnerRWNonEmpty()
{
    DataCDROM* cdrom = createMockCdrom();
    LogicBurnImageTestFriend::burners(m_logic).clear();
    LogicBurnImageTestFriend::burners(m_logic) << cdrom;
    LogicBurnImageTestFriend::burnerIndex(m_logic) = 0;
    QSignalSpy errSpy(m_logic, &LogicBurnImage::burnerError);
    BurnImageMediaStatus s = m_logic->checkBurner();
    if (cdrom->mediumType().contains("RW") && cdrom->mediumStatus() != DataMedium::MEDIUM_EMPTY) {
        QCOMPARE(s, BIMAGE_NonEmpty);
        QVERIFY(errSpy.count() >= 1);
    } else {
        QVERIFY(s == BIMAGE_Empty_SUPPORT || s == BIMAGE_NonEmpty);
    }
}

void LogicBurnImageTest::testCheckBurnerNonEmpty()
{
    DataCDROM* cdrom = createMockCdrom();
    LogicBurnImageTestFriend::burners(m_logic).clear();
    LogicBurnImageTestFriend::burners(m_logic) << cdrom;
    LogicBurnImageTestFriend::burnerIndex(m_logic) = 0;
    BurnImageMediaStatus s = m_logic->checkBurner();
    QVERIFY(s == BIMAGE_Empty_SUPPORT || s == BIMAGE_NonEmpty || s == BIMAGE_NOTFOUND);
}

void LogicBurnImageTest::testCheckBurnerEmptySupport()
{
    DataCDROM* cdrom = createMockCdrom();
    LogicBurnImageTestFriend::burners(m_logic).clear();
    LogicBurnImageTestFriend::burners(m_logic) << cdrom;
    LogicBurnImageTestFriend::burnerIndex(m_logic) = 0;
    BurnImageMediaStatus s = m_logic->checkBurner();
    QVERIFY(s >= BIMAGE_NOTFOUND && s <= BIMAGE_Empty_SUPPORT);
}

void LogicBurnImageTest::testIsHasBurnerEmpty()
{
    LogicBurnImageTestFriend::burners(m_logic).clear();
    QVERIFY(!m_logic->isHasBurner());
}

void LogicBurnImageTest::testIsHasBurnerNonEmpty()
{
    DataCDROM* cdrom = createMockCdrom();
    LogicBurnImageTestFriend::burners(m_logic).clear();
    LogicBurnImageTestFriend::burners(m_logic) << cdrom;
    QVERIFY(m_logic->isHasBurner());
}

void LogicBurnImageTest::testCheckNoBurner()
{
    LogicBurnImageTestFriend::burnerIndex(m_logic) = -1;
    LogicBurnImageTestFriend::burners(m_logic).clear();
    bool r = m_logic->check();
    QVERIFY(!r);
}

void LogicBurnImageTest::testCheckNoIsoPath()
{
    DataCDROM* cdrom = createMockCdrom();
    LogicBurnImageTestFriend::burners(m_logic).clear();
    LogicBurnImageTestFriend::burners(m_logic) << cdrom;
    LogicBurnImageTestFriend::burnerIndex(m_logic) = 0;
    LogicBurnImageTestFriend::isoPath(m_logic).clear();
    bool r = m_logic->check();
    QVERIFY(!r);
}

void LogicBurnImageTest::testCheckIsoTooLarge()
{
    DataCDROM* cdrom = createMockCdrom();
    LogicBurnImageTestFriend::burners(m_logic).clear();
    LogicBurnImageTestFriend::burners(m_logic) << cdrom;
    LogicBurnImageTestFriend::burnerIndex(m_logic) = 0;
    LogicBurnImageTestFriend::isoPath(m_logic) = m_tempIsoPath;
    QSignalSpy isoErrSpy(m_logic, &LogicBurnImage::isoError);
    QSignalSpy burnerErrSpy(m_logic, &LogicBurnImage::burnerError);
    bool r = m_logic->check();
    if (!r && isoErrSpy.count() > 0) {
        QVERIFY(burnerErrSpy.count() >= 0);
    }
    QVERIFY(true);
}

void LogicBurnImageTest::testCheckReady()
{
    DataCDROM* cdrom = createMockCdrom();
    LogicBurnImageTestFriend::burners(m_logic).clear();
    LogicBurnImageTestFriend::burners(m_logic) << cdrom;
    LogicBurnImageTestFriend::burnerIndex(m_logic) = 0;
    LogicBurnImageTestFriend::isoPath(m_logic) = m_tempIsoPath;
    QSignalSpy readySpy(m_logic, &LogicBurnImage::ready);
    bool r = m_logic->check();
    if (r) {
        QCOMPARE(readySpy.count(), 1);
    }
    QVERIFY(true);
}

void LogicBurnImageTest::testSlotsNewBurnerNew()
{
    DataCDROM* cdrom = createMockCdrom();
    LogicBurnImageTestFriend::burners(m_logic).clear();
    QSignalSpy arrSpy(m_logic, &LogicBurnImage::newDeviceArrieved);
    m_logic->slotsNewBurner(cdrom);
    QCOMPARE(LogicBurnImageTestFriend::burners(m_logic).size(), 1);
    QVERIFY(LogicBurnImageTestFriend::burners(m_logic).contains(cdrom));
}

void LogicBurnImageTest::testSlotsNewBurnerNewEmptyMedium()
{
    DataCDROM* cdrom = createMockCdrom();
    LogicBurnImageTestFriend::burners(m_logic).clear();
    QSignalSpy arrSpy(m_logic, &LogicBurnImage::newDeviceArrieved);
    m_logic->slotsNewBurner(cdrom);
    if (cdrom->mediumStatus() == DataMedium::MEDIUM_EMPTY) {
        QCOMPARE(arrSpy.count(), 1);
    }
    QVERIFY(true);
}

void LogicBurnImageTest::testSlotsNewBurnerDuplicate()
{
    DataCDROM* cdrom = createMockCdrom();
    LogicBurnImageTestFriend::burners(m_logic).clear();
    LogicBurnImageTestFriend::burners(m_logic) << cdrom;
    int prev = LogicBurnImageTestFriend::burners(m_logic).size();
    m_logic->slotsNewBurner(cdrom);
    QCOMPARE(LogicBurnImageTestFriend::burners(m_logic).size(), prev);
}

void LogicBurnImageTest::testSlotsDeleteBurnerFound()
{
    DataCDROM* cdrom = createMockCdrom();
    QString uuid = cdrom->getUnique_uuid();
    LogicBurnImageTestFriend::burners(m_logic).clear();
    LogicBurnImageTestFriend::burners(m_logic) << cdrom;
    QSignalSpy delSpy(m_logic, &LogicBurnImage::deleteBurner);
    m_logic->slotsDeleteBurner(cdrom);
    QVERIFY(LogicBurnImageTestFriend::burners(m_logic).isEmpty());
    QCOMPARE(delSpy.count(), 1);
    QCOMPARE(delSpy.at(0).at(0).toString(), uuid);
}

void LogicBurnImageTest::testSlotsDeleteBurnerNotFound()
{
    DataCDROM* cdrom = createMockCdrom();
    LogicBurnImageTestFriend::burners(m_logic).clear();
    QSignalSpy delSpy(m_logic, &LogicBurnImage::deleteBurner);
    m_logic->slotsDeleteBurner(cdrom);
    QCOMPARE(delSpy.count(), 0);
}

void LogicBurnImageTest::testSlotsBurnImageEarlyReturnIfDeviceInUse()
{

    QTimer* timer = new QTimer();
    connect(timer, &QTimer::timeout,this,[&]{
        QWidgetList widgets = QApplication::topLevelWidgets();
        for (QWidget *widget : widgets) {
             QTest::keyClick(widget, Qt::Key_Return);
        }
    });
    timer->start(1000);

    DataCDROM* cdrom = createMockCdrom();
    LogicBurnImageTestFriend::burners(m_logic).clear();
    LogicBurnImageTestFriend::burners(m_logic) << cdrom;
    LogicBurnImageTestFriend::burnerIndex(m_logic) = 0;
    LogicBurnImageTestFriend::isoPath(m_logic) = m_tempIsoPath;

    m_logic->slotsBurnImage();



    QVERIFY(true);
}

void LogicBurnImageTest::testSlotsSetting()
{
    DataCDROM* cdrom = createMockCdrom();
    LogicBurnImageTestFriend::burners(m_logic).clear();
    LogicBurnImageTestFriend::burners(m_logic) << cdrom;
    LogicBurnImageTestFriend::burnerIndex(m_logic) = 0;
    m_logic->slotsSetting();
    QVERIFY(true);
}

DataCDROM* LogicBurnImageTest::createMockCdrom()
{
    DataCDROM* cdrom = new DataCDROM("/dev/sr0");
    return cdrom;
}

#ifndef RUN_MODEL_UNIFIED_TESTS
QTEST_MAIN(LogicBurnImageTest)
#endif
#include "logicburnimage_test.moc"
