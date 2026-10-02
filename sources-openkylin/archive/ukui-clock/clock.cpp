/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
#include "clock.h"
#include "ui_clock.h"
#include <QMessageBox>
#include "fieldvalidutil.h"
#include <QThread>
#include "coreplayer/playcontroller.h"
#include "kwidget.h"
#include "kwindowbuttonbar.h"
#include <QDBusReply>
#include "kwindowsystem.h"
#include <QScrollBar>
#include <kaboutdialog.h>
#include "windowmanager/windowmanager.h"
#include "dbusnoticeshowagainadaptor.h"
#include "clockdbusadaptor.h"
#include "dbusupdateclockadaptor.h"
#include "dbusselectclockbyidadaptor.h"
#include "dbusdeleteclockbyidadaptor.h"
#ifdef NOTKAIMING
#include "ukuiwindowhelper.h"
#endif
#include "globalinstance.h"
#include <QDebug>
#include <QDateTime>
#include <QRegExp>

#include <QLibraryInfo>
using std::string;
using clockInterface::ReturnMsg;
using clockInterface::STATUS_CODE;
using namespace kdk;

extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);
const double PI=3.141592;


#define SWITCH_BTN_HIGHLIGHT_BACK_COLOR 61,107,229,255

const static int DIALOG_MOVE_WIDTH = 24;
const static int DIALOG_WIDTH = 340;

Clock::Clock(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Clock)
{
    // 创建预加载窗口
    preLoadWidget = new QWidget(this);
    preLoadWidget->resize(392, 584);

    // 创建布局管理器，用于居中显示图片
    QVBoxLayout *layout = new QVBoxLayout(preLoadWidget);
    QLabel *imageLabel = new QLabel(preLoadWidget);
    imageLabel->setFixedSize(96,96);
    imageLabel->setPixmap(QIcon::fromTheme("kylin-alarm-clock").pixmap(imageLabel->size()));
    layout->addWidget(imageLabel , 0, Qt::AlignCenter);

    // 显示预加载窗口
    preLoadWidget->show();

    //右键菜单，选择”create new translation file“；项目中会添加一个以”.ts“为后缀的文件，例如”testtranslator_zh.ts“;
    //使用Qt Linguist打开该文件，就可以为指定的字符串设置译文；
    //在相应的“.ts”文件的右键菜单中点击“lupdate”项，才会更新。
    //翻译完成后，点击”文件“菜单下的”发布“项，就会生成.qm文件；
    QTranslator *translator = new QTranslator;
    // Get system locale
    //切换英文
    QLocale en_us = QLocale("en_US");
    QLocale bo_cn = QLocale("bo_CN");
    QLocale current = QLocale();
#ifdef NOTKAIMING
    if (translator->load(current, QLatin1String("ukui-clock"), QLatin1String("_"), QLatin1String("/usr/share/ukui-clock/ukui31"))){
        QApplication::installTranslator(translator);
    }
    else{
        qDebug() << "cannot load translator ukui-clock_" << QLocale::system().name() << ".qm!";
    }
#else
    if (translator->load(current, QLatin1String("ukui-clock"), QLatin1String("_"), QLatin1String("/opt/apps/top.openkylin.ukui-clock/files"))){
        QApplication::installTranslator(translator);
    }
    else{
        qDebug() << "cannot load translator ukui-clock_" << QLocale::system().name() << ".qm!";
    }
#endif
    //加载sdk翻译
    QTranslator *translator1 = new QTranslator;
    if (translator1->load(current, QLatin1String("gui"), QLatin1String("_"), QLatin1String(":translations"))){
        QApplication::installTranslator(translator1);
    }
    else{
        qDebug() << "cannot load translator gui_" << QLocale::system().name() << ".qm!";
    }
    //加载Qt翻译

    QTranslator * qtTranslator = new QTranslator;
    QString translationsPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
    if(qtTranslator->load(QLocale(), QStringLiteral("qt"), QLatin1String("_"), translationsPath)){
        QApplication::installTranslator(qtTranslator);
    }else{
        qDebug()<<"dbq- cannot load qt"<< QLocale::system().name() << ".qm!";
    }
    this->setWindowTitle(tr(CLOCK_TITLE_NAME));
}

Clock::~Clock()
{
    // 释放动态分配的指针，统一做空指针检查，避免风格不一致
    if (timer != nullptr) {
        delete timer;
        timer = nullptr; // 置空防止野指针
    }

    if (timer_2 != nullptr) {
        delete timer_2;
        timer_2 = nullptr;
    }

    if (countdown_timer != nullptr) {
        delete countdown_timer;
        countdown_timer = nullptr;
    }

    if (model != nullptr) {
        delete model;
        model = nullptr;
    }

    if (model_setup != nullptr) {
        delete model_setup;
        model_setup = nullptr;
    }

    if (watchTimeModel != nullptr) {
        delete watchTimeModel;
        watchTimeModel = nullptr;
    }

    if (dialog_repeat != nullptr) {
        delete dialog_repeat;
        dialog_repeat = nullptr;
    }

    if (dialog_music != nullptr) {
        delete dialog_music;
        dialog_music = nullptr;
    }

    if (count_music_sellect != nullptr) {
        delete count_music_sellect;
        count_music_sellect = nullptr;
    }

    // Qt UI 组件：先释放子页面，再释放 ui 根对象（需确认 ui 是 new 分配的）
    if (ui != nullptr) {
        if (ui->countdownRunPage != nullptr) {
            delete ui->countdownRunPage;
            ui->countdownRunPage = nullptr;
        }
        delete ui;
        ui = nullptr;
    }

    if (utils != nullptr) {
        delete utils;
        utils = nullptr;
    }

    if (primaryManager != nullptr) {
        delete primaryManager;
        primaryManager = nullptr;
    }

    if (alarmNoticeDialog != nullptr) {
        delete alarmNoticeDialog;
        alarmNoticeDialog = nullptr;
    }

    if (countdownNoticeDialog != nullptr) {
        delete countdownNoticeDialog;
        countdownNoticeDialog = nullptr;
    }

    if (tinycountdownDia != nullptr) {
        delete tinycountdownDia;
        tinycountdownDia = nullptr;
    }

    if (currentTheme != nullptr) {
        delete currentTheme;
        currentTheme = nullptr;
    }

    if (m_bellQueryModel != nullptr) {
        delete m_bellQueryModel;
        m_bellQueryModel = nullptr;
    }

    if (m_obj != nullptr) {
        delete m_obj;
        m_obj = nullptr;
    }

    if (m_updateObj != nullptr) {
        delete m_updateObj;
        m_updateObj = nullptr;
    }

    if (m_deleteObj != nullptr) {
        delete m_deleteObj;
        m_deleteObj = nullptr;
    }

    if (m_selectObj != nullptr) {
        delete m_selectObj;
        m_selectObj = nullptr;
    }
}


void Clock::showList()
{
    //    MediaPlayerPool::getInstance();
    // 创建定时器，用于延迟显示真实UI
    if(preLoadWidget != nullptr){
        preLoadWidget->hide();
        delete preLoadWidget;
        preLoadWidget = nullptr;
    }
    // 初始化并显示真实UI
    ui->setupUi(this);
    this->show();

    auto starttimer = QDateTime::currentMSecsSinceEpoch();
    qDebug()<< "main start time : " << starttimer;
    music = new QMediaPlayer();
    music->setObjectName("ukui-clock_QMediaPlayer_music");
    qDebug()<< "QMediaPlayer time : " << QDateTime::currentMSecsSinceEpoch() - starttimer;


    currentTheme = new theme();
    currentTheme->setObjectName("ukui-clock_theme_currentTheme");
    qDebug()<< "currentTheme time : " << QDateTime::currentMSecsSinceEpoch() - starttimer;
    //in order to use the same world in English
    ui->titileWidget->setWidgetName(tr(CLOCK_TITLE_NAME));
    //创建用户手册dbus客户端
    createUserGuideDebusClient();
    qDebug()<< "createUserGuideDebusClient time : " << QDateTime::currentMSecsSinceEpoch() - starttimer;
    mousePressed = 0;
    //创建或打开数据库
    clock_sql::createConnection();
    //设置数据库
    model_setup = clock_sql::getSetupTable(this);
    model_setup->select();
    //创建数据库
    m_bellQueryModel = clock_sql::getBellTable(this);
    watchTimeModel = clock_sql::getWatchTimeTable(this);
    qDebug()<< "clock_sql time : " << QDateTime::currentMSecsSinceEpoch() - starttimer;

    bellIni();
    qDebug()<< "bellIni time : " << QDateTime::currentMSecsSinceEpoch() - starttimer;
    //闹钟按钮图片初始化
    onTablet = GsettingSubject::getInstance()->getOnTablet();
    buttonImageInit();
    qDebug()<< "buttonImageInit time : " << QDateTime::currentMSecsSinceEpoch() - starttimer;
    //发布dbus接口
    publishDbusInterface();
    qDebug()<< "publishDbusInterface time : " << QDateTime::currentMSecsSinceEpoch() - starttimer;
    publishDbusUpdateClock();
    qDebug()<< "publishDbusUpdateClock time : " << QDateTime::currentMSecsSinceEpoch() - starttimer;
    publishDbusSelectClock();
    qDebug()<< "publishDbusSelectClock time : " << QDateTime::currentMSecsSinceEpoch() - starttimer;
    publishDbusDeleteClock();
    qDebug()<< "publishDbusDeleteClock time : " << QDateTime::currentMSecsSinceEpoch() - starttimer;
    //稍后提醒接口
    publishDbusNoticeShowAgain();
    qDebug()<< "publishDbusNoticeShowAgain time : " << QDateTime::currentMSecsSinceEpoch() - starttimer;
    //倒计时 运行光圈页初始化
    CountdownInit();
    qDebug()<< "CountdownInit time : " << QDateTime::currentMSecsSinceEpoch() - starttimer;
    //秒表初始化
    stopwatchInit();
    qDebug()<< "stopwatchInit time : " << QDateTime::currentMSecsSinceEpoch() - starttimer;
    //闹钟页初始化
    clockInit();
    qDebug()<< "clockInit time : " << QDateTime::currentMSecsSinceEpoch() - starttimer;
    setupInit();
    qDebug()<< "setupInit time : " << QDateTime::currentMSecsSinceEpoch() - starttimer;
    this->setFixedSize(392,584);
    /*实现鼠标左键滑动效果
    *Realize the sliding effect of left mouse button*/
    ui->alarmListWidget -> setFrameShape(QListWidget::NoFrame);
    ui->timeListWidget -> setFrameShape(QListWidget::NoFrame);
    ui->alarmListWidget->setAccessibleName("ukui-clock_QListWidget_alarmListWidget");
    ui->timeListWidget->setAccessibleName("ukui-clock_QListWidget_timeListWidget");
    ui->alarmListWidget -> setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);  /*设置像素级滑动    Set pixel level slide*/
    ui->timeListWidget -> setVerticalScrollMode(QAbstractItemView::ScrollPerPixel); /*设置像素级滑动  Set pixel level slide*/
    ui->alarmListWidget->setProperty("contextMenuPolicy", Qt::CustomContextMenu);    /*重要：设置QListWidget的contextMenuPolicy属性，不然不能显示右键菜单*/
    ui->alarmListWidget ->setGridSize(QSize(340, 108+15));
    ui->timeListWidget ->setGridSize(QSize(340, 58+10));
    ui->alarmListWidget->setProperty("drawScrollBarGroove", false);
    ui->timeListWidget->setProperty("drawScrollBarGroove", false);
    qDebug()<< "left mouse button time : " << QDateTime::currentMSecsSinceEpoch() - starttimer;
    //主屏信息
    primaryManager = new PrimaryManager();
    primaryManager->setObjectName("ukui-clock_PrimaryManager_primaryManager");
    qDebug()<< "primaryManager time : " << QDateTime::currentMSecsSinceEpoch() - starttimer;
    //闹钟居中
    //move((m_pSreenInfo->m_screenWidth - this->width() + m_pSreenInfo->m_nScreen_x )/2, (m_pSreenInfo->m_screenHeight - this->height())/2);
    utils = new Utils();
    utils->setObjectName("ukui-clock_Utils_utils");
    //utils->centerToScreen(this);
    qDebug()<< "centerToScreen time : " << QDateTime::currentMSecsSinceEpoch() - starttimer;
    ui->switchClock->setFixedSize(40,40);
    ui->switchClock->setAccessibleName("ukui-clock_KPushButton_switchClock");
    ui->switchClock->setAccessibleDescription(tr(CLOCK_TITLE_NAME));
    navigationBtnStyle(ui->switchClock);
    ui->switchClock->setToolTip(tr(CLOCK_TITLE_NAME));
    ui->switchCountdown->setFixedSize(40,40);
    navigationBtnStyle(ui->switchCountdown);
    ui->switchCountdown->setToolTip(tr("Count down"));
    ui->switchCountdown->setAccessibleName("ukui-clock_KPushButton_switchCountdown");
    ui->switchCountdown->setAccessibleDescription(tr("Count down"));
    ui->switchStopwatch->setFixedSize(40,40);
    navigationBtnStyle(ui->switchStopwatch);
    ui->switchStopwatch->setToolTip(tr("Watch"));
    ui->switchStopwatch->setAccessibleName("ukui-clock_KPushButton_switchStopwatch");
    qDebug()<< "switchClock time : " << QDateTime::currentMSecsSinceEpoch() - starttimer;

    connect(ui->switchCountdown, SIGNAL(clicked()), this, SLOT( CountdownPageSwitch ()));
    connect(ui->switchClock, SIGNAL(clicked()), this, SLOT( AlarmPageSwitch ()));
    connect(ui->switchStopwatch, SIGNAL(clicked()), this, SLOT( StopwatchPageSwitch ()));

    AlarmPageSwitch ();/*初始显示闹钟界面
                               Initial display alarm interface*/
    qDebug()<< "AlarmPageSwitch time : " << QDateTime::currentMSecsSinceEpoch() - starttimer;

    shadow->installEventFilter(this);
    ui->alarmListWidget->installEventFilter(this);
    ui->switchClock->installEventFilter(this);
    ui->switchCountdown->installEventFilter(this);
    ui->switchStopwatch->installEventFilter(this);

    //蓝色
    QPalette palette = ui->suspendCountdownBtn->palette();
    QColor ColorPlaceholderText(248,163,76,255);
    QBrush brush3;
    brush3.setColor(ColorPlaceholderText);
    //主题框架1.0.6-5kylin2
    palette.setColor(QPalette::Highlight,QColor(248,163,76,255));
    palette.setColor(QPalette::Button,QColor(248,163,76,255));
    palette.setBrush(QPalette::ButtonText, QBrush(Qt::white));
    ui->suspendCountdownBtn->setPalette(palette);
    startCountSingle->setEnabled(false);
    ui->countdownStackedW->setCurrentIndex(0);
    drawNoAlarmPrompt();//绘制无闹钟提示图标
    onMin_5btnClicked();//倒计时初始时间默认五分钟
    qDebug()<< "startCountSingle time : " << QDateTime::currentMSecsSinceEpoch() - starttimer;
    //字体设置
    //    QString selfFont = Utils::loadFontFamilyFromTTF();
    //    QFont f(selfFont);
    QFont f;
    f.setFamily("NotoSansCJKsc-Regular");
    f.setWeight(400);
    f.setPixelSize(38);
    ui->timeShowBig->setFont(f);
    ui->suspendCountdownBtn->setText(Utils::getOmitStr(tr("suspend"),7));
    ui->ringBtn->setText(Utils::getOmitStr(tr("count"),7));
    qDebug()<< "setFont time : " << QDateTime::currentMSecsSinceEpoch() - starttimer;
    //监听主题
    settingsStyle();
    //主题框架1.0.6-5kylin2
    qDebug()<< "settingsStyle time : " << QDateTime::currentMSecsSinceEpoch() - starttimer;
    //配置重要按钮

    //添加
    ui->addAlarmBtn->setProperty("isImportant", true);
    //新建闹钟保存
    ui->saveBtnOnEditAlarm->setProperty("isImportant", true);
    //倒计时 滚轮页设置页，开始
    startCountSingle->setProperty("isImportant", true);
    //倒计时，运行页，开始结束
    //    ui->count_stat->setProperty("isImportant", true);
    //秒表 开始结束
    ui->startStopwatch->setProperty("isImportant", true);
    //    倒计时 暂停继续
    ui->suspendCountdownBtn->setProperty("isImportant", true);
    //倒计时 开始结束
    ui->startCountdownBtn->setProperty("useButtonPalette", true);
    //闹钟编辑 取消
    ui->cancelbtnOnEditAlarm->setProperty("useButtonPalette", true);
    //秒表 计次
    ui->ringBtn->setProperty("useButtonPalette", true);
    qDebug()<< "setProperty time : " << QDateTime::currentMSecsSinceEpoch() - starttimer;
    //迷你窗口
    updateTinyBtn();
    //倒计时上的小闹钟
    //    ui->countdownAlarmIcon->setProperty("useButtonPalette", true);
    ui->countdownAlarmIcon->setAccessibleName("ukui-clock_QLabel_countdownAlarmIcon");
    qDebug()<< "updateTinyBtn time : " << QDateTime::currentMSecsSinceEpoch() - starttimer;

    iniWaylandWinId();
    qDebug()<< "iniWaylandWinId time : " << QDateTime::currentMSecsSinceEpoch() - starttimer;
}

//重写关闭事件
void Clock::closeEvent(QCloseEvent *event)
{
    exit(0);
    event->accept();
}

void Clock::sltMessageReceived()
{
    KWindowSystem::forceActiveWindow(this->winId());
}

void Clock::enableTrayIcon()
{
    if (m_trayIcon) {
        return;
    }
    openClock = new QAction(tr("open Clock") , this);
    openClock->setIcon(QIcon::fromTheme("kylin-alarm-clock"));
    openClock->setObjectName("ukui-clock_QAction_openClock");
    quitAction = new QAction(tr("Close"), this);
    quitAction->setIcon(QIcon::fromTheme("window-close-symbolic"));
    quitAction->setObjectName("ukui-clock_QAction_quitAction");
    connect(quitAction, &QAction::triggered, qApp,
            [&](){exit(0);});
    connect(openClock, &QAction::triggered, this, [=](){
        if (tinycountdownDia != nullptr) {
            tinycountdownDia->hide();
        }
        setWindowState(windowState() & ~Qt::WindowMinimized);
        activateWindow();// 激活窗口
        this->showThisWindow();
    });
    QMenu *trayIconMenu = new QMenu();
    trayIconMenu->setObjectName("ukui-clock_QMenu_trayIconMenu");
    trayIconMenu->addAction(openClock);
    trayIconMenu->addAction(quitAction);
    m_trayIcon = new QSystemTrayIcon();
    m_trayIcon->setObjectName("ukui-clock_QSystemTrayIcon_m_trayIcon");
    setDefaultTrayIconTooltip();
    m_trayIcon->setContextMenu(trayIconMenu);
    QIcon trayicon = QIcon::fromTheme("kylin-alarm-clock");
    m_trayIcon->setIcon(trayicon);
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, [=](QSystemTrayIcon::ActivationReason r){
        if (r == QSystemTrayIcon::Trigger) {
            if (isHidden() || !isActiveWindow()) {
                if(tinycountdownDia != nullptr){             //迷你小窗与主页面仅保留一个，弹出主界面后隐藏倒计时小窗
                    tinycountdownDia->hide();                //#279573 【闹钟】倒计时界面mini弹窗，任务栏缩略图会显示两个闹钟弹窗
                }
                setWindowState(windowState() & ~Qt::WindowMinimized);
                activateWindow(); // Qt 原生激活窗口
                this->showThisWindow();
            }else{
                hide();
            }
        }
    });
    m_trayIcon->show();
}
void Clock::updateTrayIconTooltip(QString info)
{
    if (m_trayIcon) {
        m_trayIcon->setToolTip(info);
    }
}

void Clock::setDefaultTrayIconTooltip()
{
    if (m_trayIcon) {
        if(m_trayIconTooltip!=""){
            m_trayIcon->setToolTip(m_trayIconTooltip);
        }else{
            m_trayIcon->setToolTip(tr(CLOCK_TITLE_NAME));
    }
    }
}
void Clock::disableTrayIcon()
{
#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
    if (m_trayIcon) {
        m_trayIcon->deleteLater();
    }
#endif
}
/*
*监听主题
*/
void Clock::settingsStyle()
{

    GsettingSubject * subject = GsettingSubject::getInstance();
    connect(subject,&GsettingSubject::blackStyle, this,[=](){
        this->blackStyle();
    });
    connect(subject,&GsettingSubject::whiteStyle, this,[=](){
        this->whiteStyle();
    });
    connect(subject,&GsettingSubject::iconChnaged, this,[=](){
        ui->titileWidget->setIcon("kylin-alarm-clock");
    });


    connect(subject,&GsettingSubject::timeZoneChanged, this,[=](QString timeZone){
        this->m_timeZone =timeZone;
    });

    connect(subject,&GsettingSubject::fontChanged, this,[=](int size){
        this->CURRENT_FONT_SIZE=size;
        this->updateFront(size);
    });

    subject->iniWidgetStyle();
    subject->iniFontSize();
    subject->iniTimeZone();

    connect(this,&Clock::timeJump,this,[=](qint64 timeValue){
        qWarning()<<"dbq检测到时间跳跃"<<timeValue;
        qDebug() << "##################################################### countdown " << countdown_isStarted;
        //倒计时运行时，做时间跳跃
        if(countdown_isStarted){
            int ringmax = countdown_hour*3600 + countdown_minute*60 + countdown_second;
            qWarning()<<"dbq-当前时间"<<countdown_hour<<countdown_minute<<countdown_second<<ringmax;
            if(timeValue>ringmax){
                //倒计时归零
                countdown_hour = 0;
                countdown_minute = 0;
                countdown_second = 0;
            }else{
                int jumpH = timeValue/3600;
                countdown_hour = countdown_hour-jumpH;
                timeValue = timeValue-jumpH*3600;
                int jumpM = timeValue/60;
                //分钟
                if(countdown_minute>=jumpM){
                    countdown_minute = countdown_minute-jumpM;
                }else{
                    countdown_hour--;
                    countdown_minute = countdown_minute-jumpM+60;
                }
                timeValue = timeValue-jumpM*60;
                //秒
                if(countdown_second>=timeValue){
                    countdown_second = countdown_second-timeValue;
                }else{
                    if(countdown_minute>=1){
                        countdown_minute--;
                    }else{
                        //借小时位
                        countdown_hour--;
                        countdown_minute = 59;
                    }
                    countdown_second = countdown_second-timeValue+60;
                }

            }
            qWarning()<<"dbq-减去跳跃时间"<<countdown_hour<<countdown_minute<<countdown_second;
            updateCountdownTime();
        }
    });

}
void Clock::iniWaylandWinId()
{
    connect(WindowManager::self(),&WindowManager::windowAdded,this,[=](const WindowId& windowId){
        if(getpid() == WindowManager::getPid(windowId))
        {
            if(m_first_pid){
                m_pid = windowId.toULongLong();
                m_first_pid=false;
            }
        }
    });
}

/**
 * @brief 闹钟按钮图片初始化 Alarm button picture initialization
 */
