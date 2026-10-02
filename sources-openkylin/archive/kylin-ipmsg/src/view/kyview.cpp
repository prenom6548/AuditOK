/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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

#include "kyview.h"
#include "global/utils/inisetting.h"
#include "global/utils/addrset.h"
#include "controller/control.h"
#include <QColor>
#include <QWindow>
#include <QMessageBox>
#include <KWindowEffects>
#include <gsettingmonitor.h>
#include <usermanual.h>
#include "windowmanage.hpp"
#include "ukuiwindowhelper.h"

#include <QApplication>
#include <QDesktopWidget>
#include <kysdk/applications/windowmanager/windowmanager.h>
#include <windowmanager/windowmanager.h>
#include <kysdk/diagnosetest/libkydatacollect.h>


KyView::KyView(QWidget *parent) : QWidget(parent)
    , m_titleBar(nullptr)
    , m_mainWid(nullptr)
    , m_stackedWid(nullptr)
    , m_blankWid(nullptr)
    , m_localInfo(nullptr)
    , m_friendView(nullptr)
    , m_searchPage(nullptr)
    , m_chatMsg(nullptr)
    , m_mainLayout(nullptr)
    , m_trayIconWid(nullptr)
    , m_windowHelper(nullptr)
{
    ::kabase::WindowManage::getWindowId(&m_windowId);
    this->setFixedSize(WINDOWW, WINDOWH);

    // 初始化组件
//    setWidgetUi();

    m_windowHelper = new UkuiWindowHelper(this);


    this->setAttribute(Qt::WA_TranslucentBackground, true);
    // 设置组件样式

//    setWidgetStyle();

    // 获取窗口
    connect(kdk::WindowManager::self(),&WindowManager::windowAdded,this,[=](const WindowId& windowId) {
        /* 注意：最新创建的窗体被设置为操作窗体，所有接口函数操作的窗口都是该最新创建的窗体 */
        if ((quint32)getpid() == WindowManager::getPid(windowId)) {
            static bool firstAddFlag = true;
            if(firstAddFlag){
                m_mainwindowId = windowId;
                firstAddFlag = false;
            }
        }
    });
}

KyView::~KyView()
{
    if (GlobalUtils::DEBUG_MODE) {
        qDebug() << "~KyView()";
    }

    //在程序推出前调用，不需要传入参数
    kdk_dia_upload_program_exit();

    // 仅在非单元测试环境下执行某些清理逻辑
    if (Control::getInstance()) {
        Control::getInstance()->sayGoodbye();
    }

#ifndef TEST_MODE
    LocalInfo::getInstance()->deleteLater();
    FriendListView::getInstance()->deleteLater();
    m_trayIconWid->deleteLater();
    m_searchPage->deleteLater();
    m_blankWid->deleteLater();

    Control::getInstance()->deleteLater();    
#else
    // 安全移除单例组件，防止它们随 KyView 一起被销毁
    // 这些单例由其各自的 getInstance() 管理生命周期
    if (m_stackedWid) {
        if (m_friendView) m_stackedWid->removeWidget(m_friendView);
        if (m_searchPage) m_stackedWid->removeWidget(m_searchPage);
    }
    
    if (m_mainLayout) {
        if (m_localInfo) m_mainLayout->removeWidget(m_localInfo);
    }

    // 断开与单例的父子关系，确保单例不会被销毁
    if (m_localInfo) m_localInfo->setParent(nullptr);
    if (m_friendView) m_friendView->setParent(nullptr);
    if (m_searchPage) m_searchPage->setParent(nullptr);
    if (m_trayIconWid) m_trayIconWid->setParent(nullptr);
#endif

}

// 生成静态实例
KyView *KyView::getInstance()
{
    static KyView *instance = nullptr;
    if (nullptr == instance) {
        instance = new KyView();
        instance->setAccessibleName("kylin-ipmsg_KyView_instance");
    }
    return instance;
}

void KyView::showList()
{
    // 初始化组件
     setWidgetUi();

    m_localInfo->showList();

    // 设置组件样式
    setWidgetStyle();
}

