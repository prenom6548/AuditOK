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
#include "test_titlesearchmodel.h"
#include "model/titlesearchmodel.h"
#include <QtTest/QtTest>
#include <QApplication>
#include <QHash>

void TestTitleSearchModel::initTestCase()
{
    // 在所有测试之前运行
}

void TestTitleSearchModel::cleanupTestCase()
{
    // 在所有测试之后运行
}

void TestTitleSearchModel::init()
{
    // 在每个测试之前运行
    titleSearchModel = new TitleSearchModel();
}

void TestTitleSearchModel::cleanup()
{
    // 在每个测试之后运行
    delete titleSearchModel;
}

void TestTitleSearchModel::testConstructor()
{
    QVERIFY(titleSearchModel != nullptr);
    // 验证对象是否正确创建
    QCOMPARE(titleSearchModel->rowCount(), 0);
    QCOMPARE(titleSearchModel->columnCount(), 0);
}

void TestTitleSearchModel::testRoleNames()
{
    QHash<int, QByteArray> roleNames = titleSearchModel->roleNames();
    
    // 验证角色名称是否正确设置
    QVERIFY(roleNames.contains(TitleSearchModel::CityId));
    QVERIFY(roleNames.contains(TitleSearchModel::CityName));
    QVERIFY(roleNames.contains(TitleSearchModel::CityStyle));
    
    QCOMPARE(roleNames[TitleSearchModel::CityId], QByteArray("cityid"));
    QCOMPARE(roleNames[TitleSearchModel::CityName], QByteArray("cityname"));
    QCOMPARE(roleNames[TitleSearchModel::CityStyle], QByteArray("citystyle"));
}

void TestTitleSearchModel::testEnums()
{
    // 验证枚举值是否正确
    QCOMPARE(static_cast<int>(TitleSearchModel::CityId), Qt::UserRole + 1);
    QCOMPARE(static_cast<int>(TitleSearchModel::CityName), Qt::UserRole + 2);
    QCOMPARE(static_cast<int>(TitleSearchModel::CityStyle), Qt::UserRole + 3);
    
    // 验证CityType枚举
    QCOMPARE(static_cast<int>(TitleSearchModel::Location), 0);
    QCOMPARE(static_cast<int>(TitleSearchModel::Collection), 1);
}