void Clock::buttonImageInit()
{
    pixmap4 = QPixmap(":/image/icon-4-16x16.png");
    //“重复”下拉框对号勾选
    repeat_on_Pixmap = QPixmap(":/image/object-select-symbolic.png");
    repeat_off_Pixmap = QPixmap("");
    //启用主题框架不需要代码
    //    this->setWindowIcon(QIcon::fromTheme("kylin-alarm-clock",QIcon(":/image/kylin-alarm-clock.svg")));
    //左上角闹钟图标
    ui->titileWidget->setIcon("kylin-alarm-clock");
    //倒计时上的小闹钟图标
    ui->countdownAlarmIcon->setPixmap(pixmap4);
    ui->countdownAlarmIcon->setVisible(true);
    ui->countdownAlarmIcon->setFocusPolicy(Qt::NoFocus);
    //    ui->countdownAlarmIcon->move(152,100);

    muteBtnStyle();
    minBtnStyle();
    menuBtnStyle();
    closeBtnStyle();
    ui->titileWidget->windowButtonBar()->maximumButton()->hide();
    disconnect(ui->titileWidget->windowButtonBar()->closeButton(), &QPushButton::clicked, this, &Clock::windowClosingClicked);


    //提醒铃声 倒计时设置页
    countdownBellLabelOnSet = new QLabel(ui->countdownSetPage);
    countdownBellLabelOnSet->setObjectName("ukui-clock_QLabel_countdownBellLabelOnSet");

    //提醒铃声 倒计时页面运行页
    countdownBellLabelOnRun = new QLabel(ui->countdownRunPage);
    countdownBellLabelOnRun->setObjectName("ukui-clock_QLabel_countdownBellLabelOnRun");
    int moveHeight = 334;
    updateClockSelectBtnLabel(countdownBellLabelOnSet,moveHeight,tr("  bell"));

    updateClockSelectBtnLabel(countdownBellLabelOnRun,moveHeight,tr("  bell"));
    //闹钟名称 新建闹钟
    int selectHeight = 225;
    int selectGap = 44;
    clockNamelabel = new QLabel(ui->editAlarmPage);
    clockNamelabel->setObjectName("ukui-clock_QLabel_clockNamelabel");
    updateClockSelectBtnLabel(clockNamelabel,selectHeight,tr("  name"));
    utils->resetText(clockNamelabel , 1);
    selectHeight+=selectGap;

    //重复 新建闹钟
    clockRepeatLabel = new QLabel(ui->editAlarmPage);
    clockRepeatLabel->setObjectName("ukui-clock_QLabel_clockRepeatLabel");
    updateClockSelectBtnLabel(clockRepeatLabel,selectHeight,tr("  repeat"));
    utils->resetText(clockRepeatLabel , 1);
    selectHeight+=selectGap;
    //提醒铃声 新建闹钟
    clockBellLabel = new QLabel(ui->addAlarmPage);
    clockBellLabel->setObjectName("ukui-clock_QLabel_clockBellLabel");
    updateClockSelectBtnLabel(clockBellLabel,selectHeight,tr("  bell"));

    selectHeight+=selectGap;
    //稍后提醒 新建闹钟
    clockRemindLabel = new QLabel(ui->editAlarmPage);
    clockRemindLabel->setObjectName("ukui-clock_QLabel_clockRemindLabel");
    updateClockSelectBtnLabel(clockRemindLabel,selectHeight,tr("  remind"));
    utils->resetText(clockRemindLabel , 1);
    if(onTablet){
        countdownBellLabelOnRun->setFixedWidth(100);//设置固定大小，系统放大字体遮挡
        countdownBellLabelOnRun->setText(tr("  bell"));
        countdownBellLabelOnRun->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
        countdownBellLabelOnSet->setFixedWidth(100);//设置固定大小，系统放大字体遮挡
        countdownBellLabelOnSet->setText(tr("  bell"));
        countdownBellLabelOnSet->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
        QFont font;
        font.setPixelSize(14);
        countdownBellLabelOnRun->setFont(font);
        countdownBellLabelOnSet->setFont(font);
        countdownBellLabelOnSet->move(this->width()/2-195+20,this->height()/2-168-238+330);
        countdownBellLabelOnRun->move(this->width()/2-195+20,this->height()/2-168-238+330);
        countdownBellLabelOnRun->hide();

        int selectHeight = 221;
        int selectGap = 56;
        updateClockEditPageLabel(selectHeight,selectGap,9);
    } else{
        updateClockSelectBtnLabel(countdownBellLabelOnSet,moveHeight,tr("  bell") , true);
        updateClockSelectBtnLabel(countdownBellLabelOnRun,moveHeight,tr("  bell") , true);
        utils->resetText(countdownBellLabelOnSet , 2);
        utils->resetText(countdownBellLabelOnRun , 2);

        int selectHeight = 218;
        int selectGap = 44;
        updateClockEditPageLabel(selectHeight,selectGap,7);
        utils->resetText(clockBellLabel , 1);

    }
}
/*
*倒计时页初始化
*Countdown page initialization
*/
void Clock::CountdownInit()
{
    /*初始化定时器
     Initialize timer*/
    countdown_timer = new QTimer();
    countdown_timer->setObjectName("ukui-clock_QTimer_countdown_timer");
    /*信号和槽
     Signals and slots*/
    //倒计时开始-结束
    connect(ui->startCountdownBtn, SIGNAL(clicked()), this, SLOT(startbtnCountdown()) );
    //    connect(tinycountdownDia, SIGNAL(finishClick()), this, SLOT(tinyCountdownFinish()) );
    //倒计时执行
    connect(countdown_timer, SIGNAL(timeout()), this, SLOT(statCountdownMsec()));
    /*设置定时器每个多少毫秒发送一个timeout()信号
     Set the timer to send a timeout () signal every milliseconds*/
    countdown_timer->setInterval(10);
    countdown_hour = 0;
    countdown_minute = 0;
    countdown_second = 9;
    countdown_pushflag = 0;

    countdown_isStarted = 0;
    countdown_isStarted_2 = 0;
    ui->countdownRunPage->countdownRunRoundBar->setRange(0,COUNTDOWN_TIME);
    /*初始化倒计时进度圈
     Initialize countdown progress circle*/
    ui->countdownRunPage->countdownRunRoundBar->setValue(COUNTDOWN_TIME);
    //父的顶部
    ui->startCountdownBtn->raise();
    //初始化倒计时弹窗
    countdownNoticeDialog = new Natice_alarm(60,-1);
    countdownNoticeDialog->setObjectName("ukui-clock_Natice_alarm_countdownNoticeDialog");
    countdownNoticeDialog->timer->stop();
    countdownNoticeDialog->timer_xumhuan->stop();
    countdownNoticeDialog->m_musicPlayer->stop();
    //小型窗体
    if(tinycountdownDia==nullptr){
        tinycountdownDia = new tinyCountdown();
        tinycountdownDia->setObjectName("ukui-clock_tinyCountdown_tinycountdownDia");
    }
    //绑定主窗体操作
    connect(tinycountdownDia,SIGNAL(mainWindowClick()),this,SLOT(activeWindow()));
    //结束
    connect(tinycountdownDia, &tinyCountdown::finishClick, this, [=](){
        tinyCountdownFinish();
    } );
    //暂停
    connect(tinycountdownDia, &tinyCountdown::suspendClick, this, [=](int onRun){
        countdown_isStarted_2 = onRun;
        onCountPushClicked();
    } );
}

void Clock::publishDbusNoticeShowAgain(){
    m_obj = new QObject();
    m_obj->setObjectName("ukui-clock_QObject_m_obj");
    auto noticePool = NoticeAlarmPoolUtil::getInstance();
    DbusNoticeShowAgainAdaptor *adaptor = new DbusNoticeShowAgainAdaptor(m_obj,noticePool);
    adaptor->setObjectName("ukui-clock_DbusNoticeShowAgainAdaptor_adaptor");
    //注册接口
    QDBusConnection::sessionBus().registerObject(CLOCK_DBUS_NOTICE_SHOW_AGAIN_PATH, m_obj);
    //判断是否注册成功
    if (!QDBusConnection::sessionBus().registerService(CLOCK_DBUS_SERVICE_NAME)) {
        qCritical()<<"注册接口失败"<<QDBusConnection::sessionBus().lastError().message();
        exit(1);
    }
    Q_UNUSED(adaptor);
}

QString Clock::getBellNameById(QString bellId)
{
    QString bellName;
    int index = m_selectBtnUtil->getBellIndexById(bellId);
    if(index<5){
        QList<QString> * bellTrList = m_selectBtnUtil->getDefaultBellTrList();
        bellName = bellTrList->at(index);
    }else{
        bellName = m_selectBtnUtil->getBellNameById(bellId);
    }
    return bellName;
}
qint64 Clock::updateNowMill() {
    qint64 n = QDateTime::currentDateTime().toMSecsSinceEpoch();
    qint64 d = n - last;
    last = n;
    return d;
}

qint64 getNowMill() {
    return QDateTime::currentDateTime().toMSecsSinceEpoch();
}

/*
*秒表页初始化
*Stopwatch page initialization
*/
void Clock::stopwatchInit()
{
    /*初始化定时器
     Initialize timer*/
    timer = new QTimer();
    timer->setObjectName("ukui-clock_QTimer_timer");
    /*信号和槽
     Signals and slots*/
    connect(timer, SIGNAL(timeout()), this, SLOT(CountDown()));
    /*设置定时器每个多少毫秒发送一个timeout()信号
     Set the timer to send a timeout () signal every milliseconds*/
    timer->setInterval(10);
    //显示时间大
    ui->timeShowBig->setText("00:00.00");
    //显示时间小
    ui->timeShowSmall->setText("00:00.00");
    QPalette ptext;
    ptext.setColor(QPalette::WindowText,Qt::gray);
    ui->timeShowSmall->setPalette(ptext);
    isStarted = 0;
    hour=0;
    minute=0;
    second=0;
    pushflag=0 ;
    /*时间间隔定时器
     Time interval timer*/
    timer_2 = new QTimer();
    timer_2->setObjectName("ukui-clock_QTimer_timer_2");
    //秒表的 每次计次后清零重新计算的时间
    connect(timer_2, SIGNAL(timeout()), this, SLOT(stopwatchJg()));
    timer_2->setInterval(10);
    stopwatch_minute = 0;
    stopwatch_second = 0;
    stopwatch_centi_second = 0;
    stopwatch_isStarted = 0;

    ui->timeShowBig->move(90,125);
    ui->timeShowSmall->move(90,190);
    ui->timeListWidget->move(25,230);
    ui->startStopwatch->raise();
    ui->ringBtn->raise();
    //计次间隔时间最长，最短
    timeSepOrderIndex = new QMap<long,int>();
    hisLongShortIndex = new QList<int>();
}

/*
*闹钟页初始化
*Alarm page initialization
*/
void Clock::clockInit()
{
    //    alarmNoticeDialog = new Natice_alarm(60,-1);
    QTimer *timer_clock = new QTimer(this);
    timer_clock->setObjectName("ukui-clock_QTimer_timer_clock");
    //动态监控闹钟与本地时间
    connect(timer_clock, SIGNAL(timeout()), this, SLOT(timerUpdate()) );/*动态获取时间
                                                                         Dynamic acquisition time*/
    timer_clock->start(1000);
    connect( ui->addAlarmBtn, SIGNAL(clicked()), this, SLOT(setAlarmClock()) );/*添加闹钟
                                                                                  Add alarm*/
    ui->timeShowBig->setAlignment(Qt::AlignHCenter);
    ui->timeShowSmall->setAlignment(Qt::AlignHCenter);
    ui->remainTime->setAlignment(Qt::AlignHCenter);
    //单个表读取和写入数据库记录的高级接口
    model = new QSqlTableModel(this);
    model->setObjectName("ukui-clock_QSqlTableModel_model");
    model->setTable("clock");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->select(); /*选取整个表的所有行
                      Select all rows of the entire table*/
    /*初始化一个包含两个Action(Delete和ClearAll)的菜单*/
    popMenu_In_ListWidget_ = new QMenu(this);
    popMenu_In_ListWidget_->setObjectName("ukui-clock_QMenu_popMenu_In_ListWidget_");
    action_Delete_In_ListWidget_ = new QAction(tr("Delete"), this);
    action_Delete_In_ListWidget_->setObjectName("ukui-clock_QAction_action_Delete_In_ListWidget_");
    action_Clear_In_ListWidget_ = new QAction(tr("ClearAll"), this);
    action_Clear_In_ListWidget_->setObjectName("ukui-clock_QAction_action_Clear_In_ListWidget_");
    action_edit_In_ListWidget_ = new QAction(tr("edit"), this);
    action_edit_In_ListWidget_->setObjectName("ukui-clock_QAction_action_edit_In_ListWidget_");
    popMenu_In_ListWidget_->addAction(action_edit_In_ListWidget_);
    popMenu_In_ListWidget_->addAction(action_Delete_In_ListWidget_);

    connect(this->action_Delete_In_ListWidget_, SIGNAL(triggered()), this, SLOT(deleteAlarm()));
    //编辑
    connect(this->action_edit_In_ListWidget_, SIGNAL(triggered()), this, SLOT(listdoubleClickslot()));
    connect(ui->alarmListWidget,SIGNAL(itemDoubleClicked(QListWidgetItem*)),this,SLOT(listdoubleClickslot()));
    connect(ui->saveBtnOnEditAlarm, SIGNAL(clicked()), this, SLOT(setAlarmSave()) );
    connect(ui->startStopwatch, SIGNAL(clicked()), this, SLOT(onPushbuttonStartClicked()));
    connect(ui->ringBtn, SIGNAL(clicked()), this, SLOT(onPushbuttonRingClicked()));
    connect(ui->cancelbtnOnEditAlarm, SIGNAL(clicked()), this, SLOT(alarmCancelSave()) );

    //暂停继续按钮
    connect(ui->suspendCountdownBtn, SIGNAL(clicked()), this, SLOT(onCountPushClicked()));


    connect(ui->tinyWindowBtn, SIGNAL(clicked()), this, SLOT(onTinyClicked()));
    /*绑定右键显示菜单：在单击右键之后会执行槽函数， 槽函数中负责弹出右键菜单*/
    connect(ui->alarmListWidget, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(onCustomContextMenuRequested(const QPoint &)));

    //初始化SystemTimeFlag，不然初始值可能不为0或者1
    iniSystemTimeFlag();
    //绘制闹钟列表
    updateAlarmClock();

}

void Clock::keyPressEvent(QKeyEvent *event)
{
    if(16777264==event->key()){
        callUserGuide();
    }

}

void Clock::callUserGuide()
{
    if(userGuideInterface!=nullptr){
        userGuideInterface->call(QString("showGuide"), "kylin-alarm-clock");
    }
}



/*
* 事件处理函数
*/
void Clock::onCustomContextMenuRequested(const QPoint &pos)
{
    /*弹出右键菜单*/
    if(ui->alarmListWidget->itemAt(pos)!=NULL){
        //右键弹窗与闹钟绑定
        popMenu_In_ListWidget_->exec(QCursor::pos());
    }
}

/*
* 默认初始设置
* Default initial settings
*/
void Clock::setupInit()
{
    countdownSetStartTime();/*倒计时初始化数字转盘
                               Countdown initialization digital turntable*/

    alarmSetStartTime();/*闹钟初始化数字转盘
                            Alarm initialization digital turntable*/
    modelSetupSet(); /*设置数据库初始化
                        Set database initialization*/
    textTimerupdate();
    comboxclickedflag = false;
    //闹钟名称
    int selectHeight = 218;
    int selectWidth = 100;
    int selectGap = 44;
    clockEditOnClockNew = new QLineEdit(ui->editAlarmPage);
    clockEditOnClockNew->setAccessibleName("ukui-clock_QLineEdit_clockEditOnClockNew");
    clockEditOnClockNew->resize(256,36);
    clockEditOnClockNew->move(selectWidth,selectHeight);
    clockEditOnClockNew->setMaxLength(16);
    selectHeight+=selectGap;
    connect(clockEditOnClockNew,&QLineEdit::textChanged,this,&Clock::textChanged);

    /*设置输入框无视空格,过滤特殊字符*/
    //  QRegExp rx = QRegExp("^[\u4E00-\u9FA5A-Za-z0-9_]+$");
    //  QRegExpValidator* validator = new QRegExpValidator(rx);
    //  clockEditOnClockNew->setValidator(validator);

    //倒计时的铃声
    QString bellId = model_setup->index(0, 1).data().toString();
    //    QString bellName = getBellNameById(bellId);

    //重复下拉框
    dialog_repeat = new CustomComBox(9,ui->editAlarmPage);
    dialog_repeat->setObjectName("ukui-clock_CustomComBox_dialog_repeat");
    dialog_repeat->resize(256,36);
    dialog_repeat->move(selectWidth,selectHeight);
    alarmRepeat();
    selectHeight+=selectGap;

    //铃声下拉框
    dialog_music = new QComboBox(ui->editAlarmPage);
    dialog_music->setObjectName("ukui-clock_QComboBox_dialog_music");
    dialog_music->resize(256,36);
    dialog_music->move(selectWidth,selectHeight);
    dialog_music->setEditable(true);  // 必须开启，才有 lineEdit
    dialog_music->setFocusPolicy(Qt::NoFocus);
    // 开启鼠标追踪，保证lineEdit在悬停时能收到MouseMove事件，进而触发Tooltip
    dialog_music->setMouseTracking(true);
    // 点击lineEdit任意位置弹出下拉框
    if (dialog_music->lineEdit()) {
        dialog_music->lineEdit()->setMouseTracking(true);
        dialog_music->lineEdit()->installEventFilter(this);
    }
    setMusicSelectDialogListById(bellId,dialog_music);
    selectAlarmMusic();
    selectHeight+=selectGap;

    //稍后提醒下拉框
    dialog_remind_late = new QComboBox(ui->editAlarmPage);
    dialog_remind_late->setObjectName("ukui-clock_QComboBox_dialog_remind_late");
    dialog_remind_late->resize(256,36);
    dialog_remind_late->move(selectWidth,selectHeight);
    selectRemindLate();

    //倒计时铃声选择
    count_music_sellect = new QComboBox(ui->countdownPage);
    count_music_sellect->setObjectName("ukui-clock_QComboBox_count_music_sellect");
    bool onTablet = GsettingSubject::getInstance()->getOnTablet();
    if(onTablet){
        int selectHeight = 221;
        int selectWidth = 100-6;
        int selectGap = 56;
        updateClockEditPageBtn(selectWidth,selectHeight,selectGap,48);

        count_music_sellect->resize(256,48);
        count_music_sellect->move(this->width()/2-95,this->height()/2-80);
    }else{
        int selectHeight = 218;
        int selectWidth = 100;
        int selectGap = 44;
        updateClockEditPageBtn(selectWidth,selectHeight,selectGap,36);

        int localeNum = m_selectBtnUtil->getCurrentLocale();
        if(localeNum == 6||localeNum == 7||localeNum == 9){
            //铃声下拉框
            count_music_sellect->resize(256,36);
            count_music_sellect->move(20,326);
        }else{
            //铃声下拉框
            count_music_sellect->resize(256,36);
            count_music_sellect->move(100,326);
        }
    }

    // 初始化 count_music_sellect 的缩略显示
    count_music_sellect->setEditable(true);
    count_music_sellect->setFocusPolicy(Qt::NoFocus);
    // 开启鼠标追踪，保证lineEdit在悬停时能收到MouseMove事件，进而触发Tooltip
    count_music_sellect->setMouseTracking(true);
    if (count_music_sellect->lineEdit()) {
        count_music_sellect->lineEdit()->setMouseTracking(true);
        count_music_sellect->lineEdit()->installEventFilter(this);
    }

    countdownMusicSellect();
    setMusicSelectDialogListById(bellId,count_music_sellect);
    //存储重复选择的星期，默认清零
    for (int i = 0; i < 9; i++) {
        repeat_day[i] = 0;
    }


    //下拉框选择
    connect(dialog_repeat, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &Clock::repeatListclickslot);
    connect(dialog_music, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &Clock::musicListclickslot);
    connect(dialog_remind_late, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &Clock::remindLateListClickSlot);
    connect(count_music_sellect, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &Clock::countMusicListclickslot);
    indexWeekdayMap = new QHash<int,QString>();
    indexWeekdayMap->insert(0,":monday");
    indexWeekdayMap->insert(1,":tuesday");
    indexWeekdayMap->insert(2,":wednesday");
    indexWeekdayMap->insert(3,":thusday");
    indexWeekdayMap->insert(4,":friday");
    indexWeekdayMap->insert(5,":saturday");
    indexWeekdayMap->insert(6,":sunday");
}

void Clock::bellIni()
{
    m_selectBtnUtil = new SelectBtnUtil();
    m_selectBtnUtil->setObjectName("ukui-clock_SelectBtnUtil_m_selectBtnUtil");
    int rowNum = m_bellQueryModel->rowCount();
    if (rowNum < 1) {
        QList<QString> * defaultBell = SelectBtnUtil::getDefaultBellList();
        for (int i=0;i<defaultBell->size();i++) {
            m_bellQueryModel->insertRow(i);
            QString temp = defaultBell->at(i);
            m_bellQueryModel->setData(m_bellQueryModel->index(i, 0), Utils::getRandomId());
            m_bellQueryModel->setData(m_bellQueryModel->index(i, 1), temp);
            QString path = DEFAULT_BELL_SAVE_PATH;
            path = path.append(temp).append(".wav");
            m_bellQueryModel->setData(m_bellQueryModel->index(i, 2), path);
            QDateTime time = QDateTime::currentDateTime();   //获取当前时间
            qint64 timeT = time.toMSecsSinceEpoch();
            m_bellQueryModel->setData(m_bellQueryModel->index(i, 3), timeT);
            m_bellQueryModel->setData(m_bellQueryModel->index(i, 4), 0);
            //排序，暂停1ms
            QThread::msleep(1);
        }
        delete defaultBell;
        m_bellQueryModel->submitAll();
        m_selectBtnUtil->refreshBellData();
    }else{
        QList<QString> * defaultBell = SelectBtnUtil::getDefaultBellList();
        delete defaultBell;
        m_bellQueryModel->submitAll();
        m_selectBtnUtil->refreshBellData();
    }
}

/*
 * 时间间隔执行
 * Interval calculation execution callback
 */
void Clock::stopwatchJg()
{
    qint64 now = getNowMill();
    m_runTime2 +=  now - m_stopWatchLastTime2;
    m_stopWatchLastTime2 = now;
    stopwatch_minute = m_runTime2 / 60000;
    int seconds = m_runTime2 % 60000;
    stopwatch_second = seconds / 1000;
    int mseconds = seconds % 1000;
    stopwatch_centi_second = mseconds / 10;

    if (stopwatch_minute < 10) {
        QString hours_str = QString::number(stopwatch_minute);
        stopwatch_jg_h = "0"+hours_str;
    } else {
        stopwatch_jg_h = QString::number(stopwatch_minute);
    }

    if (stopwatch_second < 10) {
        QString minute_str = QString::number(stopwatch_second);
        stopwatch_jg_m = "0"+minute_str;
    }else {
        stopwatch_jg_m = QString::number(stopwatch_second);
    }

    if (stopwatch_centi_second < 10) {
        QString second_str = QString::number(stopwatch_centi_second);
        stopwatch_jg_s = "0"+second_str;
    } else {
        stopwatch_jg_s = QString::number(stopwatch_centi_second);
    }

    ui->timeShowSmall->setText(stopwatch_jg_h+TIME_SEPARATOR+stopwatch_jg_m+"."+stopwatch_jg_s);
}

/*
 * 秒表执行
 * Stopwatch execution
 */
void Clock::CountDown()
{
    qint64 now = getNowMill();
    m_runTime +=  now - m_stopWatchLastTime;
    m_stopWatchLastTime = now;
    hour = m_runTime / 60000;
    int seconds = m_runTime % 60000;
    minute = seconds / 1000;
    int mseconds = seconds % 1000;
    second = mseconds / 10;

    if (hour < 10) {
        QString minutes_str = QString::number(hour);
        stopwatch_h = "0"+minutes_str;
    } else {
        stopwatch_h = QString::number(hour);
    }

    if (minute < 10) {
        QString second_str = QString::number(minute);
        stopwatch_m = "0"+second_str;
    } else {
        stopwatch_m = QString::number(minute);
    }

    if (second < 10) {
        QString msecond_str = QString::number(second);
        stopwatch_s = "0"+msecond_str;
    } else {
        stopwatch_s = QString::number(second);
    }
    ui->timeShowBig->setText(stopwatch_h+TIME_SEPARATOR+stopwatch_m+"."+stopwatch_s);
    //    QString selfFont = Utils::loadFontFamilyFromTTF();
    //    QFont f(selfFont);
    QFont f;
    f.setFamily("NotoSansCJKsc-Regular");
    f.setWeight(400);
    f.setPixelSize(38);
    ui->timeShowBig->setFont(f);
    //命名很不友好
    //second单位是10ms
    //    second++;
    //    //minute单位是秒
    //    if (second==100){
    //        minute++; second=0;
    //    }
    //    //hour单位是分钟
    //    if (minute==60){
    //        hour++; minute=0;
    //    }

}
/*
 * 秒表开始暂停继续
 * Stopwatch start pause continue
 */
