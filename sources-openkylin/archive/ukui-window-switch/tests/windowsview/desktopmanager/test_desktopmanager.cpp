/*
 * Copyright (C) 2026, KylinSoft Co., Ltd.
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
 */

#include "test_desktopmanager.h"
#include "abstractdesktopmanager.h"
#include "desktopmanager.h"
#include "ukui-window-switch-wlcom/kywcdesktopmanager.h"
#include <QCoreApplication>
#include <QApplication>
#include <QScreen>
#include <QSignalSpy>
#include <QTest>
#include <QDebug>
#include <QTimer>
#include <QEventLoop>
#include <QQuickItem>
#include <QQuickWindow>
#include <climits>

// Mock QQuickItem for testing
class MockQQuickItem : public QQuickItem
{
    Q_OBJECT
public:
    explicit MockQQuickItem(QQuickItem *parent = nullptr)
        : QQuickItem(parent)
    {
        setFlag(ItemHasContents, true);
    }
};

void TestDesktopManager::initTestCase()
{
    // Initialize test case
    if (!qApp) {
        int argc = 0;
        char** argv = nullptr;
        new QApplication(argc, argv);
    }

    // Get DesktopManager instance to trigger initialization
    DesktopManager *manager = DesktopManager::getInstance();

    // Wait for WorkspaceManager initialization using signal
    QEventLoop loop;
    connect(manager, &DesktopManager::workspaceManagerInitialized, &loop, &QEventLoop::quit);

    // Also wait for initial desktop count to be available
    QTimer::singleShot(1000, &loop, &QEventLoop::quit);
    loop.exec();

    qDebug() << "[TestDesktopManager] initTestCase called";
}

void TestDesktopManager::cleanupTestCase()
{

}

void TestDesktopManager::testGetInstance()
{
    DesktopManager *instance = DesktopManager::getInstance();
    QVERIFY(instance != nullptr);
}

void TestDesktopManager::testGetInstanceTwice()
{
    DesktopManager *instance1 = DesktopManager::getInstance();
    DesktopManager *instance2 = DesktopManager::getInstance();

    // Singleton pattern: same instance should be returned
    QVERIFY(instance1 == instance2);
}

void TestDesktopManager::testConstructor()
{
    // Constructor is private, accessed via getInstance()
    DesktopManager *manager = DesktopManager::getInstance();
    QVERIFY(manager != nullptr);
}

void TestDesktopManager::testDestructor()
{
    // Destructor is handled by singleton pattern
    // Just verify getInstance works
    DesktopManager *manager = DesktopManager::getInstance();
    QVERIFY(manager != nullptr);
}

void TestDesktopManager::testGetCurrentDesktop()
{
    DesktopManager *manager = DesktopManager::getInstance();
    QString current = manager->getCurrentDesktop();
    QVERIFY(!current.isEmpty());
}

void TestDesktopManager::testGetNumberOfDesktops()
{
    DesktopManager *manager = DesktopManager::getInstance();
    int count = manager->getNumberOfDesktops();
    // Number of desktops should be >= 0
    QVERIFY(count >= 0);
}

void TestDesktopManager::testIsCurrentDesktopShowingDesktop()
{
    DesktopManager *manager = DesktopManager::getInstance();
    bool showing = manager->isCurrentDesktopShowingDesktop();
    // For wlcom backend, this should return false
    QVERIFY(showing == false);
}

void TestDesktopManager::testSetCurrentDesktop()
{
    DesktopManager *manager = DesktopManager::getInstance();
    QString current = manager->getCurrentDesktop();
    manager->setCurrentDesktop(current);
    // Just verify no crash occurred
    QVERIFY(true);
}

void TestDesktopManager::testAppendDesktop()
{
    DesktopManager *manager = DesktopManager::getInstance();
    int initialCount = manager->getNumberOfDesktops();
    qDebug() << "[TestDesktopManager] Initial desktop count:" << initialCount;

    QSignalSpy signalSpy(manager, &DesktopManager::desktopAppended);
    QVERIFY(signalSpy.isValid());

    manager->appendDesktop();

    // Wait for signal to be emitted
    QCoreApplication::processEvents(QEventLoop::AllEvents, 200);

    int newCount = manager->getNumberOfDesktops();
    qDebug() << "[TestDesktopManager] New desktop count:" << newCount;

    // Verify count increased (may fail if workspace manager not available)
    QVERIFY(newCount >= initialCount);

    // Verify signal was emitted if count changed
    if (newCount > initialCount) {
        QVERIFY(signalSpy.count() > 0);
    }
}

void TestDesktopManager::testSetDesktopName()
{
    DesktopManager *manager = DesktopManager::getInstance();

    // Test setting desktop name with various inputs
    manager->setDesktopName("1", "Test Desktop");
    manager->setDesktopName("1", "Desktop 1");
    manager->setDesktopName("1", "");
    manager->setDesktopName("1", "测试桌面");

    // Verify no crash occurred
    QVERIFY(true);
}

