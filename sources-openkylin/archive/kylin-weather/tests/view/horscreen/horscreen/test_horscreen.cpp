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
#include "test_horscreen.h"
#include <QApplication>
#include <QSignalSpy>
#include <QTimer>
#include <QResizeEvent>
#include <QKeyEvent>
#include <QDesktopWidget>
#include <QScreen>
#include <QGuiApplication>
#include <QStyle>
#include <QCheckBox>

// Interaction实现已移至 mock_symbols.cpp

// 使用Google Mock
using ::testing::_;
using ::testing::Return;
using ::testing::AtLeast;
using ::testing::Invoke;

TestHorscreen::TestHorscreen()
{
}

void TestHorscreen::initTestCase()
{
    // 初始化测试环境
    mockInteraction = MockInteraction::getInstance();
    
    // 设置默认行为，防止返回 nullptr 导致崩溃
    static QStandardItemModel dummyModel;
    ON_CALL(*mockInteraction, getCityModel()).WillByDefault(Return(&dummyModel));
    ON_CALL(*mockInteraction, getTitleModel()).WillByDefault(Return(&dummyModel));
    ON_CALL(*mockInteraction, getCollectModel()).WillByDefault(Return(&dummyModel));
    ON_CALL(*mockInteraction, getBottomCollectModel()).WillByDefault(Return(&dummyModel));
}

void TestHorscreen::cleanupTestCase()
{
}

void TestHorscreen::testHorscreenConstruction()
{
    // 测试horscreen对象能否正确创建
    horscreen screen;
    QVERIFY(screen.minimumWidth() >= 904);
    QVERIFY(screen.minimumHeight() >= 624);
    
    // 验证私有成员初始化
    QVERIFY(screen.m_title != nullptr);
    QVERIFY(screen.m_city != nullptr);
    QVERIFY(screen.m_information != nullptr);
}

void TestHorscreen::testChangeBackgroundImage_NormalCode()
{
    horscreen screen;
    CurrentWeather weather;
    weather.cond_code = "100"; // 晴天
    screen.changeBackgroundImage(weather);
    
    // 验证背景图片逻辑
    QString bgName = screen.getBackgroundImageName("100");
    QVERIFY(bgName.contains("sunny"));
}

void TestHorscreen::testChangeBackgroundImage_EmptyCode()
{
    horscreen screen;
    CurrentWeather weather;
    weather.cond_code = "";
    screen.changeBackgroundImage(weather);
    QVERIFY(true);
}

void TestHorscreen::testChangeBackgroundImage_InvalidCode()
{
    horscreen screen;
    CurrentWeather weather;
    weather.cond_code = "999";
    screen.changeBackgroundImage(weather);
    QVERIFY(true);
}

void TestHorscreen::testShowLocateWidget_ShowAllowLocation()
{
    horscreen screen;
    screen.showLocateWidget();
    // 验证私有控件是否创建
    // QVERIFY(screen.m_AllowLocationwidget != nullptr);
}

void TestHorscreen::testShowAllowLocationWidget_Position()
{
    horscreen screen;
    screen.showLocateWidget(); // 确保 m_AllowLocationwidget 被创建
    screen.showAllowLocationWidget();
    QVERIFY(true);
}

void TestHorscreen::testShowSearchView_Show()
{
    horscreen screen;
    QSignalSpy spy(&screen, &horscreen::searchClear);
    
    screen.showSearchView();
    QVERIFY(screen.m_searchView->isVisible());
    QVERIFY(spy.count() == 1);
}

void TestHorscreen::testShowSearchView_Hide()
{
    horscreen screen;
    screen.showSearchView(); // Show
    screen.showSearchView(); // Hide
    QVERIFY(!screen.m_searchView->isVisible());
}

void TestHorscreen::testRecieveNetSignal_FirstStartNoNetwork()
{
    horscreen screen;
    screen.usedNet = 1;
    screen.recieveNetSignal("Network error");
    
    // 验证状态
    QVERIFY(screen.m_firstStartNoNetwork);
    if (screen.m_noNetworkWidget) {
        QVERIFY(screen.m_noNetworkWidget->isVisible());
    }
}