void Clock::onPushbuttonStartClicked()
{
    if (!isStarted) {
        ui->startStopwatch->setText(Utils::getOmitStr(tr("suspend"),7));
        QPalette palette = ui->startStopwatch->palette();
        QColor ColorPlaceholderText(248,163,76,255);
        QBrush brush2;
        brush2.setColor(ColorPlaceholderText);
        //主题框架1.0.6-5kylin2
        palette.setColor(QPalette::Highlight,QColor(248,163,76,255));
        palette.setColor(QPalette::Button,QColor(248,163,76,255));
        palette.setBrush(QPalette::ButtonText, QBrush(Qt::white));
        ui->startStopwatch->setPalette(palette);

        if (stopwatch_isStarted == 0) {
            timer_2->start();
            m_stopWatchLastTime2 = getNowMill();
            stopwatch_isStarted = 1;
        }
        timer->start();
        // 每次都从新开始计算启动时间
        m_stopWatchLastTime = getNowMill();

        isStarted=1;
        if (!stopwatch_Animation) {
            stopwatchStartAnimation();
            stopwatch_Animation = 1;
        }
        ui->ringBtn->setText(Utils::getOmitStr(tr("count"),7));
    } else {
        // 暂停计时器，需要记录已经运行的时间
        timer->stop();
        qint64 now = getNowMill();
        m_runTime +=  now - m_stopWatchLastTime;

        /*查询间隔计时器是否启动
         Query whether the interval timer starts*/
        if (stopwatch_isStarted == 1) {
            timer_2->stop();
            m_runTime2 +=  now - m_stopWatchLastTime2;
            stopwatch_isStarted = 0;
        }
        isStarted=0;
        ui->startStopwatch->setText(tr("continue"));

        QPalette palette = ui->startStopwatch->palette();
        QColor ColorPlaceholderText(248,163,76,255);
        QBrush brush2;
        brush2.setColor(ColorPlaceholderText);
        //主题框架1.0.6-5kylin2
        palette.setColor(QPalette::Highlight,QColor(69, 173, 110,255));
        palette.setColor(QPalette::Button,QColor(69, 173, 110,255));
        palette.setBrush(QPalette::ButtonText, QBrush(Qt::white));
        ui->startStopwatch->setPalette(palette);
        ui->ringBtn->setText(Utils::getOmitStr(tr("reset"),7));
    }
    return;
}

/*
 *倒计时标签动画移动
 *Countdown start animation move
 */
void Clock::stopwatchStartAnimation()
{
    animation1 = new QPropertyAnimation(ui->timeShowBig, "geometry");
    animation1->setObjectName("ukui-clock_QPropertyAnimation_animation1");
    animation1->setDuration(1000);
    animation1->setKeyValueAt(0, QRect(90, 125, 210, 70));
    animation1->setEndValue(QRect(90, 4, 210, 70));
    animation1->start();

    animation2 = new QPropertyAnimation(ui->timeShowSmall, "geometry");
    animation2->setObjectName("ukui-clock_QPropertyAnimation_animation2");
    animation2->setDuration(1000);
    animation2->setKeyValueAt(0, QRect(90, 173, 210, 41));
    animation2->setEndValue(QRect(90, 60, 210, 41));
    animation2->start();

    animation3 = new QPropertyAnimation(ui->timeListWidget, "geometry");
    animation3->setObjectName("ukui-clock_QPropertyAnimation_animation3");
    animation3->setDuration(1000);
    animation3->setKeyValueAt(0, QRect(25 ,230, 356, 270));
    animation3->setEndValue(QRect(25, 109, 356, 270));
    animation3->start();
}
/*
 * 倒计时开始动画移动
 * Countdown start animation move
 */
void Clock::stopwatchStopAnimation()
{
    animation1 = new QPropertyAnimation(ui->timeShowBig, "geometry");
    animation1->setObjectName("ukui-clock_QPropertyAnimation_animation1");
    animation1->setDuration(1000);
    animation1->setKeyValueAt(0, QRect(90, 4, 210, 70));
    animation1->setEndValue(QRect(90, 125, 210, 70));
    animation1->start();

    animation2 = new QPropertyAnimation(ui->timeShowSmall, "geometry");
    animation2->setObjectName("ukui-clock_QPropertyAnimation_animation2");
    animation2->setDuration(1000);
    animation2->setKeyValueAt(0, QRect(90, 55, 210, 41));
    animation2->setEndValue(QRect(90, 183, 210, 41));
    animation2->start();

    animation3 = new QPropertyAnimation(ui->timeListWidget, "geometry");
    animation3->setObjectName("ukui-clock_QPropertyAnimation_animation3");
    animation3->setDuration(1000);
    animation3->setKeyValueAt(0, QRect(25 ,109, 356, 270));
    animation3->setEndValue(QRect(25, 230, 356, 270));
    animation3->start();
}

/*
 * 计次
 * times count
 */
void Clock::onPushbuttonRingClicked()
{
    if (!isStarted) {
        //复位
        //清空数据
        timeSepOrderIndex->clear();
        hisLongShortIndex->clear();
        ui->timeListWidget->verticalScrollBar()->setHidden(true);
        if (nullptr != timer) {
            timer->stop();
            timer_2->stop();
            ui->timeShowBig->setText("00:00.00");
            ui->timeShowSmall->setText("00:00.00");
            isStarted = 0;
            stopwatch_isStarted = 0;
            hour = 0; minute = 0;second = 0;
            stopwatch_minute = 0;
            stopwatch_second = 0;
            stopwatch_centi_second = 0;
            m_runTime = 0;
            m_stopWatchLastTime = 0;
            m_runTime2 = 0;
            m_stopWatchLastTime2 = 0;
            ui->startStopwatch->setText(tr("start"));
            ui->ringBtn->setText(tr("count"));
            ui->startStopwatch->setPalette(ui->addAlarmBtn->palette());
            for (int i=0; i < stopwatch_item_flag; i++) {
                delete stopwatch_w[i];
                delete stopwatch_aItem[i];
            }
            stopwatch_item_flag = 0;
            if (stopwatch_Animation) {
                stopwatchStopAnimation();
                stopwatch_Animation = 0;
            }
        }
    }else{
        //计次
        if (stopwatch_isStarted == 0) {
            timer_2->start();
            m_stopWatchLastTime2 = getNowMill();

            stopwatch_isStarted = 1;
        }
        if (stopwatch_item_flag < 100) {
            stopwatch_aItem[stopwatch_item_flag] =new QListWidgetItem;
            stopwatch_aItem[stopwatch_item_flag]->setSizeHint(QSize(340,58));
            stopwatch_aItem[stopwatch_item_flag]->setTextColor(QColor(255, 0, 0, 255));
            stopwatch_aItem[stopwatch_item_flag]->setToolTip(tr("count")+QString::number(stopwatch_item_flag+1));
            stopwatch_aItem[stopwatch_item_flag]->setToolTip(tr("interval ")+stopwatch_jg_h+TIME_SEPARATOR+stopwatch_jg_m+"."+stopwatch_jg_s);
            stopwatch_aItem[stopwatch_item_flag]->setData(Qt::AccessibleTextRole,tr("interval ")+stopwatch_jg_h+TIME_SEPARATOR+stopwatch_jg_m+"."+stopwatch_jg_s);
            ui->timeListWidget->insertItem(0,stopwatch_aItem[stopwatch_item_flag]);

            ui->timeListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
            ui->timeListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            ui->timeListWidget->verticalScrollBar()->setHidden(true);
            if(stopwatch_item_flag>2){
                ui->timeListWidget->verticalScrollBar()->setHidden(false);
            }
            stopwatch_w[stopwatch_item_flag] = new stopwatch_item(ui->timeListWidget);
            stopwatch_w[stopwatch_item_flag]->stopwatch1->setText(tr("count")+QString::number(stopwatch_item_flag+1));
            stopwatch_w[stopwatch_item_flag]->stopwatch2->setText(tr("interval ")+stopwatch_jg_h+TIME_SEPARATOR+stopwatch_jg_m+"."+stopwatch_jg_s);
            stopwatch_w[stopwatch_item_flag]->setAccessibleName("ukui-clock_QWidget_stopwatch_w");
            updateLabelStrCnEn(stopwatch_w[stopwatch_item_flag]->stopwatch2,14,18,14);
            //间隔时间存入列表
            long sepTime = static_cast<long>(stopwatch_minute) * 6000 +
                    static_cast<long>(stopwatch_second) * 100 +
                    static_cast<long>(stopwatch_centi_second);
            timeSepOrderIndex->insert(sepTime,stopwatch_item_flag);
            //添加标签
            updateLongestShortLabel();

            stopwatch_w[stopwatch_item_flag]->stopwatch3->setText(stopwatch_h+TIME_SEPARATOR+stopwatch_m+"."+stopwatch_s);
            //            QString selfFont = Utils::loadFontFamilyFromTTF();
            //            QFont f(selfFont);
            QFont f;
            f.setFamily("NotoSansCJKsc-Regular");
            f.setWeight(400);
            f.setPixelSize(24);
            stopwatch_w[stopwatch_item_flag]->stopwatch3->setFont(f);

            ui->timeListWidget->setItemWidget(stopwatch_aItem[stopwatch_item_flag],stopwatch_w[stopwatch_item_flag]);

            stopwatch_minute = 0;
            stopwatch_second = 0;
            stopwatch_centi_second = 0;
            m_runTime2 = 0;
            stopwatch_item_flag++;

        } else if(stopwatch_item_flag >= 100) {
            QToolTip::showText(mapToGlobal(ui->timeListWidget->pos()+QPoint(90,340)),tr("up to 100 times"),this);
        }
    }


    //秒表页面子项字体更新
    updateStopwatchItemFront(CURRENT_FONT_SIZE);
}

void Clock::updateLongestShortLabel()
{
    QListIterator<int> iterator(*hisLongShortIndex);
    while (iterator.hasNext()) {
        int index = iterator.next();
        stopwatch_w[index]->updateTipCommonStyle();
    }
    long shortIndex = timeSepOrderIndex->first();
    stopwatch_w[shortIndex]->updateTipShortestStyle();
    int longIndex = timeSepOrderIndex->last();
    stopwatch_w[longIndex]->updateTipLongestStyle();
    updateLabelStrCnEn(stopwatch_w[longIndex]->tipLabel,2,3,3);
    updateLabelStrCnEn(stopwatch_w[shortIndex]->tipLabel,2,3,3);
    //存储到历史
    hisLongShortIndex->clear();
    hisLongShortIndex->append(shortIndex);
    hisLongShortIndex->append(longIndex);


}

/*
 *复位
 *reset
 */
void Clock::onPushbuttonTimeselectClicked()
{
    if (nullptr != timer) {
        if (isStarted)
            return;
        timer->stop();
        timer_2->stop();
        ui->timeShowBig->setText("00:00.00");
        ui->timeShowSmall->setText("00:00.00");
        isStarted = 0;
        stopwatch_isStarted = 0;
        hour = 0; minute = 0;second = 0;
        stopwatch_minute = 0;
        stopwatch_second = 0;
        stopwatch_centi_second = 0;
        ui->startStopwatch->setText(tr("start"));
        ui->startStopwatch->setPalette(ui->addAlarmBtn->palette());

        for (int i=0; i < stopwatch_item_flag; i++) {
            delete stopwatch_w[i];
            delete stopwatch_aItem[i];
        }
        stopwatch_item_flag = 0;
        if (stopwatch_Animation) {
            stopwatchStopAnimation();
            stopwatch_Animation = 0;
        }
    }
}

/*
 * 窗口关闭
 * window closing
 */
void Clock::windowClosingClicked()
{
    ui->titileWidget->setVisible(true);
    this->hide();
    //加入托盘
    enableTrayIcon();
}

/*
 *窗口最小化
 *window minimizing
 */
void Clock::windowMinimizingClicked()
{
    this->showNormal();
    this->showMinimized();
}

void Clock::muteAllBell()
{
    //静音反转
    qInfo() << "position x" << m_commonToolTip->geometry().width()/2 << ":y" <<m_commonToolTip->geometry().height()/2;
    m_muteOn = !m_muteOn;
    QPixmap pixmap = QPixmap(":/image/miniIcon/mute-off.png");
    if(currentTheme->themetype==0){
        pixmap = QPixmap(":/image/miniIcon/mute-off.png");
    }else{
        pixmap = QPixmap(":/image/miniIcon/mute-off-dark.png");
    }
    if(m_muteOn){
        pixmap = QPixmap(":/image/miniIcon/mute-on.png");
    }
    muteBtn->setIcon(pixmap);
    if(m_muteOn){
        muteBtn->setToolTip(tr("Cancel Mute"));
        m_commonToolTip->setText(tr("All bells are off"));
        m_commonToolTip->adjustSize();
        QPoint position = frameGeometry().center();
        // 最初是通过m_commonToolTip大小计算显示位置，但由于程序启动后初次点击静音按钮时，m_commonToolTip未完成渲染，获取不到争取尺寸
        //　直接固定m_commonToolTip大小进行计算
        int x = position.x() - m_commonToolTip->width() / 2;
        int y = position.y() - m_commonToolTip->height() / 2;
        m_commonToolTip->move(x,y);
        m_commonToolTip->showThisToolTip();

        //存储静音到数据库
        model_setup->setData(model_setup->index(0, 0), 1);//静音    Mute
        if(countdownNoticeDialog->getNoticeId()==1){
            if(countdownNoticeDialog->m_musicPlayer->state()!= MMediaPlayer::State::PausedState){
                countdownNoticeDialog->m_musicPlayer->pause();
            }
        }else{
            countdownNoticeDialog->m_musicPlayer->stop();
        }
        if(alarmNoticeDialog){
            if(alarmNoticeDialog->getNoticeId()==1)
            {
                if(alarmNoticeDialog->m_musicPlayer->state() != MMediaPlayer::State::PausedState){
                    alarmNoticeDialog->m_musicPlayer->pause();
                }
            }else{
                alarmNoticeDialog->m_musicPlayer->stop();
            }
        }
        //试听音乐开启静音
        if(playController::getInstance().getState() == playController::PlayState::PLAY_STATE){
            playController::getInstance().pause();
        }
    }else{
        muteBtn->setToolTip(tr("Mute"));
        m_commonToolTip->setText(tr("All bells are on"));
        m_commonToolTip->adjustSize();
        QPoint position = frameGeometry().center();
        int x = position.x() - m_commonToolTip->width() / 2;
        int y = position.y() - m_commonToolTip->height() / 2;
        m_commonToolTip->move(x,y);
        m_commonToolTip->showThisToolTip();

        model_setup->setData(model_setup->index(0, 0), 0);//静音    Mute
        if(countdownNoticeDialog->getNoticeId()==1){
            ringContinueWhenMute(countdownNoticeDialog);
        }else{
            countdownNoticeDialog->m_musicPlayer->stop();
        }
        if(alarmNoticeDialog){
            if(alarmNoticeDialog->m_musicPlayer->state()!=MMediaPlayer::State::PausedState){
                alarmNoticeDialog->m_musicPlayer->pause();
            }
            if(alarmNoticeDialog->getNoticeId()==1){
                if(alarmNoticeDialog->m_musicPlayer->state()==MMediaPlayer::State::PlayingState){
                    alarmNoticeDialog->m_musicPlayer->pause();
                }
                ringContinueWhenMute(alarmNoticeDialog);
            }else{
                alarmNoticeDialog->m_musicPlayer->stop();
            }
        }
        //试听音乐取消静音
        auto sumtime = playController::getInstance().getPlayer()->duration();
        QTime time = QTime::currentTime();
        int timeH = time.hour()-musicStartTime.hour();
        int timeM = time.minute()-musicStartTime.minute();
        int timeS = time.second()-musicStartTime.second();
        auto restarttime = (timeH*3600+timeM*60+timeS) * 1000;
        if(playController::getInstance().getState() == playController::PlayState::PLAY_STATE){
            playController::getInstance().getPlayer()->pause();
        }
        if(restarttime<sumtime && playController::getInstance().getState() == playController::PlayState::PAUSED_STATE){
            playController::getInstance().getPlayer()->setPosition(restarttime);
        }
    }
    model_setup->submitAll();
}

void Clock::ringContinueWhenMute(Natice_alarm *tempdialog)
{
    auto startpos = tempdialog->timer_value * 1000;
    auto starttime = 60000-startpos;
    auto sumtime = tempdialog->m_musicPlayer->duration();
    int num = 0;
    if(tempdialog == countdownNoticeDialog){
        num=count_music_sellect->currentIndex();
    }else if(tempdialog == alarmNoticeDialog){
        //根据id查询数据
        auto sqlQuery = getClockByPK(tempdialog->getClockId());
        QString bellId =  sqlQuery.value( 2).toString();
        num=m_selectBtnUtil->getBellIndexById(bellId);
    }
    //调整自定义铃声的播放位置
    if(num<5){
        tempdialog->m_musicPlayer->pause();
    }else{
        if(sumtime>1){
            if(starttime>sumtime){
                starttime = starttime%sumtime;
            }
            tempdialog->m_musicPlayer->setPosition(starttime);
        }else{
            tempdialog->m_musicPlayer->pause();
        }
    }
}



/*
 * 倒计时切换
 * Countdown switch
 */
void Clock:: CountdownPageSwitch ()
{
    setSwitchHighlightColor(ui->switchCountdown);
    setSwitchDefaultColor(ui->switchClock);
    setSwitchDefaultColor(ui->switchStopwatch);
    ui->mainWidget->setCurrentIndex(0);
    // 切换到倒计时页面后更新缩略显示
    updateMusicLineEditElidedText(count_music_sellect);
}
void Clock::setBtnIcon(KPushButton *btn, QString imgUrl, QString localUrl)
{
    QIcon icon=Utils::getQIcon(imgUrl,localUrl);
    //    btn->setIcon(QIcon::fromTheme(imgUrl));
    btn->setIcon(icon);
    btn->setIconSize(QSize(18, 18));
    if(theme::themetype==0){
        btn->setIconColor(Qt::black);
    }else{
        btn->setIconColor(Qt::white);
    }
    QPainter p(btn);
    //    表示引擎应尽可能对图元的边缘进行抗锯齿。
    p.setRenderHint(QPainter::Antialiasing);
}
void Clock::setDefaultIcon(KPushButton *btn)
{
    if(btn==ui->switchClock){
        setBtnIcon(ui->switchClock,"ukui-alarm-symbolic",":/image/alarm.png");
    }else if(btn==ui->switchCountdown){
        setBtnIcon(ui->switchCountdown,"ukui-countselect-symbolic",":/image/count.png");
    }else if(btn==ui->switchStopwatch){
        setBtnIcon(ui->switchStopwatch,"stopwatch-symbolic",":/image/stopwatch.png");
    }
    if(theme::themetype==1){
        changeIconHeight(btn);
    }
}
QColor Clock::getHighlightActive()
{
    return palette().color(QPalette::Active,QPalette::Highlight);
}

QColor Clock::getButtonActive()
{
    return palette().color(QPalette::Active,QPalette::Button);
}
void Clock::setSwitchDefaultColor(KPushButton * btn)
{
    //    auto pa = btn->palette();
    //    QColor btnAcCol = getButtonActive();
    //    pa.setColor(QPalette::Active,QPalette::Button,btnAcCol);
    //    btn->setPalette(pa);
    btn->setBackgroundColorHighlight(false);
    setDefaultIcon(btn);
}
void Clock::setSwitchHighlightColor(KPushButton * btn)
{

    //    auto pa = btn->palette();
    //    QColor highAcCol = getHighlightActive();
    //    pa.setColor(QPalette::Active,QPalette::Button,highAcCol);
    //    btn->setPalette(pa);
    btn->setBackgroundColorHighlight(true);
    //修改图标颜色
    btn->setIconColor(Qt::white);
    //    changeIconHeight(btn);

}
/*
 * 闹钟窗口切换
 * Alarm window switchin
 */
void Clock:: AlarmPageSwitch ()
{
    setSwitchDefaultColor(ui->switchCountdown);
    setSwitchHighlightColor(ui->switchClock);
    setSwitchDefaultColor(ui->switchStopwatch);
    ui->mainWidget->setCurrentIndex(1);
}
void Clock::changeIconHeight(KPushButton *btn)
{
    QPixmap pix = btn->icon().pixmap(btn->iconSize());
    QPixmap tarPix=theme::changeIconColor(pix,palette().highlightedText().color());
    btn->setIcon(QIcon(tarPix));
}
/*
 * 秒表窗口切换
 * Stopwatch window switch
 */
void Clock:: StopwatchPageSwitch ()
{
    setSwitchDefaultColor(ui->switchCountdown);
    setSwitchDefaultColor(ui->switchClock);
    setSwitchHighlightColor(ui->switchStopwatch);
    ui->mainWidget->setCurrentIndex(2);
}


/**
 * @brief 闹钟列表项
 */
void Clock::textTimerupdate()
{
    //闹钟表条数
    int rowNum = model->rowCount();
    //1s调用一次
    model_setup->select();
    QTime time = QTime::currentTime();
    int time_H = time.hour();
    int time_M = time.minute();
    int time_S = time.second();
    //Timeformat
    //系统24时
    if (checkSystem24()) {
        set24ClockItem(time_H,time_M,time_S,rowNum);
    } else {
        //系统12时制
        set12ClockItem(time_H,time_M,time_S,rowNum);
    }
}

/**
 * @brief 设置24时制闹钟
 */
void Clock::set24ClockItem(int time_H,int time_M,int time_S,int rowNum)
{
    if(system_time_flag == 0){
        system_time_flag = 1;
        clearClockItem( rowNum);
    } else {
        system_time_flag = 1;
    }
}
/**
 * @brief 设置12时制闹钟
 */
void Clock::set12ClockItem(int time_H,int time_M,int time_S,int rowNum)
{
    //原来是24时，则重新构建
    if (system_time_flag == 1) {
        system_time_flag = 0;
        clearClockItem(rowNum);
    } else {
        system_time_flag = 0;
    }
}
/**
 * @brief 清理重新构建闹钟列表
 */
void Clock::clearClockItem(int rowNum)
{
    for(int i=0; i<rowNum; i++) {
        delete aItem[i];
        delete w1[i];
    }
    updateAlarmClock();
}
/**
 * @brief 初始化系统时间标识
 */
void Clock::iniSystemTimeFlag()
{
    //跟随系统
    if(checkSystem24()){
        system_time_flag = 1;
    }else{
        system_time_flag = 0;
    }

}
/**
 * @brief 判断系统是否是24时制
 */
bool Clock::checkSystem24()
{
    //Qt提供了一个QProcess类用于启动外部程序并与之通信
    QProcess process;
    process.start("gsettings get org.ukui.control-center.panel.plugins hoursystem");
    //阻塞，直到外部程序结束
    process.waitForFinished();
    QByteArray output = process.readAllStandardOutput();
    QString str_output = output;
    return str_output.compare("'24'\n") == 0;
}

void Clock::muteBtnStyle()
{
    muteBtn = new QPushButton(ui->titileWidget);
    muteBtn->setAccessibleName("ukui-clock_QPushButton_muteBtn");
    muteBtn->setAccessibleDescription(tr("Mute"));
    muteBtn->setFixedSize(30,30);

    QString systemLang = QLocale::system().name();
    if(systemLang=="ug_CN"||systemLang=="kk_KZ"||systemLang=="ky_KG"){
        muteBtn->move(105,5);
    }else{
        muteBtn->move(253,5);
    }
    int mute = model_setup->index(0, 0).data().toInt();
    m_muteOn = mute==1?true:false;
    muteBtn->setIconSize(QSize(32, 32));
    QPainter p(muteBtn);
    //表示引擎应尽可能对图元的边缘进行抗锯齿。
    p.setRenderHint(QPainter::Antialiasing);
    muteBtn->setFlat(true);
    muteBtn->setVisible(true);
    muteBtn->setFocusPolicy(Qt::NoFocus);
    muteBtn->setProperty("isWindowButton", 0x1);
    if(!m_muteOn){
        muteBtn->setToolTip(tr("Mute"));
    } else {
        muteBtn->setToolTip(tr("Cancel Mute"));
    }
    //弹窗初始化
    m_commonToolTip = new CommonToolTip(this);
    m_commonToolTip->setObjectName("ukui-clock_CommonToolTip_m_commonToolTip");
    connect(muteBtn, SIGNAL(clicked()), this, SLOT(muteAllBell()));


}
/**
 * @brief 最小化按钮样式
 */
void Clock::minBtnStyle()
{
    //窗口最小化
    QPushButton* btn = ui->titileWidget->windowButtonBar()->minimumButton();
    btn->setToolTip(tr("Minimize"));
    connect(btn, SIGNAL(clicked()), this, SLOT(windowMinimizingClicked()));
}
/**
 * @brief 关闭按钮样式
 */
void Clock::closeBtnStyle()
{
    //适配sdk
    QPushButton* btn = ui->titileWidget->windowButtonBar()->closeButton();
    btn->setToolTip(tr("Close"));
    disconnect(ui->titileWidget->windowButtonBar()->closeButton(), &QPushButton::clicked, this, &KWidget::close);
    connect(btn, SIGNAL(clicked()), this, SLOT(windowClosingClicked()));
}
/**
 * @brief 菜单按钮样式
 */
