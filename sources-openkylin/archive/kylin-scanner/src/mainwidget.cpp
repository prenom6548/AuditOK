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

#include "mainwidget.h"
#include "about/custom_about_dialog.h"
#define cimg_use_jpeg
#define cimg_use_png

#include <QMessageBox>
#include <QWidgetList>
#include <QPropertyAnimation>
#include <QTimer>
#include <kysdk/applications/gsettingmonitor.h>
#include "Qt/windowmanage.hpp"
#include <kysdk/applications/usermanual.h>
#include <KWindowEffects>
#include <kysdk/kysdk-system/libkylockscreen.h>
#include "usb.h"
#include <CImg.h>
#include <QStringList>

int const MainWidget::EXIT_CODE_REBOOT = -123456789;

MainWidget::MainWidget(QWidget *parent)
    : kdk::KWidget(parent)
{
    this->setMinimumSize(MainWidgetWidth, MainWidgetHeight);

    // Center window
    QScreen *screen = QGuiApplication::primaryScreen();
    move((screen->geometry().width() - MainWidgetWidth) / 2, (screen->geometry().height() - MainWidgetHeight) / 2);
}

void MainWidget::initcontent()
{
    m_displayWidget = new DisplayWidget(this);
    m_mainWidgetVLayout = new QVBoxLayout(this);
    setupGui();
    initConnect();
    initGsettings();

    if (g_config_signal->m_kylinScannerImageDebug) {
        scanThreadFinishedSlot(SANE_STATUS_GOOD);
    } else {
        g_sane_object->hotplug_sock = create_uevent_socket();
        m_detectScanDevicesThread.start();
    }
    this->setAttribute(Qt::WA_AlwaysShowToolTips);
}

MainWidget::~MainWidget()
{
    // 析构时停止扫描读取，但不强制退出
    // 退出应该由 closeWindowSlot() 正常处理
    if (g_sane_object) {
        g_sane_object->stopSaneRead(true);
    }
    // 不再使用 exit(0)，让正常的关闭流程处理退出
}
void MainWidget::rotationChanged(bool isPCMode,int width,int height){
    GlobalUserSignal::getInstance()->setCurrentMode(isPCMode);
    emit GlobalUserSignal::getInstance()->rotationChangedSig(isPCMode);
}
void MainWidget::setupGui()
{
    this->setIcon("kylin-scanner");
    this->setWidgetName(QApplication::tr("Scanner"));
    this->setObjectName("MainWindow");
    this->setMinimumSize(MainWidgetWidth, MainWidgetHeight);
    this->setLayoutType(kdk::LayoutType::VerticalType);
    setMenuContent();

    m_mainWidgetVLayout->addWidget(m_displayWidget);
    m_mainWidgetVLayout->setSpacing(0);
    m_mainWidgetVLayout->setContentsMargins(0, 0, 0, 0);
    baseBar()->setLayout(m_mainWidgetVLayout);
    // Center window
    QScreen *screen = QGuiApplication::primaryScreen();
    move((screen->geometry().width() - MainWidgetWidth) / 2, (screen->geometry().height() - MainWidgetHeight) / 2);
}

void MainWidget::setMenuContent()
{
    kdk::KWindowButtonBar *bar = this->windowButtonBar();
    kdk::KMenuButton *menu_btn = bar->menuButton();
    QMenu *menu = menu_btn->menu();
    menu->removeAction(menu_btn->themeAction());
    menu->removeAction(menu_btn->settingAction());
    QAction *refreshAction = new QAction(tr("Refresh List"), this);
    menu->insertAction(menu_btn->assistAction(), refreshAction);
    connect(refreshAction, &QAction::triggered, [=](){
        if(!g_sane_object->isOnScanning && g_sane_object->onDetection == false){
            // 1、关闭热插拔
            emit g_user_signal->setExitFlagTrueSignal();
            // 2、检测设备 & 打开热插拔
            // 正在刷新列表，请等待刷新成功信息...
            QString msg = QApplication::tr("Refreshing list. Please wait for the refresh success information...");
            g_user_signal->warnMsg(msg);
            g_user_signal->setScanIconDisableSignal();
            QTimer::singleShot(2000, this, [this]() {emit g_user_signal->refreshListSignal();});
        }else{
            // 弹窗告诉用户扫描期间不支持识别设备
            QString msg = QApplication::tr("Scanner is on detecting...");
            g_user_signal->warnMsg(msg);
        }
    });

    connect(menu_btn->assistAction(), &QAction::triggered, [=](){
        kdk::UserManual userManualTest;
        if (!userManualTest.callUserManual("kylin-scanner")) {
            qCritical() << "user manual call fail!";
        }
    });

    connect(menu_btn->aboutAction(), &QAction::triggered, [=](){
        CustomAboutDialog dialog(this);
        dialog.exec();
    });
}

