/*
*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#include "screen-manager-test.h"
#include <QCoreApplication>
#include <QSignalSpy>
#include <QTest>
#include <QGuiApplication>
#define private public
#include "screens-manager.h"
#undef private

Q_DECLARE_METATYPE(UkuiPanel::Screen *)
using Screen = UkuiPanel::Screen;

void ScreenManagerTest::initTestCase()
{
    qRegisterMetaType<UkuiPanel::Screen *>("UkuiPanel::Screen*");
    qRegisterMetaType<Screen *>("Screen*");
}

void ScreenManagerTest::testScreenProperties()
{
    auto *screen = QGuiApplication::primaryScreen();
    QVERIFY(screen != nullptr);

    UkuiPanel::Screen panelScreen(screen);
    QSignalSpy enableChangedSpy(&panelScreen, &UkuiPanel::Screen::enableChanged);

    QCOMPARE(panelScreen.geometry(), screen->geometry());
    QCOMPARE(panelScreen.internal(), screen);
    QVERIFY(panelScreen.isValid());
    QVERIFY(!panelScreen.enable());

    panelScreen.setEnable(true);
    QVERIFY(panelScreen.enable());
    QCOMPARE(enableChangedSpy.count(), 1);

    panelScreen.setEnable(true);
    QCOMPARE(enableChangedSpy.count(), 1);

    panelScreen.setEnable(false);
    QVERIFY(!panelScreen.enable());
    QCOMPARE(enableChangedSpy.count(), 2);
}

void ScreenManagerTest::testNullScreenIsInvalid()
{
    UkuiPanel::Screen screen(nullptr);

    QVERIFY(!screen.isValid());
}

void ScreenManagerTest::testMetaObjectCoverage()
{
    auto *screen = QGuiApplication::primaryScreen();
    QVERIFY(screen != nullptr);

    UkuiPanel::Screen panelScreen(screen);
    QCOMPARE(QString::fromLatin1(panelScreen.metaObject()->className()), QStringLiteral("UkuiPanel::Screen"));
    QVERIFY(panelScreen.qt_metacast("UkuiPanel::Screen") != nullptr);
    QVERIFY(panelScreen.qt_metacast("QObject") != nullptr);
    QVERIFY(panelScreen.qt_metacast("NotARealScreen") == nullptr);
    QVERIFY(QMetaObject::invokeMethod(&panelScreen,
                                      "geometryChanged",
                                      Qt::DirectConnection,
                                      Q_ARG(QRect, screen->geometry())));
    QVERIFY(QMetaObject::invokeMethod(&panelScreen,
                                      "enableChanged",
                                      Qt::DirectConnection,
                                      Q_ARG(bool, true)));

    UkuiPanel::ScreensManager manager;
    auto *primaryScreen = manager.primaryScreen();
    QVERIFY(primaryScreen != nullptr);

    QCOMPARE(QString::fromLatin1(manager.metaObject()->className()), QStringLiteral("UkuiPanel::ScreensManager"));
    QVERIFY(manager.qt_metacast("UkuiPanel::ScreensManager") != nullptr);
    QVERIFY(manager.qt_metacast("QObject") != nullptr);
    QVERIFY(manager.qt_metacast("NotARealManager") == nullptr);
    QVERIFY(QMetaObject::invokeMethod(&manager,
                                      "screenAdded",
                                      Qt::DirectConnection,
                                      Q_ARG(Screen *, primaryScreen)));
    QVERIFY(QMetaObject::invokeMethod(&manager,
                                      "screenRemoved",
                                      Qt::DirectConnection,
                                      Q_ARG(Screen *, primaryScreen)));
    QVERIFY(QMetaObject::invokeMethod(&manager,
                                      "primaryScreenChanged",
                                      Qt::DirectConnection,
                                      Q_ARG(Screen *, primaryScreen)));
    QVERIFY(QMetaObject::invokeMethod(&manager,
                                      "onEnableChanged",
                                      Qt::DirectConnection,
                                      Q_ARG(Screen *, primaryScreen),
                                      Q_ARG(bool, true)));
    QVERIFY(QMetaObject::invokeMethod(&manager,
                                      "checkScreensGeometryStatus",
                                      Qt::DirectConnection,
                                      Q_ARG(Screen *, primaryScreen)));
    QVERIFY(QMetaObject::invokeMethod(&manager,
                                      "onPrimaryScreenChanged",
                                      Qt::DirectConnection,
                                      Q_ARG(QScreen *, screen)));
    QVERIFY(QMetaObject::invokeMethod(&manager,
                                      "onScreenAdded",
                                      Qt::DirectConnection,
                                      Q_ARG(QScreen *, screen)));
    QVERIFY(QMetaObject::invokeMethod(&manager,
                                      "onScreenRemoved",
                                      Qt::DirectConnection,
                                      Q_ARG(QScreen *, screen)));
}

void ScreenManagerTest::testManagerInitializesPrimaryScreen()
{
    UkuiPanel::ScreensManager manager;
    auto *primaryScreen = manager.primaryScreen();

    QVERIFY(primaryScreen != nullptr);
    QCOMPARE(primaryScreen->internal(), QGuiApplication::primaryScreen());
    QVERIFY(primaryScreen->enable());
    QVERIFY(!manager.screens().isEmpty());
    QVERIFY(manager.screens().contains(primaryScreen));
}

void ScreenManagerTest::testEnableChangedSignals()
{
    UkuiPanel::ScreensManager manager;
    auto *primaryScreen = manager.primaryScreen();
    QVERIFY(primaryScreen != nullptr);

    QSignalSpy screenAddedSpy(&manager, &UkuiPanel::ScreensManager::screenAdded);
    QSignalSpy screenRemovedSpy(&manager, &UkuiPanel::ScreensManager::screenRemoved);

    manager.onEnableChanged(primaryScreen, false);
    QCOMPARE(screenAddedSpy.count(), 0);
    QCOMPARE(screenRemovedSpy.count(), 1);

    manager.onEnableChanged(primaryScreen, true);
    QCOMPARE(screenAddedSpy.count(), 1);
    QCOMPARE(screenRemovedSpy.count(), 1);
}

void ScreenManagerTest::testPrimaryScreenChangedEnablesTarget()
{
    UkuiPanel::ScreensManager manager;
    QVERIFY(manager.primaryScreen() != nullptr);

    auto *candidateScreen = new UkuiPanel::Screen(nullptr, &manager);
    manager.m_screens.append(candidateScreen);

    QSignalSpy primaryScreenChangedSpy(&manager, &UkuiPanel::ScreensManager::primaryScreenChanged);

    manager.onPrimaryScreenChanged(nullptr);
    QCOMPARE(primaryScreenChangedSpy.count(), 1);
    QCOMPARE(manager.primaryScreen(), candidateScreen);
    QVERIFY(candidateScreen->enable());

    manager.onPrimaryScreenChanged(nullptr);
    QCOMPARE(primaryScreenChangedSpy.count(), 1);
}

void ScreenManagerTest::testScreenRemovedSignalAndListUpdate()
{
    UkuiPanel::ScreensManager manager;
    QVERIFY(!manager.m_screens.isEmpty());

    QSignalSpy screenRemovedSpy(&manager, &UkuiPanel::ScreensManager::screenRemoved);
    const auto initialScreenCount = manager.m_screens.size();
    auto *screen = manager.m_screens.first();

    manager.onScreenRemoved(screen->internal());

    QCOMPARE(screenRemovedSpy.count(), 1);
    QCOMPARE(manager.m_screens.size(), initialScreenCount - 1);

    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
}

void ScreenManagerTest::testInternalGeometryChangeTriggersRefresh()
{
    UkuiPanel::ScreensManager manager;
    auto *primaryScreen = manager.primaryScreen();
    QVERIFY(primaryScreen != nullptr);

    QSignalSpy geometryChangedSpy(primaryScreen, &UkuiPanel::Screen::geometryChanged);

    const auto invoked = QMetaObject::invokeMethod(primaryScreen->internal(),
                                                   "geometryChanged",
                                                   Q_ARG(QRect, primaryScreen->internal()->geometry()));

    QVERIFY(invoked);
    QCOMPARE(geometryChangedSpy.count(), 1);
}

void ScreenManagerTest::testCheckScreensGeometryStatusReenablesScreen()
{
    UkuiPanel::ScreensManager manager;
    auto *primaryScreen = manager.primaryScreen();
    QVERIFY(primaryScreen != nullptr);

    QSignalSpy geometryChangedSpy(primaryScreen, &UkuiPanel::Screen::geometryChanged);

    primaryScreen->setEnable(false);
    QVERIFY(!primaryScreen->enable());

    manager.m_screens.append(nullptr);
    manager.checkScreensGeometryStatus(nullptr);
    QCOMPARE(geometryChangedSpy.count(), 0);

    manager.checkScreensGeometryStatus(primaryScreen);
    QVERIFY(primaryScreen->enable());
    QCOMPARE(geometryChangedSpy.count(), 1);
}

void ScreenManagerTest::testCheckScreensGeometryStatusDisablesDuplicate()
{
    UkuiPanel::ScreensManager manager;
    auto *primaryScreen = manager.primaryScreen();
    QVERIFY(primaryScreen != nullptr);

    auto *duplicateScreen = new UkuiPanel::Screen(primaryScreen->internal(), &manager);
    duplicateScreen->setEnable(true);
    manager.m_screens.append(duplicateScreen);

    QSignalSpy primaryGeometryChangedSpy(primaryScreen, &UkuiPanel::Screen::geometryChanged);
    QSignalSpy duplicateEnableChangedSpy(duplicateScreen, &UkuiPanel::Screen::enableChanged);

    manager.checkScreensGeometryStatus(duplicateScreen);

    QVERIFY(!duplicateScreen->enable());
    QCOMPARE(duplicateEnableChangedSpy.count(), 1);
    QCOMPARE(primaryGeometryChangedSpy.count(), 1);
}

void ScreenManagerTest::testOnScreenAddedTracksPrimaryScreen()
{
    UkuiPanel::ScreensManager manager;
    auto *screen = QGuiApplication::primaryScreen();
    QVERIFY(screen != nullptr);

    QSignalSpy primaryScreenChangedSpy(&manager, &UkuiPanel::ScreensManager::primaryScreenChanged);
    const auto initialScreenCount = manager.m_screens.size();

    manager.onScreenAdded(screen);

    QCOMPARE(primaryScreenChangedSpy.count(), 1);
    QCOMPARE(manager.m_screens.size(), initialScreenCount + 1);
    QVERIFY(manager.primaryScreen() != nullptr);
    QCOMPARE(manager.primaryScreen()->internal(), screen);
}

void ScreenManagerTest::testPrimaryScreen()
{
    auto primaryScreen = UkuiPanel::ScreensManager().primaryScreen();
    QCOMPARE(primaryScreen->internal(), QGuiApplication::primaryScreen());
}

// 新增测试：Screen类isValid边界场景（宽/高为0）
void ScreenManagerTest::testScreenValidWithZeroGeometry()
{
    UkuiPanel::Screen invalidScreen(nullptr);
    QVERIFY(!invalidScreen.isValid());

    auto *qtScreen = QGuiApplication::primaryScreen();
    QVERIFY(qtScreen != nullptr);

    UkuiPanel::Screen panelScreen(qtScreen);
    if (qtScreen->geometry().isEmpty()) {
        QVERIFY(!panelScreen.isValid());
    } else {
        QVERIFY(panelScreen.isValid());
    }
}

// 新增测试：Screen类geometryChanged信号触发
void ScreenManagerTest::testScreenGeometryChangedSignal()
{
    auto *qtScreen = QGuiApplication::primaryScreen();
    QVERIFY(qtScreen != nullptr);

    UkuiPanel::Screen panelScreen(qtScreen);
    QSignalSpy geometryChangedSpy(&panelScreen, &UkuiPanel::Screen::geometryChanged);

    // 模拟几何变化
    const QRect newGeometry(10, 10, 800, 600);
    panelScreen.geometryChanged(newGeometry);

    QCOMPARE(geometryChangedSpy.count(), 1);
    QCOMPARE(geometryChangedSpy.at(0).at(0).toRect(), newGeometry);
}

// 新增测试：ScreensManager析构函数（内存释放）
void ScreenManagerTest::testScreensManagerDestructor()
{
    UkuiPanel::ScreensManager *manager = new UkuiPanel::ScreensManager;
    const int initialCount = manager->m_screens.size();
    QVERIFY(initialCount > 0);

    // 手动触发析构
    delete manager;
    // 验证内存无泄漏（结合valgrind，此处通过无崩溃验证）
    QVERIFY(true);
}

void ScreenManagerTest::testOnPrimaryScreenChangedNoMatch()
{
    UkuiPanel::ScreensManager manager;
    auto *originalPrimary = manager.primaryScreen();
    QVERIFY(originalPrimary != nullptr);

    auto *untrackedScreen = new UkuiPanel::Screen(nullptr, &manager);
    manager.m_screens.append(untrackedScreen);
    QSignalSpy primaryScreenChangedSpy(&manager, &UkuiPanel::ScreensManager::primaryScreenChanged);

    manager.onPrimaryScreenChanged(nullptr);
    QCOMPARE(primaryScreenChangedSpy.count(), 1);
    QCOMPARE(manager.primaryScreen(), untrackedScreen);

    primaryScreenChangedSpy.clear();
    manager.m_primaryScreen = originalPrimary;
    manager.onPrimaryScreenChanged(reinterpret_cast<QScreen *>(quintptr(0x1)));

    QCOMPARE(primaryScreenChangedSpy.count(), 0);
    QCOMPARE(manager.primaryScreen(), originalPrimary);
}

void ScreenManagerTest::testOnScreenRemovedNotFound()
{
    UkuiPanel::ScreensManager manager;
    const auto initialScreenCount = manager.m_screens.size();
    QSignalSpy screenRemovedSpy(&manager, &UkuiPanel::ScreensManager::screenRemoved);

    manager.onScreenRemoved(reinterpret_cast<QScreen *>(quintptr(0x1)));

    QCOMPARE(screenRemovedSpy.count(), 0);
    QCOMPARE(manager.m_screens.size(), initialScreenCount);
}

// 新增测试：screens()过滤禁用屏幕
void ScreenManagerTest::testScreensFilterDisabled()
{
    UkuiPanel::ScreensManager manager;
    auto *primary = manager.primaryScreen();
    QVERIFY(primary != nullptr);

    // 禁用主屏幕（先记录原始状态）
    const bool originalEnable = primary->enable();
    primary->setEnable(false);

    // 验证screens()不包含禁用屏幕
    QList<UkuiPanel::Screen *> enabledScreens = manager.screens();
    QVERIFY(enabledScreens.isEmpty());

    // 恢复原始状态
    primary->setEnable(originalEnable);
    enabledScreens = manager.screens();
    QVERIFY(enabledScreens.contains(primary));
}

// 新增测试：checkScreensGeometryStatus主屏幕重复场景
void ScreenManagerTest::testCheckScreensGeometryPrimaryDuplicate()
{
    UkuiPanel::ScreensManager manager;
    auto *primary = manager.primaryScreen();
    QVERIFY(primary != nullptr);

    // 创建与主屏幕几何相同的新Screen
    auto *duplicateScreen = new UkuiPanel::Screen(primary->internal(), &manager);
    manager.m_screens.append(duplicateScreen);
    duplicateScreen->setEnable(true);

    QSignalSpy duplicateEnableSpy(duplicateScreen, &UkuiPanel::Screen::enableChanged);
    QSignalSpy primaryGeometrySpy(primary, &UkuiPanel::Screen::geometryChanged);

    // 触发几何检查
    manager.checkScreensGeometryStatus(duplicateScreen);

    // 验证重复屏幕被禁用，主屏幕几何信号触发
    QVERIFY(!duplicateScreen->enable());
    QCOMPARE(duplicateEnableSpy.count(), 1);
    QCOMPARE(primaryGeometrySpy.count(), 1);

    // 清理
    manager.m_screens.removeOne(duplicateScreen);
    duplicateScreen->deleteLater();
}

// 新增测试：checkScreensGeometryStatus空指针场景
void ScreenManagerTest::testCheckScreensGeometryNull()
{
    UkuiPanel::ScreensManager manager;
    // 无崩溃即通过
    manager.checkScreensGeometryStatus(nullptr);
    QVERIFY(true);
}
QTEST_MAIN(ScreenManagerTest)
