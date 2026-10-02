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

#include <QApplication>
#include <usermanual.h>
#include <QPainterPath>

#include "windowmanage.hpp"

#include "view/titlebar/titleseting.h"
#include "trayiconwid.h"
#include "view/friendlist/friendlist.h"
#include "view/kyview.h"
#include "view/common/globalsizedata.h"
#include "ukuiwindowhelper.h"

TrayIconWid::TrayIconWid(QWidget *parent) : QWidget(parent)
{
    // 初始化组件
    setWidgetUi();

    // 设置组件样式
    setWidgetStyle();

    // 设置信号绑定
    setSignalConn();
}

TrayIconWid::~TrayIconWid() {}

void TrayIconWid::hideTrayIcon()
{
    if (m_trayIcon) {
        m_trayIcon->hide();
    }
}

// 生成静态实例
TrayIconWid *TrayIconWid::getInstance()
{
    static TrayIconWid *instance = nullptr;

    if (instance == nullptr) {
#ifndef TEST_MODE
        try {
            instance = new TrayIconWid();
            instance->setAccessibleName("kylin-ipmsg_TrayIconWid_instance");
        } catch (const std::runtime_error &re) {
            qDebug() << "runtime_error:" << re.what();
        }
#else
        instance = new TrayIconWid();
        instance->setAccessibleName("kylin-ipmsg_TrayIconWid_instance");
#endif
    }

    return instance;
}

// 初始化组件
void TrayIconWid::setWidgetUi()
{
    // 初始化界面组件
    m_mainWid = new QWidget(this);
    m_friendView = new QListView(this);
    m_funcAreaWid = new QWidget(this);
    m_splitLine = new QFrame(this);
    m_ignoreBtn = new QPushButton(this);
    m_trayIcon = new QSystemTrayIcon(this);

    // 初始化界面布局
    m_mainLayout = new QVBoxLayout(this);
    m_funcAreaLayout = new QHBoxLayout(m_funcAreaWid);

    // 初始化图标闪烁
    m_timeFlag = 0;
    m_alertTimer = new QTimer(this);
    m_themeIcon = QIcon::fromTheme("kylin-ipmsg");
    m_blankIcon = QIcon(":/res/blank.png");

    //设置objectname
    m_mainWid->setAccessibleName("kylin-ipmsg_trayiconwid_QWidget_mmainWid");
    m_friendView->setAccessibleName("kylin-ipmsg_trayiconwid_QListView_mfriendView");
    m_funcAreaWid->setAccessibleName("kylin-ipmsg_trayiconwid_QWidget_mfuncAreaWid");
    m_splitLine->setAccessibleName("kylin-ipmsg_trayiconwid_QFrame_msplitLine");
    m_ignoreBtn->setAccessibleName("kylin-ipmsg_trayiconwid_QPushButton_mignoreBtn");
    m_trayIcon->setObjectName("kylin-ipmsg_trayiconwid_QSystemTrayIcon_mtrayIcon");
    m_mainLayout->setObjectName("kylin-ipmsg_trayiconwid_QVBoxLayout_mmainLayout");
    m_funcAreaLayout->setObjectName("kylin-ipmsg_trayiconwid_QHBoxLayout_muncAreaLayout");
    m_alertTimer->setObjectName("kylin-ipmsg_trayiconwid_QTimer_malertTimer");

    // 消息数
    m_msgCounter = 0;

    // 数据过滤模型
    m_trayFilterModel = new TrayFilterModel();
    m_trayFilterModel->setObjectName("kylin-ipmsg_trayiconwid_TrayFilterModel_mtrayFilterModel");
    m_trayFilterModel->setSourceModel(FriendListModel::getInstance());
    m_trayFilterModel->setDynamicSortFilter(true);
    m_trayFilterModel->sort(0);
    m_trayFilterModel->setFilterKeyColumn(0);

    // 视图代理
    m_trayItemDelegate = new TrayItemDelegate();
    m_trayItemDelegate->setObjectName("kylin-ipmsg_trayiconwid_TrayItemDelegate_mtrayItemDelegate");

    // 聊天消息列表
    m_friendView->setModel(m_trayFilterModel);
    m_friendView->setItemDelegate(m_trayItemDelegate);
    m_friendView->setSpacing(0);
    m_friendView->setDragEnabled(false);
    m_friendView->setFrameShape(QListView::NoFrame);
    m_friendView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_friendView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_friendView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    // 功能区域布局
    m_funcAreaLayout->setMargin(0);
    m_funcAreaLayout->setSpacing(0);
    m_funcAreaLayout->addStretch();
    m_funcAreaLayout->addWidget(m_ignoreBtn);
    m_funcAreaLayout->addSpacing(16);

    // 界面整体布局
    m_mainLayout->addWidget(m_friendView);
    m_mainLayout->addWidget(m_splitLine, 0, Qt::AlignHCenter);
    m_mainLayout->addWidget(m_funcAreaWid);
    m_mainLayout->setMargin(0);
    m_mainLayout->setSpacing(0);

    this->setFocusPolicy(Qt::NoFocus);

    m_menu = new QMenu(this);
    m_settingAction = new QAction(QObject::tr("Settings"), this);
    m_closeAction = new QAction(QObject::tr("Quit"), this);
    m_menu->addAction(m_settingAction);
    m_menu->addAction(m_closeAction);
    m_menu->setAccessibleName("kylin-ipmsg_trayiconwid_QMenu_mMenu");
    m_settingAction->setObjectName("kylin-ipmsg_trayiconwid_QAction_msettingAction");
    m_closeAction->setObjectName("kylin-ipmsg_trayiconwid_QAction_mcloseAction");
    // 添加分割线
    m_menu->insertSeparator(m_closeAction);

    m_trayIcon->setContextMenu(m_menu);
}