void MainWidget::initConnect()
{
    //三联按钮响应
    connect(g_user_signal, &GlobalUserSignal::minimumWindowSignal, this, &MainWidget::showMinimized);
    connect(g_user_signal, &GlobalUserSignal::maximumWindowSignal, this, &MainWidget::maximizeWindowSlot);
    connect(g_user_signal, &GlobalUserSignal::exitApplicationSignal, this, &MainWidget::closeWindowSlot);
    //扫描设备
    connect(m_displayWidget, &DisplayWidget::detectScanDevicesSignal, this, &MainWidget::detectScanDevicesSlot, Qt::QueuedConnection);
    connect(&m_detectScanDevicesThread, &DetectScanDevicesThread::detectScanDevicesFinishedSignal, this, &MainWidget::detectScanDeviceThreadFinishedSlot);
    connect(g_user_signal, &GlobalUserSignal::startUsbHotPlugThreadSignal, this, &MainWidget::startUsbHotPlugThreadSlot);
    //扫描过程
    connect(g_user_signal, &GlobalUserSignal::startScanOperationSignal, this, &MainWidget::startScanOperationSlot, Qt::QueuedConnection);
    connect(g_user_signal, &GlobalUserSignal::scanThreadFinishedSignal, this, &MainWidget::scanThreadFinishedSlot);
    connect(g_user_signal, &GlobalUserSignal::cancelScanning, this, &MainWidget::cancelScanningSlot);
    connect(g_user_signal, &GlobalUserSignal::closeScanDialogSignal, this, &MainWidget::closeScanDialogSlot);
    //阻止锁屏
    connect(g_user_signal, &GlobalUserSignal::dbusInhabitSignal, this, &MainWidget::dbusInhabitSlot);
    connect(g_user_signal, &GlobalUserSignal::dbusUnInhabitSignal, this, &MainWidget::dbusUnInhabitSlot);
    connect(g_user_signal, &GlobalUserSignal::warnMsgSignal,this,&MainWidget::warnMsg);
    //锁屏信号
    QDBusConnection::systemBus().connect(QString("org.freedesktop.login1"), QString("/org/freedesktop/login1"),
                                         QString("org.freedesktop.login1.Manager"), QString("PrepareForSleep"), this,
                                         SLOT(onPrepareForSleep(bool)));
    connect(g_user_signal, &GlobalUserSignal::closeUsbHotPlugThreadSignal, [=](){m_usbHotplugThread.exitWindowFlag = true;});
    connect(g_user_signal, &GlobalUserSignal::startHotPlugSignal, this, &MainWidget::startHotPlutSlot);
    connect(g_user_signal, &GlobalUserSignal::hotPlugScanCompleteSignal, this, [=](){g_sane_object->hotPlugScanCompleteSlot();});

    //字体，透明度改变
    connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemFontSizeChange, this, &MainWidget::fontSizeChange);
    connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemTransparencyChange, this, &MainWidget::transparencyChange);

    connect(g_user_signal, &GlobalUserSignal::findNoDriverDeviceSignal, this, [=](){
        m_waittingDialog = new WaittingDialog(this);
        m_waittingDialog->show();
    });

    connect(qApp, &QGuiApplication::focusWindowChanged, this, [=](QWindow *window) {
        bool active = (window == this->windowHandle());
        if (active) {
            emit g_user_signal->windowFocousSignal(true);
        } else {
            emit g_user_signal->windowFocousSignal(false);
        }

    });

}

