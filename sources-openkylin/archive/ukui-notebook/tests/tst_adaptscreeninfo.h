// tst_adaptscreeninfo.h
#ifndef TST_ADAPTSCREENINFO_H
#define TST_ADAPTSCREENINFO_H

#include <QObject>
#include <QtTest>
#include "adaptscreeninfo.h"

class TestAdaptScreenInfo : public QObject
{
    Q_OBJECT

public:
    explicit TestAdaptScreenInfo(QObject *parent = nullptr) : QObject(parent) {}

private slots:
    void init();
    void cleanup();
    
    // 测试初始化
    void testInitialization();
    
    // 测试屏幕尺寸初始化
    void testInitScreenSize();
    
    // 测试主屏坐标初始化
    void testInitializeHomeScreenGeometry();
    
    // 测试分辨率改变响应
    void testResolutionChanged();
    
    // 测试主屏变化响应
    void testPrimaryScreenChanged();
    
    // 测试屏幕数量变化响应
    void testScreenCountChanged();
    
    // 测试屏幕列表更新
    void testScreenNumChange();

private:
    adaptScreenInfo* screenInfo;
};

#endif // TST_ADAPTSCREENINFO_H
