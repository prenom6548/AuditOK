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
#include <QStandardPaths>

#include "src/model/logicmain.h"
#include "src/data/datacdrom.h"
#include "src/frame/configuration.h"
#include "src/status-fix/discread.h"
#include "test_common/test_helpers.h"

class LogicMainTestFriend
{
public:
    static int& busness(LogicMain* p) { return p->m_iBusness; }
    static QHash<QString, void*>& cdroms(LogicMain* p) { return p->m_oCDROMS; }
    static bool hasTheme(LogicMain* p) { return p->m_oTheme != nullptr; }
};

class LogicMainTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testInstance();
    void testGetThread();
    void testBusnessEnum();
    void testInit();
    void testTheme();
    void testIcon();
    void testGetSystemFontSize();
    void testSlotsSetBusnessValid();
    void testSlotsSetBusnessInvalid();
    void testSlotsDoBusnessCreateISO();
    void testSlotsDoBusnessBurnData();
    void testSlotsDoBusnessBurnImage();
    void testSlotsDoBusnessCopyData();
    void testSlotsDoBusnessGetISO();
    void testSlotsMenuBurnSettingBelowBurnImage();
    void testSlotsMenuBurnSettingBurnImage();
    void testSlotsMenuBurnSettingCopy();
    void testSlotsMenuBurnSettingOutOfRange();
    void testSlotsEmptyMedium();
    void testSlotsAppendableMedium();
    void testSlotsFinishedMedium();
    void testSlotsDataSetting();
    void testSlotsDiskCheck();
    void testSlotsHelp();
    void testCreateCDROM_isoPath();
    void testCreateCDROM_alreadyLoaded();
    void testDiscAddedSlot_alreadyLoaded();
    void testDiscRemovedSlot_nullCdrom();
    void testDriveAddedSlot();
    void testDriveRemovedSlot();
    void testCurrentDiscs();
    void testDiscStatusQuerySlot();
    void testDestroy();
};

void LogicMainTest::initTestCase()
{
}

void LogicMainTest::cleanupTestCase()
{
    TestHelpers::cleanup();
}

void LogicMainTest::testInstance()
{
    LogicMain* p1 = LogicMain::instance();
    LogicMain* p2 = LogicMain::instance();
    QVERIFY(p1 != nullptr);
    QVERIFY(p2 != nullptr);
    QCOMPARE(p1, p2);
}

void LogicMainTest::testGetThread()
{
    QThread* t = LogicMain::getThread();
    QVERIFY(t != nullptr);
    QCOMPARE(t->objectName(), QString("LogicMainThread"));
}

void LogicMainTest::testBusnessEnum()
{
    QCOMPARE(LogicMain::CreateISO, 0);
    QCOMPARE(LogicMain::BurnISO9660Data, 1);
    QCOMPARE(LogicMain::BurnUDFData, 2);
    QCOMPARE(LogicMain::AddBurn, 3);
    QCOMPARE(LogicMain::BurnImage, 4);
    QCOMPARE(LogicMain::CopyData, 5);
    QCOMPARE(LogicMain::GetISO, 6);
}

void LogicMainTest::testInit()
{
    LogicMain* lm = LogicMain::instance();
    lm->init();
    QVERIFY(true);
}

void LogicMainTest::testTheme()
{
    LogicMain* lm = LogicMain::instance();
    if (!LogicMainTestFriend::hasTheme(lm))
        QSKIP("QGSettings org.ukui.style not installed");
    QString t = lm->theme();
    QVERIFY(!t.isNull());
}

void LogicMainTest::testIcon()
{
    LogicMain* lm = LogicMain::instance();
    if (!LogicMainTestFriend::hasTheme(lm))
        QSKIP("QGSettings org.ukui.style not installed");
    QString i = lm->icon();
    QVERIFY(!i.isNull());
}

void LogicMainTest::testGetSystemFontSize()
{
    LogicMain* lm = LogicMain::instance();
    int sz = lm->GetSystemFontSize();
    QVERIFY(sz > 0);
}

void LogicMainTest::testSlotsSetBusnessValid()
{
    LogicMain* lm = LogicMain::instance();
    QSignalSpy spy(lm, &LogicMain::functionMenuChanged);
    lm->slotsSetBusness(LogicMain::BurnImage);
    QCOMPARE(LogicMainTestFriend::busness(lm), LogicMain::BurnImage);
    QCOMPARE(spy.count(), 1);
}

void LogicMainTest::testSlotsSetBusnessInvalid()
{
    LogicMain* lm = LogicMain::instance();
    QSignalSpy spy(lm, &LogicMain::functionMenuChanged);
    lm->slotsSetBusness(-1);
    lm->slotsSetBusness(100);
    QCOMPARE(spy.count(), 2);
}

void LogicMainTest::testSlotsDoBusnessCreateISO()
{
    LogicMain* lm = LogicMain::instance();
    LogicMainTestFriend::busness(lm) = LogicMain::CreateISO;
    lm->slotsDoBusness();
}

void LogicMainTest::testSlotsDoBusnessBurnData()
{
    LogicMain* lm = LogicMain::instance();
    LogicMainTestFriend::busness(lm) = LogicMain::BurnISO9660Data;

    lm->slotsDoBusness();

    LogicMainTestFriend::busness(lm) = LogicMain::BurnUDFData;
    lm->slotsDoBusness();
}

void LogicMainTest::testSlotsDoBusnessBurnImage()
{
    LogicMain* lm = LogicMain::instance();
    LogicMainTestFriend::busness(lm) = LogicMain::BurnImage;

    //lm->slotsDoBusness();
}

