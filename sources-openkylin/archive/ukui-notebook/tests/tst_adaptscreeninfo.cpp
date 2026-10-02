// tst_adaptscreeninfo.cpp
#include "tst_adaptscreeninfo.h"
#include <QSignalSpy>

void TestAdaptScreenInfo::init()
{
    screenInfo = new adaptScreenInfo(this);
}

void TestAdaptScreenInfo::cleanup()
{
    delete screenInfo;
}

void TestAdaptScreenInfo::testInitialization()
{
    // 验证初始化后基本属性
    QVERIFY(screenInfo->m_pDeskWgt != nullptr);
    QVERIFY(screenInfo->m_pListScreen.count() > 0);
    
    // 验证屏幕列表是否正确获取
    QList<QScreen*> screens = QGuiApplication::screens();
    QCOMPARE(screenInfo->m_pListScreen.count(), screens.count());
}

void TestAdaptScreenInfo::testInitScreenSize()
{
    // 获取实际屏幕尺寸
    QDesktopWidget* desktop = QApplication::desktop();
    int screenCount = desktop->screenCount();
    
    // 调用初始化函数
    screenInfo->initScreenSize();
    
    if (screenCount > 1) {
        // 多屏幕情况
        QList<QScreen*> screens = QGuiApplication::screens();
        QCOMPARE(screenInfo->m_screenWidth, 
                 screens[0]->geometry().width() + screenInfo->m_nScreen_x);
        QCOMPARE(screenInfo->m_screenHeight, 
                 screens[0]->geometry().height() + screenInfo->m_nScreen_y);
    } else {
        // 单屏幕情况
        QCOMPARE(screenInfo->m_screenWidth, 
                 desktop->width() + screenInfo->m_nScreen_x);
        QCOMPARE(screenInfo->m_screenHeight, 
                 desktop->height() + screenInfo->m_nScreen_y);
    }
}

void TestAdaptScreenInfo::testInitializeHomeScreenGeometry()
{
    QDesktopWidget* desktop = QApplication::desktop();
    int screenCount = desktop->screenCount();
    
    screenInfo->InitializeHomeScreenGeometry();
    
    if (screenCount > 1) {
        QList<QScreen*> screens = QGuiApplication::screens();
        QCOMPARE(screenInfo->m_nScreen_x, screens[0]->geometry().x());
        QCOMPARE(screenInfo->m_nScreen_y, screens[0]->geometry().y());
    } else {
        QCOMPARE(screenInfo->m_nScreen_x, 0);
        QCOMPARE(screenInfo->m_nScreen_y, 0);
    }
}

void TestAdaptScreenInfo::testResolutionChanged()
{
    // 存储原始值
    int oldWidth = screenInfo->m_screenWidth;
    int oldHeight = screenInfo->m_screenHeight;
    
    // 模拟分辨率改变
    QRect newGeometry(0, 0, 1920, 1080);
    screenInfo->onResolutionChanged(newGeometry);
    
    // 验证值是否已更新
    QVERIFY(screenInfo->m_screenWidth != oldWidth || 
            screenInfo->m_screenHeight != oldHeight);
}

void TestAdaptScreenInfo::testPrimaryScreenChanged()
{
    // 存储原始值
    int oldX = screenInfo->m_nScreen_x;
    int oldY = screenInfo->m_nScreen_y;
    
    // 触发主屏变化
    screenInfo->primaryScreenChangedSlot();
    
    // 验证坐标是否更新
    QCOMPARE(screenInfo->m_nScreen_x, oldX);
    QCOMPARE(screenInfo->m_nScreen_y, oldY);
}

void TestAdaptScreenInfo::testScreenCountChanged()
{
    // 存储原始值
    int oldWidth = screenInfo->m_screenWidth;
    int oldHeight = screenInfo->m_screenHeight;
    
    // 模拟屏幕数量变化
    screenInfo->screenCountChangedSlots(2);
    
    // 验证尺寸是否更新
    QVERIFY(screenInfo->m_screenWidth >= oldWidth);
    QVERIFY(screenInfo->m_screenHeight >= oldHeight);
}

void TestAdaptScreenInfo::testScreenNumChange()
{
    int originalCount = screenInfo->m_pListScreen.count();
    
    // 触发屏幕列表更新
    screenInfo->screenNumChange();
    
    // 验证列表是否更新
    QCOMPARE(screenInfo->m_pListScreen.count(), 
             QGuiApplication::screens().count());
}

#include "moc_tst_adaptscreeninfo.cpp"
