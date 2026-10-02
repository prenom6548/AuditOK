/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

#include "maindialog.h"
#include "ui_maindialog.h"

#include <QMenu>
#include <QProcess>
#include <QMessageBox>
#include <KWindowSystem>
#include <KWindowEffects>
#include <ukuistylehelper/ukuistylehelper.h>
#include <windowmanager/windowmanager.h>
#include <unistd.h>
#include "../common/mydefine.h"
#include "../common/dynamiccreator.h"
#include "backup_manager_interface.h"
#include "globalbackupinfo.h"
#include "messageboxutils.h"
#include "aboutdialog.h"
#include "gsettingswrapper.h"
#include "component/mywidget.h"

#include <libkydatacollect.h>

MainDialog::MainDialog(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainDialog)
{
    ui->setupUi(this);
    // 主窗口
    this->setWindowTitle(tr("Backup & Restore"));
    // 窗口背景透明化；setWindowOpacity可使得窗口及其上控件都透明或半透明-w.setWindowOpacity(0.7);
    this->setAttribute(Qt::WA_TranslucentBackground);
    // 【窗口】焦点不在窗口上时不显示tooltips
    this->setAttribute(Qt::WA_AlwaysShowToolTips);
    // 使得窗口无边框
    // w.setWindowFlag(Qt::FramelessWindowHint);
    // 指示窗口管理器模糊给定窗口后面指定区域的背景(毛玻璃化背景)
    // KWindowEffects::enableBlurBehind(this->winId(), true);
    // 或使用设置毛玻璃属性的方式实现毛玻璃背景（两者都行，可使用其一）
    this->setProperty("useSystemStyleBlur", true);
    this->setAutoFillBackground(true);

    // 去除窗管标题栏，传入参数为QWidget *
    kdk::UkuiStyleHelper::self()->removeHeader(this);

    GlobelBackupInfo::instance().setMainWidget(this);

    // 注册BackupWrapper类型，之后qdbus接口才能使用
    BackupWrapper::registerMetaType();
    //mountBackupPartition();

    initUI();
    initConnect();
}

MainDialog::~MainDialog()
{
    delete ui;
}

void MainDialog::initUI()
{
    // 总体布局为横向布局，分为左侧和右侧
    m_totalHLayout = new QHBoxLayout;
    m_totalHLayout->setSpacing(0);
    m_totalHLayout->setObjectName(QString::fromUtf8("m_totalHLayout"));
    m_totalHLayout->setContentsMargins(0, 0, 0, 0);
    m_totalHLayout->setSpacing(0);
    ui->centralwidget->setLayout(m_totalHLayout);

    // 左栏窗口
    if (GlobelBackupInfo::instance().isCasper()){  // 试安装模式中只能使用系统还原
        m_leftSiderBarWidget = new LeftsiderbarWidget(ui->centralwidget, LeftsiderbarWidget::StartMode::casper);
    }else if (GlobelBackupInfo::instance().isManager()){  // 管理员可以使用全部功能
        m_leftSiderBarWidget = new LeftsiderbarWidget(ui->centralwidget);
    }else{  // 标准用户可以使用数据备份、数据还原
        m_leftSiderBarWidget = new LeftsiderbarWidget(ui->centralwidget, LeftsiderbarWidget::StartMode::stdUser);
    }
    m_leftSiderBarWidget->setObjectName("yhkylin-backup-tools_kybackup_LeftsiderbarWidget_m_leftSiderBarWidget");
    m_leftSiderBarWidget->setFixedWidth(200);
    m_leftSiderBarWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    m_totalHLayout->addWidget(m_leftSiderBarWidget);

    // 右侧窗口
    m_rightWidget = new MyWidget;
    m_rightWidget->setObjectName("yhkylin-backup-tools_kybackup_MyWidget_m_rightWidget");
    m_totalHLayout->addWidget(m_rightWidget);

    m_rightVLayout = new QVBoxLayout;
    m_rightVLayout->setObjectName(QString::fromUtf8("m_rightVLayout"));
    m_rightVLayout->setContentsMargins(0, 0, 0, 0);
    m_rightVLayout->setSpacing(0);

    // 右侧窗口：标题栏
    m_titleWidget = new DoubleClickWidget;
    m_titleWidget->setObjectName("yhkylin-backup-tools_kybackup_DoubleClickWidget_m_titleWidget");
    m_titleWidget->setObjectName(QString::fromUtf8("m_titleWidget"));
    m_titleWidget->setFixedHeight(40);
    m_rightVLayout->addWidget(m_titleWidget);

    // 右侧窗口：工作区
    m_workWidget = new QWidget;
    m_workWidget->setObjectName("yhkylin-backup-tools_kybackup_QWidget_m_workWidget");
    m_workWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_rightVLayout->addWidget(m_workWidget);
    m_rightWidget->setLayout(m_rightVLayout);

    // 工作区随左边框功能切换界面
    m_workVLayout = new QVBoxLayout;
    if (GlobelBackupInfo::instance().isCasper())
        selected(FuncTypeConverter::FunType::RESTORE_SYSTEM);
    else if (GlobelBackupInfo::instance().isManager() && GlobelBackupInfo::instance().hasArgRestore())
        selected(FuncTypeConverter::FunType::RESTORE_SYSTEM);
    else if (!GlobelBackupInfo::instance().isManager()  && GlobelBackupInfo::instance().hasArgRestore())
        selected(FuncTypeConverter::FunType::RESTORE_DATA);
    else if (!GlobelBackupInfo::instance().isManager())
        selected(FuncTypeConverter::FunType::BACKUP_DATA);
    else
        selected(FuncTypeConverter::FunType::BACKUP_SYSTEM);
    m_workWidget->setLayout(m_workVLayout);

    initTileBar();
}

