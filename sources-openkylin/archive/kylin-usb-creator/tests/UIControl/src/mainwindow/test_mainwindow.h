#ifndef TEST_MAINWINDOW_H
#define TEST_MAINWINDOW_H

#include <QObject>
#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QApplication>
#include <QWidget>
#include "../../../UIControl/src/mainwindow.h"

class TestMainWindow : public QObject
{
    Q_OBJECT

public:
    TestMainWindow(QObject *parent = nullptr);

private slots:
    // 测试初始化和清理
    void initTestCase();
    void cleanupTestCase();
    
    // 构造函数测试
    void testConstructor();
    void testConstructorWithParent();
    void testConstructorWithIsoPath();
    void testDestructor();
    
    // 主要功能测试
    void testReShow();
    void testReShowWithIsoPath();
    void testMakeStart();
    void testMakeFinish();
    void testReturnMain();
    void testDoubleCheck();
    
    // 样式和主题测试
    void testDealSetFontSize();
    void testSetThemeStyle();
    void testThemeSwitching();
    void testSetThemeDarkLight();
    
    // 事件处理测试
    void testKeyPressEvent();
    void testDragDropEvents();
    
    // 窗口属性测试
    void testWindowGeometry();
    void testWindowTitle();
    void testCloseMain();
    void testWindowFlags();
    
    // 组件初始化测试
    void testStatusbarInit();
    void testMyStyle();
    void testInitGsetting();
    void testFontSizeInit();
    void testCreateTrayActions();
    
    // 功能测试
    void testSignalConnections();
    void testMenuFunctions();
    void testFontSizeSignal();
    void testPageNavigation();
    void testChangePage();
    void testAuthorityStatusHandling();
    void testDataTracking();
    void testHandleIconClickedSub();
    void testSlotActiveWindow();
    void testAboutClick();
    
    // 功能测试
    void testSetIconchange();
    void testShowaboutslot();
    
    // 信号测试
    void testSignalEmission();
    
    // 辅助功能测试
    void testAccessibilityNames();
    void testComponentVisibility();
    void testLanguageSupport();
    void testResourceLoading();
    void testMemoryManagement();
    
    // 新增的测试函数
    void testCreateTrayActionsDirect();
    void testDoubleCheckWithDifferentStates();
    void testChangePageWithDifferentIndexes();
    void testReturnMainWithDifferentStates();
    void testKeyPressEventWithDifferentKeys();
    void testReturnMainWithCancelText();
    void testChangePageBoundaryConditions();
    void testDoubleCheckWithMessageBoxResponse();
};

#endif // TEST_MAINWINDOW_H