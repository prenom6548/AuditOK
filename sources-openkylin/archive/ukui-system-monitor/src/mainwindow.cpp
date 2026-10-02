/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "mainwindow.h"
#include "macro.h"

#include <unistd.h>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QtDBus/QtDBus>
#include <QApplication>
#include <QTimer>
#include <QScreen>
#include <QDebug>
#include <QSessionManager>

#include <kysdk/applications/accessinfohelper.h>
using namespace kdk;

#include "dialog/settingdialog.h"
#include "dialog/onclosedialog.h"
#include "prescene.h"

const int        kPreSceneTime   = 10;

MainWindow::MainWindow(QWidget *parent)
    : KWidget(parent)
{
    this->setIcon(QIcon::fromTheme("ukui-system-monitor"));
    this->setWidgetName(tr("System Monitor"));
    this->setObjectName("SystemMonitor");
    this->setAttribute(Qt::WA_TranslucentBackground);//背景透明
    this->setAttribute(Qt::WA_AlwaysShowToolTips);
    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    m_pUkuiWindowHelper = new UkuiWindowHelper(this);
    const QByteArray ifid(MENU_SCHEMA);
    if(QGSettings::isSchemaInstalled(ifid)) {
        m_pSystemMonitorGSetting = new QGSettings(ifid);
    }
    this->installEventFilter(this);

    m_lastWndSize.setWidth(MAINWINDOWWIDTH);
    m_lastWndSize.setHeight(MAINWINDOWHEIGHT_DEFAULT);
    m_bIsWndMax = false;

    m_proSettings = new QSettings(UKUI_COMPANY_SETTING, UKUI_SETTING_FILE_NAME_SETTING, this);
    m_proSettings->setIniCodec("UTF-8");
    loadSettings();
    this->resize(m_lastWndSize.width(),m_lastWndSize.height());
    if (m_bIsWndMax) {
        QTimer::singleShot(0, this, SLOT(onMaximizeWindow()));
    }
    setMinimumSize(MAINWINDOWWIDTH, MAINWINDOWHEIGHT);  //set the minimum size of the mainwindow

    QSize m_windowSize = this->size();
    PreScene *m_prescene = new PreScene(m_windowSize);

    QHBoxLayout *m_rightLayout = new QHBoxLayout;
    m_rightLayout->addWidget(m_prescene, 1);
    baseBar()->setLayout(m_rightLayout);

    // 先显示主窗口，否则子控件无法显示
    this->moveCenter();
    this->show();
    m_prescene->raise();

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [=]() {
        initUI();
        initConnections();
        // 隐藏预览界面
        if (m_prescene) {
            m_prescene->hide();
            qDebug() << Q_FUNC_INFO << "Prescene hidden";
        }
        this->setLayoutType(HorizontalType);
        timer->stop();
    });
    timer->start(kPreSceneTime);
}

MainWindow::~MainWindow()
{
    qDebug() << "exit process " << __FUNCTION__ << __LINE__;

    if (m_searchTimer) {
        disconnect(m_searchTimer, SIGNAL(timeout()), this, SLOT(onRefreshSearchResult()));
        if(m_searchTimer->isActive()) {
            m_searchTimer->stop();
        }
    }
    if (m_proSettings) {
        delete m_proSettings;
        m_proSettings = nullptr;
    }
    if (m_pSystemMonitorGSetting) {
        delete m_pSystemMonitorGSetting;
        m_pSystemMonitorGSetting = nullptr;
    }
    if (m_trayIcon) {
        delete m_trayIcon;
        m_trayIcon = nullptr;
    }
}

