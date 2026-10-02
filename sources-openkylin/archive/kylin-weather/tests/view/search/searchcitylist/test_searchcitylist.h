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
#ifndef TEST_SEARCHCITYLIST_H
#define TEST_SEARCHCITYLIST_H

#include <QObject>
#include <QTableView>
#include <QStandardItemModel>
#include <QLabel>
#include "view/search/searchcitylist.h"

class TestSearchCityList : public QObject
{
    Q_OBJECT

public:
    explicit TestSearchCityList(QObject *parent = nullptr);

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // 测试用例
    void testConstructor();
    void testSearchUiChange_data();
    void testSearchUiChange();
    void testSetViewModel_data();
    void testSetViewModel();
    void testSignals();

private:
    SearchCityLIst *m_searchCityList;
    QStandardItemModel *m_model;
};

#endif // TEST_SEARCHCITYLIST_H