// 初始化组件
void KyView::setWidgetUi()
{
    // 初始化并监听gsetting
    initGsetting();

//    this->setFixedSize(WINDOWW, WINDOWH);

    // mainWid = new QWidget(this);

    m_mainLayout = new QVBoxLayout();
    m_mainLayout->setObjectName("kylin-ipmsg_kyview_QVBoxLayout_mmainLayout");

    // 堆栈窗口
    m_stackedWid = new QStackedWidget();
    m_stackedWid->setAccessibleName("kylin-ipmsg_kyview_QStackedWidget_mstackedWid");

    // 空白页
    m_blankWid = new BlankPage();
    m_blankWid->setAccessibleName("kylin-ipmsg_kyview_BlankPage_mblankWid");

    // 标题栏
    m_titleBar = new TitleBar();
    m_titleBar->setAccessibleName("kylin-ipmsg_kyview_TitleBar_mtitleBar");
    m_titleBar->setFixedHeight(GlobalSizeData::TITLEBAR_HEIGHT);

    // 本机信息
    m_localInfo = LocalInfo::getInstance(this);
    //m_localInfo->installEventFilter(this);

    // 好友列表
    m_friendView = FriendListView::getInstance();

    // 搜索列表
    m_searchPage = SearchPage::getInstance();

    //创建托盘图标
    m_trayIconWid = TrayIconWid::getInstance();
    // m_trayIconWid->installEventFilter(this);
    // m_trayIconWid->show();

    // 堆栈窗体
    m_stackedWid->addWidget(m_blankWid);
    m_stackedWid->addWidget(m_friendView);
    m_stackedWid->addWidget(m_searchPage);

    if (this->m_friendView->model()->rowCount() == 0) {
        m_stackedWid->setCurrentIndex(0);
    } else {
        m_stackedWid->setCurrentIndex(1);
    }

    // 将组件添加到布局中
    m_mainLayout->addWidget(m_titleBar, Qt::AlignTop);
    m_mainLayout->addWidget(m_localInfo, Qt::AlignTop);
    m_mainLayout->addWidget(m_stackedWid, Qt::AlignTop);
    m_mainLayout->setMargin(0);
    m_mainLayout->setSpacing(0);
    this->setLayout(m_mainLayout);

    connect(m_titleBar->m_pCloseButton, &QPushButton::clicked, this, &KyView::deleteLater);
    connect(this->m_searchPage, &SearchPage::sigSearchEmpty, this, &KyView::slotShowSearchBlankPage);
    connect(this->m_searchPage, &SearchPage::sigSearchExist, this, &KyView::slotChangeStatck);
    connect(this->m_friendView, &FriendListView::sigFriendState, this, &KyView::slotFriendInterface);
    connect(this->m_friendView, &FriendListView::sigFriendsEmpty, this, &KyView::slotShowBlankPage);

    connect(this, &KyView::sigChangeBlank, this, &KyView::slotShowBlankPage);
    connect(this, &KyView::sigChangeFriend, this, &KyView::slotFriendInterface);
    connect(this, &KyView::sigChangeSearch, this, &KyView::slotSearchInterface);
    connect(this, &KyView::sigChangeSearchBlank, this, &KyView::slotShowSearchBlankPage);

    switch (GlobalData::getInstance()->m_currentMode)
    {
        case CurrentMode::PCMode:
        slotChangePCMode();        
            break;
        case CurrentMode::HMode:
        slotChangeHScreenMode();
            break;
        case CurrentMode::VMode:
        slotChangeVScreenMode();
            break;
    }
    connect(GlobalData::getInstance(), &GlobalData::signalChangePC, this, &KyView::slotChangePCMode);
    connect(GlobalData::getInstance(), &GlobalData::signalChangeFlatH, this, &KyView::slotChangeHScreenMode);
    connect(GlobalData::getInstance(), &GlobalData::signalChangeFlatV, this, &KyView::slotChangeVScreenMode);

     m_windowHelper= new UkuiWindowHelper(this);
     m_windowHelper->setBlurEffect(QRegion(), 0 ,true);
}

// 设置组件样式
void KyView::setWidgetStyle()
{
    this->setWindowTitle(tr("Messages"));

    // //毛玻璃
    // // this->setProperty("useSystemStyleBlur", true);
    // this->setAttribute(Qt::WA_TranslucentBackground, true);
    //UkuiWindowHelper *windowhelper = new UkuiWindowHelper(this);
    //windowhelper->setBlurEffect(QRegion());

    //失焦仍显示提示信息
    this->setAttribute(Qt::WA_AlwaysShowToolTips);

    // 应用居中
    QScreen *screen = QGuiApplication::primaryScreen();
    this->move(screen->geometry().center() - this->rect().center());

    this->update();
    m_titleBar->changeTheme();
    m_localInfo->changeTheme();
    m_searchPage->changeTheme();
    m_blankWid->changeTheme();
    TrayIconWid::getInstance()->changeTheme();
}