// 设置组件样式
void TrayIconWid::setWidgetStyle()
{
    // this->setWindowFlag(Qt::Tool);
    this->setWindowTitle(tr("Messages"));

    // 毛玻璃
    this->setProperty("useSystemStyleBlur", true);
    this->setAttribute(Qt::WA_TranslucentBackground, true);

    /* 适配kysdk的窗管 */
    UkuiWindowHelper *windowhelper = new UkuiWindowHelper(this);
    windowhelper->removeTitleBar();

    // 固定界面宽度
    this->setFixedWidth(280);

    this->setAutoFillBackground(true);
    this->setBackgroundRole(QPalette::Base);

    // 设置组件透明
    m_friendView->resize(280, 232);
    m_friendView->verticalScrollBar()->setProperty("drawScrollBarGroove", false);
    m_friendView->setStyleSheet("QListView{background:transparent;}");
    m_friendView->installEventFilter(this);

    // 分割线
    m_splitLine->setFrameShape(QFrame::HLine);
    m_splitLine->setFrameShadow(QFrame::Sunken);
    m_splitLine->setFixedSize(248, 1);
    m_splitLine->setWindowOpacity(0.08);

    m_funcAreaWid->setAttribute(Qt::WA_TranslucentBackground, true);
    m_funcAreaWid->setFixedHeight(47);

    // 忽略消息
    m_ignoreBtn->setFixedSize(70, 25);
    m_ignoreBtn->setAttribute(Qt::WA_TranslucentBackground, true);
    m_ignoreBtn->setText(tr("Ignore"));
    m_ignoreBtn->setFocusPolicy(Qt::NoFocus);
    m_ignoreBtn->setProperty("isWindowButton", 0x1);
    m_ignoreBtn->setProperty("useIconHighlightEffect", 0x2);
    m_ignoreBtn->setStyleSheet(
        "QPushButton{border:0px;border-radius:4px;background:transparent;color:#3790FA;}"
        "QPushButton:hover{border:0px;border-radius:4px;background:transparent;color:#40A9FB;}"
        "QPushButton:pressed{border:0px;border-radius:4px;background:transparent;color:#296CD9;}");

    // 托盘图标
    m_trayIcon->setIcon(QIcon::fromTheme("kylin-ipmsg"));
    m_trayIcon->setToolTip(QString(tr("Messages")));
    m_trayIcon->setVisible(true);
    m_trayIcon->installEventFilter(this);
    m_trayIcon->show();

    // 图标闪烁
    m_alertTimer->setInterval(500);

    this->move(1600, 850);

    this->changeTheme();
    this->update();
    this->hide();
}

// 设置信号绑定
void TrayIconWid::setSignalConn()
{
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &TrayIconWid::iconActivated);

    connect(m_friendView, &QListView::doubleClicked, this, &TrayIconWid::slotStartChat);

    connect(m_alertTimer, &QTimer::timeout, this, &TrayIconWid::slotMsgAlert);

    connect(this, &TrayIconWid::newUnreadMsg, this, &TrayIconWid::slotStartAlert);

    connect(m_ignoreBtn, &QPushButton::clicked, this, &TrayIconWid::slotIgnoreAll);

    connect(m_settingAction, &QAction::triggered, this, &TrayIconWid::slotSetting);
    connect(m_closeAction, &QAction::triggered, this, &TrayIconWid::slotCloseWin);
}