void MainWidget::initGsettings()
{   
    transparencyChange();
    fontSizeChange();
}

void MainWidget::resizeDisplayWidget()
{
    m_displayWidget->resize(this->width(), this->height());
}

void MainWidget::fontSizeChange()
{    
    float fontSize = kdk::GsettingMonitor::getSystemFontSize().toFloat();
    qDebug() << "fontSize = " << fontSize;
}

void MainWidget::transparencyChange()
{
    double transparencyValue = kdk::GsettingMonitor::getSystemTransparency().toDouble();
    g_config_signal->m_transparency = transparencyValue * 255;
    this->update();
}

void MainWidget::warnMsg(QString msg)
{
    QMessageBox *msgBox = new QMessageBox();
    m_msgBoxList.append(msgBox);
    msgBox->setWindowModality(Qt::ApplicationModal);
    msgBox->setAttribute(Qt::WA_ShowModal, true);

    msgBox->setText(msg);
    msgBox->setIcon(QMessageBox::Warning);
    msgBox->setWindowTitle(QApplication::tr("Scanner"));
    QPushButton *okBtn = new QPushButton(msgBox);
    okBtn->setText(tr("Ok"));
    msgBox->addButton(okBtn, QMessageBox::YesRole);
    msgBox->setContextMenuPolicy(Qt::NoContextMenu);

    QWidget *widget = nullptr;
    QWidgetList widgetList = QApplication::allWidgets();
    for (int i=0; i<widgetList.length(); ++i) {
        if (widgetList.at(i)->objectName() == "MainWindow") {
            widget = widgetList.at(i);
        }
    }
    if (widget) {
        msgBox->setParent(widget);
        QRect rect = widget->geometry();
        int x = rect.x() + rect.width()/2 - msgBox->width()/2;
        int y = rect.y() + rect.height()/2 - msgBox->height()/2;
        msgBox->move(x,y);
    }

    QTimer* timer = new QTimer();
    timer->start(20000);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, msgBox, &QMessageBox::accept);
    connect(msgBox, &QMessageBox::accepted, msgBox, [=](){
        if(m_msgBoxList.contains(msgBox)){
            m_msgBoxList.removeOne(msgBox);
        }
        msgBox->deleteLater();
    });

    if(m_msgBoxList.length() > 1){
        for(int i = 0; i < m_msgBoxList.length(); ++i){
            if(m_msgBoxList.at(i) != msgBox){
                m_msgBoxList.at(i)->accept();
            }
        }
    }
    msgBox->exec();
}

int MainWidget::warnCloseWindow()
{
    QMessageBox *msgBox = new QMessageBox();
    msgBox->setModal(Qt::WindowModal);
    msgBox->setAttribute(Qt::WA_ShowModal, true);
    connect(msgBox, &QMessageBox::finished, msgBox, &QMessageBox::deleteLater);
    msgBox->setText(tr("The current file is not saved. Do you want to save it?"));
    msgBox->setIcon(QMessageBox::Warning);
    msgBox->setWindowTitle(tr("Scanner"));
    msgBox->setContextMenuPolicy(Qt::NoContextMenu);
    QAbstractButton *noButton = (QAbstractButton *)msgBox->addButton(tr("Straight &Exit"), QMessageBox::NoRole);
    QAbstractButton *yesButton =  (QAbstractButton *)msgBox->addButton(tr("&Save Exit"), QMessageBox::YesRole);

    msgBox->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);

    QWidget *widget = nullptr;
    QWidgetList widgetList = QApplication::allWidgets();
    for (int i=0; i<widgetList.length(); ++i) {
        if (widgetList.at(i)->objectName() == "MainWindow") {
            widget = widgetList.at(i);
        }
    }
    if (widget) {
        msgBox->setParent(widget);
        QRect rect = widget->geometry();
        int x = rect.x() + rect.width()/2 - msgBox->width()/2;
        int y = rect.y() + rect.height()/2 - msgBox->height()/2;
        msgBox->move(x,y);
    }

    msgBox->exec();

    if (msgBox->clickedButton() == yesButton) {
        msgBox->deleteLater();
        qDebug() << "Accepeted !";
        return 1;
    } else if (msgBox->clickedButton() == noButton) {
        g_user_signal->exitApplication();
        msgBox->deleteLater();
        qDebug() << "Rejected !";
        return 0;
    }
    return 0;
}

