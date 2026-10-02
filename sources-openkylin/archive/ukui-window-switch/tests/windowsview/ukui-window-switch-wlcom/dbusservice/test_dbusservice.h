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

#ifndef TEST_DBUSSERVICE_H
#define TEST_DBUSSERVICE_H

#include <QObject>
#include <QTest>
#include <QSignalSpy>

// Include the minimal stub for MultitaskViewManagerByWlcom
#include "minimal_multitaskviewmanager.h"

// Include the actual DBusService header
#include "dbusservice.h"

class TestDBusService : public QObject
{
    Q_OBJECT

public:
    TestDBusService();
    ~TestDBusService();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Test constructor - creates DBusService and registers D-Bus service
    void testConstructor();
    
    // Test destructor - should stop service properly
    void testDestructor();
    
    // Test show() - should call m_viewManager->open()
    void testShow();
    
    // Test hide() - should call m_viewManager->close()
    void testHide();
    
    // Test D-Bus service name property
    void testDBusServiceName();
    
    // Test D-Bus service path property  
    void testDBusServicePath();

    // Test startService and stopService indirectly via constructor/destructor
    void testServiceRegistration();

    // Test multiple show/hide calls
    void testMultipleShowHide();

private:
    class DBusService *m_dbusService;
    MultitaskViewManagerByWlcom *m_manager;
};

#endif // TEST_DBUSSERVICE_H