// 跟随主题深浅模式
void TrayIconWid::changeTheme()
{
    if (GlobalSizeData::THEME_COLOR == GlobalSizeData::UKUILight) {
        m_splitLine->setStyleSheet("background-color:rgba(0, 0, 0, 10%);");
    } else {
        m_splitLine->setStyleSheet("background-color:rgba(255, 255, 255, 10%);");
    }
    m_friendView->update();
    m_funcAreaWid->update();
    m_ignoreBtn->update();
    this->update();
}

// 窗口关闭事件
void TrayIconWid::closeEvent(QCloseEvent *event)
{
    this->hide();
    event->ignore();
}

// 键盘响应事件
void TrayIconWid::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_F1) {
        // 帮助手册
        kdk::UserManual userManual;
        if (!userManual.callUserManual("messages")) {
            qCritical() << "user manual call fail!";
        }
    }
}

// 开始图标闪烁
void TrayIconWid::slotStartAlert()
{
    /* sdk功能点:托盘 */
    GlobalSizeData::SDKPointTray();

    if (this->isHidden() && !(m_alertTimer->isActive())) {
        m_alertTimer->start();
    }
}

// 图标闪烁
void TrayIconWid::slotMsgAlert()
{
    /* sdk功能点:托盘 */
    GlobalSizeData::SDKPointTray();

    if (this->isHidden() && m_trayFilterModel->rowCount() > 0) {
        if (m_timeFlag % 2 == 0) {
            m_trayIcon->setIcon(m_blankIcon);
        } else {
            m_trayIcon->setIcon(m_themeIcon);
        }

        m_timeFlag++;
    } else {
        m_trayIcon->setIcon(m_themeIcon);
        m_alertTimer->stop();
    }
}

// 事件过滤器
bool TrayIconWid::eventFilter(QObject *watch, QEvent *event)
{
    // if (watch == m_friendView) {
    //     qDebug() << "watch m_friendView" << event->type();
    // } else if (watch == m_ignoreBtn) {
    //     qDebug() << "watch m_ignoreBtn" << event->type();
    // } else {
    //     qDebug() << "watch this" << event->type();
    // }

    if (event->type() == QEvent::Show) {

        QPoint trayIconPoint = m_trayIcon->geometry().topRight();
        this->move(int(trayIconPoint.x() - this->width() / 2), trayIconPoint.y() - this->height());

        m_friendView->setFocus();

    } else if (event->type() == QEvent::FocusOut) {
        this->hide();
    }

    if (watch == m_trayIcon) {
        if (event->type() == QEvent::ToolTip) {
            if (m_trayFilterModel->rowCount() > 0) {
                // TrayIconWid::getInstance()->update();
                TrayIconWid::getInstance()->show();
                m_trayIcon->setIcon(m_themeIcon);
            }

        } else {
            this->hide();
        }
    }

    return QWidget::eventFilter(watch, event);
    // return false;
}

// 绘制事件
void TrayIconWid::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    int msgCount = m_trayFilterModel->rowCount();
    bool isMove = false;
    if (msgCount != m_msgCounter) {

        if (msgCount < 4) {
            m_friendView->resize(280, 58 * msgCount);
            this->resize(280, m_friendView->height() + 48);
            isMove = true;
        } else if (m_msgCounter < 4) {
            m_friendView->resize(280, 232);
            this->resize(280, 280);
            isMove = true;
        }

        if (isMove) {
            QPoint trayIconPoint = m_trayIcon->geometry().topRight();
            this->move(int(trayIconPoint.x() - this->width() / 2), trayIconPoint.y() - this->height() - 24);
        }

        m_msgCounter = msgCount;
    }

    // QPalette pe;
    // pe.setColor(QPalette::WindowText, QColor("#3790FA"));
    // m_ignoreBtn->setPalette(pe);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing); // 反锯齿;

    QPainterPath rectPath;
    rectPath.addRoundedRect(this->rect(), 0, 0); // 左上右下
    // double tran=m_pGsettingControlTrans->get("transparency").toDouble()*255;
    QStyleOption opt;
    opt.init(this);

    QColor mainColor;

    if (QColor(255, 255, 255) == opt.palette.color(QPalette::Base)
        || QColor(248, 248, 248) == opt.palette.color(QPalette::Base)) {
        mainColor = QColor(245, 245, 245, GlobalSizeData::BLUR_TRANSPARENCY);
        // mainColor = QColor(245, 245, 245, tran);
    } else {
        mainColor = QColor(26, 26, 26, GlobalSizeData::BLUR_TRANSPARENCY);
        // mainColor = QColor(26, 26, 26, tran);
    }

    p.fillPath(rectPath, QBrush(mainColor));

    // m_friendView->update();
    // m_funcAreaWid->update();
    // m_ignoreBtn->update();
}