void Clock::menuBtnStyle()
{
    //适配sdk
    //删除原sdk中的多余选项
    KMenuButton* btn = ui->titileWidget->windowButtonBar()->menuButton();
    btn->setToolTip(tr("Option"));
    QMenu* menu = ui->titileWidget->windowButtonBar()->menuButton()->menu();
    QAction* settingAction = ui->titileWidget->windowButtonBar()->menuButton()->settingAction();
    QAction* themeAction = ui->titileWidget->windowButtonBar()->menuButton()->themeAction();
    menu->removeAction(settingAction);
    menu->removeAction(themeAction);
    //帮助
    QAction* m_helpAction = ui->titileWidget->windowButtonBar()->menuButton()->assistAction();
    //关于
    QAction* m_aboutAction = ui->titileWidget->windowButtonBar()->menuButton()->aboutAction();
    //关闭
    QAction* m_closeAction = ui->titileWidget->windowButtonBar()->menuButton()->quitAction();
    m_helpAction->setText(tr("Help"));
    m_aboutAction->setText(tr("About"));
    m_closeAction->setText(tr("Quit"));
    connect(m_helpAction, &QAction::triggered, this, [=](){
        qDebug() << "help clicked";
        callUserGuide();
    });
    connect(m_aboutAction, &QAction::triggered, this, [=](){
        QApplication::setApplicationName(tr(CLOCK_TITLE_NAME));
#ifdef NOTKAIMING
    QString version = utils->getClockVersion();
#else
    QString version = QString(qgetenv("APP_VERSION"));
#endif
        KAboutDialog *aboutDialog = new KAboutDialog(this,QIcon::fromTheme("kylin-alarm-clock"),tr(CLOCK_TITLE_NAME),tr("Version：")+version);
        aboutDialog->setObjectName("ukui-clock_KAboutDialog_aboutDialog");
#ifdef NOTKAIMING
        if(GlobalInstance::getInstance()->session_desktop() == GlobalInstance::KYLIN_WLCOM){
            UkuiWindowHelper *windowHelper = new UkuiWindowHelper(aboutDialog);
            windowHelper->removeTitleBar();
        } else {
            kdk::UkuiStyleHelper::self()->removeHeader(aboutDialog);
        }
#else
        kdk::UkuiStyleHelper::self()->removeHeader(aboutDialog);
#endif
        aboutDialog->setBodyTextVisiable(true);
        aboutDialog->exec();
    });
    //    connect(m_closeAction, SIGNAL(triggered()), this, SLOT(windowClosingClicked()));
    connect(m_closeAction, &QAction::triggered, this, [=](){                              //“退出”按钮为直接退出应用
        exit(0);
    });
}
/*
 * @brief 检查小窗体是否初始化
 */
bool Clock::checkTinyCountdownDia()
{
    return tinycountdownDia!=nullptr?true:false;
}

void Clock::navigationBtnStyle(KPushButton * btn)
{
    setDefaultIcon(btn);;
    //不使用主题高亮
    btn->setProperty("useIconHighlightEffect", false);
    //使用灰按钮
    btn->setProperty("useButtonPalette", true);
}

void Clock::updateClockSelectBtnLabel(QLabel *temp, int moveHeight, QString text , bool flag)
{
    temp->setFixedWidth(100);//设置固定大小，系统放大字体遮挡
    temp->setText(text);
    int labelLeftSpace;
    QString systemLang = QLocale::system().name();
    temp->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    if(systemLang=="ug_CN"||systemLang=="kk_KZ"||systemLang=="ky_KG"){
        labelLeftSpace = 280;
    }else{
        if(flag)
            labelLeftSpace = 55;
        else
            labelLeftSpace = 20;
    }
    QFont font;
    font.setPixelSize(14);
    temp->setFont(font);
    // int labelLeftSpace = 20;
    temp->move(labelLeftSpace, moveHeight);
}

void Clock::updateClockEditPageLabel(int selectHeight, int selectGap, int labelHeightOffset)
{
    //闹钟名称
    updateClockSelectBtnLabel(clockNamelabel,selectHeight+labelHeightOffset,tr("  name"));
    selectHeight+=selectGap;
    //重复下拉框
    updateClockSelectBtnLabel(clockRepeatLabel,selectHeight+labelHeightOffset,tr("  repeat"));
    selectHeight+=selectGap;
    //铃声下拉框
    updateClockSelectBtnLabel(clockBellLabel,selectHeight+labelHeightOffset,tr("  bell"));
    selectHeight+=selectGap;
    //稍后提醒下拉框
    updateClockSelectBtnLabel(clockRemindLabel,selectHeight+labelHeightOffset,tr("  remind"));
}

void Clock::updateClockEditPageBtn(int selectMoveWidth, int selectMoveHeight, int selectGap, int btnHeight)
{
    int localeNum = m_selectBtnUtil->getCurrentLocale();
    if(localeNum == 6||localeNum == 7||localeNum == 9){
        selectMoveWidth = selectMoveWidth-80;
    }
    int btnWidth = 256;
    //闹钟名称
    clockEditOnClockNew->resize(btnWidth,btnHeight);
    clockEditOnClockNew->move(selectMoveWidth,selectMoveHeight);
    selectMoveHeight+=selectGap;
    //重复下拉框
    dialog_repeat->resize(btnWidth,btnHeight);
    dialog_repeat->move(selectMoveWidth,selectMoveHeight);
    selectMoveHeight+=selectGap;
    //铃声下拉框
    dialog_music->resize(btnWidth,btnHeight);
    dialog_music->move(selectMoveWidth,selectMoveHeight);
    selectMoveHeight+=selectGap;
    //稍后提醒下拉框
    dialog_remind_late->resize(btnWidth,btnHeight);
    dialog_remind_late->move(selectMoveWidth,selectMoveHeight);

}
void Clock::widgetListWhiteStyle(QListWidget *listWidget)
{
    QString listBackColor = getDefaultGreyColor();
    QString selectItemRadius = QString::number(SELECT_ITEM_RADIUS);
    listWidget->setStyleSheet("QListWidget{background-color: rgba(0, 0, 0, 0);}\
                              QListWidget::Item{background-color:"+listBackColor+";border-radius:"+selectItemRadius+"px;}\
                              QListWidget::item::selected{background-color:rgba(233, 233, 233,255);border-radius:"+selectItemRadius+"px;border:1px solid rgba(131, 131, 131,0);}\
                              QListWidget::item:hover{background-color:rgba(233, 233, 233,255);border-radius:"+selectItemRadius+"px;}\
                              ");
}

void Clock::widgetListBlackStyle(QListWidget *listWidget)
{
    QString selectItemRadius = QString::number(SELECT_ITEM_RADIUS);
    listWidget->setStyleSheet("QListWidget{background-color: rgba(0, 0, 0, 0);}\
                              QListWidget::Item{background-color:rgba(41, 41, 41, 255);border-radius:"+selectItemRadius+"px;}\
                              QListWidget::item::selected{background-color:rgba(55, 55, 59, 255);border-radius:"+selectItemRadius+"px;border:1px solid rgba(131, 131, 131,0);}\
                              QListWidget::item:hover{background-color:rgba(55, 55, 59, 255);border-radius:"+selectItemRadius+"px;}\
                              ");
}
void Clock::watchTimeJump()
{
    QTime time = QTime::currentTime();
    int timeH = time.hour();
    int timeM = time.minute();
    int timeS = time.second();
    auto dateTimeSecs = QDateTime::currentSecsSinceEpoch();
    //    qDebug()<<"dbq-闹钟"<<timeH<<":"<<timeM<<":"<<timeS<<dateTimeSecs;
    int rowCount = watchTimeModel->rowCount();
    QString timeInfo =QString::number(dateTimeSecs);
    qint64 lastTimeInfo = 0;
    int index = 0;
    if(rowCount<1){
        //新建
        watchTimeModel->insertRow(index);
        watchTimeModel->setData(watchTimeModel->index(index, 0), Utils::getRandomId());
        watchTimeModel->setData(watchTimeModel->index(index, 1), timeInfo);
    }else{
        QString lastTimeInfoStr = watchTimeModel->index(index, 1).data().toString();
        lastTimeInfo = lastTimeInfoStr.toLong();
        //        qDebug()<<"dbq-lastTimeInfoStr"<<lastTimeInfoStr<<"lastTimeInfo"<<lastTimeInfo;
        int timeGap = dateTimeSecs-lastTimeInfo;
        if(timeGap>1){
            //            qDebug()<<"dbq-timeGap"<<timeGap;
            qDebug() << "###################testtest";
            emit timeJump(timeGap);
        }
        watchTimeModel->setData(watchTimeModel->index(index, 1), timeInfo);
    }
    watchTimeModel->submitAll();

}
/*
 * 动态监控闹钟与本地时间
 * Dynamic monitoring alarm clock and local time
 */
void Clock::timerUpdate()
{
    QTime time = QTime::currentTime();
    int timeH = time.hour();
    int timeM = time.minute();
    int timeS = time.second();

    watchTimeJump();

    model_setup->select();
    //更新闹钟列表项
    textTimerupdate();

    int rowNum = model->rowCount();
    bool hasNextAlarm = false;
    QString nextAlarmClock = "";
    //记录最近的闹钟时间差
    int spanTime = 0;
    //遍历闹钟
    for (int i = 0; i < rowNum; i++) {
        /*判断星期
         Judgment week*/
        QDateTime current_date_time = QDateTime::currentDateTime();
        //今天没提醒闹钟，则continue
        //缩写的本地化日期名称
        if (current_date_time.toString("ddd").compare("周一")==0 &&  model->index(i, 6).data().toInt() == 0)
            continue;
        if (current_date_time.toString("ddd").compare("周二")==0 &&  model->index(i, 7).data().toInt() == 0)
            continue;
        if (current_date_time.toString("ddd").compare("周三")==0 &&  model->index(i, 8).data().toInt() == 0)
            continue;
        if (current_date_time.toString("ddd").compare("周四")==0 &&  model->index(i, 9).data().toInt() == 0)
            continue;
        if (current_date_time.toString("ddd").compare("周五")==0 &&  model->index(i, 10).data().toInt() == 0)
            continue;
        if (current_date_time.toString("ddd").compare("周六")==0 &&  model->index(i, 11).data().toInt() == 0)
            continue;
        if (current_date_time.toString("ddd").compare("周日")==0 &&  model->index(i, 12).data().toInt() == 0)
            continue;

        /*判断开关
         Judgment switch*/
        //闹钟关闭 则 continue
        if ( model->index(i, 3).data().toInt() == 1)
            continue;
        //闹钟时间到
        int alarmH = model->index(i, 0).data().toInt();
        int alarmM = model->index(i, 1).data().toInt();
        if (timeH == alarmH
                && timeM == alarmM
                && timeS == 0) {
            double music_time = 30;
            if (model->index(i, 13).data().toString().compare(tr("2min"))==0) {
                music_time = 2*60;
            } else if (model->index(i, 13).data().toString().compare(tr("3min"))==0) {
                music_time = 3*60;
            } else if (model->index(i, 13).data().toString().compare(tr("4min"))==0) {
                music_time = 4*60;
            } else if (model->index(i, 13).data().toString().compare(tr("6min"))==0) {
                music_time = 6*60;
            } else {
                music_time = 60;
            }
            QString id = model->index(i, 14).data().toString();
            noticeDialogShow(music_time, i ,id);
            if(model->index(i, 5).data().toString().compare(tr("No repetition"))==0)
                offAlarm(i);
        }else{
            int tempSpan = (alarmH-timeH)*60+alarmM-timeM;
            if(tempSpan>0){
                hasNextAlarm = true;
                if(spanTime==0){
                    spanTime=tempSpan;
                    nextAlarmClock = changeNumToStrWithAm(alarmH)+TIME_SEPARATOR+changeNumToStr(alarmM);
                }else{
                    if(tempSpan<spanTime){
                        spanTime=tempSpan;
                        nextAlarmClock = changeNumToStrWithAm(alarmH)+TIME_SEPARATOR+changeNumToStr(alarmM);
                    }

                }
            }
        }
    }
    if(hasNextAlarm){
        m_trayIconTooltip=tr("recent alarm")+TIME_SEPARATOR_CN+nextAlarmClock;
    }else{
        m_trayIconTooltip="";
    }
    // 只有倒计时不在运行时才更新托盘tooltip为闹钟信息
    // 倒计时运行时由updateCountdownTime()控制tooltip
    bool countdownRunning = (countdown_timer && countdown_timer->isActive());
    if (!countdownRunning) {
        setDefaultTrayIconTooltip();
    }
    //触发一次paintEvent
    update();
    //更新闹钟页面子项字体
//    updateAlarmItemFront(CURRENT_FONT_SIZE);
    //刷新倒计时上 时间格式
    getCountdownOverTime();
    //刷新上下午滚轮
    timerAlarmStartUpdate();
}

/*
 * 通知弹窗
 * Notification Popup
 */
void Clock::noticeDialogShow(int close_time, int alarm_num, QString id)
{
    model_setup->select();
    //闹钟弹窗
    alarmNoticeDialog = new Natice_alarm(close_time,alarm_num,nullptr,id);
    alarmNoticeDialog->setObjectName("ukui-clock_Natice_alarm_alarmNoticeDialog");
    alarmNoticeDialog->playMusic();
    NoticeAlarmPoolUtil::getInstance()->
            inseartNoticesMap(alarmNoticeDialog->getNoticeId(),alarmNoticeDialog);
}


/**
 * @brief 重绘窗口，更新闹钟 Redraw window, update alarm clock
 */
void Clock::updateAlarmClock()
{
    int rowNum = model->rowCount();
    int hour_now;
    int min_now;
    if(rowNum){
        ui->noAlarm->hide();
        ui->noAlarmIcon->hide();
    }else{
        ui->noAlarm->setText(tr("No alarm"));
        utils->resetText(ui->noAlarm , 0);
        ui->noAlarm->show();
        ui->noAlarmIcon->show();

    }
    auto sqlQuery = clock_sql::getQSqlQuery();
    sqlQuery.exec("select * from clock order by Hour ASC,Minute ASC");
    int alarmNum = 0;
    qInfo() << "--------------------------------------------------------------------------11:" << updateNowMill();
    //实现id补偿，原表设计没有主键，为实现旧数据兼容
    //    QList<int> * needIdNumList = new QList<int>();
    //    QHash<int,int> * numToIndex =new QHash<int,int>();
    while (sqlQuery.next()) {
        //窗体
        aItem[alarmNum] =new QListWidgetItem;
        aItem[alarmNum]->setSizeHint(QSize(340,108));
        aItem[alarmNum]->setTextColor(QColor(255, 0, 0, 255));
        aItem[alarmNum]->setData(Qt::AccessibleTextRole, QString("ukui-clock_QListWidgetItem_aItem"));
        aItem[alarmNum]->setData(Qt::AccessibleDescriptionRole, QObject::tr("ukui-clock_QListWidgetItem_aItem"));
        ui->alarmListWidget->addItem(aItem[alarmNum]);
        //避免滚动条因占据item空间而使item发生变化
        ui->alarmListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        ui->alarmListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        ui->alarmListWidget->verticalScrollBar()->setHidden(true);
        connect(ui->alarmListWidget->verticalScrollBar(),&QScrollBar::rangeChanged,this, [=](){
            if (ui->alarmListWidget->verticalScrollBar()->maximum() == 0) {
                ui->alarmListWidget->verticalScrollBar()->setHidden(true);
            } else {
                ui->alarmListWidget->verticalScrollBar()->setHidden(false);
            }
        });
        //闹钟信息
        int value =sqlQuery.value(3).toInt();
        w1[alarmNum] = new item_new(ui->alarmListWidget,value);
        //信息与展示窗体绑定
        ui->alarmListWidget->setItemWidget(aItem[alarmNum],w1[alarmNum]);
        //绑定Id到闹钟子项
        qInfo() << "--------------------------------------------------------------------------121:" << updateNowMill();

        auto hisId =  sqlQuery.value(14).toString();
        w1[alarmNum]->setId(hisId);
        hour_now =sqlQuery.value( 0).toInt();
        min_now =sqlQuery.value( 1).toInt();
        int onOff = sqlQuery.value(3).toInt();
        qInfo() << "--------------------------------------------------------------------------122:" << updateNowMill();

        if(onOff==1){
            w1[alarmNum]->closeStyle(CURRENT_FONT_SIZE);
        }else{
            w1[alarmNum]->openStyle(CURRENT_FONT_SIZE);
        }
        qInfo() << "--------------------------------------------------------------------------13:" << updateNowMill();
        if (system_time_flag) {
            //24时制
            changeTimeNum(hour_now,min_now);//转换int为QString
            // Convert int to qstring
            w1[alarmNum]->alarmLabel0->setText(alarmHour_str+TIME_SEPARATOR+alarmMinute_str);
            QString selfFont = Utils::loadFontFamilyFromTTF();
            QFont f(selfFont);
            f.setPixelSize(38);
            w1[alarmNum]->alarmLabel0->setFont(f);

            w1[alarmNum]->alarmLabel1->hide();
        } else {
            //12时制
            if (hour_now >= 12) {
                w1[alarmNum]->alarmLabel1->setText(tr("PM"));
                if (hour_now == 12) {
                    w1[alarmNum]->alarmLabel0->setText(changeNumToStr(hour_now)+TIME_SEPARATOR+changeNumToStr(min_now));
                } else {
                    w1[alarmNum]->alarmLabel0->setText(changeNumToStr(hour_now-12)+TIME_SEPARATOR+changeNumToStr(min_now));
                }
            } else {
                w1[alarmNum]->alarmLabel1->setText(tr("AM"));
                if (hour_now == 0) {
                    w1[alarmNum]->alarmLabel0->setText(changeNumToStr(12)+TIME_SEPARATOR+changeNumToStr(min_now));
                } else {
                    w1[alarmNum]->alarmLabel0->setText(changeNumToStr(hour_now)+TIME_SEPARATOR+changeNumToStr(min_now));
                }
            }
        }
        qInfo() << "--------------------------------------------------------------------------14:" << updateNowMill();
        /*闹钟开关
         Alarm switch*/
        //onoroff
        if (sqlQuery.value( 3).toInt() == 1) {
            w1[alarmNum]->alarm_on_off0->setChecked(false);
        } else {
            w1[alarmNum]->alarm_on_off0->setChecked(true);
        }
        //Name 闹钟名
        QString clockName = sqlQuery.value( 13).toString();
        int sizeLimit = 8;
        if(clockName.length()>sizeLimit){
            w1[alarmNum]->alarmLabel_w0->setToolTip(clockName);
            //名字超长
            clockName = clockName.left(sizeLimit);
            clockName = clockName+"..";
        }
        w1[alarmNum]->alarmLabel_w0->setText(clockName);
        //repeat 重复
        QString werk_str = sqlQuery.value( 5).toString();
        QString werk_day ;
        int werk = 0;
        for (int i=0; i<7; i++) {
            //从周一到周日开始判断 为1
            if (sqlQuery.value( 6+i).toInt()) {
                //拼接werk_day
                if(i == 0){
                    werk_day = werk_day + tr("Mon")+"  ";
                }else if(i == 1){
                    werk_day = werk_day + tr("Tue")+"  ";
                }else if(i == 2){
                    werk_day = werk_day + tr("Wed")+"  ";
                }else if(i == 3){
                    werk_day = werk_day + tr("Thu")+"  ";
                }else if(i == 4){
                    werk_day = werk_day + tr("Fri")+"  ";
                }else if(i == 5){
                    werk_day = werk_day + tr("Sat")+"  ";
                }else if(i == 6){
                    werk_day = werk_day + tr("Sun")+"  ";
                }
            }else{
                //不是全为1
                werk = 1;
            }
        }
        qInfo() << "--------------------------------------------------------------------------15:" << updateNowMill();
        if(werk){
            w1[alarmNum]->alarmLabel_s0->setText(werk_day);
            w1[alarmNum]->alarmLabel_s0->setProperty("fullText", werk_day);
        }else {
            if(werk_str == tr("Every day") || werk_str == "每天"){
                w1[alarmNum]->alarmLabel_s0->setProperty("fullText", tr("Every day"));
                w1[alarmNum]->alarmLabel_s0->setText(tr("Every day"));
            }else if(werk_str == tr("Workingday") || werk_str == "工作日"){
                w1[alarmNum]->alarmLabel_s0->setProperty("fullText", tr("Workingday"));
                w1[alarmNum]->alarmLabel_s0->setText(tr("Workingday"));
            }else if(werk_str == tr("No repetition") || werk_str == "不重复"){
                w1[alarmNum]->alarmLabel_s0->setProperty("fullText", tr("No repetition"));
                w1[alarmNum]->alarmLabel_s0->setText(tr("No repetition"));
            }
        }
        updateLabelStrCnEn(w1[alarmNum]->alarmLabel_s0,20,25,40);
        //闹钟开关绑定
        connect(w1[alarmNum]->alarm_on_off0,SIGNAL(stateChanged(bool)),this, SLOT(OnOffAlarm()));
        alarmNum++;
        qInfo() << "--------------------------------------------------------------------------16:" << updateNowMill();

    }
    //    updateClockIdByNum(needIdNumList,numToIndex);

}

void Clock::timerAlarmStartUpdate()
{
    if(system_time_flag==1){
        timer_alarm_startAPM->hide();
    }else{
        timer_alarm_startAPM->show();
    }
}

/*
 * 绘制无闹钟提示
 * Draw no alarm prompt
 */
void Clock::drawNoAlarmPrompt()
{
    ui->noAlarmIcon->setPixmap(QPixmap(":/image/noClockWhite.png"));//.pixmap(164,194)
    ui->noAlarm->setAlignment(Qt::AlignHCenter);
}

/*
 * 修改时间单数 为两位数
 * Modify time singular to two digits
 */
void Clock::changeTimeNum(int alarmHour, int alarmMinute)
{
    if (alarmHour < 10) {
        QString hours_str = QString::number(alarmHour);
        alarmHour_str = "0"+hours_str;
    } else {
        alarmHour_str = QString::number(alarmHour);
    }
    if (alarmMinute < 10) {
        QString minute_str = QString::number(alarmMinute);
        alarmMinute_str = "0"+minute_str;
    } else {
        alarmMinute_str = QString::number(alarmMinute);
    }
}




/*
 * 新建闹钟按钮回调
 * New alarm button callback
 */
void Clock::setAlarmClock()
{
    onEditPage = false;
    //新建闹钟页
    ui->mainWidget->setCurrentIndex(3);
    //取消
    ui->cancelbtnOnEditAlarm->show();
    repeat_new_or_edit_flag = 0;

    model_setup->select();/*调用默认设置
                           Call default settings*/
    repeat_str_model = tr("Workingday");
    //数字盘时间设置为当前时间
    QTime time = QTime::currentTime();
    int timeH = time.hour();
    int timeM = time.minute();
    timer_alarm_start24->m_currentValue = timeH;
    timer_alarm_start60->m_currentValue = timeM;
    if(timer_alarm_start24->m_currentValue>=12){
        timer_alarm_startAPM->m_currentValue=2;
    }else{
        timer_alarm_startAPM->m_currentValue=1;
    }
    //工作日默认
    dialog_repeat->line_edit_->setText(repeat_str_model+tr("(default)"));
    //工作日设置的逻辑
    //存储重复选择的星期，默认清零
    for (int i = 0; i < 9; i++) {
        if(i<5){
            repeat_day[i] = 1;
        }else{
            repeat_day[i] = 0;
        }
    }
    remind_late_str_model= tr("none");
    //设置新建闹钟默认铃声
    QString defaultBellId = m_selectBtnUtil->getDefaultBellId();
    QString bellName = getBellNameById(defaultBellId);
    dialog_music->setCurrentText(bellName);
    updateMusicLineEditElidedText(dialog_music);
    setMusicSelectDialogListById(defaultBellId,dialog_music);
    //设置默认稍后提醒
    setRemindLateSelectDialogListByName(remind_late_str_model,dialog_remind_late);
    clock_name = tr(CLOCK_TITLE_NAME);
    clockEditOnClockNew->setText(clock_name);
}
/**
 * @brief 设置新建闹钟的铃声的默认值
 * @param name
 */
void Clock::setMusicSelectDialogListById(QString bellId, QComboBox *temp)
{
    int status = m_selectBtnUtil->getBellIndexById(bellId);
    temp->setCurrentIndex(status);
}

void Clock::setRemindLateSelectDialogListByName(QString name, QComboBox *temp)
{

    int status = getRemindStatusByName(name);
    temp->setCurrentIndex(status);
}

