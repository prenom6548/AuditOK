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

#ifndef TEST_ADDRSET_H
#define TEST_ADDRSET_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <QtTest/QtTest>
#include <QApplication>
#include "addrset.h"

class TestAddrSet : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testGetInstance();
    void testGetHostIpAddress();
    void testGetHostMacAddress();
    void testGetLocalHost();
    void testCoverageAdditions();
};

#endif // TEST_ADDRSET_H