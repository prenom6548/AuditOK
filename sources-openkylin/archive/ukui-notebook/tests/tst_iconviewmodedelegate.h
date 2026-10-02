// tst_iconviewmodedelegate.h
#ifndef TST_ICONVIEWMODEDELEGATE_H
#define TST_ICONVIEWMODEDELEGATE_H

#include <QObject>
#include <QtTest>
#include <QStandardItemModel>
#include "iconViewModeDelegate.h"
#include "noteModel.h"

class TestIconViewModeDelegate : public QObject
{
    Q_OBJECT

public:
    explicit TestIconViewModeDelegate(QObject *parent = nullptr) {}

private slots:
    void init();
    void cleanup();

    // 测试颜色转换
    void testColorConversion();
    
    // 测试日期时间解析
    void testParseDateTime();
    
    // 测试代理尺寸
    void testSizeHint();
    
    // 测试状态设置
    void testSetState();
    
    // 测试动画设置
    void testSetAnimationDuration();
    
    // 测试活动状态设置
    void testSetActive();

private:
    iconViewModeDelegate* delegate;
    QStandardItemModel* model;
};

#endif // TST_ICONVIEWMODEDELEGATE_H