int Clock::getRemindStatusByName(QString name)
{
    int status = 0;
    if(name.compare("none") == 0 || name.compare("不需要") == 0){
        status = 0;
    }else if(name.compare("five mins late") == 0 || name.compare("5分钟后") == 0){
        status = 1;
    }else if(name.compare("ten mins late") == 0 || name.compare("10分钟后") == 0){
        status = 2;
    }else if(name.compare("twenty mins late") == 0 || name.compare("20分钟后") == 0){
        status = 3;
    }else if(name.compare("thirsty mins late") == 0 || name.compare("30分钟后") == 0){
        status = 4;
    }else if(name.compare("one hour late") == 0 || name.compare("1小时后") == 0){
        status = 5;
    }
    return status;
}

/*
 * 闹钟新建界面保存回调
 * Alarm new interface save callback
 */
void Clock::setAlarmSave()
{
    //12小时制时间转化
    if(system_time_flag==0){
        int apmindex = timer_alarm_startAPM->m_currentValue-1;
        int time_24 = timer_alarm_start24->m_currentValue;
        timer_alarm_start24->m_currentValue=time_24%12+apmindex*12;
    }
    if(onEditPage){
        //编辑页面保存
        QString clockName = clockEditOnClockNew->text();
        if(clockName.isEmpty()){
            delete_msg *deletemsg = new delete_msg();
            deletemsg->setObjectName("ukui-clock_delete_msg_deletemsg");
            deletemsg->ui->msgInfo->setText(tr("Please set alarm name!"));
            QPointF position = this->pos();
            deletemsg->move(position.x()+20,position.y()+203);
            deletemsg->exec();
            return ;
        }
        auto curentId = getClockPkByCurrentNum();
        auto sqlQuery = clock_sql::getQSqlQuery();
        sqlQuery.prepare("update clock set hour=:hour,minute=:minute,bell_id=:music,repeat=:repeat,"
                         "name=:name,monday=:monday,tuesday=:tuesday,wednesday=:wednesday,thursday=:thusday,"
                         "friday=:friday,saturday=:saturday,sunday=:sunday,remind_status=:remind_status,editstatus=:editstatus where id=:id");
        sqlQuery.bindValue(":id",curentId);
        sqlQuery.bindValue(":hour",timer_alarm_start24->m_currentValue);
        sqlQuery.bindValue(":minute",timer_alarm_start60->m_currentValue);
        QString bellId = getSelectBellId(dialog_music);
        sqlQuery.bindValue(":music",bellId);
        sqlQuery.bindValue(":repeat",repeat_str_model);
        sqlQuery.bindValue(":remind_status",remind_late_str_model);
        sqlQuery.bindValue(":name",clockName);
        sqlQuery.bindValue(":editstatus",true);
        for (int i=0; i<7; i++) {
            QString day = indexWeekdayMap->value(i);
            if (repeat_day[i]) {
                sqlQuery.bindValue(day,1);
            } else {
                sqlQuery.bindValue(day,0);
            }
        }
        sqlQuery.exec();
        model->submitAll();
        int m = model->rowCount();
        /*每次updateAlarmClock()之前，删除全部闹钟相关控件
          Delete all alarm related controls before updatealrmclock()*/
        for (int i = 0; i < m; i++) {
            delete aItem[i];
            delete w1[i];
        }
        updateAlarmClock();
        updateAlarmItemFront(CURRENT_FONT_SIZE);
        ui->mainWidget->setCurrentIndex(1);
        ui->mainWidget->raise();/*将页面放置最前方
                                    Put the page at the front*/

    }else{
        //新建页面保存
//        clockEditOnClockNew->setText(clockEditOnClockNew->text().remove(QRegExp("\\s")));//去除所以空格
        if(clockEditOnClockNew->text().isEmpty()){
            delete_msg *deletemsg = new delete_msg();
            deletemsg->setObjectName("ukui-clock_delete_msg_deletemsg");
            deletemsg->ui->msgInfo->setText(tr("Please set alarm name!"));
            QPointF position = this->pos();
            deletemsg->move(position.x()+20,position.y()+203);
            deletemsg->exec();
            return ;
        }
        int rowNum;
        rowNum = model->rowCount();

        qDebug() << rowNum << "闹钟数";
        if (rowNum < 20) {
            saveClockToDatabase(rowNum);
        } else {
            QMessageBox::warning(this, tr("warning"), tr("the number of alarms reaches limit!！"), tr("yes"));
        }

        ui->mainWidget->setCurrentIndex(1);
        ui->mainWidget->raise();//将页面放置最前方
        // Put the page at the front

    }
    updateAlarmItemFront(CURRENT_FONT_SIZE);
    playController::getInstance().stop();
}

void Clock::saveClockToDatabase(int rowNum)
{
    QString clockName = clockEditOnClockNew->text();
    model->insertRow(rowNum);

    model->setData(model->index(rowNum, 0), timer_alarm_start24->m_currentValue);
    model->setData(model->index(rowNum, 1), timer_alarm_start60->m_currentValue);
    QString bellId = getSelectBellId(dialog_music);
    model->setData(model->index(rowNum, 2), bellId);
    model->setData(model->index(rowNum, 15), remind_late_str_model);
    model->setData(model->index(rowNum, 3), int(0));
    model->setData(model->index(rowNum, 4), int(model->index(rowNum-1, 4).data().toInt()+1));
    model->setData(model->index(rowNum, 5), repeat_str_model);
    model->setData(model->index(rowNum, 13), clockName);
    model->setData(model->index(rowNum,16),true);
    //主键
    auto id = Utils::getRandomId();
    model->setData(model->index(rowNum, 14), id);
    for (int i=0; i<7; i++) {
        model->setData(model->index(rowNum, i+6), repeat_day[i]);
    }
    model->submitAll();

    for (int i=0; i<rowNum; i++) {
        delete aItem[i];
        delete w1[i];
    }
    qDebug() << model->index(rowNum, 0).data().toString()
             << model->index(rowNum, 1).data().toString()
             << QFileInfo( model->index(rowNum, 2).data().toString() ).fileName();
    updateAlarmClock();
}

void Clock::setTimerAlarmStart()
{
    int localNum = m_selectBtnUtil->getCurrentLocale();
    int ringMoveHeight=timer_alarm_start24->pos().y()+6;
    if(localNum==1){
        timer_alarm_startAPM->move(64,ringMoveHeight);
    }else{
        timer_alarm_startAPM->move(268,ringMoveHeight);
    }
    int apmIndex = timer_alarm_startAPM->m_currentValue-1;
    int hourNum = timer_alarm_start24->m_currentValue;
    timer_alarm_start24->m_currentValue = hourNum%12+apmIndex*12;
    timer_alarm_start24->homing();
}

QString Clock::getSelectBellId(QComboBox *temp)
{
    int num=temp->currentIndex();
    QString id = m_selectBtnUtil->getBellIdByIndex(num);
    return id;
}

QString Clock::addAlarm(clockInterface::RequestParams param)
{
    return addAlarm(param.clockName,param.hour,param.minute);
}

QString Clock::addAlarmJosn(QString param)
{
    // 打印原始参数，确认接收正确
    qDebug() << "原始参数 param:" <<  param;

    neb::CJsonObject oJson(param.toStdString());
    qDebug() << "原始参数 param:" <<  QString::fromStdString(oJson["hour"].ToString());

    // 校验字段是否存在
    if(!oJson.KeyExist("hour")){
        return formatReturnMsg(clockInterface::FAIL, "缺少 hour 字段");
    }
    if(!oJson.KeyExist("minute")){
        return formatReturnMsg(clockInterface::FAIL, "缺少 minute 字段");
    }

    // 解析字段（clockName 可选，不存在则为空）
    QString clockName = oJson.KeyExist("clockName") ? QString::fromStdString(oJson["clockName"].ToString()) : "";
    QString hourStr = QString::fromStdString(oJson["hour"].ToString());
    QString minuteStr = QString::fromStdString(oJson["minute"].ToString());

    // 解析稍后提醒字段（remindLater 可选，不存在则为"none"）
    QString remindLater = oJson.KeyExist("remindLater") ? QString::fromStdString(oJson["remindLater"].ToString()) : "none";

    // 解析重复提醒字段（repeat 可选，不存在则为"不重复"）
    QString repeat = oJson.KeyExist("repeat") ? QString::fromStdString(oJson["repeat"].ToString()) : "不重复";

    // 清理字符串：去除首尾空格 + 全角转半角 + 合并多余空格 + 去除首尾双引号（核心修复）
    auto cleanString = [](const QString& str) -> QString {
        QString result = str.trimmed(); // 去除首尾空白
        // 去除首尾的双引号（解决嵌套引号问题）
        if (result.startsWith("\"") && result.endsWith("\"")) {
            result = result.mid(1, result.length() - 2);
        }
        // 全角空格转半角
        result = result.replace(QChar(0x3000), QChar(' '));
        // 合并多个连续空格为一个
        result = result.replace(QRegExp("\\s+"), " ");
        return result;
    };

    // 对所有字符串字段统一清洗
    clockName = cleanString(clockName); // 补充：clockName 也建议清洗
    hourStr = cleanString(hourStr);
    minuteStr = cleanString(minuteStr);
    remindLater = cleanString(remindLater); // 关键：清洗后会去除多余引号
    repeat = cleanString(repeat); // 关键：清洗后会去除多余引号
    // 全角数字转半角（兼容所有 Qt 版本）
    auto fullWidthToHalfWidth = [](const QString& str) -> QString {
        QString result;
        for (const QChar& ch : str) {
            ushort unicode = ch.unicode();
            // 全角数字转半角
            if (unicode >= 0xFF10 && unicode <= 0xFF19) {
                result.append(QChar(unicode - 0xFEE0));
            } else {
                result.append(ch);
            }
        }
        return result;
    };
    hourStr = fullWidthToHalfWidth(hourStr);
    minuteStr = fullWidthToHalfWidth(minuteStr);

    // 打印解析后的字符串
    qDebug() << "解析并清理后的 hour 字符串:" << hourStr << "，minute 字符串:" << minuteStr
             << "，remindLater 字符串:" << remindLater << "，repeat 字符串:" << repeat;

    // 校验数值转换结果
    bool hourOk = false;
    int hour = hourStr.toInt(&hourOk);
    if(!hourOk || hour < 0 || hour > 23){
        std::string hourErrMsg = QString("小时格式错误（值：%1）").arg(hourStr).toStdString();
        return formatReturnMsg(clockInterface::FAIL, hourErrMsg);
    }

    bool minuteOk = false;
    int minute = minuteStr.toInt(&minuteOk);
    if(!minuteOk || minute < 0 || minute > 59){
        std::string minuteErrMsg = QString("分钟格式错误（值：%1）").arg(minuteStr).toStdString();
        return formatReturnMsg(clockInterface::FAIL, minuteErrMsg);
    }

    // 校验稍后提醒选项
    QStringList validRemindLaterOptions;
    validRemindLaterOptions << "无" << "5分钟后" << "10分钟后" << "20分钟后" << "30分钟后" << "1小时后"
                           << "none" << "five mins late" << "ten mins late" << "twenty mins late" << "thirty mins late" << "one hour late";

    // 匹配时忽略大小写（可选，增强兼容性）
    if (!validRemindLaterOptions.contains(remindLater, Qt::CaseInsensitive)) {
        std::string errMsg = QString("稍后提醒选项无效（值：%1，合法值：%2）")
                            .arg(remindLater)
                            .arg(validRemindLaterOptions.join(", "))
                            .toStdString();
        return formatReturnMsg(clockInterface::FAIL, errMsg);
    }

    // 校验重复提醒选项
    QStringList validRepeatOptions;
    validRepeatOptions << "不重复" << "工作日" << "每天" << "周一" << "周二" << "周三" << "周四" << "周五" << "周六" << "周日"
                      << "No repetition" << "Workingday" << "Every day" << "Mon" << "Tue" << "Wed" << "Thu" << "Fri" << "Sat" << "Sun";
    if (!validRepeatOptions.contains(repeat, Qt::CaseInsensitive)) {
        std::string errMsg = QString("重复提醒选项无效（值：%1）").arg(repeat).toStdString();
        return formatReturnMsg(clockInterface::FAIL, errMsg);
    }

    // 打印最终传递的数值
    qDebug() << "传给 addAlarm 的小时:" << hour << "，分钟:" << minute
             << "，稍后提醒:" << remindLater << "，重复提醒:" << repeat;

    // 调用扩展版本的 addAlarm 函数
    return addAlarm(clockName, hour, minute, remindLater, repeat);
}
QString Clock::updateClockByIdJosn(QString param)
{
    neb::CJsonObject oJson(param.toStdString());
    QString id = QString::fromStdString(oJson["id"].ToString());
    if(FieldValidUtil::isNull(id)){
        return formatReturnMsg(clockInterface::FAIL,"主键id为空");
    }
    id = FieldValidUtil::QStringFilter(id);
    QString clockName = QString::fromStdString(oJson["clockName"].ToString());
    QString hour = QString::fromStdString(oJson["hour"].ToString());
    QString minute = QString::fromStdString(oJson["minute"].ToString());
    if(FieldValidUtil::isNull(clockName)&&FieldValidUtil::isNull(hour)&&FieldValidUtil::isNull(minute)){
        return formatReturnMsg(clockInterface::FAIL,"没有需要修改的数据");
    }
    updateClockDatabase(id,clockName,hour,minute,0);
    return formatReturnMsg(clockInterface::SUCCESS,"闹钟修改成功");
}

QString Clock::selectClockByIdJosn(QString param)
{
    neb::CJsonObject oJson(param.toStdString());
    QString id = QString::fromStdString(oJson["id"].ToString());
    if(FieldValidUtil::isNull(id)){
        return formatReturnMsg(clockInterface::FAIL,"主键id为空");
    }
    id = FieldValidUtil::QStringFilter(id);
    auto sqlQuery = getClockByPK(id);
    QString queryId = sqlQuery.value(14).toString();
    if(FieldValidUtil::isNull(queryId)){
        return formatReturnMsg(clockInterface::FAIL,"查询结果为空");
    }
    oJson.Add("name",sqlQuery.value(13).toString().toStdString());
    oJson.Add("hour",sqlQuery.value( 0).toInt());
    oJson.Add("minute",sqlQuery.value( 1).toInt());
    return formatReturnMsg(oJson,clockInterface::SUCCESS,"闹钟查询成功");
}

QString Clock::deleteClockByIdJosn(QString param)
{
    neb::CJsonObject oJson(param.toStdString());
    QString id = QString::fromStdString(oJson["id"].ToString());
    if(FieldValidUtil::isNull(id)){
        return formatReturnMsg(clockInterface::FAIL,"主键id为空");
    }
    id = FieldValidUtil::QStringFilter(id);
    deleteAlarmDatabase(id);
    return formatReturnMsg(clockInterface::SUCCESS,"闹钟删除成功");
}

QString Clock::addAlarm(QString clockName, int hour, int minute)
{
    //表单验证
    if(FieldValidUtil::isNull(clockName)){
        return formatReturnMsg(clockInterface::FAIL,"闹钟名为空");
    }
    QString clockNameQstr = FieldValidUtil::QStringFilter(clockName);
    if(clockNameQstr.length()>16){
        return formatReturnMsg(clockInterface::FAIL,"闹钟名超过16个字");
    }
    if(!FieldValidUtil::isValueBetweenRange(hour,0,23)){
        return formatReturnMsg(clockInterface::FAIL,"小时位不是0-23之间的整数");
    }
    if(!FieldValidUtil::isValueBetweenRange(minute,0,59)){
        return formatReturnMsg(clockInterface::FAIL,"分钟位不是0-59之间的整数");
    }
    int rowNum;
    rowNum = model->rowCount();
    if(rowNum>=20){
        return formatReturnMsg(clockInterface::FAIL,"闹钟数已达上限");
    }

    //存储到数据库
    model->insertRow(rowNum);
    model->setData(model->index(rowNum, 0), hour);
    model->setData(model->index(rowNum, 1), minute);
    model->setData(model->index(rowNum, 2), tr("glass"));
    model->setData(model->index(rowNum, 3), int(0));
    model->setData(model->index(rowNum, 4), int(model->index(rowNum-1, 4).data().toInt()+1));
    model->setData(model->index(rowNum, 5), tr("No repetition"));
    model->setData(model->index(rowNum, 13), clockNameQstr);
    model->setData(model->index(rowNum, 15), tr("none")); // 稍后提醒默认为"无"
    //主键
    auto id = Utils::getRandomId();
    model->setData(model->index(rowNum, 14), id);
    for (int i=0; i<7; i++) {
        model->setData(model->index(rowNum, i+6), 1);
    }
    model->submitAll();
    for (int i=0; i<rowNum; i++) {
        delete aItem[i];
        delete w1[i];
    }
    updateAlarmClock();
    updateAlarmItemFront(CURRENT_FONT_SIZE);
    neb::CJsonObject oJson;
    oJson.Add("id",id.toStdString());
    return formatReturnMsg(oJson,clockInterface::SUCCESS,"闹钟添加成功");
}

QString Clock::addAlarm(QString clockName, int hour, int minute, QString remindLater, QString repeat)
{
    //表单验证
    if(FieldValidUtil::isNull(clockName)){
        return formatReturnMsg(clockInterface::FAIL,"闹钟名为空");
    }

    QString clockNameQstr = FieldValidUtil::QStringFilter(clockName);
    if(clockNameQstr.length()>16){
        return formatReturnMsg(clockInterface::FAIL,"闹钟名超过16个字");
    }
    if(!FieldValidUtil::isValueBetweenRange(hour,0,23)){
        return formatReturnMsg(clockInterface::FAIL,"小时位不是0-23之间的整数");
    }
    if(!FieldValidUtil::isValueBetweenRange(minute,0,59)){
        return formatReturnMsg(clockInterface::FAIL,"分钟位不是0-59之间的整数");
    }
    int rowNum;
    rowNum = model->rowCount();
    if(rowNum>=20){
        return formatReturnMsg(clockInterface::FAIL,"闹钟数已达上限");
    }

    //存储到数据库
    model->insertRow(rowNum);
    model->setData(model->index(rowNum, 0), hour);
    model->setData(model->index(rowNum, 1), minute);
    model->setData(model->index(rowNum, 2), "76727");
    model->setData(model->index(rowNum, 3), int(0));
    model->setData(model->index(rowNum, 4), int(model->index(rowNum-1, 4).data().toInt()+1));

    // 设置重复提醒
    QString repeatStr = repeat;
    if (repeat == "工作日" || repeat == "Workingday") {
        repeatStr = tr("Workingday");
    } else if (repeat == "不重复" || repeat == "No repetition") {
        repeatStr = tr("No repetition");
    } else if (repeat == "每天" || repeat == "Every day") {
        repeatStr = tr("Every day");
    } else if (repeat == "周一" || repeat == "Mon") {
        repeatStr = tr("Mon");
    } else if (repeat == "周二" || repeat == "Tue") {
        repeatStr = tr("Tue");
    } else if (repeat == "周三" || repeat == "Wed") {
        repeatStr = tr("Wed");
    } else if (repeat == "周四" || repeat == "Thu") {
        repeatStr = tr("Thu");
    } else if (repeat == "周五" || repeat == "Fri") {
        repeatStr = tr("Fri");
    } else if (repeat == "周六" || repeat == "Sat") {
        repeatStr = tr("Sat");
    } else if (repeat == "周日" || repeat == "Sun") {
        repeatStr = tr("Sun");
    }
    model->setData(model->index(rowNum, 5), repeatStr);
    
    model->setData(model->index(rowNum, 13), clockNameQstr);
    
    // 设置稍后提醒
    QString remindLaterStr = remindLater;
    if (remindLater == "无" || remindLater == "none") {
        remindLaterStr = tr("none");
    } else if (remindLater == "5分钟后" || remindLater == "five mins late") {
        remindLaterStr = tr("five mins late");
    } else if (remindLater == "10分钟后" || remindLater == "ten mins late") {
        remindLaterStr = tr("ten mins late");
    } else if (remindLater == "20分钟后" || remindLater == "twenty mins late") {
        remindLaterStr = tr("twenty mins late");
    } else if (remindLater == "30分钟后" || remindLater == "thirsty mins late") {
        remindLaterStr = tr("thirsty mins late");
    } else if (remindLater == "1小时后" || remindLater == "one hour late") {
        remindLaterStr = tr("one hour late");
    }
    model->setData(model->index(rowNum, 15), remindLaterStr);
    
    //主键
    auto id = Utils::getRandomId();
    model->setData(model->index(rowNum, 14), id);
    
    // 设置重复提醒的星期几标记
    // 默认情况下，如果是"每天"或"工作日"，设置相应的标记
    if (repeat == "每天" || repeat == "Every day") {
        // 每天都提醒
        for (int i=0; i<7; i++) {
            model->setData(model->index(rowNum, i+6), 1);
        }
    } else if (repeat == "工作日" || repeat == "Workingday") {
        // 工作日提醒（周一到周五）
        for (int i=0; i<7; i++) {
            if (i < 5) {
                model->setData(model->index(rowNum, i+6), 1);
            } else {
                model->setData(model->index(rowNum, i+6), 0);
            }
        }
    } else if (repeat == "不重复" || repeat == "No repetition") {
        // 不重复，每天都标记为1（这是默认行为）
        for (int i=0; i<7; i++) {
            model->setData(model->index(rowNum, i+6), 1);
        }
    } else {
        // 特定星期几
        for (int i=0; i<7; i++) {
            if ((repeat == "周一" || repeat == "Mon") && i == 0) {
                model->setData(model->index(rowNum, i+6), 1);
            } else if ((repeat == "周二" || repeat == "Tue") && i == 1) {
                model->setData(model->index(rowNum, i+6), 1);
            } else if ((repeat == "周三" || repeat == "Wed") && i == 2) {
                model->setData(model->index(rowNum, i+6), 1);
            } else if ((repeat == "周四" || repeat == "Thu") && i == 3) {
                model->setData(model->index(rowNum, i+6), 1);
            } else if ((repeat == "周五" || repeat == "Fri") && i == 4) {
                model->setData(model->index(rowNum, i+6), 1);
            } else if ((repeat == "周六" || repeat == "Sat") && i == 5) {
                model->setData(model->index(rowNum, i+6), 1);
            } else if ((repeat == "周日" || repeat == "Sun") && i == 6) {
                model->setData(model->index(rowNum, i+6), 1);
            } else {
                model->setData(model->index(rowNum, i+6), 0);
            }
        }
    }
    
    model->submitAll();
    for (int i=0; i<rowNum; i++) {
        delete aItem[i];
        delete w1[i];
    }
    updateAlarmClock();
    updateAlarmItemFront(CURRENT_FONT_SIZE);
    neb::CJsonObject oJson;
    oJson.Add("id",id.toStdString());
    return formatReturnMsg(oJson,clockInterface::SUCCESS,"闹钟添加成功");
}

QString Clock::formatReturnMsg(neb::CJsonObject oJson, clockInterface::STATUS_CODE status, std::string msg)
{
    oJson.Add("status",status);
    oJson.Add("msg",msg);
    return QString::fromStdString(oJson.ToString());
}
/**
 * @brief 构造返回值
 * @param status 状态
 * @param msg 返回信息
 * @return
 */
QString Clock::formatReturnMsg(STATUS_CODE status, string msg)
{

    neb::CJsonObject oJson;
    oJson.Add("status",status);
    oJson.Add("msg",msg);
    return QString::fromStdString(oJson.ToString());
}





/*
 *闹钟新建界面取消回调
 * Cancel callback in alarm new interface
 */
void Clock::alarmCancelSave()
{
    ui->mainWidget->setCurrentIndex(1);
    ui->mainWidget->raise();
    playController::getInstance().stop();
}

/*
 * 双击闹钟打开重编辑页面
 * Double click the alarm clock to open the re edit page
 */
