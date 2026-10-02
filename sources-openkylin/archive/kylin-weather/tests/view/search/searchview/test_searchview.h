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
#ifndef TEST_SEARCHVIEW_H
#define TEST_SEARCHVIEW_H

#include <QObject>
#include <QApplication>
#include <QtTest/QtTest>
#include "view/search/searchview.h"

class TestSearchView : public QObject
{
    Q_OBJECT

public:
    explicit TestSearchView(QObject *parent = nullptr);

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testConstructor();
    void testPublicMethods();
    void testEvents();
    void testSignals();
    void testLayoutLogic();
    void testSearchListUi();
    void testSendSearchInput();
    void testAddCollectCity();
    void testChangeCitySlots();
    void testChangeStackedWidgetLimit();

private:
    SearchView *m_searchView;
};

#endif // TEST_SEARCHVIEW_H
