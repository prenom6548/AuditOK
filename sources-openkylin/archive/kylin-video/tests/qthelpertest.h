/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef QTHELPERTEST_H
#define QTHELPERTEST_H

#include <QObject>
#include <QtTest/QtTest>

class QTHelperTest : public QObject
{
    Q_OBJECT

public:
    explicit QTHelperTest(QObject *parent = nullptr);

private slots:
    // 测试 node_to_variant 函数
    void testNodeToVariant();
    
    // 测试 node_builder 结构体
    void testNodeBuilder();
    
    // 测试 get_property 函数
    void testGetProperty();
    
    // 测试 set_property 函数
    void testSetProperty();
    
    // 测试 command 函数
    void testCommand();
    
    // 测试 ErrorReturn 相关函数
    void testErrorReturn();
};

#endif // QTHELPERTEST_H