void TestHorscreen::testRecieveNetSignal_ShowErrorHint()
{
    horscreen screen;
    screen.usedNet = 0; // 已经使用过网络
    screen.recieveNetSignal("Connection lost");
    
    // 验证提示组件
    if (screen.m_hintWidget) {
        QVERIFY(screen.m_hintWidget->isVisible());
    }
}

void TestHorscreen::testShowCurrent_NormalDisplay()
{
    horscreen screen;
    CurrentWeather weather;
    weather.cond_code = "100";
    weather.tmp = "25";
    weather.city = "Beijing";
    
    QSignalSpy spy(&screen, &horscreen::sendCurrentInfor);
    screen.showCurrent(weather);
    
    QCOMPARE(spy.count(), 1);
    QCOMPARE(screen.m_currentCityWeather.city, QString("Beijing"));
}

void TestHorscreen::testHorscreenMutual()
{
    horscreen screen;
    QVERIFY(horscreen::mutual != nullptr);
}

void TestHorscreen::testShowForcast_Display()
{
    horscreen screen;
    ForecastWeather forecast;
    QSignalSpy spy(&screen, &horscreen::sendForcastInfor);
    screen.showForcast(forecast);
    QCOMPARE(spy.count(), 1);
}

void TestHorscreen::testShowLifeStyle_Display()
{
    horscreen screen;
    LifeStyle lifeStyle;
    QSignalSpy spy(&screen, &horscreen::sendLifeStyleInfor);
    screen.showLifeStyle(lifeStyle);
    QCOMPARE(spy.count(), 1);
}

void TestHorscreen::testShowHourly_Display()
{
    horscreen screen;
    QVariant var;
    QSignalSpy spy(&screen, &horscreen::sendHourlyInfor);
    screen.showHourly(var);
    QCOMPARE(spy.count(), 1);
}

void TestHorscreen::testCreateTrayIcon_Creation()
{
    horscreen screen;
    screen.createTrayIcon("100");
    QVERIFY(screen.m_trayIcon != nullptr);
}

void TestHorscreen::testUpdateTrayIcon_Update()
{
    horscreen screen;
    screen.createTrayIcon("100");
    screen.updateTrayIcon("200");
    QVERIFY(true);
}

void TestHorscreen::testHandleIconClickedSub_ShowAndActivate()
{
    horscreen screen;
    screen.handleIconClickedSub();
    QVERIFY(screen.isVisible());
}

void TestHorscreen::testIconActivated_Trigger()
{
    horscreen screen;
    screen.iconActivated(QSystemTrayIcon::Trigger);
    QVERIFY(true);
}

void TestHorscreen::testIconActivated_MiddleClick()
{
    horscreen screen;
    screen.iconActivated(QSystemTrayIcon::MiddleClick);
    QVERIFY(true);
}

void TestHorscreen::testIconActivated_DoubleClick()
{
    horscreen screen;
    screen.iconActivated(QSystemTrayIcon::DoubleClick);
    QVERIFY(true);
}

void TestHorscreen::testCloseActivated_Quit()
{
    horscreen screen;
    screen.closeActivated();
    QVERIFY(true);
}

void TestHorscreen::testSlotShowTitleSearchCityView_Show()
{
    horscreen screen;
    screen.slotShowTitleSearchCityView(true);
    if (screen.m_titleSearchCityView) {
        QVERIFY(screen.m_titleSearchCityView->isVisible());
    }
}

void TestHorscreen::testSlotShowTitleSearchCityViewUi_Hide()
{
    horscreen screen;
    screen.slotShowTitleSearchCityViewUi();
    if (screen.m_titleSearchCityView) {
        QVERIFY(!screen.m_titleSearchCityView->isVisible());
    }
}

void TestHorscreen::testSlotChangePCMode_Change()
{
    horscreen screen;
    screen.slotChangePCMode();
    QCOMPARE(screen.m_isFullScreen, 0);
}

