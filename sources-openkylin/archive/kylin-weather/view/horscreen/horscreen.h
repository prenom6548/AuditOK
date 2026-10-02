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
#ifndef HORSCREEN_H
#define HORSCREEN_H

#include <QWidget>
#include <QLabel>
#include <QIcon>
#include <QPushButton>
#include <QDebug>
#include <QResizeEvent>
#include <QScreen>
#include <QDBusInterface>
#include <QScrollArea>
#include "view/search/searchview.h"
#include "controller/Interaction.h"
#include "global/corevar.h"
#include "title.h"
#include "empty.h"
#include "city.h"
#include "information.h"
#include "daemondbus.h"
#include "promptwidget.h"
#include "view/horscreen/allowlocation.h"
#include <user_manual.hpp>
#include "kaboutdialog.h"
#include <buried_point.hpp>
#include <QJsonObject>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

#include "changecitywidget.h"
#include "view/search/titlesearchcityview.h"
#include "view/search/bottomlistview.h"
#include "ukuiwindowhelper.h"
#include "nonetworkwidget.h"

using namespace kdk;
class horscreen : public QWidget
{
    Q_OBJECT
    friend class TestHorscreen;
public:
    explicit horscreen(QWidget *parent = nullptr);
    static horscreen *mutual; //指针类型静态成员变量
    static CurrentWeather *m_showCurrentWeather;
    void changeBackgroundImage(CurrentWeather m_currentWeather);
    //    static bool isFullScreen = 1; // 判断是全屏还是普通界面，初始未启动下等同于全屏值为 1
    void showLocateWidget();
    void showAllowLocationWidget();

protected:
    bool eventFilter(QObject *watch, QEvent *e) override;

private:
    void initWidget();
private:
    title *m_title = nullptr;
    Empty *m_empty = nullptr;
    ChangeCityWidget *m_changeCity = nullptr;
    QWidget *m_widget = nullptr;
    city *m_cityL = nullptr;
    city *m_city = nullptr;
    city *m_cityR = nullptr;
    QVBoxLayout *m_mainLayout;
    QVBoxLayout *m_mainCityLayout = nullptr;
    QLabel *m_label;
    SearchView *m_searchView = nullptr;
    information *m_information;
    QScrollArea *m_scrollarea;
    QWidget *m_scrollwidget;
    QWidget *m_networkErrorWidget = nullptr;
    PromptWidget *m_hintWidget = nullptr;
    AllowLocation *m_AllowLocationwidget = nullptr;
    NoNetworkWidget *m_noNetworkWidget = nullptr;  // 首次启动无网络界面
    QSystemTrayIcon *m_trayIcon = nullptr;
    QMenu *m_mainMenu = nullptr;
    QAction *m_openAction;
    QAction *m_quitAction;
    TitleSearchCityView *m_titleSearchCityView = nullptr;
    QWidget *m_titleSearchWidget = nullptr;

    bool m_animationBool = false;
    bool m_firstLoad = true;
    int move_distancex = 0;
    int move_x = 0;

    int m_isFullScreen = 1;
    bool isShow = false; // 网络异常提示界面是否显示
    QString netErrorInfor = "";
    // 判断前面是否有网导致界面显示出正常天气信息
    // 0 : 使用过网络； 1 : 未使用过(初始值为 1)
    int usedNet = 1;
    bool m_firstStartNoNetwork = false;  // 标记是否首次启动时无网络
    double detio = 1.0;
    // 饱满填充
    QPixmap m_pixmap;
    //    bool m_searchShow = false;//默认搜索弹窗没有弹出--先保留
    
    void initMainWidgetSize(); // 设置主窗口的大小
    void initInteraction(); //后端初始化信号槽
    void initConnect();     //初始化前端信号槽
    void resizeEvent(QResizeEvent *event);
    bool event(QEvent *event); //做searchView的失焦处理
    void changOrigSize();
    void initLayout();
    void changeHeight(double detio);
    //    void mouseDoubleClickEvent(QMouseEvent *event);//双击响应--全屏和还原
    void keyPressEvent(QKeyEvent *event); // 键盘响应事件--f1
    QString getBackgroundImageName(QString code);
    void showNetworkErrorDialog(QString);
    KAboutDialog *m_aboutDialog = nullptr;
    void showOrHideMainWindow();

    QString m_rightCity;
    QString m_leftCity;
    CityType m_rightCityType;
    CityType m_leftCityType;
    CurrentWeather m_rightCityWeather;
    CurrentWeather m_leftCityWeather;
    CurrentWeather m_currentCityWeather;

    QString m_windowId = "";

    // 控件
    QPushButton *m_rightBtn = nullptr; // 向右切换按钮
    QPushButton *m_leftBtn = nullptr; // 向左切换按钮
    QHBoxLayout *m_hMainlayout = nullptr; // 横向布局
    QHBoxLayout *m_hlayout = nullptr; // 横向布局
    QVBoxLayout *m_vlayout = nullptr; // 纵向布局
    QHBoxLayout *m_hCityLayout = nullptr; // 横向布局

private slots:
    void hideSearchView();
    void showSearchView();
    void recieveNetSignal(QString netError); //接收网络错误信息
    void showCurrent(CurrentWeather m_currentWeather); //接收当前城市信息
    void showForcast(ForecastWeather forecastWeather); //接收预测信息
    void showLifeStyle(LifeStyle m_lifeStyle);         //接收生活指数信息
    void showHourly(QVariant var);                     //接收逐小时信息
    void createTrayIcon(QString code);                 //创建托盘图标
    void updateTrayIcon(QString code);                 //更新托盘图标
    void handleIconClickedSub();                       //托盘图标被点击,显示在屏幕中央
                                                       //    void handleIconClicked();//处理点击托盘图标事件
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void closeActivated();

    void slotShowTitleSearchCityView(bool hasCity); // 显示界面搜索列表
    void slotShowTitleSearchCityViewUi();
    void slotShowNoNetworkWidget(); // 显示首次启动无网络界面
    void slotHideNoNetworkWidget(); // 隐藏首次启动无网络界面

    void slotChangePCMode();
    void slotChangePadHMode();
    void slotChangePadVMode();

signals:
    void sendCurrentInfor(CurrentWeather m_currentWeather); //
    void sendLifeStyleInfor(LifeStyle m_lifeStyle, int m_isFullScreen);
    void sendHourlyInfor(QList<Hourly> tmp_list, int m_isFullScreen); // 逐小时信息
    void sendIsFullScreen(int isFullScreen);
    void sendWeatherIconInfor(int isFullScreen);
    void sendIsFullScreenAndShow(int isFullScreen, bool isShow);
    void sendDetio(double detio);
    void sendForcastInfor(ForecastWeather m_forecastWeather, int m_isFullScreen);
    void loadCollectCity(); //发送信号给后端--加载收藏城市列表
    void recallInitLayout(QString netErrorInfor, int m_isFullScreen);
    void searchClear();     //当搜索隐藏时，发信号搜索界面，重置界面
    void showCenterAfter(); // 显示在屏幕中间
    void setCentralWidFixedSize(bool isFullScreen, double detio); //设置当前城市widget普通大小和全屏界面大小
    void changeLoadingIconToButton();
    void sigTitleLineEditClear(); // 鼠标移除搜索界面触发信号
    void sigGetDataFromCore();
};

#endif // HORSCREEN_H