// 初始化并监听gsetting
void KyView::initGsetting()
{
    // 主题
    GlobalData::getInstance()->getSystemTheme();

    this->connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemThemeChange, this, [=]() {
        GlobalData::getInstance()->getSystemTheme();
        m_localInfo->changeTheme();
        m_titleBar->changeTheme();
        TrayIconWid::getInstance()->changeTheme();
        this->update();
    });

    // 透明度
    GlobalData::getInstance()->getSystemTransparency();

    this->connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemTransparencyChange, this, [=]() {
        GlobalData::getInstance()->getSystemTransparency();
        transChange();
    });
    return;
}

// 显示主界面后检查本机昵称
void KyView::checkLocalName()
{
    this->m_localInfo->checkLocalName();
}

// 窗口关闭事件
void KyView::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    this->hide();

    if (m_trayIconWid) {
        m_trayIconWid->hideTrayIcon();
    }

    this->deleteLater();
}

// 透明度变化
void KyView::transChange()
{
    // m_titleBar->update();
    m_localInfo->update();
    m_trayIconWid->update();
}

// 绘制事件
void KyView::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    /* 反锯齿 */
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setCompositionMode(QPainter::CompositionMode_Source);

    // m_localInfo->update();
    // m_titleBar->update();
}

void KyView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_F1) {
        // 帮助点击事件处理
        kdk::UserManual userManual;
        if (!userManual.callUserManual("kylin-ipmsg")) {
            qCritical() << "user manual call fail!";
        }
    }
    if(event->modifiers() == Qt::ControlModifier&&event->key()==Qt::Key_E){
        qDebug()<<__LINE__<<__func__;
       m_localInfo->m_searchLineEdit->setFocus();
    }
}

bool KyView::event(QEvent *event)
{
//    if (event->type() == QEvent::WindowActivate) {
//        GlobalData::getInstance()->getSystemTransparency();
//        m_backColor.setAlpha(GlobalSizeData::BLUR_TRANSPARENCY);
//        m_windowHelper->setBlurEffect(QRegion(), 0, true);

//    }
//    else if (event->type() == QEvent::WindowDeactivate){
//        m_backColor.setAlpha(255);
//        m_windowHelper->setBlurEffect(QRegion(), 0, false);
//    }

    return QWidget::event(event);
}

// 事件过滤器
bool KyView::eventFilter(QObject *watch, QEvent *event)
{
    if (m_localInfo->eventFilter(watch, event)) {
        return QObject::eventFilter(watch, event);
    }
    return false;
}

// 切换堆栈窗口
void KyView::slotChangeStatck()
{
    if (this->m_localInfo->m_searchLineEdit->text() == "") {
        if (this->m_friendView->model()->rowCount() != 0) {
            emit sigChangeFriend();
        } else {
            emit sigChangeBlank();
        }
    } else {
        emit sigChangeSearch();
    }
    return;
}

void KyView::slotSearchInterface()
{
    m_stackedWid->setCurrentIndex(2);
}

void KyView::slotShowBlankPage()
{
    m_stackedWid->setCurrentIndex(0);
    m_blankWid->changeBlankText();
}

void KyView::slotShowSearchBlankPage()
{
    m_stackedWid->setCurrentIndex(0);
    m_blankWid->changeSearchText();
}

void KyView::slotFriendInterface()
{
    m_stackedWid->setCurrentIndex(1);
}

//QString KyView::getWindowId(void)
//{
//    kabase::WindowManage::getWindowId(&m_windowId);
//    return m_windowId;
//}

void KyView::slotChangeHScreenMode()
{
    int h = GlobalData::getInstance()->m_currentSize.second;
    this->setFixedSize(WINDOWW_MAX_H_SCREEN, h);
    m_titleBar->slotChangeHScreenMode();
    m_localInfo->slotChangeFlatHSize();
    ::kabase::WindowManage::setMiddleOfScreen(this);

    return;
}

void KyView::slotChangeVScreenMode()
{
    int w = GlobalData::getInstance()->m_currentSize.first;
    int h = GlobalData::getInstance()->m_currentSize.second;
    this->setFixedSize(w, h);
    m_titleBar->slotChangeVScreenMode();
    m_localInfo->slotChangeFlatVSize();
    ::kabase::WindowManage::setMiddleOfScreen(this);
    
    return;
}

void KyView::slotChangePCMode()
{
    this->setFixedSize(WINDOWW, WINDOWH);
    m_titleBar->slotChangePCMode();
    m_localInfo->slotChangePCSize();
    ::kabase::WindowManage::setMiddleOfScreen(this);

    return;
}