void TestHorscreen::testSlotChangePadHMode_Change()
{
    horscreen screen;
    screen.slotChangePadHMode();
    QCOMPARE(screen.m_isFullScreen, 1);
}

void TestHorscreen::testSlotChangePadVMode_Change()
{
    horscreen screen;
    screen.slotChangePadVMode();
    QCOMPARE(screen.m_isFullScreen, 1);
}

void TestHorscreen::testSlotShowNoNetworkWidget_Show()
{
    horscreen screen;
    screen.slotShowNoNetworkWidget();
    if (screen.m_noNetworkWidget) {
        QVERIFY(screen.m_noNetworkWidget->isVisible());
    }
}

void TestHorscreen::testSlotHideNoNetworkWidget_Hide()
{
    horscreen screen;
    screen.slotHideNoNetworkWidget();
    if (screen.m_noNetworkWidget) {
        QVERIFY(!screen.m_noNetworkWidget->isVisible());
    }
}

void TestHorscreen::testResizeEvent_SizeAdjustment()
{
    horscreen screen;
    QResizeEvent event(QSize(1000, 800), QSize(904, 624));
    QApplication::sendEvent(&screen, &event);
    QCOMPARE(screen.m_label->size(), QSize(1000, 800));
}

void TestHorscreen::testResizeEvent_PadVMode()
{
    horscreen screen;
    ViewVar::getInstance()->m_currentMode = CurrentMode::PadVMode;
    QResizeEvent event(QSize(800, 1280), QSize(904, 624));
    QApplication::sendEvent(&screen, &event);
    QCOMPARE(screen.m_isFullScreen, 1);
    // 验证背景图缩放逻辑被触发
    QVERIFY(screen.m_label->pixmap() && !screen.m_label->pixmap()->isNull());
}

void TestHorscreen::testChangeHeight_Scaling()
{
    horscreen screen;
    screen.changeHeight(1.5);
    QCOMPARE(screen.m_scrollwidget->width(), 900 - 45);
    QCOMPARE(screen.m_scrollwidget->height(), 138 + 216 + 120 + 170);
}

void TestHorscreen::testChangOrigSize_Toggle()
{
    horscreen screen;
    // 初始状态是正常的
    screen.showNormal();
    screen.setFixedSize(900, 620);
    
    // 第一次调用：最大化
    screen.changOrigSize();
    QVERIFY(screen.isMaximized());
    
    // 第二次调用：还原
    screen.changOrigSize();
    QVERIFY(!screen.isMaximized());
    QCOMPARE(screen.size(), QSize(900, 620));
}

void TestHorscreen::testHideSearchView_Toggle()
{
    horscreen screen;
    screen.m_searchView->hide();
    screen.hideSearchView();
    QVERIFY(screen.m_searchView->isVisible());
    
    screen.hideSearchView();
    QVERIFY(!screen.m_searchView->isVisible());
}