void MainWindow::initUI()
{
    if (m_pSystemMonitorGSetting && m_pSystemMonitorGSetting->keys().contains(SUPPORT_SERVICE_KEY)) {
        QVariant surport = m_pSystemMonitorGSetting->get(SUPPORT_SERVICE);
        if (surport.isValid()) {
            m_isSurportService = (bool)(surport.toInt());
        }
    }

    this->windowButtonBar()->menuButton()->themeAction()->setVisible(false);

    m_searchEditNew = new KSearchLineEdit();
    KDK_EXTEND_ALL_INFO_FORMAT(m_searchEditNew, "mainwindow", "", "search Edit");
    m_searchEditNew->setClearButtonEnabled(true);
    m_searchEditNew->setFixedWidth(SPECIALWIDTH);
    m_searchEditNew->installEventFilter(this);
    m_searchTimer = new QTimer(this);
    m_searchTimer->setSingleShot(true);

    QHBoxLayout *titleLayout = static_cast<QHBoxLayout*>(this->windowButtonBar()->layout());
    titleLayout->insertStretch(0);
    titleLayout->insertWidget(1, m_searchEditNew, Qt::AlignHCenter);
    titleLayout->insertStretch(2);

    int width = this->windowButtonBar()->menuButton()->width() + this->windowButtonBar()->minimumButton()->width()
            + this->windowButtonBar()->maximumButton()->width() + this->windowButtonBar()->closeButton()->width()
            + this->windowButtonBar()->layout()->spacing() * 3;
    QMargins margin = this->windowButtonBar()->layout()->contentsMargins();

    // 维吾尔 哈萨克斯 柯尔克孜 语言控件翻转
    QString language = QLocale::system().name();
    if (language == "ug_CN" || language == "kk_KZ" || language == "ky_KG") {
        titleLayout->setContentsMargins(margin.left(), margin.top(), width + margin.right(), margin.bottom());
    } else {
        titleLayout->setContentsMargins(width + margin.left(), margin.top(), margin.right(), margin.bottom());
    }

    // 初始控件
    // 初始化左侧列表
    m_leftWidget = new KLeftWidget(this);
    QHBoxLayout *m_leftLayout = new QHBoxLayout();
    m_leftLayout->setContentsMargins(0,0,0,0);
    m_leftLayout->setSpacing(0);
    m_leftLayout->addWidget(m_leftWidget, 0, Qt::AlignLeft);
    sideBar()->setLayout(m_leftLayout);
    setSidebarFollowMode(false);
    iconBar()->setFixedWidth(LEFT_WIDGET_WIDTH);
    sideBar()->setFixedWidth(LEFT_WIDGET_WIDTH);

    initRightPanel();

    int nDefautPage = 0;
    QTimer::singleShot(0, this, [&,this,nDefautPage](){
        this->switchPage(nDefautPage);
    });
    this->moveCenter();
}

void MainWindow::initConnections()
{
    connect(this->windowButtonBar()->menuButton()->aboutAction(), &QAction::triggered, this, [=]{
        KAboutDialog *k_aboutDialog = new KAboutDialog(this);
        k_aboutDialog->setAppIcon(QIcon::fromTheme("ukui-system-monitor"));
        k_aboutDialog->setWindowTitle(tr("System Monitor"));
        k_aboutDialog->setAppName(tr("System Monitor"));
        k_aboutDialog->setAppVersion(getUsmVersion());
        k_aboutDialog->setAppSupport(k_aboutDialog->appSupport());
        k_aboutDialog->exec();
    });

    connect(this->windowButtonBar()->menuButton()->assistAction(), &QAction::triggered, this, &MainWindow::showUserGuide);

    connect(this->windowButtonBar()->menuButton()->settingAction(), &QAction::triggered, this, [=](){
        // setting dialog
        SettingDialog *settingDialog = new SettingDialog(this);

        settingDialog->exec();
    });

    connect(this->windowButtonBar()->menuButton()->quitAction(),&QAction::triggered,this,[=]{
        saveSettings();
        qApp->exit();
    });

    // connect(this->windowButtonBar()->closeButton(), &QPushButton::clicked, this, &MainWindow::onCloseBtnClicked);
    connect(this->windowButtonBar()->minimumButton(), &QPushButton::clicked, this, &MainWindow::onMinimizeWindow);
    connect(this->windowButtonBar()->maximumButton(), &QPushButton::clicked, this, &MainWindow::onMaximizeWindow);

    connect(m_searchTimer, SIGNAL(timeout()), this, SLOT(onRefreshSearchResult()));

    connect(m_searchEditNew,&QLineEdit::textChanged,this, &MainWindow::handleSearchTextChanged, Qt::QueuedConnection);

    connect(m_leftWidget, SIGNAL(switchPage(int)), this, SLOT(onSwitchPage(int)));

    if (m_procWidget) {
        connect(this, &MainWindow::searchSignal, m_procWidget, &ProcessWidget::searchSignal);
    }
    if (m_svcWidget) {
        connect(this, &MainWindow::searchSignal, m_svcWidget, &ServiceWidget::searchSignal);
    }
    if (m_fsWidget) {
        connect(this, &MainWindow::searchSignal, m_fsWidget, &FileSystemWidget::onSearch);
    }

    connect(WindowManager::self(),&WindowManager::windowAdded,this,[=](const WindowId& windowId){
        if (getpid() == WindowManager::getPid(windowId)) {
            m_listWinIds.append(windowId);
        }
    });
    connect(WindowManager::self(),&WindowManager::windowRemoved,this,[=](const WindowId& windowId){
        if (m_listWinIds.contains(windowId)) {
            m_listWinIds.removeOne(windowId);
        }
    });

#ifdef ENBALE_TRAY
    if(!QSystemTrayIcon::isSystemTrayAvailable()) {
        qDebug() << "isSystemTrayAvailable = " << QSystemTrayIcon::isSystemTrayAvailable();
    }

    // 设置父对象，但未添加到布局时，在哈维科语言下会导致标题栏无法选择
    m_trayIcon = new TrayIcon(m_leftWidget->getCpuHisWidget(), m_leftWidget->getMemHisWidget(), m_leftWidget->getNetHisWidget(), m_fsWidget);
    connect(m_trayIcon, &TrayIcon::openMonitor, this, &MainWindow::handleMessage);
    connect(m_trayIcon, &TrayIcon::needSaveSetting, this, &MainWindow::saveSettings);
#endif

    QObject::connect(qApp, &QApplication::commitDataRequest, [=](QSessionManager &sessionManager){
        isShutDown = true;
        qDebug() << "logout,shutdown,reboot";
    });

    QObject::connect(qApp, &QApplication::saveStateRequest, [=](QSessionManager &sessionManager){
        isShutDown = false;
        qDebug() << "cancel logout,shutdown,reboot";
    });
}