void MainDialog::initTileBar()
{
    m_titleLayout = new QHBoxLayout;
    m_titleLayout->setContentsMargins(8, 4, 4, 0);
    m_titleLayout->setSpacing(4);

    m_menuOptionBtn  = new QToolButton;
    m_menuOptionBtn->setObjectName("yhkylin-backup-tools_kybackup_m_menuOptionBtn_m_menuOptionBtn");
    m_minBtn      = new QPushButton;
    m_minBtn->setObjectName("yhkylin-backup-tools_kybackup_QPushButton_m_minBtn");
    m_maxBtn      = new QPushButton;
    m_maxBtn->setObjectName("yhkylin-backup-tools_kybackup_QPushButton_m_maxBtn");
    m_closeBtn    = new QPushButton;
    m_closeBtn->setObjectName("yhkylin-backup-tools_kybackup_QPushButton_m_closeBtn");

    m_menuOptionBtn->setToolTip(tr("Options"));
    m_minBtn->setToolTip(tr("Minimize"));
    m_maxBtn->setToolTip(tr("Maximize"));
    m_closeBtn->setToolTip(tr("Close"));

    m_menuOptionBtn->setProperty("useButtonPalette", true);
    m_menuOptionBtn->setProperty("useIconHighlightEffect", 0x2);
    m_menuOptionBtn->setIcon(QIcon::fromTheme("open-menu-symbolic"));
    m_menuOptionBtn->setFixedSize(PC_TITLE_BAR_SIZE, PC_TITLE_BAR_SIZE);
    m_menuOptionBtn->setAutoRaise(true);
    m_menuOptionBtn->setPopupMode(QToolButton::InstantPopup);

    m_minBtn->setProperty("useButtonPalette", true);
    m_minBtn->setProperty("useIconHighlightEffect", 0x2);
    m_minBtn->setFixedSize(PC_TITLE_BAR_SIZE, PC_TITLE_BAR_SIZE);
    m_minBtn->setFlat(true);
    m_minBtn->setIcon(QIcon::fromTheme("window-minimize-symbolic"));

    m_maxBtn->setProperty("useButtonPalette", true);
    m_maxBtn->setProperty("useIconHighlightEffect", 0x2);
    m_maxBtn->setFixedSize(PC_TITLE_BAR_SIZE, PC_TITLE_BAR_SIZE);
    m_maxBtn->setFlat(true);
    m_maxBtn->setIcon(QIcon::fromTheme("window-maximize-symbolic"));

    m_closeBtn->setProperty("isWindowButton", 0x2);
    m_closeBtn->setProperty("useIconHighlightEffect", 0x8);
    m_closeBtn->setFixedSize(PC_TITLE_BAR_SIZE, PC_TITLE_BAR_SIZE);
    m_closeBtn->setFlat(true);
    m_closeBtn->setIcon(QIcon::fromTheme("window-close-symbolic"));

    QMenu* backupMain = new QMenu(m_titleWidget);
    backupMain->setObjectName("yhkylin-backup-tools_kybackup_QMenu_backupMain");
    backupMain->setObjectName("mainMenu");
    m_menuOptionBtn->setMenu(backupMain);

    // 暂时只需跟随主题，浅色主题和深色主题后续可能会独立出来
//    QAction* backupTheme = new QAction(tr("Theme"), m_titleWidget);
//    backupMain->addAction(backupTheme);
//    QMenu* selectTheme = new QMenu( m_titleWidget);
//    selectTheme->setObjectName("selectTheme");
//    backupTheme->setMenu(selectTheme);
//    QAction* defaultTheme = new QAction(tr("Auto"), m_titleWidget);
//    selectTheme->addAction(defaultTheme);
//    QAction* lightTheme = new QAction(tr("Light"), m_titleWidget);
//    selectTheme->addAction(lightTheme);
//    QAction* darkTheme = new QAction(tr("Dark"), m_titleWidget);
//    selectTheme->addAction(darkTheme);

    m_backupHelp = new QAction(tr("Help"), m_titleWidget);
    m_backupHelp->setObjectName("yhkylin-backup-tools_kybackup_QAction_m_backupHelp");
    // 设计竟然让快捷键标记不能在页面中提示出来，要悄默默的快捷，奇葩。需要重定义键盘事件代替通用快捷键设置方法。
    // m_backupHelp->setShortcut(QKeySequence::HelpContents);
    backupMain->addAction(m_backupHelp);
    m_backupAbout = new QAction(tr("About"), m_titleWidget);
    m_backupAbout->setObjectName("yhkylin-backup-tools_kybackup_QAction_m_backupAbout");
    backupMain->addAction(m_backupAbout);

    kdk::KMenuButton* menubutton = new kdk::KMenuButton();
    menubutton->setObjectName("yhkylin-backup-tools_kybackup_KMenuButton_menubutton");
    menubutton->setText(tr("Feedback"));
    backupMain->addAction(menubutton->feedbackAction());

    m_backupExit = new QAction(tr("Quit"), m_titleWidget);
    m_backupExit->setObjectName("yhkylin-backup-tools_kybackup_QAction_m_backupExit");
    backupMain->addAction(m_backupExit);

    m_titleLayout->addStretch();
    m_titleLayout->addWidget(m_menuOptionBtn);
    m_titleLayout->addWidget(m_minBtn);
    m_titleLayout->addWidget(m_maxBtn);
    m_titleLayout->addWidget(m_closeBtn);
    m_titleWidget->setLayout(m_titleLayout);
}