void Clock::listdoubleClickslot()
{
    onEditPage = true;
    ui->mainWidget->setCurrentIndex(3);
    //以链表序号找寻数据
    auto curentId = getClockPkByCurrentNum();
    //根据id查询数据
    auto sqlQuery = getClockByPK(curentId);
    timer_alarm_start24->m_currentValue=sqlQuery.value( 0).toInt();
    timer_alarm_start60->m_currentValue=sqlQuery.value( 1).toInt();
    if(timer_alarm_start24->m_currentValue>=12){
        timer_alarm_startAPM->m_currentValue=2;
    }else{
        timer_alarm_startAPM->m_currentValue=1;
    }
    repeat_new_or_edit_flag = 1;
    QString werk_day ;
    int werk = 0;
    for (int i=0; i<7; i++) {
        if (sqlQuery.value( 6+i).toInt()) {
            if(i == 0){
                werk_day = werk_day + tr("Mon")+"  ";
            }else if(i == 1){
                werk_day = werk_day + tr("Tue")+"  ";
            }else if(i == 2){
                werk_day = werk_day + tr("Wed")+"  ";
            }else if(i == 3){
                werk_day = werk_day + tr("Thu")+"  ";
            }else if(i == 4){
                werk_day = werk_day + tr("Fri")+"  ";
            }else if(i == 5){
                werk_day = werk_day + tr("Sat")+"  ";
            }else if(i == 6){
                werk_day = werk_day + tr("Sun")+"  ";
            }
        }else{
            werk = 1;
        }
    }
    if(werk){
        dialog_repeat->line_edit_->setText(werk_day);
    }else {
        dialog_repeat->line_edit_->setText(tr("Every day"));
    }
    if(sqlQuery.value( 5).toString() == tr("  work") || sqlQuery.value( 5).toString() == tr("  工作日")){
        dialog_repeat->line_edit_->setText(tr("  work"));
    }
    if(sqlQuery.value( 5).toString() == tr("No repetition") || sqlQuery.value( 5).toString() == tr("不重复")){
        dialog_repeat->line_edit_->setText(tr("No repetition"));
    }
    repeat_str_model = sqlQuery.value( 5).toString();

    updateRepeatLineEditStr(dialog_repeat->line_edit_);

    QString bellId =  sqlQuery.value( 2).toString();
    auto name = m_selectBtnUtil->getBellNameById(bellId);
    dialog_music->setCurrentText(name);
    setMusicSelectDialogListById(bellId,dialog_music);
    updateMusicLineEditElidedText(dialog_music);

    //重复
    remind_late_str_model =  sqlQuery.value( 15).toString();
    qDebug()<<"从数据库查稍后"<<remind_late_str_model;
    int status = getRemindStatusByName(remind_late_str_model);
    remind_late_str_model = getRemindLateStrFromNum(status);
    setRemindLateSelectDialogListByName(remind_late_str_model,dialog_remind_late);

    clock_name = sqlQuery.value( 13).toString();
    clockEditOnClockNew->setText(clock_name);
    for (int i=0; i<7; i++) {
        if (sqlQuery.value( 6+i).toInt()) {
            repeat_day[i] = 1;
        } else {
            repeat_day[i] = 0;
        }
    }
}

QString Clock::getClockPkByCurrentNum()
{
    int num = ui->alarmListWidget->currentRow();
    auto widgetItem = static_cast<item_new *>(ui->alarmListWidget->itemWidget(aItem[num]));
    auto curentId = widgetItem->id();
    return curentId;
}

QSqlQuery Clock::getClockByPK(QString id)
{
    auto sqlQuery = clock_sql::getQSqlQuery();
    sqlQuery.prepare("select * from clock where id=:id");
    sqlQuery.bindValue(":id",id);
    sqlQuery.exec();
    sqlQuery.next();
    return sqlQuery;
}



/*
 * 闹钟重编辑页面删除闹钟回调
 * Alarm re edit page delete alarm callback
 */
void Clock::deleteAlarm()
{
    int rowNum = model->rowCount();
    int ret;
    if(deleteAlarmBox == nullptr){
        deleteAlarmBox = new QMessageBox(this);
        deleteAlarmBox->setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    }
    deleteAlarmBox->setIcon(QMessageBox::Question);
    deleteAlarmBox->setWindowTitle(QObject::tr("Hint"));
    deleteAlarmBox->setText(QObject::tr("Are you sure to delete？"));
    // 替换自定义按钮为标准按钮的文本设置，避免addButton导致的索引问题
    deleteAlarmBox->button(QMessageBox::Ok)->setText(QObject::tr("sure"));
    deleteAlarmBox->button(QMessageBox::Cancel)->setText(QObject::tr("cancel"));

#ifdef NOTKAIMING
    if(GlobalInstance::getInstance()->session_desktop() == GlobalInstance::KYLIN_WLCOM){
        UkuiWindowHelper *windowHelper = new UkuiWindowHelper(deleteAlarmBox);
        windowHelper->removeTitleBar();
    } else {
        kdk::UkuiStyleHelper::self()->removeHeader(deleteAlarmBox);
    }
#else
    kdk::UkuiStyleHelper::self()->removeHeader(deleteAlarmBox);
#endif
    ret = deleteAlarmBox->exec();
    
    // 检查对话框返回值：使用标准枚举值判断，而非自定义按钮索引
    if (ret == QMessageBox::Cancel) { // 取消按钮
        qInfo()<<"dbq-不删除";
    } else if(ret == QMessageBox::Ok) { // 确认按钮
        //删除数据
        auto curentId = getClockPkByCurrentNum();
        deleteAlarmDatabase(curentId);
        rowNum = model->rowCount();
        qDebug() << rowNum;

    }
    //闹钟页面子项，即列表内容高度 ≤ 列表可视区域高度，隐藏滚动条
    updateAlarmItemFront(CURRENT_FONT_SIZE);
    if (ui->alarmListWidget->verticalScrollBar()->maximum() == 0) {
        ui->alarmListWidget->verticalScrollBar()->setHidden(true);
    } else {
        ui->alarmListWidget->verticalScrollBar()->setHidden(false);
    }
}

void Clock::deleteAlarmDatabase(QString id)
{
    int rowNum = model->rowCount();
    auto sqlQuery = clock_sql::getQSqlQuery();
    sqlQuery.prepare("delete from clock where id=:id");
    sqlQuery.bindValue(":id",id);
    sqlQuery.exec();
    for (int i=0; i<rowNum; i++) {
        delete w1[i];
        delete aItem[i];
    }
    last = getNowMill();
    model->submitAll();
    qInfo() << "--------------------------------------------------------------------------1:" << updateNowMill();
    updateAlarmClock();
    qInfo() << "---------------------------------------------------------------------------2:" << updateNowMill();

    updateAlarmItemFront(CURRENT_FONT_SIZE);
    qInfo() << "---------------------------------------------------------------------------3:" << updateNowMill();

}

void Clock::updateClockDatabase(QString id, QString name, QString hour, QString minute, int onOff)
{
    QString sql = "update clock set ";
    if(FieldValidUtil::isNotNull(name)){
        sql = sql.append(" Name=:clockName , ");
    }
    if(FieldValidUtil::isNotNull(hour)){
        sql = sql.append(" Hour=:hour , ");
    }
    if(FieldValidUtil::isNotNull(minute)){
        sql = sql.append(" Minute=:minute , ");
    }
    sql = sql.append(" on_or_off=:onOff , ");
    sql = sql.remove(sql.length()-2,2);
    sql = sql.append(" where Id=:id");
    int rowNum = model->rowCount();
    auto sqlQuery = clock_sql::getQSqlQuery();
    sqlQuery.prepare(sql);
    sqlQuery.bindValue(":id",id);
    //绑定数据
    if(FieldValidUtil::isNotNull(name)){
        name = FieldValidUtil::QStringFilter(name);
        sqlQuery.bindValue(":clockName",name);
    }
    if(FieldValidUtil::isNotNull(hour)){
        sqlQuery.bindValue(":hour",hour.toInt());
    }
    if(FieldValidUtil::isNotNull(minute)){
        sqlQuery.bindValue(":minute",minute.toInt());
    }
    sqlQuery.bindValue(":onOff",onOff);
    sqlQuery.exec();
    model->submitAll();
    if(FieldValidUtil::isNotNull(name)||FieldValidUtil::isNotNull(hour)||FieldValidUtil::isNotNull(minute)){
        for (int i=0; i<rowNum; i++) {
            delete w1[i];
            delete aItem[i];
        }
        updateAlarmClock();
        updateAlarmItemFront(CURRENT_FONT_SIZE);
    }
}



/*
 * 闹钟开关
 * Alarm switch
 */
void Clock::OnOffAlarm()
{
    int i=0 ;
    KSwitchButton *btn = qobject_cast<KSwitchButton*>(QObject::sender());
    while (btn != w1[i]->alarm_on_off0) {
        i++;
    }
    QString id = w1[i]->id();
    auto sqlQuery  = getClockByPK(id);
    if (sqlQuery.value(3).toInt() == 0) {
        w1[i]->closeStyle(CURRENT_FONT_SIZE);
        updateClockDatabase(id,nullptr,nullptr,nullptr,1);
    } else {
        w1[i]->openStyle(CURRENT_FONT_SIZE);
        updateClockDatabase(id,nullptr,nullptr,nullptr,0);
    }
}
/*
 * 不重复时单独关闭闹钟
 * Turn off the alarm separately if it is not repeated
 */
void Clock::offAlarm(int i)
{
    w1[i]->alarm_on_off0->setChecked(false);
    model->setData(model->index(i, 3), int(1));
    model->submitAll();

    int rowNum = model->rowCount();
    for (int i = 0; i < rowNum; i++) {
        delete aItem[i];
        delete w1[i];
    }
    updateAlarmClock();
}

/*
 * 倒计时音乐选择
 * Countdown music selection
 */
void Clock::countdownMusicSellect()
{
    refreshMusicSelectList(count_music_sellect);
    updateMusicLineEditElidedText(count_music_sellect);
}

/*
 * 闹钟初始化工作日选择界面绘制回调
 * Alarm clock initialization workday selection interface drawing callback
 */
void Clock::alarmRepeat()
{
    int num;
    //repeat_new_or_edit_flag 编辑是1
    if(repeat_new_or_edit_flag)
        num = ui->alarmListWidget->currentRow();
    else {
        num= model->rowCount();
    }
    dialog_repeat->btn[0]->textLabel->setText(tr("No repetition"));
    dialog_repeat->btn[1]->textLabel->setText(tr("Workingday"));
    dialog_repeat->btn[2]->textLabel->setText(tr("Mon"));
    dialog_repeat->btn[3]->textLabel->setText(tr("Tue"));
    dialog_repeat->btn[4]->textLabel->setText(tr("Wed"));
    dialog_repeat->btn[5]->textLabel->setText(tr("Thu"));
    dialog_repeat->btn[6]->textLabel->setText(tr("Fri"));
    dialog_repeat->btn[7]->textLabel->setText(tr("Sat"));
    dialog_repeat->btn[8]->textLabel->setText(tr("Sun"));
    connect(dialog_repeat,&CustomComBox::comboxPopup,this,[=](){
        /*切记：更新pixmap时需要同步更新btn的值
        repeat_day[0]	周一(Mon)	btn[2]
        repeat_day[1]	周二(Tue)	btn[3]
        repeat_day[2]	周三(Wed)	btn[4]
        repeat_day[3]	周四(Thu)	btn[5]
        repeat_day[4]	周五(Fri)	btn[6]
        repeat_day[5]	周六(Sat)	btn[7]
        repeat_day[6]	周日(Sun)	btn[8]*/
        // 获取line_edit_的文本并判断是否为“无重复”
        if (dialog_repeat->line_edit_->text() == tr("No repetition")) {
            // 1. 选中“不重复”按钮（btn[0]）
            dialog_repeat->btn[0]->iconLabel->setPixmap(repeat_on_Pixmap);
            // 2. 取消“工作日”按钮（btn[1]）选中
            dialog_repeat->btn[1]->iconLabel->setPixmap(repeat_off_Pixmap);
            // 3. 取消7天日期按钮（btn[2-8]）选中
            for (int i = 0; i < 7; i++) {
                int btn_index = i + 2; // 日期按钮从索引2开始
                repeat_day[i] = 0;
                dialog_repeat->btn[btn_index]->iconLabel->setPixmap(repeat_off_Pixmap);
            }
        } else {
            // 非“无重复”时：取消“不重复”按钮选中，按repeat_day更新7天日期按钮
            dialog_repeat->btn[0]->iconLabel->setPixmap(repeat_off_Pixmap);
            for (int i = 0; i < 7; i++) {
                int btn_index = i + 2;
                // 按repeat_day数组更新日期按钮图标
                if (repeat_day[i]) {
                    dialog_repeat->btn[btn_index]->iconLabel->setPixmap(repeat_on_Pixmap);
                } else {
                    dialog_repeat->btn[btn_index]->iconLabel->setPixmap(repeat_off_Pixmap);
                }
            }
        }
    });
    connect(dialog_repeat->list_widget_,&CustomListWidget::getPosition,this,[=](QPointF position){
        int yy = position.y();
        int xx=position.x();
        int honum=0;
        if(comboxclickedflag){
            honum = dialog_repeat->list_widget_->currentRow();
        }else{
            honum=dialog_repeat->list_widget_->row(dialog_repeat->list_widget_->itemAt(xx,yy));
            dialog_repeat->setCurrentHoverNum(honum);
        }
        dialog_repeat->setTextHoverStyle(honum);
    });
}

/*
 * 倒计时音乐选择单机回调
 * Countdown music selection single callback
 */
void Clock::countMusicListclickslot()
{
    int num=count_music_sellect->currentIndex();
    int size=count_music_sellect->count();
    //最后一个自定义铃声
    if(num==(size-1)){
        addDivBell(count_music_sellect,Clock::count_down);
    }else{
        QString id = m_selectBtnUtil->getBellIdByIndex(num);
        QString path = m_selectBtnUtil->getBellPathById(id);
        QString name = getBellNameById(id);
        //排除无
        if(num==0){
            path="";
        }
        playMusicFromPath(path);
        // 更新缩略显示
        updateMusicLineEditElidedText(count_music_sellect);
        model_setup->setData(model_setup->index(0, 1), id);
        model_setup->submitAll();
    }
}

void Clock::playMusicFromPath(QString path)
{
    if(m_muteOn){
        playController::getInstance().playSingleSong(path,0);
        playController::getInstance().pause();
    }else{
        playController::getInstance().playSingleSong(path,0);
    }
    musicStartTime = QTime::currentTime();
}

void Clock::stopHisPlay()
{
    if(hisPlayer!=nullptr){
        hisPlayer->stop();
    }
}
//TODO 加按钮
void Clock::addDivBell(QComboBox *temp, btnType type)
{
    QString filePath = m_selectBtnUtil->openAudioFileDialog(temp);
    int size=temp->count();
    if(FieldValidUtil::isNotNull(filePath)){
        // 获取目标路径
        QString targetPath = m_selectBtnUtil->getTargetPath(filePath);

        // 检查目标路径的文件是否已存在
        QString existBellId = m_selectBtnUtil->checkBellExists(targetPath);
        if (!existBellId.isEmpty()) {
            // 目标文件已存在，跳转到对应的铃声
            int index = m_selectBtnUtil->getBellIndexById(existBellId);
            temp->setCurrentIndex(index);
            updateMusicLineEditElidedText(dialog_music);
            updateMusicLineEditElidedText(count_music_sellect);
            playMusicFromPath(targetPath);
            return;
        }

        //复制文件
        QString toPatn = m_selectBtnUtil->copyAudioFile(filePath);
        if(!toPatn.isEmpty()){
            //存储到数据库
            QString bellId = m_selectBtnUtil->saveToBellTable(toPatn);
            QString name = m_selectBtnUtil->getBellNameById(bellId);

            count_music_sellect->insertItem(size-1,name);
            dialog_music->insertItem(size-1,name);
            temp->setCurrentIndex(size-1);
            // 更新省略显示
            updateMusicLineEditElidedText(dialog_music);
            updateMusicLineEditElidedText(count_music_sellect);

            playMusicFromPath(toPatn);
        }

    }
}

void Clock::statCountdownMsec()
{
    qint64 now = getNowMill();
    countdown_msec -= (now - m_countdownLastTime);
    m_countdownLastTime = now;

    if(countdown_msec<=0){
        //减1算法
        if(countdown_second>0){
            countdown_second--;
        }else if(countdown_second==0&&countdown_minute>=1){
            countdown_minute--;
            countdown_second=59;
        }else if(countdown_second==0&&countdown_minute==0&&countdown_hour>=1){
            countdown_hour--;
            countdown_minute=59;
            countdown_second=59;
        }
        countdown_msec += 1000;
    }
    updateCountdownTime();

}

void Clock::updateCountdownTime()
{
    //设置当前时间
    int ringmax = countdown_hour*3600 + countdown_minute*60 + countdown_second;
    ui->countdownRunPage->countdownRunRoundBar->setValue(ringmax);

    QString h; QString m; QString s;
    if (countdown_hour < 10){
        QString hours_str = QString::number(countdown_hour);
        h = "0"+hours_str;
    } else {
        h = QString::number(countdown_hour);
    }
    if (countdown_minute < 10) {
        QString minute_str = QString::number(countdown_minute);
        m = "0"+minute_str;
    } else {
        m = QString::number(countdown_minute);
    }

    if (countdown_second < 10) {
        QString second_str = QString::number(countdown_second);
        s = "0"+second_str;
    }else {
        s = QString::number(countdown_second);
    }
    ui->remainTime->setText(h+TIME_SEPARATOR+m+TIME_SEPARATOR+s);
    //    QString selfFont = Utils::loadFontFamilyFromTTF();
    //    QFont f(selfFont);
    QFont f;
    f.setFamily("NotoSansCJKsc-Regular");
    f.setWeight(400);
    f.setPixelSize(40);
    ui->remainTime->setFont(f);
    //设置倒计时托盘tooltip
    QString tooltip = tr("Count down");
    tooltip +=TIME_SEPARATOR_CN+h+TIME_SEPARATOR+m+TIME_SEPARATOR+s;
    updateTrayIconTooltip(tooltip);

    //小型倒计时 显示倒计时时间
    if(this->m_selectTinyCountdown){
        if(checkTinyCountdownDia()){
            tinycountdownDia->updateTimeInfo(h+TIME_SEPARATOR+m+TIME_SEPARATOR+s);
        }
    }
    //    qDebug()<<"dbq-倒计时"<<countdown_hour<<":"<<countdown_minute<<":"<<countdown_second;
    //时间归零
    if (countdown_hour==0 && countdown_minute==0 && (countdown_second)==0) {
        startbtnCountdown();
        countdown_timer->stop();
        //托盘恢复默认
        setDefaultTrayIconTooltip();
        //倒计时通知弹窗
        countdownNoticeDialogShow();
    }
}

/*
 * 倒计时通知弹窗
 * Countdown notification pop-up
 */
void Clock::countdownNoticeDialogShow()
{
    //关闭tiny倒计时
    if(checkTinyCountdownDia()){
        tinycountdownDia->set_dialog_close();
    }
    model_setup->select();
    countdownNoticeDialog->playMusic();
    NoticeAlarmPoolUtil::getInstance()->
            inseartNoticesMap(countdownNoticeDialog->getNoticeId(),countdownNoticeDialog);
}



/*
 * 倒计时开始-结束
 * Countdown start end callback
 */
void Clock::startbtnCountdown(){
    if (!countdown_isStarted) {
        playController::getInstance().stop();

        //点击了开始
        if (hourTimerRing->m_currentValue==0 && minuteTimeRing->m_currentValue==0 && secondTimeRing->m_currentValue==0) {
            return;
        }
        //当前剩余时间，单位秒
        int ringmax = hourTimerRing->m_currentValue*3600 + minuteTimeRing->m_currentValue*60 + secondTimeRing->m_currentValue;
        //进度圈
        ui->countdownRunPage->countdownRunRoundBar->setRange(0,ringmax);
        //切换进度条颜色
        ui->countdownRunPage->countdownRunRoundBar->switchRunRingColor();
        ui->startCountdownBtn->setStyleSheet("");
        countdown_isStarted=1;
        //结束
        ui->startCountdownBtn->setText(tr("End"));
        //点击开始，刷新数值
        refreshCountdownLabel11Flag = true;
        //设置倒计时初始时间label9 中间数字
        setcoutdownNumber(hourTimerRing->m_currentValue, minuteTimeRing->m_currentValue, secondTimeRing->m_currentValue);//获取转轮当前值
        //倒计时页面
        ui->countdownStackedW->setCurrentIndex(1);
        countdown_timer->start();
        m_countdownLastTime = getNowMill();
        //光圈的进度值修改定时启动

    } else {
        //点击了结束，或者时间耗尽
        ui->countdownRunPage->countdownRunRoundBar->setRange(0,COUNTDOWN_TIME);
        ui->countdownRunPage->countdownRunRoundBar->setValue(COUNTDOWN_TIME);/*初始化倒计时进度圈
                                               Initialize countdown progress circle*/
        countdown_timer->stop();
        countdown_isStarted=0;
        countdown_isStarted_2=0;
        hourTimerRing->m_currentValue = 0;
        minuteTimeRing->m_currentValue = 0;
        secondTimeRing->m_currentValue = 0;
        ui->startCountdownBtn->setText(tr("start"));
        ui->remainTime->setText("00:00:00");
        //切换到倒计时滚轮页
        ui->countdownStackedW->setCurrentIndex(0);
        //暂停
        ui->suspendCountdownBtn->setText(Utils::getOmitStr(tr("suspend"),7));

        QPalette palette = ui->suspendCountdownBtn->palette();
        QColor ColorPlaceholderText(248,163,76,255);
        QBrush brush2;
        brush2.setColor(ColorPlaceholderText);
        //主题框架1.0.6-5kylin2
        palette.setColor(QPalette::Highlight,QColor(248,163,76,255));
        palette.setColor(QPalette::Button,QColor(248,163,76,255));
        palette.setBrush(QPalette::ButtonText, QBrush(Qt::white));
        ui->suspendCountdownBtn->setPalette(palette);
        //小窗体清零，隐藏
        if(checkTinyCountdownDia()){
            tinycountdownDia->clearData();
        }
        //恢复默认托盘
        setDefaultTrayIconTooltip();
    }
    return;
}

void Clock::tinyCountdownFinish()
{
    tinycountdownDia->set_dialog_close();
    activeWindow();
    startbtnCountdown();
}

/*
 * 设置倒计时初始时间
 * Set the initial countdown time
 */
void Clock::setcoutdownNumber(int h1, int m1, int s1){
    countdown_hour=h1; countdown_minute=m1 ; countdown_second=s1;

    QString h; QString m; QString s;

    if (countdown_hour < 10){
        QString hours_str = QString::number(countdown_hour);
        h = "0"+hours_str;
    } else {
        h = QString::number(countdown_hour);
    }

    if (countdown_minute < 10) {
        QString minute_str = QString::number(countdown_minute);
        m = "0"+minute_str;
    } else {
        m = QString::number(countdown_minute);
    }

    if (countdown_second < 10) {
        QString second_str = QString::number(countdown_second);
        s = "0"+second_str;
    } else {
        s = QString::number(countdown_second);
    }

    ui->remainTime->setText(h+TIME_SEPARATOR+m+TIME_SEPARATOR+s);
    //    QString selfFont = Utils::loadFontFamilyFromTTF();
    //    QFont f(selfFont);
    QFont f;
    f.setFamily("NotoSansCJKsc-Regular");
    f.setWeight(400);
    f.setPixelSize(40);
    ui->remainTime->setFont(f);
    //获取倒计时结束时间
    getCountdownOverTime();
}

/*
 * 倒计时5分钟
 * Countdown 5 minutes set callback
 */
void Clock::onMin_5btnClicked()
{
    hourTimerRing->m_currentValue = 0;
    minuteTimeRing->m_currentValue = 5;
    secondTimeRing->m_currentValue = 0;
    setcoutdownNumber(0, 5, 0);
}

/*
 * 获取倒计时结束时间
 * Get countdown end time
 */
void Clock::getCountdownOverTime()
{
    QTime currentTime = QTime::currentTime();

    // 仅在首次触发时计算【固定的目标时间】（倒计时结束的时间），后续不再变动
    if (refreshCountdownLabel11Flag) {
        // 核心：计算目标时间 = 当前时间 + 倒计时时长（时/分/秒）
        targetTime = currentTime.addSecs(countdown_hour * 3600 + countdown_minute * 60 + countdown_second);
        refreshCountdownLabel11Flag = false;
    }

    // 提取目标时间的小时和分钟（这就是要显示的“到时时间”）
    int targetH = targetTime.hour();
    int targetM = targetTime.minute();

    // 计算当前时间到目标时间的总秒数差（用于判断是否为明天到时）
    int secsDiff = currentTime.secsTo(targetTime);
    // 处理跨天的负数差值（比如当前23:59，目标00:01，secsTo会返回-86398，需转为正数）
    if (secsDiff < 0) {
        secsDiff += 86400 * 2; // 最多补2天，覆盖“后天”的判断
    }

    // 根据时间差判断显示文案
    QString timeText;
    if (secsDiff >= 24 * 3600) { // 目标时间在24后，文案显示明天
        timeText = tr("Tomorrow") + formatX_h(targetH) + TIME_SEPARATOR + changeNumToStr(targetM);
    } else { // 目标时间在24小时内
        timeText = formatX_h(targetH) + TIME_SEPARATOR + changeNumToStr(targetM);
    }

    // 更新UI
    ui->countdownAlarmTime->setText(timeText);
}
//上下午格式
QString Clock::get12hourStr(int x_h)
{
    QString str;
    //12时
    if (x_h >= 12) {
        x_h = x_h - 12;
        str=tr("PM")+" "+changeNumToStr(x_h);
    } else {
        str=tr("AM")+" "+changeNumToStr(x_h);
    }
    return str;
}
/**
 * @brief 创建用户手册dbus客户端
 */
