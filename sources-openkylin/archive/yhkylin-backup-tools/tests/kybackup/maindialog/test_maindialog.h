#ifndef TEST_MAINDIALOG_H
#define TEST_MAINDIALOG_H

#include <QObject>
#include <QtTest/QtTest>
#include <QStackedWidget>
#include "../../kybackup/maindialog.h"

class TestMainDialog : public QObject
{
    Q_OBJECT

public:
    TestMainDialog();

private slots:
    void initTestCase();
    void cleanupTestCase();
    
    // 测试构造函数和析构函数
    void testConstructor();
    
    // 测试公共函数
    void testInitUI();
    void testInitTileBar();
    void testInitConnect();
    void testMountBackupPartition();
    void testUmountBackupPartition();
    void testCreateWidgetFactory();
    
    // 测试公共槽函数
    void testSltMessageReceived();
    void testCloseBtn();
    void testTabletModeChange();
    void testSelected();
    
    // 测试信号
    void testSignals();

private:
    MainDialog* m_mainDialog;
};

#endif // TEST_MAINDIALOG_H