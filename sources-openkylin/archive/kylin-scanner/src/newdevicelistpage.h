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
#ifndef NEWDEVICELISTPAGE_H
#define NEWDEVICELISTPAGE_H

#include <QDialog>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QStackedWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QMessageBox>
#include <QIcon>
#include <QFileDialog>

#include "nodevicewidget.h"
#include "waittingdialog.h"
#include "custom_push_button.h"
#include "include/theme.h"
#include "customtablewidget.h"
#include "device_information.h"
#include "ukui_apt.h"
#include "globalsignal.h"
#include "deviceFinder.h"
#include "include/customlabel.h"

class newDeviceListPage : public QDialog
{
    Q_OBJECT
public:
    explicit newDeviceListPage(QList<DeviceInformation> devInfoList,QWidget *parent = nullptr);

private:
    QVBoxLayout *m_VLayout = nullptr;       // 整体layout
    QHBoxLayout *m_titleHLayout = nullptr;  // 标题栏 layout
    QPushButton *m_closeButton = nullptr;

    QStackedWidget * m_mainWidget = nullptr;
    QVBoxLayout *m_mainVLayout = nullptr;   // 除标题栏外主体layout
    QWidget *m_mainVWidget1 = nullptr;      // 用于存放mainVlayout的widget
    QVBoxLayout *m_mainVLayout2 = nullptr;   // 除标题栏外主体layout
    QWidget *m_mainVWidget2 = nullptr;      // 用于存放mainVlayout的widget

    // mainVlayout 的内容
    QHBoxLayout *m_labelbuttonLayout = nullptr; // 设备列表/重新检测按钮布局
    QLabel *m_deviceLabel = nullptr;
    QPushButton *m_reDetectButton = nullptr;
    QStackedWidget *m_stackDeviceList = nullptr;
    NoDeviceWidget *m_noDeviceWidget = nullptr;
    CustomTableWidget *m_deviceList = nullptr;
    QHBoxLayout *m_buttonHLayout = nullptr;
    QPushButton *m_cancelButton = nullptr;
    QPushButton *m_nextStepButton = nullptr;

    // mainVlayout2 的内容
    QLabel *m_selectedDeviceName = nullptr;
    CustomLabel *m_selectedDriverName = nullptr;
    QPushButton *m_cancelButton2 = nullptr;
    QPushButton *m_beforeButton = nullptr;
    QPushButton *m_installButton = nullptr;

    QHBoxLayout *m_buttonHLayout2 = nullptr;

    WaittingDialog *m_waittingDialog = nullptr;

    deviceFinder *m_deviceFinder = nullptr;
    QList<DeviceInformation> m_devInfoList;
    QMap<QString, QString> m_nameToLever;
    DeviceInformation m_waitForInstallOne;
    AptUtilHelper *m_aptHelper = nullptr;
    DebUtilHelper *m_debHelper = nullptr;
    QMessageBox *m_msg;

    void init();
    void initConnect();
    void handleData();

public Q_SLOTS:
    void nextButtonClickSlot();
    void installDriverSlot();

    void showSuccessDialog();
    void showFailDialog(QString err);
    void reDetectDevicesSlot();
    void installHandlerSlot();
public:

};



#endif // NEWDEVICELISTPAGE_H