void Clock::createUserGuideDebusClient()
{
    // 用户手册
    QString serviceName = "com.kylinUserGuide.hotel"
            + QString("%1%2").arg("_").arg(QString::number(getuid()));
    //创建dbus-client
    userGuideInterface = new QDBusInterface(serviceName,
                                            "/",
                                            "com.guide.hotel",
                                            QDBusConnection::sessionBus());
    qDebug() << "connect to kylinUserGuide" << userGuideInterface->isValid();
    if (!userGuideInterface->isValid()) {
        qDebug() << "fail to connect to kylinUserGuide";
        qDebug() << qPrintable(QDBusConnection::sessionBus().lastError().message());
        return;
    }
}

void Clock::onTinyClicked()
{
    this->m_selectTinyCountdown=true;
    //主窗体最小化
    this->hide();                   //迷你小窗与主页面仅保留一个，弹出主界面后隐藏倒计时小窗
    enableTrayIcon();               //#279573 【闹钟】倒计时界面mini弹窗，任务栏缩略图会显示两个闹钟弹窗
    tinycountdownDia->showThisWindow();
    primaryManager->moveUnderMultiScreen(PrimaryManager::UP_RIGHT,tinycountdownDia,1);
}

void Clock::activeWindow()
{
    if(isHidden()){
        this->show();
    }
    if(primaryManager->checkWayland()){
        kdk::WindowManager::activateWindow(m_pid);
    }else{
        setWindowState(Qt::WindowActive);
    }
}
void Clock::showThisWindow()
{
    kdk::UkuiStyleHelper::self()->removeHeader(this);
    this->show();
    QRect rect = utils->centerToScreenQRect(this);
    qWarning()<<"dbq-showThisWindow"<<rect;
    kdk::WindowManager::setGeometry(this->windowHandle(),rect);
}

void Clock::selectRemindLate()
{
    dialog_remind_late->addItem(tr("none"));
    dialog_remind_late->addItem(tr("five mins late"));
    dialog_remind_late->addItem(tr("ten mins late"));
    dialog_remind_late->addItem(tr("twenty mins late"));
    dialog_remind_late->addItem(tr("thirsty mins late"));
    dialog_remind_late->addItem(tr("one hour late"));
}

//刷新时间格式
QString Clock::formatX_h(int x_h)
{
    QString str;
    //跟随系统时间
    if(m_timeZone == "24"){
        str=changeNumToStr(x_h);
    }else{
        //12时
        str =get12hourStr(x_h);
    }
    return str;
}
/*
 * 单位变双位
 * Integer to character
 */
QString Clock::changeNumToStr(int alarmHour)
{
    QString str;
    if (alarmHour < 10) {
        QString hours_str = QString::number(alarmHour);
        str = "0"+hours_str;
    } else {
        str = QString::number(alarmHour);
    }
    return str;
}
QString Clock::changeNumToStrWithAm(int alarmHour)
{
    QString str;
    if (system_time_flag) {
        //24时制
        str = changeNumToStr(alarmHour);//转换int为QString
    } else {
        //12时制
        if (alarmHour >= 12) {
            str+=tr("PM")+" ";
            if (alarmHour == 12) {
                str+=changeNumToStr(alarmHour);
            } else {
                str+=changeNumToStr(alarmHour-12);
            }
        } else {
            str+=tr("AM")+" ";
            if (alarmHour == 0) {
                str+=changeNumToStr(12);
            } else {
                str+=changeNumToStr(alarmHour);
            }
        }
    }
    return str;
}
/*
 * 倒计时-暂停继续
 * Countdown - pause resume callback
 */
void Clock::onCountPushClicked()
{
    tinycountdownDia->updateOnRunState(countdown_isStarted_2);
    if (countdown_isStarted_2){
        //点击了继续
        ui->suspendCountdownBtn->setText(Utils::getOmitStr(tr("suspend"),7));
        //切换进度条颜色
        ui->countdownRunPage->countdownRunRoundBar->switchRunRingColor();

        QPalette palette = ui->suspendCountdownBtn->palette();
        QColor ColorPlaceholderText(248,163,76,255);
        QBrush brush2;
        brush2.setColor(ColorPlaceholderText);
        //主题框架1.0.6-5kylin2
        palette.setColor(QPalette::Highlight,QColor(248,163,76,255));
        palette.setColor(QPalette::Button,QColor(248,163,76,255));
        palette.setBrush(QPalette::ButtonText, QBrush(Qt::white));
        ui->suspendCountdownBtn->setPalette(palette);

        countdown_timer->start();
        m_countdownLastTime = getNowMill();
        countdown_isStarted_2=0;
        refreshCountdownLabel11Flag = true;
        getCountdownOverTime();
    } else {
        //点击了暂停
        ui->suspendCountdownBtn->setText(tr("continue"));
        //切换进度条颜色
        ui->countdownRunPage->countdownRunRoundBar->switchStopRingColor();

        QPalette palette = ui->suspendCountdownBtn->palette();
        QColor ColorPlaceholderText(SWITCH_BTN_HIGHLIGHT_BACK_COLOR);
        QBrush brush2;
        brush2.setColor(ColorPlaceholderText);
        //主题框架1.0.6-5kylin2
        palette.setColor(QPalette::Highlight,QColor(69, 173, 110,255));
        palette.setColor(QPalette::Button,QColor(69, 173, 110,255));
        palette.setBrush(QPalette::ButtonText, QBrush(Qt::white));
        ui->suspendCountdownBtn->setPalette(palette);

        countdown_timer->stop();
        countdown_isStarted_2=1;
    }

    return;
}

/*
 * 倒计时初始数字转盘
 * Countdown initial digital dial
 */
void Clock::countdownSetStartTime()
{

    //倒计时小时滚轮
    hourTimerRing = new VerticalScroll_99(ui->countdownSetPage);
    hourTimerRing->setObjectName("ukui-clock_VerticalScroll_99_hourTimerRing");
    QLabel * h_in_m = new QLabel (ui->countdownSetPage);
    h_in_m->setAccessibleName("ukui-clock_QLabel_h_in_m");
    //倒计时分钟
    minuteTimeRing = new VerticalScroll_60(ui->countdownSetPage);
    minuteTimeRing->setObjectName("ukui-clock_VerticalScroll_60_minuteTimeRing");
    QLabel * m_in_s = new QLabel (ui->countdownSetPage);
    m_in_s->setObjectName("ukui-clock_QLabel_m_in_s");
    //倒计时秒
    secondTimeRing = new VerticalScroll_60(ui->countdownSetPage);
    secondTimeRing->setObjectName("ukui-clock_VerticalScroll_60_secondTimeRing");
    QLabel * after_s = new QLabel (ui->countdownSetPage);
    after_s->setObjectName("ukui-clock_QLabel_after_s");

    hourTimerRing->resize(60,228);
    minuteTimeRing->resize(60,228);
    secondTimeRing->resize(60,228);


    h_in_m->resize(50,30);
    h_in_m->setText(tr("hour"));
    //    h_in_m->setStyleSheet("font: 30pt 'Sans Serif';");
    m_in_s->resize(50,30);
    m_in_s->setText(tr("min"));
    //    m_in_s->setStyleSheet("font: 30pt 'Sans Serif';");
    after_s->resize(50,30);
    after_s->setText(tr("sec"));
    //    after_s->setStyleSheet("font: 30pt 'Sans Serif';");
    int ringMoveWidth = 67;
    int ringMoveHeight = 30;
    int timeSep = 100;
    int labelSep = 55;
    int timeLabelMoveHeight = 130;
    hourTimerRing->move(ringMoveWidth, ringMoveHeight);
    h_in_m->move(ringMoveWidth+labelSep,timeLabelMoveHeight);
    //hour 要比min长
    if(Utils::checkLocalUs()){
        ringMoveWidth+=timeSep+4;
    }else{
        ringMoveWidth+=timeSep;
    }
    minuteTimeRing->move(ringMoveWidth, ringMoveHeight);
    m_in_s->move(ringMoveWidth+labelSep,timeLabelMoveHeight);
    ringMoveWidth+=timeSep;
    secondTimeRing->move(ringMoveWidth, ringMoveHeight);
    after_s->move(ringMoveWidth+labelSep,timeLabelMoveHeight);
    updateLabelStrCnEn(h_in_m,4,4,4);
    updateLabelStrCnEn(m_in_s,4,4,4);
    //配置倒计时页的开始按钮
    startCountSingle = new QPushButton(ui->countdownSetPage);
    startCountSingle->setObjectName("ukui-clock_QPushButton_startCountSingle");
    startCountSingle->resize(120,34);
    startCountSingle->move(135,401);
    startCountSingle->setText(tr("start"));
    connect(startCountSingle, SIGNAL(clicked()), this, SLOT(startbtnCountdown()) );

    QPalette palette2 = startCountSingle->palette();
    QColor ColorPlaceholderText2(233,233,233,255);
    QBrush brush2;
    brush2.setColor(ColorPlaceholderText2);
    palette2.setColor(QPalette::Button,QColor(233,233,233,255));
    //palette2.setBrush(QPalette::ButtonText, QBrush(Qt::white));
    startCountSingle->setPalette(palette2);

    QTimer *timer_count_start;
    timer_count_start = new QTimer();
    timer_count_start->setObjectName("ukui-clock_QTimer_timer_count_start");
    connect(timer_count_start, SIGNAL(timeout()), this, SLOT(countStatBtnGray()));
    timer_count_start->setInterval(100);
    timer_count_start->start();

    connect(hourTimerRing,&VerticalScroll_99::BaseVerticalScroll::srcollDoubleClicked,this,[=](){
        hourTimerRing->timeEdit_99->show();
        hourTimerRing->timeEdit_99->setFocus();
    });
    connect(hourTimerRing->timeEdit_99,&QLineEdit::editingFinished,this,[=](){
        int timevalue = hourTimerRing->timeEdit_99->text().toInt();
        hourTimerRing->m_currentValue = timevalue;
        hourTimerRing->homing();
        hourTimerRing->timeEdit_99->hide();
    });
    connect(minuteTimeRing,&VerticalScroll_99::BaseVerticalScroll::srcollDoubleClicked,this,[=](){
        minuteTimeRing->timeEdit_60->show();
        minuteTimeRing->timeEdit_60->setFocus();
    });
    connect(minuteTimeRing->timeEdit_60,&QLineEdit::editingFinished,this,[=](){
        int timevalue = minuteTimeRing->timeEdit_60->text().toInt();
        minuteTimeRing->m_currentValue = timevalue;
        minuteTimeRing->homing();
        minuteTimeRing->timeEdit_60->hide();
    });
    connect(secondTimeRing,&VerticalScroll_99::BaseVerticalScroll::srcollDoubleClicked,this,[=](){
        secondTimeRing->timeEdit_60->show();
        secondTimeRing->timeEdit_60->setFocus();
    });
    connect(secondTimeRing->timeEdit_60,&QLineEdit::editingFinished,this,[=](){
        int timevalue = secondTimeRing->timeEdit_60->text().toInt();
        secondTimeRing->m_currentValue = timevalue;
        secondTimeRing->homing();
        secondTimeRing->timeEdit_60->hide();
    });
}

/*
 * 倒计时页开始键致灰
 *  Start button of countdown page grays out
 */
void Clock::countStatBtnGray()
{
    if(hourTimerRing->m_currentValue==0 && minuteTimeRing->m_currentValue==0 && secondTimeRing->m_currentValue==0) {
        startCountSingle->setEnabled(false);
        QStyleOption opt;
        opt.init(this);

        if(theme::themetype==0){
            QPalette palette2 = startCountSingle->palette();
            QColor ColorPlaceholderText2(233,233,233,255);
            QBrush brush2;
            brush2.setColor(ColorPlaceholderText2);
            palette2.setColor(QPalette::Button,QColor(233,233,233,255));
            palette2.setBrush(QPalette::ButtonText, QBrush(QColor(158,158,158,255)));
            startCountSingle->setPalette(palette2);
        }else{
            QPalette palette2 = startCountSingle->palette();
            QColor ColorPlaceholderText2(45,45,48,255);
            QBrush brush2;
            brush2.setColor(ColorPlaceholderText2);
            palette2.setColor(QPalette::Button,QColor(45,45,48,255));
            palette2.setBrush(QPalette::ButtonText, QBrush(QColor(58,58,58,255)));
            startCountSingle->setPalette(palette2);
        }
    }else{
        startCountSingle->setEnabled(true);
        QPalette palette2 = startCountSingle->palette();
        QColor ColorPlaceholderText2(SWITCH_BTN_HIGHLIGHT_BACK_COLOR);
        QBrush brush2;
        brush2.setColor(ColorPlaceholderText2);
        palette2.setColor(QPalette::Button,QColor(SWITCH_BTN_HIGHLIGHT_BACK_COLOR));
        palette2.setBrush(QPalette::ButtonText, QBrush(Qt::white));
        startCountSingle->setPalette(palette2);
    }
}


void Clock::updateLabelFront(QLabel *label, int size)
{
    int finalSize = size < 14 ? 14  : size;
    QString styleSheet = "font-size:";
    styleSheet.append(QString::number(finalSize)).append("px;");
    label->setStyleSheet(styleSheet);
}

void Clock::updateQLineEditFront(QLineEdit *lineEdit, int size)
{
    QFont font ;
    font.setPixelSize(size);
    lineEdit->setFont(font);
}
/**
 * @brief 放大闹钟子项字体
 */
void Clock::updateAlarmItemFront(int size)
{
    QListWidget * alarmList = ui->alarmListWidget;
    for (int i=0; i<alarmList->count() && i<20; i++) {  // 添加边界检查
        QListWidgetItem * varItem =  alarmList->item(i);
        if (!varItem) continue;  // 添加空指针检查
        
        item_new * varWidget = static_cast<item_new*>(alarmList->itemWidget(varItem));
        if (!varWidget) continue;  // 添加空指针检查
        
        varWidget->commonStyle(size);
        
        // 字体大小发生变化时，需要更新闹钟列表中"重复项"的展示文本
        if (w1[i]) {  // 添加空指针检查
            updateLabelTextByPixel(w1[i]->alarmLabel_s0,300);
        }
    }
}

void Clock::updateStopwatchItemFront(int size)
{
    QListWidget * stopwatchList = ui->timeListWidget;
    for (int i=0;i<stopwatchList->count() ;i++ ) {
        QListWidgetItem * varItem =  stopwatchList->item(i);
        stopwatch_item * varWidget = static_cast<stopwatch_item*>(stopwatchList->itemWidget(varItem)) ;

        // 计算stopwatch1、stopwatch2 的目标字体大小
        int targetSize = round(1.1 * size);
        // 判断是否小于14px，取较大值
        int finalSize = (targetSize < 14) ? 14 : targetSize;
        // 应用最终字体大小
        updateLabelFront(varWidget->stopwatch1, finalSize);
        updateLabelFront(varWidget->stopwatch2,finalSize);
        updateLabelFront(varWidget->stopwatch3,round(2.2*size));
    }
}
void Clock::updateFront(const int size)
{

    //闹钟页面子项
    updateAlarmItemFront(size);
    //秒表页面 两个时间
    updateLabelFront(ui->timeShowBig,round(3.2*size));
    updateLabelFront(ui->timeShowSmall,round(1.73*size));
    //秒表页面子项
    updateStopwatchItemFront(size);
    double font = 1.15;
    double lineEditFont = 1.3;
    //闹钟编辑页面
    //重复
    updateLabelFront(clockRepeatLabel,round(font*size));
    updateQLineEditFront(dialog_repeat->line_edit_,round(lineEditFont*size));
    //提醒铃声
    updateLabelFront(clockBellLabel,round(font*size));
    updateMusicLineEditElidedText(dialog_music);
    // 倒计时铃声设置字体和缩略显示
    if (count_music_sellect && count_music_sellect->lineEdit()) {
        QFont musicFont;
        musicFont.setPixelSize(round(lineEditFont*size));
        count_music_sellect->lineEdit()->setFont(musicFont);
    }
    updateMusicLineEditElidedText(count_music_sellect);
    tinycountdownDia->updateTimeLabelElided();
    //稍后提醒
    updateLabelFront(clockRemindLabel,round(font*size));
    //倒计时 set 铃声
    updateLabelFront(countdownBellLabelOnSet,round(font*size));
    //倒计时 run 铃声
    updateLabelFront(countdownBellLabelOnRun,round(font*size));
    //闹钟名
    updateLabelFront(clockNamelabel,round(font*size));
    //闹钟编辑
    updateQLineEditFront(clockEditOnClockNew,round(lineEditFont*size));

    // 拼接样式字符串，将传入的size参数作为字体大小
    QString styleSheet = QString("font-size: %1px;").arg(font*size);
    // 应用样式到控件,无闹钟提示文字大小一起跟随设置变化
    ui->noAlarm->setStyleSheet(styleSheet);
}
void Clock::updateRepeatStr(QLabel *label)
{
    if(Utils::checkLocalChina()){
        updateLabelTextByLength(label,15);
    }
    if(Utils::checkLocalUs()){
        updateLabelTextByLength(label,25);
    }
    if(Utils::checkLocalBo()){
        updateLabelTextByLength(label,23);
    }
}

void Clock::updateLabelStrCnEn(QLabel *label, int cnLimit, int enLimit,int boLimit)
{
    if(Utils::checkLocalChina()){
        updateLabelTextByLength(label,cnLimit);
    }
    else if(Utils::checkLocalUs()){
        updateLabelTextByLength(label,enLimit);
    }
    else{
        updateLabelTextByLength(label,boLimit);
    }
}

void Clock::updateLabelTextByPixel(QLabel *label, int pixelWidthLimit)
{
    // 1. 安全校验：防止传入空指针导致程序崩溃
    if (label == nullptr) {
        qWarning() << "updateLabelTextByPixel: 传入的QLabel指针为空！";
        return;
    }

    // 2. 获取QLabel的核心信息：原有文本 + 实际使用的字体
    QString originalText = label->property("fullText").toString();          // 获取Label当前的文本
    QFont labelFont = label->font();               // 获取Label实际生效的字体（包含字号、字体族等）
    if(originalText == ""){
        return;
    }
    // 3. 调用工具函数截断文本（基于像素宽度和字体）
    QString truncatedText = Utils::getOmitStr(originalText, labelFont, pixelWidthLimit);

    // 4. 将截断后的文本设置回QLabel
    label->setText(truncatedText);

    // 可选：打印日志，便于调试
    qInfo() << QString("Label文本更新完成：原文本=[%1]，截断后=[%2]，限制宽度=[%3像素]")
              .arg(originalText)
              .arg(truncatedText)
              .arg(pixelWidthLimit);
}

void Clock::updateLabelTextByLength(QLabel *label, int limitSize)
{
    QString str = label->text().trimmed();
    if(str.length()>limitSize){
        label->setToolTip(str);
        str = Utils::getOmitStr(str,limitSize);
    }
    label->setText(str);
}

void Clock::updateRepeatLineEditStr(QLineEdit *temp)
{
    if(Utils::checkLocalChina()){
        updateLineEditTextByLength(temp,15);
    }
    if(Utils::checkLocalUs()){
        updateLineEditTextByLength(temp,25);
    }
}

void Clock::updateLineEditStrCnEn(QLineEdit *temp, int cnLimit, int enLimit)
{
    if(Utils::checkLocalChina()){
        updateLineEditTextByLength(temp,cnLimit);
    }
    if(Utils::checkLocalUs()){
        updateLineEditTextByLength(temp,enLimit);
    }
}

void Clock::updateLineEditTextByLength(QLineEdit *temp, int limitSize)
{
    QString str = temp->text();
    if(str.length()>limitSize){
        temp->setToolTip(str);
        str = Utils::getOmitStr(str,limitSize);
    }
    temp->setText(str);
}

/*
 * 闹钟初始化数字转盘
 * Alarm clock initialization digital turntable drawing
 */
void Clock::alarmSetStartTime()
{
    shadow = new QWidget(ui->editAlarmPage);
//    shadow->setObjectName("ukui-clock_QWidget_shadow");
    shadow->setAccessibleName("ukui-clock_QWidget_shadow");
    shadow->move(115,58);
    shadow->resize(160,58);
    //闹钟小时滚轮
    timer_alarm_start24 = new VerticalScroll_24(ui->editAlarmPage, this);
    timer_alarm_start24->setObjectName("ukui-clock_VerticalScroll_24_timer_alarm_start24");
    QLabel * h_in_m = new QLabel (ui->editAlarmPage);
    h_in_m->setObjectName("ukui-clock_QLabel_h_in_m");
    //闹钟分钟滚轮
    timer_alarm_start60 = new VerticalScroll_60(ui->editAlarmPage);
    timer_alarm_start60->setObjectName("ukui-clock_VerticalScroll_60_timer_alarm_start60");
    //闹中上下午滚轮
    timer_alarm_startAPM = new VerticalScroll_APM(ui->editAlarmPage);
    timer_alarm_startAPM->setObjectName("ukui-clock_VerticalScroll_APM_timer_alarm_startAPM");

    timer_alarm_start24->resize(63,228);
    timer_alarm_start60->resize(63,228);
    timer_alarm_startAPM->resize(50,228);

    QLabel * hour_ring = new QLabel (ui->editAlarmPage);
    hour_ring->setObjectName("ukui-clock_QLabel_hour_ring");
    QLabel * min_ring = new QLabel (ui->editAlarmPage);
    min_ring->setObjectName("ukui-clock_QLabel_min_ring");

    hour_ring->hide();
    min_ring->hide();

    hour_ring->setAlignment(Qt::AlignHCenter);
    min_ring->setAlignment(Qt::AlignHCenter);

    h_in_m->resize(10,40);
    h_in_m->setText(TIME_SEPARATOR);
    h_in_m->setStyleSheet("font: 25pt;");

    hour_ring->resize(50,30);
    hour_ring->setText(tr("hour"));
    hour_ring->setStyleSheet("font: 13pt ;color: rgb(148, 148, 148);");
    min_ring->resize(50,30);
    min_ring->setText(tr("min"));
    min_ring->setStyleSheet("font: 13pt ;color: rgb(148, 148, 148);");
    int ringMoveHeight = -8;
    timer_alarm_start24->move(116, ringMoveHeight);
    hour_ring->move(140,7);
    h_in_m->move(189,82);
    timer_alarm_start60->move(206, ringMoveHeight);
    min_ring->move(212,7);

    //上下午滚轮初始化
    setTimerAlarmStart();

    connect(timer_alarm_start24,&VerticalScroll_24::BaseVerticalScroll::srcollDoubleClicked,this,[=](){
        timer_alarm_start24->timeEdit_24->show();
        timer_alarm_start24->timeEdit_24->setFocus();
    });
    connect(timer_alarm_start24->timeEdit_24,&QLineEdit::editingFinished,this,[=](){
        int timevalue = timer_alarm_start24->timeEdit_24->text().toInt();
        timer_alarm_start24->m_currentValue = timevalue;
        timer_alarm_start24->homing();
        timer_alarm_start24->timeEdit_24->hide();
    });
    connect(timer_alarm_start60,&VerticalScroll_99::BaseVerticalScroll::srcollDoubleClicked,this,[=](){
        timer_alarm_start60->timeEdit_60->show();
        timer_alarm_start60->timeEdit_60->setFocus();
    });
    connect(timer_alarm_start60->timeEdit_60,&QLineEdit::editingFinished,this,[=](){
        int timevalue = timer_alarm_start60->timeEdit_60->text().toInt();
        timer_alarm_start60->m_currentValue = timevalue;
        timer_alarm_start60->homing();
        timer_alarm_start60->timeEdit_60->hide();
    });
}

/*
 * 重复选项单击回调
 * Repeat option click callback
 */
