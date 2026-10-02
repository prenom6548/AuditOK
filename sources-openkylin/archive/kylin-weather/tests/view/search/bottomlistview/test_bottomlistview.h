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
#ifndef TEST_BOTTOMLISTVIEW_H
#define TEST_BOTTOMLISTVIEW_H

#include <QObject>
#include <QtTest/QtTest>
#include "view/search/bottomlistview.h"

class TestBottomListView : public QObject
{
    Q_OBJECT
public:
    explicit TestBottomListView(QObject *parent = nullptr);

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testConstructor();
    void testSetModelforView();
    void testSetModelforView_MultipleCalls();
    void testSlotChangeCityFromTitleSearchList();
    void testSlotChangeCityFromTitleSearchList_NoInfo();
    void testSlotChangeCityFromTitleSearchList_Types();
    void testSlotShowBottomList();
    void testWidgetStyle();

private:
    BottomListView *m_view;
};

#endif // TEST_BOTTOMLISTVIEW_H
