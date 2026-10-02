/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 or
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

#ifndef MOCK_MULTITASKVIEWMANAGERBYWLCOM_H
#define MOCK_MULTITASKVIEWMANAGERBYWLCOM_H

#include <QObject>
#include <QDebug>

// Minimal stub that provides the interface needed by DBusService
// This is used to test D-Bus registration without requiring heavy QML initialization
// Note: This inherits from QObject only, not from MultitaskViewManagerByWlcom
// Because the real MultitaskViewManagerByWlcom requires QML environment that is not 
// available in unit test context, we use this stub and rely on static type casting
// for testing D-Bus registration functionality
class StubMultitaskViewManagerByWlcom : public QObject {
    Q_OBJECT

public:
    explicit StubMultitaskViewManagerByWlcom(QObject *parent = nullptr) 
        : QObject(parent), m_openCalled(false), m_closeCalled(false) {}

    void open() {
        m_openCalled = true;
        qDebug() << "StubMultitaskViewManagerByWlcom::open() called";
    }

    void close() {
        m_closeCalled = true;
        qDebug() << "StubMultitaskViewManagerByWlcom::close() called";
    }

    bool wasOpenCalled() const { return m_openCalled; }
    bool wasCloseCalled() const { return m_closeCalled; }
    void reset() { m_openCalled = false; m_closeCalled = false; }

private:
    bool m_openCalled;
    bool m_closeCalled;
};

#endif // MOCK_MULTITASKVIEWMANAGERBYWLCOM_H