void MainWindow::onCloseBtnClicked()
{
    window()->close();
}

void MainWindow::onMinimizeWindow()
{
    this->showMinimized();
}

void MainWindow::onMaximizeWindow()
{
    if (this->isMaximized()) {
        this->showNormal();
        this->windowButtonBar()->setMaximumButtonState(MaximumButtonState::Restore);
    } else {
        this->showMaximized();
        this->windowButtonBar()->setMaximumButtonState(MaximumButtonState::Maximum);
    }
}

void MainWindow::showUserGuide()
{
    QString service_name = "com.kylinUserGuide.hotel_" + QString::number(getuid());
    QDBusInterface interface(service_name,
                             KYLIN_USER_GUIDE_PATH,
                             KYLIN_USER_GUIDE_INTERFACE,
                             QDBusConnection::sessionBus(),
                             this);
    QDBusMessage msg = interface.call("showGuide", "ukui-system-monitor");
}

void MainWindow::handleSearchTextChanged()
{
    m_searchTextCache = m_searchEditNew->text();
    KRightWidget::searchText = m_searchEditNew->text();
    if (m_searchTextCache.isEmpty()) {
        m_isSearching = false;
    } else {
        m_isSearching = true;
    }
    if (m_searchTimer->isActive()) {
        m_searchTimer->stop();
    }
    m_searchTimer->start(300);
}

void MainWindow::onRefreshSearchResult()
{
    if (m_searchEditNew->text() == m_searchTextCache) {
        //qInfo()<<"Search Keyword:"<<m_searchTextCache;
        emit this->searchSignal(m_searchTextCache);
    }
}

void MainWindow::initRightPanel()
{
    //初始化右侧窗口
    unsigned panelIndex = 0;
    m_rightWidget = new KRightWidget(this);

    m_procWidget = new ProcessWidget(m_proSettings);
    m_rightWidget->addPanel(m_procWidget, tr("Processes"), QString(":/img/process_list.svg"), panelIndex++);

    if (m_isSurportService) {
        m_moudleTypes.service = panelIndex;
        m_svcWidget = new ServiceWidget(m_proSettings);
        m_rightWidget->addPanel(m_svcWidget, tr("Services"), QString(":/img/service_list.svg"), panelIndex++);
        // 提前加载服务界面数据
        if (m_svcWidget) {
            Q_EMIT m_svcWidget->switchOnWidget();
        }
        // 连接服务管理器的服务列表更新完成信号
        ServiceManager* serviceManager = ServiceManager::instance();
        connect(serviceManager, &ServiceManager::serviceListUpdated, this, [this]() {
            m_isServiceListLoaded = true;
            if (m_svcWidget) {
                m_svcWidget->showLoadingIndicator(false);
            }
        });
    }

    m_fsWidget = new FileSystemWidget();
    m_moudleTypes.disk = panelIndex;
    m_rightWidget->addPanel(m_fsWidget, tr("Disks"), QString(":/img/filesystem_list.svg"), panelIndex++);

    m_moudleTypes.cpuDetails = panelIndex;
    m_moudleTypes.memDetails = ++panelIndex;
    m_moudleTypes.netDetails = ++panelIndex;
    m_moudleTypes.diskDetails = ++panelIndex;

    baseBar()->layout()->addWidget(m_rightWidget);
}