void MainWidget::reboot()
{
    qApp->exit(EXIT_CODE_REBOOT);
}

void MainWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
}


void MainWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Return:
    case Qt::Key_Enter:
    case Qt::Key_Space:
        if(g_sane_object->cropFlag){

        }else{
            if(event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter){
                if(g_sane_object->getSaneHaveHandle()){
                    GlobalUserSignal::getInstance()->scanStart();
                }
            }
        }
        break;

    case Qt::Key_Z:
        if (event->modifiers() == Qt::ControlModifier) {
            qDebug() << "pressed key(ctrl+z): " << event->key();
        }
        break;
    case Qt::Key_Escape:
        if (this->isFullScreen()) {
            this->showNormal();
        } else {
            if (this->isMaximized()) {
                this->showNormal();
            }
        }
        break;
    default:
        break;
    }
    QWidget::keyPressEvent(event);
}
void MainWidget::keyReleaseEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_F1:
        qDebug() << "F1 ==========";
        showHelpDialog();
        break;
    default:
        QWidget::keyReleaseEvent(event);
        break;
    }
    return QWidget::keyReleaseEvent(event);
}

void MainWidget::closeEvent(QCloseEvent *event)
{
    event->accept();
    qDebug() << "window close.";
    if (g_user_signal->exitWindowWithSaveFlag) {
        int saveFlag = warnCloseWindow();
        if (saveFlag == 1) {
            qDebug() << "close window save as";
            g_user_signal->closeWindowSaveAs(true);
        }else{
            event->ignore();
            return;
        }
    }
    g_user_signal->exitApplication();
}
void MainWidget::showHelpDialog()
{
    kdk::UserManual userManualTest;
    if (!userManualTest.callUserManual("kylin-scanner")) {
        qCritical() << "user manual call fail!";
    }
    return;
}

void MainWidget::loadAndSave(QStringList loadPath)
{
    for(int i = 0; i < loadPath.length(); i++){
        QFileInfo loadPathInfo(loadPath[i]);
        QString newLoadPath = loadPathInfo.absolutePath() + "/" + loadPathInfo.baseName() + ".jpg";
        QString newPNGLoadPath = loadPathInfo.absolutePath() + "/" + loadPathInfo.baseName() + ".png";
        cimg_library::CImg<unsigned char> img(QString(loadPath[i]).toLocal8Bit().data());
        if(!QFile(newLoadPath).exists()){
            img.save_jpeg(newLoadPath.toLocal8Bit().data());
            img.save_png(newPNGLoadPath.toLocal8Bit().data());
        }else{
            QFile(newLoadPath).remove();
            img.save_jpeg(newLoadPath.toLocal8Bit().data());
            img.save_png(newPNGLoadPath.toLocal8Bit().data());
        }
    }
}
void MainWidget::dbusInhabitSlot(){
    flag = kdk_set_inhibit_lockscreen("kylin-scanner" , "TestReason");
    if (flag == 0) {
        qCritical() << "set inhibit lock screen fail";
        return;
    }
    qDebug()<<"test result:inhabit";
}
void MainWidget::dbusUnInhabitSlot(){
    /* 取消禁止锁屏 */
    kdk_un_inhibit_lockscreen(flag);
    qDebug()<<"test result:un_inhabit";
}

