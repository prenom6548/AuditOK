/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 */
#ifndef TEST_VIEWCONTROL_H
#define TEST_VIEWCONTROL_H

#include <QtTest>
#include <QSignalSpy>
#include <QObject>
#include "controller/core/viewcontrol.h"

class TestViewControl : public QObject
{
    Q_OBJECT

public:
    TestViewControl();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testGetInstance();
    void testUpdateIsStart();
    void testInitCore();
    void testActiveWindow();
    void testAutoStart();
    void testToShowView();
    void testShowView();
    void testShowInCenter();

private:
    viewControl *m_viewControl;
};

#endif // TEST_VIEWCONTROL_H