// 托盘图标点击
void TrayIconWid::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    /* sdk功能点:托盘 */
    GlobalSizeData::SDKPointTray();

//    QString mainWindowId = KyView::getInstance()->getWindowId();
//    if (mainWindowId.isEmpty()) {
//        qWarning() << "tray active window : window id is 0";
//        return;
//    }

    switch (reason) {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::MiddleClick:
        if (KyView::getInstance()->isHidden() || KyView::getInstance()->isMinimized() || !KyView::getInstance()->isActiveWindow()) {
            qDebug() << "tray icon test: into show";
            KyView::getInstance()->hide();
            // KWindowSystem::forceActiveWindow(KyView::getInstance()->winId());
            KyView::getInstance()->show();
            KyView::getInstance()->activateWindow();
            KyView::getInstance()->raise();

        } else {
            KyView::getInstance()->showMinimized();
        }
        break;

    case QSystemTrayIcon::DoubleClick:
        KyView::getInstance()->showMinimized();
        break;

    case QSystemTrayIcon::Context:
        //右键点击托盘图标弹出菜单
        // //showTrayIconMenu(); //显示右键菜单
        // m_mainMenu->show();
        m_menu->show();
        break;
    case QSystemTrayIcon::Unknown:
        qDebug() << "unknow";
        break;
    default:
        break;
    }
}

// 防止event冲突
//#include <X11/Xlib.h>

// 发起聊天
void TrayIconWid::slotStartChat()
{
    /* sdk功能点:托盘 */
    GlobalSizeData::SDKPointTray();

    // 将代理index转换为好友index
    QModelIndex friendIndex = m_trayFilterModel->mapToSource(m_friendView->currentIndex());
    int friendId = friendIndex.data(FriendListModel::Id).toInt();

    // 清空好友未读消息数
    FriendListModel::getInstance()->clearUnreadMsgNum(friendId);

    ChatMsg *chatMsg = nullptr;
    chatMsg = FriendListView::getInstance()->getMsgWidById(friendId);

    KWindowSystem::forceActiveWindow(chatMsg->winId());

    this->hide();
    chatMsg->show();

    /* 任务栏未提供鼠标悬浮事件，等提供相应事件后在做处理 */

#if 0
    // 使用X11的点击事件
    XEvent xEvent;
    memset(&xEvent, 0, sizeof(XEvent));
    Display *display = QX11Info::display();
    xEvent.type = ButtonPress;
    xEvent.xbutton.button = Button1;
    xEvent.xbutton.window = chatMsg->effectiveWinId();
    xEvent.xbutton.x = 100;
    xEvent.xbutton.y = 10;
    xEvent.xbutton.x_root = 100;
    xEvent.xbutton.y_root = 10;
    xEvent.xbutton.display = display;

    XSendEvent(display, chatMsg->effectiveWinId(), False, ButtonPressMask, &xEvent);

    xEvent.type = ButtonRelease;
    XSendEvent(display, chatMsg->effectiveWinId(), False, ButtonReleaseMask, &xEvent);

    XFlush(display);
#endif

    chatMsg->getFocus();
}

// 忽略全部消息
void TrayIconWid::slotIgnoreAll()
{
    /* sdk功能点:托盘 */
    GlobalSizeData::SDKPointTray();

    QList<int> friendIdList;

    // 获取所有的好友id
    for (int i = 0; i < m_trayFilterModel->rowCount(); i++) {
        QModelIndex friendIndex = m_trayFilterModel->index(i, 0);
        int friendId = friendIndex.data(FriendListModel::Id).toInt();

        friendIdList.append(friendId);
    }

    // 清空好友未读消息数
    for (int i = 0; i < friendIdList.size(); i++) {
        FriendListModel::getInstance()->clearUnreadMsgNum(friendIdList[i]);
    }

    this->hide();
}

void TrayIconWid::slotCloseWin()
{
    KyView::getInstance()->deleteLater();
    return;
}

void TrayIconWid::slotSetting()
{
    TitleSeting *titleSetUp = new TitleSeting(this);
    titleSetUp->show();
    UkuiWindowHelper *windowhelper = new UkuiWindowHelper(titleSetUp->windowHandle());
    windowhelper->setSkipTaskBar(true);
    windowhelper->setSkipSwitcher(true);
    titleSetUp->update();
    return;
}
