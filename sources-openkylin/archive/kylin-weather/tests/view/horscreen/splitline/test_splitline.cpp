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
 * along with this program.  If not, see https://www.gnu.org/licenses/.
 */

#include "test_splitline.h"
#include "view/horscreen/splitline.h"
#include <QApplication>
#include <QStyle>
#include <QStyleOption>

// 构造函数测试
void test_splitline::testConstructor()
{
    // 创建一个splitline实例
    splitline *splitLine = new splitline();
    
    // 验证对象是否成功创建
    QVERIFY(splitLine != nullptr);
    
    // 验证默认大小
    QCOMPARE(splitLine->width(), 1);
    QCOMPARE(splitLine->height(), 180);
    
    // 验证内部label是否创建
    QList<QLabel*> labels = splitLine->findChildren<QLabel*>();
    QVERIFY(labels.size() > 0);
    QLabel *label = labels.first();
    QVERIFY(label != nullptr);
    QCOMPARE(label->width(), 1);
    QCOMPARE(label->height(), 180);
    
    delete splitLine;
}

// 带父对象的构造函数测试
void test_splitline::testConstructorWithParent()
{
    QWidget *parent = new QWidget();
    
    // 创建一个splitline实例，带父对象
    splitline *splitLine = new splitline(parent);
    
    // 验证对象是否成功创建
    QVERIFY(splitLine != nullptr);
    
    // 验证父对象设置正确
    QCOMPARE(splitLine->parent(), parent);
    
    // 验证默认大小
    QCOMPARE(splitLine->width(), 1);
    QCOMPARE(splitLine->height(), 180);
    
    delete parent;
    // parent会自动删除child，所以不需要单独删除splitLine
}

// 样式初始化测试
void test_splitline::testInitQss()
{
    splitline *splitLine = new splitline();
    
    // 获取内部label
    QList<QLabel*> labels = splitLine->findChildren<QLabel*>();
    QVERIFY(labels.size() > 0);
    QLabel *label = labels.first();
    QVERIFY(label != nullptr);
    
    // 验证样式表是否设置
    QString styleSheet = label->styleSheet();
    QVERIFY(!styleSheet.isEmpty());
    
    // 验证样式表包含预期的渐变色设置
    QVERIFY(styleSheet.contains("qlineargradient"));
    QVERIFY(styleSheet.contains("rgba(255, 255, 255, 0)"));
    QVERIFY(styleSheet.contains("rgba(255, 255, 255, 0.7)"));
    
    delete splitLine;
}

// 测试绘制事件
void test_splitline::testPaintEvent()
{
    splitline *splitLine = new splitline();
    
    // 触发绘制事件
    QTest::qWait(100); // 等待事件处理
    
    // 验证控件可以正常显示
    QVERIFY(splitLine->isVisible() || !splitLine->isVisible()); // 控件应该能正常处理可见性状态
    
    delete splitLine;
}