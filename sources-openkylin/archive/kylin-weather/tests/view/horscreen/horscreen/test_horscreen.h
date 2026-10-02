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
#ifndef TEST_HORSCREEN_H
#define TEST_HORSCREEN_H

#include <QObject>
#include <QtTest>
#include <QApplication>
#include "../../../../view/horscreen/horscreen.h"

// Google Mock headers
#include "gmock/gmock.h"

// Mock类声明
class MockInteraction : public Interaction {
    Q_OBJECT
public:
    static MockInteraction* getInstance() {
        static MockInteraction instance;
        return &instance;
    }

    MOCK_METHOD1(creatCore, void(const QStringList &list));
    MOCK_METHOD1(changeCity, void(QString cityCode));
    MOCK_METHOD1(searchResult, void(QString inputText));
    MOCK_METHOD0(getCityModel, QStandardItemModel *());
    MOCK_METHOD0(getTitleModel, QStandardItemModel *());
    MOCK_METHOD0(getCollectModel, QStandardItemModel *());
    MOCK_METHOD0(getBottomCollectModel, QStandardItemModel *());
    MOCK_METHOD1(changeCityFromSearch, void(QModelIndex index));
    MOCK_METHOD1(addCollectCity, void(QModelIndex index));
    MOCK_METHOD1(changeCityFromCollect, void(QModelIndex index));
    MOCK_METHOD1(delCollectCity, void(QModelIndex index));
    MOCK_METHOD0(test, void());
    MOCK_METHOD0(loadCollectCity, void());
    MOCK_METHOD0(showCenterAfter, void());
    MOCK_METHOD0(closePrograme, void());
    MOCK_METHOD0(allowLocationYes, void());
    MOCK_METHOD0(allowLocationNo, void());
    MOCK_METHOD0(refresh, void());
    MOCK_METHOD1(addCollectCityFromWidget, void(QString cityId));
    MOCK_METHOD1(delCollectCityFromWidget, void(QString cityId));
    MOCK_METHOD1(changeCityFromTitleSearch, void(QString cityId));
    MOCK_METHOD1(getLAndRCityInfo, void(QString cityId));

private:
    MockInteraction() {}
};

class TestHorscreen : public QObject
{
    Q_OBJECT

public:
    TestHorscreen();

private slots:
    void initTestCase();
    void cleanupTestCase();

    // 构造函数测试
    void testHorscreenConstruction();

    // changeBackgroundImage测试
    void testChangeBackgroundImage_NormalCode();
    void testChangeBackgroundImage_EmptyCode();
    void testChangeBackgroundImage_InvalidCode();

    // showLocateWidget测试
    void testShowLocateWidget_ShowAllowLocation();

    // showAllowLocationWidget测试
    void testShowAllowLocationWidget_Position();

    // showSearchView测试
    void testShowSearchView_Show();
    void testShowSearchView_Hide();

    // recieveNetSignal测试
    void testRecieveNetSignal_FirstStartNoNetwork();
    void testRecieveNetSignal_ShowErrorHint();

    // showCurrent测试
    void testShowCurrent_NormalDisplay();
    void testHorscreenMutual();

    // showForcast测试
    void testShowForcast_Display();

    // showLifeStyle测试
    void testShowLifeStyle_Display();

    // showHourly测试
    void testShowHourly_Display();

    // createTrayIcon测试
    void testCreateTrayIcon_Creation();

    // updateTrayIcon测试
    void testUpdateTrayIcon_Update();

    // handleIconClickedSub测试
    void testHandleIconClickedSub_ShowAndActivate();

    // iconActivated测试
    void testIconActivated_Trigger();
    void testIconActivated_MiddleClick();
    void testIconActivated_DoubleClick();

    // closeActivated测试
    void testCloseActivated_Quit();

    // slotShowTitleSearchCityView测试
    void testSlotShowTitleSearchCityView_Show();

    // slotShowTitleSearchCityViewUi测试
    void testSlotShowTitleSearchCityViewUi_Hide();

    // slotChangePCMode测试
    void testSlotChangePCMode_Change();

    // slotChangePadHMode测试
    void testSlotChangePadHMode_Change();

    // slotChangePadVMode测试
    void testSlotChangePadVMode_Change();

    // slotShowNoNetworkWidget测试
    void testSlotShowNoNetworkWidget_Show();

    // slotHideNoNetworkWidget测试
    void testSlotHideNoNetworkWidget_Hide();

    // resizeEvent测试
    void testResizeEvent_SizeAdjustment();
    void testResizeEvent_PadVMode();

    // changeHeight测试
    void testChangeHeight_Scaling();

    // changOrigSize测试
    void testChangOrigSize_Toggle();

    // hideSearchView测试
    void testHideSearchView_Toggle();

    // eventFilter测试
    void testEventFilter_SearchHide();
    void testEventFilter_TitleSearchClear();

    // keyPressEvent测试
    void testKeyPressEvent_CtrlE();
    void testInitMainWidgetSize_PCMode();
    void testInitMainWidgetSize_PadHMode();
    void testInitMainWidgetSize_PadVMode();
    void testRecieveNetSignal_Timeout();
    void testShowCurrent_FirstStartRecovery();
    void testIconActivated_Context();
    void testShowLocateWidget_SettingsDisabled();
    void testHandleIconClickedSub_WithAboutWindow();
    void testKeyPressEvent_F1();

    // showOrHideMainWindow测试
    void testShowOrHideMainWindow_Show();
    void testShowOrHideMainWindow_Minimize();

private:
    MockInteraction *mockInteraction;
};

#endif // TEST_HORSCREEN_H