void MainWidget::maximizeWindowSlot()
{
    if (this->isFullScreen()) {
        this->showNormal();
    } else {
        if (this->isMaximized()) {
            this->showNormal();
        } else {
            this->showMaximized();
        }
    }
}
kdk::WindowId MainWidget::getWindowId(){
    windowId = this->winId();
    return windowId;
}
void MainWidget::closeWindowSlot()
{
    if(g_sane_object->handle){
    sane_close(g_sane_object->handle);
    }
    exit(0);
}

void MainWidget::detectScanDevicesSlot()
{
    if (! m_detectScanDevicesThread.isRunning()) {
        qDebug() << "Not running detect scan devices thread. Let's detect ...";
        m_detectScanDevicesThread.start();
        g_user_signal->detectPageWaitTimerStart();
    }
}

void MainWidget::detectScanDeviceThreadFinishedSlot(bool isDetected)
{
    g_user_signal->detectPageWaitTimerStop();

    if (isDetected) {
        // while detect scan device thread finished, we should open the first device to get scan parameters
        g_sane_object->openSaneDeviceForPage(0);
        m_displayWidget->showSuccessPageSlot(isDetected);
    } else {
        emit g_user_signal->findNoDriverDeviceSignal();  // 展示正在检测扫描仪模态窗口
        m_displayWidget->showFailedPageSlot();
        m_devFinder = new deviceFinder();
        QObject::connect(m_devFinder, &deviceFinder::succeed, this, &MainWidget::showNewDeviceListPageSuccessSlot);
        QObject::connect(m_devFinder, &deviceFinder::succeed, this, &MainWidget::showNewDeviceListPageFailSlot);
        QObject::connect(m_devFinder, &deviceFinder::succeed, [=](){g_sane_object->onDetection = false;});
        QObject::connect(m_devFinder, &deviceFinder::failed, [=](){g_sane_object->onDetection = false;});

        g_sane_object->onDetection = true;
        m_devFinder->startWorker();
    }
}

static int getSubStringNumber(QString src, QString sub)
{
    QStringList srclist = src.split(sub);
    qDebug() << "srclist = " << srclist
             << "srclist.size = " << srclist.size();
    return srclist.size()-1;
}



void MainWidget::startScanOperationSlot()
{
    g_sane_object->ocrFlag = 0;

    g_user_signal->exitWindowWithSaveFlag = false;

    g_sane_object->stopSaneRead(false);

    if (m_scanThread.isRunning()) {
        // complicate click scan button, will be crashed
        qDebug() << "complicate click scan button.";
        return;
    }
    m_scanThread.start();

    showScanDialogSlot();
}

void MainWidget::showScanDialogSlot()
{
    m_scanDialog = new ScanDialog();
    m_scanDialog->setAttribute(Qt::WA_DeleteOnClose);

    QPoint globalPos = this->mapToGlobal(QPoint(0, 0));
    m_scanDialog->move(globalPos.x() + (this->width() - m_scanDialog->width())/2,
                       globalPos.y() + (this->height() - m_scanDialog->height())/2);

    m_scanDialog->exec();
}

void MainWidget::closeScanDialogSlot()
{
    if(m_scanDialog && m_scanDialog->isVisible()){
        m_scanDialog->close();
    }
}

