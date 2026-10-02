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
#ifndef TEST_XATOMHELPER_H
#define TEST_XATOMHELPER_H

#include <QObject>
#include <QApplication>
#include <QTest>
#include <QSignalSpy>

class TestXAtomHelper : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testGetInstance();
    void testIsFrameLessWindow();
    void testIsWindowDecorateBorderOnly();
    void testIsWindowMotifHintDecorateBorderOnly();
    void testIsUKUICsdSupported();
    void testIsUKUIDecorationWindow();
    void testGetWindowBorderRadius();
    void testSetWindowBorderRadiusWithStruct();
    void testSetWindowBorderRadiusWithParams();
    void testSetUKUIDecoraiontHint();
    void testSetWindowMotifHint();
    void testGetWindowMotifHint();
};

#endif // TEST_XATOMHELPER_H
