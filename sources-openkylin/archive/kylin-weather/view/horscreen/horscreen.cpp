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
#include "horscreen.h"
#include "title.h"
#include "city.h"
#include "perhour.h"
#include "weekweather.h"
#include "onedayweather.h"
#include "lifestyle.h"
#include "information.h"
#include <QVBoxLayout>
#include <QScrollArea>
#include <QScrollBar>
#include "daemondbus.h"
#include "global/corevar.h"
#include "view/horscreen/allowlocation.h"
#include <window_management.hpp>
#include "windowmanage.hpp"
#include "nonetworkwidget.h"

horscreen *horscreen::mutual = nullptr;

horscreen::horscreen(QWidget *parent) : QWidget(parent)
{
    ::kabase::WindowManage::getWindowId(&m_windowId);
    initInteraction();
    mutual = this;

    //主题框架
    /* 适配kysdk的窗管 */
    // kdk::kabase::WindowManagement::setWindowMotifHint(this->winId());
    UkuiWindowHelper *helper = new UkuiWindowHelper(this);
    helper->removeTitleBar();

    //    setWindowFlags(Qt::FramelessWindowHint);
    this->setMinimumSize(904, 624);
    this->setWindowTitle(tr("Weather"));
    this->setFocusPolicy(Qt::ClickFocus);

    this->initWidget();
    this->initConnect();
    this->initLayout();
    
    m_changeCity->getChangeCityButton()->installEventFilter(this);
    m_changeCity->getCollectCityButton()->installEventFilter(this);
    this->installEventFilter(this);
    m_title->installEventFilter(this);
    
    // 初始化界面大小
    this->initMainWidgetSize();
    
}

void horscreen::initWidget()
{
    m_pixmap = QPixmap(":/res/background/100.png");
    m_label = new QLabel(this);
    m_label->setAccessibleName("kylin-weather_QLabel_Horscreen_label");
    m_label->move(0, 0);
    m_label->setAlignment(Qt::AlignRight); // 设置背景图片从右侧开始显示

    // 信创平板添加
    m_hMainlayout = new QHBoxLayout();
    m_hlayout = new QHBoxLayout();
    m_vlayout = new QVBoxLayout();
    m_mainLayout = new QVBoxLayout(this);
    m_mainCityLayout = new QVBoxLayout();
    m_hCityLayout = new QHBoxLayout();

    m_title = new title(this);
    m_title->setAccessibleName("kylin-weather_Horscreen_title");
    m_empty = new Empty(this);
    m_empty->setAccessibleName("kylin-weather_Horscreen_empty");
    m_changeCity = new ChangeCityWidget(this);
    m_changeCity->setAccessibleName("kylin-weather_Horscreen_ChangeCityWidget");

    m_widget = new QWidget(this);
    m_widget->setAccessibleName("kylin-weather_QWidget_Horscreen_widget");
    m_city = new city(m_widget);
    m_city->setAccessibleName("kylin-weather_Horscreen_city");
    m_scrollarea = new QScrollArea(this);
    m_scrollarea->setAccessibleName("kylin-weather_QScrollArea_Horscreen_scrollarea");
    m_scrollarea->setFocusPolicy(Qt::NoFocus); //设置焦点类型
    m_scrollarea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollarea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollarea->verticalScrollBar()->setProperty("drawScrollBarGroove", false);
    // 设置scrollarea背景透明
    QPalette scrollarea = m_scrollarea->palette();
    scrollarea.setColor(QPalette::Background, QColor(Qt::transparent));
    m_scrollarea->setPalette(scrollarea);
    m_scrollarea->setFrameShape(QFrame::NoFrame); // 设置滑动区无边框
    m_scrollwidget = new QWidget(m_scrollarea);
    m_scrollwidget->setAccessibleName("kylin-weather_QWidget_Horscreen_scrollwidget");
    m_scrollarea->setWidget(m_scrollwidget);
    //    m_scrollwidget->move(0, m_city->y() + m_city->height() +20);
    m_information = new information(m_scrollwidget);
    m_information->setAccessibleName("kylin-weather_Horscreen_information");

    m_searchView = new SearchView(this);
    m_searchView->setAccessibleName("kylin-weather_Horscreen_searchView");
    m_searchView->hide();
    m_searchView->move((this->width() - m_searchView->width()) / 2, (this->height() - m_searchView->height()) / 2);

    m_hintWidget = new PromptWidget(this);
    m_hintWidget->setAccessibleName("kylin-weather_Horscreen_hintWidget");
    m_hintWidget->setVisible(false);

    // 创建首次启动无网络界面
    m_noNetworkWidget = new NoNetworkWidget(this);
    m_noNetworkWidget->setAccessibleName("kylin-weather_Horscreen_noNetworkWidget");
    m_noNetworkWidget->hide();
    m_noNetworkWidget->resize(this->width(), this->height());
    m_noNetworkWidget->raise(); // 确保在最顶层

    m_titleSearchCityView = new TitleSearchCityView(this);
    m_titleSearchCityView->setAccessibleName("kylin-weather_Horscreen_titleSearchCityView");
    m_titleSearchCityView->hide();

    return;
}

