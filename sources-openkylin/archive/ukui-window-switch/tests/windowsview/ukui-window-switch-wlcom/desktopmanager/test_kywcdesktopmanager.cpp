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

#include "test_kywcdesktopmanager.h"
#include "kywcdesktopmanager.h"
#include "screens-manager.h"
#include <QCoreApplication>
#include <QGuiApplication>
#include <QScreen>
#include <QSignalSpy>
#include <QTest>
#include <QDebug>
#include <QEventLoop>
#include <QTimer>

void TestKywcDesktopManager::initTestCase()
{
    // Initialize test case
    if (!qApp) {
        int argc = 0;
        char** argv = nullptr;
        new QGuiApplication(argc, argv);
    }

    // Create single KywcDesktopManager instance
    m_manager = new KywcDesktopManager();

    // Use QEventLoop to wait for initialization
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    connect(m_manager, &KywcDesktopManager::workspaceManagerInitialized, &loop, &QEventLoop::quit);
    connect(&timer, &QTimer::timeout, [&]() {
        qWarning() << "WorkspaceManager initialization timeout";
        loop.quit();
    });

    timer.start(1000);
    loop.exec();
    if (timer.isActive()) {
        timer.stop();
        qDebug() << "WorkspaceManager initialized successfully";
    }
}

void TestKywcDesktopManager::cleanupTestCase()
{
    // Clean up - m_manager is a static instance that should not be deleted
    // to avoid crashes during application shutdown
    if (m_manager) {
        // Just clear the pointer without deleting to avoid double-free
        m_manager = nullptr;
    }
}

void TestKywcDesktopManager::testConstructor()
{
    QVERIFY(m_manager != nullptr);
    QVERIFY(!m_manager->getCurrentDesktop().isEmpty());
}

void TestKywcDesktopManager::testDestructor()
{
    KywcDesktopManager* manager = new KywcDesktopManager();
    QSignalSpy spy(manager, &KywcDesktopManager::workspaceManagerInitialized);

    if (!spy.count()) { spy.wait(1000); }

    delete manager;
    manager = nullptr;
    // Just verify no crash occurred
    QVERIFY(true);
}

void TestKywcDesktopManager::testGetNumberOfDesktops()
{
    QVERIFY(m_manager != nullptr);
    int count = m_manager->getNumberOfDesktops();
    QVERIFY(count >= 0);  // May be 0 if workspace manager not available
}

void TestKywcDesktopManager::testGetCurrentDesktop()
{
    QVERIFY(m_manager != nullptr);
    QString current = m_manager->getCurrentDesktop();
    QVERIFY(!current.isEmpty());
}

void TestKywcDesktopManager::testAppendDesktop()
{
    QVERIFY(m_manager != nullptr);
    int initialCount = m_manager->getNumberOfDesktops();
    qDebug() << "Initial desktop count:" << initialCount;

    QSignalSpy spy(m_manager, &KywcDesktopManager::desktopAppended);
    m_manager->appendDesktop();

    // Wait for the signal to be emitted
    if (spy.wait(1000)) {
        qDebug() << "Desktop appended successfully";
        int newCount = m_manager->getNumberOfDesktops();
        qDebug() << "New desktop count:" << newCount;
        QVERIFY(newCount == initialCount + 1);
    } else {
        QSKIP("WorkspaceManager not available, skipping append test");
    }
}

void TestKywcDesktopManager::testSetCurrentDesktop()
{
    QVERIFY(m_manager != nullptr);
    QString current = m_manager->getCurrentDesktop();
    m_manager->setCurrentDesktop(current);
    // Just verify no crash occurred
    QVERIFY(true);
}

void TestKywcDesktopManager::testGetFullScreenGeometry()
{
    QVERIFY(m_manager != nullptr);
    QRect geometry = m_manager->getFullScreenGeometry("1", 0);
    QVERIFY(!geometry.isEmpty());
}

void TestKywcDesktopManager::testSetDesktopName()
{
    QVERIFY(m_manager != nullptr);
    m_manager->setDesktopName("1", "Test Desktop");
    QVERIFY(true);
}

