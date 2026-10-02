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
#ifndef TEST_ERRORREACT_H
#define TEST_ERRORREACT_H

#include <QObject>
#include <QtTest>
#include <QSignalSpy>
#include "../../../controller/core/errorreact.h"

class TestErrorReact : public QObject
{
    Q_OBJECT

public:
    TestErrorReact();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testGetInstance();
    void testNetworkFeedBack_data();
    void testNetworkFeedBack();
    void testServerFeedBack_data();
    void testServerFeedBack();
    void testDataFeedBack_data();
    void testDataFeedBack();
    void testNetworkTimeout_data();
    void testNetworkTimeout();
    void testGeoFail_data();
    void testGeoFail();
    void testCollectAddError();

private:
    ErrorReact* m_errorReact;
};

#endif // TEST_ERRORREACT_H