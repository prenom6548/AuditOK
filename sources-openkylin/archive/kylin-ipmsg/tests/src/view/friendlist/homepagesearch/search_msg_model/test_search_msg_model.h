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

#ifndef TEST_SEARCH_MSG_MODEL_H
#define TEST_SEARCH_MSG_MODEL_H

#include <QObject>
#include <QtTest>
#include "view/friendlist/homepagesearch/search_msg_model.h"

class TestSearchMsgModel : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testConstructorDestructor();
    void testGetInstance();
    void testSlotSelect();
    void testGetModel();
    void testGetSearchResult();

private:
    SearchMsgModel *m_model;
};

#endif // TEST_SEARCH_MSG_MODEL_H