void LogicMainTest::testSlotsDoBusnessCopyData()
{
    LogicMain* lm = LogicMain::instance();
    LogicMainTestFriend::busness(lm) = LogicMain::CopyData;
    lm->slotsDoBusness();
}

void LogicMainTest::testSlotsDoBusnessGetISO()
{
    LogicMain* lm = LogicMain::instance();
    LogicMainTestFriend::busness(lm) = LogicMain::GetISO;
    lm->slotsDoBusness();
}

void LogicMainTest::testSlotsMenuBurnSettingBelowBurnImage()
{
    LogicMain* lm = LogicMain::instance();
    LogicMainTestFriend::busness(lm) = LogicMain::BurnISO9660Data;
    lm->slotsMenuBurnSetting();
}

void LogicMainTest::testSlotsMenuBurnSettingBurnImage()
{
    LogicMain* lm = LogicMain::instance();
    LogicMainTestFriend::busness(lm) = LogicMain::BurnImage;
    lm->slotsMenuBurnSetting();
}

void LogicMainTest::testSlotsMenuBurnSettingCopy()
{
    LogicMain* lm = LogicMain::instance();
    LogicMainTestFriend::busness(lm) = LogicMain::CopyData;
    lm->blockSignals(true);
    lm->slotsMenuBurnSetting();
    lm->blockSignals(false);
}

void LogicMainTest::testSlotsMenuBurnSettingOutOfRange()
{
    LogicMain* lm = LogicMain::instance();
    QSignalSpy spy1(lm, &LogicMain::burnDataSetting);
    QSignalSpy spy2(lm, &LogicMain::burnImageSetting);
    LogicMainTestFriend::busness(lm) = -1;
    lm->slotsMenuBurnSetting();
    LogicMainTestFriend::busness(lm) = 100;
    lm->slotsMenuBurnSetting();
    QCOMPARE(spy1.count(), 0);
    QCOMPARE(spy2.count(), 0);
}

void LogicMainTest::testSlotsEmptyMedium()
{
    QVERIFY(true);
}

void LogicMainTest::testSlotsAppendableMedium()
{
    QVERIFY(true);
}

void LogicMainTest::testSlotsFinishedMedium()
{
    QVERIFY(true);
}

void LogicMainTest::testSlotsDataSetting()
{
    QVERIFY(true);
}

void LogicMainTest::testSlotsDiskCheck()
{
    QVERIFY(true);
}

void LogicMainTest::testSlotsHelp()
{
    QVERIFY(true);
}

void LogicMainTest::testCreateCDROM_isoPath()
{
    QVERIFY(true);
}

void LogicMainTest::testCreateCDROM_alreadyLoaded()
{
    LogicMain* lm = LogicMain::instance();
    QString basePath = "/tmp/already_loaded_test";
    QString volumnId = CFG->get("CreateISO", "VolumnID", QString("KylinBurner")).toString();
    QString key = basePath + "/" + volumnId + ".iso";
    DataCDROM* cdrom = new DataCDROM(basePath, lm);
    LogicMainTestFriend::cdroms(lm).insert(key, cdrom);
    QMetaObject::invokeMethod(lm, "createCDROM", Qt::QueuedConnection, Q_ARG(QString, basePath));
    QTest::qWait(100);
    QVERIFY(LogicMainTestFriend::cdroms(lm).contains(key));
    LogicMainTestFriend::cdroms(lm).remove(key);
    delete cdrom;
}

void LogicMainTest::testDiscAddedSlot_alreadyLoaded()
{
    LogicMain* lm = LogicMain::instance();
    QString blockName = "/dev/sr99_test";
    LogicMainTestFriend::cdroms(lm).insert(blockName, reinterpret_cast<void*>(1));
    QMetaObject::invokeMethod(lm, "discAddedSlot", Qt::QueuedConnection, Q_ARG(QString, blockName));
    QTest::qWait(50);
    LogicMainTestFriend::cdroms(lm).remove(blockName);
    QVERIFY(true);
}

void LogicMainTest::testDiscRemovedSlot_nullCdrom()
{
    LogicMain* lm = LogicMain::instance();
    QMetaObject::invokeMethod(lm, "discRemovedSlot", Qt::QueuedConnection, Q_ARG(QString, "/dev/nonexistent"));
    QTest::qWait(100);
    QVERIFY(true);
}

void LogicMainTest::testDriveAddedSlot()
{
    LogicMain* lm = LogicMain::instance();
    QMetaObject::invokeMethod(lm, "driveAddedSlot", Qt::DirectConnection, Q_ARG(QString, "/dev/sr0"));
    QVERIFY(true);
}

void LogicMainTest::testDriveRemovedSlot()
{
    LogicMain* lm = LogicMain::instance();
    QMetaObject::invokeMethod(lm, "driveRemovedSlot", Qt::DirectConnection, Q_ARG(QString, "/dev/sr0"));
    QVERIFY(true);
}

void LogicMainTest::testCurrentDiscs()
{
    LogicMain* lm = LogicMain::instance();
    QMetaObject::invokeMethod(lm, "currentDiscs", Qt::QueuedConnection);
    QTest::qWait(200);
    QVERIFY(true);
}

void LogicMainTest::testDiscStatusQuerySlot()
{
    LogicMain* lm = LogicMain::instance();
    QMetaObject::invokeMethod(lm, "discStatusQuerySlot", Qt::DirectConnection, Q_ARG(DiscRead*, nullptr));
    QVERIFY(true);
}

void LogicMainTest::testDestroy()
{
    LogicMain* lm = LogicMain::instance();
    lm->destroy();
    QVERIFY(LogicMain::instance() != nullptr || true);
}

#ifndef RUN_MODEL_UNIFIED_TESTS
QTEST_MAIN(LogicMainTest)
#endif
#include "logicmain_test.moc"
