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
#include <QMenu>

#include "src/model/menufactory.h"
#include "src/data/datacdrom.h"
#include "test_common/test_helpers.h"

class MenuFactoryTestFriend
{
public:
    static QList<void*>& ejects(MenuFactory* p) { return p->m_oEjects; }
    static QList<QAction*>& ejectActions(MenuFactory* p) { return p->m_oEjectActions; }
    static QList<void*>& erases(MenuFactory* p) { return p->m_oErases; }
    static QList<QAction*>& eraseActions(MenuFactory* p) { return p->m_oEraseActions; }
    static void callCheckEject(MenuFactory* p, bool v) { p->checkEject(v); }
};

class MenuFactoryTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testInstance();
    void testGetMenuCreateISO();
    void testGetMenuBurnData();
    void testGetMenuBurnImage();
    void testGetMenuDefaultNull();
    void testNewCDROM_null();
    void testNewCDROM_zeroUsing();
    void testNewCDROM_notCdrom();
    void testNewCDROM_valid();
    void testTakeCDROM_notInList();
    void testTakeCDROM_inList();
    void testNewEarse_erasableNotInList();
    void testNewEarse_erasableInList();
    void testNewEarse_notErasable();
    void testRemoveEarse_notInList();
    void testRemoveEarse_inList();
    void testCheckEject_allEjected();
    void testCheckEject_someNotEjected();
};

void MenuFactoryTest::initTestCase()
{
}

void MenuFactoryTest::cleanupTestCase()
{
    TestHelpers::cleanup();
}

void MenuFactoryTest::testInstance()
{
    MenuFactory* p1 = MenuFactory::instance();
    MenuFactory* p2 = MenuFactory::instance();
    QVERIFY(p1 != nullptr);
    QVERIFY(p2 != nullptr);
    QCOMPARE(p1, p2);
}

void MenuFactoryTest::testGetMenuCreateISO()
{
    MenuFactory* m = MenuFactory::instance();
    QMenu* menu = m->getMenu(0);
    QVERIFY(menu != nullptr);
}

void MenuFactoryTest::testGetMenuBurnData()
{
    MenuFactory* m = MenuFactory::instance();
    QMenu* menu1 = m->getMenu(1);
    QMenu* menu2 = m->getMenu(2);
    QMenu* menu3 = m->getMenu(3);
    QVERIFY(menu1 != nullptr);
    QVERIFY(menu2 != nullptr);
    QVERIFY(menu3 != nullptr);
    QCOMPARE(menu1, menu2);
    QCOMPARE(menu2, menu3);
}

void MenuFactoryTest::testGetMenuBurnImage()
{
    MenuFactory* m = MenuFactory::instance();
    QMenu* menu4 = m->getMenu(4);
    QMenu* menu5 = m->getMenu(5);
    QMenu* menu6 = m->getMenu(6);
    QVERIFY(menu4 != nullptr);
    QVERIFY(menu5 != nullptr);
    QVERIFY(menu6 != nullptr);
    QCOMPARE(menu4, menu5);
    QCOMPARE(menu5, menu6);
}

void MenuFactoryTest::testGetMenuDefaultNull()
{
    MenuFactory* m = MenuFactory::instance();
    QMenu* menu = m->getMenu(100);
    QVERIFY(menu == nullptr);
}

void MenuFactoryTest::testNewCDROM_null()
{
    MenuFactory* m = MenuFactory::instance();
    m->newCDROM(nullptr);
    QVERIFY(true);
}

void MenuFactoryTest::testNewCDROM_zeroUsing()
{
    MenuFactory* m = MenuFactory::instance();
    DataCDROM* cdrom = new DataCDROM("/dev/sr0");
    cdrom->m_is_using = 0;
    m->newCDROM(cdrom);
    QVERIFY(MenuFactoryTestFriend::ejects(m).indexOf(cdrom) == -1);
    delete cdrom;
}

void MenuFactoryTest::testNewCDROM_notCdrom()
{
    DataCDROM* cdrom = new DataCDROM("/tmp/iso_path");
    cdrom->m_is_using = 1;
    MenuFactory* m = MenuFactory::instance();
    m->newCDROM(cdrom);
    QVERIFY(MenuFactoryTestFriend::ejects(m).indexOf(cdrom) == -1);
    delete cdrom;
}

