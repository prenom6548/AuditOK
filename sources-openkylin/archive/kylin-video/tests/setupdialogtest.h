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
#ifndef SETUPDIALOGTEST_H
#define SETUPDIALOGTEST_H

#include <QObject>
#include <QtTest/QtTest>

class SetUpDialog;

class SetUpDialogTest : public QObject
{
    Q_OBJECT

public:
    explicit SetUpDialogTest(QObject *parent = nullptr);

private slots:
    // 测试 hideEvent 函数
    void testHideEvent();
    
    // 测试 showEvent 函数
    void testShowEvent();
    
    // 测试 paintEvent 函数
    void testPaintEvent();
    
    // 测试 event 函数
    void testEvent();

private:
    SetUpDialog* m_dialog { nullptr };
};

#endif // SETUPDIALOGTEST_H