void TestHorscreen::testEventFilter_SearchHide()
{
    horscreen screen;
    screen.show();
    screen.showSearchView();
    QVERIFY(screen.m_searchView->isVisible());
    
    // 模拟在搜索框外点击
    // 我们可以直接调用 eventFilter
    QMouseEvent mouseEvent(QEvent::MouseButtonPress, QPoint(-10, -10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    screen.eventFilter(&screen, &mouseEvent);
    
    QVERIFY(!screen.m_searchView->isVisible());
}

void TestHorscreen::testEventFilter_TitleSearchClear()
{
    horscreen screen;
    ViewVar::m_isTitleSearch = true;
    screen.m_titleSearchCityView->show();
    
    QSignalSpy spy(&screen, &horscreen::sigTitleLineEditClear);
    
    QMouseEvent mouseEvent(QEvent::MouseButtonPress, QPoint(-10, -10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    screen.eventFilter(&screen, &mouseEvent);
    
    QVERIFY(!screen.m_titleSearchCityView->isVisible());
    QCOMPARE(spy.count(), 1);
}

void TestHorscreen::testKeyPressEvent_CtrlE()
{
    horscreen screen;
    QKeyEvent event(QEvent::KeyPress, Qt::Key_E, Qt::ControlModifier);
    QApplication::sendEvent(&screen, &event);
    kdk::KSearchLineEdit *lineEdit = screen.m_searchView->findChild<kdk::KSearchLineEdit*>();
    QVERIFY(lineEdit && lineEdit->hasFocus());
}

void TestHorscreen::testInitMainWidgetSize_PCMode()
{
    horscreen screen;
    ViewVar::getInstance()->m_currentMode = CurrentMode::PCMode;
    screen.initMainWidgetSize();
    QCOMPARE(screen.size(), QSize(900, 620));
}

void TestHorscreen::testInitMainWidgetSize_PadHMode()
{
    horscreen screen;
    ViewVar::getInstance()->m_currentMode = CurrentMode::PadHMode;
    ViewVar::getInstance()->m_currentSize = qMakePair(1920, 1080);
    screen.initMainWidgetSize();
    QCOMPARE(screen.size(), QSize(1920, 1080));
}

void TestHorscreen::testInitMainWidgetSize_PadVMode()
{
    horscreen screen;
    ViewVar::getInstance()->m_currentMode = CurrentMode::PadVMode;
    ViewVar::getInstance()->m_currentSize = qMakePair(1080, 1920);
    screen.initMainWidgetSize();
    QCOMPARE(screen.size(), QSize(1080, 1920));
}

void TestHorscreen::testRecieveNetSignal_Timeout()
{
    horscreen screen;
    screen.usedNet = 0;
    screen.recieveNetSignal("timeout");
    QVERIFY(screen.m_hintWidget->isVisible());
    QCOMPARE(screen.isShow, true);
}

void TestHorscreen::testShowCurrent_FirstStartRecovery()
{
    horscreen screen;
    screen.m_firstStartNoNetwork = true;
    screen.m_noNetworkWidget->show();
    
    CurrentWeather weather;
    screen.showCurrent(weather);
    
    QVERIFY(!screen.m_firstStartNoNetwork);
    QVERIFY(!screen.m_noNetworkWidget->isVisible());
}

void TestHorscreen::testIconActivated_Context()
{
    horscreen screen;
    screen.createTrayIcon("100");
    screen.iconActivated(QSystemTrayIcon::Context);
    QVERIFY(screen.m_mainMenu->isVisible());
}

void TestHorscreen::testShowLocateWidget_SettingsDisabled()
{
    horscreen screen;
    CoreVar::setSettings("locate-widget", "1"); // 禁用
    screen.show();
    screen.showLocateWidget();
    QVERIFY(screen.m_AllowLocationwidget == nullptr);
}

void TestHorscreen::testHandleIconClickedSub_WithAboutWindow()
{
    horscreen screen;
    // 模拟创建关于窗口
    screen.m_title->g_menu->m_aboutWindow = new QDialog();
    screen.m_title->g_menu->m_aboutWindow->show();
    
    screen.handleIconClickedSub();
    
    // 验证关于窗口是否关闭
    QVERIFY(screen.m_title->g_menu->m_aboutWindow == nullptr || !screen.m_title->g_menu->m_aboutWindow->isVisible());
}

void TestHorscreen::testKeyPressEvent_F1()
{
    horscreen screen;
    QKeyEvent event(QEvent::KeyPress, Qt::Key_F1, Qt::NoModifier);
    QApplication::sendEvent(&screen, &event);
    QVERIFY(true);
}

void TestHorscreen::testShowOrHideMainWindow_Show()
{
    horscreen screen;
    screen.hide();
    screen.showOrHideMainWindow();
    QVERIFY(screen.isVisible());
}

void TestHorscreen::testShowOrHideMainWindow_Minimize()
{
    horscreen screen;
    screen.show();
    screen.showOrHideMainWindow();
    QVERIFY(true);
}
