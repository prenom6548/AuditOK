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

#ifndef TEST_TRAYITEMDELEGATE_H
#define TEST_TRAYITEMDELEGATE_H

#include <QtTest/QtTest>
#include <QObject>
#include "trayitemdelegate.h"

class TestTrayItemDelegate : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testConstructor();
    void testSizeHint();
    void testPaint();
    void testCoverageAdditions();

private:
    TrayItemDelegate *m_delegate;
};

#endif // TEST_TRAYITEMDELEGATE_H
