/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/


#include "mainwindow.h"
#include "src/common/utils.h"
#include "src/view/ui_mainwindow.h"
#include "src/view/burndata.h"
#include "src/view/burnimage.h"
#include "src/view/copydisc.h"
#include "src/view/modaldialog.h"
#include "src/view/execdialog.h"
#include "src/view/aboutdialog.h"
#include "src/view/viewfilterfiles.h"
#include "src/view/filefilterrules.h"
#include "src/view/tip_burnresult.h"
#include "src/frame/xatom-helper.h"
#include "src/frame/logger.h"
#include "src/frame/signals.h"
#include "src/frame/configuration.h"
#include "src/po/translation.h"
#include "src/model/logicmain.h"
#include "src/model/menufactory.h"
#include "src/data/datacdrom.h"
#include "src/common/utils.h"
#include "v11sdk.h"
#include "src/common/trackingmanager.h"

#include <fstream>
#include <QPair>
#include <QtDBus>
#include <QDBusInterface>
#include <QDebug>
#include <QPainter>
#include <QPainterPath>
#include <QFileInfo>
#include <QMoveEvent>
#include <QFileSystemWatcher>
#include <QSharedMemory>
#include <QDBusReply>
#include <QDesktopWidget>

#include <pwd.h>
#include <unistd.h>

#include <KF5/KWindowSystem/kwindowsystem.h>
#include <windowmanager/windowmanager.h>
#include <kaboutdialog.h>
#if UKUI_SESSION_TYPE == 2
  #include <ukuiwindowhelper.h>
#endif

using namespace kdk;

#define ICON_SIZE QSize(26, 26)
MainWindow *MainWindow::m_mainWindow = nullptr;
QMutex      MainWindow::m_oMutex;
bool        MainWindow::firstAddFlag = true;

