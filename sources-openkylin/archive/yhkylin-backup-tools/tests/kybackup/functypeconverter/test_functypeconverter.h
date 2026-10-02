#ifndef TEST_FUNCTYPECONVERTER_H
#define TEST_FUNCTYPECONVERTER_H

#include <QObject>
#include <QtTest>
#include <QString>

// 前置声明
class FuncTypeConverter;

class TestFuncTypeConverter : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    
    // 测试keyCodeToKeyString方法
    void testKeyCodeToKeyString_data();
    void testKeyCodeToKeyString();
    
    // 测试keystringTokeycode方法
    void testKeystringTokeycode_data();
    void testKeystringTokeycode();
    
    // 测试funcTypeToFuncName方法
    void testFuncTypeToFuncName_data();
    void testFuncTypeToFuncName();
    
    // 测试funcTypeToThemeIconName方法
    void testFuncTypeToThemeIconName_data();
    void testFuncTypeToThemeIconName();
    
    // 测试funcTypeToWidgetClassName方法
    void testFuncTypeToWidgetClassName_data();
    void testFuncTypeToWidgetClassName();

private:
    FuncTypeConverter* m_converter;
};

#endif // TEST_FUNCTYPECONVERTER_H