void MainWidget::scanThreadFinishedSlot(int saneStatus)
{
    qDebug() << "saneStatus: " << saneStatus;

    if (saneStatus != SANE_STATUS_GOOD) {
        if (saneStatus == SANE_STATUS_INVAL) {
            warnMsg(tr("Invalid argument, please change arguments or switch other scanners."));
            qDebug() << "Invalid argument, please change arguments or switch other scanners. error code:" << QString::number(saneStatus);
        } else if (saneStatus == SANE_STATUS_EOF) {
            g_sane_object->haveScanSuccessImage = true;
            if(g_sane_object->loadFullScanFileNames.length() != 0){
                m_displayWidget->showSuccessImageHandlePageSlot();
            }
            if(cancelFlag){
                QString loadPath = g_sane_object->loadFullScanFileName;
                if(QFile::remove(loadPath)){
                    qDebug()<<"delete current image :" << loadPath <<" success!";
                }
                QString savePath = g_sane_object->saveFullScanFileName;
                if(QFile::remove(savePath)){
                    qDebug()<<"delete current image :" << savePath <<" success!";
                }
                cancelFlag = false;
                if(ThumbnailWidget::scanPictureIsEmpty){
                    emit g_user_signal->changeToConnectuccessPageSignal();
                }
            }else{
                if(g_sane_object->loadFullScanFileNames.length() != 0){
                    g_sane_object->scanPageNumber = g_sane_object->saveFullScanFileNames.length();
                    loadAndSave(g_sane_object->loadFullScanFileNames);
                    g_user_signal->scanThreadFinishedImageLoad(g_sane_object->loadFullScanFileNames, g_sane_object->saveFullScanFileNames);                   
                    ThumbnailWidget::scanPictureIsEmpty = false;
                }
            }
            qDebug() << "no more file to read!";

        } else if (saneStatus == SANE_STATUS_DEVICE_BUSY) {
            closeScanDialogSlot();
            warnMsg(tr("Device busy, please wait or switch other scanners."));
            qDebug() << "Device busy, please wait or switch other scanners. error code:" << QString::number(saneStatus);
        } else if(saneStatus == SANE_STATUS_NO_DOCS) {
            closeScanDialogSlot();
            warnMsg(tr("Document feeder out of documents, please place papers and scan again."));
            qDebug() << "Document feeder out of documents, please place papers and scan again. error code:" << QString::number(saneStatus);
        } else if(saneStatus == SANE_STATUS_CANCELLED) {
            closeScanDialogSlot();
            warnMsg(tr("Scan operation has been cancelled."));
            qDebug() << "Scan operation has been cancelled. error code:" << QString::number(saneStatus);
            cancelFlag = false;
        } else if(saneStatus == SANE_STATUS_UNSUPPORTED){
            warnMsg(tr("Scan failed, operation is not supported."));
            qDebug() << "Scan failed, operation is not supported. error code:" << QString::number(saneStatus);
        }else if(saneStatus == SANE_STATUS_JAMMED){
            warnMsg(tr("Scan failed, Document fedder jammed."));
            qDebug() << "Scan failed, Document fedder jammed. error code:" << QString::number(saneStatus);
        }else if(saneStatus == SANE_STATUS_IO_ERROR){
            warnMsg(tr("Scan failed, Error during device I/O."));
            qDebug() << "Scan failed, Error during device I/O. error code:" << QString::number(saneStatus);
        }else if(saneStatus == SANE_STATUS_NO_MEM){
            warnMsg(tr("Scan failed, Out of memory."));
            qDebug() << "Scan failed, Out of memory. error code:" << QString::number(saneStatus);
        }else if(saneStatus == SANE_STATUS_ACCESS_DENIED){
            warnMsg(tr("Scan failed, Access to resource has been denied."));
            qDebug() << "Scan failed, Access to resource has been denied. error code:" << QString::number(saneStatus);
        }else if(saneStatus == SANE_STATUS_COVER_OPEN){
            warnMsg(tr("Scan failed, Scanner cover is open."));
            qDebug() << "Scan failed, Scanner cover is open. error code:" << QString::number(saneStatus);
        }else{
            qDebug() << "met error! saneStatus:" << saneStatus;
            warnMsg(tr("Scan failed, please check your scanner or switch other scanners. If you want to continue using the scanner, click Options, refresh the list to restart the device."));
            qDebug() << "Scan failed, please check your scanner or switch other scanners. If you want to continue using the scanner, click Options, refresh the list to restart the device." << QString::number(saneStatus);
        }
    } else {
        g_sane_object->haveScanSuccessImage = true;

        m_displayWidget->showSuccessImageHandlePageSlot();
        if(cancelFlag){
            QString loadPath = g_sane_object->loadFullScanFileName;
            if(QFile::remove(loadPath)){
                qDebug()<<"delete current image :" << loadPath <<" success!";
            }
            QString savePath = g_sane_object->saveFullScanFileName;
            if(QFile::remove(savePath)){
                qDebug()<<"delete current image :" << savePath <<" success!";
            }
            cancelFlag = false;
            if(ThumbnailWidget::scanPictureIsEmpty){
                emit g_user_signal->changeToConnectuccessPageSignal();
            }
        }else{
            g_sane_object->scanPageNumber = g_sane_object->saveFullScanFileNames.length();

            loadAndSave(g_sane_object->loadFullScanFileNames);
            g_user_signal->scanThreadFinishedImageLoad(g_sane_object->loadFullScanFileNames, g_sane_object->saveFullScanFileNames);
            ThumbnailWidget::scanPictureIsEmpty = false;
        }
        // to do: update thumbtailwidget icon to current filepath
    }
}

