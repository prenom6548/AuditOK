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

#ifndef TEST_SEARCHFRIEND_DELEGATE_H
#define TEST_SEARCHFRIEND_DELEGATE_H

#include <QObject>
#include <QtTest>
#include "view/friendlist/homepagesearch/searchfriend_delegate.h"

class TestSearchFriendDelegate : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testConstructorDestructor();
    void testSizeHint();
    void testPaint();
    void testPaintStatesAndThemes();

private:
    SearchFriendDelegate *m_delegate;
};

#endif // TEST_SEARCHFRIEND_DELEGATE_H
