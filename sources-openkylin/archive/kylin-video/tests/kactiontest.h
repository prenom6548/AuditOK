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
#ifndef KACTIONTEST_H
#define KACTIONTEST_H

#include <QObject>
#include <QtTest/QtTest>

class KAction;

class KActionTest : public QObject
{
    Q_OBJECT

public:
    explicit KActionTest(QObject *parent = nullptr);

private slots:
    // 测试 KAction(QObject*, bool) 构造函数
    void testConstructorWithParentAndAutoAdd();
    
    // 测试 KAction(QObject*, const char*, bool) 构造函数
    void testConstructorWithParentNameAndAutoAdd();
    
    // 测试 KAction(const QString&, QKeySequence, QObject*, const char*, bool) 构造函数
    void testConstructorWithTextKeySequenceAndMore();
    
    // 测试 KAction(QKeySequence, QObject*, const char*, bool) 构造函数
    void testConstructorWithKeySequenceAndMore();
    
    // 测试 addShortcut 函数
    void testAddShortcut();
    
    // 测试 change(QIcon, QString) 函数
    void testChangeWithIconAndText();
    
    // 测试 change(QString) 函数
    void testChangeWithText();
    
    // 测试析构函数
    void testDestructor();

private:
    KAction* m_action { nullptr };
};

#endif // KACTIONTEST_H