void MainWidget::cancelScanningSlot(){
    cancelFlag = true;
    g_sane_object->stopSaneReadFlag = true;
}

void MainWidget::onPrepareForSleep(bool isSleep)
{
    if (isSleep) //睡眠分支
    {
        qDebug() << "睡眠！！！";
    } else //唤醒分支
    {
        qDebug() << "唤醒！！！";
        // 提示用户  检测到休眠/睡眠，请检测扫描仪状态，若未继续扫描，需手动点击取消或刷新列表恢复设备扫描功能
        QMessageBox *msg = new QMessageBox();
        msg->setFixedSize(450, 210);
        QString tipsStr = tr("System sleep/sleep detected. To ensure normal use of the scanner, please click on restart to restart the scanner application. If you want to continue operating the scanned image, click cancel, but the scanning related functions will be disabled and will take effect after restarting.");
        msg->setText(tipsStr);
        msg->setIcon(QMessageBox::Icon::Information);
        QPushButton *cancleBtn = new QPushButton;
        QPushButton *useBtn = new QPushButton;
        cancleBtn = msg->addButton(tr("Cancel"), QMessageBox::RejectRole);
        useBtn = msg->addButton(tr("Restart"), QMessageBox::AcceptRole);

        connect(useBtn, &QPushButton::clicked, [](){
            QProcess::startDetached(qApp->applicationFilePath());
            qApp->quit();
        });
        connect(cancleBtn, &QPushButton::clicked, g_user_signal, &GlobalUserSignal::updateSettingBtnSignal);
        connect(cancleBtn, &QPushButton::clicked, msg, &QMessageBox::close);
        connect(cancleBtn, &QPushButton::clicked, msg, &QMessageBox::deleteLater);
        msg->show();

        QWidget *widget = nullptr;
        QWidgetList widgetList = QApplication::allWidgets();
        for (int i=0; i<widgetList.length(); ++i) {
            if (widgetList.at(i)->objectName() == "MainWindow") {
                widget = widgetList.at(i);
            }
        }
        if (widget) {
            QRect rect = widget->geometry();
            int x = rect.x() + rect.width()/2 - msg->width()/2;
            int y = rect.y() + rect.height()/2 - msg->height()/2;
            msg->move(x,y);
        }
    }
}

void MainWidget::newUsbConnectedSlot()
{
    QMessageBox::information(this, tr("Alert"), tr("A new Scanner has been connected."));
}

void MainWidget::startUsbHotPlugThreadSlot()
{
    m_usbHotplugThread.exitWindowFlag = false;
    while(1){
        if(m_detectScanDevicesThread.isFinished()){
            m_usbHotplugThread.start();
            break;
        }
    }
}

void MainWidget::startHotPlutSlot()
{
    emit g_user_signal->setExitFlagFalseSignal();
    m_usbHotplugThread.start();
}

void MainWidget::showNewDeviceListPageSuccessSlot()
{
    qDebug() << "Install succeed";
    m_waittingDialog->close();
    // 初始化页面并，解析内容并展示
    m_noDriverDevices.clear();
    m_noDriverDevices = m_devFinder->getList();
    if(m_noDriverDevices.length() != 0){
        m_deviceListPage = new newDeviceListPage(m_noDriverDevices, this);
    }
    m_devFinder->finished();
}

