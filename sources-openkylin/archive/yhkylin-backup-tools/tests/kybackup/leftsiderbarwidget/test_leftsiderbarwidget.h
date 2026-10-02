#ifndef TEST_LEFTSIDERBARWIDGET_H
#define TEST_LEFTSIDERBARWIDGET_H

#include <QObject>
#include <QApplication>
#include <QWidget>
#include <QtTest/QtTest>
#include "../../../kybackup/leftsiderbarwidget.h"

class TestLeftsiderbarWidget : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    
    // 测试构造函数和析构函数
    void testConstructorDestructor();
    
    // 测试公共方法
    void testInitUi();
    void testSetBusy();
    void testSetCheckedFunc();
    
    // 测试受保护方法（通过公共接口间接测试）
    void testPaintEvent();
    void testEventFilter();

private:
    LeftsiderbarWidget *m_leftSidebar;
};

#endif // TEST_LEFTSIDERBARWIDGET_H