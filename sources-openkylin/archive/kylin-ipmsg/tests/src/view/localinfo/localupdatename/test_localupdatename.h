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

#ifndef TEST_LOCALUPDATENAME_H
#define TEST_LOCALUPDATENAME_H

#include <QtTest/QtTest>
#include <QObject>
#include "localupdatename.h"

class TestLocalUpdateName : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testConstructorLocal();
    void testConstructorFriend();
    void testSetInitData();
    void testSetWidgetUi();
    void testSetWidgetStyle();
    void testKeyPressEvent();
    void testSlotConfirmBtn();
    void testSlotCancelBtn();
    void testSlotEnableSureBtn();
    void testCoverageAdditions();

private:
    LocalUpdateName *m_updateName;
};

#endif // TEST_LOCALUPDATENAME_H