void MainDialog::initConnect()
{
    // 左侧功能选择栏
    connect(m_leftSiderBarWidget, &LeftsiderbarWidget::selected, this, &MainDialog::selected);
    connect(GlobelBackupInfo::instance().getGlobalSignals(), &GlobalSignals::busy, m_leftSiderBarWidget, &LeftsiderbarWidget::setBusy);

    // 标题栏右侧按钮区域
    // 退出
    connect(m_backupExit, &QAction::triggered, this, &MainDialog::closeBtn);

    // 关于
    connect(m_backupAbout, &QAction::triggered, this, [=] {
        AboutDialog *about = new AboutDialog(this);
        about->setObjectName("yhkylin-backup-tools_kybackup_AboutDialog_about");
        QPointF position = this->pos();
        about->move(position.x()+this->width()/2-450/2,position.y()+this->height()/2-370/2);
        about->exec();
        about->deleteLater();
    });

    // 用户手册
    connect(m_backupHelp, &QAction::triggered, this, [=] {
        QProcess process(this);
        process.startDetached("kylin-user-guide -A yhkylin-backup-tools");
    });

    //最小化按钮
    connect(m_minBtn, &QPushButton::clicked, this, &MainDialog::showMinimized);

    //最大化按钮
    connect(m_maxBtn, &QPushButton::clicked, this, [=] {
        if (isMaximized()) {
            showNormal();
        } else {
            showMaximized();
        }
    });
    connect(m_titleWidget, &DoubleClickWidget::doubleClicked, m_maxBtn, &QPushButton::click);
    connect(m_leftSiderBarWidget, &LeftsiderbarWidget::doubleClicked, m_maxBtn, &QPushButton::click);

    //关闭按钮
    connect(m_closeBtn, &QPushButton::clicked, this, &MainDialog::closeBtn);

    //监听平板模式切换
    m_statusSessionDbus = new QDBusInterface("com.kylin.statusmanager.interface",
                                             "/",
                                             "com.kylin.statusmanager.interface",
                                             QDBusConnection::sessionBus(),
                                             this);
    if (m_statusSessionDbus->isValid()) {
        QDBusReply<bool> isTabletMode = m_statusSessionDbus->call("get_current_tabletmode");
        GlobelBackupInfo::instance().setIsTabletMode(isTabletMode);
        tabletModeChange(isTabletMode);
        connect(m_statusSessionDbus, SIGNAL(mode_change_signal(bool)), this, SLOT(tabletModeChange(bool)));
    } else {
        GlobelBackupInfo::instance().setIsTabletMode(false);
        m_maxBtn->setVisible(true);
    }
}

