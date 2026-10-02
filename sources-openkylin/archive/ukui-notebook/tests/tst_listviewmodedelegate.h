#ifndef TST_LISTVIEWMODEDELEGATE_H
#define TST_LISTVIEWMODEDELEGATE_H

#include <QtTest>
#include <QObject>
#include <QStandardItemModel>
#include "listViewModeDelegate.h"
#include "noteModel.h"

class TestListViewModeDelegate : public QObject {
    Q_OBJECT

private:
    listViewModeDelegate *m_delegate;
    QStandardItemModel *m_model;
    QModelIndex m_testIndex;

private slots:
    void init();
    void cleanup();

    // 测试构造函数和基本属性
    void testConstructor();

    // 测试颜色转换方法
    void testColorConversion();

    // 测试时间日期解析
    void testDateTimeParsing();

    // 测试状态设置和动画状态
    void testStateAndAnimation();

    // 测试绘制相关方法（可能需要模拟）
    void testPaintMethods();

    // 测试大小提示
    void testSizeHint();

    // 测试设置活跃状态
    void testSetActive();

    // 测试设置当前选中索引
    void testSetCurrentSelectedIndex();
};

#endif // TST_LISTVIEWMODEDELEGATE_H
