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
#ifndef MainWidget_H
#define MainWidget_H

#include <QScrollArea>
#include <QScreen>
#include <QStringList>
#include <QApplication>
#include <QBrush>
#include <QColor>
#include <QEvent>
#include <QList>
#include <QMessageBox>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QPalette>
#include <QStackedLayout>
#include <QPainter>
#include <QKeyEvent>
#include <QPainterPath>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QStyleOption>
#include <QVBoxLayout>
#include <QWidget>
#include <QX11Info>
#include <QThread>
#include <QInputDialog>
#include <qmath.h>
#include "utils/HorizontalOrVerticalMode.h"
#include <QCommandLineOption>
#include <QCommandLineParser>
#include "runningdialog.h"
#include "scandialog.h"
#include "displaywidget.h"
#include "include/common.h"
#include "include/theme.h"
#include "globalsignal.h"
#include "usb.h"
#include "rectify.h"
#include "usbhotplugthread.h"
#include <sane/sane.h>
#include "waittingdialog.h"
#include "deviceFinder.h"
#include "newdevicelistpage.h"
#include "Qt/windowmanage.hpp"
#include "kwidget.h"
#include <libusb-1.0/libusb.h>

class ScanThread : public QThread
{
    Q_OBJECT

protected:
    void run() Q_DECL_OVERRIDE;

Q_SIGNALS:
    void scanThreadFinishedSignal(int);
};

class DetectScanDevicesThread : public QThread
{
    Q_OBJECT
public:
    void run() Q_DECL_OVERRIDE;


Q_SIGNALS:
    void detectScanDevicesFinishedSignal(bool);
};



class RectifyThread : public QThread
{
    Q_OBJECT
public:
    explicit RectifyThread(QObject *parent = 0);
    ~RectifyThread();

    void run() Q_DECL_OVERRIDE;

    void rectifyThreadStop();
    void waitDialog();

};

class MainWidget : public kdk::KWidget
{
    Q_OBJECT

public:
    MainWidget(QWidget *parent = nullptr);
    ~MainWidget();

    void setupGui();
    void initConnect();
    void initGsettings();
    void setMenuContent();
    void initcontent();
//    void resizeTitleBar();
    void resizeDisplayWidget();
    void fontSizeChange();
    void transparencyChange();

    kdk::WindowId getWindowId();
    void reboot();

    static int const EXIT_CODE_REBOOT;
    kdk::WindowId windowId = 0;

protected:
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void keyReleaseEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void closeEvent(QCloseEvent *event);

private:
    QDBusInterface *server = nullptr;
    QDBusMessage serverReturnValue;

    quint32 flag;

//    TitleBar *m_titleBar = nullptr;
    DisplayWidget *m_displayWidget = nullptr;

    QVBoxLayout *m_mainWidgetVLayout = nullptr;

    DetectScanDevicesThread m_detectScanDevicesThread;
    UsbHotplugThread m_usbHotplugThread;

    ScanThread m_scanThread;
    ScanDialog *m_scanDialog = nullptr;
    WatermarkDialog *m_watermarkDialog = nullptr;

    RunningDialog *m_beautyRunningDialog = nullptr;

    RunningDialog *m_rectifyRunningDialog = nullptr;
    RectifyThread m_rectifyThread;

    bool cancelFlag = false;
    QList<QMessageBox*> m_msgBoxList;
    QMessageBox *m_sleepWarningMsgBox = nullptr;  // 睡眠/休眠提示消息框
    bool m_isShowingSleepWarning = false;  // 标记是否正在显示睡眠/休眠提示消息框，防止重复创建

    WaittingDialog *m_waittingDialog = nullptr;

    deviceFinder *m_devFinder = nullptr;
    QList<DeviceInformation> m_noDriverDevices;
    newDeviceListPage *m_deviceListPage = nullptr;

    void showHelpDialog();

    void loadAndSave(QStringList loadPath);
public slots:
    void rotationChanged(bool isPCMode,int width,int height);
    void warnMsg(QString msg);
    void maximizeWindowSlot();
    void closeWindowSlot();
    void detectScanDevicesSlot();
    void detectScanDeviceThreadFinishedSlot(bool isDetected);


    void startScanOperationSlot();
    void showScanDialogSlot();
    void closeScanDialogSlot();
    void scanThreadFinishedSlot(int saneStatus);

    void showBeautyRunningDialog();
    void hideBeautyRunningDialog();

    void showRectifyRunningDialog();
    void startRectifyOperationSlot();
    void isRejectedRectifySlot();
    void hideRectifyRunningDialog(bool isTrue);

    void dbusInhabitSlot();
    void dbusUnInhabitSlot();

    void cancelScanningSlot();
    void onPrepareForSleep(bool isSleep);

    void newUsbConnectedSlot();
    void startUsbHotPlugThreadSlot();
    void startHotPlutSlot();

    void showNewDeviceListPageSuccessSlot();
    void showNewDeviceListPageFailSlot();

    int warnCloseWindow();

};


#endif // MainWidget_H