/**
 * @brief 平板模式切换
 * @param isTabletMode  true-平板模式；false-PC模式
 */
void MainDialog::tabletModeChange(bool isTabletMode)
{
    GlobelBackupInfo::instance().setIsTabletMode(isTabletMode);
    if(isTabletMode){
        m_menuOptionBtn->setFixedSize(TABLET_TITLE_BAR_SIZE, TABLET_TITLE_BAR_SIZE);
        m_minBtn->setFixedSize(TABLET_TITLE_BAR_SIZE, TABLET_TITLE_BAR_SIZE);
        m_maxBtn->setFixedSize(TABLET_TITLE_BAR_SIZE, TABLET_TITLE_BAR_SIZE);
        m_closeBtn->setFixedSize(TABLET_TITLE_BAR_SIZE, TABLET_TITLE_BAR_SIZE);
        m_titleWidget->setFixedHeight(56);
        m_maxBtn->setVisible(false);
    } else {
        m_menuOptionBtn->setFixedSize(PC_TITLE_BAR_SIZE, PC_TITLE_BAR_SIZE);
        m_minBtn->setFixedSize(PC_TITLE_BAR_SIZE, PC_TITLE_BAR_SIZE);
        m_maxBtn->setFixedSize(PC_TITLE_BAR_SIZE, PC_TITLE_BAR_SIZE);
        m_closeBtn->setFixedSize(PC_TITLE_BAR_SIZE, PC_TITLE_BAR_SIZE);
        m_titleWidget->setFixedHeight(40);
        m_maxBtn->setVisible(true);
    }
}

void MainDialog::changeEvent(QEvent *event)
{
    if (QEvent::WindowStateChange == event->type()) {
        Qt::WindowStates state = this->windowState();
        switch (state) {
        case Qt::WindowMaximized:
        case Qt::WindowFullScreen:
            m_maxBtn->setIcon(QIcon::fromTheme("window-restore-symbolic"));
            m_maxBtn->setToolTip(tr("Normal"));

            break;
        case Qt::WindowNoState:
            m_maxBtn->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
            m_maxBtn->setToolTip(tr("Maximize"));

            break;
        default:
            break;
        }
    }

    QMainWindow::changeEvent(event);
}

void MainDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_F1)
        emit m_backupHelp->triggered();

    QMainWindow::keyPressEvent(event);
}

/**
 * @brief 左侧功能选择栏响应槽
 * @param func_type，功能类型
 */
void MainDialog::selected(int funcType)
{
    if (GlobelBackupInfo::instance().getFuncType() == funcType)
        return ;

    if (m_stackedWidget) {
        m_workVLayout->removeWidget(m_stackedWidget);
        delete m_stackedWidget;
    }

    m_stackedWidget = createWidgetFactory(funcType, m_workWidget);
    if (!m_stackedWidget){
        m_stackedWidget = new QStackedWidget(m_workWidget);
        m_stackedWidget->setObjectName("yhkylin-backup-tools_kybackup_QStackedWidget_m_stackedWidget");
    }
    GlobelBackupInfo::instance().setFuncType((FuncTypeConverter::FunType)funcType);
    m_stackedWidget->setObjectName(QString::fromUtf8("m_stackedWidget"));
    // m_stackedWidget->setMinimumSize(760, 600);
    m_workVLayout->addWidget(m_stackedWidget);

    m_leftSiderBarWidget->setCheckedFunc(funcType);
}