void MainWindow::handleMessage(const QString &msg)
{
    qDebug() << Q_FUNC_INFO << msg;
    if (this->isActiveWindow() && msg.compare("trayicon") == 0) {
        this->showMinimized();
    } else {
        QString platform = QGuiApplication::platformName();
        qDebug() << Q_FUNC_INFO << platform;
        if(platform.startsWith(QLatin1String("wayland"),Qt::CaseInsensitive)) {
            if (!m_listWinIds.isEmpty()) {
                WindowManager::activateWindow(m_listWinIds.back());
            }
        } else {
            KWindowSystem::forceActiveWindow(this->winId());
        }
        this->windowShow();
        this->activateWindow();
    }
}

void MainWindow::onSwitchPage(int nIndex)
{
    qDebug()<<"SwitchPage:"<<nIndex;

    if (mCpuDetailsWidget == nullptr) {
        mCpuDetailsWidget = new CpuDetailsWidget();
        m_rightWidget->addPanel(mCpuDetailsWidget, tr("Cpu Details"), "", m_moudleTypes.cpuDetails);
    }
    if (mMemDetailsWidget == nullptr) {
        mMemDetailsWidget = new MemDetailsWidget();
        m_rightWidget->addPanel(mMemDetailsWidget, tr("Mem Details"), "", m_moudleTypes.memDetails);
    }
    if (mNetDetailsWidget == nullptr) {
        mNetDetailsWidget = new NetDetailsWidget();
        m_rightWidget->addPanel(mNetDetailsWidget, tr("Net Details"), "", m_moudleTypes.netDetails);
    }
    if (m_pDiskDetailsWidget == nullptr) {
        m_pDiskDetailsWidget = new DiskDetailsWidget();
        m_rightWidget->addPanel(m_pDiskDetailsWidget, tr("Disk Details"), "", m_moudleTypes.diskDetails);
    }

    switchPage(nIndex);

    if (nIndex == m_moudleTypes.service && m_svcWidget) {
        // 如果是第一次切换到服务页面
        if (m_isFirstSwitchToService) {
            m_isFirstSwitchToService = false;
            if (!m_isServiceListLoaded) {
                // 如果refreshServiceList没有跑完，显示loading界面
                m_svcWidget->showLoadingIndicator(true);
            } else {
                // 如果refreshServiceList已经跑完，隐藏loading界面
                m_svcWidget->showLoadingIndicator(false);
            }
        }
    }

    // 第一次切换到磁盘界面，刷新数据
    if (nIndex == m_moudleTypes.disk && m_fsWidget) {
        m_fsWidget->onSwitchOnView();
    }

    // 第一次切换到CPU详情页，刷新数据，并建立刷新连接
    if (nIndex == m_moudleTypes.cpuDetails && mCpuDetailsWidget) {
        if (m_firstInCpuWidget) {
            m_firstInCpuWidget = false;
            mCpuDetailsWidget->onUpdateData();
            connect(mCpuDetailsWidget, &BaseDetailViewWidget::hideDetails, this, &MainWindow::onSwitchPage);
            connect(m_leftWidget, &KLeftWidget::updateCpuStatus, mCpuDetailsWidget, &CpuDetailsWidget::onUpdateCpuChartView);
            connect(m_leftWidget, &KLeftWidget::updateCpuStatus, mCpuDetailsWidget, &CpuDetailsWidget::onUpdateData);
        }
    }

    // 第一次切换到内存详情页，刷新数据，并建立刷新连接
    if (nIndex == m_moudleTypes.memDetails && mMemDetailsWidget) {
        if (m_firstInMemWidget) {
            m_firstInMemWidget = false;
            mMemDetailsWidget->onUpdateMemData();
            connect(mMemDetailsWidget, &BaseDetailViewWidget::hideDetails, this, &MainWindow::onSwitchPage);
            connect(m_leftWidget, &KLeftWidget::updateMemStatus, mMemDetailsWidget, &MemDetailsWidget::onUpdateMemChartView);
            connect(m_leftWidget, &KLeftWidget::updateSwapStatus, mMemDetailsWidget, &MemDetailsWidget::onUpdateSwapChartView);
            connect(m_leftWidget, &KLeftWidget::updateMemStatus, mMemDetailsWidget, &MemDetailsWidget::onUpdateMemData);
        }
    }

    // 第一次切换到网络详情页，刷新数据，并建立刷新连接
    if (nIndex == m_moudleTypes.netDetails && mNetDetailsWidget) {
        if (m_firstInNetWidget) {
            m_firstInNetWidget = false;
            mNetDetailsWidget->onUpdateData();
            connect(mNetDetailsWidget, &BaseDetailViewWidget::hideDetails, this, &MainWindow::onSwitchPage);
            connect(m_leftWidget, &KLeftWidget::onUpdateNetStatus, mNetDetailsWidget, &NetDetailsWidget::onUpdateNetChartView);
            connect(m_leftWidget, &KLeftWidget::onUpdateNetStatus, mNetDetailsWidget, &NetDetailsWidget::onUpdateData);
        }
    }

    // 第一次切换到磁盘详情页，刷新数据，并建立刷新连接
    if (nIndex == m_moudleTypes.diskDetails && m_pDiskDetailsWidget) {
        if (m_firstInDiskWidget) {
            m_firstInDiskWidget = false;
            m_pDiskDetailsWidget->onUpdateData();
            connect(m_pDiskDetailsWidget, &BaseDetailViewWidget::hideDetails, this, &MainWindow::onSwitchPage);
            connect(m_leftWidget, &KLeftWidget::onUpdateDiskStatus, m_pDiskDetailsWidget, &DiskDetailsWidget::onUpdateDiskChartView);
            connect(m_leftWidget, &KLeftWidget::onUpdateDiskStatus, m_pDiskDetailsWidget, &DiskDetailsWidget::onUpdateData);
        }
    }
}

