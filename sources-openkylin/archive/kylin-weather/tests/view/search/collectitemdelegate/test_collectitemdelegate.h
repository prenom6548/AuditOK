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
#ifndef TEST_COLLECTITEMDELEGATE_H
#define TEST_COLLECTITEMDELEGATE_H

#include <QObject>
#include <QApplication>
#include <QStandardItemModel>
#include <QListView>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QMouseEvent>
#include "view/search/collectitemdelegate.h"

class TestCollectItemDelegate : public QObject
{
    Q_OBJECT

public:
    TestCollectItemDelegate(QObject *parent = nullptr);

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // 测试构造函数
    void testConstructor();
    
    // 测试sizeHint方法
    void testSizeHint();
    
    // 测试paint方法
    void testPaint();
    
    // 测试editorEvent方法
    void testEditorEvent();
    
    // 测试changeHighLightColor方法
    void testChangeHighLightColor();
    
    // 测试drawCustomButton方法
    void testDrawCustomButton();
    
    // 测试信号
    void testSignals();
    void testCoverageEnhancement();

private:
    CollectItemDelegate *m_collectItemDelegate;
    QStandardItemModel *m_model;
    QListView *m_view;
};

#endif // TEST_COLLECTITEMDELEGATE_H