void MainWidget::showNewDeviceListPageFailSlot()
{
    qDebug() << "Install failed";
}

void MainWidget::showBeautyRunningDialog()
{
    m_beautyRunningDialog = new RunningDialog(this, tr("Running beauty ..."));

    QPoint globalPos = this->mapToGlobal(QPoint(0, 0));
    int m_x = (this->width() - m_beautyRunningDialog->width()) / 2;
    int m_y = (this->height() - m_beautyRunningDialog->height()) / 2;

    m_beautyRunningDialog->move(globalPos.x() + m_x, globalPos.y() + m_y);

    m_beautyRunningDialog->exec();


}

void MainWidget::hideBeautyRunningDialog()
{
    m_beautyRunningDialog->hide();
    m_beautyRunningDialog->reject();
}

void MainWidget::showRectifyRunningDialog()
{
    m_rectifyRunningDialog = new RunningDialog(this, tr("Running rectify ..."));

    QPoint globalPos = this->mapToGlobal(QPoint(0, 0));
    int m_x = (this->width() - m_rectifyRunningDialog->width()) / 2;
    int m_y = (this->height() - m_rectifyRunningDialog->height()) / 2;

    m_rectifyRunningDialog->move(globalPos.x() + m_x, globalPos.y() + m_y);

    m_rectifyRunningDialog->setModal(true);
    m_rectifyRunningDialog->show();

    connect(m_rectifyRunningDialog, &RunningDialog::rejected, this, &MainWidget::isRejectedRectifySlot);

    g_user_signal->doRectifyOperation();
}

void MainWidget::startRectifyOperationSlot()
{
    m_rectifyThread.start();
}

void MainWidget::isRejectedRectifySlot()
{
    qDebug() << "rectify is reject.";
    if (! m_rectifyRunningDialog->isHidden()) {
        m_rectifyRunningDialog->hide();
        m_rectifyRunningDialog->reject();
    }
}

void MainWidget::hideRectifyRunningDialog(bool isTrue)
{
    qDebug() << "hide rectify dialog";
    if (! m_rectifyRunningDialog->isHidden()) {
        m_rectifyRunningDialog->hide();
        m_rectifyRunningDialog->reject();
    }
}

void DetectScanDevicesThread::run()
{
    qDebug() << "FindScanDevicesThread begin";

    bool res = g_sane_object->detectSaneDevices();

    if (! res) {
        emit detectScanDevicesFinishedSignal(false);
        emit g_user_signal->closeUsbHotPlugThreadSignal();
        qDebug() << "detect scan devices finished: false.";
    } else {
        emit detectScanDevicesFinishedSignal(true);
        qDebug() << "detect scan devices finished: true.";
        emit g_user_signal->startUsbHotPlugThreadSignal();
    }
}


void ScanThread::run()
{
    g_sane_object->isOnScanning = true;
    int ret = 0;
    g_sane_object->scanPageNumber = 0;
    g_sane_object->loadFullScanFileNames.clear();
    g_sane_object->saveFullScanFileNames.clear();

    QString pageNumber = g_sane_object->userInfo.pageNumber;

    emit g_user_signal->dbusInhabitSignal();      // 阻止扫描过程中睡眠

    qDebug() << "start_scanning start";
    ret = g_sane_object->startScanning(g_sane_object->userInfo);
    qDebug() << "start_scanning end, status = " << ret;

    int saneStatus = ret;

    g_user_signal->scanThreadFinished(ret);

    emit g_user_signal->dbusUnInhabitSignal();

    emit g_user_signal->cancelOprationOKSignal();
    quit();
}


RectifyThread::RectifyThread(QObject *parent)
    : QThread(parent)
{
    qDebug() << "Create rectify thread.";
}

RectifyThread::~RectifyThread()
{
}

void RectifyThread::run()
{
}

void RectifyThread::rectifyThreadStop()
{
    if(isRunning())
    {
        terminate();
        wait();
    }
}
