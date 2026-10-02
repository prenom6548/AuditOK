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
#ifndef TEST_HOTCITYBTN_H
#define TEST_HOTCITYBTN_H

#include <QObject>
#include <QtTest/QtTest>
#include "view/search/hotcitybtn.h"

class TestHotcityBtn : public QObject
{
    Q_OBJECT

public:
    TestHotcityBtn(QObject *parent = nullptr);

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // 测试构造函数
    void testConstructor();
    
    // 测试按钮点击信号
    void testHotCityClickSignal();
    
    // 测试按钮属性设置
    void testButtonProperties();

private:
    HotcityBtn *m_hotcityBtn;
};

#endif // TEST_HOTCITYBTN_H