/**
 * @brief 功能界面工厂，根据功能类型动态创建对应的功能界面
 * @param funcType，功能类型
 * @param parent，父窗口
 * @return 功能界面的QStackedWidget*指针
 * @note  调用方控制返回指针的释放
 */
QStackedWidget * MainDialog::createWidgetFactory(int funcType, QWidget *parent/* = nullptr*/)
{
    FuncTypeConverter converter;
    QString className = converter.funcTypeToWidgetClassName(funcType);
    return DynamicCreator<QStackedWidget*,QWidget*>::createInstance(className, parent);
}

/**
 * @brief 多例启动信息槽
 * @param msg 信息，里面存放新启动实例的账户id
 */
void MainDialog::sltMessageReceived(const QString &msg)
{
    QString user = QString::number(getuid());
    if (msg == user) {
//        this->setWindowFlag(Qt::WindowStaysOnTopHint,true);
        KWindowSystem::forceActiveWindow(this->winId());
        this->show();
    }
}

/**
 * @brief 挂载备份分区
 * @note 程序开启时挂载备份分区
 */
void MainDialog::mountBackupPartition()
{
    // 挂载/backup分区
    ComKylinBackupManagerInterface intf("com.kylin.backup", "/", QDBusConnection::systemBus());
    QDBusPendingReply<int> reply = intf.Mount_backup_partition();
    reply.waitForFinished();
    if (reply.isError()) {
        // 挂载分区时出现异常
        MessageBoxUtils::QMESSAGE_BOX_CRITICAL(this, QObject::tr("Warning"),
                                              QObject::tr("An exception occurred when mounting backup partition."),
                                              QObject::tr("OK"));

        // 此时还未进入应用的事件循环中，故不能使用qApp->quit();
        exit(1);
    } else if (int(MountResult::CANNOT_GET_BACKUPUUID) == reply.value()) {
        // 没有找到备份分区不再报错，改为只能备份到移动设备中
//        MessageBoxUtils::QMESSAGE_BOX_WARNING(this, QObject::tr("Information"),
//                                              QObject::tr("Please check if the backup partition exists which can be created when you install the Operating System."),
//                                              QObject::tr("OK"));
//        // 此时还未进入应用的事件循环中，故不能使用qApp->quit();
//        exit(1);
        GlobelBackupInfo::instance().setHasBackupPartition(false);
    } else if (int(MountResult::MOUNTED) != reply.value()) {
        // 挂载备份分区失败
        MessageBoxUtils::QMESSAGE_BOX_CRITICAL(this, QObject::tr("Warning"),
                                              QObject::tr("Failed to mount backup partition."),
                                              QObject::tr("OK"));

        // 此时还未进入应用的事件循环中，故不能使用qApp->quit();
        exit(1);
    }
}

/**
 * @brief 卸载备份分区
 * @note 应用关闭时卸载备份分区，以隐藏保护以防误删除
 */
void MainDialog::umountBackupPartition()
{
    // 卸载/backup分区
//    ComKylinBackupManagerInterface intf("com.kylin.backup", "/", QDBusConnection::systemBus());
//    intf.umountBackupPartition();
}

void MainDialog::closeEvent(QCloseEvent *e)
{
    if (GlobelBackupInfo::instance().isBusy()) {
        // 系统正忙，请稍等
        MessageBoxUtils::QMESSAGE_BOX_WARNING(this, QObject::tr("Warning"),
                                              QObject::tr("It's busy, please wait"),
                                              QObject::tr("OK"));

        e->ignore();
        return;
    }

//    if (GlobelBackupInfo::instance().hasBackupPartition())
//        umountBackupPartition();

    e->accept();
}

void MainDialog::resizeEvent(QResizeEvent *event) {
    emit GlobelBackupInfo::instance().getGlobalSignals()->widthChanged();
    QMainWindow::resizeEvent(event);
}

void MainDialog::closeBtn()
{
    if (this->close()) {
        //数据埋点：退出
        kdk_dia_upload_program_exit();
        qApp->quit();
    }
}

