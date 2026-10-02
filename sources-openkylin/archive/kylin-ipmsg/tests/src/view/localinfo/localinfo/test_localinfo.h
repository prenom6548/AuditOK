/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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

#ifndef TEST_LOCALINFO_H
#define TEST_LOCALINFO_H

#include <QtTest/QtTest>
#include <QObject>
#include <QApplication>
#include <QSignalSpy>
#include <QTimer>
#include <QDir>
#include <QPaintEvent>

#include "../../../../../../src/view/localinfo/localinfo.h"

class TestLocalInfo : public QObject
{
    Q_OBJECT

public:
    TestLocalInfo(QObject *parent = nullptr);

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testConstructor();
    void testGetInstance();
    void testSetWidgetUi();
    void testSetWidgetStyle();
    void testShowList();
    void testChangeTheme();
    void testDisplayInfo();
    void testGetIniText();
    void testCheckLocalName();
    void testGetLocalAvatar();
    void testChangeFontSize();
    void testSlotChangePCSize();
    void testSlotChangeFlatHSize();
    void testSlotChangeFlatVSize();
    void testPaintEvent();
    void testEvent();
    void testEventFilter();
    void testPrivateSlots();
    void testEventFilters();
    void testCheckLocalNameTimer();
    void testCoverageAdditions();

private:
    int argc = 1;
    char *argv[1] = {const_cast<char *>("test_localinfo")};
    QApplication *m_app;
    LocalInfo *m_localInfo;
};

#endif // TEST_LOCALINFO_H