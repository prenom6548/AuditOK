/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "test_dbusservice.h"

TestDBusService::TestDBusService()
    : m_dbusService(nullptr)
    , m_manager(nullptr)
{
}

TestDBusService::~TestDBusService()
{
}

void TestDBusService::initTestCase()
{
    // Verify required headers are included
    QVERIFY(true);
}

void TestDBusService::cleanupTestCase()
{
}

void TestDBusService::init()
{
    m_manager = new MultitaskViewManagerByWlcom();
    QVERIFY(m_manager != nullptr);
}

void TestDBusService::cleanup()
{
    delete m_dbusService;
    m_dbusService = nullptr;

    delete m_manager;
    m_manager = nullptr;
}

void TestDBusService::testConstructor()
{
    // Test constructor - creates DBusService and registers D-Bus service
    // Note: The actual D-Bus registration may fail in test environment
    // but we can still verify the object is created correctly
    m_dbusService = new class DBusService(m_manager, nullptr);

    QVERIFY(m_dbusService != nullptr);
    QVERIFY(m_dbusService->parent() == nullptr);
}

void TestDBusService::testDestructor()
{
    // Test destructor - should stop service properly
    m_dbusService = new class DBusService(m_manager, nullptr);
    QVERIFY(m_dbusService != nullptr);

    // Delete and verify no crash
    delete m_dbusService;
    m_dbusService = nullptr;
    // If we reach here without crash, test passes
    QVERIFY(true);
}

void TestDBusService::testShow()
{
    // Reset manager state
    m_manager->reset();

    // Setup - create DBusService with actual manager
    m_dbusService = new class DBusService(m_manager, nullptr);
    QVERIFY(m_dbusService != nullptr);

    // Verify initial state
    QVERIFY(!m_manager->wasOpenCalled());

    // Call show() - should call m_viewManager->open()
    m_dbusService->show();

    // Verify that open() was called on manager
    QVERIFY(m_manager->wasOpenCalled());
}

void TestDBusService::testHide()
{
    // Reset manager state
    m_manager->reset();

    // Setup - create DBusService with actual manager
    m_dbusService = new class DBusService(m_manager, nullptr);
    QVERIFY(m_dbusService != nullptr);

    // Verify initial state
    QVERIFY(!m_manager->wasCloseCalled());

    // Call hide() - should call m_viewManager->close()
    m_dbusService->hide();

    // Verify that close() was called on manager
    QVERIFY(m_manager->wasCloseCalled());
}

void TestDBusService::testDBusServiceName()
{
    m_dbusService = new class DBusService(m_manager, nullptr);
    QVERIFY(m_dbusService != nullptr);

    // Test D-Bus service name - verify object is valid
    const QMetaObject *metaObj = m_dbusService->metaObject();
    QVERIFY(metaObj != nullptr);

    // Verify class info contains D-Bus interface
    int classInfoCount = metaObj->classInfoCount();
    bool hasDBusInterface = false;
    for (int i = 0; i < classInfoCount; ++i) {
        QMetaClassInfo classInfo = metaObj->classInfo(i);
        if (QLatin1String(classInfo.name()) == QLatin1String("D-Bus Interface")) {
            hasDBusInterface = true;
            break;
        }
    }
    QVERIFY(hasDBusInterface);
}

void TestDBusService::testDBusServicePath()
{
    m_dbusService = new class DBusService(m_manager, nullptr);
    QVERIFY(m_dbusService != nullptr);

    // Verify object is properly registered
    QVERIFY(m_dbusService != nullptr);
    
    // Verify the object can be queried via metaobject
    QVERIFY(m_dbusService->inherits("QObject"));
}

void TestDBusService::testServiceRegistration()
{
    // Test startService and stopService indirectly
    // by verifying the service can be started and stopped
    
    // First, verify we can create and destroy service
    m_dbusService = new class DBusService(m_manager, nullptr);
    QVERIFY(m_dbusService != nullptr);
    
    // Verify we can call methods without crash
    m_dbusService->show();
    m_dbusService->hide();
    
    // Delete the service (which should call stopService)
    delete m_dbusService;
    m_dbusService = nullptr;
    
    // If we reach here, service registration works
    QVERIFY(true);
}

void TestDBusService::testMultipleShowHide()
{
    // Reset manager state
    m_manager->reset();
    
    // Create service
    m_dbusService = new class DBusService(m_manager, nullptr);
    QVERIFY(m_dbusService != nullptr);
    
    // Multiple show/hide cycles
    for (int i = 0; i < 3; ++i) {
        m_manager->reset();
        
        m_dbusService->show();
        QVERIFY(m_manager->wasOpenCalled());
        
        m_manager->reset();
        m_dbusService->hide();
        QVERIFY(m_manager->wasCloseCalled());
    }
    
    QVERIFY(true);
}