void MainWindow::switchPage(int nIndex)
{
    if (nIndex < 0)
        return;
    if (m_leftWidget->currentIndex() != nIndex)
        m_leftWidget->onSwichPanel(nIndex);
    if (m_rightWidget->currentIndex() != nIndex) {
        m_searchEditNew->clear();
        m_searchEditNew->clearFocus();
        emit this->searchSignal("");
        m_rightWidget->onSwichPanel(nIndex);
    }
}

bool MainWindow::dblOnEdge(QMouseEvent *event)
{
    if (!event)
        return false;
    QPoint pos = event->globalPos();
    int globalMouseY = pos.y();

    int frameY = this->y();

    bool onTopEdges = (globalMouseY >= frameY &&
                       globalMouseY <= frameY + MAINWINDOW_DBCLICK_WIDTH);
    return onTopEdges;
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == this->windowButtonBar()->maximumButton()) {
        if (event->type() == QEvent::ToolTip) {
            event->setAccepted(true);
            event->accept();
            QRect rect = this->windowButtonBar()->maximumButton()->geometry();
            qDebug() << "maxBtn rect = " << rect;
            rect.translate(m_leftWidget->width(), 0);
            m_pUkuiWindowHelper->showTileFlyout(rect);
            return true;
        }
    }

    if (event->type() == QEvent::Close) {

        qDebug() << event->type();

        // get configure
        if (isShutDown) {
            qApp->exit();
        } else {
            if (m_pSystemMonitorGSetting && m_pSystemMonitorGSetting->keys().contains(SUPPORT_SERVICE_KEY)) {
                bool inquiries = m_pSystemMonitorGSetting->get(INQUIRIES).toBool();
                if (inquiries) {
                    event->ignore();

                    // show dialog
                    OnCloseDialog *closeSettingDialog = new OnCloseDialog(this);
                    connect(closeSettingDialog, &OnCloseDialog::closeMainWindow, this, [=](){
                        window()->close();
                        event->accept();
                    });
                    connect(closeSettingDialog, &OnCloseDialog::needSaveSetting, this, &MainWindow::saveSettings);

                    closeSettingDialog->exec();
                } else {
                    bool directlyExit = m_pSystemMonitorGSetting->get(DIRECTLY_EXIT).toBool();
                    if (directlyExit) {
                        saveSettings();
                        qApp->exit();
                    } else {
                        window()->close();
                    }
                }
            } else {
                qApp->exit();
            }
        }
    }

    if (this == watched) {

        if (event->type() == QEvent::MouseButtonDblClick) {
            QMouseEvent * mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton ){
                bool res = dblOnEdge(dynamic_cast<QMouseEvent*>(event));
                if (res) {
                    onMaximizeWindow();
                }
            }
        }

        if (event->type() == QEvent::WindowStateChange) {
            if (this->windowState() == Qt::WindowMaximized) {
                this->windowButtonBar()->setMaximumButtonState(MaximumButtonState::Restore);
            } else {
                this->windowButtonBar()->setMaximumButtonState(MaximumButtonState::Maximum);
            }
        }
    }
    return KWidget::eventFilter(watched, event);
}


