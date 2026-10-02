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
#ifndef __ADD_PRINTER_WINDOW_H__
#define __ADD_PRINTER_WINDOW_H__

#include <QThread>
#include <QStandardItemModel>
#include <QMap>

#include "custom_dialog.h"

#include "custom_refresh_button.h"

#include "ukui_printer.h"
#include "device_information.h"

class QTableView;
class QComboBox;
class QPoint;
class QStackedWidget;
class QLineEdit;
class AutoSearchResultModel;
class ManualSearchResultModel;
class SortFilterProxyModel;
class CustomPushButton;
class CustomLabel;
class ChoosePpdComboBox;


class getUsbPrinterThread:public QObject{
    Q_OBJECT
public:
    explicit getUsbPrinterThread(QObject *parent = nullptr);
    ~getUsbPrinterThread(){}
public:
    void startWorker(){
        if(m_thread!=nullptr)
            m_thread->start();
    }
private:
    QThread *m_thread = nullptr;
    void startSearch();
signals:
    void finished();
    void recvUsbPrinter(DeviceInformation&);
};

QT_BEGIN_NAMESPACE
namespace Ui { class AddPrinterWindow; }
QT_END_NAMESPACE

class AddPrinterWindow : public CustomDialog
{
    Q_OBJECT
public:
    explicit AddPrinterWindow(QPoint pos, QWidget *parent = nullptr);
    ~AddPrinterWindow();
    void setQSSFontSize(QFont curFont);
private:
    void initUi();
    void initConnect();
    //开关按钮
    void initSwitchArea();
    //添加设备区域
    void initCentralWidget();
    //自动添加设备区域
    void initAutoWidget();
    //手动添加设备区域
    void initManualWidget();
    //设备信息区域
    void initDeviceComponents();
    //确定、取消
    void initBottomArea();
    //清除设备区域信息，把添加按钮置灰
    void clearDeviceArea();
    void clearDeviceComponent();
    //检测打印机名是否合法，由信号触发形参选true，带messagebox提示，直接调用选false
    bool checkPrinterNameIsLegal(bool hasMessageBox = false, const QString &printerName = QString());
    void updateForwardButtonStatus();
private:
    //---UI控件---start
    Ui::AddPrinterWindow *ui = nullptr;

    //父对象中心位置
    QPoint m_parentCenterPos;
    //添加打印机区域控件，一个整体
    QStackedWidget *m_centralWidget = nullptr;
    //手动添加打印机区域
    QWidget *m_manualWidget = nullptr;
    CustomLabel *m_manualLabel = nullptr;
    QLineEdit* m_addrEdit = nullptr;
    CustomPushButton* m_searchBtn = nullptr;
    CustomLabel* m_protocolLabel = nullptr;
    QComboBox* m_protocolComboBox = nullptr;
    QLabel* m_printerStatus = nullptr;

    // 自动添加打印机区域
    CustomRefreshButton* m_refreshBtn = nullptr;
    QWidget* m_autoWidget = nullptr;
    QTableView *m_autoTableView = nullptr;
    AutoSearchResultModel *m_autoSearchResultModel;
    SortFilterProxyModel *m_autoSearchSortFilterProxyModel;
    ManualSearchResultModel *m_manualSearchResultModel;
    CustomLabel* m_devList = nullptr;
    QLabel* m_loadingPic = nullptr;

    //设备参数区域，名称、位置、驱动
    CustomLabel *m_nameLabel = nullptr;
    CustomLabel *m_netLabel = nullptr;
    CustomLabel *m_locationLabel = nullptr;
    CustomLabel *m_drvLabel = nullptr;

    QLineEdit* m_nameEdit = nullptr;
    QLineEdit* m_locationEdit = nullptr;
    ChoosePpdComboBox* m_drvComboBox = nullptr;
    //确认，取消
    CustomPushButton *m_forwardBtn = nullptr;
    //---UI控件---End

    getUsbPrinterThread *usbThread = nullptr;

    //页面内部变量,带添加设备的信息,在点击可用设备按钮或者点击搜索按钮的时候设置
    DeviceInformation m_deviceInfo = DeviceInformation();
    //安装状态，0：安装成功，1：安装失败，2：已经安装
    int m_installOk = -1;
    bool m_needInstall = false;
    bool m_pkgHelperFlag = false;
    QRect m_parentRect = QRect();

    bool m_nameEditCheck = false;
    bool m_drvComboBoxCheck = false;
    QList<UkuiUsbPrinter> m_printers;
signals:
    void showThis(QPoint pos);
    //发送给主界面，通知主界面需要更新打印机列表，在窗口关闭前发送
    void needUpdateDests(QString);
    void installEnd();
private slots:
    //按键响应
    void onForwardClicked();
    void onShow(QPoint pos);
    void onSearchClicked();
    void handleDeviceFromIP(DeviceInformation &info);
    void onAutoClicked();
    void onManualClicked();
    void onDeviceUpdated(DeviceInformation& info);
    void onAddrChanged(const QString&);
    void onAutoDeviceAdd(DeviceInformation& info);
    void onManualDeviceAdd(DeviceInformation& info);
    //异步化，分成两部分处理
    //点击启动线程，线程中发送信号给主线程
    void onRefreshClicked();
    void onNameEditChanged();
    void showBaudRate();
    void hideBaudRate();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
};


#endif //__ADD_PRINTER_WINDOW_H__