void Clock::repeatListclickslot(int num)
{
    dialog_repeat->comboxClicked=true;
    dialog_repeat->setCurrentHoverNum(num);
    comboxclickedflag = true;
    QString day[7] ;
    day[0]= tr("Mon")+"  ";
    day[1] = tr("Tue")+"  ";
    day[2] = tr("Wed")+"  ";
    day[3] = tr("Thu")+"  ";
    day[4] = tr("Fri")+"  ";
    day[5] = tr("Sat")+"  ";
    day[6] = tr("Sun")+"  ";
    //存储"重复"数据
    switch (num)
    {
    case 0:
        dialog_repeat->line_edit_->setText(tr("No repetition"));
        repeat_str_model = tr("No repetition");
        //不重复
        for (int i=0; i<7; i++) {
            // 周一到周日 全存1
            repeat_day[i] = 1;
            qDebug() << repeat_day[i];
        }

        return;
        break;
    case 1:
        dialog_repeat->line_edit_->setText(tr("Workingday"));
        repeat_str_model = tr("Workingday");
        for (int i=0; i<7; i++) {
            if(i<5) {
                repeat_day[i] = 1;
                dialog_repeat->btn[i+2]->iconLabel->setPixmap(repeat_on_Pixmap);
            } else {
                repeat_day[i] = 0;
                dialog_repeat->btn[i+2]->iconLabel->setPixmap(repeat_off_Pixmap);
            }
        }
        dialog_repeat->btn[7]->iconLabel->setPixmap(repeat_off_Pixmap);
        dialog_repeat->btn[8]->iconLabel->setPixmap(repeat_off_Pixmap);
        return;
        break;
    case 2:
        if (repeat_day[0] == 0 ) {
            repeat_day[0] = 1;
            dialog_repeat->btn[2]->iconLabel->setPixmap(repeat_on_Pixmap);
        } else {
            if(repeat_day[1]== 0 && repeat_day[2]== 0 && repeat_day[3]== 0 && repeat_day[4]== 0 && repeat_day[5]== 0 && repeat_day[6]== 0){
                //防止全部勾选被取消
            }else{
                repeat_day[0] = 0;
                dialog_repeat->btn[2]->iconLabel->setPixmap(repeat_off_Pixmap);
            }
        }
        //    dialog_repeat->showPopup();
        break;
    case 3:
        if(repeat_day[1] == 0 ) {
            repeat_day[1] = 1;
            dialog_repeat->btn[3]->iconLabel->setPixmap(repeat_on_Pixmap);
        } else {
            if(repeat_day[0]== 0 && repeat_day[2]== 0 && repeat_day[3]== 0 && repeat_day[4]== 0 && repeat_day[5]== 0 && repeat_day[6]== 0){

            }else{
                repeat_day[1] = 0;
                dialog_repeat->btn[3]->iconLabel->setPixmap(repeat_off_Pixmap);
            }
        }
        //     dialog_repeat->showPopup();
        break;
    case 4:
        if (repeat_day[2] == 0 ) {
            repeat_day[2] = 1;
            dialog_repeat->btn[4]->iconLabel->setPixmap(repeat_on_Pixmap);
        } else {
            if(repeat_day[1]== 0 && repeat_day[0]== 0 && repeat_day[3]== 0 && repeat_day[4]== 0 && repeat_day[5]== 0 && repeat_day[6]== 0){

            }else{
                repeat_day[2] = 0;
                dialog_repeat->btn[4]->iconLabel->setPixmap(repeat_off_Pixmap);
            }
        }
        //     dialog_repeat->showPopup();
        break;
    case 5:
        if (repeat_day[3] == 0 ) {
            repeat_day[3] = 1;
            dialog_repeat->btn[5]->iconLabel->setPixmap(repeat_on_Pixmap);
        } else {
            if(repeat_day[1]== 0 && repeat_day[2]== 0 && repeat_day[0]== 0 && repeat_day[4]== 0 && repeat_day[5]== 0 && repeat_day[6]== 0){

            }else{
                repeat_day[3] = 0;
                dialog_repeat->btn[5]->iconLabel->setPixmap(repeat_off_Pixmap);
            }
        }
        //    dialog_repeat->showPopup();
        break;
    case 6:
        if (repeat_day[4] == 0 ) {
            repeat_day[4] = 1;
            dialog_repeat->btn[6]->iconLabel->setPixmap(repeat_on_Pixmap);
        } else {
            if(repeat_day[1]== 0 && repeat_day[2]== 0 && repeat_day[3]== 0 && repeat_day[0]== 0 && repeat_day[5]== 0 && repeat_day[6]== 0){

            }else{
                repeat_day[4] = 0;
                dialog_repeat->btn[6]->iconLabel->setPixmap(repeat_off_Pixmap);
            }
        }
        //    dialog_repeat->showPopup();
        break;
    case 7:
        if (repeat_day[5] == 0 ) {
            repeat_day[5] = 1;
            dialog_repeat->btn[7]->iconLabel->setPixmap(repeat_on_Pixmap);
        } else {
            if(repeat_day[1]== 0 && repeat_day[2]== 0 && repeat_day[3]== 0 && repeat_day[4]== 0 && repeat_day[0]== 0 && repeat_day[6]== 0){

            }else{
                repeat_day[5] = 0;
                dialog_repeat->btn[7]->iconLabel->setPixmap(repeat_off_Pixmap);
            }
        }
        //     dialog_repeat->showPopup();
        break;
    case 8:
        if (repeat_day[6] == 0 ) {
            repeat_day[6] = 1;
            dialog_repeat->btn[8]->iconLabel->setPixmap(repeat_on_Pixmap);
        } else {
            if(repeat_day[1]== 0 && repeat_day[2]== 0 && repeat_day[3]== 0 && repeat_day[4]== 0 && repeat_day[5]== 0 && repeat_day[0]== 0){

            }else{
                repeat_day[6] = 0;
                dialog_repeat->btn[8]->iconLabel->setPixmap(repeat_off_Pixmap);
            }
        }
        //      dialog_repeat->showPopup();
        break;
    default:
        break;
    }
    //构造重复repeat_str
    for (int i=0; i<7; i++) {
        if (repeat_day[i]) {
            repeat_str=repeat_str+day[i];
        }
    }
    if(repeat_day[0]&&repeat_day[1]&&repeat_day[2]&&repeat_day[3]&&repeat_day[4]&&repeat_day[5]&&repeat_day[6])
        repeat_str = tr("Every day");
    dialog_repeat->line_edit_->setText(repeat_str);
    repeat_str_model = repeat_str;
    repeat_str="";
    updateRepeatLineEditStr(dialog_repeat->line_edit_);
}

/*
 * 闹钟初始化音乐选择界面回调
 * Alarm clock initialization music selection interface callback
 */
void Clock::selectAlarmMusic()
{
    refreshMusicSelectList(dialog_music);
    updateMusicLineEditElidedText(dialog_music);
}

/**
 * @brief 更新lineEdit中的省略文本显示
 * @param combo
 *
 * 仅处理lineEdit收拢状态下的省略显示与tooltip，popup视图相关配置
 * 全部由showPopupWithItemTooltip负责。
 */
void Clock::updateMusicLineEditElidedText(QComboBox *combo)
{
    if (!combo || !combo->lineEdit())
        return;

    QLineEdit *edit = combo->lineEdit();
    QFontMetrics fm(edit->font());

    // 使用itemText获取完整文本，而不是currentText（可能是缩略后的）
    QString fullText = combo->itemText(combo->currentIndex());
    int availableWidth = edit->width() - edit->textMargins().left() - edit->textMargins().right() - 5;    QString elidedText = fm.elidedText(fullText, Qt::ElideRight, availableWidth);

    edit->setText(elidedText);
    // 如果文本被截断，添加tooltip显示完整文本
    if (fullText != elidedText) {
        edit->setToolTip(fullText);
    } else {
        edit->setToolTip(QString());
    }
}

void Clock::refreshMusicSelectList(QComboBox *temp)
{
    QList<QString> * bellList = m_selectBtnUtil->getAllBellItem();
    QList<QString> * bellTrList = m_selectBtnUtil->getDefaultBellTrList();
    int newSize = bellList->size();
    int currentSize = temp->count();
    if(currentSize<newSize){
        //        qWarning()<<"zds-添加后选项数目不一致";
    }
    for (int i=0;i<bellList->size();i++) {
        QString bellName = bellList->at(i);
        QString name;
        if(i<5){
            name = bellTrList->at(i);
        }else{
            name = bellName;
        }
        temp->addItem(name);
    }
}

/*
 * 弹出指定下拉框，并对popup视图做好tooltip相关事件准备：
 *   - 开WA_AlwaysShowToolTips，否则顶层popup收不到QEvent::ToolTip
 *   - 把view的最大宽度限定为combo宽度，并把popup容器自身也收紧到combo宽度
 *     （popup容器按view sizeHint算尺寸，仅约束view没用，容器仍会撑宽，
 *      出现中间滚动条+右边空白）
 *   - installEventFilter让eventFilter在QEvent::ToolTip里判elision
 * installEventFilter本身幂等，无需额外的属性标记。
 */
void Clock::showPopupWithItemTooltip(QComboBox *combo)
{
    if (!combo)
        return;
    if (QAbstractItemView *view = combo->view()) {
        view->setAttribute(Qt::WA_AlwaysShowToolTips, true);
        // 限制view宽度，container尺寸计算可能仍按sizeHint走，下面再resize容器
        view->setMaximumWidth(combo->width());
        view->installEventFilter(this);
    }
    combo->showPopup();
    // popup容器(QComboBoxPrivateContainer是QFrame子类)也按combo同宽收紧，
    // 否则容器比view还宽，滚动条会跑到中间、item超出view的区域形成右边空白
    if (QWidget *popup = combo->findChild<QFrame*>()) {
        popup->setFixedWidth(combo->width());
    }
}

/*
 * 闹钟初始化单击选择音乐
 * Alarm initialization Click to select music
 */
void Clock::musicListclickslot()
{
    int num=dialog_music->currentIndex();
    int size = dialog_music->count();
    //最后一个自定义铃声
    if(num==(size-1)){
        addDivBell(dialog_music,Clock::add_clock);
    }else{
        QString id = m_selectBtnUtil->getBellIdByIndex(num);
        QString path = m_selectBtnUtil->getBellPathById(id);
        QString name = getBellNameById(id);
        dialog_music->setCurrentText(name);
        updateMusicLineEditElidedText(dialog_music);
        //排除无
        if(num==0){
            path="";
        }
        playMusicFromPath(path);
    }
}

void Clock::remindLateListClickSlot(int num)
{
    remind_late_str_model =getRemindLateStrFromNum(num);
}

QString Clock::getRemindLateStrFromNum(int num)
{
    QString temp=tr("none");
    switch (num)
    {
    case 0:
        temp=tr("none");
        break;
    case 1:
        temp=tr("five mins late");
        break;
    case 2:
        temp=tr("ten mins late");
        break;
    case 3:
        temp=tr("twenty mins late");
        break;
    case 4:
        temp=tr("thirsty mins late");
        break;
    case 5:
        temp=tr("one hour late");
        break;
    default:
        temp=tr("none");
        break;
    }
    return temp;
}



/*
 * Default setting database data initialization
 * 默认设置数据库数据初始化
 */
void Clock::modelSetupSet()
{
    int setup_rowNum;
    setup_rowNum = model_setup->rowCount();
    if (setup_rowNum < 1) {
        model_setup->insertRow(setup_rowNum);
        model_setup->setData(model_setup->index(setup_rowNum, 0), int(0));//静音    Mute
        QString id = m_selectBtnUtil->getDefaultBellId();
        model_setup->setData(model_setup->index(setup_rowNum, 1), id);
    }
    model_setup->submitAll();
}



void Clock::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    QPainterPath rectPath;
    rectPath.addRoundedRect(this->rect(), 0, 0); // 左上右下

    QPainter painter(this);
    QStyleOption opt;
    opt.init(this);
    painter.setBrush(opt.palette.color(QPalette::Base));

    QColor mainColor;
    mainColor = palette().color(QPalette::Base);
    p.fillPath(rectPath,QBrush(mainColor));
}



bool Clock::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == ui->editAlarmPage && event->type() == QEvent::Paint)
    {
        showPaint(); //响应函数
    }

    if(watched == shadow && event->type() == QEvent::Paint)
    {
        //        showPaint7();
    }
    // 处理 dialog_music lineEdit 点击弹出下拉框
    if (watched == dialog_music->lineEdit() && event->type() == QEvent::MouseButtonPress) {
        showPopupWithItemTooltip(dialog_music);
        return true;
    }
    // 处理 count_music_sellect lineEdit 点击弹出下拉框
    if (count_music_sellect && watched == count_music_sellect->lineEdit() && event->type() == QEvent::MouseButtonPress) {
        showPopupWithItemTooltip(count_music_sellect);
        return true;
    }

    // 处理 dialog_music / count_music_sellect popup 视图的tooltip事件：
    // 用view->visualRect(index).width()拿item真实渲染宽度，判elision
    // 后决定是否QToolTip::showText弹出完整DisplayRole文本
    if (event->type() == QEvent::ToolTip) {
        QAbstractItemView *view = qobject_cast<QAbstractItemView*>(watched);
        if (view && (view == dialog_music->view() ||
                     (count_music_sellect && view == count_music_sellect->view()))) {
            QHelpEvent *helpEvent = static_cast<QHelpEvent*>(event);
            QModelIndex idx = view->indexAt(helpEvent->pos());
            if (idx.isValid()) {
                const QString text = idx.data(Qt::DisplayRole).toString();
                if (!text.isEmpty()) {
                    // visualRect返回item真实绘制时的矩形
                    // 注意：QListView默认不会做elision
                    const QRect rect = view->visualRect(idx);
                    const int viewportWidth = view->viewport()->width();
                    const int visibleWidth = qMax(0, viewportWidth - rect.x());
                    const int availableWidth = qMin(rect.width(), visibleWidth);
                    const QFontMetrics fm(view->font());
                    const int textWidth = fm.horizontalAdvance(text);
                    const bool truncated = textWidth > availableWidth;
                    if (truncated) {
                        QToolTip::showText(helpEvent->globalPos(), text, view->viewport());
                    } else {
                        QToolTip::hideText();
                    }
                }
            } else {
                QToolTip::hideText();
            }
            return true;
        }
    }

    int index = ui->mainWidget->currentIndex();
    if(watched == ui->switchClock && event->type() == QEvent::HoverEnter){
        if(index==0||index==2){
            QColor backgroundColor = palette().color(QPalette::Button);
            ui->switchClock->setBackgroundColor(backgroundColor);
        }
    }
    if(watched == ui->switchCountdown && event->type() == QEvent::HoverEnter){
        if(index!=0){
            QColor backgroundColor = palette().color(QPalette::Button);
            ui->switchCountdown->setBackgroundColor(backgroundColor);
        }
    }
    if(watched == ui->switchStopwatch && event->type() == QEvent::HoverEnter){
        if(index!=2){
            QColor backgroundColor = palette().color(QPalette::Button);
            ui->switchStopwatch->setBackgroundColor(backgroundColor);
        }
    }
    return QWidget::eventFilter(watched,event);
}

/*
 * 实现响应函数设置页
*/
void Clock::showPaint()
{
    QPainter painter(ui->editAlarmPage);
    painter.setPen(Qt::gray);
    painter.setBrush(Qt::green);
    QStyleOption opt;
    opt.init(this);
    painter.setBrush(opt.palette.color(QPalette::Base));
    painter.setPen(Qt::transparent);
    QRect rect = ui->editAlarmPage->rect();
    rect.setWidth(rect.width() - 0);
    rect.setHeight(rect.height() - 0);
    painter.drawRoundedRect(rect, 7, 7);
    {
        QPainterPath painterPath;
        painterPath.addRoundedRect(rect, 0, 0);
        painter.drawPath(painterPath);
    }
}



/*
 * 实现响应函数设置页
*/
void Clock::showPaint7()
{
    QPainter painter(shadow);
    painter.setPen(Qt::gray);
    painter.setBrush(Qt::green);
    QStyleOption opt;
    opt.init(this);
    painter.setBrush(opt.palette.color(QPalette::Base));


    painter.setBrush(QColor(theme::timeScrollBackColor));

    painter.setPen(Qt::transparent);
    QRect rect = shadow->rect();
    rect.setWidth(rect.width() - 0);
    rect.setHeight(rect.height() - 0);
    painter.drawRoundedRect(rect, 7, 7);
    {
        QPainterPath painterPath;
        painterPath.addRoundedRect(rect, 8, 8);
        painter.drawPath(painterPath);
    }
}



void Clock::updateTinyBtn()
{
    QPixmap pixmap = QPixmap(":image/switchIconB.png");
    ui->tinyWindowBtn->setProperty("useButtonPalette", true);
    ui->tinyWindowBtn->setIcon(pixmap);
    ui->tinyWindowBtn->setIconSize(QSize(24,24));
    ui->tinyWindowBtn->setToolTip(tr("mini window"));
    ui->tinyWindowBtn->setAccessibleName("ukui-clock_QPushButton_tinyWindowBtn");
    //    ui->tinyWindowBtn->setFlat(true);
}
void Clock::updateSwitchBtnStyle()
{
    int index=ui->mainWidget->currentIndex();
    if(index==1){
        AlarmPageSwitch();
    }else if(index==0){
        CountdownPageSwitch ();
    }else if(index==2){
        StopwatchPageSwitch ();
    }
}
//黑色主题
void  Clock::blackStyle()
{
    ui->timeShowBig->setStyleSheet("color: rgba(255, 255, 255, 0.9);font-size:38px;");
    //    QString selfFont = Utils::loadFontFamilyFromTTF();
    //    QFont f(selfFont);
    QFont f;
    f.setFamily("NotoSansCJKsc-Regular");
    f.setWeight(400);
    f.setPixelSize(38);
    ui->timeShowBig->setFont(f);
    ui->remainTime->setStyleSheet("color: rgba(255, 255, 255, 0.9)");
    ui->noAlarmIcon->setPixmap(QPixmap(":/image/noClockBlack.png"));
    ui->noAlarmIcon->setAccessibleName("ukui-clock_QLable_noAlarmIcon");
    ui->noAlarm->setStyleSheet("color: rgba(255, 255, 255, 0.6);font-size:14px;");

    widgetListBlackStyle(ui->alarmListWidget);
    widgetListBlackStyle(ui->timeListWidget);
    ui->tinyWindowBtn->setIcon(QIcon(":image/switchIconW.png"));
    QPixmap pixmap = QPixmap(":/image/miniIcon/mute-off-dark.png");
    if(m_muteOn){
        pixmap = QPixmap(":/image/miniIcon/mute-on.png");
    }
    muteBtn->setIcon(pixmap);
    updateSwitchBtnStyle();
    repeat_on_Pixmap=theme::changeIconColor(repeat_on_Pixmap,palette().highlightedText().color());
}
//白色主题
void  Clock::whiteStyle()
{
    ui->timeShowBig->setStyleSheet("color: rgba(49, 66, 89, 1);font-size:38px;");
    //    QString selfFont = Utils::loadFontFamilyFromTTF();
    //    QFont f(selfFont);
    QFont f;
    f.setFamily("NotoSansCJKsc-Regular");
    f.setWeight(400);
    f.setPixelSize(38);
    ui->timeShowBig->setFont(f);
    ui->remainTime->setStyleSheet("color: rgba(49, 66, 89, 1)");
    ui->noAlarmIcon->setPixmap(QPixmap(":/image/noClockWhite.png"));
    ui->noAlarmIcon->setAccessibleName("ukui-clock_QLable_noAlarmIcon");
    ui->noAlarm->setStyleSheet("color: rgba(49, 66, 89, 0.6);font-size:14px;");
    widgetListWhiteStyle(ui->alarmListWidget);
    widgetListWhiteStyle(ui->timeListWidget);
    ui->tinyWindowBtn->setIcon(QIcon(":image/switchIconB.png"));
    QPixmap pixmap = QPixmap(":/image/miniIcon/mute-off.png");
    if(m_muteOn){
        pixmap = QPixmap(":/image/miniIcon/mute-on.png");
    }
    muteBtn->setIcon(pixmap);
    updateSwitchBtnStyle();
    repeat_on_Pixmap = QPixmap(":/image/object-select-symbolic.png");
}

QString Clock::getDefaultGreyColor()
{
    QColor color = QColor(COUNTDOWN_RING_BACKCOLOR_WHITE_STYLE);
    QString str = theme::getColorStr(color);
    return str;
}

void Clock::textChanged(const QString &arg1)
{
    QString str = arg1;
    str.replace("~", "");
    str.replace("`", "");
    str.replace("!", "");
    str.replace("@", "");
    str.replace("#", "");
    str.replace("$", "");
    str.replace("%", "");
    str.replace("^", "");
    str.replace("&", "");
    str.replace("*", "");
    str.replace("(", "");
    str.replace(")", "");
    str.replace("_", "");
    str.replace("-", "");
    str.replace("+", "");
    str.replace("=", "");
    str.replace("[", "");
    str.replace("]", "");
    str.replace("{", "");
    str.replace("}", "");
    str.replace("\\", "");
    str.replace("|", "");

    str.replace(":", "");
    str.replace(";", "");
    str.replace("\"", "");
    str.replace("'", "");
    str.replace("<", "");
    str.replace(",", "");
//    str.replace(" ", "");
    str.replace(">", "");
    str.replace(".", "");
    str.replace("?", "");
    str.replace("/", "");


    str.replace("·", "");
    str.replace("！", "");
    str.replace("￥", "");
    str.replace("……", "");
    str.replace("（", "");
    str.replace("）", "");
    str.replace("{", "");
    str.replace("}", "");
    str.replace("【", "");
    str.replace("】", "");
    str.replace("、", "");
    str.replace("：", "");

    str.replace("；", "");
    str.replace("“", "");
    str.replace("”", "");
    str.replace("‘", "");
    str.replace("’", "");
    str.replace("《", "");
    str.replace("》", "");
    str.replace("，", "");
    str.replace("。", "");
    str.replace("？", "");
    str.replace("、", "");
    str.replace("——", "");


    clockEditOnClockNew->setText(str);
}




void Clock::publishDbusInterface(){
    m_obj = new QObject();
    m_obj->setObjectName("ukui-clock_QObject_m_obj");
    ClockDbusAdaptor *adaptor = new ClockDbusAdaptor(m_obj, this);
    adaptor->setObjectName("ukui-clock_ClockDbusAdaptor_adaptor");
    //注册接口
    QDBusConnection::sessionBus().registerObject(CLOCK_DBUS_SERVICE_PATH, m_obj);
    //判断是否注册成功
    if (!QDBusConnection::sessionBus().registerService(CLOCK_DBUS_SERVICE_NAME)) {
        qCritical()<<"注册接口失败"<<QDBusConnection::sessionBus().lastError().message();
        exit(1);
    }
    Q_UNUSED(adaptor);
}

void Clock::publishDbusUpdateClock(){
    m_updateObj = new QObject();
    m_updateObj->setObjectName("ukui-clock_QObject_m_updateObj");
    DbusUpdateClockAdaptor *adaptor = new DbusUpdateClockAdaptor(m_updateObj, this);
    adaptor->setObjectName("ukui-clock_DbusUpdateClockAdaptor_adaptor");
    //注册接口
    QDBusConnection::sessionBus().registerObject(CLOCK_DBUS_UPDATE_CLOCK_BY_ID_PATH, m_updateObj);
    //判断是否注册成功
    if (!QDBusConnection::sessionBus().registerService(CLOCK_DBUS_SERVICE_NAME)) {
        qCritical()<<"注册接口失败"<<QDBusConnection::sessionBus().lastError().message();
        exit(1);
    }
    Q_UNUSED(adaptor);
}

void Clock::publishDbusSelectClock(){
    m_selectObj = new QObject();
    m_selectObj->setObjectName("ukui-clock_QObject_m_selectObj");
    DbusSelectClockByIdAdaptor *adaptor = new DbusSelectClockByIdAdaptor(m_selectObj, this);
    adaptor->setObjectName("ukui-clock_DbusSelectClockByIdAdaptor_adaptor");
    //注册接口
    QDBusConnection::sessionBus().registerObject(CLOCK_DBUS_SELECT_CLOCK_BY_ID_PATH, m_selectObj);
    //判断是否注册成功
    if (!QDBusConnection::sessionBus().registerService(CLOCK_DBUS_SERVICE_NAME)) {
        qCritical()<<"注册接口失败"<<QDBusConnection::sessionBus().lastError().message();
        exit(1);
    }
    Q_UNUSED(adaptor);
}

void Clock::publishDbusDeleteClock(){
    m_deleteObj = new QObject();
    m_deleteObj->setObjectName("ukui-clock_QObject_m_deleteObj");
    DbusDeleteClockByIdAdaptor *adaptor = new DbusDeleteClockByIdAdaptor(m_deleteObj, this);
    adaptor->setObjectName("ukui-clock_DbusDeleteClockByIdAdaptor_adaptor");
    //注册接口
    QDBusConnection::sessionBus().registerObject(CLOCK_DBUS_DELETE_CLOCK_BY_ID_PATH, m_deleteObj);
    //判断是否注册成功
    if (!QDBusConnection::sessionBus().registerService(CLOCK_DBUS_SERVICE_NAME)) {
        qCritical()<<"注册接口失败"<<QDBusConnection::sessionBus().lastError().message();
        exit(1);
    }
    Q_UNUSED(adaptor);
}