void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_F1) {
        qDebug() << "onF1ButtonClicked";
        QString service_name = "com.kylinUserGuide.hotel_" + QString::number(getuid());
        QDBusInterface *interface = new QDBusInterface(service_name,
                                                       KYLIN_USER_GUIDE_PATH,
                                                       KYLIN_USER_GUIDE_INTERFACE,
                                                       QDBusConnection::sessionBus(),
                                                       this);
        QDBusMessage msg = interface->call("showGuide", "ukui-system-monitor");
    }
    if (mFirstKey == 0 && event->key() == Qt::Key_Control) {
        mFirstKey = Qt::Key_Control;
    } else if (mFirstKey == Qt::Key_Control && event->key() == Qt::Key_E) {
        m_searchEditNew->setFocus();
    } else {
        mFirstKey = 0;
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *e)
{
    mFirstKey = 0;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveSettings();
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    KWidget::mouseMoveEvent(event);
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    KWidget::mousePressEvent(event);
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    KWidget::mouseReleaseEvent(event);
}

bool MainWindow::loadSettings()
{
    bool bSuccess = false;
    if (m_proSettings) {
        m_proSettings->beginGroup("SystemMonitor");
        QVariant wndWidth = m_proSettings->value(MAINWINDOW_SIZE_W);
        QVariant wndHeight = m_proSettings->value(MAINWINDOW_SIZE_H);
        QVariant isWndMax = m_proSettings->value(MAINWINDOW_SIZE_MAX);
        m_proSettings->endGroup();
        if (wndWidth.isValid() && wndWidth.toInt() > 0 &&
            wndHeight.isValid() && wndHeight.toInt() > 0) {
            m_lastWndSize.setWidth(wndWidth.toInt());
            m_lastWndSize.setHeight(wndHeight.toInt());
        }
        if (isWndMax.isValid()) {
            m_bIsWndMax = isWndMax.toBool();
        }
        bSuccess = true;
    }
    return bSuccess;
}

void MainWindow::saveSettings()
{
    m_lastWndSize = size();
    m_bIsWndMax = isMaximized();
    if (m_procWidget) {
        m_procWidget->onWndClose();
    }
    if (m_svcWidget) {
        m_svcWidget->onWndClose();
    }

    if (m_proSettings) {
        m_proSettings->beginGroup("SystemMonitor");
        m_proSettings->setValue(MAINWINDOW_SIZE_W, m_lastWndSize.width());
        m_proSettings->setValue(MAINWINDOW_SIZE_H, m_lastWndSize.height());
        m_proSettings->setValue(MAINWINDOW_SIZE_MAX, m_bIsWndMax);
        m_proSettings->endGroup();
        m_proSettings->sync();
    }
}

void MainWindow::moveCenter()
{
    QPoint pos = QCursor::pos();
    QRect primaryGeometry;
    for (QScreen *screen : qApp->screens()) {
        if (screen->geometry().contains(pos)) {
            primaryGeometry = screen->geometry();
        }
    }

    if (primaryGeometry.isEmpty()) {
        primaryGeometry = qApp->primaryScreen()->geometry();
    }

    this->move(primaryGeometry.x() + (primaryGeometry.width() - this->width())/2,
               primaryGeometry.y() + (primaryGeometry.height() - this->height())/2);
}

void MainWindow::tabletmodeDbusSlot(bool isTabletmode)
{
    if (isTabletmode) {
        m_searchEditNew->setFixedWidth(280);
    } else {
        m_searchEditNew->setFixedWidth(SPECIALWIDTH);
    }
}

void MainWindow::windowShow()
{
    this->show();
}
