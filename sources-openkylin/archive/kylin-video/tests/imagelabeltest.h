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
#ifndef IMAGELABELTEST_H
#define IMAGELABELTEST_H

#include <QObject>
#include <QtTest/QtTest>

class MainWindow;
class ImageLabel;

class ImageLabelTest : public QObject
{
    Q_OBJECT

public:
    explicit ImageLabelTest(QObject *parent = nullptr);

private slots:
    // 测试构造函数
    void testConstructor();
    
    // 测试 setTime 函数
    void testSetTime();
    
    // 测试 paintEvent 函数
    void testPaintEvent();
    
    // 测试析构函数
    void testDestructor();

private:
    MainWindow* m_win { nullptr };
    ImageLabel* m_imageLabel { nullptr };
};

#endif // IMAGELABELTEST_H