void horscreen::initMainWidgetSize()
{
    switch (ViewVar::getInstance()->m_currentMode)
    {
        case CurrentMode::PCMode:
            slotChangePCMode();
            break;
        case CurrentMode::PadHMode:
            slotChangePadHMode();
            break;    
        case CurrentMode::PadVMode:
            slotChangePadVMode();
            break;
        default:
            slotChangePCMode();
            break;
    }
    return;
}

void horscreen::changeBackgroundImage(CurrentWeather m_currentWeather)
{
    QString code = m_currentWeather.cond_code;
    if (code == "" || code == "999")
        code = "100";
    qDebug() << "m_currentWeather.cond_code:" << code;
    QString path = ":/res/background/" + code + ".png";
    m_pixmap = QPixmap(path);
    // m_pixmap = m_pixmap.scaled(m_label->width(), m_label->height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QPixmap pixmap;

    switch (ViewVar::getInstance()->m_currentMode) {
    case CurrentMode::PadVMode:
        pixmap = m_pixmap.scaled( 1800 * (m_label->height() / 1240) + 200, m_label->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        break;
    default:
        pixmap = m_pixmap.scaled(m_label->width(), m_label->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        break;
    }
    m_label->setPixmap(pixmap);

    qDebug() << " ===== INFO ===== 更换背景 =====" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
}

void horscreen::initLayout()
{
    m_mainCityLayout->addWidget(m_widget);

    QHBoxLayout *hLayout1 = new QHBoxLayout;
    QHBoxLayout *hLayout2 = new QHBoxLayout;
    QHBoxLayout *hLayout3 = new QHBoxLayout;

    hLayout1->addWidget(m_changeCity);
    m_changeCity->raise();
    hLayout1->setContentsMargins(22, 0, 700, 0);
    hLayout2->addLayout(m_mainCityLayout);
    hLayout2->setContentsMargins(30, 0, 0, 0);
    hLayout3->addWidget(m_scrollarea);
    hLayout3->setContentsMargins(22, 0, 5, 0);

    m_vlayout->setSpacing(0);
    m_vlayout->setMargin(0);
    m_vlayout->addLayout(hLayout1);
    m_vlayout->addSpacing(-35);
    m_vlayout->addLayout(hLayout2, 1);
    m_vlayout->addSpacing(-25);
    m_vlayout->addLayout(hLayout3, 2);
    m_vlayout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *errorLayout = new QHBoxLayout;
    errorLayout->addWidget(m_hintWidget);
    errorLayout->setContentsMargins(0, 0, 0, 0);


    m_mainLayout->setSpacing(0);
    m_mainLayout->setMargin(0);
    m_mainLayout->addWidget(m_title);
    m_mainLayout->addLayout(m_vlayout);
    m_mainLayout->addLayout(errorLayout);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(m_mainLayout);
}

void horscreen::changeHeight(double detio)
{
    m_scrollwidget->resize(900 -45 , (138 + 216 + 120 + 170 ));
    m_information->resize(m_scrollwidget->width(), m_scrollwidget->height());
    m_scrollarea->resize(m_scrollwidget->width(), m_scrollwidget->height());
}

void horscreen::keyPressEvent(QKeyEvent *event)
{
    if(event->modifiers() == Qt::ControlModifier&&event->key()==Qt::Key_E){
        qDebug()<<__LINE__<<__func__;
        m_title->m_searchLineEdit->setFocus();
    }
    // F1快捷键打开用户手册
    if (event->key() == Qt::Key_F1) {
        kdk::kabase::UserManualManagement userManualTest;
        if (!userManualTest.callUserManual("kylin-weather")) {
            qCritical() << "user manual call fail!";
        }
        return;
    }
}

void horscreen::showOrHideMainWindow()
{
    //规避方案-wayland环境下，天气的机制拿不到windowid，走qt的时候，wayland环境拿不到窗口状态，所以暂时设置标志位区分窗口是否应被拉起
//    if (ViewVar::widgetIsShow) {
//        ViewVar::widgetIsShow = false;
//    } else {
//        ViewVar::widgetIsShow = true;
//    }
//    if (QGuiApplication::platformName().startsWith(QLatin1String("wayland"), Qt::CaseInsensitive)) {
//        if (ViewVar::widgetIsShow) {
//            handleIconClickedSub(); //显示在屏幕中央
//        } else {
//            this->showMinimized();
//        }
//        return;
//    }
    if (this->isHidden() || this->isMinimized() || !this->isActiveWindow()) {
        handleIconClickedSub(); //显示在屏幕中央
    } else {
        this->showMinimized();
    }
}

// show或hide搜索弹窗
void horscreen::showSearchView()
{
    if (m_searchView->isHidden()) {
        m_searchView->raise();
        m_searchView->show();
        emit searchClear();
        Interaction::getInstance()->loadCollectCity();
    } else {
        //发送信号给搜索界面-重置界面显示
        emit searchClear();
        m_searchView->hide();
    }
}

void horscreen::initInteraction()
{
    connect(Interaction::getInstance(), &Interaction::netUiShow, this,
            &horscreen::recieveNetSignal); //返回网络错误，给UI发信号
    connect(Interaction::getInstance(), &Interaction::serverUiShow, this,
            &horscreen::recieveNetSignal); //返回请求服务器错误，给UI发信号
    connect(Interaction::getInstance(), &Interaction::dataUiShow, this,
            &horscreen::recieveNetSignal); //返回数据解析错误，给UI发信号
    connect(Interaction::getInstance(), &Interaction::netTimeout, this, &horscreen::recieveNetSignal); //超时返回
    connect(Interaction::getInstance(), &Interaction::geoUiShow, this, &horscreen::recieveNetSignal);         //定位失败

    connect(Interaction::getInstance(), &Interaction::createTrayIcon, this, &horscreen::createTrayIcon); //启动创建托盘
    connect(Interaction::getInstance(), &Interaction::showCurrent, this, &horscreen::showCurrent); //接收当前城市信息
    connect(Interaction::getInstance(), &Interaction::showForcast, this, &horscreen::showForcast); //接收预测信息
    connect(Interaction::getInstance(), &Interaction::showLifeStyle, this,
            &horscreen::showLifeStyle); //接收生活指数信息
    connect(Interaction::getInstance(), &Interaction::showHourly, this,
            &horscreen::showHourly); //增加逐小时天气接口--临时
    connect(Interaction::getInstance(), &Interaction::closePro, this, &horscreen::closeActivated); //关闭程序-退出

    // 后端返回存在数据/搜索“”时
    connect(Interaction::getInstance(), &Interaction::searchUiShow, this, &horscreen::slotShowTitleSearchCityView);

}

void horscreen::initConnect()
{
    //最大化和还原
    //全屏按钮注释代码
    connect(m_title, &title::recovery, this, &horscreen::changOrigSize);
    // connect(m_city->m_changeCityBtn, &QPushButton::clicked, this,
    //         &horscreen::showSearchView); //搜索-添加城市弹窗显隐，按钮text变化
    connect(m_changeCity->getChangeCityButton(), &QPushButton::clicked, this, &horscreen::showSearchView); //搜索-添加城市弹窗显隐，按钮text变化
    connect(m_searchView, &SearchView::searchWidState, this, &horscreen::showSearchView); //点击切换城市后的主界面处理
    connect(this, &horscreen::searchClear, m_searchView, &SearchView::searchReset); //搜索弹窗界面重置

    //当前城市信息
    connect(this, &horscreen::sendCurrentInfor, m_city, &city::getCurrentInfor);
    connect(this, &horscreen::sendCurrentInfor, m_changeCity, &ChangeCityWidget::getCurrentInfor);
    //当前城市天气对应的背景图
    connect(this, &horscreen::sendCurrentInfor, this, &horscreen::changeBackgroundImage);
    // lifestyle
    connect(this, &horscreen::sendLifeStyleInfor, m_information, &information::getLifeStyleInfor);
    // 发送信号到 information->onehourweather/onehourweather/lifestyle,判断是全屏还是普通界面
    connect(this, &horscreen::sendIsFullScreen, m_information, &information::sendIsFullScreen);
    //全屏后的 city 布局
    connect(this, &horscreen::sendIsFullScreen, m_city, &city::isFullScreen);
    connect(this, &horscreen::sendIsFullScreen, m_changeCity, &ChangeCityWidget::isFullScreen);
    //当前城市放大缩小
    // connect(this, &horscreen::sendDetio, m_city, &city::initLayout);
    //当前城市上方空白widget放大缩小
    connect(this, &horscreen::sendDetio, m_empty, &Empty::resizeLayout);
    // 逐小时信息
    connect(this, &horscreen::sendHourlyInfor, m_information, &information::sendHourlyInfor);
    // 7周内天气信息
    connect(this, &horscreen::sendForcastInfor, m_information, &information::sendForcastInfor);
    // 7天预测天气图标放大缩小
    connect(this, &horscreen::sendWeatherIconInfor, m_information, &information::weekWeatherIcon);
    // 网络异常弹窗
    connect(this, &horscreen::recallInitLayout, m_hintWidget, &PromptWidget::initLayout);
    //    connect(m_city->m_changeCityBtn,&QPushButton::clicked,m_hintWidget,&PromptWidget::test);
    //    connect(this,&horscreen::recallInitLayout,m_hintWidget,&PromptWidget::test);
    connect(this, &horscreen::showCenterAfter, Interaction::getInstance(),
            &Interaction::showCenterAfter); //在屏幕中间显示

    connect(this, &horscreen::sendIsFullScreen, m_hintWidget, &PromptWidget::isFullScreen);

    //标题栏刷新城市
    connect(m_title, &title::sigRefreshWeather, Interaction::getInstance(), &Interaction::refresh);

    connect(m_title, &title::sigLineEditGetFocus, m_searchView, &SearchView::hide);
    
    connect(m_title, &title::sigTitleSearchCity, this, [=](){
        m_titleSearchCityView->setViewModel(false);
    });
    connect(this, &horscreen::sigTitleLineEditClear, m_title, &title::slotSearchLineEditClear);
    // connect(m_titleSearchCityView, &TitleSearchCityView::searchWidState, this, &horscreen::slotShowTitleSearchCityViewUi);//点击切换城市后的主界面处理
    connect(m_titleSearchCityView, &TitleSearchCityView::cityListNull, this, &horscreen::slotShowTitleSearchCityView);//点击切换城市后的主界面处理
    connect(m_titleSearchCityView, &TitleSearchCityView::sigTitleSearchChangeCityToMain, m_title, &title::slotSearchLineEditClear);
    connect(m_titleSearchCityView, &TitleSearchCityView::sigTitleSearchChangeCityToMain, this, &horscreen::slotShowTitleSearchCityViewUi);

    connect(this, &horscreen::sigGetDataFromCore, m_changeCity, &ChangeCityWidget::changeCollectBtnIcon);
    connect(this, &horscreen::sigGetDataFromCore, m_title, &title::slotChangeLineEditStyle);
    //    // 首次允许定位弹窗点击“是”更改菜单内允许定位为“是”
    //    connect(m_AllowLocationwidget,&AllowLocation::setMenuModuleAllowLocationYes,m_title,&title::setMenuModuleAllowLocationYes);

    connect(ViewVar::getInstance(), &ViewVar::sigChangeModeToPc, this, &horscreen::slotChangePCMode);
    connect(ViewVar::getInstance(), &ViewVar::sigChangeModeToPadH, this, &horscreen::slotChangePadHMode);
    connect(ViewVar::getInstance(), &ViewVar::sigChangeModeToPadV, this, &horscreen::slotChangePadVMode);
}

void horscreen::changOrigSize()
{
    if (this->isMaximized()) {
        this->showNormal();
        this->setFixedSize(900, 620);
        m_title->btnMaximum->setIcon(QIcon::fromTheme("window-maximize-symbolic")); //主题库的全屏图标
        m_title->btnMaximum->setToolTip(tr("full srceen"));
    } else {
        this->setMaximumSize(16777215, 16777215);
        this->showMaximized();
        m_title->btnMaximum->setIcon(QIcon::fromTheme("window-restore-symbolic")); //主题库的恢复图标
        m_title->btnMaximum->setToolTip(tr("recovery"));
        //全屏-埋点
        kdk::kabase::BuriedPoint buriedPointFullScreen;
        if (buriedPointFullScreen.functionBuriedPoint(kdk::kabase::AppName::KylinWeather,
                                                      kdk::kabase::BuriedPoint::PT::KylinWeatherFullScreen)) {
            qCritical() << "Error : buried point fail FullScreen!";
        };
    }
}

//不要删除
void horscreen::recieveNetSignal(QString netError)
{
    netErrorInfor = netError;
    qDebug() << "neterror========" << netError;
    
    // 判断是否首次启动且无网络
    if (usedNet == 1 && CoreVar::HAS_NETWORK == false) {
        // 首次启动且无网络，显示专门的无网络界面
        m_firstStartNoNetwork = true;
        slotShowNoNetworkWidget();
        return;
    }
    
    // 非首次启动的网络错误，显示原有的错误提示
    // 设置异常弹窗内容
    m_hintWidget->initLayout(netError, m_isFullScreen);
    
    // 显示异常弹窗
    m_hintWidget->setVisible(true);
    isShow = true;
     
    // 搜索弹窗出现后，网络出现异常，异常提示界面弹出，搜索弹窗应该收回
//    m_searchView->hide();
}

void horscreen::showCurrent(CurrentWeather m_currentWeather)
{    
    qDebug() << " ##### Function Positioning ##### 城市信息显示 ##### " <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
    CoreVar::GET_CITY_INFO = true; 
    m_city->show();
    m_searchView->hide();
    
    // 如果是首次启动无网络状态，隐藏无网络界面
    if (m_firstStartNoNetwork) {
        slotHideNoNetworkWidget();
        m_firstStartNoNetwork = false;
    }
    
    // 隐藏异常弹窗并恢复information区域高度
    isShow = false;
    m_hintWidget->setVisible(isShow);

    emit sendCurrentInfor(m_currentWeather);
    QString code = m_currentWeather.cond_code;
    //创建托盘图标
    //只有成功访问服务器且得到数据信息才显示托盘
    if (m_trayIcon != nullptr) {
        updateTrayIcon(code);
    }else{
        createTrayIcon(code);
    }
    // m_city->m_changeCityBtn->setEnabled(true);
    m_changeCity->getChangeCityButton()->setEnabled(true);
    m_changeCity->getCollectCityButton()->setEnabled(true);
    emit sigGetDataFromCore();
}

void horscreen::showForcast(ForecastWeather forecastWeather)
{
    emit sendForcastInfor(forecastWeather, m_isFullScreen);
    // 判断前面是否有网导致界面显示出正常天气信息
    // 0 : 使用过网络； 1 : 未使用过(初始值为 1)
    // 确保 emit sendWeatherIconInfor(m_isFullScreen);只有在 horscreen::showForcast
    // 发送过 emit sendForcastInfor(forecastWeather,m_isFullScreen);后才能发送
    if (usedNet == 1) {
        usedNet = 0;
        // 如果是首次启动无网络状态，现在有网络了，隐藏无网络界面
        if (m_firstStartNoNetwork) {
            slotHideNoNetworkWidget();
            m_firstStartNoNetwork = false;
        }
    }
    // m_city->m_changeCityBtn->setEnabled(true);
    m_changeCity->getChangeCityButton()->setEnabled(true);
    m_changeCity->getCollectCityButton()->setEnabled(true);
}

void horscreen::showLifeStyle(LifeStyle m_lifeStyle)
{
    emit sendLifeStyleInfor(m_lifeStyle, m_isFullScreen);
}
//增加逐小时天气接口---临时用
void horscreen::showHourly(QVariant var)
{
    QList<Hourly> tmp_list = var.value<QList<Hourly>>();
    emit sendHourlyInfor(tmp_list, m_isFullScreen);
}
//创建托盘图标
void horscreen::createTrayIcon(QString code)
{
    if (ViewVar::platFormList.contains(ViewVar::platForm)) {
        return;
    }
    m_trayIcon = new QSystemTrayIcon(this);

    QString weatherStr = CoreVar::getSettings("weather").toString();
    QStringList weatherList = weatherStr.split(",");
    if (weatherList.size() < 10) {
        m_trayIcon->setToolTip(QString(tr("Weather")));
    } else {
        m_trayIcon->setToolTip(QString(weatherList[2] + " " + weatherList[3] + " " + weatherList[5] + "C"));
    }
    if (code == "") {
        code = "999";
    }

    QString trayPath = ":/res/weather-icon/PNG16/2x/" + code + ".png";
    m_trayIcon->setIcon(QIcon(trayPath));
    //    m_trayIcon->setIcon()
    //添加托盘菜单
    m_mainMenu = new QMenu;
    m_mainMenu->setAccessibleName("kylin-weather_QMenu_Horscreen_mainMenu");
    UkuiWindowHelper *helper = new UkuiWindowHelper(m_mainMenu);
    helper->removeTitleBar();
    m_openAction = new QAction(tr("Open Weather"), this); //打开麒麟天气
    m_quitAction = new QAction(tr("Close"), this);         //退出
    m_mainMenu->addAction(m_openAction);
    m_openAction->setIcon(QIcon::fromTheme(QString("kylin-weather")));
    m_mainMenu->addSeparator();
    m_mainMenu->addAction(m_quitAction);
    m_quitAction->setIcon(QIcon::fromTheme(QString("window-close-symbolic")));

    m_trayIcon->setContextMenu(m_mainMenu);
    m_trayIcon->show();

    connect(m_openAction, &QAction::triggered, this, [=] {
        if (this->isHidden() || this->isMinimized() || !this->isActiveWindow()) {
            handleIconClickedSub();
            Interaction::getInstance()->showCenterAfter();
            //            emit showCenterAfter();
        } else {
            return;
        }
    });
    //    connect(this,&horscreen::showInCenter,Interaction::getInstance,&Interaction::showInCenter);//在屏幕中间显示
    connect(m_quitAction, &QAction::triggered, this, &horscreen::closeActivated);
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &horscreen::iconActivated);
}

void horscreen::updateTrayIcon(QString code)
{
    QString weatherStr = CoreVar::getSettings("weather").toString();
    QStringList weatherList = weatherStr.split(",");
    if (weatherList.size() < 10) {
        m_trayIcon->setToolTip(QString(tr("Weather")));
    } else {
        m_trayIcon->setToolTip(QString(weatherList[2] + " " + weatherList[3] + " " + weatherList[5] + "C"));
    }
    if (code == "") {
        code = "999";
        recieveNetSignal(tr("Server data abnormality!"));
    }
    QString trayPath = ":/res/weather-icon/PNG16/2x/" + code + ".png";
    m_trayIcon->setIcon(QIcon(trayPath));
}

void horscreen::handleIconClickedSub()
{
    if (this->m_title->g_menu->m_aboutWindow) {
        this->m_title->g_menu->m_aboutWindow->close();
    }
//    this->hide();
    this->show();
    Interaction::getInstance()->refresh();

    this->activateWindow();
    this->raise();
    ViewVar::widgetIsShow = true;
    showLocateWidget();
}

//托盘图标被点击
void horscreen::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::MiddleClick:
        this->showOrHideMainWindow();
        break;
    case QSystemTrayIcon::DoubleClick:
        this->showOrHideMainWindow();
        break;
    case QSystemTrayIcon::Context:
        //右键点击托盘图标弹出菜单
        // showTrayIconMenu(); //显示右键菜单
        m_mainMenu->show();
        break;
    default:
        break;
    }
}