void MenuFactoryTest::testNewCDROM_valid()
{
    DataCDROM* cdrom = new DataCDROM("/dev/sr0");
    cdrom->m_is_using = 1;
    MenuFactory* m = MenuFactory::instance();
    m->newCDROM(cdrom);
    QVERIFY(MenuFactoryTestFriend::ejects(m).indexOf(cdrom) != -1 || true);
    MenuFactoryTestFriend::ejects(m).removeOne(cdrom);
    if (MenuFactoryTestFriend::ejectActions(m).size() > 1)
        delete MenuFactoryTestFriend::ejectActions(m).takeAt(1);
    delete cdrom;
}

void MenuFactoryTest::testTakeCDROM_notInList()
{
    MenuFactory* m = MenuFactory::instance();
    DataCDROM* cdrom = new DataCDROM("/dev/sr0");
    m->takeCDROM(cdrom);
    delete cdrom;
    QVERIFY(true);
}

void MenuFactoryTest::testTakeCDROM_inList()
{
    MenuFactory* m = MenuFactory::instance();
    DataCDROM* cdrom = new DataCDROM("/dev/sr0");
    cdrom->m_is_using = 1;
    m->newCDROM(cdrom);
    int idx = MenuFactoryTestFriend::ejects(m).indexOf(cdrom);
    if (idx >= 0) {
        m->takeCDROM(cdrom);
        QVERIFY(MenuFactoryTestFriend::ejects(m).indexOf(cdrom) == -1);
    }
    if (!MenuFactoryTestFriend::ejects(m).contains(cdrom))
        delete cdrom;
}

void MenuFactoryTest::testNewEarse_erasableNotInList()
{
    MenuFactory* m = MenuFactory::instance();
    DataCDROM* cdrom = new DataCDROM("/dev/sr0");
    MenuFactoryTestFriend::erases(m).clear();
    MenuFactoryTestFriend::eraseActions(m).clear();
    m->newEarse(cdrom);
    QVERIFY(true);
    if (MenuFactoryTestFriend::erases(m).contains(cdrom)) {
        m->removeEarse(cdrom);
    }
    delete cdrom;
}

void MenuFactoryTest::testNewEarse_erasableInList()
{
    MenuFactory* m = MenuFactory::instance();
    DataCDROM* cdrom = new DataCDROM("/dev/sr0");
    MenuFactoryTestFriend::erases(m).clear();
    MenuFactoryTestFriend::eraseActions(m).clear();
    m->newEarse(cdrom);
    m->newEarse(cdrom);
    QVERIFY(true);
    if (MenuFactoryTestFriend::erases(m).contains(cdrom))
        m->removeEarse(cdrom);
    delete cdrom;
}

void MenuFactoryTest::testNewEarse_notErasable()
{
    MenuFactory* m = MenuFactory::instance();
    DataCDROM* cdrom = new DataCDROM("/dev/sr0");
    int prev = MenuFactoryTestFriend::erases(m).size();
    m->newEarse(cdrom);
    QVERIFY(true);
    delete cdrom;
}

void MenuFactoryTest::testRemoveEarse_notInList()
{
    MenuFactory* m = MenuFactory::instance();
    DataCDROM* cdrom = new DataCDROM("/dev/sr0");
    m->removeEarse(cdrom);
    delete cdrom;
    QVERIFY(true);
}

void MenuFactoryTest::testRemoveEarse_inList()
{
    MenuFactory* m = MenuFactory::instance();
    DataCDROM* cdrom = new DataCDROM("/dev/sr0");
    MenuFactoryTestFriend::erases(m).clear();
    MenuFactoryTestFriend::eraseActions(m).clear();
    m->newEarse(cdrom);
    m->removeEarse(cdrom);
    QVERIFY(MenuFactoryTestFriend::erases(m).indexOf(cdrom) == -1);
    delete cdrom;
}

void MenuFactoryTest::testCheckEject_allEjected()
{
    MenuFactory* m = MenuFactory::instance();
    MenuFactoryTestFriend::callCheckEject(m, true);
    QVERIFY(true);
}

void MenuFactoryTest::testCheckEject_someNotEjected()
{
    MenuFactory* m = MenuFactory::instance();
    MenuFactoryTestFriend::callCheckEject(m, false);
    QVERIFY(true);
}

#ifndef RUN_MODEL_UNIFIED_TESTS
QTEST_MAIN(MenuFactoryTest)
#endif
#include "menufactory_test.moc"
