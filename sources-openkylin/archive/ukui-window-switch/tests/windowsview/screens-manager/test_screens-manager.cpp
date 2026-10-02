/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 *
 */

#include "test_screens-manager.h"
#include <QCoreApplication>
#include <QSignalSpy>
#include <QTest>
#include <QGuiApplication>
#define private public
#include "screens-manager.h"
#undef  private

Q_DECLARE_METATYPE(Screen *)
void TestScreensManager::initTestCase()
{
    qRegisterMetaType<Screen *>("Screen*");
}
void TestScreensManager::testScreenProperties()
{
    auto *screen = QGuiApplication::primaryScreen();
    QVERIFY(screen != nullptr);
    Screen testScreen(screen);
    QSignalSpy enableChangedSpy(&testScreen, &Screen::enableChanged);
    QCOMPARE(testScreen.geometry(), screen->geometry());
    QCOMPARE(testScreen.internal(), screen);
    QVERIFY(!testScreen.enable());
    testScreen.setEnable(true);
    QVERIFY(testScreen.enable());
    QCOMPARE(enableChangedSpy.count(), 1);
    testScreen.setEnable(true);
    QCOMPARE(enableChangedSpy.count(), 1);
    testScreen.setEnable(false);
    QVERIFY(!testScreen.enable());
    QCOMPARE(enableChangedSpy.count(), 2);
}
void TestScreensManager::testManagerInitializesPrimaryScreen()
{
    ScreensManager manager;
    auto *primaryScreen = manager.primaryScreen();
    QVERIFY(primaryScreen != nullptr);
    QCOMPARE(primaryScreen->internal(), QGuiApplication::primaryScreen());
    QVERIFY(primaryScreen->enable());
    QVERIFY(!manager.screens().isEmpty());

    primaryScreen->setEnable(true);
    QVERIFY(primaryScreen->enable());
    QVERIFY(!manager.screens().isEmpty());
    QVERIFY(manager.screens().contains(primaryScreen));

    primaryScreen->setEnable(false);
    QCOMPARE(primaryScreen->enable(), false);
    QCOMPARE(!manager.screens().isEmpty(), false);
    QCOMPARE(manager.screens().contains(primaryScreen), false);
}
void TestScreensManager::testEnableChangedSignals()
{
    ScreensManager manager;
    auto *primaryScreen = manager.primaryScreen();
    QVERIFY(primaryScreen != nullptr);
    QSignalSpy screenAddedSpy(&manager, &ScreensManager::screenAdded);
    QSignalSpy screenRemovedSpy(&manager, &ScreensManager::screenRemoved);
    manager.onEnableChanged(primaryScreen, false);
    QCOMPARE(screenAddedSpy.count(), 0);
    QCOMPARE(screenRemovedSpy.count(), 1);
    manager.onEnableChanged(primaryScreen, true);
    QCOMPARE(screenAddedSpy.count(), 1);
    QCOMPARE(screenRemovedSpy.count(), 1);
}
void TestScreensManager::testPrimaryScreenChangedEnablesTarget()
{
    ScreensManager manager;
    QVERIFY(manager.primaryScreen() != nullptr);
    auto *candidateScreen = new Screen(nullptr, &manager);
    manager.m_screens.append(candidateScreen);
    QSignalSpy primaryScreenChangedSpy(&manager, &ScreensManager::primaryScreenChanged);
    manager.onPrimaryScreenChanged(nullptr);
    QCOMPARE(primaryScreenChangedSpy.count(), 1);
    QCOMPARE(manager.primaryScreen(), candidateScreen);
    QVERIFY(candidateScreen->enable());
    manager.onPrimaryScreenChanged(nullptr);
    QCOMPARE(primaryScreenChangedSpy.count(), 1);

    // test onPrimaryScreenChanged with screen enable
    manager.onPrimaryScreenChanged(candidateScreen->internal());
    // test onPrimaryScreenChanged with screen disenable
    candidateScreen->setEnable(false);
    manager.onPrimaryScreenChanged(candidateScreen->internal());

}
void TestScreensManager::testScreenRemovedSignalAndListUpdate()
{
    ScreensManager manager;
    QVERIFY(!manager.m_screens.isEmpty());
    QSignalSpy screenRemovedSpy(&manager, &ScreensManager::screenRemoved);
    const auto initialScreenCount = manager.m_screens.size();
    auto *screen = manager.m_screens.first();
    manager.onScreenRemoved(screen->internal());
    QCOMPARE(screenRemovedSpy.count(), 1);
    QCOMPARE(manager.m_screens.size(), initialScreenCount - 1);
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
}
void TestScreensManager::testInternalGeometryChangeTriggersRefresh()
{
    ScreensManager manager;
    auto *primaryScreen = manager.primaryScreen();
    QVERIFY(primaryScreen != nullptr);
    QSignalSpy geometryChangedSpy(primaryScreen, &Screen::geometryChanged);
    const auto invoked = QMetaObject::invokeMethod(primaryScreen->internal(),
                                                   "geometryChanged",
                                                   Q_ARG(QRect, primaryScreen->internal()->geometry()));
    QVERIFY(invoked);
    QCOMPARE(geometryChangedSpy.count(), 1);
}
void TestScreensManager::testCheckScreensGeometryStatusReenablesScreen()
{
    ScreensManager manager;
    auto *primaryScreen = manager.primaryScreen();
    QVERIFY(primaryScreen != nullptr);
    QSignalSpy geometryChangedSpy(primaryScreen, &Screen::geometryChanged);
    primaryScreen->setEnable(false);
    QVERIFY(!primaryScreen->enable());
    manager.m_screens.append(nullptr);
    manager.checkScreensGeometryStatus(nullptr);
    QCOMPARE(geometryChangedSpy.count(), 0);
    manager.checkScreensGeometryStatus(primaryScreen);
    QVERIFY(primaryScreen->enable());
    QCOMPARE(geometryChangedSpy.count(), 1);
}
void TestScreensManager::testCheckScreensGeometryStatusDisablesDuplicate()
{
    ScreensManager manager;
    auto *primaryScreen = manager.primaryScreen();
    QVERIFY(primaryScreen != nullptr);
    auto *duplicateScreen = new Screen(primaryScreen->internal(), &manager);
    duplicateScreen->setEnable(true);
    manager.m_screens.append(duplicateScreen);
    QSignalSpy primaryGeometryChangedSpy(primaryScreen, &Screen::geometryChanged);
    QSignalSpy duplicateEnableChangedSpy(duplicateScreen, &Screen::enableChanged);
    manager.checkScreensGeometryStatus(duplicateScreen);
    QVERIFY(!duplicateScreen->enable());
    QCOMPARE(duplicateEnableChangedSpy.count(), 1);
    QCOMPARE(primaryGeometryChangedSpy.count(), 1);
}
void TestScreensManager::testOnScreenAddedTracksPrimaryScreen()
{
    ScreensManager manager;
    auto *screen = QGuiApplication::primaryScreen();
    QVERIFY(screen != nullptr);
    QSignalSpy primaryScreenChangedSpy(&manager, &ScreensManager::primaryScreenChanged);
    const auto initialScreenCount = manager.m_screens.size();
    manager.onScreenAdded(screen);
    QCOMPARE(primaryScreenChangedSpy.count(), 1);
    QCOMPARE(manager.m_screens.size(), initialScreenCount + 1);
    QVERIFY(manager.primaryScreen() != nullptr);
    QCOMPARE(manager.primaryScreen()->internal(), screen);
}