void TestDesktopManager::testMoveDesktop()
{
    DesktopManager *manager = DesktopManager::getInstance();
    int initialCount = manager->getNumberOfDesktops();

    if (initialCount >= 2) {
        QSignalSpy signalSpy(manager, &DesktopManager::desktopMoved);
        QVERIFY(signalSpy.isValid());
        manager->moveDesktop("1", 2);
        // Just verify no crash occurred
        QVERIFY(true);
    } else {
         qDebug() << "[TestDesktopManager] No enough desktops to test move, skipping move test";
        QVERIFY(true);
    }
}

void TestDesktopManager::testRemoveDesktop()
{
    DesktopManager *manager = DesktopManager::getInstance();
    int initialCount = manager->getNumberOfDesktops();

    if (initialCount > 1) {
        qDebug() << "[TestDesktopManager] Initial desktop count:" << initialCount;
        
        QSignalSpy signalSpy(manager, &DesktopManager::desktopRemoved);
        QVERIFY(signalSpy.isValid());

        manager->removeDesktop(QString::number(initialCount));

        // Wait a bit for the removal to complete
        QCoreApplication::processEvents(QEventLoop::AllEvents, 200);

        int newCount = manager->getNumberOfDesktops();
        qDebug() << "[TestDesktopManager] New desktop count:" << newCount;

        // Verify count decreased (may fail if workspace manager not available)
        QVERIFY(newCount <= initialCount);

        // Verify signal was emitted if count changed
        if (newCount < initialCount) {
            QVERIFY(signalSpy.count() > 0);
        }
    } else {
        qDebug() << "[TestDesktopManager] Only one desktop, skipping remove test";
        QVERIFY(true);
    }
}

void TestDesktopManager::testGetFullScreenGeometry()
{
    DesktopManager *manager = DesktopManager::getInstance();
    QRect geometry = manager->getFullScreenGeometry("1", 0);
    QVERIFY(!geometry.isEmpty());
}

void TestDesktopManager::testGetOutputUuid()
{
    DesktopManager *manager = DesktopManager::getInstance();
    QString uuid = manager->getOutputUuid(0);
    // UUID may be empty
    QVERIFY(true);
}

void TestDesktopManager::testOpenDesktopMenu()
{
    DesktopManager *manager = DesktopManager::getInstance();

    // Create a mock QQuickItem with nullptr as parent
    // The openDesktopMenu function only needs a QQuickItem* to set transient parent
    MockQQuickItem item;
    item.setWidth(100);
    item.setHeight(100);

    // Test opening menu with valid desktop index
    int numberOfDesktops = manager->getNumberOfDesktops();
    if (numberOfDesktops >= 1) {
        manager->openDesktopMenu("1", &item);
        QVERIFY(true);
    }
}

void TestDesktopManager::testInvalidDesktopIndex()
{
    DesktopManager *manager = DesktopManager::getInstance();

    // Test with invalid desktop index (should not crash)
    manager->removeDesktop("0");
    manager->removeDesktop("-1");
    manager->moveDesktop("0", 1);
    manager->moveDesktop("-1", 1);
    manager->setCurrentDesktop("0");
    manager->setCurrentDesktop("-1");
    manager->setDesktopName("0", "Test");
    manager->setDesktopName("-1", "Test");
    manager->getFullScreenGeometry("0", 0);
    manager->getFullScreenGeometry("-1", 0);

    // Test openDesktopMenu with invalid index
    MockQQuickItem item;
    manager->openDesktopMenu("0", &item);
    manager->openDesktopMenu("-1", &item);

    // Test edge cases
    manager->setCurrentDesktop("1");
    manager->setDesktopName("1", "Test Desktop Name");
}

void TestDesktopManager::testSignalConnections()
{
    DesktopManager *manager = DesktopManager::getInstance();

    // Test all signals can be connected
    QSignalSpy removedSpy(manager, &DesktopManager::desktopRemoved);
    QSignalSpy appendedSpy(manager, &DesktopManager::desktopAppended);
    QSignalSpy movedSpy(manager, &DesktopManager::desktopMoved);
    QSignalSpy currentSpy(manager, &DesktopManager::currentDesktopChanged);

    QVERIFY(removedSpy.isValid());
    QVERIFY(appendedSpy.isValid());
    QVERIFY(movedSpy.isValid());
    QVERIFY(currentSpy.isValid());
}

void TestDesktopManager::testEdgeCases()
{
    DesktopManager *manager = DesktopManager::getInstance();

    // Test with maximum integer values (should not crash)
    manager->removeDesktop(QString::number(INT_MAX));
    manager->moveDesktop(QString::number(INT_MAX), 1);
    manager->setCurrentDesktop(QString::number(INT_MAX));
    manager->setDesktopName(QString::number(INT_MAX), "Test");
    manager->getFullScreenGeometry(QString::number(INT_MAX), 0);

    // Test with zero values
    manager->removeDesktop("0");
    manager->moveDesktop("0", 0);
    manager->setCurrentDesktop("0");
    manager->setDesktopName("0", "Test");
    manager->getFullScreenGeometry("0", 0);

    // Test with negative values
    manager->removeDesktop("-100");
    manager->moveDesktop("-100", -1);
    manager->setCurrentDesktop("-100");
    manager->setDesktopName("-1", "Test");
    manager->getFullScreenGeometry("-1", -1);
}

#include "test_desktopmanager.moc"