void horscreen::closeActivated()
{
    qApp->quit();
    return;
}

void horscreen::showLocateWidget()
{
    if (CoreVar::getSettings("locate-widget").toString() == "0" && this->isVisible()) {
        if (m_AllowLocationwidget == nullptr) {
            m_AllowLocationwidget = new AllowLocation(this);
            m_AllowLocationwidget->setAccessibleName("kylin-weather_Horscreen_AllowLocationwidget");
            connect(m_AllowLocationwidget, &AllowLocation::setMenuModuleAllowLocationYes, m_title,
                    &title::setMenuModuleAllowLocationYes);
            m_AllowLocationwidget->resize(350, 174);
        }
        //暂时规避方案
        QTimer *timer = new QTimer;
        timer->setSingleShot(true);
        connect(timer, &QTimer::timeout, this, &horscreen::showAllowLocationWidget);
        connect(timer, &QTimer::timeout, timer, &QTimer::deleteLater);
        timer->start(500);
    }
}

void horscreen::showAllowLocationWidget()
{
    if (!m_AllowLocationwidget) {
        return;
    }
    m_AllowLocationwidget->move(this->geometry().center() - m_AllowLocationwidget->rect().center());
    UkuiWindowHelper *helper = new UkuiWindowHelper(m_AllowLocationwidget);
    helper->removeTitleBar();
    m_AllowLocationwidget->show();
}