MainWindow::MainWindow(QStringList args, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_oGroup(new QButtonGroup(this))
{ 
    //判断是否是平板
    QDBusConnection sessionBus1 = QDBusConnection::sessionBus();
    QDBusMessage m = QDBusMessage::createMethodCall(QString("com.kylin.statusmanager.interface"),
                                                    QString("/"),
                                                    QString("com.kylin.statusmanager.interface"),
                                                    QString("get_current_tabletmode"));

    QDBusMessage res = sessionBus1.call(m);
    if (res.type() == 1 || res.type() == 2) {
        m_isTabletMode = res.arguments().first().toBool();
    } else {
        m_isTabletMode = false;
    }

    //切换模式
    QDBusConnection::sessionBus().connect(QString("com.kylin.statusmanager.interface"),
                                          QString("/"),
                                          QString("com.kylin.statusmanager.interface"),
                                          QString("mode_change_signal"), this,
                                          SLOT(uiModeChange(bool)));

    ui->setupUi(this);
    setAccessibleName();
    uiAdjust();
    m_oCDROMS.clear();

    m_maxBtnHoverDelayTimer = new QTimer(this);
    m_maxBtnHoverDelayTimer->setInterval(500);
    m_maxBtnHoverDelayTimer->setSingleShot(true);

    //判断是否是wayland环境
    if(SystemInfo::m_isWayland) {
        ukuiWaylandRemoveTitleBar(this);
    } else {
        MotifWmHints hints;
        hints.flags = MWM_HINTS_FUNCTIONS | MWM_HINTS_DECORATIONS;
        hints.functions = MWM_FUNC_ALL;
        hints.decorations = MWM_DECOR_BORDER;
        XAtomHelper::getInstance()->setWindowMotifHint(winId(), hints);
    }

    this->setAttribute(Qt::WA_AlwaysShowToolTips);

    LOGGER->registerName(this, _("MainWindow"));

    connect(g_user_signal, SIGNAL(changeBtnBusnessName()), this, SLOT(slotsChangeBtnBusnessName()));

    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if(sessionBus.registerService("com.kylin.burner.manager"))
    {
        sessionBus.registerObject("/com/kylin/burner/manager","com.kylin.burner.manager",this,
                                                  QDBusConnection::ExportAllContents);

        qDebug()<<"初始化DBUS成功";
    }
    else
    {
        LOGGER->info("init dbus error");
        qDebug() << sessionBus.lastError();
        qDebug()<<"初始化DBUS失败";
    }

    // 获取窗口
    connect(WindowManager::self(),&WindowManager::windowAdded,this,[=](const WindowId& windowId) {
        /* 注意：最新创建的窗体被设置为操作窗体，所有接口函数操作的窗口都是该最新创建的窗体 */

        if (((quint32)getpid() == WindowManager::getPid(windowId)) && (!WindowManager::getWindowTitle(windowId).contains("SplashWindow"))) {
            if(firstAddFlag){
                m_mainWindowId = windowId.toString();
                firstAddFlag = false;
            }
            m_lastestWindowId = windowId.toString();
        }
    });


    // 初始化业务界面
    connect(BurnData::instance(), &BurnData::burnDataToDiscEnabled, this, &MainWindow::slotsBusness);
    connect(BurnData::instance(), &BurnData::updateDisk, this, &MainWindow::slotsUpdateDisk);
    connect(PAGE_BurnImage, &BurnImage::busness, this, &MainWindow::slotsBusness);
    connect(CopyDisc::instance(), SIGNAL(busness(bool)), this, SLOT(slotsBusness(bool)));
    connect(CopyDisc::instance(), SIGNAL(busness(int)), this, SLOT(slotsChangeName(int)));
    ui->stackedWidget->addWidget(BurnData::instance());
    ui->stackedWidget->addWidget(BurnImage::instance());
    ui->stackedWidget->addWidget(CopyDisc::instance());

    ui->navTitle->onlyShowLogo();
    ui->burnerTitle->onlyShowButton();
    // 自定义标题栏
    connect(this, SIGNAL(windowIconChanged(QIcon)), ui->navTitle, SLOT(onWindowIconChange(QIcon)));
    connect(this, SIGNAL(windowTitleChanged(QString)), ui->navTitle, SLOT(onWindowTitleChange(QString)));
    connect(this, SIGNAL(windowIconChanged(QIcon)), ui->burnerTitle, SLOT(onWindowIconChange(QIcon)));
    connect(this, SIGNAL(windowTitleChanged(QString)), ui->burnerTitle, SLOT(onWindowTitleChange(QString)));
    connect(g_user_signal, &GlobalUserSignal::sigCloseApp, this, &MainWindow::slotsCloseApp);
    connect(ui->navTitle, SIGNAL(close()), this, SLOT(slotsCloseApp()));
    connect(ui->navTitle, SIGNAL(min()), this, SLOT(showMinimized()));
#if UKUI_SESSION_TYPE == 2
    m_windowHelper = new UkuiWindowHelper(this);
    connect(g_user_signal, &GlobalUserSignal::sigMaxButtonHoverEnter, this, &MainWindow::slotShowTileFlyout);
    connect(g_user_signal, &GlobalUserSignal::sigMaxButtonHoverLeave, this, &MainWindow::slotHideTileFlyout);
    connect(m_maxBtnHoverDelayTimer, &QTimer::timeout, this, [this](){
        m_windowHelper->showTileFlyout(m_tileFlyoutRect);
    });
    m_windowHelper->setBlurEffect(ui->nav->rect(), 0, true);
#endif
    connect(ui->navTitle, SIGNAL(close()), this, SLOT(slotsCloseApp()));
    connect(ui->navTitle, &WindowTitle::max, this, &MainWindow::slotChangeWindowState);
    connect(ui->burnerTitle, SIGNAL(close()), this, SLOT(slotsCloseApp()));
    connect(ui->burnerTitle, SIGNAL(min()), this, SLOT(showMinimized()));
    connect(ui->burnerTitle, &WindowTitle::max, this, &MainWindow::slotChangeWindowState);

    //图标库主题的刻录图片名换为burner
    setWindowIcon(QIcon::fromTheme("burner"));
    setWindowTitle(_("Burner"));
    ui->nav->setFocus();

    slotChangeUIFontSize(LogicMain::instance()->GetSystemFontSize());
    connect(LogicMain::instance(), &LogicMain::systemFontSizeChanged,this,&MainWindow::slotChangeUIFontSize);

    ui->labelBurner->setText(_("BurnerName"));
    ui->labelDisk->setText(_("DiskName"));

    connect(g_user_signal, &GlobalUserSignal::sigChangeComboDiskIndex, ViewFilterFiles::instance(), &ViewFilterFiles::slotsChangeComboDiskIndex);
    connect(ui->comboBoxBurner, SIGNAL(currentIndexChanged(int)), this, SLOT(slotsChangeCurrentIndex(int)));
    connect(ui->comboBoxDisk, SIGNAL(currentIndexChanged(int)), this, SLOT(slotsChangeCurrentIndex(int)));
    connect(ui->comboBoxDisk, SIGNAL(currentIndexChanged(int)), BurnData::instance(), SIGNAL(changeCurrentIndex(int)));

    connect(this, &MainWindow::delData, BurnData::instance(), &BurnData::delData);
    connect(this, &MainWindow::delData, ViewFilterFiles::instance(), &ViewFilterFiles::delNode);

    connect(ui->comboBoxDisk, SIGNAL(currentIndexChanged(int)), this, SLOT(slotsAdjustBurnData(int)));

    ui->btnSetting->setCheckable(true);
    connect(ui->btnSetting, SIGNAL(toggled(bool)), this, SLOT(slotsBrowseOrSetting(bool)));
    connect(this, SIGNAL(browse(QString)), LogicMain::instance(), SIGNAL(storagePath(QString)));

    connect(this, &MainWindow::busness, LogicMain::instance(), &LogicMain::slotsSetBusness);

    connect(ui->btnBusness, &QPushButton::clicked, LogicMain::instance(), &LogicMain::slotsDoBusness);

    connect(LogicMain::instance(), SIGNAL(createISO(void*)), this, SLOT(slotsInitCreateISO(void*)));

    connect(LogicMain::instance(), &LogicMain::mediumRmoved, this, &MainWindow::slotsNoMediun);
    connect(LogicMain::instance(), &LogicMain::CDRomRemoved, this, &MainWindow::slotsRemoveCDROM);
    connect(LogicMain::instance(), &LogicMain::emptyMedium, this, &MainWindow::slotsInitCreateISO);


    connect(GlobalUserSignal::getInstance(), &GlobalUserSignal::sigInitCreateISO, this, &MainWindow::slotsInitCreateISO);

    connect(LogicMain::instance(), &LogicMain::finishedMedium, this, &MainWindow::slotsInitCreateISO);

    connect(LogicMain::instance(), &LogicMain::emptyMedium, MenuFactory::instance(), &MenuFactory::newEarse);
    connect(LogicMain::instance(), &LogicMain::appendableMedium, MenuFactory::instance(), &MenuFactory::newEarse);
    connect(LogicMain::instance(), &LogicMain::finishedMedium, MenuFactory::instance(), &MenuFactory::newEarse);

    connect(LogicMain::instance(), &LogicMain::mediumRmoved, MenuFactory::instance(), &MenuFactory::removeEarse);

    connect(g_user_signal, SIGNAL(haveMedium(void*)), MenuFactory::instance() , SLOT(newEarse(void*)));

    connect(g_user_signal, &GlobalUserSignal::iconChange, [=](QIcon icon){
        setWindowIcon(icon);
    });
    connect(g_user_signal, &GlobalUserSignal::sigChangeDiscDisplayInfo, this, [=](void *cdrom, QString displayInfo){
        int index = 0;
        for (; index<m_oCDROMS.size(); index++) {
            if (cdrom == m_oCDROMS[index]) {
                ui->comboBoxDisk->setItemText(index, displayInfo);
                break;
            }
        }
    });

   ui->stackedWidget->installEventFilter(this);
   ui->comboBoxBurner->installEventFilter(this);
   ui->comboBoxDisk->installEventFilter(this);
   ui->nav->installEventFilter(this);

   ui->stackedWidget->setFocusPolicy(Qt::NoFocus);
   ui->comboBoxBurner->setFocusPolicy(Qt::NoFocus);
   ui->comboBoxDisk->setFocusPolicy(Qt::NoFocus);
   ui->nav->setFocusPolicy(Qt::NoFocus);
   installEventFilter(this);

    uiModeChange(m_isTabletMode);

    if (args.size() == 1)
    {
        slotChangeToImageBurnPage(args.first());
    }
    else
    {
        emit busness(LogicMain::CreateISO);
    }

    this->setObjectName("kylin-burner_MainWindow");
}

MainWindow* MainWindow::Instance(QStringList args, QWidget *parent)
{
    if (!m_mainWindow)
    {
        QMutexLocker locker(&m_oMutex);
        if (!m_mainWindow)
        {
            m_mainWindow = new MainWindow(args, parent);
        }
    }
    return m_mainWindow;
}

void MainWindow::moveEvent(QMoveEvent *event)
{
    QPoint p;
    QPoint sp;

    p.setX(event->pos().x() + width() / 2);
    p.setY(event->pos().y() + height() / 2);

    ModalDialog::instance()->updatePosition(p);
    FileFilterRules::instance()->updatePosition(p);
    ViewFilterFiles::instance()->updatePosition(p);
    Tip_BurnResult::instance()->updatePosition(p);

    sp.setX(p.x() - EXEC->width() / 2);
    sp.setY(p.y() - EXEC->height() / 2);
    ExecDialog::instance()->move(sp);

    QMainWindow::moveEvent(event);
}

void MainWindow::paintEvent(QPaintEvent *event)
{
#if UKUI_SESSION_TYPE == 2
    QPalette pal = ui->burner->palette();
    QColor burnerBackgourndColor, navBackgroundColor;

    if ("ukui-black" == LogicMain::instance()->theme())
    {
        navBackgroundColor = QColor("#484848");
        burnerBackgourndColor = QColor("#1D1D1D");
    }
    else
    {
        navBackgroundColor = QColor("#F0F0F0");
        burnerBackgourndColor = QColor("#FFFFFF");

    }


    QPainter painter(this);
    painter.setOpacity(m_transparentValue);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.setClipping(true);

    painter.setBrush(this->palette().window());
    painter.setPen(Qt::transparent);

    painter.fillRect(ui->nav->rect(), navBackgroundColor);

    QMainWindow::paintEvent(event);
#else

    QPalette pal = ui->burner->palette();
    if ("ukui-black" == LogicMain::instance()->theme())
    {
        pal.setBrush(QPalette::Background, QColor("#484848"));
        ui->nav->setPalette(pal);
        pal.setBrush(QPalette::Background, QColor("#1D1D1D"));
        ui->burner->setPalette(pal);
    }
    else
    {
        pal.setBrush(QPalette::Background, QColor("#F0F0F0"));
        ui->nav->setPalette(pal);
        pal.setBrush(QPalette::Background, QColor("#FFFFFF"));
        ui->burner->setPalette(pal);
    }

    QPainterPath path;
    QPainter painter(this);
    painter.setOpacity(m_transparentValue);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    painter.setClipping(true);

    path.addRect(ui->nav->rect());
    path.setFillRule(Qt::WindingFill);
    painter.setBrush(this->palette().window());
    painter.setPen(Qt::transparent);
    painter.drawPath(path);

    QMainWindow::paintEvent(event);
#endif
}

void MainWindow::setAccessibleName()
{
    ui->btnBusness->setAccessibleName("kylin-burner_QPushButton_MainWindow_btnBusness");
    ui->btnSetting->setAccessibleName("kylin-burner_QPushButton_MainWindow_btnSetting");
    ui->burnData->setAccessibleName("kylin-burner_QWidget_MainWindow_burnData");
    ui->burner->setAccessibleName("kylin-burner_QWidget_MainWindow_burner");
    ui->burnerTitle->setAccessibleName("kylin-burner_WindowTitle_MainWindow_burnerTitle");
    ui->buttom->setAccessibleName("kylin-burner_QWidget_MainWindow_buttom");
    ui->buttons->setAccessibleName("kylin-burner_QWidget_MainWindow_buttons");
    ui->centralwidget->setAccessibleName("kylin-burner_QWidget_MainWindow_centralwidget");
    ui->comboBoxBurner->setAccessibleName("kylin-burner_QComboBox_MainWindow_comboBoxBurner");
    ui->comboBoxDisk->setAccessibleName("kylin-burner_QComboBox_MainWindow_comboBoxDisk");
    ui->labelBurner->setAccessibleName("kylin-burner_QLabel_MainWindow_labelBurner");
    ui->labelDisk->setAccessibleName("kylin-burner_QLabel_MainWindow_labelDisk");
    ui->nav->setAccessibleName("kylin-burner_QWidget_MainWindow_nav");
    ui->navTitle->setAccessibleName("kylin-burner_WindowTitle_MainWindow_navTitle");
    ui->splitter->setAccessibleName("kylin-burner_QSplitter_MainWindow_splitter");
    ui->stackedWidget->setAccessibleName("kylin-burner_QStackedWidget_MainWindow_stackedWidget");
    ui->widget->setAccessibleName("kylin-burner_QWidget_MainWindow_widget");
    ui->widgetCopyRight->setAccessibleName("kylin-burner_QWidget_MainWindow_widgetCopyRight");
    ui->widget_2->setAccessibleName("kylin-burner_QWidget_MainWindow_widget2");
}

//主界面窗口设置为最顶层窗口，显示在最前面
int MainWindow::slotsReShowMainWindow(QStringList args)
{
    //存在kylin-burner进程
    LOGGER->info(_("kylin-burner process already exists"));
    LOGGER->info(_("New kylin-burner process exit"));

    qDebug()<<"kylin-burner process already exists, New kylin-burner process exit";

    QFileInfo file;
    QUrl url;
    int value = -1;
    if (0 == args.size()) {
        value = 0;
        LOGGER->info("The kylin-burner process is running, 0 argument, active mainwindow");
    } else if (1 == args.size()) {
        value = 1;
        LOGGER->info("The kylin-burner process is running, 1 argumentactive mainwindow and swith burn image view");
        file.setFile(args.first());
        if (file.suffix() == "iso")
        {
            url.setUrl(args.first());
            PAGE_BurnImage->slotsNewISOPath(args.first());
            emit busness(LogicMain::BurnImage);
            QMetaObject::invokeMethod(this,"slotsBurnImage",Qt::QueuedConnection);

            QListView* mViewOfNavBar = m_NavBar->listview();
            if(mViewOfNavBar){
                mViewOfNavBar->setCurrentIndex(mViewOfNavBar->model()->index(1,0,QModelIndex()));
            }

        }
    }
    if (SystemInfo::m_isWayland) {
        WindowManager::activateWindow(m_mainWindowId);
    } else {
        KWindowSystem::forceActiveWindow(this->winId());
    }
    return value;
}

bool MainWindow::isBusyDevice(QString device)
{
    return g_user_signal->getDeviceBusyStatus(device);
}

void MainWindow::slotChangeUIFontSize(int fontSize)
{
    int scaleRatio = qAbs(fontSize - 10.0);
    fontSize = scaleRatio  +  14;

    QFont font = ui->btnSetting->font();
    font.setPixelSize(fontSize);

    ui->btnBusness->setFont(font);
    ui->btnSetting->setFont(font);

    ui->comboBoxBurner->setFont(font);
    ui->comboBoxDisk->setFont(font);

    font = ui->labelBurner->font();
    font.setWeight(60);
    font.setPixelSize(fontSize);
    ui->labelBurner->setFont(font);
    ui->labelDisk->setFont(font);

    ui->navTitle->setFont(font);

    resetText(ui->btnBusness);
    resetText(ui->btnSetting);
}

void MainWindow::slotChangeToImageBurnPage(QString imagePath)
{
    QUrl url;
    QFileInfo file;
    file.setFile(imagePath);

    if (file.suffix() == "iso")
    {
        url.setUrl(imagePath);
        PAGE_BurnImage->slotsNewISOPath(imagePath);
        emit busness(LogicMain::BurnImage);
        QMetaObject::invokeMethod(this,"slotsBurnImage",Qt::QueuedConnection);

        QListView* mViewOfNavBar = m_NavBar->listview();

        if(mViewOfNavBar){
            mViewOfNavBar->setCurrentIndex(mViewOfNavBar->model()->index(1,0,QModelIndex()));
        }
    }
}

void MainWindow::slotChangeWindowState()
{
    if (Qt::WindowMaximized == windowState())
    {
        showNormal();
        ui->burnerTitle->onWindowShowMaxed(false);
    }
    else
    {
        showMaximized();
        ui->burnerTitle->onWindowShowMaxed(true);
    }
}

void MainWindow::slotChangeStackWindowIndex(QModelIndex index)
{
    switch(index.row()){
    case 0:
        slotsBurnData();
        break;
    case 1:
        slotsBurnImage();
        break;
    case 2:
        slotsCopyDisc();
        break;
    default:
        break;
    }
    m_current_page = index.row();
    calculatorTime(m_current_page);
}

void MainWindow::slotSwitchBurnFormat(DataCDROM *cdrom)
{
    if(!cdrom){
        return;
    }
#ifndef UNIT_TEST
    //判断是做UDF盘还是ISO9660盘
    if(cdrom->fileSystemTypeWithExtendName() == UDF102)
    {
        switch (cdrom->mediumStatus()) {
        case DataMedium::MEDIUM_EMPTY:
            ui->btnBusness->setText(_("BurnUDFData"));
            emit busness(LogicMain::BurnUDFData);
            break;
        default:
            ui->btnBusness->setText(_("AppendBurn"));
            emit busness(LogicMain::AddBurn);
            break;
        }
    }
    else if(cdrom->fileSystemTypeWithExtendName() == UDF201){
        ui->btnBusness->setText(_("DonotBurn"));
        emit busness(LogicMain::AddBurn);
    }
    else if(cdrom->getBurnFormat() == "ISO9660")
    {
        switch (cdrom->mediumStatus())
        {
        case DataMedium::MEDIUM_EMPTY:
            ui->btnBusness->setText(_("BurnData"));
            emit busness(LogicMain::BurnISO9660Data);
            break;
        case DataMedium::MEDIUM_INCOMPLETE:
            ui->btnBusness->setText(_("AppendBurn"));
            emit busness(LogicMain::AddBurn);
            break;
        default:
            if(cdrom->isCDROM()){
                ui->btnBusness->setText(_("DonotBurn"));
                emit busness(LogicMain::AddBurn);
            }
            break;
        }
    }
    else
    {
        LOGGER->error(_("cdrom->getBurnFormat() == [NULL]"));
    }
#endif
}

void MainWindow::slotsBurnData()
{
    ui->burnData->show();
    ui->stackedWidget->setCurrentIndex(0);
    // 根据comboBoxDisk 下表结合其对应的光驱信息确认业务类型
    slotsAdjustBurnData(ui->comboBoxDisk->currentIndex());
}

void MainWindow::slotsBurnImage()
{
    ui->burnData->hide();
    ui->btnBusness->setText(_("BurnImage"));
    ui->stackedWidget->setCurrentIndex(1);
    emit busness(LogicMain::BurnImage);
    PAGE_BurnImage->update();
}

void MainWindow::slotsCopyDisc()
{
    ui->burnData->hide();
    ui->btnBusness->setText(_("GetISO"));
    ui->stackedWidget->setCurrentIndex(2);
    emit busness(LogicMain::GetISO);
    PAGE_CopyDisc->check();
}

//切换主界面左侧功能模式或切换光驱的时候被调用
void MainWindow::slotsAdjustBurnData(int idx)
{
    DataCDROM *cdrom = nullptr;
    if (idx < 0)
        return;

    if ( 0 != ui->stackedWidget->currentIndex() )
        return;
    // 根据comboBoxDisk 下表结合其对应的光驱信息确认业务类型
    if (idx > 0)
    {
        if (idx >= m_oCDROMS.size()) return;
        cdrom = static_cast<DataCDROM *>(m_oCDROMS.at(idx));
        ui->btnSetting->setText(_("Setting"));

        slotSwitchBurnFormat(cdrom);

    }
    else
    {
        ui->btnSetting->setText(_("Browse"));
        ui->btnBusness->setText(_("CreateISO"));
        emit busness(LogicMain::CreateISO);
    }
    BurnData::instance()->refresh();
    resetText(ui->btnBusness);
    resetText(ui->btnSetting);
}

//光盘待刻录数据发生变化时调用
void MainWindow::slotsBusness(bool enabled)
{
    if (BurnData::instance() == sender() && 0 == ui->stackedWidget->currentIndex())
    {
        int index = ui->comboBoxDisk->currentIndex();

        int deviceCount = m_oCDROMS.count();

        // index valid check
        if((index < 0) || (index >= deviceCount)){
            LOGGER->error(_("cdrom find beyond bound "));
            return;
        }

        DataCDROM *cdrom = static_cast<DataCDROM*>(m_oCDROMS.at(index));
#ifndef UNIT_TEST
        if (cdrom->fileSystemTypeWithExtendName() == UDF102) {
            int udf_enabled = (cdrom->isEnableUDF() && enabled);
            ui->btnBusness->setEnabled(udf_enabled);
            ui->btnSetting->setEnabled(udf_enabled);
            MenuFactory::instance()->setSetting(udf_enabled);
        } else {
            ui->btnBusness->setEnabled(enabled);
            ui->btnSetting->setEnabled(enabled);
            MenuFactory::instance()->setSetting(enabled);
        }
#endif
    }
    else if (PAGE_BurnImage == sender() && 1 == ui->stackedWidget->currentIndex())
    {
        ui->btnBusness->setEnabled(enabled);
        MenuFactory::instance()->setSetting(enabled);
    }
    else if (PAGE_CopyDisc == sender() && 2 == ui->stackedWidget->currentIndex())
    {
        ui->btnBusness->setEnabled(enabled);
        MenuFactory::instance()->setSetting(enabled);
    }

    if (enabled)
    {
        ui->btnBusness->setProperty("isImportant", true);
    }
}

void MainWindow::slotsInitCreateISO(void *inputRom)
{
#ifndef UNIT_TEST
    int index;
    DataCDROM *cdrom = (DataCDROM *)inputRom;
    if(cdrom->m_is_using <= 0) return;

    qDebug() << cdrom->mDisplay();

    if (-1 != m_oCDROMS.indexOf(cdrom))
    {
        //设备已存在且查询到属性更新时，需要刷新UI界面
        ui->comboBoxDisk->setItemText(m_oCDROMS.indexOf(inputRom),cdrom->mDisplay());

        BurnData::instance()->refresh();
        return;
    }

    //追加ROOM
    m_oCDROMS << inputRom;

    ui->comboBoxBurner->addItem(cdrom->icon(), cdrom->display());
    ui->comboBoxDisk->addItem(cdrom->mediumIcon(), cdrom->mDisplay());
    index = ui->comboBoxDisk->count() - 1;


    if (index >= 0)
    {
        ui->comboBoxDisk->setCurrentIndex(index);
    }
    else
    {
        ui->comboBoxDisk->setCurrentIndex(0);
    }

    BurnData::instance()->refresh();

    if (0 != ui->stackedWidget->currentIndex()) return;

    ui->btnSetting->setText(_("Setting"));
    slotSwitchBurnFormat(cdrom);


    resetText(ui->btnBusness);
    resetText(ui->btnSetting);
#endif
}

void MainWindow::slotsNoMediun(void * d)
{
#ifndef UNIT_TEST
    int idx;
    idx = m_oCDROMS.indexOf(d);
    if (-1 == idx) return;
    m_oCDROMS.takeAt(idx);

    emit delData(idx); // 先删除数据，后移除展示

    ui->comboBoxDisk->removeItem(idx);
    ui->comboBoxBurner->removeItem(idx);
    if (idx < ui->comboBoxDisk->count())
    {
        ui->comboBoxDisk->setCurrentIndex(idx);
    }
    else
    {
        ui->comboBoxDisk->setCurrentIndex(0);
    }
    LOGGER->debug(_("Delete item %1 of total %2").arg(idx).arg(ui->comboBoxBurner->count()));
#endif
}

void MainWindow::slotsRemoveCDROM(void *d)
{
    int idx;

    idx = m_oCDROMS.indexOf(d);
    if (-1 != idx)
    {
        LOGGER->debug(_("Remove CDROM from burn data."));
        m_oCDROMS.takeAt(idx);
    }
}

void MainWindow::slotsBrowseOrSetting(bool b)
{
#ifndef UNIT_TEST
    QString path;
    QString testPath;
    QFileDialog fd(this);
    QList<QUrl> list = fd.sidebarUrls();
    int sidebarNum = 8;//最大添加U盘数，可以自己定义
    QString home = QDir::homePath().section("/", -1, -1);
    QString mnt = "/media/" + home + "/";
    QDir mntDir(mnt);
    mntDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    QFileInfoList filist = mntDir.entryInfoList();
    QList<QUrl> mntUrlList;
    for (int i = 0; i < sidebarNum && i < filist.size(); ++i) {
        QFileInfo fi = filist.at(i);
        if (fi.fileName() == "2691-6AB8") /* 990 9a0 */
                 continue;
        mntUrlList << QUrl("file://" + fi.filePath());
    }

    QFileSystemWatcher fsw(&fd);
    fsw.addPath("/media/" + home + "/");
    connect(&fsw, &QFileSystemWatcher::directoryChanged, &fd,
            [=, &sidebarNum, &mntUrlList, &list, &fd](const QString path) {
        QDir wmntDir(path);
        wmntDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
        QFileInfoList wfilist = wmntDir.entryInfoList();
        mntUrlList.clear();
        for (int i = 0; i < sidebarNum && i < wfilist.size(); ++i) {
            QFileInfo fi = wfilist.at(i);
            if (fi.fileName() == "2691-6AB8") /* 990 9a0 */
                     continue;
            mntUrlList << QUrl("file://" + fi.filePath());
        }
        fd.setSidebarUrls(list + mntUrlList);
        fd.update();
    });

    connect(&fd, &QFileDialog::finished, &fd, [=, &list, &fd]() {
        fd.setSidebarUrls(list);
    });
    if (b)
    {
        if (0 == ui->comboBoxDisk->currentIndex())
        {
            fd.setFileMode(QFileDialog::DirectoryOnly);
            fd.setSidebarUrls(list + mntUrlList);
            fd.move(BurnData::instance()->geometry().center()-fd.geometry().center());
            fd.setGeometry(fd.geometry());

            if (/*!path.isEmpty()*/ fd.exec() == QDialog::Accepted)
            {
                path = fd.selectedFiles().at(0);
                testPath = path + "/.t/.k";
                QDir dir(testPath);
                if (dir.mkpath(testPath))
                {
                    dir.removeRecursively();
                    browse(path);
                }
                else
                {
                    ui->btnSetting->setChecked(false);
                    return;
                }
            }
        }
        else
        {
            MenuFactory::instance()->burnSetting(b);
        }
        ui->btnSetting->setChecked(false);
    }
#endif
}

void MainWindow::slotsUpdateDisk(int idx, void *d)
{
    DataCDROM *cdrom = (DataCDROM *)d;
    ui->comboBoxDisk->setItemIcon(idx, cdrom->mediumIcon());
}


void MainWindow::slotsChangeName(int bus)
{
    if (ui->stackedWidget->currentWidget() != PAGE_CopyDisc) return;
    switch (bus)
    {
    case LogicMain::GetISO:
        ui->btnBusness->setText(_("GetISO"));
        busness(LogicMain::GetISO);
        break;
    default:
        ui->btnBusness->setText(_("CopyDisc"));
        busness(LogicMain::CopyData);
        break;
    }
}

//改btnBusness按钮的名字
void MainWindow::slotsChangeBtnBusnessName()
{
    DataCDROM *cdrom;
    cdrom = nullptr;
    int idx = ui->comboBoxDisk->currentIndex();
    //判断在【刻录数据】界面
    if (ui->stackedWidget->currentIndex() != 0) return;
    if(idx < 0 ) return;
    //判断comboxDisk选择的是光盘
    if(idx>0)
    {
        if (idx >= m_oCDROMS.size()) return;
        cdrom = static_cast<DataCDROM *>(m_oCDROMS.at(idx));
        ui->btnSetting->setText(_("Setting"));

        slotSwitchBurnFormat(cdrom);
    }
    resetText(ui->btnBusness);
}

void MainWindow::resetText(QPushButton* item)
{
    QFontMetrics fontWidth(font());//得到每个字符的宽度
    QString item_name = item->text();
    if (item->toolTip() != ""
            && -1 != item_name.indexOf("…")
            && -1 != item->toolTip().indexOf(item_name.mid(0, item_name.length() - 3)))
    {
        item_name = item->toolTip();
    }
    QString show_name = fontWidth.elidedText(item_name, Qt::ElideRight, item->width() - 20);
    item->setText(show_name);
    if (show_name.endsWith("…"))
        item->setToolTip(item_name);
    else
        item->setToolTip("");
}

bool MainWindow::started()
{
    QString comm;
    QDir dir("/proc/");
    QFileInfo file;
    ifstream in;
    std::string s;

    foreach (QFileInfo f, dir.entryInfoList())
    {
        if (f.baseName() == QString("%1").arg(getpid())
                || f.baseName() == "self" ||
                f.baseName() == "thread-self") continue;
        comm = QString("/proc/%1/comm").arg(f.baseName());
        file.setFile(comm);
        if (file.exists())
        {
            in.open(file.absoluteFilePath().toStdString());
            if (in.is_open())
            {
                getline(in, s);
                comm = QString::fromStdString(s);
                if (comm == "kylin-burner")
                    return true;
            }
            in.close();
        }
    }

    return false;
}


MainWindow::~MainWindow()
{
    //在程序退出前调用，不需要传入参数
    if(m_burnDataItem){
        delete m_burnDataItem;
        m_burnDataItem = nullptr;
    }
    if(m_burnImageItem){
        delete m_burnImageItem;
        m_burnImageItem = nullptr;
    }
    if(m_copyImageItem){
        delete m_copyImageItem;
        m_copyImageItem = nullptr;
    }
    if(m_NavBar){
        delete m_NavBar;
        m_NavBar = nullptr;
    }
    if(m_transparentSettings){
        delete m_transparentSettings;
        m_transparentSettings = nullptr;
    }

    delete ui;
}

QString MainWindow::getDeviceByIndex(int index)
{
    if (index < 0 || index > m_oCDROMS.size())
        return QString("Error");
    DataCDROM *cdrom = static_cast<DataCDROM*>(m_oCDROMS[index]);
    return cdrom->device();
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (ui->comboBoxBurner == watched || ui->comboBoxDisk == watched || ui->nav == watched ) {
        if (event->type() == QEvent::KeyPress) {
            event->ignore();
        }
        return false;
    }
//针对动态毛玻璃特效进行优化
#if UKUI_SESSION_TYPE == 2
    if (watched == this && (event->type() == QEvent::FocusIn || event->type() == QEvent::FocusOut)
            || event->type() == QEvent::ActivationChange
            || event->type() == QEvent::WindowActivate
            || event->type() == QEvent::WindowDeactivate
            || event->type() == QEvent::Resize ) {
        if ( event->type() == QEvent::WindowActivate ) {
            // 窗口被激活
            initTransparent();
            m_windowHelper->setBlurEffect(ui->nav->rect(), 0, true);
        } else if(event->type() == QEvent::WindowDeactivate){
            // 窗口失去激活
            m_transparentValue = 1.0;
            m_windowHelper->setBlurEffect(ui->nav->rect(), 0, false);
        } else if(event->type() == QEvent::Resize){
            // 窗口尺寸变化，重新设置毛玻璃特效区域
            if (isActiveWindow()) {
                m_windowHelper->setBlurEffect(ui->nav->rect(), 0, true);
            } else {
                m_windowHelper->setBlurEffect(ui->nav->rect(), 0, false);
            }
        }
        update();
    }
#endif
    return QMainWindow::eventFilter(watched, event);
}

/** 获取并监控透明度变化 */
void MainWindow::initTransparent(){
    m_transparentValue = 1.0;
    m_transparentSettings = nullptr;

    if(QGSettings::isSchemaInstalled(TRANSPARENT_SCHEMAS))
        m_transparentSettings = new QGSettings(TRANSPARENT_SCHEMAS);

    if(m_transparentSettings->keys().contains("transparency")){
        m_transparentValue = m_transparentSettings->get("transparency").toString().toDouble();
        connect(m_transparentSettings, &QGSettings::changed, this, [=](const QString& key){
#if UKUI_SESSION_TYPE == 2
            //为了满足显示效果禁用动态的透明度变化
#else
            m_transparentValue = m_transparentSettings->get("transparency").toString().toDouble();
            repaint();
#endif
        });
    }
}

/** 计算刻录数据、刻录镜像、拷贝镜像不同页面停留时间
 * 1. current是当前页面索引值0,1,2
 * 2. show_time是切换页面之前页面的停留时间
*/
void MainWindow::calculatorTime(int current)
{
    if((current < 0) || (current > 2)){
        return;
    }
    for(auto &pair : m_mapStartTime){
        if(pair.second != 0)
        {
            qint64 show_time = QDateTime::currentMSecsSinceEpoch() - pair.second;
            qDebug() << "页面" << pair.first << "停留时间" << show_time << "ms";
            TrackingManager::instance()->recordPageDwellTime(static_cast<TrackingManager::PageType>(pair.first), show_time);
            m_mapStartTime[pair.first] = 0;
        }
    }
    m_mapStartTime[current] = QDateTime::currentMSecsSinceEpoch();
}

/** 调整左侧导航栏栏ui
 * 1. 开启左侧栏毛玻璃效果
 * 2. 左侧栏使用KNavigationBar类
*/
void MainWindow::uiAdjust(){
    m_NavBar = nullptr;
    m_burnDataItem = nullptr;
    m_burnImageItem = nullptr;
    m_copyImageItem = nullptr;
#if UKUI_SESSION_TYPE == 2
    //session type2 disable useSystemStyleBlur
#else
    setProperty("useSystemStyleBlur",true);     //毛玻璃属性
#endif
    setAttribute(Qt::WA_TranslucentBackground); //设置整个窗体透明
    initTransparent();                          //使用系统透明度

    ui->splitter->setHandleWidth(0);            //设置分隔符宽度为0
    ui->splitter->setCollapsible(0,false);      //设置分隔符不可拖动

    ui->nav->setFixedWidth(200);                //左侧栏宽度固定
    ui->nav->setContentsMargins(0,0,0,0);
    ui->nav->setAutoFillBackground(false);      //关闭左侧栏背景自动填充

    /** 左侧栏改用KNavigationBar实现
     *  FIXME: KNavigationBar+QStandardItem会存在失去焦点后按钮配色异常的问题，后期应该由sdk进行修复
    */
    ui->buttons->hide();
    m_NavBar = new KNavigationBar(ui->nav);
    m_NavBar->setAccessibleName("kylin-burner_KNavigationBar_MainWindow_NavBar");
    m_NavBar->setGeometry(ui->buttons->x()+10,55,180,300);
    QListView* mViewOfNavBar = m_NavBar->listview();
    mViewOfNavBar->setFocus();

    m_burnDataItem = new QStandardItem(QIcon::fromTheme("ukui-burn-data-symbolic"),_("BurnData"));
    m_burnImageItem = new QStandardItem(QIcon::fromTheme("ukui-burn-mirror-symbolic"),_("BurnImage"));
    m_copyImageItem = new QStandardItem(QIcon::fromTheme("ukui-burn-cd-symbolic"),_("CopyDisk"));

    m_NavBar->addItem(m_burnDataItem);
    m_NavBar->addItem(m_burnImageItem);
    m_NavBar->addItem(m_copyImageItem);

    QModelIndex helloIndex = mViewOfNavBar->model()->index(0,0, QModelIndex());  //获取首元素的index
    mViewOfNavBar->setCurrentIndex(helloIndex);                                  //默认选中首元素
    connect(mViewOfNavBar, &QListView::pressed, this, &MainWindow::slotChangeStackWindowIndex);
    m_mapStartTime[0] = QDateTime::currentMSecsSinceEpoch();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    // 快捷键F1 ：帮助
    if(event->key() == Qt::Key_F1)
    {

        struct passwd *pwd = getpwuid(getuid());
        msg = QDBusMessage::createMethodCall( QString("com.kylinUserGuide.hotel_%1")
                                              .arg(pwd->pw_gid),"/", "com.guide.hotel",
                                              "showGuide");
        msg << "kylin-burner";
        msg.setDelayedReply(false);
        QDBusConnection::sessionBus().call(msg);
        return;
    }

    /***
     * 文件类快捷键操作部分
     *
     * 针对当前刻录内不支持udf类型光盘的追加刻录功能
     * 所以对快捷键的响应进行限制
     */

    if ((event->modifiers() == Qt::AltModifier) && (event->key() == Qt::Key_N)) {
        emit g_user_signal->sigShiftPlusNByKeyPress(true);
        return;
    }

    // 快捷键Delete : 删除指定文件
    if (event->key() == Qt::Key_Delete) {
        emit g_user_signal->sigDeleteByKeyPress(true);
        return;
    }

    // 快捷键Alt+C : 清空所有文件
    if ((event->modifiers() == Qt::AltModifier) && (event->key() == Qt::Key_C)) {
        emit g_user_signal->sigDeleteAllByKeyPress(true);
        return;
    }

    // 快捷键Alt+A : 添加文件
    if ((event->modifiers() == Qt::AltModifier) && (event->key() == Qt::Key_A)) {
        emit g_user_signal->sigAddFileByKeyPress(true);
        return;
    }

    QMainWindow::keyPressEvent(event);
}

bool MainWindow::currentDataRomIsUDF()
{
    bool ret = false;

    int currentIndexOfDisk = this->ui->comboBoxDisk->currentIndex();
    if (currentIndexOfDisk <= 0) {
        return ret;
    }

    DataCDROM *currentDataRom = static_cast<DataCDROM *>(m_oCDROMS.at(currentIndexOfDisk));
    ret = currentDataRom->isUdf();

    currentDataRom = nullptr;

    return ret;
}

void MainWindow::uiModeChange(bool is_tabletmode)
{
    // 是否平板模式，平板模式
    m_isTabletMode = is_tabletmode;
    ui->burnerTitle->setIsTabletMode(is_tabletmode);
}

void MainWindow::slotsCloseApp()
{
#ifndef UNIT_TEST
    this->hide();
    calculatorTime(m_current_page);
    LOGGER->info(QString("[%1] [%2] entrace  slotsCloseApp").arg(__func__).arg(__LINE__));
    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if(sessionBus.unregisterService("com.kylin.burner.manager"))
    {
        LOGGER->info("注销DBus成功");
        qDebug() << "注销DBus成功";
    }
    else
    {
        qDebug() << sessionBus.lastError();
        LOGGER->info(QString("注销DBus失败"));
        qDebug()<<"注销DBus失败";
    }

    BurnData::instance()->slotStopLoadingData();

    if (ViewFilterFiles::instance()) {
        ViewFilterFiles::instance()->close();
    }
    if (FileFilterRules::instance()) {
        FileFilterRules::instance()->close();
    }
    LogicMain::instance()->destroy();
    LOGGER->info(QString("[%1] [%2] 析构结束").arg(__func__).arg(__LINE__));

    qApp->exit();
#endif
}

void MainWindow::slotsChangeCurrentIndex(int idx)
{
    ui->comboBoxBurner->setCurrentIndex(idx);
    ui->comboBoxDisk->setCurrentIndex(idx);

    if (ui->stackedWidget->currentIndex() != 0)
        return;
    if(idx < 0 )
        return;
    if (idx >= m_oCDROMS.size())
        return;

    emit g_user_signal->sigChangeComboDiskIndex(m_oCDROMS.at(idx));
}

void MainWindow::slotShowTileFlyout(QRect rect)
{
    m_tileFlyoutRect = QRect(
        ui->navTitle->mapFromGlobal(rect.topLeft()),
        rect.size()
    );
    m_maxBtnHoverDelayTimer->start();
}

void MainWindow::slotHideTileFlyout()
{
    m_maxBtnHoverDelayTimer->stop();
}

void MainWindow::changeEvent(QEvent *event){
    if (QEvent::WindowStateChange == event->type()) {
        Qt::WindowStates state = this->windowState();
        switch (state) {
        case Qt::WindowMaximized:
        case Qt::WindowFullScreen:
            ui->burnerTitle->onWindowShowMaxed(true);
            break;
        case Qt::WindowNoState:
            ui->burnerTitle->onWindowShowMaxed(false);
            break;
        default:
            break;
        }
    }

    QMainWindow::changeEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    slotsCloseApp();
}
