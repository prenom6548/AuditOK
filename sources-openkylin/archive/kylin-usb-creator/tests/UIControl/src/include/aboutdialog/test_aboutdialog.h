#ifndef TEST_ABOUTDIALOG_H
#define TEST_ABOUTDIALOG_H

#include <QObject>
#include <QTest>
#include <QSignalSpy>
#include <QApplication>
#include "../../../../../UIControl/src/include/aboutdialog.h"

class TestAboutDialog : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    
    // 测试构造函数
    void testConstructor();
    
    // 测试isDarkTheme方法
    void testIsDarkTheme();
    
    // 测试fontSizeInit方法
    void testFontSizeInit();
    
    // 测试dealSetFontSize方法
    void testDealSetFontSize();
    
    // 测试uiModeChange方法
    void testUiModeChange();
    // 测试getUSBCreatorVersion方法
    void testGetUSBCreatorVersion();
    // 测试dealSystemGsettingChange方法
    void testDealSystemGsettingChange();
    // 测试字体大小变化
    void testFontSizeChange();
    
    // 测试主题相关功能
    void testThemeFunctions();
    // 测试getUSBCreatorVersion函数的替代方案
    void testVersionFunctionality();
    // 测试条件分支和边界情况
    void testConditionalBranches();
    // 测试字体大小变化的特定情况
    void testFontSizeChangeSpecific();
    
    // 测试getUSBCreatorVersion函数的完整实现
    void testGetUSBCreatorVersionFull();
 
 
 
 
 
 

private:
    aboutdialog* m_dialog;
};

#endif // TEST_ABOUTDIALOG_H