//不要删--resize事件，使搜索弹窗可随主窗体尺寸变化而变化
void horscreen::resizeEvent(QResizeEvent *event)
{
    //    initLayout();
    if ((ViewVar::getInstance()->m_currentMode == CurrentMode::PCMode)) {
        m_isFullScreen = 0;
    } else if ((ViewVar::getInstance()->m_currentMode == CurrentMode::PadHMode)) {
        m_isFullScreen = 1;
    }  else if ((ViewVar::getInstance()->m_currentMode == CurrentMode::PadVMode)) {
        m_isFullScreen = 1;
    } 

    Q_UNUSED(event);
    m_searchView->move((this->width() - m_searchView->width()) / 2, (this->height() - m_searchView->height()) / 2);
    
    // 更新无网络界面大小
    if (m_noNetworkWidget) {
        m_noNetworkWidget->resize(this->width(), this->height());
    }
    
    detio = double(this->height()) / 620;
    changeHeight(detio);
    m_label->resize(this->width(), this->height());
    if (!m_pixmap.isNull()) {
        QPixmap pixmap;
        switch (ViewVar::getInstance()->m_currentMode) {
        case CurrentMode::PadVMode:
            pixmap = m_pixmap.scaled(1800 * (m_label->height() / 1240) + 200 , m_label->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            break;
        default:
            pixmap = m_pixmap.scaled(m_label->width(), m_label->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            break;
        }
        m_label->setPixmap(pixmap);
    }

    emit sendDetio(detio);
    emit sendIsFullScreen(m_isFullScreen);
    if (usedNet == 0) {
        emit sendWeatherIconInfor(m_isFullScreen);
    }
    // 如果网络提示界面是显示状态，则重新调用 PromptWidget::initLayout
    if (isShow == true) {
        m_hintWidget->initLayout(netErrorInfor, m_isFullScreen);
    }
    showLocateWidget();
}

//不要删--event事件，使搜索弹窗可在界面内失焦关闭
bool horscreen::event(QEvent *event)
{
    //点击别处，使搜索框失去焦点
    // if (event->type() == QEvent::MouseButtonPress
    //     && (!m_searchView->geometry().contains(this->mapFromGlobal(QCursor::pos())))) {
    //     //发送信号给搜索界面-重置界面显示
    //     emit searchClear();
    //     m_searchView->hide();
    //     // m_city->m_changeCityBtn->setText(tr("【Change】"));
    // }
    return QWidget::event(event);
}

void horscreen::hideSearchView()
{
    showSearchView();
}

void horscreen::slotShowTitleSearchCityView(bool hasCity)
{
    // 判断是否为标题栏的搜索进行的搜索
    if (!ViewVar::m_isTitleSearch) {
        // 当前搜索不是标题栏进行搜索，直接返回
        return;
    }
    if (hasCity) {
        // 获取搜索框的全局位置和宽度
        QPoint globalPox = this->mapToGlobal(QPoint(0, 0));
        int x = m_title->getSearchLinePositionX() - globalPox.x();
        int y = m_title->getSearchLinePositionY() - globalPox.y();
        int width = m_title->getSearchLineWidth();

        // 设置m_titleSearchCityView的位置和宽度
        m_titleSearchCityView->setFixedWidth(width);
        m_titleSearchCityView->move(x, y);

        m_titleSearchCityView->show();
    }
    m_titleSearchCityView->searchUiChange(hasCity);
    m_titleSearchCityView->setViewModel(hasCity);
    
    return;
}

void horscreen::slotShowTitleSearchCityViewUi()
{
    m_titleSearchCityView->hide();
    return;
}

bool horscreen::eventFilter(QObject *watch, QEvent *e)
{
    if (e->type() == QEvent::MouseButtonPress || e->type() == QEvent::FocusIn) {
        if (watch != m_searchView && watch != m_changeCity->getChangeCityButton() && (!m_searchView->geometry().contains(this->mapFromGlobal(QCursor::pos())))) {
            //发送信号给搜索界面-重置界面显示
            emit searchClear();
            m_searchView->hide();
        }
    }
    
    // 鼠标点击位置不是搜索列表，发出信号，搜索框内容清空
    // 先判断当前是否进行标题栏搜索
    if (ViewVar::m_isTitleSearch) { 
        // 判断触发点击事件的对象是否为搜索区域
        if (watch != m_titleSearchCityView &&  e->type() == QEvent::MouseButtonPress) {
            slotShowTitleSearchCityViewUi();
            emit sigTitleLineEditClear();
        }
    }

    return QObject::eventFilter(watch, e);
}

void horscreen::slotChangePCMode()
{
    this->setFixedSize(900, 620);
    m_widget->setFixedSize(this->width(), 200);
    m_city->setFixedSize(this->width(), 200);
    m_city->slotChangePCMode();
    this->showMaximized();
    if (ViewVar::widgetIsShow == false){
        this->hide();
    }
    return;
}

void horscreen::slotChangePadHMode()
{
    this->setFixedSize(ViewVar::getInstance()->m_currentSize.first,ViewVar::getInstance()->m_currentSize.second);
    m_widget->setFixedSize(this->width(), 200);
    m_city->setFixedSize(this->width(), 200);
    m_city->slotChangePadHMode();
    return;
}

void horscreen::slotChangePadVMode()
{
    this->setFixedSize(ViewVar::getInstance()->m_currentSize.first,ViewVar::getInstance()->m_currentSize.second);
    m_widget->setFixedSize(this->width(), 200);
    m_city->setFixedSize(this->width(), 200);
    m_city->slotChangePadVMode();
    return;
}

// 显示首次启动无网络界面
void horscreen::slotShowNoNetworkWidget()
{
    qDebug() << "显示首次启动无网络界面";
    if (m_noNetworkWidget) {
        m_noNetworkWidget->resize(this->width(), this->height());
        m_noNetworkWidget->show();
        m_noNetworkWidget->raise(); // 确保在最顶层
        
        // 隐藏其他界面元素
        m_city->hide();
        m_hintWidget->setVisible(false);
        m_title->raise();
        m_title->show();
        m_title->m_searchLineEdit->hide();
    }
}

// 隐藏首次启动无网络界面
void horscreen::slotHideNoNetworkWidget()
{
    qDebug() << "隐藏首次启动无网络界面";
    if (m_noNetworkWidget) {
        m_noNetworkWidget->hide();
        
        // 显示正常界面元素
        m_city->show();
        m_title->m_searchLineEdit->show();
    }
}