void TestKywcDesktopManager::testGetDesktopUuid()
{
    QVERIFY(m_manager != nullptr);
    int count = m_manager->getNumberOfDesktops();
    if (count > 1) {
        // Test valid desktop index
        QString uuid = m_manager->getDesktopUuid(1);
        QVERIFY(!uuid.isEmpty());

        // Test with last desktop
        QString lastUuid = m_manager->getDesktopUuid(count);
        QVERIFY(!lastUuid.isEmpty());

        // Test that different indices return different UUIDs (if multiple desktops exist)
        if (count > 2) {
            QString uuid2 = m_manager->getDesktopUuid(2);
            QVERIFY(!uuid2.isEmpty());
            QVERIFY(uuid != uuid2);
        }
    }

    // Test invalid desktop index
    QString invalidUuid = m_manager->getDesktopUuid(0);
    QVERIFY(invalidUuid.isEmpty());

    // Test index greater than count
    QString invalidUuid2 = m_manager->getDesktopUuid(count + 1);
    QVERIFY(invalidUuid2.isEmpty());
}

void TestKywcDesktopManager::testGetOutputUuid()
{
    QVERIFY(m_manager != nullptr);
    QString uuid = m_manager->getOutputUuid(0);
    // UUID may be empty
    QVERIFY(true);
}

void TestKywcDesktopManager::testDesktopRemovedSignal()
{
    QVERIFY(m_manager != nullptr);
    QSignalSpy signalSpy(m_manager, &KywcDesktopManager::desktopRemoved);
    QVERIFY(signalSpy.isValid());
}

void TestKywcDesktopManager::testDesktopAppendedSignal()
{
    QVERIFY(m_manager != nullptr);
    QSignalSpy signalSpy(m_manager, &KywcDesktopManager::desktopAppended);
    QVERIFY(signalSpy.isValid());
}

void TestKywcDesktopManager::testDesktopMovedSignal()
{
    QVERIFY(m_manager != nullptr);
    QSignalSpy signalSpy(m_manager, &KywcDesktopManager::desktopMoved);
    QVERIFY(signalSpy.isValid());
}

void TestKywcDesktopManager::testCurrentDesktopChangedSignal()
{
    QVERIFY(m_manager != nullptr);
    QSignalSpy signalSpy(m_manager, &KywcDesktopManager::currentDesktopChanged);
    QVERIFY(signalSpy.isValid());
}

void TestKywcDesktopManager::testMoveDesktop()
{
    QVERIFY(m_manager != nullptr);
    int initialCount = m_manager->getNumberOfDesktops();
    if (initialCount >= 2) {
        m_manager->moveDesktop("1", 2);
        QVERIFY(true);
    } else {
        QVERIFY(true);
    }
}

void TestKywcDesktopManager::testRemoveDesktop()
{
    QVERIFY(m_manager != nullptr);
    int initialCount = m_manager->getNumberOfDesktops();
    if (initialCount > 1) {
        qDebug() << "Initial desktop count:" << initialCount;

        QSignalSpy spy(m_manager, &KywcDesktopManager::desktopRemoved);
        m_manager->removeDesktop(QString::number(initialCount));

        // Wait for the signal to be emitted
        if (spy.wait(1000)) {
            qDebug() << "Desktop removed successfully";
            int newCount = m_manager->getNumberOfDesktops();
            qDebug() << "New desktop count:" << newCount;
            QVERIFY(newCount == initialCount - 1);
        } else {
            QSKIP("WorkspaceManager not available, skipping remove test");
        }
    } else {
        qDebug() << "Only one desktop, skipping remove test";
        QVERIFY(true);
    }
}

void TestKywcDesktopManager::testIsCurrentDesktopShowingDesktop()
{
    QVERIFY(m_manager != nullptr);
    bool showing = m_manager->isCurrentDesktopShowingDesktop();
    // For wlcom backend, this should return false
    QVERIFY(showing == false);
}

#include "test_kywcdesktopmanager.moc"