void TestScreensManager::testPrimaryScreen()
{
    auto primaryScreen = ScreensManager().primaryScreen();
    QCOMPARE(primaryScreen->internal(), QGuiApplication::primaryScreen());
}

void TestScreensManager::testDestructor()
{
    // Test destructor by creating and destroying ScreensManager
    ScreensManager *manager = new ScreensManager();
    QVERIFY(manager->primaryScreen() != nullptr);
    delete manager;
    // Destructor will be called, covering destructor branches
}

void TestScreensManager::testEnhancedBranchCoverage()
{
    // Test 1: checkScreensGeometryStatus with null screen
    ScreensManager manager1;
    manager1.checkScreensGeometryStatus(nullptr);
    
    // Test 2: checkScreensGeometryStatus with null primary screen
    ScreensManager manager2;
    manager2.m_primaryScreen = nullptr;
    auto *testScreen = new Screen(nullptr, &manager2);
    manager2.checkScreensGeometryStatus(testScreen);
    
    // Test 3: checkScreensGeometryStatus with null screen in the list
    ScreensManager manager3;
    auto *primaryScreen3 = manager3.primaryScreen();
    manager3.m_screens.append(nullptr);
    manager3.checkScreensGeometryStatus(primaryScreen3);
    
    // Test 4: onPrimaryScreenChanged when screen is the same
    ScreensManager manager4;
    auto *primaryScreen4 = manager4.primaryScreen();
    QSignalSpy primaryScreenChangedSpy(&manager4, &ScreensManager::primaryScreenChanged);
    manager4.onPrimaryScreenChanged(primaryScreen4->internal());
    QVERIFY(primaryScreenChangedSpy.count() == 0);
    
    // Test 5: screens() returns only enabled screens
    ScreensManager manager5;
    auto *primaryScreen5 = manager5.primaryScreen();
    primaryScreen5->setEnable(false);
    QVERIFY(manager5.screens().isEmpty());
    primaryScreen5->setEnable(true);
    QVERIFY(!manager5.screens().isEmpty());
    
    // Test 6: onScreenAdded with multiple screens
    ScreensManager manager6;
    auto initialCount = manager6.m_screens.size();
    QList<QScreen*> allScreens = qApp->screens();
    for (int i = 0; i < allScreens.size() && i < 3; i++) {
        manager6.onScreenAdded(allScreens[i]);
    }
    QVERIFY(manager6.m_screens.size() >= initialCount);
    
    // Test 7: onScreenRemoved at the end of the list
    ScreensManager manager7;
    auto initialCount7 = manager7.m_screens.size();
    QSignalSpy screenRemovedSpy(&manager7, &ScreensManager::screenRemoved);
    if (initialCount7 > 1) {
        auto *lastScreen = manager7.m_screens.last();
        manager7.onScreenRemoved(lastScreen->internal());
        QVERIFY(screenRemovedSpy.count() == 1);
    }
    
    // Test 8: onEnableChanged multiple times
    ScreensManager manager8;
    auto *screen8 = manager8.primaryScreen();
    QSignalSpy screenAddedSpy(&manager8, &ScreensManager::screenAdded);
    QSignalSpy screenRemovedSpy8(&manager8, &ScreensManager::screenRemoved);
    screen8->setEnable(false);
    QVERIFY(screenRemovedSpy8.count() >= 1);
    screen8->setEnable(true);
    QVERIFY(screenAddedSpy.count() >= 1);
    screen8->setEnable(false);
    QVERIFY(screenRemovedSpy8.count() >= 2);
    
    // Test 9: checkScreensGeometryStatus when all screens are disabled
    ScreensManager manager9;
    for (auto *screen : manager9.m_screens) {
        screen->setEnable(false);
    }
    manager9.checkScreensGeometryStatus(manager9.primaryScreen());
}

QTEST_MAIN(TestScreensManager)
